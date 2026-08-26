/* Copyright (c) 2001 Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file onion_crypto.c
 * \brief Functions to handle different kinds of circuit extension crypto.
 *
 * In this module, we provide a set of abstractions to create a uniform
 * interface over the circuit extension handshakes that Tor has used
 * over the years (CREATE_FAST, ntor, hs_ntor, and ntorv3).
 * These handshakes are implemented in the onion_*.c modules.
 *
 * All[*] of these handshakes follow a similar pattern: a client, knowing
 * some key from the relay it wants to extend through, generates the
 * first part of a handshake. A relay receives that handshake, and sends
 * a reply.  Once the client handles the reply, it knows that it is
 * talking to the right relay, and it shares some freshly negotiated key
 * material with that relay.
 *
 * We sometimes call the client's part of the handshake an "onionskin".
 * We do this because historically, Onion Routing used a multi-layer
 * structure called an "onion" to construct circuits. Each layer of the
 * onion contained key material chosen by the client, the identity of
 * the next relay in the circuit, and a smaller onion, encrypted with
 * the key of the next relay.  When we changed Tor to use a telescoping
 * circuit extension design, it corresponded to sending each layer of the
 * onion separately -- as a series of onionskins.
 **/

#include "core/or/or.h"
#include "core/or/extendinfo.h"
#include "core/crypto/onion_crypto.h"
#include "core/crypto/onion_fast.h"
#include "core/crypto/onion_ntor.h"
#include "core/crypto/onion_ntor_v3.h"
#include "feature/relay/router.h"
#include "lib/crypt_ops/crypto_dh.h"
#include "lib/crypt_ops/crypto_util.h"
#include "feature/relay/routerkeys.h"
#include "core/or/congestion_control_common.h"
#include "core/crypto/relay_crypto.h"
#include "core/or/protover.h"

#include "core/or/circuitbuild.h"

#include "core/or/crypt_path_st.h"
#include "core/or/extend_info_st.h"

#include "trunnel/congestion_control.h"
#include "trunnel/extension.h"
#include "trunnel/subproto_request.h"

#define EXT_TYPE_SUBPROTO 3

static const uint8_t NTOR3_CIRC_VERIFICATION[] = "circuit extend";
static const size_t NTOR3_CIRC_VERIFICATION_LEN = 14;

#define NTOR3_VERIFICATION_ARGS \
  NTOR3_CIRC_VERIFICATION, NTOR3_CIRC_VERIFICATION_LEN

/** Set `params` to a set of defaults.
 *
 * These defaults will only change later on if we're using a handshake that has
 * parameter negotiation. */
static void
circuit_params_init(circuit_params_t *params)
{
  memset(params, 0, sizeof(*params));
  params->crypto_alg = RELAY_CRYPTO_ALG_TOR1;
  params->cell_fmt = RELAY_CELL_FORMAT_V0;
}

/** Return a new server_onion_keys_t object with all of the keys
 * and other info we might need to do onion handshakes.  (We make a copy of
 * our keys for each cpuworker to avoid race conditions with the main thread,
 * and to avoid locking) */
server_onion_keys_t *
server_onion_keys_new(void)
{
  if (!get_master_identity_key())
    return NULL;

  server_onion_keys_t *keys = tor_malloc_zero(sizeof(server_onion_keys_t));
  memcpy(keys->my_identity, router_get_my_id_digest(), DIGEST_LEN);
  ed25519_pubkey_copy(&keys->my_ed_identity, get_master_identity_key());
  dup_onion_keys(&keys->onion_key, &keys->last_onion_key);
  keys->curve25519_key_map = construct_ntor_key_map();
  keys->junk_keypair = tor_malloc_zero(sizeof(curve25519_keypair_t));
  curve25519_keypair_generate(keys->junk_keypair, 0);
  return keys;
}
/** Release all storage held in <b>keys</b>. */
void
server_onion_keys_free_(server_onion_keys_t *keys)
{
  if (! keys)
    return;

  crypto_pk_free(keys->onion_key);
  crypto_pk_free(keys->last_onion_key);
  ntor_key_map_free(keys->curve25519_key_map);
  tor_free(keys->junk_keypair);
  memwipe(keys, 0, sizeof(server_onion_keys_t));
  tor_free(keys);
}

/** Release whatever storage is held in <b>state</b>, depending on its
 * type, and clear its pointer. */
void
onion_handshake_state_release(onion_handshake_state_t *state)
{
  switch (state->tag) {
  case ONION_HANDSHAKE_TYPE_TAP:
    break;
  case ONION_HANDSHAKE_TYPE_FAST:
    fast_handshake_state_free(state->u.fast);
    state->u.fast = NULL;
    break;
  case ONION_HANDSHAKE_TYPE_NTOR:
    ntor_handshake_state_free(state->u.ntor);
    state->u.ntor = NULL;
    break;
  case ONION_HANDSHAKE_TYPE_NTOR_V3:
    ntor3_handshake_state_free(state->u.ntor3);
    break;
  default:
    /* LCOV_EXCL_START
     * This state should not even exist. */
    log_warn(LD_BUG, "called with unknown handshake state type %d",
             (int)state->tag);
    tor_fragile_assert();
    /* LCOV_EXCL_STOP */
  }
}

/** Perform the first step of a circuit-creation handshake of type <b>type</b>
 * (one of ONION_HANDSHAKE_TYPE_*): generate the initial "onion skin" in
 * <b>onion_skin_out</b> with length of up to <b>onion_skin_out_maxlen</b>,
 * and store any state information in <b>state_out</b>.
 * Return -1 on failure, and the length of the onionskin on acceptance.
 */
int
onion_skin_create(int type,
                  const extend_info_t *node,
                  onion_handshake_state_t *state_out,
                  uint8_t *onion_skin_out,
                  size_t onion_skin_out_maxlen)
{
  int r = -1;

  circuit_params_init(&state_out->chosen_params);

  switch (type) {
  case ONION_HANDSHAKE_TYPE_TAP:
    return -1;
  case ONION_HANDSHAKE_TYPE_FAST:
    if (fast_onionskin_create(&state_out->u.fast, onion_skin_out) < 0)
      return -1;

    r = CREATE_FAST_LEN;
    break;
  case ONION_HANDSHAKE_TYPE_NTOR:
    if (onion_skin_out_maxlen < NTOR_ONIONSKIN_LEN)
      return -1;
   if (!extend_info_supports_ntor(node))
      return -1;
    if (onion_skin_ntor_create((const uint8_t*)node->identity_digest,
                               &node->curve25519_onion_key,
                               &state_out->u.ntor,
                               onion_skin_out) < 0)
      return -1;

    r = NTOR_ONIONSKIN_LEN;
    break;
  case ONION_HANDSHAKE_TYPE_NTOR_V3:
    if (!extend_info_supports_ntor_v3(node)) {
      log_warn(LD_BUG, "Chose ntorv3 handshake, but no support at node");
      return -1;
    }
    if (ed25519_public_key_is_zero(&node->ed_identity)) {
      log_warn(LD_BUG, "Chose ntorv3 handshake, but no ed id");
      return -1;
    }
    size_t msg_len = 0;
    uint8_t *msg = NULL;
    if (client_circ_negotiation_message(node, &msg, &msg_len,
                                        &state_out->chosen_params) < 0) {
      log_warn(LD_BUG, "Could not create circuit negotiation msg");
      return -1;
    }
    uint8_t *onion_skin = NULL;
    size_t onion_skin_len = 0;
    int status = onion_skin_ntor3_create(
                             &node->ed_identity,
                             &node->curve25519_onion_key,
                             NTOR3_VERIFICATION_ARGS,
                             msg, msg_len, /* client message */
                             &state_out->u.ntor3,
                             &onion_skin, &onion_skin_len);
    tor_free(msg);
    if (status < 0) {
      log_warn(LD_BUG, "onion skin create failed");
      return -1;
    }
    IF_BUG_ONCE(onion_skin_len > onion_skin_out_maxlen) {
      tor_free(onion_skin);
      return -1;
    }
    memcpy(onion_skin_out, onion_skin, onion_skin_len);
    tor_free(onion_skin);
    r = (int) onion_skin_len;
    break;

  default:
    /* LCOV_EXCL_START
     * We should never try to create an impossible handshake type. */
    log_warn(LD_BUG, "called with unknown handshake state type %d", type);
    tor_fragile_assert();
    r = -1;
    /* LCOV_EXCL_STOP */
  }

  if (r > 0)
    state_out->tag = (uint16_t) type;

  return r;
}

static bool
subproto_requests_in_order(const trn_subproto_request_t *a,
                           const trn_subproto_request_t *b)
{
  if (a->protocol_id < b->protocol_id) {
    return true;
  } else if (a->protocol_id == b->protocol_id) {
    return (a->proto_cap_number < b->proto_cap_number);
  } else {
    return false;
  }
}

/**
 * Process the SUBPROTO extension, as an OR.
 *
 * This extension declares one or more subproto capabilities that the
 * relay must implement, and tells it to enable them.
 */
static int
relay_process_subproto_ext(const trn_extension_t *ext,
                           circuit_params_t *params_out)
{
  const trn_extension_field_t *field;
  trn_subproto_request_ext_t *req = NULL;
  int res = -1;

  field = trn_extension_find(ext, EXT_TYPE_SUBPROTO);
  if (!field) {
    // Nothing to do.
    res = 0;
    goto done;
  }

  const uint8_t *f = trn_extension_field_getconstarray_field(field);
  size_t len = trn_extension_field_getlen_field(field);

  if (trn_subproto_request_ext_parse(&req, f, len) < 0) {
    goto done;
  }

  const trn_subproto_request_t *prev = NULL;
  size_t n_requests = trn_subproto_request_ext_getlen_reqs(req);
  for (unsigned i = 0; i < n_requests; ++i) {
    const trn_subproto_request_t *cur =
      trn_subproto_request_ext_getconst_reqs(req, i);
    if (prev && !subproto_requests_in_order(prev, cur)) {
      // The requests were not properly sorted and deduplicated.
      goto done;
    }

    if (cur->protocol_id == PRT_RELAY &&
        cur->proto_cap_number == PROTOVER_RELAY_CRYPT_CGO) {
      params_out->crypto_alg = RELAY_CRYPTO_ALG_CGO_RELAY;
      params_out->cell_fmt = RELAY_CELL_FORMAT_V1;
    } else {
      // Unless a protocol capability is explicitly supported for use
      // with this extension, we _must_ reject when it appears.
      goto done;
    }
  }

  res = 0;

 done:
  trn_subproto_request_ext_free(req);
  return res;
}

/**
 * Takes a param request message from the client, compares it to our
 * consensus parameters, and creates a reply message and output
 * parameters.
 *
 * This function runs in a worker thread, so it can only inspect
 * arguments and local variables.
 *
 * Returns 0 if successful.
 * Returns -1 on parsing, parameter failure, or reply creation failure.
 */
static int
negotiate_v3_ntor_server_circ_params(const uint8_t *param_request_msg,
                                     size_t param_request_len,
                                     const circuit_params_t *our_ns_params,
                                     circuit_params_t *params_out,
                                     uint8_t **resp_msg_out,
                                     size_t *resp_msg_len_out)
{
  int ret = -1;
  trn_extension_t *ext = NULL;

  ssize_t len =
    trn_extension_parse(&ext, param_request_msg, param_request_len);
  if (len < 0) {
    goto err;
  }

  /* Parse request. */
  ret = congestion_control_parse_ext_request(ext);
  if (ret < 0) {
    goto err;
  }
  params_out->cc_enabled = ret && our_ns_params->cc_enabled;
  ret = relay_process_subproto_ext(ext, params_out);
  if (ret < 0) {
    goto err;
  }

  /* Build the response. */
  ret = congestion_control_build_ext_response(our_ns_params, params_out,
                                              resp_msg_out, resp_msg_len_out);
  if (ret < 0) {
    goto err;
  }
  params_out->sendme_inc_cells = our_ns_params->sendme_inc_cells;

  if (params_out->cell_fmt != RELAY_CELL_FORMAT_V0 &&
      !params_out->cc_enabled) {
    // The V1 cell format is incompatible with pre-CC circuits,
    // since it has no way to encode stream-level SENDME messages.
    ret = -1;
    goto err;
  }

  /* Success. */
  ret = 0;

 err:
  trn_extension_free(ext);
  return ret;
}

/* This is the maximum value for keys_out_len passed to
 * onion_skin_server_handshake, plus 20 for the rend_nonce.
 * We can make it bigger if needed:
 * It just defines how many bytes to stack-allocate. */
#define MAX_KEYS_TMP_LEN (MAX_RELAY_KEY_MATERIAL_LEN + DIGEST_LEN)

/** Perform the second (server-side) step of a circuit-creation handshake of
 * type <b>type</b>, responding to the client request in <b>onion_skin</b>
 * using the keys in <b>keys</b>.  On success, write our response into
 * <b>reply_out</b>, generate <b>keys_out_len</b> bytes worth of key material
 * in <b>keys_out_len</b>, a hidden service nonce to <b>rend_nonce_out</b>,
 * and return the length of the reply. On failure, return -1.
 *
 * Requires that *keys_len_out of bytes are allocated at keys_out;
 * adjusts *keys_out_len to the number of bytes actually genarated.
 */
int
onion_skin_server_handshake(int type,
                      const uint8_t *onion_skin, size_t onionskin_len,
                      const server_onion_keys_t *keys,
                      const circuit_params_t *our_ns_params,
                      uint8_t *reply_out,
                      size_t reply_out_maxlen,
                      uint8_t *keys_out, size_t *keys_len_out,
                      uint8_t *rend_nonce_out,
                      circuit_params_t *params_out)
{
  int r = -1;

  relay_crypto_alg_t relay_alg = RELAY_CRYPTO_ALG_TOR1;
  size_t keys_out_needed = relay_crypto_key_material_len(relay_alg);

  circuit_params_init(params_out);

  switch (type) {
  case ONION_HANDSHAKE_TYPE_TAP:
    return -1;
  case ONION_HANDSHAKE_TYPE_FAST:
    if (reply_out_maxlen < CREATED_FAST_LEN)
      return -1;
    if (onionskin_len != CREATE_FAST_LEN)
      return -1;
    if (BUG(*keys_len_out < keys_out_needed)) {
      return -1;
    }
    if (fast_server_handshake(onion_skin, reply_out, keys_out,
                              keys_out_needed)<0)
      return -1;
    r = CREATED_FAST_LEN;
    memcpy(rend_nonce_out, reply_out+DIGEST_LEN, DIGEST_LEN);
    break;
  case ONION_HANDSHAKE_TYPE_NTOR:
    if (reply_out_maxlen < NTOR_REPLY_LEN)
      return -1;
    if (onionskin_len < NTOR_ONIONSKIN_LEN)
      return -1;
    if (BUG(*keys_len_out < keys_out_needed)) {
      return -1;
    }
    {
      size_t keys_tmp_len = keys_out_needed + DIGEST_LEN;
      tor_assert(keys_tmp_len <= MAX_KEYS_TMP_LEN);
      uint8_t keys_tmp[MAX_KEYS_TMP_LEN];

      if (onion_skin_ntor_server_handshake(
                                   onion_skin, keys->curve25519_key_map,
                                   keys->junk_keypair,
                                   keys->my_identity,
                                   reply_out, keys_tmp, keys_tmp_len)<0) {
        /* no need to memwipe here, since the output will never be used */
        return -1;
      }

      memcpy(keys_out, keys_tmp, keys_out_needed);
      memcpy(rend_nonce_out, keys_tmp+keys_out_needed, DIGEST_LEN);
      memwipe(keys_tmp, 0, sizeof(keys_tmp));
      r = NTOR_REPLY_LEN;
    }
    break;
  case ONION_HANDSHAKE_TYPE_NTOR_V3: {
    uint8_t keys_tmp[MAX_KEYS_TMP_LEN];
    uint8_t *client_msg = NULL;
    size_t client_msg_len = 0;
    uint8_t *reply_msg = NULL;
    size_t reply_msg_len = 0;

    ntor3_server_handshake_state_t *state = NULL;

    if (onion_skin_ntor3_server_handshake_part1(
               keys->curve25519_key_map,
               keys->junk_keypair,
               &keys->my_ed_identity,
               onion_skin, onionskin_len,
               NTOR3_VERIFICATION_ARGS,
               &client_msg, &client_msg_len,
               &state) < 0) {
      return -1;
    }

    if (negotiate_v3_ntor_server_circ_params(client_msg,
                                             client_msg_len,
                                             our_ns_params,
                                             params_out,
                                             &reply_msg,
                                             &reply_msg_len) < 0) {
      ntor3_server_handshake_state_free(state);
      tor_free(client_msg);
      return -1;
    }
    tor_free(client_msg);
    /* Now we know what we negotiated,
       so we can use the right lengths. */
    relay_alg = params_out->crypto_alg;
    keys_out_needed = relay_crypto_key_material_len(relay_alg);

    if (BUG(*keys_len_out < keys_out_needed)) {
      return -1;
    }
    size_t keys_tmp_len = keys_out_needed + DIGEST_LEN;
    tor_assert(keys_tmp_len <= MAX_KEYS_TMP_LEN);

    uint8_t *server_handshake = NULL;
    size_t server_handshake_len = 0;
    if (onion_skin_ntor3_server_handshake_part2(
               state,
               NTOR3_VERIFICATION_ARGS,
               reply_msg, reply_msg_len,
               &server_handshake, &server_handshake_len,
               keys_tmp, keys_tmp_len) < 0) {
      tor_free(reply_msg);
      ntor3_server_handshake_state_free(state);
      return -1;
    }
    tor_free(reply_msg);

    if (server_handshake_len > reply_out_maxlen) {
      tor_free(server_handshake);
      ntor3_server_handshake_state_free(state);
      return -1;
    }

    memcpy(keys_out, keys_tmp, keys_out_needed);
    memcpy(rend_nonce_out, keys_tmp+keys_out_needed, DIGEST_LEN);
    memcpy(reply_out, server_handshake, server_handshake_len);
    memwipe(keys_tmp, 0, keys_tmp_len);
    memwipe(server_handshake, 0, server_handshake_len);
    tor_free(server_handshake);
    ntor3_server_handshake_state_free(state);

    r = (int) server_handshake_len;
  }
    break;
  default:
    /* LCOV_EXCL_START
     * We should have rejected this far before this point */
    log_warn(LD_BUG, "called with unknown handshake state type %d", type);
    tor_fragile_assert();
    return -1;
    /* LCOV_EXCL_STOP */
  }
  *keys_len_out = keys_out_needed;

  return r;
}

/**
 * Takes a param response message from the exit, compares it to our
 * consensus parameters for sanity, and creates output parameters
 * if sane.
 *
 * Returns -1 on parsing or insane params, 0 if success.
 */
static int
negotiate_v3_ntor_client_circ_params(const uint8_t *param_response_msg,
                                     size_t param_response_len,
                                     circuit_params_t *params_out)
{
  int ret = -1;
  trn_extension_t *ext = NULL;

  ssize_t len =
    trn_extension_parse(&ext, param_response_msg, param_response_len);
  if (len < 0) {
    goto err;
  }

  ret = congestion_control_parse_ext_response(ext,
                                              params_out);
  if (ret < 0) {
    goto err;
  }

  /* If congestion control came back enabled, but we didn't ask for it
   * because the consensus said no, close the circuit.
   *
   * This is a fatal error condition for the circuit, because it either
   * means that congestion control was disabled by the consensus
   * during the handshake, or the exit decided to send us an unsolicited
   * congestion control response.
   *
   * In either case, we cannot proceed on this circuit, and must try a
   * new one.
   */
  if (ret && !congestion_control_enabled()) {
    goto err;
  }
  params_out->cc_enabled = ret;

 err:
  trn_extension_free(ext);
  return ret;
}

/** Perform the final (client-side) step of a circuit-creation handshake of
 * type <b>type</b>, using our state in <b>handshake_state</b> and the
 * server's response in <b>reply</b>. On success, generate an appropriate
 * amount of key material in <b>keys_out</b>,
 * set <b>keys_out_len</b> to the amount generated, set
 * <b>rend_authenticator_out</b> to the "KH" field that can be used to
 * establish introduction points at this hop, and return 0. On failure,
 * return -1, and set *msg_out to an error message if this is worth
 * complaining to the user about.
 *
 * Requires that *keys_len_out of bytes are allocated at keys_out;
 * adjusts *keys_out_len to the number of bytes actually genarated.
 */
int
onion_skin_client_handshake(int type,
                      const onion_handshake_state_t *handshake_state,
                      const uint8_t *reply, size_t reply_len,
                      uint8_t *keys_out, size_t *keys_len_out,
                      uint8_t *rend_authenticator_out,
                      circuit_params_t *params_out,
                      const char **msg_out)
{
  if (handshake_state->tag != type)
    return -1;

  memcpy(params_out, &handshake_state->chosen_params,
         sizeof(circuit_params_t));

  // at this point, we know the crypto algorithm we want to use
  relay_crypto_alg_t relay_alg = params_out->crypto_alg;
  size_t keys_out_needed = relay_crypto_key_material_len(relay_alg);
  if (BUG(*keys_len_out < keys_out_needed)) {
    return -1;
  }
  *keys_len_out = keys_out_needed;

  switch (type) {
  case ONION_HANDSHAKE_TYPE_TAP:
    return -1;
  case ONION_HANDSHAKE_TYPE_FAST:
    if (reply_len != CREATED_FAST_LEN) {
      if (msg_out)
        *msg_out = "TAP reply was not of the correct length.";
      return -1;
    }
    if (fast_client_handshake(handshake_state->u.fast, reply,
                              keys_out, keys_out_needed, msg_out) < 0)
      return -1;

    memcpy(rend_authenticator_out, reply+DIGEST_LEN, DIGEST_LEN);
    return 0;
  case ONION_HANDSHAKE_TYPE_NTOR:
    if (reply_len < NTOR_REPLY_LEN) {
      if (msg_out)
        *msg_out = "ntor reply was not of the correct length.";
      return -1;
    }
    {
      size_t keys_tmp_len = keys_out_needed + DIGEST_LEN;
      uint8_t *keys_tmp = tor_malloc(keys_tmp_len);
      if (onion_skin_ntor_client_handshake(handshake_state->u.ntor,
                                        reply,
                                        keys_tmp, keys_tmp_len, msg_out) < 0) {
        tor_free(keys_tmp);
        return -1;
      }
      memcpy(keys_out, keys_tmp, keys_out_needed);
      memcpy(rend_authenticator_out, keys_tmp + keys_out_needed, DIGEST_LEN);
      memwipe(keys_tmp, 0, keys_tmp_len);
      tor_free(keys_tmp);
    }
    return 0;
  case ONION_HANDSHAKE_TYPE_NTOR_V3: {
    size_t keys_tmp_len = keys_out_needed + DIGEST_LEN;
    uint8_t *keys_tmp = tor_malloc(keys_tmp_len);
    uint8_t *server_msg = NULL;
    size_t server_msg_len = 0;
    int r = onion_ntor3_client_handshake(
              handshake_state->u.ntor3,
              reply, reply_len,
              NTOR3_VERIFICATION_ARGS,
              keys_tmp, keys_tmp_len,
              &server_msg, &server_msg_len);
    if (r < 0) {
      tor_free(keys_tmp);
      tor_free(server_msg);
      return -1;
    }

    if (negotiate_v3_ntor_client_circ_params(server_msg,
                                             server_msg_len,
                                             params_out) < 0) {
      tor_free(keys_tmp);
      tor_free(server_msg);
      return -1;
    }
    tor_free(server_msg);

    memcpy(keys_out, keys_tmp, keys_out_needed);
    memcpy(rend_authenticator_out, keys_tmp + keys_out_needed, DIGEST_LEN);
    memwipe(keys_tmp, 0, keys_tmp_len);
    tor_free(keys_tmp);

    return 0;
  }
  default:
    log_warn(LD_BUG, "called with unknown handshake state type %d", type);
    tor_fragile_assert();
    return -1;
  }
}

/**
 * If there is an extension field of type `ext_type` in `ext`,
 * return that field.  Otherwise return NULL.
 */
const trn_extension_field_t *
trn_extension_find(const trn_extension_t *ext, uint8_t ext_type)
{
  IF_BUG_ONCE(!ext) {
    return NULL;
  }
  size_t n_fields = trn_extension_get_num(ext);
  if (n_fields == 0)
    return NULL;

  for (unsigned i = 0; i < n_fields; ++i) {
    const trn_extension_field_t *field = trn_extension_getconst_fields(ext, i);
    IF_BUG_ONCE(field == NULL) {
      return NULL;
    }
    if (trn_extension_field_get_field_type(field) == ext_type) {
      return field;
    }
  }

  return NULL;
}
