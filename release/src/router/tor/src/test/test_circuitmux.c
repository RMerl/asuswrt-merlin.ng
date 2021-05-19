/* Copyright (c) 2013-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

#define CHANNEL_OBJECT_PRIVATE
#define CIRCUITMUX_PRIVATE
#define CIRCUITMUX_EWMA_PRIVATE
#define RELAY_PRIVATE

#include "core/or/or.h"
#include "core/or/channel.h"
#include "core/or/circuitmux.h"
#include "core/or/circuitmux_ewma.h"
#include "core/or/destroy_cell_queue_st.h"
#include "core/or/relay.h"
#include "core/or/scheduler.h"

#include "test/fakechans.h"
#include "test/fakecircs.h"
#include "test/test.h"

#include <math.h>

static int
mock_has_queued_writes_true(channel_t *c)
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

  MOCK(scheduler_release_channel, scheduler_release_channel_mock);

  (void) arg;

  ch = new_fake_channel();
  ch->has_queued_writes = mock_has_queued_writes_true;
  ch->wide_circ_ids = 1;
  cmux = ch->cmux;

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
  free_fake_channel(ch);
  packed_cell_free(pc);
  tor_free(dc);

  UNMOCK(scheduler_release_channel);
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

static void
test_cmux_allocate(void *arg)
{
  circuitmux_t *cmux = NULL;

  (void) arg;

  cmux = circuitmux_alloc();
  tt_assert(cmux);
  tt_assert(cmux->chanid_circid_map);
  tt_int_op(HT_SIZE(cmux->chanid_circid_map), OP_EQ, 0);
  tt_uint_op(cmux->n_circuits, OP_EQ, 0);
  tt_uint_op(cmux->n_active_circuits, OP_EQ, 0);
  tt_uint_op(cmux->n_cells, OP_EQ, 0);
  tt_uint_op(cmux->last_cell_was_destroy, OP_EQ, 0);
  tt_i64_op(cmux->destroy_ctr, OP_EQ, 0);
  tt_ptr_op(cmux->policy, OP_EQ, NULL);
  tt_ptr_op(cmux->policy_data, OP_EQ, NULL);

  tt_assert(TOR_SIMPLEQ_EMPTY(&cmux->destroy_cell_queue.head));

 done:
  circuitmux_free(cmux);
}

static void
test_cmux_attach_circuit(void *arg)
{
  circuit_t *circ = NULL;
  or_circuit_t *orcirc = NULL;
  channel_t *pchan = NULL, *nchan = NULL;
  cell_direction_t cdir;
  unsigned int n_cells;

  (void) arg;

  pchan = new_fake_channel();
  tt_assert(pchan);
  nchan = new_fake_channel();
  tt_assert(nchan);

  orcirc = new_fake_orcirc(nchan, pchan);
  tt_assert(orcirc);
  circ = TO_CIRCUIT(orcirc);

  /* While assigning a new circuit IDs, the circuitmux_attach_circuit() is
   * called for a new channel on the circuit. This means, we should now have
   * the created circuit attached on both the pchan and nchan cmux. */
  tt_uint_op(circuitmux_num_circuits(pchan->cmux), OP_EQ, 1);
  tt_uint_op(circuitmux_num_circuits(nchan->cmux), OP_EQ, 1);

  /* There should be _no_ active circuit due to no queued cells. */
  tt_uint_op(circuitmux_num_active_circuits(pchan->cmux), OP_EQ, 0);
  tt_uint_op(circuitmux_num_active_circuits(nchan->cmux), OP_EQ, 0);

  /* Circuit should not be active on the cmux. */
  tt_int_op(circuitmux_is_circuit_active(pchan->cmux, circ), OP_EQ, 0);
  tt_int_op(circuitmux_is_circuit_active(nchan->cmux, circ), OP_EQ, 0);

  /* Not active so no cells. */
  n_cells = circuitmux_num_cells_for_circuit(pchan->cmux, circ);
  tt_uint_op(n_cells, OP_EQ, 0);
  n_cells = circuitmux_num_cells(pchan->cmux);
  tt_uint_op(n_cells, OP_EQ, 0);
  n_cells = circuitmux_num_cells_for_circuit(nchan->cmux, circ);
  tt_uint_op(n_cells, OP_EQ, 0);
  n_cells = circuitmux_num_cells(nchan->cmux);
  tt_uint_op(n_cells, OP_EQ, 0);

  /* So it should be attached :) */
  tt_int_op(circuitmux_is_circuit_attached(pchan->cmux, circ), OP_EQ, 1);
  tt_int_op(circuitmux_is_circuit_attached(nchan->cmux, circ), OP_EQ, 1);

  /* Query the chanid<->circid map in the cmux subsystem with what we just
   * created and validate the cell direction. */
  cdir = circuitmux_attached_circuit_direction(pchan->cmux, circ);
  tt_int_op(cdir, OP_EQ, CELL_DIRECTION_IN);
  cdir = circuitmux_attached_circuit_direction(nchan->cmux, circ);
  tt_int_op(cdir, OP_EQ, CELL_DIRECTION_OUT);

  /*
   * We'll activate->deactivate->activate to test all code paths of
   * circuitmux_set_num_cells().
   */

  /* Activate circuit. */
  circuitmux_set_num_cells(pchan->cmux, circ, 4);
  tt_int_op(circuitmux_is_circuit_active(pchan->cmux, circ), OP_EQ, 1);

  /* Deactivate. */
  circuitmux_clear_num_cells(pchan->cmux, circ);
  tt_int_op(circuitmux_is_circuit_active(pchan->cmux, circ), OP_EQ, 0);
  tt_uint_op(circuitmux_num_cells_for_circuit(pchan->cmux, circ), OP_EQ, 0);

  /* Re-activate. */
  circuitmux_set_num_cells(pchan->cmux, circ, 4);
  tt_int_op(circuitmux_is_circuit_active(pchan->cmux, circ), OP_EQ, 1);

  /* Once re-attached, it should become inactive because the circuit has no
   * cells while the chanid<->circid object has some. The attach code will
   * reset the count on the cmux for that circuit:
   *
   * if (chanid_circid_muxinfo_t->muxinfo.cell_count > 0 && cell_count == 0) {
   */
  circuitmux_attach_circuit(pchan->cmux, circ, CELL_DIRECTION_IN);
  n_cells = circuitmux_num_cells_for_circuit(pchan->cmux, circ);
  tt_uint_op(n_cells, OP_EQ, 0);
  tt_int_op(circuitmux_is_circuit_active(pchan->cmux, circ), OP_EQ, 0);
  tt_uint_op(circuitmux_num_active_circuits(pchan->cmux), OP_EQ, 0);

  /* Lets queue a cell on the circuit now so it becomes active when
   * re-attaching:
   *
   * else if (chanid_circid_muxinfo_t->muxinfo.cell_count == 0 &&
   *          cell_count > 0) {
   */
  orcirc->p_chan_cells.n = 1;
  circuitmux_attach_circuit(pchan->cmux, circ, CELL_DIRECTION_IN);
  tt_int_op(circuitmux_is_circuit_active(pchan->cmux, circ), OP_EQ, 1);

 done:
  free_fake_orcirc(orcirc);
  free_fake_channel(pchan);
  free_fake_channel(nchan);
}

static void
test_cmux_detach_circuit(void *arg)
{
  circuit_t *circ = NULL;
  or_circuit_t *orcirc = NULL;
  channel_t *pchan = NULL, *nchan = NULL;

  (void) arg;

  pchan = new_fake_channel();
  tt_assert(pchan);
  nchan = new_fake_channel();
  tt_assert(nchan);

  orcirc = new_fake_orcirc(nchan, pchan);
  tt_assert(orcirc);
  circ = TO_CIRCUIT(orcirc);

  /* While assigning a new circuit IDs, the circuitmux_attach_circuit() is
   * called for a new channel on the circuit. This means, we should now have
   * the created circuit attached on both the pchan and nchan cmux. */
  tt_uint_op(circuitmux_num_circuits(pchan->cmux), OP_EQ, 1);
  tt_uint_op(circuitmux_num_circuits(nchan->cmux), OP_EQ, 1);
  tt_int_op(circuitmux_is_circuit_attached(pchan->cmux, circ), OP_EQ, 1);
  tt_int_op(circuitmux_is_circuit_attached(nchan->cmux, circ), OP_EQ, 1);

  /* Now, detach the circuit from pchan and then nchan. */
  circuitmux_detach_circuit(pchan->cmux, circ);
  tt_uint_op(circuitmux_num_circuits(pchan->cmux), OP_EQ, 0);
  tt_int_op(circuitmux_is_circuit_attached(pchan->cmux, circ), OP_EQ, 0);
  circuitmux_detach_circuit(nchan->cmux, circ);
  tt_uint_op(circuitmux_num_circuits(nchan->cmux), OP_EQ, 0);
  tt_int_op(circuitmux_is_circuit_attached(nchan->cmux, circ), OP_EQ, 0);

 done:
  free_fake_orcirc(orcirc);
  free_fake_channel(pchan);
  free_fake_channel(nchan);
}

static void
test_cmux_detach_all_circuits(void *arg)
{
  circuit_t *circ = NULL;
  or_circuit_t *orcirc = NULL;
  channel_t *pchan = NULL, *nchan = NULL;
  smartlist_t *detached_out = smartlist_new();

  (void) arg;

  /* Channels need to be registered in order for the detach all circuit
   * function to find them. */
  pchan = new_fake_channel();
  tt_assert(pchan);
  channel_register(pchan);
  nchan = new_fake_channel();
  tt_assert(nchan);
  channel_register(nchan);

  orcirc = new_fake_orcirc(nchan, pchan);
  tt_assert(orcirc);
  circ = TO_CIRCUIT(orcirc);

  /* Just make sure it is attached. */
  tt_uint_op(circuitmux_num_circuits(pchan->cmux), OP_EQ, 1);
  tt_uint_op(circuitmux_num_circuits(nchan->cmux), OP_EQ, 1);
  tt_int_op(circuitmux_is_circuit_attached(pchan->cmux, circ), OP_EQ, 1);
  tt_int_op(circuitmux_is_circuit_attached(nchan->cmux, circ), OP_EQ, 1);

  /* Queue some cells so we can test if the circuit becomes inactive on the
   * cmux after the mass detach. */
  circuitmux_set_num_cells(pchan->cmux, circ, 4);
  circuitmux_set_num_cells(nchan->cmux, circ, 4);

  /* Detach all on pchan and then nchan. */
  circuitmux_detach_all_circuits(pchan->cmux, detached_out);
  tt_uint_op(circuitmux_num_circuits(pchan->cmux), OP_EQ, 0);
  tt_int_op(circuitmux_is_circuit_attached(pchan->cmux, circ), OP_EQ, 0);
  tt_int_op(circuitmux_is_circuit_active(pchan->cmux, circ), OP_EQ, 0);
  tt_int_op(smartlist_len(detached_out), OP_EQ, 1);
  circuitmux_detach_all_circuits(nchan->cmux, NULL);
  tt_uint_op(circuitmux_num_circuits(nchan->cmux), OP_EQ, 0);
  tt_int_op(circuitmux_is_circuit_attached(nchan->cmux, circ), OP_EQ, 0);
  tt_int_op(circuitmux_is_circuit_active(nchan->cmux, circ), OP_EQ, 0);

 done:
  smartlist_free(detached_out);
  free_fake_orcirc(orcirc);
  free_fake_channel(pchan);
  free_fake_channel(nchan);
}

static void
test_cmux_policy(void *arg)
{
  circuit_t *circ = NULL;
  or_circuit_t *orcirc = NULL;
  channel_t *pchan = NULL, *nchan = NULL;

  (void) arg;

  pchan = new_fake_channel();
  tt_assert(pchan);
  channel_register(pchan);
  nchan = new_fake_channel();
  tt_assert(nchan);
  channel_register(nchan);

  orcirc = new_fake_orcirc(nchan, pchan);
  tt_assert(orcirc);
  circ = TO_CIRCUIT(orcirc);

  /* Confirm we have the EWMA policy by default for new channels. */
  tt_ptr_op(circuitmux_get_policy(pchan->cmux), OP_EQ, &ewma_policy);
  tt_ptr_op(circuitmux_get_policy(nchan->cmux), OP_EQ, &ewma_policy);

  /* Putting cell on the cmux means will make the notify policy code path to
   * trigger. */
  circuitmux_set_num_cells(pchan->cmux, circ, 4);

  /* Clear it out. */
  circuitmux_clear_policy(pchan->cmux);

  /* Set back the EWMA policy. */
  circuitmux_set_policy(pchan->cmux, &ewma_policy);

 done:
  free_fake_orcirc(orcirc);
  free_fake_channel(pchan);
  free_fake_channel(nchan);
}

static void
test_cmux_xmit_cell(void *arg)
{
  circuit_t *circ = NULL;
  or_circuit_t *orcirc = NULL;
  channel_t *pchan = NULL, *nchan = NULL;

  (void) arg;

  pchan = new_fake_channel();
  tt_assert(pchan);
  nchan = new_fake_channel();
  tt_assert(nchan);

  orcirc = new_fake_orcirc(nchan, pchan);
  tt_assert(orcirc);
  circ = TO_CIRCUIT(orcirc);

  /* Queue 4 cells on the circuit. */
  circuitmux_set_num_cells(pchan->cmux, circ, 4);
  tt_uint_op(circuitmux_num_cells_for_circuit(pchan->cmux, circ), OP_EQ, 4);
  tt_uint_op(circuitmux_num_cells(pchan->cmux), OP_EQ, 4);
  tt_int_op(circuitmux_is_circuit_active(pchan->cmux, circ), OP_EQ, 1);
  tt_uint_op(circuitmux_num_active_circuits(pchan->cmux), OP_EQ, 1);

  /* Emit the first cell. Circuit should still be active. */
  circuitmux_notify_xmit_cells(pchan->cmux, circ, 1);
  tt_uint_op(circuitmux_num_cells(pchan->cmux), OP_EQ, 3);
  tt_uint_op(circuitmux_num_cells_for_circuit(pchan->cmux, circ), OP_EQ, 3);
  tt_int_op(circuitmux_is_circuit_active(pchan->cmux, circ), OP_EQ, 1);
  tt_uint_op(circuitmux_num_active_circuits(pchan->cmux), OP_EQ, 1);

  /* Emit the last 3 cells. Circuit should become inactive. */
  circuitmux_notify_xmit_cells(pchan->cmux, circ, 3);
  tt_uint_op(circuitmux_num_cells(pchan->cmux), OP_EQ, 0);
  tt_uint_op(circuitmux_num_cells_for_circuit(pchan->cmux, circ), OP_EQ, 0);
  tt_int_op(circuitmux_is_circuit_active(pchan->cmux, circ), OP_EQ, 0);
  tt_uint_op(circuitmux_num_active_circuits(pchan->cmux), OP_EQ, 0);

  /* Queue a DESTROY cell. */
  pchan->has_queued_writes = mock_has_queued_writes_true;
  circuitmux_append_destroy_cell(pchan, pchan->cmux, orcirc->p_circ_id, 0);
  tt_i64_op(pchan->cmux->destroy_ctr, OP_EQ, 1);
  tt_int_op(pchan->cmux->destroy_cell_queue.n, OP_EQ, 1);
  tt_i64_op(circuitmux_count_queued_destroy_cells(pchan, pchan->cmux),
            OP_EQ, 1);

  /* Emit the DESTROY cell. */
  circuitmux_notify_xmit_destroy(pchan->cmux);
  tt_i64_op(pchan->cmux->destroy_ctr, OP_EQ, 0);

 done:
  free_fake_orcirc(orcirc);
  free_fake_channel(pchan);
  free_fake_channel(nchan);
}

static void *
cmux_setup_test(const struct testcase_t *tc)
{
  static int whatever;

  (void) tc;

  cell_ewma_initialize_ticks();
  return &whatever;
}

static int
cmux_cleanup_test(const struct testcase_t *tc, void *ptr)
{
  (void) tc;
  (void) ptr;

  circuitmux_ewma_free_all();

  return 1;
}

static struct testcase_setup_t cmux_test_setup = {
  .setup_fn = cmux_setup_test,
  .cleanup_fn = cmux_cleanup_test,
};

#define TEST_CMUX(name) \
  { #name, test_cmux_##name, TT_FORK, &cmux_test_setup, NULL }

struct testcase_t circuitmux_tests[] = {
  /* Test circuitmux_t object */
  TEST_CMUX(allocate),
  TEST_CMUX(attach_circuit),
  TEST_CMUX(detach_circuit),
  TEST_CMUX(detach_all_circuits),
  TEST_CMUX(policy),
  TEST_CMUX(xmit_cell),

  /* Misc. */
  TEST_CMUX(compute_ticks),
  TEST_CMUX(destroy_cell_queue),

  END_OF_TESTCASES
};
