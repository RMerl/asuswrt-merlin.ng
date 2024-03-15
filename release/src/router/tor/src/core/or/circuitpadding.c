/* Copyright (c) 2017 The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file circuitpadding.c
 * \brief Circuit-level padding implementation
 *
 * \details
 *
 * This file implements Tor proposal 254 "Padding Negotiation" which is heavily
 * inspired by the paper "Toward an Efficient Website Fingerprinting Defense"
 * by M. Juarez, M. Imani, M. Perry, C. Diaz, M. Wright.
 *
 * In particular the code in this file describes mechanisms for clients to
 * negotiate various types of circuit-level padding from relays.
 *
 * Each padding type is described by a state machine (circpad_machine_spec_t),
 * which is also referred as a "padding machine" in this file.  Currently,
 * these state machines are hardcoded in the source code (e.g. see
 * circpad_machines_init()), but in the future we will be able to
 * serialize them in the torrc or the consensus.
 *
 * As specified by prop#254, clients can negotiate padding with relays by using
 * PADDING_NEGOTIATE cells. After successful padding negotiation, padding
 * machines are assigned to the circuit in their mutable form as a
 * circpad_machine_runtime_t.
 *
 * Each state of a padding state machine can be either:
 * - A histogram that specifies inter-arrival padding delays.
 * - Or a parametrized probability distribution that specifies inter-arrival
 *   delays (see circpad_distribution_type_t).
 *
 * Padding machines start from the START state and finish with the END
 * state. They can transition between states using the events in
 * circpad_event_t.
 *
 * When a padding machine reaches the END state, it gets wiped from the circuit
 * so that other padding machines can take over if needed (see
 * circpad_machine_spec_transitioned_to_end()).
 *
 ****************************
 * General notes:
 *
 * All used machines should be heap allocated and placed into
 * origin_padding_machines/relay_padding_machines so that they get correctly
 * cleaned up by the circpad_free_all() function.
 **/

#define CIRCUITPADDING_PRIVATE

#include <math.h>
#include "lib/math/fp.h"
#include "lib/math/prob_distr.h"
#include "core/or/or.h"
#include "core/or/circuitpadding.h"
#include "core/or/circuitpadding_machines.h"
#include "core/or/circuitlist.h"
#include "core/or/circuituse.h"
#include "core/mainloop/netstatus.h"
#include "core/or/relay.h"
#include "feature/stats/rephist.h"
#include "feature/nodelist/networkstatus.h"

#include "core/or/channel.h"

#include "lib/time/compat_time.h"
#include "lib/defs/time.h"
#include "lib/crypt_ops/crypto_rand.h"

#include "core/or/crypt_path_st.h"
#include "core/or/circuit_st.h"
#include "core/or/origin_circuit_st.h"
#include "core/or/or_circuit_st.h"
#include "feature/nodelist/routerstatus_st.h"
#include "feature/nodelist/node_st.h"
#include "core/or/cell_st.h"
#include "core/or/extend_info_st.h"
#include "core/crypto/relay_crypto.h"
#include "feature/nodelist/nodelist.h"

#include "src/core/or/conflux_util.h"

#include "app/config/config.h"

static inline circpad_circuit_state_t circpad_circuit_state(
                                        origin_circuit_t *circ);
static void circpad_setup_machine_on_circ(circuit_t *on_circ,
                                        const circpad_machine_spec_t *machine);
static double circpad_distribution_sample(circpad_distribution_t dist);

static inline void circpad_machine_update_state_length_for_nonpadding(
        circpad_machine_runtime_t *mi);

/** Cached consensus params */
static uint8_t circpad_padding_disabled;
static uint8_t circpad_padding_reduced;
static uint8_t circpad_global_max_padding_percent;
static uint16_t circpad_global_allowed_cells;
static uint16_t circpad_max_circ_queued_cells;

/** Global cell counts, for rate limiting */
static uint64_t circpad_global_padding_sent;
static uint64_t circpad_global_nonpadding_sent;

/** This is the list of circpad_machine_spec_t's parsed from consensus and
 *  torrc that have origin_side == 1 (ie: are for client side).
 *
 *  The machines in this smartlist are considered immutable and they are used
 *  as-is by circuits so they should not change or get deallocated in Tor's
 *  runtime and as long as circuits are alive. */
STATIC smartlist_t *origin_padding_machines = NULL;

/** This is the list of circpad_machine_spec_t's parsed from consensus and
 *  torrc that have origin_side == 0 (ie: are for relay side).
 *
 *  The machines in this smartlist are considered immutable and they are used
 *  as-is by circuits so they should not change or get deallocated in Tor's
 *  runtime and as long as circuits are alive. */
STATIC smartlist_t *relay_padding_machines = NULL;

#ifndef COCCI
/** Loop over the current padding state machines using <b>loop_var</b> as the
 *  loop variable. */
#define FOR_EACH_CIRCUIT_MACHINE_BEGIN(loop_var)                         \
  STMT_BEGIN                                                             \
  for (int loop_var = 0; loop_var < CIRCPAD_MAX_MACHINES; loop_var++) {
#define FOR_EACH_CIRCUIT_MACHINE_END } STMT_END ;

/** Loop over the current active padding state machines using <b>loop_var</b>
 *  as the loop variable. If a machine is not active, skip it. */
#define FOR_EACH_ACTIVE_CIRCUIT_MACHINE_BEGIN(loop_var, circ)            \
  FOR_EACH_CIRCUIT_MACHINE_BEGIN(loop_var)                               \
  if (!(circ)->padding_info[loop_var])                           \
    continue;
#define FOR_EACH_ACTIVE_CIRCUIT_MACHINE_END } STMT_END ;
#endif /* !defined(COCCI) */

/**
 * Free the machineinfo at an index
 */
static void
circpad_circuit_machineinfo_free_idx(circuit_t *circ, int idx)
{
  if (circ->padding_info[idx]) {
    log_fn(LOG_INFO,LD_CIRC, "Freeing padding info idx %d on circuit %u (%d)",
           idx, CIRCUIT_IS_ORIGIN(circ) ?
             TO_ORIGIN_CIRCUIT(circ)->global_identifier : 0,
           circ->purpose);

    tor_free(circ->padding_info[idx]->histogram);
    timer_free(circ->padding_info[idx]->padding_timer);
    tor_free(circ->padding_info[idx]);
  }
}

/**
 * Return true if circpad has decided to hold the circuit open for additional
 * padding. This function is used to take and retain ownership of certain
 * types of circuits that have padding machines on them, that have been passed
 * to circuit_mark_for_close().
 *
 * circuit_mark_for_close() calls this function to ask circpad if any padding
 * machines want to keep the circuit open longer to pad.
 *
 * Any non-measurement circuit that was closed for a normal, non-error reason
 * code may be held open for up to CIRCPAD_DELAY_INFINITE microseconds between
 * network-driven cell events.
 *
 * After CIRCPAD_DELAY_INFINITE microseconds of silence on a circuit, this
 * function will no longer hold it open (it will return 0 regardless of
 * what the machines ask for, and thus circuit_expire_old_circuits_clientside()
 * will close the circuit after roughly 1.25hr of idle time, maximum,
 * regardless of the padding machine state.
 */
int
circpad_marked_circuit_for_padding(circuit_t *circ, int reason)
{
  /* If the circuit purpose is measurement or path bias, don't
   * hold it open */
  if (circ->purpose == CIRCUIT_PURPOSE_PATH_BIAS_TESTING ||
      circ->purpose == CIRCUIT_PURPOSE_C_MEASURE_TIMEOUT) {
    return 0;
  }

  /* If the circuit is closed for any reason other than these three valid,
   * client-side close reasons, do not try to keep it open. It is probably
   * damaged or unusable. Note this is OK with vanguards because
   * controller-closed circuits have REASON=REQUESTED, so vanguards-closed
   * circuits will not be held open (we want them to close ASAP). */
  if (!(reason == END_CIRC_REASON_NONE ||
        reason == END_CIRC_REASON_FINISHED ||
        reason == END_CIRC_REASON_IP_NOW_REDUNDANT)) {
    return 0;
  }

  FOR_EACH_ACTIVE_CIRCUIT_MACHINE_BEGIN(i, circ) {
    circpad_machine_runtime_t *mi = circ->padding_info[i];
    if (!mi) {
      continue; // No padding runtime info; check next machine
    }

    const circpad_state_t *state = circpad_machine_current_state(mi);

    /* If we're in END state (NULL here), then check next machine */
    if (!state) {
      continue; // check next machine
    }

    /* If the machine does not want to control the circuit close itself, then
     * check the next machine */
    if (!circ->padding_machine[i]->manage_circ_lifetime) {
      continue; // check next machine
    }

    /* If the machine has reached the END state, we can close. Check next
     * machine. */
    if (mi->current_state == CIRCPAD_STATE_END) {
      continue; // check next machine
    }

    log_info(LD_CIRC, "Circuit %d is not marked for close because of a "
             "pending padding machine in index %d.",
             CIRCUIT_IS_ORIGIN(circ) ?
             TO_ORIGIN_CIRCUIT(circ)->global_identifier : 0, i);

    /* If the machine has had no network events at all within the
     * last circpad_delay_t timespan, it's in some deadlock state.
     * Tell circuit_mark_for_close() that we don't own it anymore.
     * This will allow circuit_expire_old_circuits_clientside() to
     * close it.
     */
    if (circ->padding_info[i]->last_cell_time_sec +
        (time_t)CIRCPAD_DELAY_MAX_SECS < approx_time()) {
      log_notice(LD_BUG, "Circuit %d was not marked for close because of a "
               "pending padding machine in index %d for over an hour. "
               "Circuit is a %s",
               CIRCUIT_IS_ORIGIN(circ) ?
               TO_ORIGIN_CIRCUIT(circ)->global_identifier : 0,
               i, circuit_purpose_to_string(circ->purpose));

      return 0; // abort timer reached; mark the circuit for close now
    }

    /* If we weren't marked dirty yet, let's pretend we're dirty now.
     * ("Dirty" means that a circuit has been used for application traffic
     * by Tor.. Dirty circuits have different expiry times, and are not
     * considered in counts of built circuits, etc. By claiming that we're
     * dirty, the rest of Tor will make decisions as if we were actually
     * used by application data.
     *
     * This is most important for circuit_expire_old_circuits_clientside(),
     * where we want that function to expire us after the padding machine
     * has shut down, but using the MaxCircuitDirtiness timer instead of
     * the idle circuit timer (again, we want this because we're not
     * supposed to look idle to Guard nodes that can see our lifespan). */
    if (!circ->timestamp_dirty) {
      circ->timestamp_dirty = approx_time();
      if (circ->conflux && CIRCUIT_IS_ORIGIN(circ))
        conflux_sync_circ_fields(circ->conflux, TO_ORIGIN_CIRCUIT(circ));
    }

    /* Take ownership of the circuit */
    circuit_change_purpose(circ, CIRCUIT_PURPOSE_C_CIRCUIT_PADDING);

    return 1;
  } FOR_EACH_ACTIVE_CIRCUIT_MACHINE_END;

  return 0; // No machine wanted to keep the circuit open; mark for close
}

/**
 * Free all the machineinfos in <b>circ</b> that match <b>machine_num</b>.
 *
 * If machine_ctr is non-zero, also make sure it matches the padding_info's
 * machine counter before freeing.
 *
 * Returns true if any machineinfos with that number were freed.
 * False otherwise. */
static int
free_circ_machineinfos_with_machine_num(circuit_t *circ, int machine_num,
                                        uint32_t machine_ctr)
{
  int found = 0;
  FOR_EACH_CIRCUIT_MACHINE_BEGIN(i) {
    if (circ->padding_machine[i] &&
        circ->padding_machine[i]->machine_num == machine_num) {
      /* If machine_ctr is non-zero, make sure it matches too. This
       * is to ensure that old STOP messages don't shutdown newer machines. */
      if (machine_ctr && circ->padding_info[i] &&
          circ->padding_info[i]->machine_ctr != machine_ctr) {
        log_info(LD_CIRC,
                 "Padding shutdown for wrong (old?) machine ctr: %u vs %u",
                 machine_ctr, circ->padding_info[i]->machine_ctr);
      } else {
        circpad_circuit_machineinfo_free_idx(circ, i);
        circ->padding_machine[i] = NULL;
        found = 1;
      }
    }
  } FOR_EACH_CIRCUIT_MACHINE_END;

  return found;
}

/**
 * Free all padding machines and mutable info associated with circuit
 */
void
circpad_circuit_free_all_machineinfos(circuit_t *circ)
{
  FOR_EACH_CIRCUIT_MACHINE_BEGIN(i) {
    circpad_circuit_machineinfo_free_idx(circ, i);
  } FOR_EACH_CIRCUIT_MACHINE_END;
}

/**
 * Allocate a new mutable machineinfo structure.
 */
STATIC circpad_machine_runtime_t *
circpad_circuit_machineinfo_new(circuit_t *on_circ, int machine_index)
{
  circpad_machine_runtime_t *mi =
    tor_malloc_zero(sizeof(circpad_machine_runtime_t));
  mi->machine_index = machine_index;
  mi->on_circ = on_circ;
  mi->last_cell_time_sec = approx_time();
  mi->machine_ctr = on_circ->padding_machine_ctr;

  return mi;
}

/**
 * Return the circpad_state_t for the current state based on the
 * mutable info.
 *
 * This function returns NULL when the machine is in the end state or in an
 * invalid state.
 */
STATIC const circpad_state_t *
circpad_machine_current_state(const circpad_machine_runtime_t *mi)
{
  const circpad_machine_spec_t *machine = CIRCPAD_GET_MACHINE(mi);

  if (mi->current_state == CIRCPAD_STATE_END) {
    return NULL;
  } else if (BUG(mi->current_state >= machine->num_states)) {
    log_fn(LOG_WARN,LD_CIRC,
           "Invalid circuit padding state %d",
           mi->current_state);

    return NULL;
  }

  return &machine->states[mi->current_state];
}

/**
 * Get the lower bound of a histogram bin.
 *
 * You can obtain the upper bound using histogram_get_bin_upper_bound().
 *
 * This function can also be called with 'bin' set to a value equal or greater
 * than histogram_len in which case the infinity bin is chosen and
 * CIRCPAD_DELAY_INFINITE is returned.
 */
STATIC circpad_delay_t
circpad_histogram_bin_to_usec(const circpad_machine_runtime_t *mi,
                              circpad_hist_index_t bin)
{
  const circpad_state_t *state = circpad_machine_current_state(mi);
  circpad_delay_t rtt_add_usec = 0;

  /* Our state should have been checked to be non-null by the caller
   * (circpad_machine_remove_token()) */
  if (BUG(state == NULL)) {
    return CIRCPAD_DELAY_INFINITE;
  }

  /* The infinity bin has an upper bound of infinity, so make sure we return
   * that if they ask for it. */
  if (bin > CIRCPAD_INFINITY_BIN(state)) {
    return CIRCPAD_DELAY_INFINITE;
  }

  /* If we are using an RTT estimate, consider it as well. */
  if (state->use_rtt_estimate) {
    rtt_add_usec = mi->rtt_estimate_usec;
  }

  return state->histogram_edges[bin] + rtt_add_usec;
}

/**
 * Like circpad_histogram_bin_to_usec() but return the upper bound of bin.
 * (The upper bound is included in the bin.)
 */
STATIC circpad_delay_t
histogram_get_bin_upper_bound(const circpad_machine_runtime_t *mi,
                              circpad_hist_index_t bin)
{
  return circpad_histogram_bin_to_usec(mi, bin+1) - 1;
}

/** Return the midpoint of the histogram bin <b>bin_index</b>. */
static circpad_delay_t
circpad_get_histogram_bin_midpoint(const circpad_machine_runtime_t *mi,
                           int bin_index)
{
  circpad_delay_t left_bound = circpad_histogram_bin_to_usec(mi, bin_index);
  circpad_delay_t right_bound = histogram_get_bin_upper_bound(mi, bin_index);

  return left_bound + (right_bound - left_bound)/2;
}

/**
 * Return the bin that contains the usec argument.
 * "Contains" is defined as us in [lower, upper).
 *
 * This function will never return the infinity bin (histogram_len-1), in order
 * to simplify the rest of the code, so if a usec is provided that falls above
 * the highest non-infinity bin, that bin index will be returned.
 */
STATIC circpad_hist_index_t
circpad_histogram_usec_to_bin(const circpad_machine_runtime_t *mi,
                              circpad_delay_t usec)
{
  const circpad_state_t *state = circpad_machine_current_state(mi);
  circpad_delay_t rtt_add_usec = 0;
  circpad_hist_index_t bin;

  /* Our state should have been checked to be non-null by the caller
   * (circpad_machine_remove_token()) */
  if (BUG(state == NULL)) {
    return 0;
  }

  /* If we are using an RTT estimate, consider it as well. */
  if (state->use_rtt_estimate) {
    rtt_add_usec = mi->rtt_estimate_usec;
  }

  /* Walk through the bins and check the upper bound of each bin, if 'usec' is
   * less-or-equal to that, return that bin. If rtt_estimate is enabled then
   * add that to the upper bound of each bin.
   *
   * We don't want to return the infinity bin here, so don't go there. */
  for (bin = 0 ; bin < CIRCPAD_INFINITY_BIN(state) ; bin++) {
    if (usec <= histogram_get_bin_upper_bound(mi, bin) + rtt_add_usec) {
      return bin;
    }
  }

  /* We don't want to return the infinity bin here, so if we still didn't find
   * the right bin, return the highest non-infinity bin */
  return CIRCPAD_INFINITY_BIN(state)-1;
}

/**
 * Return true if the machine supports token removal.
 *
 * Token removal is equivalent to having a mutable histogram in the
 * circpad_machine_runtime_t mutable info. So while we're at it,
 * let's assert that everything is consistent between the mutable
 * runtime and the readonly machine spec.
 */
static inline int
circpad_is_token_removal_supported(circpad_machine_runtime_t *mi)
{
  /* No runtime histogram == no token removal */
  if (mi->histogram == NULL) {
    /* Machines that don't want token removal are trying to avoid
     * potentially expensive mallocs, extra memory accesses, and/or
     * potentially expensive monotime calls. Let's minimize checks
     * and keep this path fast. */
    tor_assert_nonfatal(mi->histogram_len == 0);
    return 0;
  } else {
    /* Machines that do want token removal are less sensitive to performance.
     * Let's spend some time to check that our state is consistent and sane */
    const circpad_state_t *state = circpad_machine_current_state(mi);
    if (BUG(!state)) {
      return 1;
    }
    tor_assert_nonfatal(state->token_removal != CIRCPAD_TOKEN_REMOVAL_NONE);
    tor_assert_nonfatal(state->histogram_len == mi->histogram_len);
    tor_assert_nonfatal(mi->histogram_len != 0);
    return 1;
  }

  tor_assert_nonfatal_unreached();
  return 0;
}

/**
 * This function frees any token bins allocated from a previous state
 *
 * Called after a state transition, or if the bins are empty.
 */
STATIC void
circpad_machine_setup_tokens(circpad_machine_runtime_t *mi)
{
  const circpad_state_t *state = circpad_machine_current_state(mi);

  /* If this state doesn't exist, or doesn't have token removal,
   * free any previous state's runtime histogram, and bail.
   *
   * If we don't have a token removal strategy, we also don't need a runtime
   * histogram and we rely on the immutable one in machine_spec_t. */
  if (!state || state->token_removal == CIRCPAD_TOKEN_REMOVAL_NONE) {
    if (mi->histogram) {
      tor_free(mi->histogram);
      mi->histogram = NULL;
      mi->histogram_len = 0;
    }
    return;
  }

  /* Try to avoid re-mallocing if we don't really need to */
  if (!mi->histogram || (mi->histogram
          && mi->histogram_len != state->histogram_len)) {
    tor_free(mi->histogram); // null ok
    mi->histogram = tor_malloc_zero(sizeof(circpad_hist_token_t)
                                    *state->histogram_len);
  }
  mi->histogram_len = state->histogram_len;

  memcpy(mi->histogram, state->histogram,
         sizeof(circpad_hist_token_t)*state->histogram_len);
}

/**
 * Choose a length for this state (in cells), if specified.
 */
static void
circpad_choose_state_length(circpad_machine_runtime_t *mi)
{
  const circpad_state_t *state = circpad_machine_current_state(mi);
  double length;

  if (!state || state->length_dist.type == CIRCPAD_DIST_NONE) {
    mi->state_length = CIRCPAD_STATE_LENGTH_INFINITE;
    return;
  }

  length = circpad_distribution_sample(state->length_dist);
  length = MAX(0, length);
  length += state->start_length;

  if (state->max_length) {
    length = MIN(length, state->max_length);
  }

  mi->state_length = clamp_double_to_int64(length);

  log_info(LD_CIRC, "State length sampled to %"PRIu64" for circuit %u",
      mi->state_length, CIRCUIT_IS_ORIGIN(mi->on_circ) ?
             TO_ORIGIN_CIRCUIT(mi->on_circ)->global_identifier : 0);
}

/**
 * Sample a value from our iat_dist, and clamp it safely
 * to circpad_delay_t.
 *
 * Before returning, add <b>delay_shift</b> (can be zero) to the sampled value.
 */
static circpad_delay_t
circpad_distribution_sample_iat_delay(const circpad_state_t *state,
                                      circpad_delay_t delay_shift)
{
  double val = circpad_distribution_sample(state->iat_dist);
  /* These comparisons are safe, because the output is in the range
   * [0, 2**32), and double has a precision of 53 bits. */
  /* We want a positive sample value */
  val = MAX(0, val);
  /* Respect the maximum sample setting */
  val = MIN(val, state->dist_max_sample_usec);

  /* Now apply the shift:
   * This addition is exact: val is at most 2**32-1, delay_shift is at most
   * 2**32-1, and doubles have a precision of 53 bits. */
  val += delay_shift;

  /* Clamp the distribution at infinite delay val */
  return (circpad_delay_t)MIN(tor_llround(val), CIRCPAD_DELAY_INFINITE);
}

/**
 * Sample an expected time-until-next-packet delay from the histogram or
 * probability distribution.
 *
 * A bin of the histogram is chosen with probability proportional to the number
 * of tokens in each bin, and then a time value is chosen uniformly from that
 * bin's [start,end) time range.
 */
STATIC circpad_delay_t
circpad_machine_sample_delay(circpad_machine_runtime_t *mi)
{
  const circpad_state_t *state = circpad_machine_current_state(mi);
  const circpad_hist_token_t *histogram = NULL;
  circpad_hist_index_t curr_bin = 0;
  circpad_delay_t bin_start, bin_end;
  /* These three must all be larger than circpad_hist_token_t, because
   * we sum several circpad_hist_token_t values across the histogram */
  uint64_t curr_weight = 0;
  uint64_t histogram_total_tokens = 0;
  uint64_t bin_choice;

  tor_assert(state);

  if (state->iat_dist.type != CIRCPAD_DIST_NONE) {
    /* Sample from a fixed IAT distribution and return */
    circpad_delay_t iat_delay_shift = state->use_rtt_estimate ?
      mi->rtt_estimate_usec + state->dist_added_shift_usec :
      state->dist_added_shift_usec;
    return circpad_distribution_sample_iat_delay(state, iat_delay_shift);
  } else if (circpad_is_token_removal_supported(mi)) {
    histogram = mi->histogram;
    for (circpad_hist_index_t b = 0; b < state->histogram_len; b++)
      histogram_total_tokens += histogram[b];
  } else {
    /* We have a histogram, but it's immutable */
    histogram = state->histogram;
    histogram_total_tokens = state->histogram_total_tokens;
  }

  /* If we are out of tokens, don't schedule padding. */
  if (!histogram_total_tokens) {
    return CIRCPAD_DELAY_INFINITE;
  }

  bin_choice = crypto_fast_rng_get_uint64(get_thread_fast_rng(),
                                          histogram_total_tokens);

  /* Skip all the initial zero bins */
  while (!histogram[curr_bin]) {
    curr_bin++;
  }
  curr_weight = histogram[curr_bin];

  // TODO: This is not constant-time. Pretty sure we don't
  // really need it to be, though.
  while (curr_weight < bin_choice) {
    curr_bin++;
    /* It should be impossible to run past the end of the histogram */
    if (BUG(curr_bin >= state->histogram_len)) {
      return CIRCPAD_DELAY_INFINITE;
    }
    curr_weight += histogram[curr_bin];
  }

  /* Do some basic checking of the current bin we are in */
  if (BUG(curr_bin >= state->histogram_len) ||
      BUG(histogram[curr_bin] == 0)) {
    return CIRCPAD_DELAY_INFINITE;
  }

  // Store this index to remove the token upon callback.
  if (state->token_removal != CIRCPAD_TOKEN_REMOVAL_NONE) {
    mi->chosen_bin = curr_bin;
  }

  if (curr_bin >= CIRCPAD_INFINITY_BIN(state)) {
    if (state->token_removal != CIRCPAD_TOKEN_REMOVAL_NONE &&
        mi->histogram[curr_bin] > 0) {
      mi->histogram[curr_bin]--;
    }

    // Infinity: Don't send a padding packet. Wait for a real packet
    // and then see if our bins are empty or what else we should do.
    return CIRCPAD_DELAY_INFINITE;
  }

  tor_assert(curr_bin < CIRCPAD_INFINITY_BIN(state));

  bin_start = circpad_histogram_bin_to_usec(mi, curr_bin);
  /* We don't need to reduct 1 from the upper bound because the random range
   * function below samples from [bin_start, bin_end) */
  bin_end = circpad_histogram_bin_to_usec(mi, curr_bin+1);

  /* Bin edges are monotonically increasing so this is a bug. Handle it. */
  if (BUG(bin_start >= bin_end)) {
    return bin_start;
  }

  return (circpad_delay_t)crypto_fast_rng_uint64_range(get_thread_fast_rng(),
                                                       bin_start, bin_end);
}

/**
 * Sample a value from the specified probability distribution.
 *
 * Uses functions from src/lib/math/prob_distr.c .
 */
static double
circpad_distribution_sample(circpad_distribution_t dist)
{
  log_fn(LOG_DEBUG,LD_CIRC, "Sampling delay with distribution %d",
         dist.type);

  switch (dist.type) {
    case CIRCPAD_DIST_NONE:
      {
        /* We should not get in here like this */
        tor_assert_nonfatal_unreached();
        return 0;
      }
    case CIRCPAD_DIST_UNIFORM:
      {
        // param2 is upper bound, param1 is lower
        const struct uniform_t my_uniform = {
          .base = UNIFORM(my_uniform),
          .a = dist.param1,
          .b = dist.param2,
        };
        return dist_sample(&my_uniform.base);
      }
    case CIRCPAD_DIST_LOGISTIC:
      {
      /* param1 is Mu, param2 is sigma. */
        const struct logistic_t my_logistic = {
          .base = LOGISTIC(my_logistic),
          .mu = dist.param1,
          .sigma = dist.param2,
        };
        return dist_sample(&my_logistic.base);
      }
    case CIRCPAD_DIST_LOG_LOGISTIC:
      {
        /* param1 is Alpha, param2 is 1.0/Beta */
        const struct log_logistic_t my_log_logistic = {
          .base = LOG_LOGISTIC(my_log_logistic),
          .alpha = dist.param1,
          .beta = dist.param2,
        };
        return dist_sample(&my_log_logistic.base);
      }
    case CIRCPAD_DIST_GEOMETRIC:
      {
        /* param1 is 'p' (success probability) */
        const struct geometric_t my_geometric = {
          .base = GEOMETRIC(my_geometric),
          .p = dist.param1,
        };
        return dist_sample(&my_geometric.base);
      }
    case CIRCPAD_DIST_WEIBULL:
      {
        /* param1 is k, param2 is Lambda */
        const struct weibull_t my_weibull = {
          .base = WEIBULL(my_weibull),
          .k = dist.param1,
          .lambda = dist.param2,
        };
        return dist_sample(&my_weibull.base);
      }
    case CIRCPAD_DIST_PARETO:
      {
        /* param1 is sigma, param2 is xi, no more params for mu so we use 0 */
        const struct genpareto_t my_genpareto = {
          .base = GENPARETO(my_genpareto),
          .mu = 0,
          .sigma = dist.param1,
          .xi = dist.param2,
        };
        return dist_sample(&my_genpareto.base);
      }
  }

  tor_assert_nonfatal_unreached();
  return 0;
}

/**
 * Find the index of the first bin whose upper bound is
 * greater than the target, and that has tokens remaining.
 *
 * Used for histograms with token removal.
 */
static circpad_hist_index_t
circpad_machine_first_higher_index(const circpad_machine_runtime_t *mi,
                                   circpad_delay_t target_bin_usec)
{
  circpad_hist_index_t bin = circpad_histogram_usec_to_bin(mi,
                                                           target_bin_usec);

  /* Don't remove from the infinity bin */
  for (; bin < CIRCPAD_INFINITY_BIN(mi); bin++) {
    if (mi->histogram[bin] &&
        histogram_get_bin_upper_bound(mi, bin) >= target_bin_usec) {
      return bin;
    }
  }

  return mi->histogram_len;
}

/**
 * Find the index of the first bin whose lower bound is lower or equal to
 * <b>target_bin_usec</b>, and that still has tokens remaining.
 *
 * Used for histograms with token removal.
 */
static circpad_hist_index_t
circpad_machine_first_lower_index(const circpad_machine_runtime_t *mi,
                                  circpad_delay_t target_bin_usec)
{
  circpad_hist_index_t bin = circpad_histogram_usec_to_bin(mi,
                                                           target_bin_usec);

  for (; bin >= 0; bin--) {
    if (mi->histogram[bin] &&
        circpad_histogram_bin_to_usec(mi, bin) <= target_bin_usec) {
      return bin;
    }
  }

  return -1;
}

/**
 * Remove a token from the first non-empty bin whose upper bound is
 * greater than the target.
 *
 * Used for histograms with token removal.
 */
STATIC void
circpad_machine_remove_higher_token(circpad_machine_runtime_t *mi,
                                    circpad_delay_t target_bin_usec)
{
  /* We need to remove the token from the first bin
   * whose upper bound is greater than the target, and that
   * has tokens remaining. */
  circpad_hist_index_t bin = circpad_machine_first_higher_index(mi,
                                                     target_bin_usec);

  if (bin >= 0 && bin < CIRCPAD_INFINITY_BIN(mi)) {
    if (!BUG(mi->histogram[bin] == 0)) {
      mi->histogram[bin]--;
    }
  }
}

/**
 * Remove a token from the first non-empty bin whose upper bound is
 * lower than the target.
 *
 * Used for histograms with token removal.
 */
STATIC void
circpad_machine_remove_lower_token(circpad_machine_runtime_t *mi,
                                   circpad_delay_t target_bin_usec)
{
  circpad_hist_index_t bin = circpad_machine_first_lower_index(mi,
          target_bin_usec);

  if (bin >= 0 && bin < CIRCPAD_INFINITY_BIN(mi)) {
    if (!BUG(mi->histogram[bin] == 0)) {
      mi->histogram[bin]--;
    }
  }
}

/* Helper macro: Ensure that the bin has tokens available, and BUG out of the
 * function if it's not the case. */
#define ENSURE_BIN_CAPACITY(bin_index) \
  if (BUG(mi->histogram[bin_index] == 0)) {                   \
    return;                                                   \
  }

/**
 * Remove a token from the closest non-empty bin to the target.
 *
 * If use_usec is true, measure "closest" in terms of the next closest bin
 * midpoint.
 *
 * If it is false, use bin index distance only.
 *
 * Used for histograms with token removal.
 */
STATIC void
circpad_machine_remove_closest_token(circpad_machine_runtime_t *mi,
                                     circpad_delay_t target_bin_usec,
                                     bool use_usec)
{
  circpad_hist_index_t lower, higher, current;
  circpad_hist_index_t bin_to_remove = -1;

  lower = circpad_machine_first_lower_index(mi, target_bin_usec);
  higher = circpad_machine_first_higher_index(mi, target_bin_usec);
  current = circpad_histogram_usec_to_bin(mi, target_bin_usec);

  /* Sanity check the results */
  if (BUG(lower > current) || BUG(higher < current)) {
    return;
  }

  /* Take care of edge cases first */
  if (higher == mi->histogram_len && lower == -1) {
    /* All bins are empty */
    return;
  } else if (higher == mi->histogram_len) {
    /* All higher bins are empty */
    ENSURE_BIN_CAPACITY(lower);
    mi->histogram[lower]--;
    return;
  } else if (lower == -1) {
    /* All lower bins are empty */
    ENSURE_BIN_CAPACITY(higher);
    mi->histogram[higher]--;
    return;
  }

  /* Now handle the intermediate cases */
  if (use_usec) {
    /* Find the closest bin midpoint to the target */
    circpad_delay_t lower_usec = circpad_get_histogram_bin_midpoint(mi, lower);
    circpad_delay_t higher_usec =
      circpad_get_histogram_bin_midpoint(mi, higher);

    if (target_bin_usec < lower_usec) {
      // Lower bin is closer
      ENSURE_BIN_CAPACITY(lower);
      bin_to_remove = lower;
    } else if (target_bin_usec > higher_usec) {
      // Higher bin is closer
      ENSURE_BIN_CAPACITY(higher);
      bin_to_remove = higher;
    } else if (target_bin_usec-lower_usec > higher_usec-target_bin_usec) {
      // Higher bin is closer
      ENSURE_BIN_CAPACITY(higher);
      bin_to_remove = higher;
    } else {
      // Lower bin is closer
      ENSURE_BIN_CAPACITY(lower);
      bin_to_remove = lower;
    }
    mi->histogram[bin_to_remove]--;
    log_debug(LD_CIRC, "Removing token from bin %d", bin_to_remove);
    return;
  } else {
    if (current - lower > higher - current) {
      // Higher bin is closer
      ENSURE_BIN_CAPACITY(higher);
      mi->histogram[higher]--;
      return;
    } else {
      // Lower bin is closer
      ENSURE_BIN_CAPACITY(lower);
      mi->histogram[lower]--;
      return;
    }
  }
}

#undef ENSURE_BIN_CAPACITY

/**
 * Remove a token from the exact bin corresponding to the target.
 *
 * If it is empty, do nothing.
 *
 * Used for histograms with token removal.
 */
static void
circpad_machine_remove_exact(circpad_machine_runtime_t *mi,
                             circpad_delay_t target_bin_usec)
{
  circpad_hist_index_t bin = circpad_histogram_usec_to_bin(mi,
          target_bin_usec);

  if (mi->histogram[bin] > 0)
    mi->histogram[bin]--;
}

/**
 * Check our state's cell limit count and tokens.
 *
 * Returns 1 if either limits are hit and we decide to change states,
 * otherwise returns 0.
 */
static circpad_decision_t
check_machine_token_supply(circpad_machine_runtime_t *mi)
{
  uint32_t histogram_total_tokens = 0;

  /* Check if bins empty. This requires summing up the current mutable
   * machineinfo histogram token total and checking if it is zero.
   * Machineinfo does not keep a running token count. We're assuming the
   * extra space is not worth this short loop iteration.
   *
   * We also do not count infinity bin in histogram totals.
   */
  if (circpad_is_token_removal_supported(mi)) {
    for (circpad_hist_index_t b = 0; b < CIRCPAD_INFINITY_BIN(mi); b++)
      histogram_total_tokens += mi->histogram[b];

    /* If we change state, we're done */
    if (histogram_total_tokens == 0) {
      if (circpad_internal_event_bins_empty(mi) == CIRCPAD_STATE_CHANGED)
        return CIRCPAD_STATE_CHANGED;
    }
  }

  if (mi->state_length == 0) {
    return circpad_internal_event_state_length_up(mi);
  }

  return CIRCPAD_STATE_UNCHANGED;
}

/**
 * Count that a padding packet was sent.
 *
 * This updates our state length count, our machine rate limit counts,
 * and if token removal is used, decrements the histogram.
 */
static inline void
circpad_machine_count_padding_sent(circpad_machine_runtime_t *mi)
{
  /* If we have a valid state length bound, consider it */
  if (mi->state_length != CIRCPAD_STATE_LENGTH_INFINITE &&
      !BUG(mi->state_length <= 0)) {
    mi->state_length--;
  }

  /*
   * Update non-padding counts for rate limiting: We scale at UINT16_MAX
   * because we only use this for a percentile limit of 2 sig figs, and
   * space is scare in the machineinfo struct.
   */
  mi->padding_sent++;
  if (mi->padding_sent == UINT16_MAX) {
    mi->padding_sent /= 2;
    mi->nonpadding_sent /= 2;
  }

  circpad_global_padding_sent++;

  /* If we have a mutable histogram, reduce the token count from
   * the chosen padding bin (this assumes we always send padding
   * when we intended to). */
  if (circpad_is_token_removal_supported(mi)) {
    /* Check array bounds and token count before removing */
    if (!BUG(mi->chosen_bin >= mi->histogram_len) &&
        !BUG(mi->histogram[mi->chosen_bin] == 0)) {
      mi->histogram[mi->chosen_bin]--;
    }
  }
}

/**
 * Count a nonpadding packet as being sent.
 *
 * This function updates our overhead accounting variables, as well
 * as decrements the state limit packet counter, if the latter was
 * flagged as applying to non-padding as well.
 */
static inline void
circpad_machine_count_nonpadding_sent(circpad_machine_runtime_t *mi)
{
  /* Update non-padding counts for rate limiting: We scale at UINT16_MAX
   * because we only use this for a percentile limit of 2 sig figs, and
   * space is scare in the machineinfo struct. */
  mi->nonpadding_sent++;
  if (mi->nonpadding_sent == UINT16_MAX) {
    mi->padding_sent /= 2;
    mi->nonpadding_sent /= 2;
  }

  /* Update any state packet length limits that apply */
  circpad_machine_update_state_length_for_nonpadding(mi);

  /* Remove a token from the histogram, if applicable */
  circpad_machine_remove_token(mi);
}

/**
 * Decrement the state length counter for a non-padding packet.
 *
 * Only updates the state length if we're using that feature, we
 * have a state, and the machine wants to count non-padding packets
 * towards the state length.
 */
static inline void
circpad_machine_update_state_length_for_nonpadding(
        circpad_machine_runtime_t *mi)
{
  const circpad_state_t *state = NULL;

  if (mi->state_length == CIRCPAD_STATE_LENGTH_INFINITE)
    return;

  state = circpad_machine_current_state(mi);

  /* If we are not in a padding state (like start or end), we're done */
  if (!state)
    return;

  /* If we're enforcing a state length on non-padding packets,
   * decrement it */
  if (state->length_includes_nonpadding &&
      mi->state_length > 0) {
    mi->state_length--;
  }
}

/**
 * When a non-padding packet arrives, remove a token from the bin
 * corresponding to the delta since last sent packet. If that bin
 * is empty, choose a token based on the specified removal strategy
 * in the state machine.
 */
STATIC void
circpad_machine_remove_token(circpad_machine_runtime_t *mi)
{
  const circpad_state_t *state = NULL;
  circpad_time_t current_time;
  circpad_delay_t target_bin_usec;

  /* Dont remove any tokens if there was no padding scheduled */
  if (!mi->padding_scheduled_at_usec) {
    return;
  }

  state = circpad_machine_current_state(mi);

  /* If we are not in a padding state (like start or end), we're done */
  if (!state)
    return;
  /* Don't remove any tokens if we're not doing token removal */
  if (state->token_removal == CIRCPAD_TOKEN_REMOVAL_NONE)
    return;

  current_time = monotime_absolute_usec();

  /* If we have scheduled padding some time in the future, we want to see what
     bin we are in at the current time */
  target_bin_usec = (circpad_delay_t)
                  MIN((current_time - mi->padding_scheduled_at_usec),
                      CIRCPAD_DELAY_INFINITE-1);

  /* We are treating this non-padding cell as a padding cell, so we cancel
     padding timer, if present. */
  mi->padding_scheduled_at_usec = 0;
  if (mi->is_padding_timer_scheduled) {
    mi->is_padding_timer_scheduled = 0;
    timer_disable(mi->padding_timer);
  }

  /* Perform the specified token removal strategy */
  switch (state->token_removal) {
    case CIRCPAD_TOKEN_REMOVAL_CLOSEST_USEC:
      circpad_machine_remove_closest_token(mi, target_bin_usec, 1);
      break;
    case CIRCPAD_TOKEN_REMOVAL_CLOSEST:
      circpad_machine_remove_closest_token(mi, target_bin_usec, 0);
      break;
    case CIRCPAD_TOKEN_REMOVAL_LOWER:
      circpad_machine_remove_lower_token(mi, target_bin_usec);
      break;
    case CIRCPAD_TOKEN_REMOVAL_HIGHER:
      circpad_machine_remove_higher_token(mi, target_bin_usec);
      break;
    case CIRCPAD_TOKEN_REMOVAL_EXACT:
      circpad_machine_remove_exact(mi, target_bin_usec);
      break;
    case CIRCPAD_TOKEN_REMOVAL_NONE:
    default:
      tor_assert_nonfatal_unreached();
      log_warn(LD_BUG, "Circpad: Unknown token removal strategy %d",
               state->token_removal);
      break;
  }
}

/**
 * Send a relay command with a relay cell payload on a circuit to
 * the particular hopnum.
 *
 * Hopnum starts at 1 (1=guard, 2=middle, 3=exit, etc).
 *
 * Payload may be null.
 *
 * Returns negative on error, 0 on success.
 */
MOCK_IMPL(STATIC signed_error_t,
circpad_send_command_to_hop,(origin_circuit_t *circ, uint8_t hopnum,
                             uint8_t relay_command, const uint8_t *payload,
                             ssize_t payload_len))
{
  crypt_path_t *target_hop = circuit_get_cpath_hop(circ, hopnum);
  signed_error_t ret;

  /* Check that the cpath has the target hop */
  if (!target_hop) {
    log_fn(LOG_WARN, LD_BUG, "Padding circuit %u has %d hops, not %d",
           circ->global_identifier, circuit_get_cpath_len(circ), hopnum);
    return -1;
  }

  /* Check that the target hop is opened */
  if (target_hop->state != CPATH_STATE_OPEN) {
    log_fn(LOG_WARN,LD_CIRC,
           "Padding circuit %u has %d hops, not %d",
           circ->global_identifier,
           circuit_get_cpath_opened_len(circ), hopnum);
    return -1;
  }

  /* Send the drop command to the second hop */
  ret = relay_send_command_from_edge(0, TO_CIRCUIT(circ), relay_command,
                                     (const char*)payload, payload_len,
                                     target_hop);
  return ret;
}

/**
 * Callback helper to send a padding cell.
 *
 * This helper is called after our histogram-sampled delay period passes
 * without another packet being sent first. If a packet is sent before this
 * callback happens, it is canceled. So when we're called here, send padding
 * right away.
 *
 * If sending this padding cell forced us to transition states return
 * CIRCPAD_STATE_CHANGED. Otherwise return CIRCPAD_STATE_UNCHANGED.
 */
circpad_decision_t
circpad_send_padding_cell_for_callback(circpad_machine_runtime_t *mi)
{
  circuit_t *circ = mi->on_circ;
  int machine_idx = mi->machine_index;
  mi->padding_scheduled_at_usec = 0;
  mi->is_padding_timer_scheduled = 0;
  circpad_statenum_t state = mi->current_state;

  /* Make sure circuit didn't close on us */
  if (mi->on_circ->marked_for_close) {
    log_fn(LOG_INFO,LD_CIRC,
           "Padding callback on circuit marked for close (%u). Ignoring.",
         CIRCUIT_IS_ORIGIN(mi->on_circ) ?
         TO_ORIGIN_CIRCUIT(mi->on_circ)->global_identifier : 0);
    return CIRCPAD_STATE_CHANGED;
  }

  circpad_machine_count_padding_sent(mi);

  if (CIRCUIT_IS_ORIGIN(mi->on_circ)) {
    circpad_send_command_to_hop(TO_ORIGIN_CIRCUIT(mi->on_circ),
                                CIRCPAD_GET_MACHINE(mi)->target_hopnum,
                                RELAY_COMMAND_DROP, NULL, 0);
    log_info(LD_CIRC, "Callback: Sending padding to origin circuit %u"
             " (%d) [length: %"PRIu64"]",
             TO_ORIGIN_CIRCUIT(mi->on_circ)->global_identifier,
             mi->on_circ->purpose, mi->state_length);
  } else {
    // If we're a non-origin circ, we can just send from here as if we're the
    // edge.
    if (TO_OR_CIRCUIT(circ)->p_chan_cells.n <= circpad_max_circ_queued_cells) {
      log_info(LD_CIRC, "Callback: Sending padding to circuit (%d)"
               " [length: %"PRIu64"]", mi->on_circ->purpose, mi->state_length);
      relay_send_command_from_edge(0, mi->on_circ, RELAY_COMMAND_DROP, NULL,
                                   0, NULL);
      rep_hist_padding_count_write(PADDING_TYPE_DROP);
    } else {
      static ratelim_t cell_lim = RATELIM_INIT(600);
      log_fn_ratelim(&cell_lim,LOG_NOTICE,LD_CIRC,
                     "Too many cells (%d) in circ queue to send padding.",
                      TO_OR_CIRCUIT(circ)->p_chan_cells.n);
    }
  }

  /* This is a padding cell sent from the client or from the middle node,
   * (because it's invoked from circuitpadding.c) */
  circpad_cell_event_padding_sent(circ);

  /* The circpad_cell_event_padding_sent() could cause us to transition.
   * Check that we still have a padding machineinfo, and then check our token
   * supply. */
  if (circ->padding_info[machine_idx] != NULL) {
    if (state != circ->padding_info[machine_idx]->current_state)
      return CIRCPAD_STATE_CHANGED;
    else
      return check_machine_token_supply(circ->padding_info[machine_idx]);
  } else {
    return CIRCPAD_STATE_CHANGED;
  }
}

/**
 * Tor-timer compatible callback that tells us to send a padding cell.
 *
 * Timers are associated with circpad_machine_runtime_t's. When the machineinfo
 * is freed on a circuit, the timers are cancelled. Since the lifetime
 * of machineinfo is always longer than the timers, handles are not
 * needed.
 */
static void
circpad_send_padding_callback(tor_timer_t *timer, void *args,
                              const struct monotime_t *time)
{
  circpad_machine_runtime_t *mi = ((circpad_machine_runtime_t*)args);
  (void)timer; (void)time;

  if (mi && mi->on_circ) {
    assert_circuit_ok(mi->on_circ);
    circpad_send_padding_cell_for_callback(mi);
  } else {
    // This shouldn't happen (represents a timer leak)
    log_fn(LOG_WARN,LD_CIRC,
            "Circuit closed while waiting for padding timer.");
    tor_fragile_assert();
  }

  // TODO-MP-AP: Unify this counter with channelpadding for rephist stats
  //total_timers_pending--;
}

/**
 * Cache our consensus parameters upon consensus update.
 */
void
circpad_new_consensus_params(const networkstatus_t *ns)
{
  circpad_padding_disabled =
      networkstatus_get_param(ns, "circpad_padding_disabled",
         0, 0, 1);

  circpad_padding_reduced =
      networkstatus_get_param(ns, "circpad_padding_reduced",
         0, 0, 1);

  circpad_global_allowed_cells =
      networkstatus_get_param(ns, "circpad_global_allowed_cells",
         0, 0, UINT16_MAX-1);

  circpad_global_max_padding_percent =
      networkstatus_get_param(ns, "circpad_global_max_padding_pct",
         0, 0, 100);

  circpad_max_circ_queued_cells =
      networkstatus_get_param(ns, "circpad_max_circ_queued_cells",
         CIRCWINDOW_START_MAX, 0, 50*CIRCWINDOW_START_MAX);
}

/**
 * Return true if padding is allowed by torrc and consensus.
 */
static bool
circpad_is_padding_allowed(void)
{
  /* If padding has been disabled in the consensus, don't send any more
   * padding. Technically the machine should be shut down when the next
   * machine condition check happens, but machine checks only happen on
   * certain circuit events, and if padding is disabled due to some
   * network overload or DoS condition, we really want to stop ASAP. */
  if (circpad_padding_disabled || !get_options()->CircuitPadding) {
    return 0;
  }

  return 1;
}

/**
 * Check this machine against its padding limits, as well as global
 * consensus limits.
 *
 * We have two limits: a percent and a cell count. The cell count
 * limit must be reached before the percent is enforced (this is to
 * optionally allow very light padding of things like circuit setup
 * while there is no other traffic on the circuit).
 *
 * TODO: Don't apply limits to machines form torrc.
 *
 * Returns 1 if limits are set and we've hit them. Otherwise returns 0.
 */
STATIC bool
circpad_machine_reached_padding_limit(circpad_machine_runtime_t *mi)
{
  const circpad_machine_spec_t *machine = CIRCPAD_GET_MACHINE(mi);

  /* If machine_padding_pct is non-zero, and we've sent more
   * than the allowed count of padding cells, then check our
   * percent limits for this machine. */
  if (machine->max_padding_percent &&
      mi->padding_sent >= machine->allowed_padding_count) {
    uint32_t total_cells = mi->padding_sent + mi->nonpadding_sent;

    /* Check the percent */
    if ((100*(uint32_t)mi->padding_sent) / total_cells >
        machine->max_padding_percent) {
      return 1; // limit is reached. Stop.
    }
  }

  /* If circpad_max_global_padding_pct is non-zero, and we've
   * sent more than the global padding cell limit, then check our
   * global tor process percentage limit on padding. */
  if (circpad_global_max_padding_percent &&
      circpad_global_padding_sent >= circpad_global_allowed_cells) {
    uint64_t total_cells = circpad_global_padding_sent +
              circpad_global_nonpadding_sent;

    /* Check the percent */
    if ((100*circpad_global_padding_sent) / total_cells >
        circpad_global_max_padding_percent) {
      return 1; // global limit reached. Stop.
    }
  }

  return 0; // All good!
}

/**
 * Schedule the next padding time according to the machineinfo on a
 * circuit.
 *
 * The histograms represent inter-packet-delay. Whenever you get an packet
 * event you should be scheduling your next timer (after cancelling any old
 * ones and updating tokens accordingly).
 *
 * Returns 1 if we decide to transition states (due to infinity bin),
 * 0 otherwise.
 */
MOCK_IMPL(circpad_decision_t,
circpad_machine_schedule_padding,(circpad_machine_runtime_t *mi))
{
  circpad_delay_t in_usec = 0;
  struct timeval timeout;
  tor_assert(mi);

  /* Don't schedule padding if it is disabled */
  if (!circpad_is_padding_allowed()) {
    static ratelim_t padding_lim = RATELIM_INIT(600);
    log_fn_ratelim(&padding_lim,LOG_INFO,LD_CIRC,
         "Padding has been disabled, but machine still on circuit %"PRIu64
         ", %d",
         mi->on_circ->n_chan ? mi->on_circ->n_chan->global_identifier : 0,
         mi->on_circ->n_circ_id);

    return CIRCPAD_STATE_UNCHANGED;
  }

  /* Don't schedule padding if we are currently in dormant mode. */
  if (!is_participating_on_network()) {
    log_info(LD_CIRC, "Not scheduling padding because we are dormant.");
    return CIRCPAD_STATE_UNCHANGED;
  }

  // Don't pad in end (but  also don't cancel any previously
  // scheduled padding either).
  if (mi->current_state == CIRCPAD_STATE_END) {
    log_fn(LOG_INFO, LD_CIRC, "Padding end state on circuit %u",
         CIRCUIT_IS_ORIGIN(mi->on_circ) ?
           TO_ORIGIN_CIRCUIT(mi->on_circ)->global_identifier : 0);
    return CIRCPAD_STATE_UNCHANGED;
  }

  /* Check our padding limits */
  if (circpad_machine_reached_padding_limit(mi)) {
   if (CIRCUIT_IS_ORIGIN(mi->on_circ)) {
      log_fn(LOG_INFO, LD_CIRC,
           "Padding machine has reached padding limit on circuit %u",
             TO_ORIGIN_CIRCUIT(mi->on_circ)->global_identifier);
    } else {
      static ratelim_t padding_lim = RATELIM_INIT(600);
      log_fn_ratelim(&padding_lim,LOG_INFO,LD_CIRC,
           "Padding machine has reached padding limit on circuit %"PRIu64
           ", %d",
           mi->on_circ->n_chan ? mi->on_circ->n_chan->global_identifier : 0,
           mi->on_circ->n_circ_id);
    }
    return CIRCPAD_STATE_UNCHANGED;
  }

  if (mi->is_padding_timer_scheduled) {
    /* Cancel current timer (if any) */
    timer_disable(mi->padding_timer);
    mi->is_padding_timer_scheduled = 0;
  }

  /* in_usec = in microseconds */
  in_usec = circpad_machine_sample_delay(mi);
  /* If we're using token removal, we need to know when the padding
   * was scheduled at, so we can remove the appropriate token if
   * a non-padding cell is sent before the padding timer expires.
   *
   * However, since monotime is unpredictably expensive, let's avoid
   * using it for machines that don't need token removal. */
  if (circpad_is_token_removal_supported(mi)) {
    mi->padding_scheduled_at_usec = monotime_absolute_usec();
  } else {
    mi->padding_scheduled_at_usec = 1;
  }
  log_fn(LOG_INFO,LD_CIRC,"\tPadding in %u usec on circuit %u", in_usec,
       CIRCUIT_IS_ORIGIN(mi->on_circ) ?
           TO_ORIGIN_CIRCUIT(mi->on_circ)->global_identifier : 0);

  // Don't schedule if we have infinite delay.
  if (in_usec == CIRCPAD_DELAY_INFINITE) {
    return circpad_internal_event_infinity(mi);
  }

  if (mi->state_length == 0) {
    /* If we're at length 0, that means we hit 0 after sending
     * a cell earlier, and emitted an event for it, but
     * for whatever reason we did not decide to change states then.
     * So maybe the machine is waiting for bins empty, or for an
     * infinity event later? That would be a strange machine,
     * but there's no reason to make it impossible. */
    return CIRCPAD_STATE_UNCHANGED;
  }

  if (in_usec <= 0) {
    return circpad_send_padding_cell_for_callback(mi);
  }

  timeout.tv_sec = in_usec/TOR_USEC_PER_SEC;
  timeout.tv_usec = (in_usec%TOR_USEC_PER_SEC);

  log_fn(LOG_INFO, LD_CIRC, "\tPadding circuit %u in %u sec, %u usec",
     CIRCUIT_IS_ORIGIN(mi->on_circ) ?
           TO_ORIGIN_CIRCUIT(mi->on_circ)->global_identifier : 0,
          (unsigned)timeout.tv_sec, (unsigned)timeout.tv_usec);

  if (mi->padding_timer) {
    timer_set_cb(mi->padding_timer, circpad_send_padding_callback, mi);
  } else {
    mi->padding_timer =
        timer_new(circpad_send_padding_callback, mi);
  }
  timer_schedule(mi->padding_timer, &timeout);
  mi->is_padding_timer_scheduled = 1;

  // TODO-MP-AP: Unify with channelpadding counter
  //rep_hist_padding_count_timers(++total_timers_pending);

  return CIRCPAD_STATE_UNCHANGED;
}

/**
 * If the machine transitioned to the END state, we need
 * to check to see if it wants us to shut it down immediately.
 * If it does, then we need to send the appropriate negotiation commands
 * depending on which side it is.
 *
 * After this function is called, mi may point to freed memory. Do
 * not access it.
 */
static void
circpad_machine_spec_transitioned_to_end(circpad_machine_runtime_t *mi)
{
  const circpad_machine_spec_t *machine = CIRCPAD_GET_MACHINE(mi);
  circuit_t *on_circ = mi->on_circ;

  log_fn(LOG_INFO,LD_CIRC, "Padding machine in end state on circuit %u (%d)",
         CIRCUIT_IS_ORIGIN(on_circ) ?
         TO_ORIGIN_CIRCUIT(on_circ)->global_identifier : 0,
         on_circ->purpose);

  /*
   * We allow machines to shut down and delete themselves as opposed
   * to just going back to START or waiting forever in END so that
   * we can handle the case where this machine started while it was
   * the only machine that matched conditions, but *since* then more
   * "higher ranking" machines now match the conditions, and would
   * be given a chance to take precedence over this one in
   * circpad_add_matching_machines().
   *
   * Returning to START or waiting forever in END would not give those
   * other machines a chance to be launched, where as shutting down
   * here does.
   */
  if (machine->should_negotiate_end) {
    if (machine->is_origin_side) {
      /* We free the machine info here so that we can be replaced
       * by a different machine. But we must leave the padding_machine
       * in place to wait for the negotiated response */
      uint32_t machine_ctr = mi->machine_ctr;
      circpad_circuit_machineinfo_free_idx(on_circ,
                                           machine->machine_index);
      circpad_negotiate_padding(TO_ORIGIN_CIRCUIT(on_circ),
                                machine->machine_num,
                                machine->target_hopnum,
                                CIRCPAD_COMMAND_STOP,
                                machine_ctr);
    } else {
      uint32_t machine_ctr = mi->machine_ctr;
      circpad_circuit_machineinfo_free_idx(on_circ,
                                           machine->machine_index);
      circpad_padding_negotiated(on_circ,
                                machine->machine_num,
                                CIRCPAD_COMMAND_STOP,
                                CIRCPAD_RESPONSE_OK,
                                machine_ctr);
      on_circ->padding_machine[machine->machine_index] = NULL;
    }
  }
}

/**
 * Generic state transition function for padding state machines.
 *
 * Given an event and our mutable machine info, decide if/how to
 * transition to a different state, and perform actions accordingly.
 *
 * Returns 1 if we transition states, 0 otherwise.
 */
MOCK_IMPL(circpad_decision_t,
circpad_machine_spec_transition,(circpad_machine_runtime_t *mi,
                            circpad_event_t event))
{
  const circpad_state_t *state =
      circpad_machine_current_state(mi);

  /* If state is null we are in the end state. */
  if (!state) {
    /* If we in end state we don't pad no matter what. */
    return CIRCPAD_STATE_UNCHANGED;
  }

  /* Check if this event is ignored or causes a cancel */
  if (state->next_state[event] == CIRCPAD_STATE_IGNORE) {
    return CIRCPAD_STATE_UNCHANGED;
  } else if (state->next_state[event] == CIRCPAD_STATE_CANCEL) {
    /* Check cancel events and cancel any pending padding */
    mi->padding_scheduled_at_usec = 0;
    if (mi->is_padding_timer_scheduled) {
      mi->is_padding_timer_scheduled = 0;
      /* Cancel current timer (if any) */
      timer_disable(mi->padding_timer);
    }
    return CIRCPAD_STATE_UNCHANGED;
  } else {
    circpad_statenum_t s = state->next_state[event];
    /* See if we need to transition to any other states based on this event.
     * Whenever a transition happens, even to our own state, we schedule
     * padding.
     *
     * So if a state only wants to schedule padding for an event, it specifies
     * a transition to itself. All non-specified events are ignored.
     */
    log_fn(LOG_INFO, LD_CIRC,
           "Circuit %u circpad machine %d transitioning from %u to %u",
             CIRCUIT_IS_ORIGIN(mi->on_circ) ?
             TO_ORIGIN_CIRCUIT(mi->on_circ)->global_identifier : 0,
           mi->machine_index, mi->current_state, s);

    /* If this is not the same state, switch and init tokens,
     * otherwise just reschedule padding. */
    if (mi->current_state != s) {
      mi->current_state = s;
      circpad_machine_setup_tokens(mi);
      circpad_choose_state_length(mi);

      /* If we transition to the end state, check to see
       * if this machine wants to be shut down at end */
      if (s == CIRCPAD_STATE_END) {
        circpad_machine_spec_transitioned_to_end(mi);
        /* We transitioned but we don't pad in end. Also, mi
         * may be freed. Returning STATE_CHANGED prevents us
         * from accessing it in any callers of this function. */
        return CIRCPAD_STATE_CHANGED;
      }

      /* We transitioned to a new state, schedule padding */
      circpad_machine_schedule_padding(mi);
      return CIRCPAD_STATE_CHANGED;
    }

    /* We transitioned back to the same state. Schedule padding,
     * and inform if that causes a state transition. */
    return circpad_machine_schedule_padding(mi);
  }

  return CIRCPAD_STATE_UNCHANGED;
}

/**
 * Estimate the circuit RTT from the current middle hop out to the
 * end of the circuit.
 *
 * We estimate RTT by calculating the time between "receive" and
 * "send" at a middle hop. This is because we "receive" a cell
 * from the origin, and then relay it towards the exit before a
 * response comes back. It is that response time from the exit side
 * that we want to measure, so that we can make use of it for synthetic
 * response delays.
 */
static void
circpad_estimate_circ_rtt_on_received(circuit_t *circ,
                                      circpad_machine_runtime_t *mi)
{
  /* Origin circuits don't estimate RTT. They could do it easily enough,
   * but they have no reason to use it in any delay calculations. */
  if (CIRCUIT_IS_ORIGIN(circ) || mi->stop_rtt_update)
    return;

  /* If we already have a last received packet time, that means we
   * did not get a response before this packet. The RTT estimate
   * only makes sense if we do not have multiple packets on the
   * wire, so stop estimating if this is the second packet
   * back to back. However, for the first set of back-to-back
   * packets, we can wait until the very first response comes back
   * to us, to measure that RTT (for the response to optimistic
   * data, for example). Hence stop_rtt_update is only checked
   * in this received side function, and not in send side below.
   */
  if (mi->last_received_time_usec) {
    /* We also allow multiple back-to-back packets if the circuit is not
     * opened, to handle var cells.
     * XXX: Will this work with out var cell plans? Maybe not,
     * since we're opened at the middle hop as soon as we process
     * one var extend2 :/ */
    if (circ->state == CIRCUIT_STATE_OPEN) {
      log_fn(LOG_INFO, LD_CIRC,
           "Stopping padding RTT estimation on circuit (%"PRIu64
           ", %d) after two back to back packets. Current RTT: %d",
           circ->n_chan ?  circ->n_chan->global_identifier : 0,
           circ->n_circ_id, mi->rtt_estimate_usec);
      mi->stop_rtt_update = 1;

      if (!mi->rtt_estimate_usec) {
        static ratelim_t rtt_lim = RATELIM_INIT(600);
        log_fn_ratelim(&rtt_lim,LOG_NOTICE,LD_BUG,
          "Circuit got two cells back to back before estimating RTT.");
      }
    }
  } else {
    const circpad_state_t *state = circpad_machine_current_state(mi);
    if (BUG(!state)) {
      return;
    }

    /* Since monotime is unpredictably expensive, only update this field
     * if rtt estimates are needed. Otherwise, stop the rtt update. */
    if (state->use_rtt_estimate) {
      mi->last_received_time_usec = monotime_absolute_usec();
    } else {
      /* Let's fast-path future decisions not to update rtt if the
       * feature is not in use. */
      mi->stop_rtt_update = 1;
    }
  }
}

/**
 * Handles the "send" side of RTT calculation at middle nodes.
 *
 * This function calculates the RTT from the middle to the end
 * of the circuit by subtracting the last received cell timestamp
 * from the current time. It allows back-to-back cells until
 * the circuit is opened, to allow for var cell handshakes.
 * XXX: Check our var cell plans to make sure this will work.
 */
static void
circpad_estimate_circ_rtt_on_send(circuit_t *circ,
                                  circpad_machine_runtime_t *mi)
{
  /* Origin circuits don't estimate RTT. They could do it easily enough,
   * but they have no reason to use it in any delay calculations. */
  if (CIRCUIT_IS_ORIGIN(circ))
    return;

  /* If last_received_time_usec is non-zero, we are waiting for a response
   * from the exit side. Calculate the time delta and use it as RTT. */
  if (mi->last_received_time_usec) {
    circpad_time_t rtt_time = monotime_absolute_usec() -
        mi->last_received_time_usec;

    /* Reset the last RTT packet time, so we can tell if two cells
     * arrive back to back */
    mi->last_received_time_usec = 0;

    /* Use INT32_MAX to ensure the addition doesn't overflow */
    if (rtt_time >= INT32_MAX) {
      log_fn(LOG_WARN,LD_CIRC,
             "Circuit padding RTT estimate overflowed: %"PRIu64
             " vs %"PRIu64, monotime_absolute_usec(),
               mi->last_received_time_usec);
      return;
    }

    /* If the old RTT estimate is lower than this one, use this one, because
     * the circuit is getting longer. If this estimate is somehow
     * faster than the previous, then maybe that was network jitter, or a
     * bad monotonic clock source (so our ratchet returned a zero delta).
     * In that case, average them. */
    if (mi->rtt_estimate_usec < (circpad_delay_t)rtt_time) {
      mi->rtt_estimate_usec = (circpad_delay_t)rtt_time;
    } else {
      mi->rtt_estimate_usec += (circpad_delay_t)rtt_time;
      mi->rtt_estimate_usec /= 2;
    }
  } else if (circ->state == CIRCUIT_STATE_OPEN) {
    /* If last_received_time_usec is zero, then we have gotten two cells back
     * to back. Stop estimating RTT in this case. Note that we only
     * stop RTT update if the circuit is opened, to allow for RTT estimates
     * of var cells during circ setup. */
    if (!mi->rtt_estimate_usec && !mi->stop_rtt_update) {
      static ratelim_t rtt_lim = RATELIM_INIT(600);
      log_fn_ratelim(&rtt_lim,LOG_NOTICE,LD_BUG,
        "Circuit sent two cells back to back before estimating RTT.");
    }
    mi->stop_rtt_update = 1;
  }
}

/**
 * A "non-padding" cell has been sent from this endpoint. React
 * according to any padding state machines on the circuit.
 *
 * For origin circuits, this means we sent a cell into the network.
 * For middle relay circuits, this means we sent a cell towards the
 * origin.
 */
void
circpad_cell_event_nonpadding_sent(circuit_t *on_circ)
{
  /* Update global cell count */
  circpad_global_nonpadding_sent++;

  /* If there are no machines then this loop should not iterate */
  FOR_EACH_ACTIVE_CIRCUIT_MACHINE_BEGIN(i, on_circ) {
    /* First, update any timestamps */
    on_circ->padding_info[i]->last_cell_time_sec = approx_time();
    circpad_estimate_circ_rtt_on_send(on_circ, on_circ->padding_info[i]);

    /* Then, do accounting */
    circpad_machine_count_nonpadding_sent(on_circ->padding_info[i]);

    /* Check to see if we've run out of tokens for this state already,
     * and if not, check for other state transitions */
    if (check_machine_token_supply(on_circ->padding_info[i])
        == CIRCPAD_STATE_UNCHANGED) {
      /* If removing a token did not cause a transition, check if
       * non-padding sent event should */
      circpad_machine_spec_transition(on_circ->padding_info[i],
                                 CIRCPAD_EVENT_NONPADDING_SENT);
    }
  } FOR_EACH_ACTIVE_CIRCUIT_MACHINE_END;
}

/** Check if this cell or circuit are related to circuit padding and handle
 *  them if so.  Return 0 if the cell was handled in this subsystem and does
 *  not need any other consideration, otherwise return 1.
 */
int
circpad_check_received_cell(cell_t *cell, circuit_t *circ,
                            crypt_path_t *layer_hint,
                            const relay_header_t *rh)
{
  /* First handle the padding commands, since we want to ignore any other
   * commands if this circuit is padding-specific. */
  switch (rh->command) {
    case RELAY_COMMAND_DROP:
      /* Already examined in circpad_deliver_recognized_relay_cell_events */
      return 0;
    case RELAY_COMMAND_PADDING_NEGOTIATE:
      circpad_handle_padding_negotiate(circ, cell);
      return 0;
    case RELAY_COMMAND_PADDING_NEGOTIATED:
      if (circpad_handle_padding_negotiated(circ, cell, layer_hint) == 0)
        circuit_read_valid_data(TO_ORIGIN_CIRCUIT(circ), rh->length);
      return 0;
  }

  /* If this is a padding circuit we don't need to parse any other commands
   * than the padding ones. Just drop them to the floor.
   *
   * Note: we deliberately do not call circuit_read_valid_data() here. The
   * vanguards addon (specifically the 'bandguards' component's dropped cell
   * detection) will thus close this circuit, as it would for any other
   * unexpected cell. However, default tor will *not* close the circuit.
   *
   * This is intentional. We are not yet certain that is it optimal to keep
   * padding circuits open in cases like these, rather than closing them.
   * We suspect that continuing to pad is optimal against a passive classifier,
   * but as soon as the adversary is active (even as a client adversary) this
   * might change.
   *
   * So as a way forward, we log the cell command and circuit number, to
   * help us enumerate the most common instances of this in testing with
   * vanguards, to see which are common enough to verify and handle
   * properly.
   * - Mike
   */
  if (circ->purpose == CIRCUIT_PURPOSE_C_CIRCUIT_PADDING) {
    log_fn(LOG_PROTOCOL_WARN, LD_CIRC,
           "Ignored cell (%d) that arrived in padding circuit "
                      " %u.", rh->command, CIRCUIT_IS_ORIGIN(circ) ?
                           TO_ORIGIN_CIRCUIT(circ)->global_identifier : 0);
    return 0;
  }

  return 1;
}

/**
 * A "non-padding" cell has been received by this endpoint. React
 * according to any padding state machines on the circuit.
 *
 * For origin circuits, this means we read a cell from the network.
 * For middle relay circuits, this means we received a cell from the
 * origin.
 */
void
circpad_cell_event_nonpadding_received(circuit_t *on_circ)
{
  FOR_EACH_ACTIVE_CIRCUIT_MACHINE_BEGIN(i, on_circ) {
    /* First, update any timestamps */
    on_circ->padding_info[i]->last_cell_time_sec = approx_time();
    circpad_estimate_circ_rtt_on_received(on_circ, on_circ->padding_info[i]);

    circpad_machine_spec_transition(on_circ->padding_info[i],
                               CIRCPAD_EVENT_NONPADDING_RECV);
  } FOR_EACH_ACTIVE_CIRCUIT_MACHINE_END;
}

/**
 * A padding cell has been sent from this endpoint. React
 * according to any padding state machines on the circuit.
 *
 * For origin circuits, this means we sent a cell into the network.
 * For middle relay circuits, this means we sent a cell towards the
 * origin.
 */
void
circpad_cell_event_padding_sent(circuit_t *on_circ)
{
  FOR_EACH_ACTIVE_CIRCUIT_MACHINE_BEGIN(i, on_circ) {
    /* Check to see if we've run out of tokens for this state already,
     * and if not, check for other state transitions */
    if (check_machine_token_supply(on_circ->padding_info[i])
        == CIRCPAD_STATE_UNCHANGED) {
      /* If removing a token did not cause a transition, check if
       * non-padding sent event should */

      on_circ->padding_info[i]->last_cell_time_sec = approx_time();
      circpad_machine_spec_transition(on_circ->padding_info[i],
                             CIRCPAD_EVENT_PADDING_SENT);
    }
  } FOR_EACH_ACTIVE_CIRCUIT_MACHINE_END;
}

/**
 * A padding cell has been received by this endpoint. React
 * according to any padding state machines on the circuit.
 *
 * For origin circuits, this means we read a cell from the network.
 * For middle relay circuits, this means we received a cell from the
 * origin.
 */
void
circpad_cell_event_padding_received(circuit_t *on_circ)
{
  /* identical to padding sent */
  FOR_EACH_ACTIVE_CIRCUIT_MACHINE_BEGIN(i, on_circ) {
    on_circ->padding_info[i]->last_cell_time_sec = approx_time();
    circpad_machine_spec_transition(on_circ->padding_info[i],
                              CIRCPAD_EVENT_PADDING_RECV);
  } FOR_EACH_ACTIVE_CIRCUIT_MACHINE_END;
}

/**
 * An "infinite" delay has ben chosen from one of our histograms.
 *
 * "Infinite" delays mean don't send padding -- but they can also
 * mean transition to another state depending on the state machine
 * definitions. Check the rules and react accordingly.
 *
 * Return 1 if we decide to transition, 0 otherwise.
 */
circpad_decision_t
circpad_internal_event_infinity(circpad_machine_runtime_t *mi)
{
  return circpad_machine_spec_transition(mi, CIRCPAD_EVENT_INFINITY);
}

/**
 * All of the bins of our current state's histogram's are empty.
 *
 * Check to see if this means transition to another state, and if
 * not, refill the tokens.
 *
 * Return 1 if we decide to transition, 0 otherwise.
 */
circpad_decision_t
circpad_internal_event_bins_empty(circpad_machine_runtime_t *mi)
{
  if (circpad_machine_spec_transition(mi, CIRCPAD_EVENT_BINS_EMPTY)
      == CIRCPAD_STATE_CHANGED) {
    return CIRCPAD_STATE_CHANGED;
  } else {
    /* If we dont transition, then we refill the tokens */
    circpad_machine_setup_tokens(mi);
    return CIRCPAD_STATE_UNCHANGED;
  }
}

/**
 * This state has used up its cell count. Emit the event and
 * see if we transition.
 *
 * Return 1 if we decide to transition, 0 otherwise.
 */
circpad_decision_t
circpad_internal_event_state_length_up(circpad_machine_runtime_t *mi)
{
  return circpad_machine_spec_transition(mi, CIRCPAD_EVENT_LENGTH_COUNT);
}

/**
 * Returns true if the circuit matches the conditions.
 */
static inline bool
circpad_machine_conditions_apply(origin_circuit_t *circ,
                               const circpad_machine_spec_t *machine)
{
  /* If padding is disabled, no machines should match/apply. This has
   * the effect of shutting down all machines, and not adding any more. */
  if (circpad_padding_disabled || !get_options()->CircuitPadding)
    return 0;

  /* If the consensus or our torrc has selected reduced connection padding,
   * then only allow this machine if it is flagged as acceptable under
   * reduced padding conditions */
  if (circpad_padding_reduced || get_options()->ReducedCircuitPadding) {
    if (!machine->conditions.reduced_padding_ok)
      return 0;
  }

  if (!(circpad_circ_purpose_to_mask(TO_CIRCUIT(circ)->purpose)
      & machine->conditions.apply_purpose_mask))
    return 0;

  if (machine->conditions.requires_vanguards) {
    const or_options_t *options = get_options();

    /* Pinned middles are effectively vanguards */
    if (!(options->HSLayer2Nodes || options->HSLayer3Nodes))
      return 0;
  }

  /* We check for any bits set in the circuit state mask so that machines
   * can say any of the following through their state bitmask:
   * "I want to apply to circuits with either streams or no streams"; OR
   * "I only want to apply to circuits with streams"; OR
   * "I only want to apply to circuits without streams". */
  if (!(circpad_circuit_state(circ) & machine->conditions.apply_state_mask))
    return 0;

  if (circuit_get_cpath_opened_len(circ) < machine->conditions.min_hops)
    return 0;

  return 1;
}

/**
 * Check to see if any of the keep conditions still apply to this circuit.
 *
 * These conditions keep the machines active if they match, but do not
 * cause new machines to start up.
 */
static inline bool
circpad_machine_conditions_keep(origin_circuit_t *circ,
                                const circpad_machine_spec_t *machine)
{
  if ((circpad_circ_purpose_to_mask(TO_CIRCUIT(circ)->purpose)
      & machine->conditions.keep_purpose_mask))
    return 1;

  if ((circpad_circuit_state(circ) & machine->conditions.keep_state_mask))
    return 1;

  return 0;
}

/**
 * Returns a minimized representation of the circuit state.
 *
 * The padding code only cares if the circuit is building,
 * opened, used for streams, and/or still has relay early cells.
 * This returns a bitmask of all state properties that apply to
 * this circuit.
 */
static inline
circpad_circuit_state_t
circpad_circuit_state(origin_circuit_t *circ)
{
  circpad_circuit_state_t retmask = 0;

  if (circ->p_streams)
    retmask |= CIRCPAD_CIRC_STREAMS;
  else
    retmask |= CIRCPAD_CIRC_NO_STREAMS;

  /* We use has_opened to prevent cannibialized circs from flapping. */
  if (circ->has_opened)
    retmask |= CIRCPAD_CIRC_OPENED;
  else
    retmask |= CIRCPAD_CIRC_BUILDING;

  if (circ->remaining_relay_early_cells > 0)
    retmask |= CIRCPAD_CIRC_HAS_RELAY_EARLY;
  else
    retmask |= CIRCPAD_CIRC_HAS_NO_RELAY_EARLY;

  return retmask;
}

/**
 * Convert a normal circuit purpose into a bitmask that we can
 * use for determining matching circuits.
 */
circpad_purpose_mask_t
circpad_circ_purpose_to_mask(uint8_t circ_purpose)
{
  /* Treat OR circ purposes as ignored. They should not be passed here*/
  if (BUG(circ_purpose <= CIRCUIT_PURPOSE_OR_MAX_)) {
    return 0;
  }

  /* Treat new client circuit purposes as "OMG ITS EVERYTHING".
   * This also should not happen */
  if (BUG(circ_purpose - CIRCUIT_PURPOSE_OR_MAX_ - 1 > 32)) {
    return CIRCPAD_PURPOSE_ALL;
  }

  /* Convert the purpose to a bit position */
  return 1 << (circ_purpose - CIRCUIT_PURPOSE_OR_MAX_ - 1);
}

/**
 * Shut down any machines whose conditions no longer match
 * the current circuit.
 */
static void
circpad_shutdown_old_machines(origin_circuit_t *on_circ)
{
  circuit_t *circ = TO_CIRCUIT(on_circ);

  FOR_EACH_ACTIVE_CIRCUIT_MACHINE_BEGIN(i, circ) {
    /* We shut down a machine if neither the apply conditions
     * nor the keep conditions match. If either set of conditions match,
     * keep it around. */
    if (!circpad_machine_conditions_apply(on_circ,
                                        circ->padding_machine[i]) &&
        !circpad_machine_conditions_keep(on_circ,
                                        circ->padding_machine[i])) {
      uint32_t machine_ctr = circ->padding_info[i]->machine_ctr;
      // Clear machineinfo (frees timers)
      circpad_circuit_machineinfo_free_idx(circ, i);
      // Send padding negotiate stop
      circpad_negotiate_padding(on_circ,
                                circ->padding_machine[i]->machine_num,
                                circ->padding_machine[i]->target_hopnum,
                                CIRCPAD_COMMAND_STOP,
                                machine_ctr);
    }
  } FOR_EACH_ACTIVE_CIRCUIT_MACHINE_END;
}

/**
 * Negotiate new machines that would apply to this circuit, given the machines
 * inside <b>machines_sl</b>.
 *
 * This function checks to see if we have any free machine indexes,
 * and for each free machine index, it initializes the most recently
 * added origin-side padding machine that matches the target machine
 * index and circuit conditions, and negotiates it with the appropriate
 * middle relay.
 */
STATIC void
circpad_add_matching_machines(origin_circuit_t *on_circ,
                              smartlist_t *machines_sl)
{
  circuit_t *circ = TO_CIRCUIT(on_circ);

#ifdef TOR_UNIT_TESTS
  /* Tests don't have to init our padding machines */
  if (!machines_sl)
    return;
#endif

  /* If padding negotiation failed before, do not try again */
  if (on_circ->padding_negotiation_failed)
    return;

  FOR_EACH_CIRCUIT_MACHINE_BEGIN(i) {
    /* If there is a padding machine info, this index is occupied.
     * No need to check conditions for this index. */
    if (circ->padding_info[i])
      continue;

    /* We have a free machine index. Check the origin padding
     * machines in reverse order, so that more recently added
     * machines take priority over older ones. */
    SMARTLIST_FOREACH_REVERSE_BEGIN(machines_sl,
                                    circpad_machine_spec_t *,
                                    machine) {
      /* Machine definitions have a specific target machine index.
       * This is so event ordering is deterministic with respect
       * to which machine gets events first when there are two
       * machines installed on a circuit. Make sure we only
       * add this machine if its target machine index is free. */
      if (machine->machine_index == i &&
          circpad_machine_conditions_apply(on_circ, machine)) {

        // We can only replace this machine if the target hopnum
        // is the same, otherwise we'll get invalid data
        if (circ->padding_machine[i]) {
          if (circ->padding_machine[i]->target_hopnum !=
              machine->target_hopnum)
            continue;
          /* Replace it. (Don't free - is global). */
          circ->padding_machine[i] = NULL;
        }

        /* Set up the machine immediately so that the slot is occupied.
         * We will tear it down on error return, or if there is an error
         * response from the relay. */
        circpad_setup_machine_on_circ(circ, machine);
        if (circpad_negotiate_padding(on_circ, machine->machine_num,
                                  machine->target_hopnum,
                                  CIRCPAD_COMMAND_START,
                                  circ->padding_machine_ctr) < 0) {
          log_info(LD_CIRC,
                   "Padding not negotiated. Cleaning machine from circuit %u",
             CIRCUIT_IS_ORIGIN(circ) ?
             TO_ORIGIN_CIRCUIT(circ)->global_identifier : 0);
          circpad_circuit_machineinfo_free_idx(circ, i);
          circ->padding_machine[i] = NULL;
          on_circ->padding_negotiation_failed = 1;
        } else {
          /* Success. Don't try any more machines on this index */
          break;
        }
      }
    } SMARTLIST_FOREACH_END(machine);
  } FOR_EACH_CIRCUIT_MACHINE_END;
}

/**
 * Event that tells us we added a hop to an origin circuit.
 *
 * This event is used to decide if we should create a padding machine
 * on a circuit.
 */
void
circpad_machine_event_circ_added_hop(origin_circuit_t *on_circ)
{
  /* Since our padding conditions do not specify a max_hops,
   * all we can do is add machines here */
  circpad_add_matching_machines(on_circ, origin_padding_machines);
}

/**
 * Event that tells us that an origin circuit is now built.
 *
 * Shut down any machines that only applied to un-built circuits.
 * Activate any new ones.
 */
void
circpad_machine_event_circ_built(origin_circuit_t *circ)
{
  circpad_shutdown_old_machines(circ);
  circpad_add_matching_machines(circ, origin_padding_machines);
}

/**
 * Circpad purpose changed event.
 *
 * Shut down any machines that don't apply to our circ purpose.
 * Activate any new ones that do.
 */
void
circpad_machine_event_circ_purpose_changed(origin_circuit_t *circ)
{
  circpad_shutdown_old_machines(circ);
  circpad_add_matching_machines(circ, origin_padding_machines);
}

/**
 * Event that tells us that an origin circuit is out of RELAY_EARLY
 * cells.
 *
 * Shut down any machines that only applied to RELAY_EARLY circuits.
 * Activate any new ones.
 */
void
circpad_machine_event_circ_has_no_relay_early(origin_circuit_t *circ)
{
  circpad_shutdown_old_machines(circ);
  circpad_add_matching_machines(circ, origin_padding_machines);
}

/**
 * Streams attached event.
 *
 * Called from link_apconn_to_circ() and handle_hs_exit_conn()
 *
 * Shut down any machines that only applied to machines without
 * streams. Activate any new ones.
 */
void
circpad_machine_event_circ_has_streams(origin_circuit_t *circ)
{
  circpad_shutdown_old_machines(circ);
  circpad_add_matching_machines(circ, origin_padding_machines);
}

/**
 * Streams detached event.
 *
 * Called from circuit_detach_stream()
 *
 * Shut down any machines that only applied to machines without
 * streams. Activate any new ones.
 */
void
circpad_machine_event_circ_has_no_streams(origin_circuit_t *circ)
{
  circpad_shutdown_old_machines(circ);
  circpad_add_matching_machines(circ, origin_padding_machines);
}

/**
 * Verify that padding is coming from the expected hop.
 *
 * Returns true if from_hop matches the target hop from
 * one of our padding machines.
 *
 * Returns false if we're not an origin circuit, or if from_hop
 * does not match one of the padding machines.
 */
bool
circpad_padding_is_from_expected_hop(circuit_t *circ,
                                     crypt_path_t *from_hop)
{
  crypt_path_t *target_hop = NULL;
  if (!CIRCUIT_IS_ORIGIN(circ))
    return 0;

  FOR_EACH_CIRCUIT_MACHINE_BEGIN(i) {
    /* We have to check padding_machine and not padding_info/active
     * machines here because padding may arrive after we shut down a
     * machine. The info is gone, but the padding_machine waits
     * for the padding_negotiated response to come back. */
    if (!circ->padding_machine[i])
      continue;

    target_hop = circuit_get_cpath_hop(TO_ORIGIN_CIRCUIT(circ),
                    circ->padding_machine[i]->target_hopnum);

    if (target_hop == from_hop)
      return 1;
  } FOR_EACH_CIRCUIT_MACHINE_END;

  return 0;
}

/**
 * Deliver circpad events for an "unrecognized cell".
 *
 * Unrecognized cells are sent to relays and are forwarded
 * onto the next hop of their circuits. Unrecognized cells
 * are by definition not padding. We need to tell relay-side
 * state machines that a non-padding cell was sent or received,
 * depending on the direction, so they can update their histograms
 * and decide to pad or not.
 */
void
circpad_deliver_unrecognized_cell_events(circuit_t *circ,
                                         cell_direction_t dir)
{
  // We should never see unrecognized cells at origin.
  // Our caller emits a warn when this happens.
  if (CIRCUIT_IS_ORIGIN(circ)) {
    return;
  }

  if (dir == CELL_DIRECTION_OUT) {
    /* When direction is out (away from origin), then we received non-padding
       cell coming from the origin to us. */
    circpad_cell_event_nonpadding_received(circ);
  } else if (dir == CELL_DIRECTION_IN) {
    /* It's in and not origin, so the cell is going away from us.
     * So we are relaying a non-padding cell towards the origin. */
    circpad_cell_event_nonpadding_sent(circ);
  }
}

/**
 * Deliver circpad events for "recognized" relay cells.
 *
 * Recognized cells are destined for this hop, either client or middle.
 * Check if this is a padding cell or not, and send the appropriate
 * received event.
 */
void
circpad_deliver_recognized_relay_cell_events(circuit_t *circ,
                                             uint8_t relay_command,
                                             crypt_path_t *layer_hint)
{
  if (relay_command == RELAY_COMMAND_DROP) {
    rep_hist_padding_count_read(PADDING_TYPE_DROP);

    if (CIRCUIT_IS_ORIGIN(circ)) {
      if (circpad_padding_is_from_expected_hop(circ, layer_hint)) {
        circuit_read_valid_data(TO_ORIGIN_CIRCUIT(circ), 0);
      } else {
        /* This is unexpected padding. Ignore it for now. */
        return;
      }
    }

    /* The cell should be recognized by now, which means that we are on the
       destination, which means that we received a padding cell. We might be
       the client or the Middle node, still, because leaky-pipe. */
    circpad_cell_event_padding_received(circ);
    log_fn(LOG_INFO, LD_CIRC, "Got padding cell on %s circuit %u.",
           CIRCUIT_IS_ORIGIN(circ) ? "origin" : "non-origin",
           CIRCUIT_IS_ORIGIN(circ) ?
             TO_ORIGIN_CIRCUIT(circ)->global_identifier : 0);
  } else {
    /* We received a non-padding cell on the edge */
    circpad_cell_event_nonpadding_received(circ);
  }
}

/**
 * Deliver circpad events for relay cells sent from us.
 *
 * If this is a padding cell, update our padding stats
 * and deliver the event. Otherwise just deliver the event.
 */
void
circpad_deliver_sent_relay_cell_events(circuit_t *circ,
                                       uint8_t relay_command)
{
  /* RELAY_COMMAND_DROP is the multi-hop (aka circuit-level) padding cell in
   * tor. (CELL_PADDING is a channel-level padding cell, which is not relayed
   * or processed here).
   *
   * We do generate events for PADDING_NEGOTIATE and PADDING_NEGOTIATED cells.
   */
  if (relay_command == RELAY_COMMAND_DROP) {
    /* Optimization: The event for RELAY_COMMAND_DROP is sent directly
     * from circpad_send_padding_cell_for_callback(). This is to avoid
     * putting a cell_t and a relay_header_t on the stack repeatedly
     * if we decide to send a long train of padding cells back-to-back
     * with 0 delay. So we do nothing here. */
    return;
  } else {
    /* This is a non-padding cell sent from the client or from
     * this node. */
    circpad_cell_event_nonpadding_sent(circ);
  }
}

/**
 * Initialize the states array for a circpad machine.
 */
void
circpad_machine_states_init(circpad_machine_spec_t *machine,
                            circpad_statenum_t num_states)
{
  if (BUG(num_states > CIRCPAD_MAX_MACHINE_STATES)) {
    num_states = CIRCPAD_MAX_MACHINE_STATES;
  }

  machine->num_states = num_states;
  machine->states = tor_malloc_zero(sizeof(circpad_state_t)*num_states);

  /* Initialize the default next state for all events to
   * "ignore" -- if events aren't specified, they are ignored. */
  for (circpad_statenum_t s = 0; s < num_states; s++) {
    for (int e = 0; e < CIRCPAD_NUM_EVENTS; e++) {
      machine->states[s].next_state[e] = CIRCPAD_STATE_IGNORE;
    }
  }
}

static void
circpad_setup_machine_on_circ(circuit_t *on_circ,
                              const circpad_machine_spec_t *machine)
{
  if (CIRCUIT_IS_ORIGIN(on_circ) && !machine->is_origin_side) {
    log_fn(LOG_WARN, LD_BUG,
           "Can't set up non-origin machine on origin circuit!");
    return;
  }

  if (!CIRCUIT_IS_ORIGIN(on_circ) && machine->is_origin_side) {
    log_fn(LOG_WARN, LD_BUG,
           "Can't set up origin machine on non-origin circuit!");
    return;
  }

  IF_BUG_ONCE(on_circ->padding_machine[machine->machine_index] != NULL) {
    return;
  }
  IF_BUG_ONCE(on_circ->padding_info[machine->machine_index] != NULL) {
    return;
  }

  /* Log message */
  if (CIRCUIT_IS_ORIGIN(on_circ)) {
    log_info(LD_CIRC, "Registering machine %s to origin circ %u (%d)",
             machine->name,
             TO_ORIGIN_CIRCUIT(on_circ)->global_identifier, on_circ->purpose);
  } else {
    log_info(LD_CIRC, "Registering machine %s to non-origin circ (%d)",
             machine->name, on_circ->purpose);
  }

  /* Padding machine ctr starts at 1, so we increment this ctr first.
   * (machine ctr of 0 means "any machine").
   *
   * See https://bugs.tororject.org/30992. */
  on_circ->padding_machine_ctr++;

  /* uint32 wraparound check: 0 is special, just wrap to 1 */
  if (on_circ->padding_machine_ctr == 0) {
    on_circ->padding_machine_ctr = 1;
  }

  on_circ->padding_info[machine->machine_index] =
      circpad_circuit_machineinfo_new(on_circ, machine->machine_index);
  on_circ->padding_machine[machine->machine_index] = machine;
}

/** Validate a single state of a padding machine */
static bool
padding_machine_state_is_valid(const circpad_state_t *state)
{
  int b;
  uint32_t tokens_count = 0;
  circpad_delay_t prev_bin_edge = 0;

  /* We only validate histograms */
  if (!state->histogram_len) {
    return true;
  }

  /* We need at least two bins in a histogram */
  if (state->histogram_len < 2) {
    log_warn(LD_CIRC, "You can't have a histogram with less than 2 bins");
    return false;
  }

  /* For each machine state, if it's a histogram, make sure all the
   * histogram edges are well defined (i.e. are strictly monotonic). */
  for (b = 0 ; b < state->histogram_len ; b++) {
    /* Check that histogram edges are strictly increasing. Ignore the first
     * edge since it can be zero. */
    if (prev_bin_edge >= state->histogram_edges[b] && b > 0) {
      log_warn(LD_CIRC, "Histogram edges are not increasing [%u/%u]",
               prev_bin_edge, state->histogram_edges[b]);
      return false;
    }

    prev_bin_edge = state->histogram_edges[b];

    /* Also count the number of tokens as we go through the histogram states */
    tokens_count += state->histogram[b];
  }
  /* Verify that the total number of tokens is correct */
  if (tokens_count != state->histogram_total_tokens) {
    log_warn(LD_CIRC, "Histogram token count is wrong [%u/%u]",
             tokens_count, state->histogram_total_tokens);
    return false;
  }

  return true;
}

/** Basic validation of padding machine */
static bool
padding_machine_is_valid(const circpad_machine_spec_t *machine)
{
  int i;

  /* Validate the histograms of the padding machine */
  for (i = 0 ; i < machine->num_states ; i++) {
    if (!padding_machine_state_is_valid(&machine->states[i])) {
      return false;
    }
  }

  return true;
}

/* Validate and register <b>machine</b> into <b>machine_list</b>. If
 * <b>machine_list</b> is NULL, then just validate. */
void
circpad_register_padding_machine(circpad_machine_spec_t *machine,
                                 smartlist_t *machine_list)
{
  if (!padding_machine_is_valid(machine)) {
    log_warn(LD_CIRC, "Machine #%u is invalid. Ignoring.",
             machine->machine_num);
    return;
  }

  if (machine_list) {
    smartlist_add(machine_list, machine);
  }
}

#ifdef TOR_UNIT_TESTS
/* These padding machines are only used for tests pending #28634. */
static void
circpad_circ_client_machine_init(void)
{
  circpad_machine_spec_t *circ_client_machine
      = tor_malloc_zero(sizeof(circpad_machine_spec_t));

  circ_client_machine->conditions.min_hops = 2;
  circ_client_machine->conditions.apply_state_mask =
      CIRCPAD_CIRC_BUILDING|CIRCPAD_CIRC_OPENED|CIRCPAD_CIRC_HAS_RELAY_EARLY;
  circ_client_machine->conditions.apply_purpose_mask = CIRCPAD_PURPOSE_ALL;
  circ_client_machine->conditions.reduced_padding_ok = 1;

  circ_client_machine->target_hopnum = 2;
  circ_client_machine->is_origin_side = 1;

  /* Start, gap, burst */
  circpad_machine_states_init(circ_client_machine, 3);

  circ_client_machine->states[CIRCPAD_STATE_START].
      next_state[CIRCPAD_EVENT_NONPADDING_RECV] = CIRCPAD_STATE_BURST;

  circ_client_machine->states[CIRCPAD_STATE_BURST].
      next_state[CIRCPAD_EVENT_NONPADDING_RECV] = CIRCPAD_STATE_BURST;
  circ_client_machine->states[CIRCPAD_STATE_BURST].
      next_state[CIRCPAD_EVENT_PADDING_RECV] = CIRCPAD_STATE_BURST;

  /* If we are in burst state, and we send a non-padding cell, then we cancel
     the timer for the next padding cell:
     We dont want to send fake extends when actual extends are going on */
  circ_client_machine->states[CIRCPAD_STATE_BURST].
      next_state[CIRCPAD_EVENT_NONPADDING_SENT] = CIRCPAD_STATE_CANCEL;

  circ_client_machine->states[CIRCPAD_STATE_BURST].
      next_state[CIRCPAD_EVENT_BINS_EMPTY] = CIRCPAD_STATE_END;

  circ_client_machine->states[CIRCPAD_STATE_BURST].token_removal =
      CIRCPAD_TOKEN_REMOVAL_CLOSEST;

  circ_client_machine->states[CIRCPAD_STATE_BURST].histogram_len = 2;
  circ_client_machine->states[CIRCPAD_STATE_BURST].histogram_edges[0]= 500;
  circ_client_machine->states[CIRCPAD_STATE_BURST].histogram_edges[1]= 1000000;

  /* We have 5 tokens in the histogram, which means that all circuits will look
   * like they have 7 hops (since we start this machine after the second hop,
   * and tokens are decremented for any valid hops, and fake extends are
   * used after that -- 2+5==7). */
  circ_client_machine->states[CIRCPAD_STATE_BURST].histogram[0] = 5;

  circ_client_machine->states[CIRCPAD_STATE_BURST].histogram_total_tokens = 5;

  circ_client_machine->machine_num = smartlist_len(origin_padding_machines);
  circpad_register_padding_machine(circ_client_machine,
                                   origin_padding_machines);
}

static void
circpad_circ_responder_machine_init(void)
{
  circpad_machine_spec_t *circ_responder_machine
      = tor_malloc_zero(sizeof(circpad_machine_spec_t));

  /* Shut down the machine after we've sent enough packets */
  circ_responder_machine->should_negotiate_end = 1;

  /* The relay-side doesn't care what hopnum it is, but for consistency,
   * let's match the client */
  circ_responder_machine->target_hopnum = 2;
  circ_responder_machine->is_origin_side = 0;

  /* Start, gap, burst */
  circpad_machine_states_init(circ_responder_machine, 3);

  /* This is the settings of the state machine. In the future we are gonna
     serialize this into the consensus or the torrc */

  /* We transition to the burst state on padding receive and on non-padding
   * receive */
  circ_responder_machine->states[CIRCPAD_STATE_START].
      next_state[CIRCPAD_EVENT_PADDING_RECV] = CIRCPAD_STATE_BURST;
  circ_responder_machine->states[CIRCPAD_STATE_START].
      next_state[CIRCPAD_EVENT_NONPADDING_RECV] = CIRCPAD_STATE_BURST;

  /* Inside the burst state we _stay_ in the burst state when a non-padding
   * is sent */
  circ_responder_machine->states[CIRCPAD_STATE_BURST].
      next_state[CIRCPAD_EVENT_NONPADDING_SENT] = CIRCPAD_STATE_BURST;

  /* Inside the burst state we transition to the gap state when we receive a
   * padding cell */
  circ_responder_machine->states[CIRCPAD_STATE_BURST].
      next_state[CIRCPAD_EVENT_PADDING_RECV] = CIRCPAD_STATE_GAP;

  /* These describe the padding charasteristics when in burst state */

  /* use_rtt_estimate tries to estimate how long padding cells take to go from
     C->M, and uses that as what as the base of the histogram */
  circ_responder_machine->states[CIRCPAD_STATE_BURST].use_rtt_estimate = 1;
  /* The histogram is 2 bins: an empty one, and infinity */
  circ_responder_machine->states[CIRCPAD_STATE_BURST].histogram_len = 2;
  circ_responder_machine->states[CIRCPAD_STATE_BURST].histogram_edges[0]= 500;
  circ_responder_machine->states[CIRCPAD_STATE_BURST].histogram_edges[1] =
    1000000;
  /* During burst state we wait forever for padding to arrive.

     We are waiting for a padding cell from the client to come in, so that we
     respond, and we imitate how extend looks like */
  circ_responder_machine->states[CIRCPAD_STATE_BURST].histogram[0] = 0;
  // Only infinity bin:
  circ_responder_machine->states[CIRCPAD_STATE_BURST].histogram[1] = 1;
  circ_responder_machine->states[CIRCPAD_STATE_BURST].
      histogram_total_tokens = 1;

  /* From the gap state, we _stay_ in the gap state, when we receive padding
   * or non padding */
  circ_responder_machine->states[CIRCPAD_STATE_GAP].
      next_state[CIRCPAD_EVENT_PADDING_RECV] = CIRCPAD_STATE_GAP;
  circ_responder_machine->states[CIRCPAD_STATE_GAP].
      next_state[CIRCPAD_EVENT_NONPADDING_RECV] = CIRCPAD_STATE_GAP;

  /* And from the gap state, we go to the end, when the bins are empty or a
   * non-padding cell is sent */
  circ_responder_machine->states[CIRCPAD_STATE_GAP].
      next_state[CIRCPAD_EVENT_BINS_EMPTY] = CIRCPAD_STATE_END;
  circ_responder_machine->states[CIRCPAD_STATE_GAP].
      next_state[CIRCPAD_EVENT_NONPADDING_SENT] = CIRCPAD_STATE_END;

  // FIXME: Tune this histogram

  /* The gap state is the delay you wait after you receive a padding cell
     before you send a padding response */
  circ_responder_machine->states[CIRCPAD_STATE_GAP].use_rtt_estimate = 1;
  circ_responder_machine->states[CIRCPAD_STATE_GAP].histogram_len = 6;
  /* Specify histogram bins */
  circ_responder_machine->states[CIRCPAD_STATE_GAP].histogram_edges[0]= 500;
  circ_responder_machine->states[CIRCPAD_STATE_GAP].histogram_edges[1]= 1000;
  circ_responder_machine->states[CIRCPAD_STATE_GAP].histogram_edges[2]= 2000;
  circ_responder_machine->states[CIRCPAD_STATE_GAP].histogram_edges[3]= 4000;
  circ_responder_machine->states[CIRCPAD_STATE_GAP].histogram_edges[4]= 8000;
  circ_responder_machine->states[CIRCPAD_STATE_GAP].histogram_edges[5]= 16000;
  /* Specify histogram tokens */
  circ_responder_machine->states[CIRCPAD_STATE_GAP].histogram[0] = 0;
  circ_responder_machine->states[CIRCPAD_STATE_GAP].histogram[1] = 1;
  circ_responder_machine->states[CIRCPAD_STATE_GAP].histogram[2] = 2;
  circ_responder_machine->states[CIRCPAD_STATE_GAP].histogram[3] = 2;
  circ_responder_machine->states[CIRCPAD_STATE_GAP].histogram[4] = 1;
  /* Total number of tokens */
  circ_responder_machine->states[CIRCPAD_STATE_GAP].histogram_total_tokens = 6;

  circ_responder_machine->states[CIRCPAD_STATE_GAP].token_removal =
      CIRCPAD_TOKEN_REMOVAL_CLOSEST_USEC;

  circ_responder_machine->machine_num = smartlist_len(relay_padding_machines);
  circpad_register_padding_machine(circ_responder_machine,
                                   relay_padding_machines);
}
#endif /* defined(TOR_UNIT_TESTS) */

/**
 * Initialize all of our padding machines.
 *
 * This is called at startup. It sets up some global machines, and then
 * loads some from torrc, and from the tor consensus.
 */
void
circpad_machines_init(void)
{
  tor_assert_nonfatal(origin_padding_machines == NULL);
  tor_assert_nonfatal(relay_padding_machines == NULL);

  origin_padding_machines = smartlist_new();
  relay_padding_machines = smartlist_new();

  /* Register machines for hiding client-side intro circuits */
  circpad_machine_client_hide_intro_circuits(origin_padding_machines);
  circpad_machine_relay_hide_intro_circuits(relay_padding_machines);

  /* Register machines for hiding client-side rendezvous circuits */
  circpad_machine_client_hide_rend_circuits(origin_padding_machines);
  circpad_machine_relay_hide_rend_circuits(relay_padding_machines);

  // TODO: Parse machines from consensus and torrc
#ifdef TOR_UNIT_TESTS
  circpad_circ_client_machine_init();
  circpad_circ_responder_machine_init();
#endif
}

/**
 * Free our padding machines
 */
void
circpad_machines_free(void)
{
  if (origin_padding_machines) {
    SMARTLIST_FOREACH(origin_padding_machines,
                      circpad_machine_spec_t *,
                      m, tor_free(m->states); tor_free(m));
    smartlist_free(origin_padding_machines);
  }

  if (relay_padding_machines) {
    SMARTLIST_FOREACH(relay_padding_machines,
                      circpad_machine_spec_t *,
                      m, tor_free(m->states); tor_free(m));
    smartlist_free(relay_padding_machines);
  }
}

/**
 * Check the Protover info to see if a node supports padding.
 */
static bool
circpad_node_supports_padding(const node_t *node)
{
  if (node->rs) {
    log_fn(LOG_INFO, LD_CIRC, "Checking padding: %s",
           node->rs->pv.supports_hs_setup_padding ?
              "supported" : "unsupported");
    return node->rs->pv.supports_hs_setup_padding;
  }

  log_fn(LOG_INFO, LD_CIRC, "Empty routerstatus in padding check");
  return 0;
}

/**
 * Get a node_t for the nth hop in our circuit, starting from 1.
 *
 * Returns node_t from the consensus for that hop, if it is opened.
 * Otherwise returns NULL.
 */
MOCK_IMPL(STATIC const node_t *,
circuit_get_nth_node,(origin_circuit_t *circ, int hop))
{
  crypt_path_t *iter = circuit_get_cpath_hop(circ, hop);

  if (!iter || iter->state != CPATH_STATE_OPEN)
    return NULL;

  return node_get_by_id(iter->extend_info->identity_digest);
}

/**
 * Return true if a particular circuit supports padding
 * at the desired hop.
 */
static bool
circpad_circuit_supports_padding(origin_circuit_t *circ,
                                 int target_hopnum)
{
  const node_t *hop;

  if (!(hop = circuit_get_nth_node(circ, target_hopnum))) {
    return 0;
  }

  return circpad_node_supports_padding(hop);
}

/**
 * Try to negotiate padding.
 *
 * Returns -1 on error, 0 on success.
 */
signed_error_t
circpad_negotiate_padding(origin_circuit_t *circ,
                          circpad_machine_num_t machine,
                          uint8_t target_hopnum,
                          uint8_t command,
                          uint32_t machine_ctr)
{
  circpad_negotiate_t type;
  cell_t cell;
  ssize_t len;

  /* Check that the target hop lists support for padding in
   * its ProtoVer fields */
  if (!circpad_circuit_supports_padding(circ, target_hopnum)) {
    return -1;
  }

  memset(&cell, 0, sizeof(cell_t));
  memset(&type, 0, sizeof(circpad_negotiate_t));
  // This gets reset to RELAY_EARLY appropriately by
  // relay_send_command_from_edge_. At least, it looks that way.
  // QQQ-MP-AP: Verify that.
  cell.command = CELL_RELAY;

  circpad_negotiate_set_command(&type, command);
  circpad_negotiate_set_version(&type, 0);
  circpad_negotiate_set_machine_type(&type, machine);
  circpad_negotiate_set_machine_ctr(&type, machine_ctr);

  if ((len = circpad_negotiate_encode(cell.payload, CELL_PAYLOAD_SIZE,
        &type)) < 0)
    return -1;

  log_fn(LOG_INFO,LD_CIRC,
         "Negotiating padding on circuit %u (%d), command %d, for ctr %u",
         circ->global_identifier, TO_CIRCUIT(circ)->purpose, command,
         machine_ctr);

  return circpad_send_command_to_hop(circ, target_hopnum,
                                     RELAY_COMMAND_PADDING_NEGOTIATE,
                                     cell.payload, len);
}

/**
 * Try to negotiate padding.
 *
 * Returns 1 if successful (or already set up), 0 otherwise.
 */
bool
circpad_padding_negotiated(circuit_t *circ,
                           circpad_machine_num_t machine,
                           uint8_t command,
                           uint8_t response,
                           uint32_t machine_ctr)
{
  circpad_negotiated_t type;
  cell_t cell;
  ssize_t len;

  memset(&cell, 0, sizeof(cell_t));
  memset(&type, 0, sizeof(circpad_negotiated_t));
  // This gets reset to RELAY_EARLY appropriately by
  // relay_send_command_from_edge_. At least, it looks that way.
  // QQQ-MP-AP: Verify that.
  cell.command = CELL_RELAY;

  circpad_negotiated_set_command(&type, command);
  circpad_negotiated_set_response(&type, response);
  circpad_negotiated_set_version(&type, 0);
  circpad_negotiated_set_machine_type(&type, machine);
  circpad_negotiated_set_machine_ctr(&type, machine_ctr);

  if ((len = circpad_negotiated_encode(cell.payload, CELL_PAYLOAD_SIZE,
        &type)) < 0)
    return 0;

  /* Use relay_send because we're from the middle to the origin. We don't
   * need to specify a target hop or layer_hint. */
  return relay_send_command_from_edge(0, circ,
                                      RELAY_COMMAND_PADDING_NEGOTIATED,
                                      (void*)cell.payload,
                                      (size_t)len, NULL) == 0;
}

/**
 * Parse and react to a padding_negotiate cell.
 *
 * This is called at the middle node upon receipt of the client's choice of
 * state machine, so that it can use the requested state machine index, if
 * it is available.
 *
 * Returns -1 on error, 0 on success.
 */
signed_error_t
circpad_handle_padding_negotiate(circuit_t *circ, cell_t *cell)
{
  int retval = 0;
  /* Should we send back a STOP cell? */
  bool respond_with_stop = true;
  circpad_negotiate_t *negotiate;

  if (CIRCUIT_IS_ORIGIN(circ)) {
    log_fn(LOG_PROTOCOL_WARN, LD_CIRC,
           "Padding negotiate cell unsupported at origin (circuit %u)",
           TO_ORIGIN_CIRCUIT(circ)->global_identifier);
    return -1;
  }

  if (circpad_negotiate_parse(&negotiate, cell->payload+RELAY_HEADER_SIZE,
                               CELL_PAYLOAD_SIZE-RELAY_HEADER_SIZE) < 0) {
    log_fn(LOG_PROTOCOL_WARN, LD_CIRC,
          "Received malformed PADDING_NEGOTIATE cell; dropping.");
    return -1;
  }

  if (negotiate->command == CIRCPAD_COMMAND_STOP) {
    /* Free the machine corresponding to this machine type */
    if (free_circ_machineinfos_with_machine_num(circ,
                negotiate->machine_type,
                negotiate->machine_ctr)) {
      log_info(LD_CIRC, "Received STOP command for machine %u, ctr %u",
               negotiate->machine_type, negotiate->machine_ctr);
      goto done;
    }

    /* If we reached this point we received a STOP command from an old or
       unknown machine. Don't reply with our own STOP since there is no one to
       handle it on the other end */
    respond_with_stop = false;

    if (negotiate->machine_ctr <= circ->padding_machine_ctr) {
      log_info(LD_CIRC, "Received STOP command for old machine %u, ctr %u",
               negotiate->machine_type, negotiate->machine_ctr);
      goto done;

    } else {
      log_fn(LOG_PROTOCOL_WARN, LD_CIRC,
                "Received circuit padding stop command for unknown machine.");
      goto err;
    }
 } else if (negotiate->command == CIRCPAD_COMMAND_START) {
    SMARTLIST_FOREACH_BEGIN(relay_padding_machines,
                            const circpad_machine_spec_t *, m) {
      if (m->machine_num == negotiate->machine_type) {
        circpad_setup_machine_on_circ(circ, m);
        if (negotiate->machine_ctr &&
            circ->padding_machine_ctr != negotiate->machine_ctr) {
            log_fn(LOG_PROTOCOL_WARN, LD_CIRC,
              "Client and relay have different counts for padding machines: "
              "%u vs %u", circ->padding_machine_ctr, negotiate->machine_ctr);
        }
        circpad_cell_event_nonpadding_received(circ);
        goto done;
      }
    } SMARTLIST_FOREACH_END(m);
  }

  err:
    retval = -1;

  done:
    if (respond_with_stop) {
      circpad_padding_negotiated(circ, negotiate->machine_type,
                    negotiate->command,
                    (retval == 0) ? CIRCPAD_RESPONSE_OK : CIRCPAD_RESPONSE_ERR,
                    negotiate->machine_ctr);
    }

    circpad_negotiate_free(negotiate);

    return retval;
}

/**
 * Parse and react to a padding_negotiated cell.
 *
 * This is called at the origin upon receipt of the middle's response
 * to our choice of state machine.
 *
 * Returns -1 on error, 0 on success.
 */
signed_error_t
circpad_handle_padding_negotiated(circuit_t *circ, cell_t *cell,
                                  crypt_path_t *layer_hint)
{
  circpad_negotiated_t *negotiated;

  if (!CIRCUIT_IS_ORIGIN(circ)) {
    log_fn(LOG_PROTOCOL_WARN, LD_CIRC,
           "Padding negotiated cell unsupported at non-origin.");
    return -1;
  }

  /* Verify this came from the expected hop */
  if (!circpad_padding_is_from_expected_hop(circ, layer_hint)) {
    log_fn(LOG_PROTOCOL_WARN, LD_CIRC,
           "Padding negotiated cell from wrong hop on circuit %u",
             TO_ORIGIN_CIRCUIT(circ)->global_identifier);
    return -1;
  }

  if (circpad_negotiated_parse(&negotiated, cell->payload+RELAY_HEADER_SIZE,
                               CELL_PAYLOAD_SIZE-RELAY_HEADER_SIZE) < 0) {
    log_fn(LOG_PROTOCOL_WARN, LD_CIRC,
          "Received malformed PADDING_NEGOTIATED cell on circuit %u; "
          "dropping.", TO_ORIGIN_CIRCUIT(circ)->global_identifier);
    return -1;
  }

  if (negotiated->command == CIRCPAD_COMMAND_STOP) {
    log_info(LD_CIRC,
             "Received STOP command on PADDING_NEGOTIATED for circuit %u",
             TO_ORIGIN_CIRCUIT(circ)->global_identifier);
    /* There may not be a padding_info here if we shut down the
     * machine in circpad_shutdown_old_machines(). Or, if
     * circpad_add_matching_matchines() added a new machine,
     * there may be a padding_machine for a different machine num
     * than this response. */
    free_circ_machineinfos_with_machine_num(circ, negotiated->machine_type,
                                            negotiated->machine_ctr);
  } else if (negotiated->command == CIRCPAD_COMMAND_START &&
             negotiated->response == CIRCPAD_RESPONSE_ERR) {
    // This can still happen due to consensus drift.. free the machines
    // and be sad
    if (free_circ_machineinfos_with_machine_num(circ, negotiated->machine_type,
                                            negotiated->machine_ctr)) {
      // Only fail if a machine was there and matched the error cell
      TO_ORIGIN_CIRCUIT(circ)->padding_negotiation_failed = 1;
      log_fn(LOG_PROTOCOL_WARN, LD_CIRC,
             "Middle node did not accept our padding request on circuit "
             "%u (%d)",
             TO_ORIGIN_CIRCUIT(circ)->global_identifier,
             circ->purpose);
    }
  }

  circpad_negotiated_free(negotiated);
  return 0;
}

/** Free memory allocated by this machine spec. */
STATIC void
machine_spec_free_(circpad_machine_spec_t *m)
{
  if (!m) return;

  tor_free(m->states);
  tor_free(m);
}

/** Free all memory allocated by the circuitpadding subsystem. */
void
circpad_free_all(void)
{
  if (origin_padding_machines) {
    SMARTLIST_FOREACH_BEGIN(origin_padding_machines,
                            circpad_machine_spec_t *, m) {
      machine_spec_free(m);
    } SMARTLIST_FOREACH_END(m);
    smartlist_free(origin_padding_machines);
  }
  if (relay_padding_machines) {
    SMARTLIST_FOREACH_BEGIN(relay_padding_machines,
                            circpad_machine_spec_t *, m) {
      machine_spec_free(m);
    } SMARTLIST_FOREACH_END(m);
    smartlist_free(relay_padding_machines);
  }
}

/* Serialization */
// TODO: Should we use keyword=value here? Are there helpers for that?
#if 0
static void
circpad_state_serialize(const circpad_state_t *state,
                        smartlist_t *chunks)
{
  smartlist_add_asprintf(chunks, " %u", state->histogram[0]);
  for (int i = 1; i < state->histogram_len; i++) {
    smartlist_add_asprintf(chunks, ",%u",
                           state->histogram[i]);
  }

  smartlist_add_asprintf(chunks, " 0x%x",
                         state->transition_cancel_events);

  for (int i = 0; i < CIRCPAD_NUM_STATES; i++) {
    smartlist_add_asprintf(chunks, ",0x%x",
                           state->transition_events[i]);
  }

  smartlist_add_asprintf(chunks, " %u %u",
                         state->use_rtt_estimate,
                         state->token_removal);
}

char *
circpad_machine_spec_to_string(const circpad_machine_spec_t *machine)
{
  smartlist_t *chunks = smartlist_new();
  char *out;
  (void)machine;

  circpad_state_serialize(&machine->start, chunks);
  circpad_state_serialize(&machine->gap, chunks);
  circpad_state_serialize(&machine->burst, chunks);

  out = smartlist_join_strings(chunks, "", 0, NULL);

  SMARTLIST_FOREACH(chunks, char *, cp, tor_free(cp));
  smartlist_free(chunks);
  return out;
}

// XXX: Writeme
const circpad_machine_spec_t *
circpad_string_to_machine(const char *str)
{
  (void)str;
  return NULL;
}

#endif /* 0 */
