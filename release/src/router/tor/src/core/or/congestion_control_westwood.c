/* Copyright (c) 2019-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file congestion_control_westwood.c
 * \brief Code that implements the TOR_WESTWOOD congestion control algorithm
 *        from Proposal #324.
 */

#define TOR_CONGESTION_CONTROL_WESTWOOD_PRIVATE

#include "core/or/or.h"

#include "core/or/crypt_path.h"
#include "core/or/or_circuit_st.h"
#include "core/or/sendme.h"
#include "core/or/congestion_control_st.h"
#include "core/or/congestion_control_common.h"
#include "core/or/congestion_control_westwood.h"
#include "core/or/circuitlist.h"
#include "core/or/circuituse.h"
#include "core/or/origin_circuit_st.h"
#include "core/or/channel.h"
#include "feature/nodelist/networkstatus.h"
#include "feature/control/control_events.h"

#define USEC_ONE_MS (1000)

#define WESTWOOD_CWND_BACKOFF_M 75
#define WESTWOOD_RTT_BACKOFF_M 100
#define WESTWOOD_RTT_THRESH 33
#define WESTWOOD_MIN_BACKOFF 0

/**
 * Cache westwood consensus parameters.
 */
void
congestion_control_westwood_set_params(congestion_control_t *cc)
{
  tor_assert(cc->cc_alg == CC_ALG_WESTWOOD);

  cc->westwood_params.cwnd_backoff_m =
   networkstatus_get_param(NULL, "cc_westwood_cwnd_m",
      WESTWOOD_CWND_BACKOFF_M,
      0,
      100);

  cc->westwood_params.rtt_backoff_m =
   networkstatus_get_param(NULL, "cc_westwood_rtt_m",
      WESTWOOD_RTT_BACKOFF_M,
      50,
      100);

  cc->westwood_params.rtt_thresh =
   networkstatus_get_param(NULL, "cc_westwood_rtt_thresh",
      WESTWOOD_RTT_THRESH,
      0,
      100);

  cc->westwood_params.min_backoff =
   networkstatus_get_param(NULL, "cc_westwood_min_backoff",
      WESTWOOD_MIN_BACKOFF,
      0,
      1);
}

/**
 * Return the RTT threshold that signals congestion.
 *
 * Computed from the threshold parameter that specifies a
 * percent between the min and max RTT observed so far.
 */
static inline uint64_t
westwood_rtt_signal(const congestion_control_t *cc)
{
  return ((100 - cc->westwood_params.rtt_thresh)*cc->min_rtt_usec +
          cc->westwood_params.rtt_thresh*(cc)->max_rtt_usec)/100;
}

/**
 * Compute a backoff to reduce the max RTT.
 *
 * This may be necessary to ensure that westwood does not have
 * a runaway condition where congestion inflates the max RTT, which
 * inflates the congestion threshold. That cannot happen with one
 * Westwood instance, but it may happen in aggregate. Hence, this is
 * a safety parameter, in case we need it.
 */
static inline uint64_t
westwood_rtt_max_backoff(const congestion_control_t *cc)
{
  return cc->min_rtt_usec +
          (cc->westwood_params.rtt_backoff_m *
          (cc->max_rtt_usec - cc->min_rtt_usec))/100;
}

/**
 * Returns true if the circuit is experiencing congestion, as per
 * TOR_WESTWOOD rules.
 */
static inline bool
westwood_is_congested(const congestion_control_t *cc)
{
  /* If the local channel is blocked, that is always congestion */
  if (cc->blocked_chan)
    return true;

  /* If the min RTT is within 1ms of the signal, then there is not enough
   * range in RTTs to signify congestion. Treat that as not congested. */
  if (westwood_rtt_signal(cc) < cc->min_rtt_usec ||
      westwood_rtt_signal(cc) - cc->min_rtt_usec < USEC_ONE_MS)
    return false;

  /* If the EWMA-smoothed RTT exceeds the westwood RTT threshold,
   * then it is congestion. */
  if (cc->ewma_rtt_usec > westwood_rtt_signal(cc))
    return true;

  return false;
}

/**
 * Process a SENDME and update the congestion window according to the
 * rules specified in TOR_WESTWOOD of Proposal #324.
 *
 * Essentially, this algorithm uses a threshold of 'rtt_thresh', which
 * is a midpoint between the min and max RTT. If the RTT exceeds this
 * threshold, then queue delay due to congestion is assumed to be present,
 * and the algorithm reduces the congestion window. If the RTT is below the
 * threshold, the circuit is not congested (ie: queue delay is low), and we
 * increase the congestion window.
 *
 * The congestion window is updated only once every congestion window worth of
 * packets, even if the signal persists. It is also updated whenever the
 * upstream orcon blocks, or unblocks. This minimizes local client queues.
 */
int
congestion_control_westwood_process_sendme(congestion_control_t *cc,
                                           const circuit_t *circ,
                                           const crypt_path_t *layer_hint)
{
  tor_assert(cc && cc->cc_alg == CC_ALG_WESTWOOD);
  tor_assert(circ);

  /* Update ack counter until next congestion signal event is allowed */
  if (cc->next_cc_event)
    cc->next_cc_event--;

  /* If we were unable to update our circuit estimates, Westwood must
   * *not* update its cwnd, otherwise it could run to infinity, or to 0.
   * Just update inflight from the sendme and return. */
  if (!congestion_control_update_circuit_estimates(cc, circ, layer_hint)) {
    cc->inflight = cc->inflight - cc->sendme_inc;
    return 0;
  }

  /* We only update anything once per window */
  if (cc->next_cc_event == 0) {
    if (!westwood_is_congested(cc)) {
      if (cc->in_slow_start) {
        cc->cwnd = MAX(cc->cwnd + CWND_INC_SS(cc),
                       cc->bdp[cc->bdp_alg]);
      } else {
        cc->cwnd = cc->cwnd + CWND_INC(cc);
      }
    } else {
      if (cc->westwood_params.min_backoff)
        cc->cwnd = MIN(cc->cwnd*cc->westwood_params.cwnd_backoff_m/100,
                       cc->bdp[cc->bdp_alg]);
      else
        cc->cwnd = MAX(cc->cwnd*cc->westwood_params.cwnd_backoff_m/100,
                       cc->bdp[cc->bdp_alg]);

      cc->in_slow_start = 0;

      // Because Westwood's congestion can runaway and boost max rtt,
      // which increases its congestion signal, we backoff the max rtt
      // too.
      cc->max_rtt_usec = westwood_rtt_max_backoff(cc);

      log_info(LD_CIRC, "CC: TOR_WESTWOOD congestion. New max RTT: %"PRIu64,
                 cc->max_rtt_usec/1000);

      /* We need to report that slow start has exited ASAP,
       * for sbws bandwidth measurement. */
      if (CIRCUIT_IS_ORIGIN(circ)) {
        /* We must discard const here because the event modifies fields :/ */
        control_event_circ_bandwidth_used_for_circ(
                TO_ORIGIN_CIRCUIT((circuit_t*)circ));
      }
    }

    /* cwnd can never fall below 1 increment */
    cc->cwnd = MAX(cc->cwnd, cc->cwnd_min);

    /* Schedule next update */
    cc->next_cc_event = CWND_UPDATE_RATE(cc);

    if (CIRCUIT_IS_ORIGIN(circ)) {
      log_info(LD_CIRC,
                 "CC: TOR_WESTWOOD Circuit %d "
                 "CWND: %"PRIu64", "
                 "INFL: %"PRIu64", "
                 "NCCE: %"PRIu16", "
                 "WRTT: %"PRIu64", "
                 "WSIG: %"PRIu64", "
                 "SS: %d",
               CONST_TO_ORIGIN_CIRCUIT(circ)->global_identifier,
               cc->cwnd,
               cc->inflight,
               cc->next_cc_event,
               cc->ewma_rtt_usec/1000,
               westwood_rtt_signal(cc)/1000,
               cc->in_slow_start
               );
    } else {
      log_info(LD_CIRC,
                 "CC: TOR_WESTWOOD Circuit %"PRIu64":%d "
                 "CWND: %"PRIu64", "
                 "INFL: %"PRIu64", "
                 "NCCE: %"PRIu16", "
                 "WRTT: %"PRIu64", "
                 "WSIG: %"PRIu64", "
                 "SS: %d",
               CONST_TO_OR_CIRCUIT(circ)->p_chan->global_identifier,
               CONST_TO_OR_CIRCUIT(circ)->p_circ_id,
               cc->cwnd,
               cc->inflight,
               cc->next_cc_event,
               cc->ewma_rtt_usec/1000,
               westwood_rtt_signal(cc)/1000,
               cc->in_slow_start
               );
    }
  }

  /* Update inflight with ack */
  cc->inflight = cc->inflight - cc->sendme_inc;

  return 0;
}
