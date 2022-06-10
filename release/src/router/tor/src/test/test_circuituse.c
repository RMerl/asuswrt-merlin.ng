/* Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

#define CIRCUITLIST_PRIVATE

#include "core/or/or.h"
#include "test/test.h"
#include "test/test_helpers.h"
#include "app/config/config.h"
#include "core/or/circuitlist.h"
#include "core/or/circuituse.h"
#include "core/or/circuitbuild.h"
#include "feature/nodelist/nodelist.h"

#include "core/or/cpath_build_state_st.h"
#include "core/or/origin_circuit_st.h"

static void
test_circuit_is_available_for_use_ret_false_when_marked_for_close(void *arg)
{
  (void)arg;

  circuit_t *circ = tor_malloc(sizeof(circuit_t));
  circ->marked_for_close = 1;

  tt_int_op(0, OP_EQ, circuit_is_available_for_use(circ));

  done:
    tor_free(circ);
}

static void
test_circuit_is_available_for_use_ret_false_when_timestamp_dirty(void *arg)
{
  (void)arg;

  circuit_t *circ = tor_malloc(sizeof(circuit_t));
  circ->timestamp_dirty = 1;

  tt_int_op(0, OP_EQ, circuit_is_available_for_use(circ));

  done:
    tor_free(circ);
}

static void
test_circuit_is_available_for_use_ret_false_for_non_general_purpose(void *arg)
{
  (void)arg;

  circuit_t *circ = tor_malloc(sizeof(circuit_t));
  circ->purpose = CIRCUIT_PURPOSE_REND_POINT_WAITING;

  tt_int_op(0, OP_EQ, circuit_is_available_for_use(circ));

  done:
    tor_free(circ);
}

static void
test_circuit_is_available_for_use_ret_false_for_non_general_origin(void *arg)
{
  (void)arg;

  circuit_t *circ = tor_malloc(sizeof(circuit_t));
  circ->purpose = CIRCUIT_PURPOSE_C_INTRODUCE_ACK_WAIT;

  tt_int_op(0, OP_EQ, circuit_is_available_for_use(circ));

  done:
    tor_free(circ);
}

static void
test_circuit_is_available_for_use_ret_false_for_non_origin_purpose(void *arg)
{
  (void)arg;

  circuit_t *circ = tor_malloc(sizeof(circuit_t));
  circ->purpose = CIRCUIT_PURPOSE_OR;

  tt_int_op(0, OP_EQ, circuit_is_available_for_use(circ));

  done:
    tor_free(circ);
}

static void
test_circuit_is_available_for_use_ret_false_unusable_for_new_conns(void *arg)
{
  (void)arg;

  circuit_t *circ = dummy_origin_circuit_new(30);
  mark_circuit_unusable_for_new_conns(TO_ORIGIN_CIRCUIT(circ));

  tt_int_op(0, OP_EQ, circuit_is_available_for_use(circ));

  done:
    circuit_free(circ);
}

static void
test_circuit_is_available_for_use_returns_false_for_onehop_tunnel(void *arg)
{
  (void)arg;

  circuit_t *circ = dummy_origin_circuit_new(30);
  origin_circuit_t *oc = TO_ORIGIN_CIRCUIT(circ);
  oc->build_state = tor_malloc_zero(sizeof(cpath_build_state_t));
  oc->build_state->onehop_tunnel = 1;

  tt_int_op(0, OP_EQ, circuit_is_available_for_use(circ));

  done:
    circuit_free(circ);
}

static void
test_circuit_is_available_for_use_returns_true_for_clean_circuit(void *arg)
{
  (void)arg;

  circuit_t *circ = dummy_origin_circuit_new(30);
  origin_circuit_t *oc = TO_ORIGIN_CIRCUIT(circ);
  oc->build_state = tor_malloc_zero(sizeof(cpath_build_state_t));
  oc->build_state->onehop_tunnel = 0;

  tt_int_op(1, OP_EQ, circuit_is_available_for_use(circ));

  done:
    circuit_free(circ);
}

static int
mock_circuit_all_predicted_ports_handled(time_t now,
                                         int *need_uptime,
                                         int *need_capacity)
{
  (void)now;

  if (need_uptime && need_capacity)
    return 0;
  return 1;
}

static consensus_path_type_t
mock_router_have_unknown_consensus_path(void)
{
  return CONSENSUS_PATH_UNKNOWN;
}

static consensus_path_type_t
mock_router_have_exit_consensus_path(void)
{
  return CONSENSUS_PATH_EXIT;
}

static void
test_needs_exit_circuits_ret_false_for_predicted_ports_and_path(void *arg)
{
  (void)arg;

  MOCK(circuit_all_predicted_ports_handled,
       mock_circuit_all_predicted_ports_handled);
  int needs_uptime = 1;
  int needs_capacity = 0;

  time_t now = time(NULL);
  tt_int_op(0, OP_EQ,
            needs_exit_circuits(now, &needs_uptime, &needs_capacity));

  done:
    UNMOCK(circuit_all_predicted_ports_handled);
}

static void
test_needs_exit_circuits_ret_false_for_non_exit_consensus_path(void *arg)
{
  (void)arg;

  MOCK(circuit_all_predicted_ports_handled,
       mock_circuit_all_predicted_ports_handled);
  int needs_uptime = 1;
  int needs_capacity = 1;
  MOCK(router_have_consensus_path, mock_router_have_unknown_consensus_path);

  time_t now = time(NULL);
  tt_int_op(0, OP_EQ,
            needs_exit_circuits(now, &needs_uptime, &needs_capacity));

  done:
    UNMOCK(circuit_all_predicted_ports_handled);
    UNMOCK(router_have_consensus_path);
}

static void
test_needs_exit_circuits_ret_true_for_predicted_ports_and_path(void *arg)
{
  (void)arg;

  MOCK(circuit_all_predicted_ports_handled,
       mock_circuit_all_predicted_ports_handled);
  int needs_uptime = 1;
  int needs_capacity = 1;
  MOCK(router_have_consensus_path, mock_router_have_exit_consensus_path);

  time_t now = time(NULL);
  tt_int_op(1, OP_EQ,
            needs_exit_circuits(now, &needs_uptime, &needs_capacity));

  done:
    UNMOCK(circuit_all_predicted_ports_handled);
    UNMOCK(router_have_consensus_path);
}

static void
test_needs_circuits_for_build_ret_false_consensus_path_unknown(void *arg)
{
  (void)arg;
  MOCK(router_have_consensus_path, mock_router_have_unknown_consensus_path);
  tt_int_op(0, OP_EQ, needs_circuits_for_build(0));
  done: ;
}

static void
test_needs_circuits_for_build_ret_false_if_num_less_than_max(void *arg)
{
  (void)arg;
  MOCK(router_have_consensus_path, mock_router_have_exit_consensus_path);
  tt_int_op(0, OP_EQ, needs_circuits_for_build(13));
  done:
    UNMOCK(router_have_consensus_path);
}

static void
test_needs_circuits_for_build_returns_true_when_more_are_needed(void *arg)
{
  (void)arg;
  MOCK(router_have_consensus_path, mock_router_have_exit_consensus_path);
  tt_int_op(1, OP_EQ, needs_circuits_for_build(0));
  done:
    UNMOCK(router_have_consensus_path);
}

struct testcase_t circuituse_tests[] = {
 { "marked",
   test_circuit_is_available_for_use_ret_false_when_marked_for_close,
   TT_FORK, NULL, NULL
 },
 { "timestamp",
   test_circuit_is_available_for_use_ret_false_when_timestamp_dirty,
   TT_FORK, NULL, NULL
 },
 { "non_general",
   test_circuit_is_available_for_use_ret_false_for_non_general_purpose,
   TT_FORK, NULL, NULL
 },
 { "non_general",
  test_circuit_is_available_for_use_ret_false_for_non_general_origin,
   TT_FORK, NULL, NULL
 },
 { "origin",
   test_circuit_is_available_for_use_ret_false_for_non_origin_purpose,
   TT_FORK, NULL, NULL
 },
 { "clean",
   test_circuit_is_available_for_use_ret_false_unusable_for_new_conns,
   TT_FORK, NULL, NULL
 },
 { "onehop",
   test_circuit_is_available_for_use_returns_false_for_onehop_tunnel,
   TT_FORK, NULL, NULL
 },
 { "clean_circ",
   test_circuit_is_available_for_use_returns_true_for_clean_circuit,
   TT_FORK, NULL, NULL
 },
 { "exit_f",
   test_needs_exit_circuits_ret_false_for_predicted_ports_and_path,
   TT_FORK, NULL, NULL
 },
 { "exit_t",
   test_needs_exit_circuits_ret_true_for_predicted_ports_and_path,
   TT_FORK, NULL, NULL
 },
 { "non_exit",
   test_needs_exit_circuits_ret_false_for_non_exit_consensus_path,
   TT_FORK, NULL, NULL
 },
 { "true",
   test_needs_exit_circuits_ret_true_for_predicted_ports_and_path,
   TT_FORK, NULL, NULL
 },
 { "consensus_path_unknown",
   test_needs_circuits_for_build_ret_false_consensus_path_unknown,
   TT_FORK, NULL, NULL
 },
 { "less_than_max",
   test_needs_circuits_for_build_ret_false_if_num_less_than_max,
   TT_FORK, NULL, NULL
 },
 { "more_needed",
   test_needs_circuits_for_build_returns_true_when_more_are_needed,
   TT_FORK, NULL, NULL
 },
  END_OF_TESTCASES
};

