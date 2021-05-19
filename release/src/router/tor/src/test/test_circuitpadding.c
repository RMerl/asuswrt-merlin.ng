#define CHANNEL_OBJECT_PRIVATE
#define TOR_TIMERS_PRIVATE
#define CIRCUITPADDING_PRIVATE
#define CIRCUITPADDING_MACHINES_PRIVATE
#define NETWORKSTATUS_PRIVATE
#define CRYPT_PATH_PRIVATE
#define RELAY_PRIVATE

#include "core/or/or.h"
#include "test/test.h"
#include "test/log_test_helpers.h"
#include "lib/testsupport/testsupport.h"
#include "core/or/connection_or.h"
#include "core/or/channel.h"
#include "core/or/channeltls.h"
#include "core/or/crypt_path.h"
#include <event.h>
#include "lib/evloop/compat_libevent.h"
#include "lib/time/compat_time.h"
#include "lib/defs/time.h"
#include "core/or/relay.h"
#include "core/or/circuitlist.h"
#include "core/or/circuitbuild.h"
#include "core/or/circuitpadding.h"
#include "core/or/circuitpadding_machines.h"
#include "core/or/extendinfo.h"
#include "core/mainloop/netstatus.h"
#include "core/crypto/relay_crypto.h"
#include "core/or/protover.h"
#include "feature/nodelist/nodelist.h"
#include "app/config/config.h"

#include "feature/nodelist/routerstatus_st.h"
#include "feature/nodelist/networkstatus_st.h"
#include "feature/nodelist/node_st.h"
#include "core/or/cell_st.h"
#include "core/or/crypt_path_st.h"
#include "core/or/or_circuit_st.h"
#include "core/or/origin_circuit_st.h"

#include "test/fakecircs.h"
#include "test/rng_test_helpers.h"

/* Start our monotime mocking at 1 second past whatever monotime_init()
 * thought the actual wall clock time was, for platforms with bad resolution
 * and weird timevalues during monotime_init() before mocking. */
#define MONOTIME_MOCK_START   (monotime_absolute_nsec()+\
                               TOR_NSEC_PER_USEC*TOR_USEC_PER_SEC)

extern smartlist_t *connection_array;
void circuit_expire_old_circuits_clientside(void);

circid_t get_unique_circ_id_by_chan(channel_t *chan);
void helper_create_basic_machine(void);
static void helper_create_conditional_machines(void);

channel_t *new_fake_channel(void);
void test_circuitpadding_negotiation(void *arg);
void test_circuitpadding_wronghop(void *arg);
void test_circuitpadding_conditions(void *arg);

void test_circuitpadding_serialize(void *arg);
void test_circuitpadding_rtt(void *arg);
void test_circuitpadding_tokens(void *arg);
void test_circuitpadding_state_length(void *arg);

static void
simulate_single_hop_extend(circuit_t *client, circuit_t *mid_relay,
                           int padding);
void free_fake_origin_circuit(origin_circuit_t *circ);

static int deliver_negotiated = 1;
static int64_t curr_mocked_time;

static node_t padding_node;
static node_t non_padding_node;

static channel_t dummy_channel;
static circpad_machine_spec_t circ_client_machine;

static void
timers_advance_and_run(int64_t msec_update)
{
  curr_mocked_time += msec_update*TOR_NSEC_PER_MSEC;
  monotime_coarse_set_mock_time_nsec(curr_mocked_time);
  monotime_set_mock_time_nsec(curr_mocked_time);
  timers_run_pending();
}

static void
nodes_init(void)
{
  padding_node.rs = tor_malloc_zero(sizeof(routerstatus_t));
  padding_node.rs->pv.supports_hs_setup_padding = 1;

  non_padding_node.rs = tor_malloc_zero(sizeof(routerstatus_t));
  non_padding_node.rs->pv.supports_hs_setup_padding = 0;
}

static void
nodes_free(void)
{
  tor_free(padding_node.rs);

  tor_free(non_padding_node.rs);
}

static const node_t *
node_get_by_id_mock(const char *identity_digest)
{
  if (identity_digest[0] == 1) {
    return &padding_node;
  } else if (identity_digest[0] == 0) {
    return &non_padding_node;
  }

  return NULL;
}

static const node_t *
circuit_get_nth_node_mock(origin_circuit_t *circ, int hop)
{
  (void) circ;
  (void) hop;

  return &padding_node;
}

void
free_fake_origin_circuit(origin_circuit_t *circ)
{
  circpad_circuit_free_all_machineinfos(TO_CIRCUIT(circ));
  circuit_clear_cpath(circ);
  tor_free(circ);
}

void dummy_nop_timer(void);

//static int dont_stop_libevent = 0;

static circuit_t *client_side;
static circuit_t *relay_side;

static int n_client_cells = 0;
static int n_relay_cells = 0;

static int
circuit_package_relay_cell_mock(cell_t *cell, circuit_t *circ,
                           cell_direction_t cell_direction,
                           crypt_path_t *layer_hint, streamid_t on_stream,
                           const char *filename, int lineno);

static void
circuitmux_attach_circuit_mock(circuitmux_t *cmux, circuit_t *circ,
                               cell_direction_t direction);

static void
circuitmux_attach_circuit_mock(circuitmux_t *cmux, circuit_t *circ,
                               cell_direction_t direction)
{
  (void)cmux;
  (void)circ;
  (void)direction;

  return;
}

static int
circuit_package_relay_cell_mock(cell_t *cell, circuit_t *circ,
                           cell_direction_t cell_direction,
                           crypt_path_t *layer_hint, streamid_t on_stream,
                           const char *filename, int lineno)
{
  (void)cell; (void)on_stream; (void)filename; (void)lineno;

  if (circ == client_side) {
    if (cell->payload[0] == RELAY_COMMAND_PADDING_NEGOTIATE) {
      // Deliver to relay
      circpad_handle_padding_negotiate(relay_side, cell);
    } else {

      int is_target_hop = circpad_padding_is_from_expected_hop(circ,
                                                             layer_hint);
      tt_int_op(cell_direction, OP_EQ, CELL_DIRECTION_OUT);
      tt_int_op(is_target_hop, OP_EQ, 1);

      // No need to pretend a padding cell was sent: This event is
      // now emitted internally when the circuitpadding code sends them.
      //circpad_cell_event_padding_sent(client_side);

      // Receive padding cell at middle
      circpad_deliver_recognized_relay_cell_events(relay_side,
              cell->payload[0], NULL);
    }
    n_client_cells++;
  } else if (circ == relay_side) {
    tt_int_op(cell_direction, OP_EQ, CELL_DIRECTION_IN);

    if (cell->payload[0] == RELAY_COMMAND_PADDING_NEGOTIATED) {
      // XXX: blah need right layer_hint..
      if (deliver_negotiated)
        circpad_handle_padding_negotiated(client_side, cell,
                                          TO_ORIGIN_CIRCUIT(client_side)
                                             ->cpath->next);
    } else if (cell->payload[0] == RELAY_COMMAND_PADDING_NEGOTIATE) {
      circpad_handle_padding_negotiate(client_side, cell);
    } else {
      // No need to pretend a padding cell was sent: This event is
      // now emitted internally when the circuitpadding code sends them.
      //circpad_cell_event_padding_sent(relay_side);

      // Receive padding cell at client
      circpad_deliver_recognized_relay_cell_events(client_side,
              cell->payload[0],
              TO_ORIGIN_CIRCUIT(client_side)->cpath->next);
    }

    n_relay_cells++;
  }

 done:
  timers_advance_and_run(1);
  return 0;
}

// Test reading and writing padding to strings (or options_t + consensus)
void
test_circuitpadding_serialize(void *arg)
{
  (void)arg;
}

static signed_error_t
circpad_send_command_to_hop_mock(origin_circuit_t *circ, uint8_t hopnum,
                                 uint8_t relay_command, const uint8_t *payload,
                                 ssize_t payload_len)
{
  (void) circ;
  (void) hopnum;
  (void) relay_command;
  (void) payload;
  (void) payload_len;
  return 0;
}

void
test_circuitpadding_rtt(void *arg)
{
  /* Test Plan:
   *
   * 1. Test RTT measurement server side
   *    a. test usage of measured RTT
   * 2. Test termination of RTT measurement
   *    a. test non-update of RTT
   * 3. Test client side circuit and non-application of RTT..
   */
  circpad_delay_t rtt_estimate;
  int64_t actual_mocked_monotime_start;
  (void)arg;

  MOCK(circuitmux_attach_circuit, circuitmux_attach_circuit_mock);
  MOCK(circpad_send_command_to_hop, circpad_send_command_to_hop_mock);
  testing_enable_reproducible_rng();

  dummy_channel.cmux = circuitmux_alloc();
  relay_side = TO_CIRCUIT(new_fake_orcirc(&dummy_channel, &dummy_channel));
  client_side = TO_CIRCUIT(origin_circuit_new());
  relay_side->purpose = CIRCUIT_PURPOSE_OR;
  client_side->purpose = CIRCUIT_PURPOSE_C_GENERAL;

  monotime_init();
  monotime_enable_test_mocking();
  actual_mocked_monotime_start = MONOTIME_MOCK_START;
  monotime_set_mock_time_nsec(actual_mocked_monotime_start);
  monotime_coarse_set_mock_time_nsec(actual_mocked_monotime_start);
  curr_mocked_time = actual_mocked_monotime_start;

  timers_initialize();
  circpad_machines_init();
  helper_create_basic_machine();

  MOCK(circuit_package_relay_cell,
       circuit_package_relay_cell_mock);

  client_side->padding_machine[0] = &circ_client_machine;
  client_side->padding_info[0] = circpad_circuit_machineinfo_new(client_side,
                                                                 0);

  relay_side->padding_machine[0] = &circ_client_machine;
  relay_side->padding_info[0] = circpad_circuit_machineinfo_new(client_side,0);

  /* Test 1: Test measuring RTT */
  circpad_cell_event_nonpadding_received(relay_side);
  tt_u64_op(relay_side->padding_info[0]->last_received_time_usec, OP_NE, 0);

  timers_advance_and_run(20);

  circpad_cell_event_nonpadding_sent(relay_side);
  tt_u64_op(relay_side->padding_info[0]->last_received_time_usec, OP_EQ, 0);

  tt_int_op(relay_side->padding_info[0]->rtt_estimate_usec, OP_GE, 19000);
  tt_int_op(relay_side->padding_info[0]->rtt_estimate_usec, OP_LE, 30000);
  tt_int_op(circpad_histogram_bin_to_usec(relay_side->padding_info[0], 0),
            OP_EQ,
            relay_side->padding_info[0]->rtt_estimate_usec+
            circpad_machine_current_state(
             relay_side->padding_info[0])->histogram_edges[0]);

  circpad_cell_event_nonpadding_received(relay_side);
  circpad_cell_event_nonpadding_received(relay_side);
  tt_u64_op(relay_side->padding_info[0]->last_received_time_usec, OP_NE, 0);
  timers_advance_and_run(20);
  circpad_cell_event_nonpadding_sent(relay_side);
  circpad_cell_event_nonpadding_sent(relay_side);
  tt_u64_op(relay_side->padding_info[0]->last_received_time_usec, OP_EQ, 0);

  tt_int_op(relay_side->padding_info[0]->rtt_estimate_usec, OP_GE, 20000);
  tt_int_op(relay_side->padding_info[0]->rtt_estimate_usec, OP_LE, 21000);
  tt_int_op(circpad_histogram_bin_to_usec(relay_side->padding_info[0], 0),
            OP_EQ,
            relay_side->padding_info[0]->rtt_estimate_usec+
            circpad_machine_current_state(
             relay_side->padding_info[0])->histogram_edges[0]);

  /* Test 2: Termination of RTT measurement (from the previous test) */
  tt_int_op(relay_side->padding_info[0]->stop_rtt_update, OP_EQ, 1);
  rtt_estimate = relay_side->padding_info[0]->rtt_estimate_usec;

  circpad_cell_event_nonpadding_received(relay_side);
  timers_advance_and_run(4);
  circpad_cell_event_nonpadding_sent(relay_side);

  tt_int_op(relay_side->padding_info[0]->rtt_estimate_usec, OP_EQ,
            rtt_estimate);
  tt_u64_op(relay_side->padding_info[0]->last_received_time_usec, OP_EQ, 0);
  tt_int_op(relay_side->padding_info[0]->stop_rtt_update, OP_EQ, 1);
  tt_int_op(circpad_histogram_bin_to_usec(relay_side->padding_info[0], 0),
            OP_EQ,
            relay_side->padding_info[0]->rtt_estimate_usec+
            circpad_machine_current_state(
             relay_side->padding_info[0])->histogram_edges[0]);

  /* Test 3: Make sure client side machine properly ignores RTT */
  circpad_cell_event_nonpadding_received(client_side);
  tt_u64_op(client_side->padding_info[0]->last_received_time_usec, OP_EQ, 0);

  timers_advance_and_run(20);
  circpad_cell_event_nonpadding_sent(client_side);
  tt_u64_op(client_side->padding_info[0]->last_received_time_usec, OP_EQ, 0);

  tt_int_op(client_side->padding_info[0]->rtt_estimate_usec, OP_EQ, 0);
  tt_int_op(circpad_histogram_bin_to_usec(client_side->padding_info[0], 0),
            OP_NE, client_side->padding_info[0]->rtt_estimate_usec);
  tt_int_op(circpad_histogram_bin_to_usec(client_side->padding_info[0], 0),
            OP_EQ,
            circpad_machine_current_state(
                client_side->padding_info[0])->histogram_edges[0]);
 done:
  free_fake_orcirc(TO_OR_CIRCUIT(relay_side));
  circuitmux_detach_all_circuits(dummy_channel.cmux, NULL);
  circuitmux_free(dummy_channel.cmux);
  timers_shutdown();
  monotime_disable_test_mocking();
  UNMOCK(circuit_package_relay_cell);
  UNMOCK(circuitmux_attach_circuit);
  tor_free(circ_client_machine.states);
  testing_disable_reproducible_rng();

  return;
}

void
helper_create_basic_machine(void)
{
  /* Start, burst */
  circpad_machine_states_init(&circ_client_machine, 2);

  circ_client_machine.name = "basic";

  circ_client_machine.states[CIRCPAD_STATE_START].
      next_state[CIRCPAD_EVENT_NONPADDING_RECV] = CIRCPAD_STATE_BURST;
  circ_client_machine.states[CIRCPAD_STATE_START].use_rtt_estimate = 1;

  circ_client_machine.states[CIRCPAD_STATE_BURST].
      next_state[CIRCPAD_EVENT_PADDING_RECV] = CIRCPAD_STATE_BURST;
  circ_client_machine.states[CIRCPAD_STATE_BURST].
      next_state[CIRCPAD_EVENT_NONPADDING_RECV] = CIRCPAD_STATE_BURST;

  circ_client_machine.states[CIRCPAD_STATE_BURST].
      next_state[CIRCPAD_EVENT_NONPADDING_SENT] = CIRCPAD_STATE_CANCEL;

  circ_client_machine.states[CIRCPAD_STATE_BURST].token_removal =
      CIRCPAD_TOKEN_REMOVAL_HIGHER;

  circ_client_machine.states[CIRCPAD_STATE_BURST].histogram_len = 5;

  circ_client_machine.states[CIRCPAD_STATE_BURST].histogram_edges[0] = 500;
  circ_client_machine.states[CIRCPAD_STATE_BURST].histogram_edges[1] = 2500;
  circ_client_machine.states[CIRCPAD_STATE_BURST].histogram_edges[2] = 5000;
  circ_client_machine.states[CIRCPAD_STATE_BURST].histogram_edges[3] = 10000;
  circ_client_machine.states[CIRCPAD_STATE_BURST].histogram_edges[4] = 20000;

  circ_client_machine.states[CIRCPAD_STATE_BURST].histogram[0] = 1;
  circ_client_machine.states[CIRCPAD_STATE_BURST].histogram[1] = 0;
  circ_client_machine.states[CIRCPAD_STATE_BURST].histogram[2] = 2;
  circ_client_machine.states[CIRCPAD_STATE_BURST].histogram[3] = 2;
  circ_client_machine.states[CIRCPAD_STATE_BURST].histogram[4] = 2;

  circ_client_machine.states[CIRCPAD_STATE_BURST].histogram_total_tokens = 7;
  circ_client_machine.states[CIRCPAD_STATE_BURST].use_rtt_estimate = 1;

  return;
}

#define BIG_HISTOGRAM_LEN 10

/** Setup a machine with a big histogram */
static void
helper_create_machine_with_big_histogram(circpad_removal_t removal_strategy)
{
  const int tokens_per_bin = 2;

  /* Start, burst */
  circpad_machine_states_init(&circ_client_machine, 2);

  circpad_state_t *burst_state =
    &circ_client_machine.states[CIRCPAD_STATE_BURST];

  circ_client_machine.states[CIRCPAD_STATE_START].
    next_state[CIRCPAD_EVENT_NONPADDING_RECV] = CIRCPAD_STATE_BURST;

  burst_state->next_state[CIRCPAD_EVENT_PADDING_RECV] = CIRCPAD_STATE_BURST;
  burst_state->next_state[CIRCPAD_EVENT_NONPADDING_RECV] =CIRCPAD_STATE_BURST;

  burst_state->next_state[CIRCPAD_EVENT_NONPADDING_SENT] =CIRCPAD_STATE_CANCEL;

  burst_state->token_removal = CIRCPAD_TOKEN_REMOVAL_HIGHER;

  burst_state->histogram_len = BIG_HISTOGRAM_LEN;

  int n_tokens = 0;
  int i;
  for (i = 0; i < BIG_HISTOGRAM_LEN ; i++) {
    burst_state->histogram[i] = tokens_per_bin;
    n_tokens += tokens_per_bin;
  }

  burst_state->histogram_edges[0] = 0;
  burst_state->histogram_edges[1] = 1;
  burst_state->histogram_edges[2] = 7;
  burst_state->histogram_edges[3] = 15;
  burst_state->histogram_edges[4] = 31;
  burst_state->histogram_edges[5] = 62;
  burst_state->histogram_edges[6] = 125;
  burst_state->histogram_edges[7] = 250;
  burst_state->histogram_edges[8] = 500;
  burst_state->histogram_edges[9] = 1000;

  burst_state->histogram_total_tokens = n_tokens;
  burst_state->length_dist.type = CIRCPAD_DIST_UNIFORM;
  burst_state->length_dist.param1 = n_tokens;
  burst_state->length_dist.param2 = n_tokens;
  burst_state->max_length = n_tokens;
  burst_state->length_includes_nonpadding = 1;
  burst_state->use_rtt_estimate = 0;
  burst_state->token_removal = removal_strategy;
}

static circpad_decision_t
circpad_machine_schedule_padding_mock(circpad_machine_runtime_t *mi)
{
  (void)mi;
  return 0;
}

static uint64_t
mock_monotime_absolute_usec(void)
{
  return 100;
}

/** Test higher token removal strategy by bin  */
static void
test_circuitpadding_token_removal_higher(void *arg)
{
  circpad_machine_runtime_t *mi;
  (void)arg;

  /* Mock it up */
  MOCK(monotime_absolute_usec, mock_monotime_absolute_usec);
  MOCK(circpad_machine_schedule_padding,circpad_machine_schedule_padding_mock);
  testing_enable_reproducible_rng();

  /* Setup test environment (time etc.) */
  client_side = TO_CIRCUIT(origin_circuit_new());
  client_side->purpose = CIRCUIT_PURPOSE_C_GENERAL;
  monotime_enable_test_mocking();

  /* Create test machine */
  helper_create_machine_with_big_histogram(CIRCPAD_TOKEN_REMOVAL_HIGHER);
  client_side->padding_machine[0] = &circ_client_machine;
  client_side->padding_info[0] =
    circpad_circuit_machineinfo_new(client_side, 0);

  /* move the machine to the right state */
  circpad_cell_event_nonpadding_received(client_side);
  tt_int_op(client_side->padding_info[0]->current_state, OP_EQ,
            CIRCPAD_STATE_BURST);

  /* Get the machine and setup tokens */
  mi = client_side->padding_info[0];
  tt_assert(mi);

  /*************************************************************************/

  uint64_t current_time = monotime_absolute_usec();

  /* Test left boundaries of each histogram bin: */
  const circpad_delay_t bin_left_bounds[] =
    {0, 1,    7,  15,  31,  62,  125, 250, 500, 1000, CIRCPAD_DELAY_INFINITE};
  for (int i = 0; i <= BIG_HISTOGRAM_LEN ; i++) {
    tt_uint_op(bin_left_bounds[i], OP_EQ,
               circpad_histogram_bin_to_usec(mi, i));
  }

  /* Test right boundaries of each histogram bin: */
  const circpad_delay_t bin_right_bounds[] =
    {0,    6,  14,  30,  61,  124, 249, 499, 999, CIRCPAD_DELAY_INFINITE-1};
  for (int i = 0; i < BIG_HISTOGRAM_LEN ; i++) {
    tt_uint_op(bin_right_bounds[i], OP_EQ,
               histogram_get_bin_upper_bound(mi, i));
  }

  /* Check that all bins have two tokens right now */
  for (int i = 0; i < BIG_HISTOGRAM_LEN ; i++) {
    tt_int_op(mi->histogram[i], OP_EQ, 2);
  }

  /* This is the right order to remove tokens from this histogram. That is, we
   * first remove tokens from the 4th bin since 57 usec is nearest to the 4th
   * bin midpoint (31 + (62-31)/2 == 46). Then we remove from the 3rd bin for
   * the same reason, then from the 5th, etc. */
  const int bin_removal_order[] = {4, 5, 6, 7, 8};
  unsigned i;

  /* Remove all tokens from all bins apart from the infinity bin */
  for (i = 0; i < sizeof(bin_removal_order)/sizeof(int) ; i++) {
    int bin_to_remove = bin_removal_order[i];
    log_debug(LD_GENERAL, "Testing that %d attempt removes %d bin",
              i, bin_to_remove);

    tt_int_op(mi->histogram[bin_to_remove], OP_EQ, 2);

    mi->padding_scheduled_at_usec = current_time - 57;
    circpad_cell_event_nonpadding_sent(client_side);

    tt_int_op(mi->histogram[bin_to_remove], OP_EQ, 1);

    mi->padding_scheduled_at_usec = current_time - 57;
    circpad_cell_event_nonpadding_sent(client_side);

    /* Test that we cleaned out this bin. Don't do this in the case of the last
       bin since the tokens will get refilled */
    if (i != BIG_HISTOGRAM_LEN - 2) {
      tt_int_op(mi->histogram[bin_to_remove], OP_EQ, 0);
    }
  }

  /* Check that all lower bins are not touched */
  for (i=0; i < 4 ; i++) {
    tt_int_op(mi->histogram[i], OP_EQ, 2);
  }

  /* Test below the lowest bin, for coverage */
  tt_int_op(client_side->padding_info[0]->current_state, OP_EQ,
            CIRCPAD_STATE_BURST);
  circ_client_machine.states[CIRCPAD_STATE_BURST].histogram_edges[0] = 100;
  mi->padding_scheduled_at_usec = current_time;
  circpad_cell_event_nonpadding_sent(client_side);
  tt_int_op(mi->histogram[0], OP_EQ, 1);

 done:
  free_fake_origin_circuit(TO_ORIGIN_CIRCUIT(client_side));
  monotime_disable_test_mocking();
  tor_free(circ_client_machine.states);
  testing_disable_reproducible_rng();
}

/** Test lower token removal strategy by bin  */
static void
test_circuitpadding_token_removal_lower(void *arg)
{
  circpad_machine_runtime_t *mi;
  (void)arg;

  /* Mock it up */
  MOCK(monotime_absolute_usec, mock_monotime_absolute_usec);
  MOCK(circpad_machine_schedule_padding,circpad_machine_schedule_padding_mock);
  testing_enable_reproducible_rng();

  /* Setup test environment (time etc.) */
  client_side = TO_CIRCUIT(origin_circuit_new());
  client_side->purpose = CIRCUIT_PURPOSE_C_GENERAL;
  monotime_enable_test_mocking();

  /* Create test machine */
  helper_create_machine_with_big_histogram(CIRCPAD_TOKEN_REMOVAL_LOWER);
  client_side->padding_machine[0] = &circ_client_machine;
  client_side->padding_info[0] =
    circpad_circuit_machineinfo_new(client_side, 0);

  /* move the machine to the right state */
  circpad_cell_event_nonpadding_received(client_side);
  tt_int_op(client_side->padding_info[0]->current_state, OP_EQ,
            CIRCPAD_STATE_BURST);

  /* Get the machine and setup tokens */
  mi = client_side->padding_info[0];
  tt_assert(mi);

  /*************************************************************************/

  uint64_t current_time = monotime_absolute_usec();

  /* Test left boundaries of each histogram bin: */
  const circpad_delay_t bin_left_bounds[] =
    {0, 1,    7,  15,  31,  62,  125, 250, 500, 1000, CIRCPAD_DELAY_INFINITE};
  for (int i = 0; i <= BIG_HISTOGRAM_LEN ; i++) {
    tt_uint_op(bin_left_bounds[i], OP_EQ,
               circpad_histogram_bin_to_usec(mi, i));
  }

  /* Check that all bins have two tokens right now */
  for (int i = 0; i < BIG_HISTOGRAM_LEN ; i++) {
    tt_int_op(mi->histogram[i], OP_EQ, 2);
  }

  /* This is the right order to remove tokens from this histogram. That is, we
   * first remove tokens from the 4th bin since 57 usec is nearest to the 4th
   * bin midpoint (31 + (62-31)/2 == 46). Then we remove from the 3rd bin for
   * the same reason, then from the 5th, etc. */
  const int bin_removal_order[] = {4, 3, 2, 1, 0};
  unsigned i;

  /* Remove all tokens from all bins apart from the infinity bin */
  for (i = 0; i < sizeof(bin_removal_order)/sizeof(int) ; i++) {
    int bin_to_remove = bin_removal_order[i];
    log_debug(LD_GENERAL, "Testing that %d attempt removes %d bin",
              i, bin_to_remove);

    tt_int_op(mi->histogram[bin_to_remove], OP_EQ, 2);

    mi->padding_scheduled_at_usec = current_time - 57;
    circpad_cell_event_nonpadding_sent(client_side);

    tt_int_op(mi->histogram[bin_to_remove], OP_EQ, 1);

    mi->padding_scheduled_at_usec = current_time - 57;
    circpad_cell_event_nonpadding_sent(client_side);

    /* Test that we cleaned out this bin. Don't do this in the case of the last
       bin since the tokens will get refilled */
    if (i != BIG_HISTOGRAM_LEN - 2) {
      tt_int_op(mi->histogram[bin_to_remove], OP_EQ, 0);
    }
  }

  /* Check that all higher bins are untouched */
  for (i = 5; i < BIG_HISTOGRAM_LEN ; i++) {
    tt_int_op(mi->histogram[i], OP_EQ, 2);
  }

  /* Test above the highest bin, for coverage */
  tt_int_op(client_side->padding_info[0]->current_state, OP_EQ,
            CIRCPAD_STATE_BURST);
  circ_client_machine.states[CIRCPAD_STATE_BURST].
    histogram_edges[BIG_HISTOGRAM_LEN-2] = 100;
  mi->padding_scheduled_at_usec = current_time - 29202;
  circpad_cell_event_nonpadding_sent(client_side);
  tt_int_op(mi->histogram[BIG_HISTOGRAM_LEN-2], OP_EQ, 1);

 done:
  free_fake_origin_circuit(TO_ORIGIN_CIRCUIT(client_side));
  monotime_disable_test_mocking();
  tor_free(circ_client_machine.states);
  testing_disable_reproducible_rng();
}

/** Test closest token removal strategy by bin  */
static void
test_circuitpadding_closest_token_removal(void *arg)
{
  circpad_machine_runtime_t *mi;
  (void)arg;

  /* Mock it up */
  MOCK(monotime_absolute_usec, mock_monotime_absolute_usec);
  MOCK(circpad_machine_schedule_padding,circpad_machine_schedule_padding_mock);
  testing_enable_reproducible_rng();

  /* Setup test environment (time etc.) */
  client_side = TO_CIRCUIT(origin_circuit_new());
  client_side->purpose = CIRCUIT_PURPOSE_C_GENERAL;
  monotime_enable_test_mocking();

  /* Create test machine */
  helper_create_machine_with_big_histogram(CIRCPAD_TOKEN_REMOVAL_CLOSEST);
  client_side->padding_machine[0] = &circ_client_machine;
  client_side->padding_info[0] =
    circpad_circuit_machineinfo_new(client_side, 0);

  /* move the machine to the right state */
  circpad_cell_event_nonpadding_received(client_side);
  tt_int_op(client_side->padding_info[0]->current_state, OP_EQ,
            CIRCPAD_STATE_BURST);

  /* Get the machine and setup tokens */
  mi = client_side->padding_info[0];
  tt_assert(mi);

  /*************************************************************************/

  uint64_t current_time = monotime_absolute_usec();

  /* Test left boundaries of each histogram bin: */
  const circpad_delay_t bin_left_bounds[] =
    {0, 1,    7,  15,  31,  62,  125, 250, 500, 1000, CIRCPAD_DELAY_INFINITE};
  for (int i = 0; i <= BIG_HISTOGRAM_LEN ; i++) {
    tt_uint_op(bin_left_bounds[i], OP_EQ,
               circpad_histogram_bin_to_usec(mi, i));
  }

  /* Check that all bins have two tokens right now */
  for (int i = 0; i < BIG_HISTOGRAM_LEN ; i++) {
    tt_int_op(mi->histogram[i], OP_EQ, 2);
  }

  /* This is the right order to remove tokens from this histogram. That is, we
   * first remove tokens from the 4th bin since 57 usec is nearest to the 4th
   * bin midpoint (31 + (62-31)/2 == 46). Then we remove from the 3rd bin for
   * the same reason, then from the 5th, etc. */
  const int bin_removal_order[] = {4, 3, 5, 2, 6, 1, 7, 0, 8, 9};

  /* Remove all tokens from all bins apart from the infinity bin */
  for (int i = 0; i < BIG_HISTOGRAM_LEN-1 ; i++) {
    int bin_to_remove = bin_removal_order[i];
    log_debug(LD_GENERAL, "Testing that %d attempt removes %d bin",
              i, bin_to_remove);

    tt_int_op(mi->histogram[bin_to_remove], OP_EQ, 2);

    mi->padding_scheduled_at_usec = current_time - 57;
    circpad_cell_event_nonpadding_sent(client_side);

    tt_int_op(mi->histogram[bin_to_remove], OP_EQ, 1);

    mi->padding_scheduled_at_usec = current_time - 57;
    circpad_cell_event_nonpadding_sent(client_side);

    /* Test that we cleaned out this bin. Don't do this in the case of the last
       bin since the tokens will get refilled */
    if (i != BIG_HISTOGRAM_LEN - 2) {
      tt_int_op(mi->histogram[bin_to_remove], OP_EQ, 0);
    }
  }

  /* Check that all bins have been refilled */
  for (int i = 0; i < BIG_HISTOGRAM_LEN ; i++) {
    tt_int_op(mi->histogram[i], OP_EQ, 2);
  }

  /* Test below the lowest bin, for coverage */
  tt_int_op(client_side->padding_info[0]->current_state, OP_EQ,
            CIRCPAD_STATE_BURST);
  circ_client_machine.states[CIRCPAD_STATE_BURST].histogram_edges[0] = 100;
  circ_client_machine.states[CIRCPAD_STATE_BURST].histogram_edges[1] = 101;
  circ_client_machine.states[CIRCPAD_STATE_BURST].histogram_edges[2] = 120;
  mi->padding_scheduled_at_usec = current_time - 102;
  mi->histogram[0] = 0;
  circpad_cell_event_nonpadding_sent(client_side);
  tt_int_op(mi->histogram[1], OP_EQ, 1);

  /* Test above the highest bin, for coverage */
  tt_int_op(client_side->padding_info[0]->current_state, OP_EQ,
            CIRCPAD_STATE_BURST);
  mi->padding_scheduled_at_usec = current_time - 29202;
  circpad_cell_event_nonpadding_sent(client_side);
  tt_int_op(mi->histogram[BIG_HISTOGRAM_LEN-2], OP_EQ, 1);

 done:
  free_fake_origin_circuit(TO_ORIGIN_CIRCUIT(client_side));
  monotime_disable_test_mocking();
  tor_free(circ_client_machine.states);
  testing_disable_reproducible_rng();
}

/** Test closest token removal strategy with usec  */
static void
test_circuitpadding_closest_token_removal_usec(void *arg)
{
  circpad_machine_runtime_t *mi;
  (void)arg;

  /* Mock it up */
  MOCK(monotime_absolute_usec, mock_monotime_absolute_usec);
  MOCK(circpad_machine_schedule_padding,circpad_machine_schedule_padding_mock);
  testing_enable_reproducible_rng();

  /* Setup test environment (time etc.) */
  client_side = TO_CIRCUIT(origin_circuit_new());
  client_side->purpose = CIRCUIT_PURPOSE_C_GENERAL;
  monotime_enable_test_mocking();

  /* Create test machine */
  helper_create_machine_with_big_histogram(CIRCPAD_TOKEN_REMOVAL_CLOSEST_USEC);
  client_side->padding_machine[0] = &circ_client_machine;
  client_side->padding_info[0] =
    circpad_circuit_machineinfo_new(client_side, 0);

  /* move the machine to the right state */
  circpad_cell_event_nonpadding_received(client_side);
  tt_int_op(client_side->padding_info[0]->current_state, OP_EQ,
            CIRCPAD_STATE_BURST);

  /* Get the machine and setup tokens */
  mi = client_side->padding_info[0];
  tt_assert(mi);

  /*************************************************************************/

  uint64_t current_time = monotime_absolute_usec();

  /* Test left boundaries of each histogram bin: */
  const circpad_delay_t bin_left_bounds[] =
    {0, 1,    7,  15,  31,  62,  125, 250, 500, 1000, CIRCPAD_DELAY_INFINITE};
  for (int i = 0; i <= BIG_HISTOGRAM_LEN ; i++) {
    tt_uint_op(bin_left_bounds[i], OP_EQ,
               circpad_histogram_bin_to_usec(mi, i));
  }

  /* XXX we want to test remove_token_exact and
     circpad_machine_remove_closest_token() with usec */

  /* Check that all bins have two tokens right now */
  for (int i = 0; i < BIG_HISTOGRAM_LEN ; i++) {
    tt_int_op(mi->histogram[i], OP_EQ, 2);
  }

  /* This is the right order to remove tokens from this histogram. That is, we
   * first remove tokens from the 4th bin since 57 usec is nearest to the 4th
   * bin midpoint (31 + (62-31)/2 == 46). Then we remove from the 3rd bin for
   * the same reason, then from the 5th, etc. */
  const int bin_removal_order[] = {4, 3, 5, 2, 1, 0, 6, 7, 8, 9};

  /* Remove all tokens from all bins apart from the infinity bin */
  for (int i = 0; i < BIG_HISTOGRAM_LEN-1 ; i++) {
    int bin_to_remove = bin_removal_order[i];
    log_debug(LD_GENERAL, "Testing that %d attempt removes %d bin",
              i, bin_to_remove);

    tt_int_op(mi->histogram[bin_to_remove], OP_EQ, 2);

    mi->padding_scheduled_at_usec = current_time - 57;
    circpad_cell_event_nonpadding_sent(client_side);

    tt_int_op(mi->histogram[bin_to_remove], OP_EQ, 1);

    mi->padding_scheduled_at_usec = current_time - 57;
    circpad_cell_event_nonpadding_sent(client_side);

    /* Test that we cleaned out this bin. Don't do this in the case of the last
       bin since the tokens will get refilled */
    if (i != BIG_HISTOGRAM_LEN - 2) {
      tt_int_op(mi->histogram[bin_to_remove], OP_EQ, 0);
    }
  }

  /* Check that all bins have been refilled */
  for (int i = 0; i < BIG_HISTOGRAM_LEN ; i++) {
    tt_int_op(mi->histogram[i], OP_EQ, 2);
  }

  /* Test below the lowest bin, for coverage */
  tt_int_op(client_side->padding_info[0]->current_state, OP_EQ,
            CIRCPAD_STATE_BURST);
  circ_client_machine.states[CIRCPAD_STATE_BURST].histogram_edges[0] = 100;
  circ_client_machine.states[CIRCPAD_STATE_BURST].histogram_edges[1] = 101;
  circ_client_machine.states[CIRCPAD_STATE_BURST].histogram_edges[2] = 120;
  mi->padding_scheduled_at_usec = current_time - 102;
  mi->histogram[0] = 0;
  circpad_cell_event_nonpadding_sent(client_side);
  tt_int_op(mi->histogram[1], OP_EQ, 1);

  /* Test above the highest bin, for coverage */
  tt_int_op(client_side->padding_info[0]->current_state, OP_EQ,
            CIRCPAD_STATE_BURST);
  circ_client_machine.states[CIRCPAD_STATE_BURST].
    histogram_edges[BIG_HISTOGRAM_LEN-2] = 100;
  mi->padding_scheduled_at_usec = current_time - 29202;
  circpad_cell_event_nonpadding_sent(client_side);
  tt_int_op(mi->histogram[BIG_HISTOGRAM_LEN-2], OP_EQ, 1);

 done:
  free_fake_origin_circuit(TO_ORIGIN_CIRCUIT(client_side));
  monotime_disable_test_mocking();
  tor_free(circ_client_machine.states);
  testing_disable_reproducible_rng();
}

/** Test closest token removal strategy with usec  */
static void
test_circuitpadding_token_removal_exact(void *arg)
{
  circpad_machine_runtime_t *mi;
  (void)arg;

  /* Mock it up */
  MOCK(monotime_absolute_usec, mock_monotime_absolute_usec);
  MOCK(circpad_machine_schedule_padding,circpad_machine_schedule_padding_mock);
  testing_enable_reproducible_rng();

  /* Setup test environment (time etc.) */
  client_side = TO_CIRCUIT(origin_circuit_new());
  client_side->purpose = CIRCUIT_PURPOSE_C_GENERAL;
  monotime_enable_test_mocking();

  /* Create test machine */
  helper_create_machine_with_big_histogram(CIRCPAD_TOKEN_REMOVAL_EXACT);
  client_side->padding_machine[0] = &circ_client_machine;
  client_side->padding_info[0] =
    circpad_circuit_machineinfo_new(client_side, 0);

  /* move the machine to the right state */
  circpad_cell_event_nonpadding_received(client_side);
  tt_int_op(client_side->padding_info[0]->current_state, OP_EQ,
            CIRCPAD_STATE_BURST);

  /* Get the machine and setup tokens */
  mi = client_side->padding_info[0];
  tt_assert(mi);

  /**********************************************************************/
  uint64_t current_time = monotime_absolute_usec();

  /* Ensure that we will clear out bin #4 with this usec */
  mi->padding_scheduled_at_usec = current_time - 57;
  tt_int_op(mi->histogram[4], OP_EQ, 2);
  circpad_cell_event_nonpadding_sent(client_side);
  mi->padding_scheduled_at_usec = current_time - 57;
  tt_int_op(mi->histogram[4], OP_EQ, 1);
  circpad_cell_event_nonpadding_sent(client_side);
  tt_int_op(mi->histogram[4], OP_EQ, 0);

  /* Ensure that we will not remove any other tokens even tho we try to, since
   * this is what the exact strategy dictates */
  mi->padding_scheduled_at_usec = current_time - 57;
  circpad_cell_event_nonpadding_sent(client_side);
  for (int i = 0; i < BIG_HISTOGRAM_LEN ; i++) {
    if (i != 4) {
      tt_int_op(mi->histogram[i], OP_EQ, 2);
    }
  }

 done:
  free_fake_origin_circuit(TO_ORIGIN_CIRCUIT(client_side));
  monotime_disable_test_mocking();
  tor_free(circ_client_machine.states);
  testing_disable_reproducible_rng();
}

#undef BIG_HISTOGRAM_LEN

void
test_circuitpadding_tokens(void *arg)
{
  const circpad_state_t *state;
  circpad_machine_runtime_t *mi;
  int64_t actual_mocked_monotime_start;
  (void)arg;

  testing_enable_reproducible_rng();

  /** Test plan:
   *
   * 1. Test symmetry between bin_to_usec and usec_to_bin
   *    a. Test conversion
   *    b. Test edge transitions (lower, upper)
   * 2. Test remove higher on an empty bin
   *    a. Normal bin
   *    b. Infinity bin
   *    c. Bin 0
   *    d. No higher
   * 3. Test remove lower
   *    a. Normal bin
   *    b. Bin 0
   *    c. No lower
   * 4. Test remove closest
   *    a. Closest lower
   *    b. Closest higher
   *    c. Closest 0
   *    d. Closest Infinity
   */
  client_side = TO_CIRCUIT(origin_circuit_new());
  client_side->purpose = CIRCUIT_PURPOSE_C_GENERAL;

  monotime_init();
  monotime_enable_test_mocking();
  actual_mocked_monotime_start = MONOTIME_MOCK_START;
  monotime_set_mock_time_nsec(actual_mocked_monotime_start);
  monotime_coarse_set_mock_time_nsec(actual_mocked_monotime_start);
  curr_mocked_time = actual_mocked_monotime_start;

  /* This is needed so that we are not considered to be dormant */
  note_user_activity(20);

  timers_initialize();

  helper_create_basic_machine();
  client_side->padding_machine[0] = &circ_client_machine;
  client_side->padding_info[0] = circpad_circuit_machineinfo_new(client_side,
                                                                 0);

  mi = client_side->padding_info[0];

  // Pretend a non-padding cell was sent
  circpad_cell_event_nonpadding_received(client_side);
  circpad_cell_event_nonpadding_sent(client_side);
  /* We have to save the infinity bin because one inf delay
   * could have been chosen when we transition to burst */
  circpad_hist_token_t inf_bin = mi->histogram[4];

  tt_int_op(client_side->padding_info[0]->current_state, OP_EQ,
            CIRCPAD_STATE_BURST);

  state = circpad_machine_current_state(client_side->padding_info[0]);

  // Test 0: convert bin->usec->bin
  // Bin 0+1 have different semantics
  for (int bin = 0; bin < 2; bin++) {
    circpad_delay_t usec =
        circpad_histogram_bin_to_usec(client_side->padding_info[0], bin);
    int bin2 = circpad_histogram_usec_to_bin(client_side->padding_info[0],
                                             usec);
    tt_int_op(bin, OP_EQ, bin2);
  }
  for (int bin = 2; bin < state->histogram_len-1; bin++) {
    circpad_delay_t usec =
        circpad_histogram_bin_to_usec(client_side->padding_info[0], bin);
    int bin2 = circpad_histogram_usec_to_bin(client_side->padding_info[0],
                                             usec);
    tt_int_op(bin, OP_EQ, bin2);
    /* Verify we round down */
    bin2 = circpad_histogram_usec_to_bin(client_side->padding_info[0],
                                             usec+3);
    tt_int_op(bin, OP_EQ, bin2);

    bin2 = circpad_histogram_usec_to_bin(client_side->padding_info[0],
                                             usec-1);
    tt_int_op(bin, OP_EQ, bin2+1);
  }

  // Test 1: converting usec->bin->usec->bin
  // Bin 0+1 have different semantics.
  for (circpad_delay_t i = 0; i <= state->histogram_edges[0]; i++) {
    int bin = circpad_histogram_usec_to_bin(client_side->padding_info[0],
                                            i);
    circpad_delay_t usec =
        circpad_histogram_bin_to_usec(client_side->padding_info[0], bin);
    int bin2 = circpad_histogram_usec_to_bin(client_side->padding_info[0],
                                             usec);
    tt_int_op(bin, OP_EQ, bin2);
    tt_int_op(i, OP_LE, usec);
  }
  for (circpad_delay_t i = state->histogram_edges[0]+1;
       i <= state->histogram_edges[0] +
         state->histogram_edges[state->histogram_len-2]; i++) {
    int bin = circpad_histogram_usec_to_bin(client_side->padding_info[0],
                                            i);
    circpad_delay_t usec =
        circpad_histogram_bin_to_usec(client_side->padding_info[0], bin);
    int bin2 = circpad_histogram_usec_to_bin(client_side->padding_info[0],
                                             usec);
    tt_int_op(bin, OP_EQ, bin2);
    tt_int_op(i, OP_GE, usec);
  }

  /* 2.a. Normal higher bin */
  {
    tt_int_op(mi->histogram[2], OP_EQ, 2);
    tt_int_op(mi->histogram[3], OP_EQ, 2);
    circpad_machine_remove_higher_token(mi,
         circpad_histogram_bin_to_usec(mi, 2)+1);
    tt_int_op(mi->histogram[3], OP_EQ, 2);
    tt_int_op(mi->histogram[2], OP_EQ, 1);

    circpad_machine_remove_higher_token(mi,
         circpad_histogram_bin_to_usec(mi, 2)+1);
    tt_int_op(mi->histogram[2], OP_EQ, 0);

    tt_int_op(mi->histogram[3], OP_EQ, 2);
    circpad_machine_remove_higher_token(mi,
         circpad_histogram_bin_to_usec(mi, 2)+1);
    circpad_machine_remove_higher_token(mi,
         circpad_histogram_bin_to_usec(mi, 2)+1);
    tt_int_op(mi->histogram[3], OP_EQ, 0);
    circpad_machine_remove_higher_token(mi,
         circpad_histogram_bin_to_usec(mi, 2)+1);
    tt_int_op(mi->histogram[3], OP_EQ, 0);
  }

  /* 2.b. Higher Infinity bin */
  {
    tt_int_op(mi->histogram[4], OP_EQ, inf_bin);
    circpad_machine_remove_higher_token(mi,
         circpad_histogram_bin_to_usec(mi, 2)+1);
    tt_int_op(mi->histogram[4], OP_EQ, inf_bin);

    /* Test past the infinity bin */
    circpad_machine_remove_higher_token(mi,
         circpad_histogram_bin_to_usec(mi, 5)+1000000);

    tt_int_op(mi->histogram[4], OP_EQ, inf_bin);
  }

  /* 2.c. Bin 0 */
  {
    tt_int_op(mi->histogram[0], OP_EQ, 0);
    mi->histogram[0] = 1;
    circpad_machine_remove_higher_token(mi, state->histogram_edges[0]/2);
    tt_int_op(mi->histogram[0], OP_EQ, 0);
  }

  /* Drain the infinity bin and cause a refill */
  while (inf_bin != 0) {
    tt_int_op(mi->histogram[4], OP_EQ, inf_bin);
    circpad_cell_event_nonpadding_received(client_side);
    inf_bin--;
  }

  circpad_cell_event_nonpadding_sent(client_side);

  // We should have refilled here.
  tt_int_op(mi->histogram[4], OP_EQ, 2);

  /* 3.a. Bin 0 */
  {
    tt_int_op(mi->histogram[0], OP_EQ, 1);
    circpad_machine_remove_higher_token(mi, state->histogram_edges[0]/2);
    tt_int_op(mi->histogram[0], OP_EQ, 0);
  }

  /* 3.b. Test remove lower normal bin */
  {
    tt_int_op(mi->histogram[3], OP_EQ, 2);
    circpad_machine_remove_lower_token(mi,
         circpad_histogram_bin_to_usec(mi, 3)+1);
    circpad_machine_remove_lower_token(mi,
         circpad_histogram_bin_to_usec(mi, 3)+1);
    tt_int_op(mi->histogram[3], OP_EQ, 0);
    tt_int_op(mi->histogram[2], OP_EQ, 2);
    circpad_machine_remove_lower_token(mi,
         circpad_histogram_bin_to_usec(mi, 3)+1);
    circpad_machine_remove_lower_token(mi,
         circpad_histogram_bin_to_usec(mi, 3)+1);
    /* 3.c. No lower */
    circpad_machine_remove_lower_token(mi,
         circpad_histogram_bin_to_usec(mi, 3)+1);
    tt_int_op(mi->histogram[2], OP_EQ, 0);
  }

  /* 4. Test remove closest
   *    a. Closest lower
   *    b. Closest higher
   *    c. Closest 0
   *    d. Closest Infinity
   */
  circpad_machine_setup_tokens(mi);
  tt_int_op(mi->histogram[2], OP_EQ, 2);
  circpad_machine_remove_closest_token(mi,
         circpad_histogram_bin_to_usec(mi, 2)+1, 0);
  circpad_machine_remove_closest_token(mi,
         circpad_histogram_bin_to_usec(mi, 2)+1, 0);
  tt_int_op(mi->histogram[2], OP_EQ, 0);
  tt_int_op(mi->histogram[3], OP_EQ, 2);
  circpad_machine_remove_closest_token(mi,
         circpad_histogram_bin_to_usec(mi, 2)+1, 0);
  circpad_machine_remove_closest_token(mi,
         circpad_histogram_bin_to_usec(mi, 2)+1, 0);
  tt_int_op(mi->histogram[3], OP_EQ, 0);
  tt_int_op(mi->histogram[0], OP_EQ, 1);
  circpad_machine_remove_closest_token(mi,
         circpad_histogram_bin_to_usec(mi, 2)+1, 0);
  tt_int_op(mi->histogram[0], OP_EQ, 0);
  tt_int_op(mi->histogram[4], OP_EQ, 2);
  circpad_machine_remove_closest_token(mi,
         circpad_histogram_bin_to_usec(mi, 2)+1, 0);
  tt_int_op(mi->histogram[4], OP_EQ, 2);

  /* 5. Test remove closest usec
   *    a. Closest 0
   *    b. Closest lower (below midpoint)
   *    c. Closest higher (above midpoint)
   *    d. Closest Infinity
   */
  circpad_machine_setup_tokens(mi);

  tt_int_op(mi->histogram[0], OP_EQ, 1);
  circpad_machine_remove_closest_token(mi,
         circpad_histogram_bin_to_usec(mi, 0)/3, 1);
  tt_int_op(mi->histogram[0], OP_EQ, 0);
  tt_int_op(mi->histogram[2], OP_EQ, 2);
  circpad_machine_remove_closest_token(mi,
         circpad_histogram_bin_to_usec(mi, 0)/3, 1);
  circpad_machine_remove_closest_token(mi,
         circpad_histogram_bin_to_usec(mi, 0)/3, 1);
  tt_int_op(mi->histogram[2], OP_EQ, 0);
  tt_int_op(mi->histogram[3], OP_EQ, 2);
  circpad_machine_remove_closest_token(mi,
         circpad_histogram_bin_to_usec(mi, 4), 1);
  circpad_machine_remove_closest_token(mi,
         circpad_histogram_bin_to_usec(mi, 4), 1);
  tt_int_op(mi->histogram[3], OP_EQ, 0);
  tt_int_op(mi->histogram[4], OP_EQ, 2);
  circpad_machine_remove_closest_token(mi,
         circpad_histogram_bin_to_usec(mi, 4), 1);
  circpad_machine_remove_closest_token(mi,
         circpad_histogram_bin_to_usec(mi, 4), 1);
  tt_int_op(mi->histogram[4], OP_EQ, 2);

  // XXX: Need more coverage of the actual usec branches

 done:
  free_fake_origin_circuit(TO_ORIGIN_CIRCUIT(client_side));
  monotime_disable_test_mocking();
  tor_free(circ_client_machine.states);
  testing_disable_reproducible_rng();
}

void
test_circuitpadding_wronghop(void *arg)
{
  /**
   * Test plan:
   * 1. Padding sent from hop 1 and 3 to client
   * 2. Send negotiated from hop 1 and 3 to client
   * 3. Garbled negotiated cell
   * 4. Padding negotiate sent to client
   * 5. Send negotiate stop command for unknown machine
   * 6. Send negotiated to relay
   * 7. Garbled padding negotiate cell
   */
  (void)arg;
  uint32_t read_bw = 0, overhead_bw = 0;
  cell_t cell;
  signed_error_t ret;
  origin_circuit_t *orig_client;
  int64_t actual_mocked_monotime_start;

  MOCK(circuitmux_attach_circuit, circuitmux_attach_circuit_mock);

  /* Mock this function so that our cell counting tests don't get confused by
   * padding that gets sent by scheduled timers. */
  MOCK(circpad_machine_schedule_padding,circpad_machine_schedule_padding_mock);
  testing_enable_reproducible_rng();

  client_side = TO_CIRCUIT(origin_circuit_new());
  dummy_channel.cmux = circuitmux_alloc();
  relay_side = TO_CIRCUIT(new_fake_orcirc(&dummy_channel,
                                            &dummy_channel));
  orig_client = TO_ORIGIN_CIRCUIT(client_side);

  relay_side->purpose = CIRCUIT_PURPOSE_OR;
  client_side->purpose = CIRCUIT_PURPOSE_C_GENERAL;
  nodes_init();

  monotime_init();
  monotime_enable_test_mocking();
  actual_mocked_monotime_start = MONOTIME_MOCK_START;
  monotime_set_mock_time_nsec(actual_mocked_monotime_start);
  monotime_coarse_set_mock_time_nsec(actual_mocked_monotime_start);
  curr_mocked_time = actual_mocked_monotime_start;

  timers_initialize();
  circpad_machines_init();

  MOCK(node_get_by_id,
       node_get_by_id_mock);

  MOCK(circuit_package_relay_cell,
       circuit_package_relay_cell_mock);

  /* Build three hops */
  simulate_single_hop_extend(client_side, relay_side, 1);
  simulate_single_hop_extend(client_side, relay_side, 1);
  simulate_single_hop_extend(client_side, relay_side, 1);

  /* verify padding was negotiated */
  tt_ptr_op(relay_side->padding_machine[0], OP_NE, NULL);
  tt_ptr_op(relay_side->padding_info[0], OP_NE, NULL);

  /* verify echo was sent */
  tt_int_op(n_relay_cells, OP_EQ, 1);
  tt_int_op(n_client_cells, OP_EQ, 1);

  read_bw = orig_client->n_delivered_read_circ_bw;
  overhead_bw = orig_client->n_overhead_read_circ_bw;

  /* 1. Test padding from first and third hop */
  circpad_deliver_recognized_relay_cell_events(client_side,
              RELAY_COMMAND_DROP,
              TO_ORIGIN_CIRCUIT(client_side)->cpath);
  tt_int_op(read_bw, OP_EQ,
            orig_client->n_delivered_read_circ_bw);
  tt_int_op(overhead_bw, OP_EQ,
            orig_client->n_overhead_read_circ_bw);

  circpad_deliver_recognized_relay_cell_events(client_side,
              RELAY_COMMAND_DROP,
              TO_ORIGIN_CIRCUIT(client_side)->cpath->next->next);
  tt_int_op(read_bw, OP_EQ,
            orig_client->n_delivered_read_circ_bw);
  tt_int_op(overhead_bw, OP_EQ,
            orig_client->n_overhead_read_circ_bw);

  circpad_deliver_recognized_relay_cell_events(client_side,
              RELAY_COMMAND_DROP,
              TO_ORIGIN_CIRCUIT(client_side)->cpath->next);
  tt_int_op(read_bw, OP_EQ,
            orig_client->n_delivered_read_circ_bw);
  tt_int_op(overhead_bw, OP_LT,
            orig_client->n_overhead_read_circ_bw);

  /* 2. Test padding negotiated not handled from hops 1,3 */
  ret = circpad_handle_padding_negotiated(client_side, &cell,
          TO_ORIGIN_CIRCUIT(client_side)->cpath);
  tt_int_op(ret, OP_EQ, -1);

  ret = circpad_handle_padding_negotiated(client_side, &cell,
          TO_ORIGIN_CIRCUIT(client_side)->cpath->next->next);
  tt_int_op(ret, OP_EQ, -1);

  /* 3. Garbled negotiated cell */
  memset(&cell, 255, sizeof(cell));
  ret = circpad_handle_padding_negotiated(client_side, &cell,
          TO_ORIGIN_CIRCUIT(client_side)->cpath->next);
  tt_int_op(ret, OP_EQ, -1);

  /* 4. Test that negotiate is dropped at origin */
  read_bw = orig_client->n_delivered_read_circ_bw;
  overhead_bw = orig_client->n_overhead_read_circ_bw;
  relay_send_command_from_edge(0, relay_side,
                               RELAY_COMMAND_PADDING_NEGOTIATE,
                               (void*)cell.payload,
                               (size_t)3, NULL);
  tt_int_op(read_bw, OP_EQ,
            orig_client->n_delivered_read_circ_bw);
  tt_int_op(overhead_bw, OP_EQ,
            orig_client->n_overhead_read_circ_bw);

  tt_int_op(n_relay_cells, OP_EQ, 2);
  tt_int_op(n_client_cells, OP_EQ, 1);

  /* 5. Test that asking to stop the wrong machine does nothing */
  circpad_negotiate_padding(TO_ORIGIN_CIRCUIT(client_side),
                            255, 2, CIRCPAD_COMMAND_STOP, 0);
  tt_ptr_op(client_side->padding_machine[0], OP_NE, NULL);
  tt_ptr_op(client_side->padding_info[0], OP_NE, NULL);
  tt_ptr_op(relay_side->padding_machine[0], OP_NE, NULL);
  tt_ptr_op(relay_side->padding_info[0], OP_NE, NULL);
  tt_int_op(n_relay_cells, OP_EQ, 3);
  tt_int_op(n_client_cells, OP_EQ, 2);

  /* 6. Sending negotiated command to relay does nothing */
  ret = circpad_handle_padding_negotiated(relay_side, &cell, NULL);
  tt_int_op(ret, OP_EQ, -1);

  /* 7. Test garbled negotiated cell (bad command 255) */
  memset(&cell, 0, sizeof(cell));
  ret = circpad_handle_padding_negotiate(relay_side, &cell);
  tt_int_op(ret, OP_EQ, -1);
  tt_int_op(n_client_cells, OP_EQ, 2);

  /* Test 2: Test no padding */
  free_fake_origin_circuit(TO_ORIGIN_CIRCUIT(client_side));
  free_fake_orcirc(TO_OR_CIRCUIT(relay_side));

  client_side = TO_CIRCUIT(origin_circuit_new());
  relay_side = TO_CIRCUIT(new_fake_orcirc(&dummy_channel,
                                            &dummy_channel));
  relay_side->purpose = CIRCUIT_PURPOSE_OR;
  client_side->purpose = CIRCUIT_PURPOSE_C_GENERAL;

  simulate_single_hop_extend(client_side, relay_side, 1);
  simulate_single_hop_extend(client_side, relay_side, 0);

  /* verify no padding was negotiated */
  tt_ptr_op(relay_side->padding_machine[0], OP_EQ, NULL);
  tt_ptr_op(client_side->padding_machine[0], OP_EQ, NULL);
  tt_int_op(n_relay_cells, OP_EQ, 3);
  tt_int_op(n_client_cells, OP_EQ, 2);

  /* verify no echo was sent */
  tt_int_op(n_relay_cells, OP_EQ, 3);
  tt_int_op(n_client_cells, OP_EQ, 2);

  /* Finish circuit */
  simulate_single_hop_extend(client_side, relay_side, 1);

  /* Spoof padding negotiated on circuit with no padding */
  circpad_padding_negotiated(relay_side,
                             CIRCPAD_MACHINE_CIRC_SETUP,
                             CIRCPAD_COMMAND_START,
                             CIRCPAD_RESPONSE_OK, 0);

  /* verify no padding was negotiated */
  tt_ptr_op(relay_side->padding_machine[0], OP_EQ, NULL);
  tt_ptr_op(client_side->padding_machine[0], OP_EQ, NULL);

  circpad_padding_negotiated(relay_side,
                             CIRCPAD_MACHINE_CIRC_SETUP,
                             CIRCPAD_COMMAND_START,
                             CIRCPAD_RESPONSE_ERR, 0);

  /* verify no padding was negotiated */
  tt_ptr_op(relay_side->padding_machine[0], OP_EQ, NULL);
  tt_ptr_op(client_side->padding_machine[0], OP_EQ, NULL);

 done:
  free_fake_origin_circuit(TO_ORIGIN_CIRCUIT(client_side));
  free_fake_orcirc(TO_OR_CIRCUIT(relay_side));
  circuitmux_detach_all_circuits(dummy_channel.cmux, NULL);
  circuitmux_free(dummy_channel.cmux);
  monotime_disable_test_mocking();
  UNMOCK(node_get_by_id);
  UNMOCK(circuit_package_relay_cell);
  UNMOCK(circuitmux_attach_circuit);
  nodes_free();
  testing_disable_reproducible_rng();
}

void
test_circuitpadding_negotiation(void *arg)
{
  /**
   * Test plan:
   * 1. Test circuit where padding is supported by middle
   *    a. Make sure padding negotiation is sent
   *    b. Test padding negotiation delivery and parsing
   * 2. Test circuit where padding is unsupported by middle
   *    a. Make sure padding negotiation is not sent
   * 3. Test failure to negotiate a machine due to desync.
   */
  int64_t actual_mocked_monotime_start;
  (void)arg;

  MOCK(circuitmux_attach_circuit, circuitmux_attach_circuit_mock);

  client_side = TO_CIRCUIT(origin_circuit_new());
  dummy_channel.cmux = circuitmux_alloc();
  relay_side = TO_CIRCUIT(new_fake_orcirc(&dummy_channel, &dummy_channel));

  relay_side->purpose = CIRCUIT_PURPOSE_OR;
  client_side->purpose = CIRCUIT_PURPOSE_C_GENERAL;
  nodes_init();

  monotime_init();
  monotime_enable_test_mocking();
  actual_mocked_monotime_start = MONOTIME_MOCK_START;
  monotime_set_mock_time_nsec(actual_mocked_monotime_start);
  monotime_coarse_set_mock_time_nsec(actual_mocked_monotime_start);
  curr_mocked_time = actual_mocked_monotime_start;

  timers_initialize();
  circpad_machines_init();

  MOCK(node_get_by_id,
       node_get_by_id_mock);

  MOCK(circuit_package_relay_cell,
       circuit_package_relay_cell_mock);

  /* Build two hops */
  simulate_single_hop_extend(client_side, relay_side, 1);
  simulate_single_hop_extend(client_side, relay_side, 1);

  /* verify padding was negotiated */
  tt_ptr_op(relay_side->padding_machine[0], OP_NE, NULL);
  tt_ptr_op(relay_side->padding_info[0], OP_NE, NULL);

  /* verify echo was sent */
  tt_int_op(n_relay_cells, OP_EQ, 1);
  tt_int_op(n_client_cells, OP_EQ, 1);

  /* Finish circuit */
  simulate_single_hop_extend(client_side, relay_side, 1);

  /* Test 2: Test no padding */
  free_fake_origin_circuit(TO_ORIGIN_CIRCUIT(client_side));
  free_fake_orcirc(TO_OR_CIRCUIT(relay_side));

  client_side = TO_CIRCUIT(origin_circuit_new());
  relay_side = TO_CIRCUIT(new_fake_orcirc(&dummy_channel, &dummy_channel));
  relay_side->purpose = CIRCUIT_PURPOSE_OR;
  client_side->purpose = CIRCUIT_PURPOSE_C_GENERAL;

  simulate_single_hop_extend(client_side, relay_side, 1);
  simulate_single_hop_extend(client_side, relay_side, 0);

  /* verify no padding was negotiated */
  tt_ptr_op(client_side->padding_machine[0], OP_EQ, NULL);
  tt_ptr_op(relay_side->padding_machine[0], OP_EQ, NULL);
  tt_int_op(n_relay_cells, OP_EQ, 1);
  tt_int_op(n_client_cells, OP_EQ, 1);

  /* verify no echo was sent */
  tt_int_op(n_relay_cells, OP_EQ, 1);
  tt_int_op(n_client_cells, OP_EQ, 1);

  /* Finish circuit */
  simulate_single_hop_extend(client_side, relay_side, 1);

  /* Force negotiate padding. */
  circpad_negotiate_padding(TO_ORIGIN_CIRCUIT(client_side),
                            CIRCPAD_MACHINE_CIRC_SETUP,
                            2, CIRCPAD_COMMAND_START, 0);

  /* verify no padding was negotiated */
  tt_ptr_op(relay_side->padding_machine[0], OP_EQ, NULL);
  tt_ptr_op(client_side->padding_machine[0], OP_EQ, NULL);

  /* verify no echo was sent */
  tt_int_op(n_relay_cells, OP_EQ, 1);
  tt_int_op(n_client_cells, OP_EQ, 1);

  /* 3. Test failure to negotiate a machine due to desync */
  free_fake_origin_circuit(TO_ORIGIN_CIRCUIT(client_side));
  free_fake_orcirc(TO_OR_CIRCUIT(relay_side));

  client_side = TO_CIRCUIT(origin_circuit_new());
  relay_side = TO_CIRCUIT(new_fake_orcirc(&dummy_channel, &dummy_channel));
  relay_side->purpose = CIRCUIT_PURPOSE_OR;
  client_side->purpose = CIRCUIT_PURPOSE_C_GENERAL;

  SMARTLIST_FOREACH(relay_padding_machines,
          circpad_machine_spec_t *,
          m, tor_free(m->states); tor_free(m));
  smartlist_free(relay_padding_machines);
  relay_padding_machines = smartlist_new();

  simulate_single_hop_extend(client_side, relay_side, 1);
  simulate_single_hop_extend(client_side, relay_side, 1);

  /* verify echo was sent */
  tt_int_op(n_client_cells, OP_EQ, 2);
  tt_int_op(n_relay_cells, OP_EQ, 2);

  /* verify no padding was negotiated */
  tt_ptr_op(client_side->padding_info[0], OP_EQ, NULL);
  tt_ptr_op(client_side->padding_machine[0], OP_EQ, NULL);
  tt_ptr_op(relay_side->padding_machine[0], OP_EQ, NULL);
  tt_ptr_op(relay_side->padding_info[0], OP_EQ, NULL);

 done:
  free_fake_origin_circuit(TO_ORIGIN_CIRCUIT(client_side));
  free_fake_orcirc(TO_OR_CIRCUIT(relay_side));
  circuitmux_detach_all_circuits(dummy_channel.cmux, NULL);
  circuitmux_free(dummy_channel.cmux);
  monotime_disable_test_mocking();
  UNMOCK(node_get_by_id);
  UNMOCK(circuit_package_relay_cell);
  UNMOCK(circuitmux_attach_circuit);
  nodes_free();
}

static void
simulate_single_hop_extend(circuit_t *client, circuit_t *mid_relay,
                           int padding)
{
  char whatevs_key[CPATH_KEY_MATERIAL_LEN];
  char digest[DIGEST_LEN];
  tor_addr_t addr;

  // Pretend a non-padding cell was sent
  circpad_cell_event_nonpadding_sent(client);

  // Receive extend cell at middle
  circpad_cell_event_nonpadding_received(mid_relay);

  // Advance time a tiny bit so we can calculate an RTT
  curr_mocked_time += 10 * TOR_NSEC_PER_MSEC;
  monotime_coarse_set_mock_time_nsec(curr_mocked_time);
  monotime_set_mock_time_nsec(curr_mocked_time);

  // Receive extended cell at middle
  circpad_cell_event_nonpadding_sent(mid_relay);

  // Receive extended cell at first hop
  circpad_cell_event_nonpadding_received(client);

  // Add a hop to cpath
  crypt_path_t *hop = tor_malloc_zero(sizeof(crypt_path_t));
  cpath_extend_linked_list(&TO_ORIGIN_CIRCUIT(client)->cpath, hop);

  hop->magic = CRYPT_PATH_MAGIC;
  hop->state = CPATH_STATE_OPEN;

  // add an extend info to indicate if this node supports padding or not.
  // (set the first byte of the digest for our mocked node_get_by_id)
  digest[0] = padding;

  hop->extend_info = extend_info_new(
          padding ? "padding" : "non-padding",
          digest, NULL, NULL, NULL,
          &addr, padding);

  cpath_init_circuit_crypto(hop, whatevs_key, sizeof(whatevs_key), 0, 0);

  hop->package_window = circuit_initial_package_window();
  hop->deliver_window = CIRCWINDOW_START;

  // Signal that the hop was added
  circpad_machine_event_circ_added_hop(TO_ORIGIN_CIRCUIT(client));
}

static circpad_machine_spec_t *
helper_create_length_machine(void)
{
  circpad_machine_spec_t *ret =
    tor_malloc_zero(sizeof(circpad_machine_spec_t));

  /* Start, burst */
  circpad_machine_states_init(ret, 2);

  ret->states[CIRCPAD_STATE_START].
      next_state[CIRCPAD_EVENT_PADDING_SENT] = CIRCPAD_STATE_BURST;

  ret->states[CIRCPAD_STATE_BURST].
      next_state[CIRCPAD_EVENT_PADDING_SENT] = CIRCPAD_STATE_BURST;

  ret->states[CIRCPAD_STATE_BURST].
      next_state[CIRCPAD_EVENT_LENGTH_COUNT] = CIRCPAD_STATE_END;

  ret->states[CIRCPAD_STATE_BURST].
      next_state[CIRCPAD_EVENT_BINS_EMPTY] = CIRCPAD_STATE_END;

  /* No token removal.. end via state_length only */
  ret->states[CIRCPAD_STATE_BURST].token_removal =
      CIRCPAD_TOKEN_REMOVAL_NONE;

  /* Let's have this one end after 12 packets */
  ret->states[CIRCPAD_STATE_BURST].length_dist.type = CIRCPAD_DIST_UNIFORM;
  ret->states[CIRCPAD_STATE_BURST].length_dist.param1 = 12;
  ret->states[CIRCPAD_STATE_BURST].length_dist.param2 = 13;
  ret->states[CIRCPAD_STATE_BURST].max_length = 12;

  ret->states[CIRCPAD_STATE_BURST].histogram_len = 4;

  ret->states[CIRCPAD_STATE_BURST].histogram_edges[0] = 0;
  ret->states[CIRCPAD_STATE_BURST].histogram_edges[1] = 1;
  ret->states[CIRCPAD_STATE_BURST].histogram_edges[2] = 1000000;
  ret->states[CIRCPAD_STATE_BURST].histogram_edges[3] = 10000000;

  ret->states[CIRCPAD_STATE_BURST].histogram[0] = 0;
  ret->states[CIRCPAD_STATE_BURST].histogram[1] = 0;
  ret->states[CIRCPAD_STATE_BURST].histogram[2] = 6;

  ret->states[CIRCPAD_STATE_BURST].histogram_total_tokens = 6;
  ret->states[CIRCPAD_STATE_BURST].use_rtt_estimate = 0;
  ret->states[CIRCPAD_STATE_BURST].length_includes_nonpadding = 0;

  return ret;
}

static circpad_machine_spec_t *
helper_create_conditional_machine(void)
{
  circpad_machine_spec_t *ret =
    tor_malloc_zero(sizeof(circpad_machine_spec_t));

  /* Start, burst */
  circpad_machine_states_init(ret, 2);

  ret->states[CIRCPAD_STATE_START].
      next_state[CIRCPAD_EVENT_PADDING_SENT] = CIRCPAD_STATE_BURST;

  ret->states[CIRCPAD_STATE_BURST].
      next_state[CIRCPAD_EVENT_PADDING_SENT] = CIRCPAD_STATE_BURST;

  ret->states[CIRCPAD_STATE_BURST].
      next_state[CIRCPAD_EVENT_LENGTH_COUNT] = CIRCPAD_STATE_END;

  /* Use EXACT removal strategy, otherwise setup_tokens() does not work */
  ret->states[CIRCPAD_STATE_BURST].token_removal =
      CIRCPAD_TOKEN_REMOVAL_EXACT;

  ret->states[CIRCPAD_STATE_BURST].histogram_len = 3;

  ret->states[CIRCPAD_STATE_BURST].histogram_edges[0] = 0;
  ret->states[CIRCPAD_STATE_BURST].histogram_edges[1] = 1;
  ret->states[CIRCPAD_STATE_BURST].histogram_edges[2] = 1000000;

  ret->states[CIRCPAD_STATE_BURST].histogram[0] = 6;
  ret->states[CIRCPAD_STATE_BURST].histogram[1] = 0;
  ret->states[CIRCPAD_STATE_BURST].histogram[2] = 0;

  ret->states[CIRCPAD_STATE_BURST].histogram_total_tokens = 6;
  ret->states[CIRCPAD_STATE_BURST].use_rtt_estimate = 0;
  ret->states[CIRCPAD_STATE_BURST].length_includes_nonpadding = 1;

  return ret;
}

static void
helper_create_conditional_machines(void)
{
  circpad_machine_spec_t *add = helper_create_conditional_machine();

  if (!origin_padding_machines)
    origin_padding_machines = smartlist_new();
  if (!relay_padding_machines)
    relay_padding_machines = smartlist_new();

  add->machine_num = 2;
  add->is_origin_side = 1;
  add->should_negotiate_end = 1;
  add->target_hopnum = 2;

  /* Let's have this one end after 4 packets */
  add->states[CIRCPAD_STATE_BURST].length_dist.type = CIRCPAD_DIST_UNIFORM;
  add->states[CIRCPAD_STATE_BURST].length_dist.param1 = 4;
  add->states[CIRCPAD_STATE_BURST].length_dist.param2 = 4;
  add->states[CIRCPAD_STATE_BURST].max_length = 4;

  add->conditions.requires_vanguards = 0;
  add->conditions.min_hops = 2;
  add->conditions.apply_state_mask = CIRCPAD_CIRC_BUILDING|
           CIRCPAD_CIRC_NO_STREAMS|CIRCPAD_CIRC_HAS_RELAY_EARLY;
  add->conditions.apply_purpose_mask = CIRCPAD_PURPOSE_ALL;
  circpad_register_padding_machine(add, origin_padding_machines);

  add = helper_create_conditional_machine();
  add->machine_num = 3;
  add->is_origin_side = 1;
  add->should_negotiate_end = 1;
  add->target_hopnum = 2;

  /* Let's have this one end after 4 packets */
  add->states[CIRCPAD_STATE_BURST].length_dist.type = CIRCPAD_DIST_UNIFORM;
  add->states[CIRCPAD_STATE_BURST].length_dist.param1 = 4;
  add->states[CIRCPAD_STATE_BURST].length_dist.param2 = 4;
  add->states[CIRCPAD_STATE_BURST].max_length = 4;

  add->conditions.requires_vanguards = 1;
  add->conditions.min_hops = 3;
  add->conditions.apply_state_mask = CIRCPAD_CIRC_OPENED|
           CIRCPAD_CIRC_STREAMS|CIRCPAD_CIRC_HAS_NO_RELAY_EARLY;
  add->conditions.apply_purpose_mask = CIRCPAD_PURPOSE_ALL;
  circpad_register_padding_machine(add, origin_padding_machines);

  add = helper_create_conditional_machine();
  add->machine_num = 2;
  circpad_register_padding_machine(add, relay_padding_machines);

  add = helper_create_conditional_machine();
  add->machine_num = 3;
  circpad_register_padding_machine(add, relay_padding_machines);
}

void
test_circuitpadding_state_length(void *arg)
{
  /**
   * Test plan:
   *  * Explicitly test that with no token removal enabled, we hit
   *    the state length limit due to either padding, or non-padding.
   *  * Repeat test with an arbitrary token removal strategy, and
   *    verify that if we run out of tokens due to padding before we
   *    hit the state length, we still go to state end (all our
   *    token removal tests only test nonpadding token removal).
   */
  int64_t actual_mocked_monotime_start;
  (void)arg;
  MOCK(circuitmux_attach_circuit, circuitmux_attach_circuit_mock);
  MOCK(circpad_send_command_to_hop, circpad_send_command_to_hop_mock);

  nodes_init();
  dummy_channel.cmux = circuitmux_alloc();
  relay_side = TO_CIRCUIT(new_fake_orcirc(&dummy_channel,
                                            &dummy_channel));
  client_side = TO_CIRCUIT(origin_circuit_new());
  relay_side->purpose = CIRCUIT_PURPOSE_OR;
  client_side->purpose = CIRCUIT_PURPOSE_C_GENERAL;

  monotime_init();
  monotime_enable_test_mocking();
  actual_mocked_monotime_start = MONOTIME_MOCK_START;
  monotime_set_mock_time_nsec(actual_mocked_monotime_start);
  monotime_coarse_set_mock_time_nsec(actual_mocked_monotime_start);
  curr_mocked_time = actual_mocked_monotime_start;

  /* This is needed so that we are not considered to be dormant */
  note_user_activity(20);

  timers_initialize();
  circpad_machine_spec_t *client_machine =
      helper_create_length_machine();

  MOCK(circuit_package_relay_cell,
       circuit_package_relay_cell_mock);
  MOCK(node_get_by_id,
       node_get_by_id_mock);

  client_side->padding_machine[0] = client_machine;
  client_side->padding_info[0] =
    circpad_circuit_machineinfo_new(client_side, 0);
  circpad_machine_runtime_t *mi = client_side->padding_info[0];

  circpad_cell_event_padding_sent(client_side);
  tt_i64_op(mi->state_length, OP_EQ, 12);
  tt_ptr_op(mi->histogram, OP_EQ, NULL);

  /* Verify that non-padding does not change our state length */
  circpad_cell_event_nonpadding_sent(client_side);
  tt_i64_op(mi->state_length, OP_EQ, 12);

  /* verify that sending padding changes our state length */
  for (uint64_t i = mi->state_length-1; i > 0; i--) {
    circpad_send_padding_cell_for_callback(mi);
    tt_i64_op(mi->state_length, OP_EQ, i);
  }
  circpad_send_padding_cell_for_callback(mi);

  tt_i64_op(mi->state_length, OP_EQ, -1);
  tt_int_op(mi->current_state, OP_EQ, CIRCPAD_STATE_END);

  /* Restart machine */
  mi->current_state = CIRCPAD_STATE_START;

  /* Now, count nonpadding as part of the state length */
  client_machine->states[CIRCPAD_STATE_BURST].length_includes_nonpadding = 1;

  circpad_cell_event_padding_sent(client_side);
  tt_i64_op(mi->state_length, OP_EQ, 12);

  /* Verify that non-padding does change our state length now */
  for (uint64_t i = mi->state_length-1; i > 0; i--) {
    circpad_cell_event_nonpadding_sent(client_side);
    tt_i64_op(mi->state_length, OP_EQ, i);
  }

  circpad_cell_event_nonpadding_sent(client_side);
  tt_i64_op(mi->state_length, OP_EQ, -1);
  tt_int_op(mi->current_state, OP_EQ, CIRCPAD_STATE_END);

  /* Now, just test token removal when we send padding */
  client_machine->states[CIRCPAD_STATE_BURST].token_removal =
      CIRCPAD_TOKEN_REMOVAL_EXACT;

  /* Restart machine */
  mi->current_state = CIRCPAD_STATE_START;
  circpad_cell_event_padding_sent(client_side);
  tt_i64_op(mi->state_length, OP_EQ, 12);
  tt_ptr_op(mi->histogram, OP_NE, NULL);
  tt_int_op(mi->chosen_bin, OP_EQ, 2);

  /* verify that sending padding changes our state length and
   * our histogram now */
  for (uint32_t i = mi->histogram[2]-1; i > 0; i--) {
    circpad_send_padding_cell_for_callback(mi);
    tt_int_op(mi->chosen_bin, OP_EQ, 2);
    tt_int_op(mi->histogram[2], OP_EQ, i);
  }

  tt_i64_op(mi->state_length, OP_EQ, 7);
  tt_int_op(mi->histogram[2], OP_EQ, 1);

  circpad_send_padding_cell_for_callback(mi);
  tt_int_op(mi->current_state, OP_EQ, CIRCPAD_STATE_END);

 done:
  tor_free(client_machine->states);
  tor_free(client_machine);

  free_fake_origin_circuit(TO_ORIGIN_CIRCUIT(client_side));
  free_fake_orcirc(TO_OR_CIRCUIT(relay_side));

  circuitmux_detach_all_circuits(dummy_channel.cmux, NULL);
  circuitmux_free(dummy_channel.cmux);
  timers_shutdown();
  monotime_disable_test_mocking();
  UNMOCK(circuit_package_relay_cell);
  UNMOCK(circuitmux_attach_circuit);
  UNMOCK(node_get_by_id);

  return;
}

void
test_circuitpadding_conditions(void *arg)
{
  /**
   * Test plan:
   *  0. Make a few origin and client machines with diff conditions
   *     * vanguards, purposes, has_opened circs, no relay early
   *     * Client side should_negotiate_end
   *     * Length limits
   *  1. Test STATE_END transitions
   *  2. Test new machine after end with same conditions
   *  3. Test new machine due to changed conditions
   *     * Esp: built event, no relay early, no streams
   * XXX: Diff test:
   *  1. Test STATE_END with pending timers
   *  2. Test marking a circuit before padding callback fires
   *  3. Test freeing a circuit before padding callback fires
   */
  int64_t actual_mocked_monotime_start;
  (void)arg;
  MOCK(circuitmux_attach_circuit, circuitmux_attach_circuit_mock);
  testing_enable_reproducible_rng();

  nodes_init();
  dummy_channel.cmux = circuitmux_alloc();
  relay_side = TO_CIRCUIT(new_fake_orcirc(&dummy_channel,
                                            &dummy_channel));
  client_side = TO_CIRCUIT(origin_circuit_new());
  relay_side->purpose = CIRCUIT_PURPOSE_OR;
  client_side->purpose = CIRCUIT_PURPOSE_C_GENERAL;

  monotime_init();
  monotime_enable_test_mocking();
  actual_mocked_monotime_start = MONOTIME_MOCK_START;
  monotime_set_mock_time_nsec(actual_mocked_monotime_start);
  monotime_coarse_set_mock_time_nsec(actual_mocked_monotime_start);
  curr_mocked_time = actual_mocked_monotime_start;

  /* This is needed so that we are not considered to be dormant */
  note_user_activity(20);

  timers_initialize();
  helper_create_conditional_machines();

  MOCK(circuit_package_relay_cell,
       circuit_package_relay_cell_mock);
  MOCK(node_get_by_id,
       node_get_by_id_mock);

  /* Simulate extend. This should result in the original machine getting
   * added, since the circuit is not built */
  simulate_single_hop_extend(client_side, relay_side, 1);
  simulate_single_hop_extend(client_side, relay_side, 1);

  /* Verify that machine #2 is added */
  tt_int_op(client_side->padding_machine[0]->machine_num, OP_EQ, 2);
  tt_int_op(relay_side->padding_machine[0]->machine_num, OP_EQ, 2);

  /* Deliver a padding cell to the client, to trigger burst state */
  circpad_cell_event_padding_sent(client_side);

  /* This should have trigger length shutdown condition on client.. */
  tt_ptr_op(client_side->padding_info[0], OP_EQ, NULL);
  tt_ptr_op(client_side->padding_machine[0], OP_EQ, NULL);

  /* Verify machine is gone from both sides */
  tt_ptr_op(relay_side->padding_info[0], OP_EQ, NULL);
  tt_ptr_op(relay_side->padding_machine[0], OP_EQ, NULL);

  /* Send another event.. verify machine gets re-added properly
   * (test race with shutdown) */
  simulate_single_hop_extend(client_side, relay_side, 1);
  tt_int_op(client_side->padding_machine[0]->machine_num, OP_EQ, 2);
  tt_int_op(relay_side->padding_machine[0]->machine_num, OP_EQ, 2);

  TO_ORIGIN_CIRCUIT(client_side)->p_streams = 0;
  circpad_machine_event_circ_has_no_streams(TO_ORIGIN_CIRCUIT(client_side));
  tt_int_op(client_side->padding_machine[0]->machine_num, OP_EQ, 2);
  tt_int_op(relay_side->padding_machine[0]->machine_num, OP_EQ, 2);

  /* Now make the circuit opened and send built event */
  TO_ORIGIN_CIRCUIT(client_side)->has_opened = 1;
  circpad_machine_event_circ_built(TO_ORIGIN_CIRCUIT(client_side));
  tt_int_op(client_side->padding_machine[0]->machine_num, OP_EQ, 2);
  tt_int_op(relay_side->padding_machine[0]->machine_num, OP_EQ, 2);

  TO_ORIGIN_CIRCUIT(client_side)->remaining_relay_early_cells = 0;
  circpad_machine_event_circ_has_no_relay_early(
          TO_ORIGIN_CIRCUIT(client_side));
  tt_int_op(client_side->padding_machine[0]->machine_num, OP_EQ, 2);
  tt_int_op(relay_side->padding_machine[0]->machine_num, OP_EQ, 2);

  get_options_mutable()->HSLayer2Nodes = (void*)1;
  TO_ORIGIN_CIRCUIT(client_side)->p_streams = (void*)1;
  circpad_machine_event_circ_has_streams(TO_ORIGIN_CIRCUIT(client_side));

  /* Verify different machine is added */
  tt_int_op(client_side->padding_machine[0]->machine_num, OP_EQ, 3);
  tt_int_op(relay_side->padding_machine[0]->machine_num, OP_EQ, 3);

  /* Hold off on negotiated */
  deliver_negotiated = 0;

  /* Deliver a padding cell to the client, to trigger burst state */
  circpad_cell_event_padding_sent(client_side);

  /* This should have trigger length shutdown condition on client
   * but not the response for the padding machine */
  tt_ptr_op(client_side->padding_info[0], OP_EQ, NULL);
  tt_ptr_op(client_side->padding_machine[0], OP_NE, NULL);

  /* Verify machine is gone from the relay (but negotiated not back yet */
  tt_ptr_op(relay_side->padding_info[0], OP_EQ, NULL);
  tt_ptr_op(relay_side->padding_machine[0], OP_EQ, NULL);

  /* Add another hop and verify it's back */
  simulate_single_hop_extend(client_side, relay_side, 1);

  tt_int_op(client_side->padding_machine[0]->machine_num, OP_EQ, 3);
  tt_int_op(relay_side->padding_machine[0]->machine_num, OP_EQ, 3);

  tt_ptr_op(client_side->padding_info[0], OP_NE, NULL);
  tt_ptr_op(relay_side->padding_info[0], OP_NE, NULL);

 done:
  /* XXX: Free everything */
  testing_disable_reproducible_rng();
  return;
}

/** Disabled unstable test until #29298 is implemented (see #29122) */
#if 0
void
test_circuitpadding_circuitsetup_machine(void *arg)
{
  int64_t actual_mocked_monotime_start;
  /**
   * Test case plan:
   *
   * 1. Simulate a normal circuit setup pattern
   *    a. Application traffic
   *
   * FIXME: This should focus more on exercising the machine
   * features rather than actual traffic patterns. For example,
   * test cancellation and bins empty/refill
   */
  (void)arg;

  MOCK(circuitmux_attach_circuit, circuitmux_attach_circuit_mock);

  dummy_channel.cmux = circuitmux_alloc();
  client_side = TO_CIRCUIT(origin_circuit_new());
  relay_side = TO_CIRCUIT(new_fake_orcirc(&dummy_channel, &dummy_channel));

  relay_side->purpose = CIRCUIT_PURPOSE_OR;
  client_side->purpose = CIRCUIT_PURPOSE_C_GENERAL;

  nodes_init();

  monotime_init();
  monotime_enable_test_mocking();
  actual_mocked_monotime_start = MONOTIME_MOCK_START;
  monotime_set_mock_time_nsec(actual_mocked_monotime_start);
  monotime_coarse_set_mock_time_nsec(actual_mocked_monotime_start);
  curr_mocked_time = actual_mocked_monotime_start;

  timers_initialize();
  circpad_machines_init();

  MOCK(circuit_package_relay_cell,
       circuit_package_relay_cell_mock);
  MOCK(node_get_by_id,
       node_get_by_id_mock);

  /* Test case #1: Build a 3 hop circuit, then wait and let pad */
  simulate_single_hop_extend(client_side, relay_side, 1);
  simulate_single_hop_extend(client_side, relay_side, 1);
  simulate_single_hop_extend(client_side, relay_side, 1);

  tt_int_op(n_client_cells, OP_EQ, 1);
  tt_int_op(n_relay_cells, OP_EQ, 1);
  tt_int_op(client_side->padding_info[0]->current_state, OP_EQ,
                CIRCPAD_STATE_BURST);
  tt_int_op(relay_side->padding_info[0]->current_state, OP_EQ,
          CIRCPAD_STATE_BURST);

  tt_u64_op(client_side->padding_info[0]->padding_scheduled_at_usec,
            OP_NE, 0);
  tt_int_op(relay_side->padding_info[0]->is_padding_timer_scheduled,
            OP_EQ, 0);
  timers_advance_and_run(2000);
  tt_int_op(n_client_cells, OP_EQ, 2);
  tt_int_op(n_relay_cells, OP_EQ, 1);

  tt_int_op(relay_side->padding_info[0]->current_state, OP_EQ,
              CIRCPAD_STATE_GAP);

  tt_u64_op(client_side->padding_info[0]->padding_scheduled_at_usec,
            OP_EQ, 0);
  tt_u64_op(relay_side->padding_info[0]->padding_scheduled_at_usec,
            OP_NE, 0);
  timers_advance_and_run(5000);
  tt_int_op(n_client_cells, OP_EQ, 2);
  tt_int_op(n_relay_cells, OP_EQ, 2);

  tt_u64_op(client_side->padding_info[0]->padding_scheduled_at_usec,
            OP_NE, 0);
  tt_u64_op(relay_side->padding_info[0]->padding_scheduled_at_usec,
            OP_EQ, 0);
  timers_advance_and_run(2000);
  tt_int_op(n_client_cells, OP_EQ, 3);
  tt_int_op(n_relay_cells, OP_EQ, 2);

  tt_u64_op(client_side->padding_info[0]->padding_scheduled_at_usec,
            OP_EQ, 0);
  tt_u64_op(relay_side->padding_info[0]->padding_scheduled_at_usec,
            OP_NE, 0);
  timers_advance_and_run(5000);
  tt_int_op(n_client_cells, OP_EQ, 3);
  tt_int_op(n_relay_cells, OP_EQ, 3);

  tt_u64_op(client_side->padding_info[0]->padding_scheduled_at_usec,
            OP_NE, 0);
  tt_u64_op(relay_side->padding_info[0]->padding_scheduled_at_usec,
            OP_EQ, 0);
  timers_advance_and_run(2000);
  tt_int_op(n_client_cells, OP_EQ, 4);
  tt_int_op(n_relay_cells, OP_EQ, 3);

  tt_u64_op(client_side->padding_info[0]->padding_scheduled_at_usec,
            OP_EQ, 0);
  tt_u64_op(relay_side->padding_info[0]->padding_scheduled_at_usec,
            OP_NE, 0);
  timers_advance_and_run(5000);
  tt_int_op(n_client_cells, OP_EQ, 4);
  tt_int_op(n_relay_cells, OP_EQ, 4);

  tt_u64_op(client_side->padding_info[0]->padding_scheduled_at_usec,
            OP_NE, 0);
  tt_u64_op(relay_side->padding_info[0]->padding_scheduled_at_usec,
            OP_EQ, 0);
  timers_advance_and_run(2000);
  tt_int_op(n_client_cells, OP_EQ, 5);
  tt_int_op(n_relay_cells, OP_EQ, 4);

  tt_u64_op(client_side->padding_info[0]->padding_scheduled_at_usec,
            OP_EQ, 0);
  tt_u64_op(relay_side->padding_info[0]->padding_scheduled_at_usec,
            OP_NE, 0);
  timers_advance_and_run(5000);
  tt_int_op(n_client_cells, OP_EQ, 5);
  tt_int_op(n_relay_cells, OP_EQ, 5);

  tt_u64_op(client_side->padding_info[0]->padding_scheduled_at_usec,
            OP_NE, 0);
  tt_u64_op(relay_side->padding_info[0]->padding_scheduled_at_usec,
            OP_EQ, 0);
  timers_advance_and_run(2000);
  tt_int_op(n_client_cells, OP_EQ, 6);
  tt_int_op(n_relay_cells, OP_EQ, 5);

  tt_u64_op(client_side->padding_info[0]->padding_scheduled_at_usec,
            OP_EQ, 0);
  tt_u64_op(relay_side->padding_info[0]->padding_scheduled_at_usec,
            OP_NE, 0);
  timers_advance_and_run(5000);
  tt_int_op(n_client_cells, OP_EQ, 6);
  tt_int_op(n_relay_cells, OP_EQ, 6);

  tt_int_op(client_side->padding_info[0]->current_state,
            OP_EQ, CIRCPAD_STATE_END);
  tt_u64_op(client_side->padding_info[0]->padding_scheduled_at_usec,
            OP_EQ, 0);
  tt_int_op(relay_side->padding_info[0]->current_state,
            OP_EQ, CIRCPAD_STATE_GAP);
  tt_u64_op(relay_side->padding_info[0]->padding_scheduled_at_usec,
            OP_EQ, 0);

  /* Verify we can't schedule padding in END state */
  circpad_decision_t ret =
      circpad_machine_schedule_padding(client_side->padding_info[0]);
  tt_int_op(ret, OP_EQ, CIRCPAD_STATE_UNCHANGED);

  /* Simulate application traffic */
  circpad_cell_event_nonpadding_sent(client_side);
  circpad_deliver_unrecognized_cell_events(relay_side, CELL_DIRECTION_OUT);
  circpad_deliver_unrecognized_cell_events(relay_side, CELL_DIRECTION_IN);
  circpad_deliver_recognized_relay_cell_events(client_side, RELAY_COMMAND_DATA,
                                  TO_ORIGIN_CIRCUIT(client_side)->cpath->next);

  tt_ptr_op(client_side->padding_info[0], OP_EQ, NULL);
  tt_ptr_op(client_side->padding_machine[0], OP_EQ, NULL);

  tt_ptr_op(relay_side->padding_info[0], OP_EQ, NULL);
  tt_ptr_op(relay_side->padding_machine[0], OP_EQ, NULL);
  tt_int_op(n_client_cells, OP_EQ, 6);
  tt_int_op(n_relay_cells, OP_EQ, 7);

  // Test timer cancellation
  simulate_single_hop_extend(client_side, relay_side, 1);
  simulate_single_hop_extend(client_side, relay_side, 1);
  timers_advance_and_run(5000);
  circpad_cell_event_padding_received(client_side);

  tt_int_op(client_side->padding_info[0]->current_state, OP_EQ,
                CIRCPAD_STATE_BURST);
  tt_int_op(relay_side->padding_info[0]->current_state, OP_EQ,
          CIRCPAD_STATE_GAP);

  tt_int_op(n_client_cells, OP_EQ, 8);
  tt_int_op(n_relay_cells, OP_EQ, 8);
  tt_u64_op(client_side->padding_info[0]->padding_scheduled_at_usec,
            OP_NE, 0);
  tt_u64_op(relay_side->padding_info[0]->padding_scheduled_at_usec,
            OP_NE, 0);

  /* Test timer cancel due to state rules */
  circpad_cell_event_nonpadding_sent(client_side);
  tt_u64_op(client_side->padding_info[0]->padding_scheduled_at_usec,
            OP_EQ, 0);
  circpad_cell_event_padding_received(client_side);
  tt_u64_op(client_side->padding_info[0]->padding_scheduled_at_usec,
            OP_NE, 0);

  /* Simulate application traffic to cancel timer */
  circpad_cell_event_nonpadding_sent(client_side);
  circpad_deliver_unrecognized_cell_events(relay_side, CELL_DIRECTION_OUT);
  circpad_deliver_unrecognized_cell_events(relay_side, CELL_DIRECTION_IN);
  circpad_deliver_recognized_relay_cell_events(client_side, RELAY_COMMAND_DATA,
                                  TO_ORIGIN_CIRCUIT(client_side)->cpath->next);

  tt_ptr_op(client_side->padding_info[0], OP_EQ, NULL);
  tt_ptr_op(client_side->padding_machine[0], OP_EQ, NULL);

  tt_ptr_op(relay_side->padding_info[0], OP_EQ, NULL);
  tt_ptr_op(relay_side->padding_machine[0], OP_EQ, NULL);

  /* No cells sent, except negotiate end from relay */
  tt_int_op(n_client_cells, OP_EQ, 8);
  tt_int_op(n_relay_cells, OP_EQ, 9);

  /* Test mark for close and free */
  simulate_single_hop_extend(client_side, relay_side, 1);
  simulate_single_hop_extend(client_side, relay_side, 1);
  timers_advance_and_run(5000);
  circpad_cell_event_padding_received(client_side);

  tt_int_op(n_client_cells, OP_EQ, 10);
  tt_int_op(n_relay_cells, OP_EQ, 10);

  tt_int_op(client_side->padding_info[0]->current_state, OP_EQ,
                CIRCPAD_STATE_BURST);
  tt_int_op(relay_side->padding_info[0]->current_state, OP_EQ,
          CIRCPAD_STATE_GAP);

  tt_u64_op(client_side->padding_info[0]->padding_scheduled_at_usec,
            OP_NE, 0);
  tt_u64_op(relay_side->padding_info[0]->padding_scheduled_at_usec,
            OP_NE, 0);
  circuit_mark_for_close(client_side, END_CIRC_REASON_FLAG_REMOTE);
  free_fake_orcirc(TO_OR_CIRCUIT(relay_side));
  timers_advance_and_run(5000);

  /* No cells sent */
  tt_int_op(n_client_cells, OP_EQ, 10);
  tt_int_op(n_relay_cells, OP_EQ, 10);

 done:
  free_fake_origin_circuit(TO_ORIGIN_CIRCUIT(client_side));

  circuitmux_detach_all_circuits(dummy_channel.cmux, NULL);
  circuitmux_free(dummy_channel.cmux);
  timers_shutdown();
  monotime_disable_test_mocking();
  UNMOCK(circuit_package_relay_cell);
  UNMOCK(circuitmux_attach_circuit);

  return;
}
#endif /* 0 */

/** Helper function: Initializes a padding machine where every state uses the
 *  uniform probability distribution.  */
static void
helper_circpad_circ_distribution_machine_setup(int min, int max)
{
  circpad_machine_states_init(&circ_client_machine, 7);

  circpad_state_t *zero_st = &circ_client_machine.states[0];
  zero_st->next_state[CIRCPAD_EVENT_NONPADDING_RECV] = 1;
  zero_st->iat_dist.type = CIRCPAD_DIST_UNIFORM;
  /* param2 is upper bound, param1 is lower */
  zero_st->iat_dist.param1 = min;
  zero_st->iat_dist.param2 = max;
  zero_st->dist_added_shift_usec = min;
  zero_st->dist_max_sample_usec = max;

  circpad_state_t *first_st = &circ_client_machine.states[1];
  first_st->next_state[CIRCPAD_EVENT_NONPADDING_RECV] = 2;
  first_st->iat_dist.type = CIRCPAD_DIST_LOGISTIC;
  /* param1 is Mu, param2 is sigma. */
  first_st->iat_dist.param1 = 9;
  first_st->iat_dist.param2 = 3;
  first_st->dist_added_shift_usec = min;
  first_st->dist_max_sample_usec = max;

  circpad_state_t *second_st = &circ_client_machine.states[2];
  second_st->next_state[CIRCPAD_EVENT_NONPADDING_RECV] = 3;
  second_st->iat_dist.type = CIRCPAD_DIST_LOG_LOGISTIC;
  /* param1 is Alpha, param2 is 1.0/Beta */
  second_st->iat_dist.param1 = 1;
  second_st->iat_dist.param2 = 0.5;
  second_st->dist_added_shift_usec = min;
  second_st->dist_max_sample_usec = max;

  circpad_state_t *third_st = &circ_client_machine.states[3];
  third_st->next_state[CIRCPAD_EVENT_NONPADDING_RECV] = 4;
  third_st->iat_dist.type = CIRCPAD_DIST_GEOMETRIC;
  /* param1 is 'p' (success probability) */
  third_st->iat_dist.param1 = 0.2;
  third_st->dist_added_shift_usec = min;
  third_st->dist_max_sample_usec = max;

  circpad_state_t *fourth_st = &circ_client_machine.states[4];
  fourth_st->next_state[CIRCPAD_EVENT_NONPADDING_RECV] = 5;
  fourth_st->iat_dist.type = CIRCPAD_DIST_WEIBULL;
  /* param1 is k, param2 is Lambda */
  fourth_st->iat_dist.param1 = 1.5;
  fourth_st->iat_dist.param2 = 1;
  fourth_st->dist_added_shift_usec = min;
  fourth_st->dist_max_sample_usec = max;

  circpad_state_t *fifth_st = &circ_client_machine.states[5];
  fifth_st->next_state[CIRCPAD_EVENT_NONPADDING_RECV] = 6;
  fifth_st->iat_dist.type = CIRCPAD_DIST_PARETO;
  /* param1 is sigma, param2 is xi */
  fifth_st->iat_dist.param1 = 1;
  fifth_st->iat_dist.param2 = 5;
  fifth_st->dist_added_shift_usec = min;
  fifth_st->dist_max_sample_usec = max;
}

/** Simple test that the padding delays sampled from a uniform distribution
 *  actually fail within the uniform distribution range. */
static void
test_circuitpadding_sample_distribution(void *arg)
{
  circpad_machine_runtime_t *mi;
  int n_samples;
  int n_states;

  (void) arg;

  /* mock this function so that we dont actually schedule any padding */
  MOCK(circpad_machine_schedule_padding,
       circpad_machine_schedule_padding_mock);
  testing_enable_reproducible_rng();

  /* Initialize a machine with multiple probability distributions */
  circpad_machines_init();
  helper_circpad_circ_distribution_machine_setup(0, 10);

  /* Initialize machine and circuits */
  client_side = TO_CIRCUIT(origin_circuit_new());
  client_side->purpose = CIRCUIT_PURPOSE_C_GENERAL;
  client_side->padding_machine[0] = &circ_client_machine;
  client_side->padding_info[0] =
    circpad_circuit_machineinfo_new(client_side, 0);
  mi = client_side->padding_info[0];

  /* For every state, sample a bunch of values from the distribution and ensure
   * they fall within range. */
  for (n_states = 0 ; n_states < 6; n_states++) {
    /* Make sure we in the right state */
    tt_int_op(client_side->padding_info[0]->current_state, OP_EQ, n_states);

    for (n_samples = 0; n_samples < 100; n_samples++) {
      circpad_delay_t delay = circpad_machine_sample_delay(mi);
      tt_int_op(delay, OP_GE, 0);
      tt_int_op(delay, OP_LE, 10);
    }

    /* send a non-padding cell to move to the next machine state */
    circpad_cell_event_nonpadding_received(client_side);
  }

 done:
  free_fake_origin_circuit(TO_ORIGIN_CIRCUIT(client_side));
  UNMOCK(circpad_machine_schedule_padding);
  testing_disable_reproducible_rng();
}

static circpad_decision_t
circpad_machine_spec_transition_mock(circpad_machine_runtime_t *mi,
                                circpad_event_t event)
{
  (void) mi;
  (void) event;

  return CIRCPAD_STATE_UNCHANGED;
}

/* Test per-machine padding rate limits */
static void
test_circuitpadding_machine_rate_limiting(void *arg)
{
  (void) arg;
  bool retval;
  circpad_machine_runtime_t *mi;
  int i;

  /* Ignore machine transitions for the purposes of this function, we only
   * really care about padding counts */
  MOCK(circpad_machine_spec_transition, circpad_machine_spec_transition_mock);
  MOCK(circpad_send_command_to_hop, circpad_send_command_to_hop_mock);
  testing_enable_reproducible_rng();

  /* Setup machine and circuits */
  client_side = TO_CIRCUIT(origin_circuit_new());
  client_side->purpose = CIRCUIT_PURPOSE_C_GENERAL;
  helper_create_basic_machine();
  client_side->padding_machine[0] = &circ_client_machine;
  client_side->padding_info[0] =
    circpad_circuit_machineinfo_new(client_side, 0);
  mi = client_side->padding_info[0];
  /* Set up the machine info so that we can get through the basic functions */
  mi->state_length = CIRCPAD_STATE_LENGTH_INFINITE;

  /* First we are going to test the per-machine rate limits */
  circ_client_machine.max_padding_percent = 50;
  circ_client_machine.allowed_padding_count = 100;

  /* Check padding limit, should be fine since we haven't sent anything yet. */
  retval = circpad_machine_reached_padding_limit(mi);
  tt_int_op(retval, OP_EQ, 0);

  /* Send 99 padding cells which is below circpad_global_allowed_cells=100, so
   * the rate limit will not trigger */
  for (i=0;i<99;i++) {
    circpad_send_padding_cell_for_callback(mi);
  }
  retval = circpad_machine_reached_padding_limit(mi);
  tt_int_op(retval, OP_EQ, 0);

  /* Now send another padding cell to pass circpad_global_allowed_cells=100,
     and see that the limit will trigger */
  circpad_send_padding_cell_for_callback(mi);
  retval = circpad_machine_reached_padding_limit(mi);
  tt_int_op(retval, OP_EQ, 1);

  retval = circpad_machine_schedule_padding(mi);
  tt_int_op(retval, OP_EQ, CIRCPAD_STATE_UNCHANGED);

  /* Cover wrap */
  for (;i<UINT16_MAX;i++) {
    circpad_send_padding_cell_for_callback(mi);
  }
  tt_int_op(mi->padding_sent, OP_EQ, UINT16_MAX/2+1);

  tt_ptr_op(client_side->padding_info[0], OP_EQ, mi);
  for (i=0;i<UINT16_MAX;i++) {
    circpad_cell_event_nonpadding_sent(client_side);
  }

  tt_int_op(mi->nonpadding_sent, OP_EQ, UINT16_MAX/2);
  tt_int_op(mi->padding_sent, OP_EQ, UINT16_MAX/4+1);

 done:
  free_fake_origin_circuit(TO_ORIGIN_CIRCUIT(client_side));
  testing_disable_reproducible_rng();
}

/* Test global padding rate limits */
static void
test_circuitpadding_global_rate_limiting(void *arg)
{
  (void) arg;
  bool retval;
  circpad_machine_runtime_t *mi;
  int i;
  int64_t actual_mocked_monotime_start;

  /* Ignore machine transitions for the purposes of this function, we only
   * really care about padding counts */
  MOCK(circpad_machine_spec_transition, circpad_machine_spec_transition_mock);
  MOCK(circuitmux_attach_circuit, circuitmux_attach_circuit_mock);
  MOCK(circuit_package_relay_cell,
       circuit_package_relay_cell_mock);
  MOCK(monotime_absolute_usec, mock_monotime_absolute_usec);
  testing_enable_reproducible_rng();

  monotime_init();
  monotime_enable_test_mocking();
  actual_mocked_monotime_start = MONOTIME_MOCK_START;
  monotime_set_mock_time_nsec(actual_mocked_monotime_start);
  monotime_coarse_set_mock_time_nsec(actual_mocked_monotime_start);
  curr_mocked_time = actual_mocked_monotime_start;
  timers_initialize();

  client_side = TO_CIRCUIT(origin_circuit_new());
  client_side->purpose = CIRCUIT_PURPOSE_C_GENERAL;
  dummy_channel.cmux = circuitmux_alloc();

  /* Setup machine and circuits */
  relay_side = TO_CIRCUIT(new_fake_orcirc(&dummy_channel, &dummy_channel));
  relay_side->purpose = CIRCUIT_PURPOSE_OR;
  helper_create_basic_machine();
  relay_side->padding_machine[0] = &circ_client_machine;
  relay_side->padding_info[0] =
    circpad_circuit_machineinfo_new(relay_side, 0);
  mi = relay_side->padding_info[0];
  /* Set up the machine info so that we can get through the basic functions */
  mi->state_length = CIRCPAD_STATE_LENGTH_INFINITE;

  simulate_single_hop_extend(client_side, relay_side, 1);
  simulate_single_hop_extend(client_side, relay_side, 1);

  /* Now test the global limits by setting up the consensus */
  networkstatus_t vote1;
  vote1.net_params = smartlist_new();
  smartlist_split_string(vote1.net_params,
         "circpad_global_allowed_cells=100 circpad_global_max_padding_pct=50",
                         NULL, 0, 0);
  /* Register global limits with the padding subsystem */
  circpad_new_consensus_params(&vote1);

  /* Check padding limit, should be fine since we haven't sent anything yet. */
  retval = circpad_machine_reached_padding_limit(mi);
  tt_int_op(retval, OP_EQ, 0);

  /* Send 99 padding cells which is below circpad_global_allowed_cells=100, so
   * the rate limit will not trigger */
  for (i=0;i<99;i++) {
    circpad_send_padding_cell_for_callback(mi);
  }
  retval = circpad_machine_reached_padding_limit(mi);
  tt_int_op(retval, OP_EQ, 0);

  /* Now send another padding cell to pass circpad_global_allowed_cells=100,
     and see that the limit will trigger */
  circpad_send_padding_cell_for_callback(mi);
  retval = circpad_machine_reached_padding_limit(mi);
  tt_int_op(retval, OP_EQ, 1);

  retval = circpad_machine_schedule_padding(mi);
  tt_int_op(retval, OP_EQ, CIRCPAD_STATE_UNCHANGED);

  /* Now send 92 non-padding cells to get near the
   * circpad_global_max_padding_pct=50 limit; in particular with 96 non-padding
   * cells, the padding traffic is still 51% of total traffic so limit should
   * trigger */
  for (i=0;i<92;i++) {
    circpad_cell_event_nonpadding_sent(relay_side);
  }
  retval = circpad_machine_reached_padding_limit(mi);
  tt_int_op(retval, OP_EQ, 1);

  /* Send another non-padding cell to bring the padding traffic to 50% of total
   * traffic and get past the limit */
  circpad_cell_event_nonpadding_sent(relay_side);
  retval = circpad_machine_reached_padding_limit(mi);
  tt_int_op(retval, OP_EQ, 0);

 done:
  free_fake_orcirc(TO_OR_CIRCUIT(relay_side));
  circuitmux_detach_all_circuits(dummy_channel.cmux, NULL);
  circuitmux_free(dummy_channel.cmux);
  SMARTLIST_FOREACH(vote1.net_params, char *, cp, tor_free(cp));
  smartlist_free(vote1.net_params);
  testing_disable_reproducible_rng();
}

/* Test reduced and disabled padding */
static void
test_circuitpadding_reduce_disable(void *arg)
{
  (void) arg;
  int64_t actual_mocked_monotime_start;

  MOCK(circuitmux_attach_circuit, circuitmux_attach_circuit_mock);
  testing_enable_reproducible_rng();

  nodes_init();
  dummy_channel.cmux = circuitmux_alloc();
  relay_side = (circuit_t *)new_fake_orcirc(&dummy_channel,
                                            &dummy_channel);
  client_side = (circuit_t *)origin_circuit_new();
  relay_side->purpose = CIRCUIT_PURPOSE_OR;
  client_side->purpose = CIRCUIT_PURPOSE_C_GENERAL;

  circpad_machines_init();
  helper_create_conditional_machines();

  monotime_init();
  monotime_enable_test_mocking();
  actual_mocked_monotime_start = MONOTIME_MOCK_START;
  monotime_set_mock_time_nsec(actual_mocked_monotime_start);
  monotime_coarse_set_mock_time_nsec(actual_mocked_monotime_start);
  curr_mocked_time = actual_mocked_monotime_start;
  timers_initialize();

  /* This is needed so that we are not considered to be dormant */
  note_user_activity(20);

  MOCK(circuit_package_relay_cell,
       circuit_package_relay_cell_mock);
  MOCK(node_get_by_id,
       node_get_by_id_mock);

  /* Simulate extend. This should result in the original machine getting
   * added, since the circuit is not built */
  simulate_single_hop_extend(client_side, relay_side, 1);
  simulate_single_hop_extend(client_side, relay_side, 1);

  /* Verify that machine #2 is added */
  tt_int_op(client_side->padding_machine[0]->machine_num, OP_EQ, 2);
  tt_int_op(relay_side->padding_machine[0]->machine_num, OP_EQ, 2);

  /* Deliver a padding cell to the client, to trigger burst state */
  circpad_cell_event_padding_sent(client_side);

  /* This should have trigger length shutdown condition on client.. */
  tt_ptr_op(client_side->padding_info[0], OP_EQ, NULL);
  tt_ptr_op(client_side->padding_machine[0], OP_EQ, NULL);

  /* Verify machine is gone from both sides */
  tt_ptr_op(relay_side->padding_info[0], OP_EQ, NULL);
  tt_ptr_op(relay_side->padding_machine[0], OP_EQ, NULL);

  /* Now test the reduced padding machine by setting up the consensus */
  networkstatus_t vote1;
  vote1.net_params = smartlist_new();
  smartlist_split_string(vote1.net_params,
         "circpad_padding_reduced=1", NULL, 0, 0);

  /* Register reduced padding machine with the padding subsystem */
  circpad_new_consensus_params(&vote1);

  simulate_single_hop_extend(client_side, relay_side, 1);

  /* Verify that machine #0 is added */
  tt_int_op(client_side->padding_machine[0]->machine_num, OP_EQ, 2);
  tt_int_op(relay_side->padding_machine[0]->machine_num, OP_EQ, 2);

  tt_int_op(
    circpad_machine_reached_padding_limit(client_side->padding_info[0]),
    OP_EQ, 0);
  tt_int_op(
    circpad_machine_reached_padding_limit(relay_side->padding_info[0]),
    OP_EQ, 0);

  /* Test that machines get torn down when padding is disabled */
  SMARTLIST_FOREACH(vote1.net_params, char *, cp, tor_free(cp));
  smartlist_free(vote1.net_params);
  vote1.net_params = smartlist_new();
  smartlist_split_string(vote1.net_params,
         "circpad_padding_disabled=1", NULL, 0, 0);

  /* Register reduced padding machine with the padding subsystem */
  circpad_new_consensus_params(&vote1);

  tt_int_op(
    circpad_machine_schedule_padding(client_side->padding_info[0]),
    OP_EQ, CIRCPAD_STATE_UNCHANGED);
  tt_int_op(
    circpad_machine_schedule_padding(relay_side->padding_info[0]),
    OP_EQ, CIRCPAD_STATE_UNCHANGED);

  /* Signal that circuit is built: this event causes us to re-evaluate
   * machine conditions (which don't apply because padding is disabled). */
  circpad_machine_event_circ_built(TO_ORIGIN_CIRCUIT(client_side));

  tt_ptr_op(client_side->padding_info[0], OP_EQ, NULL);
  tt_ptr_op(client_side->padding_machine[0], OP_EQ, NULL);
  tt_ptr_op(relay_side->padding_info[0], OP_EQ, NULL);
  tt_ptr_op(relay_side->padding_machine[0], OP_EQ, NULL);

  SMARTLIST_FOREACH(vote1.net_params, char *, cp, tor_free(cp));
  smartlist_free(vote1.net_params);
  vote1.net_params = NULL;
  circpad_new_consensus_params(&vote1);

  get_options_mutable()->ReducedCircuitPadding = 1;

  simulate_single_hop_extend(client_side, relay_side, 1);

  /* Verify that machine #0 is added */
  tt_int_op(client_side->padding_machine[0]->machine_num, OP_EQ, 2);
  tt_int_op(relay_side->padding_machine[0]->machine_num, OP_EQ, 2);

  tt_int_op(
    circpad_machine_reached_padding_limit(client_side->padding_info[0]),
    OP_EQ, 0);
  tt_int_op(
    circpad_machine_reached_padding_limit(relay_side->padding_info[0]),
    OP_EQ, 0);

  get_options_mutable()->CircuitPadding = 0;

  tt_int_op(
    circpad_machine_schedule_padding(client_side->padding_info[0]),
    OP_EQ, CIRCPAD_STATE_UNCHANGED);
  tt_int_op(
    circpad_machine_schedule_padding(relay_side->padding_info[0]),
    OP_EQ, CIRCPAD_STATE_UNCHANGED);

  /* Signal that circuit is built: this event causes us to re-evaluate
   * machine conditions (which don't apply because padding is disabled). */

  circpad_machine_event_circ_built(TO_ORIGIN_CIRCUIT(client_side));

  tt_ptr_op(client_side->padding_info[0], OP_EQ, NULL);
  tt_ptr_op(client_side->padding_machine[0], OP_EQ, NULL);
  tt_ptr_op(relay_side->padding_info[0], OP_EQ, NULL);
  tt_ptr_op(relay_side->padding_machine[0], OP_EQ, NULL);

 done:
  free_fake_orcirc(TO_OR_CIRCUIT(relay_side));
  circuitmux_detach_all_circuits(dummy_channel.cmux, NULL);
  circuitmux_free(dummy_channel.cmux);
  testing_disable_reproducible_rng();
}

/** Just a basic machine whose whole purpose is to reach the END state */
static void
helper_create_ender_machine(void)
{
  /* Start, burst */
  circpad_machine_states_init(&circ_client_machine, 2);

  circ_client_machine.states[CIRCPAD_STATE_START].
      next_state[CIRCPAD_EVENT_NONPADDING_RECV] = CIRCPAD_STATE_END;

  circ_client_machine.conditions.apply_state_mask = CIRCPAD_STATE_ALL;
  circ_client_machine.conditions.apply_purpose_mask = CIRCPAD_PURPOSE_ALL;
}

static time_t mocked_timeofday;
/** Set timeval to a mock date and time. This is necessary
 * to make tor_gettimeofday() mockable. */
static void
mock_tor_gettimeofday(struct timeval *timeval)
{
  timeval->tv_sec = mocked_timeofday;
  timeval->tv_usec = 0;
}

/** Test manual managing of circuit lifetimes by the circuitpadding
 *  subsystem. In particular this test goes through all the cases of the
 *  circpad_marked_circuit_for_padding() function, via
 *  circuit_mark_for_close() as well as
 *  circuit_expire_old_circuits_clientside(). */
static void
test_circuitpadding_manage_circuit_lifetime(void *arg)
{
  circpad_machine_runtime_t *mi;

  (void) arg;

  client_side = (circuit_t *)origin_circuit_new();
  client_side->purpose = CIRCUIT_PURPOSE_C_GENERAL;
  monotime_enable_test_mocking();
  MOCK(tor_gettimeofday, mock_tor_gettimeofday);
  mocked_timeofday = 23;

  helper_create_ender_machine();

  /* Enable manual circuit lifetime manage for this test */
  circ_client_machine.manage_circ_lifetime = 1;

  /* Test setup */
  client_side->padding_machine[0] = &circ_client_machine;
  client_side->padding_info[0] =
    circpad_circuit_machineinfo_new(client_side, 0);
  mi = client_side->padding_info[0];

  tt_int_op(mi->current_state, OP_EQ, CIRCPAD_STATE_START);

  /* Check that the circuit is not marked for close */
  tt_int_op(client_side->marked_for_close, OP_EQ, 0);
  tt_int_op(client_side->purpose, OP_EQ, CIRCUIT_PURPOSE_C_GENERAL);

  /* Mark this circuit for close due to a remote reason */
  circuit_mark_for_close(client_side,
                         END_CIRC_REASON_FLAG_REMOTE|END_CIRC_REASON_NONE);
  tt_ptr_op(client_side->padding_info[0], OP_NE, NULL);
  tt_int_op(client_side->marked_for_close, OP_NE, 0);
  tt_int_op(client_side->purpose, OP_EQ, CIRCUIT_PURPOSE_C_GENERAL);
  client_side->marked_for_close = 0;

  /* Mark this circuit for close due to a protocol issue */
  circuit_mark_for_close(client_side, END_CIRC_REASON_TORPROTOCOL);
  tt_int_op(client_side->marked_for_close, OP_NE, 0);
  tt_int_op(client_side->purpose, OP_EQ, CIRCUIT_PURPOSE_C_GENERAL);
  client_side->marked_for_close = 0;

  /* Mark a measurement circuit for close */
  client_side->purpose = CIRCUIT_PURPOSE_C_MEASURE_TIMEOUT;
  circuit_mark_for_close(client_side, END_CIRC_REASON_NONE);
  tt_int_op(client_side->marked_for_close, OP_NE, 0);
  tt_int_op(client_side->purpose, OP_EQ, CIRCUIT_PURPOSE_C_MEASURE_TIMEOUT);
  client_side->marked_for_close = 0;

  /* Mark a general circuit for close */
  client_side->purpose = CIRCUIT_PURPOSE_C_GENERAL;
  circuit_mark_for_close(client_side, END_CIRC_REASON_NONE);

  /* Check that this circuit is still not marked for close since we are
   * managing the lifetime manually, but the circuit was tagged as such by the
   * circpadding subsystem */
  tt_int_op(client_side->marked_for_close, OP_EQ, 0);
  tt_int_op(client_side->purpose, OP_EQ, CIRCUIT_PURPOSE_C_CIRCUIT_PADDING);

  /* We just tested case (1) from the comments of
   * circpad_circuit_should_be_marked_for_close() */

  /* Transition the machine to the END state but did not delete its machine */
  tt_ptr_op(client_side->padding_info[0], OP_NE, NULL);
  circpad_cell_event_nonpadding_received(client_side);
  tt_int_op(mi->current_state, OP_EQ, CIRCPAD_STATE_END);

  /* We just tested case (3) from the comments of
   * circpad_circuit_should_be_marked_for_close().
   * Now let's go for case (2). */

  /* Reset the close mark */
  client_side->marked_for_close = 0;

  /* Mark this circuit for close */
  circuit_mark_for_close(client_side, 0);

  /* See that the circ got closed since we are already in END state */
  tt_int_op(client_side->marked_for_close, OP_NE, 0);

  /* We just tested case (2). Now let's see that case (4) is unreachable as
     that comment claims */

  /* First, reset all close marks and tags */
  client_side->marked_for_close = 0;
  client_side->purpose = CIRCUIT_PURPOSE_C_GENERAL;

  /* Now re-create the ender machine so that we can transition to END again */
  /* Free up some stuff first */
  circpad_circuit_free_all_machineinfos(client_side);
  tor_free(circ_client_machine.states);
  helper_create_ender_machine();

  client_side->padding_machine[0] = &circ_client_machine;
  client_side->padding_info[0] =
    circpad_circuit_machineinfo_new(client_side, 0);
  mi = client_side->padding_info[0];

  /* Check we are in START. */
  tt_int_op(mi->current_state, OP_EQ, CIRCPAD_STATE_START);

  /* Test that we don't expire this circuit yet */
  client_side->timestamp_dirty = 0;
  client_side->state = CIRCUIT_STATE_OPEN;
  tor_gettimeofday(&client_side->timestamp_began);
  TO_ORIGIN_CIRCUIT(client_side)->circuit_idle_timeout = 23;
  mocked_timeofday += 24;
  circuit_expire_old_circuits_clientside();
  circuit_expire_old_circuits_clientside();
  circuit_expire_old_circuits_clientside();
  tt_int_op(client_side->timestamp_dirty, OP_NE, 0);
  tt_int_op(client_side->marked_for_close, OP_EQ, 0);
  tt_int_op(client_side->purpose, OP_EQ, CIRCUIT_PURPOSE_C_CIRCUIT_PADDING);

  /* Runaway circpad test: if the machine does not transition to end,
   * test that after CIRCPAD_DELAY_MAX_SECS, we get marked anyway */
  mocked_timeofday = client_side->timestamp_dirty
      + get_options()->MaxCircuitDirtiness + 2;
  client_side->padding_info[0]->last_cell_time_sec =
      approx_time()-(CIRCPAD_DELAY_MAX_SECS+10);
  circuit_expire_old_circuits_clientside();
  tt_int_op(client_side->marked_for_close, OP_NE, 0);

  /* Test back to normal: if we had activity, we won't close */
  client_side->padding_info[0]->last_cell_time_sec = approx_time();
  client_side->marked_for_close = 0;
  circuit_expire_old_circuits_clientside();
  tt_int_op(client_side->marked_for_close, OP_EQ, 0);

  /* Transition to END, but before we're past the dirty timer */
  mocked_timeofday = client_side->timestamp_dirty;
  circpad_cell_event_nonpadding_received(client_side);
  tt_int_op(mi->current_state, OP_EQ, CIRCPAD_STATE_END);

  /* Verify that the circuit was not closed. */
  tt_int_op(client_side->marked_for_close, OP_EQ, 0);

  /* Now that we are in END state, we can be closed by expiry, but via
   * the timestamp_dirty path, not the idle path. So first test not dirty
   * enough. */
  mocked_timeofday = client_side->timestamp_dirty;
  circuit_expire_old_circuits_clientside();
  tt_int_op(client_side->marked_for_close, OP_EQ, 0);
  mocked_timeofday = client_side->timestamp_dirty
      + get_options()->MaxCircuitDirtiness + 2;
  circuit_expire_old_circuits_clientside();
  tt_int_op(client_side->marked_for_close, OP_NE, 0);

 done:
  free_fake_origin_circuit(TO_ORIGIN_CIRCUIT(client_side));
  tor_free(circ_client_machine.states);
  monotime_disable_test_mocking();
  UNMOCK(tor_gettimeofday);
}

/** Helper for the test_circuitpadding_hs_machines test:
 *
 *  - Create a client and relay circuit.
 *  - Setup right circuit purpose and attach a machine to the client circuit.
 *  - Verify that state transitions work as intended and state length gets
 *    enforced.
 *
 *  This function is able to do this test both for intro and rend circuits
 *  depending on the value of <b>test_intro_circs</b>.
 */
static void
helper_test_hs_machines(bool test_intro_circs)
{
  /* Setup the circuits */
  origin_circuit_t *origin_client_side = origin_circuit_new();
  client_side = TO_CIRCUIT(origin_client_side);
  client_side->purpose = CIRCUIT_PURPOSE_C_GENERAL;

  dummy_channel.cmux = circuitmux_alloc();
  relay_side = TO_CIRCUIT(new_fake_orcirc(&dummy_channel, &dummy_channel));
  relay_side->purpose = CIRCUIT_PURPOSE_OR;

  /* extend the client circ to two hops */
  simulate_single_hop_extend(client_side, relay_side, 1);
  simulate_single_hop_extend(client_side, relay_side, 1);

  /* machines only apply on opened circuits */
  origin_client_side->has_opened = 1;

  /************************************/

  /* Attaching the client machine now won't work here because of a wrong
   * purpose */
  tt_assert(!client_side->padding_machine[0]);
  circpad_add_matching_machines(origin_client_side, origin_padding_machines);
  tt_assert(!client_side->padding_machine[0]);

  /* Change the purpose, see the machine getting attached */
  client_side->purpose = test_intro_circs ?
    CIRCUIT_PURPOSE_C_INTRODUCE_ACK_WAIT : CIRCUIT_PURPOSE_C_REND_JOINED;
  circpad_add_matching_machines(origin_client_side, origin_padding_machines);
  tt_ptr_op(client_side->padding_info[0], OP_NE, NULL);
  tt_ptr_op(client_side->padding_machine[0], OP_NE, NULL);

  tt_ptr_op(relay_side->padding_info[0], OP_NE, NULL);
  tt_ptr_op(relay_side->padding_machine[0], OP_NE, NULL);

  /* Verify that the right machine is attached */
  tt_str_op(client_side->padding_machine[0]->name, OP_EQ,
            test_intro_circs ? "client_ip_circ" : "client_rp_circ");
  tt_str_op(relay_side->padding_machine[0]->name, OP_EQ,
            test_intro_circs ? "relay_ip_circ": "relay_rp_circ");

  /***********************************/

  /* Intro machines are at START state, but rend machines have already skipped
   * to OBFUSCATE_CIRC_SETUP because of the sent PADDING_NEGOTIATE. */
  tt_int_op(client_side->padding_info[0]->current_state, OP_EQ,
            CIRCPAD_STATE_OBFUSCATE_CIRC_SETUP);
  tt_int_op(relay_side->padding_info[0]->current_state, OP_EQ,
            CIRCPAD_STATE_OBFUSCATE_CIRC_SETUP);

  /*Send non-padding to move the machines from START to OBFUSCATE_CIRC_SETUP */
  circpad_cell_event_nonpadding_received(client_side);
  circpad_cell_event_nonpadding_received(relay_side);
  tt_int_op(client_side->padding_info[0]->current_state, OP_EQ,
            CIRCPAD_STATE_OBFUSCATE_CIRC_SETUP);
  tt_int_op(relay_side->padding_info[0]->current_state, OP_EQ,
            CIRCPAD_STATE_OBFUSCATE_CIRC_SETUP);

  /* Check that the state lengths have been sampled and are within range */
  circpad_machine_runtime_t *client_machine_runtime =
    client_side->padding_info[0];
  circpad_machine_runtime_t *relay_machine_runtime =
    relay_side->padding_info[0];

  if (test_intro_circs) {
    /* on the client side, we don't send any padding so
     * state length is not set */
    tt_i64_op(client_machine_runtime->state_length, OP_EQ, -1);
    /* relay side has state limits. check them */
    tt_i64_op(relay_machine_runtime->state_length, OP_GE,
              INTRO_MACHINE_MINIMUM_PADDING);
    tt_i64_op(relay_machine_runtime->state_length, OP_LT,
              INTRO_MACHINE_MAXIMUM_PADDING);
  } else {
    tt_i64_op(client_machine_runtime->state_length, OP_EQ, 1);
    tt_i64_op(relay_machine_runtime->state_length, OP_EQ, 1);
  }

  if (test_intro_circs) {
    int i;
    /* Send state_length worth of padding from the relay and see that the
     * client state goes to END */
    for (i = (int) relay_machine_runtime->state_length ; i > 0 ; i--) {
      circpad_send_padding_cell_for_callback(relay_machine_runtime);
    }
    /* See that the machine has been teared down after all the length has been
     * exhausted (the padding info should now be null on both sides) */
    tt_ptr_op(relay_side->padding_info[0], OP_EQ, NULL);
    tt_ptr_op(client_side->padding_info[0], OP_EQ, NULL);
  } else {
    int i;
    /* Send state_length worth of padding and see that the state goes to END */
    for (i = (int) client_machine_runtime->state_length ; i > 0 ; i--) {
      circpad_send_padding_cell_for_callback(client_machine_runtime);
    }
    /* See that the machine has been teared down after all the length has been
     * exhausted. */
    tt_int_op(client_side->padding_info[0]->current_state, OP_EQ,
              CIRCPAD_STATE_END);
  }

 done:
  free_fake_orcirc(TO_OR_CIRCUIT(relay_side));
  circuitmux_detach_all_circuits(dummy_channel.cmux, NULL);
  circuitmux_free(dummy_channel.cmux);
  free_fake_origin_circuit(TO_ORIGIN_CIRCUIT(client_side));
}

/** Test that the HS circuit padding machines work as intended. */
static void
test_circuitpadding_hs_machines(void *arg)
{
  (void)arg;

  /* Test logic:
   *
   * 1) Register the HS machines, which aim to hide the presence of
   *    onion service traffic on the client-side
   *
   * 2) Call helper_test_hs_machines() to perform tests for the intro circuit
   *    machines and for the rend circuit machines.
  */

  MOCK(circuitmux_attach_circuit, circuitmux_attach_circuit_mock);
  MOCK(circuit_package_relay_cell, circuit_package_relay_cell_mock);
  MOCK(circuit_get_nth_node, circuit_get_nth_node_mock);
  MOCK(circpad_machine_schedule_padding,circpad_machine_schedule_padding_mock);

  origin_padding_machines = smartlist_new();
  relay_padding_machines = smartlist_new();

  nodes_init();

  monotime_init();
  monotime_enable_test_mocking();
  monotime_set_mock_time_nsec(1*TOR_NSEC_PER_USEC);
  monotime_coarse_set_mock_time_nsec(1*TOR_NSEC_PER_USEC);
  curr_mocked_time = 1*TOR_NSEC_PER_USEC;

  timers_initialize();

  /* This is needed so that we are not considered to be dormant */
  note_user_activity(20);

  /************************************/

  /* Register the HS machines */
  circpad_machine_client_hide_intro_circuits(origin_padding_machines);
  circpad_machine_client_hide_rend_circuits(origin_padding_machines);
  circpad_machine_relay_hide_intro_circuits(relay_padding_machines);
  circpad_machine_relay_hide_rend_circuits(relay_padding_machines);

  /***********************************/

  /* Do the tests for the intro circuit machines */
  helper_test_hs_machines(true);
  /* Do the tests for the rend circuit machines */
  helper_test_hs_machines(false);

  timers_shutdown();
  monotime_disable_test_mocking();

  SMARTLIST_FOREACH_BEGIN(origin_padding_machines,
                          circpad_machine_spec_t *, m) {
    machine_spec_free(m);
  } SMARTLIST_FOREACH_END(m);

  SMARTLIST_FOREACH_BEGIN(relay_padding_machines,
                          circpad_machine_spec_t *, m) {
    machine_spec_free(m);
  } SMARTLIST_FOREACH_END(m);

  smartlist_free(origin_padding_machines);
  smartlist_free(relay_padding_machines);

  UNMOCK(circuitmux_attach_circuit);
  UNMOCK(circuit_package_relay_cell);
  UNMOCK(circuit_get_nth_node);
  UNMOCK(circpad_machine_schedule_padding);
}

/** Test that we effectively ignore non-padding cells in padding circuits. */
static void
test_circuitpadding_ignore_non_padding_cells(void *arg)
{
  int retval;
  relay_header_t rh;

  (void) arg;

  client_side = (circuit_t *)origin_circuit_new();
  client_side->purpose = CIRCUIT_PURPOSE_C_CIRCUIT_PADDING;

  rh.command = RELAY_COMMAND_BEGIN;

  setup_full_capture_of_logs(LOG_INFO);
  retval = handle_relay_cell_command(NULL, client_side, NULL, NULL, &rh, 0);
  tt_int_op(retval, OP_EQ, 0);
  expect_log_msg_containing("Ignored cell");

 done:
  ;
}

#define TEST_CIRCUITPADDING(name, flags) \
    { #name, test_##name, (flags), NULL, NULL }

struct testcase_t circuitpadding_tests[] = {
  TEST_CIRCUITPADDING(circuitpadding_tokens, TT_FORK),
  TEST_CIRCUITPADDING(circuitpadding_state_length, TT_FORK),
  TEST_CIRCUITPADDING(circuitpadding_negotiation, TT_FORK),
  TEST_CIRCUITPADDING(circuitpadding_wronghop, TT_FORK),
  /** Disabled unstable test until #29298 is implemented (see #29122) */
  //  TEST_CIRCUITPADDING(circuitpadding_circuitsetup_machine, TT_FORK),
  TEST_CIRCUITPADDING(circuitpadding_conditions, TT_FORK),
  TEST_CIRCUITPADDING(circuitpadding_rtt, TT_FORK),
  TEST_CIRCUITPADDING(circuitpadding_sample_distribution, TT_FORK),
  TEST_CIRCUITPADDING(circuitpadding_machine_rate_limiting, TT_FORK),
  TEST_CIRCUITPADDING(circuitpadding_global_rate_limiting, TT_FORK),
  TEST_CIRCUITPADDING(circuitpadding_reduce_disable, TT_FORK),
  TEST_CIRCUITPADDING(circuitpadding_token_removal_lower, TT_FORK),
  TEST_CIRCUITPADDING(circuitpadding_token_removal_higher, TT_FORK),
  TEST_CIRCUITPADDING(circuitpadding_closest_token_removal, TT_FORK),
  TEST_CIRCUITPADDING(circuitpadding_closest_token_removal_usec, TT_FORK),
  TEST_CIRCUITPADDING(circuitpadding_token_removal_exact, TT_FORK),
  TEST_CIRCUITPADDING(circuitpadding_manage_circuit_lifetime, TT_FORK),
  TEST_CIRCUITPADDING(circuitpadding_hs_machines, TT_FORK),
  TEST_CIRCUITPADDING(circuitpadding_ignore_non_padding_cells, TT_FORK),
  END_OF_TESTCASES
};
