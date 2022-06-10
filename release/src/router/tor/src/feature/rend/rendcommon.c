/* Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file rendcommon.c
 * \brief Rendezvous implementation: shared code between
 * introducers, services, clients, and rendezvous points.
 **/

#define RENDCOMMON_PRIVATE

#include "core/or/or.h"

#include "app/config/config.h"

#include "core/or/circuitlist.h"
#include "core/or/circuituse.h"

#include "feature/hs/hs_client.h"
#include "feature/hs/hs_common.h"
#include "feature/hs/hs_intropoint.h"
#include "feature/rend/rendcommon.h"
#include "feature/rend/rendmid.h"

#include "core/or/circuit_st.h"
#include "core/or/cpath_build_state_st.h"
#include "core/or/crypt_path_st.h"
#include "core/or/origin_circuit_st.h"

/** Called when we get a rendezvous-related relay cell on circuit
 * <b>circ</b>.  Dispatch on rendezvous relay command. */
void
rend_process_relay_cell(circuit_t *circ, const crypt_path_t *layer_hint,
                        int command, size_t length,
                        const uint8_t *payload)
{
  or_circuit_t *or_circ = NULL;
  origin_circuit_t *origin_circ = NULL;
  int r = -2;
  if (CIRCUIT_IS_ORIGIN(circ)) {
    origin_circ = TO_ORIGIN_CIRCUIT(circ);
    if (!layer_hint || layer_hint != origin_circ->cpath->prev) {
      log_fn(LOG_PROTOCOL_WARN, LD_APP,
             "Relay cell (rend purpose %d) from wrong hop on origin circ",
             command);
      origin_circ = NULL;
    }
  } else {
    or_circ = TO_OR_CIRCUIT(circ);
  }

  switch (command) {
    case RELAY_COMMAND_ESTABLISH_INTRO:
      if (or_circ)
        r = hs_intro_received_establish_intro(or_circ, payload, length);
      break;
    case RELAY_COMMAND_ESTABLISH_RENDEZVOUS:
      if (or_circ)
        r = rend_mid_establish_rendezvous(or_circ, payload, length);
      break;
    case RELAY_COMMAND_INTRODUCE1:
      if (or_circ)
        r = hs_intro_received_introduce1(or_circ, payload, length);
      break;
    case RELAY_COMMAND_INTRODUCE2:
      if (origin_circ)
        r = hs_service_receive_introduce2(origin_circ, payload, length);
      break;
    case RELAY_COMMAND_INTRODUCE_ACK:
      if (origin_circ)
        r = hs_client_receive_introduce_ack(origin_circ, payload, length);
      break;
    case RELAY_COMMAND_RENDEZVOUS1:
      if (or_circ)
        r = rend_mid_rendezvous(or_circ, payload, length);
      break;
    case RELAY_COMMAND_RENDEZVOUS2:
      if (origin_circ)
        r = hs_client_receive_rendezvous2(origin_circ, payload, length);
      break;
    case RELAY_COMMAND_INTRO_ESTABLISHED:
      if (origin_circ)
        r = hs_service_receive_intro_established(origin_circ, payload, length);
      break;
    case RELAY_COMMAND_RENDEZVOUS_ESTABLISHED:
      if (origin_circ)
        r = hs_client_receive_rendezvous_acked(origin_circ, payload, length);
      break;
    default:
      tor_fragile_assert();
  }

  if (r == 0 && origin_circ) {
    /* This was a valid cell. Count it as delivered + overhead. */
    circuit_read_valid_data(origin_circ, length);
  }

  if (r == -2)
    log_info(LD_PROTOCOL, "Dropping cell (type %d) for wrong circuit type.",
             command);
}

/* Make sure that tor only builds one-hop circuits when they would not
 * compromise user anonymity.
 *
 * One-hop circuits are permitted in Single Onion modes.
 *
 * Single Onion modes are also allowed to make multi-hop circuits.
 * For example, single onion HSDir circuits are 3-hop to prevent denial of
 * service.
 */
void
assert_circ_anonymity_ok(const origin_circuit_t *circ,
                         const or_options_t *options)
{
  tor_assert(options);
  tor_assert(circ);
  tor_assert(circ->build_state);

  if (circ->build_state->onehop_tunnel) {
    tor_assert(hs_service_allow_non_anonymous_connection(options));
  }
}
