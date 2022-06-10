/* Copyright (c) 2013-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

#include "core/or/or.h"

#include "test/test.h"
#include "test/test_helpers.h"
#include "test/log_test_helpers.h"

#define OCIRC_EVENT_PRIVATE
#define ORCONN_EVENT_PRIVATE
#include "core/or/ocirc_event.h"
#include "core/or/orconn_event.h"

static void
send_state(const orconn_state_msg_t *msg_in)
{
  orconn_state_msg_t *msg = tor_malloc(sizeof(*msg));

  *msg = *msg_in;
  orconn_state_publish(msg);
}

static void
send_status(const orconn_status_msg_t *msg_in)
{
  orconn_status_msg_t *msg = tor_malloc(sizeof(*msg));

  *msg = *msg_in;
  orconn_status_publish(msg);
}

static void
send_chan(const ocirc_chan_msg_t *msg_in)
{
  ocirc_chan_msg_t *msg = tor_malloc(sizeof(*msg));

  *msg = *msg_in;
  ocirc_chan_publish(msg);
}

static void
test_btrack_launch(void *arg)
{
  orconn_state_msg_t conn;
  ocirc_chan_msg_t circ;
  memset(&conn, 0, sizeof(conn));
  memset(&circ, 0, sizeof(circ));

  (void)arg;
  conn.gid = 1;
  conn.chan = 1;
  conn.proxy_type = PROXY_NONE;
  conn.state = OR_CONN_STATE_CONNECTING;

  setup_full_capture_of_logs(LOG_DEBUG);
  send_state(&conn);
  expect_log_msg_containing("ORCONN gid=1 chan=1 proxy_type=0 state=1");
  expect_no_log_msg_containing("ORCONN BEST_");
  teardown_capture_of_logs();

  circ.chan = 1;
  circ.onehop = true;

  setup_full_capture_of_logs(LOG_DEBUG);
  send_chan(&circ);
  expect_log_msg_containing("ORCONN LAUNCH chan=1 onehop=1");
  expect_log_msg_containing("ORCONN BEST_ANY state -1->1 gid=1");
  teardown_capture_of_logs();

  conn.gid = 2;
  conn.chan = 2;

  setup_full_capture_of_logs(LOG_DEBUG);
  send_state(&conn);
  expect_log_msg_containing("ORCONN gid=2 chan=2 proxy_type=0 state=1");
  expect_no_log_msg_containing("ORCONN BEST_");
  teardown_capture_of_logs();

  circ.chan = 2;
  circ.onehop = false;

  setup_full_capture_of_logs(LOG_DEBUG);
  send_chan(&circ);
  expect_log_msg_containing("ORCONN LAUNCH chan=2 onehop=0");
  expect_log_msg_containing("ORCONN BEST_AP state -1->1 gid=2");
  teardown_capture_of_logs();

 done:
  ;
}

static void
test_btrack_delete(void *arg)
{
  orconn_state_msg_t state;
  orconn_status_msg_t status;
  memset(&state, 0, sizeof(state));
  memset(&status, 0, sizeof(status));

  (void)arg;
  state.gid = 1;
  state.chan = 1;
  state.proxy_type = PROXY_NONE;
  state.state = OR_CONN_STATE_CONNECTING;

  setup_full_capture_of_logs(LOG_DEBUG);
  send_state(&state);
  expect_log_msg_containing("ORCONN gid=1 chan=1 proxy_type=0");
  teardown_capture_of_logs();

  status.gid = 1;
  status.status = OR_CONN_EVENT_CLOSED;
  status.reason = 0;

  setup_full_capture_of_logs(LOG_DEBUG);
  send_status(&status);
  expect_log_msg_containing("ORCONN DELETE gid=1 status=3 reason=0");
  teardown_capture_of_logs();

 done:
  ;
}

struct testcase_t btrack_tests[] = {
  { "launch", test_btrack_launch, TT_FORK, &helper_pubsub_setup, NULL },
  { "delete", test_btrack_delete, TT_FORK, &helper_pubsub_setup, NULL },
  END_OF_TESTCASES
};
