/* Copyright (c) 2018-2019, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file test_mainloop.c
 * \brief Tests for functions closely related to the Tor main loop
 */

#include "test/test.h"
#include "test/log_test_helpers.h"

#include "core/or/or.h"
#include "core/mainloop/mainloop.h"

static const uint64_t BILLION = 1000000000;

static void
test_mainloop_update_time_normal(void *arg)
{
  (void)arg;

  monotime_enable_test_mocking();
  /* This is arbitrary */
  uint64_t mt_now = UINT64_C(7493289274986);
  /* This time is in the past as of when this test was written. */
  time_t now = 1525272090;
  monotime_coarse_set_mock_time_nsec(mt_now);
  reset_uptime();
  update_current_time(now);
  tt_int_op(approx_time(), OP_EQ, now);
  tt_int_op(get_uptime(), OP_EQ, 0);

  update_current_time(now); // Same time as before is a no-op.
  tt_int_op(get_uptime(), OP_EQ, 0);

  now += 1;
  mt_now += BILLION;
  monotime_coarse_set_mock_time_nsec(mt_now);
  update_current_time(now);
  tt_int_op(approx_time(), OP_EQ, now);
  tt_int_op(get_uptime(), OP_EQ, 1);

  now += 2; // two-second jump is unremarkable.
  mt_now += 2*BILLION;
  update_current_time(now);
  monotime_coarse_set_mock_time_nsec(mt_now);
  tt_int_op(approx_time(), OP_EQ, now);
  tt_int_op(get_uptime(), OP_EQ, 3);

  now -= 1; // a one-second hop backwards is also unremarkable.
  update_current_time(now);
  tt_int_op(approx_time(), OP_EQ, now); // it changes the approx time...
  tt_int_op(get_uptime(), OP_EQ, 3); // but it doesn't roll back our uptime

 done:
  monotime_disable_test_mocking();
}

static void
test_mainloop_update_time_jumps(void *arg)
{
  (void)arg;

  monotime_enable_test_mocking();
  /* This is arbitrary */
  uint64_t mt_now = UINT64_C(7493289274986);
  /* This time is in the past as of when this test was written. */
  time_t now = 220897152;
  monotime_coarse_set_mock_time_nsec(mt_now);
  reset_uptime();
  update_current_time(now);
  tt_int_op(approx_time(), OP_EQ, now);
  tt_int_op(get_uptime(), OP_EQ, 0);

  /* Put some uptime on the clock.. */
  now += 3;
  mt_now += 3*BILLION;
  monotime_coarse_set_mock_time_nsec(mt_now);
  update_current_time(now);
  tt_int_op(approx_time(), OP_EQ, now);
  tt_int_op(get_uptime(), OP_EQ, 3);

  /* Now try jumping forward and backward, without updating the monotonic
   * clock.  */
  setup_capture_of_logs(LOG_NOTICE);
  now += 1800;
  update_current_time(now);
  expect_single_log_msg_containing(
               "Your system clock just jumped 1800 seconds forward");
  tt_int_op(approx_time(), OP_EQ, now);
  tt_int_op(get_uptime(), OP_EQ, 3); // no uptime change.
  mock_clean_saved_logs();

  now -= 600;
  update_current_time(now);
  expect_single_log_msg_containing(
               "Your system clock just jumped 600 seconds backward");
  tt_int_op(approx_time(), OP_EQ, now);
  tt_int_op(get_uptime(), OP_EQ, 3); // no uptime change.
  mock_clean_saved_logs();

  /* uptime tracking should go normally now if the clock moves sensibly. */
  now += 2;
  mt_now += 2*BILLION;
  update_current_time(now);
  tt_int_op(approx_time(), OP_EQ, now);
  tt_int_op(get_uptime(), OP_EQ, 5);

  /* If we skip forward by a few minutes but the monotonic clock agrees,
   * we've just been idle: that counts as not worth warning about. */
  now += 1800;
  mt_now += 1800*BILLION;
  monotime_coarse_set_mock_time_nsec(mt_now);
  update_current_time(now);
  expect_no_log_entry();
  tt_int_op(approx_time(), OP_EQ, now);
  tt_int_op(get_uptime(), OP_EQ, 5); // this doesn't count to uptime, though.

  /* If we skip forward by a long time, even if the clock agrees, it's
   * idnless that counts. */
  now += 4000;
  mt_now += 4000*BILLION;
  monotime_coarse_set_mock_time_nsec(mt_now);
  update_current_time(now);
  expect_single_log_msg_containing("Tor has been idle for 4000 seconds");
  tt_int_op(approx_time(), OP_EQ, now);
  tt_int_op(get_uptime(), OP_EQ, 5);

 done:
  teardown_capture_of_logs();
  monotime_disable_test_mocking();
}

#define MAINLOOP_TEST(name) \
  { #name, test_mainloop_## name , TT_FORK, NULL, NULL }

struct testcase_t mainloop_tests[] = {
  MAINLOOP_TEST(update_time_normal),
  MAINLOOP_TEST(update_time_jumps),
  END_OF_TESTCASES
};

