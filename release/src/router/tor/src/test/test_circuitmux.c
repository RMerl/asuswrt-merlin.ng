/* Copyright (c) 2013-2019, The Tor Project, Inc. */
/* See LICENSE for licensing information */

#define TOR_CHANNEL_INTERNAL_
#define CIRCUITMUX_PRIVATE
#define CIRCUITMUX_EWMA_PRIVATE
#define RELAY_PRIVATE
#include "core/or/or.h"
#include "core/or/channel.h"
#include "core/or/circuitmux.h"
#include "core/or/circuitmux_ewma.h"
#include "core/or/relay.h"
#include "core/or/scheduler.h"
#include "test/test.h"

#include "core/or/destroy_cell_queue_st.h"

#include <math.h>

/* XXXX duplicated function from test_circuitlist.c */
static channel_t *
new_fake_channel(void)
{
  channel_t *chan = tor_malloc_zero(sizeof(channel_t));
  channel_init(chan);
  return chan;
}

static int
has_queued_writes(channel_t *c)
{
  (void) c;
  return 1;
}

/** Test destroy cell queue with no interference from other queues. */
static void
test_cmux_destroy_cell_queue(void *arg)
{
  circuitmux_t *cmux = NULL;
  channel_t *ch = NULL;
  circuit_t *circ = NULL;
  destroy_cell_queue_t *cq = NULL;
  packed_cell_t *pc = NULL;
  destroy_cell_t *dc = NULL;

  scheduler_init();

  (void) arg;

  cmux = circuitmux_alloc();
  tt_assert(cmux);
  ch = new_fake_channel();
  circuitmux_set_policy(cmux, &ewma_policy);
  ch->has_queued_writes = has_queued_writes;
  ch->wide_circ_ids = 1;

  circ = circuitmux_get_first_active_circuit(cmux, &cq);
  tt_ptr_op(circ, OP_EQ, NULL);
  tt_ptr_op(cq, OP_EQ, NULL);

  circuitmux_append_destroy_cell(ch, cmux, 100, 10);
  circuitmux_append_destroy_cell(ch, cmux, 190, 6);
  circuitmux_append_destroy_cell(ch, cmux, 30, 1);

  tt_int_op(circuitmux_num_cells(cmux), OP_EQ, 3);

  circ = circuitmux_get_first_active_circuit(cmux, &cq);
  tt_ptr_op(circ, OP_EQ, NULL);
  tt_assert(cq);

  tt_int_op(cq->n, OP_EQ, 3);

  dc = destroy_cell_queue_pop(cq);
  tt_assert(dc);
  tt_uint_op(dc->circid, OP_EQ, 100);

  tt_int_op(circuitmux_num_cells(cmux), OP_EQ, 2);

 done:
  circuitmux_free(cmux);
  channel_free(ch);
  packed_cell_free(pc);
  tor_free(dc);
}

static void
test_cmux_compute_ticks(void *arg)
{
  const int64_t NS_PER_S = 1000 * 1000 * 1000;
  const int64_t START_NS = UINT64_C(1217709000)*NS_PER_S;
  int64_t now;
  double rem;
  unsigned tick;
  (void)arg;
  circuitmux_ewma_free_all();
  monotime_enable_test_mocking();

  monotime_coarse_set_mock_time_nsec(START_NS);
  cell_ewma_initialize_ticks();
  const unsigned tick_zero = cell_ewma_get_current_tick_and_fraction(&rem);
  tt_double_op(rem, OP_GT, -1e-9);
  tt_double_op(rem, OP_LT, 1e-9);

  /* 1.5 second later and we should still be in the same tick. */
  now = START_NS + NS_PER_S + NS_PER_S/2;
  monotime_coarse_set_mock_time_nsec(now);
  tick = cell_ewma_get_current_tick_and_fraction(&rem);
  tt_uint_op(tick, OP_EQ, tick_zero);
#ifdef USING_32BIT_MSEC_HACK
  const double tolerance = .0005;
#else
  const double tolerance = .00000001;
#endif
  tt_double_op(fabs(rem - .15), OP_LT, tolerance);

  /* 25 second later and we should be in another tick. */
  now = START_NS + NS_PER_S * 25;
  monotime_coarse_set_mock_time_nsec(now);
  tick = cell_ewma_get_current_tick_and_fraction(&rem);
  tt_uint_op(tick, OP_EQ, tick_zero + 2);
  tt_double_op(fabs(rem - .5), OP_LT, tolerance);

 done:
  ;
}

struct testcase_t circuitmux_tests[] = {
  { "destroy_cell_queue", test_cmux_destroy_cell_queue, TT_FORK, NULL, NULL },
  { "compute_ticks", test_cmux_compute_ticks, TT_FORK, NULL, NULL },
  END_OF_TESTCASES
};

