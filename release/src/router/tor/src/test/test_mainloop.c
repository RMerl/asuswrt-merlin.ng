/* Copyright (c) 2018-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file test_mainloop.c
 * \brief Tests for functions closely related to the Tor main loop
 */

#define CONFIG_PRIVATE
#define MAINLOOP_PRIVATE
#define STATEFILE_PRIVATE

#include "test/test.h"
#include "test/log_test_helpers.h"

#include "lib/confmgt/confmgt.h"

#include "core/or/or.h"
#include "core/mainloop/connection.h"
#include "core/mainloop/mainloop.h"
#include "core/mainloop/mainloop_state_st.h"
#include "core/mainloop/mainloop_sys.h"
#include "core/mainloop/netstatus.h"

#include "feature/hs/hs_service.h"

#include "app/config/config.h"
#include "app/config/statefile.h"
#include "app/config/or_state_st.h"

#include "app/main/subsysmgr.h"

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

static int schedule_rescan_called = 0;
static void
mock_schedule_rescan_periodic_events(void)
{
  ++schedule_rescan_called;
}

static void
test_mainloop_user_activity(void *arg)
{
  (void)arg;
  const time_t start = 1542658829;
  update_approx_time(start);

  MOCK(schedule_rescan_periodic_events, mock_schedule_rescan_periodic_events);

  reset_user_activity(start);
  tt_i64_op(get_last_user_activity_time(), OP_EQ, start);

  set_network_participation(false);

  // reset can move backwards and forwards, but does not change network
  // participation.
  reset_user_activity(start-10);
  tt_i64_op(get_last_user_activity_time(), OP_EQ, start-10);
  reset_user_activity(start+10);
  tt_i64_op(get_last_user_activity_time(), OP_EQ, start+10);

  tt_int_op(schedule_rescan_called, OP_EQ, 0);
  tt_int_op(false, OP_EQ, is_participating_on_network());

  // "note" can only move forward.  Calling it from a non-participating
  // state makes us rescan the periodic callbacks and set participation.
  note_user_activity(start+20);
  tt_i64_op(get_last_user_activity_time(), OP_EQ, start+20);
  tt_int_op(true, OP_EQ, is_participating_on_network());
  tt_int_op(schedule_rescan_called, OP_EQ, 1);

  // Calling it again will move us forward, but not call rescan again.
  note_user_activity(start+25);
  tt_i64_op(get_last_user_activity_time(), OP_EQ, start+25);
  tt_int_op(true, OP_EQ, is_participating_on_network());
  tt_int_op(schedule_rescan_called, OP_EQ, 1);

  // We won't move backwards.
  note_user_activity(start+20);
  tt_i64_op(get_last_user_activity_time(), OP_EQ, start+25);
  tt_int_op(true, OP_EQ, is_participating_on_network());
  tt_int_op(schedule_rescan_called, OP_EQ, 1);

  // We _will_ adjust if the clock jumps though.
  netstatus_note_clock_jumped(500);
  tt_i64_op(get_last_user_activity_time(), OP_EQ, start+525);

  netstatus_note_clock_jumped(-400);
  tt_i64_op(get_last_user_activity_time(), OP_EQ, start+125);

 done:
  UNMOCK(schedule_rescan_periodic_events);
}

static unsigned int
mock_get_num_services(void)
{
  return 1;
}

static connection_t *
mock_connection_gbtu(int type)
{
  (void) type;
  return (void *)"hello fellow connections";
}

static void
test_mainloop_check_participation(void *arg)
{
  (void)arg;
  or_options_t *options = options_new();
  const time_t start = 1542658829;
  const time_t ONE_DAY = 24*60*60;

  options->DormantTimeoutEnabled = 1;

  // Suppose we've been idle for a day or two
  reset_user_activity(start - 2*ONE_DAY);
  set_network_participation(true);
  check_network_participation_callback(start, options);
  tt_int_op(is_participating_on_network(), OP_EQ, false);
  tt_i64_op(get_last_user_activity_time(), OP_EQ, start-2*ONE_DAY);

  // suppose we've been idle for 2 days... but we are a server.
  reset_user_activity(start - 2*ONE_DAY);
  options->ORPort_set = 1;
  set_network_participation(true);
  check_network_participation_callback(start+2, options);
  tt_int_op(is_participating_on_network(), OP_EQ, true);
  tt_i64_op(get_last_user_activity_time(), OP_EQ, start+2);
  options->ORPort_set = 0;

  // idle for 2 days, but we have a hidden service.
  reset_user_activity(start - 2*ONE_DAY);
  set_network_participation(true);
  MOCK(hs_service_get_num_services, mock_get_num_services);
  check_network_participation_callback(start+3, options);
  tt_int_op(is_participating_on_network(), OP_EQ, true);
  tt_i64_op(get_last_user_activity_time(), OP_EQ, start+3);
  UNMOCK(hs_service_get_num_services);

  // idle for 2 days but we have at least one user connection
  MOCK(connection_get_by_type_nonlinked, mock_connection_gbtu);
  reset_user_activity(start - 2*ONE_DAY);
  set_network_participation(true);
  options->DormantTimeoutDisabledByIdleStreams = 1;
  check_network_participation_callback(start+10, options);
  tt_int_op(is_participating_on_network(), OP_EQ, true);
  tt_i64_op(get_last_user_activity_time(), OP_EQ, start+10);

  // as above, but DormantTimeoutDisabledByIdleStreams is not set
  reset_user_activity(start - 2*ONE_DAY);
  set_network_participation(true);
  options->DormantTimeoutDisabledByIdleStreams = 0;
  check_network_participation_callback(start+13, options);
  tt_int_op(is_participating_on_network(), OP_EQ, false);
  tt_i64_op(get_last_user_activity_time(), OP_EQ, start-2*ONE_DAY);
  UNMOCK(connection_get_by_type_nonlinked);
  options->DormantTimeoutDisabledByIdleStreams = 1;

  // idle for 2 days but DormantClientTimeout is 3 days
  reset_user_activity(start - 2*ONE_DAY);
  set_network_participation(true);
  options->DormantClientTimeout = ONE_DAY * 3;
  check_network_participation_callback(start+30, options);
  tt_int_op(is_participating_on_network(), OP_EQ, true);
  tt_i64_op(get_last_user_activity_time(), OP_EQ, start-2*ONE_DAY);

 done:
  or_options_free(options);
  UNMOCK(hs_service_get_num_services);
  UNMOCK(connection_get_by_type_nonlinked);
}

static void
test_mainloop_dormant_load_state(void *arg)
{
  (void)arg;
  or_state_t *or_state = or_state_new();
  mainloop_state_t *state;
  {
    int idx = subsystems_get_state_idx(&sys_mainloop);
    tor_assert(idx >= 0);
    state = config_mgr_get_obj_mutable(get_state_mgr(), or_state, idx);
  }
  const time_t start = 1543956575;

  reset_user_activity(0);
  set_network_participation(false);

  // When we construct a new state, it starts out in "auto" mode.
  tt_int_op(state->Dormant, OP_EQ, -1);

  // Initializing from "auto" makes us start out (by default) non-Dormant,
  // with activity right now.
  netstatus_load_from_state(state, start);
  tt_assert(is_participating_on_network());
  tt_i64_op(get_last_user_activity_time(), OP_EQ, start);

  // Initializing from dormant clears the last user activity time, and
  // makes us dormant.
  state->Dormant = 1;
  netstatus_load_from_state(state, start);
  tt_assert(! is_participating_on_network());
  tt_i64_op(get_last_user_activity_time(), OP_EQ, 0);

  // Initializing from non-dormant sets the last user activity time, and
  // makes us non-dormant.
  state->Dormant = 0;
  state->MinutesSinceUserActivity = 123;
  netstatus_load_from_state(state, start);
  tt_assert(is_participating_on_network());
  tt_i64_op(get_last_user_activity_time(), OP_EQ, start - 123*60);

  // If we would start dormant, but DormantCanceledByStartup is set, then
  // we start up non-dormant.
  state->Dormant = 1;
  get_options_mutable()->DormantCanceledByStartup = 1;
  netstatus_load_from_state(state, start);
  tt_assert(is_participating_on_network());
  tt_i64_op(get_last_user_activity_time(), OP_EQ, start);

 done:
  or_state_free(or_state);
}

static void
test_mainloop_dormant_save_state(void *arg)
{
  (void)arg;
  mainloop_state_t *state = tor_malloc_zero(sizeof(mainloop_state_t));
  const time_t start = 1543956575;

  // Can we save a non-dormant state correctly?
  reset_user_activity(start - 1000);
  set_network_participation(true);
  netstatus_flush_to_state(state, start);

  tt_int_op(state->Dormant, OP_EQ, 0);
  tt_int_op(state->MinutesSinceUserActivity, OP_EQ, 1000 / 60);

  // Can we save a dormant state correctly?
  set_network_participation(false);
  netstatus_flush_to_state(state, start);

  tt_int_op(state->Dormant, OP_EQ, 1);
  tt_int_op(state->MinutesSinceUserActivity, OP_EQ, 0);

 done:
  tor_free(state);
}

#define MAINLOOP_TEST(name) \
  { #name, test_mainloop_## name , TT_FORK, NULL, NULL }

struct testcase_t mainloop_tests[] = {
  MAINLOOP_TEST(update_time_normal),
  MAINLOOP_TEST(update_time_jumps),
  MAINLOOP_TEST(user_activity),
  MAINLOOP_TEST(check_participation),
  MAINLOOP_TEST(dormant_load_state),
  MAINLOOP_TEST(dormant_save_state),
  END_OF_TESTCASES
};
