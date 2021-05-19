/* Copyright (c) 2017-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file test_hs_cell.c
 * \brief Test hidden service cell functionality.
 */

#define CIRCUITLIST_PRIVATE
#define NETWORKSTATUS_PRIVATE
#define HS_DOS_PRIVATE
#define HS_INTROPOINT_PRIVATE

#include "test/test.h"
#include "test/test_helpers.h"
#include "test/log_test_helpers.h"

#include "app/config/config.h"

#include "core/or/circuitlist.h"
#include "core/or/circuituse.h"
#include "core/or/or_circuit_st.h"

#include "feature/hs/hs_dos.h"
#include "feature/hs/hs_intropoint.h"
#include "feature/nodelist/networkstatus.h"

static void
setup_mock_consensus(void)
{
  current_ns_consensus = tor_malloc_zero(sizeof(networkstatus_t));
  current_ns_consensus->net_params = smartlist_new();
  smartlist_add(current_ns_consensus->net_params,
                (void *) "HiddenServiceEnableIntroDoSDefense=1");
  hs_dos_consensus_has_changed(current_ns_consensus);
}

static void
free_mock_consensus(void)
{
  smartlist_free(current_ns_consensus->net_params);
  tor_free(current_ns_consensus);
}

static void
test_can_send_intro2(void *arg)
{
  uint32_t now = (uint32_t) approx_time();
  or_circuit_t *or_circ = NULL;

  (void) arg;

  hs_init();
  hs_dos_init();

  get_options_mutable()->ORPort_set = 1;
  setup_mock_consensus();

  or_circ =  or_circuit_new(1, NULL);

  /* Make that circuit a service intro point. */
  circuit_change_purpose(TO_CIRCUIT(or_circ), CIRCUIT_PURPOSE_INTRO_POINT);
  hs_dos_setup_default_intro2_defenses(or_circ);
  or_circ->introduce2_dos_defense_enabled = 1;

  /* Brand new circuit, we should be able to send INTRODUCE2 cells. */
  tt_int_op(true, OP_EQ, hs_dos_can_send_intro2(or_circ));

  /* Simulate that 10 cells have arrived in 1 second. There should be no
   * refill since the bucket is already at maximum on the first cell. */
  update_approx_time(++now);
  for (int i = 0; i < 10; i++) {
    tt_int_op(true, OP_EQ, hs_dos_can_send_intro2(or_circ));
  }
  tt_uint_op(token_bucket_ctr_get(&or_circ->introduce2_bucket), OP_EQ,
             get_intro2_burst_consensus_param(NULL) - 10);

  /* Fully refill the bucket minus 1 cell. */
  update_approx_time(++now);
  tt_int_op(true, OP_EQ, hs_dos_can_send_intro2(or_circ));
  tt_uint_op(token_bucket_ctr_get(&or_circ->introduce2_bucket), OP_EQ,
             get_intro2_burst_consensus_param(NULL) - 1);

  /* Receive an INTRODUCE2 at each second. We should have the bucket full
   * since at every second it gets refilled. */
  for (int i = 0; i < 10; i++) {
    update_approx_time(++now);
    tt_int_op(true, OP_EQ, hs_dos_can_send_intro2(or_circ));
  }
  /* Last check if we can send the cell decrements the bucket so minus 1. */
  tt_uint_op(token_bucket_ctr_get(&or_circ->introduce2_bucket), OP_EQ,
             get_intro2_burst_consensus_param(NULL) - 1);

  /* Manually reset bucket for next test. */
  token_bucket_ctr_reset(&or_circ->introduce2_bucket, now);
  tt_uint_op(token_bucket_ctr_get(&or_circ->introduce2_bucket), OP_EQ,
             get_intro2_burst_consensus_param(NULL));

  /* Do a full burst in the current second which should empty the bucket and
   * we shouldn't be allowed to send one more cell after that. We go minus 1
   * cell else the very last check if we can send the INTRO2 cell returns
   * false because the bucket goes down to 0. */
  for (uint32_t i = 0; i < get_intro2_burst_consensus_param(NULL) - 1; i++) {
    tt_int_op(true, OP_EQ, hs_dos_can_send_intro2(or_circ));
  }
  tt_uint_op(token_bucket_ctr_get(&or_circ->introduce2_bucket), OP_EQ, 1);
  /* Get the last remaining cell, we shouldn't be allowed to send it. */
  tt_int_op(false, OP_EQ, hs_dos_can_send_intro2(or_circ));
  tt_uint_op(token_bucket_ctr_get(&or_circ->introduce2_bucket), OP_EQ, 0);

  /* Make sure the next 100 cells aren't allowed and bucket stays at 0. */
  for (int i = 0; i < 100; i++) {
    tt_int_op(false, OP_EQ, hs_dos_can_send_intro2(or_circ));
    tt_uint_op(token_bucket_ctr_get(&or_circ->introduce2_bucket), OP_EQ, 0);
  }

  /* One second has passed, we should have the rate minus 1 cell added. */
  update_approx_time(++now);
  tt_int_op(true, OP_EQ, hs_dos_can_send_intro2(or_circ));
  tt_uint_op(token_bucket_ctr_get(&or_circ->introduce2_bucket), OP_EQ,
             get_intro2_rate_consensus_param(NULL) - 1);

 done:
  circuit_free_(TO_CIRCUIT(or_circ));

  hs_free_all();
  free_mock_consensus();
}

static void
test_validate_dos_extension_params(void *arg)
{
  bool ret;

  (void) arg;

  /* Validate the default values. */
  ret = cell_dos_extension_parameters_are_valid(
                                      get_intro2_rate_consensus_param(NULL),
                                      get_intro2_burst_consensus_param(NULL));
  tt_assert(ret);

  /* Valid custom rate/burst. */
  ret = cell_dos_extension_parameters_are_valid(17, 42);
  tt_assert(ret);
  ret = cell_dos_extension_parameters_are_valid(INT32_MAX, INT32_MAX);
  tt_assert(ret);

  /* Invalid rate. */
  ret = cell_dos_extension_parameters_are_valid(UINT64_MAX, 42);
  tt_assert(!ret);

  /* Invalid burst. */
  ret = cell_dos_extension_parameters_are_valid(42, UINT64_MAX);
  tt_assert(!ret);

  /* Value of 0 is valid (but should disable defenses) */
  ret = cell_dos_extension_parameters_are_valid(0, 0);
  tt_assert(ret);

  /* Can't have burst smaller than rate. */
  ret = cell_dos_extension_parameters_are_valid(42, 40);
  tt_assert(!ret);

 done:
  return;
}

struct testcase_t hs_dos_tests[] = {
  { "can_send_intro2", test_can_send_intro2, TT_FORK,
    NULL, NULL },
  { "validate_dos_extension_params", test_validate_dos_extension_params,
    TT_FORK, NULL, NULL },

  END_OF_TESTCASES
};
