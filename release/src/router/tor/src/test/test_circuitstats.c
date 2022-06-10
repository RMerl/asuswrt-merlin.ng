/* Copyright (c) 2017-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

#define CIRCUITBUILD_PRIVATE
#define CIRCUITSTATS_PRIVATE
#define CIRCUITLIST_PRIVATE
#define CHANNEL_FILE_PRIVATE

#include "core/or/or.h"
#include "test/test.h"
#include "test/test_helpers.h"
#include "test/log_test_helpers.h"
#include "app/config/config.h"
#include "core/or/circuitlist.h"
#include "core/or/circuitbuild.h"
#include "core/or/circuitstats.h"
#include "core/or/circuituse.h"
#include "core/or/channel.h"

#include "core/or/crypt_path_st.h"
#include "core/or/extend_info_st.h"
#include "core/or/origin_circuit_st.h"

static origin_circuit_t *add_opened_threehop(void);
static origin_circuit_t *build_unopened_fourhop(struct timeval);
static origin_circuit_t *subtest_fourhop_circuit(struct timeval, int);

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

#define TEST_CIRCUITSTATS(name, flags) \
    { #name, test_##name, (flags), &helper_pubsub_setup, NULL }

struct testcase_t circuitstats_tests[] = {
  TEST_CIRCUITSTATS(circuitstats_hoplen, TT_FORK),
  END_OF_TESTCASES
};

