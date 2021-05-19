/* Copyright (c) 2018-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file voting_schedule.c
 * \brief Compute information about our voting schedule as a directory
 *    authority.
 **/

#include "feature/dirauth/voting_schedule.h"

#include "core/or/or.h"
#include "app/config/config.h"
#include "feature/nodelist/networkstatus.h"

#include "feature/nodelist/networkstatus_st.h"

/* =====
 * Vote scheduling
 * ===== */

/* Populate and return a new voting_schedule_t that can be used to schedule
 * voting. The object is allocated on the heap and it's the responsibility of
 * the caller to free it. Can't fail. */
static voting_schedule_t *
create_voting_schedule(const or_options_t *options, time_t now, int severity)
{
  int interval, vote_delay, dist_delay;
  time_t start;
  time_t end;
  networkstatus_t *consensus;
  voting_schedule_t *new_voting_schedule;

  new_voting_schedule = tor_malloc_zero(sizeof(voting_schedule_t));

  consensus = networkstatus_get_live_consensus(now);

  if (consensus) {
    interval = (int)( consensus->fresh_until - consensus->valid_after );
    vote_delay = consensus->vote_seconds;
    dist_delay = consensus->dist_seconds;

    /* Note down the consensus valid after, so that we detect outdated voting
     * schedules in case of skewed clocks etc. */
    new_voting_schedule->live_consensus_valid_after = consensus->valid_after;
  } else {
    interval = options->TestingV3AuthInitialVotingInterval;
    vote_delay = options->TestingV3AuthInitialVoteDelay;
    dist_delay = options->TestingV3AuthInitialDistDelay;
  }

  tor_assert(interval > 0);
  new_voting_schedule->interval = interval;

  if (vote_delay + dist_delay > interval/2)
    vote_delay = dist_delay = interval / 4;

  start = new_voting_schedule->interval_starts =
    voting_sched_get_start_of_interval_after(now,interval,
                                      options->TestingV3AuthVotingStartOffset);
  end = voting_sched_get_start_of_interval_after(start+1, interval,
                                      options->TestingV3AuthVotingStartOffset);

  tor_assert(end > start);

  new_voting_schedule->fetch_missing_signatures = start - (dist_delay/2);
  new_voting_schedule->voting_ends = start - dist_delay;
  new_voting_schedule->fetch_missing_votes =
    start - dist_delay - (vote_delay/2);
  new_voting_schedule->voting_starts = start - dist_delay - vote_delay;

  {
    char tbuf[ISO_TIME_LEN+1];
    format_iso_time(tbuf, new_voting_schedule->interval_starts);
    tor_log(severity, LD_DIR,"Choosing expected valid-after time as %s: "
            "consensus_set=%d, interval=%d",
            tbuf, consensus?1:0, interval);
  }

  return new_voting_schedule;
}

#define voting_schedule_free(s) \
  FREE_AND_NULL(voting_schedule_t, voting_schedule_free_, (s))

/** Frees a voting_schedule_t. This should be used instead of the generic
 * tor_free. */
static void
voting_schedule_free_(voting_schedule_t *voting_schedule_to_free)
{
  if (!voting_schedule_to_free)
    return;
  tor_free(voting_schedule_to_free);
}

voting_schedule_t voting_schedule;

/**
 * Return the current voting schedule, recreating it if necessary.
 *
 * Dirauth only.
 **/
static const voting_schedule_t *
dirauth_get_voting_schedule(void)
{
  time_t now = approx_time();
  bool need_to_recalculate_voting_schedule = false;

  /* This is a safe guard in order to make sure that the voting schedule
   * static object is at least initialized. Using this function with a zeroed
   * voting schedule can lead to bugs. */
  if (fast_mem_is_zero((const char *) &voting_schedule,
                      sizeof(voting_schedule))) {
    need_to_recalculate_voting_schedule = true;
    goto done; /* no need for next check if we have to recalculate anyway */
  }

  /* Also make sure we are not using an outdated voting schedule. If we have a
   * newer consensus, make sure we recalculate the voting schedule. */
  const networkstatus_t *ns = networkstatus_get_live_consensus(now);
  if (ns && ns->valid_after != voting_schedule.live_consensus_valid_after) {
    log_info(LD_DIR, "Voting schedule is outdated: recalculating (%d/%d)",
             (int) ns->valid_after,
             (int) voting_schedule.live_consensus_valid_after);
    need_to_recalculate_voting_schedule = true;
  }

 done:
  if (need_to_recalculate_voting_schedule) {
    dirauth_sched_recalculate_timing(get_options(), approx_time());
    voting_schedule.created_on_demand = 1;
  }

  return &voting_schedule;
}

/** Return the next voting valid-after time.
 *
 * Dirauth only. */
time_t
dirauth_sched_get_next_valid_after_time(void)
{
  return dirauth_get_voting_schedule()->interval_starts;
}

/**
 * Return our best idea of what the valid-after time for the _current_
 * consensus, whether we have one or not.
 *
 * Dirauth only.
 **/
time_t
dirauth_sched_get_cur_valid_after_time(void)
{
  const voting_schedule_t *sched = dirauth_get_voting_schedule();
  time_t next_start = sched->interval_starts;
  int interval = sched->interval;
  int offset = get_options()->TestingV3AuthVotingStartOffset;
  return voting_sched_get_start_of_interval_after(next_start - interval - 1,
                                                  interval,
                                                  offset);
}

/** Return the voting interval that we are configured to use.
 *
 * Dirauth only. */
int
dirauth_sched_get_configured_interval(void)
{
  return get_options()->V3AuthVotingInterval;
}

/** Set voting_schedule to hold the timing for the next vote we should be
 * doing. All type of tor do that because HS subsystem needs the timing as
 * well to function properly. */
void
dirauth_sched_recalculate_timing(const or_options_t *options, time_t now)
{
  voting_schedule_t *new_voting_schedule;

  /* get the new voting schedule */
  new_voting_schedule = create_voting_schedule(options, now, LOG_INFO);
  tor_assert(new_voting_schedule);

  /* Fill in the global static struct now */
  memcpy(&voting_schedule, new_voting_schedule, sizeof(voting_schedule));
  voting_schedule_free(new_voting_schedule);
}
