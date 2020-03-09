/* Copyright (c) 2001 Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2019, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file onion_queue.c
 * \brief Functions to queue create cells for processing.
 *
 * Relays invoke these functions when they receive a CREATE or EXTEND
 * cell in command.c or relay.c, in order to queue the pending request.
 * They also invoke them from cpuworker.c, which handles dispatching
 * onionskin requests to different worker threads.
 *
 * <br>
 *
 * This module also handles:
 *  <ul>
 *  <li> Queueing incoming onionskins on the relay side before passing
 *      them to worker threads.
 *   <li>Expiring onionskins on the relay side if they have waited for
 *     too long.
 * </ul>
 **/

#include "core/or/or.h"

#include "feature/relay/onion_queue.h"

#include "app/config/config.h"
#include "core/mainloop/cpuworker.h"
#include "core/or/circuitlist.h"
#include "core/or/onion.h"
#include "feature/nodelist/networkstatus.h"

#include "core/or/or_circuit_st.h"

/** Type for a linked list of circuits that are waiting for a free CPU worker
 * to process a waiting onion handshake. */
typedef struct onion_queue_t {
  TOR_TAILQ_ENTRY(onion_queue_t) next;
  or_circuit_t *circ;
  uint16_t handshake_type;
  create_cell_t *onionskin;
  time_t when_added;
} onion_queue_t;

/** 5 seconds on the onion queue til we just send back a destroy */
#define ONIONQUEUE_WAIT_CUTOFF 5

/** Array of queues of circuits waiting for CPU workers. An element is NULL
 * if that queue is empty.*/
static TOR_TAILQ_HEAD(onion_queue_head_t, onion_queue_t)
              ol_list[MAX_ONION_HANDSHAKE_TYPE+1] =
{ TOR_TAILQ_HEAD_INITIALIZER(ol_list[0]), /* tap */
  TOR_TAILQ_HEAD_INITIALIZER(ol_list[1]), /* fast */
  TOR_TAILQ_HEAD_INITIALIZER(ol_list[2]), /* ntor */
};

/** Number of entries of each type currently in each element of ol_list[]. */
static int ol_entries[MAX_ONION_HANDSHAKE_TYPE+1];

static int num_ntors_per_tap(void);
static void onion_queue_entry_remove(onion_queue_t *victim);

/* XXXX Check lengths vs MAX_ONIONSKIN_{CHALLENGE,REPLY}_LEN.
 *
 * (By which I think I meant, "make sure that no
 * X_ONIONSKIN_CHALLENGE/REPLY_LEN is greater than
 * MAX_ONIONSKIN_CHALLENGE/REPLY_LEN."  Also, make sure that we can pass
 * over-large values via EXTEND2/EXTENDED2, for future-compatibility.*/

/** Return true iff we have room to queue another onionskin of type
 * <b>type</b>. */
static int
have_room_for_onionskin(uint16_t type)
{
  const or_options_t *options = get_options();
  int num_cpus;
  uint64_t tap_usec, ntor_usec;
  uint64_t ntor_during_tap_usec, tap_during_ntor_usec;

  /* If we've got fewer than 50 entries, we always have room for one more. */
  if (ol_entries[type] < 50)
    return 1;
  num_cpus = get_num_cpus(options);
  /* Compute how many microseconds we'd expect to need to clear all
   * onionskins in various combinations of the queues. */

  /* How long would it take to process all the TAP cells in the queue? */
  tap_usec  = estimated_usec_for_onionskins(
                                    ol_entries[ONION_HANDSHAKE_TYPE_TAP],
                                    ONION_HANDSHAKE_TYPE_TAP) / num_cpus;

  /* How long would it take to process all the NTor cells in the queue? */
  ntor_usec = estimated_usec_for_onionskins(
                                    ol_entries[ONION_HANDSHAKE_TYPE_NTOR],
                                    ONION_HANDSHAKE_TYPE_NTOR) / num_cpus;

  /* How long would it take to process the tap cells that we expect to
   * process while draining the ntor queue? */
  tap_during_ntor_usec  = estimated_usec_for_onionskins(
    MIN(ol_entries[ONION_HANDSHAKE_TYPE_TAP],
        ol_entries[ONION_HANDSHAKE_TYPE_NTOR] / num_ntors_per_tap()),
                                    ONION_HANDSHAKE_TYPE_TAP) / num_cpus;

  /* How long would it take to process the ntor cells that we expect to
   * process while draining the tap queue? */
  ntor_during_tap_usec  = estimated_usec_for_onionskins(
    MIN(ol_entries[ONION_HANDSHAKE_TYPE_NTOR],
        ol_entries[ONION_HANDSHAKE_TYPE_TAP] * num_ntors_per_tap()),
                                    ONION_HANDSHAKE_TYPE_NTOR) / num_cpus;

  /* See whether that exceeds MaxOnionQueueDelay. If so, we can't queue
   * this. */
  if (type == ONION_HANDSHAKE_TYPE_NTOR &&
      (ntor_usec + tap_during_ntor_usec) / 1000 >
       (uint64_t)options->MaxOnionQueueDelay)
    return 0;

  if (type == ONION_HANDSHAKE_TYPE_TAP &&
      (tap_usec + ntor_during_tap_usec) / 1000 >
       (uint64_t)options->MaxOnionQueueDelay)
    return 0;

  /* If we support the ntor handshake, then don't let TAP handshakes use
   * more than 2/3 of the space on the queue. */
  if (type == ONION_HANDSHAKE_TYPE_TAP &&
      tap_usec / 1000 > (uint64_t)options->MaxOnionQueueDelay * 2 / 3)
    return 0;

  return 1;
}

/** Add <b>circ</b> to the end of ol_list and return 0, except
 * if ol_list is too long, in which case do nothing and return -1.
 */
int
onion_pending_add(or_circuit_t *circ, create_cell_t *onionskin)
{
  onion_queue_t *tmp;
  time_t now = time(NULL);

  if (onionskin->handshake_type > MAX_ONION_HANDSHAKE_TYPE) {
    /* LCOV_EXCL_START
     * We should have rejected this far before this point */
    log_warn(LD_BUG, "Handshake %d out of range! Dropping.",
             onionskin->handshake_type);
    return -1;
    /* LCOV_EXCL_STOP */
  }

  tmp = tor_malloc_zero(sizeof(onion_queue_t));
  tmp->circ = circ;
  tmp->handshake_type = onionskin->handshake_type;
  tmp->onionskin = onionskin;
  tmp->when_added = now;

  if (!have_room_for_onionskin(onionskin->handshake_type)) {
#define WARN_TOO_MANY_CIRC_CREATIONS_INTERVAL (60)
    static ratelim_t last_warned =
      RATELIM_INIT(WARN_TOO_MANY_CIRC_CREATIONS_INTERVAL);
    char *m;
    if (onionskin->handshake_type == ONION_HANDSHAKE_TYPE_NTOR &&
        (m = rate_limit_log(&last_warned, approx_time()))) {
      log_warn(LD_GENERAL,
               "Your computer is too slow to handle this many circuit "
               "creation requests! Please consider using the "
               "MaxAdvertisedBandwidth config option or choosing a more "
               "restricted exit policy.%s",m);
      tor_free(m);
    }
    tor_free(tmp);
    return -1;
  }

  ++ol_entries[onionskin->handshake_type];
  log_info(LD_OR, "New create (%s). Queues now ntor=%d and tap=%d.",
    onionskin->handshake_type == ONION_HANDSHAKE_TYPE_NTOR ? "ntor" : "tap",
    ol_entries[ONION_HANDSHAKE_TYPE_NTOR],
    ol_entries[ONION_HANDSHAKE_TYPE_TAP]);

  circ->onionqueue_entry = tmp;
  TOR_TAILQ_INSERT_TAIL(&ol_list[onionskin->handshake_type], tmp, next);

  /* cull elderly requests. */
  while (1) {
    onion_queue_t *head = TOR_TAILQ_FIRST(&ol_list[onionskin->handshake_type]);
    if (now - head->when_added < (time_t)ONIONQUEUE_WAIT_CUTOFF)
      break;

    circ = head->circ;
    circ->onionqueue_entry = NULL;
    onion_queue_entry_remove(head);
    log_info(LD_CIRC,
             "Circuit create request is too old; canceling due to overload.");
    if (! TO_CIRCUIT(circ)->marked_for_close) {
      circuit_mark_for_close(TO_CIRCUIT(circ), END_CIRC_REASON_RESOURCELIMIT);
    }
  }
  return 0;
}

/** Return a fairness parameter, to prefer processing NTOR style
 * handshakes but still slowly drain the TAP queue so we don't starve
 * it entirely. */
static int
num_ntors_per_tap(void)
{
#define DEFAULT_NUM_NTORS_PER_TAP 10
#define MIN_NUM_NTORS_PER_TAP 1
#define MAX_NUM_NTORS_PER_TAP 100000

  int result = networkstatus_get_param(NULL, "NumNTorsPerTAP",
                                       DEFAULT_NUM_NTORS_PER_TAP,
                                       MIN_NUM_NTORS_PER_TAP,
                                       MAX_NUM_NTORS_PER_TAP);
  tor_assert(result > 0);
  return result;
}

/** Choose which onion queue we'll pull from next. If one is empty choose
 * the other; if they both have elements, load balance across them but
 * favoring NTOR. */
static uint16_t
decide_next_handshake_type(void)
{
  /* The number of times we've chosen ntor lately when both were available. */
  static int recently_chosen_ntors = 0;

  if (!ol_entries[ONION_HANDSHAKE_TYPE_NTOR])
    return ONION_HANDSHAKE_TYPE_TAP; /* no ntors? try tap */

  if (!ol_entries[ONION_HANDSHAKE_TYPE_TAP]) {

    /* Nick wants us to prioritize new tap requests when there aren't
     * any in the queue and we've processed k ntor cells since the last
     * tap cell. This strategy is maybe a good idea, since it starves tap
     * less in the case where tap is rare, or maybe a poor idea, since it
     * makes the new tap cell unfairly jump in front of ntor cells that
     * got here first. In any case this edge case will only become relevant
     * once tap is rare. We should reevaluate whether we like this decision
     * once tap gets more rare. */
    if (ol_entries[ONION_HANDSHAKE_TYPE_NTOR] &&
        recently_chosen_ntors <= num_ntors_per_tap())
      ++recently_chosen_ntors;

    return ONION_HANDSHAKE_TYPE_NTOR; /* no taps? try ntor */
  }

  /* They both have something queued. Pick ntor if we haven't done that
   * too much lately. */
  if (++recently_chosen_ntors <= num_ntors_per_tap()) {
    return ONION_HANDSHAKE_TYPE_NTOR;
  }

  /* Else, it's time to let tap have its turn. */
  recently_chosen_ntors = 0;
  return ONION_HANDSHAKE_TYPE_TAP;
}

/** Remove the highest priority item from ol_list[] and return it, or
 * return NULL if the lists are empty.
 */
or_circuit_t *
onion_next_task(create_cell_t **onionskin_out)
{
  or_circuit_t *circ;
  uint16_t handshake_to_choose = decide_next_handshake_type();
  onion_queue_t *head = TOR_TAILQ_FIRST(&ol_list[handshake_to_choose]);

  if (!head)
    return NULL; /* no onions pending, we're done */

  tor_assert(head->circ);
  tor_assert(head->handshake_type <= MAX_ONION_HANDSHAKE_TYPE);
//  tor_assert(head->circ->p_chan); /* make sure it's still valid */
/* XXX I only commented out the above line to make the unit tests
 * more manageable. That's probably not good long-term. -RD */
  circ = head->circ;
  if (head->onionskin)
    --ol_entries[head->handshake_type];
  log_info(LD_OR, "Processing create (%s). Queues now ntor=%d and tap=%d.",
    head->handshake_type == ONION_HANDSHAKE_TYPE_NTOR ? "ntor" : "tap",
    ol_entries[ONION_HANDSHAKE_TYPE_NTOR],
    ol_entries[ONION_HANDSHAKE_TYPE_TAP]);

  *onionskin_out = head->onionskin;
  head->onionskin = NULL; /* prevent free. */
  circ->onionqueue_entry = NULL;
  onion_queue_entry_remove(head);
  return circ;
}

/** Return the number of <b>handshake_type</b>-style create requests pending.
 */
int
onion_num_pending(uint16_t handshake_type)
{
  return ol_entries[handshake_type];
}

/** Go through ol_list, find the onion_queue_t element which points to
 * circ, remove and free that element. Leave circ itself alone.
 */
void
onion_pending_remove(or_circuit_t *circ)
{
  onion_queue_t *victim;

  if (!circ)
    return;

  victim = circ->onionqueue_entry;
  if (victim)
    onion_queue_entry_remove(victim);

  cpuworker_cancel_circ_handshake(circ);
}

/** Remove a queue entry <b>victim</b> from the queue, unlinking it from
 * its circuit and freeing it and any structures it owns.*/
static void
onion_queue_entry_remove(onion_queue_t *victim)
{
  if (victim->handshake_type > MAX_ONION_HANDSHAKE_TYPE) {
    /* LCOV_EXCL_START
     * We should have rejected this far before this point */
    log_warn(LD_BUG, "Handshake %d out of range! Dropping.",
             victim->handshake_type);
    /* XXX leaks */
    return;
    /* LCOV_EXCL_STOP */
  }

  TOR_TAILQ_REMOVE(&ol_list[victim->handshake_type], victim, next);

  if (victim->circ)
    victim->circ->onionqueue_entry = NULL;

  if (victim->onionskin)
    --ol_entries[victim->handshake_type];

  tor_free(victim->onionskin);
  tor_free(victim);
}

/** Remove all circuits from the pending list.  Called from tor_free_all. */
void
clear_pending_onions(void)
{
  onion_queue_t *victim, *next;
  int i;
  for (i=0; i<=MAX_ONION_HANDSHAKE_TYPE; i++) {
    for (victim = TOR_TAILQ_FIRST(&ol_list[i]); victim; victim = next) {
      next = TOR_TAILQ_NEXT(victim,next);
      onion_queue_entry_remove(victim);
    }
    tor_assert(TOR_TAILQ_EMPTY(&ol_list[i]));
  }
  memset(ol_entries, 0, sizeof(ol_entries));
}
