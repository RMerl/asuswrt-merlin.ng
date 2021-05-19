/* Copyright (c) 2018-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

#include "orconfig.h"

#include "core/or/or.h"
#include "feature/dirauth/voting_schedule.h"
#include "feature/nodelist/networkstatus.h"

#include "test/test.h"

static void
test_voting_schedule_interval_start(void *arg)
{
#define next_interval voting_sched_get_start_of_interval_after
  (void)arg;
  char buf[ISO_TIME_LEN+1];

  // Midnight UTC tonight (as I am writing this test)
  const time_t midnight = 1525651200;
  format_iso_time(buf, midnight);
  tt_str_op(buf, OP_EQ, "2018-05-07 00:00:00");

  /* Some simple tests with a 50-minute voting interval */

  tt_i64_op(next_interval(midnight, 3000, 0), OP_EQ,
            midnight+3000);

  tt_i64_op(next_interval(midnight+100, 3000, 0), OP_EQ,
            midnight+3000);

  tt_i64_op(next_interval(midnight+3000, 3000, 0), OP_EQ,
            midnight+6000);

  tt_i64_op(next_interval(midnight+3001, 3000, 0), OP_EQ,
            midnight+6000);

  /* Make sure that we roll around properly at midnight */
  tt_i64_op(next_interval(midnight+83000, 3000, 0), OP_EQ,
            midnight+84000);

  /* We start fresh at midnight UTC, even if there are leftover seconds. */
  tt_i64_op(next_interval(midnight+84005, 3000, 0), OP_EQ,
            midnight+86400);

  /* Now try with offsets.  (These are only used for test networks.) */
  tt_i64_op(next_interval(midnight, 3000, 99), OP_EQ,
            midnight+99);

  tt_i64_op(next_interval(midnight+100, 3000, 99), OP_EQ,
            midnight+3099);

 done:
  ;
#undef next_interval
}

#define VS(name,flags)                                          \
  { #name, test_voting_schedule_##name, (flags), NULL, NULL }

struct testcase_t voting_schedule_tests[] = {
  VS(interval_start, 0),
  END_OF_TESTCASES
};
