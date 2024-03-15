/* Copyright (c) 2013-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

#define CHANNEL_OBJECT_PRIVATE
#define CHANNEL_FILE_PRIVATE
#include "core/or/or.h"
#include "core/or/channel.h"
/* For channel_note_destroy_not_pending */
#define CIRCUITLIST_PRIVATE
#include "core/or/circuitlist.h"
#include "core/or/circuitmux.h"
#include "core/or/circuitmux_ewma.h"
/* For var_cell_free */
#include "core/or/connection_or.h"
#include "lib/crypt_ops/crypto_rand.h"
/* For packed_cell stuff */
#define RELAY_PRIVATE
#include "core/or/relay.h"
/* For channel_tls_t object and private functions. */
#define CHANNEL_OBJECT_PRIVATE
#define CHANNELTLS_PRIVATE
#include "core/or/channeltls.h"
/* For init/free stuff */
#include "core/or/scheduler.h"
#include "feature/nodelist/networkstatus.h"

#include "core/or/cell_st.h"
#include "feature/nodelist/networkstatus_st.h"
#include "core/or/origin_circuit_st.h"
#include "feature/nodelist/routerstatus_st.h"
#include "core/or/var_cell_st.h"
#include "core/or/or_connection_st.h"
#include "lib/net/inaddr.h"

/* Test suite stuff */
#include "test/log_test_helpers.h"
#include "test/test.h"
#include "test/fakechans.h"

static int test_chan_accept_cells = 0;
static int test_chan_fixed_cells_recved = 0;
static cell_t * test_chan_last_seen_fixed_cell_ptr = NULL;
static int test_cells_written = 0;
static int test_doesnt_want_writes_count = 0;
static int test_dumpstats_calls = 0;
static int test_has_waiting_cells_count = 0;
static int test_releases_count = 0;
static channel_t *dump_statistics_mock_target = NULL;
static int dump_statistics_mock_matches = 0;
static int test_close_called = 0;
static int test_chan_should_be_canonical = 0;
static int test_chan_should_match_target = 0;
static int test_chan_listener_close_fn_called = 0;
static int test_chan_listener_fn_called = 0;

static const char *
chan_test_describe_transport(channel_t *ch)
{
  tt_ptr_op(ch, OP_NE, NULL);

 done:
  return "Fake channel for unit tests";
}

/**
 * Mock for channel_dump_statistics(); if the channel matches the
 * target, bump a counter - otherwise ignore.
 */

static void
chan_test_channel_dump_statistics_mock(channel_t *chan, int severity)
{
  tt_ptr_op(chan, OP_NE, NULL);

  (void)severity;

  if (chan != NULL && chan == dump_statistics_mock_target) {
    ++dump_statistics_mock_matches;
  }

 done:
  return;
}

/*
 * Handle an incoming fixed-size cell for unit tests
 */

static void
chan_test_cell_handler(channel_t *chan, cell_t *cell)
{
  tt_assert(chan);
  tt_assert(cell);

  test_chan_last_seen_fixed_cell_ptr = cell;
  ++test_chan_fixed_cells_recved;

 done:
  return;
}

/*
 * Fake transport-specific stats call
 */

static void
chan_test_dumpstats(channel_t *ch, int severity)
{
  tt_ptr_op(ch, OP_NE, NULL);

  (void)severity;

  ++test_dumpstats_calls;

 done:
  return;
}

static void
chan_test_close(channel_t *ch)
{
  tt_assert(ch);

  ++test_close_called;

 done:
  return;
}

/*
 * Close a channel through the error path
 */

static void
chan_test_error(channel_t *ch)
{
  tt_assert(ch);
  tt_assert(!(ch->state == CHANNEL_STATE_CLOSING ||
                ch->state == CHANNEL_STATE_ERROR ||
                ch->state == CHANNEL_STATE_CLOSED));

  channel_close_for_error(ch);

 done:
  return;
}

/*
 * Finish closing a channel from CHANNEL_STATE_CLOSING
 */

static void
chan_test_finish_close(channel_t *ch)
{
  tt_assert(ch);
  tt_assert(ch->state == CHANNEL_STATE_CLOSING);

  channel_closed(ch);

 done:
  return;
}

static const char *
chan_test_describe_peer(const channel_t *ch)
{
  tt_assert(ch);

 done:
  return "Fake channel for unit tests; no real endpoint";
}

static int
chan_test_get_remote_addr(const channel_t *ch, tor_addr_t *out)
{
  (void)ch;
  tor_addr_from_ipv4h(out, 0x7f000001);
  return 1;
}

static int
chan_test_num_cells_writeable(channel_t *ch)
{
  tt_assert(ch);

 done:
  return 32;
}

static int
chan_test_write_packed_cell(channel_t *ch,
                            packed_cell_t *packed_cell)
{
  int rv = 0;

  tt_assert(ch);
  tt_assert(packed_cell);

  if (test_chan_accept_cells) {
    /* Free the cell and bump the counter */
    ++test_cells_written;
    rv = 1;
  }
  /* else return 0, we didn't accept it */

 done:
  return rv;
}

static int
chan_test_write_var_cell(channel_t *ch, var_cell_t *var_cell)
{
  int rv = 0;

  tt_assert(ch);
  tt_assert(var_cell);

  if (test_chan_accept_cells) {
    /* Free the cell and bump the counter */
    var_cell_free(var_cell);
    ++test_cells_written;
    rv = 1;
  }
  /* else return 0, we didn't accept it */

 done:
  return rv;
}

/**
 * Fill out c with a new fake cell for test suite use
 */

void
make_fake_cell(cell_t *c)
{
  tt_ptr_op(c, OP_NE, NULL);

  c->circ_id = 1;
  c->command = CELL_RELAY;
  memset(c->payload, 0, CELL_PAYLOAD_SIZE);

 done:
  return;
}

/**
 * Fill out c with a new fake var_cell for test suite use
 */

void
make_fake_var_cell(var_cell_t *c)
{
  tt_ptr_op(c, OP_NE, NULL);

  c->circ_id = 1;
  c->command = CELL_VERSIONS;
  c->payload_len = CELL_PAYLOAD_SIZE / 2;
  memset(c->payload, 0, c->payload_len);

 done:
  return;
}

/**
 * Set up a new fake channel for the test suite
 */

channel_t *
new_fake_channel(void)
{
  channel_t *chan = tor_malloc_zero(sizeof(channel_t));
  channel_init(chan);

  chan->close = chan_test_close;
  chan->num_cells_writeable = chan_test_num_cells_writeable;
  chan->describe_peer = chan_test_describe_peer;
  chan->get_remote_addr = chan_test_get_remote_addr;
  chan->write_packed_cell = chan_test_write_packed_cell;
  chan->write_var_cell = chan_test_write_var_cell;
  chan->state = CHANNEL_STATE_OPEN;

  chan->cmux = circuitmux_alloc();
  circuitmux_set_policy(chan->cmux, &ewma_policy);

  return chan;
}

void
free_fake_channel(channel_t *chan)
{
  if (! chan)
    return;

  if (chan->cmux)
    circuitmux_free(chan->cmux);

  tor_free(chan);
}

/**
 * Counter query for scheduler_channel_has_waiting_cells_mock()
 */

int
get_mock_scheduler_has_waiting_cells_count(void)
{
  return test_has_waiting_cells_count;
}

/**
 * Mock for scheduler_channel_has_waiting_cells()
 */

void
scheduler_channel_has_waiting_cells_mock(channel_t *ch)
{
  (void)ch;

  /* Increment counter */
  ++test_has_waiting_cells_count;

  return;
}

static void
scheduler_channel_doesnt_want_writes_mock(channel_t *ch)
{
  (void)ch;

  /* Increment counter */
  ++test_doesnt_want_writes_count;

  return;
}

/**
 * Mock for scheduler_release_channel()
 */

void
scheduler_release_channel_mock(channel_t *ch)
{
  (void)ch;

  /* Increment counter */
  ++test_releases_count;

  return;
}

static int
test_chan_is_canonical(channel_t *chan)
{
  tor_assert(chan);

  if (test_chan_should_be_canonical) {
    return 1;
  }
  return 0;
}

static int
test_chan_matches_target(channel_t *chan, const tor_addr_t *target)
{
  (void) chan;
  (void) target;

  if (test_chan_should_match_target) {
    return 1;
  }
  return 0;
}

static void
test_chan_listener_close(channel_listener_t *chan)
{
  (void) chan;
  ++test_chan_listener_close_fn_called;
  return;
}

static void
test_chan_listener_fn(channel_listener_t *listener, channel_t *chan)
{
  (void) listener;
  (void) chan;

  ++test_chan_listener_fn_called;
  return;
}

static const char *
test_chan_listener_describe_transport(channel_listener_t *chan)
{
  (void) chan;
  return "Fake listener channel.";
}

/**
 * Test for channel_dumpstats() and limited test for
 * channel_dump_statistics()
 */

static void
test_channel_dumpstats(void *arg)
{
  channel_t *ch = NULL;
  cell_t *cell = NULL;
  packed_cell_t *p_cell = NULL;
  int old_count;

  (void)arg;

  /* Mock these for duration of the test */
  MOCK(scheduler_channel_doesnt_want_writes,
       scheduler_channel_doesnt_want_writes_mock);
  MOCK(scheduler_release_channel,
       scheduler_release_channel_mock);

  /* Set up a new fake channel */
  ch = new_fake_channel();
  tt_assert(ch);

  /* Try to register it */
  channel_register(ch);
  tt_assert(ch->registered);

  /* Set up mock */
  dump_statistics_mock_target = ch;
  dump_statistics_mock_matches = 0;
  MOCK(channel_dump_statistics,
       chan_test_channel_dump_statistics_mock);

  /* Call channel_dumpstats() */
  channel_dumpstats(LOG_DEBUG);

  /* Assert that we hit the mock */
  tt_int_op(dump_statistics_mock_matches, OP_EQ, 1);

  /* Close the channel */
  channel_mark_for_close(ch);
  tt_int_op(ch->state, OP_EQ, CHANNEL_STATE_CLOSING);
  chan_test_finish_close(ch);
  tt_int_op(ch->state, OP_EQ, CHANNEL_STATE_CLOSED);

  /* Try again and hit the finished channel */
  channel_dumpstats(LOG_DEBUG);
  tt_int_op(dump_statistics_mock_matches, OP_EQ, 2);

  channel_run_cleanup();
  ch = NULL;

  /* Now we should hit nothing */
  channel_dumpstats(LOG_DEBUG);
  tt_int_op(dump_statistics_mock_matches, OP_EQ, 2);

  /* Unmock */
  UNMOCK(channel_dump_statistics);
  dump_statistics_mock_target = NULL;
  dump_statistics_mock_matches = 0;

  /* Now make another channel */
  ch = new_fake_channel();
  tt_assert(ch);
  channel_register(ch);
  tt_int_op(ch->registered, OP_EQ, 1);
  /* Lie about its age so dumpstats gets coverage for rate calculations */
  ch->timestamp_created = time(NULL) - 30;
  tt_int_op(ch->timestamp_created, OP_GT, 0);
  tt_int_op(time(NULL), OP_GT, ch->timestamp_created);

  /* Put cells through it both ways to make the counters non-zero */
  p_cell = packed_cell_new();
  test_chan_accept_cells = 1;
  old_count = test_cells_written;
  channel_write_packed_cell(ch, p_cell);
  tt_int_op(test_cells_written, OP_EQ, old_count + 1);
  tt_u64_op(ch->n_bytes_xmitted, OP_GT, 0);
  tt_u64_op(ch->n_cells_xmitted, OP_GT, 0);

  /* Receive path */
  channel_set_cell_handlers(ch,
                            chan_test_cell_handler);
  tt_ptr_op(channel_get_cell_handler(ch), OP_EQ, chan_test_cell_handler);
  cell = tor_malloc_zero(sizeof(*cell));
  old_count = test_chan_fixed_cells_recved;
  channel_process_cell(ch, cell);
  tt_int_op(test_chan_fixed_cells_recved, OP_EQ, old_count + 1);
  tt_u64_op(ch->n_bytes_recved, OP_GT, 0);
  tt_u64_op(ch->n_cells_recved, OP_GT, 0);

  /* Test channel_dump_statistics */
  ch->describe_transport = chan_test_describe_transport;
  ch->dumpstats = chan_test_dumpstats;
  test_chan_should_be_canonical = 1;
  ch->is_canonical = test_chan_is_canonical;
  old_count = test_dumpstats_calls;
  channel_dump_statistics(ch, LOG_DEBUG);
  tt_int_op(test_dumpstats_calls, OP_EQ, old_count + 1);

  /* Close the channel */
  channel_mark_for_close(ch);
  tt_int_op(ch->state, OP_EQ, CHANNEL_STATE_CLOSING);
  chan_test_finish_close(ch);
  tt_int_op(ch->state, OP_EQ, CHANNEL_STATE_CLOSED);
  channel_run_cleanup();
  ch = NULL;

 done:
  free_fake_channel(ch);
  tor_free(cell);

  UNMOCK(scheduler_channel_doesnt_want_writes);
  UNMOCK(scheduler_release_channel);

  return;
}

/* Test outbound cell. The callstack is:
 *  channel_flush_some_cells()
 *   -> channel_flush_from_first_active_circuit()
 *     -> channel_write_packed_cell()
 *       -> write_packed_cell()
 *         -> chan->write_packed_cell() fct ptr.
 *
 * This test goes from a cell in a circuit up to the channel write handler
 * that should put them on the connection outbuf. */
static void
test_channel_outbound_cell(void *arg)
{
  int old_count;
  channel_t *chan = NULL;
  packed_cell_t *p_cell = NULL, *p_cell2 = NULL;
  origin_circuit_t *circ = NULL;
  cell_queue_t *queue;

  (void) arg;

  /* Set the test time to be mocked, since this test assumes that no
   * time will pass, ewma values will not need to be re-scaled, and so on */
  monotime_enable_test_mocking();
  monotime_set_mock_time_nsec(UINT64_C(1000000000) * 12345);

  cmux_ewma_set_options(NULL,NULL);

  /* The channel will be freed so we need to hijack this so the scheduler
   * doesn't get confused. */
  MOCK(scheduler_release_channel, scheduler_release_channel_mock);

  /* Accept cells to lower layer */
  test_chan_accept_cells = 1;

  /* Setup a valid circuit to queue a cell. */
  circ = origin_circuit_new();
  tt_assert(circ);
  /* Circuit needs an origin purpose to be considered origin. */
  TO_CIRCUIT(circ)->purpose = CIRCUIT_PURPOSE_C_GENERAL;
  TO_CIRCUIT(circ)->n_circ_id = 42;
  /* This is the outbound test so use the next channel queue. */
  queue = &TO_CIRCUIT(circ)->n_chan_cells;
  /* Setup packed cell to queue on the circuit. */
  p_cell = packed_cell_new();
  tt_assert(p_cell);
  p_cell2 = packed_cell_new();
  tt_assert(p_cell2);
  /* Setup a channel to put the circuit on. */
  chan = new_fake_channel();
  tt_assert(chan);
  chan->state = CHANNEL_STATE_OPENING;
  channel_change_state_open(chan);
  /* Outbound channel. */
  channel_mark_outgoing(chan);
  /* Try to register it so we can clean it through the channel cleanup
   * process. */
  channel_register(chan);
  tt_int_op(chan->registered, OP_EQ, 1);
  /* Set EWMA policy so we can pick it when flushing. */
  circuitmux_set_policy(chan->cmux, &ewma_policy);
  tt_ptr_op(circuitmux_get_policy(chan->cmux), OP_EQ, &ewma_policy);

  /* Register circuit to the channel circid map which will attach the circuit
   * to the channel's cmux as well. */
  circuit_set_n_circid_chan(TO_CIRCUIT(circ), 42, chan);
  tt_int_op(channel_num_circuits(chan), OP_EQ, 1);
  /* Test the cmux state. */
  tt_int_op(circuitmux_is_circuit_attached(chan->cmux, TO_CIRCUIT(circ)),
            OP_EQ, 1);

  /* Flush the channel without any cell on it. */
  old_count = test_cells_written;
  ssize_t flushed = channel_flush_some_cells(chan, 1);
  tt_i64_op(flushed, OP_EQ, 0);
  tt_int_op(test_cells_written, OP_EQ, old_count);
  tt_int_op(channel_more_to_flush(chan), OP_EQ, 0);
  tt_int_op(circuitmux_num_active_circuits(chan->cmux), OP_EQ, 0);
  tt_int_op(circuitmux_num_cells(chan->cmux), OP_EQ, 0);
  tt_int_op(circuitmux_is_circuit_active(chan->cmux, TO_CIRCUIT(circ)),
            OP_EQ, 0);
  tt_u64_op(chan->n_cells_xmitted, OP_EQ, 0);
  tt_u64_op(chan->n_bytes_xmitted, OP_EQ, 0);

  /* Queue cell onto the next queue that is the outbound direction. Than
   * update its cmux so the circuit can be picked when flushing cells. */
  cell_queue_append(queue, p_cell);
  p_cell = NULL;
  tt_int_op(queue->n, OP_EQ, 1);
  cell_queue_append(queue, p_cell2);
  p_cell2 = NULL;
  tt_int_op(queue->n, OP_EQ, 2);

  update_circuit_on_cmux(TO_CIRCUIT(circ), CELL_DIRECTION_OUT);
  tt_int_op(circuitmux_num_active_circuits(chan->cmux), OP_EQ, 1);
  tt_int_op(circuitmux_num_cells(chan->cmux), OP_EQ, 2);
  tt_int_op(circuitmux_is_circuit_active(chan->cmux, TO_CIRCUIT(circ)),
            OP_EQ, 1);

  /* From this point on, we have a queued cell on an active circuit attached
   * to the channel's cmux. */

  /* Flush the first cell. This is going to go down the call stack. */
  old_count = test_cells_written;
  flushed = channel_flush_some_cells(chan, 1);
  tt_i64_op(flushed, OP_EQ, 1);
  tt_int_op(test_cells_written, OP_EQ, old_count + 1);
  tt_int_op(circuitmux_num_cells(chan->cmux), OP_EQ, 1);
  tt_int_op(channel_more_to_flush(chan), OP_EQ, 1);
  /* Circuit should remain active because there is a second cell queued. */
  tt_int_op(circuitmux_is_circuit_active(chan->cmux, TO_CIRCUIT(circ)),
            OP_EQ, 1);
  /* Should still be attached. */
  tt_int_op(circuitmux_is_circuit_attached(chan->cmux, TO_CIRCUIT(circ)),
            OP_EQ, 1);
  tt_u64_op(chan->n_cells_xmitted, OP_EQ, 1);
  tt_u64_op(chan->n_bytes_xmitted, OP_EQ, get_cell_network_size(0));

  /* Flush second cell. This is going to go down the call stack. */
  old_count = test_cells_written;
  flushed = channel_flush_some_cells(chan, 1);
  tt_i64_op(flushed, OP_EQ, 1);
  tt_int_op(test_cells_written, OP_EQ, old_count + 1);
  tt_int_op(circuitmux_num_cells(chan->cmux), OP_EQ, 0);
  tt_int_op(channel_more_to_flush(chan), OP_EQ, 0);
  /* No more cells should make the circuit inactive. */
  tt_int_op(circuitmux_is_circuit_active(chan->cmux, TO_CIRCUIT(circ)),
            OP_EQ, 0);
  /* Should still be attached. */
  tt_int_op(circuitmux_is_circuit_attached(chan->cmux, TO_CIRCUIT(circ)),
            OP_EQ, 1);
  tt_u64_op(chan->n_cells_xmitted, OP_EQ, 2);
  tt_u64_op(chan->n_bytes_xmitted, OP_EQ, get_cell_network_size(0) * 2);

 done:
  if (circ) {
    circuit_free_(TO_CIRCUIT(circ));
  }
  tor_free(p_cell);
  channel_free_all();
  UNMOCK(scheduler_release_channel);
  monotime_disable_test_mocking();
}

/* Test inbound cell. The callstack is:
 *  channel_process_cell()
 *    -> chan->cell_handler()
 *
 * This test is about checking if we can process an inbound cell down to the
 * channel handler. */
static void
test_channel_inbound_cell(void *arg)
{
  channel_t *chan = NULL;
  cell_t *cell = NULL;
  int old_count;

  (void) arg;

  /* The channel will be freed so we need to hijack this so the scheduler
   * doesn't get confused. */
  MOCK(scheduler_release_channel, scheduler_release_channel_mock);

  /* Accept cells to lower layer */
  test_chan_accept_cells = 1;

  chan = new_fake_channel();
  tt_assert(chan);
  /* Start it off in OPENING */
  chan->state = CHANNEL_STATE_OPENING;

  /* Try to register it */
  channel_register(chan);
  tt_int_op(chan->registered, OP_EQ, 1);

  /* Open it */
  channel_change_state_open(chan);
  tt_int_op(chan->state, OP_EQ, CHANNEL_STATE_OPEN);
  tt_int_op(chan->has_been_open, OP_EQ, 1);

  /* Receive a cell now. */
  cell = tor_malloc_zero(sizeof(*cell));
  make_fake_cell(cell);
  old_count = test_chan_fixed_cells_recved;
  channel_process_cell(chan, cell);
  tt_int_op(test_chan_fixed_cells_recved, OP_EQ, old_count);
  tt_assert(monotime_coarse_is_zero(&chan->timestamp_xfer));
  tt_u64_op(chan->timestamp_active, OP_EQ, 0);
  tt_u64_op(chan->timestamp_recv, OP_EQ, 0);

  /* Setup incoming cell handlers. We don't care about var cell, the channel
   * layers is not handling those. */
  channel_set_cell_handlers(chan, chan_test_cell_handler);
  tt_ptr_op(chan->cell_handler, OP_EQ, chan_test_cell_handler);
  /* Now process the cell, we should see it. */
  old_count = test_chan_fixed_cells_recved;
  channel_process_cell(chan, cell);
  tt_int_op(test_chan_fixed_cells_recved, OP_EQ, old_count + 1);
  /* We should have a series of timestamp set. */
  tt_assert(!monotime_coarse_is_zero(&chan->timestamp_xfer));
  tt_u64_op(chan->timestamp_active, OP_NE, 0);
  tt_u64_op(chan->timestamp_recv, OP_NE, 0);
  tt_assert(monotime_coarse_is_zero(&chan->next_padding_time));
  tt_u64_op(chan->n_cells_recved, OP_EQ, 1);
  tt_u64_op(chan->n_bytes_recved, OP_EQ, get_cell_network_size(0));

  /* Close it */
  old_count = test_close_called;
  channel_mark_for_close(chan);
  tt_int_op(chan->state, OP_EQ, CHANNEL_STATE_CLOSING);
  tt_int_op(chan->reason_for_closing, OP_EQ, CHANNEL_CLOSE_REQUESTED);
  tt_int_op(test_close_called, OP_EQ, old_count + 1);

  /* This closes the channel so it calls in the scheduler, make sure of it. */
  old_count = test_releases_count;
  chan_test_finish_close(chan);
  tt_int_op(test_releases_count, OP_EQ, old_count + 1);
  tt_int_op(chan->state, OP_EQ, CHANNEL_STATE_CLOSED);

  /* The channel will be free, lets make sure it is not accessible. */
  uint64_t chan_id = chan->global_identifier;
  tt_ptr_op(channel_find_by_global_id(chan_id), OP_EQ, chan);
  channel_run_cleanup();
  chan = channel_find_by_global_id(chan_id);
  tt_assert(chan == NULL);

 done:
  tor_free(cell);
  UNMOCK(scheduler_release_channel);
}

/**
 * Normal channel lifecycle test:
 *
 * OPENING->OPEN->MAINT->OPEN->CLOSING->CLOSED
 */

static void
test_channel_lifecycle(void *arg)
{
  channel_t *ch1 = NULL, *ch2 = NULL;
  packed_cell_t *p_cell = NULL;
  int old_count, init_doesnt_want_writes_count;
  int init_releases_count;

  (void)arg;

  /* Mock these for the whole lifecycle test */
  MOCK(scheduler_channel_doesnt_want_writes,
       scheduler_channel_doesnt_want_writes_mock);
  MOCK(scheduler_release_channel,
       scheduler_release_channel_mock);

  /* Cache some initial counter values */
  init_doesnt_want_writes_count = test_doesnt_want_writes_count;
  init_releases_count = test_releases_count;

  /* Accept cells to lower layer */
  test_chan_accept_cells = 1;

  ch1 = new_fake_channel();
  tt_assert(ch1);
  /* Start it off in OPENING */
  ch1->state = CHANNEL_STATE_OPENING;

  /* Try to register it */
  channel_register(ch1);
  tt_assert(ch1->registered);

  /* Try to write a cell through (should queue) */
  p_cell = packed_cell_new();
  old_count = test_cells_written;
  channel_write_packed_cell(ch1, p_cell);
  tt_int_op(old_count, OP_EQ, test_cells_written);

  /* Move it to OPEN and flush */
  channel_change_state_open(ch1);

/* Get another one */
  ch2 = new_fake_channel();
  tt_assert(ch2);
  ch2->state = CHANNEL_STATE_OPENING;

  /* Register */
  channel_register(ch2);
  tt_assert(ch2->registered);

  /* Check counters */
  tt_int_op(test_doesnt_want_writes_count, OP_EQ,
            init_doesnt_want_writes_count);
  tt_int_op(test_releases_count, OP_EQ, init_releases_count);

  /* Move ch1 to MAINT */
  channel_change_state(ch1, CHANNEL_STATE_MAINT);
  tt_int_op(test_doesnt_want_writes_count, OP_EQ,
            init_doesnt_want_writes_count + 1);
  tt_int_op(test_releases_count, OP_EQ, init_releases_count);

  /* Move ch2 to OPEN */
  channel_change_state_open(ch2);
  tt_int_op(test_doesnt_want_writes_count, OP_EQ,
            init_doesnt_want_writes_count + 1);
  tt_int_op(test_releases_count, OP_EQ, init_releases_count);

  /* Move ch1 back to OPEN */
  channel_change_state_open(ch1);
  tt_int_op(test_doesnt_want_writes_count, OP_EQ,
            init_doesnt_want_writes_count + 1);
  tt_int_op(test_releases_count, OP_EQ, init_releases_count);

  /* Mark ch2 for close */
  channel_mark_for_close(ch2);
  tt_int_op(ch2->state, OP_EQ, CHANNEL_STATE_CLOSING);
  tt_int_op(test_doesnt_want_writes_count, OP_EQ,
            init_doesnt_want_writes_count + 1);
  tt_int_op(test_releases_count, OP_EQ, init_releases_count + 1);

  /* Shut down channels */
  channel_free_all();
  ch1 = ch2 = NULL;
  tt_int_op(test_doesnt_want_writes_count, OP_EQ,
            init_doesnt_want_writes_count + 1);
  /* channel_free() calls scheduler_release_channel() */
  tt_int_op(test_releases_count, OP_EQ, init_releases_count + 4);

 done:
  free_fake_channel(ch1);
  free_fake_channel(ch2);

  UNMOCK(scheduler_channel_doesnt_want_writes);
  UNMOCK(scheduler_release_channel);
}

/**
 * Weird channel lifecycle test:
 *
 * OPENING->CLOSING->CLOSED
 * OPENING->OPEN->CLOSING->ERROR
 * OPENING->OPEN->MAINT->CLOSING->CLOSED
 * OPENING->OPEN->MAINT->CLOSING->ERROR
 */

static void
test_channel_lifecycle_2(void *arg)
{
  channel_t *ch = NULL;

  (void)arg;

  /* Mock these for the whole lifecycle test */
  MOCK(scheduler_channel_doesnt_want_writes,
       scheduler_channel_doesnt_want_writes_mock);
  MOCK(scheduler_release_channel,
       scheduler_release_channel_mock);

  /* Accept cells to lower layer */
  test_chan_accept_cells = 1;

  ch = new_fake_channel();
  tt_assert(ch);
  /* Start it off in OPENING */
  ch->state = CHANNEL_STATE_OPENING;

  /* Try to register it */
  channel_register(ch);
  tt_assert(ch->registered);

  /* Try to close it */
  channel_mark_for_close(ch);
  tt_int_op(ch->state, OP_EQ, CHANNEL_STATE_CLOSING);

  /* Finish closing it */
  chan_test_finish_close(ch);
  tt_int_op(ch->state, OP_EQ, CHANNEL_STATE_CLOSED);
  channel_run_cleanup();
  ch = NULL;

  /* Now try OPENING->OPEN->CLOSING->ERROR */
  ch = new_fake_channel();
  tt_assert(ch);
  ch->state = CHANNEL_STATE_OPENING;
  channel_register(ch);
  tt_assert(ch->registered);

  /* Finish opening it */
  channel_change_state_open(ch);

  /* Error exit from lower layer */
  chan_test_error(ch);
  tt_int_op(ch->state, OP_EQ, CHANNEL_STATE_CLOSING);
  chan_test_finish_close(ch);
  tt_int_op(ch->state, OP_EQ, CHANNEL_STATE_ERROR);
  channel_run_cleanup();
  ch = NULL;

  /* OPENING->OPEN->MAINT->CLOSING->CLOSED close from maintenance state */
  ch = new_fake_channel();
  tt_assert(ch);
  ch->state = CHANNEL_STATE_OPENING;
  channel_register(ch);
  tt_assert(ch->registered);

  /* Finish opening it */
  channel_change_state_open(ch);
  tt_int_op(ch->state, OP_EQ, CHANNEL_STATE_OPEN);

  /* Go to maintenance state */
  channel_change_state(ch, CHANNEL_STATE_MAINT);
  tt_int_op(ch->state, OP_EQ, CHANNEL_STATE_MAINT);

  /* Lower layer close */
  channel_mark_for_close(ch);
  tt_int_op(ch->state, OP_EQ, CHANNEL_STATE_CLOSING);

  /* Finish */
  chan_test_finish_close(ch);
  tt_int_op(ch->state, OP_EQ, CHANNEL_STATE_CLOSED);
  channel_run_cleanup();
  ch = NULL;

  /*
   * OPENING->OPEN->MAINT->CLOSING->CLOSED lower-layer close during
   * maintenance state
   */
  ch = new_fake_channel();
  tt_assert(ch);
  ch->state = CHANNEL_STATE_OPENING;
  channel_register(ch);
  tt_assert(ch->registered);

  /* Finish opening it */
  channel_change_state_open(ch);
  tt_int_op(ch->state, OP_EQ, CHANNEL_STATE_OPEN);

  /* Go to maintenance state */
  channel_change_state(ch, CHANNEL_STATE_MAINT);
  tt_int_op(ch->state, OP_EQ, CHANNEL_STATE_MAINT);

  /* Lower layer close */
  channel_close_from_lower_layer(ch);
  tt_int_op(ch->state, OP_EQ, CHANNEL_STATE_CLOSING);

  /* Finish */
  chan_test_finish_close(ch);
  tt_int_op(ch->state, OP_EQ, CHANNEL_STATE_CLOSED);
  channel_run_cleanup();
  ch = NULL;

  /* OPENING->OPEN->MAINT->CLOSING->ERROR */
  ch = new_fake_channel();
  tt_assert(ch);
  ch->state = CHANNEL_STATE_OPENING;
  channel_register(ch);
  tt_assert(ch->registered);

  /* Finish opening it */
  channel_change_state_open(ch);
  tt_int_op(ch->state, OP_EQ, CHANNEL_STATE_OPEN);

  /* Go to maintenance state */
  channel_change_state(ch, CHANNEL_STATE_MAINT);
  tt_int_op(ch->state, OP_EQ, CHANNEL_STATE_MAINT);

  /* Lower layer close */
  chan_test_error(ch);
  tt_int_op(ch->state, OP_EQ, CHANNEL_STATE_CLOSING);

  /* Finish */
  chan_test_finish_close(ch);
  tt_int_op(ch->state, OP_EQ, CHANNEL_STATE_ERROR);
  channel_run_cleanup();
  ch = NULL;

  /* Shut down channels */
  channel_free_all();

 done:
  tor_free(ch);

  UNMOCK(scheduler_channel_doesnt_want_writes);
  UNMOCK(scheduler_release_channel);

  return;
}

static void
test_channel_id_map(void *arg)
{
  (void)arg;
#define N_CHAN 6
  char rsa_id[N_CHAN][DIGEST_LEN];
  ed25519_public_key_t *ed_id[N_CHAN];
  channel_t *chan[N_CHAN];
  int i;
  ed25519_public_key_t ed_zero;
  memset(&ed_zero, 0, sizeof(ed_zero));

  tt_int_op(DIGEST_LEN, OP_EQ, sizeof(rsa_id[0])); // Do I remember C?

  for (i = 0; i < N_CHAN; ++i) {
    crypto_rand(rsa_id[i], DIGEST_LEN);
    ed_id[i] = tor_malloc_zero(sizeof(*ed_id[i]));
    crypto_rand((char*)ed_id[i]->pubkey, sizeof(ed_id[i]->pubkey));
  }

  /* For channel 3, have no Ed identity. */
  tor_free(ed_id[3]);

  /* Channel 2 and 4 have same ROSA identity */
  memcpy(rsa_id[4], rsa_id[2], DIGEST_LEN);

  /* Channel 2 and 4 and 5 have same RSA identity */
  memcpy(rsa_id[4], rsa_id[2], DIGEST_LEN);
  memcpy(rsa_id[5], rsa_id[2], DIGEST_LEN);

  /* Channels 2 and 5 have same Ed25519 identity */
  memcpy(ed_id[5], ed_id[2], sizeof(*ed_id[2]));

  for (i = 0; i < N_CHAN; ++i) {
    chan[i] = new_fake_channel();
    channel_register(chan[i]);
    channel_set_identity_digest(chan[i], rsa_id[i], ed_id[i]);
  }

  /* Lookup by RSA id only */
  tt_ptr_op(chan[0], OP_EQ,
            channel_find_by_remote_identity(rsa_id[0], NULL));
  tt_ptr_op(chan[1], OP_EQ,
            channel_find_by_remote_identity(rsa_id[1], NULL));
  tt_ptr_op(chan[3], OP_EQ,
            channel_find_by_remote_identity(rsa_id[3], NULL));
  channel_t *ch;
  ch = channel_find_by_remote_identity(rsa_id[2], NULL);
  tt_assert(ch == chan[2] || ch == chan[4] || ch == chan[5]);
  ch = channel_next_with_rsa_identity(ch);
  tt_assert(ch == chan[2] || ch == chan[4] || ch == chan[5]);
  ch = channel_next_with_rsa_identity(ch);
  tt_assert(ch == chan[2] || ch == chan[4] || ch == chan[5]);
  ch = channel_next_with_rsa_identity(ch);
  tt_ptr_op(ch, OP_EQ, NULL);

  /* As above, but with zero Ed25519 ID (meaning "any ID") */
  tt_ptr_op(chan[0], OP_EQ,
            channel_find_by_remote_identity(rsa_id[0], &ed_zero));
  tt_ptr_op(chan[1], OP_EQ,
            channel_find_by_remote_identity(rsa_id[1], &ed_zero));
  tt_ptr_op(chan[3], OP_EQ,
            channel_find_by_remote_identity(rsa_id[3], &ed_zero));
  ch = channel_find_by_remote_identity(rsa_id[2], &ed_zero);
  tt_assert(ch == chan[2] || ch == chan[4] || ch == chan[5]);
  ch = channel_next_with_rsa_identity(ch);
  tt_assert(ch == chan[2] || ch == chan[4] || ch == chan[5]);
  ch = channel_next_with_rsa_identity(ch);
  tt_assert(ch == chan[2] || ch == chan[4] || ch == chan[5]);
  ch = channel_next_with_rsa_identity(ch);
  tt_ptr_op(ch, OP_EQ, NULL);

  /* Lookup nonexistent RSA identity */
  tt_ptr_op(NULL, OP_EQ,
            channel_find_by_remote_identity("!!!!!!!!!!!!!!!!!!!!", NULL));

  /* Look up by full identity pair */
  tt_ptr_op(chan[0], OP_EQ,
            channel_find_by_remote_identity(rsa_id[0], ed_id[0]));
  tt_ptr_op(chan[1], OP_EQ,
            channel_find_by_remote_identity(rsa_id[1], ed_id[1]));
  tt_ptr_op(chan[3], OP_EQ,
            channel_find_by_remote_identity(rsa_id[3], ed_id[3] /*NULL*/));
  tt_ptr_op(chan[4], OP_EQ,
            channel_find_by_remote_identity(rsa_id[4], ed_id[4]));
  ch = channel_find_by_remote_identity(rsa_id[2], ed_id[2]);
  tt_assert(ch == chan[2] || ch == chan[5]);

  /* Look up RSA identity with wrong ed25519 identity */
  tt_ptr_op(NULL, OP_EQ,
            channel_find_by_remote_identity(rsa_id[4], ed_id[0]));
  tt_ptr_op(NULL, OP_EQ,
            channel_find_by_remote_identity(rsa_id[2], ed_id[1]));
  tt_ptr_op(NULL, OP_EQ,
            channel_find_by_remote_identity(rsa_id[3], ed_id[1]));

 done:
  for (i = 0; i < N_CHAN; ++i) {
    channel_clear_identity_digest(chan[i]);
    channel_unregister(chan[i]);
    free_fake_channel(chan[i]);
    tor_free(ed_id[i]);
  }
#undef N_CHAN
}

static void
test_channel_state(void *arg)
{
  (void) arg;

  /* Test state validity. */
  tt_int_op(channel_state_is_valid(CHANNEL_STATE_CLOSED), OP_EQ, 1);
  tt_int_op(channel_state_is_valid(CHANNEL_STATE_CLOSING), OP_EQ, 1);
  tt_int_op(channel_state_is_valid(CHANNEL_STATE_ERROR), OP_EQ, 1);
  tt_int_op(channel_state_is_valid(CHANNEL_STATE_OPEN), OP_EQ, 1);
  tt_int_op(channel_state_is_valid(CHANNEL_STATE_OPENING), OP_EQ, 1);
  tt_int_op(channel_state_is_valid(CHANNEL_STATE_MAINT), OP_EQ, 1);
  tt_int_op(channel_state_is_valid(CHANNEL_STATE_LAST), OP_EQ, 0);
  tt_int_op(channel_state_is_valid(INT_MAX), OP_EQ, 0);

  /* Test listener state validity. */
  tt_int_op(channel_listener_state_is_valid(CHANNEL_LISTENER_STATE_CLOSED),
            OP_EQ, 1);
  tt_int_op(channel_listener_state_is_valid(CHANNEL_LISTENER_STATE_LISTENING),
            OP_EQ, 1);
  tt_int_op(channel_listener_state_is_valid(CHANNEL_LISTENER_STATE_CLOSING),
            OP_EQ, 1);
  tt_int_op(channel_listener_state_is_valid(CHANNEL_LISTENER_STATE_ERROR),
            OP_EQ, 1);
  tt_int_op(channel_listener_state_is_valid(CHANNEL_LISTENER_STATE_LAST),
            OP_EQ, 0);
  tt_int_op(channel_listener_state_is_valid(INT_MAX), OP_EQ, 0);

  /* Test state transition. */
  tt_int_op(channel_state_can_transition(CHANNEL_STATE_CLOSED,
                                         CHANNEL_STATE_OPENING), OP_EQ, 1);
  tt_int_op(channel_state_can_transition(CHANNEL_STATE_CLOSED,
                                         CHANNEL_STATE_ERROR), OP_EQ, 0);
  tt_int_op(channel_state_can_transition(CHANNEL_STATE_CLOSING,
                                         CHANNEL_STATE_ERROR), OP_EQ, 1);
  tt_int_op(channel_state_can_transition(CHANNEL_STATE_CLOSING,
                                         CHANNEL_STATE_CLOSED), OP_EQ, 1);
  tt_int_op(channel_state_can_transition(CHANNEL_STATE_CLOSING,
                                         CHANNEL_STATE_OPEN), OP_EQ, 0);
  tt_int_op(channel_state_can_transition(CHANNEL_STATE_MAINT,
                                         CHANNEL_STATE_CLOSING), OP_EQ, 1);
  tt_int_op(channel_state_can_transition(CHANNEL_STATE_MAINT,
                                         CHANNEL_STATE_ERROR), OP_EQ, 1);
  tt_int_op(channel_state_can_transition(CHANNEL_STATE_MAINT,
                                         CHANNEL_STATE_OPEN), OP_EQ, 1);
  tt_int_op(channel_state_can_transition(CHANNEL_STATE_MAINT,
                                         CHANNEL_STATE_OPENING), OP_EQ, 0);
  tt_int_op(channel_state_can_transition(CHANNEL_STATE_OPENING,
                                         CHANNEL_STATE_OPEN), OP_EQ, 1);
  tt_int_op(channel_state_can_transition(CHANNEL_STATE_OPENING,
                                         CHANNEL_STATE_CLOSING), OP_EQ, 1);
  tt_int_op(channel_state_can_transition(CHANNEL_STATE_OPENING,
                                         CHANNEL_STATE_ERROR), OP_EQ, 1);
  tt_int_op(channel_state_can_transition(CHANNEL_STATE_OPEN,
                                         CHANNEL_STATE_ERROR), OP_EQ, 1);
  tt_int_op(channel_state_can_transition(CHANNEL_STATE_OPEN,
                                         CHANNEL_STATE_CLOSING), OP_EQ, 1);
  tt_int_op(channel_state_can_transition(CHANNEL_STATE_OPEN,
                                         CHANNEL_STATE_ERROR), OP_EQ, 1);
  tt_int_op(channel_state_can_transition(CHANNEL_STATE_OPEN,
                                         CHANNEL_STATE_MAINT), OP_EQ, 1);
  tt_int_op(channel_state_can_transition(CHANNEL_STATE_LAST,
                                         CHANNEL_STATE_MAINT), OP_EQ, 0);
  tt_int_op(channel_state_can_transition(CHANNEL_STATE_LAST, INT_MAX),
            OP_EQ, 0);

  /* Test listener state transition. */
  tt_int_op(channel_listener_state_can_transition(
                                       CHANNEL_LISTENER_STATE_CLOSED,
                                       CHANNEL_LISTENER_STATE_LISTENING),
            OP_EQ, 1);
  tt_int_op(channel_listener_state_can_transition(
                                       CHANNEL_LISTENER_STATE_CLOSED,
                                       CHANNEL_LISTENER_STATE_ERROR),
            OP_EQ, 0);

  tt_int_op(channel_listener_state_can_transition(
                                       CHANNEL_LISTENER_STATE_CLOSING,
                                       CHANNEL_LISTENER_STATE_CLOSED),
            OP_EQ, 1);

  tt_int_op(channel_listener_state_can_transition(
                                       CHANNEL_LISTENER_STATE_CLOSING,
                                       CHANNEL_LISTENER_STATE_ERROR),
            OP_EQ, 1);
  tt_int_op(channel_listener_state_can_transition(
                                       CHANNEL_LISTENER_STATE_ERROR,
                                       CHANNEL_LISTENER_STATE_CLOSING),
            OP_EQ, 0);

  tt_int_op(channel_listener_state_can_transition(
                                       CHANNEL_LISTENER_STATE_LISTENING,
                                       CHANNEL_LISTENER_STATE_CLOSING),
            OP_EQ, 1);
  tt_int_op(channel_listener_state_can_transition(
                                       CHANNEL_LISTENER_STATE_LISTENING,
                                       CHANNEL_LISTENER_STATE_ERROR),
            OP_EQ, 1);
  tt_int_op(channel_listener_state_can_transition(
                                       CHANNEL_LISTENER_STATE_LAST,
                                       INT_MAX),
            OP_EQ, 0);

  /* Test state string. */
  tt_str_op(channel_state_to_string(CHANNEL_STATE_CLOSING), OP_EQ,
            "closing");
  tt_str_op(channel_state_to_string(CHANNEL_STATE_ERROR), OP_EQ,
            "channel error");
  tt_str_op(channel_state_to_string(CHANNEL_STATE_CLOSED), OP_EQ,
            "closed");
  tt_str_op(channel_state_to_string(CHANNEL_STATE_OPEN), OP_EQ,
            "open");
  tt_str_op(channel_state_to_string(CHANNEL_STATE_OPENING), OP_EQ,
            "opening");
  tt_str_op(channel_state_to_string(CHANNEL_STATE_MAINT), OP_EQ,
            "temporarily suspended for maintenance");
  tt_str_op(channel_state_to_string(CHANNEL_STATE_LAST), OP_EQ,
            "unknown or invalid channel state");
  tt_str_op(channel_state_to_string(INT_MAX), OP_EQ,
            "unknown or invalid channel state");

  /* Test listener state string. */
  tt_str_op(channel_listener_state_to_string(CHANNEL_LISTENER_STATE_CLOSING),
            OP_EQ, "closing");
  tt_str_op(channel_listener_state_to_string(CHANNEL_LISTENER_STATE_ERROR),
            OP_EQ, "channel listener error");
  tt_str_op(channel_listener_state_to_string(CHANNEL_LISTENER_STATE_LISTENING),
            OP_EQ, "listening");
  tt_str_op(channel_listener_state_to_string(CHANNEL_LISTENER_STATE_LAST),
            OP_EQ, "unknown or invalid channel listener state");
  tt_str_op(channel_listener_state_to_string(INT_MAX),
            OP_EQ, "unknown or invalid channel listener state");

 done:
  ;
}

static networkstatus_t *mock_ns = NULL;

static networkstatus_t *
mock_networkstatus_get_latest_consensus(void)
{
  return mock_ns;
}

static void
test_channel_duplicates(void *arg)
{
  channel_t *chan = NULL;
  routerstatus_t rs;

  (void) arg;

  setup_full_capture_of_logs(LOG_INFO);
  /* Try a flat call with channel nor connections. */
  channel_check_for_duplicates();
  expect_log_msg_containing(
    "Found 0 connections to authorities, 0 connections to 0 relays. "
    "Found 0 current canonical connections, "
    "in 0 of which we were a non-canonical peer. "
    "0 relays had more than 1 connection, 0 had more than 2, and "
    "0 had more than 4 connections.");

  mock_ns = tor_malloc_zero(sizeof(*mock_ns));
  mock_ns->routerstatus_list = smartlist_new();
  MOCK(networkstatus_get_latest_consensus,
       mock_networkstatus_get_latest_consensus);

  chan = new_fake_channel();
  tt_assert(chan);
  chan->is_canonical = test_chan_is_canonical;
  memset(chan->identity_digest, 'A', sizeof(chan->identity_digest));
  channel_add_to_digest_map(chan);
  tt_ptr_op(channel_find_by_remote_identity(chan->identity_digest, NULL),
            OP_EQ, chan);

  /* No relay has been associated with this channel. */
  channel_check_for_duplicates();
  expect_log_msg_containing(
    "Found 0 connections to authorities, 0 connections to 0 relays. "
    "Found 0 current canonical connections, "
    "in 0 of which we were a non-canonical peer. "
    "0 relays had more than 1 connection, 0 had more than 2, and "
    "0 had more than 4 connections.");

  /* Associate relay to this connection in the consensus. */
  memset(&rs, 0, sizeof(rs));
  memset(rs.identity_digest, 'A', sizeof(rs.identity_digest));
  smartlist_add(mock_ns->routerstatus_list, &rs);

  /* Non opened channel. */
  chan->state = CHANNEL_STATE_CLOSING;
  channel_check_for_duplicates();
  expect_log_msg_containing(
    "Found 0 connections to authorities, 0 connections to 0 relays. "
    "Found 0 current canonical connections, "
    "in 0 of which we were a non-canonical peer. "
    "0 relays had more than 1 connection, 0 had more than 2, and "
    "0 had more than 4 connections.");
  chan->state = CHANNEL_STATE_OPEN;

  channel_check_for_duplicates();
  expect_log_msg_containing(
    "Found 0 connections to authorities, 1 connections to 1 relays. "
    "Found 0 current canonical connections, "
    "in 0 of which we were a non-canonical peer. "
    "0 relays had more than 1 connection, 0 had more than 2, and "
    "0 had more than 4 connections.");

  test_chan_should_be_canonical = 1;
  channel_check_for_duplicates();
  expect_log_msg_containing(
    "Found 0 connections to authorities, 1 connections to 1 relays. "
    "Found 1 current canonical connections, "
    "in 1 of which we were a non-canonical peer. "
    "0 relays had more than 1 connection, 0 had more than 2, and "
    "0 had more than 4 connections.");
  teardown_capture_of_logs();

 done:
  free_fake_channel(chan);
  smartlist_clear(mock_ns->routerstatus_list);
  networkstatus_vote_free(mock_ns);
  UNMOCK(networkstatus_get_latest_consensus);
}

static void
test_channel_for_extend(void *arg)
{
  channel_t *chan1 = NULL, *chan2 = NULL;
  channel_t *ret_chan = NULL;
  char digest[DIGEST_LEN];
  ed25519_public_key_t ed_id;
  tor_addr_t ipv4_addr, ipv6_addr;
  const char *msg;
  int launch;
  time_t now = time(NULL);

  (void) arg;

  memset(digest, 'A', sizeof(digest));
  memset(&ed_id, 'B', sizeof(ed_id));

  tor_addr_make_null(&ipv4_addr, AF_INET);
  tor_addr_make_null(&ipv6_addr, AF_INET6);

  chan1 = new_fake_channel();
  tt_assert(chan1);
  /* Need to be registered to get added to the id map. */
  channel_register(chan1);
  tt_int_op(chan1->registered, OP_EQ, 1);
  /* We need those for the test. */
  chan1->is_canonical = test_chan_is_canonical;
  chan1->matches_target = test_chan_matches_target;
  chan1->timestamp_created = now - 9;

  chan2 = new_fake_channel();
  tt_assert(chan2);
  /* Need to be registered to get added to the id map. */
  channel_register(chan2);
  tt_int_op(chan2->registered, OP_EQ, 1);
  /* We need those for the test. */
  chan2->is_canonical = test_chan_is_canonical;
  chan2->matches_target = test_chan_matches_target;
  /* Make it older than chan1. */
  chan2->timestamp_created = chan1->timestamp_created - 1;

  /* Say it's all canonical. */
  test_chan_should_be_canonical = 1;

  /* Set channel identities and add it to the channel map. The last one to be
   * added is made the first one in the list so the lookup will always return
   * that one first. */
  channel_set_identity_digest(chan2, digest, &ed_id);
  channel_set_identity_digest(chan1, digest, &ed_id);
  tt_ptr_op(channel_find_by_remote_identity(digest, NULL), OP_EQ, chan1);
  tt_ptr_op(channel_find_by_remote_identity(digest, &ed_id), OP_EQ, chan1);

  /* The expected result is chan2 because it is older than chan1. */
  ret_chan = channel_get_for_extend(digest, &ed_id, &ipv4_addr, &ipv6_addr,
                                    false, &msg, &launch);
  tt_assert(ret_chan);
  tt_ptr_op(ret_chan, OP_EQ, chan2);
  tt_int_op(launch, OP_EQ, 0);
  tt_str_op(msg, OP_EQ, "Connection is fine; using it.");

  /* Switch that around from previous test. */
  chan2->timestamp_created = chan1->timestamp_created + 1;
  ret_chan = channel_get_for_extend(digest, &ed_id, &ipv4_addr, &ipv6_addr,
                                    false, &msg, &launch);
  tt_assert(ret_chan);
  tt_ptr_op(ret_chan, OP_EQ, chan1);
  tt_int_op(launch, OP_EQ, 0);
  tt_str_op(msg, OP_EQ, "Connection is fine; using it.");

  /* Same creation time, num circuits will be used and they both have 0 so the
   * channel 2 should be picked due to how channel_is_better() works. */
  chan2->timestamp_created = chan1->timestamp_created;
  ret_chan = channel_get_for_extend(digest, &ed_id, &ipv4_addr, &ipv6_addr,
                                    false, &msg, &launch);
  tt_assert(ret_chan);
  tt_ptr_op(ret_chan, OP_EQ, chan1);
  tt_int_op(launch, OP_EQ, 0);
  tt_str_op(msg, OP_EQ, "Connection is fine; using it.");

  /* For the rest of the tests, we need channel 1 to be the older. */
  chan2->timestamp_created = chan1->timestamp_created + 1;

  /* Condemned the older channel. */
  chan1->state = CHANNEL_STATE_CLOSING;
  ret_chan = channel_get_for_extend(digest, &ed_id, &ipv4_addr, &ipv6_addr,
                                    false, &msg, &launch);
  tt_assert(ret_chan);
  tt_ptr_op(ret_chan, OP_EQ, chan2);
  tt_int_op(launch, OP_EQ, 0);
  tt_str_op(msg, OP_EQ, "Connection is fine; using it.");
  chan1->state = CHANNEL_STATE_OPEN;

  /* Make the older channel a client one. */
  channel_mark_client(chan1);
  ret_chan = channel_get_for_extend(digest, &ed_id, &ipv4_addr, &ipv6_addr,
                                    false, &msg, &launch);
  tt_assert(ret_chan);
  tt_ptr_op(ret_chan, OP_EQ, chan2);
  tt_int_op(launch, OP_EQ, 0);
  tt_str_op(msg, OP_EQ, "Connection is fine; using it.");
  channel_clear_client(chan1);

  /* Non matching ed identity with valid digest. */
  ed25519_public_key_t dumb_ed_id;
  memset(&dumb_ed_id, 0, sizeof(dumb_ed_id));
  ret_chan = channel_get_for_extend(digest, &dumb_ed_id,
                                    &ipv4_addr, &ipv6_addr,
                                    false, &msg, &launch);
  tt_assert(!ret_chan);
  tt_str_op(msg, OP_EQ, "Not connected. Connecting.");
  tt_int_op(launch, OP_EQ, 1);

  /* Opening channel, we'll check if the target address matches. */
  test_chan_should_match_target = 1;
  chan1->state = CHANNEL_STATE_OPENING;
  chan2->state = CHANNEL_STATE_OPENING;
  ret_chan = channel_get_for_extend(digest, &ed_id, &ipv4_addr, &ipv6_addr,
                                    false, &msg, &launch);
  tt_assert(!ret_chan);
  tt_str_op(msg, OP_EQ, "Connection in progress; waiting.");
  tt_int_op(launch, OP_EQ, 0);
  chan1->state = CHANNEL_STATE_OPEN;
  chan2->state = CHANNEL_STATE_OPEN;

  /* Mark channel 1 as bad for circuits. */
  channel_mark_bad_for_new_circs(chan1);
  ret_chan = channel_get_for_extend(digest, &ed_id, &ipv4_addr, &ipv6_addr,
                                    false, &msg, &launch);
  tt_assert(ret_chan);
  tt_ptr_op(ret_chan, OP_EQ, chan2);
  tt_int_op(launch, OP_EQ, 0);
  tt_str_op(msg, OP_EQ, "Connection is fine; using it.");
  chan1->is_bad_for_new_circs = 0;

  /* Mark both channels as unusable. */
  channel_mark_bad_for_new_circs(chan1);
  channel_mark_bad_for_new_circs(chan2);
  ret_chan = channel_get_for_extend(digest, &ed_id, &ipv4_addr, &ipv6_addr,
                                    false, &msg, &launch);
  tt_assert(!ret_chan);
  tt_str_op(msg, OP_EQ, "Connections all too old, or too non-canonical. "
                        " Launching a new one.");
  tt_int_op(launch, OP_EQ, 1);
  chan1->is_bad_for_new_circs = 0;
  chan2->is_bad_for_new_circs = 0;

  /* Non canonical channels. */
  test_chan_should_be_canonical = 0;
  test_chan_should_match_target = 0;
  ret_chan = channel_get_for_extend(digest, &ed_id, &ipv4_addr, &ipv6_addr,
                                    false, &msg, &launch);
  tt_assert(!ret_chan);
  tt_str_op(msg, OP_EQ, "Connections all too old, or too non-canonical. "
                        " Launching a new one.");
  tt_int_op(launch, OP_EQ, 1);

 done:
  free_fake_channel(chan1);
  free_fake_channel(chan2);
}

static void
test_channel_listener(void *arg)
{
  int old_count;
  time_t now = time(NULL);
  channel_listener_t *chan = NULL;

  (void) arg;

  chan = tor_malloc_zero(sizeof(*chan));
  tt_assert(chan);
  channel_init_listener(chan);
  tt_u64_op(chan->global_identifier, OP_EQ, 1);
  tt_int_op(chan->timestamp_created, OP_GE, now);
  chan->close = test_chan_listener_close;

  /* Register it. At this point, it is not open so it will be put in the
   * finished list. */
  channel_listener_register(chan);
  tt_int_op(chan->registered, OP_EQ, 1);
  channel_listener_unregister(chan);

  /* Register it as listening now thus active. */
  chan->state = CHANNEL_LISTENER_STATE_LISTENING;
  channel_listener_register(chan);
  tt_int_op(chan->registered, OP_EQ, 1);

  /* Set the listener function. */
  channel_listener_set_listener_fn(chan, test_chan_listener_fn);
  tt_ptr_op(chan->listener, OP_EQ, test_chan_listener_fn);

  /* Put a channel in the listener incoming list and queue it.
   * function. By doing this, the listener() handler will be called. */
  channel_t *in_chan = new_fake_channel();
  old_count = test_chan_listener_fn_called;
  channel_listener_queue_incoming(chan, in_chan);
  free_fake_channel(in_chan);
  tt_int_op(test_chan_listener_fn_called, OP_EQ, old_count + 1);

  /* Put listener channel in CLOSING state. */
  old_count = test_chan_listener_close_fn_called;
  channel_listener_mark_for_close(chan);
  tt_int_op(test_chan_listener_close_fn_called, OP_EQ, old_count + 1);
  channel_listener_change_state(chan, CHANNEL_LISTENER_STATE_CLOSED);

  /* Dump stats so we at least hit the code path. */
  chan->describe_transport = test_chan_listener_describe_transport;
  /* There is a check for "now > timestamp_created" when dumping the stats so
   * make sure we go in. */
  chan->timestamp_created = now - 10;
  channel_listener_dump_statistics(chan, LOG_INFO);

 done:
  if (chan) {
    channel_listener_unregister(chan);
    tor_free(chan);
  }
  channel_free_all();
}

#define TEST_SETUP_MATCHES_ADDR(orcon, addr, src, rv) STMT_BEGIN \
    rv = tor_inet_pton(addr.family, src, &addr.addr); \
    tt_int_op(rv, OP_EQ, 1); \
    orcon->base_.addr = addr; \
  STMT_END;

#define TEST_MATCHES_ADDR(chan, addr4, addr6, rv, exp) STMT_BEGIN       \
    rv = channel_matches_target_addr_for_extend(chan, addr4, addr6);    \
    tt_int_op(rv, OP_EQ, exp); \
  STMT_END;

static void
test_channel_matches_target_addr_for_extend(void *arg)
{
  (void) arg;

  channel_tls_t *tlschan = tor_malloc_zero(sizeof(*tlschan));
  or_connection_t *orcon = tor_malloc_zero(sizeof(*orcon));
  channel_t *chan = &(tlschan->base_);
  tor_addr_t addr;
  int rv;

  tlschan->conn = orcon;
  channel_tls_common_init(tlschan);

  /* Test for IPv4 addresses. */
  addr.family = AF_INET;
  TEST_SETUP_MATCHES_ADDR(orcon, addr, "1.2.3.4", rv);
  TEST_MATCHES_ADDR(chan, &addr, NULL, rv, 1);

  tor_inet_pton(addr.family, "2.5.3.4", &addr.addr);
  TEST_MATCHES_ADDR(chan, &addr, NULL, rv, 0);

  /* Test for IPv6 addresses. */
  addr.family = AF_INET6;
  TEST_SETUP_MATCHES_ADDR(orcon, addr, "3:4:7:1:9:8:09:10", rv);
  TEST_MATCHES_ADDR(chan, NULL, &addr, rv, 1);

  tor_inet_pton(addr.family, "::", &addr.addr);
  TEST_MATCHES_ADDR(chan, NULL, &addr, rv, 0);

 done:
  circuitmux_clear_policy(chan->cmux);
  circuitmux_free(chan->cmux);
  tor_free(orcon);
  tor_free(tlschan);
}

struct testcase_t channel_tests[] = {
  { "inbound_cell", test_channel_inbound_cell, TT_FORK,
    NULL, NULL },
  { "outbound_cell", test_channel_outbound_cell, TT_FORK,
    NULL, NULL },
  { "id_map", test_channel_id_map, TT_FORK,
    NULL, NULL },
  { "lifecycle", test_channel_lifecycle, TT_FORK,
    NULL, NULL },
  { "lifecycle_2", test_channel_lifecycle_2, TT_FORK,
    NULL, NULL },
  { "dumpstats", test_channel_dumpstats, TT_FORK,
    NULL, NULL },
  { "state", test_channel_state, TT_FORK,
    NULL, NULL },
  { "duplicates", test_channel_duplicates, TT_FORK,
    NULL, NULL },
  { "get_channel_for_extend", test_channel_for_extend, TT_FORK,
    NULL, NULL },
  { "listener", test_channel_listener, TT_FORK,
    NULL, NULL },
  { "matches_target", test_channel_matches_target_addr_for_extend, TT_FORK,
    NULL, NULL },
  END_OF_TESTCASES
};
