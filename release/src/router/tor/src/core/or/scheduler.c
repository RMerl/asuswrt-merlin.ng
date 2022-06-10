/* Copyright (c) 2013-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

#include "core/or/or.h"
#include "app/config/config.h"

#include "lib/evloop/compat_libevent.h"
#define SCHEDULER_PRIVATE
#define SCHEDULER_KIST_PRIVATE
#include "core/or/scheduler.h"
#include "core/mainloop/mainloop.h"
#include "lib/buf/buffers.h"
#define CHANNEL_OBJECT_PRIVATE
#include "core/or/channeltls.h"

#include "core/or/or_connection_st.h"

/**
 * \file scheduler.c
 * \brief Channel scheduling system: decides which channels should send and
 * receive when.
 *
 * This module is the global/common parts of the scheduling system. This system
 * is what decides what channels get to send cells on their circuits and when.
 *
 * Terms:
 * - "Scheduling system": the collection of scheduler*.{h,c} files and their
 *   aggregate behavior.
 * - "Scheduler implementation": a scheduler_t. The scheduling system has one
 *   active scheduling implementation at a time.
 *
 * In this file you will find state that any scheduler implementation can have
 * access to as well as the functions the rest of Tor uses to interact with the
 * scheduling system.
 *
 * The earliest versions of Tor approximated a kind of round-robin system
 * among active connections, but only approximated it. It would only consider
 * one connection (roughly equal to a channel in today's terms) at a time, and
 * thus could only prioritize circuits against others on the same connection.
 *
 * Then in response to the KIST paper[0], Tor implemented a global
 * circuit scheduler. It was supposed to prioritize circuits across many
 * channels, but wasn't effective. It is preserved in scheduler_vanilla.c.
 *
 * [0]: https://www.robgjansen.com/publications/kist-sec2014.pdf
 *
 * Then we actually got around to implementing KIST for real. We decided to
 * modularize the scheduler so new ones can be implemented. You can find KIST
 * in scheduler_kist.c.
 *
 * Channels have one of four scheduling states based on whether or not they
 * have cells to send and whether or not they are able to send.
 *
 * <ol>
 * <li>
 *   Not open for writes, no cells to send.
 *     <ul><li> Not much to do here, and the channel will have scheduler_state
 *       == SCHED_CHAN_IDLE
 *     <li> Transitions from:
 *       <ul>
 *       <li>Open for writes/has cells by simultaneously draining all circuit
 *         queues and filling the output buffer.
 *       </ul>
 *     <li> Transitions to:
 *      <ul>
 *       <li> Not open for writes/has cells by arrival of cells on an attached
 *         circuit (this would be driven from append_cell_to_circuit_queue())
 *       <li> Open for writes/no cells by a channel type specific path;
 *         driven from connection_or_flushed_some() for channel_tls_t.
 *      </ul>
 *    </ul>
 *
 * <li> Open for writes, no cells to send
 *   <ul>
 *     <li>Not much here either; this will be the state an idle but open
 *       channel can be expected to settle in.  It will have scheduler_state
 *       == SCHED_CHAN_WAITING_FOR_CELLS
 *     <li> Transitions from:
 *       <ul>
 *       <li>Not open for writes/no cells by flushing some of the output
 *         buffer.
 *       <li>Open for writes/has cells by the scheduler moving cells from
 *         circuit queues to channel output queue, but not having enough
 *         to fill the output queue.
 *       </ul>
 *     <li> Transitions to:
 *       <ul>
 *        <li>Open for writes/has cells by arrival of new cells on an attached
 *         circuit, in append_cell_to_circuit_queue()
 *       </ul>
 *     </ul>
 *
 * <li>Not open for writes, cells to send
 *     <ul>
 *     <li>This is the state of a busy circuit limited by output bandwidth;
 *       cells have piled up in the circuit queues waiting to be relayed.
 *       The channel will have scheduler_state == SCHED_CHAN_WAITING_TO_WRITE.
 *     <li> Transitions from:
 *       <ul>
 *       <li>Not open for writes/no cells by arrival of cells on an attached
 *         circuit
 *       <li>Open for writes/has cells by filling an output buffer without
 *         draining all cells from attached circuits
 *       </ul>
 *    <li> Transitions to:
 *       <ul>
 *       <li>Opens for writes/has cells by draining some of the output buffer
 *         via the connection_or_flushed_some() path (for channel_tls_t).
 *       </ul>
 *    </ul>
 *
 * <li>Open for writes, cells to send
 *     <ul>
 *     <li>This connection is ready to relay some cells and waiting for
 *       the scheduler to choose it.  The channel will have scheduler_state ==
 *       SCHED_CHAN_PENDING.
 *     <li>Transitions from:
 *       <ul>
 *       <li>Not open for writes/has cells by the connection_or_flushed_some()
 *         path
 *       <li>Open for writes/no cells by the append_cell_to_circuit_queue()
 *         path
 *       </ul>
 *     <li> Transitions to:
 *       <ul>
 *        <li>Not open for writes/no cells by draining all circuit queues and
 *          simultaneously filling the output buffer.
 *        <li>Not open for writes/has cells by writing enough cells to fill the
 *         output buffer
 *        <li>Open for writes/no cells by draining all attached circuit queues
 *         without also filling the output buffer
 *       </ul>
 *    </ul>
 * </ol>
 *
 * Other event-driven parts of the code move channels between these scheduling
 * states by calling scheduler functions. The scheduling system builds up a
 * list of channels in the SCHED_CHAN_PENDING state that the scheduler
 * implementation should then use when it runs. Scheduling implementations need
 * to properly update channel states during their scheduler_t->run() function
 * as that is the only opportunity for channels to move from SCHED_CHAN_PENDING
 * to any other state.
 *
 * The remainder of this file is a small amount of state that any scheduler
 * implementation should have access to, and the functions the rest of Tor uses
 * to interact with the scheduling system.
 */

/*****************************************************************************
 * Scheduling system state
 *
 * State that can be accessed from any scheduler implementation (but not
 * outside the scheduling system)
 *****************************************************************************/

/** DOCDOC */
STATIC const scheduler_t *the_scheduler;

/**
 * We keep a list of channels that are pending - i.e, have cells to write
 * and can accept them to send. The enum scheduler_state in channel_t
 * is reserved for our use.
 *
 * Priority queue of channels that can write and have cells (pending work)
 */
STATIC smartlist_t *channels_pending = NULL;

/**
 * This event runs the scheduler from its callback, and is manually
 * activated whenever a channel enters open for writes/cells to send.
 */
STATIC struct mainloop_event_t *run_sched_ev = NULL;

static int have_logged_kist_suddenly_disabled = 0;

/*****************************************************************************
 * Scheduling system static function definitions
 *
 * Functions that can only be accessed from this file.
 *****************************************************************************/

/** Return a human readable string for the given scheduler type. */
static const char *
get_scheduler_type_string(scheduler_types_t type)
{
  switch (type) {
  case SCHEDULER_VANILLA:
    return "Vanilla";
  case SCHEDULER_KIST:
    return "KIST";
  case SCHEDULER_KIST_LITE:
    return "KISTLite";
  case SCHEDULER_NONE:
    FALLTHROUGH;
  default:
    tor_assert_unreached();
    return "(N/A)";
  }
}

/**
 * Scheduler event callback; this should get triggered once per event loop
 * if any scheduling work was created during the event loop.
 */
static void
scheduler_evt_callback(mainloop_event_t *event, void *arg)
{
  (void) event;
  (void) arg;

  log_debug(LD_SCHED, "Scheduler event callback called");

  /* Run the scheduler. This is a mandatory function. */

  /* We might as well assert on this. If this function doesn't exist, no cells
   * are getting scheduled. Things are very broken. scheduler_t says the run()
   * function is mandatory. */
  tor_assert(the_scheduler->run);
  the_scheduler->run();

  /* Schedule itself back in if it has more work. */

  /* Again, might as well assert on this mandatory scheduler_t function. If it
   * doesn't exist, there's no way to tell libevent to run the scheduler again
   * in the future. */
  tor_assert(the_scheduler->schedule);
  the_scheduler->schedule();
}

/** Using the global options, select the scheduler we should be using. */
static void
select_scheduler(void)
{
  scheduler_t *new_scheduler = NULL;

#ifdef TOR_UNIT_TESTS
  /* This is hella annoying to set in the options for every test that passes
   * through the scheduler and there are many so if we don't explicitly have
   * a list of types set, just put the vanilla one. */
  if (get_options()->SchedulerTypes_ == NULL) {
    the_scheduler = get_vanilla_scheduler();
    return;
  }
#endif /* defined(TOR_UNIT_TESTS) */

  /* This list is ordered that is first entry has the first priority. Thus, as
   * soon as we find a scheduler type that we can use, we use it and stop. */
  SMARTLIST_FOREACH_BEGIN(get_options()->SchedulerTypes_, int *, type) {
    switch (*type) {
    case SCHEDULER_VANILLA:
      new_scheduler = get_vanilla_scheduler();
      goto end;
    case SCHEDULER_KIST:
      if (!scheduler_can_use_kist()) {
#ifdef HAVE_KIST_SUPPORT
        if (!have_logged_kist_suddenly_disabled) {
          /* We should only log this once in most cases. If it was the kernel
           * losing support for kist that caused scheduler_can_use_kist() to
           * return false, then this flag makes sure we only log this message
           * once. If it was the consensus that switched from "yes use kist"
           * to "no don't use kist", then we still set the flag so we log
           * once, but we unset the flag elsewhere if we ever can_use_kist()
           * again.
           */
          have_logged_kist_suddenly_disabled = 1;
          log_notice(LD_SCHED, "Scheduler type KIST has been disabled by "
                               "the consensus or no kernel support.");
        }
#else /* !defined(HAVE_KIST_SUPPORT) */
        log_info(LD_SCHED, "Scheduler type KIST not built in");
#endif /* defined(HAVE_KIST_SUPPORT) */
        continue;
      }
      /* This flag will only get set in one of two cases:
       * 1 - the kernel lost support for kist. In that case, we don't expect to
       *     ever end up here
       * 2 - the consensus went from "yes use kist" to "no don't use kist".
       * We might end up here if the consensus changes back to "yes", in which
       * case we might want to warn the user again if it goes back to "no"
       * yet again. Thus we unset the flag */
      have_logged_kist_suddenly_disabled = 0;
      new_scheduler = get_kist_scheduler();
      scheduler_kist_set_full_mode();
      goto end;
    case SCHEDULER_KIST_LITE:
      new_scheduler = get_kist_scheduler();
      scheduler_kist_set_lite_mode();
      goto end;
    case SCHEDULER_NONE:
      FALLTHROUGH;
    default:
      /* Our option validation should have caught this. */
      tor_assert_unreached();
    }
  } SMARTLIST_FOREACH_END(type);

 end:
  if (new_scheduler == NULL) {
    log_err(LD_SCHED, "Tor was unable to select a scheduler type. Please "
                      "make sure Schedulers is correctly configured with "
                      "what Tor does support.");
    /* We weren't able to choose a scheduler which means that none of the ones
     * set in Schedulers are supported or usable. We will respect the user
     * wishes of using what it has been configured and don't do a sneaky
     * fallback. Because this can be changed at runtime, we have to stop tor
     * right now. */
    exit(1); // XXXX bad exit
  }

  /* Set the chosen scheduler. */
  the_scheduler = new_scheduler;
}

/**
 * Helper function called from a few different places. It changes the
 * scheduler implementation, if necessary. And if it did, it then tells the
 * old one to free its state and the new one to initialize.
 */
static void
set_scheduler(void)
{
  const scheduler_t *old_scheduler = the_scheduler;
  scheduler_types_t old_scheduler_type = SCHEDULER_NONE;

  /* We keep track of the type in order to log only if the type switched. We
   * can't just use the scheduler pointers because KIST and KISTLite share the
   * same object. */
  if (the_scheduler) {
    old_scheduler_type = the_scheduler->type;
  }

  /* From the options, select the scheduler type to set. */
  select_scheduler();
  tor_assert(the_scheduler);

  /* We look at the pointer difference in case the old sched and new sched
   * share the same scheduler object, as is the case with KIST and KISTLite. */
  if (old_scheduler != the_scheduler) {
    /* Allow the old scheduler to clean up, if needed. */
    if (old_scheduler && old_scheduler->free_all) {
      old_scheduler->free_all();
    }

    /* Initialize the new scheduler. */
    if (the_scheduler->init) {
      the_scheduler->init();
    }
  }

  /* Finally we notice log if we switched schedulers. We use the type in case
   * two schedulers share a scheduler object. */
  if (old_scheduler_type != the_scheduler->type) {
    log_info(LD_CONFIG, "Scheduler type %s has been enabled.",
             get_scheduler_type_string(the_scheduler->type));
  }
}

/*****************************************************************************
 * Scheduling system private function definitions
 *
 * Functions that can only be accessed from scheduler*.c
 *****************************************************************************/

/** Returns human readable string for the given channel scheduler state. */
const char *
get_scheduler_state_string(int scheduler_state)
{
  switch (scheduler_state) {
  case SCHED_CHAN_IDLE:
    return "IDLE";
  case SCHED_CHAN_WAITING_FOR_CELLS:
    return "WAITING_FOR_CELLS";
  case SCHED_CHAN_WAITING_TO_WRITE:
    return "WAITING_TO_WRITE";
  case SCHED_CHAN_PENDING:
    return "PENDING";
  default:
    return "(invalid)";
  }
}

/** Helper that logs channel scheduler_state changes. Use this instead of
 * setting scheduler_state directly. */
void
scheduler_set_channel_state(channel_t *chan, int new_state)
{
  log_debug(LD_SCHED, "chan %" PRIu64 " changed from scheduler state %s to %s",
      chan->global_identifier,
      get_scheduler_state_string(chan->scheduler_state),
      get_scheduler_state_string(new_state));
  chan->scheduler_state = new_state;
}

/** Return the pending channel list. */
smartlist_t *
get_channels_pending(void)
{
  return channels_pending;
}

/** Comparison function to use when sorting pending channels. */
MOCK_IMPL(int,
scheduler_compare_channels, (const void *c1_v, const void *c2_v))
{
  const channel_t *c1 = NULL, *c2 = NULL;
  /* These are a workaround for -Wbad-function-cast throwing a fit */
  const circuitmux_policy_t *p1, *p2;
  uintptr_t p1_i, p2_i;

  tor_assert(c1_v);
  tor_assert(c2_v);

  c1 = (const channel_t *)(c1_v);
  c2 = (const channel_t *)(c2_v);

  if (c1 != c2) {
    if (circuitmux_get_policy(c1->cmux) ==
        circuitmux_get_policy(c2->cmux)) {
      /* Same cmux policy, so use the mux comparison */
      return circuitmux_compare_muxes(c1->cmux, c2->cmux);
    } else {
      /*
       * Different policies; not important to get this edge case perfect
       * because the current code never actually gives different channels
       * different cmux policies anyway.  Just use this arbitrary but
       * definite choice.
       */
      p1 = circuitmux_get_policy(c1->cmux);
      p2 = circuitmux_get_policy(c2->cmux);
      p1_i = (uintptr_t)p1;
      p2_i = (uintptr_t)p2;

      return (p1_i < p2_i) ? -1 : 1;
    }
  } else {
    /* c1 == c2, so always equal */
    return 0;
  }
}

/*****************************************************************************
 * Scheduling system global functions
 *
 * Functions that can be accessed from anywhere in Tor.
 *****************************************************************************/

/**
 * This is how the scheduling system is notified of Tor's configuration
 * changing. For example: a SIGHUP was issued.
 */
void
scheduler_conf_changed(void)
{
  /* Let the scheduler decide what it should do. */
  set_scheduler();

  /* Then tell the (possibly new) scheduler that we have new options. */
  if (the_scheduler->on_new_options) {
    the_scheduler->on_new_options();
  }
}

/**
 * Whenever we get a new consensus, this function is called.
 */
void
scheduler_notify_networkstatus_changed(void)
{
  /* Maybe the consensus param made us change the scheduler. */
  set_scheduler();

  /* Then tell the (possibly new) scheduler that we have a new consensus */
  if (the_scheduler->on_new_consensus) {
    the_scheduler->on_new_consensus();
  }
}

/**
 * Free everything scheduling-related from main.c. Note this is only called
 * when Tor is shutting down, while scheduler_t->free_all() is called both when
 * Tor is shutting down and when we are switching schedulers.
 */
void
scheduler_free_all(void)
{
  log_debug(LD_SCHED, "Shutting down scheduler");

  if (run_sched_ev) {
    mainloop_event_free(run_sched_ev);
    run_sched_ev = NULL;
  }

  if (channels_pending) {
    /* We don't have ownership of the objects in this list. */
    smartlist_free(channels_pending);
    channels_pending = NULL;
  }

  if (the_scheduler && the_scheduler->free_all) {
    the_scheduler->free_all();
  }
  the_scheduler = NULL;
}

/** Mark a channel as no longer ready to accept writes.
  *
  * Possible state changes:
  *  - SCHED_CHAN_PENDING -> SCHED_CHAN_WAITING_TO_WRITE
  *  - SCHED_CHAN_WAITING_FOR_CELLS -> SCHED_CHAN_IDLE
  */
MOCK_IMPL(void,
scheduler_channel_doesnt_want_writes,(channel_t *chan))
{
  IF_BUG_ONCE(!chan) {
    return;
  }
  IF_BUG_ONCE(!channels_pending) {
    return;
  }

  if (chan->scheduler_state == SCHED_CHAN_PENDING) {
    /*
     * It has cells but no longer can write, so it becomes
     * SCHED_CHAN_WAITING_TO_WRITE. It's in channels_pending, so we
     * should remove it from the list.
     */
    smartlist_pqueue_remove(channels_pending,
                            scheduler_compare_channels,
                            offsetof(channel_t, sched_heap_idx),
                            chan);
    scheduler_set_channel_state(chan, SCHED_CHAN_WAITING_TO_WRITE);
  } else if (chan->scheduler_state == SCHED_CHAN_WAITING_FOR_CELLS) {
    /*
     * It does not have cells and no longer can write, so it becomes
     * SCHED_CHAN_IDLE.
     */
    scheduler_set_channel_state(chan, SCHED_CHAN_IDLE);
  }
}

/** Mark a channel as having waiting cells.
  *
  * Possible state changes:
  *  - SCHED_CHAN_WAITING_FOR_CELLS -> SCHED_CHAN_PENDING
  *  - SCHED_CHAN_IDLE -> SCHED_CHAN_WAITING_TO_WRITE
  */
MOCK_IMPL(void,
scheduler_channel_has_waiting_cells,(channel_t *chan))
{
  IF_BUG_ONCE(!chan) {
    return;
  }
  IF_BUG_ONCE(!channels_pending) {
    return;
  }

  if (chan->scheduler_state == SCHED_CHAN_WAITING_FOR_CELLS) {
    /*
     * It is able to write and now has cells, so it becomes
     * SCHED_CHAN_PENDING. It must be added to the channels_pending
     * list.
     */
    scheduler_set_channel_state(chan, SCHED_CHAN_PENDING);
    if (!SCHED_BUG(chan->sched_heap_idx != -1, chan)) {
      smartlist_pqueue_add(channels_pending,
                           scheduler_compare_channels,
                           offsetof(channel_t, sched_heap_idx),
                           chan);
    }
    /* If we made a channel pending, we potentially have scheduling work to
     * do. */
    the_scheduler->schedule();
  } else if (chan->scheduler_state == SCHED_CHAN_IDLE) {
    /*
     * It is not able to write but now has cells, so it becomes
     * SCHED_CHAN_WAITING_TO_WRITE.
     */
    scheduler_set_channel_state(chan, SCHED_CHAN_WAITING_TO_WRITE);
  }
}

/** Add the scheduler event to the set of pending events with next_run being
 * the longest time libevent should wait before triggering the event. */
void
scheduler_ev_add(const struct timeval *next_run)
{
  tor_assert(run_sched_ev);
  tor_assert(next_run);
  if (BUG(mainloop_event_schedule(run_sched_ev, next_run) < 0)) {
    log_warn(LD_SCHED, "Adding to libevent failed. Next run time was set to: "
                       "%ld.%06ld", next_run->tv_sec, (long)next_run->tv_usec);
    return;
  }
}

/** Make the scheduler event active with the given flags. */
void
scheduler_ev_active(void)
{
  tor_assert(run_sched_ev);
  mainloop_event_activate(run_sched_ev);
}

/*
 * Initialize everything scheduling-related from config.c. Note this is only
 * called when Tor is starting up, while scheduler_t->init() is called both
 * when Tor is starting up and when we are switching schedulers.
 */
void
scheduler_init(void)
{
  log_debug(LD_SCHED, "Initting scheduler");

  // Two '!' because we really do want to check if the pointer is non-NULL
  IF_BUG_ONCE(!!run_sched_ev) {
    log_warn(LD_SCHED, "We should not already have a libevent scheduler event."
             "I'll clean the old one up, but this is odd.");
    mainloop_event_free(run_sched_ev);
    run_sched_ev = NULL;
  }
  run_sched_ev = mainloop_event_new(scheduler_evt_callback, NULL);
  channels_pending = smartlist_new();

  set_scheduler();
}

/*
 * If a channel is going away, this is how the scheduling system is informed
 * so it can do any freeing necessary. This ultimately calls
 * scheduler_t->on_channel_free() so the current scheduler can release any
 * state specific to this channel.
 */
MOCK_IMPL(void,
scheduler_release_channel,(channel_t *chan))
{
  IF_BUG_ONCE(!chan) {
    return;
  }
  IF_BUG_ONCE(!channels_pending) {
    return;
  }

  /* Try to remove the channel from the pending list regardless of its
   * scheduler state. We can release a channel in many places in the tor code
   * so we can't rely on the channel state (PENDING) to remove it from the
   * list.
   *
   * For instance, the channel can change state from OPEN to CLOSING while
   * being handled in the scheduler loop leading to the channel being in
   * PENDING state but not in the pending list. Furthermore, we release the
   * channel when it changes state to close and a second time when we free it.
   * Not ideal at all but for now that is the way it is. */
  if (chan->sched_heap_idx != -1) {
    smartlist_pqueue_remove(channels_pending,
                            scheduler_compare_channels,
                            offsetof(channel_t, sched_heap_idx),
                            chan);
  }

  if (the_scheduler->on_channel_free) {
    the_scheduler->on_channel_free(chan);
  }
  scheduler_set_channel_state(chan, SCHED_CHAN_IDLE);
}

/** Mark a channel as ready to accept writes.
  * Possible state changes:
  *
  *  - SCHED_CHAN_WAITING_TO_WRITE -> SCHED_CHAN_PENDING
  *  - SCHED_CHAN_IDLE -> SCHED_CHAN_WAITING_FOR_CELLS
  */
void
scheduler_channel_wants_writes(channel_t *chan)
{
  IF_BUG_ONCE(!chan) {
    return;
  }
  IF_BUG_ONCE(!channels_pending) {
    return;
  }

  if (chan->scheduler_state == SCHED_CHAN_WAITING_TO_WRITE) {
    /*
     * It has cells and can now write, so it becomes
     * SCHED_CHAN_PENDING. It must be added to the channels_pending
     * list.
     */
    scheduler_set_channel_state(chan, SCHED_CHAN_PENDING);
    if (!SCHED_BUG(chan->sched_heap_idx != -1, chan)) {
      smartlist_pqueue_add(channels_pending,
                           scheduler_compare_channels,
                           offsetof(channel_t, sched_heap_idx),
                           chan);
    }
    /* We just made a channel pending, we have scheduling work to do. */
    the_scheduler->schedule();
  } else if (chan->scheduler_state == SCHED_CHAN_IDLE) {
    /*
     * It does not have cells but can now write, so it becomes
     * SCHED_CHAN_WAITING_FOR_CELLS.
     */
    scheduler_set_channel_state(chan, SCHED_CHAN_WAITING_FOR_CELLS);
  }
}

/* Log warn the given channel and extra scheduler context as well. This is
 * used by SCHED_BUG() in order to be able to extract as much information as
 * we can when we hit a bug. Channel chan can be NULL. */
void
scheduler_bug_occurred(const channel_t *chan)
{
  char buf[128];

  if (chan != NULL) {
    const size_t outbuf_len =
      buf_datalen(TO_CONN(CONST_BASE_CHAN_TO_TLS(chan)->conn)->outbuf);
    tor_snprintf(buf, sizeof(buf),
                 "Channel %" PRIu64 " in state %s and scheduler state %s."
                 " Num cells on cmux: %d. Connection outbuf len: %lu.",
                 chan->global_identifier,
                 channel_state_to_string(chan->state),
                 get_scheduler_state_string(chan->scheduler_state),
                 circuitmux_num_cells(chan->cmux),
                 (unsigned long)outbuf_len);
  }

  {
    char *msg;
    /* Rate limit every 60 seconds. If we start seeing this every 60 sec, we
     * know something is stuck/wrong. It *should* be loud but not too much. */
    static ratelim_t rlimit = RATELIM_INIT(60);
    if ((msg = rate_limit_log(&rlimit, approx_time()))) {
      log_warn(LD_BUG, "%s Num pending channels: %d. "
                       "Channel in pending list: %s.%s",
               (chan != NULL) ? buf : "No channel in bug context.",
               smartlist_len(channels_pending),
               (smartlist_pos(channels_pending, chan) == -1) ? "no" : "yes",
               msg);
      tor_free(msg);
    }
  }
}

#ifdef TOR_UNIT_TESTS

/*
 * Notify scheduler that a channel's queue position may have changed.
 */
void
scheduler_touch_channel(channel_t *chan)
{
  IF_BUG_ONCE(!chan) {
    return;
  }

  if (chan->scheduler_state == SCHED_CHAN_PENDING) {
    /* Remove and re-add it */
    smartlist_pqueue_remove(channels_pending,
                            scheduler_compare_channels,
                            offsetof(channel_t, sched_heap_idx),
                            chan);
    smartlist_pqueue_add(channels_pending,
                         scheduler_compare_channels,
                         offsetof(channel_t, sched_heap_idx),
                         chan);
  }
  /* else no-op, since it isn't in the queue */
}

#endif /* defined(TOR_UNIT_TESTS) */
