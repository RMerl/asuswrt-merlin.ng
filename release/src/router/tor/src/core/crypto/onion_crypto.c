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
 * interface over the three circuit extension handshakes that Tor has used
 * over the years (TAP, CREATE_FAST, and ntor).  These handshakes are
 * implemented in onion_tap.c, onion_fast.c, and onion_ntor.c respectively.
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
#include "core/crypto/onion_tap.h"
#include "feature/relay/router.h"
#include "lib/crypt_ops/crypto_dh.h"
#include "lib/crypt_ops/crypto_util.h"
#include "feature/relay/routerkeys.h"
#include "core/or/congestion_control_common.h"

#include "core/or/circuitbuild.h"

#include "core/or/crypt_path_st.h"
#include "core/or/extend_info_st.h"

#include "trunnel/congestion_control.h"
#include "trunnel/extension.h"

static const uint8_t NTOR3_CIRC_VERIFICATION[] = "circuit extend";
static const size_t NTOR3_CIRC_VERIFICATION_LEN = 14;

#define NTOR3_VERIFICATION_ARGS \
  NTOR3_CIRC_VERIFICATION, NTOR3_CIRC_VERIFICATION_LEN

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
    crypto_dh_free(state->u.tap);
    state->u.tap = NULL;
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

  switch (type) {
  case ONION_HANDSHAKE_TYPE_TAP:
    if (onion_skin_out_maxlen < TAP_ONIONSKIN_CHALLENGE_LEN)
      return -1;
    if (!node->onion_key)
      return -1;

    if (onion_skin_TAP_create(node->onion_key,
                              &state_out->u.tap,
                              (char*)onion_skin_out) < 0)
      return -1;

    r = TAP_ONIONSKIN_CHALLENGE_LEN;
    break;
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
    if (!extend_info_supports_ntor_v3(node))
      return -1;
    if (ed25519_public_key_is_zero(&node->ed_identity))
      return -1;
    size_t msg_len = 0;
    uint8_t *msg = NULL;
    if (client_circ_negotiation_message(node, &msg, &msg_len) < 0)
      return -1;
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
      return -1;
    }
    if (onion_skin_len > onion_skin_out_maxlen) {
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
  int ret;

  /* Parse request. */
  ret = congestion_control_parse_ext_request(param_request_msg,
                                             param_request_len);
  if (ret < 0) {
    goto err;
  }
  params_out->cc_enabled = ret && our_ns_params->cc_enabled;

  /* Build the response. */
  ret = congestion_control_build_ext_response(our_ns_params, params_out,
                                              resp_msg_out, resp_msg_len_out);
  if (ret < 0) {
    goto err;
  }
  params_out->sendme_inc_cells = our_ns_params->sendme_inc_cells;

  /* Success. */
  ret = 0;

 err:
  return ret;
}

/* This is the maximum value for keys_out_len passed to
 * onion_skin_server_handshake, plus 16. We can make it bigger if needed:
 * It just defines how many bytes to stack-allocate. */
#define MAX_KEYS_TMP_LEN 128

/** Perform the second (server-side) step of a circuit-creation handshake of
 * type <b>type</b>, responding to the client request in <b>onion_skin</b>
 * using the keys in <b>keys</b>.  On success, write our response into
 * <b>reply_out</b>, generate <b>keys_out_len</b> bytes worth of key material
 * in <b>keys_out_len</b>, a hidden service nonce to <b>rend_nonce_out</b>,
 * and return the length of the reply. On failure, return -1.
 */
int
onion_skin_server_handshake(int type,
                      const uint8_t *onion_skin, size_t onionskin_len,
                      const server_onion_keys_t *keys,
                      const circuit_params_t *our_ns_params,
                      uint8_t *reply_out,
                      size_t reply_out_maxlen,
                      uint8_t *keys_out, size_t keys_out_len,
                      uint8_t *rend_nonce_out,
                      circuit_params_t *params_out)
{
  int r = -1;
  memset(params_out, 0, sizeof(*params_out));

  switch (type) {
  case ONION_HANDSHAKE_TYPE_TAP:
    if (reply_out_maxlen < TAP_ONIONSKIN_REPLY_LEN)
      return -1;
    if (onionskin_len != TAP_ONIONSKIN_CHALLENGE_LEN)
      return -1;
    if (onion_skin_TAP_server_handshake((const char*)onion_skin,
                                        keys->onion_key, keys->last_onion_key,
                                        (char*)reply_out,
                                        (char*)keys_out, keys_out_len)<0)
      return -1;
    r = TAP_ONIONSKIN_REPLY_LEN;
    memcpy(rend_nonce_out, reply_out+DH1024_KEY_LEN, DIGEST_LEN);
    break;
  case ONION_HANDSHAKE_TYPE_FAST:
    if (reply_out_maxlen < CREATED_FAST_LEN)
      return -1;
    if (onionskin_len != CREATE_FAST_LEN)
      return -1;
    if (fast_server_handshake(onion_skin, reply_out, keys_out, keys_out_len)<0)
      return -1;
    r = CREATED_FAST_LEN;
    memcpy(rend_nonce_out, reply_out+DIGEST_LEN, DIGEST_LEN);
    break;
  case ONION_HANDSHAKE_TYPE_NTOR:
    if (reply_out_maxlen < NTOR_REPLY_LEN)
      return -1;
    if (onionskin_len < NTOR_ONIONSKIN_LEN)
      return -1;
    {
      size_t keys_tmp_len = keys_out_len + DIGEST_LEN;
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

      memcpy(keys_out, keys_tmp, keys_out_len);
      memcpy(rend_nonce_out, keys_tmp+keys_out_len, DIGEST_LEN);
      memwipe(keys_tmp, 0, sizeof(keys_tmp));
      r = NTOR_REPLY_LEN;
    }
    break;
  case ONION_HANDSHAKE_TYPE_NTOR_V3: {
    size_t keys_tmp_len = keys_out_len + DIGEST_LEN;
    tor_assert(keys_tmp_len <= MAX_KEYS_TMP_LEN);
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

    memcpy(keys_out, keys_tmp, keys_out_len);
    memcpy(rend_nonce_out, keys_tmp+keys_out_len, DIGEST_LEN);
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
  int ret = congestion_control_parse_ext_response(param_response_msg,
                                                  param_response_len,
                                                  params_out);
  if (ret < 0) {
    return -1;
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
    return -1;
  }
  params_out->cc_enabled = ret;

  return 0;
}

/** Perform the final (client-side) step of a circuit-creation handshake of
 * type <b>type</b>, using our state in <b>handshake_state</b> and the
 * server's response in <b>reply</b>. On success, generate <b>keys_out_len</b>
 * bytes worth of key material in <b>keys_out_len</b>, set
 * <b>rend_authenticator_out</b> to the "KH" field that can be used to
 * establish introduction points at this hop, and return 0. On failure,
 * return -1, and set *msg_out to an error message if this is worth
 * complaining to the user about. */
int
onion_skin_client_handshake(int type,
                      const onion_handshake_state_t *handshake_state,
                      const uint8_t *reply, size_t reply_len,
                      uint8_t *keys_out, size_t keys_out_len,
                      uint8_t *rend_authenticator_out,
                      circuit_params_t *params_out,
                      const char **msg_out)
{
  if (handshake_state->tag != type)
    return -1;

  memset(params_out, 0, sizeof(*params_out));

  switch (type) {
  case ONION_HANDSHAKE_TYPE_TAP:
    if (reply_len != TAP_ONIONSKIN_REPLY_LEN) {
      if (msg_out)
        *msg_out = "TAP reply was not of the correct length.";
      return -1;
    }
    if (onion_skin_TAP_client_handshake(handshake_state->u.tap,
                                        (const char*)reply,
                                        (char *)keys_out, keys_out_len,
                                        msg_out) < 0)
      return -1;

    memcpy(rend_authenticator_out, reply+DH1024_KEY_LEN, DIGEST_LEN);

    return 0;
  case ONION_HANDSHAKE_TYPE_FAST:
    if (reply_len != CREATED_FAST_LEN) {
      if (msg_out)
        *msg_out = "TAP reply was not of the correct length.";
      return -1;
    }
    if (fast_client_handshake(handshake_state->u.fast, reply,
                              keys_out, keys_out_len, msg_out) < 0)
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
      size_t keys_tmp_len = keys_out_len + DIGEST_LEN;
      uint8_t *keys_tmp = tor_malloc(keys_tmp_len);
      if (onion_skin_ntor_client_handshake(handshake_state->u.ntor,
                                        reply,
                                        keys_tmp, keys_tmp_len, msg_out) < 0) {
        tor_free(keys_tmp);
        return -1;
      }
      memcpy(keys_out, keys_tmp, keys_out_len);
      memcpy(rend_authenticator_out, keys_tmp + keys_out_len, DIGEST_LEN);
      memwipe(keys_tmp, 0, keys_tmp_len);
      tor_free(keys_tmp);
    }
    return 0;
  case ONION_HANDSHAKE_TYPE_NTOR_V3: {
    size_t keys_tmp_len = keys_out_len + DIGEST_LEN;
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

    memcpy(keys_out, keys_tmp, keys_out_len);
    memcpy(rend_authenticator_out, keys_tmp + keys_out_len, DIGEST_LEN);
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
