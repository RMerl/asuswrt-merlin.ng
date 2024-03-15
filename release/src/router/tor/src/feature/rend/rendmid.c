/* Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file rendmid.c
 * \brief Implement introductions points and rendezvous points.
 **/

#include "core/or/or.h"
#include "core/or/channel.h"
#include "core/or/circuitlist.h"
#include "core/or/circuituse.h"
#include "app/config/config.h"
#include "lib/crypt_ops/crypto_cipher.h"
#include "core/or/dos.h"
#include "core/or/relay.h"
#include "feature/rend/rendmid.h"
#include "feature/hs/hs_circuitmap.h"
#include "feature/hs/hs_dos.h"
#include "feature/hs/hs_intropoint.h"
#include "feature/relay/relay_metrics.h"

#include "core/or/or_circuit_st.h"

/** Process an ESTABLISH_RENDEZVOUS cell by setting the circuit's purpose and
 * rendezvous cookie.
 */
int
rend_mid_establish_rendezvous(or_circuit_t *circ, const uint8_t *request,
                              size_t request_len)
{
  char hexid[9];
  int reason = END_CIRC_REASON_TORPROTOCOL;

  log_info(LD_REND, "Received an ESTABLISH_RENDEZVOUS request on circuit %u",
           (unsigned)circ->p_circ_id);

  if (circ->base_.purpose != CIRCUIT_PURPOSE_OR) {
    relay_increment_est_rend_action(EST_REND_UNSUITABLE_CIRCUIT);
    log_fn(LOG_PROTOCOL_WARN, LD_PROTOCOL,
           "Tried to establish rendezvous on non-OR circuit with purpose %s",
           circuit_purpose_to_string(circ->base_.purpose));
    goto err;
  }

  /* Check if we are configured to defend ourselves from clients that
   * attempt to establish rendezvous points directly to us. */
  if (channel_is_client(circ->p_chan) &&
      dos_should_refuse_single_hop_client()) {
    relay_increment_est_rend_action(EST_REND_SINGLE_HOP);
    /* Note it down for the heartbeat log purposes. */
    dos_note_refuse_single_hop_client();
    /* Silent drop so the client has to time out before moving on. */
    return 0;
  }

  if (circ->base_.n_chan) {
    relay_increment_est_rend_action(EST_REND_UNSUITABLE_CIRCUIT);
    log_fn(LOG_PROTOCOL_WARN, LD_PROTOCOL,
             "Tried to establish rendezvous on non-edge circuit");
    goto err;
  }

  if (request_len != REND_COOKIE_LEN) {
    relay_increment_est_rend_action(EST_REND_MALFORMED);
    log_fn(LOG_PROTOCOL_WARN,
           LD_PROTOCOL, "Invalid length on ESTABLISH_RENDEZVOUS.");
    goto err;
  }

  if (hs_circuitmap_get_rend_circ_relay_side(request)) {
    relay_increment_est_rend_action(EST_REND_DUPLICATE_COOKIE);
    log_fn(LOG_PROTOCOL_WARN, LD_PROTOCOL,
           "Duplicate rendezvous cookie in ESTABLISH_RENDEZVOUS.");
    goto err;
  }

  /* Acknowledge the request. */
  if (relay_send_command_from_edge(0,TO_CIRCUIT(circ),
                                   RELAY_COMMAND_RENDEZVOUS_ESTABLISHED,
                                   "", 0, NULL)<0) {
    relay_increment_est_rend_action(EST_REND_CIRCUIT_DEAD);
    log_warn(LD_PROTOCOL, "Couldn't send RENDEZVOUS_ESTABLISHED cell.");
    /* Stop right now, the circuit has been closed. */
    return -1;
  }

  relay_increment_est_rend_action(EST_REND_SUCCESS);
  circuit_change_purpose(TO_CIRCUIT(circ), CIRCUIT_PURPOSE_REND_POINT_WAITING);
  hs_circuitmap_register_rend_circ_relay_side(circ, request);

  base16_encode(hexid,9,(char*)request,4);

  log_info(LD_REND,
           "Established rendezvous point on circuit %u for cookie %s",
           (unsigned)circ->p_circ_id, hexid);

  return 0;
 err:
  circuit_mark_for_close(TO_CIRCUIT(circ), reason);
  return -1;
}

/** Process a RENDEZVOUS1 cell by looking up the correct rendezvous
 * circuit by its relaying the cell's body in a RENDEZVOUS2 cell, and
 * connecting the two circuits.
 */
int
rend_mid_rendezvous(or_circuit_t *circ, const uint8_t *request,
                    size_t request_len)
{
  const or_options_t *options = get_options();
  or_circuit_t *rend_circ;
  char hexid[9];
  int reason = END_CIRC_REASON_INTERNAL;

  if (circ->base_.purpose != CIRCUIT_PURPOSE_OR || circ->base_.n_chan) {
    relay_increment_rend1_action(REND1_UNSUITABLE_CIRCUIT);
    log_fn(LOG_PROTOCOL_WARN, LD_PROTOCOL,
           "Tried to complete rendezvous on non-OR or non-edge circuit %u.",
           (unsigned)circ->p_circ_id);
    reason = END_CIRC_REASON_TORPROTOCOL;
    goto err;
  }

  if (request_len < REND_COOKIE_LEN) {
    relay_increment_rend1_action(REND1_MALFORMED);
    log_fn(LOG_PROTOCOL_WARN, LD_PROTOCOL,
         "Rejecting RENDEZVOUS1 cell with bad length (%d) on circuit %u.",
         (int)request_len, (unsigned)circ->p_circ_id);
    reason = END_CIRC_REASON_TORPROTOCOL;
    goto err;
  }

  base16_encode(hexid, sizeof(hexid), (const char*)request, 4);

  log_info(LD_REND,
           "Got request for rendezvous from circuit %u to cookie %s.",
           (unsigned)circ->p_circ_id, hexid);

  rend_circ = hs_circuitmap_get_rend_circ_relay_side(request);
  if (!rend_circ) {
    /* Once this was a LOG_PROTOCOL_WARN, but it can happen naturally if a
     * client gives up on a rendezvous circuit after sending INTRODUCE1, but
     * before the onion service sends the RENDEZVOUS1 cell.
     */
    relay_increment_rend1_action(REND1_UNKNOWN_COOKIE);
    log_fn(LOG_DEBUG, LD_PROTOCOL,
         "Rejecting RENDEZVOUS1 cell with unrecognized rendezvous cookie %s.",
         hexid);
    reason = END_CIRC_REASON_TORPROTOCOL;
    goto err;
  }

  /* Statistics: Mark circuits as RP circuits */
  if (options->HiddenServiceStatistics) {
    /* `circ` is the RP <-> service circuit */
    circ->circuit_carries_hs_traffic_stats = 1;
    /* `rend_circ` is the client <-> RP circuit */
    rend_circ->circuit_carries_hs_traffic_stats = 1;
  }

  /* Send the RENDEZVOUS2 cell to the client. */
  if (relay_send_command_from_edge(0, TO_CIRCUIT(rend_circ),
                                   RELAY_COMMAND_RENDEZVOUS2,
                                   (char*)(request+REND_COOKIE_LEN),
                                   request_len-REND_COOKIE_LEN, NULL)) {
    relay_increment_rend1_action(REND1_CIRCUIT_DEAD);
    log_warn(LD_GENERAL,
             "Unable to send RENDEZVOUS2 cell to client on circuit %u.",
             (unsigned)rend_circ->p_circ_id);
    /* Stop right now, the circuit has been closed. */
    return -1;
  }

  relay_increment_rend1_action(REND1_SUCCESS);
  /* Join the circuits. */
  log_info(LD_REND,
           "Completing rendezvous: circuit %u joins circuit %u (cookie %s)",
           (unsigned)circ->p_circ_id, (unsigned)rend_circ->p_circ_id, hexid);

  circuit_change_purpose(TO_CIRCUIT(circ), CIRCUIT_PURPOSE_REND_ESTABLISHED);
  circuit_change_purpose(TO_CIRCUIT(rend_circ),
                         CIRCUIT_PURPOSE_REND_ESTABLISHED);
  hs_circuitmap_remove_circuit(TO_CIRCUIT(circ));

  rend_circ->rend_splice = circ;
  circ->rend_splice = rend_circ;

  return 0;
 err:
  circuit_mark_for_close(TO_CIRCUIT(circ), reason);
  return -1;
}
