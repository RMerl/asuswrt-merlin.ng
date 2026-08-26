#define CHANNEL_OBJECT_PRIVATE
#define TOR_TIMERS_PRIVATE
#define TOR_CONFLUX_PRIVATE
#define CIRCUITLIST_PRIVATE
#define NETWORKSTATUS_PRIVATE
#define CRYPT_PATH_PRIVATE
#define RELAY_PRIVATE
#define CONNECTION_PRIVATE
#define TOR_CONGESTION_CONTROL_COMMON_PRIVATE
#define TOR_CONGESTION_CONTROL_PRIVATE

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
#include "core/or/circuitstats.h"
#include "core/or/circuitbuild.h"
#include "core/or/circuituse.h"
#include "core/or/congestion_control_st.h"
#include "core/or/congestion_control_common.h"
#include "core/or/extendinfo.h"
#include "core/or/relay_msg.h"
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

#include "core/mainloop/connection.h"
#include "core/or/connection_edge.h"
#include "core/or/edge_connection_st.h"
#include "core/or/entry_connection_st.h"
#include "core/or/socks_request_st.h"

#include "test/fakecircs.h"
#include "test/rng_test_helpers.h"
#include "core/or/conflux_pool.h"
#include "core/or/conflux_util.h"
#include "core/or/conflux_params.h"
#include "core/or/conflux.h"
#include "core/or/conflux_st.h"
#include "trunnel/conflux.h"
#include "lib/crypt_ops/crypto_rand.h"

/* Start our monotime mocking at 1 second past whatever monotime_init()
 * thought the actual wall clock time was, for platforms with bad resolution
 * and weird timevalues during monotime_init() before mocking. */
#define MONOTIME_MOCK_START   (monotime_absolute_nsec()+\
                               TOR_NSEC_PER_USEC*TOR_USEC_PER_SEC)

extern smartlist_t *connection_array;
void circuit_expire_old_circuits_clientside(void);

circid_t get_unique_circ_id_by_chan(channel_t *chan);

channel_t *new_fake_channel(void);

static void simulate_single_hop_extend(origin_circuit_t *client, int exit);
static void free_fake_origin_circuit(origin_circuit_t *circ);
static circuit_t * get_exit_circ(circuit_t *client_circ);
static circuit_t * get_client_circ(circuit_t *exit_circ);
static void simulate_circuit_build(circuit_t *client_circ);

static int64_t curr_mocked_time;

static channel_t dummy_channel;

static void
timers_advance_and_run(int64_t msec_update)
{
  curr_mocked_time += msec_update*TOR_NSEC_PER_MSEC;
  monotime_coarse_set_mock_time_nsec(curr_mocked_time);
  monotime_set_mock_time_nsec(curr_mocked_time);
}

/* These lists of circuit endpoints send to eachother via
 * circuit_package_relay_cell_mocked */
static smartlist_t *client_circs;
static smartlist_t *exit_circs;
static smartlist_t *client_streams;
static smartlist_t *exit_streams;

typedef struct {
  int called;
  streamid_t stream_id;
  circuit_t *circ;
  uint8_t relay_command;
  uint8_t end_reason;
  size_t payload_len;
} begindir_end_capture_t;

static begindir_end_capture_t begindir_end_capture;

static void
reset_begindir_end_capture(void)
{
  memset(&begindir_end_capture, 0, sizeof(begindir_end_capture));
}

static int
mock_relay_send_command_from_edge(streamid_t stream_id,
                                  circuit_t *circ,
                                  uint8_t relay_command,
                                  const char *payload,
                                  size_t payload_len,
                                  crypt_path_t *cpath_layer,
                                  const char *filename,
                                  int lineno)
{
  (void)cpath_layer;
  (void)filename;
  (void)lineno;

  begindir_end_capture.called++;
  begindir_end_capture.stream_id = stream_id;
  begindir_end_capture.circ = circ;
  begindir_end_capture.relay_command = relay_command;
  begindir_end_capture.payload_len = payload_len;
  begindir_end_capture.end_reason =
    (payload && payload_len) ? (uint8_t)payload[0] : 0;

  return 0;
}

static void
make_test_relay_msg(relay_msg_t *out, uint8_t command,
                    const void *body, size_t bodylen,
                    streamid_t stream_id)
{
  memset(out, 0, sizeof(*out));
  out->command = command;
  out->body = (uint8_t *)body;
  out->length = bodylen;
  out->stream_id = stream_id;
}

typedef struct {
  circuit_t *client;
  circuit_t *exit;
} circ_pair_t;
static smartlist_t *circ_pairs;

static void
simulate_circuit_built(circuit_t *client, circuit_t *exit)
{
  circ_pair_t *pair = tor_malloc_zero(sizeof(circ_pair_t));
  pair->client = client;
  pair->exit = exit;
  smartlist_add(circ_pairs, pair);
}

static origin_circuit_t *
circuit_establish_circuit_conflux_mock(const uint8_t *conflux_nonce,
                                   uint8_t purpose, extend_info_t *exit_ei,
                                   int flags)
{
  (void)exit_ei;
  (void)flags;
  origin_circuit_t *circ = origin_circuit_init(purpose, flags);
  circ->base_.conflux_pending_nonce = tor_memdup(conflux_nonce, DIGEST256_LEN);
  circ->base_.purpose = CIRCUIT_PURPOSE_CONFLUX_UNLINKED;
  smartlist_add(client_circs, circ);

  // This also moves the clock forward as if these hops were opened..
  // Not a problem, unless we want to accurately test buildtimeouts
  simulate_single_hop_extend(circ, 0);
  simulate_single_hop_extend(circ, 0);
  simulate_single_hop_extend(circ, 1);
  circ->cpath->prev->ccontrol = tor_malloc_zero(sizeof(congestion_control_t));
  circ->cpath->prev->ccontrol->sendme_pending_timestamps = smartlist_new();
  circ->cpath->prev->ccontrol->sendme_inc = 31;

  return circ;
}

static void
free_fake_origin_circuit(origin_circuit_t *circ)
{
  circuit_clear_cpath(circ);
  tor_free(circ);
}

void dummy_nop_timer(void);

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
/* For use in the mock_net smartlist queue:
 * this struct contains a circuit and a cell to
 * deliver on it. */
typedef struct {
  circuit_t *circ;
  cell_t *cell;
} cell_delivery_t;

static smartlist_t *mock_cell_delivery = NULL;

static int
circuit_package_relay_cell_mock(cell_t *cell, circuit_t *circ,
                           cell_direction_t cell_direction,
                           crypt_path_t *layer_hint, streamid_t on_stream,
                           const char *filename, int lineno)
{
  (void)cell; (void)on_stream; (void)filename; (void)lineno;
  (void)cell_direction;
  circuit_t *dest_circ = NULL;

  // If we have a layer hint, we are sending to the exit. Look
  // up the exit circ based on our circuit index in the smartlist
  if (layer_hint) {
    tor_assert(CIRCUIT_IS_ORIGIN(circ));
    tt_int_op(cell_direction, OP_EQ, CELL_DIRECTION_OUT);

    // Search the circ pairs list for the pair whose client is this circ,
    // and set dest_circ to the exit circ in that pair
    SMARTLIST_FOREACH_BEGIN(circ_pairs, circ_pair_t *, pair) {
      if (pair->client == circ) {
        dest_circ = pair->exit;
        break;
      }
    } SMARTLIST_FOREACH_END(pair);
  } else {
    tt_int_op(cell_direction, OP_EQ, CELL_DIRECTION_IN);

    // Search the circ pairs list for the pair whose exit is this circ,
    // and set dest_circ to the client circ in that pair
    SMARTLIST_FOREACH_BEGIN(circ_pairs, circ_pair_t *, pair) {
      if (pair->exit == circ) {
        dest_circ = pair->client;
        break;
      }
    } SMARTLIST_FOREACH_END(pair);
  }

  cell_delivery_t *delivery = tor_malloc_zero(sizeof(cell_delivery_t));
  delivery->circ = dest_circ;
  delivery->cell = tor_memdup(cell, sizeof(cell_t));
  smartlist_add(mock_cell_delivery, delivery);
 done:
  return 0;
}

/** Pull the next cell from the mock delivery queue and deliver it. */
static void
process_mock_cell_delivery(void)
{
  relay_header_t rh;
  relay_msg_t *msg = NULL;

  cell_delivery_t *delivery = smartlist_pop_last(mock_cell_delivery);
  tor_assert(delivery);
  cell_t *cell = delivery->cell;
  circuit_t *dest_circ = delivery->circ;
  relay_header_unpack(&rh, cell->payload);

  timers_advance_and_run(1);

  msg = relay_msg_decode_cell(RELAY_CELL_FORMAT_V0, cell);

  tor_assert(msg);

  switch (msg->command) {
  case RELAY_COMMAND_CONFLUX_LINK:
    tor_assert(!CIRCUIT_IS_ORIGIN(dest_circ));
    conflux_process_link(dest_circ, msg);
    break;
  case RELAY_COMMAND_CONFLUX_LINKED:
    tor_assert(CIRCUIT_IS_ORIGIN(dest_circ));
    conflux_process_linked(dest_circ,
                           TO_ORIGIN_CIRCUIT(dest_circ)->cpath->prev,
                           msg);
    break;
  case RELAY_COMMAND_CONFLUX_LINKED_ACK:
    tor_assert(!CIRCUIT_IS_ORIGIN(dest_circ));
    conflux_process_linked_ack(dest_circ);
    break;
  case RELAY_COMMAND_CONFLUX_SWITCH:
    // We only test the case where the switch is initiated by the client.
    // It is symmetric, so this should not matter. If we ever want to test
    // the case where the switch is initiated by the exit, we will need to
    // get the cpath layer hint for the client.
    tor_assert(!CIRCUIT_IS_ORIGIN(dest_circ));
    conflux_process_switch_command(dest_circ, NULL, msg);
    break;
  }

  tor_free(delivery);
  tor_free(cell);
  relay_msg_free(msg);
  return;
}

static uint64_t
mock_monotime_absolute_usec(void)
{
  return 100;
}

static int
channel_get_addr_if_possible_mock(const channel_t *chan, tor_addr_t *addr_out)
{
  (void)chan;
  tt_int_op(AF_INET,OP_EQ, tor_addr_parse(addr_out, "18.0.0.1"));
  return 1;

 done:
  return 0;
}

static const node_t *
build_state_get_exit_node_mock(cpath_build_state_t *state)
{
  static node_t node;
  (void)state;
  memset(&node, 0, sizeof(node));
  return &node;
}

static void
circuit_mark_for_close_mock(circuit_t *circ, int reason,
                            int line, const char *file)
{
  (void)circ;
  (void)reason;
  (void)line;
  (void)file;

  log_info(LD_CIRC, "Marking circuit for close at %s:%d", file, line);

  if (BUG(circ->marked_for_close)) {
    log_warn(LD_BUG,
        "Duplicate call to circuit_mark_for_close at %s:%d"
        " (first at %s:%d)", file, line,
        circ->marked_for_close_file, circ->marked_for_close);
    return;
  }

  circ->marked_for_close = line;
  circ->marked_for_close_file = file;
  circ->marked_for_close_reason = reason;

  if (CIRCUIT_IS_CONFLUX(circ)) {
    conflux_circuit_has_closed(circ);
  }

  // Mark the other side for close too. No idea if this even improves things;
  // We might also want to make this go through the cell queue as a destroy
  if (CIRCUIT_IS_ORIGIN(circ)) {
    circuit_t *exit_circ = get_exit_circ(circ);
    if (!exit_circ->marked_for_close)
      circuit_mark_for_close_mock(get_exit_circ(circ), reason, line, file);
  } else {
    circuit_t *client_circ = get_client_circ(circ);
    if (!client_circ->marked_for_close)
      circuit_mark_for_close_mock(get_client_circ(circ), reason, line, file);
  }

  // XXX: Should we do this?
  //if (circuits_pending_close == NULL)
  //  circuits_pending_close = smartlist_new();
  //smartlist_add(circuits_pending_close, circ);
}

static void
simulate_single_hop_extend(origin_circuit_t *client, int exit)
{
  char whatevs_key[CPATH_KEY_MATERIAL_LEN];
  char digest[DIGEST_LEN];
  tor_addr_t addr = TOR_ADDR_NULL;

  // Advance time a tiny bit so we can calculate an RTT
  curr_mocked_time += 10 * TOR_NSEC_PER_MSEC;
  monotime_coarse_set_mock_time_nsec(curr_mocked_time);
  monotime_set_mock_time_nsec(curr_mocked_time);

  // Add a hop to cpath
  crypt_path_t *hop = tor_malloc_zero(sizeof(crypt_path_t));
  cpath_extend_linked_list(&client->cpath, hop);

  hop->magic = CRYPT_PATH_MAGIC;
  hop->state = CPATH_STATE_OPEN;

  // add an extend info to indicate if this node supports padding or not.
  // (set the first byte of the digest for our mocked node_get_by_id)
  digest[0] = exit;

  hop->extend_info = extend_info_new(
          exit ? "exit" : "non-exit",
          digest, NULL, NULL,
          &addr, exit, NULL, exit);

  cpath_init_circuit_crypto(RELAY_CRYPTO_ALG_TOR1, hop,
                            whatevs_key, sizeof(whatevs_key));

  hop->package_window = circuit_initial_package_window();
  hop->deliver_window = CIRCWINDOW_START;
}

static void
test_setup(void)
{
  int64_t actual_mocked_monotime_start;

  MOCK(circuitmux_attach_circuit, circuitmux_attach_circuit_mock);
  MOCK(channel_get_addr_if_possible, channel_get_addr_if_possible_mock);
  MOCK(circuit_establish_circuit_conflux,
       circuit_establish_circuit_conflux_mock);
  MOCK(circuit_package_relay_cell,
       circuit_package_relay_cell_mock);
  MOCK(circuit_mark_for_close_,
       circuit_mark_for_close_mock);
  MOCK(monotime_absolute_usec, mock_monotime_absolute_usec);
  testing_enable_reproducible_rng();

  monotime_init();
  monotime_enable_test_mocking();
  actual_mocked_monotime_start = MONOTIME_MOCK_START;
  monotime_set_mock_time_nsec(actual_mocked_monotime_start);
  monotime_coarse_set_mock_time_nsec(actual_mocked_monotime_start);
  curr_mocked_time = actual_mocked_monotime_start;

  client_circs = smartlist_new();
  exit_circs = smartlist_new();
  circ_pairs = smartlist_new();
  mock_cell_delivery = smartlist_new();
  dummy_channel.cmux = circuitmux_alloc();

  get_circuit_build_times_mutable()->timeout_ms = 1000;

  congestion_control_set_cc_enabled();
  max_unlinked_leg_retry = UINT32_MAX;
}

static void
test_clear_circs(void)
{
  conflux_notify_shutdown();
  SMARTLIST_FOREACH(circ_pairs, circ_pair_t *, circ_pair, {
    tor_free(circ_pair);
  });
  SMARTLIST_FOREACH(client_circs, circuit_t *, client_side, {
    conflux_circuit_about_to_free(client_side);
    circuit_free(client_side);
  });
  SMARTLIST_FOREACH(exit_circs, or_circuit_t *, relay_side, {
    free_fake_orcirc(relay_side);
  });

  smartlist_clear(circ_pairs);
  smartlist_clear(client_circs);
  smartlist_clear(exit_circs);

  if (client_streams) {
    // Free each edge connection
    SMARTLIST_FOREACH(client_streams, edge_connection_t *, edge_conn, {
      connection_free_minimal(TO_CONN(edge_conn));
    });
    smartlist_free(client_streams);
  }

  if (exit_streams) {
    // Free each edge connection
    SMARTLIST_FOREACH(exit_streams, edge_connection_t *, edge_conn, {
      connection_free_minimal(TO_CONN(edge_conn));
    });
    smartlist_free(exit_streams);
  }

  tor_assert(smartlist_len(mock_cell_delivery) == 0);

  (void)free_fake_origin_circuit;

  /* Clear shutdown flag so we can resume testing again. */
  conflux_clear_shutdown();
}

static void
test_teardown(void)
{
  conflux_pool_free_all();
  smartlist_free(client_circs);
  smartlist_free(exit_circs);
  smartlist_free(mock_cell_delivery);
  circuitmux_detach_all_circuits(dummy_channel.cmux, NULL);
  circuitmux_free(dummy_channel.cmux);
  testing_disable_reproducible_rng();
}

/* Test linking a conflux circuit */
static void
test_conflux_link(void *arg)
{
  (void) arg;
  test_setup();

  launch_new_set(2);

  // For each circuit in the client_circs list, we need to create an
  // exit side circuit and simulate two extends
  SMARTLIST_FOREACH(client_circs, circuit_t *, client_side, {
    simulate_circuit_build(client_side);

    /* Handle network activity*/
    while (smartlist_len(mock_cell_delivery) > 0) {
      process_mock_cell_delivery();
    }
  });

  tt_int_op(smartlist_len(client_circs), OP_EQ, 2);
  tt_int_op(smartlist_len(exit_circs), OP_EQ, 2);

  // Test that the cells have all been delivered
  tt_int_op(smartlist_len(mock_cell_delivery), OP_EQ, 0);

  // Test that the client side circuits are linked
  conflux_t *cfx = ((circuit_t*)smartlist_get(client_circs, 0))->conflux;
  SMARTLIST_FOREACH_BEGIN(client_circs, circuit_t *, client_side) {
    tt_int_op(client_side->purpose, OP_EQ, CIRCUIT_PURPOSE_CONFLUX_LINKED);
    tt_ptr_op(client_side->conflux, OP_EQ, cfx);
    tt_ptr_op(client_side->conflux_pending_nonce, OP_EQ, NULL);
  } SMARTLIST_FOREACH_END(client_side);

  // Test circuit teardown
  SMARTLIST_FOREACH_BEGIN(client_circs, circuit_t *, client_side) {
    circuit_mark_for_close(client_side, END_CIRC_REASON_FINISHED);
  } SMARTLIST_FOREACH_END(client_side);

 done:
  test_clear_circs();
  test_teardown();
}

static void
simulate_circuit_build(circuit_t *client_circ)
{
  // Create a relay circuit, and simulate the extend and open
  circuit_t *relay_side = NULL;

  relay_side = (circuit_t*)new_fake_orcirc(&dummy_channel, &dummy_channel);
  relay_side->purpose = CIRCUIT_PURPOSE_OR;
  relay_side->n_chan = NULL; // No next hop
  relay_side->ccontrol = tor_malloc_zero(sizeof(congestion_control_t));
  relay_side->ccontrol->sendme_pending_timestamps = smartlist_new();
  relay_side->ccontrol->sendme_inc = 31;
  smartlist_add(exit_circs, relay_side);
  simulate_circuit_built(client_circ, relay_side);
  conflux_circuit_has_opened(TO_ORIGIN_CIRCUIT(client_circ));
}

static circuit_t *
simulate_close_retry(circuit_t *close, bool manual_launch)
{
  // Find the dest pair for the circuit in the circ pair list,
  // and close it too
  circuit_t *dest = NULL;
  uint8_t *nonce = NULL;

  if (manual_launch) {
    nonce = tor_memdup(close->conflux->nonce, DIGEST256_LEN);
  }

  SMARTLIST_FOREACH_BEGIN(circ_pairs, circ_pair_t *, pair) {
    if (pair->client == close) {
      dest = pair->exit;
      SMARTLIST_DEL_CURRENT_KEEPORDER(circ_pairs, pair);
      tor_free(pair);
    } else if (pair->exit == close) {
      // This function should not be called on the exit side..
      tor_assert(0);
    }
  } SMARTLIST_FOREACH_END(pair);

  tor_assert(dest);
  log_info(LD_CIRC, "Simulating close of %p->%p, dest %p->%p",
           close, close->conflux, dest, dest->conflux);

  // Free all pending cells related to this close in mock_cell_delivery
  SMARTLIST_FOREACH(mock_cell_delivery, cell_delivery_t *, cd, {
    if (cd->circ == close || cd->circ == dest) {
      SMARTLIST_DEL_CURRENT_KEEPORDER(mock_cell_delivery, cd);
      tor_free(cd->cell);
      tor_free(cd);
    }
  });

  // When a circuit closes, both ends get notification,
  // and the client will launch a new circuit. We need to find
  // that circuit at the end of the list, and then simulate
  // building it, and creating a relay circuit for it.
  conflux_circuit_has_closed(close);
  conflux_circuit_has_closed(dest);

  //tor_assert(digest256map_size(get_unlinked_pool(true)) != 0);

  // Find these legs in our circuit lists, and free them
  tor_assert(CIRCUIT_IS_ORIGIN(close));
  tor_assert(!CIRCUIT_IS_ORIGIN(dest));
  SMARTLIST_FOREACH_BEGIN(client_circs, circuit_t *, client_side) {
    if (client_side == close) {
      SMARTLIST_DEL_CURRENT_KEEPORDER(client_circs, client_side);
      conflux_circuit_about_to_free(client_side);
      circuit_free(client_side);
    }
  } SMARTLIST_FOREACH_END(client_side);
  SMARTLIST_FOREACH_BEGIN(exit_circs, or_circuit_t *, exit_side) {
    if (exit_side == (or_circuit_t *)dest) {
      SMARTLIST_DEL_CURRENT_KEEPORDER(exit_circs, exit_side);
      free_fake_orcirc(exit_side);
    }
  } SMARTLIST_FOREACH_END(exit_side);

  if (manual_launch) {
    // Launch a new leg for this nonce
    tor_assert(nonce);
    conflux_launch_leg(nonce);
    tor_free(nonce);
  }

  if (smartlist_len(client_circs) == 0) {
    // No new circuit was launched
    return NULL;
  }

  // At this point, a new circuit will have launched on the client
  // list. Get that circuit from the end of the list and return it
  circuit_t * circ = smartlist_get(client_circs,
                        smartlist_len(client_circs) - 1);

  //tor_assert(circ->purpose == CIRCUIT_PURPOSE_CONFLUX_UNLINKED);

  return circ;
}

static void
test_retry(void)
{
  log_info(LD_CIRC, "==========NEW RUN ===========");
  launch_new_set(2);

  tt_int_op(smartlist_len(client_circs), OP_EQ, 2);
  circuit_t *client1 = smartlist_get(client_circs, 0);
  circuit_t *client2 = smartlist_get(client_circs, 1);

  get_circuit_build_times_mutable()->timeout_ms = 1000;

  // Dice roll on which leg builds first
  if (crypto_rand_int(2) == 0) {
    simulate_circuit_build(client1);
    simulate_circuit_build(client2);
  } else {
    simulate_circuit_build(client2);
    simulate_circuit_build(client1);
  }

  while (smartlist_len(mock_cell_delivery) > 0) {
    tt_int_op(smartlist_len(client_circs), OP_EQ, 2);
    tt_int_op(smartlist_len(exit_circs), OP_EQ, 2);
    tt_int_op(smartlist_len(circ_pairs), OP_EQ, 2);

    if (crypto_rand_int(2) == 0) {
      if (crypto_rand_int(2) == 0) {
        if (client1->purpose != CIRCUIT_PURPOSE_CONFLUX_LINKED) {
          client1 = simulate_close_retry(client1, false);
          simulate_circuit_build(client1);
        }
      } else {
        if (client2->purpose != CIRCUIT_PURPOSE_CONFLUX_LINKED) {
          client2 = simulate_close_retry(client2, false);
          simulate_circuit_build(client2);
        }
      }
    }

    process_mock_cell_delivery();
  }

  // Test that the cells have all been delivered
  tt_int_op(smartlist_len(mock_cell_delivery), OP_EQ, 0);
  tt_int_op(smartlist_len(client_circs), OP_EQ, 2);
  tt_int_op(smartlist_len(exit_circs), OP_EQ, 2);
  tt_int_op(smartlist_len(circ_pairs), OP_EQ, 2);

  conflux_t *cfx = ((circuit_t *)smartlist_get(client_circs, 0))->conflux;
  SMARTLIST_FOREACH_BEGIN(client_circs, circuit_t *, client_side) {
    tt_int_op(client_side->purpose, OP_EQ, CIRCUIT_PURPOSE_CONFLUX_LINKED);
    tt_ptr_op(client_side->conflux, OP_EQ, cfx);
    tt_ptr_op(client_side->conflux_pending_nonce, OP_EQ, NULL);
  } SMARTLIST_FOREACH_END(client_side);

  tt_int_op(digest256map_size(get_linked_pool(true)), OP_EQ, 1);
  tt_int_op(digest256map_size(get_unlinked_pool(true)), OP_EQ, 0);
  tt_int_op(digest256map_size(get_unlinked_pool(false)), OP_EQ, 0);
  tt_int_op(digest256map_size(get_linked_pool(false)), OP_EQ, 1);

  cfx = ((circuit_t *)smartlist_get(exit_circs, 0))->conflux;
  SMARTLIST_FOREACH_BEGIN(exit_circs, circuit_t *, exit_side) {
    tt_ptr_op(exit_side->conflux, OP_EQ, cfx);
    tt_ptr_op(exit_side->conflux_pending_nonce, OP_EQ, NULL);
  } SMARTLIST_FOREACH_END(exit_side);

  // Test circuit teardown
  SMARTLIST_FOREACH_BEGIN(client_circs, circuit_t *, client_side) {
    tt_int_op(client_side->purpose, OP_EQ, CIRCUIT_PURPOSE_CONFLUX_LINKED);
    circuit_mark_for_close(client_side, END_CIRC_REASON_FINISHED);
  } SMARTLIST_FOREACH_END(client_side);

  tt_int_op(digest256map_size(get_linked_pool(true)), OP_EQ, 0);
  tt_int_op(digest256map_size(get_unlinked_pool(true)), OP_EQ, 0);
  tt_int_op(digest256map_size(get_unlinked_pool(false)), OP_EQ, 0);
  tt_int_op(digest256map_size(get_linked_pool(false)), OP_EQ, 0);

  test_clear_circs();

 done:
   return;
}

/* Test linking a conflux circuit with build failures */
static void
test_conflux_link_retry(void *arg)
{
  (void) arg;
  test_setup();

  for (int i = 0; i < 500; i++) {
    test_retry();
  }

  test_teardown();
}

#if 0
/* Test closing both circuits in the set before the link handshake completes
 * on either leg, by closing circuits before process_mock_cell_delivery.
 *
 * XXX: This test currently fails because conflux keeps relaunching closed
 * circuits. We need to set a limit on the number of times we relaunch a
 * circuit before we can fix this test.
 */
static void
test_conflux_link_fail(void *arg)
{
  (void) arg;
  test_setup();

  launch_new_set(2);

  tt_int_op(smartlist_len(client_circs), OP_EQ, 2);
  circuit_t *client1 = smartlist_get(client_circs, 0);
  circuit_t *client2 = smartlist_get(client_circs, 1);

  get_circuit_build_times_mutable()->timeout_ms = 1000;

  // Close both circuits before the link handshake completes
  conflux_circuit_has_closed(client1);
  conflux_circuit_has_closed(client2);

  tt_int_op(digest256map_size(get_linked_pool(true)), OP_EQ, 0);
  tt_int_op(digest256map_size(get_unlinked_pool(true)), OP_EQ, 0);
  tt_int_op(digest256map_size(get_unlinked_pool(false)), OP_EQ, 0);
  tt_int_op(digest256map_size(get_linked_pool(false)), OP_EQ, 0);
 done:
  test_clear_circs();
  test_teardown();
}
#endif

//  - Relink test:
//    - More than 2 legs
//    - Close one linked leg; relink
//      - Test mismatching sequence numbers for link and data
//        - This should destroy the whole set
//    - RTT timeout relinking test
//    - Three circuits; close 1; retry and link
static void
test_conflux_link_relink(void *arg)
{
  (void) arg;
  test_setup();

  launch_new_set(3);

  tt_int_op(smartlist_len(client_circs), OP_EQ, 3);
  circuit_t *client1 = smartlist_get(client_circs, 0);
  circuit_t *client2 = smartlist_get(client_circs, 1);
  circuit_t *client3 = smartlist_get(client_circs, 2);

  get_circuit_build_times_mutable()->timeout_ms = 1000;

  simulate_circuit_build(client1);
  simulate_circuit_build(client2);
  simulate_circuit_build(client3);

  while (smartlist_len(mock_cell_delivery) > 0) {
    tt_int_op(smartlist_len(client_circs), OP_EQ, 3);
    tt_int_op(smartlist_len(exit_circs), OP_EQ, 3);
    tt_int_op(smartlist_len(circ_pairs), OP_EQ, 3);

    process_mock_cell_delivery();
  }

  // Now test closing and relinking the third leg
  client3 = simulate_close_retry(client3, true);
  simulate_circuit_build(client3);
  while (smartlist_len(mock_cell_delivery) > 0) {
    tt_int_op(smartlist_len(client_circs), OP_EQ, 3);
    tt_int_op(smartlist_len(exit_circs), OP_EQ, 3);
    tt_int_op(smartlist_len(circ_pairs), OP_EQ, 3);

    process_mock_cell_delivery();
  }

  tt_int_op(digest256map_size(get_linked_pool(true)), OP_EQ, 1);
  tt_int_op(digest256map_size(get_unlinked_pool(true)), OP_EQ, 0);
  tt_int_op(digest256map_size(get_unlinked_pool(false)), OP_EQ, 0);
  tt_int_op(digest256map_size(get_linked_pool(false)), OP_EQ, 1);

  // Now test closing all circuits and verify the conflux object is gone
  simulate_close_retry(client1, false);
  simulate_close_retry(client2, false);
  simulate_close_retry(client3, false);

  tt_int_op(digest256map_size(get_linked_pool(true)), OP_EQ, 0);
  tt_int_op(digest256map_size(get_unlinked_pool(true)), OP_EQ, 0);
  tt_int_op(digest256map_size(get_unlinked_pool(false)), OP_EQ, 0);
  tt_int_op(digest256map_size(get_linked_pool(false)), OP_EQ, 0);

 done:
  test_clear_circs();
  test_teardown();
}

#if 0
static void
test_conflux_close(void *arg)
{
  (void) arg;
  test_setup();

  launch_new_set(2);

  tt_int_op(smartlist_len(client_circs), OP_EQ, 2);
  circuit_t *client1 = smartlist_get(client_circs, 0);
  circuit_t *client2 = smartlist_get(client_circs, 1);

  get_circuit_build_times_mutable()->timeout_ms = 1000;

  simulate_circuit_build(client1);
  simulate_circuit_build(client2);

  while (smartlist_len(mock_cell_delivery) > 0) {
    tt_int_op(smartlist_len(client_circs), OP_EQ, 2);
    tt_int_op(smartlist_len(exit_circs), OP_EQ, 2);
    tt_int_op(smartlist_len(circ_pairs), OP_EQ, 2);

    process_mock_cell_delivery();
  }

  // There are actually 3 kinds of close: mark, mark+free,
  // and purpose change. We need to test these in link_retry, but
  // here our focus is on after the set is linked.

  // Additionally, we can close on an unlinked leg, or a non-critical linked
  // leg, or a critical linked leg that causes teardown
  // And we can close on linked legs when there are unlinked legs, or not.

  // And we can do this at the client, or the exit.
  // And we can do this with a circuit that has streams, or not.

  tt_int_op(digest256map_size(get_linked_pool(true)), OP_EQ, 0);
  tt_int_op(digest256map_size(get_unlinked_pool(true)), OP_EQ, 0);
  tt_int_op(digest256map_size(get_unlinked_pool(false)), OP_EQ, 0);
  tt_int_op(digest256map_size(get_linked_pool(false)), OP_EQ, 0);
 done:
  test_clear_circs();
  test_teardown();
}
#endif

// Test launching a new set and closing the first leg, but
// with mismatched sequence numbers (missing data)
//  - Test this teardown with only linked circs, and with some
//    unlinked circs
// Test mismatching sequence numbers for link and data:
// Test missing sent data from client (link cell mismatch):
// Test missing sent data from relay (linked cell mismatch):

static circuit_t *
get_exit_circ(circuit_t *client_circ)
{
  circuit_t *exit_circ = NULL;
  SMARTLIST_FOREACH_BEGIN(circ_pairs, circ_pair_t *, pair) {
    if (pair->client == client_circ) {
      exit_circ = pair->exit;
      break;
    }
  } SMARTLIST_FOREACH_END(pair);
  tor_assert(exit_circ);
  return exit_circ;
}

static circuit_t *
get_client_circ(circuit_t *exit_circ)
{
  circuit_t *client_circ = NULL;
  SMARTLIST_FOREACH_BEGIN(circ_pairs, circ_pair_t *, pair) {
    if (pair->exit == exit_circ) {
      client_circ = pair->client;
      break;
    }
  } SMARTLIST_FOREACH_END(pair);
  tor_assert(client_circ);
  return client_circ;
}

static edge_connection_t *
new_client_stream(origin_circuit_t *on_circ)
{
  edge_connection_t *stream = edge_connection_new(CONN_TYPE_EXIT, AF_INET);

  stream->stream_id = get_unique_stream_id_by_circ(on_circ);
  stream->on_circuit = TO_CIRCUIT(on_circ);
  stream->cpath_layer = on_circ->cpath->prev;

  stream->next_stream = on_circ->p_streams;
  on_circ->p_streams = stream;
  conflux_update_p_streams(on_circ, stream);

  smartlist_add(client_streams, stream);

  return stream;
}

static edge_connection_t *
new_exit_stream(circuit_t *on_circ, streamid_t stream_id)
{
  edge_connection_t *stream = edge_connection_new(CONN_TYPE_EXIT, AF_INET);

  stream->stream_id = stream_id;
  stream->on_circuit = on_circ;

  stream->next_stream = TO_OR_CIRCUIT(on_circ)->n_streams;
  conflux_update_n_streams(TO_OR_CIRCUIT(on_circ), stream);

  smartlist_add(exit_streams, stream);

  return stream;
}

static void
validate_stream_counts(circuit_t *circ, int expected)
{
  int count = 0;

  conflux_validate_stream_lists(circ->conflux);

  if (CIRCUIT_IS_ORIGIN(circ)) {
    origin_circuit_t *ocirc = TO_ORIGIN_CIRCUIT(circ);
    tor_assert_nonfatal(circ->purpose == CIRCUIT_PURPOSE_CONFLUX_LINKED);
    /* Iterate over stream list using next_stream pointer, until null */
    for (edge_connection_t *stream = ocirc->p_streams; stream;
           stream = stream->next_stream) {
      count++;
    }
  } else {
    or_circuit_t *orcirc = TO_OR_CIRCUIT(circ);
    /* Iterate over stream list using next_stream pointer, until null */
    for (edge_connection_t *stream = orcirc->n_streams; stream;
           stream = stream->next_stream) {
      count++;
    }
  }
  tt_int_op(count, OP_EQ, expected);

 done:
  return;
}

//  - Streams test
//    - Attach streams
//      - Fail one leg, free it, attach new leg, new stream
//      - Fail both legs
//    - Shutdown
//      - With streams attached
static void
test_conflux_link_streams(void *arg)
{
  (void) arg;
  test_setup();

  launch_new_set(2);

  client_streams = smartlist_new();
  exit_streams = smartlist_new();

  tt_int_op(smartlist_len(client_circs), OP_EQ, 2);
  circuit_t *client1 = smartlist_get(client_circs, 0);
  circuit_t *client2 = smartlist_get(client_circs, 1);

  get_circuit_build_times_mutable()->timeout_ms = 1000;

  simulate_circuit_build(client1);
  simulate_circuit_build(client2);

  while (smartlist_len(mock_cell_delivery) > 0) {
    tt_int_op(smartlist_len(client_circs), OP_EQ, 2);
    tt_int_op(smartlist_len(exit_circs), OP_EQ, 2);
    tt_int_op(smartlist_len(circ_pairs), OP_EQ, 2);

    process_mock_cell_delivery();
  }

  // Attach a stream to the client1 circuit
  new_client_stream(TO_ORIGIN_CIRCUIT(client1));
  new_client_stream(TO_ORIGIN_CIRCUIT(client2));
  new_client_stream(TO_ORIGIN_CIRCUIT(client1));
  new_exit_stream(get_exit_circ(client2), 1);
  new_exit_stream(get_exit_circ(client1), 1);
  new_exit_stream(get_exit_circ(client1), 1);

  // Test that we can close the first leg, and attach a new one
  // with a new stream
  client1 = simulate_close_retry(client1, true);
  simulate_circuit_build(client1);

  while (smartlist_len(mock_cell_delivery) > 0) {
    tt_int_op(smartlist_len(client_circs), OP_EQ, 2);
    tt_int_op(smartlist_len(exit_circs), OP_EQ, 2);
    tt_int_op(smartlist_len(circ_pairs), OP_EQ, 2);

    process_mock_cell_delivery();
  }

  tt_ptr_op(client1->conflux, OP_EQ, client2->conflux);

  new_client_stream(TO_ORIGIN_CIRCUIT(client1));
  new_exit_stream(get_exit_circ(client2), 1);

  // Use Ensure that there are four streams on each circuit
  validate_stream_counts(client1, 4);
  validate_stream_counts(client2, 4);
  validate_stream_counts(get_exit_circ(client1), 4);
  validate_stream_counts(get_exit_circ(client2), 4);

  // Test that we can close all streams on either circuit,
  // in any order
  circuit_detach_stream(get_exit_circ(client1),
                        TO_OR_CIRCUIT(get_exit_circ(client1))->n_streams);
  validate_stream_counts(get_exit_circ(client2), 3);
  circuit_detach_stream(get_exit_circ(client2),
               TO_OR_CIRCUIT(get_exit_circ(client2))->n_streams->next_stream);
  validate_stream_counts(get_exit_circ(client1), 2);
  circuit_detach_stream(get_exit_circ(client1),
                        TO_OR_CIRCUIT(get_exit_circ(client1))->n_streams);
  validate_stream_counts(get_exit_circ(client1), 1);
  circuit_detach_stream(get_exit_circ(client1),
                        TO_OR_CIRCUIT(get_exit_circ(client1))->n_streams);
  validate_stream_counts(get_exit_circ(client1), 0);

  circuit_detach_stream(client1,
     TO_ORIGIN_CIRCUIT(client1)->p_streams->next_stream->
                                 next_stream->next_stream);
  circuit_detach_stream(client2,
                     TO_ORIGIN_CIRCUIT(client2)->p_streams);
  circuit_detach_stream(client2,
   TO_ORIGIN_CIRCUIT(client2)->p_streams->next_stream);
  circuit_detach_stream(client2,
   TO_ORIGIN_CIRCUIT(client2)->p_streams);
  validate_stream_counts(client1, 0);
  validate_stream_counts(client2, 0);

 done:
  test_clear_circs();
  test_teardown();
}

// Right now this does not involve congestion control.. But it could,
// if we actually build and send real RELAY_DATA cells (and also handle them
// and SENDME cells in the mocked cell delivery)
static void
send_fake_cell(circuit_t *client_circ)
{
  circuit_t *exit_circ = get_exit_circ(client_circ);
  conflux_leg_t *exit_leg = conflux_get_leg(exit_circ->conflux,
                                            exit_circ);

  TO_ORIGIN_CIRCUIT(client_circ)->cpath->prev->ccontrol->inflight++;
  conflux_note_cell_sent(client_circ->conflux, client_circ,
                          RELAY_COMMAND_DATA);

  exit_leg->last_seq_recv++;
  exit_circ->conflux->last_seq_delivered++;
}

static circuit_t *
send_until_switch(circuit_t *client_circ)
{
  conflux_leg_t *client_leg = conflux_get_leg(client_circ->conflux,
                                              client_circ);
  circuit_t *exit_circ = get_exit_circ(client_circ);
  conflux_leg_t *exit_leg = conflux_get_leg(exit_circ->conflux,
                                            exit_circ);
  circuit_t *next_circ = client_circ;
  int i = 0;

  // XXX: This is a hack so the tests pass using cc->sendme_inc
  // (There is another hack in circuit_ready_to_send() that causes
  // us to block early below, and return NULL for next_circ)
  TO_ORIGIN_CIRCUIT(client_circ)->cpath->prev->ccontrol->sendme_inc = 0;

  while (client_circ == next_circ) {
    next_circ = conflux_decide_circ_for_send(client_circ->conflux, client_circ,
                                             RELAY_COMMAND_DATA);
    tor_assert(next_circ);
    send_fake_cell(next_circ);
    i++;
  }

  // XXX: This too:
  TO_ORIGIN_CIRCUIT(client_circ)->cpath->prev->ccontrol->sendme_inc = 31;

  log_info(LD_CIRC, "Sent %d cells on client circ", i-1);
  process_mock_cell_delivery();

  circuit_t *new_client =
    (circuit_t*)conflux_decide_next_circ(client_circ->conflux);
  tt_ptr_op(new_client, OP_NE, client_circ);
  conflux_leg_t *new_client_leg = conflux_get_leg(new_client->conflux,
                                                  new_client);
  circuit_t *new_exit = get_exit_circ(new_client);
  conflux_leg_t *new_exit_leg = conflux_get_leg(new_exit->conflux,
                                                new_exit);

  // Verify sequence numbers make sense
  tt_int_op(new_client_leg->last_seq_sent, OP_EQ, client_leg->last_seq_sent+1);
  tt_int_op(new_client_leg->last_seq_recv, OP_EQ, client_leg->last_seq_recv);
  tt_int_op(exit_leg->last_seq_sent, OP_EQ, new_exit_leg->last_seq_sent);
  tt_int_op(exit_leg->last_seq_recv+1, OP_EQ, new_exit_leg->last_seq_recv);

  tt_int_op(client_leg->last_seq_sent+1, OP_EQ, new_exit_leg->last_seq_recv);
  tt_int_op(client_leg->last_seq_recv, OP_EQ, new_exit_leg->last_seq_sent);

 done:
  return new_client;
}

/**
 * This tests switching as well as the UDP optimization that attaches
 * a third circuit and closes the slowest one. (This optimization is not
 * implemented in C-Tor but must be supported at exits, for arti).
 */
static void
test_conflux_switch(void *arg)
{
   (void) arg;
  test_setup();
  DEFAULT_EXIT_UX = CONFLUX_UX_HIGH_THROUGHPUT;

  launch_new_set(2);

  tt_int_op(smartlist_len(client_circs), OP_EQ, 2);
  circuit_t *client1 = smartlist_get(client_circs, 0);
  circuit_t *client2 = smartlist_get(client_circs, 1);
  get_circuit_build_times_mutable()->timeout_ms = 1000;

  simulate_circuit_build(client1);
  simulate_circuit_build(client2);

  circuit_t *exit1 = get_exit_circ(client1);
  circuit_t *exit2 = get_exit_circ(client2);
  circuit_t *next_circ = client1;

  while (smartlist_len(mock_cell_delivery) > 0) {
    tt_int_op(smartlist_len(client_circs), OP_EQ, 2);
    tt_int_op(smartlist_len(exit_circs), OP_EQ, 2);
    tt_int_op(smartlist_len(circ_pairs), OP_EQ, 2);

    process_mock_cell_delivery();
  }

  // Check to make sure everything is linked`up
  tt_ptr_op(client1->conflux, OP_EQ, client2->conflux);
  tt_ptr_op(exit1->conflux, OP_EQ, exit2->conflux);
  tt_ptr_op(client1->conflux, OP_NE, NULL);
  tt_ptr_op(exit1->conflux, OP_NE, NULL);
  tt_int_op(smartlist_len(client1->conflux->legs), OP_EQ, 2);
  tt_int_op(smartlist_len(exit1->conflux->legs), OP_EQ, 2);
  tt_int_op(client1->purpose, OP_EQ, CIRCUIT_PURPOSE_CONFLUX_LINKED);
  tt_int_op(client2->purpose, OP_EQ, CIRCUIT_PURPOSE_CONFLUX_LINKED);

  tt_int_op(digest256map_size(get_linked_pool(true)), OP_EQ, 1);
  tt_int_op(digest256map_size(get_unlinked_pool(true)), OP_EQ, 0);
  tt_int_op(digest256map_size(get_unlinked_pool(false)), OP_EQ, 0);
  tt_int_op(digest256map_size(get_linked_pool(false)), OP_EQ, 1);
  tt_ptr_op(get_exit_circ(client1), OP_EQ, exit1);
  tt_ptr_op(get_exit_circ(client2), OP_EQ, exit2);

  // Give circuits approximately equal RTT:
  conflux_update_rtt(client1->conflux, client1, 100);
  conflux_update_rtt(client2->conflux, client2, 125);

  client1->conflux->params.alg = CONFLUX_ALG_LOWRTT;
  get_exit_circ(client1)->conflux->params.alg = CONFLUX_ALG_LOWRTT;
  TO_ORIGIN_CIRCUIT(client1)->cpath->prev->ccontrol->cwnd = 300;
  TO_ORIGIN_CIRCUIT(client2)->cpath->prev->ccontrol->cwnd = 300;

  // Keep sending fake cells until we decide to switch four times
  for (int i = 0; i < 4; i++) {
    next_circ = send_until_switch(next_circ);

    // XXX: This can't be set to 0 or we will decide we can switch immediately,
    // because the client1 has a lower RTT
    TO_ORIGIN_CIRCUIT(client1)->cpath->prev->ccontrol->inflight = 1;

    // Check to make sure everything is linked`up
    tt_ptr_op(client1->conflux, OP_EQ, client2->conflux);
    tt_ptr_op(exit1->conflux, OP_EQ, exit2->conflux);
    tt_ptr_op(client1->conflux, OP_NE, NULL);
    tt_ptr_op(exit1->conflux, OP_NE, NULL);
    tt_int_op(smartlist_len(client1->conflux->legs), OP_EQ, 2);
    tt_int_op(smartlist_len(exit1->conflux->legs), OP_EQ, 2);
    tt_int_op(client1->purpose, OP_EQ, CIRCUIT_PURPOSE_CONFLUX_LINKED);
    tt_int_op(client2->purpose, OP_EQ, CIRCUIT_PURPOSE_CONFLUX_LINKED);

    tt_int_op(digest256map_size(get_linked_pool(true)), OP_EQ, 1);
    tt_int_op(digest256map_size(get_unlinked_pool(true)), OP_EQ, 0);
    tt_int_op(digest256map_size(get_unlinked_pool(false)), OP_EQ, 0);
    tt_int_op(digest256map_size(get_linked_pool(false)), OP_EQ, 1);

    tt_ptr_op(get_exit_circ(client1), OP_EQ, exit1);
    tt_ptr_op(get_exit_circ(client2), OP_EQ, exit2);
    tt_ptr_op(next_circ, OP_EQ, client2);

    next_circ = send_until_switch(next_circ);

    // Check to make sure everything is linked`up
    tt_ptr_op(client1->conflux, OP_EQ, client2->conflux);
    tt_ptr_op(exit1->conflux, OP_EQ, exit2->conflux);
    tt_ptr_op(client1->conflux, OP_NE, NULL);
    tt_ptr_op(exit1->conflux, OP_NE, NULL);
    tt_int_op(smartlist_len(client1->conflux->legs), OP_EQ, 2);
    tt_int_op(smartlist_len(exit1->conflux->legs), OP_EQ, 2);
    tt_int_op(client1->purpose, OP_EQ, CIRCUIT_PURPOSE_CONFLUX_LINKED);
    tt_int_op(client2->purpose, OP_EQ, CIRCUIT_PURPOSE_CONFLUX_LINKED);

    tt_int_op(digest256map_size(get_linked_pool(true)), OP_EQ, 1);
    tt_int_op(digest256map_size(get_unlinked_pool(true)), OP_EQ, 0);
    tt_int_op(digest256map_size(get_unlinked_pool(false)), OP_EQ, 0);
    tt_int_op(digest256map_size(get_linked_pool(false)), OP_EQ, 1);

    tt_ptr_op(get_exit_circ(client1), OP_EQ, exit1);
    tt_ptr_op(get_exit_circ(client2), OP_EQ, exit2);
    tt_ptr_op(next_circ, OP_EQ, client1);

    TO_ORIGIN_CIRCUIT(client2)->cpath->prev->ccontrol->inflight = 0;
  }

  // Try the UDP minRTT reconnect optimization a few times
  for (int i = 0; i < 500; i++) {
    tt_int_op(smartlist_len(client_circs), OP_EQ, 2);
    client1 = smartlist_get(client_circs, 0);
    client2 = smartlist_get(client_circs, 1);
    exit1 = get_exit_circ(client1);
    exit2 = get_exit_circ(client2);

    // Attach a third leg
    conflux_launch_leg(client1->conflux->nonce);

    // It should be added to the end of the local test list
    circuit_t *client3 = smartlist_get(client_circs,
                                       smartlist_len(client_circs)-1);
    simulate_circuit_build(client3);

    while (smartlist_len(mock_cell_delivery) > 0) {
      tt_int_op(smartlist_len(client_circs), OP_EQ, 3);
      tt_int_op(smartlist_len(exit_circs), OP_EQ, 3);
      tt_int_op(smartlist_len(circ_pairs), OP_EQ, 3);

      process_mock_cell_delivery();
    }

    circuit_t *exit3 = get_exit_circ(client3);

    // Check to make sure everything is linked`up
    tt_ptr_op(client3->conflux, OP_EQ, client2->conflux);
    tt_ptr_op(exit3->conflux, OP_EQ, exit2->conflux);
    tt_ptr_op(client3->conflux, OP_NE, NULL);
    tt_ptr_op(exit3->conflux, OP_NE, NULL);
    tt_int_op(smartlist_len(client1->conflux->legs), OP_EQ, 3);
    tt_int_op(smartlist_len(exit1->conflux->legs), OP_EQ, 3);
    tt_int_op(client3->purpose, OP_EQ, CIRCUIT_PURPOSE_CONFLUX_LINKED);

    tt_int_op(digest256map_size(get_linked_pool(true)), OP_EQ, 1);
    tt_int_op(digest256map_size(get_unlinked_pool(true)), OP_EQ, 0);
    tt_int_op(digest256map_size(get_unlinked_pool(false)), OP_EQ, 0);
    tt_int_op(digest256map_size(get_linked_pool(false)), OP_EQ, 1);

    conflux_update_rtt(client3->conflux, client3,
                       crypto_rand_int_range(90, 200));
    TO_ORIGIN_CIRCUIT(client3)->cpath->prev->ccontrol->cwnd = 300;

    circuit_t *circ_close = NULL;
    uint64_t max_rtt = 0;
    // Pick the leg with the highest RTT and close it
    tor_assert(client3);
    tor_assert(client3->conflux);
    tor_assert(client3->conflux->legs);
    CONFLUX_FOR_EACH_LEG_BEGIN(client3->conflux, leg) {
      if (client3->conflux->curr_leg == leg)
        continue;

      if (leg->circ_rtts_usec > max_rtt) {
        max_rtt = leg->circ_rtts_usec;
        circ_close = leg->circ;
      }
    } CONFLUX_FOR_EACH_LEG_END(leg);

    // Let the second leg "send" all data and close it.
    tor_assert(circ_close);
    tor_assert(circ_close->conflux);
    tor_assert(circ_close->conflux->legs);
    CONFLUX_FOR_EACH_LEG_BEGIN(circ_close->conflux, leg) {
      TO_ORIGIN_CIRCUIT(leg->circ)->cpath->prev->ccontrol->inflight = 0;
    } CONFLUX_FOR_EACH_LEG_END(leg);

    // Close without manual launch (code will not relaunch for linked)
    simulate_close_retry(circ_close, false);

    tt_int_op(smartlist_len(mock_cell_delivery), OP_EQ, 0);
    tt_int_op(smartlist_len(client_circs), OP_EQ, 2);
    tt_int_op(smartlist_len(exit_circs), OP_EQ, 2);
    tt_int_op(smartlist_len(circ_pairs), OP_EQ, 2);

    // Send until we switch to the third leg
    next_circ = send_until_switch(next_circ);

    // Check to make sure everything is linked`up
    tt_ptr_op(next_circ->conflux, OP_NE, NULL);
    tt_int_op(smartlist_len(next_circ->conflux->legs), OP_EQ, 2);
    tt_int_op(next_circ->purpose, OP_EQ, CIRCUIT_PURPOSE_CONFLUX_LINKED);

    tt_int_op(digest256map_size(get_linked_pool(true)), OP_EQ, 1);
    tt_int_op(digest256map_size(get_unlinked_pool(true)), OP_EQ, 0);
    tt_int_op(digest256map_size(get_unlinked_pool(false)), OP_EQ, 0);
    tt_int_op(digest256map_size(get_linked_pool(false)), OP_EQ, 1);

    CONFLUX_FOR_EACH_LEG_BEGIN(next_circ->conflux, leg) {
      TO_ORIGIN_CIRCUIT(leg->circ)->cpath->prev->ccontrol->inflight = 0;
    } CONFLUX_FOR_EACH_LEG_END(leg);

    // send until we switch back to the first leg
    next_circ = send_until_switch(next_circ);

    // Check to make sure everything is linked`up
    tt_ptr_op(next_circ->conflux, OP_NE, NULL);
    tt_int_op(smartlist_len(next_circ->conflux->legs), OP_EQ, 2);
    tt_int_op(next_circ->purpose, OP_EQ, CIRCUIT_PURPOSE_CONFLUX_LINKED);

    tt_int_op(digest256map_size(get_linked_pool(true)), OP_EQ, 1);
    tt_int_op(digest256map_size(get_unlinked_pool(true)), OP_EQ, 0);
    tt_int_op(digest256map_size(get_unlinked_pool(false)), OP_EQ, 0);
    tt_int_op(digest256map_size(get_linked_pool(false)), OP_EQ, 1);
  }

 done:
  test_clear_circs();
  test_teardown();
  return;
 }

static void
assert_begindir_rejected_with_end(circuit_t *relay_circ, streamid_t stream_id)
{
  relay_msg_t msg;
  static const char begin_dir_body[] = "";
  int rv;

  tor_assert(!CIRCUIT_IS_ORIGIN(relay_circ));

  reset_begindir_end_capture();
  make_test_relay_msg(&msg, RELAY_COMMAND_BEGIN_DIR,
                      begin_dir_body, 0, stream_id);

  tt_ptr_op(TO_OR_CIRCUIT(relay_circ)->n_streams, OP_EQ, NULL);
  rv = connection_exit_begin_conn(&msg, relay_circ);
  tt_int_op(rv, OP_EQ, 0);

  tt_int_op(begindir_end_capture.called, OP_EQ, 1);
  tt_int_op(begindir_end_capture.stream_id, OP_EQ, stream_id);
  tt_ptr_op(begindir_end_capture.circ, OP_EQ, relay_circ);
  tt_int_op(begindir_end_capture.relay_command, OP_EQ, RELAY_COMMAND_END);
  tt_int_op((int)begindir_end_capture.payload_len, OP_EQ, 1);
  tt_int_op(begindir_end_capture.end_reason,
            OP_EQ, END_STREAM_REASON_NOTDIRECTORY);
  tt_ptr_op(TO_OR_CIRCUIT(relay_circ)->n_streams, OP_EQ, NULL);

 done:
  return;
}

static void
test_conflux_begindir_rejects_pending_unlinked_with_end(void *arg)
{
  (void)arg;
  int orig_bridge_relay;
  int orig_orport_set;
  or_circuit_t *pending_exit;
  test_setup();

  orig_bridge_relay = get_options_mutable()->BridgeRelay;
  orig_orport_set = get_options_mutable()->ORPort_set;
  get_options_mutable()->BridgeRelay = 1;
  get_options_mutable()->ORPort_set = 1;

  pending_exit = new_fake_orcirc(&dummy_channel, &dummy_channel);
  pending_exit->base_.purpose = CIRCUIT_PURPOSE_OR;
  pending_exit->base_.conflux_pending_nonce = tor_malloc_zero(DIGEST256_LEN);
  tt_int_op(CIRCUIT_IS_CONFLUX(TO_CIRCUIT(pending_exit)), OP_EQ, 1);
  smartlist_add(exit_circs, pending_exit);

  MOCK(relay_send_command_from_edge_, mock_relay_send_command_from_edge);
  assert_begindir_rejected_with_end(TO_CIRCUIT(pending_exit), 11);
  UNMOCK(relay_send_command_from_edge_);

 done:
  while (smartlist_len(mock_cell_delivery) > 0) {
    process_mock_cell_delivery();
  }
  get_options_mutable()->BridgeRelay = orig_bridge_relay;
  get_options_mutable()->ORPort_set = orig_orport_set;
  test_clear_circs();
  test_teardown();
}

static void
test_conflux_begindir_rejects_linked_with_end(void *arg)
{
  (void)arg;
  int orig_bridge_relay;
  int orig_orport_set;
  test_setup();

  orig_bridge_relay = get_options_mutable()->BridgeRelay;
  orig_orport_set = get_options_mutable()->ORPort_set;
  get_options_mutable()->BridgeRelay = 1;
  get_options_mutable()->ORPort_set = 1;

  launch_new_set(2);
  tt_int_op(smartlist_len(client_circs), OP_EQ, 2);

  circuit_t *client1 = smartlist_get(client_circs, 0);
  circuit_t *client2 = smartlist_get(client_circs, 1);
  simulate_circuit_build(client1);
  simulate_circuit_build(client2);

  while (smartlist_len(mock_cell_delivery) > 0) {
    process_mock_cell_delivery();
  }

  circuit_t *exit1 = get_exit_circ(client1);
  tt_ptr_op(exit1->conflux, OP_NE, NULL);
  tt_int_op(smartlist_len(exit1->conflux->legs), OP_EQ, 2);

  MOCK(relay_send_command_from_edge_, mock_relay_send_command_from_edge);
  assert_begindir_rejected_with_end(exit1, 12);
  UNMOCK(relay_send_command_from_edge_);

 done:
  get_options_mutable()->BridgeRelay = orig_bridge_relay;
  get_options_mutable()->ORPort_set = orig_orport_set;
  test_clear_circs();
  test_teardown();
}

static void
test_conflux_begindir_rejects_one_leg_with_end(void *arg)
{
  (void)arg;
  int orig_bridge_relay;
  int orig_orport_set;
  test_setup();

  orig_bridge_relay = get_options_mutable()->BridgeRelay;
  orig_orport_set = get_options_mutable()->ORPort_set;
  get_options_mutable()->BridgeRelay = 1;
  get_options_mutable()->ORPort_set = 1;

  launch_new_set(2);
  tt_int_op(smartlist_len(client_circs), OP_EQ, 2);

  circuit_t *client1 = smartlist_get(client_circs, 0);
  circuit_t *client2 = smartlist_get(client_circs, 1);
  simulate_circuit_build(client1);
  simulate_circuit_build(client2);

  while (smartlist_len(mock_cell_delivery) > 0) {
    process_mock_cell_delivery();
  }

  (void)simulate_close_retry(client2, false);
  tt_int_op(smartlist_len(client_circs), OP_EQ, 1);

  circuit_t *remaining_client = smartlist_get(client_circs, 0);
  circuit_t *remaining_exit = get_exit_circ(remaining_client);
  tt_ptr_op(remaining_exit->conflux, OP_NE, NULL);
  tt_int_op(smartlist_len(remaining_exit->conflux->legs), OP_EQ, 1);

  MOCK(relay_send_command_from_edge_, mock_relay_send_command_from_edge);
  assert_begindir_rejected_with_end(remaining_exit, 13);
  UNMOCK(relay_send_command_from_edge_);

 done:
  get_options_mutable()->BridgeRelay = orig_bridge_relay;
  get_options_mutable()->ORPort_set = orig_orport_set;
  test_clear_circs();
  test_teardown();
}

static void
test_conflux_circuit_get_best_rejects_internal(void *arg)
{
  (void)arg;
  entry_connection_t *entry_conn = NULL;
  origin_circuit_t *selected = NULL;
  test_setup();

  launch_new_set(2);
  tt_int_op(smartlist_len(client_circs), OP_EQ, 2);

  circuit_t *client1 = smartlist_get(client_circs, 0);
  circuit_t *client2 = smartlist_get(client_circs, 1);
  simulate_circuit_build(client1);
  simulate_circuit_build(client2);

  while (smartlist_len(mock_cell_delivery) > 0) {
    process_mock_cell_delivery();
  }

  tt_int_op(digest256map_size(get_linked_pool(true)), OP_EQ, 1);

  SMARTLIST_FOREACH_BEGIN(client_circs, circuit_t *, client_circ) {
    client_circ->n_chan = &dummy_channel;
    client_circ->state = CIRCUIT_STATE_OPEN;
  } SMARTLIST_FOREACH_END(client_circ);

  MOCK(build_state_get_exit_node, build_state_get_exit_node_mock);

  entry_conn = entry_connection_new(CONN_TYPE_AP, AF_INET);
  tt_ptr_op(entry_conn, OP_NE, NULL);
  tt_ptr_op(entry_conn->socks_request, OP_NE, NULL);

  entry_conn->socks_request->command = SOCKS_COMMAND_CONNECT;
  strlcpy(entry_conn->socks_request->address, "18.0.0.1",
          sizeof(entry_conn->socks_request->address));
  entry_conn->socks_request->port = 443;
  entry_conn->use_begindir = 1;
  entry_conn->want_onehop = 0;

  selected = circuit_get_best(entry_conn, 1, CIRCUIT_PURPOSE_C_GENERAL,
                              0, 0);
  tt_ptr_op(selected, OP_NE, NULL);
  tt_int_op(selected->base_.purpose, OP_EQ, CIRCUIT_PURPOSE_CONFLUX_LINKED);

  selected = circuit_get_best(entry_conn, 1, CIRCUIT_PURPOSE_C_GENERAL,
                              0, 1);
  tt_ptr_op(selected, OP_EQ, NULL);

 done:
  UNMOCK(build_state_get_exit_node);
  if (entry_conn) {
    connection_free_minimal(ENTRY_TO_CONN(entry_conn));
  }
  test_clear_circs();
  test_teardown();
}

/* Regression test for #41251: ensure OOO queue bytes accounting is cleared
 * when a conflux set is torn down with queued out-of-order cells. */
static void
test_conflux_ooo_q_teardown_accounting(void *arg)
{
  (void) arg;
  test_setup();

  launch_new_set(2);

  tt_int_op(smartlist_len(client_circs), OP_EQ, 2);
  circuit_t *client1 = smartlist_get(client_circs, 0);
  circuit_t *client2 = smartlist_get(client_circs, 1);

  simulate_circuit_build(client1);
  simulate_circuit_build(client2);

  while (smartlist_len(mock_cell_delivery) > 0) {
    process_mock_cell_delivery();
  }

  circuit_t *exit1 = get_exit_circ(client1);
  tor_assert(exit1);
  tor_assert(exit1->conflux);

  conflux_t *cfx = exit1->conflux;
  conflux_leg_t *leg = conflux_get_leg(cfx, exit1);
  tor_assert(leg);

  /* Force RELAY_DATA cells into the OOO queue by creating a sequence gap. */
  cfx->last_seq_delivered = 1;
  leg->last_seq_recv = 5;

  relay_msg_t msg;
  const char body[16] = {'A'};
  make_test_relay_msg(&msg, RELAY_COMMAND_DATA, &body, sizeof(body), 42);
  tt_int_op(conflux_process_relay_msg(cfx, exit1, NULL, &msg), OP_EQ, 0);
  tt_int_op(conflux_process_relay_msg(cfx, exit1, NULL, &msg), OP_EQ, 0);

  tt_int_op(smartlist_len(cfx->ooo_q), OP_EQ, 2);
  /* Same as conflux_msg_alloc_cost() but we have a relay_msg_t. */
  tt_u64_op(conflux_get_total_bytes_allocation(), OP_EQ,
            2 * (msg.length + sizeof(msg) + sizeof(conflux_msg_t)));

  /* test_clear_circs() frees all legs and should free the conflux object.
   * Accounting must go back to zero even if OOO queue is non-empty. */
  test_clear_circs();
  tt_u64_op(conflux_get_total_bytes_allocation(), OP_EQ, 0);

 done:
  test_teardown();
}

/* Regression test for #41306 (TROVE-2026-026): a recovery leg launched for an
 * already-linked set must not end up sharing ownership of the conflux object
 * with a previously-closed (but not yet reaped) last leg. Otherwise both
 * circuits free the same conflux object on reap, causing a use-after-free and
 * double free.
 *
 * The scenario: (a) link a 2-leg set, (b) launch a recovery leg for the same
 * nonce, (c) close both linked legs without reaping them -- the second close
 * empties the linked set and pulls the conflux object out of the pool while
 * keeping it attached to the closed leg, (d) link the recovery leg, which
 * revives the very same conflux object and re-adds it to the linked pool, then
 * (e) close the recovery leg. At this point, pre-fix, two distinct circuits
 * point at the same conflux object. Reaping them all then frees it twice. */
static void
test_conflux_recovery_leg(void *arg)
{
  (void) arg;
  test_setup();

  /* (a) Build and link a 2-leg set. */
  launch_new_set(2);
  tt_int_op(smartlist_len(client_circs), OP_EQ, 2);
  circuit_t *client1 = smartlist_get(client_circs, 0);
  circuit_t *client2 = smartlist_get(client_circs, 1);

  simulate_circuit_build(client1);
  simulate_circuit_build(client2);
  while (smartlist_len(mock_cell_delivery) > 0) {
    process_mock_cell_delivery();
  }

  /* Both legs are linked and share the same conflux object. */
  conflux_t *cfx = client1->conflux;
  tt_ptr_op(cfx, OP_NE, NULL);
  tt_ptr_op(client2->conflux, OP_EQ, cfx);
  tt_int_op(digest256map_size(get_linked_pool(true)), OP_EQ, 1);
  tt_int_op(digest256map_size(get_unlinked_pool(true)), OP_EQ, 0);

  /* (b) Launch a recovery leg for the same nonce. It shares the same conflux
   * object and sits in the unlinked pool until it links. */
  uint8_t nonce[DIGEST256_LEN];
  memcpy(nonce, cfx->nonce, sizeof(nonce));
  tt_assert(conflux_launch_leg(nonce));
  tt_int_op(smartlist_len(client_circs), OP_EQ, 3);
  circuit_t *recovery = smartlist_get(client_circs, 2);
  tt_int_op(digest256map_size(get_unlinked_pool(true)), OP_EQ, 1);

  /* (c) Close both linked legs, but do NOT reap them yet. The second close
   * empties the linked set: the conflux object leaves the linked pool but,
   * with a recovery leg still around, ownership must be handed over to the
   * recovery set rather than kept on the closing leg. */
  conflux_circuit_has_closed(client1);
  conflux_circuit_has_closed(client2);
  tt_ptr_op(client2->conflux, OP_EQ, NULL);
  tt_int_op(digest256map_size(get_linked_pool(true)), OP_EQ, 0);
  tt_int_op(digest256map_size(get_unlinked_pool(true)), OP_EQ, 1);

  /* (d) Link the recovery leg. It revives the same conflux object and re-adds
   * it to the linked pool, becoming its owner. */
  simulate_circuit_build(recovery);
  while (smartlist_len(mock_cell_delivery) > 0) {
    process_mock_cell_delivery();
  }
  tt_int_op(recovery->purpose, OP_EQ, CIRCUIT_PURPOSE_CONFLUX_LINKED);
  tt_ptr_op(recovery->conflux, OP_EQ, cfx);
  tt_int_op(digest256map_size(get_linked_pool(true)), OP_EQ, 1);
  tt_int_op(digest256map_size(get_unlinked_pool(true)), OP_EQ, 0);

  /* (e) Close the recovery leg as well, without reaping it. */
  conflux_circuit_has_closed(recovery);
  tt_int_op(digest256map_size(get_linked_pool(true)), OP_EQ, 0);

  /* Reap everything. Pre-fix, client2 and the recovery leg both point at the
   * same conflux object and both free it -> use-after-free / double free.
   * Post-fix, only the recovery leg owns it and frees it exactly once. */
  test_clear_circs();

  tt_int_op(digest256map_size(get_linked_pool(true)), OP_EQ, 0);
  tt_int_op(digest256map_size(get_unlinked_pool(true)), OP_EQ, 0);

 done:
  test_teardown();
}

struct testcase_t conflux_pool_tests[] = {
  { "link", test_conflux_link, TT_FORK, NULL, NULL },
  { "link_retry", test_conflux_link_retry, TT_FORK, NULL, NULL },
  { "link_relink", test_conflux_link_relink, TT_FORK, NULL, NULL },
  { "link_streams", test_conflux_link_streams, TT_FORK, NULL, NULL },
  { "switch", test_conflux_switch, TT_FORK, NULL, NULL },
  { "begindir_rejects_pending_unlinked_with_end",
    test_conflux_begindir_rejects_pending_unlinked_with_end,
    TT_FORK, NULL, NULL },
  { "begindir_rejects_linked_with_end",
    test_conflux_begindir_rejects_linked_with_end,
    TT_FORK, NULL, NULL },
  { "begindir_rejects_one_leg_with_end",
    test_conflux_begindir_rejects_one_leg_with_end,
    TT_FORK, NULL, NULL },
  { "circuit_get_best_rejects_internal",
    test_conflux_circuit_get_best_rejects_internal,
    TT_FORK, NULL, NULL },
  { "ooo_q_teardown_accounting", test_conflux_ooo_q_teardown_accounting,
    TT_FORK, NULL, NULL },
  { "recovery_leg", test_conflux_recovery_leg,
    TT_FORK, NULL, NULL },
  // XXX: These two currently fail, because they are not finished:
  //{ "link_fail", test_conflux_linkconflux_process_relay_msg_fail,
  //  TT_FORK, NULL, NULL },
  //{ "close", test_conflux_close, TT_FORK, NULL, NULL },
  END_OF_TESTCASES
};
