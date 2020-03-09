/* Copyright (c) 2018-2019, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file voting_schedule.h
 * \brief Header file for voting_schedule.c.
 **/

#ifndef TOR_VOTING_SCHEDULE_H
#define TOR_VOTING_SCHEDULE_H

#include "core/or/or.h"

/** Scheduling information for a voting interval. */
typedef struct {
  /** When do we generate and distribute our vote for this interval? */
  time_t voting_starts;
  /** When do we send an HTTP request for any votes that we haven't
   * been posted yet?*/
  time_t fetch_missing_votes;
  /** When do we give up on getting more votes and generate a consensus? */
  time_t voting_ends;
  /** When do we send an HTTP request for any signatures we're expecting to
   * see on the consensus? */
  time_t fetch_missing_signatures;
  /** When do we publish the consensus? */
  time_t interval_starts;

  /* True iff we have generated and distributed our vote. */
  int have_voted;
  /* True iff we've requested missing votes. */
  int have_fetched_missing_votes;
  /* True iff we have built a consensus and sent the signatures around. */
  int have_built_consensus;
  /* True iff we've fetched missing signatures. */
  int have_fetched_missing_signatures;
  /* True iff we have published our consensus. */
  int have_published_consensus;

  /* True iff this voting schedule was set on demand meaning not through the
   * normal vote operation of a dirauth or when a consensus is set. This only
   * applies to a directory authority that needs to recalculate the voting
   * timings only for the first vote even though this object was initilized
   * prior to voting. */
  int created_on_demand;

  /** The valid-after time of the last live consensus that filled this voting
   *  schedule.  It's used to detect outdated voting schedules. */
  time_t live_consensus_valid_after;
} voting_schedule_t;

/* Public API. */

extern voting_schedule_t voting_schedule;

void voting_schedule_recalculate_timing(const or_options_t *options,
                                        time_t now);

time_t voting_schedule_get_start_of_next_interval(time_t now,
                                                  int interval,
                                                  int offset);
time_t voting_schedule_get_next_valid_after_time(void);

#endif /* !defined(TOR_VOTING_SCHEDULE_H) */

