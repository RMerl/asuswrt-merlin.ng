/* Copyright (c) 2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file conflux.c
 * \brief Conflux multipath core algorithms
 */

#define TOR_CONFLUX_PRIVATE

#include "core/or/or.h"

#include "core/or/circuit_st.h"
#include "core/or/sendme.h"
#include "core/or/relay.h"
#include "core/or/congestion_control_common.h"
#include "core/or/congestion_control_st.h"
#include "core/or/origin_circuit_st.h"
#include "core/or/circuitlist.h"
#include "core/or/circuituse.h"
#include "core/or/conflux.h"
#include "core/or/conflux_params.h"
#include "core/or/conflux_util.h"
#include "core/or/conflux_pool.h"
#include "core/or/conflux_st.h"
#include "core/or/conflux_cell.h"
#include "lib/time/compat_time.h"
#include "app/config/config.h"

/** One million microseconds in a second */
#define USEC_PER_SEC 1000000

static inline uint64_t cwnd_sendable(const circuit_t *on_circ,
                                     uint64_t in_usec, uint64_t our_usec);

/* Track the total number of bytes used by all ooo_q so it can be used by the
 * OOM handler to assess. */
static uint64_t total_ooo_q_bytes = 0;

/**
 * Determine if we should multiplex a specific relay command or not.
 *
 * TODO: Version of this that is the set of forbidden commands
 * on linked circuits
 */
bool
conflux_should_multiplex(int relay_command)
{
  switch (relay_command) {
    /* These are all fine to multiplex, and must be
     * so that ordering is preserved */
    case RELAY_COMMAND_BEGIN:
    case RELAY_COMMAND_DATA:
    case RELAY_COMMAND_END:
    case RELAY_COMMAND_CONNECTED:
      return true;

    /* We can't multiplex these because they are
     * circuit-specific */
    case RELAY_COMMAND_SENDME:
    case RELAY_COMMAND_EXTEND:
    case RELAY_COMMAND_EXTENDED:
    case RELAY_COMMAND_TRUNCATE:
    case RELAY_COMMAND_TRUNCATED:
    case RELAY_COMMAND_DROP:
      return false;

    /* We must multiplex RESOLVEs because their ordering
     * impacts begin/end. */
    case RELAY_COMMAND_RESOLVE:
    case RELAY_COMMAND_RESOLVED:
      return true;

    /* These are all circuit-specific */
    case RELAY_COMMAND_BEGIN_DIR:
    case RELAY_COMMAND_EXTEND2:
    case RELAY_COMMAND_EXTENDED2:
    case RELAY_COMMAND_ESTABLISH_INTRO:
    case RELAY_COMMAND_ESTABLISH_RENDEZVOUS:
    case RELAY_COMMAND_INTRODUCE1:
    case RELAY_COMMAND_INTRODUCE2:
    case RELAY_COMMAND_RENDEZVOUS1:
    case RELAY_COMMAND_RENDEZVOUS2:
    case RELAY_COMMAND_INTRO_ESTABLISHED:
    case RELAY_COMMAND_RENDEZVOUS_ESTABLISHED:
    case RELAY_COMMAND_INTRODUCE_ACK:
    case RELAY_COMMAND_PADDING_NEGOTIATE:
    case RELAY_COMMAND_PADDING_NEGOTIATED:
      return false;

    /* These must be multiplexed because their ordering
     * relative to BEGIN/END must be preserved */
    case RELAY_COMMAND_XOFF:
    case RELAY_COMMAND_XON:
      return true;

    /* These two are not multiplexed, because they must
     * be processed immediately to update sequence numbers
     * before any other cells are processed on the circuit */
    case RELAY_COMMAND_CONFLUX_SWITCH:
    case RELAY_COMMAND_CONFLUX_LINK:
    case RELAY_COMMAND_CONFLUX_LINKED:
    case RELAY_COMMAND_CONFLUX_LINKED_ACK:
      return false;

    default:
      log_warn(LD_BUG, "Conflux asked to multiplex unknown relay command %d",
               relay_command);
      return false;
  }
}

/** Return the leg for a circuit in a conflux set. Return NULL if not found. */
conflux_leg_t *
conflux_get_leg(conflux_t *cfx, const circuit_t *circ)
{
  conflux_leg_t *leg_found = NULL;
  tor_assert(cfx);
  tor_assert(cfx->legs);

  // Find the leg that the cell is written on
  CONFLUX_FOR_EACH_LEG_BEGIN(cfx, leg) {
    if (leg->circ == circ) {
      leg_found = leg;
      break;
    }
  } CONFLUX_FOR_EACH_LEG_END(leg);

  return leg_found;
}

/**
 * Gets the maximum last_seq_sent from all legs.
 */
uint64_t
conflux_get_max_seq_sent(const conflux_t *cfx)
{
  uint64_t max_seq_sent = 0;

  CONFLUX_FOR_EACH_LEG_BEGIN(cfx, leg) {
    if (leg->last_seq_sent > max_seq_sent) {
      max_seq_sent = leg->last_seq_sent;
    }
  } CONFLUX_FOR_EACH_LEG_END(leg);

  return max_seq_sent;
}

/**
 * Gets the maximum last_seq_recv from all legs.
 */
uint64_t
conflux_get_max_seq_recv(const conflux_t *cfx)
{
  uint64_t max_seq_recv = 0;

  CONFLUX_FOR_EACH_LEG_BEGIN(cfx, leg) {
    if (leg->last_seq_recv > max_seq_recv) {
      max_seq_recv = leg->last_seq_recv;
    }
  } CONFLUX_FOR_EACH_LEG_END(leg);

  return max_seq_recv;
}

/** Return the total memory allocation the circuit is using by conflux. If this
 * circuit is not a Conflux circuit, 0 is returned. */
uint64_t
conflux_get_circ_bytes_allocation(const circuit_t *circ)
{
  if (circ->conflux) {
    return smartlist_len(circ->conflux->ooo_q) * sizeof(conflux_cell_t);
  }
  return 0;
}

/** Return the total memory allocation in bytes by the subsystem.
 *
 * At the moment, only out of order queues are consiered. */
uint64_t
conflux_get_total_bytes_allocation(void)
{
  return total_ooo_q_bytes;
}

/** The OOM handler is asking us to try to free at least bytes_to_remove. */
size_t
conflux_handle_oom(size_t bytes_to_remove)
{
  (void) bytes_to_remove;

  /* We are not doing anything on the sets, the OOM handler will trigger a
   * circuit clean up which will affect conflux sets, by pruning oldest
   * circuits. */

  log_info(LD_CIRC, "OOM handler triggered. OOO queus allocation: %" PRIu64,
           total_ooo_q_bytes);
  return 0;
}

/**
 * Returns true if a circuit has package window space to send, and is
 * not blocked locally.
 */
static inline bool
circuit_ready_to_send(const circuit_t *circ)
{
  const congestion_control_t *cc = circuit_ccontrol(circ);
  bool cc_sendable = true;

  /* We consider ourselves blocked if we're within 1 sendme of the
   * cwnd, because inflight is decremented before this check */
  // TODO-329-TUNING: This subtraction not be right.. It depends
  // on call order wrt decisions and sendme arrival
  if (cc->inflight >= cc->cwnd) {
    cc_sendable = false;
  }

  /* Origin circuits use the package window of the last hop, and
   * have an outbound cell direction (towards exit). Otherwise,
   * there is no cpath and direction is inbound. */
  if (CIRCUIT_IS_ORIGIN(circ)) {
    return cc_sendable && !circ->circuit_blocked_on_n_chan;
  } else {
    return cc_sendable && !circ->circuit_blocked_on_p_chan;
  }
}

/**
 * Return the circuit with the minimum RTT. Do not use any
 * other circuit.
 *
 * This algorithm will minimize RTT always, and will not provide
 * any throughput benefit. We expect it to be useful for VoIP/UDP
 * use cases. Because it only uses one circuit on a leg at a time,
 * it can have more than one circuit per guard (ie: to find
 * lower-latency middles for the path).
 */
static const circuit_t *
conflux_decide_circ_minrtt(const conflux_t *cfx)
{
  uint64_t min_rtt = UINT64_MAX;
  const circuit_t *circ = NULL;

  /* Can't get here without any legs. */
  tor_assert(CONFLUX_NUM_LEGS(cfx));

  CONFLUX_FOR_EACH_LEG_BEGIN(cfx, leg) {

    /* Ignore circuits with no RTT measurement */
    if (leg->circ_rtts_usec && leg->circ_rtts_usec < min_rtt) {
      circ = leg->circ;
      min_rtt = leg->circ_rtts_usec;
    }
  } CONFLUX_FOR_EACH_LEG_END(leg);

  /* If the minRTT circuit can't send, dont send on any circuit. */
  if (!circ || !circuit_ready_to_send(circ)) {
    return NULL;
  }
  return circ;
}

/**
 * Favor the circuit with the lowest RTT that still has space in the
 * congestion window.
 *
 * This algorithm will maximize total throughput at the expense of
 * bloating out-of-order queues.
 */
static const circuit_t *
conflux_decide_circ_lowrtt(const conflux_t *cfx)
{
  uint64_t low_rtt = UINT64_MAX;
  const circuit_t *circ = NULL;

  /* Can't get here without any legs. */
  tor_assert(CONFLUX_NUM_LEGS(cfx));

  CONFLUX_FOR_EACH_LEG_BEGIN(cfx, leg) {
    /* If the package window is full, skip it */
    if (!circuit_ready_to_send(leg->circ)) {
      continue;
    }

    /* Ignore circuits with no RTT */
    if (leg->circ_rtts_usec && leg->circ_rtts_usec < low_rtt) {
      low_rtt = leg->circ_rtts_usec;
      circ = leg->circ;
    }
  } CONFLUX_FOR_EACH_LEG_END(leg);

  /* At this point, if we found a circuit, we've already validated that its
   * congestion window has room. */
  return circ;
}

/**
 * Returns the amount of room in a cwnd on a circuit.
 */
static inline uint64_t
cwnd_available(const circuit_t *on_circ)
{
  const congestion_control_t *cc = circuit_ccontrol(on_circ);
  tor_assert(cc);

  if (cc->cwnd < cc->inflight)
    return 0;

  return cc->cwnd - cc->inflight;
}

/**
 * Return the amount of congestion window we can send on
 * on_circ during in_usec. However, if we're still in
 * slow-start, send the whole window to establish the true
 * cwnd.
 */
static inline uint64_t
cwnd_sendable(const circuit_t *on_circ, uint64_t in_usec,
              uint64_t our_usec)
{
  const congestion_control_t *cc = circuit_ccontrol(on_circ);
  tor_assert(cc);
  uint64_t cwnd_adjusted = cwnd_available(on_circ);

  if (our_usec == 0 || in_usec == 0) {
    log_fn(LOG_PROTOCOL_WARN, LD_CIRC,
       "cwnd_sendable: Missing RTT data. in_usec: %" PRIu64
       " our_usec: %" PRIu64, in_usec, our_usec);
    return cwnd_adjusted;
  }

  if (cc->in_slow_start) {
    return cwnd_adjusted;
  } else {
    /* For any given leg, it has min_rtt/2 time before the 'primary'
     * leg's acks start arriving. So, the amount of data this
     * 'secondary' leg can send while the min_rtt leg transmits these
     * acks is:
     *   (cwnd_leg/(leg_rtt/2))*min_rtt/2 = cwnd_leg*min_rtt/leg_rtt.
     */
    uint64_t sendable = cwnd_adjusted*in_usec/our_usec;
    return MIN(cc->cwnd, sendable);
  }
}

/**
 * Returns true if we can switch to a new circuit, false otherwise.
 *
 * This function assumes we're primarily switching between two circuits,
 * the current and the prev. If we're using more than two circuits, we
 * need to set cfx_drain_pct to 100.
 */
static inline bool
conflux_can_switch(const conflux_t *cfx)
{
  /* If we still expected to send more cells on this circuit,
   * we're only allowed to switch if the previous circuit emptied. */
  if (cfx->cells_until_switch > 0) {
    /* If there is no prev leg, skip the inflight check. */
    if (!cfx->prev_leg) {
      return false;
    }
    const congestion_control_t *ccontrol =
      circuit_ccontrol(cfx->prev_leg->circ);

    /* If the inflight count has drained to below cfx_drain_pct
     * of the congestion window, then we can switch.
     * We check the sendme_inc because there may be un-ackable
     * data in inflight as well, and we can still switch then. */
    // TODO-329-TUNING: Should we try to switch if the prev_leg is
    // ready to send, instead of this?
    if (ccontrol->inflight < ccontrol->sendme_inc ||
        100*ccontrol->inflight <=
        conflux_params_get_drain_pct()*ccontrol->cwnd) {
      return true;
    }

    return false;
  }

  return true;
}

/**
 * Favor the circuit with the lowest RTT that still has space in the
 * congestion window up to the ratio of RTTs.
 *
 * This algorithm should only use auxillary legs up to the point
 * where their data arrives roughly the same time as the lowest
 * RTT leg. It will not utilize the full cwnd of auxillary legs,
 * except in slow start. Therefore, out-of-order queue bloat should
 * be minimized to just the slow-start phase.
 */
static const circuit_t *
conflux_decide_circ_cwndrtt(const conflux_t *cfx)
{
  uint64_t min_rtt = UINT64_MAX;
  const conflux_leg_t *leg = NULL;

  /* Can't get here without any legs. */
  tor_assert(!CONFLUX_NUM_LEGS(cfx));

  /* Find the leg with the minimum RTT.*/
  CONFLUX_FOR_EACH_LEG_BEGIN(cfx, l) {
    /* Ignore circuits with invalid RTT */
    if (l->circ_rtts_usec && l->circ_rtts_usec < min_rtt) {
      min_rtt = l->circ_rtts_usec;
      leg = l;
    }
  } CONFLUX_FOR_EACH_LEG_END(l);

  /* If the package window is has room, use it */
  if (leg && circuit_ready_to_send(leg->circ)) {
    return leg->circ;
  }

  leg = NULL;

  CONFLUX_FOR_EACH_LEG_BEGIN(cfx, l) {
    if (!circuit_ready_to_send(l->circ)) {
      continue;
    }

    /* Pick a 'min_leg' with the lowest RTT that still has
     * room in the congestion window. Note that this works for
     * min_leg itself, up to inflight. */
    if (l->circ_rtts_usec &&
        cwnd_sendable(l->circ, min_rtt, l->circ_rtts_usec) > 0) {
      leg = l;
    }
  } CONFLUX_FOR_EACH_LEG_END(l);

  /* If the circuit can't send, don't send on any circuit. */
  if (!leg || !circuit_ready_to_send(leg->circ)) {
    return NULL;
  }
  return leg->circ;
}

/**
 * This function is called when we want to send a relay cell on a
 * conflux, as well as when we want to compute available space in
 * to package from streams.
 *
 * It determines the circuit that relay command should be sent on,
 * and sends a SWITCH cell if necessary.
 *
 * It returns the circuit we should send on. If no circuits are ready
 * to send, it returns NULL.
 */
circuit_t *
conflux_decide_circ_for_send(conflux_t *cfx,
                             circuit_t *orig_circ,
                             uint8_t relay_command)
{
  /* If this command should not be multiplexed, send it on the original
   * circuit */
  if (!conflux_should_multiplex(relay_command)) {
    return orig_circ;
  }

  circuit_t *new_circ = conflux_decide_next_circ(cfx);

  /* Because our congestion window only cover relay data command, we can end up
   * in a situation where we need to send non data command when all circuits
   * are at capacity. For those cases, keep using the *current* leg,
   * so these commands arrive in-order. */
  if (!new_circ && relay_command != RELAY_COMMAND_DATA) {
    /* Curr leg should be set, because conflux_decide_next_circ() should
     * have set it earlier. No BUG() here because the only caller BUG()s. */
    if (!cfx->curr_leg) {
      log_warn(LD_BUG, "No current leg for conflux with relay command %d",
               relay_command);
      return NULL;
    }
    return cfx->curr_leg->circ;
  }

  /*
   * If we are switching to a new circuit, we need to send a SWITCH command.
   * We also need to compute an estimate of how much data we can send on
   * the new circuit before we are allowed to switch again, to rate
   * limit the frequency of switching.
   */
  if (new_circ) {
    conflux_leg_t *new_leg = conflux_get_leg(cfx, new_circ);
    tor_assert(cfx->curr_leg);

    if (new_circ != cfx->curr_leg->circ) {
      // TODO-329-TUNING: This is one mechanism to rate limit switching,
      // which should reduce the OOQ mem. However, we're not going to do that
      // until we get some data on if the memory usage is high
      cfx->cells_until_switch = 0;
        //cwnd_sendable(new_circ,cfx->curr_leg->circ_rtts_usec,
        //                         new_leg->circ_rtts_usec);

      conflux_validate_stream_lists(cfx);

      cfx->prev_leg = cfx->curr_leg;
      cfx->curr_leg = new_leg;

      tor_assert(cfx->prev_leg);
      tor_assert(cfx->curr_leg);

      uint64_t relative_seq = cfx->prev_leg->last_seq_sent -
                              cfx->curr_leg->last_seq_sent;

      tor_assert(cfx->prev_leg->last_seq_sent >=
                 cfx->curr_leg->last_seq_sent);
      conflux_send_switch_command(cfx->curr_leg->circ, relative_seq);
      cfx->curr_leg->last_seq_sent = cfx->prev_leg->last_seq_sent;
    }
  }

  return new_circ;
}

/** Called after conflux actually sent a cell on a circuit.
 * This function updates sequence number counters, and
 * switch counters.
 */
void
conflux_note_cell_sent(conflux_t *cfx, circuit_t *circ, uint8_t relay_command)
{
  conflux_leg_t *leg = NULL;

  if (!conflux_should_multiplex(relay_command)) {
    return;
  }

  leg = conflux_get_leg(cfx, circ);
  tor_assert(leg);

  leg->last_seq_sent++;

  if (cfx->cells_until_switch > 0) {
    cfx->cells_until_switch--;
  }
}

/** Find the leg with lowest non-zero curr_rtt_usec, and
 * pick it for our current leg. */
static inline bool
conflux_pick_first_leg(conflux_t *cfx)
{
  conflux_leg_t *min_leg = NULL;

  CONFLUX_FOR_EACH_LEG_BEGIN(cfx, leg) {
    /* We need to skip 0-RTT legs, since this can happen at the exit
     * when there is a race between BEGIN and LINKED_ACK, and BEGIN
     * wins the race. The good news is that because BEGIN won,
     * we don't need to consider those other legs, since they are
     * slower. */
    if (leg->circ_rtts_usec > 0) {
      if (!min_leg || leg->circ_rtts_usec < min_leg->circ_rtts_usec) {
        min_leg = leg;
      }
    }
  } CONFLUX_FOR_EACH_LEG_END(leg);

  if (!min_leg) {
    // Get the 0th leg; if it does not exist, log the set.
    // Bug 40827 managed to hit this, so let's dump the sets
    // in case it happens again.
    if (BUG(smartlist_len(cfx->legs) <= 0)) {
      // Since we have no legs, we have no idea if this is really a client
      // or server set. Try to find any that match:
      log_warn(LD_BUG, "Matching client sets:");
      conflux_log_set(LOG_WARN, cfx, true);
      log_warn(LD_BUG, "Matching server sets:");
      conflux_log_set(LOG_WARN, cfx, false);
      log_warn(LD_BUG, "End conflux set dump");
      return false;
    }

    min_leg = smartlist_get(cfx->legs, 0);
    tor_assert(min_leg);
    if (BUG(min_leg->linked_sent_usec == 0)) {
      log_warn(LD_BUG, "Conflux has no legs with non-zero RTT. "
               "Using first leg.");
      conflux_log_set(LOG_WARN, cfx, CIRCUIT_IS_ORIGIN(min_leg->circ));
    }
  }

  // TODO-329-TUNING: We may want to initialize this to a cwnd, to
  // minimize early switching?
  //cfx->cells_until_switch = circuit_ccontrol(min_leg->circ)->cwnd;
  cfx->cells_until_switch = 0;

  cfx->curr_leg = min_leg;

  return true;
}

/**
 * Returns the circuit that conflux would send on next, if
 * conflux_decide_circ_for_send were called. This is used to compute
 * available space in the package window.
 */
circuit_t *
conflux_decide_next_circ(conflux_t *cfx)
{
  // TODO-329-TUNING: Temporarily validate legs here. We can remove
  // this once tuning is complete.
  conflux_validate_legs(cfx);

  /* If the conflux set is tearing down and has no current leg,
   * bail and give up */
  if (cfx->in_full_teardown) {
    return NULL;
  }

  /* If we don't have a current leg yet, pick one.
   * (This is the only non-const operation in this function). */
  if (!cfx->curr_leg) {
    if (!conflux_pick_first_leg(cfx))
      return NULL;
  }

  /* First, check if we can switch. */
  if (!conflux_can_switch(cfx)) {
    tor_assert(cfx->curr_leg);
    circuit_t *curr_circ = cfx->curr_leg->circ;

    /* If we can't switch, and the current circuit can't send,
     * then return null. */
    if (circuit_ready_to_send(curr_circ)) {
      return curr_circ;
    }
    log_info(LD_CIRC, "Conflux can't switch; no circuit to send on.");
    return NULL;
  }

  switch (cfx->params.alg) {
    case CONFLUX_ALG_MINRTT: // latency (no ooq)
      return (circuit_t*)conflux_decide_circ_minrtt(cfx);
    case CONFLUX_ALG_LOWRTT: // high throughput (high oooq)
      return (circuit_t*)conflux_decide_circ_lowrtt(cfx);
    case CONFLUX_ALG_CWNDRTT: // throughput (low oooq)
      return (circuit_t*)conflux_decide_circ_cwndrtt(cfx);
    default:
      return NULL;
  }
}

/**
 * Called when we have a new RTT estimate for a circuit.
 */
void
conflux_update_rtt(conflux_t *cfx, circuit_t *circ, uint64_t rtt_usec)
{
  conflux_leg_t *leg = conflux_get_leg(cfx, circ);

  if (!leg) {
    log_warn(LD_BUG, "Got RTT update for circuit not in conflux");
    return;
  }

  // Update RTT
  leg->circ_rtts_usec = rtt_usec;

  // TODO-329-ARTI: For UDP latency targeting, arti could decide to launch
  // new a test leg to potentially replace this one, if a latency target
  // was requested and we now exceed it. Since C-Tor client likely
  // will not have UDP support, we aren't doing this here.
}

/**
 * Comparison function for ooo_q pqueue.
 *
 * Ensures that lower sequence numbers are at the head of the pqueue.
 */
static int
conflux_queue_cmp(const void *a, const void *b)
{
  // Compare a and b as conflux_cell_t using the seq field, and return a
  // comparison result such that the lowest seq is at the head of the pqueue.
  const conflux_cell_t *cell_a = a;
  const conflux_cell_t *cell_b = b;

  tor_assert(cell_a);
  tor_assert(cell_b);

  if (cell_a->seq < cell_b->seq) {
    return -1;
  } else if (cell_a->seq > cell_b->seq) {
    return 1;
  } else {
    return 0;
  }
}

/**
 * Get the congestion control object for a conflux circuit.
 *
 * Because conflux can only be negotiated with the last hop, we
 * can use the last hop of the cpath to obtain the congestion
 * control object for origin circuits. For non-origin circuits,
 * we can use the circuit itself.
 */
const congestion_control_t *
circuit_ccontrol(const circuit_t *circ)
{
  const congestion_control_t *ccontrol = NULL;
  tor_assert(circ);

  if (CIRCUIT_IS_ORIGIN(circ)) {
    tor_assert(CONST_TO_ORIGIN_CIRCUIT(circ)->cpath);
    tor_assert(CONST_TO_ORIGIN_CIRCUIT(circ)->cpath->prev);
    ccontrol = CONST_TO_ORIGIN_CIRCUIT(circ)->cpath->prev->ccontrol;
  } else {
    ccontrol = circ->ccontrol;
  }

  /* Conflux circuits always have congestion control*/
  tor_assert(ccontrol);
  return ccontrol;
}

// TODO-329-TUNING: For LowRTT, we can at most switch every SENDME,
// but for BLEST, we should switch at most every cwnd.. But
// we do not know the other side's CWND here.. We can at best
// asssume it is above the cwnd_min
#define CONFLUX_MIN_LINK_INCREMENT 31
/**
 * Validate and handle RELAY_COMMAND_CONFLUX_SWITCH.
 */
int
conflux_process_switch_command(circuit_t *in_circ,
                               crypt_path_t *layer_hint, cell_t *cell,
                               relay_header_t *rh)
{
  tor_assert(in_circ);
  tor_assert(cell);
  tor_assert(rh);

  conflux_t *cfx = in_circ->conflux;
  uint32_t relative_seq;
  conflux_leg_t *leg;

  if (!conflux_is_enabled(in_circ)) {
    circuit_mark_for_close(in_circ, END_CIRC_REASON_TORPROTOCOL);
    return -1;
  }

  /* If there is no conflux object negotiated, this is invalid.
   * log and close circ */
  if (!cfx) {
    log_warn(LD_BUG, "Got a conflux switch command on a circuit without "
             "conflux negotiated. Closing circuit.");

    circuit_mark_for_close(in_circ, END_CIRC_REASON_TORPROTOCOL);
    return -1;
  }

  // TODO-329-TUNING: Temporarily validate that we have all legs.
  // After tuning is complete, we can remove this.
  conflux_validate_legs(cfx);

  leg = conflux_get_leg(cfx, in_circ);

  /* If we can't find the conflux leg, we got big problems..
   * Close the circuit. */
  if (!leg) {
    log_warn(LD_BUG, "Got a conflux switch command on a circuit without "
             "conflux leg. Closing circuit.");
    circuit_mark_for_close(in_circ, END_CIRC_REASON_INTERNAL);
    return -1;
  }

  // Check source hop via layer_hint
  if (!conflux_validate_source_hop(in_circ, layer_hint)) {
    log_warn(LD_BUG, "Got a conflux switch command on a circuit with "
             "invalid source hop. Closing circuit.");
    circuit_mark_for_close(in_circ, END_CIRC_REASON_TORPROTOCOL);
    return -1;
  }

  relative_seq = conflux_cell_parse_switch(cell, rh->length);

  /*
   * We have to make sure that the switch command is truely
   * incrementing the sequence number, or else it becomes
   * a side channel that can be spammed for traffic analysis.
   */
  // TODO-329-TUNING: This can happen. Disabling for now..
  //if (relative_seq < CONFLUX_MIN_LINK_INCREMENT) {
  // log_warn(LD_CIRC, "Got a conflux switch command with a relative "
  //          "sequence number less than the minimum increment. Closing "
  //          "circuit.");
  // circuit_mark_for_close(in_circ, END_CIRC_REASON_TORPROTOCOL);
  // return -1;
  //}

  // TODO-329-UDP: When Prop#340 exits and was negotiated, ensure we're
  // in a packed cell, with another cell following, otherwise
  // this is a spammed side-channel.
  //   - We definitely should never get switches back-to-back.
  //   - We should not get switches across all legs with no data
  // But before Prop#340, it doesn't make much sense to do this.
  // C-Tor is riddled with side-channels like this anyway, unless
  // vanguards is in use. And this feature is not supported by
  // onion servicees in C-Tor, so we're good there.

  /* Update the absolute sequence number on this leg by the delta.
   * Since this cell is not multiplexed, we do not count it towards
   * absolute sequence numbers. We only increment the sequence
   * numbers for multiplexed cells. Hence there is no +1 here. */
  leg->last_seq_recv += relative_seq;

  /* Mark this data as validated for controlport and vanguards
   * dropped cell handling */
  if (CIRCUIT_IS_ORIGIN(in_circ)) {
    circuit_read_valid_data(TO_ORIGIN_CIRCUIT(in_circ), rh->length);
  }

  return 0;
}

/**
 * Process an incoming relay cell for conflux. Called from
 * connection_edge_process_relay_cell().
 *
 * Returns true if the conflux system now has well-ordered cells to deliver
 * to streams, false otherwise.
 */
bool
conflux_process_cell(conflux_t *cfx, circuit_t *in_circ,
                     crypt_path_t *layer_hint, cell_t *cell)
{
  // TODO-329-TUNING: Temporarily validate legs here. We can remove
  // this after tuning is complete.
  conflux_validate_legs(cfx);

  conflux_leg_t *leg = conflux_get_leg(cfx, in_circ);
  if (!leg) {
    log_warn(LD_BUG, "Got a conflux cell on a circuit without "
             "conflux leg. Closing circuit.");
    circuit_mark_for_close(in_circ, END_CIRC_REASON_INTERNAL);
    return false;
  }

  /* We need to make sure this cell came from the expected hop, or
   * else it could be a data corruption attack from a middle node. */
  if (!conflux_validate_source_hop(in_circ, layer_hint)) {
    circuit_mark_for_close(in_circ, END_CIRC_REASON_TORPROTOCOL);
    return false;
  }

  /* Update the running absolute sequence number */
  leg->last_seq_recv++;

  /* If this cell is next, fast-path it by processing the cell in-place */
  if (leg->last_seq_recv == cfx->last_seq_delivered + 1) {
    /* The cell is now ready to be processed, and rest of the queue should
     * now be checked for remaining elements */
    cfx->last_seq_delivered++;
    return true;
  } else if (BUG(leg->last_seq_recv <= cfx->last_seq_delivered)) {
    log_warn(LD_BUG, "Got a conflux cell with a sequence number "
             "less than the last delivered. Closing circuit.");
    circuit_mark_for_close(in_circ, END_CIRC_REASON_INTERNAL);
    return false;
  } else {
    conflux_cell_t *c_cell = tor_malloc_zero(sizeof(conflux_cell_t));
    c_cell->seq = leg->last_seq_recv;

    memcpy(&c_cell->cell, cell, sizeof(cell_t));

    smartlist_pqueue_add(cfx->ooo_q, conflux_queue_cmp,
            offsetof(conflux_cell_t, heap_idx), c_cell);
    total_ooo_q_bytes += sizeof(cell_t);

    /* This cell should not be processed yet, and the queue is not ready
     * to process because the next absolute seqnum has not yet arrived */
    return false;
  }
}

/**
 * Dequeue the top cell from our queue.
 *
 * Returns the cell as a conflux_cell_t, or NULL if the queue is empty
 * or has a hole.
 */
conflux_cell_t *
conflux_dequeue_cell(conflux_t *cfx)
{
  conflux_cell_t *top = NULL;
  if (smartlist_len(cfx->ooo_q) == 0)
    return NULL;

  top = smartlist_get(cfx->ooo_q, 0);

  /* If the top cell is the next sequence number we need, then
   * pop and return it. */
  if (top->seq == cfx->last_seq_delivered+1) {
    smartlist_pqueue_pop(cfx->ooo_q, conflux_queue_cmp,
                         offsetof(conflux_cell_t, heap_idx));
    total_ooo_q_bytes -= sizeof(cell_t);
    cfx->last_seq_delivered++;
    return top;
  } else {
    return NULL;
  }
}
