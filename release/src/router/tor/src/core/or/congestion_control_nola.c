/* Copyright (c) 2019-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file congestion_control_nola.c
 * \brief Code that implements the TOR_NOLA congestion control algorithm
 *        from Proposal #324.
 */

#define TOR_CONGESTION_CONTROL_NOLA_PRIVATE

#include "core/or/or.h"

#include "core/or/crypt_path.h"
#include "core/or/or_circuit_st.h"
#include "core/or/sendme.h"
#include "core/or/congestion_control_st.h"
#include "core/or/congestion_control_common.h"
#include "core/or/congestion_control_nola.h"
#include "core/or/circuituse.h"
#include "core/or/circuitlist.h"
#include "core/or/origin_circuit_st.h"
#include "core/or/channel.h"
#include "feature/nodelist/networkstatus.h"
#include "feature/control/control_events.h"

#define NOLA_BDP_OVERSHOOT    100

/**
 * Cache NOLA consensus parameters.
 */
void
congestion_control_nola_set_params(congestion_control_t *cc)
{
  tor_assert(cc->cc_alg == CC_ALG_NOLA);

  cc->nola_params.bdp_overshoot =
      networkstatus_get_param(NULL, "cc_nola_overshoot",
              NOLA_BDP_OVERSHOOT,
              0,
              1000);
}

/**
* Process a SENDME and update the congestion window according to the
* rules specified in TOR_NOLA of Proposal #324.
*
* TOR_NOLA updates the congestion window to match the current
* BDP estimate, every sendme. Because this can result in downward
* drift, a fixed overhead is added to the BDP estimate. This will
* cause some queuing, but ensures that the algorithm always uses
* the full BDP.
*
* To handle the case where the local orconn blocks, TOR_NOLA uses
* the 'piecewise' BDP estimate, which uses more a conservative BDP
* estimate method when blocking occurs, but a more aggressive BDP
* estimate when there is no local blocking. This minimizes local
* client queues.
*/
int
congestion_control_nola_process_sendme(congestion_control_t *cc,
                                       const circuit_t *circ,
                                       const crypt_path_t *layer_hint)
{
  tor_assert(cc && cc->cc_alg == CC_ALG_NOLA);
  tor_assert(circ);

  if (cc->next_cc_event)
    cc->next_cc_event--;

  /* If we get a congestion event, the only thing NOLA
   * does is note this as if we exited slow-start
   * (which for NOLA just means we finished our ICW). */
  if (cc->next_cc_event == 0) {
    if (cc->in_slow_start) {
      cc->in_slow_start = 0;

      /* We need to report that slow start has exited ASAP,
       * for sbws bandwidth measurement. */
      if (CIRCUIT_IS_ORIGIN(circ)) {
        /* We must discard const here because the event modifies fields :/ */
        control_event_circ_bandwidth_used_for_circ(
                TO_ORIGIN_CIRCUIT((circuit_t*)circ));
      }
    }
  }

  /* If we did not successfully update BDP, we must return. Otherwise,
   * NOLA can drift downwards */
  if (!congestion_control_update_circuit_estimates(cc, circ, layer_hint)) {
    cc->inflight = cc->inflight - cc->sendme_inc;
    return 0;
  }

  /* We overshoot the BDP by the cwnd_inc param amount, because BDP
   * may otherwise drift down. This helps us probe for more capacity.
   * But there is no sense to do it if the local channel is blocked. */
  if (cc->blocked_chan)
    cc->cwnd = cc->bdp[cc->bdp_alg];
  else
    cc->cwnd = cc->bdp[cc->bdp_alg] + cc->nola_params.bdp_overshoot;

  /* cwnd can never fall below 1 increment */
  cc->cwnd = MAX(cc->cwnd, cc->cwnd_min);

  if (CIRCUIT_IS_ORIGIN(circ)) {
    log_info(LD_CIRC,
               "CC TOR_NOLA: Circuit %d "
               "CWND: %"PRIu64", "
               "INFL: %"PRIu64", "
               "NCCE: %"PRIu16", "
               "SS: %d",
             CONST_TO_ORIGIN_CIRCUIT(circ)->global_identifier,
             cc->cwnd,
             cc->inflight,
             cc->next_cc_event,
             cc->in_slow_start
             );
  } else {
    log_info(LD_CIRC,
               "CC TOR_NOLA: Circuit %"PRIu64":%d "
               "CWND: %"PRIu64", "
               "INFL: %"PRIu64", "
               "NCCE: %"PRIu16", "
               "SS: %d",
             CONST_TO_OR_CIRCUIT(circ)->p_chan->global_identifier,
             CONST_TO_OR_CIRCUIT(circ)->p_circ_id,
             cc->cwnd,
             cc->inflight,
             cc->next_cc_event,
             cc->in_slow_start
             );
  }

  /* Update inflight with ack */
  cc->inflight = cc->inflight - cc->sendme_inc;

  return 0;
}
