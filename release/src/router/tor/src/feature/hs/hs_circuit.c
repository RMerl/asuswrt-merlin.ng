/* Copyright (c) 2017-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file hs_circuit.c
 **/

#define HS_CIRCUIT_PRIVATE

#include "core/or/or.h"
#include "app/config/config.h"
#include "core/crypto/hs_ntor.h"
#include "core/or/circuitbuild.h"
#include "core/or/circuitlist.h"
#include "core/or/circuituse.h"
#include "core/or/policies.h"
#include "core/or/relay.h"
#include "core/or/crypt_path.h"
#include "core/or/extendinfo.h"
#include "core/or/congestion_control_common.h"
#include "core/crypto/onion_crypto.h"
#include "feature/client/circpathbias.h"
#include "feature/hs/hs_cell.h"
#include "feature/hs/hs_circuit.h"
#include "feature/hs/hs_ob.h"
#include "feature/hs/hs_circuitmap.h"
#include "feature/hs/hs_client.h"
#include "feature/hs/hs_ident.h"
#include "feature/hs/hs_metrics.h"
#include "feature/hs/hs_service.h"
#include "feature/nodelist/describe.h"
#include "feature/nodelist/nodelist.h"
#include "feature/stats/rephist.h"
#include "lib/crypt_ops/crypto_dh.h"
#include "lib/crypt_ops/crypto_rand.h"
#include "lib/crypt_ops/crypto_util.h"

/* Trunnel. */
#include "trunnel/ed25519_cert.h"
#include "trunnel/hs/cell_establish_intro.h"

#include "core/or/congestion_control_st.h"
#include "core/or/cpath_build_state_st.h"
#include "core/or/crypt_path_st.h"
#include "feature/nodelist/node_st.h"
#include "core/or/origin_circuit_st.h"

/** A circuit is about to become an e2e rendezvous circuit. Check
 * <b>circ_purpose</b> and ensure that it's properly set. Return true iff
 * circuit purpose is properly set, otherwise return false. */
static int
circuit_purpose_is_correct_for_rend(unsigned int circ_purpose,
                                    int is_service_side)
{
  if (is_service_side) {
    if (circ_purpose != CIRCUIT_PURPOSE_S_CONNECT_REND) {
      log_warn(LD_BUG,
            "HS e2e circuit setup with wrong purpose (%d)", circ_purpose);
      return 0;
    }
  }

  if (!is_service_side) {
    if (circ_purpose != CIRCUIT_PURPOSE_C_REND_READY &&
        circ_purpose != CIRCUIT_PURPOSE_C_REND_READY_INTRO_ACKED) {
      log_warn(LD_BUG,
            "Client e2e circuit setup with wrong purpose (%d)", circ_purpose);
      return 0;
    }
  }

  return 1;
}

/** Create and return a crypt path for the final hop of a v3 prop224 rendezvous
 * circuit. Initialize the crypt path crypto using the output material from the
 * ntor key exchange at <b>ntor_key_seed</b>.
 *
 * If <b>is_service_side</b> is set, we are the hidden service and the final
 * hop of the rendezvous circuit is the client on the other side. */
static crypt_path_t *
create_rend_cpath(const uint8_t *ntor_key_seed, size_t seed_len,
                  int is_service_side)
{
  uint8_t keys[HS_NTOR_KEY_EXPANSION_KDF_OUT_LEN];
  crypt_path_t *cpath = NULL;

  /* Do the key expansion */
  if (hs_ntor_circuit_key_expansion(ntor_key_seed, seed_len,
                                    keys, sizeof(keys)) < 0) {
    goto err;
  }

  /* Setup the cpath */
  cpath = tor_malloc_zero(sizeof(crypt_path_t));
  cpath->magic = CRYPT_PATH_MAGIC;

  if (cpath_init_circuit_crypto(cpath, (char*)keys, sizeof(keys),
                                is_service_side, 1) < 0) {
    tor_free(cpath);
    goto err;
  }

 err:
  memwipe(keys, 0, sizeof(keys));
  return cpath;
}

/** Append the final <b>hop</b> to the cpath of the rend <b>circ</b>, and mark
 * <b>circ</b> ready for use to transfer HS relay cells. */
static void
finalize_rend_circuit(origin_circuit_t *circ, crypt_path_t *hop,
                      int is_service_side)
{
  tor_assert(circ);
  tor_assert(hop);

  /* Notify the circuit state machine that we are splicing this circuit */
  int new_circ_purpose = is_service_side ?
    CIRCUIT_PURPOSE_S_REND_JOINED : CIRCUIT_PURPOSE_C_REND_JOINED;
  circuit_change_purpose(TO_CIRCUIT(circ), new_circ_purpose);

  /* All is well. Extend the circuit. */
  hop->state = CPATH_STATE_OPEN;
  /* Set the windows to default. */
  hop->package_window = circuit_initial_package_window();
  hop->deliver_window = CIRCWINDOW_START;

  /* Now that this circuit has finished connecting to its destination,
   * make sure circuit_get_open_circ_or_launch is willing to return it
   * so we can actually use it. */
  circ->hs_circ_has_timed_out = 0;

  /* If congestion control, transfer ccontrol onto the cpath. */
  if (TO_CIRCUIT(circ)->ccontrol) {
    hop->ccontrol = TO_CIRCUIT(circ)->ccontrol;
    TO_CIRCUIT(circ)->ccontrol = NULL;
  }

  /* Append the hop to the cpath of this circuit */
  cpath_extend_linked_list(&circ->cpath, hop);

  /* Finally, mark circuit as ready to be used for client streams */
  if (!is_service_side) {
    circuit_try_attaching_streams(circ);
  }
}

/** For a given circuit and a service introduction point object, register the
 * intro circuit to the circuitmap. */
static void
register_intro_circ(const hs_service_intro_point_t *ip,
                    origin_circuit_t *circ)
{
  tor_assert(ip);
  tor_assert(circ);

  hs_circuitmap_register_intro_circ_v3_service_side(circ,
                                                    &ip->auth_key_kp.pubkey);
}

/** Return the number of opened introduction circuit for the given circuit that
 * is matching its identity key. */
static unsigned int
count_opened_desc_intro_point_circuits(const hs_service_t *service,
                                       const hs_service_descriptor_t *desc)
{
  unsigned int count = 0;

  tor_assert(service);
  tor_assert(desc);

  DIGEST256MAP_FOREACH(desc->intro_points.map, key,
                       const hs_service_intro_point_t *, ip) {
    const circuit_t *circ;
    const origin_circuit_t *ocirc = hs_circ_service_get_intro_circ(ip);
    if (ocirc == NULL) {
      continue;
    }
    circ = TO_CIRCUIT(ocirc);
    tor_assert(circ->purpose == CIRCUIT_PURPOSE_S_ESTABLISH_INTRO ||
               circ->purpose == CIRCUIT_PURPOSE_S_INTRO);
    /* Having a circuit not for the requested service is really bad. */
    tor_assert(ed25519_pubkey_eq(&service->keys.identity_pk,
                                 &ocirc->hs_ident->identity_pk));
    /* Only count opened circuit and skip circuit that will be closed. */
    if (!circ->marked_for_close && circ->state == CIRCUIT_STATE_OPEN) {
      count++;
    }
  } DIGEST256MAP_FOREACH_END;
  return count;
}

/** From a given service, rendezvous cookie and handshake info, create a
 * rendezvous point circuit identifier. This can't fail. */
STATIC hs_ident_circuit_t *
create_rp_circuit_identifier(const hs_service_t *service,
                             const uint8_t *rendezvous_cookie,
                             const curve25519_public_key_t *server_pk,
                             const hs_ntor_rend_cell_keys_t *keys)
{
  hs_ident_circuit_t *ident;
  uint8_t handshake_info[CURVE25519_PUBKEY_LEN + DIGEST256_LEN];

  tor_assert(service);
  tor_assert(rendezvous_cookie);
  tor_assert(server_pk);
  tor_assert(keys);

  ident = hs_ident_circuit_new(&service->keys.identity_pk);
  /* Copy the RENDEZVOUS_COOKIE which is the unique identifier. */
  memcpy(ident->rendezvous_cookie, rendezvous_cookie,
         sizeof(ident->rendezvous_cookie));
  /* Build the HANDSHAKE_INFO which looks like this:
   *    SERVER_PK        [32 bytes]
   *    AUTH_INPUT_MAC   [32 bytes]
   */
  memcpy(handshake_info, server_pk->public_key, CURVE25519_PUBKEY_LEN);
  memcpy(handshake_info + CURVE25519_PUBKEY_LEN, keys->rend_cell_auth_mac,
         DIGEST256_LEN);
  tor_assert(sizeof(ident->rendezvous_handshake_info) ==
             sizeof(handshake_info));
  memcpy(ident->rendezvous_handshake_info, handshake_info,
         sizeof(ident->rendezvous_handshake_info));
  /* Finally copy the NTOR_KEY_SEED for e2e encryption on the circuit. */
  tor_assert(sizeof(ident->rendezvous_ntor_key_seed) ==
             sizeof(keys->ntor_key_seed));
  memcpy(ident->rendezvous_ntor_key_seed, keys->ntor_key_seed,
         sizeof(ident->rendezvous_ntor_key_seed));
  return ident;
}

/** From a given service and service intro point, create an introduction point
 * circuit identifier. This can't fail. */
static hs_ident_circuit_t *
create_intro_circuit_identifier(const hs_service_t *service,
                                const hs_service_intro_point_t *ip)
{
  hs_ident_circuit_t *ident;

  tor_assert(service);
  tor_assert(ip);

  ident = hs_ident_circuit_new(&service->keys.identity_pk);
  ed25519_pubkey_copy(&ident->intro_auth_pk, &ip->auth_key_kp.pubkey);

  return ident;
}

/** For a given introduction point and an introduction circuit, send the
 * ESTABLISH_INTRO cell. The service object is used for logging. This can fail
 * and if so, the circuit is closed and the intro point object is flagged
 * that the circuit is not established anymore which is important for the
 * retry mechanism. */
static void
send_establish_intro(const hs_service_t *service,
                     hs_service_intro_point_t *ip, origin_circuit_t *circ)
{
  ssize_t cell_len;
  uint8_t payload[RELAY_PAYLOAD_SIZE];

  tor_assert(service);
  tor_assert(ip);
  tor_assert(circ);

  /* Encode establish intro cell. */
  cell_len = hs_cell_build_establish_intro(circ->cpath->prev->rend_circ_nonce,
                                           &service->config, ip, payload);
  if (cell_len < 0) {
    log_warn(LD_REND, "Unable to encode ESTABLISH_INTRO cell for service %s "
                      "on circuit %u. Closing circuit.",
             safe_str_client(service->onion_address),
             TO_CIRCUIT(circ)->n_circ_id);
    goto err;
  }

  /* Send the cell on the circuit. */
  if (relay_send_command_from_edge(CONTROL_CELL_ID, TO_CIRCUIT(circ),
                                   RELAY_COMMAND_ESTABLISH_INTRO,
                                   (char *) payload, cell_len,
                                   circ->cpath->prev) < 0) {
    log_info(LD_REND, "Unable to send ESTABLISH_INTRO cell for service %s "
                      "on circuit %u.",
             safe_str_client(service->onion_address),
             TO_CIRCUIT(circ)->n_circ_id);
    /* On error, the circuit has been closed. */
    goto done;
  }

  /* Record the attempt to use this circuit. */
  pathbias_count_use_attempt(circ);
  goto done;

 err:
  circuit_mark_for_close(TO_CIRCUIT(circ), END_CIRC_REASON_INTERNAL);
 done:
  memwipe(payload, 0, sizeof(payload));
}

/** Return a string constant describing the anonymity of service. */
static const char *
get_service_anonymity_string(const hs_service_t *service)
{
  if (service->config.is_single_onion) {
    return "single onion";
  } else {
    return "hidden";
  }
}

/** For a given service, the ntor onion key and a rendezvous cookie, launch a
 * circuit to the rendezvous point specified by the link specifiers. On
 * success, a circuit identifier is attached to the circuit with the needed
 * data. This function will try to open a circuit for a maximum value of
 * MAX_REND_FAILURES then it will give up. */
MOCK_IMPL(STATIC void,
launch_rendezvous_point_circuit,(const hs_service_t *service,
                                 const hs_service_intro_point_t *ip,
                                 const hs_cell_introduce2_data_t *data))
{
  int circ_needs_uptime;
  time_t now = time(NULL);
  extend_info_t *info = NULL;
  origin_circuit_t *circ;

  tor_assert(service);
  tor_assert(ip);
  tor_assert(data);

  circ_needs_uptime = hs_service_requires_uptime_circ(service->config.ports);

  /* Get the extend info data structure for the chosen rendezvous point
   * specified by the given link specifiers. */
  info = hs_get_extend_info_from_lspecs(data->link_specifiers,
                                        &data->onion_pk,
                                        service->config.is_single_onion);
  if (info == NULL) {
    /* We are done here, we can't extend to the rendezvous point. */
    log_fn(LOG_PROTOCOL_WARN, LD_REND,
           "Not enough info to open a circuit to a rendezvous point for "
           "%s service %s.",
           get_service_anonymity_string(service),
           safe_str_client(service->onion_address));
    goto end;
  }

  for (int i = 0; i < MAX_REND_FAILURES; i++) {
    int circ_flags = CIRCLAUNCH_NEED_CAPACITY | CIRCLAUNCH_IS_INTERNAL;
    if (circ_needs_uptime) {
      circ_flags |= CIRCLAUNCH_NEED_UPTIME;
    }
    /* Firewall and policies are checked when getting the extend info.
     *
     * We only use a one-hop path on the first attempt. If the first attempt
     * fails, we use a 3-hop path for reachability / reliability.
     * See the comment in retry_service_rendezvous_point() for details. */
    if (service->config.is_single_onion && i == 0) {
      circ_flags |= CIRCLAUNCH_ONEHOP_TUNNEL;
    }

    circ = circuit_launch_by_extend_info(CIRCUIT_PURPOSE_S_CONNECT_REND, info,
                                         circ_flags);
    if (circ != NULL) {
      /* Stop retrying, we have a circuit! */
      break;
    }
  }
  if (circ == NULL) {
    log_warn(LD_REND, "Giving up on launching a rendezvous circuit to %s "
                      "for %s service %s",
             safe_str_client(extend_info_describe(info)),
             get_service_anonymity_string(service),
             safe_str_client(service->onion_address));
    goto end;
  }
  /* Update metrics with this new rendezvous circuit launched. */
  hs_metrics_new_rdv(&service->keys.identity_pk);

  log_info(LD_REND, "Rendezvous circuit launched to %s with cookie %s "
                    "for %s service %s",
           safe_str_client(extend_info_describe(info)),
           safe_str_client(hex_str((const char *) data->rendezvous_cookie,
                                   REND_COOKIE_LEN)),
           get_service_anonymity_string(service),
           safe_str_client(service->onion_address));
  tor_assert(circ->build_state);
  /* Rendezvous circuit have a specific timeout for the time spent on trying
   * to connect to the rendezvous point. */
  circ->build_state->expiry_time = now + MAX_REND_TIMEOUT;

  /* Create circuit identifier and key material. */
  {
    hs_ntor_rend_cell_keys_t keys;
    curve25519_keypair_t ephemeral_kp;
    /* No need for extra strong, this is only for this circuit life time. This
     * key will be used for the RENDEZVOUS1 cell that will be sent on the
     * circuit once opened. */
    curve25519_keypair_generate(&ephemeral_kp, 0);
    if (hs_ntor_service_get_rendezvous1_keys(&ip->auth_key_kp.pubkey,
                                             &ip->enc_key_kp,
                                             &ephemeral_kp, &data->client_pk,
                                             &keys) < 0) {
      /* This should not really happened but just in case, don't make tor
       * freak out, close the circuit and move on. */
      log_info(LD_REND, "Unable to get RENDEZVOUS1 key material for "
                        "service %s",
               safe_str_client(service->onion_address));
      circuit_mark_for_close(TO_CIRCUIT(circ), END_CIRC_REASON_INTERNAL);
      goto end;
    }
    circ->hs_ident = create_rp_circuit_identifier(service,
                                                  data->rendezvous_cookie,
                                                  &ephemeral_kp.pubkey, &keys);
    memwipe(&ephemeral_kp, 0, sizeof(ephemeral_kp));
    memwipe(&keys, 0, sizeof(keys));
    tor_assert(circ->hs_ident);
  }

  /* Setup congestion control if asked by the client from the INTRO cell. */
  if (data->cc_enabled) {
    hs_circ_setup_congestion_control(circ, congestion_control_sendme_inc(),
                                     service->config.is_single_onion);
  }

 end:
  extend_info_free(info);
}

/** Return true iff the given service rendezvous circuit circ is allowed for a
 * relaunch to the rendezvous point. */
static int
can_relaunch_service_rendezvous_point(const origin_circuit_t *circ)
{
  tor_assert(circ);
  /* This is initialized when allocating an origin circuit. */
  tor_assert(circ->build_state);
  tor_assert(TO_CIRCUIT(circ)->purpose == CIRCUIT_PURPOSE_S_CONNECT_REND);

  /* XXX: Retrying under certain condition. This is related to #22455. */

  /* Avoid to relaunch twice a circuit to the same rendezvous point at the
   * same time. */
  if (circ->hs_service_side_rend_circ_has_been_relaunched) {
    log_info(LD_REND, "Rendezvous circuit to %s has already been retried. "
                      "Skipping retry.",
             safe_str_client(
                  extend_info_describe(circ->build_state->chosen_exit)));
    goto disallow;
  }

  /* We check failure_count >= hs_get_service_max_rend_failures()-1 below, and
   * the -1 is because we increment the failure count for our current failure
   * *after* this clause. */
  int max_rend_failures = hs_get_service_max_rend_failures() - 1;

  /* A failure count that has reached maximum allowed or circuit that expired,
   * we skip relaunching. */
  if (circ->build_state->failure_count > max_rend_failures ||
      circ->build_state->expiry_time <= time(NULL)) {
    log_info(LD_REND, "Attempt to build a rendezvous circuit to %s has "
                      "failed with %d attempts and expiry time %ld. "
                      "Giving up building.",
             safe_str_client(
                  extend_info_describe(circ->build_state->chosen_exit)),
             circ->build_state->failure_count,
             (long int) circ->build_state->expiry_time);
    goto disallow;
  }

  /* Allowed to relaunch. */
  return 1;
 disallow:
  return 0;
}

/** Retry the rendezvous point of circ by launching a new circuit to it. */
static void
retry_service_rendezvous_point(const origin_circuit_t *circ)
{
  int flags = 0;
  origin_circuit_t *new_circ;
  cpath_build_state_t *bstate;

  tor_assert(circ);
  /* This is initialized when allocating an origin circuit. */
  tor_assert(circ->build_state);
  tor_assert(TO_CIRCUIT(circ)->purpose == CIRCUIT_PURPOSE_S_CONNECT_REND);

  /* Ease our life. */
  bstate = circ->build_state;

  log_info(LD_REND, "Retrying rendezvous point circuit to %s",
           safe_str_client(extend_info_describe(bstate->chosen_exit)));

  /* Get the current build state flags for the next circuit. */
  flags |= (bstate->need_uptime) ? CIRCLAUNCH_NEED_UPTIME : 0;
  flags |= (bstate->need_capacity) ? CIRCLAUNCH_NEED_CAPACITY : 0;
  flags |= (bstate->is_internal) ? CIRCLAUNCH_IS_INTERNAL : 0;

  /* We do NOT add the onehop tunnel flag even though it might be a single
   * onion service. The reason is that if we failed once to connect to the RP
   * with a direct connection, we consider that chances are that we will fail
   * again so try a 3-hop circuit and hope for the best. Because the service
   * has no anonymity (single onion), this change of behavior won't affect
   * security directly. */

  new_circ = circuit_launch_by_extend_info(CIRCUIT_PURPOSE_S_CONNECT_REND,
                                           bstate->chosen_exit, flags);
  if (new_circ == NULL) {
    log_warn(LD_REND, "Failed to launch rendezvous circuit to %s",
             safe_str_client(extend_info_describe(bstate->chosen_exit)));
    goto done;
  }

  /* Transfer build state information to the new circuit state in part to
   * catch any other failures. */
  new_circ->build_state->failure_count = bstate->failure_count+1;
  new_circ->build_state->expiry_time = bstate->expiry_time;
  new_circ->hs_ident = hs_ident_circuit_dup(circ->hs_ident);

  /* Setup congestion control if asked by the client from the INTRO cell. */
  if (TO_CIRCUIT(circ)->ccontrol) {
    /* As per above, in this case, we are a full 3 hop rend, even if we're a
     * single-onion service. */
    hs_circ_setup_congestion_control(new_circ,
                                     TO_CIRCUIT(circ)->ccontrol->sendme_inc,
                                     false);
  }

 done:
  return;
}

/** Using the given descriptor intro point ip, the node of the
 * rendezvous point rp_node and the service's subcredential, populate the
 * already allocated intro1_data object with the needed key material and link
 * specifiers.
 *
 * Return 0 on success or a negative value if we couldn't properly filled the
 * introduce1 data from the RP node. In other word, it means the RP node is
 * unusable to use in the introduction. */
static int
setup_introduce1_data(const hs_desc_intro_point_t *ip,
                      const node_t *rp_node,
                      const hs_subcredential_t *subcredential,
                      hs_cell_introduce1_data_t *intro1_data)
{
  int ret = -1;
  smartlist_t *rp_lspecs;

  tor_assert(ip);
  tor_assert(rp_node);
  tor_assert(subcredential);
  tor_assert(intro1_data);

  /* Build the link specifiers from the node at the end of the rendezvous
   * circuit that we opened for this introduction. */
  rp_lspecs = node_get_link_specifier_smartlist(rp_node, 0);
  if (smartlist_len(rp_lspecs) == 0) {
    /* We can't rendezvous without link specifiers. */
    smartlist_free(rp_lspecs);
    goto end;
  }

  /* Populate the introduce1 data object. */
  memset(intro1_data, 0, sizeof(hs_cell_introduce1_data_t));
  intro1_data->auth_pk = &ip->auth_key_cert->signed_key;
  intro1_data->enc_pk = &ip->enc_key;
  intro1_data->subcredential = subcredential;
  intro1_data->link_specifiers = rp_lspecs;
  intro1_data->onion_pk = node_get_curve25519_onion_key(rp_node);
  if (intro1_data->onion_pk == NULL) {
    /* We can't rendezvous without the curve25519 onion key. */
    goto end;
  }

  /* Success, we have valid introduce data. */
  ret = 0;

 end:
  return ret;
}

/** Helper: cleanup function for client circuit. This is for every HS version.
 * It is called from hs_circ_cleanup_on_close() entry point. */
static void
cleanup_on_close_client_circ(circuit_t *circ)
{
  tor_assert(circ);

  if (circuit_is_hs_v3(circ)) {
    hs_client_circuit_cleanup_on_close(circ);
  }
  /* It is possible the circuit has an HS purpose but no identifier (hs_ident).
   * Thus possible that this passes through. */
}

/** Helper: cleanup function for client circuit. This is for every HS version.
 * It is called from hs_circ_cleanup_on_free() entry point. */
static void
cleanup_on_free_client_circ(circuit_t *circ)
{
  tor_assert(circ);

  if (circuit_is_hs_v3(circ)) {
    hs_client_circuit_cleanup_on_free(circ);
  }
  /* It is possible the circuit has an HS purpose but no identifier (hs_ident).
   * Thus possible that this passes through. */
}

/* ========== */
/* Public API */
/* ========== */

/** Setup on the given circuit congestion control with the given parameters.
 *
 * This function assumes that congestion control is enabled on the network and
 * so it is the caller responsability to make sure of it. */
void
hs_circ_setup_congestion_control(origin_circuit_t *origin_circ,
                                 uint8_t sendme_inc, bool is_single_onion)
{
  circuit_t *circ = NULL;
  circuit_params_t circ_params = {0};

  tor_assert(origin_circ);

  /* Ease our lives */
  circ = TO_CIRCUIT(origin_circ);

  circ_params.cc_enabled = true;
  circ_params.sendme_inc_cells = sendme_inc;

  /* It is setup on the circuit in order to indicate that congestion control is
   * enabled. It will be transferred to the RP crypt_path_t once the handshake
   * is finalized in finalize_rend_circuit() for both client and service
   * because the final hop is not available until then. */

  if (is_single_onion) {
    circ->ccontrol = congestion_control_new(&circ_params, CC_PATH_ONION_SOS);
  } else {
    if (get_options()->HSLayer3Nodes) {
      circ->ccontrol = congestion_control_new(&circ_params, CC_PATH_ONION_VG);
    } else {
      circ->ccontrol = congestion_control_new(&circ_params, CC_PATH_ONION);
    }
  }
}

/** Return an introduction point circuit matching the given intro point object.
 * NULL is returned is no such circuit can be found. */
origin_circuit_t *
hs_circ_service_get_intro_circ(const hs_service_intro_point_t *ip)
{
  tor_assert(ip);

  return hs_circuitmap_get_intro_circ_v3_service_side(&ip->auth_key_kp.pubkey);
}

/** Return an introduction point established circuit matching the given intro
 * point object. The circuit purpose has to be CIRCUIT_PURPOSE_S_INTRO. NULL
 * is returned is no such circuit can be found. */
origin_circuit_t *
hs_circ_service_get_established_intro_circ(const hs_service_intro_point_t *ip)
{
  origin_circuit_t *circ;

  tor_assert(ip);

  circ = hs_circuitmap_get_intro_circ_v3_service_side(&ip->auth_key_kp.pubkey);

  /* Only return circuit if it is established. */
  return (circ && TO_CIRCUIT(circ)->purpose == CIRCUIT_PURPOSE_S_INTRO) ?
          circ : NULL;
}

/** Called when we fail building a rendezvous circuit at some point other than
 * the last hop: launches a new circuit to the same rendezvous point.
 *
 * We currently relaunch connections to rendezvous points if:
 * - A rendezvous circuit timed out before connecting to RP.
 * - The rendezvous circuit failed to connect to the RP.
 *
 * We avoid relaunching a connection to this rendezvous point if:
 * - We have already tried MAX_REND_FAILURES times to connect to this RP,
 * - We've been trying to connect to this RP for more than MAX_REND_TIMEOUT
 *   seconds, or
 * - We've already retried this specific rendezvous circuit.
 */
void
hs_circ_retry_service_rendezvous_point(origin_circuit_t *circ)
{
  tor_assert(circ);
  tor_assert(TO_CIRCUIT(circ)->purpose == CIRCUIT_PURPOSE_S_CONNECT_REND);

  /* Check if we are allowed to relaunch to the rendezvous point of circ. */
  if (!can_relaunch_service_rendezvous_point(circ)) {
    goto done;
  }

  /* Flag the circuit that we are relaunching, to avoid to relaunch twice a
   * circuit to the same rendezvous point at the same time. */
  circ->hs_service_side_rend_circ_has_been_relaunched = 1;

  /* Legacy services don't have a hidden service ident. */
  if (circ->hs_ident) {
    retry_service_rendezvous_point(circ);
  }

 done:
  return;
}

/** For a given service and a service intro point, launch a circuit to the
 * extend info ei. If the service is a single onion, and direct_conn is true,
 * a one-hop circuit will be requested.
 *
 * Return 0 if the circuit was successfully launched and tagged
 * with the correct identifier. On error, a negative value is returned. */
int
hs_circ_launch_intro_point(hs_service_t *service,
                           const hs_service_intro_point_t *ip,
                           extend_info_t *ei,
                           bool direct_conn)
{
  /* Standard flags for introduction circuit. */
  int ret = -1, circ_flags = CIRCLAUNCH_NEED_UPTIME | CIRCLAUNCH_IS_INTERNAL;
  origin_circuit_t *circ;

  tor_assert(service);
  tor_assert(ip);
  tor_assert(ei);

  /* Update circuit flags in case of a single onion service that requires a
   * direct connection. */
  tor_assert_nonfatal(ip->circuit_retries > 0);
  /* Only single onion services can make direct conns */
  if (BUG(!service->config.is_single_onion && direct_conn)) {
    goto end;
  }
  /* We only use a one-hop path on the first attempt. If the first attempt
   * fails, we use a 3-hop path for reachability / reliability. */
  if (direct_conn && ip->circuit_retries == 1) {
    circ_flags |= CIRCLAUNCH_ONEHOP_TUNNEL;
  }

  log_info(LD_REND, "Launching a circuit to intro point %s for service %s.",
           safe_str_client(extend_info_describe(ei)),
           safe_str_client(service->onion_address));

  /* Note down the launch for the retry period. Even if the circuit fails to
   * be launched, we still want to respect the retry period to avoid stress on
   * the circuit subsystem. */
  service->state.num_intro_circ_launched++;
  circ = circuit_launch_by_extend_info(CIRCUIT_PURPOSE_S_ESTABLISH_INTRO,
                                       ei, circ_flags);
  if (circ == NULL) {
    goto end;
  }

  /* Setup the circuit identifier and attach it to it. */
  circ->hs_ident = create_intro_circuit_identifier(service, ip);
  tor_assert(circ->hs_ident);
  /* Register circuit in the global circuitmap. */
  register_intro_circ(ip, circ);

  /* Success. */
  ret = 0;
 end:
  return ret;
}

/** Called when a service introduction point circuit is done building. Given
 * the service and intro point object, this function will send the
 * ESTABLISH_INTRO cell on the circuit. Return 0 on success. Return 1 if the
 * circuit has been repurposed to General because we already have too many
 * opened. */
int
hs_circ_service_intro_has_opened(hs_service_t *service,
                                 hs_service_intro_point_t *ip,
                                 const hs_service_descriptor_t *desc,
                                 origin_circuit_t *circ)
{
  int ret = 0;
  unsigned int num_intro_circ, num_needed_circ;

  tor_assert(service);
  tor_assert(ip);
  tor_assert(desc);
  tor_assert(circ);

  /* Count opened circuits that have sent ESTABLISH_INTRO cells or are already
   * established introduction circuits */
  num_intro_circ = count_opened_desc_intro_point_circuits(service, desc);
  num_needed_circ = service->config.num_intro_points;
  if (num_intro_circ > num_needed_circ) {
    /* There are too many opened valid intro circuit for what the service
     * needs so repurpose this one. */

    /* XXX: Legacy code checks options->ExcludeNodes and if not NULL it just
     * closes the circuit. I have NO idea why it does that so it hasn't been
     * added here. I can only assume in case our ExcludeNodes list changes but
     * in that case, all circuit are flagged unusable (config.c). --dgoulet */

    log_info(LD_CIRC | LD_REND, "Introduction circuit just opened but we "
                                "have enough for service %s. Repurposing "
                                "it to general and leaving internal.",
             safe_str_client(service->onion_address));
    tor_assert(circ->build_state->is_internal);
    /* Remove it from the circuitmap. */
    hs_circuitmap_remove_circuit(TO_CIRCUIT(circ));
    /* Cleaning up the hidden service identifier and repurpose. */
    hs_ident_circuit_free(circ->hs_ident);
    circ->hs_ident = NULL;
    if (circuit_should_use_vanguards(TO_CIRCUIT(circ)->purpose))
      circuit_change_purpose(TO_CIRCUIT(circ), CIRCUIT_PURPOSE_HS_VANGUARDS);
    else
      circuit_change_purpose(TO_CIRCUIT(circ), CIRCUIT_PURPOSE_C_GENERAL);

    /* Inform that this circuit just opened for this new purpose. */
    circuit_has_opened(circ);
    /* This return value indicate to the caller that the IP object should be
     * removed from the service because it's corresponding circuit has just
     * been repurposed. */
    ret = 1;
    goto done;
  }

  log_info(LD_REND, "Introduction circuit %u established for service %s.",
           TO_CIRCUIT(circ)->n_circ_id,
           safe_str_client(service->onion_address));
  circuit_log_path(LOG_INFO, LD_REND, circ);

  /* Time to send an ESTABLISH_INTRO cell on this circuit. On error, this call
   * makes sure the circuit gets closed. */
  send_establish_intro(service, ip, circ);

 done:
  return ret;
}

/** Called when a service rendezvous point circuit is done building. Given the
 * service and the circuit, this function will send a RENDEZVOUS1 cell on the
 * circuit using the information in the circuit identifier. If the cell can't
 * be sent, the circuit is closed. */
void
hs_circ_service_rp_has_opened(const hs_service_t *service,
                              origin_circuit_t *circ)
{
  size_t payload_len;
  uint8_t payload[RELAY_PAYLOAD_SIZE] = {0};

  tor_assert(service);
  tor_assert(circ);
  tor_assert(circ->hs_ident);

  /* Some useful logging. */
  log_info(LD_REND, "Rendezvous circuit %u has opened with cookie %s "
                    "for service %s",
           TO_CIRCUIT(circ)->n_circ_id,
           hex_str((const char *) circ->hs_ident->rendezvous_cookie,
                   REND_COOKIE_LEN),
           safe_str_client(service->onion_address));
  circuit_log_path(LOG_INFO, LD_REND, circ);

  /* This can't fail. */
  payload_len = hs_cell_build_rendezvous1(
                        circ->hs_ident->rendezvous_cookie,
                        sizeof(circ->hs_ident->rendezvous_cookie),
                        circ->hs_ident->rendezvous_handshake_info,
                        sizeof(circ->hs_ident->rendezvous_handshake_info),
                        payload);

  /* Pad the payload with random bytes so it matches the size of a legacy cell
   * which is normally always bigger. Also, the size of a legacy cell is
   * always smaller than the RELAY_PAYLOAD_SIZE so this is safe. */
  if (payload_len < HS_LEGACY_RENDEZVOUS_CELL_SIZE) {
    crypto_rand((char *) payload + payload_len,
                HS_LEGACY_RENDEZVOUS_CELL_SIZE - payload_len);
    payload_len = HS_LEGACY_RENDEZVOUS_CELL_SIZE;
  }

  if (relay_send_command_from_edge(CONTROL_CELL_ID, TO_CIRCUIT(circ),
                                   RELAY_COMMAND_RENDEZVOUS1,
                                   (const char *) payload, payload_len,
                                   circ->cpath->prev) < 0) {
    /* On error, circuit is closed. */
    log_warn(LD_REND, "Unable to send RENDEZVOUS1 cell on circuit %u "
                      "for service %s",
             TO_CIRCUIT(circ)->n_circ_id,
             safe_str_client(service->onion_address));
    goto done;
  }

  /* Setup end-to-end rendezvous circuit between the client and us. */
  if (hs_circuit_setup_e2e_rend_circ(circ,
                       circ->hs_ident->rendezvous_ntor_key_seed,
                       sizeof(circ->hs_ident->rendezvous_ntor_key_seed),
                       1) < 0) {
    log_warn(LD_GENERAL, "Failed to setup circ");
    goto done;
  }

 done:
  memwipe(payload, 0, sizeof(payload));
}

/** Circ has been expecting an INTRO_ESTABLISHED cell that just arrived. Handle
 * the INTRO_ESTABLISHED cell payload of length payload_len arriving on the
 * given introduction circuit circ. The service is only used for logging
 * purposes. Return 0 on success else a negative value. */
int
hs_circ_handle_intro_established(const hs_service_t *service,
                                 const hs_service_intro_point_t *ip,
                                 origin_circuit_t *circ,
                                 const uint8_t *payload, size_t payload_len)
{
  int ret = -1;

  tor_assert(service);
  tor_assert(ip);
  tor_assert(circ);
  tor_assert(payload);

  if (BUG(TO_CIRCUIT(circ)->purpose != CIRCUIT_PURPOSE_S_ESTABLISH_INTRO)) {
    goto done;
  }

  /* Try to parse the payload into a cell making sure we do actually have a
   * valid cell. */
  if (hs_cell_parse_intro_established(payload, payload_len) < 0) {
    log_warn(LD_REND, "Unable to parse the INTRO_ESTABLISHED cell on "
                      "circuit %u for service %s",
             TO_CIRCUIT(circ)->n_circ_id,
             safe_str_client(service->onion_address));
    goto done;
  }

  /* Switch the purpose to a fully working intro point. */
  circuit_change_purpose(TO_CIRCUIT(circ), CIRCUIT_PURPOSE_S_INTRO);
  /* Getting a valid INTRODUCE_ESTABLISHED means we've successfully used the
   * circuit so update our pathbias subsystem. */
  pathbias_mark_use_success(circ);
  /* Success. */
  ret = 0;

 done:
  return ret;
}

/**
 *  Go into <b>data</b> and add the right subcredential to be able to handle
 *  this incoming cell.
 *
 *  <b>desc_subcred</b> is the subcredential of the descriptor that corresponds
 *  to the intro point that received this intro request. This subcredential
 *  should be used if we are not an onionbalance instance.
 *
 *  Return 0 if everything went well, or -1 in case of internal error.
 */
static int
get_subcredential_for_handling_intro2_cell(const hs_service_t *service,
                                        hs_cell_introduce2_data_t *data,
                                        const hs_subcredential_t *desc_subcred)
{
  /* Handle the simple case first: We are not an onionbalance instance and we
   * should just use the regular descriptor subcredential */
  if (!hs_ob_service_is_instance(service)) {
    data->n_subcredentials = 1;
    data->subcredentials = desc_subcred;
    return 0;
  }

  /* This should not happen since we should have made onionbalance
   * subcredentials when we created our descriptors. */
  if (BUG(!service->state.ob_subcreds)) {
    return -1;
  }

  /* We are an onionbalance instance: */
  data->n_subcredentials = service->state.n_ob_subcreds;
  data->subcredentials = service->state.ob_subcreds;

  return 0;
}

/** We just received an INTRODUCE2 cell on the established introduction circuit
 * circ.  Handle the INTRODUCE2 payload of size payload_len for the given
 * circuit and service. This cell is associated with the intro point object ip
 * and the subcredential. Return 0 on success else a negative value. */
int
hs_circ_handle_introduce2(const hs_service_t *service,
                          const origin_circuit_t *circ,
                          hs_service_intro_point_t *ip,
                          const hs_subcredential_t *subcredential,
                          const uint8_t *payload, size_t payload_len)
{
  int ret = -1;
  time_t elapsed;
  hs_cell_introduce2_data_t data;

  tor_assert(service);
  tor_assert(circ);
  tor_assert(ip);
  tor_assert(subcredential);
  tor_assert(payload);

  /* Populate the data structure with everything we need for the cell to be
   * parsed, decrypted and key material computed correctly. */
  data.auth_pk = &ip->auth_key_kp.pubkey;
  data.enc_kp = &ip->enc_key_kp;
  data.payload = payload;
  data.payload_len = payload_len;
  data.link_specifiers = smartlist_new();
  data.replay_cache = ip->replay_cache;
  data.cc_enabled = 0;

  if (get_subcredential_for_handling_intro2_cell(service,
                                                 &data, subcredential)) {
    goto done;
  }

  if (hs_cell_parse_introduce2(&data, circ, service) < 0) {
    goto done;
  }

  /* Check whether we've seen this REND_COOKIE before to detect repeats. */
  if (replaycache_add_test_and_elapsed(
           service->state.replay_cache_rend_cookie,
           data.rendezvous_cookie, sizeof(data.rendezvous_cookie),
           &elapsed)) {
    /* A Tor client will send a new INTRODUCE1 cell with the same REND_COOKIE
     * as its previous one if its intro circ times out while in state
     * CIRCUIT_PURPOSE_C_INTRODUCE_ACK_WAIT. If we received the first
     * INTRODUCE1 cell (the intro-point relay converts it into an INTRODUCE2
     * cell), we are already trying to connect to that rend point (and may
     * have already succeeded); drop this cell. */
    log_info(LD_REND, "We received an INTRODUCE2 cell with same REND_COOKIE "
                      "field %ld seconds ago. Dropping cell.",
             (long int) elapsed);
    goto done;
  }

  /* At this point, we just confirmed that the full INTRODUCE2 cell is valid
   * so increment our counter that we've seen one on this intro point. */
  ip->introduce2_count++;

  /* Launch rendezvous circuit with the onion key and rend cookie. */
  launch_rendezvous_point_circuit(service, ip, &data);
  /* Success. */
  ret = 0;

 done:
  link_specifier_smartlist_free(data.link_specifiers);
  memwipe(&data, 0, sizeof(data));
  return ret;
}

/** Circuit <b>circ</b> just finished the rend ntor key exchange. Use the key
 * exchange output material at <b>ntor_key_seed</b> and setup <b>circ</b> to
 * serve as a rendezvous end-to-end circuit between the client and the
 * service. If <b>is_service_side</b> is set, then we are the hidden service
 * and the other side is the client.
 *
 * Return 0 if the operation went well; in case of error return -1. */
int
hs_circuit_setup_e2e_rend_circ(origin_circuit_t *circ,
                               const uint8_t *ntor_key_seed, size_t seed_len,
                               int is_service_side)
{
  if (BUG(!circuit_purpose_is_correct_for_rend(TO_CIRCUIT(circ)->purpose,
                                        is_service_side))) {
    return -1;
  }

  crypt_path_t *hop = create_rend_cpath(ntor_key_seed, seed_len,
                                        is_service_side);
  if (!hop) {
    log_warn(LD_REND, "Couldn't get v3 %s cpath!",
             is_service_side ? "service-side" : "client-side");
    return -1;
  }

  finalize_rend_circuit(circ, hop, is_service_side);

  return 0;
}

/** Given the introduction circuit intro_circ, the rendezvous circuit
 * rend_circ, a descriptor intro point object ip and the service's
 * subcredential, send an INTRODUCE1 cell on intro_circ.
 *
 * This will also setup the circuit identifier on rend_circ containing the key
 * material for the handshake and e2e encryption. Return 0 on success else
 * negative value. Because relay_send_command_from_edge() closes the circuit
 * on error, it is possible that intro_circ is closed on error. */
int
hs_circ_send_introduce1(origin_circuit_t *intro_circ,
                        origin_circuit_t *rend_circ,
                        const hs_desc_intro_point_t *ip,
                        const hs_subcredential_t *subcredential)
{
  int ret = -1;
  ssize_t payload_len;
  uint8_t payload[RELAY_PAYLOAD_SIZE] = {0};
  hs_cell_introduce1_data_t intro1_data;

  tor_assert(intro_circ);
  tor_assert(rend_circ);
  tor_assert(ip);
  tor_assert(subcredential);

  /* It is undefined behavior in hs_cell_introduce1_data_clear() if intro1_data
   * has been declared on the stack but not initialized. Here, we set it to 0.
   */
  memset(&intro1_data, 0, sizeof(hs_cell_introduce1_data_t));

  /* This takes various objects in order to populate the introduce1 data
   * object which is used to build the content of the cell. */
  const node_t *exit_node = build_state_get_exit_node(rend_circ->build_state);
  if (exit_node == NULL) {
    log_info(LD_REND, "Unable to get rendezvous point for circuit %u. "
             "Failing.", TO_CIRCUIT(intro_circ)->n_circ_id);
    goto done;
  }

  /* We should never select an invalid rendezvous point in theory but if we
   * do, this function will fail to populate the introduce data. */
  if (setup_introduce1_data(ip, exit_node, subcredential, &intro1_data) < 0) {
    log_info(LD_REND, "Unable to setup INTRODUCE1 data. The chosen rendezvous "
                      "point is unusable. Closing circuit.");
    goto close;
  }

  /* If the rend circ was set up for congestion control, add that to the
   * intro data, to signal it in an extension */
  if (TO_CIRCUIT(rend_circ)->ccontrol) {
    intro1_data.cc_enabled = 1;
  }

  /* Final step before we encode a cell, we setup the circuit identifier which
   * will generate both the rendezvous cookie and client keypair for this
   * connection. Those are put in the ident. */
  intro1_data.rendezvous_cookie = rend_circ->hs_ident->rendezvous_cookie;
  intro1_data.client_kp = &rend_circ->hs_ident->rendezvous_client_kp;

  memcpy(intro_circ->hs_ident->rendezvous_cookie,
         rend_circ->hs_ident->rendezvous_cookie,
         sizeof(intro_circ->hs_ident->rendezvous_cookie));

  /* From the introduce1 data object, this will encode the INTRODUCE1 cell
   * into payload which is then ready to be sent as is. */
  payload_len = hs_cell_build_introduce1(&intro1_data, payload);
  if (BUG(payload_len < 0)) {
    goto close;
  }

  if (relay_send_command_from_edge(CONTROL_CELL_ID, TO_CIRCUIT(intro_circ),
                                   RELAY_COMMAND_INTRODUCE1,
                                   (const char *) payload, payload_len,
                                   intro_circ->cpath->prev) < 0) {
    /* On error, circuit is closed. */
    log_warn(LD_REND, "Unable to send INTRODUCE1 cell on circuit %u.",
             TO_CIRCUIT(intro_circ)->n_circ_id);
    goto done;
  }

  /* Success. */
  ret = 0;
  goto done;

 close:
  circuit_mark_for_close(TO_CIRCUIT(rend_circ), END_CIRC_REASON_INTERNAL);
 done:
  hs_cell_introduce1_data_clear(&intro1_data);
  memwipe(payload, 0, sizeof(payload));
  return ret;
}

/** Send an ESTABLISH_RENDEZVOUS cell along the rendezvous circuit circ. On
 * success, 0 is returned else -1 and the circuit is marked for close. */
int
hs_circ_send_establish_rendezvous(origin_circuit_t *circ)
{
  ssize_t cell_len = 0;
  uint8_t cell[RELAY_PAYLOAD_SIZE] = {0};

  tor_assert(circ);
  tor_assert(TO_CIRCUIT(circ)->purpose == CIRCUIT_PURPOSE_C_ESTABLISH_REND);

  log_info(LD_REND, "Send an ESTABLISH_RENDEZVOUS cell on circuit %u",
           TO_CIRCUIT(circ)->n_circ_id);

  /* Set timestamp_dirty, because circuit_expire_building expects it,
   * and the rend cookie also means we've used the circ. */
  TO_CIRCUIT(circ)->timestamp_dirty = time(NULL);

  /* We've attempted to use this circuit. Probe it if we fail */
  pathbias_count_use_attempt(circ);

  /* Generate the RENDEZVOUS_COOKIE and place it in the identifier so we can
   * complete the handshake when receiving the acknowledgement. */
  crypto_rand((char *) circ->hs_ident->rendezvous_cookie, HS_REND_COOKIE_LEN);
  /* Generate the client keypair. No need to be extra strong, not long term */
  curve25519_keypair_generate(&circ->hs_ident->rendezvous_client_kp, 0);

  cell_len =
    hs_cell_build_establish_rendezvous(circ->hs_ident->rendezvous_cookie,
                                       cell);
  if (BUG(cell_len < 0)) {
    goto err;
  }

  if (relay_send_command_from_edge(CONTROL_CELL_ID, TO_CIRCUIT(circ),
                                   RELAY_COMMAND_ESTABLISH_RENDEZVOUS,
                                   (const char *) cell, cell_len,
                                   circ->cpath->prev) < 0) {
    /* Circuit has been marked for close */
    log_warn(LD_REND, "Unable to send ESTABLISH_RENDEZVOUS cell on "
                      "circuit %u", TO_CIRCUIT(circ)->n_circ_id);
    memwipe(cell, 0, cell_len);
    goto err;
  }

  memwipe(cell, 0, cell_len);
  return 0;
 err:
  return -1;
}

/** Circuit cleanup strategy:
 *
 *  What follows is a series of functions that notifies the HS subsystem of 3
 *  different circuit cleanup phase: close, free and repurpose.
 *
 *  Tor can call any of those in any orders so they have to be safe between
 *  each other. In other words, the free should never depend on close to be
 *  called before.
 *
 *  The "on_close()" is called from circuit_mark_for_close() which is
 *  considered the tor fast path and thus as little work as possible should
 *  done in that function. Currently, we only remove the circuit from the HS
 *  circuit map and move on.
 *
 *  The "on_free()" is called from circuit circuit_free_() and it is very
 *  important that at the end of the function, no state or objects related to
 *  this circuit remains alive.
 *
 *  The "on_repurpose()" is called from circuit_change_purpose() for which we
 *  simply remove it from the HS circuit map. We do not have other cleanup
 *  requirements after that.
 *
 *  NOTE: The onion service code, specifically the service code, cleans up
 *  lingering objects or state if any of its circuit disappear which is why
 *  our cleanup strategy doesn't involve any service specific actions. As long
 *  as the circuit is removed from the HS circuit map, it won't be used.
 */

/** We are about to close this <b>circ</b>. Clean it up from any related HS
 * data structures. This function can be called multiple times safely for the
 * same circuit. */
void
hs_circ_cleanup_on_close(circuit_t *circ)
{
  tor_assert(circ);

  if (circuit_purpose_is_hs_client(circ->purpose)) {
    cleanup_on_close_client_circ(circ);
  }

  if (circuit_purpose_is_hs_service(circ->purpose)) {
    if (circuit_is_hs_v3(circ)) {
      hs_service_circuit_cleanup_on_close(circ);
    }
  }

  /* On close, we simply remove it from the circuit map. It can not be used
   * anymore. We keep this code path fast and lean. */

  if (circ->hs_token) {
    hs_circuitmap_remove_circuit(circ);
  }
}

/** We are about to free this <b>circ</b>. Clean it up from any related HS
 * data structures. This function can be called multiple times safely for the
 * same circuit. */
void
hs_circ_cleanup_on_free(circuit_t *circ)
{
  tor_assert(circ);

  /* NOTE: Bulk of the work of cleaning up a circuit is done here. */

  if (circuit_purpose_is_hs_client(circ->purpose)) {
    cleanup_on_free_client_circ(circ);
  }

  /* We have no assurance that the given HS circuit has been closed before and
   * thus removed from the HS map. This actually happens in unit tests. */
  if (circ->hs_token) {
    hs_circuitmap_remove_circuit(circ);
  }
}

/** We are about to repurpose this <b>circ</b>. Clean it up from any related
 * HS data structures. This function can be called multiple times safely for
 * the same circuit. */
void
hs_circ_cleanup_on_repurpose(circuit_t *circ)
{
  tor_assert(circ);

  /* On repurpose, we simply remove it from the circuit map but we do not do
   * the on_free actions since we don't treat a repurpose as something we need
   * to report in the client cache failure. */

  if (circ->hs_token) {
    hs_circuitmap_remove_circuit(circ);
  }
}

/** Return true iff the given established client rendezvous circuit was sent
 * into the INTRODUCE1 cell. This is called so we can take a decision on
 * expiring or not the circuit.
 *
 * The caller MUST make sure the circuit is an established client rendezvous
 * circuit (purpose: CIRCUIT_PURPOSE_C_REND_READY).
 *
 * This function supports all onion service versions. */
bool
hs_circ_is_rend_sent_in_intro1(const origin_circuit_t *circ)
{
  tor_assert(circ);
  /* This can only be called for a rendezvous circuit that is an established
   * confirmed rendezsvous circuit but without an introduction ACK. */
  tor_assert(TO_CIRCUIT(circ)->purpose == CIRCUIT_PURPOSE_C_REND_READY);

  /* When the INTRODUCE1 cell is sent, the introduction encryption public
   * key is copied in the rendezvous circuit hs identifier. If it is a valid
   * key, we know that this circuit is waiting the ACK on the introduction
   * circuit. We want to _not_ spare the circuit if the key was never set. */

  if (circ->hs_ident) {
    /* v3. */
    if (curve25519_public_key_is_ok(&circ->hs_ident->intro_enc_pk)) {
      return true;
    }
  } else {
    /* A circuit with an HS purpose without an hs_ident in theory can not
     * happen. In case, scream loudly and return false to the caller that the
     * rendezvous was not sent in the INTRO1 cell. */
    tor_assert_nonfatal_unreached();
  }

  /* The rendezvous has not been specified in the INTRODUCE1 cell. */
  return false;
}
