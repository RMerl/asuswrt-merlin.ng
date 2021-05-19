/* Copyright (c) 2010-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

#define COMPAT_LIBEVENT_PRIVATE
#include "orconfig.h"
#include "core/or/or.h"

#include "test/test.h"

#include "lib/evloop/compat_libevent.h"

#include <event2/event.h>

#include "test/log_test_helpers.h"

static void
test_compat_libevent_logging_callback(void *ignored)
{
  (void)ignored;
  setup_full_capture_of_logs(LOG_DEBUG);

  libevent_logging_callback(_EVENT_LOG_DEBUG, "hello world");
  expect_log_msg("Message from libevent: hello world\n");
  expect_log_severity(LOG_DEBUG);
  tt_int_op(smartlist_len(mock_saved_logs()), OP_EQ, 1);

  mock_clean_saved_logs();
  libevent_logging_callback(_EVENT_LOG_MSG, "hello world another time");
  expect_log_msg("Message from libevent: hello world another time\n");
  expect_log_severity(LOG_INFO);
  tt_int_op(smartlist_len(mock_saved_logs()), OP_EQ, 1);

  mock_clean_saved_logs();
  libevent_logging_callback(_EVENT_LOG_WARN, "hello world a third time");
  expect_log_msg("Warning from libevent: hello world a third time\n");
  expect_log_severity(LOG_WARN);
  tt_int_op(smartlist_len(mock_saved_logs()), OP_EQ, 1);

  mock_clean_saved_logs();
  libevent_logging_callback(_EVENT_LOG_ERR, "hello world a fourth time");
  expect_log_msg("Error from libevent: hello world a fourth time\n");
  expect_log_severity(LOG_ERR);
  tt_int_op(smartlist_len(mock_saved_logs()), OP_EQ, 1);

  mock_clean_saved_logs();
  libevent_logging_callback(42, "hello world a fifth time");
  expect_log_msg("Message [42] from libevent: hello world a fifth time\n");
  expect_log_severity(LOG_WARN);
  tt_int_op(smartlist_len(mock_saved_logs()), OP_EQ, 1);

  mock_clean_saved_logs();
  libevent_logging_callback(_EVENT_LOG_DEBUG,
                            "012345678901234567890123456789"
                            "012345678901234567890123456789"
                            "012345678901234567890123456789"
                            "012345678901234567890123456789"
                            "012345678901234567890123456789"
                            "012345678901234567890123456789"
                            "012345678901234567890123456789"
                            "012345678901234567890123456789"
                            "012345678901234567890123456789"
                            "012345678901234567890123456789"
                            "012345678901234567890123456789"
                            "012345678901234567890123456789"
                            );
  expect_log_msg("Message from libevent: "
                            "012345678901234567890123456789"
                            "012345678901234567890123456789"
                            "012345678901234567890123456789"
                            "012345678901234567890123456789"
                            "012345678901234567890123456789"
                            "012345678901234567890123456789"
                            "012345678901234567890123456789"
                            "012345678901234567890123456789"
                            "012345678901234567890123456789"
                            "012345678901234567890123456789"
                            "012345678901234567890123456789"
            "012345678901234567890123456789\n");
  expect_log_severity(LOG_DEBUG);
  tt_int_op(smartlist_len(mock_saved_logs()), OP_EQ, 1);

  mock_clean_saved_logs();
  libevent_logging_callback(42, "xxx\n");
  expect_log_msg("Message [42] from libevent: xxx\n");
  expect_log_severity(LOG_WARN);
  tt_int_op(smartlist_len(mock_saved_logs()), OP_EQ, 1);

  suppress_libevent_log_msg("something");
  mock_clean_saved_logs();
  libevent_logging_callback(_EVENT_LOG_MSG, "hello there");
  expect_log_msg("Message from libevent: hello there\n");
  expect_log_severity(LOG_INFO);
  tt_int_op(smartlist_len(mock_saved_logs()), OP_EQ, 1);

  mock_clean_saved_logs();
  libevent_logging_callback(_EVENT_LOG_MSG, "hello there something else");
  expect_no_log_msg("hello there something else");
  if (mock_saved_logs())
    tt_int_op(smartlist_len(mock_saved_logs()), OP_EQ, 0);

  // No way of verifying the result of this, it seems =/
  configure_libevent_logging();

 done:
  suppress_libevent_log_msg(NULL);
  teardown_capture_of_logs();
}

static void
test_compat_libevent_header_version(void *ignored)
{
  (void)ignored;
  const char *res;

  res = tor_libevent_get_header_version_str();
  tt_str_op(res, OP_EQ, LIBEVENT_VERSION);

 done:
  (void)0;
}

/* Test for postloop events */

/* Event callback to increment a counter. */
static void
increment_int_counter_cb(periodic_timer_t *timer, void *arg)
{
  (void)timer;
  int *ctr = arg;
  ++*ctr;
}

static int activated_counter = 0;

/* Mainloop event callback to activate another mainloop event */
static void
activate_event_cb(mainloop_event_t *ev, void *arg)
{
  (void)ev;
  mainloop_event_t **other_event = arg;
  mainloop_event_activate(*other_event);
  ++activated_counter;
}

static void
test_compat_libevent_postloop_events(void *arg)
{
  (void)arg;
  mainloop_event_t *a = NULL, *b = NULL;
  periodic_timer_t *timed = NULL;

  /* If postloop events don't work, then these events will activate one
   * another ad infinitum and, and the periodic event will never occur. */
  b = mainloop_event_postloop_new(activate_event_cb, &a);
  a = mainloop_event_postloop_new(activate_event_cb, &b);

  int counter = 0;
  struct timeval fifty_ms = { 0, 10 * 1000 };
  timed = periodic_timer_new(tor_libevent_get_base(), &fifty_ms,
                             increment_int_counter_cb, &counter);

  mainloop_event_activate(a);
  int r;
  do {
    r = tor_libevent_run_event_loop(tor_libevent_get_base(), 0);
    if (r == -1)
      break;
  } while (counter < 5);

  tt_int_op(activated_counter, OP_GE, 2);

 done:
  mainloop_event_free(a);
  mainloop_event_free(b);
  periodic_timer_free(timed);
}

struct testcase_t compat_libevent_tests[] = {
  { "logging_callback", test_compat_libevent_logging_callback,
    TT_FORK, NULL, NULL },
  { "header_version", test_compat_libevent_header_version, 0, NULL, NULL },
  { "postloop_events", test_compat_libevent_postloop_events,
    TT_FORK, NULL, NULL },
  END_OF_TESTCASES
};
