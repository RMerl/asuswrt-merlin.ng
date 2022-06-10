/* Copyright (c) 2014-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/* Unit tests for OOM handling logic */

#define RELAY_PRIVATE
#define BUFFERS_PRIVATE
#define CIRCUITLIST_PRIVATE
#define CONNECTION_PRIVATE
#include "core/or/or.h"
#include "lib/buf/buffers.h"
#include "core/or/circuitlist.h"
#include "lib/evloop/compat_libevent.h"
#include "core/mainloop/connection.h"
#include "app/config/config.h"
#include "lib/crypt_ops/crypto_rand.h"
#include "core/or/relay.h"
#include "test/test.h"
#include "test/test_helpers.h"

#include "core/or/cell_st.h"
#include "core/or/entry_connection_st.h"
#include "core/or/or_circuit_st.h"
#include "core/or/origin_circuit_st.h"

/* small replacement mock for circuit_mark_for_close_ to avoid doing all
 * the other bookkeeping that comes with marking circuits. */
static void
circuit_mark_for_close_dummy_(circuit_t *circ, int reason, int line,
                              const char *file)
{
  (void) reason;
  if (circ->marked_for_close) {
    TT_FAIL(("Circuit already marked for close at %s:%d, but we are marking "
             "it again at %s:%d",
             circ->marked_for_close_file, (int)circ->marked_for_close,
             file, line));
  }

  circ->marked_for_close = line;
  circ->marked_for_close_file = file;
}

static circuit_t *
dummy_or_circuit_new(int n_p_cells, int n_n_cells)
{
  or_circuit_t *circ = or_circuit_new(0, NULL);
  int i;
  cell_t cell;

  for (i=0; i < n_p_cells; ++i) {
    crypto_rand((void*)&cell, sizeof(cell));
    cell_queue_append_packed_copy(TO_CIRCUIT(circ), &circ->p_chan_cells,
                                  0, &cell, 1, 0);
  }

  for (i=0; i < n_n_cells; ++i) {
    crypto_rand((void*)&cell, sizeof(cell));
    cell_queue_append_packed_copy(TO_CIRCUIT(circ),
                                  &TO_CIRCUIT(circ)->n_chan_cells,
                                  1, &cell, 1, 0);
  }

  TO_CIRCUIT(circ)->purpose = CIRCUIT_PURPOSE_OR;
  return TO_CIRCUIT(circ);
}

static void
add_bytes_to_buf(buf_t *buf, size_t n_bytes)
{
  char b[3000];

  while (n_bytes) {
    size_t this_add = n_bytes > sizeof(b) ? sizeof(b) : n_bytes;
    crypto_rand(b, this_add);
    buf_add(buf, b, this_add);
    n_bytes -= this_add;
  }
}

static edge_connection_t *
dummy_edge_conn_new(circuit_t *circ,
                    int type, size_t in_bytes, size_t out_bytes)
{
  edge_connection_t *conn;
  buf_t *inbuf, *outbuf;

  if (type == CONN_TYPE_EXIT)
    conn = edge_connection_new(type, AF_INET);
  else
    conn = ENTRY_TO_EDGE_CONN(entry_connection_new(type, AF_INET));

  inbuf = TO_CONN(conn)->inbuf;
  outbuf = TO_CONN(conn)->outbuf;

  /* We add these bytes directly to the buffers, to avoid all the
   * edge connection read/write machinery. */
  add_bytes_to_buf(inbuf, in_bytes);
  add_bytes_to_buf(outbuf, out_bytes);

  conn->on_circuit = circ;
  if (type == CONN_TYPE_EXIT) {
    or_circuit_t *oc  = TO_OR_CIRCUIT(circ);
    conn->next_stream = oc->n_streams;
    oc->n_streams = conn;
  } else {
    origin_circuit_t *oc = TO_ORIGIN_CIRCUIT(circ);
    conn->next_stream = oc->p_streams;
    oc->p_streams = conn;
  }

  return conn;
}

/** Run unit tests for buffers.c */
static void
test_oom_circbuf(void *arg)
{
  or_options_t *options = get_options_mutable();
  circuit_t *c1 = NULL, *c2 = NULL, *c3 = NULL, *c4 = NULL;
  uint64_t now_ns = 1389631048 * (uint64_t)1000000000;
  const uint64_t start_ns = now_ns;

  (void) arg;

  monotime_enable_test_mocking();
  MOCK(circuit_mark_for_close_, circuit_mark_for_close_dummy_);

  /* Far too low for real life. */
  options->MaxMemInQueues = 256*packed_cell_mem_cost();
  options->CellStatistics = 0;

  tt_int_op(cell_queues_check_size(), OP_EQ, 0); /* We don't start out OOM. */
  tt_int_op(cell_queues_get_total_allocation(), OP_EQ, 0);
  tt_int_op(buf_get_total_allocation(), OP_EQ, 0);

  /* Now we're going to fake up some circuits and get them added to the global
     circuit list. */
  monotime_coarse_set_mock_time_nsec(now_ns);
  c1 = dummy_origin_circuit_new(30);

  now_ns += 10 * 1000000;
  monotime_coarse_set_mock_time_nsec(now_ns);
  c2 = dummy_or_circuit_new(20, 20);

  tt_int_op(packed_cell_mem_cost(), OP_EQ,
            sizeof(packed_cell_t));
  tt_int_op(cell_queues_get_total_allocation(), OP_EQ,
            packed_cell_mem_cost() * 70);
  tt_int_op(cell_queues_check_size(), OP_EQ, 0); /* We are still not OOM */

  now_ns += 10 * 1000000;
  monotime_coarse_set_mock_time_nsec(now_ns);
  c3 = dummy_or_circuit_new(100, 85);
  tt_int_op(cell_queues_check_size(), OP_EQ, 0); /* We are still not OOM */
  tt_int_op(cell_queues_get_total_allocation(), OP_EQ,
            packed_cell_mem_cost() * 255);

  now_ns += 10 * 1000000;
  monotime_coarse_set_mock_time_nsec(now_ns);
  /* Adding this cell will trigger our OOM handler. */
  c4 = dummy_or_circuit_new(2, 0);

  tt_int_op(cell_queues_get_total_allocation(), OP_EQ,
            packed_cell_mem_cost() * 257);

  tt_int_op(cell_queues_check_size(), OP_EQ, 1); /* We are now OOM */

  tt_assert(c1->marked_for_close);
  tt_assert(! c2->marked_for_close);
  tt_assert(! c3->marked_for_close);
  tt_assert(! c4->marked_for_close);

  tt_int_op(cell_queues_get_total_allocation(), OP_EQ,
            packed_cell_mem_cost() * (257 - 30));

  circuit_free(c1);

  monotime_coarse_set_mock_time_nsec(start_ns); /* go back in time */
  c1 = dummy_or_circuit_new(90, 0);

  now_ns += 10 * 1000000;
  monotime_coarse_set_mock_time_nsec(now_ns);

  tt_int_op(cell_queues_check_size(), OP_EQ, 1); /* We are now OOM */

  tt_assert(c1->marked_for_close);
  tt_assert(! c2->marked_for_close);
  tt_assert(! c3->marked_for_close);
  tt_assert(! c4->marked_for_close);

  tt_int_op(cell_queues_get_total_allocation(), OP_EQ,
            packed_cell_mem_cost() * (257 - 30));

 done:
  circuit_free(c1);
  circuit_free(c2);
  circuit_free(c3);
  circuit_free(c4);

  UNMOCK(circuit_mark_for_close_);
  monotime_disable_test_mocking();
}

/** Run unit tests for buffers.c */
static void
test_oom_streambuf(void *arg)
{
  or_options_t *options = get_options_mutable();
  circuit_t *c1 = NULL, *c2 = NULL, *c3 = NULL, *c4 = NULL, *c5 = NULL;
  uint32_t tvts;
  int i;
  smartlist_t *edgeconns = smartlist_new();
  const uint64_t start_ns = 1389641159 * (uint64_t)1000000000;
  uint64_t now_ns = start_ns;

  (void) arg;
  monotime_enable_test_mocking();

  MOCK(circuit_mark_for_close_, circuit_mark_for_close_dummy_);

  /* Far too low for real life. */
  options->MaxMemInQueues = 81*packed_cell_mem_cost() + 4096 * 34;
  options->CellStatistics = 0;

  tt_int_op(cell_queues_check_size(), OP_EQ, 0); /* We don't start out OOM. */
  tt_int_op(cell_queues_get_total_allocation(), OP_EQ, 0);
  tt_int_op(buf_get_total_allocation(), OP_EQ, 0);

  monotime_coarse_set_mock_time_nsec(start_ns);

  /* Start all circuits with a bit of data queued in cells */

  /* go halfway into the second. */
  monotime_coarse_set_mock_time_nsec(start_ns + 500 * 1000000);
  c1 = dummy_or_circuit_new(10,10);

  monotime_coarse_set_mock_time_nsec(start_ns + 510 * 1000000);
  c2 = dummy_origin_circuit_new(20);
  monotime_coarse_set_mock_time_nsec(start_ns + 520 * 1000000);
  c3 = dummy_or_circuit_new(20,20);
  monotime_coarse_set_mock_time_nsec(start_ns + 530 * 1000000);
  c4 = dummy_or_circuit_new(0,0);
  tt_int_op(cell_queues_get_total_allocation(), OP_EQ,
            packed_cell_mem_cost() * 80);

  now_ns = start_ns + 600 * 1000000;
  monotime_coarse_set_mock_time_nsec(now_ns);

  /* Add some connections to c1...c4. */
  for (i = 0; i < 4; ++i) {
    edge_connection_t *ec;
    /* link it to a circuit */
    now_ns += 10 * 1000000;
    monotime_coarse_set_mock_time_nsec(now_ns);
    ec = dummy_edge_conn_new(c1, CONN_TYPE_EXIT, 1000, 1000);
    tt_assert(ec);
    smartlist_add(edgeconns, ec);
    now_ns += 10 * 1000000;
    monotime_coarse_set_mock_time_nsec(now_ns);
    ec = dummy_edge_conn_new(c2, CONN_TYPE_AP, 1000, 1000);
    tt_assert(ec);
    smartlist_add(edgeconns, ec);
    now_ns += 10 * 1000000;
    monotime_coarse_set_mock_time_nsec(now_ns);
    ec = dummy_edge_conn_new(c4, CONN_TYPE_EXIT, 1000, 1000); /* Yes, 4 twice*/
    tt_assert(ec);
    smartlist_add(edgeconns, ec);
    now_ns += 10 * 1000000;
    monotime_coarse_set_mock_time_nsec(now_ns);
    ec = dummy_edge_conn_new(c4, CONN_TYPE_EXIT, 1000, 1000);
    smartlist_add(edgeconns, ec);
    tt_assert(ec);
  }

  now_ns -= now_ns % 1000000000;
  now_ns += 1000000000;
  monotime_coarse_set_mock_time_nsec(now_ns);
  tvts = monotime_coarse_get_stamp();

#define ts_is_approx(ts, val) do {                                   \
    uint32_t x_ = (uint32_t) monotime_coarse_stamp_units_to_approx_msec(ts); \
    tt_int_op(x_, OP_GE, val - 5);                                      \
    tt_int_op(x_, OP_LE, val + 5);                                      \
  } while (0)

  ts_is_approx(circuit_max_queued_cell_age(c1, tvts), 500);
  ts_is_approx(circuit_max_queued_cell_age(c2, tvts), 490);
  ts_is_approx(circuit_max_queued_cell_age(c3, tvts), 480);
  ts_is_approx(circuit_max_queued_cell_age(c4, tvts), 0);

  ts_is_approx(circuit_max_queued_data_age(c1, tvts), 390);
  ts_is_approx(circuit_max_queued_data_age(c2, tvts), 380);
  ts_is_approx(circuit_max_queued_data_age(c3, tvts), 0);
  ts_is_approx(circuit_max_queued_data_age(c4, tvts), 370);

  ts_is_approx(circuit_max_queued_item_age(c1, tvts), 500);
  ts_is_approx(circuit_max_queued_item_age(c2, tvts), 490);
  ts_is_approx(circuit_max_queued_item_age(c3, tvts), 480);
  ts_is_approx(circuit_max_queued_item_age(c4, tvts), 370);

  tt_int_op(cell_queues_get_total_allocation(), OP_EQ,
            packed_cell_mem_cost() * 80);
  tt_int_op(buf_get_total_allocation(), OP_EQ, 4096*16*2);

  /* Now give c4 a very old buffer of modest size */
  {
    edge_connection_t *ec;
    now_ns -= 1000000000;
    monotime_coarse_set_mock_time_nsec(now_ns);
    ec = dummy_edge_conn_new(c4, CONN_TYPE_EXIT, 1000, 1000);
    tt_assert(ec);
    smartlist_add(edgeconns, ec);
  }
  tt_int_op(buf_get_total_allocation(), OP_EQ, 4096*17*2);
  ts_is_approx(circuit_max_queued_item_age(c4, tvts), 1000);

  tt_int_op(cell_queues_check_size(), OP_EQ, 0);

  /* And run over the limit. */
  now_ns += 800*1000000;
  monotime_coarse_set_mock_time_nsec(now_ns);
  c5 = dummy_or_circuit_new(0,5);

  tt_int_op(cell_queues_get_total_allocation(), OP_EQ,
            packed_cell_mem_cost() * 85);
  tt_int_op(buf_get_total_allocation(), OP_EQ, 4096*17*2);

  tt_int_op(cell_queues_check_size(), OP_EQ, 1); /* We are now OOM */

  /* C4 should have died. */
  tt_assert(! c1->marked_for_close);
  tt_assert(! c2->marked_for_close);
  tt_assert(! c3->marked_for_close);
  tt_assert(c4->marked_for_close);
  tt_assert(! c5->marked_for_close);

  tt_int_op(cell_queues_get_total_allocation(), OP_EQ,
            packed_cell_mem_cost() * 85);
  tt_int_op(buf_get_total_allocation(), OP_EQ, 4096*8*2);

 done:
  circuit_free(c1);
  circuit_free(c2);
  circuit_free(c3);
  circuit_free(c4);
  circuit_free(c5);

  SMARTLIST_FOREACH(edgeconns, edge_connection_t *, ec,
                    connection_free_minimal(TO_CONN(ec)));
  smartlist_free(edgeconns);

  UNMOCK(circuit_mark_for_close_);
  monotime_disable_test_mocking();
}

struct testcase_t oom_tests[] = {
  { "circbuf", test_oom_circbuf, TT_FORK, NULL, NULL },
  { "streambuf", test_oom_streambuf, TT_FORK, NULL, NULL },
  END_OF_TESTCASES
};

