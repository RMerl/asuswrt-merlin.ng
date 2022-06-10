/* Copyright (c) 2017-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * @file scheduler_vanilla.c
 * @brief "Vanilla" (pre-KIST) cell scheduler code.
 **/

#include "core/or/or.h"
#include "app/config/config.h"
#define CHANNEL_OBJECT_PRIVATE
#include "core/or/channel.h"
#define SCHEDULER_PRIVATE
#include "core/or/scheduler.h"

/*****************************************************************************
 * Other internal data
 *****************************************************************************/

/* Maximum cells to flush in a single call to channel_flush_some_cells(); */
#define MAX_FLUSH_CELLS 1000

/*****************************************************************************
 * Externally called function implementations
 *****************************************************************************/

/* Return true iff the scheduler has work to perform. */
static int
have_work(void)
{
  smartlist_t *cp = get_channels_pending();
  IF_BUG_ONCE(!cp) {
    return 0; // channels_pending doesn't exist so... no work?
  }
  return smartlist_len(cp) > 0;
}

/** Re-trigger the scheduler in a way safe to use from the callback */

static void
vanilla_scheduler_schedule(void)
{
  if (!have_work()) {
    return;
  }

  /* Activate our event so it can process channels. */
  scheduler_ev_active();
}

static void
vanilla_scheduler_run(void)
{
  int n_cells, n_chans_before, n_chans_after;
  ssize_t flushed, flushed_this_time;
  smartlist_t *cp = get_channels_pending();
  smartlist_t *to_readd = NULL;
  channel_t *chan = NULL;

  log_debug(LD_SCHED, "We have a chance to run the scheduler");

  n_chans_before = smartlist_len(cp);

  while (smartlist_len(cp) > 0) {
    /* Pop off a channel */
    chan = smartlist_pqueue_pop(cp,
                                scheduler_compare_channels,
                                offsetof(channel_t, sched_heap_idx));
    IF_BUG_ONCE(!chan) {
      /* Some-freaking-how a NULL got into the channels_pending. That should
       * never happen, but it should be harmless to ignore it and keep looping.
       */
      continue;
    }

    /* Figure out how many cells we can write */
    n_cells = channel_num_cells_writeable(chan);
    if (n_cells > 0) {
      log_debug(LD_SCHED,
                "Scheduler saw pending channel %"PRIu64 " at %p with "
                "%d cells writeable",
                (chan->global_identifier), chan, n_cells);

      flushed = 0;
      while (flushed < n_cells) {
        flushed_this_time =
          channel_flush_some_cells(chan,
                        MIN(MAX_FLUSH_CELLS, (size_t) n_cells - flushed));
        if (flushed_this_time <= 0) break;
        flushed += flushed_this_time;
      }

      if (flushed < n_cells) {
        /* We ran out of cells to flush */
        scheduler_set_channel_state(chan, SCHED_CHAN_WAITING_FOR_CELLS);
      } else {
        /* The channel may still have some cells */
        if (channel_more_to_flush(chan)) {
        /* The channel goes to either pending or waiting_to_write */
          if (channel_num_cells_writeable(chan) > 0) {
            /* Add it back to pending later */
            if (!to_readd) to_readd = smartlist_new();
            smartlist_add(to_readd, chan);
            log_debug(LD_SCHED,
                      "Channel %"PRIu64 " at %p "
                      "is still pending",
                      (chan->global_identifier),
                      chan);
          } else {
            /* It's waiting to be able to write more */
            scheduler_set_channel_state(chan, SCHED_CHAN_WAITING_TO_WRITE);
          }
        } else {
          /* No cells left; it can go to idle or waiting_for_cells */
          if (channel_num_cells_writeable(chan) > 0) {
            /*
             * It can still accept writes, so it goes to
             * waiting_for_cells
             */
            scheduler_set_channel_state(chan, SCHED_CHAN_WAITING_FOR_CELLS);
          } else {
            /*
             * We exactly filled up the output queue with all available
             * cells; go to idle.
             */
            scheduler_set_channel_state(chan, SCHED_CHAN_IDLE);
          }
        }
      }

      log_debug(LD_SCHED,
                "Scheduler flushed %d cells onto pending channel "
                "%"PRIu64 " at %p",
                (int)flushed, (chan->global_identifier),
                chan);
    } else {
      log_info(LD_SCHED,
               "Scheduler saw pending channel %"PRIu64 " at %p with "
               "no cells writeable",
               (chan->global_identifier), chan);
      /* Put it back to WAITING_TO_WRITE */
      scheduler_set_channel_state(chan, SCHED_CHAN_WAITING_TO_WRITE);
    }
  }

  /* Readd any channels we need to */
  if (to_readd) {
    SMARTLIST_FOREACH_BEGIN(to_readd, channel_t *, readd_chan) {
      scheduler_set_channel_state(readd_chan, SCHED_CHAN_PENDING);
      smartlist_pqueue_add(cp,
                           scheduler_compare_channels,
                           offsetof(channel_t, sched_heap_idx),
                           readd_chan);
    } SMARTLIST_FOREACH_END(readd_chan);
    smartlist_free(to_readd);
  }

  n_chans_after = smartlist_len(cp);
  log_debug(LD_SCHED, "Scheduler handled %d of %d pending channels",
            n_chans_before - n_chans_after, n_chans_before);
}

/* Stores the vanilla scheduler function pointers. */
static scheduler_t vanilla_scheduler = {
  .type = SCHEDULER_VANILLA,
  .free_all = NULL,
  .on_channel_free = NULL,
  .init = NULL,
  .on_new_consensus = NULL,
  .schedule = vanilla_scheduler_schedule,
  .run = vanilla_scheduler_run,
  .on_new_options = NULL,
};

scheduler_t *
get_vanilla_scheduler(void)
{
  return &vanilla_scheduler;
}
