#include "core/or/or.h"
#include "test/test.h"
#include "test/log_test_helpers.h"
#include "lib/testsupport/testsupport.h"
#include "test/fakecircs.h"
#include "test/rng_test_helpers.h"

#include "lib/time/compat_time.h"

#include "core/or/circuitlist.h"
#include "core/or/circuitmux.h"
#include "core/or/channel.h"

#define TOR_CONGESTION_CONTROL_COMMON_PRIVATE
#define TOR_CONGESTION_CONTROL_PRIVATE
#include "core/or/congestion_control_st.h"
#include "core/or/congestion_control_common.h"
#include "core/or/congestion_control_vegas.h"

void test_congestion_control_rtt(void *arg);
void test_congestion_control_clock(void *arg);
void test_congestion_control_vegas_cwnd(void *arg);

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

/* =============== Clock Heuristic Test Vectors =============== */

typedef struct clock_vec
{
  uint64_t old_delta_in;
  uint64_t new_delta_in;
  bool in_slow_start_in;
  bool cached_result_out;
  bool result_out;
} clock_vec_t;

static void
run_clock_test_vec(congestion_control_t *cc,
             clock_vec_t *vec, size_t vec_len)
{
  for (size_t i = 0; i < vec_len; i++) {
    cc->in_slow_start = vec[i].in_slow_start_in;
    cc->ewma_rtt_usec = vec[i].old_delta_in*1000;
    bool ret = time_delta_stalled_or_jumped(cc,
                                            vec[i].old_delta_in,
                                            vec[i].new_delta_in);

    tt_int_op(ret, OP_EQ, vec[i].result_out);
    tt_int_op(is_monotime_clock_broken, OP_EQ, vec[i].cached_result_out);
  }

 done:
  is_monotime_clock_broken = false;
}

/**
 * This test verifies the behavior of Section 2.1.1 of
 * Prop#324 (CLOCK_HEURISTICS).
 *
 * It checks that we declare the clock value stalled,
 * and cache that value, on various time deltas.
 *
 * It also verifies that our heuristics behave correctly
 * with respect to slow start and large clock jumps/stalls.
 */
void
test_congestion_control_clock(void *arg)
{
  (void)arg;
  clock_vec_t vect1[] =
    {
      {0, 1, 1, 0, 0}, // old delta 0, slow start -> false
      {0, 0, 1, 1, 1}, // New delta 0 -> cache true, return true
      {1, 1, 1, 1, 0}, // In slow start -> keep cache, but return false
      {1, 4999, 0, 0, 0}, // Not slow start, edge -> update cache, and false
      {4999, 1, 0, 0, 0}, // Not slow start, other edge -> false
      {5001, 1, 0, 0, 0}, // Not slow start w/ -5000x -> use cache (false)
      {5001, 0, 0, 1, 1}, // New delta 0 -> cache true, return true
      {5001, 1, 0, 1, 1}, // Not slow start w/ -5000x -> use cache (true)
      {5001, 1, 1, 1, 0}, // In slow start w/ -5000x -> false
      {0, 5001, 0, 1, 0}, // Not slow start w/ no EWMA -> false
      {1, 5001, 1, 1, 0}, // In slow start w/ +5000x -> false
      {1, 1, 0, 0, 0}, // Not slow start -> update cache to false
      {5001, 1, 0, 0, 0}, // Not slow start w/ -5000x -> use cache (false)
      {1, 5001, 0, 0, 1}, // Not slow start w/ +5000x -> true
      {0, 5001, 0, 0, 0}, // Not slow start w/ no EWMA -> false
      {5001, 1, 1, 0, 0}, // In slow start w/ -5000x change -> false
      {1, 1, 0, 0, 0} // Not slow start -> false
    };

  circuit_params_t params;

  params.cc_enabled = 1;
  params.sendme_inc_cells = TLS_RECORD_MAX_CELLS;
  cc_alg = CC_ALG_VEGAS;
  congestion_control_t *cc = congestion_control_new(&params, CC_PATH_EXIT);

  run_clock_test_vec(cc, vect1, sizeof(vect1)/sizeof(clock_vec_t));

  congestion_control_free(cc);
}

/* =========== RTT Test Vectors ================== */

typedef struct rtt_vec {
  uint64_t sent_usec_in;
  uint64_t got_sendme_usec_in;
  uint64_t cwnd_in;
  bool ss_in;
  uint64_t curr_rtt_usec_out;
  uint64_t ewma_rtt_usec_out;
  uint64_t min_rtt_usec_out;
} rtt_vec_t;

static void
run_rtt_test_vec(congestion_control_t *cc,
                 rtt_vec_t *vec, size_t vec_len)
{
  for (size_t i = 0; i < vec_len; i++) {
    enqueue_timestamp(cc->sendme_pending_timestamps,
                      vec[i].sent_usec_in);
  }

  for (size_t i = 0; i < vec_len; i++) {
    cc->cwnd = vec[i].cwnd_in;
    cc->in_slow_start = vec[i].ss_in;
    uint64_t curr_rtt_usec = congestion_control_update_circuit_rtt(cc,
                                         vec[i].got_sendme_usec_in);

    tt_int_op(curr_rtt_usec, OP_EQ, vec[i].curr_rtt_usec_out);
    tt_int_op(cc->min_rtt_usec, OP_EQ, vec[i].min_rtt_usec_out);
    tt_int_op(cc->ewma_rtt_usec, OP_EQ, vec[i].ewma_rtt_usec_out);
  }
 done:
  is_monotime_clock_broken = false;
}

/**
 * This test validates current, EWMA, and minRTT calculation
 * from Sections 2.1 of Prop#324.
 *
 * We also do NOT exercise the sendme pacing code here. See
 * test_sendme_is_next() for that, in test_sendme.c.
 */
void
test_congestion_control_rtt(void *arg)
{
  (void)arg;
  rtt_vec_t vect1[] = {
    {100000, 200000, 124, 1, 100000, 100000, 100000},
    {200000, 300000, 124, 1, 100000, 100000, 100000},
    {350000, 500000, 124, 1, 150000, 133333, 100000},
    {500000, 550000, 124, 1, 50000,  77777, 77777},
    {600000, 700000, 124, 1, 100000, 92592, 77777},
    {700000, 750000, 124, 1, 50000, 64197, 64197},
    {750000, 875000, 124, 0, 125000, 104732, 104732},
    {875000, 900000, 124, 0, 25000, 51577, 104732},
    {900000, 950000, 200, 0, 50000, 50525, 50525}
  };

  circuit_params_t params;
  congestion_control_t *cc = NULL;

  params.cc_enabled = 1;
  params.sendme_inc_cells = TLS_RECORD_MAX_CELLS;
  cc_alg = CC_ALG_VEGAS;

  cc = congestion_control_new(&params, CC_PATH_EXIT);
  run_rtt_test_vec(cc, vect1, sizeof(vect1)/sizeof(rtt_vec_t));
  congestion_control_free(cc);

  return;
}

/* =========== Vegas CWND Test Vectors ============== */

typedef struct cwnd_vec {
  uint64_t sent_usec_in;
  uint64_t got_sendme_usec_in;
  bool or_conn_blocked_in;
  uint64_t inflight_in;
  uint64_t ewma_rtt_usec_out;
  uint64_t min_rtt_usec_out;
  uint64_t cwnd_out;
  bool in_slow_start_out;
  bool cwnd_full_out;
  bool blocked_chan_out;
} cwnd_vec_t;

static void
run_vegas_cwnd_test_vec(congestion_control_t *cc,
                        circuit_t *circ,
                        cwnd_vec_t *vec, size_t vec_len)
{
  for (size_t i = 0; i < vec_len; i++) {
    enqueue_timestamp(cc->sendme_pending_timestamps,
                      vec[i].sent_usec_in);
  }

  for (size_t i = 0; i < vec_len; i++) {
    log_notice(LD_CIRC, "Step %d", (int)i);
    monotime_set_mock_time_nsec(vec[i].got_sendme_usec_in*1000);
    circ->circuit_blocked_on_p_chan = vec[i].or_conn_blocked_in;
    cc->inflight = vec[i].inflight_in;

    congestion_control_vegas_process_sendme(cc, circ);

    /* If the or conn was blocked, ensure we updated our
     * CC state */
    if (vec[i].or_conn_blocked_in) {
      tt_int_op(cc->next_cc_event, OP_EQ, CWND_UPDATE_RATE(cc));
    }

    tt_int_op(cc->ewma_rtt_usec, OP_EQ, vec[i].ewma_rtt_usec_out);
    tt_int_op(cc->min_rtt_usec, OP_EQ, vec[i].min_rtt_usec_out);
    tt_int_op(cc->cwnd, OP_EQ, vec[i].cwnd_out);

    tt_int_op(cc->in_slow_start, OP_EQ, vec[i].in_slow_start_out);
    tt_int_op(cc->cwnd_full, OP_EQ, vec[i].cwnd_full_out);
    tt_int_op(cc->blocked_chan, OP_EQ, vec[i].blocked_chan_out);
  }

 done:
  is_monotime_clock_broken = false;
}

/**
 * This test validates congestion window updates for the
 * TOR_VEGAS congestion control algorithm, from Section 3.3
 * of Prop#324.
 *
 * It tests updates as a function of the timestamp of the
 * cell that would trigger a sendme and the sendme arrival
 * timestamp, and as a function of orconn blocking.
 *
 * It ensures that at least one test vector caused a cwnd update
 * due to a blocked OR connection. The orconn blocking logic is
 * simulated -- we do NOT actually exercise the orconn code here.
 *
 * We also do NOT exercise the sendme pacing code here. See
 * test_sendme_is_next() for that, in test_sendme.c.
 *
 * We also do NOT exercise the negotiation code here. See
 * test_ntor3_handshake() for that, in test_ntor_v3.c.
 */
void
test_congestion_control_vegas_cwnd(void *arg)
{
  (void)arg;
  circuit_params_t params;
  /* Replay of RTT edge case checks, plus some extra to exit
   * slow start via RTT, and exercise full/not full */
  cwnd_vec_t vect1[] = {
    {100000, 200000, 0, 124, 100000, 100000, 155, 1, 0, 0},
    {200000, 300000, 0, 155, 100000, 100000, 186, 1, 1, 0},
    {350000, 500000, 0, 186, 133333, 100000, 217, 1, 1, 0},
    {500000, 550000, 0, 217, 77777, 77777, 248, 1, 1, 0},
    {600000, 700000, 0, 248, 92592, 77777, 279, 1, 1, 0},
    {700000, 750000, 0, 279, 64197, 64197, 310, 1, 0, 0}, // Fullness expiry
    {750000, 875000, 0, 310, 104732, 64197, 341, 1, 1, 0},
    {875000, 900000, 0, 341, 51577, 51577, 372, 1, 1, 0},
    {900000, 950000, 0, 279, 50525, 50525, 403, 1, 1, 0},
    {950000, 1000000, 0, 279, 50175, 50175, 434, 1, 1, 0},
    {1000000, 1050000, 0, 279, 50058, 50058, 465, 1, 1, 0},
    {1050000, 1100000, 0, 279, 50019, 50019, 496, 1, 1, 0},
    {1100000, 1150000, 0, 279, 50006, 50006, 527, 1, 1, 0},
    {1150000, 1200000, 0, 279, 50002, 50002, 558, 1, 1, 0},
    {1200000, 1250000, 0, 550, 50000, 50000, 589, 1, 1, 0},
    {1250000, 1300000, 0, 550, 50000, 50000, 620, 1, 0, 0}, // Fullness expiry
    {1300000, 1350000, 0, 550, 50000, 50000, 635, 1, 1, 0},
    {1350000, 1400000, 0, 550, 50000, 50000, 650, 1, 1, 0},
    {1400000, 1450000, 0, 150, 50000, 50000, 650, 1, 0, 0}, // cwnd not full
    {1450000, 1500000, 0, 150, 50000, 50000, 650, 1, 0, 0}, // cwnd not full
    {1500000, 1550000, 0, 550, 50000, 50000, 664, 1, 1, 0}, // cwnd full
    {1500000, 1600000, 0, 550, 83333, 50000, 584, 0, 1, 0}, // gamma exit
    {1600000, 1650000, 0, 550, 61111, 50000, 585, 0, 1, 0}, // alpha
    {1650000, 1700000, 0, 550, 53703, 50000, 586, 0, 1, 0},
    {1700000, 1750000, 0, 100, 51234, 50000, 586, 0, 0, 0}, // alpha, not full
    {1750000, 1900000, 0, 100, 117078, 50000, 559, 0, 0, 0}, // delta, not full
    {1900000, 2000000, 0, 100, 105692, 50000, 558, 0, 0, 0}, // beta, not full
    {2000000, 2075000, 0, 500, 85230, 50000, 558, 0, 1, 0}, // no change
    {2075000, 2125000, 1, 500, 61743, 50000, 557, 0, 1, 1}, // beta, blocked
    {2125000, 2150000, 0, 500, 37247, 37247, 558, 0, 1, 0}, // alpha
    {2150000, 2350000, 0, 500, 145749, 37247, 451, 0, 1, 0} // delta
  };
  /* Test exiting slow start via blocked orconn */
  cwnd_vec_t vect2[] = {
      {100000, 200000, 0, 124, 100000, 100000, 155, 1, 0, 0},
      {200000, 300000, 0, 155, 100000, 100000, 186, 1, 1, 0},
      {350000, 500000, 0, 186, 133333, 100000, 217, 1, 1, 0},
      {500000, 550000, 1, 217, 77777, 77777, 403, 0, 1, 1}, // ss exit, blocked
      {600000, 700000, 0, 248, 92592, 77777, 404, 0, 1, 0}, // alpha
      {700000, 750000, 1, 404, 64197, 64197, 403, 0, 0, 1}, // blocked beta
      {750000, 875000, 0, 403, 104732, 64197, 404, 0, 1, 0}
  };
  /* Real upload 1 */
  cwnd_vec_t vect3[] = {
      { 18258527, 19002938, 0, 83, 744411, 744411, 155, 1, 0, 0 },
      { 18258580, 19254257, 0, 52, 911921, 744411, 186, 1, 1, 0 },
      { 20003224, 20645298, 0, 164, 732023, 732023, 217, 1, 1, 0 },
      { 20003367, 21021444, 0, 133, 922725, 732023, 248, 1, 1, 0 },
      { 20003845, 21265508, 0, 102, 1148683, 732023, 279, 1, 1, 0 },
      { 20003975, 21429157, 0, 71, 1333015, 732023, 310, 1, 0, 0 },
      { 20004309, 21707677, 0, 40, 1579917, 732023, 310, 1, 0, 0 }
  };
  /* Real upload 2 */
  cwnd_vec_t vect4[] = {
      { 358297091, 358854163, 0, 83, 557072, 557072, 155, 1, 0, 0 },
      { 358297649, 359123845, 0, 52, 736488, 557072, 186, 1, 1, 0 },
      { 359492879, 359995330, 0, 186, 580463, 557072, 217, 1, 1, 0 },
      { 359493043, 360489243, 0, 217, 857621, 557072, 248, 1, 1, 0 },
      { 359493232, 360489673, 0, 248, 950167, 557072, 279, 1, 1, 0 },
      { 359493795, 360489971, 0, 279, 980839, 557072, 310, 1, 0, 0 },
      { 359493918, 360490248, 0, 310, 991166, 557072, 341, 1, 1, 0 },
      { 359494029, 360716465, 0, 341, 1145346, 557072, 372, 1, 1, 0 },
      { 359996888, 360948867, 0, 372, 1016434, 557072, 403, 1, 1, 0 },
      { 359996979, 360949330, 0, 403, 973712, 557072, 434, 1, 1, 0 },
      { 360489528, 361113615, 0, 434, 740628, 557072, 465, 1, 1, 0 },
      { 360489656, 361281604, 0, 465, 774841, 557072, 496, 1, 1, 0 },
      { 360489837, 361500461, 0, 496, 932029, 557072, 482, 0, 1, 0 },
      { 360489963, 361500631, 0, 482, 984455, 557072, 482, 0, 1, 0 },
      { 360490117, 361842481, 0, 482, 1229727, 557072, 481, 0, 1, 0 }
  };

  congestion_control_t *cc = NULL;
  channel_t dummy_channel = {0};

  MOCK(circuitmux_attach_circuit, circuitmux_attach_circuit_mock);
  testing_enable_reproducible_rng();

  monotime_init();
  monotime_enable_test_mocking();
  monotime_set_mock_time_nsec(0);

  dummy_channel.cmux = circuitmux_alloc();
  circuit_t *circ = TO_CIRCUIT(new_fake_orcirc(&dummy_channel,
                                               &dummy_channel));
  circ->purpose = CIRCUIT_PURPOSE_OR;

  params.cc_enabled = 1;
  params.sendme_inc_cells = TLS_RECORD_MAX_CELLS;
  cc_alg = CC_ALG_VEGAS;

  cc = congestion_control_new(&params, CC_PATH_EXIT);
  run_vegas_cwnd_test_vec(cc, circ, vect1,
                          sizeof(vect1)/sizeof(cwnd_vec_t));
  congestion_control_free(cc);

  cc = congestion_control_new(&params, CC_PATH_EXIT);
  run_vegas_cwnd_test_vec(cc, circ, vect2,
                          sizeof(vect2)/sizeof(cwnd_vec_t));
  congestion_control_free(cc);

  cc = congestion_control_new(&params, CC_PATH_EXIT);
  run_vegas_cwnd_test_vec(cc, circ, vect3,
                          sizeof(vect3)/sizeof(cwnd_vec_t));
  congestion_control_free(cc);

  cc = congestion_control_new(&params, CC_PATH_EXIT);
  run_vegas_cwnd_test_vec(cc, circ, vect4,
                          sizeof(vect4)/sizeof(cwnd_vec_t));
  congestion_control_free(cc);

 //done:
  circuitmux_free(dummy_channel.cmux);
  return;
}

#define TEST_CONGESTION_CONTROL(name, flags) \
    { #name, test_##name, (flags), NULL, NULL }

struct testcase_t congestion_control_tests[] = {
  TEST_CONGESTION_CONTROL(congestion_control_clock, TT_FORK),
  TEST_CONGESTION_CONTROL(congestion_control_rtt, TT_FORK),
  TEST_CONGESTION_CONTROL(congestion_control_vegas_cwnd, TT_FORK),
  END_OF_TESTCASES
};
