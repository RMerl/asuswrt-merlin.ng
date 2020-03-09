/* Copyright (c) 2013-2019, The Tor Project, Inc. */
/* See LICENSE for licensing information */

#define CONNECTION_PRIVATE
#define TOR_CHANNEL_INTERNAL_
#define CONTROL_PRIVATE
#define CONTROL_EVENTS_PRIVATE
#define OCIRC_EVENT_PRIVATE
#define ORCONN_EVENT_PRIVATE
#include "app/main/subsysmgr.h"
#include "core/or/or.h"
#include "core/or/channel.h"
#include "core/or/channeltls.h"
#include "core/or/circuitlist.h"
#include "core/or/ocirc_event.h"
#include "core/or/orconn_event.h"
#include "core/mainloop/connection.h"
#include "feature/control/control_events.h"
#include "test/test.h"
#include "test/test_helpers.h"

#include "core/or/or_circuit_st.h"
#include "core/or/origin_circuit_st.h"

static void
add_testing_cell_stats_entry(circuit_t *circ, uint8_t command,
                             unsigned int waiting_time,
                             unsigned int removed, unsigned int exitward)
{
  testing_cell_stats_entry_t *ent = tor_malloc_zero(
                                    sizeof(testing_cell_stats_entry_t));
  ent->command = command;
  ent->waiting_time = waiting_time;
  ent->removed = removed;
  ent->exitward = exitward;
  if (!circ->testing_cell_stats)
    circ->testing_cell_stats = smartlist_new();
  smartlist_add(circ->testing_cell_stats, ent);
}

static void
test_cntev_sum_up_cell_stats(void *arg)
{
  or_circuit_t *or_circ;
  circuit_t *circ;
  cell_stats_t *cell_stats = NULL;
  (void)arg;

  /* This circuit is fake. */
  or_circ = tor_malloc_zero(sizeof(or_circuit_t));
  or_circ->base_.magic = OR_CIRCUIT_MAGIC;
  or_circ->base_.purpose = CIRCUIT_PURPOSE_OR;
  circ = TO_CIRCUIT(or_circ);

  /* A single RELAY cell was added to the appward queue. */
  cell_stats = tor_malloc_zero(sizeof(cell_stats_t));
  add_testing_cell_stats_entry(circ, CELL_RELAY, 0, 0, 0);
  sum_up_cell_stats_by_command(circ, cell_stats);
  tt_u64_op(1, OP_EQ, cell_stats->added_cells_appward[CELL_RELAY]);

  /* A single RELAY cell was added to the exitward queue. */
  add_testing_cell_stats_entry(circ, CELL_RELAY, 0, 0, 1);
  sum_up_cell_stats_by_command(circ, cell_stats);
  tt_u64_op(1, OP_EQ, cell_stats->added_cells_exitward[CELL_RELAY]);

  /* A single RELAY cell was removed from the appward queue where it spent
   * 20 msec. */
  add_testing_cell_stats_entry(circ, CELL_RELAY, 2, 1, 0);
  sum_up_cell_stats_by_command(circ, cell_stats);
  tt_u64_op(20, OP_EQ, cell_stats->total_time_appward[CELL_RELAY]);
  tt_u64_op(1, OP_EQ, cell_stats->removed_cells_appward[CELL_RELAY]);

  /* A single RELAY cell was removed from the exitward queue where it
   * spent 30 msec. */
  add_testing_cell_stats_entry(circ, CELL_RELAY, 3, 1, 1);
  sum_up_cell_stats_by_command(circ, cell_stats);
  tt_u64_op(30, OP_EQ, cell_stats->total_time_exitward[CELL_RELAY]);
  tt_u64_op(1, OP_EQ, cell_stats->removed_cells_exitward[CELL_RELAY]);

 done:
  tor_free(cell_stats);
  tor_free(or_circ);
}

static void
test_cntev_append_cell_stats(void *arg)
{
  smartlist_t *event_parts;
  char *cp = NULL;
  const char *key = "Z";
  uint64_t include_if_non_zero[CELL_COMMAND_MAX_ + 1],
           number_to_include[CELL_COMMAND_MAX_ + 1];
  (void)arg;

  event_parts = smartlist_new();
  memset(include_if_non_zero, 0,
         (CELL_COMMAND_MAX_ + 1) * sizeof(uint64_t));
  memset(number_to_include, 0,
         (CELL_COMMAND_MAX_ + 1) * sizeof(uint64_t));

  /* All array entries empty. */
  append_cell_stats_by_command(event_parts, key,
                               include_if_non_zero,
                               number_to_include);
  tt_int_op(0, OP_EQ, smartlist_len(event_parts));

  /* There's a RELAY cell to include, but the corresponding field in
   * include_if_non_zero is still zero. */
  number_to_include[CELL_RELAY] = 1;
  append_cell_stats_by_command(event_parts, key,
                               include_if_non_zero,
                               number_to_include);
  tt_int_op(0, OP_EQ, smartlist_len(event_parts));

  /* Now include single RELAY cell. */
  include_if_non_zero[CELL_RELAY] = 2;
  append_cell_stats_by_command(event_parts, key,
                               include_if_non_zero,
                               number_to_include);
  cp = smartlist_pop_last(event_parts);
  tt_str_op("Z=relay:1", OP_EQ, cp);
  tor_free(cp);

  /* Add four CREATE cells. */
  include_if_non_zero[CELL_CREATE] = 3;
  number_to_include[CELL_CREATE] = 4;
  append_cell_stats_by_command(event_parts, key,
                               include_if_non_zero,
                               number_to_include);
  cp = smartlist_pop_last(event_parts);
  tt_str_op("Z=create:4,relay:1", OP_EQ, cp);

 done:
  tor_free(cp);
  smartlist_free(event_parts);
}

static void
test_cntev_format_cell_stats(void *arg)
{
  char *event_string = NULL;
  origin_circuit_t *ocirc = NULL;
  or_circuit_t *or_circ = NULL;
  cell_stats_t *cell_stats = NULL;
  channel_tls_t *n_chan=NULL, *p_chan=NULL;
  (void)arg;

  n_chan = tor_malloc_zero(sizeof(channel_tls_t));
  n_chan->base_.global_identifier = 1;

  ocirc = tor_malloc_zero(sizeof(origin_circuit_t));
  ocirc->base_.magic = ORIGIN_CIRCUIT_MAGIC;
  ocirc->base_.purpose = CIRCUIT_PURPOSE_C_GENERAL;
  ocirc->global_identifier = 2;
  ocirc->base_.n_circ_id = 3;
  ocirc->base_.n_chan = &(n_chan->base_);

  /* Origin circuit was completely idle. */
  cell_stats = tor_malloc_zero(sizeof(cell_stats_t));
  format_cell_stats(&event_string, TO_CIRCUIT(ocirc), cell_stats);
  tt_str_op("ID=2 OutboundQueue=3 OutboundConn=1", OP_EQ, event_string);
  tor_free(event_string);

  /* Origin circuit had 4 RELAY cells added to its exitward queue. */
  cell_stats->added_cells_exitward[CELL_RELAY] = 4;
  format_cell_stats(&event_string, TO_CIRCUIT(ocirc), cell_stats);
  tt_str_op("ID=2 OutboundQueue=3 OutboundConn=1 OutboundAdded=relay:4",
            OP_EQ, event_string);
  tor_free(event_string);

  /* Origin circuit also had 5 CREATE2 cells added to its exitward
   * queue. */
  cell_stats->added_cells_exitward[CELL_CREATE2] = 5;
  format_cell_stats(&event_string, TO_CIRCUIT(ocirc), cell_stats);
  tt_str_op("ID=2 OutboundQueue=3 OutboundConn=1 OutboundAdded=relay:4,"
            "create2:5", OP_EQ, event_string);
  tor_free(event_string);

  /* Origin circuit also had 7 RELAY cells removed from its exitward queue
   * which together spent 6 msec in the queue. */
  cell_stats->total_time_exitward[CELL_RELAY] = 6;
  cell_stats->removed_cells_exitward[CELL_RELAY] = 7;
  format_cell_stats(&event_string, TO_CIRCUIT(ocirc), cell_stats);
  tt_str_op("ID=2 OutboundQueue=3 OutboundConn=1 OutboundAdded=relay:4,"
            "create2:5 OutboundRemoved=relay:7 OutboundTime=relay:6",
            OP_EQ, event_string);
  tor_free(event_string);

  p_chan = tor_malloc_zero(sizeof(channel_tls_t));
  p_chan->base_.global_identifier = 2;

  or_circ = tor_malloc_zero(sizeof(or_circuit_t));
  or_circ->base_.magic = OR_CIRCUIT_MAGIC;
  or_circ->base_.purpose = CIRCUIT_PURPOSE_OR;
  or_circ->p_circ_id = 8;
  or_circ->p_chan = &(p_chan->base_);
  or_circ->base_.n_circ_id = 9;
  or_circ->base_.n_chan = &(n_chan->base_);

  tor_free(cell_stats);

  /* OR circuit was idle. */
  cell_stats = tor_malloc_zero(sizeof(cell_stats_t));
  format_cell_stats(&event_string, TO_CIRCUIT(or_circ), cell_stats);
  tt_str_op("InboundQueue=8 InboundConn=2 OutboundQueue=9 OutboundConn=1",
            OP_EQ, event_string);
  tor_free(event_string);

  /* OR circuit had 3 RELAY cells added to its appward queue. */
  cell_stats->added_cells_appward[CELL_RELAY] = 3;
  format_cell_stats(&event_string, TO_CIRCUIT(or_circ), cell_stats);
  tt_str_op("InboundQueue=8 InboundConn=2 InboundAdded=relay:3 "
            "OutboundQueue=9 OutboundConn=1", OP_EQ, event_string);
  tor_free(event_string);

  /* OR circuit had 7 RELAY cells removed from its appward queue which
   * together spent 6 msec in the queue. */
  cell_stats->total_time_appward[CELL_RELAY] = 6;
  cell_stats->removed_cells_appward[CELL_RELAY] = 7;
  format_cell_stats(&event_string, TO_CIRCUIT(or_circ), cell_stats);
  tt_str_op("InboundQueue=8 InboundConn=2 InboundAdded=relay:3 "
            "InboundRemoved=relay:7 InboundTime=relay:6 "
            "OutboundQueue=9 OutboundConn=1", OP_EQ, event_string);

 done:
  tor_free(cell_stats);
  tor_free(event_string);
  tor_free(or_circ);
  tor_free(ocirc);
  tor_free(p_chan);
  tor_free(n_chan);
}

static void
test_cntev_event_mask(void *arg)
{
  unsigned int test_event, selected_event;
  (void)arg;

  /* Check that nothing is interesting when no events are set */
  control_testing_set_global_event_mask(EVENT_MASK_NONE_);

  /* Check that nothing is interesting between EVENT_MIN_ and EVENT_MAX_ */
  for (test_event = EVENT_MIN_; test_event <= EVENT_MAX_; test_event++)
    tt_assert(!control_event_is_interesting(test_event));

  /* Check that nothing is interesting outside EVENT_MIN_ to EVENT_MAX_
   * This will break if control_event_is_interesting() checks its arguments */
  for (test_event = 0; test_event < EVENT_MIN_; test_event++)
    tt_assert(!control_event_is_interesting(test_event));
  for (test_event = EVENT_MAX_ + 1;
       test_event < EVENT_CAPACITY_;
       test_event++)
    tt_assert(!control_event_is_interesting(test_event));

  /* Check that all valid events are interesting when all events are set */
  control_testing_set_global_event_mask(EVENT_MASK_ALL_);

  /* Check that everything is interesting between EVENT_MIN_ and EVENT_MAX_ */
  for (test_event = EVENT_MIN_; test_event <= EVENT_MAX_; test_event++)
    tt_assert(control_event_is_interesting(test_event));

  /* Check that nothing is interesting outside EVENT_MIN_ to EVENT_MAX_
   * This will break if control_event_is_interesting() checks its arguments */
  for (test_event = 0; test_event < EVENT_MIN_; test_event++)
    tt_assert(!control_event_is_interesting(test_event));
  for (test_event = EVENT_MAX_ + 1;
       test_event < EVENT_CAPACITY_;
       test_event++)
    tt_assert(!control_event_is_interesting(test_event));

  /* Check that only that event is interesting when a single event is set */
  for (selected_event = EVENT_MIN_;
       selected_event <= EVENT_MAX_;
       selected_event++) {
    control_testing_set_global_event_mask(EVENT_MASK_(selected_event));

    /* Check that only this event is interesting
     * between EVENT_MIN_ and EVENT_MAX_ */
    for (test_event = EVENT_MIN_; test_event <= EVENT_MAX_; test_event++) {
      if (test_event == selected_event) {
        tt_assert(control_event_is_interesting(test_event));
      } else {
        tt_assert(!control_event_is_interesting(test_event));
      }
    }

    /* Check that nothing is interesting outside EVENT_MIN_ to EVENT_MAX_
     * This will break if control_event_is_interesting checks its arguments */
    for (test_event = 0; test_event < EVENT_MIN_; test_event++)
      tt_assert(!control_event_is_interesting(test_event));
    for (test_event = EVENT_MAX_ + 1;
         test_event < EVENT_CAPACITY_;
         test_event++)
      tt_assert(!control_event_is_interesting(test_event));
  }

  /* Check that only that event is not-interesting
   * when a single event is un-set */
  for (selected_event = EVENT_MIN_;
       selected_event <= EVENT_MAX_;
       selected_event++) {
    control_testing_set_global_event_mask(
                                          EVENT_MASK_ALL_
                                          & ~(EVENT_MASK_(selected_event))
                                          );

    /* Check that only this event is not-interesting
     * between EVENT_MIN_ and EVENT_MAX_ */
    for (test_event = EVENT_MIN_; test_event <= EVENT_MAX_; test_event++) {
      if (test_event == selected_event) {
        tt_assert(!control_event_is_interesting(test_event));
      } else {
        tt_assert(control_event_is_interesting(test_event));
      }
    }

    /* Check that nothing is interesting outside EVENT_MIN_ to EVENT_MAX_
     * This will break if control_event_is_interesting checks its arguments */
    for (test_event = 0; test_event < EVENT_MIN_; test_event++)
      tt_assert(!control_event_is_interesting(test_event));
    for (test_event = EVENT_MAX_ + 1;
         test_event < EVENT_CAPACITY_;
         test_event++)
      tt_assert(!control_event_is_interesting(test_event));
  }

 done:
  ;
}

static char *saved_event_str = NULL;

static void
mock_queue_control_event_string(uint16_t event, char *msg)
{
  (void)event;

  tor_free(saved_event_str);
  saved_event_str = msg;
}

/* Helper macro for checking bootstrap control event strings */
#define assert_bootmsg(s)                                               \
  tt_ptr_op(strstr(saved_event_str, "650 STATUS_CLIENT NOTICE "         \
                   "BOOTSTRAP PROGRESS=" s), OP_EQ, saved_event_str)

/* Test deferral of directory bootstrap messages (requesting_descriptors) */
static void
test_cntev_dirboot_defer_desc(void *arg)
{
  (void)arg;

  MOCK(queue_control_event_string, mock_queue_control_event_string);
  control_testing_set_global_event_mask(EVENT_MASK_(EVENT_STATUS_CLIENT));
  control_event_bootstrap(BOOTSTRAP_STATUS_STARTING, 0);
  assert_bootmsg("0 TAG=starting");
  /* This event should get deferred */
  control_event_boot_dir(BOOTSTRAP_STATUS_REQUESTING_DESCRIPTORS, 0);
  assert_bootmsg("0 TAG=starting");
  control_event_bootstrap(BOOTSTRAP_STATUS_CONN, 0);
  assert_bootmsg("5 TAG=conn");
  control_event_bootstrap(BOOTSTRAP_STATUS_HANDSHAKE, 0);
  assert_bootmsg("14 TAG=handshake");
  /* The deferred event should appear */
  control_event_boot_first_orconn();
  assert_bootmsg("45 TAG=requesting_descriptors");
 done:
  tor_free(saved_event_str);
  UNMOCK(queue_control_event_string);
}

/* Test deferral of directory bootstrap messages (conn_or) */
static void
test_cntev_dirboot_defer_orconn(void *arg)
{
  (void)arg;

  MOCK(queue_control_event_string, mock_queue_control_event_string);
  control_testing_set_global_event_mask(EVENT_MASK_(EVENT_STATUS_CLIENT));
  control_event_bootstrap(BOOTSTRAP_STATUS_STARTING, 0);
  assert_bootmsg("0 TAG=starting");
  /* This event should get deferred */
  control_event_boot_dir(BOOTSTRAP_STATUS_ENOUGH_DIRINFO, 0);
  assert_bootmsg("0 TAG=starting");
  control_event_bootstrap(BOOTSTRAP_STATUS_CONN, 0);
  assert_bootmsg("5 TAG=conn");
  control_event_bootstrap(BOOTSTRAP_STATUS_HANDSHAKE, 0);
  assert_bootmsg("14 TAG=handshake");
  /* The deferred event should appear */
  control_event_boot_first_orconn();
  assert_bootmsg("75 TAG=enough_dirinfo");
 done:
  tor_free(saved_event_str);
  UNMOCK(queue_control_event_string);
}

static void
setup_orconn_state(orconn_state_msg_t *msg, uint64_t gid, uint64_t chan,
                   int proxy_type)
{
  msg->gid = gid;
  msg->chan = chan;
  msg->proxy_type = proxy_type;
}

static void
send_orconn_state(const orconn_state_msg_t *msg_in, uint8_t state)
{
  orconn_state_msg_t *msg = tor_malloc(sizeof(*msg));

  *msg = *msg_in;
  msg->state = state;
  orconn_state_publish(msg);
}

static void
send_ocirc_chan(uint32_t gid, uint64_t chan, bool onehop)
{
  ocirc_chan_msg_t *msg = tor_malloc(sizeof(*msg));

  msg->gid = gid;
  msg->chan = chan;
  msg->onehop = onehop;
  ocirc_chan_publish(msg);
}

static void
test_cntev_orconn_state(void *arg)
{
  orconn_state_msg_t conn;
  memset(&conn, 0, sizeof(conn));

  (void)arg;
  MOCK(queue_control_event_string, mock_queue_control_event_string);
  control_testing_set_global_event_mask(EVENT_MASK_(EVENT_STATUS_CLIENT));
  setup_orconn_state(&conn, 1, 1, PROXY_NONE);

  send_orconn_state(&conn, OR_CONN_STATE_CONNECTING);
  send_ocirc_chan(1, 1, true);
  assert_bootmsg("5 TAG=conn");
  send_orconn_state(&conn, OR_CONN_STATE_TLS_HANDSHAKING);
  assert_bootmsg("10 TAG=conn_done");
  send_orconn_state(&conn, OR_CONN_STATE_OR_HANDSHAKING_V3);
  assert_bootmsg("14 TAG=handshake");
  send_orconn_state(&conn, OR_CONN_STATE_OPEN);
  assert_bootmsg("15 TAG=handshake_done");

  conn.gid = 2;
  conn.chan = 2;
  send_orconn_state(&conn, OR_CONN_STATE_CONNECTING);
  /* It doesn't know it's an origin circuit yet */
  assert_bootmsg("15 TAG=handshake_done");
  send_ocirc_chan(2, 2, false);
  assert_bootmsg("80 TAG=ap_conn");
  send_orconn_state(&conn, OR_CONN_STATE_TLS_HANDSHAKING);
  assert_bootmsg("85 TAG=ap_conn_done");
  send_orconn_state(&conn, OR_CONN_STATE_OR_HANDSHAKING_V3);
  assert_bootmsg("89 TAG=ap_handshake");
  send_orconn_state(&conn, OR_CONN_STATE_OPEN);
  assert_bootmsg("90 TAG=ap_handshake_done");

 done:
  tor_free(saved_event_str);
  UNMOCK(queue_control_event_string);
}

static void
test_cntev_orconn_state_pt(void *arg)
{
  orconn_state_msg_t conn;
  memset(&conn, 0, sizeof(conn));

  (void)arg;
  MOCK(queue_control_event_string, mock_queue_control_event_string);
  control_testing_set_global_event_mask(EVENT_MASK_(EVENT_STATUS_CLIENT));
  setup_orconn_state(&conn, 1, 1, PROXY_PLUGGABLE);
  send_ocirc_chan(1, 1, true);

  send_orconn_state(&conn, OR_CONN_STATE_CONNECTING);
  assert_bootmsg("1 TAG=conn_pt");
  send_orconn_state(&conn, OR_CONN_STATE_PROXY_HANDSHAKING);
  assert_bootmsg("2 TAG=conn_done_pt");
  send_orconn_state(&conn, OR_CONN_STATE_TLS_HANDSHAKING);
  assert_bootmsg("10 TAG=conn_done");
  send_orconn_state(&conn, OR_CONN_STATE_OR_HANDSHAKING_V3);
  assert_bootmsg("14 TAG=handshake");
  send_orconn_state(&conn, OR_CONN_STATE_OPEN);
  assert_bootmsg("15 TAG=handshake_done");

  send_ocirc_chan(2, 2, false);
  conn.gid = 2;
  conn.chan = 2;
  send_orconn_state(&conn, OR_CONN_STATE_CONNECTING);
  assert_bootmsg("76 TAG=ap_conn_pt");
  send_orconn_state(&conn, OR_CONN_STATE_PROXY_HANDSHAKING);
  assert_bootmsg("77 TAG=ap_conn_done_pt");

 done:
  tor_free(saved_event_str);
  UNMOCK(queue_control_event_string);
}

static void
test_cntev_orconn_state_proxy(void *arg)
{
  orconn_state_msg_t conn;
  memset(&conn, 0, sizeof(conn));

  (void)arg;
  MOCK(queue_control_event_string, mock_queue_control_event_string);
  control_testing_set_global_event_mask(EVENT_MASK_(EVENT_STATUS_CLIENT));
  setup_orconn_state(&conn, 1, 1, PROXY_CONNECT);
  send_ocirc_chan(1, 1, true);

  send_orconn_state(&conn, OR_CONN_STATE_CONNECTING);
  assert_bootmsg("3 TAG=conn_proxy");
  send_orconn_state(&conn, OR_CONN_STATE_PROXY_HANDSHAKING);
  assert_bootmsg("4 TAG=conn_done_proxy");
  send_orconn_state(&conn, OR_CONN_STATE_TLS_HANDSHAKING);
  assert_bootmsg("10 TAG=conn_done");
  send_orconn_state(&conn, OR_CONN_STATE_OR_HANDSHAKING_V3);
  assert_bootmsg("14 TAG=handshake");
  send_orconn_state(&conn, OR_CONN_STATE_OPEN);
  assert_bootmsg("15 TAG=handshake_done");

  send_ocirc_chan(2, 2, false);
  conn.gid = 2;
  conn.chan = 2;
  send_orconn_state(&conn, OR_CONN_STATE_CONNECTING);
  assert_bootmsg("78 TAG=ap_conn_proxy");
  send_orconn_state(&conn, OR_CONN_STATE_PROXY_HANDSHAKING);
  assert_bootmsg("79 TAG=ap_conn_done_proxy");

 done:
  tor_free(saved_event_str);
  UNMOCK(queue_control_event_string);
}

#define TEST(name, flags)                               \
  { #name, test_cntev_ ## name, flags, 0, NULL }

#define T_PUBSUB(name, setup)                                           \
  { #name, test_cntev_ ## name, TT_FORK, &helper_pubsub_setup, NULL }

struct testcase_t controller_event_tests[] = {
  TEST(sum_up_cell_stats, TT_FORK),
  TEST(append_cell_stats, TT_FORK),
  TEST(format_cell_stats, TT_FORK),
  TEST(event_mask, TT_FORK),
  T_PUBSUB(dirboot_defer_desc, TT_FORK),
  T_PUBSUB(dirboot_defer_orconn, TT_FORK),
  T_PUBSUB(orconn_state, TT_FORK),
  T_PUBSUB(orconn_state_pt, TT_FORK),
  T_PUBSUB(orconn_state_proxy, TT_FORK),
  END_OF_TESTCASES
};
