/* Copyright (c) 2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file congestion_control_common.c
 * \brief Common code used by all congestion control algorithms.
 */

#define TOR_CONGESTION_CONTROL_COMMON_PRIVATE
#define TOR_CONGESTION_CONTROL_PRIVATE

#include "core/or/or.h"

#include "core/crypto/onion_crypto.h"
#include "core/or/circuitlist.h"
#include "core/or/crypt_path.h"
#include "core/or/or_circuit_st.h"
#include "core/or/origin_circuit_st.h"
#include "core/or/channel.h"
#include "core/mainloop/connection.h"
#include "core/or/sendme.h"
#include "core/or/congestion_control_st.h"
#include "core/or/congestion_control_common.h"
#include "core/or/congestion_control_vegas.h"
#include "core/or/congestion_control_st.h"
#include "core/or/conflux.h"
#include "core/or/conflux_util.h"
#include "core/or/trace_probes_cc.h"
#include "lib/time/compat_time.h"
#include "feature/nodelist/networkstatus.h"
#include "app/config/config.h"

#include "trunnel/congestion_control.h"
#include "trunnel/extension.h"

/* Consensus parameter defaults.
 *
 * More details for each of the parameters can be found in proposal 324,
 * section 6.5 including tuning notes. */
#define SENDME_INC_DFLT (TLS_RECORD_MAX_CELLS)
#define CIRCWINDOW_INIT (4*SENDME_INC_DFLT)

#define CC_ALG_DFLT (CC_ALG_VEGAS)
#define CC_ALG_DFLT_ALWAYS (CC_ALG_VEGAS)

#define CWND_INC_DFLT (1)
#define CWND_INC_PCT_SS_DFLT (100)
#define CWND_INC_RATE_DFLT (SENDME_INC_DFLT)

#define CWND_MIN_DFLT (CIRCWINDOW_INIT)
#define CWND_MAX_DFLT (INT32_MAX)

#define BWE_SENDME_MIN_DFLT (5)

#define N_EWMA_CWND_PCT_DFLT (50)
#define N_EWMA_MAX_DFLT (10)
#define N_EWMA_SS_DFLT (2)

#define RTT_RESET_PCT_DFLT (100)

/* BDP algorithms for each congestion control algorithms use the piecewise
 * estimattor. See section 3.1.4 of proposal 324. */
#define WESTWOOD_BDP_ALG BDP_ALG_PIECEWISE
#define VEGAS_BDP_MIX_ALG BDP_ALG_PIECEWISE
#define NOLA_BDP_ALG BDP_ALG_PIECEWISE

/* Indicate OR connection buffer limitations used to stop or start accepting
 * cells in its outbuf.
 *
 * These watermarks are historical to tor in a sense that they've been used
 * almost from the genesis point. And were likely defined to fit the bounds of
 * TLS records of 16KB which would be around 32 cells.
 *
 * These are defaults of the consensus parameter "orconn_high" and "orconn_low"
 * values. */
#define OR_CONN_HIGHWATER_DFLT (32*1024)
#define OR_CONN_LOWWATER_DFLT (16*1024)

/* Low and high values of circuit cell queue sizes. They are used to tell when
 * to start or stop reading on the streams attached on the circuit.
 *
 * These are defaults of the consensus parameters "cellq_high" and "cellq_low".
 */
#define CELL_QUEUE_LOW_DFLT (10)
#define CELL_QUEUE_HIGH_DFLT (256)

static bool congestion_control_update_circuit_bdp(congestion_control_t *,
                                                  const circuit_t *,
                                                  uint64_t);
/* Number of times the RTT value was reset. For MetricsPort. */
static uint64_t num_rtt_reset;

/* Number of times the clock was stalled. For MetricsPort. */
static uint64_t num_clock_stalls;

/* Consensus parameters cached. The non static ones are extern. */
static uint32_t cwnd_max = CWND_MAX_DFLT;
int32_t cell_queue_high = CELL_QUEUE_HIGH_DFLT;
int32_t cell_queue_low = CELL_QUEUE_LOW_DFLT;
uint32_t or_conn_highwater = OR_CONN_HIGHWATER_DFLT;
uint32_t or_conn_lowwater = OR_CONN_LOWWATER_DFLT;
uint8_t cc_sendme_inc = SENDME_INC_DFLT;
STATIC cc_alg_t cc_alg = CC_ALG_DFLT;

/**
 * Number of cwnd worth of sendme acks to smooth RTT and BDP with,
 * using N_EWMA */
static uint8_t n_ewma_cwnd_pct = N_EWMA_CWND_PCT_DFLT;

/**
 * Maximum number N for the N-count EWMA averaging of RTT and BDP.
 */
static uint8_t n_ewma_max = N_EWMA_MAX_DFLT;

/**
 * Maximum number N for the N-count EWMA averaging of RTT in Slow Start.
 */
static uint8_t n_ewma_ss = N_EWMA_SS_DFLT;

/**
 * Minimum number of sendmes before we begin BDP estimates
 */
static uint8_t bwe_sendme_min = BWE_SENDME_MIN_DFLT;

/**
 * Percentage of the current RTT to use when resetting the minimum RTT
 * for a circuit. (RTT is reset when the cwnd hits cwnd_min).
 */
static uint8_t rtt_reset_pct = RTT_RESET_PCT_DFLT;

/** Metric to count the number of congestion control circuits **/
uint64_t cc_stats_circs_created = 0;

/** Return the number of RTT reset that have been done. */
uint64_t
congestion_control_get_num_rtt_reset(void)
{
  return num_rtt_reset;
}

/** Return the number of clock stalls that have been done. */
uint64_t
congestion_control_get_num_clock_stalls(void)
{
  return num_clock_stalls;
}

/**
 * Update global congestion control related consensus parameter values,
 * every consensus update.
 */
void
congestion_control_new_consensus_params(const networkstatus_t *ns)
{
#define CELL_QUEUE_HIGH_MIN (1)
#define CELL_QUEUE_HIGH_MAX (1000)
  cell_queue_high = networkstatus_get_param(ns, "cellq_high",
      CELL_QUEUE_HIGH_DFLT,
      CELL_QUEUE_HIGH_MIN,
      CELL_QUEUE_HIGH_MAX);

#define CELL_QUEUE_LOW_MIN (1)
#define CELL_QUEUE_LOW_MAX (1000)
  cell_queue_low = networkstatus_get_param(ns, "cellq_low",
      CELL_QUEUE_LOW_DFLT,
      CELL_QUEUE_LOW_MIN,
      CELL_QUEUE_LOW_MAX);

#define OR_CONN_HIGHWATER_MIN (CELL_PAYLOAD_SIZE)
#define OR_CONN_HIGHWATER_MAX (INT32_MAX)
  or_conn_highwater =
    networkstatus_get_param(ns, "orconn_high",
        OR_CONN_HIGHWATER_DFLT,
        OR_CONN_HIGHWATER_MIN,
        OR_CONN_HIGHWATER_MAX);

#define OR_CONN_LOWWATER_MIN (CELL_PAYLOAD_SIZE)
#define OR_CONN_LOWWATER_MAX (INT32_MAX)
  or_conn_lowwater =
    networkstatus_get_param(ns, "orconn_low",
        OR_CONN_LOWWATER_DFLT,
        OR_CONN_LOWWATER_MIN,
        OR_CONN_LOWWATER_MAX);

#define CWND_MAX_MIN 500
#define CWND_MAX_MAX (INT32_MAX)
  cwnd_max =
    networkstatus_get_param(NULL, "cc_cwnd_max",
        CWND_MAX_DFLT,
        CWND_MAX_MIN,
        CWND_MAX_MAX);

#define RTT_RESET_PCT_MIN (0)
#define RTT_RESET_PCT_MAX (100)
  rtt_reset_pct =
    networkstatus_get_param(NULL, "cc_rtt_reset_pct",
        RTT_RESET_PCT_DFLT,
        RTT_RESET_PCT_MIN,
        RTT_RESET_PCT_MAX);

#define SENDME_INC_MIN 1
#define SENDME_INC_MAX (254)
  cc_sendme_inc =
    networkstatus_get_param(NULL, "cc_sendme_inc",
        SENDME_INC_DFLT,
        SENDME_INC_MIN,
        SENDME_INC_MAX);

#define CC_ALG_MIN 0
#define CC_ALG_MAX (NUM_CC_ALGS-1)
  cc_alg =
    networkstatus_get_param(NULL, "cc_alg",
        CC_ALG_DFLT,
        CC_ALG_MIN,
        CC_ALG_MAX);
  if (cc_alg != CC_ALG_SENDME && cc_alg != CC_ALG_VEGAS) {
    // Does not need rate limiting because consensus updates
    // are at most 1x/hour
    log_warn(LD_BUG, "Unsupported congestion control algorithm %d",
               cc_alg);
    cc_alg = CC_ALG_DFLT;
  }

#define BWE_SENDME_MIN_MIN 2
#define BWE_SENDME_MIN_MAX (20)
  bwe_sendme_min =
    networkstatus_get_param(NULL, "cc_bwe_min",
        BWE_SENDME_MIN_DFLT,
        BWE_SENDME_MIN_MIN,
        BWE_SENDME_MIN_MAX);

#define N_EWMA_CWND_PCT_MIN 1
#define N_EWMA_CWND_PCT_MAX (255)
  n_ewma_cwnd_pct =
    networkstatus_get_param(NULL, "cc_ewma_cwnd_pct",
        N_EWMA_CWND_PCT_DFLT,
        N_EWMA_CWND_PCT_MIN,
        N_EWMA_CWND_PCT_MAX);

#define N_EWMA_MAX_MIN 2
#define N_EWMA_MAX_MAX (INT32_MAX)
  n_ewma_max =
    networkstatus_get_param(NULL, "cc_ewma_max",
        N_EWMA_MAX_DFLT,
        N_EWMA_MAX_MIN,
        N_EWMA_MAX_MAX);

#define N_EWMA_SS_MIN 2
#define N_EWMA_SS_MAX (INT32_MAX)
  n_ewma_ss =
    networkstatus_get_param(NULL, "cc_ewma_ss",
        N_EWMA_SS_DFLT,
        N_EWMA_SS_MIN,
        N_EWMA_SS_MAX);
}

/**
 * Set congestion control parameters on a circuit's congestion
 * control object based on values from the consensus.
 *
 * cc_alg is the negotiated congestion control algorithm.
 *
 * sendme_inc is the number of packaged cells that a sendme cell
 * acks. This parameter will come from circuit negotiation.
 */
static void
congestion_control_init_params(congestion_control_t *cc,
                               const circuit_params_t *params,
                               cc_path_t path)
{
  const or_options_t *opts = get_options();
  cc->sendme_inc = params->sendme_inc_cells;

#define CWND_INIT_MIN SENDME_INC_DFLT
#define CWND_INIT_MAX (10000)
  cc->cwnd =
    networkstatus_get_param(NULL, "cc_cwnd_init",
        CIRCWINDOW_INIT,
        CWND_INIT_MIN,
        CWND_INIT_MAX);

#define CWND_INC_PCT_SS_MIN 1
#define CWND_INC_PCT_SS_MAX (500)
  cc->cwnd_inc_pct_ss =
    networkstatus_get_param(NULL, "cc_cwnd_inc_pct_ss",
        CWND_INC_PCT_SS_DFLT,
        CWND_INC_PCT_SS_MIN,
        CWND_INC_PCT_SS_MAX);

#define CWND_INC_MIN 1
#define CWND_INC_MAX (1000)
  cc->cwnd_inc =
    networkstatus_get_param(NULL, "cc_cwnd_inc",
        CWND_INC_DFLT,
        CWND_INC_MIN,
        CWND_INC_MAX);

#define CWND_INC_RATE_MIN 1
#define CWND_INC_RATE_MAX (250)
  cc->cwnd_inc_rate =
    networkstatus_get_param(NULL, "cc_cwnd_inc_rate",
        CWND_INC_RATE_DFLT,
        CWND_INC_RATE_MIN,
        CWND_INC_RATE_MAX);

#define CWND_MIN_MIN SENDME_INC_DFLT
#define CWND_MIN_MAX (1000)
  cc->cwnd_min =
    networkstatus_get_param(NULL, "cc_cwnd_min",
        CWND_MIN_DFLT,
        CWND_MIN_MIN,
        CWND_MIN_MAX);

  /* If the consensus says to use OG sendme, but torrc has
   * always-enabled, use the default "always" alg (vegas),
   * else use cached conensus alg. */
  if (cc_alg == CC_ALG_SENDME && opts->AlwaysCongestionControl) {
    cc->cc_alg = CC_ALG_DFLT_ALWAYS;
  } else {
    cc->cc_alg = cc_alg;
  }

  /* Algorithm-specific parameters */
  if (cc->cc_alg == CC_ALG_VEGAS) {
    congestion_control_vegas_set_params(cc, path);
  } else {
    // This should not happen anymore
    log_warn(LD_BUG, "Unknown congestion control algorithm %d",
             cc->cc_alg);
  }
}

/** Returns true if congestion control is enabled in the most recent
 * consensus, or if __AlwaysCongestionControl is set to true.
 *
 * Note that this function (and many many other functions) should not
 * be called from the CPU worker threads when handling congestion
 * control negotiation. Relevant values are marshaled into the
 * `circuit_params_t` struct, in order to be used in worker threads
 * without touching global state. Use those values in CPU worker
 * threads, instead of calling this function.
 *
 * The danger is still present, in your time, as it was in ours.
 */
bool
congestion_control_enabled(void)
{
  const or_options_t *opts = NULL;

  tor_assert_nonfatal_once(in_main_thread());

  opts = get_options();

  /* If the user has set "__AlwaysCongesttionControl",
   * then always try to negotiate congestion control, regardless
   * of consensus param. This is to be used for testing and sbws.
   *
   * Note that we do *not* allow disabling congestion control
   * if the consensus says to use it, as this is bad for queueing
   * and fairness. */
  if (opts->AlwaysCongestionControl)
    return 1;

  return cc_alg != CC_ALG_SENDME;
}

#ifdef TOR_UNIT_TESTS
/**
 * For unit tests only: set the cached consensus cc alg to
 * specified value.
 */
void
congestion_control_set_cc_enabled(void)
{
  cc_alg = CC_ALG_VEGAS;
}

/**
 * For unit tests only: set the cached consensus cc alg to
 * specified value.
 */
void
congestion_control_set_cc_disabled(void)
{
  cc_alg = CC_ALG_SENDME;
}
#endif

/**
 * Allocate and initialize fields in congestion control object.
 *
 * cc_alg is the negotiated congestion control algorithm.
 *
 * sendme_inc is the number of packaged cells that a sendme cell
 * acks. This parameter will come from circuit negotiation.
 */
static void
congestion_control_init(congestion_control_t *cc,
                        const circuit_params_t *params,
                        cc_path_t path)
{
  cc->sendme_pending_timestamps = smartlist_new();

  cc->in_slow_start = 1;
  congestion_control_init_params(cc, params, path);

  cc->next_cc_event = CWND_UPDATE_RATE(cc);
}

/** Allocate and initialize a new congestion control object */
congestion_control_t *
congestion_control_new(const circuit_params_t *params, cc_path_t path)
{
  congestion_control_t *cc = tor_malloc_zero(sizeof(congestion_control_t));

  congestion_control_init(cc, params, path);

  cc_stats_circs_created++;

  return cc;
}

/**
 * Free a congestion control object and its associated state.
 */
void
congestion_control_free_(congestion_control_t *cc)
{
  if (!cc)
    return;

  SMARTLIST_FOREACH(cc->sendme_pending_timestamps, uint64_t *, t, tor_free(t));
  smartlist_free(cc->sendme_pending_timestamps);

  tor_free(cc);
}

/**
 * Enqueue a u64 timestamp to the end of a queue of timestamps.
 */
STATIC inline void
enqueue_timestamp(smartlist_t *timestamps_u64, uint64_t timestamp_usec)
{
  uint64_t *timestamp_ptr = tor_malloc(sizeof(uint64_t));
  *timestamp_ptr = timestamp_usec;

  smartlist_add(timestamps_u64, timestamp_ptr);
}

/**
 * Dequeue a u64 monotime usec timestamp from the front of a
 * smartlist of pointers to 64.
 */
static inline uint64_t
dequeue_timestamp(smartlist_t *timestamps_u64_usecs)
{
  uint64_t *timestamp_ptr = smartlist_get(timestamps_u64_usecs, 0);
  uint64_t timestamp_u64;

  if (BUG(!timestamp_ptr)) {
    log_err(LD_CIRC, "Congestion control timestamp list became empty!");
    return 0;
  }

  timestamp_u64 = *timestamp_ptr;
  smartlist_del_keeporder(timestamps_u64_usecs, 0);
  tor_free(timestamp_ptr);

  return timestamp_u64;
}

/**
 * Returns the number N of N-count EWMA, for averaging RTT and BDP over
 * N SENDME acks.
 *
 * This N is bracketed between a divisor of the number of acks in a CWND
 * and a max value. It is always at least 2.
 */
static inline uint64_t
n_ewma_count(const congestion_control_t *cc)
{
  uint64_t ewma_cnt = 0;

  if (cc->in_slow_start) {
    /* In slow-start, we check the Vegas condition every sendme,
     * so much lower ewma counts are needed. */
    ewma_cnt = n_ewma_ss;
  } else {
    /* After slow-start, we check the Vegas condition only once per
     * CWND, so it is better to average over longer periods. */
    ewma_cnt = MIN(CWND_UPDATE_RATE(cc)*n_ewma_cwnd_pct/100,
                          n_ewma_max);
  }
  ewma_cnt = MAX(ewma_cnt, 2);
  return ewma_cnt;
}

/**
 * Get a package window from either old sendme logic, or congestion control.
 *
 * A package window is how many cells you can still send.
 */
int
congestion_control_get_package_window(const circuit_t *circ,
                                      const crypt_path_t *cpath)
{
  int package_window;
  congestion_control_t *cc;

  tor_assert(circ);

  if (cpath) {
    package_window = cpath->package_window;
    cc = cpath->ccontrol;
  } else {
    package_window = circ->package_window;
    cc = circ->ccontrol;
  }

  if (!cc) {
    return package_window;
  } else {
    /* Inflight can be above cwnd if cwnd was just reduced */
    if (cc->inflight > cc->cwnd)
      return 0;
    /* In the extremely unlikely event that cwnd-inflight is larger than
     * INT32_MAX, just return that cap, so old code doesn't explode. */
    else if (cc->cwnd - cc->inflight > INT32_MAX)
      return INT32_MAX;
    else
      return (int)(cc->cwnd - cc->inflight);
  }
}

/**
 * Returns the number of cells that are acked by every sendme.
 */
int
sendme_get_inc_count(const circuit_t *circ, const crypt_path_t *layer_hint)
{
  int sendme_inc = CIRCWINDOW_INCREMENT;
  congestion_control_t *cc = NULL;

  if (layer_hint) {
    cc = layer_hint->ccontrol;
  } else {
    cc = circ->ccontrol;
  }

  if (cc) {
    sendme_inc = cc->sendme_inc;
  }

  return sendme_inc;
}

/** Return true iff the next cell we send will result in the other endpoint
 * sending a SENDME.
 *
 * We are able to know that because the package or inflight window value minus
 * one cell (the possible SENDME cell) should be a multiple of the
 * cells-per-sendme increment value (set via consensus parameter, negotiated
 * for the circuit, and passed in as sendme_inc).
 *
 * This function is used when recording a cell digest and this is done quite
 * low in the stack when decrypting or encrypting a cell. The window is only
 * updated once the cell is actually put in the outbuf.
 */
bool
circuit_sent_cell_for_sendme(const circuit_t *circ,
                             const crypt_path_t *layer_hint)
{
  congestion_control_t *cc;
  int window;

  tor_assert(circ);

  if (layer_hint) {
    window = layer_hint->package_window;
    cc = layer_hint->ccontrol;
  } else {
    window = circ->package_window;
    cc = circ->ccontrol;
  }

  /* If we are using congestion control and the alg is not
   * old-school 'fixed', then use cc->inflight to determine
   * when sendmes will be sent */
  if (cc) {
    if (!cc->inflight)
      return false;

    /* This check must be +1 because this function is called *before*
     * inflight is incremented for the sent cell */
    if ((cc->inflight+1) % cc->sendme_inc != 0)
      return false;

    return true;
  }

  /* At the start of the window, no SENDME will be expected. */
  if (window == CIRCWINDOW_START) {
    return false;
  }

  /* Are we at the limit of the increment and if not, we don't expect next
   * cell is a SENDME.
   *
   * We test against the window minus 1 because when we are looking if the
   * next cell is a SENDME, the window (either package or deliver) hasn't been
   * decremented just yet so when this is called, we are currently processing
   * the "window - 1" cell.
   */
  if (((window - 1) % CIRCWINDOW_INCREMENT) != 0) {
    return false;
  }

  /* Next cell is expected to be a SENDME. */
  return true;
}

/**
 * Call-in to tell congestion control code that this circuit sent a cell.
 *
 * This updates the 'inflight' counter, and if this is a cell that will
 * cause the other end to send a SENDME, record the current time in a list
 * of pending timestamps, so that we can later compute the circuit RTT when
 * the SENDME comes back. */
void
congestion_control_note_cell_sent(congestion_control_t *cc,
                                  const circuit_t *circ,
                                  const crypt_path_t *cpath)
{
  tor_assert(circ);
  tor_assert(cc);

  /* Is this the last cell before a SENDME? The idea is that if the
   * package_window reaches a multiple of the increment, after this cell, we
   * should expect a SENDME. Note that this function must be called *before*
   * we account for the sent cell. */
  if (!circuit_sent_cell_for_sendme(circ, cpath)) {
    cc->inflight++;
    return;
  }

  cc->inflight++;

  /* Record this cell time for RTT computation when SENDME arrives */
  enqueue_timestamp(cc->sendme_pending_timestamps,
                    monotime_absolute_usec());
}

/**
 * Upon receipt of a SENDME, pop the oldest timestamp off the timestamp
 * list, and use this to update RTT.
 *
 * Returns true if circuit estimates were successfully updated, false
 * otherwise.
 */
bool
congestion_control_update_circuit_estimates(congestion_control_t *cc,
                                            const circuit_t *circ)
{
  uint64_t now_usec = monotime_absolute_usec();

  /* Update RTT first, then BDP. BDP needs fresh RTT */
  uint64_t curr_rtt_usec = congestion_control_update_circuit_rtt(cc, now_usec);
  return congestion_control_update_circuit_bdp(cc, circ, curr_rtt_usec);
}

/**
 * Returns true if we have enough time data to use heuristics
 * to compare RTT to a baseline.
 */
static bool
time_delta_should_use_heuristics(const congestion_control_t *cc)
{
  /* If we have exited slow start and also have an EWMA RTT, we
   * should have processed at least a cwnd worth of RTTs */
  if (!cc->in_slow_start && cc->ewma_rtt_usec) {
    return true;
  }

  /* Not enough data to estimate clock jumps */
  return false;
}

STATIC bool is_monotime_clock_broken = false;

/**
 * Returns true if the monotime delta is 0, or is significantly
 * different than the previous delta. Either case indicates
 * that the monotime time source stalled or jumped.
 *
 * Also caches the clock state in the is_monotime_clock_broken flag,
 * so we can also provide a is_monotime_clock_reliable() function,
 * used by flow control rate timing.
 */
STATIC bool
time_delta_stalled_or_jumped(const congestion_control_t *cc,
                             uint64_t old_delta, uint64_t new_delta)
{
#define DELTA_DISCREPENCY_RATIO_MAX 5000
  /* If we have a 0 new_delta, that is definitely a monotime stall */
  if (new_delta == 0) {
    static ratelim_t stall_info_limit = RATELIM_INIT(60);
    log_fn_ratelim(&stall_info_limit, LOG_INFO, LD_CIRC,
           "Congestion control cannot measure RTT due to monotime stall.");

    is_monotime_clock_broken = true;
    return true;
  }

  /*
   * For the heuristic cases, we need at least a few timestamps,
   * to average out any previous partial stalls or jumps. So until
   * that point, let's just assume its OK.
   */
  if (!time_delta_should_use_heuristics(cc)) {
    return false;
  }

  /* If old_delta is significantly larger than new_delta, then
   * this means that the monotime clock could have recently
   * stopped moving forward. However, use the cache for this
   * value, because it may also be caused by network activity,
   * or by a previous clock jump that was not detected.
   *
   * So if we have not gotten a 0-delta recently, we will
   * still allow this new low RTT, but just yell about it. */
  if (old_delta > new_delta * DELTA_DISCREPENCY_RATIO_MAX) {
    static ratelim_t dec_notice_limit = RATELIM_INIT(300);
    log_fn_ratelim(&dec_notice_limit, LOG_NOTICE, LD_CIRC,
           "Sudden decrease in circuit RTT (%"PRIu64" vs %"PRIu64
           "), likely due to clock jump.",
           new_delta/1000, old_delta/1000);

    return is_monotime_clock_broken;
  }

  /* If new_delta is significantly larger than old_delta, then
   * this means that the monotime clock suddenly jumped forward.
   * However, do not cache this value, because it may also be caused
   * by network activity.
   */
  if (new_delta > old_delta * DELTA_DISCREPENCY_RATIO_MAX) {
    static ratelim_t dec_notice_limit = RATELIM_INIT(300);
    log_fn_ratelim(&dec_notice_limit, LOG_PROTOCOL_WARN, LD_CIRC,
           "Sudden increase in circuit RTT (%"PRIu64" vs %"PRIu64
           "), likely due to clock jump or suspended remote endpoint.",
           new_delta/1000, old_delta/1000);

    return true;
  }

  /* All good! Update cached status, too */
  is_monotime_clock_broken = false;

  return false;
}

/**
 * Is the monotime clock stalled according to any circuits?
 */
bool
is_monotime_clock_reliable(void)
{
  return !is_monotime_clock_broken;
}

/**
 * Called when we get a SENDME. Updates circuit RTT by pulling off a
 * timestamp of when we sent the CIRCWINDOW_INCREMENT-th cell from
 * the queue of such timestamps, and comparing that to current time.
 *
 * Also updates min, max, and EWMA of RTT.
 *
 * Returns the current circuit RTT in usecs, or 0 if it could not be
 * measured (due to clock jump, stall, etc).
 */
STATIC uint64_t
congestion_control_update_circuit_rtt(congestion_control_t *cc,
                                      uint64_t now_usec)
{
  uint64_t rtt, ewma_cnt;
  uint64_t sent_at_timestamp;

  tor_assert(cc);

  /* Get the time that we sent the cell that resulted in the other
   * end sending this sendme. Use this to calculate RTT */
  sent_at_timestamp = dequeue_timestamp(cc->sendme_pending_timestamps);

  rtt = now_usec - sent_at_timestamp;

  /* Do not update RTT at all if it looks fishy */
  if (time_delta_stalled_or_jumped(cc, cc->ewma_rtt_usec, rtt)) {
    num_clock_stalls++; /* Accounting */
    return 0;
  }

  ewma_cnt = n_ewma_count(cc);

  cc->ewma_rtt_usec = n_count_ewma(rtt, cc->ewma_rtt_usec, ewma_cnt);

  if (rtt > cc->max_rtt_usec) {
    cc->max_rtt_usec = rtt;
  }

  if (cc->min_rtt_usec == 0) {
    // If we do not have a min_rtt yet, use current ewma
    cc->min_rtt_usec = cc->ewma_rtt_usec;
  } else if (cc->cwnd == cc->cwnd_min && !cc->in_slow_start) {
    // Raise min rtt if cwnd hit cwnd_min. This gets us out of a wedge state
    // if we hit cwnd_min due to an abnormally low rtt.
    uint64_t new_rtt = percent_max_mix(cc->ewma_rtt_usec, cc->min_rtt_usec,
                                       rtt_reset_pct);

    static ratelim_t rtt_notice_limit = RATELIM_INIT(300);
    log_fn_ratelim(&rtt_notice_limit, LOG_NOTICE, LD_CIRC,
            "Resetting circ RTT from %"PRIu64" to %"PRIu64" due to low cwnd",
            cc->min_rtt_usec/1000, new_rtt/1000);

    cc->min_rtt_usec = new_rtt;
    num_rtt_reset++; /* Accounting */
  } else if (cc->ewma_rtt_usec < cc->min_rtt_usec) {
    // Using the EWMA for min instead of current RTT helps average out
    // effects from other conns
    cc->min_rtt_usec = cc->ewma_rtt_usec;
  }

  return rtt;
}

/**
 * Called when we get a SENDME. Updates the bandwidth-delay-product (BDP)
 * estimates of a circuit. Several methods of computing BDP are used,
 * depending on scenario. While some congestion control algorithms only
 * use one of these methods, we update them all because it's quick and easy.
 *
 * - now_usec is the current monotime in usecs.
 * - curr_rtt_usec is the current circuit RTT in usecs. It may be 0 if no
 *   RTT could bemeasured.
 *
 * Returns true if we were able to update BDP, false otherwise.
 */
static bool
congestion_control_update_circuit_bdp(congestion_control_t *cc,
                                      const circuit_t *circ,
                                      uint64_t curr_rtt_usec)
{
  int chan_q = 0;
  unsigned int blocked_on_chan = 0;

  tor_assert(cc);

  if (CIRCUIT_IS_ORIGIN(circ)) {
    /* origin circs use n_chan */
    chan_q = circ->n_chan_cells.n;
    blocked_on_chan = circ->circuit_blocked_on_n_chan;
  } else {
    /* Both onion services and exits use or_circuit and p_chan */
    chan_q = CONST_TO_OR_CIRCUIT(circ)->p_chan_cells.n;
    blocked_on_chan = circ->circuit_blocked_on_p_chan;
  }

  /* If we have no EWMA RTT, it is because monotime has been stalled
   * or messed up the entire time so far. Set our BDP estimates directly
   * to current cwnd */
  if (!cc->ewma_rtt_usec) {
     uint64_t cwnd = cc->cwnd;

     tor_assert_nonfatal(cc->cwnd <= cwnd_max);

     /* If the channel is blocked, keep subtracting off the chan_q
      * until we hit the min cwnd. */
     if (blocked_on_chan) {
       /* Cast is fine because we're less than int32 */
       if (chan_q >= (int64_t)cwnd) {
         log_notice(LD_CIRC,
                    "Clock stall with large chanq: %d %"PRIu64, chan_q, cwnd);
         cwnd = cc->cwnd_min;
       } else {
         cwnd = MAX(cwnd - chan_q, cc->cwnd_min);
       }
       cc->blocked_chan = 1;
     } else {
       cc->blocked_chan = 0;
     }

     cc->bdp = cwnd;

     static ratelim_t dec_notice_limit = RATELIM_INIT(300);
     log_fn_ratelim(&dec_notice_limit, LOG_NOTICE, LD_CIRC,
            "Our clock has been stalled for the entire lifetime of a circuit. "
            "Performance may be sub-optimal.");

     return blocked_on_chan;
  }

  /* Congestion window based BDP will respond to changes in RTT only, and is
   * relative to cwnd growth. It is useful for correcting for BDP
   * overestimation, but if BDP is higher than the current cwnd, it will
   * underestimate it.
   *
   * We multiply here first to avoid precision issues from min_RTT being
   * close to ewma RTT. Since all fields are u64, there is plenty of
   * room here to multiply first.
   */
  cc->bdp = cc->cwnd*cc->min_rtt_usec/cc->ewma_rtt_usec;

  /* The orconn is blocked; use smaller of inflight vs SENDME */
  if (blocked_on_chan) {
    log_info(LD_CIRC, "CC: Streams blocked on circ channel. Chanq: %d",
             chan_q);

    /* A blocked channel is an immediate congestion signal, but it still
     * happens only once per cwnd */
    if (!cc->blocked_chan) {
      cc->next_cc_event = 0;
      cc->blocked_chan = 1;
    }
  } else {
    /* If we were previously blocked, emit a new congestion event
     * now that we are unblocked, to re-evaluate cwnd */
    if (cc->blocked_chan) {
      cc->blocked_chan = 0;
      cc->next_cc_event = 0;
      log_info(LD_CIRC, "CC: Streams un-blocked on circ channel. Chanq: %d",
               chan_q);
    }
  }

  if (cc->next_cc_event == 0) {
    if (CIRCUIT_IS_ORIGIN(circ)) {
      log_info(LD_CIRC,
                 "CC: Circuit %d "
                 "SENDME RTT: %"PRIu64", %"PRIu64", %"PRIu64", %"PRIu64", "
                 "BDP estimate: %"PRIu64,
               CONST_TO_ORIGIN_CIRCUIT(circ)->global_identifier,
               cc->min_rtt_usec/1000,
               curr_rtt_usec/1000,
               cc->ewma_rtt_usec/1000,
               cc->max_rtt_usec/1000,
               cc->bdp);
    } else {
      log_info(LD_CIRC,
                 "CC: Circuit %"PRIu64":%d "
                 "SENDME RTT: %"PRIu64", %"PRIu64", %"PRIu64", %"PRIu64", "
                 "%"PRIu64,
                 CONST_TO_OR_CIRCUIT(circ)->p_chan->global_identifier,
                 CONST_TO_OR_CIRCUIT(circ)->p_circ_id,
                 cc->min_rtt_usec/1000,
                 curr_rtt_usec/1000,
                 cc->ewma_rtt_usec/1000,
                 cc->max_rtt_usec/1000,
                 cc->bdp);
    }
  }

  /* We updated BDP this round if either we had a blocked channel, or
   * the curr_rtt_usec was not 0. */
  bool ret = (blocked_on_chan || curr_rtt_usec != 0);
  if (ret) {
    tor_trace(TR_SUBSYS(cc), TR_EV(bdp_update), circ, cc, curr_rtt_usec);
  }
  return ret;
}

/**
 * Dispatch the sendme to the appropriate congestion control algorithm.
 */
int
congestion_control_dispatch_cc_alg(congestion_control_t *cc,
                                   circuit_t *circ)
{
  int ret = -END_CIRC_REASON_INTERNAL;

  tor_assert_nonfatal_once(cc->cc_alg == CC_ALG_VEGAS);
  ret = congestion_control_vegas_process_sendme(cc, circ);

  if (cc->cwnd > cwnd_max) {
    static ratelim_t cwnd_limit = RATELIM_INIT(60);
    log_fn_ratelim(&cwnd_limit, LOG_NOTICE, LD_CIRC,
           "Congestion control cwnd %"PRIu64" exceeds max %d, clamping.",
           cc->cwnd, cwnd_max);
    cc->cwnd = cwnd_max;
  }

  /* If we have a non-zero RTT measurement, update conflux. */
  if (circ->conflux && cc->ewma_rtt_usec)
    conflux_update_rtt(circ->conflux, circ, cc->ewma_rtt_usec);

  return ret;
}

/**
 * Build an extension field request to negotiate congestion control.
 *
 * If congestion control is enabled, field TRUNNEL_EXT_TYPE_CC_FIELD_REQUEST
 * is created in msg_out. It is a single 0-length field that signifies that we
 * want to use congestion control. The length of msg_out is provided via
 * msg_len_out.
 *
 * If congestion control is not enabled, a payload with 0 extensions is created
 * and returned.
 *
 * If there is a failure building the request, -1 is returned, else 0.
 *
 * *msg_out must be freed if the return value is 0.
 */
int
congestion_control_build_ext_request(uint8_t **msg_out, size_t *msg_len_out)
{
  uint8_t *request = NULL;
  trn_extension_t *ext = NULL;
  trn_extension_field_t *field = NULL;

  ext = trn_extension_new();

  /* With congestion control enabled, add the request, else it is an empty
   * request in the payload. */

  if (congestion_control_enabled()) {
    /* Build the extension field that will hold the CC field. */
    field = trn_extension_field_new();
    trn_extension_field_set_field_type(field,
                                       TRUNNEL_EXT_TYPE_CC_FIELD_REQUEST);

    /* No payload indicating a request to use congestion control. */
    trn_extension_field_set_field_len(field, 0);

    /* Build final extension. */
    trn_extension_add_fields(ext, field);
    trn_extension_set_num(ext, 1);
  }

  /* Encode extension. */
  ssize_t ret = trn_extension_encoded_len(ext);
  if (BUG(ret < 0)) {
    goto err;
  }
  size_t request_len = ret;
  request = tor_malloc_zero(request_len);
  ret = trn_extension_encode(request, request_len, ext);
  if (BUG(ret < 0)) {
    tor_free(request);
    goto err;
  }
  *msg_out = request;
  *msg_len_out = request_len;

  /* Free everything, we've encoded the request now. */
  ret = 0;

 err:
  trn_extension_free(ext);
  return (int)ret;
}

/**
 * Parse a congestion control ntorv3 request payload for extensions.
 *
 * On parsing failure, -1 is returned.
 *
 * If congestion control request is present, return 1. If it is not present,
 * return 0.
 *
 * WARNING: Called from CPU worker! Must not access any global state.
 */
int
congestion_control_parse_ext_request(const uint8_t *msg, const size_t msg_len)
{
  ssize_t ret = 0;
  trn_extension_t *ext = NULL;
  size_t num_fields = 0;

  /* Parse extension from payload. */
  ret = trn_extension_parse(&ext, msg, msg_len);
  if (ret < 0) {
    goto end;
  }

  /* No extension implies no support for congestion control. In this case, we
   * simply return 0 to indicate CC is disabled. */
  if ((num_fields = trn_extension_get_num(ext)) == 0) {
    ret = 0;
    goto end;
  }

  /* Go over all fields. If any field is TRUNNEL_EXT_TYPE_CC_FIELD_REQUEST,
   * then congestion control is enabled. Ignore unknown fields. */
  for (size_t f = 0; f < num_fields; f++) {
    const trn_extension_field_t *field = trn_extension_get_fields(ext, f);
    if (field == NULL) {
      ret = -1;
      goto end;
    }

    /* For congestion control to be enabled, we only need the field type. */
    if (trn_extension_field_get_field_type(field) ==
        TRUNNEL_EXT_TYPE_CC_FIELD_REQUEST) {
      ret = 1;
      break;
    }
  }

 end:
  trn_extension_free(ext);
  return (int)ret;
}

/**
 * Given our observed parameters for circuits and congestion control,
 * as well as the parameters for the resulting circuit, build a response
 * payload using extension fields into *msg_out, with length specified in
 * *msg_out_len.
 *
 * If congestion control will be enabled, the extension field for
 * TRUNNEL_EXT_TYPE_CC_FIELD_RESPONSE will contain the sendme_inc value.
 *
 * If congestion control won't be enabled, an extension payload with 0
 * fields will be created.
 *
 * Return 0 if an extension payload was created in *msg_out, and -1 on
 * error.
 *
 * *msg_out must be freed if the return value is 0.
 *
 * WARNING: Called from CPU worker! Must not access any global state.
 */
int
congestion_control_build_ext_response(const circuit_params_t *our_params,
                                      const circuit_params_t *circ_params,
                                      uint8_t **msg_out, size_t *msg_len_out)
{
  ssize_t ret;
  uint8_t *request = NULL;
  trn_extension_t *ext = NULL;
  trn_extension_field_t *field = NULL;
  trn_extension_field_cc_t *cc_field = NULL;

  tor_assert(our_params);
  tor_assert(circ_params);
  tor_assert(msg_out);
  tor_assert(msg_len_out);

  ext = trn_extension_new();

  if (circ_params->cc_enabled) {
    /* Build the extension field that will hold the CC field. */
    field = trn_extension_field_new();
    trn_extension_field_set_field_type(field,
                                       TRUNNEL_EXT_TYPE_CC_FIELD_RESPONSE);

    /* Build the congestion control field response. */
    cc_field = trn_extension_field_cc_new();
    trn_extension_field_cc_set_sendme_inc(cc_field,
                                          our_params->sendme_inc_cells);

    ret = trn_extension_field_cc_encoded_len(cc_field);
    if (BUG(ret <= 0)) {
      trn_extension_field_free(field);
      goto err;
    }
    size_t field_len = ret;
    trn_extension_field_set_field_len(field, field_len);
    trn_extension_field_setlen_field(field, field_len);

    uint8_t *field_array = trn_extension_field_getarray_field(field);
    ret = trn_extension_field_cc_encode(field_array,
              trn_extension_field_getlen_field(field), cc_field);
    if (BUG(ret <= 0)) {
      trn_extension_field_free(field);
      goto err;
    }

    /* Build final extension. */
    trn_extension_add_fields(ext, field);
    trn_extension_set_num(ext, 1);
  }

  /* Encode extension. */
  ret = trn_extension_encoded_len(ext);
  if (BUG(ret < 0)) {
    goto err;
  }
  size_t request_len = ret;
  request = tor_malloc_zero(request_len);
  ret = trn_extension_encode(request, request_len, ext);
  if (BUG(ret < 0)) {
    tor_free(request);
    goto err;
  }
  *msg_out = request;
  *msg_len_out = request_len;

  /* We've just encoded the extension, clean everything. */
  ret = 0;

 err:
  trn_extension_free(ext);
  trn_extension_field_cc_free(cc_field);
  return (int)ret;
}

/** Return true iff the given sendme increment is within the acceptable
 * margins. */
bool
congestion_control_validate_sendme_increment(uint8_t sendme_inc)
{
  /* We will only accept this response (and this circuit) if sendme_inc
   * is within +/- 1 of the current consensus value. We should not need
   * to change cc_sendme_inc much, and if we do, we can spread out those
   * changes over smaller increments once every 4 hours. Exits that
   * violate this range should just not be used. */

  if (sendme_inc == 0)
    return false;

  if (sendme_inc > (congestion_control_sendme_inc() + 1) ||
      sendme_inc < (congestion_control_sendme_inc() - 1)) {
    return false;
  }
  return true;
}

/** Return 1 if CC is enabled which also will set the SENDME increment into our
 * params_out. Return 0 if CC is disabled. Else, return -1 on error. */
int
congestion_control_parse_ext_response(const uint8_t *msg,
                                      const size_t msg_len,
                                      circuit_params_t *params_out)
{
  ssize_t ret = 0;
  size_t num_fields = 0;
  trn_extension_t *ext = NULL;
  trn_extension_field_cc_t *cc_field = NULL;

  /* We will only accept this response (and this circuit) if sendme_inc
   * is within a factor of 2 of our consensus value. We should not need
   * to change cc_sendme_inc much, and if we do, we can spread out those
   * changes over smaller increments once every 4 hours. Exits that
   * violate this range should just not be used. */
#define MAX_SENDME_INC_NEGOTIATE_FACTOR 2

  /* Parse extension from payload. */
  ret = trn_extension_parse(&ext, msg, msg_len);
  if (ret < 0) {
    goto end;
  }

  if ((num_fields = trn_extension_get_num(ext)) == 0) {
    ret = 0;
    goto end;
  }

  /* Go over all fields. If any field is TRUNNEL_EXT_TYPE_CC_FIELD_RESPONSE,
   * then congestion control is enabled. Ignore unknown fields. */
  for (size_t f = 0; f < num_fields; f++) {
    const trn_extension_field_t *field = trn_extension_get_fields(ext, f);
    if (field == NULL) {
      ret = -1;
      goto end;
    }

    /* Only examine TRUNNEL_EXT_TYPE_CC_FIELD_RESPONSE; ignore other fields */
    if (trn_extension_field_get_field_type(field) ==
        TRUNNEL_EXT_TYPE_CC_FIELD_RESPONSE) {

      /* Parse the field into the congestion control field. */
      ret = trn_extension_field_cc_parse(&cc_field,
                trn_extension_field_getconstarray_field(field),
                trn_extension_field_getlen_field(field));
      if (ret < 0) {
        goto end;
      }

      uint8_t sendme_inc_cells =
              trn_extension_field_cc_get_sendme_inc(cc_field);
      if (!congestion_control_validate_sendme_increment(sendme_inc_cells)) {
        ret = -1;
        goto end;
      }

      /* All good. Get value and break */
      params_out->sendme_inc_cells = sendme_inc_cells;
      ret = 1;
      break;
    }
  }

 end:
  trn_extension_free(ext);
  trn_extension_field_cc_free(cc_field);

  return (int)ret;
}

/**
 * Returns a formatted string of fields containing congestion
 * control information, for the CIRC_BW control port event.
 *
 * An origin circuit can have a ccontrol object directly on it,
 * if it is an onion service, or onion client. Exit-bound clients
 * will have the ccontrol on the cpath associated with their exit
 * (the last one in the cpath list).
 *
 * WARNING: This function does not support leaky-pipe topology. It
 * is to be used for control port information only.
 */
char *
congestion_control_get_control_port_fields(const origin_circuit_t *circ)
{
  const congestion_control_t *ccontrol = NULL;
  char *ret = NULL;
  int len;

  if (TO_CIRCUIT(circ)->ccontrol) {
    ccontrol = TO_CIRCUIT(circ)->ccontrol;
  } else if (circ->cpath && circ->cpath->prev->ccontrol) {
    /* Get ccontrol for last hop (exit) if it exists */
    ccontrol = circ->cpath->prev->ccontrol;
  }

  if (!ccontrol)
    return NULL;

  len = tor_asprintf(&ret,
                     " SS=%d CWND=%"PRIu64" RTT=%"PRIu64" MIN_RTT=%"PRIu64,
                     ccontrol->in_slow_start, ccontrol->cwnd,
                     ccontrol->ewma_rtt_usec/1000,
                     ccontrol->min_rtt_usec/1000);
  if (len < 0) {
    log_warn(LD_BUG, "Unable to format event for controller.");
    return NULL;
  }

  return ret;
}
