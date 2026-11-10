/* Copyright (c) 2017-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

#define CIRCUITBUILD_PRIVATE
#define CIRCUITSTATS_PRIVATE
#define CIRCUITLIST_PRIVATE
#define STATEFILE_PRIVATE
#define CONTROL_EVENTS_PRIVATE
#define CHANNEL_FILE_PRIVATE
#define CHANNEL_OBJECT_PRIVATE

#include "core/or/or.h"
#include "test/test.h"
#include "test/test_helpers.h"
#include "test/log_test_helpers.h"
#include "app/config/config.h"
#include "app/config/statefile.h"
#include "app/config/or_state_st.h"
#include "app/config/or_options_st.h"
#include "feature/control/control_events.h"
#include "lib/encoding/confline.h"
#include "lib/confmgt/confmgt.h"
#include "lib/fs/dir.h"
#include "lib/fs/files.h"
#include <math.h>
#include "feature/nodelist/nodelist.h"
#include "core/or/socks_request_st.h"
#include "lib/evloop/compat_libevent.h"
#include "core/or/circuitmux.h"
#include "core/or/circuitmux_ewma.h"
#include "core/or/scheduler.h"
#include "core/or/circuitlist.h"
#include "core/or/circuitbuild.h"
#include "core/or/circuitstats.h"
#include "core/or/circuituse.h"
#include "core/or/channel.h"
#include "core/or/relay.h"
#include "core/or/onion.h"
#include "core/crypto/onion_crypto.h"
#include "core/crypto/onion_fast.h"
#include "core/mainloop/connection.h"
#include "core/mainloop/mainloop.h"
#include "core/or/connection_edge.h"
#include "core/or/entry_connection_st.h"

#include "core/or/cpath_build_state_st.h"
#include "feature/hs/hs_ident.h"
#include "core/or/crypt_path_st.h"
#include "core/or/extend_info_st.h"
#include "core/or/extendinfo.h"
#include "core/or/origin_circuit_st.h"

static origin_circuit_t *add_opened_threehop(void);
static origin_circuit_t *build_unopened_fourhop(struct timeval);
static origin_circuit_t *subtest_fourhop_circuit(struct timeval, int);
static const char *mock_cbt_describe_peer(channel_t *chan);
static int mock_cbt_channel_is_canonical(channel_t *chan);

static int marked_for_close;
/* Mock function because we are not trying to test the close circuit that does
 * an awful lot of checks on the circuit object. */
static void
mock_circuit_mark_for_close(circuit_t *circ, int reason, int line,
                            const char *file)
{
  (void) circ;
  (void) reason;
  (void) line;
  (void) file;
  marked_for_close = 1;
  return;
}

static origin_circuit_t *
add_opened_threehop(void)
{
  struct timeval circ_start_time;
  memset(&circ_start_time, 0, sizeof(circ_start_time));
  extend_info_t fakehop;
  memset(&fakehop, 0, sizeof(fakehop));
  extend_info_t *fakehop_list[DEFAULT_ROUTE_LEN] = {&fakehop,
                                                    &fakehop,
                                                    &fakehop};

  return new_test_origin_circuit(true,
                                 circ_start_time,
                                 DEFAULT_ROUTE_LEN,
                                 fakehop_list);
}

static origin_circuit_t *
build_unopened_fourhop(struct timeval circ_start_time)
{
  extend_info_t fakehop;
  memset(&fakehop, 0, sizeof(fakehop));
  extend_info_t *fakehop_list[4] = {&fakehop,
                                    &fakehop,
                                    &fakehop,
                                    &fakehop};

  return new_test_origin_circuit(false,
                                 circ_start_time,
                                 4,
                                 fakehop_list);
}

static origin_circuit_t *
subtest_fourhop_circuit(struct timeval circ_start_time, int should_timeout)
{
  origin_circuit_t *origin_circ = build_unopened_fourhop(circ_start_time);

  // Now make them open one at a time and call
  // circuit_build_times_handle_completed_hop();
  origin_circ->cpath->state = CPATH_STATE_OPEN;
  circuit_build_times_handle_completed_hop(origin_circ);
  tt_int_op(get_circuit_build_times()->total_build_times, OP_EQ, 0);

  origin_circ->cpath->next->state = CPATH_STATE_OPEN;
  circuit_build_times_handle_completed_hop(origin_circ);
  tt_int_op(get_circuit_build_times()->total_build_times, OP_EQ, 0);

  // Third hop: We should count it now.
  origin_circ->cpath->next->next->state = CPATH_STATE_OPEN;
  circuit_build_times_handle_completed_hop(origin_circ);
  tt_int_op(get_circuit_build_times()->total_build_times, OP_EQ,
            !should_timeout); // 1 if counted, 0 otherwise

  // Fourth hop: Don't double count
  origin_circ->cpath->next->next->next->state = CPATH_STATE_OPEN;
  circuit_build_times_handle_completed_hop(origin_circ);
  tt_int_op(get_circuit_build_times()->total_build_times, OP_EQ,
            !should_timeout);

 done:
  return origin_circ;
}

static void
test_circuitstats_hoplen(void *arg)
{
  /* Plan:
   *   0. Test no other opened circs (relaxed timeout)
   *   1. Check >3 hop circ building w/o timeout
   *   2. Check >3 hop circs w/ timeouts..
   */
  struct timeval circ_start_time;
  origin_circuit_t *threehop = NULL;
  origin_circuit_t *fourhop = NULL;
  (void)arg;
  MOCK(circuit_mark_for_close_, mock_circuit_mark_for_close);

  circuit_build_times_init(get_circuit_build_times_mutable());

  // Let's set a close_ms to 2X the initial timeout, so we can
  // test relaxed functionality (which uses the close_ms timeout)
  get_circuit_build_times_mutable()->close_ms *= 2;

  tor_gettimeofday(&circ_start_time);
  circ_start_time.tv_sec -= 119; // make us hit "relaxed" cutoff

  // Test 1: Build a fourhop circuit that should get marked
  // as relaxed and eventually counted by circuit_expire_building
  // (but not before)
  fourhop = subtest_fourhop_circuit(circ_start_time, 0);
  tt_int_op(fourhop->relaxed_timeout, OP_EQ, 0);
  tt_int_op(marked_for_close, OP_EQ, 0);
  circuit_expire_building();
  tt_int_op(marked_for_close, OP_EQ, 0);
  tt_int_op(fourhop->relaxed_timeout, OP_EQ, 1);
  TO_CIRCUIT(fourhop)->timestamp_began.tv_sec -= 119;
  circuit_expire_building();
  tt_int_op(get_circuit_build_times()->total_build_times, OP_EQ, 1);
  tt_int_op(marked_for_close, OP_EQ, 1);

  circuit_free_(TO_CIRCUIT(fourhop));
  circuit_build_times_reset(get_circuit_build_times_mutable());

  // Test 2: Add a threehop circuit for non-relaxed timeouts
  threehop = add_opened_threehop();

  /* This circuit should not timeout */
  tor_gettimeofday(&circ_start_time);
  circ_start_time.tv_sec -= 59;
  fourhop = subtest_fourhop_circuit(circ_start_time, 0);
  circuit_expire_building();
  tt_int_op(get_circuit_build_times()->total_build_times, OP_EQ, 1);
  tt_int_op(TO_CIRCUIT(fourhop)->purpose, OP_NE,
            CIRCUIT_PURPOSE_C_MEASURE_TIMEOUT);

  circuit_free_((circuit_t *)fourhop);
  circuit_build_times_reset(get_circuit_build_times_mutable());

  /* Test 3: This circuit should now time out and get marked as a
   * measurement circuit, but still get counted (and counted only once)
   */
  circ_start_time.tv_sec -= 2;
  fourhop = subtest_fourhop_circuit(circ_start_time, 0);
  tt_int_op(TO_CIRCUIT(fourhop)->purpose, OP_EQ,
            CIRCUIT_PURPOSE_C_MEASURE_TIMEOUT);
  tt_int_op(get_circuit_build_times()->total_build_times, OP_EQ, 1);
  circuit_expire_building();
  tt_int_op(get_circuit_build_times()->total_build_times, OP_EQ, 1);

 done:
  UNMOCK(circuit_mark_for_close_);
  circuit_free_(TO_CIRCUIT(threehop));
  circuit_free_(TO_CIRCUIT(fourhop));
  circuit_build_times_free_timeouts(get_circuit_build_times_mutable());
}

/* A fixed clock keeps both completion and expiration order reproducible. */
static struct timeval cbt_test_now;
static void
mock_cbt_gettimeofday(struct timeval *now)
{
  *now = cbt_test_now;
}

/* Preserve close bookkeeping without needing transport fixtures. */
static void
mock_cbt_mark_closed(circuit_t *circ, int reason, int line, const char *file)
{
  (void)reason;
  (void)file;
  circ->marked_for_close = line;
}

/* A recovery probe must neither block request selection nor accumulate with
 * each retry. Exercise directory selection and the actual pending limit. */
static void
test_circuitstats_recovery_retries(void *arg)
{
  circuit_build_times_t *cbt = get_circuit_build_times_mutable();
  origin_circuit_t *probe = NULL, *retry = NULL;
  entry_connection_t *request = NULL;
  channel_t channel = { 0 };
  const struct timeval start = { 1000, 0 };
  extend_info_t hop = { 0 };
  extend_info_t *path[] = { &hop, &hop, &hop };
  (void)arg;
  circuitbuild_running_unit_tests();
  tor_init_connection_lists();
  MOCK(tor_gettimeofday, mock_cbt_gettimeofday);
  MOCK(channel_describe_peer, mock_cbt_describe_peer);
  MOCK(circuit_mark_for_close_, mock_cbt_mark_closed);
  request = entry_connection_new(CONN_TYPE_AP, AF_INET);
  request->chosen_exit_name = tor_strdup(
      "$4242424242424242424242424242424242424242");

  /* Fixed-timeout directory and general circuits, then insufficient adaptive
   * history. None takes the usual statistical measurement conversion. */
  for (int kind = 0; kind < 3; ++kind) {
    const bool onehop = kind == 0;
    get_options_mutable()->LearnCircuitBuildTimeout = kind == 2;
    get_options_mutable()->CircuitBuildTimeout = 5;
    circuit_build_times_init(cbt);
    cbt->timeout_ms = cbt->close_ms = 5000;
    channel.state = CHANNEL_STATE_OPEN;
    channel.is_bad_for_new_circs = 0;
    cbt_test_now = start;
    probe = new_test_origin_circuit(false, start, onehop ? 1 : 3, path);
    probe->build_state->onehop_tunnel = onehop;
    probe->base_.state = CIRCUIT_STATE_BUILDING;
    probe->base_.n_chan = &channel;
    probe->cpath->state = CPATH_STATE_AWAITING_KEYS;
    request->want_onehop = onehop;
    tt_int_op(count_pending_general_client_circuits(), OP_EQ, 1);
    if (onehop)
      tt_ptr_op(circuit_get_best(request, 0, CIRCUIT_PURPOSE_C_GENERAL,
                                0, 0), OP_EQ, probe);

    cbt_test_now.tv_sec += 5;
    circuit_expire_building();
    tt_assert(!probe->base_.marked_for_close);
    tt_int_op(count_pending_general_client_circuits(), OP_EQ, 0);
    tt_ptr_op(circuit_get_best(request, 0, CIRCUIT_PURPOSE_C_GENERAL,
                              0, 0), OP_EQ, NULL);
    tt_int_op(probe->base_.purpose, OP_EQ, CIRCUIT_PURPOSE_C_MEASURE_TIMEOUT);

    /* Repeated retries can use the same TLS connection, but must not leave
     * a new retained probe behind at each usage deadline. */
    for (int attempt = 0; attempt < 3; ++attempt) {
      retry = new_test_origin_circuit(false, cbt_test_now,
                                      onehop ? 1 : 3, path);
      retry->build_state->onehop_tunnel = onehop;
      retry->base_.state = CIRCUIT_STATE_BUILDING;
      retry->base_.n_chan = &channel;
      retry->cpath->state = CPATH_STATE_AWAITING_KEYS;
      tt_int_op(count_pending_general_client_circuits(), OP_EQ, 1);
      if (onehop)
        tt_ptr_op(circuit_get_best(request, 0, CIRCUIT_PURPOSE_C_GENERAL,
                                  0, 0), OP_EQ, retry);
      cbt_test_now.tv_sec += 5;
      circuit_expire_building();
      tt_assert(retry->base_.marked_for_close);
      tt_assert(!probe->base_.marked_for_close);
      tt_int_op(channel.is_bad_for_new_circs, OP_EQ, 0);
      tt_int_op(count_pending_general_client_circuits(), OP_EQ, 0);
      retry->base_.n_chan = NULL;
      circuit_free_(TO_CIRCUIT(retry));
      retry = NULL;
    }
    cbt_test_now.tv_sec = start.tv_sec + 60;
    circuit_expire_building();
    tt_assert(probe->base_.marked_for_close);
    tt_int_op(channel.is_bad_for_new_circs, OP_EQ, 1);
    probe->base_.n_chan = NULL;
    circuit_free_(TO_CIRCUIT(probe));
    probe = NULL;
    circuit_build_times_free_timeouts(cbt);
  }
 done:
  if (probe)
    probe->base_.n_chan = NULL;
  if (retry)
    retry->base_.n_chan = NULL;
  circuit_free_(TO_CIRCUIT(probe));
  circuit_free_(TO_CIRCUIT(retry));
  connection_free_(ENTRY_TO_CONN(request));
  circuit_build_times_free_timeouts(cbt);
  UNMOCK(tor_gettimeofday);
  UNMOCK(channel_describe_peer);
  UNMOCK(circuit_mark_for_close_);
}

static void
test_circuitstats_prefix_accounting(void *arg)
{
  circuit_build_times_t *cbt = get_circuit_build_times_mutable();
  origin_circuit_t *circ = NULL, *opened = NULL;
  (void)arg;
  circuitbuild_running_unit_tests();
  MOCK(tor_gettimeofday, mock_cbt_gettimeofday);
  MOCK(circuit_mark_for_close_, mock_circuit_mark_for_close);
  circuit_build_times_init(cbt);

  /* Fast, slow, nonlive, clock-discarded, and cannibalized prefixes,
   * with and without another usable circuit. */
  for (int have_open = 0; have_open < 2; ++have_open) {
    if (have_open)
      opened = add_opened_threehop();
    for (int variant = 0; variant < 5; ++variant) {
      struct timeval start = { 1000, 0 };
      circuit_build_times_reset(cbt);
      memset(cbt->liveness.timeouts_after_firsthop, 0,
             cbt->liveness.num_recent_circs);
      cbt->liveness.after_firsthop_idx = 0;
      cbt->liveness.nonlive_timeouts = variant == 2;
      for (int i = 0; i < CBT_DEFAULT_MIN_CIRCUITS_TO_OBSERVE; ++i)
        circuit_build_times_add_time(cbt, 500);
      cbt->timeout_ms = 1000;
      cbt->close_ms = 60000;
      circ = build_unopened_fourhop(start);
    circ->has_opened = variant == 4;
    circ->cpath->state = CPATH_STATE_OPEN;
    circ->cpath->next->state = CPATH_STATE_OPEN;
    circ->cpath->next->next->state = CPATH_STATE_OPEN;
    cbt_test_now = start;
    cbt_test_now.tv_usec = 100000;
    if (variant == 1)
      cbt_test_now.tv_sec += 2;
    if (variant == 3)
      cbt_test_now.tv_sec -= 1;
    circuit_build_times_handle_completed_hop(circ);
    int expected = CBT_DEFAULT_MIN_CIRCUITS_TO_OBSERVE + (variant < 2);
    tt_int_op(cbt->total_build_times, OP_EQ, expected);
    tt_int_op(cbt->num_circ_timeouts, OP_EQ, have_open && variant == 1);
    uint32_t timeouts = cbt->num_circ_timeouts;
    int recent_idx = cbt->liveness.after_firsthop_idx;
    circuit_build_times_handle_completed_hop(circ);
    tt_int_op(cbt->total_build_times, OP_EQ, expected);

    /* Leave hop four stalled. The soft timeout still relaxes or changes
     * purpose, and the hard timeout still closes, without CBT failures. */
    cbt->timeout_ms = 1000;
    cbt->close_ms = 60000;
    cbt->liveness.nonlive_timeouts = 0;
    marked_for_close = 0;

    cbt_test_now.tv_sec = 1003;
      circuit_expire_building();
      if (variant != 4)
        tt_int_op(marked_for_close, OP_EQ, 0);
      cbt_test_now.tv_sec = 1061;
      circuit_expire_building();
      circuit_expire_building();
      tt_int_op(marked_for_close, OP_EQ, 1);
      tt_int_op(cbt->total_build_times, OP_EQ, expected);
      tt_int_op(cbt->num_circ_timeouts, OP_EQ, timeouts);
      tt_int_op(cbt->num_circ_closed, OP_EQ, 0);
      tt_int_op(cbt->liveness.after_firsthop_idx, OP_EQ, recent_idx);
      circuit_free_(TO_CIRCUIT(circ));
      circ = NULL;
    }
    circuit_free_(TO_CIRCUIT(opened));
    opened = NULL;
  }
 done:
  circuit_free_(TO_CIRCUIT(circ));
  circuit_free_(TO_CIRCUIT(opened));
  circuit_build_times_free_timeouts(cbt);
  UNMOCK(tor_gettimeofday);
  UNMOCK(circuit_mark_for_close_);
}

static int cbt_rend_launches;
static origin_circuit_t *cbt_rend_retry;
static origin_circuit_t *
mock_cbt_rend_launch(uint8_t purpose, extend_info_t *ei, int flags)
{
  (void)ei;
  (void)flags;
  ++cbt_rend_launches;
  cbt_rend_retry = build_unopened_fourhop(cbt_test_now);
  cbt_rend_retry->base_.purpose = purpose;
  return cbt_rend_retry;
}

static void
mock_cbt_assert_circuit_ok(const circuit_t *circ)
{
  (void)circ;
}

/* Exercise real repurpose/close/free cleanup. Only the new network launch
 * and assertions about cryptographic fixture completeness are mocked. */
static void
test_circuitstats_prefix_lifecycle(void *arg)
{
  circuit_build_times_t *cbt = get_circuit_build_times_mutable();
  origin_circuit_t *circ = NULL, *opened = NULL;
  (void)arg;
  circuitbuild_running_unit_tests();
  MOCK(tor_gettimeofday, mock_cbt_gettimeofday);
  MOCK(assert_circuit_ok, mock_cbt_assert_circuit_ok);
  MOCK(circuit_launch_by_extend_info, mock_cbt_rend_launch);
  circuit_build_times_init(cbt);
  opened = add_opened_threehop();
  for (int kind = 0; kind < 2; ++kind) {
    struct timeval start = { 1000, 0 };
    cbt_rend_launches = 0;
    circuit_build_times_reset(cbt);
    for (int i = 0; i < CBT_DEFAULT_MIN_CIRCUITS_TO_OBSERVE; ++i)
      circuit_build_times_add_time(cbt, 500);
    cbt->timeout_ms = 1000;
    cbt->close_ms = 60000;
    circ = build_unopened_fourhop(start);
    circ->base_.state = CIRCUIT_STATE_BUILDING;
    circ->base_.purpose = CIRCUIT_PURPOSE_S_CONNECT_REND;
    circ->has_opened = kind == 1;
    circ->hs_ident = tor_malloc_zero(sizeof(*circ->hs_ident));
    memset(circ->hs_ident->rendezvous_cookie, 0x42, REND_COOKIE_LEN);
    memset(circ->hs_ident->rendezvous_handshake_info, 0x43,
           sizeof(circ->hs_ident->rendezvous_handshake_info));
    circ->build_state->expiry_time = time(NULL) + 30;
    circ->cpath->state = CPATH_STATE_OPEN;
    circ->cpath->next->state = CPATH_STATE_OPEN;
    circ->cpath->next->next->state = CPATH_STATE_OPEN;
    cbt_test_now = start;
    cbt_test_now.tv_usec = 100000;
    circuit_build_times_handle_completed_hop(circ);
    cbt->timeout_ms = 1000;
    cbt->close_ms = 60000;
    cbt_test_now.tv_sec = 1003;
    circuit_expire_building();
    if (kind == 0) {
      tt_int_op(cbt_rend_launches, OP_EQ, 1);
      tt_int_op(cbt_rend_retry->build_state->failure_count, OP_EQ, 1);
      tt_mem_op(cbt_rend_retry->hs_ident, OP_EQ, circ->hs_ident,
                sizeof(*circ->hs_ident));
    }
    cbt_test_now.tv_sec = 1061;
    circuit_expire_building();
    tt_assert(circ->base_.marked_for_close);
    tt_int_op(cbt_rend_launches, OP_EQ, 1);
    tt_int_op(cbt->num_circ_timeouts, OP_EQ, 0);
    tt_int_op(cbt->num_circ_closed, OP_EQ, 0);
    circuit_free_(TO_CIRCUIT(circ));
    circ = NULL;
    circuit_free_(TO_CIRCUIT(cbt_rend_retry));
    cbt_rend_retry = NULL;
  }
 done:
  circuit_free_(TO_CIRCUIT(circ));
  circuit_free_(TO_CIRCUIT(opened));
  circuit_free_(TO_CIRCUIT(cbt_rend_retry));
  cbt_rend_retry = NULL;
  circuit_build_times_free_timeouts(cbt);
  UNMOCK(tor_gettimeofday);
  UNMOCK(assert_circuit_ok);
  UNMOCK(circuit_launch_by_extend_info);
}

static or_state_t *cbt_test_state;
static int cbt_reset_events;
static or_state_t *
mock_cbt_get_state(void)
{
  return cbt_test_state;
}

static void
mock_cbt_event(uint16_t event, char *msg)
{
  if (event == EVENT_BUILDTIMEOUT_SET && strstr(msg, "RESET"))
    ++cbt_reset_events;
  tor_free(msg);
}

static void
test_circuitstats_recovery(void *arg)
{
  circuit_build_times_t *cbt = get_circuit_build_times_mutable();
  or_options_t *options = get_options_mutable();
  origin_circuit_t *old = NULL, *fresh = NULL;
  struct timeval start = { 1000, 0 };
  (void)arg;
  cbt_test_state = or_state_new();
  MOCK(get_or_state, mock_cbt_get_state);
  MOCK(queue_control_event_string, mock_cbt_event);
  MOCK(tor_gettimeofday, mock_cbt_gettimeofday);
  control_testing_set_global_event_mask(EVENT_MASK_(EVENT_BUILDTIMEOUT_SET));
  options->LearnCircuitBuildTimeout = 1;
  circuit_build_times_init(cbt);
  old = build_unopened_fourhop(start);
  for (int i = 0; i < CBT_NCIRCUITS_TO_OBSERVE; ++i)
    circuit_build_times_add_time(cbt, 465);
  cbt->timeout_ms = 465;
  cbt->close_ms = 90000;
  cbt->have_computed_timeout = 1;
  for (int i = 0; i < CBT_NCIRCUITS_TO_OBSERVE; ++i) {
    circuit_build_times_network_is_live(cbt);
    circuit_build_times_count_timeout(cbt, 0);
    tt_assert(circuit_build_times_count_close(cbt, 0, 1));
    if (i < CBT_NCIRCUITS_TO_OBSERVE - 1)
      tt_int_op(cbt->total_build_times, OP_EQ, CBT_NCIRCUITS_TO_OBSERVE);
  }
  cbt_reset_events = 0;
  setup_full_capture_of_logs(LOG_NOTICE);
  circuit_build_times_set_timeout(cbt);
  tt_int_op(cbt_reset_events, OP_EQ, 1);
  expect_log_msg_containing("restarting conservative learning");
  tt_int_op(cbt->total_build_times, OP_EQ, 0);
  tt_int_op(cbt->build_times_idx, OP_EQ, 0);
  tt_int_op(cbt->have_computed_timeout, OP_EQ, 0);
  tt_int_op(cbt->liveness.after_firsthop_idx, OP_EQ, 0);
  tt_double_op(fabs(cbt->timeout_ms - (60000)), OP_LT, 0.001);
  tt_double_op(fabs(cbt->close_ms - (90000)), OP_LT, 0.001);
  tt_assert(circuit_build_times_needs_circuits(cbt));
  tt_assert(old->cbt_observation_invalidated);
  mock_clean_saved_logs();
  for (int i = 0; i < 100; ++i)
    circuit_build_times_set_timeout(cbt);
  expect_no_log_entry();
  tt_int_op(cbt_reset_events, OP_EQ, 1);

  /* An old success still builds but cannot enter the repaired population. */
  cbt_test_now = start;
  cbt_test_now.tv_usec = 100000;
  old->cpath->state = CPATH_STATE_OPEN;
  old->cpath->next->state = CPATH_STATE_OPEN;
  old->cpath->next->next->state = CPATH_STATE_OPEN;
  circuit_build_times_handle_completed_hop(old);
  tt_int_op(cbt->total_build_times, OP_EQ, 0);
  fresh = build_unopened_fourhop(start);
  tt_assert(!fresh->cbt_observation_invalidated);
  fresh->cpath->state = CPATH_STATE_OPEN;
  fresh->cpath->next->state = CPATH_STATE_OPEN;
  fresh->cpath->next->next->state = CPATH_STATE_OPEN;
  circuit_build_times_handle_completed_hop(fresh);
  tt_int_op(cbt->total_build_times, OP_EQ, 1);

  /* Partial histories can keep learning, but a full sparse one recovers. */
  circuit_build_times_set_timeout(cbt);
  tt_int_op(cbt->total_build_times, OP_EQ, 1);
  for (int i = 1; i < CBT_NCIRCUITS_TO_OBSERVE; ++i)
    circuit_build_times_add_time(cbt, CBT_BUILD_ABANDONED);
  circuit_build_times_set_timeout(cbt);
  tt_int_op(cbt->total_build_times, OP_EQ, 0);
  tt_int_op(cbt_reset_events, OP_EQ, 2);
  /* An entirely abandoned history also recovers. Configured floor wins. */
  options->CircuitBuildTimeout = 120;
  circuit_build_times_add_time(cbt, CBT_BUILD_ABANDONED);
  circuit_build_times_set_timeout(cbt);
  tt_double_op(fabs(cbt->timeout_ms - (120000)), OP_LT, 0.001);
  tt_double_op(fabs(cbt->close_ms - (120000)), OP_LT, 0.001);
  tt_int_op(cbt_reset_events, OP_EQ, 3);
  for (int variant = 0; variant < 3; ++variant) {
    circuit_build_times_add_time(cbt, CBT_BUILD_ABANDONED);
    cbt->timeout_ms = variant == 0 ? (double)NAN : variant == 1 ? -1 : 180000;
    cbt->close_ms = variant == 0 ? (double)INFINITY :
                    variant == 1 ? -1 : 240000;
    circuit_build_times_set_timeout(cbt);
    tt_double_op(fabs(cbt->timeout_ms - (variant == 2 ? 180000 : 120000)),
                 OP_LT, 0.001);
    tt_double_op(fabs(cbt->close_ms - (variant == 2 ? 240000 : 120000)),
                 OP_LT, 0.001);
  }
  /* Disabled learning neither fits nor repairs observations. */
  options->LearnCircuitBuildTimeout = 0;
  circuit_build_times_add_time(cbt, CBT_BUILD_ABANDONED);
  circuit_build_times_set_timeout(cbt);
  tt_int_op(cbt->total_build_times, OP_EQ, 1);
 done:
  teardown_capture_of_logs();
  circuit_free_(TO_CIRCUIT(old));
  circuit_free_(TO_CIRCUIT(fresh));
  circuit_build_times_free_timeouts(cbt);
  or_state_free(cbt_test_state);
  UNMOCK(get_or_state);
  UNMOCK(queue_control_event_string);
  UNMOCK(tor_gettimeofday);
}

static void
test_circuitstats_recovery_mixed(void *arg)
{
  circuit_build_times_t cbt = {0}, loaded = {0};
  or_state_t *state = or_state_new();
  const int mincircs = CBT_DEFAULT_MIN_CIRCUITS_TO_OBSERVE;
  const struct {
    int usable;
    int total;
    int recover;
  } cases[] = {
    { mincircs - 1, CBT_NCIRCUITS_TO_OBSERVE - 1, 0 },
    { mincircs - 1, CBT_NCIRCUITS_TO_OBSERVE, 1 },
    { mincircs, CBT_NCIRCUITS_TO_OBSERVE, 0 },
    { 1, CBT_NCIRCUITS_TO_OBSERVE, 1 },
    { mincircs - 1, mincircs - 1, 0 },
    { mincircs, mincircs, 0 },
  };
  (void)arg;
  cbt_test_state = state;
  MOCK(get_or_state, mock_cbt_get_state);
  get_options_mutable()->LearnCircuitBuildTimeout = 1;

  for (unsigned i = 0; i < ARRAY_LENGTH(cases); ++i) {
    circuit_build_times_init(&cbt);
    for (int j = 0; j < cases[i].total; ++j)
      circuit_build_times_add_time(&cbt, j < cases[i].usable ?
                                   (build_time_t)(400 + (j % 10) * 50) :
                                   CBT_BUILD_ABANDONED);
    cbt.timeout_ms = 465;
    cbt.close_ms = 90000;
    /* Startup and runtime must apply the same recovery threshold. */
    circuit_build_times_update_state(&cbt, state);
    tt_int_op(circuit_build_times_parse_state(&loaded, state), OP_EQ, 0);
    circuit_build_times_set_timeout(&cbt);
    tt_int_op(cbt.total_build_times, OP_EQ,
              cases[i].recover ? 0 : cases[i].total);
    tt_int_op(loaded.total_build_times, OP_EQ, cbt.total_build_times);
    tt_int_op(cbt.have_computed_timeout, OP_EQ,
              !cases[i].recover && cases[i].total >= mincircs);
    tt_int_op(loaded.have_computed_timeout, OP_EQ, cbt.have_computed_timeout);
    if (cases[i].recover) {
      tt_double_op(fabs(cbt.timeout_ms - 60000), OP_LT, 0.001);
      tt_double_op(fabs(cbt.close_ms - 90000), OP_LT, 0.001);
      tt_double_op(fabs(loaded.timeout_ms - 60000), OP_LT, 0.001);
      tt_assert(circuit_build_times_needs_circuits(&cbt));
    }
    circuit_build_times_free_timeouts(&cbt);
    circuit_build_times_free_timeouts(&loaded);
  }
 done:
  circuit_build_times_free_timeouts(&cbt);
  circuit_build_times_free_timeouts(&loaded);
  or_state_free(state);
  UNMOCK(get_or_state);
}

static void
test_circuitstats_recovery_load(void *arg)
{
  circuit_build_times_t cbt;
  or_options_t *options = get_options_mutable();
  char *before = NULL, *after = NULL;
  (void)arg;
  memset(&cbt, 0, sizeof(cbt));
  cbt_test_state = or_state_new();
  MOCK(get_or_state, mock_cbt_get_state);
  options->LearnCircuitBuildTimeout = 1;
  config_line_append(&cbt_test_state->Guard, "Guard", "unchanged-guard-state");
  before = config_dump(get_state_mgr(), NULL, cbt_test_state, 1, 0);
  for (int avoid = 0; avoid < 2; ++avoid) {
    options->AvoidDiskWrites = avoid;
    cbt_test_state->TotalBuildTimes = CBT_NCIRCUITS_TO_OBSERVE;
    cbt_test_state->CircuitBuildAbandonedCount = CBT_NCIRCUITS_TO_OBSERVE;
    cbt_test_state->next_write = TIME_MAX;
    tt_int_op(circuit_build_times_parse_state(&cbt, cbt_test_state), OP_EQ, 0);
    tt_int_op(cbt.total_build_times, OP_EQ, 0);
    tt_double_op(fabs(cbt.timeout_ms - (60000)), OP_LT, 0.001);
    tt_assert(circuit_build_times_needs_circuits(&cbt));
    if (avoid) {
      tt_i64_op(cbt_test_state->next_write, OP_GE, time(NULL));
      tt_i64_op(cbt_test_state->next_write, OP_LE, time(NULL) + 3600);
    } else {
      tt_i64_op(cbt_test_state->next_write, OP_EQ, 0);
    }
    circuit_build_times_update_state(&cbt, cbt_test_state);
    after = config_dump(get_state_mgr(), NULL, cbt_test_state, 1, 0);
    tt_str_op(before, OP_EQ, after);
    tor_free(after);
    circuit_build_times_free_timeouts(&cbt);
  }
  /* Restarting in fixed mode ignores abandoned history. */
  options->LearnCircuitBuildTimeout = 0;
  options->CircuitBuildTimeout = 60;
  cbt_test_state->TotalBuildTimes = CBT_NCIRCUITS_TO_OBSERVE;
  cbt_test_state->CircuitBuildAbandonedCount = CBT_NCIRCUITS_TO_OBSERVE;
  tt_int_op(circuit_build_times_parse_state(&cbt, cbt_test_state), OP_EQ, 0);
  tt_double_op(fabs(cbt.timeout_ms - (60000)), OP_LT, 0.001);
  tt_double_op(fabs(cbt.close_ms - (60000)), OP_LT, 0.001);
  tt_int_op(cbt.total_build_times, OP_EQ, 0);
 done:
  circuit_build_times_free_timeouts(&cbt);
  or_state_free(cbt_test_state);
  tor_free(before);
  tor_free(after);
  UNMOCK(get_or_state);
}

static void
test_circuitstats_recovery_backlog(void *arg)
{
  circuit_build_times_t *cbt = get_circuit_build_times_mutable();
  smartlist_t *old = smartlist_new();
  origin_circuit_t *opened = NULL;
  struct timeval start = { 1000, 0 };
  channel_t channel = { 0 };
  channel.state = CHANNEL_STATE_OPEN;
  (void)arg;
  circuitbuild_running_unit_tests();
  MOCK(tor_gettimeofday, mock_cbt_gettimeofday);
  MOCK(channel_describe_peer, mock_cbt_describe_peer);
  MOCK(assert_circuit_ok, mock_cbt_assert_circuit_ok);
  circuit_build_times_init(cbt);
  setup_full_capture_of_logs(LOG_NOTICE);
  opened = add_opened_threehop();
  for (int i = 0; i < 400; ++i) {
    origin_circuit_t *circ = build_unopened_fourhop(start);
    circ->base_.n_chan = &channel;
    circ->base_.state = CIRCUIT_STATE_BUILDING;
    circ->base_.purpose = CIRCUIT_PURPOSE_C_MEASURE_TIMEOUT;
    circ->cpath->state = CPATH_STATE_AWAITING_KEYS;
    smartlist_add(old, circ);
  }
  for (int i = 0; i < CBT_NCIRCUITS_TO_OBSERVE; ++i)
    circuit_build_times_add_time(cbt, i ? CBT_BUILD_ABANDONED : 465);
  cbt->timeout_ms = 465;
  cbt->close_ms = 60000;
  cbt_test_now.tv_sec = 1061;
  cbt_test_now.tv_usec = 0;
  circuit_build_times_network_is_live(cbt);
  /* First terminal close overwrites the last completion; the rest of this
   * very same expiration pass must not refill the repaired history. */
  circuit_expire_building();
  tt_int_op(channel.is_bad_for_new_circs, OP_EQ, 1);
  expect_log_msg_containing("prefix_hops_0=400");
  expect_log_msg_containing("abandoned=1 excluded=399");
  tt_int_op(cbt->total_build_times, OP_EQ, 0);
  tt_int_op(cbt->num_circ_closed, OP_EQ, 0);
  SMARTLIST_FOREACH_BEGIN(old, origin_circuit_t *, circ) {
    tt_assert(circ->base_.marked_for_close);
    tt_assert(circ->cbt_observation_invalidated);
  } SMARTLIST_FOREACH_END(circ);
  tt_double_op(fabs(cbt->timeout_ms - (60000)), OP_LT, 0.001);
 done:
  teardown_capture_of_logs();
  SMARTLIST_FOREACH_BEGIN(old, origin_circuit_t *, circ) {
    circ->base_.n_chan = NULL;
    circuit_free_(TO_CIRCUIT(circ));
  } SMARTLIST_FOREACH_END(circ);
  smartlist_free(old);
  circuit_free_(TO_CIRCUIT(opened));
  circuit_build_times_free_timeouts(cbt);
  UNMOCK(tor_gettimeofday);
  UNMOCK(channel_describe_peer);
  UNMOCK(assert_circuit_ok);
}

/* Use the actual installed state and file writer, including delayed writes. */
static void
test_circuitstats_recovery_statefile(void *arg)
{
  or_options_t *options = get_options_mutable();
  circuit_build_times_t *cbt = get_circuit_build_times_mutable();
  char *path = NULL, *contents = NULL;
  const char *abandoned = "TotalBuildTimes 1000\n"
                          "CircuitBuildAbandonedCount 1000\n";
  (void)arg;
  tor_free(options->DataDirectory);
  options->DataDirectory = tor_strdup(get_fname("cbt-state"));
  tt_int_op(check_private_dir(options->DataDirectory, CPD_CREATE, NULL),
            OP_EQ, 0);
  path = get_datadir_fname("state");
  options->LearnCircuitBuildTimeout = 1;
  for (int avoid = 0; avoid < 2; ++avoid) {
    time_t now = time(NULL);
    options->AvoidDiskWrites = avoid;
    tt_int_op(write_str_to_file(path, abandoned, 0), OP_EQ, 0);
    tt_int_op(or_state_load(), OP_EQ, 0);
    tt_int_op(cbt->total_build_times, OP_EQ, 0);
    tt_int_op(or_state_save(now), OP_EQ, 0);
    contents = read_file_to_str(path, 0, NULL);
    tt_assert(contents);
    if (avoid) {
      /* Startup already has a write pending. Once flushed, runtime recovery
       * must schedule a delayed write without overriding earlier work. */
      circuit_build_times_add_time(cbt, CBT_BUILD_ABANDONED);
      circuit_build_times_set_timeout(cbt);
      tt_i64_op(get_or_state()->next_write, OP_GT, now);
      tt_int_op(or_state_save(now), OP_EQ, 0);
      tt_i64_op(get_or_state()->LastWritten, OP_EQ, now);
      tor_free(contents);
      tt_int_op(or_state_save(now + 3601), OP_EQ, 0);
      tt_i64_op(get_or_state()->LastWritten, OP_EQ, now + 3601);
      contents = read_file_to_str(path, 0, NULL);
      tt_assert(contents);
    }
    tt_assert(!strstr(contents, "TotalBuildTimes 1000"));
    tt_assert(!strstr(contents, "CircuitBuildAbandonedCount 1000"));
    tor_free(contents);
    circuit_build_times_free_timeouts(cbt);
    or_state_free_all();
    tt_int_op(or_state_load(), OP_EQ, 0);
    tt_int_op(cbt->total_build_times, OP_EQ, 0);
    circuit_build_times_free_timeouts(cbt);
    or_state_free_all();
  }
 done:
  circuit_build_times_free_timeouts(cbt);
  or_state_free_all();
  tor_free(contents);
  tor_free(path);
}

static void
test_circuitstats_diagnostics(void *arg)
{
  circuit_build_times_t *cbt = get_circuit_build_times_mutable();
  origin_circuit_t *circ = NULL;
  channel_t channel;
  struct timeval start = { 1000, 0 };
  (void)arg;
  memset(&channel, 0, sizeof(channel));
  memset(&cbt_diagnostics, 0, sizeof(cbt_diagnostics));
  circuitbuild_running_unit_tests();
  circuit_build_times_init(cbt);
  MOCK(tor_gettimeofday, mock_cbt_gettimeofday);
  setup_full_capture_of_logs(LOG_NOTICE);
  circuit_build_times_report_diagnostics(1000);
  expect_no_log_entry();
  /* One-hop attempts were never eligible three-hop observations. */
  circ = build_unopened_fourhop(start);
  circ->build_state->desired_path_len = 1;
  circ->cbt_observation_invalidated = 1;
  circuit_build_times_count_circ_timeout(circ);
  tt_u64_op(cbt_diagnostics.excluded, OP_EQ, 0);
  circuit_free_(TO_CIRCUIT(circ));
  circ = NULL;
  for (int hops = 0; hops < 4; ++hops) {
    circ = build_unopened_fourhop(start);
    circ->base_.state = CIRCUIT_STATE_BUILDING;
    circ->global_identifier = 987654321;
    circ->base_.n_circ_id = 123456789;
    circ->hs_ident = tor_malloc_zero(sizeof(*circ->hs_ident));
    memset(circ->hs_ident, 0x5a, sizeof(*circ->hs_ident));
    strlcpy(circ->cpath->extend_info->nickname, "PrivateRelayFixture",
            sizeof(circ->cpath->extend_info->nickname));
    memset(circ->cpath->extend_info->identity_digest, 0x6b, DIGEST_LEN);
    tor_addr_parse(&circ->cpath->extend_info->orports[0].addr, "192.0.2.123");
    circ->cpath->extend_info->orports[0].port = 23456;
    circ->base_.n_chan = &channel;
    channel.state = hops % 2 ? CHANNEL_STATE_OPEN : CHANNEL_STATE_OPENING;
    crypt_path_t *hop = circ->cpath;
    for (int i = 0; i < hops; ++i, hop = hop->next)
      hop->state = CPATH_STATE_OPEN;
    /* Nonlive circuits still appear in expiry diagnostics. */
    cbt->liveness.nonlive_timeouts = 1;
    circuit_build_times_note_expiry(circ);
    circuit_build_times_note_expiry(circ);
    circ->base_.n_chan = NULL;
    circuit_free_(TO_CIRCUIT(circ));
    circ = NULL;
  }
  circuit_build_times_reset(cbt);
  tt_u64_op(cbt_diagnostics.prefix_hops[0], OP_EQ, 1);
  tt_u64_op(cbt_diagnostics.prefix_hops[1], OP_EQ, 1);
  tt_u64_op(cbt_diagnostics.prefix_hops[2], OP_EQ, 1);
  tt_u64_op(cbt_diagnostics.post_prefix, OP_EQ, 1);
  tt_u64_op(cbt_diagnostics.open_channel, OP_EQ, 2);
  tt_u64_op(cbt_diagnostics.other_channel, OP_EQ, 2);
  circuit_build_times_report_diagnostics(1000);
  /* Exact fixed vocabulary also excludes all injected identifiers and
   * individual timings, without enumerating possible encodings of them. */
  expect_single_log_msg("Circuit build expiry summary: prefix_hops_0=1 "
      "prefix_hops_1=1 prefix_hops_2=1 post_prefix=1 open_channel=2 "
      "other_channel=2 late_firsthop=0 completed=0 abandoned=0 excluded=0 "
      "adaptive=1 connection_failed=0 channels_retired=0 "
      "firsthop_preserved=0 (build stages are at measurement or circuit "
      "expiry; recovery probes may finish later)\n");
  mock_clean_saved_logs();
  circ = build_unopened_fourhop(start);
  circ->cpath->state = CPATH_STATE_AWAITING_KEYS;
  circuit_build_times_count_circ_timeout(circ);
  tt_u64_op(cbt->num_circ_timeouts, OP_EQ, 1);
  tt_u64_op(cbt_diagnostics.prefix_hops[0], OP_EQ, 0);
  circ->cpath->state = CPATH_STATE_OPEN;
  cbt_test_now = start;
  cbt_test_now.tv_usec = 100000;
  circuit_build_times_handle_completed_hop(circ);
  circuit_build_times_handle_completed_hop(circ);
  tt_u64_op(cbt_diagnostics.late_firsthop, OP_EQ, 1);
  tt_u64_op(cbt->num_circ_timeouts, OP_EQ, 1);
  tt_int_op(cbt->liveness.after_firsthop_idx, OP_EQ, 0);
  circ->cpath->next->state = CPATH_STATE_OPEN;
  circ->cpath->next->next->state = CPATH_STATE_OPEN;
  cbt->liveness.nonlive_timeouts = 0;
  circuit_build_times_handle_completed_hop(circ);
  tt_u64_op(cbt_diagnostics.completed, OP_EQ, 1);
  circuit_build_times_report_diagnostics(1300);
  expect_no_log_entry(); /* Healthy completion alone is quiet. */
  circ->cbt_observation_invalidated = 1;
  circ->cbt_prefix_measurement_done = 0;
  circ->cbt_soft_timeout_before_firsthop = 1;
  circuit_build_times_handle_completed_hop(circ);
  tt_u64_op(cbt_diagnostics.late_firsthop, OP_EQ, 1);
  circuit_build_times_note_expiry(circ);
  circuit_build_times_note_expiry(circ);
  tt_u64_op(cbt_diagnostics.excluded, OP_EQ, 1);
  circuit_build_times_report_diagnostics(1299);
  expect_no_log_entry();
  circuit_build_times_report_diagnostics(999); /* Clock moved backwards. */
  expect_no_log_entry();
  circuit_build_times_report_diagnostics(1300);
  expect_single_log_msg("late_firsthop=1 completed=1 abandoned=0 excluded=1");
  mock_clean_saved_logs();
  circuit_build_times_report_diagnostics(1600);
  expect_no_log_entry();

  cbt_diagnostics.post_prefix = UINT64_MAX;
  circ->cbt_expiry_reported = 0;
  circuit_build_times_note_expiry(circ);
  tt_u64_op(cbt_diagnostics.post_prefix, OP_EQ, UINT64_MAX);
 done:
  if (circ)
    circ->base_.n_chan = NULL;
  circuit_free_(TO_CIRCUIT(circ));
  circuit_build_times_free_timeouts(cbt);
  teardown_capture_of_logs();
  UNMOCK(tor_gettimeofday);
}

/* Stalled bootstrap: liveness permits abandonment only in the padding case,
 * but both cases must report a zero-hop terminal failure exactly once. */
static const char *
mock_cbt_describe_peer(channel_t *chan)
{
  (void)chan;
  return "192.0.2.123:23456";
}

static void
test_circuitstats_stalled_expiry(void *arg)
{
  circuit_build_times_t *cbt = get_circuit_build_times_mutable();
  origin_circuit_t *circ = NULL;
  struct timeval start = { 1000, 0 };
  channel_t channel;
  (void)arg;
  memset(&channel, 0, sizeof(channel));
  channel.state = CHANNEL_STATE_OPEN;
  circuitbuild_running_unit_tests();
  circuit_build_times_init(cbt);
  MOCK(tor_gettimeofday, mock_cbt_gettimeofday);
  MOCK(channel_describe_peer, mock_cbt_describe_peer);
  MOCK(circuit_mark_for_close_, mock_circuit_mark_for_close);
  setup_full_capture_of_logs(LOG_NOTICE);
  for (int live = 0; live < 2; ++live) {
    channel.is_bad_for_new_circs = 0;
    memset(&cbt_diagnostics, 0, sizeof(cbt_diagnostics));
    circuit_build_times_reset(cbt);
    for (int i = 0; i < CBT_DEFAULT_MIN_CIRCUITS_TO_OBSERVE; ++i)
      circuit_build_times_add_time(cbt, 500);
    cbt->timeout_ms = 465;
    cbt->close_ms = 60000;
    cbt->liveness.nonlive_timeouts = !live;
    cbt->liveness.network_last_live = live ? 1060 : 0;
    circ = build_unopened_fourhop(start);
    circ->base_.state = CIRCUIT_STATE_BUILDING;
    circ->base_.n_chan = &channel;
    circ->cpath->state = CPATH_STATE_AWAITING_KEYS;
    cbt_test_now.tv_sec = 1002;
    cbt_test_now.tv_usec = 0;
    circuit_expire_building();
    tt_assert(circ->relaxed_timeout);
    tt_int_op(channel.is_bad_for_new_circs, OP_EQ, 0);
    tt_u64_op(cbt_diagnostics.prefix_hops[0], OP_EQ, 0);
    cbt_test_now.tv_sec = 1061;
    circuit_expire_building(); /* Repurpose only, no terminal expiry yet. */
    tt_int_op(channel.is_bad_for_new_circs, OP_EQ, 0);
    tt_u64_op(cbt_diagnostics.prefix_hops[0], OP_EQ, 0);
    mock_clean_saved_logs();
    circuit_expire_building();
    tt_int_op(channel.is_bad_for_new_circs, OP_EQ, 1);
    expect_single_log_msg("prefix_hops_0=1 prefix_hops_1=0 prefix_hops_2=0 "
                          "post_prefix=0 open_channel=1 other_channel=0");
    if (live) {
      expect_log_msg_containing("abandoned=1");
    } else {
      expect_log_msg_containing("abandoned=0");
    }
    circ->base_.n_chan = NULL;
    circuit_free_(TO_CIRCUIT(circ));
    circ = NULL;
  }
 done:
  if (circ)
    circ->base_.n_chan = NULL;
  circuit_free_(TO_CIRCUIT(circ));
  circuit_build_times_free_timeouts(cbt);
  teardown_capture_of_logs();
  UNMOCK(tor_gettimeofday);
  UNMOCK(channel_describe_peer);
  UNMOCK(circuit_mark_for_close_);
}

/* A short learned measurement deadline must still account for a retained
 * first-hop probe, including when liveness rejects the abandoned sample. */
static void
test_circuitstats_probe_accounting(void *arg)
{
  circuit_build_times_t *cbt = get_circuit_build_times_mutable();
  origin_circuit_t *circ = NULL, *opened = NULL;
  const struct timeval start = { 1000, 0 };
  channel_t channel = { 0 };
  (void)arg;
  circuitbuild_running_unit_tests();
  circuit_build_times_init(cbt);
  MOCK(tor_gettimeofday, mock_cbt_gettimeofday);
  MOCK(channel_describe_peer, mock_cbt_describe_peer);
  MOCK(circuit_mark_for_close_, mock_cbt_mark_closed);
  setup_full_capture_of_logs(LOG_NOTICE);
  opened = add_opened_threehop();

  for (int live = 0; live < 2; ++live) {
    for (int completes = 0; completes < 2; ++completes) {
      circuit_build_times_reset(cbt);
      for (int i = 0; i < CBT_DEFAULT_MIN_CIRCUITS_TO_OBSERVE; ++i) {
        circuit_build_times_add_time(cbt, 500);
      }
      const int samples = cbt->total_build_times;
      cbt->timeout_ms = 1000;
      cbt->close_ms = 5000;
      cbt->liveness.nonlive_timeouts = 0;
      cbt->liveness.network_last_live = live ? 1060 : 0;
      channel.state = CHANNEL_STATE_OPEN;
      channel.is_bad_for_new_circs = 0;
      circ = build_unopened_fourhop(start);
      circ->base_.state = CIRCUIT_STATE_BUILDING;
      circ->base_.n_chan = &channel;
      circ->first_hop_success_count_at_create =
        channel.first_hop_success_count;
      circ->cpath->state = CPATH_STATE_AWAITING_KEYS;
      cbt_test_now = start;
      cbt_test_now.tv_sec += 3;
      circuit_expire_building();
      tt_int_op(circ->base_.purpose, OP_EQ, CIRCUIT_PURPOSE_C_MEASURE_TIMEOUT);
      tt_int_op(cbt->num_circ_closed, OP_EQ, 0);
      cbt_test_now.tv_sec = start.tv_sec + 5;
      circuit_expire_building();
      tt_assert(!circ->base_.marked_for_close);
      tt_assert(circ->cbt_measurement_closed);
      tt_assert(!circuit_build_times_circ_can_record(circ));
      tt_int_op(cbt->num_circ_closed, OP_EQ, 1);
      tt_int_op(cbt->total_build_times, OP_EQ, samples + live);
      tt_int_op(cbt->liveness.nonlive_timeouts, OP_EQ, !live);
      tt_int_op(channel.is_bad_for_new_circs, OP_EQ, 0);

      mock_clean_saved_logs();
      cbt_test_now.tv_sec = start.tv_sec + 59;
      circuit_expire_building();
      tt_assert(!circ->base_.marked_for_close);
      if (completes) {
        /* Even if the network returns and the prefix completes, its
         * observation ended at the close deadline. */
        cbt->liveness.nonlive_timeouts = 0;
        cbt->liveness.network_last_live = cbt_test_now.tv_sec;
        channel.first_hop_success_count++;
        circ->cpath->state = CPATH_STATE_OPEN;
        circ->cpath->next->state = CPATH_STATE_OPEN;
        circ->cpath->next->next->state = CPATH_STATE_OPEN;
        circuit_build_times_handle_completed_hop(circ);
      }
      cbt_test_now.tv_sec = start.tv_sec + 60;
      circuit_expire_building();
      circuit_expire_building();
      tt_assert(circ->base_.marked_for_close);
      tt_int_op(cbt->num_circ_closed, OP_EQ, 1);
      tt_int_op(cbt->total_build_times, OP_EQ, samples + live);
      tt_int_op(channel.is_bad_for_new_circs, OP_EQ, !completes);
      expect_no_log_msg_containing("Assuming clock jump");
      circ->base_.n_chan = NULL;
      circuit_free_(TO_CIRCUIT(circ));
      circ = NULL;
    }
  }
 done:
  if (circ) {
    circ->base_.n_chan = NULL;
  }
  circuit_free_(TO_CIRCUIT(circ));
  circuit_free_(TO_CIRCUIT(opened));
  circuit_build_times_free_timeouts(cbt);
  teardown_capture_of_logs();
  UNMOCK(tor_gettimeofday);
  UNMOCK(channel_describe_peer);
  UNMOCK(circuit_mark_for_close_);
}

/* Use real one-hop selection with a pending request, but intercept closing
 * the request so repeated notifications remain observable. */
static smartlist_t *cbt_pending_connections;
static unsigned cbt_onehop_failures;

static smartlist_t *
mock_cbt_connections(void)
{
  return cbt_pending_connections;
}

static void
mock_cbt_fail_request(entry_connection_t *conn, int reason, int line,
                      const char *file)
{
  (void)conn;
  (void)reason;
  (void)line;
  (void)file;
  ++cbt_onehop_failures;
}

/* A channel that opened and then failed must release directory requests
 * awaiting CREATED. Exercise the real channel, circuit-map, and close
 * paths. */
static void
test_circuitstats_open_channel_failure(void *arg)
{
  channel_t channel = { 0 };
  origin_circuit_t *circ = NULL;
  entry_connection_t *request = NULL;
  const struct timeval start = { 1000, 0 };
  (void)arg;
  channel_init(&channel);
  scheduler_init();
  channel.cmux = circuitmux_alloc();
  circuitmux_set_policy(channel.cmux, &ewma_policy);
  channel.has_been_open = 1;
  channel.reason_for_closing = CHANNEL_CLOSE_FOR_ERROR;
  MOCK(assert_circuit_ok, mock_cbt_assert_circuit_ok);
  MOCK(channel_describe_peer, mock_cbt_describe_peer);
  MOCK(get_connection_array, mock_cbt_connections);
  MOCK(connection_mark_unattached_ap_, mock_cbt_fail_request);
  tor_init_connection_lists();
  cbt_pending_connections = smartlist_new();
  request = entry_connection_new(CONN_TYPE_AP, AF_INET);
  ENTRY_TO_CONN(request)->state = AP_CONN_STATE_CIRCUIT_WAIT;
  request->want_onehop = 1;
  request->chosen_exit_name = tor_strdup(
      "$4242424242424242424242424242424242424242");
  smartlist_add(cbt_pending_connections, ENTRY_TO_CONN(request));

  for (int firsthop_open = 0; firsthop_open < 2; ++firsthop_open) {
    channel.state = CHANNEL_STATE_OPEN;
    channel_register(&channel);
    cbt_onehop_failures = 0;
    circ = build_unopened_fourhop(start);
    circ->base_.state = CIRCUIT_STATE_BUILDING;
    circ->cpath->state = firsthop_open ? CPATH_STATE_OPEN :
                                       CPATH_STATE_AWAITING_KEYS;
    memset(circ->cpath->extend_info->identity_digest, 0x42, DIGEST_LEN);
    circuit_set_n_circid_chan(TO_CIRCUIT(circ), 42, &channel);
    channel.state = CHANNEL_STATE_CLOSING;
    channel_closed(&channel);
    tt_assert(circ->base_.marked_for_close);
    tt_ptr_op(circ->base_.n_chan, OP_EQ, NULL);
    tt_uint_op(cbt_onehop_failures, OP_EQ, !firsthop_open);
    tt_int_op(channel.is_bad_for_new_circs, OP_EQ, 0);
    circ = NULL;
    circuit_close_all_marked();
    channel_closed(&channel);
    tt_uint_op(cbt_onehop_failures, OP_EQ, !firsthop_open);
    channel_unregister(&channel);
  }
 done:
  circuit_free_(TO_CIRCUIT(circ));
  channel_unregister(&channel);
  circuitmux_free(channel.cmux);
  scheduler_free_all();
  smartlist_clear(cbt_pending_connections);
  connection_free_(ENTRY_TO_CONN(request));
  smartlist_free(cbt_pending_connections);
  cbt_pending_connections = NULL;
  UNMOCK(assert_circuit_ok);
  UNMOCK(channel_describe_peer);
  UNMOCK(get_connection_array);
  UNMOCK(connection_mark_unattached_ap_);
}

/* These counters must report even without any three-hop CBT observations. */
static void
test_circuitstats_channel_diagnostics(void *arg)
{
  (void)arg;
  memset(&cbt_diagnostics, 0, sizeof(cbt_diagnostics));
  setup_full_capture_of_logs(LOG_NOTICE);
  circuit_build_times_note_channel_timeout(true);
  circuit_build_times_report_diagnostics(1000);
  expect_single_log_msg("channels_retired=1 firsthop_preserved=0");
  tt_u64_op(cbt_diagnostics.channels_retired, OP_EQ, 0);
  mock_clean_saved_logs();
  circuit_build_times_note_channel_timeout(false);
  circuit_build_times_report_diagnostics(1299);
  expect_no_log_entry();
  circuit_build_times_report_diagnostics(1300);
  expect_single_log_msg("channels_retired=0 firsthop_preserved=1");
  tt_u64_op(cbt_diagnostics.firsthop_preserved, OP_EQ, 0);
  mock_clean_saved_logs();
  circuit_build_times_report_diagnostics(1600);
  expect_no_log_entry();
  cbt_diagnostics.channels_retired = UINT64_MAX;
  cbt_diagnostics.firsthop_preserved = UINT64_MAX;
  circuit_build_times_note_channel_timeout(true);
  circuit_build_times_note_channel_timeout(false);
  tt_u64_op(cbt_diagnostics.channels_retired, OP_EQ, UINT64_MAX);
  tt_u64_op(cbt_diagnostics.firsthop_preserved, OP_EQ, UINT64_MAX);
 done:
  teardown_capture_of_logs();
}

/* Shared transport and pending-request fixture for channel expiry tests. */
static void
setup_channel_expiry_test(void)
{
  circuitbuild_running_unit_tests();
  circuit_build_times_init(get_circuit_build_times_mutable());
  MOCK(tor_gettimeofday, mock_cbt_gettimeofday);
  MOCK(channel_describe_peer, mock_cbt_describe_peer);
  MOCK(circuit_mark_for_close_, mock_circuit_mark_for_close);
  MOCK(get_connection_array, mock_cbt_connections);
  MOCK(connection_mark_unattached_ap_, mock_cbt_fail_request);
  tor_init_connection_lists();
  cbt_pending_connections = smartlist_new();
  entry_connection_t *request = entry_connection_new(CONN_TYPE_AP, AF_INET);
  ENTRY_TO_CONN(request)->state = AP_CONN_STATE_CIRCUIT_WAIT;
  request->want_onehop = 1;
  request->chosen_exit_name = tor_strdup(
      "$4242424242424242424242424242424242424242");
  smartlist_add(cbt_pending_connections, ENTRY_TO_CONN(request));
}

static void
teardown_channel_expiry_test(void)
{
  connection_free_(smartlist_get(cbt_pending_connections, 0));
  smartlist_free(cbt_pending_connections);
  cbt_pending_connections = NULL;
  circuit_build_times_free_timeouts(get_circuit_build_times_mutable());
  UNMOCK(tor_gettimeofday);
  UNMOCK(channel_describe_peer);
  UNMOCK(circuit_mark_for_close_);
  UNMOCK(get_connection_array);
  UNMOCK(connection_mark_unattached_ap_);
}

static channel_t *cbt_bootstrap_channel;

static channel_t *
mock_cbt_get_channel(const char *rsa_id,
                     const struct ed25519_public_key_t *ed_id,
                     const tor_addr_t *ipv4, const tor_addr_t *ipv6,
                     bool origin, const char **msg, int *launch)
{
  (void)rsa_id;
  (void)ed_id;
  (void)ipv4;
  (void)ipv6;
  (void)origin;
  *msg = "test channel";
  *launch = 0;
  return cbt_bootstrap_channel;
}

static int
mock_cbt_create_send_failure(circuit_t *circ, const create_cell_t *cell,
                             int relayed)
{
  (void)cell;
  (void)relayed;
  /* Match circuit-ID exhaustion: delivery drops the channel pointer. */
  circ->n_chan = NULL;
  return -1;
}

/* A failed first CREATE must release directory requests both when reusing
 * an open channel and when finishing a newly opened connection. */
static void
test_circuitstats_create_send_failure(void *arg)
{
  origin_circuit_t *circ = NULL;
  extend_info_t *hop = NULL;
  const struct timeval start = { 1000, 0 };
  channel_t channel = { 0 };
  tor_addr_t addr;
  (void)arg;
  setup_channel_expiry_test();
  MOCK(channel_get_for_extend, mock_cbt_get_channel);
  MOCK(circuit_deliver_create_cell, mock_cbt_create_send_failure);
  channel.state = CHANNEL_STATE_OPEN;
  channel.global_identifier = 1;
  channel.is_canonical = mock_cbt_channel_is_canonical;
  memset(channel.identity_digest, 0x42, DIGEST_LEN);
  cbt_bootstrap_channel = &channel;
  tor_addr_parse(&addr, "192.0.2.123");
  hop = extend_info_new("test", channel.identity_digest, NULL, NULL,
                        &addr, 9001, NULL, false);

  for (int pending = 0; pending < 2; ++pending) {
    cbt_onehop_failures = 0;
    marked_for_close = 0;
    circ = new_test_origin_circuit(false, start, 1, &hop);
    circ->build_state->onehop_tunnel = 1;
    if (pending) {
      circ->base_.n_hop = extend_info_dup(hop);
      circuit_set_state(TO_CIRCUIT(circ), CIRCUIT_STATE_CHAN_WAIT);
      circuit_n_chan_done(&channel, 1);
      tt_int_op(marked_for_close, OP_EQ, 1);
    } else {
      tt_int_op(circuit_handle_first_hop(circ), OP_EQ,
                -END_CIRC_REASON_RESOURCELIMIT);
    }
    tt_uint_op(cbt_onehop_failures, OP_EQ, 1);
    tt_int_op(channel.is_bad_for_new_circs, OP_EQ, 0);
    tt_int_op(circ->cpath->state, OP_EQ, CPATH_STATE_CLOSED);
    /* Generic cleanup must not repeat the notification. */
    circuit_build_failed(circ);
    tt_uint_op(cbt_onehop_failures, OP_EQ, 1);
    circuit_free_(TO_CIRCUIT(circ));
    circ = NULL;
  }
 done:
  if (circ) {
    circ->base_.n_chan = NULL;
  }
  circuit_free_(TO_CIRCUIT(circ));
  extend_info_free(hop);
  cbt_bootstrap_channel = NULL;
  UNMOCK(channel_get_for_extend);
  UNMOCK(circuit_deliver_create_cell);
  teardown_channel_expiry_test();
}

static create_cell_t cbt_last_create;

static int
mock_cbt_deliver_create(circuit_t *circ, const create_cell_t *cell,
                        int relayed)
{
  (void)circ;
  (void)relayed;
  cbt_last_create = *cell;
  return 0;
}

/* Use the real pending queue, event callback and circuit launch during
 * bootstrap. Conversion and subsequent retry expiry must both wake it. */
static void
test_circuitstats_bootstrap_probe_retry(void *arg)
{
  origin_circuit_t *probe = NULL, *retry = NULL;
  entry_connection_t *request = NULL;
  channel_t channel = { 0 };
  created_cell_t reply = { 0 };
  create_cell_t probe_create;
  const struct timeval no_delay = { 0, 0 };
  uint8_t keys[128];
  (void)arg;
  circuitbuild_running_unit_tests();
  tor_init_connection_lists();
  get_options_mutable()->LearnCircuitBuildTimeout = 0;
  get_options_mutable()->CircuitBuildTimeout = 5;
  circuit_build_times_init(get_circuit_build_times_mutable());
  /* CBT unit-test mode ignores the configured initial timeout. */
  get_circuit_build_times_mutable()->timeout_ms = 5000;
  get_circuit_build_times_mutable()->close_ms = 5000;
  MOCK(tor_gettimeofday, mock_cbt_gettimeofday);
  MOCK(channel_describe_peer, mock_cbt_describe_peer);
  MOCK(channel_get_for_extend, mock_cbt_get_channel);
  MOCK(circuit_deliver_create_cell, mock_cbt_deliver_create);
  MOCK(circuit_mark_for_close_, mock_cbt_mark_closed);
  channel.state = CHANNEL_STATE_OPEN;
  channel.global_identifier = 1;
  cbt_bootstrap_channel = &channel;
  cbt_test_now.tv_sec = time(NULL);
  cbt_test_now.tv_usec = 0;
  tt_assert(!router_have_minimum_dir_info());
  request = entry_connection_new(CONN_TYPE_AP, AF_INET);
  ENTRY_TO_CONN(request)->state = AP_CONN_STATE_CIRCUIT_WAIT;
  request->want_onehop = request->use_begindir = 1;
  request->chosen_exit_name = tor_strdup(
      "$4242424242424242424242424242424242424242");
  request->socks_request->command = SOCKS_COMMAND_CONNECT;
  strlcpy(request->socks_request->address, "192.0.2.123",
          sizeof(request->socks_request->address));
  request->socks_request->port = 9001;
  request->original_dest_address = tor_strdup("192.0.2.123");
  connection_ap_mark_as_pending_circuit(request);
  tor_libevent_exit_loop_after_delay(tor_libevent_get_base(), &no_delay);
  tor_libevent_run_event_loop(tor_libevent_get_base(), 1);
  tt_int_op(request->num_circuits_launched, OP_EQ, 1);
  probe = circuit_get_best(request, 0, CIRCUIT_PURPOSE_C_GENERAL, 0, 1);
  tt_assert(probe);
  probe_create = cbt_last_create;

  cbt_test_now.tv_sec += 5;
  circuit_expire_building();
  tt_int_op(probe->base_.purpose, OP_EQ, CIRCUIT_PURPOSE_C_MEASURE_TIMEOUT);
  tt_int_op(request->num_circuits_launched, OP_EQ, 1);
  tor_libevent_exit_loop_after_delay(tor_libevent_get_base(), &no_delay);
  tor_libevent_run_event_loop(tor_libevent_get_base(), 1);
  tt_int_op(request->num_circuits_launched, OP_EQ, 2);
  retry = circuit_get_best(request, 0, CIRCUIT_PURPOSE_C_GENERAL, 0, 1);
  tt_assert(retry);
  tt_ptr_op(retry, OP_NE, probe);

  /* The next attempt expires while the original probe remains pending. */
  cbt_test_now.tv_sec += 5;
  circuit_expire_building();
  tt_assert(retry->base_.marked_for_close);
  tt_assert(!probe->base_.marked_for_close);
  tor_libevent_exit_loop_after_delay(tor_libevent_get_base(), &no_delay);
  tor_libevent_run_event_loop(tor_libevent_get_base(), 1);
  tt_int_op(request->num_circuits_launched, OP_EQ, 3);

  /* A late valid response completes and closes the probe. Its request has
   * already retried without directory information or a periodic rescan. */
  cbt_test_now.tv_sec += 1;
  reply.cell_type = CELL_CREATED_FAST;
  reply.handshake_len = CREATED_FAST_LEN;
  tt_int_op(fast_server_handshake(probe_create.onionskin, reply.reply,
                                  keys, sizeof(keys)), OP_EQ, 0);
  tt_int_op(circuit_finish_handshake(probe, &reply), OP_EQ, 0);
  tt_int_op(circuit_send_next_onion_skin(probe), OP_EQ, 0);
  tt_assert(probe->base_.marked_for_close);
  tt_assert(!ENTRY_TO_CONN(request)->marked_for_close);
  tt_int_op(channel.is_bad_for_new_circs, OP_EQ, 0);
  tt_assert(!router_have_minimum_dir_info());
  tt_assert(circuit_get_best(request, 0, CIRCUIT_PURPOSE_C_GENERAL, 0, 1));
 done:
  if (request) {
    connection_ap_mark_as_non_pending_circuit(request);
    connection_free_(ENTRY_TO_CONN(request));
  }
  SMARTLIST_FOREACH_BEGIN(circuit_get_global_origin_circuit_list(),
                         origin_circuit_t *, circ) {
    circ->base_.n_chan = NULL;
  } SMARTLIST_FOREACH_END(circ);
  circuit_free_all();
  circuit_build_times_free_timeouts(get_circuit_build_times_mutable());
  cbt_bootstrap_channel = NULL;
  UNMOCK(tor_gettimeofday);
  UNMOCK(channel_describe_peer);
  UNMOCK(channel_get_for_extend);
  UNMOCK(circuit_deliver_create_cell);
  UNMOCK(circuit_mark_for_close_);
}

/* Retirement eligibility is independent of CBT statistical eligibility. */
static void
test_circuitstats_channel_retirement(void *arg)
{
  circuit_build_times_t *cbt = get_circuit_build_times_mutable();
  origin_circuit_t *circ = NULL;
  const struct timeval start = { 1000, 0 };
  channel_t channel;
  (void)arg;
  setup_channel_expiry_test();

  enum {
    FIRST_HOP, EXCLUDED_OBSERVATION, LATER_HOP,
    CONNECTION_WAIT, CLOSING_CHANNEL, MISSING_CHANNEL,
    RETIRED_CHANNEL, RECEIVED_DESTROY, DIRECTORY_ATTEMPT, CONCURRENT_PROGRESS,
    N_RETIREMENT_CASES
  };
  for (int adaptive = 0; adaptive < 2; ++adaptive) {
    get_options_mutable()->LearnCircuitBuildTimeout = adaptive;
    get_options_mutable()->CircuitBuildTimeout = 60;
    for (int kind = 0; kind < N_RETIREMENT_CASES; ++kind) {
      memset(&channel, 0, sizeof(channel));
      channel.state = CHANNEL_STATE_OPEN;
      cbt_onehop_failures = 0;
      circuit_build_times_reset(cbt);
      if (adaptive) {
        for (int i = 0; i < CBT_DEFAULT_MIN_CIRCUITS_TO_OBSERVE; ++i)
          circuit_build_times_add_time(cbt, 500);
      }
      cbt->timeout_ms = 1000;
      cbt->close_ms = 60000;
      circ = build_unopened_fourhop(start);
      circ->base_.state = CIRCUIT_STATE_BUILDING;
      circ->base_.purpose = adaptive ? CIRCUIT_PURPOSE_C_MEASURE_TIMEOUT :
                                      CIRCUIT_PURPOSE_C_GENERAL;
      circ->base_.n_chan = &channel;
      memset(circ->cpath->extend_info->identity_digest, 0x42, DIGEST_LEN);
      circ->cpath->state = CPATH_STATE_AWAITING_KEYS;
      /* Normal first-hop expiry, excluded observation, later-hop expiry,
       * connection wait, closing/missing/retired channel, DESTROY, one-hop. */
      if (kind == EXCLUDED_OBSERVATION)
        circ->cbt_observation_invalidated = 1;
      if (kind == LATER_HOP)
        circ->cpath->state = CPATH_STATE_OPEN;
      if (kind == CONNECTION_WAIT)
        circ->cpath->state = CPATH_STATE_CLOSED;
      if (kind == CLOSING_CHANNEL)
        channel.state = CHANNEL_STATE_CLOSING;
      if (kind == MISSING_CHANNEL)
        circ->base_.n_chan = NULL;
      if (kind == RETIRED_CHANNEL)
        channel.is_bad_for_new_circs = 1;
      if (kind == RECEIVED_DESTROY)
        circ->base_.received_destroy = 1;
      if (kind == DIRECTORY_ATTEMPT) {
        circ->build_state->onehop_tunnel = 1;
        circ->build_state->desired_path_len = 1;
      }
      /* Existing successes do not protect a new attempt; only progress
       * since its snapshot does. A subsequent stalled attempt can recover. */
      channel.first_hop_success_count = 7;
      circ->first_hop_success_count_at_create =
        kind == CONCURRENT_PROGRESS ? 6 : 7;
      cbt_test_now = start;
      cbt_test_now.tv_sec += 120;
      marked_for_close = 0;
      circuit_expire_building();

      tt_int_op(marked_for_close, OP_EQ, kind != CONNECTION_WAIT);
      const bool retire = kind == FIRST_HOP ||
        kind == EXCLUDED_OBSERVATION || kind == DIRECTORY_ATTEMPT;
      tt_int_op(channel.is_bad_for_new_circs, OP_EQ,
                retire || kind == RETIRED_CHANNEL);
      tt_uint_op(cbt_onehop_failures, OP_EQ, retire);
      /* Repeated expiry/cleanup cannot abort new requests on this channel. */
      circuit_expire_building();
      circuit_build_failed(circ);
      tt_uint_op(cbt_onehop_failures, OP_EQ, retire);
      circ->base_.n_chan = NULL;
      circuit_free_(TO_CIRCUIT(circ));
      circ = NULL;
    }
  }

 done:
  if (circ)
    circ->base_.n_chan = NULL;
  circuit_free_(TO_CIRCUIT(circ));
  teardown_channel_expiry_test();
}

/* Recovery deadlines survive short usage timeouts and CBT resets. */
static void
test_circuitstats_channel_deadlines(void *arg)
{
  circuit_build_times_t *cbt = get_circuit_build_times_mutable();
  origin_circuit_t *circ = NULL, *opened = NULL;
  const struct timeval start = { 1000, 0 };
  channel_t channel;
  (void)arg;
  setup_channel_expiry_test();

  /* An existing usable circuit disables the bootstrap timeout relaxation.
   * In particular, one-hop directory requests must not retire a working
   * channel at a subsecond usage timeout. Keep the same attempt alive. */
  opened = add_opened_threehop();
  enum {
    DIRECTORY_TIMEOUT, LONG_CLOSE_TIMEOUT, FIXED_TIMEOUT,
    INVALIDATED_MEASUREMENT, ADAPTIVE_TIMEOUT,
    DIRECTORY_COMPLETES, FIRST_HOP_COMPLETES, INSUFFICIENT_HISTORY,
    N_DEADLINE_CASES
  };
  for (int kind = 0; kind < N_DEADLINE_CASES; ++kind) {
    const bool onehop = kind <= FIXED_TIMEOUT ||
                        kind == DIRECTORY_COMPLETES;
    const int deadline = kind == LONG_CLOSE_TIMEOUT ? 120 : 60;
    get_options_mutable()->LearnCircuitBuildTimeout = kind != FIXED_TIMEOUT;
    get_options_mutable()->CircuitBuildTimeout = 1;
    circuit_build_times_reset(cbt);
    if (kind != FIXED_TIMEOUT && kind != INSUFFICIENT_HISTORY) {
      for (int i = 0; i < CBT_DEFAULT_MIN_CIRCUITS_TO_OBSERVE; ++i)
        circuit_build_times_add_time(cbt, 500);
    }
    cbt->timeout_ms = 465;
    cbt->close_ms = kind == FIXED_TIMEOUT ||
                    kind == INVALIDATED_MEASUREMENT ||
                    kind == INSUFFICIENT_HISTORY ? 465 :
                    deadline * 1000;
    memset(&channel, 0, sizeof(channel));
    channel.state = CHANNEL_STATE_OPEN;
    cbt_onehop_failures = 0;
    if (onehop) {
      extend_info_t fakehop = { 0 };
      extend_info_t *path[] = { &fakehop };
      circ = new_test_origin_circuit(false, start, 1, path);
      circ->build_state->onehop_tunnel = 1;
    } else {
      circ = build_unopened_fourhop(start);
    }
    circ->base_.state = CIRCUIT_STATE_BUILDING;
    circ->base_.n_chan = &channel;
    circ->cpath->state = CPATH_STATE_AWAITING_KEYS;
    memset(circ->cpath->extend_info->identity_digest, 0x42, DIGEST_LEN);
    if (kind == INVALIDATED_MEASUREMENT) {
      circ->cbt_observation_invalidated = 1;
      circ->base_.purpose = CIRCUIT_PURPOSE_C_MEASURE_TIMEOUT;
    }
    marked_for_close = 0;
    cbt_test_now = start;
    cbt_test_now.tv_sec += 2;
    circuit_expire_building();
    tt_int_op(marked_for_close, OP_EQ, 0);
    tt_int_op(channel.is_bad_for_new_circs, OP_EQ, 0);
    tt_uint_op(cbt_onehop_failures, OP_EQ, 0);
    if (kind == ADAPTIVE_TIMEOUT || kind == FIRST_HOP_COMPLETES) {
      tt_int_op(circ->base_.purpose, OP_EQ,
                CIRCUIT_PURPOSE_C_MEASURE_TIMEOUT);
      tt_int_op(cbt->num_circ_timeouts, OP_EQ, 1);
    }
    /* A first hop that answers before the deadline saves the channel,
     * even if a later hop subsequently expires. */
    if (kind == DIRECTORY_COMPLETES || kind == FIRST_HOP_COMPLETES) {
      circ->cpath->state = CPATH_STATE_OPEN;
      if (onehop)
        circ->base_.state = CIRCUIT_STATE_OPEN;
    }
    cbt_test_now.tv_sec = start.tv_sec + deadline - 1;
    cbt_test_now.tv_usec = 999000;
    circuit_expire_building();
    tt_int_op(marked_for_close, OP_EQ, 0);
    tt_int_op(channel.is_bad_for_new_circs, OP_EQ, 0);
    tt_uint_op(cbt_onehop_failures, OP_EQ, 0);
    tt_int_op(cbt->num_circ_closed, OP_EQ, 0);
    cbt_test_now.tv_sec = start.tv_sec + deadline;
    cbt_test_now.tv_usec = 0;
    circuit_expire_building();
    tt_int_op(marked_for_close, OP_EQ, kind != DIRECTORY_COMPLETES);
    const bool retire = kind != DIRECTORY_COMPLETES &&
                        kind != FIRST_HOP_COMPLETES;
    tt_int_op(channel.is_bad_for_new_circs, OP_EQ, retire);
    tt_uint_op(cbt_onehop_failures, OP_EQ, retire);
    circ->base_.n_chan = NULL;
    circuit_free_(TO_CIRCUIT(circ));
    circ = NULL;
  }

 done:
  if (circ)
    circ->base_.n_chan = NULL;
  circuit_free_(TO_CIRCUIT(circ));
  circuit_free_(TO_CIRCUIT(opened));
  teardown_channel_expiry_test();
}

/* Measurements retain their sampling window, but only the oldest eligible
 * first-hop probe on each channel survives beyond the close deadline. */
static void
test_circuitstats_measurement_probe_limit(void *arg)
{
  circuit_build_times_t *cbt = get_circuit_build_times_mutable();
  origin_circuit_t *circs[4] = { NULL }, *opened = NULL;
  channel_t channels[2];
  const struct timeval start = { 1000, 0 };
  (void)arg;
  setup_channel_expiry_test();
  UNMOCK(circuit_mark_for_close_);
  MOCK(circuit_mark_for_close_, mock_cbt_mark_closed);
  opened = add_opened_threehop();
  get_options_mutable()->LearnCircuitBuildTimeout = 1;

  for (int reverse = 0; reverse < 2; ++reverse) {
    for (int kind = 0; kind < 4; ++kind) {
      /* Different ages, equal ages, an older probe with concurrent success,
       * and an older probe that already received DESTROY. */
      memset(channels, 0, sizeof(channels));
      channels[0].state = channels[1].state = CHANNEL_STATE_OPEN;
      channels[0].first_hop_success_count = 1;
      cbt_onehop_failures = 0;
      circuit_build_times_reset(cbt);
      for (int i = 0; i < CBT_DEFAULT_MIN_CIRCUITS_TO_OBSERVE; ++i)
        circuit_build_times_add_time(cbt, 500);
      const int samples = cbt->total_build_times;
      cbt->timeout_ms = 1000;
      cbt->close_ms = 5000;
      cbt->liveness.nonlive_timeouts = 0;
      cbt->liveness.network_last_live = 1060;
      for (int n = 0; n < 4; ++n) {
        const int i = reverse ? 3-n : n;
        struct timeval began = start;
        if (kind != 1 && i != 0)
          ++began.tv_sec;
        circs[i] = build_unopened_fourhop(began);
        circs[i]->global_identifier = i+1;
        circs[i]->base_.state = CIRCUIT_STATE_BUILDING;
        circs[i]->base_.purpose = CIRCUIT_PURPOSE_C_GENERAL;
        circs[i]->base_.n_chan = &channels[i == 3];
        circs[i]->first_hop_success_count_at_create =
          channels[i == 3].first_hop_success_count;
        circs[i]->cpath->state = CPATH_STATE_AWAITING_KEYS;
        memset(circs[i]->cpath->extend_info->identity_digest,
               0x42, DIGEST_LEN);
      }
      if (kind == 2)
        circs[0]->first_hop_success_count_at_create = 0;
      if (kind == 3)
        circs[0]->base_.received_destroy = 1;

      /* All four attempts must get their full statistical lifetime. */
      cbt_test_now = start;
      cbt_test_now.tv_sec += 4;
      circuit_expire_building();
      for (int i = 0; i < 4; ++i) {
        tt_assert(!circs[i]->base_.marked_for_close);
        tt_int_op(circs[i]->base_.purpose, OP_EQ,
                  CIRCUIT_PURPOSE_C_MEASURE_TIMEOUT);
        tt_assert(!circs[i]->cbt_measurement_closed);
      }
      tt_int_op(cbt->num_circ_timeouts, OP_EQ, 4);
      tt_int_op(cbt->num_circ_closed, OP_EQ, 0);

      cbt_test_now.tv_sec = start.tv_sec + 8;
      circuit_expire_building();
      const int survivor = kind >= 2 ? 1 : 0;
      for (int i = 0; i < 4; ++i) {
        tt_int_op(!!circs[i]->base_.marked_for_close, OP_EQ,
                  i != survivor && i != 3);
        tt_assert(circs[i]->cbt_measurement_closed);
      }
      tt_int_op(cbt->num_circ_closed, OP_EQ, 4);
      tt_int_op(cbt->total_build_times, OP_EQ, samples + 4);
      tt_int_op(channels[0].is_bad_for_new_circs, OP_EQ, 0);
      tt_int_op(channels[1].is_bad_for_new_circs, OP_EQ, 0);
      tt_uint_op(cbt_onehop_failures, OP_EQ, 0);

      /* Repeated expiry cannot duplicate observations or move the oldest
       * eligible probe's recovery deadline to a newer attempt's deadline. */
      cbt->close_ms = 5000;
      cbt_test_now.tv_sec =
        circs[survivor]->base_.timestamp_began.tv_sec + 59;
      circuit_expire_building();
      tt_assert(!circs[survivor]->base_.marked_for_close);
      tt_int_op(channels[0].is_bad_for_new_circs, OP_EQ, 0);
      ++cbt_test_now.tv_sec;
      circuit_expire_building();
      tt_assert(circs[survivor]->base_.marked_for_close);
      tt_int_op(channels[0].is_bad_for_new_circs, OP_EQ, 1);
      tt_int_op(cbt->num_circ_timeouts, OP_EQ, 4);
      tt_int_op(cbt->num_circ_closed, OP_EQ, 4);
      tt_int_op(cbt->total_build_times, OP_EQ, samples + 4);
      for (int i = 0; i < 4; ++i) {
        circs[i]->base_.n_chan = NULL;
        circuit_free_(TO_CIRCUIT(circs[i]));
        circs[i] = NULL;
      }
    }
  }

 done:
  for (int i = 0; i < 4; ++i) {
    if (circs[i])
      circs[i]->base_.n_chan = NULL;
    circuit_free_(TO_CIRCUIT(circs[i]));
  }
  circuit_free_(TO_CIRCUIT(opened));
  teardown_channel_expiry_test();
}

/* Generic cancellation must preserve the channel and pending requests. */
static void
test_circuitstats_channel_cancellation(void *arg)
{
  origin_circuit_t *circ = NULL;
  const struct timeval start = { 1000, 0 };
  channel_t channel;
  const int cancel_reasons[] = {
    END_CIRC_REASON_INTERNAL, END_CIRC_REASON_FINISHED,
    END_CIRC_REASON_RESOURCELIMIT, END_CIRC_REASON_REQUESTED,
    END_CIRC_REASON_TORPROTOCOL, END_CIRC_REASON_TIMEOUT,
    END_CIRC_REASON_MEASUREMENT_EXPIRED
  };
  (void)arg;
  setup_channel_expiry_test();

  for (unsigned i = 0; i < ARRAY_LENGTH(cancel_reasons); ++i) {
    memset(&channel, 0, sizeof(channel));
    channel.state = CHANNEL_STATE_OPEN;
    cbt_onehop_failures = 0;
    circ = build_unopened_fourhop(start);
    circ->base_.state = CIRCUIT_STATE_BUILDING;
    circ->base_.n_chan = &channel;
    circ->base_.marked_for_close_orig_reason = cancel_reasons[i];
    circ->cpath->state = CPATH_STATE_AWAITING_KEYS;
    memset(circ->cpath->extend_info->identity_digest, 0x42, DIGEST_LEN);
    circuit_build_failed(circ);
    tt_int_op(channel.is_bad_for_new_circs, OP_EQ, 0);
    tt_uint_op(cbt_onehop_failures, OP_EQ, 0);
    circ->base_.n_chan = NULL;
    circuit_free_(TO_CIRCUIT(circ));
    circ = NULL;
  }

 done:
  if (circ)
    circ->base_.n_chan = NULL;
  circuit_free_(TO_CIRCUIT(circ));
  teardown_channel_expiry_test();
}

/* An actual connection failure must promptly release directory requests. */
static void
test_circuitstats_pending_channel_failure(void *arg)
{
  origin_circuit_t *circ = NULL;
  const struct timeval start = { 1000, 0 };
  channel_t channel;
  (void)arg;
  setup_channel_expiry_test();

  /* Actual connection failure still wakes directory requests promptly. */
  memset(&channel, 0, sizeof(channel));
  memset(channel.identity_digest, 0x42, DIGEST_LEN);
  channel.is_canonical = mock_cbt_channel_is_canonical;
  cbt_onehop_failures = 0;
  circ = build_unopened_fourhop(start);
  memset(circ->cpath->extend_info->identity_digest, 0x42, DIGEST_LEN);
  circ->base_.n_hop = extend_info_dup(circ->cpath->extend_info);
  circuit_set_state(TO_CIRCUIT(circ), CIRCUIT_STATE_CHAN_WAIT);
  circuit_n_chan_done(&channel, 0);
  tt_uint_op(cbt_onehop_failures, OP_EQ, 1);
  tt_int_op(channel.is_bad_for_new_circs, OP_EQ, 0);

 done:
  if (circ)
    circ->base_.n_chan = NULL;
  circuit_free_(TO_CIRCUIT(circ));
  teardown_channel_expiry_test();
}

/* Once recovery is unnecessary or impossible, the usage timeout suffices.
 * This also applies to an attempt already retained as a recovery probe. */
static void
test_circuitstats_recovery_not_needed(void *arg)
{
  circuit_build_times_t *cbt = get_circuit_build_times_mutable();
  origin_circuit_t *circ = NULL;
  channel_t channel = { 0 };
  const struct timeval start = { 1000, 0 };
  extend_info_t hop = { 0 };
  extend_info_t *path[] = { &hop };
  (void)arg;
  circuitbuild_running_unit_tests();
  tor_init_connection_lists();
  get_options_mutable()->LearnCircuitBuildTimeout = 0;
  circuit_build_times_init(cbt);
  cbt->timeout_ms = cbt->close_ms = 1000;
  MOCK(tor_gettimeofday, mock_cbt_gettimeofday);
  MOCK(channel_describe_peer, mock_cbt_describe_peer);
  MOCK(circuit_mark_for_close_, mock_cbt_mark_closed);

  enum { PROGRESS, RETIRED, CLOSING, MISSING, DESTROYED, N_CASES };
  for (int retained = 0; retained < 2; ++retained) {
    for (int kind = 0; kind < N_CASES; ++kind) {
      memset(&channel, 0, sizeof(channel));
      channel.state = CHANNEL_STATE_OPEN;
      circ = new_test_origin_circuit(false, start, 1, path);
      circ->build_state->onehop_tunnel = 1;
      circ->base_.state = CIRCUIT_STATE_BUILDING;
      circ->base_.n_chan = &channel;
      circ->cpath->state = CPATH_STATE_AWAITING_KEYS;
      cbt_test_now = start;
      cbt_test_now.tv_sec += 2;
      if (retained) {
        circuit_expire_building();
        tt_assert(!circ->base_.marked_for_close);
        tt_int_op(circ->base_.purpose, OP_EQ,
                  CIRCUIT_PURPOSE_C_MEASURE_TIMEOUT);
      }
      if (kind == PROGRESS)
        channel.first_hop_success_count++;
      if (kind == RETIRED)
        channel.is_bad_for_new_circs = 1;
      if (kind == CLOSING)
        channel.state = CHANNEL_STATE_CLOSING;
      if (kind == MISSING)
        circ->base_.n_chan = NULL;
      if (kind == DESTROYED)
        circ->base_.received_destroy = 1;
      circuit_expire_building();
      tt_assert(circ->base_.marked_for_close);
      tt_int_op(channel.is_bad_for_new_circs, OP_EQ, kind == RETIRED);
      circ->base_.n_chan = NULL;
      circuit_free_(TO_CIRCUIT(circ));
      circ = NULL;
    }
  }
 done:
  if (circ)
    circ->base_.n_chan = NULL;
  circuit_free_(TO_CIRCUIT(circ));
  circuit_build_times_free_timeouts(cbt);
  UNMOCK(tor_gettimeofday);
  UNMOCK(channel_describe_peer);
  UNMOCK(circuit_mark_for_close_);
}

/* Exercise the real CREATE snapshot and validated handshake success hooks.
 * Only cell delivery is mocked; malformed replies must not count as
 * progress. */
static int
mock_cbt_has_queued_writes(channel_t *chan)
{
  (void)chan;
  return 1;
}

static void
test_circuitstats_firsthop_progress(void *arg)
{
  origin_circuit_t *circ = NULL;
  channel_t channel = { 0 }, other_channel = { 0 };
  const struct timeval start = { 1000, 0 };
  created_cell_t reply = { 0 }, invalid;
  uint8_t keys[128];
  (void)arg;
  channel.state = other_channel.state = CHANNEL_STATE_OPEN;
  channel.first_hop_success_count = 7;
  get_options_mutable()->UseEntryGuards = 0;
  MOCK(circuit_deliver_create_cell, mock_cbt_deliver_create);
  MOCK(channel_describe_peer, mock_cbt_describe_peer);
  circ = build_unopened_fourhop(start);
  circ->base_.n_chan = &channel;
  tt_int_op(circuit_send_next_onion_skin(circ), OP_EQ, 0);
  tt_int_op(cbt_last_create.cell_type, OP_EQ, CELL_CREATE_FAST);
  tt_u64_op(circ->first_hop_success_count_at_create, OP_EQ, 7);
  tt_u64_op(channel.first_hop_success_count, OP_EQ, 7);
  reply.cell_type = CELL_CREATED_FAST;
  reply.handshake_len = CREATED_FAST_LEN;
  tt_int_op(fast_server_handshake(cbt_last_create.onionskin, reply.reply,
                                  keys, sizeof(keys)), OP_EQ, 0);
  invalid = reply;
  invalid.reply[DIGEST_LEN] ^= 1;
  setup_full_capture_of_logs(LOG_WARN);
  tt_int_op(circuit_finish_handshake(circ, &invalid), OP_LT, 0);
  expect_log_msg_containing("onion_skin_client_handshake failed");
  teardown_capture_of_logs();
  tt_u64_op(channel.first_hop_success_count, OP_EQ, 7);
  tt_int_op(circuit_finish_handshake(circ, &reply), OP_EQ, 0);
  tt_u64_op(channel.first_hop_success_count, OP_EQ, 8);
  tt_u64_op(other_channel.first_hop_success_count, OP_EQ, 0);

  /* The shared handshake finisher also handles EXTENDED replies. Completing
   * another hop must not count as successful first-hop CREATE processing. */
  crypt_path_t *hop = circ->cpath->next;
  int len = onion_skin_create(ONION_HANDSHAKE_TYPE_FAST, hop->extend_info,
                              &hop->handshake_state, cbt_last_create.onionskin,
                              sizeof(cbt_last_create.onionskin));
  tt_int_op(len, OP_GT, 0);
  hop->state = CPATH_STATE_AWAITING_KEYS;
  tt_int_op(fast_server_handshake(cbt_last_create.onionskin, reply.reply,
                                  keys, sizeof(keys)), OP_EQ, 0);
  tt_int_op(circuit_finish_handshake(circ, &reply), OP_EQ, 0);
  tt_u64_op(channel.first_hop_success_count, OP_EQ, 8);
  circ->base_.n_chan = NULL;
  circuit_free_(TO_CIRCUIT(circ));

  circ = build_unopened_fourhop(start);
  circ->base_.n_chan = &other_channel;
  tt_int_op(circuit_send_next_onion_skin(circ), OP_EQ, 0);
  tt_u64_op(circ->first_hop_success_count_at_create, OP_EQ, 0);
  tt_int_op(fast_server_handshake(cbt_last_create.onionskin, reply.reply,
                                  keys, sizeof(keys)), OP_EQ, 0);
  tt_int_op(circuit_finish_handshake(circ, &reply), OP_EQ, 0);
  tt_u64_op(other_channel.first_hop_success_count, OP_EQ, 1);
  tt_u64_op(channel.first_hop_success_count, OP_EQ, 8);
 done:
  if (circ)
    circ->base_.n_chan = NULL;
  circuit_free_(TO_CIRCUIT(circ));
  teardown_capture_of_logs();
  UNMOCK(circuit_deliver_create_cell);
  UNMOCK(channel_describe_peer);
}

/* A productive channel survives an old timeout, but a fresh CREATE that
 * subsequently stalls must still retire it. Use real close/free handling and
 * one-hop attempts so the channel diagnostics cannot rely on CBT samples. */
static void
test_circuitstats_channel_progress_lifecycle(void *arg)
{
  channel_t channel = { 0 };
  origin_circuit_t *pending = NULL, *successful = NULL;
  circuit_build_times_t *cbt = get_circuit_build_times_mutable();
  struct timeval start = { 1000, 0 };
  extend_info_t hop = { 0 };
  extend_info_t *path[] = { &hop };
  created_cell_t reply = { 0 };
  uint8_t keys[128];
  (void)arg;
  get_options_mutable()->LearnCircuitBuildTimeout = 0;
  get_options_mutable()->CircuitBuildTimeout = 60;
  get_options_mutable()->UseEntryGuards = 0;
  circuit_build_times_init(cbt);
  tor_init_connection_lists();
  scheduler_init();
  channel_init(&channel);
  channel.state = CHANNEL_STATE_OPEN;
  channel.cmux = circuitmux_alloc();
  channel.has_queued_writes = mock_cbt_has_queued_writes;
  circuitmux_set_policy(channel.cmux, &ewma_policy);
  channel_register(&channel);
  MOCK(tor_gettimeofday, mock_cbt_gettimeofday);
  MOCK(assert_circuit_ok, mock_cbt_assert_circuit_ok);
  MOCK(channel_describe_peer, mock_cbt_describe_peer);
  MOCK(circuit_deliver_create_cell, mock_cbt_deliver_create);
  memset(&cbt_diagnostics, 0, sizeof(cbt_diagnostics));
  cbt_diagnostics.have_reported = true;
  cbt_diagnostics.last_report = start.tv_sec;
  cbt_test_now = start;

  pending = new_test_origin_circuit(false, start, 1, path);
  pending->build_state->onehop_tunnel = 1;
  circuit_set_n_circid_chan(TO_CIRCUIT(pending), 42, &channel);
  tt_int_op(circuit_send_next_onion_skin(pending), OP_EQ, 0);
  tt_u64_op(pending->first_hop_success_count_at_create, OP_EQ, 0);

  successful = new_test_origin_circuit(false, start, 1, path);
  successful->build_state->onehop_tunnel = 1;
  circuit_set_n_circid_chan(TO_CIRCUIT(successful), 43, &channel);
  tt_int_op(circuit_send_next_onion_skin(successful), OP_EQ, 0);
  tt_int_op(cbt_last_create.cell_type, OP_EQ, CELL_CREATE_FAST);
  reply.cell_type = CELL_CREATED_FAST;
  reply.handshake_len = CREATED_FAST_LEN;
  tt_int_op(fast_server_handshake(cbt_last_create.onionskin, reply.reply,
                                  keys, sizeof(keys)), OP_EQ, 0);
  tt_int_op(circuit_finish_handshake(successful, &reply), OP_EQ, 0);
  successful->base_.state = CIRCUIT_STATE_OPEN;
  tt_u64_op(channel.first_hop_success_count, OP_EQ, 1);

  cbt_test_now.tv_sec += 61;
  circuit_expire_building();
  tt_assert(pending->base_.marked_for_close);
  tt_assert(!successful->base_.marked_for_close);
  tt_int_op(channel.is_bad_for_new_circs, OP_EQ, 0);
  tt_u64_op(cbt_diagnostics.firsthop_preserved, OP_EQ, 1);
  circuit_expire_building();
  tt_u64_op(cbt_diagnostics.firsthop_preserved, OP_EQ, 1);
  pending = NULL;
  circuit_close_all_marked();

  /* This is a new circuit and a new snapshot, not a revived expired one. */
  pending = new_test_origin_circuit(false, cbt_test_now, 1, path);
  pending->build_state->onehop_tunnel = 1;
  circuit_set_n_circid_chan(TO_CIRCUIT(pending), 44, &channel);
  tt_int_op(circuit_send_next_onion_skin(pending), OP_EQ, 0);
  tt_u64_op(pending->first_hop_success_count_at_create, OP_EQ, 1);
  cbt_test_now.tv_sec += 61;
  circuit_expire_building();
  tt_assert(pending->base_.marked_for_close);
  tt_int_op(channel.is_bad_for_new_circs, OP_EQ, 1);
  tt_u64_op(cbt_diagnostics.channels_retired, OP_EQ, 1);
  tt_u64_op(cbt_diagnostics.firsthop_preserved, OP_EQ, 1);
  tt_u64_op(cbt_diagnostics.prefix_hops[0], OP_EQ, 0);
  circuit_expire_building();
  tt_u64_op(cbt_diagnostics.channels_retired, OP_EQ, 1);
  pending = NULL;
  circuit_close_all_marked();
 done:
  circuit_free_(TO_CIRCUIT(pending));
  circuit_free_(TO_CIRCUIT(successful));
  channel_unregister(&channel);
  circuitmux_free(channel.cmux);
  scheduler_free_all();
  circuit_build_times_free_timeouts(cbt);
  UNMOCK(tor_gettimeofday);
  UNMOCK(assert_circuit_ok);
  UNMOCK(channel_describe_peer);
  UNMOCK(circuit_deliver_create_cell);
}

static int
mock_cbt_channel_is_canonical(channel_t *chan)
{
  (void)chan;
  return 1;
}

/* Exercise channel failure notification and real close/free cleanup. */
static void
test_circuitstats_connection_failure(void *arg)
{
  origin_circuit_t *circ = NULL;
  channel_t channel;
  struct timeval start = { 1000, 0 };
  (void)arg;
  memset(&channel, 0, sizeof(channel));
  memset(channel.identity_digest, 0x42, DIGEST_LEN);
  channel.is_canonical = mock_cbt_channel_is_canonical;
  memset(&cbt_diagnostics, 0, sizeof(cbt_diagnostics));
  MOCK(assert_circuit_ok, mock_cbt_assert_circuit_ok);
  MOCK(channel_describe_peer, mock_cbt_describe_peer);
  setup_full_capture_of_logs(LOG_NOTICE);

  /* Fixed-timeout diagnostics must work without adaptive learning. */
  get_options_mutable()->LearnCircuitBuildTimeout = 0;
  for (int variant = 0; variant < 3; ++variant) {
    circ = build_unopened_fourhop(start);
    circ->base_.purpose = CIRCUIT_PURPOSE_C_GENERAL;
    circ->cpath->state = CPATH_STATE_CLOSED;
    circ->base_.n_hop = extend_info_dup(circ->cpath->extend_info);
    memcpy(circ->base_.n_hop->identity_digest, channel.identity_digest,
           DIGEST_LEN);
    if (variant == 1)
      circ->build_state->desired_path_len = 1;
    circuit_set_state(TO_CIRCUIT(circ), CIRCUIT_STATE_CHAN_WAIT);
    tt_int_op(circuit_count_pending_on_channel(&channel), OP_EQ, 1);
    if (variant == 2)
      circuit_mark_for_close(TO_CIRCUIT(circ), END_CIRC_REASON_FINISHED);
    else
      circuit_n_chan_done(&channel, 0);
    tt_assert(circ->base_.marked_for_close);
    tt_int_op(circuit_count_pending_on_channel(&channel), OP_EQ, 0);
    circuit_n_chan_done(&channel, 0); /* No duplicate accounting. */
    circ = NULL;
    circuit_close_all_marked();
    tt_u64_op(cbt_diagnostics.connection_failed, OP_EQ, 1);
    tt_u64_op(cbt_diagnostics.prefix_hops[0], OP_EQ, 0);
    tt_u64_op(cbt_diagnostics.other_channel, OP_EQ, 0);
  }
  circuit_build_times_report_diagnostics(1000);
  expect_log_msg_containing("adaptive=0 connection_failed=1");
  mock_clean_saved_logs();
  cbt_diagnostics.connection_failed = 1;
  circuit_build_times_report_diagnostics(1299);
  expect_no_log_entry();
  circuit_build_times_report_diagnostics(1300);
  expect_log_msg_containing("connection_failed=1");
 done:
  circuit_free_(TO_CIRCUIT(circ));
  teardown_capture_of_logs();
  UNMOCK(assert_circuit_ok);
  UNMOCK(channel_describe_peer);
}

/* Integrate repeated HS fourth-hop failures with healthy and slow prefix
 * observations, using real expiration, repurpose, close and free paths. */
static void
test_circuitstats_mixed_rendezvous(void *arg)
{
  circuit_build_times_t *cbt = get_circuit_build_times_mutable();
  origin_circuit_t *circ = NULL, *opened = NULL;
  (void)arg;
  circuitbuild_running_unit_tests();
  MOCK(tor_gettimeofday, mock_cbt_gettimeofday);
  MOCK(assert_circuit_ok, mock_cbt_assert_circuit_ok);
  MOCK(circuit_launch_by_extend_info, mock_cbt_rend_launch);
  circuit_build_times_init(cbt);
  opened = add_opened_threehop();
  tt_assert(circuit_any_opened_circuits());
  for (int i = 0; i < CBT_NCIRCUITS_TO_OBSERVE; ++i)
    circuit_build_times_add_time(cbt, 500);
  for (int i = 0; i < 200; ++i) {
    struct timeval start = { 1000 + i*100, 0 };
    cbt->timeout_ms = 1000;
    cbt->close_ms = 60000;
    circ = build_unopened_fourhop(start);
    circ->base_.purpose = CIRCUIT_PURPOSE_S_CONNECT_REND;
    circ->base_.state = CIRCUIT_STATE_BUILDING;
    circ->hs_ident = tor_malloc_zero(sizeof(*circ->hs_ident));
    circ->build_state->expiry_time = time(NULL) + 30;
    circ->cpath->state = CPATH_STATE_OPEN;
    circ->cpath->next->state = CPATH_STATE_OPEN;
    circ->cpath->next->next->state = CPATH_STATE_OPEN;
    cbt_test_now = start;
    cbt_test_now.tv_usec = 500000;
    if (i % 4 == 0)
      cbt_test_now.tv_sec += 2;
    cbt_rend_launches = 0;
    circuit_build_times_handle_completed_hop(circ);
    cbt->timeout_ms = 1000;
    cbt->close_ms = 60000;
    cbt_test_now.tv_sec = start.tv_sec + 3;
    circuit_expire_building();
    cbt_test_now.tv_sec = start.tv_sec + 61;
    circuit_expire_building();
    tt_assert(circ->base_.marked_for_close);
    tt_int_op(cbt_rend_launches, OP_EQ, 1);
    tt_int_op(cbt->total_build_times, OP_EQ, CBT_NCIRCUITS_TO_OBSERVE);
    tt_int_op(cbt->num_circ_closed, OP_EQ, 0);
    tt_int_op(cbt->num_circ_timeouts, OP_EQ, i/4 + 1);
    tt_int_op(opened->base_.state, OP_EQ, CIRCUIT_STATE_OPEN);
    tt_assert(!opened->base_.marked_for_close);
    circuit_free_(TO_CIRCUIT(circ));
    circ = NULL;
    circuit_free_(TO_CIRCUIT(cbt_rend_retry));
    cbt_rend_retry = NULL;
    tt_int_op(smartlist_len(circuit_get_global_origin_circuit_list()),
              OP_EQ, 1);
  }
 done:
  circuit_free_(TO_CIRCUIT(circ));
  circuit_free_(TO_CIRCUIT(opened));
  circuit_free_(TO_CIRCUIT(cbt_rend_retry));
  cbt_rend_retry = NULL;
  circuit_build_times_free_timeouts(cbt);
  UNMOCK(tor_gettimeofday);
  UNMOCK(assert_circuit_ok);
  UNMOCK(circuit_launch_by_extend_info);
}

/* Callback/expiry order must not change the number of timeout events. */
static void
test_circuitstats_late_firsthop(void *arg)
{
  circuit_build_times_t *cbt = get_circuit_build_times_mutable();
  origin_circuit_t *circ = NULL, *opened = NULL;
  const struct timeval start = { 1000, 0 };
  channel_t channel = { 0 };
  channel.state = CHANNEL_STATE_OPEN;
  (void)arg;
  get_options_mutable()->LearnCircuitBuildTimeout = 1;
  get_options_mutable()->AvoidDiskWrites = 1;
  MOCK(tor_gettimeofday, mock_cbt_gettimeofday);
  MOCK(channel_describe_peer, mock_cbt_describe_peer);
  MOCK(circuit_mark_for_close_, mock_circuit_mark_for_close);
  circuit_build_times_init(cbt);
  for (int have_open = 0; have_open < 2; ++have_open) {
    if (have_open)
      opened = add_opened_threehop();
    /* Callback first, expiration first, no response, recovery exclusion,
     * and learning disabled between the timeout and the response. */
    for (int kind = 0; kind < 5; ++kind) {
      circuit_build_times_reset(cbt);
      memset(cbt->liveness.timeouts_after_firsthop, 0,
             cbt->liveness.num_recent_circs);
      cbt->liveness.after_firsthop_idx = 0;
      for (int i = 0; i < CBT_DEFAULT_MIN_CIRCUITS_TO_OBSERVE; ++i)
        circuit_build_times_add_time(cbt, 500);
      cbt->timeout_ms = 1000;
      cbt->close_ms = 60000;
      circ = build_unopened_fourhop(start);
      circ->base_.n_chan = &channel;
      circ->cpath->state = CPATH_STATE_AWAITING_KEYS;
      cbt_test_now = start;
      cbt_test_now.tv_sec += 3;
      if (kind == 0) {
        circ->cpath->state = CPATH_STATE_OPEN;
        circuit_build_times_handle_completed_hop(circ);
      }
      circuit_expire_building();
      tt_int_op(cbt->num_circ_timeouts, OP_EQ, 1);
      if (kind == 3)
        circ->cbt_observation_invalidated = 1;
      if (kind == 4)
        get_options_mutable()->LearnCircuitBuildTimeout = 0;
      if (kind != 2)
        circ->cpath->state = CPATH_STATE_OPEN;
      circuit_build_times_handle_completed_hop(circ);
      circuit_build_times_handle_completed_hop(circ);
      circuit_expire_building();
      tt_int_op(cbt->num_circ_timeouts, OP_EQ, 1);
      tt_int_op(cbt->liveness.after_firsthop_idx, OP_EQ, kind < 2);
      tt_int_op(cbt->liveness.timeouts_after_firsthop[0], OP_EQ, kind < 2);
      get_options_mutable()->LearnCircuitBuildTimeout = 1;
      if (kind < 2) {
        circ->cpath->next->state = CPATH_STATE_OPEN;
        circ->cpath->next->next->state = CPATH_STATE_OPEN;
        circuit_build_times_handle_completed_hop(circ);
        tt_int_op(cbt->total_build_times, OP_EQ,
                  CBT_DEFAULT_MIN_CIRCUITS_TO_OBSERVE + 1);
        tt_int_op(cbt->liveness.timeouts_after_firsthop[0], OP_EQ, 1);
        int recent_idx = cbt->liveness.after_firsthop_idx;
        circuit_build_times_handle_completed_hop(circ);
        cbt_test_now.tv_sec = 1061;
        circuit_expire_building();
        circuit_expire_building();
        tt_int_op(cbt->liveness.after_firsthop_idx, OP_EQ, recent_idx);
        tt_int_op(cbt->num_circ_timeouts, OP_EQ, 1);
      }
      tt_int_op(channel.is_bad_for_new_circs, OP_EQ, 0);
      circ->base_.n_chan = NULL;
      circuit_free_(TO_CIRCUIT(circ));
      circ = NULL;
    }
    circuit_free_(TO_CIRCUIT(opened));
    opened = NULL;
  }
 done:
  if (circ)
    circ->base_.n_chan = NULL;
  get_options_mutable()->LearnCircuitBuildTimeout = 1;
  circuit_free_(TO_CIRCUIT(circ));
  circuit_free_(TO_CIRCUIT(opened));
  circuit_build_times_free_timeouts(cbt);
  UNMOCK(tor_gettimeofday);
  UNMOCK(channel_describe_peer);
  UNMOCK(circuit_mark_for_close_);
}

/* Reset in the callback is allowed. Old short deadlines must not be replayed,
 * but genuine completion samples and later current-deadline misses survive. */
static void
test_circuitstats_late_firsthop_reset(void *arg)
{
  circuit_build_times_t *cbt = get_circuit_build_times_mutable();
  origin_circuit_t *circs[40] = { NULL };
  origin_circuit_t *opened = NULL;
  const struct timeval start = { 1000, 0 };
  channel_t channel = { 0 };
  channel.state = CHANNEL_STATE_OPEN;
  (void)arg;
  circuitbuild_running_unit_tests();
  MOCK(tor_gettimeofday, mock_cbt_gettimeofday);
  MOCK(channel_describe_peer, mock_cbt_describe_peer);
  MOCK(circuit_mark_for_close_, mock_circuit_mark_for_close);
  circuit_build_times_init(cbt);
  opened = add_opened_threehop();
  for (int i = 0; i < CBT_DEFAULT_MIN_CIRCUITS_TO_OBSERVE; ++i)
    circuit_build_times_add_time(cbt, 500);
  cbt->timeout_ms = 1000;
  cbt->close_ms = 60000;
  cbt_test_now = start;
  cbt_test_now.tv_sec += 3;
  for (unsigned i = 0; i < ARRAY_LENGTH(circs); ++i) {
    circs[i] = build_unopened_fourhop(start);
    circs[i]->base_.n_chan = &channel;
    circs[i]->cpath->state = CPATH_STATE_AWAITING_KEYS;
  }
  circuit_expire_building();
  tt_int_op(cbt->num_circ_timeouts, OP_EQ, ARRAY_LENGTH(circs));
  tt_int_op(cbt->liveness.after_firsthop_idx, OP_EQ, 0);
  for (int i = 0; i < CBT_DEFAULT_MAX_RECENT_TIMEOUT_COUNT; ++i) {
    circs[i]->cpath->state = CPATH_STATE_OPEN;
    circuit_build_times_handle_completed_hop(circs[i]);
  }
  tt_double_op(fabs(cbt->timeout_ms - 60000), OP_LT, 0.001);
  tt_int_op(cbt->total_build_times, OP_EQ, 0);
  tt_int_op(cbt->num_circ_timeouts, OP_EQ, 0); /* Reset clears totals. */
  tt_int_op(cbt->liveness.after_firsthop_idx, OP_EQ, 0);
  for (unsigned i = 0; i < ARRAY_LENGTH(circs); ++i) {
    circs[i]->cpath->state = CPATH_STATE_OPEN;
    circuit_build_times_handle_completed_hop(circs[i]);
    circuit_build_times_handle_completed_hop(circs[i]);
  }
  tt_double_op(fabs(cbt->timeout_ms - 60000), OP_LT, 0.001);
  tt_int_op(cbt->liveness.after_firsthop_idx, OP_EQ, 0);

  /* An old attempt finishing below the new deadline remains a real sample. */
  origin_circuit_t *success = circs[39];
  success->cpath->next->state = CPATH_STATE_OPEN;
  success->cpath->next->next->state = CPATH_STATE_OPEN;
  circuit_build_times_handle_completed_hop(success);
  tt_int_op(cbt->total_build_times, OP_EQ, 1);
  tt_int_op(cbt->circuit_build_times[0], OP_EQ, 3000);
  tt_assert(success->cbt_prefix_measurement_done);
  tt_int_op(cbt->liveness.after_firsthop_idx, OP_EQ, 0);

  /* No new hop callbacks: the expiry pass qualifies pending attempts when
   * they really exceed 60 seconds. Its reset to 120 seconds must apply to
   * the rest of the backlog and to this pass's physical expiry cutoffs. */
  cbt_test_now.tv_sec = 1060;
  circuit_expire_building();
  tt_int_op(cbt->liveness.after_firsthop_idx, OP_EQ, 0);
  cbt_test_now.tv_sec = 1061;
  marked_for_close = 0;
  circuit_expire_building();
  tt_double_op(fabs(cbt->timeout_ms - 120000), OP_LT, 0.001);
  tt_int_op(cbt->liveness.after_firsthop_idx, OP_EQ, 0);
  tt_int_op(marked_for_close, OP_EQ, 0);
  tt_int_op(channel.is_bad_for_new_circs, OP_EQ, 0);
  circuit_expire_building();
  tt_double_op(fabs(cbt->timeout_ms - 120000), OP_LT, 0.001);
  tt_int_op(cbt->num_circ_timeouts, OP_EQ, 0);
 done:
  for (unsigned i = 0; i < ARRAY_LENGTH(circs); ++i) {
    if (circs[i])
      circs[i]->base_.n_chan = NULL;
    circuit_free_(TO_CIRCUIT(circs[i]));
  }
  circuit_free_(TO_CIRCUIT(opened));
  circuit_build_times_free_timeouts(cbt);
  UNMOCK(tor_gettimeofday);
  UNMOCK(channel_describe_peer);
  UNMOCK(circuit_mark_for_close_);
}

static int fourth_extend_result, fourth_extend_calls;
static int
mock_fourth_extend(streamid_t stream_id, circuit_t *circ, uint8_t command,
                  const char *payload, size_t len, crypt_path_t *layer,
                  const char *file, int line)
{
  (void)stream_id; (void)circ; (void)payload; (void)len;
  (void)layer; (void)file; (void)line;
  tor_assert(command == RELAY_COMMAND_EXTEND2);
  ++fourth_extend_calls;
  return fourth_extend_result;
}

static void
test_circuitstats_fresh_rend_extension(void *arg)
{
  (void)arg;
  origin_circuit_t *circ = NULL, *opened = NULL;
  circuit_build_times_t *cbt = get_circuit_build_times_mutable();
  const struct timeval start = { 1000, 0 };
  circuitbuild_running_unit_tests();
  MOCK(tor_gettimeofday, mock_cbt_gettimeofday);
  MOCK(relay_send_command_from_edge_, mock_fourth_extend);
  MOCK(circuit_mark_for_close_, mock_circuit_mark_for_close);
  circuit_build_times_init(cbt);
  opened = add_opened_threehop();
  enum { FRESH, CLIENT_INTRO, VANGUARD, MEASUREMENT, CANNIBALIZED,
         LONG_PATH, EARLIER_HOP, FIXED, SEND_FAILED, SLOW_PREFIX, N_CASES };
  for (int kind = 0; kind < N_CASES; ++kind) {
    circuit_build_times_reset(cbt);
    get_options_mutable()->LearnCircuitBuildTimeout = kind != FIXED;
    get_options_mutable()->CircuitBuildTimeout = kind == FIXED ? 60 : 0;
    cbt->timeout_ms = cbt->close_ms = 60000;
    cbt->liveness.nonlive_timeouts = 0;
    cbt->liveness.network_last_live = 1005;
    circ = build_unopened_fourhop(start);
    circ->base_.state = CIRCUIT_STATE_BUILDING;
    circ->base_.purpose = CIRCUIT_PURPOSE_S_CONNECT_REND;
    circ->build_state->expiry_time = 1030;
    if (kind == CLIENT_INTRO)
      circ->base_.purpose = CIRCUIT_PURPOSE_C_INTRODUCING;
    if (kind == VANGUARD)
      circ->base_.purpose = CIRCUIT_PURPOSE_HS_VANGUARDS;
    if (kind == MEASUREMENT)
      circ->base_.purpose = CIRCUIT_PURPOSE_C_MEASURE_TIMEOUT;
    if (kind == CANNIBALIZED)
      circ->has_opened = 1;
    if (kind == LONG_PATH)
      circuit_append_new_exit(circ, circ->cpath->extend_info);
    circ->cpath->state = CPATH_STATE_OPEN;
    circ->cpath->next->state = CPATH_STATE_OPEN;
    crypt_path_t *hop = circ->cpath->next->next;
    if (kind != EARLIER_HOP) {
      hop->state = CPATH_STATE_OPEN;
      hop = hop->next;
    }
    tor_addr_parse(&hop->extend_info->orports[0].addr, "192.0.2.1");
    hop->extend_info->orports[0].port = 9001;
    memset(hop->extend_info->identity_digest, 0x43, DIGEST_LEN);
    memset(hop->extend_info->curve25519_onion_key.public_key, 0x42,
           CURVE25519_PUBKEY_LEN);
    fourth_extend_calls = 0;
    fourth_extend_result = kind == SEND_FAILED ? -1 : 0;
    cbt_test_now = start;
    cbt_test_now.tv_sec = 1005;
    if (kind == SLOW_PREFIX)
      cbt->timeout_ms = 1000;
    tt_int_op(circuit_send_next_onion_skin(circ), OP_EQ, 0);
    tt_int_op(fourth_extend_calls, OP_EQ, 1);
    const bool reset = kind == FRESH || kind == FIXED || kind == LONG_PATH;
    tt_int_op(circ->base_.timestamp_began.tv_sec, OP_EQ,
              reset ? 1005 : 1000);
    tt_int_op(circ->build_state->expiry_time, OP_EQ, 1030);
    if (kind == SLOW_PREFIX)
      tt_int_op(circ->base_.purpose, OP_EQ, CIRCUIT_PURPOSE_C_MEASURE_TIMEOUT);
    if (kind == LONG_PATH) {
      /* L3 vanguards use G-L2-L3-M-R. Give each extension beyond the
       * measured prefix its own budget without sampling that prefix again. */
      tt_assert(circ->cbt_prefix_measurement_done);
      tt_int_op(cbt->total_build_times, OP_EQ, 1);
      tt_int_op(cbt->circuit_build_times[0], OP_EQ, 5000);
      hop->state = CPATH_STATE_OPEN;
      hop = hop->next;
      tor_addr_parse(&hop->extend_info->orports[0].addr, "192.0.2.2");
      hop->extend_info->orports[0].port = 9001;
      memset(hop->extend_info->identity_digest, 0x44, DIGEST_LEN);
      memset(hop->extend_info->curve25519_onion_key.public_key, 0x45,
             CURVE25519_PUBKEY_LEN);
      cbt_test_now.tv_sec = 1006;
      tt_int_op(circuit_send_next_onion_skin(circ), OP_EQ, 0);
      tt_int_op(fourth_extend_calls, OP_EQ, 2);
      tt_int_op(circ->base_.timestamp_began.tv_sec, OP_EQ, 1006);
      tt_int_op(cbt->total_build_times, OP_EQ, 1);
      tt_int_op(circ->build_state->expiry_time, OP_EQ, 1030);
    }
    if (kind == FRESH) {
      /* The prefix was sampled before resetting the extension's clock. */
      tt_assert(circ->cbt_prefix_measurement_done);
      tt_int_op(cbt->total_build_times, OP_EQ, 1);
      tt_int_op(cbt->circuit_build_times[0], OP_EQ, 5000);
      cbt->timeout_ms = 1000; /* CBT can still fall after EXTEND. */
      cbt_test_now.tv_sec = 1006;
      marked_for_close = 0;
      circuit_expire_building();
      tt_int_op(marked_for_close, OP_EQ, 0);
      tt_int_op(circ->base_.purpose, OP_EQ, CIRCUIT_PURPOSE_S_CONNECT_REND);
      /* The shared timestamp intentionally shifts the hard close too. */
      circ->base_.purpose = CIRCUIT_PURPOSE_C_MEASURE_TIMEOUT;
      cbt_test_now.tv_sec = 1061;
      circuit_expire_building();
      tt_int_op(marked_for_close, OP_EQ, 0);
      cbt_test_now.tv_sec = 1066;
      circuit_expire_building();
      tt_int_op(marked_for_close, OP_EQ, 1);
      tt_int_op(cbt->total_build_times, OP_EQ, 1);
    }
    circuit_free_(TO_CIRCUIT(circ));
    circ = NULL;
  }
 done:
  get_options_mutable()->LearnCircuitBuildTimeout = 1;
  get_options_mutable()->CircuitBuildTimeout = 0;
  circuit_free_(TO_CIRCUIT(circ));
  circuit_free_(TO_CIRCUIT(opened));
  circuit_build_times_free_timeouts(cbt);
  UNMOCK(tor_gettimeofday);
  UNMOCK(relay_send_command_from_edge_);
  UNMOCK(circuit_mark_for_close_);
}

#define TEST_CIRCUITSTATS(name, flags) \
    { #name, test_##name, (flags), &helper_pubsub_setup, NULL }

struct testcase_t circuitstats_tests[] = {
  TEST_CIRCUITSTATS(circuitstats_fresh_rend_extension, TT_FORK),
  TEST_CIRCUITSTATS(circuitstats_hoplen, TT_FORK),
  TEST_CIRCUITSTATS(circuitstats_late_firsthop, TT_FORK),
  TEST_CIRCUITSTATS(circuitstats_late_firsthop_reset, TT_FORK),
  TEST_CIRCUITSTATS(circuitstats_recovery_retries, TT_FORK),
  TEST_CIRCUITSTATS(circuitstats_prefix_accounting, TT_FORK),
  TEST_CIRCUITSTATS(circuitstats_prefix_lifecycle, TT_FORK),
  TEST_CIRCUITSTATS(circuitstats_recovery, TT_FORK),
  TEST_CIRCUITSTATS(circuitstats_recovery_mixed, TT_FORK),
  TEST_CIRCUITSTATS(circuitstats_recovery_load, TT_FORK),
  TEST_CIRCUITSTATS(circuitstats_recovery_backlog, TT_FORK),
  TEST_CIRCUITSTATS(circuitstats_recovery_statefile, TT_FORK),
  TEST_CIRCUITSTATS(circuitstats_diagnostics, TT_FORK),
  TEST_CIRCUITSTATS(circuitstats_stalled_expiry, TT_FORK),
  TEST_CIRCUITSTATS(circuitstats_open_channel_failure, TT_FORK),
  TEST_CIRCUITSTATS(circuitstats_channel_diagnostics, TT_FORK),
  TEST_CIRCUITSTATS(circuitstats_channel_retirement, TT_FORK),
  TEST_CIRCUITSTATS(circuitstats_channel_deadlines, TT_FORK),
  TEST_CIRCUITSTATS(circuitstats_measurement_probe_limit, TT_FORK),
  TEST_CIRCUITSTATS(circuitstats_channel_cancellation, TT_FORK),
  TEST_CIRCUITSTATS(circuitstats_pending_channel_failure, TT_FORK),
  TEST_CIRCUITSTATS(circuitstats_recovery_not_needed, TT_FORK),
  TEST_CIRCUITSTATS(circuitstats_firsthop_progress, TT_FORK),
  TEST_CIRCUITSTATS(circuitstats_channel_progress_lifecycle, TT_FORK),
  TEST_CIRCUITSTATS(circuitstats_connection_failure, TT_FORK),
  TEST_CIRCUITSTATS(circuitstats_mixed_rendezvous, TT_FORK),
  TEST_CIRCUITSTATS(circuitstats_create_send_failure, TT_FORK),
  TEST_CIRCUITSTATS(circuitstats_probe_accounting, TT_FORK),
  TEST_CIRCUITSTATS(circuitstats_bootstrap_probe_retry, TT_FORK),
  END_OF_TESTCASES
};
