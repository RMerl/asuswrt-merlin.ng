/* Copyright (c) 2019-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file congestion_control_vegas.c
 * \brief Code that implements the TOR_VEGAS congestion control algorithm
 *        from Proposal #324.
 */

#define TOR_CONGESTION_CONTROL_VEGAS_PRIVATE

#include "core/or/or.h"

#include "core/or/crypt_path.h"
#include "core/or/or_circuit_st.h"
#include "core/or/sendme.h"
#include "core/or/congestion_control_st.h"
#include "core/or/congestion_control_common.h"
#include "core/or/congestion_control_vegas.h"
#include "core/or/circuitlist.h"
#include "core/or/circuituse.h"
#include "core/or/origin_circuit_st.h"
#include "core/or/channel.h"
#include "feature/nodelist/networkstatus.h"
#include "feature/control/control_events.h"

#define OUTBUF_CELLS (2*TLS_RECORD_MAX_CELLS)

/* sbws circs are two hops, so params are based on 2 outbufs of cells */
#define VEGAS_ALPHA_SBWS_DFLT (2*OUTBUF_CELLS-TLS_RECORD_MAX_CELLS)
#define VEGAS_BETA_SBWS_DFLT (2*OUTBUF_CELLS)
#define VEGAS_GAMMA_SBWS_DFLT (2*OUTBUF_CELLS)
#define VEGAS_DELTA_SBWS_DFLT (4*OUTBUF_CELLS)

/* Exits are three hops, so params are based on 3 outbufs of cells */
#define VEGAS_ALPHA_EXIT_DFLT (3*OUTBUF_CELLS-TLS_RECORD_MAX_CELLS)
#define VEGAS_BETA_EXIT_DFLT (3*OUTBUF_CELLS)
#define VEGAS_GAMMA_EXIT_DFLT (3*OUTBUF_CELLS)
#define VEGAS_DELTA_EXIT_DFLT (5*OUTBUF_CELLS)

/* Onion rends are six hops, so params are based on 6 outbufs of cells */
#define VEGAS_ALPHA_ONION_DFLT (6*OUTBUF_CELLS-TLS_RECORD_MAX_CELLS)
#define VEGAS_BETA_ONION_DFLT (6*OUTBUF_CELLS)
#define VEGAS_GAMMA_ONION_DFLT (6*OUTBUF_CELLS)
#define VEGAS_DELTA_ONION_DFLT (8*OUTBUF_CELLS)

/* Single Onions are three hops, so params are based on 3 outbufs of cells */
#define VEGAS_ALPHA_SOS_DFLT (3*OUTBUF_CELLS-TLS_RECORD_MAX_CELLS)
#define VEGAS_BETA_SOS_DFLT (3*OUTBUF_CELLS)
#define VEGAS_GAMMA_SOS_DFLT (3*OUTBUF_CELLS)
#define VEGAS_DELTA_SOS_DFLT (5*OUTBUF_CELLS)

/* Vanguard Onions are 7 hops (or 8 if both sides use vanguards, but that
 * should be rare), so params are based on 7 outbufs of cells */
#define VEGAS_ALPHA_VG_DFLT (7*OUTBUF_CELLS-TLS_RECORD_MAX_CELLS)
#define VEGAS_BETA_VG_DFLT (7*OUTBUF_CELLS)
#define VEGAS_GAMMA_VG_DFLT (7*OUTBUF_CELLS)
#define VEGAS_DELTA_VG_DFLT (9*OUTBUF_CELLS)

#define VEGAS_BDP_MIX_PCT       100

/**
 * The original TCP Vegas used only a congestion window BDP estimator. We
 * believe that the piecewise estimator is likely to perform better, but
 * for purposes of experimentation, we might as well have a way to blend
 * them. It also lets us set Vegas to its original estimator while other
 * algorithms on the same network use piecewise (by setting the
 * 'vegas_bdp_mix_pct' consensus parameter to 100, while leaving the
 * 'cc_bdp_alg' parameter set to piecewise).
 *
 * Returns a percentage weighted average between the CWND estimator and
 * the specified consensus BDP estimator.
 */
static inline uint64_t
vegas_bdp_mix(const congestion_control_t *cc)
{
  return cc->vegas_params.bdp_mix_pct*cc->bdp[BDP_ALG_CWND_RTT]/100 +
         (100-cc->vegas_params.bdp_mix_pct)*cc->bdp[cc->bdp_alg]/100;
}

/**
 * Cache Vegas consensus parameters.
 */
void
congestion_control_vegas_set_params(congestion_control_t *cc,
                                    cc_path_t path)
{
  tor_assert(cc->cc_alg == CC_ALG_VEGAS);
  const char *alpha_str = NULL, *beta_str = NULL, *gamma_str = NULL;
  const char *delta_str = NULL;
  int alpha, beta, gamma, delta;

  switch (path) {
    case CC_PATH_SBWS:
      alpha_str = "cc_vegas_alpha_sbws";
      beta_str = "cc_vegas_beta_sbws";
      gamma_str = "cc_vegas_gamma_sbws";
      delta_str = "cc_vegas_delta_sbws";
      alpha = VEGAS_ALPHA_SBWS_DFLT;
      beta = VEGAS_BETA_SBWS_DFLT;
      gamma = VEGAS_GAMMA_SBWS_DFLT;
      delta = VEGAS_DELTA_SBWS_DFLT;
      break;
    case CC_PATH_EXIT:
      alpha_str = "cc_vegas_alpha_exit";
      beta_str = "cc_vegas_beta_exit";
      gamma_str = "cc_vegas_gamma_exit";
      delta_str = "cc_vegas_delta_exit";
      alpha = VEGAS_ALPHA_EXIT_DFLT;
      beta = VEGAS_BETA_EXIT_DFLT;
      gamma = VEGAS_GAMMA_EXIT_DFLT;
      delta = VEGAS_DELTA_EXIT_DFLT;
      break;
    case CC_PATH_ONION:
      alpha_str = "cc_vegas_alpha_onion";
      beta_str = "cc_vegas_beta_onion";
      gamma_str = "cc_vegas_gamma_onion";
      delta_str = "cc_vegas_delta_onion";
      alpha = VEGAS_ALPHA_ONION_DFLT;
      beta = VEGAS_BETA_ONION_DFLT;
      gamma = VEGAS_GAMMA_ONION_DFLT;
      delta = VEGAS_DELTA_ONION_DFLT;
      break;
    case CC_PATH_ONION_SOS:
      alpha_str = "cc_vegas_alpha_sos";
      beta_str = "cc_vegas_beta_sos";
      gamma_str = "cc_vegas_gamma_sos";
      delta_str = "cc_vegas_delta_sos";
      alpha = VEGAS_ALPHA_SOS_DFLT;
      beta = VEGAS_BETA_SOS_DFLT;
      gamma = VEGAS_GAMMA_SOS_DFLT;
      delta = VEGAS_DELTA_SOS_DFLT;
      break;
    case CC_PATH_ONION_VG:
      alpha_str = "cc_vegas_alpha_vg";
      beta_str = "cc_vegas_beta_vg";
      gamma_str = "cc_vegas_gamma_vg";
      delta_str = "cc_vegas_delta_vg";
      alpha = VEGAS_ALPHA_VG_DFLT;
      beta = VEGAS_BETA_VG_DFLT;
      gamma = VEGAS_GAMMA_VG_DFLT;
      delta = VEGAS_DELTA_VG_DFLT;
      break;
    default:
      tor_assert(0);
      break;
  }

  cc->vegas_params.alpha =
   networkstatus_get_param(NULL, alpha_str,
      alpha,
      0,
      1000);

  cc->vegas_params.beta =
   networkstatus_get_param(NULL, beta_str,
      beta,
      0,
      1000);

  cc->vegas_params.gamma =
   networkstatus_get_param(NULL, gamma_str,
      gamma,
      0,
      1000);

  cc->vegas_params.delta =
   networkstatus_get_param(NULL, delta_str,
      delta,
      0,
      INT32_MAX);

  cc->vegas_params.bdp_mix_pct =
   networkstatus_get_param(NULL, "cc_vegas_bdp_mix",
      VEGAS_BDP_MIX_PCT,
      0,
      100);
}

/**
 * Process a SENDME and update the congestion window according to the
 * rules specified in TOR_VEGAS of Proposal #324.
 *
 * Essentially, this algorithm attempts to measure queue lengths on
 * the circuit by subtracting the bandwidth-delay-product estimate
 * from the current congestion window.
 *
 * If the congestion window is larger than the bandwidth-delay-product,
 * then data is assumed to be queuing. We reduce the congestion window
 * in that case.
 *
 * If the congestion window is smaller than the bandwidth-delay-product,
 * then there is spare bandwidth capacity on the circuit. We increase the
 * congestion window in that case.
 *
 * The congestion window is updated only once every congestion window worth of
 * packets, even if the signal persists. It is also updated whenever the
 * upstream orcon blocks, or unblocks. This minimizes local client queues.
 */
int
congestion_control_vegas_process_sendme(congestion_control_t *cc,
                                        const circuit_t *circ,
                                        const crypt_path_t *layer_hint)
{
  uint64_t queue_use;

  tor_assert(cc && cc->cc_alg == CC_ALG_VEGAS);
  tor_assert(circ);

  /* Update ack counter until next congestion signal event is allowed */
  if (cc->next_cc_event)
    cc->next_cc_event--;

  /* Compute BDP and RTT. If we did not update, don't run the alg */
  if (!congestion_control_update_circuit_estimates(cc, circ, layer_hint)) {
    cc->inflight = cc->inflight - cc->sendme_inc;
    return 0;
  }

  /* We only update anything once per window */
  if (cc->next_cc_event == 0) {
    /* The queue use is the amount in which our cwnd is above BDP;
     * if it is below, then 0 queue use. */
    if (vegas_bdp_mix(cc) > cc->cwnd)
      queue_use = 0;
    else
      queue_use = cc->cwnd - vegas_bdp_mix(cc);

    if (cc->in_slow_start) {
      if (queue_use < cc->vegas_params.gamma && !cc->blocked_chan) {
        /* Grow to BDP immediately, then exponential growth until
         * congestion signal. Increment by at least 2 sendme's worth. */
        cc->cwnd = MAX(cc->cwnd + MAX(CWND_INC_SS(cc), 2*cc->sendme_inc),
                       vegas_bdp_mix(cc));
      } else {
        /* Congestion signal: Set cwnd to gamma threshhold */
        cc->cwnd = vegas_bdp_mix(cc) + cc->vegas_params.gamma;
        cc->in_slow_start = 0;
        log_info(LD_CIRC, "CC: TOR_VEGAS exiting slow start");

        /* We need to report that slow start has exited ASAP,
         * for sbws bandwidth measurement. */
        if (CIRCUIT_IS_ORIGIN(circ)) {
          /* We must discard const here because the event modifies fields :/ */
          control_event_circ_bandwidth_used_for_circ(
                  TO_ORIGIN_CIRCUIT((circuit_t*)circ));
        }
      }
    } else {
      if (queue_use > cc->vegas_params.delta) {
        cc->cwnd = vegas_bdp_mix(cc) + cc->vegas_params.delta - CWND_INC(cc);
      } else if (queue_use > cc->vegas_params.beta || cc->blocked_chan) {
        cc->cwnd -= CWND_INC(cc);
      } else if (queue_use < cc->vegas_params.alpha) {
        cc->cwnd += CWND_INC(cc);
      }
    }

    /* cwnd can never fall below 1 increment */
    cc->cwnd = MAX(cc->cwnd, cc->cwnd_min);

    /* Schedule next update */
    cc->next_cc_event = CWND_UPDATE_RATE(cc);

    if (CIRCUIT_IS_ORIGIN(circ)) {
      log_info(LD_CIRC,
                 "CC: TOR_VEGAS Circuit %d "
                 "CWND: %"PRIu64", "
                 "INFL: %"PRIu64", "
                 "VBDP: %"PRIu64", "
                 "QUSE: %"PRIu64", "
                 "NCCE: %"PRIu64", "
                 "SS: %d",
               CONST_TO_ORIGIN_CIRCUIT(circ)->global_identifier,
               cc->cwnd,
               cc->inflight,
               vegas_bdp_mix(cc),
               queue_use,
               cc->next_cc_event,
               cc->in_slow_start
               );
    } else {
      log_info(LD_CIRC,
                 "CC: TOR_VEGAS Circuit %"PRIu64":%d "
                 "CWND: %"PRIu64", "
                 "INFL: %"PRIu64", "
                 "VBDP: %"PRIu64", "
                 "QUSE: %"PRIu64", "
                 "NCCE: %"PRIu64", "
                 "SS: %d",
               CONST_TO_OR_CIRCUIT(circ)->p_chan->global_identifier,
               CONST_TO_OR_CIRCUIT(circ)->p_circ_id,
               cc->cwnd,
               cc->inflight,
               vegas_bdp_mix(cc),
               queue_use,
               cc->next_cc_event,
               cc->in_slow_start
               );
    }
  }

  /* Update inflight with ack */
  cc->inflight = cc->inflight - cc->sendme_inc;

  return 0;
}
