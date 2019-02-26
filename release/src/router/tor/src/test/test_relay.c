/* Copyright (c) 2014-2019, The Tor Project, Inc. */
/* See LICENSE for licensing information */

#define CIRCUITBUILD_PRIVATE
#define RELAY_PRIVATE
#define REPHIST_PRIVATE
#include "core/or/or.h"
#include "core/or/circuitbuild.h"
#include "core/or/circuitlist.h"
#include "core/or/channeltls.h"
#include "feature/stats/rephist.h"
#include "core/or/relay.h"
#include "feature/stats/rephist.h"
#include "lib/container/order.h"
/* For init/free stuff */
#include "core/or/scheduler.h"

#include "core/or/cell_st.h"
#include "core/or/or_circuit_st.h"

/* Test suite stuff */
#include "test/test.h"
#include "test/fakechans.h"

static or_circuit_t * new_fake_orcirc(channel_t *nchan, channel_t *pchan);

static void test_relay_append_cell_to_circuit_queue(void *arg);

static or_circuit_t *
new_fake_orcirc(channel_t *nchan, channel_t *pchan)
{
  or_circuit_t *orcirc = NULL;
  circuit_t *circ = NULL;

  orcirc = tor_malloc_zero(sizeof(*orcirc));
  circ = &(orcirc->base_);
  circ->magic = OR_CIRCUIT_MAGIC;

  circuit_set_n_circid_chan(circ, get_unique_circ_id_by_chan(nchan), nchan);
  cell_queue_init(&(circ->n_chan_cells));

  circ->n_hop = NULL;
  circ->streams_blocked_on_n_chan = 0;
  circ->streams_blocked_on_p_chan = 0;
  circ->n_delete_pending = 0;
  circ->p_delete_pending = 0;
  circ->received_destroy = 0;
  circ->state = CIRCUIT_STATE_OPEN;
  circ->purpose = CIRCUIT_PURPOSE_OR;
  circ->package_window = CIRCWINDOW_START_MAX;
  circ->deliver_window = CIRCWINDOW_START_MAX;
  circ->n_chan_create_cell = NULL;

  circuit_set_p_circid_chan(orcirc, get_unique_circ_id_by_chan(pchan), pchan);
  cell_queue_init(&(orcirc->p_chan_cells));

  return orcirc;
}

static void
assert_circuit_ok_mock(const circuit_t *c)
{
  (void) c;
  return;
}

static void
test_relay_close_circuit(void *arg)
{
  channel_t *nchan = NULL, *pchan = NULL;
  or_circuit_t *orcirc = NULL;
  cell_t *cell = NULL;
  int old_count, new_count;

  (void)arg;

  /* Make fake channels to be nchan and pchan for the circuit */
  nchan = new_fake_channel();
  tt_assert(nchan);

  pchan = new_fake_channel();
  tt_assert(pchan);

  /* Make a fake orcirc */
  orcirc = new_fake_orcirc(nchan, pchan);
  tt_assert(orcirc);
  circuitmux_attach_circuit(nchan->cmux, TO_CIRCUIT(orcirc),
                            CELL_DIRECTION_OUT);
  circuitmux_attach_circuit(pchan->cmux, TO_CIRCUIT(orcirc),
                            CELL_DIRECTION_IN);

  /* Make a cell */
  cell = tor_malloc_zero(sizeof(cell_t));
  make_fake_cell(cell);

  MOCK(scheduler_channel_has_waiting_cells,
       scheduler_channel_has_waiting_cells_mock);
  MOCK(assert_circuit_ok,
       assert_circuit_ok_mock);

  /* Append it */
  old_count = get_mock_scheduler_has_waiting_cells_count();
  append_cell_to_circuit_queue(TO_CIRCUIT(orcirc), nchan, cell,
                               CELL_DIRECTION_OUT, 0);
  new_count = get_mock_scheduler_has_waiting_cells_count();
  tt_int_op(new_count, OP_EQ, old_count + 1);

  /* Now try the reverse direction */
  old_count = get_mock_scheduler_has_waiting_cells_count();
  append_cell_to_circuit_queue(TO_CIRCUIT(orcirc), pchan, cell,
                               CELL_DIRECTION_IN, 0);
  new_count = get_mock_scheduler_has_waiting_cells_count();
  tt_int_op(new_count, OP_EQ, old_count + 1);

  /* Ensure our write totals are 0 */
  tt_u64_op(find_largest_max(write_array), OP_EQ, 0);

  /* Mark the circuit for close */
  circuit_mark_for_close(TO_CIRCUIT(orcirc), 0);

  /* Check our write totals. */
  advance_obs(write_array);
  commit_max(write_array);
  /* Check for two cells plus overhead */
  tt_u64_op(find_largest_max(write_array), OP_EQ,
                             2*(get_cell_network_size(nchan->wide_circ_ids)
                                +TLS_PER_CELL_OVERHEAD));

  UNMOCK(scheduler_channel_has_waiting_cells);

  /* Get rid of the fake channels */
  MOCK(scheduler_release_channel, scheduler_release_channel_mock);
  channel_mark_for_close(nchan);
  channel_mark_for_close(pchan);
  UNMOCK(scheduler_release_channel);

  /* Shut down channels */
  channel_free_all();

 done:
  tor_free(cell);
  if (orcirc) {
    circuitmux_detach_circuit(nchan->cmux, TO_CIRCUIT(orcirc));
    circuitmux_detach_circuit(pchan->cmux, TO_CIRCUIT(orcirc));
    cell_queue_clear(&orcirc->base_.n_chan_cells);
    cell_queue_clear(&orcirc->p_chan_cells);
  }
  tor_free(orcirc);
  free_fake_channel(nchan);
  free_fake_channel(pchan);
  UNMOCK(assert_circuit_ok);

  return;
}

static void
test_relay_append_cell_to_circuit_queue(void *arg)
{
  channel_t *nchan = NULL, *pchan = NULL;
  or_circuit_t *orcirc = NULL;
  cell_t *cell = NULL;
  int old_count, new_count;

  (void)arg;

  /* Make fake channels to be nchan and pchan for the circuit */
  nchan = new_fake_channel();
  tt_assert(nchan);

  pchan = new_fake_channel();
  tt_assert(pchan);

  /* Make a fake orcirc */
  orcirc = new_fake_orcirc(nchan, pchan);
  tt_assert(orcirc);
  circuitmux_attach_circuit(nchan->cmux, TO_CIRCUIT(orcirc),
                            CELL_DIRECTION_OUT);
  circuitmux_attach_circuit(pchan->cmux, TO_CIRCUIT(orcirc),
                            CELL_DIRECTION_IN);

  /* Make a cell */
  cell = tor_malloc_zero(sizeof(cell_t));
  make_fake_cell(cell);

  MOCK(scheduler_channel_has_waiting_cells,
       scheduler_channel_has_waiting_cells_mock);

  /* Append it */
  old_count = get_mock_scheduler_has_waiting_cells_count();
  append_cell_to_circuit_queue(TO_CIRCUIT(orcirc), nchan, cell,
                               CELL_DIRECTION_OUT, 0);
  new_count = get_mock_scheduler_has_waiting_cells_count();
  tt_int_op(new_count, OP_EQ, old_count + 1);

  /* Now try the reverse direction */
  old_count = get_mock_scheduler_has_waiting_cells_count();
  append_cell_to_circuit_queue(TO_CIRCUIT(orcirc), pchan, cell,
                               CELL_DIRECTION_IN, 0);
  new_count = get_mock_scheduler_has_waiting_cells_count();
  tt_int_op(new_count, OP_EQ, old_count + 1);

  UNMOCK(scheduler_channel_has_waiting_cells);

  /* Get rid of the fake channels */
  MOCK(scheduler_release_channel, scheduler_release_channel_mock);
  channel_mark_for_close(nchan);
  channel_mark_for_close(pchan);
  UNMOCK(scheduler_release_channel);

  /* Shut down channels */
  channel_free_all();

 done:
  tor_free(cell);
  if (orcirc) {
    circuitmux_detach_circuit(nchan->cmux, TO_CIRCUIT(orcirc));
    circuitmux_detach_circuit(pchan->cmux, TO_CIRCUIT(orcirc));
    cell_queue_clear(&orcirc->base_.n_chan_cells);
    cell_queue_clear(&orcirc->p_chan_cells);
  }
  tor_free(orcirc);
  free_fake_channel(nchan);
  free_fake_channel(pchan);

  return;
}

struct testcase_t relay_tests[] = {
  { "append_cell_to_circuit_queue", test_relay_append_cell_to_circuit_queue,
    TT_FORK, NULL, NULL },
  { "close_circ_rephist", test_relay_close_circuit,
    TT_FORK, NULL, NULL },
  END_OF_TESTCASES
};
