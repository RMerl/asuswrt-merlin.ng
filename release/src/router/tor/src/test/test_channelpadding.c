/* Copyright (c) 2016-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

#define CHANNEL_OBJECT_PRIVATE
#define MAINLOOP_PRIVATE
#define NETWORKSTATUS_PRIVATE
#define TOR_TIMERS_PRIVATE
#include "core/or/or.h"
#include "test/test.h"
#include "lib/testsupport/testsupport.h"
#include "core/mainloop/connection.h"
#include "core/or/connection_or.h"
#include "core/or/channel.h"
#include "core/or/channeltls.h"
#include "core/or/channelpadding.h"
#include "lib/evloop/compat_libevent.h"
#include "app/config/config.h"
#include "lib/time/compat_time.h"
#include "core/mainloop/mainloop.h"
#include "feature/nodelist/networkstatus.h"
#include "test/log_test_helpers.h"
#include "lib/tls/tortls.h"
#include "lib/evloop/timers.h"
#include "lib/buf/buffers.h"

#include "core/or/cell_st.h"
#include "feature/nodelist/networkstatus_st.h"
#include "core/or/or_connection_st.h"
#include "feature/nodelist/routerstatus_st.h"

int channelpadding_get_netflow_inactive_timeout_ms(channel_t *chan);
int64_t channelpadding_compute_time_until_pad_for_netflow(channel_t *chan);
int channelpadding_send_disable_command(channel_t*);
int channelpadding_find_timerslot(channel_t *chan);

void test_channelpadding_timers(void *arg);
void test_channelpadding_consensus(void *arg);
void test_channelpadding_negotiation(void *arg);
void test_channelpadding_decide_to_pad_channel(void *arg);
void test_channelpadding_killonehop(void *arg);

void dummy_nop_timer(void);

#define NSEC_PER_MSEC (1000*1000)

/* Thing to cast to fake tor_tls_t * to appease assert_connection_ok() */
static int fake_tortls = 0; /* Bleh... */

static int dont_stop_libevent = 0;

// From test_channel.c
channel_t * new_fake_channel(void);
void free_fake_channel(channel_t*);

static int
mock_channel_has_queued_writes(channel_t *chan)
{
  (void)chan;
  return 0;
}

static int tried_to_write_cell = 0;

static channel_t *relay1_relay2;
static channel_t *relay2_relay1;
static channel_t *relay3_client;
static channel_t *client_relay3;

static int
mock_channel_write_cell_relay2(channel_t *chan, cell_t *cell)
{
  (void)chan;
  tried_to_write_cell++;
  channel_tls_handle_cell(cell, ((channel_tls_t*)relay1_relay2)->conn);
  tor_libevent_exit_loop_after_callback(tor_libevent_get_base());
  return 0;
}

static int
mock_channel_write_cell_relay1(channel_t *chan, cell_t *cell)
{
  (void)chan;
  tried_to_write_cell++;
  channel_tls_handle_cell(cell, ((channel_tls_t*)relay2_relay1)->conn);
  tor_libevent_exit_loop_after_callback(tor_libevent_get_base());
  return 0;
}

static int
mock_channel_write_cell_relay3(channel_t *chan, cell_t *cell)
{
  (void)chan;
  tried_to_write_cell++;
  channel_tls_handle_cell(cell, ((channel_tls_t*)client_relay3)->conn);
  tor_libevent_exit_loop_after_callback(tor_libevent_get_base());
  return 0;
}

static int
mock_channel_write_cell_client(channel_t *chan, cell_t *cell)
{
  (void)chan;
  tried_to_write_cell++;
  channel_tls_handle_cell(cell, ((channel_tls_t*)relay3_client)->conn);
  tor_libevent_exit_loop_after_callback(tor_libevent_get_base());
  return 0;
}

static int
mock_channel_write_cell(channel_t *chan, cell_t *cell)
{
  tried_to_write_cell++;
  channel_tls_handle_cell(cell, ((channel_tls_t*)chan)->conn);
  if (!dont_stop_libevent)
    tor_libevent_exit_loop_after_callback(tor_libevent_get_base());
  return 0;
}

static void
setup_fake_connection_for_channel(channel_tls_t *chan)
{
  or_connection_t *conn = (or_connection_t*)connection_new(CONN_TYPE_OR,
                                                           AF_INET);

  conn->base_.conn_array_index = smartlist_len(connection_array);
  smartlist_add(connection_array, conn);

  conn->chan = chan;
  chan->conn = conn;

  conn->base_.magic = OR_CONNECTION_MAGIC;
  conn->base_.state = OR_CONN_STATE_OPEN;
  conn->base_.type = CONN_TYPE_OR;
  conn->base_.socket_family = AF_INET;
  conn->base_.address = tor_strdup("<fake>");

  conn->base_.port = 4242;

  conn->tls = (tor_tls_t *)((void *)(&fake_tortls));

  conn->link_proto = MIN_LINK_PROTO_FOR_CHANNEL_PADDING;

  connection_or_set_canonical(conn, 1);
}

static channel_tls_t *
new_fake_channeltls(uint8_t id)
{
  channel_tls_t *chan = tor_realloc(new_fake_channel(), sizeof(channel_tls_t));
  chan->base_.magic = TLS_CHAN_MAGIC;
  setup_fake_connection_for_channel(chan);
  chan->base_.channel_usage = CHANNEL_USED_FOR_FULL_CIRCS;
  chan->base_.has_queued_writes = mock_channel_has_queued_writes;
  chan->base_.write_cell = mock_channel_write_cell;
  chan->base_.padding_enabled = 1;

  chan->base_.identity_digest[0] = id;
  channel_register(&chan->base_);

  return chan;
}

static void
free_fake_channeltls(channel_tls_t *chan)
{
  channel_unregister(&chan->base_);

  tor_free(((channel_tls_t*)chan)->conn->base_.address);
  buf_free(((channel_tls_t*)chan)->conn->base_.inbuf);
  buf_free(((channel_tls_t*)chan)->conn->base_.outbuf);
  tor_free(((channel_tls_t*)chan)->conn);

  timer_free(chan->base_.padding_timer);
  channel_handle_free(chan->base_.timer_handle);
  channel_handles_clear(&chan->base_);

  free_fake_channel(&chan->base_);

  return;
}

static void
setup_mock_consensus(void)
{
  current_md_consensus = current_ns_consensus
        = tor_malloc_zero(sizeof(networkstatus_t));
  current_md_consensus->net_params = smartlist_new();
  current_md_consensus->routerstatus_list = smartlist_new();
  channelpadding_new_consensus_params(current_md_consensus);
}

static void
free_mock_consensus(void)
{
  SMARTLIST_FOREACH(current_md_consensus->routerstatus_list, void *, r,
                    tor_free(r));
  smartlist_free(current_md_consensus->routerstatus_list);
  smartlist_free(current_ns_consensus->net_params);
  tor_free(current_ns_consensus);
}

static void
setup_mock_network(void)
{
  routerstatus_t *relay;
  if (!connection_array)
    connection_array = smartlist_new();

  relay1_relay2 = (channel_t*)new_fake_channeltls(2);
  relay1_relay2->write_cell = mock_channel_write_cell_relay1;
  channel_timestamp_active(relay1_relay2);
  relay = tor_malloc_zero(sizeof(routerstatus_t));
  relay->identity_digest[0] = 1;
  smartlist_add(current_md_consensus->routerstatus_list, relay);

  relay2_relay1 = (channel_t*)new_fake_channeltls(1);
  relay2_relay1->write_cell = mock_channel_write_cell_relay2;
  channel_timestamp_active(relay2_relay1);
  relay = tor_malloc_zero(sizeof(routerstatus_t));
  relay->identity_digest[0] = 2;
  smartlist_add(current_md_consensus->routerstatus_list, relay);

  relay3_client = (channel_t*)new_fake_channeltls(0);
  relay3_client->write_cell = mock_channel_write_cell_relay3;
  relay3_client->is_client = 1;
  channel_timestamp_active(relay3_client);
  relay = tor_malloc_zero(sizeof(routerstatus_t));
  relay->identity_digest[0] = 3;
  smartlist_add(current_md_consensus->routerstatus_list, relay);

  client_relay3 = (channel_t*)new_fake_channeltls(3);
  client_relay3->write_cell = mock_channel_write_cell_client;
  channel_timestamp_active(client_relay3);

  channel_do_open_actions(relay1_relay2);
  channel_do_open_actions(relay2_relay1);
  channel_do_open_actions(relay3_client);
  channel_do_open_actions(client_relay3);
}

static void
free_mock_network(void)
{
  free_fake_channeltls((channel_tls_t*)relay1_relay2);
  free_fake_channeltls((channel_tls_t*)relay2_relay1);
  free_fake_channeltls((channel_tls_t*)relay3_client);
  free_fake_channeltls((channel_tls_t*)client_relay3);

  smartlist_free(connection_array);
}

static void
dummy_timer_cb(tor_timer_t *t, void *arg, const monotime_t *now_mono)
{
  (void)t; (void)arg; (void)now_mono;
  tor_libevent_exit_loop_after_callback(tor_libevent_get_base());
  return;
}

// This hack adds a dummy timer so that the libevent base loop
// actually returns when we don't expect any timers to fire. Otherwise,
// the global_timer_event gets scheduled an hour from now, and the
// base loop never returns.
void
dummy_nop_timer(void)
{
  tor_timer_t *dummy_timer = timer_new(dummy_timer_cb, NULL);
  struct timeval timeout;
  timeout.tv_sec = 1;
  timeout.tv_usec = 0;

  timer_schedule(dummy_timer, &timeout);

  tor_libevent_run_event_loop(tor_libevent_get_base(), 0);

  timer_free(dummy_timer);
}

#define CHANNELPADDING_MAX_TIMERS 25
#define CHANNELS_TO_TEST (CHANNELPADDING_MAX_TIMERS*4)
/**
 * Tests to ensure that we handle more than the max number of pending
 * timers properly.
 */
void
test_channelpadding_timers(void *arg)
{
  channelpadding_decision_t decision;
  channel_t *chans[CHANNELS_TO_TEST];
  (void)arg;

  if (!connection_array)
    connection_array = smartlist_new();

  monotime_init();
  monotime_enable_test_mocking();
  uint64_t nsec_mock = 1;
  monotime_set_mock_time_nsec(nsec_mock);
  monotime_coarse_set_mock_time_nsec(nsec_mock);

  timers_initialize();
  channelpadding_new_consensus_params(NULL);

  for (int i = 0; i < CHANNELS_TO_TEST; i++) {
    chans[i] = (channel_t*)new_fake_channeltls(0);
    channel_timestamp_active(chans[i]);
  }

  for (int j = 0; j < 2; j++) {
    tried_to_write_cell = 0;
    int i = 0;

    monotime_coarse_t now;
    monotime_coarse_get(&now);

    /* This loop fills our timerslot array with timers of increasing time
     * until they fire */
    for (; i < CHANNELPADDING_MAX_TIMERS; i++) {
      monotime_coarse_add_msec(&chans[i]->next_padding_time,
                        &now, 10 + i*4);
      decision = channelpadding_decide_to_pad_channel(chans[i]);
      tt_int_op(decision, OP_EQ, CHANNELPADDING_PADDING_SCHEDULED);
      tt_assert(chans[i]->pending_padding_callback);
      tt_int_op(tried_to_write_cell, OP_EQ, 0);
    }

    /* This loop should add timers to the first position in the timerslot
     * array, since its timeout is before all other timers. */
    for (; i < CHANNELS_TO_TEST/3; i++) {
      monotime_coarse_add_msec(&chans[i]->next_padding_time,
                        &now, 1);
      decision = channelpadding_decide_to_pad_channel(chans[i]);
      tt_int_op(decision, OP_EQ, CHANNELPADDING_PADDING_SCHEDULED);
      tt_assert(chans[i]->pending_padding_callback);
      tt_int_op(tried_to_write_cell, OP_EQ, 0);
    }

    /* This loop should add timers to our existing lists in a weak
     * pseudorandom pattern.  It ensures that the lists can grow with multiple
     * timers in them. */
    for (; i < CHANNELS_TO_TEST/2; i++) {
      monotime_coarse_add_msec(&chans[i]->next_padding_time,
                        &now, 10 + i*3 % CHANNELPADDING_MAX_TIMERS);
      decision = channelpadding_decide_to_pad_channel(chans[i]);
      tt_int_op(decision, OP_EQ, CHANNELPADDING_PADDING_SCHEDULED);
      tt_assert(chans[i]->pending_padding_callback);
      tt_int_op(tried_to_write_cell, OP_EQ, 0);
    }

    /* This loop should add timers to the last position in the timerslot
     * array, since its timeout is after all other timers. */
    for (; i < CHANNELS_TO_TEST; i++) {
      monotime_coarse_add_msec(&chans[i]->next_padding_time,
                               &now, 500 + i % CHANNELPADDING_MAX_TIMERS);
      decision = channelpadding_decide_to_pad_channel(chans[i]);
      tt_int_op(decision, OP_EQ, CHANNELPADDING_PADDING_SCHEDULED);
      tt_assert(chans[i]->pending_padding_callback);
      tt_int_op(tried_to_write_cell, OP_EQ, 0);
    }

    // Wait for the timers and then kill the event loop.
    nsec_mock += 1001 * NSEC_PER_MSEC;
    monotime_coarse_set_mock_time_nsec(nsec_mock);
    monotime_set_mock_time_nsec(nsec_mock);
    timers_run_pending();

    tt_int_op(tried_to_write_cell, OP_EQ, CHANNELS_TO_TEST);

    // Test that we have no pending callbacks and all empty slots now
    for (i = 0; i < CHANNELS_TO_TEST; i++) {
      tt_assert(!chans[i]->pending_padding_callback);
    }
  }

 done:
  for (int i = 0; i < CHANNELS_TO_TEST; i++) {
    free_fake_channeltls((channel_tls_t*)chans[i]);
  }
  smartlist_free(connection_array);

  timers_shutdown();
  monotime_disable_test_mocking();
  channel_free_all();

  return;
}

void
test_channelpadding_killonehop(void *arg)
{
  channelpadding_decision_t decision;
  int64_t new_time;
  (void)arg;

  routerstatus_t *relay = tor_malloc_zero(sizeof(routerstatus_t));
  monotime_init();
  monotime_enable_test_mocking();
  monotime_set_mock_time_nsec(1);
  monotime_coarse_set_mock_time_nsec(1);
  new_time = 1;

  timers_initialize();
  setup_mock_consensus();
  setup_mock_network();

  /* Do we disable padding if rsos is enabled, and the consensus says don't
   * pad?  */

  monotime_coarse_t now;
  monotime_coarse_get(&now);

  // First, test that padding works if either is enabled
  smartlist_clear(current_md_consensus->net_params);
  channelpadding_new_consensus_params(current_md_consensus);

  relay3_client->padding_enabled = 1;
  client_relay3->padding_enabled = 1;

  tried_to_write_cell = 0;
  get_options_mutable()->ORPort_set = 0;
  get_options_mutable()->HiddenServiceSingleHopMode = 1;
  get_options_mutable()->HiddenServiceNonAnonymousMode = 1;

  monotime_coarse_add_msec(&client_relay3->next_padding_time, &now, 100);
  decision = channelpadding_decide_to_pad_channel(client_relay3);
  tt_int_op(decision, OP_EQ, CHANNELPADDING_PADDING_SCHEDULED);
  tt_assert(client_relay3->pending_padding_callback);
  tt_int_op(tried_to_write_cell, OP_EQ, 0);

  decision = channelpadding_decide_to_pad_channel(client_relay3);
  tt_int_op(decision, OP_EQ, CHANNELPADDING_PADDING_ALREADY_SCHEDULED);

  // Wait for the timer
  new_time += 101 * NSEC_PER_MSEC;
  monotime_coarse_set_mock_time_nsec(new_time);
  monotime_set_mock_time_nsec(new_time);
  monotime_coarse_get(&now);
  timers_run_pending();
  tt_int_op(tried_to_write_cell, OP_EQ, 1);
  tt_assert(!client_relay3->pending_padding_callback);

  // Then test disabling each via consensus param
  smartlist_add(current_md_consensus->net_params,
                (void*)"nf_pad_single_onion=0");
  channelpadding_new_consensus_params(current_md_consensus);

  // Before the client tries to pad, the relay will still pad:
  tried_to_write_cell = 0;
  monotime_coarse_add_msec(&relay3_client->next_padding_time, &now, 100);
  get_options_mutable()->ORPort_set = 1;
  get_options_mutable()->HiddenServiceSingleHopMode = 0;
  get_options_mutable()->HiddenServiceNonAnonymousMode = 0;
  decision = channelpadding_decide_to_pad_channel(relay3_client);
  tt_int_op(decision, OP_EQ, CHANNELPADDING_PADDING_SCHEDULED);
  tt_assert(relay3_client->pending_padding_callback);

  // Wait for the timer
  new_time += 101 * NSEC_PER_MSEC;
  monotime_coarse_set_mock_time_nsec(new_time);
  monotime_set_mock_time_nsec(new_time);
  monotime_coarse_get(&now);
  timers_run_pending();
  tt_int_op(tried_to_write_cell, OP_EQ, 1);
  tt_assert(!client_relay3->pending_padding_callback);

  // Test client side (it should stop immediately)
  get_options_mutable()->HiddenServiceSingleHopMode = 1;
  get_options_mutable()->HiddenServiceNonAnonymousMode = 1;
  /* For the relay to receive the negotiate: */
  get_options_mutable()->ORPort_set = 1;
  decision = channelpadding_decide_to_pad_channel(client_relay3);
  tt_int_op(decision, OP_EQ, CHANNELPADDING_WONTPAD);
  tt_assert(!client_relay3->pending_padding_callback);

  // Test relay side (it should have gotten the negotiation to disable)
  get_options_mutable()->ORPort_set = 1;
  get_options_mutable()->HiddenServiceSingleHopMode = 0;
  get_options_mutable()->HiddenServiceNonAnonymousMode = 0;
  tt_int_op(channelpadding_decide_to_pad_channel(relay3_client), OP_EQ,
      CHANNELPADDING_WONTPAD);
  tt_assert(!relay3_client->padding_enabled);

 done:
  free_mock_consensus();
  free_mock_network();
  tor_free(relay);

  timers_shutdown();
  monotime_disable_test_mocking();
  channel_free_all();
}

void
test_channelpadding_consensus(void *arg)
{
  channelpadding_decision_t decision;
  or_options_t *options = get_options_mutable();
  int64_t val;
  int64_t new_time;
  (void)arg;

  /*
   * Params tested:
   *   nf_pad_before_usage
   *   nf_pad_relays
   *   nf_ito_low
   *   nf_ito_high
   *
   * Plan:
   * 1. Padding can be completely disabled via consensus
   * 2. Negotiation can't re-enable consensus-disabled padding
   * 3. Negotiation can't increase padding from relays beyond
   *    consensus defaults
   * 4. Relay-to-relay padding can be enabled/disabled in consensus
   * 5. Can enable/disable padding before actually using a connection
   * 6. Can we control circ and TLS conn lifetime from the consensus?
   */
  channel_t *chan;
  routerstatus_t *relay = tor_malloc_zero(sizeof(routerstatus_t));
  monotime_enable_test_mocking();
  monotime_set_mock_time_nsec(1);
  monotime_coarse_set_mock_time_nsec(1);
  new_time = 1;
  monotime_coarse_t now;
  monotime_coarse_get(&now);
  timers_initialize();

  if (!connection_array)
    connection_array = smartlist_new();
  chan = (channel_t*)new_fake_channeltls(0);
  channel_timestamp_active(chan);

  setup_mock_consensus();

  get_options_mutable()->ORPort_set = 1;

  /* Test 1: Padding can be completely disabled via consensus */
  tried_to_write_cell = 0;
  monotime_coarse_add_msec(&chan->next_padding_time, &now, 100);
  decision = channelpadding_decide_to_pad_channel(chan);
  tt_int_op(decision, OP_EQ, CHANNELPADDING_PADDING_SCHEDULED);
  tt_assert(chan->pending_padding_callback);
  tt_int_op(tried_to_write_cell, OP_EQ, 0);

  decision = channelpadding_decide_to_pad_channel(chan);
  tt_int_op(decision, OP_EQ, CHANNELPADDING_PADDING_ALREADY_SCHEDULED);

  // Wait for the timer
  new_time += 101*NSEC_PER_MSEC;
  monotime_coarse_set_mock_time_nsec(new_time);
  monotime_set_mock_time_nsec(new_time);
  monotime_coarse_get(&now);
  timers_run_pending();
  tt_int_op(tried_to_write_cell, OP_EQ, 1);
  tt_assert(!chan->pending_padding_callback);

  smartlist_add(current_md_consensus->net_params,
                (void*)"nf_ito_low=0");
  smartlist_add(current_md_consensus->net_params,
                (void*)"nf_ito_high=0");
  get_options_mutable()->ConnectionPadding = 1;
  channelpadding_new_consensus_params(current_md_consensus);

  decision = channelpadding_decide_to_pad_channel(chan);
  tt_int_op(decision, OP_EQ, CHANNELPADDING_WONTPAD);
  tt_assert(!chan->pending_padding_callback);
  val = channelpadding_get_netflow_inactive_timeout_ms(chan);
  tt_i64_op(val, OP_EQ, 0);
  val = channelpadding_compute_time_until_pad_for_netflow(chan);
  tt_i64_op(val, OP_EQ, -2);

  /* Test 2: Negotiation can't re-enable consensus-disabled padding */
  channelpadding_send_enable_command(chan, 100, 200);
  tried_to_write_cell = 0;
  decision = channelpadding_decide_to_pad_channel(chan);
  tt_int_op(decision, OP_EQ, CHANNELPADDING_WONTPAD);
  tt_assert(!chan->pending_padding_callback);
  val = channelpadding_get_netflow_inactive_timeout_ms(chan);
  tt_i64_op(val, OP_EQ, 0);
  val = channelpadding_compute_time_until_pad_for_netflow(chan);
  tt_i64_op(val, OP_EQ, -2);
  tt_assert(monotime_coarse_is_zero(&chan->next_padding_time));

  smartlist_clear(current_md_consensus->net_params);

  /* Test 3: Negotiation can't increase padding from relays beyond consensus
   * values */
  smartlist_add(current_md_consensus->net_params,
                (void*)"nf_ito_low=100");
  smartlist_add(current_md_consensus->net_params,
                (void*)"nf_ito_high=200");
  channelpadding_new_consensus_params(current_md_consensus);

  tried_to_write_cell = 0;
  monotime_coarse_add_msec(&chan->next_padding_time, &now, 100);
  decision = channelpadding_decide_to_pad_channel(chan);
  tt_int_op(decision, OP_EQ, CHANNELPADDING_PADDING_SCHEDULED);
  tt_assert(chan->pending_padding_callback);
  tt_int_op(tried_to_write_cell, OP_EQ, 0);
  val = channelpadding_get_netflow_inactive_timeout_ms(chan);
  tt_i64_op(val, OP_GE, 100);
  tt_i64_op(val, OP_LE, 200);
  val = channelpadding_compute_time_until_pad_for_netflow(chan);
  tt_i64_op(val, OP_LE, 200);

  // Wait for the timer
  new_time += 201*NSEC_PER_MSEC;
  monotime_set_mock_time_nsec(new_time);
  monotime_coarse_set_mock_time_nsec(new_time);
  monotime_coarse_get(&now);
  timers_run_pending();
  tt_int_op(tried_to_write_cell, OP_EQ, 1);
  tt_assert(!chan->pending_padding_callback);

  smartlist_clear(current_md_consensus->net_params);
  smartlist_add(current_md_consensus->net_params,
                (void*)"nf_ito_low=1500");
  smartlist_add(current_md_consensus->net_params,
                (void*)"nf_ito_high=4500");
  channelpadding_new_consensus_params(current_md_consensus);

  channelpadding_send_enable_command(chan, 100, 200);
  tried_to_write_cell = 0;
  decision = channelpadding_decide_to_pad_channel(chan);
  tt_int_op(decision, OP_EQ, CHANNELPADDING_PADLATER);
  tt_assert(!chan->pending_padding_callback);
  val = channelpadding_get_netflow_inactive_timeout_ms(chan);
  tt_i64_op(val, OP_GE, 1500);
  tt_i64_op(val, OP_LE, 4500);
  val = channelpadding_compute_time_until_pad_for_netflow(chan);
  tt_i64_op(val, OP_LE, 4500);

  /* Test 4: Relay-to-relay padding can be enabled/disabled in consensus */
  /* Make this channel a relay's channel */
  memcpy(relay->identity_digest,
          ((channel_tls_t *)chan)->conn->identity_digest, DIGEST_LEN);
  smartlist_add(current_md_consensus->routerstatus_list, relay);
  relay = NULL; /* Prevent double-free */

  tried_to_write_cell = 0;
  decision = channelpadding_decide_to_pad_channel(chan);
  tt_int_op(decision, OP_EQ, CHANNELPADDING_WONTPAD);
  tt_assert(!chan->pending_padding_callback);

  smartlist_add(current_md_consensus->net_params,
                (void*)"nf_pad_relays=1");
  channelpadding_new_consensus_params(current_md_consensus);

  decision = channelpadding_decide_to_pad_channel(chan);
  tt_int_op(decision, OP_EQ, CHANNELPADDING_PADLATER);
  tt_assert(!chan->pending_padding_callback);
  val = channelpadding_get_netflow_inactive_timeout_ms(chan);
  tt_i64_op(val, OP_GE, 1500);
  tt_i64_op(val, OP_LE, 4500);
  val = channelpadding_compute_time_until_pad_for_netflow(chan);
  tt_i64_op(val, OP_LE, 4500);

  /* Test 5: If we disable padding before channel usage, does that work? */
  smartlist_add(current_md_consensus->net_params,
                (void*)"nf_pad_before_usage=0");
  channelpadding_new_consensus_params(current_md_consensus);
  tried_to_write_cell = 0;
  decision = channelpadding_decide_to_pad_channel(chan);
  tt_int_op(decision, OP_EQ, CHANNELPADDING_WONTPAD);
  tt_assert(!chan->pending_padding_callback);

  /* Test 6: Can we control circ and TLS conn lifetime from the consensus? */
  val = channelpadding_get_channel_idle_timeout(NULL, 0);
  tt_i64_op(val, OP_GE, 180);
  tt_i64_op(val, OP_LE, 180+90);
  val = channelpadding_get_channel_idle_timeout(chan, 0);
  tt_i64_op(val, OP_GE, 180);
  tt_i64_op(val, OP_LE, 180+90);
  options->ReducedConnectionPadding = 1;
  val = channelpadding_get_channel_idle_timeout(chan, 0);
  tt_i64_op(val, OP_GE, 180/2);
  tt_i64_op(val, OP_LE, (180+90)/2);

  options->ReducedConnectionPadding = 0;
  options->ORPort_set = 1;
  smartlist_add(current_md_consensus->net_params,
                (void*)"nf_conntimeout_relays=600");
  channelpadding_new_consensus_params(current_md_consensus);
  val = channelpadding_get_channel_idle_timeout(chan, 1);
  tt_i64_op(val, OP_GE, 450);
  tt_i64_op(val, OP_LE, 750);

  val = channelpadding_get_circuits_available_timeout();
  tt_i64_op(val, OP_GE, 30*60);
  tt_i64_op(val, OP_LE, 30*60*2);

  options->ReducedConnectionPadding = 1;
  smartlist_add(current_md_consensus->net_params,
                (void*)"nf_conntimeout_clients=600");
  channelpadding_new_consensus_params(current_md_consensus);
  val = channelpadding_get_circuits_available_timeout();
  tt_i64_op(val, OP_GE, 600/2);
  tt_i64_op(val, OP_LE, 600*2/2);

  options->ReducedConnectionPadding = 0;
  options->CircuitsAvailableTimeout = 24*60*60;
  val = channelpadding_get_circuits_available_timeout();
  tt_i64_op(val, OP_GE, 24*60*60);
  tt_i64_op(val, OP_LE, 24*60*60*2);

 done:
  tor_free(relay);

  free_mock_consensus();
  free_fake_channeltls((channel_tls_t*)chan);
  smartlist_free(connection_array);

  timers_shutdown();
  monotime_disable_test_mocking();
  channel_free_all();

  return;
}

void
test_channelpadding_negotiation(void *arg)
{
  channelpadding_negotiate_t disable;
  cell_t cell;
  channelpadding_decision_t decision;
  int val;
  (void)arg;

  /* Plan:
   * 1. Clients reject negotiation, relays accept it.
   *    * Bridges accept negotiation from their clients,
   *      but not from relays.
   * 2. Torrc options can override client-side negotiation
   * 3. Test a version issue in channelpadidng cell
   * 4. Test channelpadding_reduced_padding
   */
  monotime_init();
  monotime_enable_test_mocking();
  monotime_set_mock_time_nsec(1);
  monotime_coarse_set_mock_time_nsec(1);
  timers_initialize();
  setup_mock_consensus();
  setup_mock_network();

  /* Test case #1: Do the right things ignore negotiation? */
  /* relay-to-client case: */
  channelpadding_send_disable_command(relay3_client);
  tt_assert(client_relay3->padding_enabled);

  /* client-to-relay case: */
  get_options_mutable()->ORPort_set = 1;
  channelpadding_disable_padding_on_channel(client_relay3);
  tt_int_op(channelpadding_decide_to_pad_channel(relay3_client), OP_EQ,
      CHANNELPADDING_WONTPAD);
  tt_assert(!relay3_client->padding_enabled);
  relay3_client->padding_enabled = 1;
  client_relay3->padding_enabled = 1;

  /* Bridge case from relay */
  get_options_mutable()->BridgeRelay = 1;
  channelpadding_disable_padding_on_channel(relay2_relay1);
  tt_assert(relay1_relay2->padding_enabled);

  /* Bridge case from client */
  channelpadding_disable_padding_on_channel(client_relay3);
  tt_assert(!relay3_client->padding_enabled);
  tt_int_op(channelpadding_decide_to_pad_channel(relay3_client), OP_EQ,
      CHANNELPADDING_WONTPAD);
  relay3_client->padding_enabled = 1;
  client_relay3->padding_enabled = 1;
  get_options_mutable()->BridgeRelay = 0;
  get_options_mutable()->ORPort_set = 0;

  /* Test case #2: Torrc options */
  /* ConnectionPadding auto; Relay doesn't support us */
  ((channel_tls_t*)relay3_client)->conn->link_proto = 4;
  relay3_client->padding_enabled = 0;
  tried_to_write_cell = 0;
  decision = channelpadding_decide_to_pad_channel(relay3_client);
  tt_int_op(decision, OP_EQ, CHANNELPADDING_WONTPAD);
  tt_assert(!relay3_client->pending_padding_callback);
  tt_int_op(tried_to_write_cell, OP_EQ, 0);
  ((channel_tls_t*)relay3_client)->conn->link_proto = 5;
  relay3_client->padding_enabled = 1;

  /* ConnectionPadding 1; Relay doesn't support us */
  get_options_mutable()->ConnectionPadding = 1;
  tried_to_write_cell = 0;
  decision = channelpadding_decide_to_pad_channel(client_relay3);
  tt_int_op(decision, OP_EQ, CHANNELPADDING_PADLATER);
  tt_assert(!client_relay3->pending_padding_callback);
  tt_int_op(tried_to_write_cell, OP_EQ, 0);
  get_options_mutable()->ConnectionPadding = 0;

  /* Test case #3: Test a version issue in channelpadding cell */
  get_options_mutable()->ORPort_set = 1;
  client_relay3->padding_enabled = 1;
  relay3_client->padding_enabled = 1;
  memset(&cell, 0, sizeof(cell_t));
  memset(&disable, 0, sizeof(channelpadding_negotiate_t));
  cell.command = CELL_PADDING_NEGOTIATE;

  channelpadding_negotiate_set_command(&disable, CHANNELPADDING_COMMAND_STOP);
  disable.version = 1;
  channelpadding_negotiate_encode(cell.payload, CELL_PAYLOAD_SIZE, &disable);
  client_relay3->write_cell(client_relay3, &cell);
  tt_assert(relay3_client->padding_enabled);
  tt_int_op(channelpadding_update_padding_for_channel(client_relay3, &disable),
          OP_EQ, -1);
  tt_assert(client_relay3->padding_enabled);

  disable.version = 0;
  channelpadding_negotiate_encode(cell.payload, CELL_PAYLOAD_SIZE, &disable);
  client_relay3->write_cell(client_relay3, &cell);
  tt_assert(!relay3_client->padding_enabled);

  /* Test case 4: Reducing padding actually reduces it */
  relay3_client->padding_enabled = 1;
  client_relay3->padding_enabled = 1;

  decision = channelpadding_decide_to_pad_channel(relay3_client);
  tt_int_op(decision, OP_EQ, CHANNELPADDING_PADLATER);

  channelpadding_reduce_padding_on_channel(client_relay3);

  tried_to_write_cell = 0;
  decision = channelpadding_decide_to_pad_channel(relay3_client);
  tt_int_op(decision, OP_EQ, CHANNELPADDING_WONTPAD);

  get_options_mutable()->ORPort_set = 0;
  decision = channelpadding_decide_to_pad_channel(client_relay3);
  tt_int_op(decision, OP_EQ, CHANNELPADDING_PADLATER);

  tt_assert(!client_relay3->pending_padding_callback);
  val = channelpadding_get_netflow_inactive_timeout_ms(client_relay3);
  tt_int_op(val, OP_GE, 9000);
  tt_int_op(val, OP_LE, 14000);
  int64_t val64 =
    channelpadding_compute_time_until_pad_for_netflow(client_relay3);
  tt_i64_op(val64, OP_LE, 14000);

 done:
  free_mock_network();
  free_mock_consensus();

  timers_shutdown();
  monotime_disable_test_mocking();
  channel_free_all();

  return;
}

void
test_channelpadding_decide_to_pad_channel(void *arg)
{
  channelpadding_decision_t decision;
  /**
   * Test case plan:
   *
   * 1. Channel that has "sent a packet" before the timeout.
   *    + We should decide to pad later
   * 2. Channel that has not "sent a packet" before the timeout:
   * 2a. Not within 1.1s of the timeout.
   *    + We should decide to pad later
   * 2b. Within 1.1s of the timeout.
   *    + We should schedule padding
   *    + We should get feedback that we wrote a cell
   * 2c. Within 0.1s of the timeout.
   *    + We should schedule padding
   *    + We should get feedback that we wrote a cell
   * 2d. Channel that asks to pad while timeout is scheduled
   *    + We should schedule padding
   *    + We should get feedback that we wrote a cell
   * 2e. 0s of the timeout
   *    + We should send padding immediately
   *    + We should get feedback that we wrote a cell
   * 2f. <0s of the timeout
   *    + We should send padding immediately
   *    + We should get feedback that we wrote a cell
   * 3. Channel that sends a packet while timeout is scheduled
   *    + We should not get feedback that we wrote a cell
   * 4. Channel that closes while timeout is scheduled
   *    + We should not get feedback that we wrote a cell
   * 5. Make sure the channel still would work if repaired
   *    + We should be able to schedule padding and resend
   * 6. Channel is not used for full circuits
   * 7. Channel that disappears while timeout is scheduled
   *    + We should not send padding
   */
  channel_t *chan;
  int64_t new_time;
  if (!connection_array)
    connection_array = smartlist_new();
  (void)arg;

  monotime_init();
  monotime_enable_test_mocking();
  monotime_set_mock_time_nsec(1);
  monotime_coarse_set_mock_time_nsec(1);
  new_time = 1;
  monotime_coarse_t now;
  monotime_coarse_get(&now);
  timers_initialize();
  setup_full_capture_of_logs(LOG_WARN);
  channelpadding_new_consensus_params(NULL);

  chan = (channel_t*)new_fake_channeltls(0);
  channel_timestamp_active(chan);

  /* Test case #1: Channel that has "sent a packet" before the timeout. */
  tried_to_write_cell = 0;
  decision = channelpadding_decide_to_pad_channel(chan);
  tt_int_op(decision, OP_EQ, CHANNELPADDING_PADLATER);
  tt_assert(!chan->pending_padding_callback);
  tt_int_op(tried_to_write_cell, OP_EQ, 0);

  /* Test case #2a: > 1.1s until timeout */
  tried_to_write_cell = 0;
  monotime_coarse_add_msec(&chan->next_padding_time, &now, 1200);
  decision = channelpadding_decide_to_pad_channel(chan);
  tt_int_op(decision, OP_EQ, CHANNELPADDING_PADLATER);
  tt_assert(!chan->pending_padding_callback);
  tt_int_op(tried_to_write_cell, OP_EQ, 0);

  /* Test case #2b: >= 1.0s until timeout */
  tried_to_write_cell = 0;
  monotime_coarse_add_msec(&chan->next_padding_time, &now, 1000);
  decision = channelpadding_decide_to_pad_channel(chan);
  tt_int_op(decision, OP_EQ, CHANNELPADDING_PADDING_SCHEDULED);
  tt_assert(chan->pending_padding_callback);
  tt_int_op(tried_to_write_cell, OP_EQ, 0);

  // Set up a timer for the <0 case below.
  monotime_coarse_t now_minus_100s;
  monotime_coarse_add_msec(&now_minus_100s, &now, 900);
  // Wait for the timer from case #2b
  new_time += 1000*NSEC_PER_MSEC;
  monotime_set_mock_time_nsec(new_time);
  monotime_coarse_set_mock_time_nsec(new_time);
  monotime_coarse_get(&now);
  timers_run_pending();
  tt_int_op(tried_to_write_cell, OP_EQ, 1);
  tt_assert(!chan->pending_padding_callback);

  /* Test case #2c: > 0.1s until timeout */
  tried_to_write_cell = 0;
  monotime_coarse_add_msec(&chan->next_padding_time, &now, 100);
  decision = channelpadding_decide_to_pad_channel(chan);
  tt_int_op(decision, OP_EQ, CHANNELPADDING_PADDING_SCHEDULED);
  tt_assert(chan->pending_padding_callback);
  tt_int_op(tried_to_write_cell, OP_EQ, 0);

  /* Test case #2d: Channel that asks to pad while timeout is scheduled */
  decision = channelpadding_decide_to_pad_channel(chan);
  tt_int_op(decision, OP_EQ, CHANNELPADDING_PADDING_ALREADY_SCHEDULED);

  // Wait for the timer
  new_time += 101*NSEC_PER_MSEC;
  monotime_coarse_set_mock_time_nsec(new_time);
  monotime_set_mock_time_nsec(new_time);
  monotime_coarse_get(&now);
  timers_run_pending();
  tt_int_op(tried_to_write_cell, OP_EQ, 1);
  tt_assert(!chan->pending_padding_callback);

  /* Test case #2e: 0s until timeout */
  tried_to_write_cell = 0;
  monotime_coarse_add_msec(&chan->next_padding_time, &now, 0);
  decision = channelpadding_decide_to_pad_channel(chan);
  tt_int_op(decision, OP_EQ, CHANNELPADDING_PADDING_SENT);
  tt_int_op(tried_to_write_cell, OP_EQ, 1);
  tt_assert(!chan->pending_padding_callback);

  /* Test case #2f: <0s until timeout */
  tried_to_write_cell = 0;
  monotime_coarse_add_msec(&chan->next_padding_time, &now_minus_100s, 0);
  decision = channelpadding_decide_to_pad_channel(chan);
  tt_int_op(decision, OP_EQ, CHANNELPADDING_PADDING_SENT);
  tt_int_op(tried_to_write_cell, OP_EQ, 1);
  tt_assert(!chan->pending_padding_callback);

  /* Test case #3: Channel that sends a packet while timeout is scheduled */
  tried_to_write_cell = 0;
  monotime_coarse_add_msec(&chan->next_padding_time, &now, 100);
  decision = channelpadding_decide_to_pad_channel(chan);
  tt_int_op(decision, OP_EQ, CHANNELPADDING_PADDING_SCHEDULED);
  tt_int_op(tried_to_write_cell, OP_EQ, 0);
  tt_assert(chan->pending_padding_callback);

  // Pretend the channel sent a packet
  channel_timestamp_active(chan);

  // We don't expect any timer callbacks here. Make a dummy one to be sure.
  // Wait for the timer
  new_time += 101*NSEC_PER_MSEC;
  monotime_coarse_set_mock_time_nsec(new_time);
  monotime_set_mock_time_nsec(new_time);
  monotime_coarse_get(&now);
  timers_run_pending();

  tt_int_op(tried_to_write_cell, OP_EQ, 0);
  tt_assert(!chan->pending_padding_callback);

  /* Test case #4: Channel that closes while a timeout is scheduled */
  tried_to_write_cell = 0;
  monotime_coarse_add_msec(&chan->next_padding_time, &now, 100);
  decision = channelpadding_decide_to_pad_channel(chan);
  tt_int_op(decision, OP_EQ, CHANNELPADDING_PADDING_SCHEDULED);
  tt_int_op(tried_to_write_cell, OP_EQ, 0);
  tt_assert(chan->pending_padding_callback);

  // Pretend the channel is temporarily down
  chan->state = CHANNEL_STATE_MAINT;

  // We don't expect any timer callbacks here. Make a dummy one to be sure.
  new_time += 101*NSEC_PER_MSEC;
  monotime_coarse_set_mock_time_nsec(new_time);
  monotime_set_mock_time_nsec(new_time);
  monotime_coarse_get(&now);
  timers_run_pending();

  tt_int_op(tried_to_write_cell, OP_EQ, 0);
  tt_assert(!chan->pending_padding_callback);
  chan->state = CHANNEL_STATE_OPEN;

  /* Test case #5: Make sure previous test case didn't break everything */
  tried_to_write_cell = 0;
  monotime_coarse_add_msec(&chan->next_padding_time, &now, 100);
  decision = channelpadding_decide_to_pad_channel(chan);
  tt_int_op(decision, OP_EQ, CHANNELPADDING_PADDING_SCHEDULED);
  tt_assert(chan->pending_padding_callback);
  tt_int_op(tried_to_write_cell, OP_EQ, 0);

  // Wait for the timer
  new_time += 101*NSEC_PER_MSEC;
  monotime_coarse_set_mock_time_nsec(new_time);
  monotime_set_mock_time_nsec(new_time);
  monotime_coarse_get(&now);
  timers_run_pending();

  tt_int_op(tried_to_write_cell, OP_EQ, 1);
  tt_assert(!chan->pending_padding_callback);

  /* Test case #6. Channel is not used for full circuits */
  chan->channel_usage = CHANNEL_USED_NOT_USED_FOR_FULL_CIRCS;
  decision = channelpadding_decide_to_pad_channel(chan);
  tt_int_op(decision, OP_EQ, CHANNELPADDING_WONTPAD);
  tt_assert(!chan->pending_padding_callback);
  chan->channel_usage = CHANNEL_USED_FOR_FULL_CIRCS;

  /* Test case #7. Channel is closed while timeout is scheduled.
   *
   * NOTE: This test deliberately breaks the channel callback mechanism.
   * It must be last.
   */
  tried_to_write_cell = 0;
  monotime_coarse_add_msec(&chan->next_padding_time, &now, 100);
  decision = channelpadding_decide_to_pad_channel(chan);
  tt_int_op(decision, OP_EQ, CHANNELPADDING_PADDING_SCHEDULED);
  tt_int_op(tried_to_write_cell, OP_EQ, 0);
  tt_assert(chan->pending_padding_callback);

  // Close the connection while the timer is scheduled
  free_fake_channeltls((channel_tls_t*)chan);

  // We don't expect any timer callbacks here. Make a dummy one to be sure.
  new_time = 101*NSEC_PER_MSEC;
  monotime_coarse_set_mock_time_nsec(new_time);
  monotime_set_mock_time_nsec(new_time);
  monotime_coarse_get(&now);
  timers_run_pending();

  tt_int_op(tried_to_write_cell, OP_EQ, 0);

 done:
  smartlist_free(connection_array);

  teardown_capture_of_logs();
  monotime_disable_test_mocking();
  timers_shutdown();
  channel_free_all();

  return;
}

#define TEST_CHANNELPADDING(name, flags) \
    { #name, test_##name, (flags), NULL, NULL }

struct testcase_t channelpadding_tests[] = {
  //TEST_CHANNELPADDING(channelpadding_decide_to_pad_channel, 0),
  TEST_CHANNELPADDING(channelpadding_decide_to_pad_channel, TT_FORK),
  TEST_CHANNELPADDING(channelpadding_negotiation, TT_FORK),
  TEST_CHANNELPADDING(channelpadding_consensus, TT_FORK),
  TEST_CHANNELPADDING(channelpadding_killonehop, TT_FORK),
  TEST_CHANNELPADDING(channelpadding_timers, TT_FORK),
  END_OF_TESTCASES
};
