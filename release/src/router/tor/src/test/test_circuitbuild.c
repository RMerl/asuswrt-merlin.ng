/* Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

#define CIRCUITBUILD_PRIVATE
#define CIRCUITLIST_PRIVATE
#define ENTRYNODES_PRIVATE

#include "core/or/or.h"

#include "test/test.h"
#include "test/test_helpers.h"
#include "test/log_test_helpers.h"

#define CONFIG_PRIVATE
#include "app/config/config.h"

#include "core/or/channel.h"
#include "core/or/circuitbuild.h"
#include "core/or/circuitlist.h"
#include "core/or/circuituse.h"
#include "core/or/onion.h"

#include "core/or/cell_st.h"
#include "core/or/cpath_build_state_st.h"
#include "core/or/extend_info_st.h"
#include "core/or/origin_circuit_st.h"
#include "core/or/or_circuit_st.h"

#include "feature/client/entrynodes.h"
#include "feature/nodelist/nodelist.h"
#include "feature/nodelist/node_select.h"
#include "feature/relay/circuitbuild_relay.h"
#include "feature/relay/router.h"
#include "feature/relay/routermode.h"

#include "feature/nodelist/node_st.h"
#include "feature/nodelist/routerinfo_st.h"

/* Dummy nodes smartlist for testing */
static smartlist_t dummy_nodes;
/* Dummy exit extend_info for testing */
static extend_info_t dummy_ei;

static int
mock_count_acceptable_nodes(const smartlist_t *nodes, int direct)
{
  (void)nodes;

  return direct ? 1 : DEFAULT_ROUTE_LEN + 1;
}

/* Test route lengths when the caller of new_route_len() doesn't
 * specify exit_ei. */
static void
test_new_route_len_noexit(void *arg)
{
  int r;

  (void)arg;
  MOCK(count_acceptable_nodes, mock_count_acceptable_nodes);

  r = new_route_len(CIRCUIT_PURPOSE_C_GENERAL, NULL, &dummy_nodes);
  tt_int_op(DEFAULT_ROUTE_LEN, OP_EQ, r);

  r = new_route_len(CIRCUIT_PURPOSE_C_INTRODUCE_ACK_WAIT, NULL, &dummy_nodes);
  tt_int_op(DEFAULT_ROUTE_LEN, OP_EQ, r);

  r = new_route_len(CIRCUIT_PURPOSE_S_CONNECT_REND, NULL, &dummy_nodes);
  tt_int_op(DEFAULT_ROUTE_LEN, OP_EQ, r);

 done:
  UNMOCK(count_acceptable_nodes);
}

/* Test route lengths where someone else chose the "exit" node, which
 * require an extra hop for safety. */
static void
test_new_route_len_unsafe_exit(void *arg)
{
  int r;

  (void)arg;
  MOCK(count_acceptable_nodes, mock_count_acceptable_nodes);

  /* connecting to hidden service directory */
  r = new_route_len(CIRCUIT_PURPOSE_C_GENERAL, &dummy_ei, &dummy_nodes);
  tt_int_op(DEFAULT_ROUTE_LEN + 1, OP_EQ, r);

  /* client connecting to introduction point */
  r = new_route_len(CIRCUIT_PURPOSE_C_INTRODUCING, &dummy_ei, &dummy_nodes);
  tt_int_op(DEFAULT_ROUTE_LEN + 1, OP_EQ, r);

  /* hidden service connecting to rendezvous point */
  r = new_route_len(CIRCUIT_PURPOSE_S_CONNECT_REND, &dummy_ei, &dummy_nodes);
  tt_int_op(DEFAULT_ROUTE_LEN + 1, OP_EQ, r);

 done:
  UNMOCK(count_acceptable_nodes);
}

/* Test route lengths where we chose the "exit" node, which don't
 * require an extra hop for safety. */
static void
test_new_route_len_safe_exit(void *arg)
{
  int r;

  (void)arg;
  MOCK(count_acceptable_nodes, mock_count_acceptable_nodes);

  /* hidden service connecting to introduction point */
  r = new_route_len(CIRCUIT_PURPOSE_S_ESTABLISH_INTRO, &dummy_ei,
                    &dummy_nodes);
  tt_int_op(DEFAULT_ROUTE_LEN, OP_EQ, r);

  /* router testing its own reachability */
  r = new_route_len(CIRCUIT_PURPOSE_TESTING, &dummy_ei, &dummy_nodes);
  tt_int_op(DEFAULT_ROUTE_LEN, OP_EQ, r);

 done:
  UNMOCK(count_acceptable_nodes);
}

/* Make sure a non-fatal assertion fails when new_route_len() gets an
 * unexpected circuit purpose. */
static void
test_new_route_len_unhandled_exit(void *arg)
{
  int r;

  (void)arg;
#ifdef ALL_BUGS_ARE_FATAL
  /* Coverity (and maybe clang analyser) complain that the code following
   * tt_skip() is unconditionally unreachable. */
#if !defined(__COVERITY__) && !defined(__clang_analyzer__)
  tt_skip();
#endif
#endif /* defined(ALL_BUGS_ARE_FATAL) */

  MOCK(count_acceptable_nodes, mock_count_acceptable_nodes);

  tor_capture_bugs_(1);
  setup_full_capture_of_logs(LOG_WARN);
  r = new_route_len(CIRCUIT_PURPOSE_CONTROLLER, &dummy_ei, &dummy_nodes);
  tt_int_op(DEFAULT_ROUTE_LEN + 1, OP_EQ, r);
  tt_int_op(smartlist_len(tor_get_captured_bug_log_()), OP_EQ, 1);
  tt_str_op(smartlist_get(tor_get_captured_bug_log_(), 0), OP_EQ,
            "!(exit_ei && !known_purpose)");
  expect_single_log_msg_containing("Unhandled purpose");
  expect_single_log_msg_containing("with a chosen exit; assuming routelen");

 done:
  teardown_capture_of_logs();
  tor_end_capture_bugs_();
  UNMOCK(count_acceptable_nodes);
}

static void
test_upgrade_from_guard_wait(void *arg)
{
  circuit_t *circ = NULL;
  origin_circuit_t *orig_circ = NULL;
  entry_guard_t *guard = NULL;
  smartlist_t *list = NULL;

  (void) arg;

  circ = dummy_origin_circuit_new(0);
  orig_circ = TO_ORIGIN_CIRCUIT(circ);
  tt_assert(orig_circ);

  orig_circ->build_state = tor_malloc_zero(sizeof(cpath_build_state_t));

  circuit_set_state(circ, CIRCUIT_STATE_GUARD_WAIT);

  /* Put it in guard wait state. */
  guard = tor_malloc_zero(sizeof(*guard));
  guard->in_selection = get_guard_selection_info();

  orig_circ->guard_state =
    circuit_guard_state_new(guard, GUARD_CIRC_STATE_WAITING_FOR_BETTER_GUARD,
                            NULL);

  /* Mark the circuit for close. */
  circuit_mark_for_close(circ, END_CIRC_REASON_TORPROTOCOL);
  tt_int_op(circ->marked_for_close, OP_NE, 0);

  /* We shouldn't pick the mark for close circuit. */
  list = circuit_find_circuits_to_upgrade_from_guard_wait();
  tt_assert(!list);

 done:
  smartlist_free(list);
  circuit_free(circ);
  entry_guard_free_(guard);
}

static int server = 0;
static int
mock_server_mode(const or_options_t *options)
{
  (void)options;
  return server;
}

/* Test the different cases in circuit_extend_state_valid_helper(). */
static void
test_circuit_extend_state_valid(void *arg)
{
  (void)arg;
  circuit_t *circ = tor_malloc_zero(sizeof(circuit_t));

  server = 0;
  MOCK(server_mode, mock_server_mode);

  setup_full_capture_of_logs(LOG_INFO);

  /* Clients can't extend */
  server = 0;
  tt_int_op(circuit_extend_state_valid_helper(NULL), OP_EQ, -1);
  expect_log_msg("Got an extend cell, but running as a client. Closing.\n");
  mock_clean_saved_logs();

#ifndef ALL_BUGS_ARE_FATAL
  /* Circuit must be non-NULL */
  tor_capture_bugs_(1);
  server = 1;
  tt_int_op(circuit_extend_state_valid_helper(NULL), OP_EQ, -1);
  tt_int_op(smartlist_len(tor_get_captured_bug_log_()), OP_EQ, 1);
  tt_str_op(smartlist_get(tor_get_captured_bug_log_(), 0), OP_EQ,
            "!(ASSERT_PREDICT_UNLIKELY_(!circ))");
  tor_end_capture_bugs_();
  mock_clean_saved_logs();
#endif /* !defined(ALL_BUGS_ARE_FATAL) */

  /* n_chan and n_hop are NULL, this should succeed */
  server = 1;
  tt_int_op(circuit_extend_state_valid_helper(circ), OP_EQ, 0);
  mock_clean_saved_logs();

  /* But clients still can't extend */
  server = 0;
  tt_int_op(circuit_extend_state_valid_helper(circ), OP_EQ, -1);
  expect_log_msg("Got an extend cell, but running as a client. Closing.\n");
  mock_clean_saved_logs();

  /* n_chan must be NULL */
  circ->n_chan = tor_malloc_zero(sizeof(channel_t));
  server = 1;
  tt_int_op(circuit_extend_state_valid_helper(circ), OP_EQ, -1);
  expect_log_msg("n_chan already set. Bug/attack. Closing.\n");
  mock_clean_saved_logs();
  tor_free(circ->n_chan);

  /* n_hop must be NULL */
  circ->n_hop = tor_malloc_zero(sizeof(extend_info_t));
  server = 1;
  tt_int_op(circuit_extend_state_valid_helper(circ), OP_EQ, -1);
  expect_log_msg("conn to next hop already launched. Bug/attack. Closing.\n");
  mock_clean_saved_logs();
  tor_free(circ->n_hop);

 done:
  tor_end_capture_bugs_();
  teardown_capture_of_logs();

  UNMOCK(server_mode);
  server = 0;

  tor_free(circ->n_chan);
  tor_free(circ->n_hop);
  tor_free(circ);
}

static node_t *mocked_node = NULL;
static const node_t *
mock_node_get_by_id(const char *identity_digest)
{
  (void)identity_digest;
  return mocked_node;
}

static bool mocked_supports_ed25519_link_authentication = 0;
static bool
mock_node_supports_ed25519_link_authentication(const node_t *node,
                                               bool compatible_with_us)
{
  (void)node;
  (void)compatible_with_us;
  return mocked_supports_ed25519_link_authentication;
}

static ed25519_public_key_t * mocked_ed25519_id = NULL;
static const ed25519_public_key_t *
mock_node_get_ed25519_id(const node_t *node)
{
  (void)node;
  return mocked_ed25519_id;
}

/* Test the different cases in circuit_extend_add_ed25519_helper(). */
static void
test_circuit_extend_add_ed25519(void *arg)
{
  (void)arg;
  extend_cell_t *ec = tor_malloc_zero(sizeof(extend_cell_t));
  extend_cell_t *old_ec = tor_malloc_zero(sizeof(extend_cell_t));
  extend_cell_t *zero_ec = tor_malloc_zero(sizeof(extend_cell_t));

  node_t *fake_node = tor_malloc_zero(sizeof(node_t));
  ed25519_public_key_t *fake_ed25519_id = NULL;
  fake_ed25519_id = tor_malloc_zero(sizeof(ed25519_public_key_t));

  MOCK(node_get_by_id, mock_node_get_by_id);
  MOCK(node_supports_ed25519_link_authentication,
       mock_node_supports_ed25519_link_authentication);
  MOCK(node_get_ed25519_id, mock_node_get_ed25519_id);

  setup_full_capture_of_logs(LOG_INFO);

#ifndef ALL_BUGS_ARE_FATAL
  /* The extend cell must be non-NULL */
  tor_capture_bugs_(1);
  tt_int_op(circuit_extend_add_ed25519_helper(NULL), OP_EQ, -1);
  tt_int_op(smartlist_len(tor_get_captured_bug_log_()), OP_EQ, 1);
  tt_str_op(smartlist_get(tor_get_captured_bug_log_(), 0), OP_EQ,
            "!(ASSERT_PREDICT_UNLIKELY_(!ec))");
  tor_end_capture_bugs_();
  mock_clean_saved_logs();
#endif /* !defined(ALL_BUGS_ARE_FATAL) */

  /* The node id must be non-zero */
  memcpy(old_ec, ec, sizeof(extend_cell_t));
  tt_int_op(circuit_extend_add_ed25519_helper(ec), OP_EQ, -1);
  expect_log_msg(
    "Client asked me to extend without specifying an id_digest.\n");
  /* And nothing should have changed */
  tt_mem_op(ec, OP_EQ, old_ec, sizeof(extend_cell_t));
  mock_clean_saved_logs();

  /* Fill in fake node_id, and try again */
  memset(ec->node_id, 0xAA, sizeof(ec->node_id));
  memcpy(old_ec, ec, sizeof(extend_cell_t));
  tt_int_op(circuit_extend_add_ed25519_helper(ec), OP_EQ, 0);
  /* There's no node with that id, so the ed pubkey should still be zeroed */
  tt_mem_op(&ec->ed_pubkey, OP_EQ, &zero_ec->ed_pubkey, sizeof(ec->ed_pubkey));
  /* In fact, nothing should have changed */
  tt_mem_op(ec, OP_EQ, old_ec, sizeof(extend_cell_t));
  mock_clean_saved_logs();

  /* Provide 2 out of 3 of node, supports link auth, and ed_id.
   * The ed_id should remain zeroed. */

  /* Provide node and supports link auth */
  memset(ec->node_id, 0xAA, sizeof(ec->node_id));
  memcpy(old_ec, ec, sizeof(extend_cell_t));
  /* Set up the fake variables */
  mocked_node = fake_node;
  mocked_supports_ed25519_link_authentication = 1;
  /* Do the test */
  tt_int_op(circuit_extend_add_ed25519_helper(ec), OP_EQ, 0);
  /* The ed pubkey should still be zeroed */
  tt_mem_op(&ec->ed_pubkey, OP_EQ, &zero_ec->ed_pubkey, sizeof(ec->ed_pubkey));
  /* In fact, nothing should have changed */
  tt_mem_op(ec, OP_EQ, old_ec, sizeof(extend_cell_t));
  /* Cleanup */
  mock_clean_saved_logs();
  mocked_node = NULL;
  mocked_supports_ed25519_link_authentication = 0;
  mocked_ed25519_id = NULL;
  memset(fake_ed25519_id, 0x00, sizeof(ed25519_public_key_t));

  /* Provide supports link auth and ed id */
  memset(ec->node_id, 0xAA, sizeof(ec->node_id));
  memcpy(old_ec, ec, sizeof(extend_cell_t));
  /* Set up the fake variables */
  mocked_supports_ed25519_link_authentication = 1;
  memset(fake_ed25519_id, 0xEE, sizeof(ed25519_public_key_t));
  mocked_ed25519_id = fake_ed25519_id;
  /* Do the test */
  tt_int_op(circuit_extend_add_ed25519_helper(ec), OP_EQ, 0);
  /* The ed pubkey should still be zeroed */
  tt_mem_op(&ec->ed_pubkey, OP_EQ, &zero_ec->ed_pubkey, sizeof(ec->ed_pubkey));
  /* In fact, nothing should have changed */
  tt_mem_op(ec, OP_EQ, old_ec, sizeof(extend_cell_t));
  /* Cleanup */
  mock_clean_saved_logs();
  mocked_node = NULL;
  mocked_supports_ed25519_link_authentication = 0;
  mocked_ed25519_id = NULL;
  memset(fake_ed25519_id, 0x00, sizeof(ed25519_public_key_t));

  /* Provide node and ed id */
  memset(ec->node_id, 0xAA, sizeof(ec->node_id));
  memcpy(old_ec, ec, sizeof(extend_cell_t));
  /* Set up the fake variables */
  mocked_node = fake_node;
  memset(fake_ed25519_id, 0xEE, sizeof(ed25519_public_key_t));
  mocked_ed25519_id = fake_ed25519_id;
  /* Do the test */
  tt_int_op(circuit_extend_add_ed25519_helper(ec), OP_EQ, 0);
  /* The ed pubkey should still be zeroed */
  tt_mem_op(&ec->ed_pubkey, OP_EQ, &zero_ec->ed_pubkey, sizeof(ec->ed_pubkey));
  /* In fact, nothing should have changed */
  tt_mem_op(ec, OP_EQ, old_ec, sizeof(extend_cell_t));
  /* Cleanup */
  mock_clean_saved_logs();
  mocked_node = NULL;
  mocked_supports_ed25519_link_authentication = 0;
  mocked_ed25519_id = NULL;
  memset(fake_ed25519_id, 0x00, sizeof(ed25519_public_key_t));

  /* Now do the real lookup */
  memset(ec->node_id, 0xAA, sizeof(ec->node_id));
  memcpy(old_ec, ec, sizeof(extend_cell_t));
  /* Set up the fake variables */
  mocked_node = fake_node;
  mocked_supports_ed25519_link_authentication = 1;
  memset(fake_ed25519_id, 0xEE, sizeof(ed25519_public_key_t));
  mocked_ed25519_id = fake_ed25519_id;
  /* Do the test */
  tt_int_op(circuit_extend_add_ed25519_helper(ec), OP_EQ, 0);
  /* The ed pubkey should match */
  tt_mem_op(&ec->ed_pubkey, OP_EQ, fake_ed25519_id, sizeof(ec->ed_pubkey));
  /* Nothing else should have changed */
  memcpy(&ec->ed_pubkey, &old_ec->ed_pubkey, sizeof(ec->ed_pubkey));
  tt_mem_op(ec, OP_EQ, old_ec, sizeof(extend_cell_t));
  /* Cleanup */
  mock_clean_saved_logs();
  mocked_node = NULL;
  mocked_supports_ed25519_link_authentication = 0;
  mocked_ed25519_id = NULL;
  memset(fake_ed25519_id, 0x00, sizeof(ed25519_public_key_t));

  /* Now do the real lookup, but with a zeroed ed id */
  memset(ec->node_id, 0xAA, sizeof(ec->node_id));
  memcpy(old_ec, ec, sizeof(extend_cell_t));
  /* Set up the fake variables */
  mocked_node = fake_node;
  mocked_supports_ed25519_link_authentication = 1;
  memset(fake_ed25519_id, 0x00, sizeof(ed25519_public_key_t));
  mocked_ed25519_id = fake_ed25519_id;
  /* Do the test */
  tt_int_op(circuit_extend_add_ed25519_helper(ec), OP_EQ, 0);
  /* The ed pubkey should match */
  tt_mem_op(&ec->ed_pubkey, OP_EQ, fake_ed25519_id, sizeof(ec->ed_pubkey));
  /* Nothing else should have changed */
  memcpy(&ec->ed_pubkey, &old_ec->ed_pubkey, sizeof(ec->ed_pubkey));
  tt_mem_op(ec, OP_EQ, old_ec, sizeof(extend_cell_t));
  /* Cleanup */
  mock_clean_saved_logs();
  mocked_node = NULL;
  mocked_supports_ed25519_link_authentication = 0;
  mocked_ed25519_id = NULL;
  memset(fake_ed25519_id, 0x00, sizeof(ed25519_public_key_t));

 done:
  UNMOCK(node_get_by_id);
  UNMOCK(node_supports_ed25519_link_authentication);
  UNMOCK(node_get_ed25519_id);

  tor_end_capture_bugs_();
  teardown_capture_of_logs();

  tor_free(ec);
  tor_free(old_ec);
  tor_free(zero_ec);

  tor_free(fake_ed25519_id);
  tor_free(fake_node);
}

static or_options_t *mocked_options = NULL;
static const or_options_t *
mock_get_options(void)
{
  return mocked_options;
}

#define PUBLIC_IPV4   "1.2.3.4"
#define INTERNAL_IPV4 "0.0.0.1"

#define PUBLIC_IPV6   "1234::cdef"
#define INTERNAL_IPV6 "::1"

#define VALID_PORT    0x1234

/* Test the different cases in circuit_extend_lspec_valid_helper(). */
static void
test_circuit_extend_lspec_valid(void *arg)
{
  (void)arg;
  extend_cell_t *ec = tor_malloc_zero(sizeof(extend_cell_t));
  channel_t *p_chan = tor_malloc_zero(sizeof(channel_t));
  or_circuit_t *or_circ = tor_malloc_zero(sizeof(or_circuit_t));
  circuit_t *circ = TO_CIRCUIT(or_circ);

  or_options_t *fake_options = options_new();
  MOCK(get_options, mock_get_options);
  mocked_options = fake_options;

  setup_full_capture_of_logs(LOG_INFO);

#ifndef ALL_BUGS_ARE_FATAL
  /* Extend cell must be non-NULL */
  tor_capture_bugs_(1);
  tt_int_op(circuit_extend_lspec_valid_helper(NULL, circ), OP_EQ, -1);
  tt_int_op(smartlist_len(tor_get_captured_bug_log_()), OP_EQ, 1);
  tt_str_op(smartlist_get(tor_get_captured_bug_log_(), 0), OP_EQ,
            "!(ASSERT_PREDICT_UNLIKELY_(!ec))");
  tor_end_capture_bugs_();
  mock_clean_saved_logs();

  /* Circuit must be non-NULL */
  tor_capture_bugs_(1);
  tt_int_op(circuit_extend_lspec_valid_helper(ec, NULL), OP_EQ, -1);
  tt_int_op(smartlist_len(tor_get_captured_bug_log_()), OP_EQ, 1);
  tt_str_op(smartlist_get(tor_get_captured_bug_log_(), 0), OP_EQ,
            "!(ASSERT_PREDICT_UNLIKELY_(!circ))");
  tor_end_capture_bugs_();
  mock_clean_saved_logs();

  /* Extend cell and circuit must be non-NULL */
  tor_capture_bugs_(1);
  tt_int_op(circuit_extend_lspec_valid_helper(NULL, NULL), OP_EQ, -1);
  /* Since we're using IF_BUG_ONCE(), we might not log any bugs */
  tt_int_op(smartlist_len(tor_get_captured_bug_log_()), OP_GE, 0);
  tt_int_op(smartlist_len(tor_get_captured_bug_log_()), OP_LE, 2);
  tor_end_capture_bugs_();
  mock_clean_saved_logs();
#endif /* !defined(ALL_BUGS_ARE_FATAL) */

  /* IPv4 and IPv6 addr and port are all zero, this should fail */
  tt_int_op(circuit_extend_lspec_valid_helper(ec, circ), OP_EQ, -1);
  expect_log_msg("Client asked me to extend to a zero destination port "
                 "or unspecified address '[scrubbed]'.\n");
  mock_clean_saved_logs();

  /* Now ask for the actual address in the logs */
  fake_options->SafeLogging_ = SAFELOG_SCRUB_NONE;

  /* IPv4 port is 0, IPv6 addr and port are both zero, this should fail */
  tor_addr_parse(&ec->orport_ipv4.addr, PUBLIC_IPV4);
  tt_int_op(circuit_extend_lspec_valid_helper(ec, circ), OP_EQ, -1);
  expect_log_msg("Client asked me to extend to a zero destination port "
                 "or IPv4 address '1.2.3.4:0'.\n");
  mock_clean_saved_logs();
  tor_addr_port_make_null_ap(&ec->orport_ipv4, AF_INET);
  tor_addr_port_make_null_ap(&ec->orport_ipv6, AF_INET6);

  /* IPv4 addr is 0, IPv6 addr and port are both zero, this should fail */
  ec->orport_ipv4.port = VALID_PORT;
  tt_int_op(circuit_extend_lspec_valid_helper(ec, circ), OP_EQ, -1);
  expect_log_msg("Client asked me to extend to a zero destination port "
                 "or IPv4 address '0.0.0.0:4660'.\n");
  mock_clean_saved_logs();
  ec->orport_ipv4.port = 0;
  tor_addr_port_make_null_ap(&ec->orport_ipv4, AF_INET);
  tor_addr_port_make_null_ap(&ec->orport_ipv6, AF_INET6);

  /* IPv4 addr is internal, and port is valid.
   * (IPv6 addr and port are both zero.)
   * Result depends on ExtendAllowPrivateAddresses. */
  tor_addr_parse(&ec->orport_ipv4.addr, INTERNAL_IPV4);
  ec->orport_ipv4.port = VALID_PORT;

  fake_options->ExtendAllowPrivateAddresses = 0;
  tt_int_op(circuit_extend_lspec_valid_helper(ec, circ), OP_EQ, -1);
  expect_log_msg("Client asked me to extend "
                 "to a private IPv4 address '0.0.0.1'.\n");
  mock_clean_saved_logs();
  fake_options->ExtendAllowPrivateAddresses = 0;
  tor_addr_port_make_null_ap(&ec->orport_ipv4, AF_INET);
  tor_addr_port_make_null_ap(&ec->orport_ipv6, AF_INET6);

  /* Now do the same tests, but for IPv6 */

  /* IPv6 port is 0, IPv4 addr and port are both zero, this should fail */
  tor_addr_parse(&ec->orport_ipv6.addr, PUBLIC_IPV6);
  tt_int_op(circuit_extend_lspec_valid_helper(ec, circ), OP_EQ, -1);
  expect_log_msg("Client asked me to extend to a zero destination port "
                 "or IPv6 address '[1234::cdef]:0'.\n");
  mock_clean_saved_logs();
  tor_addr_port_make_null_ap(&ec->orport_ipv4, AF_INET);
  tor_addr_port_make_null_ap(&ec->orport_ipv6, AF_INET6);

  /* IPv6 addr is 0, IPv4 addr and port are both zero, this should fail */
  ec->orport_ipv6.port = VALID_PORT;
  tt_int_op(circuit_extend_lspec_valid_helper(ec, circ), OP_EQ, -1);
  expect_log_msg("Client asked me to extend to a zero destination port "
                 "or IPv6 address '[::]:4660'.\n");
  mock_clean_saved_logs();
  ec->orport_ipv4.port = 0;
  tor_addr_port_make_null_ap(&ec->orport_ipv4, AF_INET);
  tor_addr_port_make_null_ap(&ec->orport_ipv6, AF_INET6);

  /* IPv6 addr is internal, and port is valid.
   * (IPv4 addr and port are both zero.)
   * Result depends on ExtendAllowPrivateAddresses. */
  tor_addr_parse(&ec->orport_ipv6.addr, INTERNAL_IPV6);
  ec->orport_ipv6.port = VALID_PORT;

  fake_options->ExtendAllowPrivateAddresses = 0;
  tt_int_op(circuit_extend_lspec_valid_helper(ec, circ), OP_EQ, -1);
  expect_log_msg("Client asked me to extend "
                 "to a private IPv6 address '[::1]'.\n");
  mock_clean_saved_logs();
  fake_options->ExtendAllowPrivateAddresses = 0;
  tor_addr_port_make_null_ap(&ec->orport_ipv4, AF_INET);
  tor_addr_port_make_null_ap(&ec->orport_ipv6, AF_INET6);

  /* Both addresses are internal.
   * Result depends on ExtendAllowPrivateAddresses. */
  tor_addr_parse(&ec->orport_ipv4.addr, INTERNAL_IPV4);
  ec->orport_ipv4.port = VALID_PORT;
  tor_addr_parse(&ec->orport_ipv6.addr, INTERNAL_IPV6);
  ec->orport_ipv6.port = VALID_PORT;

  fake_options->ExtendAllowPrivateAddresses = 0;
  tt_int_op(circuit_extend_lspec_valid_helper(ec, circ), OP_EQ, -1);
  expect_log_msg("Client asked me to extend "
                 "to a private IPv4 address '0.0.0.1'.\n");
  expect_log_msg("Client asked me to extend "
                 "to a private IPv6 address '[::1]'.\n");
  mock_clean_saved_logs();
  fake_options->ExtendAllowPrivateAddresses = 0;
  tor_addr_port_make_null_ap(&ec->orport_ipv4, AF_INET);
  tor_addr_port_make_null_ap(&ec->orport_ipv6, AF_INET6);

#ifndef ALL_BUGS_ARE_FATAL
  /* If we pass the private address check, but don't have the right
   * OR circuit magic number, we trigger another bug */
  tor_addr_parse(&ec->orport_ipv4.addr, INTERNAL_IPV4);
  ec->orport_ipv4.port = VALID_PORT;
  tor_addr_parse(&ec->orport_ipv6.addr, INTERNAL_IPV6);
  ec->orport_ipv6.port = VALID_PORT;
  fake_options->ExtendAllowPrivateAddresses = 1;

  tor_capture_bugs_(1);
  tt_int_op(circuit_extend_lspec_valid_helper(ec, circ), OP_EQ, -1);
  tt_int_op(smartlist_len(tor_get_captured_bug_log_()), OP_EQ, 1);
  tt_str_op(smartlist_get(tor_get_captured_bug_log_(), 0), OP_EQ,
            "!(ASSERT_PREDICT_UNLIKELY_(circ->magic != 0x98ABC04Fu))");
  tor_end_capture_bugs_();
  mock_clean_saved_logs();
  fake_options->ExtendAllowPrivateAddresses = 0;
  tor_addr_port_make_null_ap(&ec->orport_ipv4, AF_INET);
  tor_addr_port_make_null_ap(&ec->orport_ipv6, AF_INET6);

  /* Fail again, but this time only set an IPv4 address. */
  tor_addr_parse(&ec->orport_ipv4.addr, INTERNAL_IPV4);
  ec->orport_ipv4.port = VALID_PORT;
  fake_options->ExtendAllowPrivateAddresses = 1;
  tor_capture_bugs_(1);
  tt_int_op(circuit_extend_lspec_valid_helper(ec, circ), OP_EQ, -1);
  /* Since we're using IF_BUG_ONCE(), expect 0-1 bug logs */
  tt_int_op(smartlist_len(tor_get_captured_bug_log_()), OP_GE, 0);
  tt_int_op(smartlist_len(tor_get_captured_bug_log_()), OP_LE, 1);
  tor_end_capture_bugs_();
  mock_clean_saved_logs();
  fake_options->ExtendAllowPrivateAddresses = 0;
#endif /* !defined(ALL_BUGS_ARE_FATAL) */

  /* Now set the right magic */
  or_circ->base_.magic = OR_CIRCUIT_MAGIC;

#ifndef ALL_BUGS_ARE_FATAL
  /* If we pass the OR circuit magic check, but don't have p_chan,
   * we trigger another bug */
  fake_options->ExtendAllowPrivateAddresses = 1;
  tor_capture_bugs_(1);
  tt_int_op(circuit_extend_lspec_valid_helper(ec, circ), OP_EQ, -1);
  tt_int_op(smartlist_len(tor_get_captured_bug_log_()), OP_EQ, 1);
  tt_str_op(smartlist_get(tor_get_captured_bug_log_(), 0), OP_EQ,
            "!(ASSERT_PREDICT_UNLIKELY_(!p_chan))");
  tor_end_capture_bugs_();
  mock_clean_saved_logs();
  fake_options->ExtendAllowPrivateAddresses = 0;

  /* We can also pass the OR circuit magic check with a public address */
  tor_addr_parse(&ec->orport_ipv4.addr, PUBLIC_IPV4);
  fake_options->ExtendAllowPrivateAddresses = 0;
  tor_capture_bugs_(1);
  tt_int_op(circuit_extend_lspec_valid_helper(ec, circ), OP_EQ, -1);
  /* Since we're using IF_BUG_ONCE(), expect 0-1 bug logs */
  tt_int_op(smartlist_len(tor_get_captured_bug_log_()), OP_GE, 0);
  tt_int_op(smartlist_len(tor_get_captured_bug_log_()), OP_LE, 1);
  tor_end_capture_bugs_();
  mock_clean_saved_logs();
  fake_options->ExtendAllowPrivateAddresses = 0;

  tor_addr_make_null(&ec->orport_ipv4.addr, AF_INET);
  ec->orport_ipv4.port = 0x0000;
#endif /* !defined(ALL_BUGS_ARE_FATAL) */

  /* Now let's fake a p_chan and the addresses */
  tor_addr_parse(&ec->orport_ipv4.addr, PUBLIC_IPV4);
  ec->orport_ipv4.port = VALID_PORT;
  or_circ->p_chan = p_chan;

  /* This is a trivial failure: node_id and p_chan->identity_digest are both
   * zeroed */
  tt_int_op(circuit_extend_lspec_valid_helper(ec, circ), OP_EQ, -1);
  expect_log_msg("Client asked me to extend back to the previous hop.\n");
  mock_clean_saved_logs();

  /* Let's check with non-zero identities as well */
  memset(ec->node_id, 0xAA, sizeof(ec->node_id));
  memset(p_chan->identity_digest, 0xAA, sizeof(p_chan->identity_digest));

  tt_int_op(circuit_extend_lspec_valid_helper(ec, circ), OP_EQ, -1);
  expect_log_msg("Client asked me to extend back to the previous hop.\n");
  mock_clean_saved_logs();

  memset(ec->node_id, 0, sizeof(ec->node_id));
  memset(p_chan->identity_digest, 0, sizeof(p_chan->identity_digest));

  /* Let's pass the node_id test */
  memset(ec->node_id, 0xAA, sizeof(ec->node_id));
  memset(p_chan->identity_digest, 0xBB, sizeof(p_chan->identity_digest));

  /* ed_pubkey is zero, and that's allowed, so we should succeed */
  tt_int_op(circuit_extend_lspec_valid_helper(ec, circ), OP_EQ, 0);
  mock_clean_saved_logs();

  /* Now let's check that we warn, but succeed, when only one address is
   * private */
  tor_addr_parse(&ec->orport_ipv4.addr, INTERNAL_IPV4);
  ec->orport_ipv4.port = VALID_PORT;
  tor_addr_parse(&ec->orport_ipv6.addr, PUBLIC_IPV6);
  ec->orport_ipv6.port = VALID_PORT;
  fake_options->ExtendAllowPrivateAddresses = 0;

  tt_int_op(circuit_extend_lspec_valid_helper(ec, circ), OP_EQ, 0);
  expect_log_msg("Client asked me to extend "
                 "to a private IPv4 address '0.0.0.1'.\n");
  mock_clean_saved_logs();
  tor_addr_port_make_null_ap(&ec->orport_ipv4, AF_INET);
  tor_addr_port_make_null_ap(&ec->orport_ipv6, AF_INET6);

  /* Now with private IPv6 */
  tor_addr_parse(&ec->orport_ipv4.addr, PUBLIC_IPV4);
  ec->orport_ipv4.port = VALID_PORT;
  tor_addr_parse(&ec->orport_ipv6.addr, INTERNAL_IPV6);
  ec->orport_ipv6.port = VALID_PORT;
  fake_options->ExtendAllowPrivateAddresses = 0;

  tt_int_op(circuit_extend_lspec_valid_helper(ec, circ), OP_EQ, 0);
  expect_log_msg("Client asked me to extend "
                 "to a private IPv6 address '[::1]'.\n");
  mock_clean_saved_logs();
  tor_addr_port_make_null_ap(&ec->orport_ipv4, AF_INET);
  tor_addr_port_make_null_ap(&ec->orport_ipv6, AF_INET6);

  /* Now reset to public IPv4 and IPv6 */
  tor_addr_parse(&ec->orport_ipv4.addr, PUBLIC_IPV4);
  ec->orport_ipv4.port = VALID_PORT;
  tor_addr_parse(&ec->orport_ipv6.addr, PUBLIC_IPV6);
  ec->orport_ipv6.port = VALID_PORT;

  /* Fail on matching non-zero identities */
  memset(&ec->ed_pubkey, 0xEE, sizeof(ec->ed_pubkey));
  memset(&p_chan->ed25519_identity, 0xEE, sizeof(p_chan->ed25519_identity));

  tt_int_op(circuit_extend_lspec_valid_helper(ec, circ), OP_EQ, -1);
  expect_log_msg("Client asked me to extend back to the previous hop "
                 "(by Ed25519 ID).\n");
  mock_clean_saved_logs();

  memset(&ec->ed_pubkey, 0, sizeof(ec->ed_pubkey));
  memset(&p_chan->ed25519_identity, 0, sizeof(p_chan->ed25519_identity));

  /* Succeed on different, non-zero identities */
  memset(&ec->ed_pubkey, 0xDD, sizeof(ec->ed_pubkey));
  memset(&p_chan->ed25519_identity, 0xEE, sizeof(p_chan->ed25519_identity));

  tt_int_op(circuit_extend_lspec_valid_helper(ec, circ), OP_EQ, 0);
  mock_clean_saved_logs();

  memset(&ec->ed_pubkey, 0, sizeof(ec->ed_pubkey));
  memset(&p_chan->ed25519_identity, 0, sizeof(p_chan->ed25519_identity));

  /* Succeed if the client knows the identity, but we don't */
  memset(&ec->ed_pubkey, 0xDD, sizeof(ec->ed_pubkey));
  memset(&p_chan->ed25519_identity, 0x00, sizeof(p_chan->ed25519_identity));

  tt_int_op(circuit_extend_lspec_valid_helper(ec, circ), OP_EQ, 0);
  mock_clean_saved_logs();

  memset(&ec->ed_pubkey, 0, sizeof(ec->ed_pubkey));
  memset(&p_chan->ed25519_identity, 0, sizeof(p_chan->ed25519_identity));

  /* Succeed if we know the identity, but the client doesn't */
  memset(&ec->ed_pubkey, 0x00, sizeof(ec->ed_pubkey));
  memset(&p_chan->ed25519_identity, 0xEE, sizeof(p_chan->ed25519_identity));

  tt_int_op(circuit_extend_lspec_valid_helper(ec, circ), OP_EQ, 0);
  mock_clean_saved_logs();

  memset(&ec->ed_pubkey, 0, sizeof(ec->ed_pubkey));
  memset(&p_chan->ed25519_identity, 0, sizeof(p_chan->ed25519_identity));

  /* Cleanup the node ids */
  memset(ec->node_id, 0, sizeof(ec->node_id));
  memset(p_chan->identity_digest, 0, sizeof(p_chan->identity_digest));

  /* Cleanup the p_chan and the addresses */
  tor_addr_make_null(&ec->orport_ipv4.addr, AF_UNSPEC);
  ec->orport_ipv4.port = 0;
  or_circ->p_chan = NULL;

 done:
  tor_end_capture_bugs_();
  teardown_capture_of_logs();

  UNMOCK(get_options);
  or_options_free(fake_options);
  mocked_options = NULL;

  tor_free(ec);
  tor_free(or_circ);
  tor_free(p_chan);
}

#define NODE_SET_IPV4(node, ipv4_addr_str, ipv4_port) { \
    tor_addr_parse(&node->ri->ipv4_addr, ipv4_addr_str); \
    node->ri->ipv4_orport = ipv4_port; \
  }

#define NODE_CLEAR_IPV4(node) { \
    tor_addr_make_unspec(&node->ri->ipv4_addr); \
    node->ri->ipv4_orport = 0; \
  }

#define NODE_SET_IPV6(node, ipv6_addr_str, ipv6_port) { \
    tor_addr_parse(&node->ri->ipv6_addr, ipv6_addr_str); \
    node->ri->ipv6_orport = ipv6_port; \
  }

/* Test the different cases in circuit_extend_add_ed25519_helper(). */
static void
test_circuit_extend_add_ip(void *arg)
{
  (void) arg;
  tor_addr_t ipv4_tmp;
  extend_cell_t *ec = tor_malloc_zero(sizeof(extend_cell_t));
  extend_cell_t *old_ec = tor_malloc_zero(sizeof(extend_cell_t));

  node_t *fake_node = tor_malloc_zero(sizeof(node_t));
  routerinfo_t *ri = tor_malloc_zero(sizeof(routerinfo_t));

  MOCK(node_get_by_id, mock_node_get_by_id);

  /* Set up the fake variables for the IPv4 test */
  fake_node->ri = ri;
  mocked_node = fake_node;
  memset(ec->node_id, 0xAA, sizeof(ec->node_id));
  memcpy(old_ec, ec, sizeof(extend_cell_t));
  NODE_SET_IPV4(fake_node, PUBLIC_IPV4, VALID_PORT);

  /* Do the IPv4 test */
  tt_int_op(circuit_extend_add_ipv4_helper(ec), OP_EQ, 0);
  tor_addr_copy(&ipv4_tmp, &fake_node->ri->ipv4_addr);
  /* The IPv4 should match */
  tt_int_op(tor_addr_compare(&ec->orport_ipv4.addr, &ipv4_tmp, CMP_SEMANTIC),
            OP_EQ, 0);
  tt_int_op(ec->orport_ipv4.port, OP_EQ, VALID_PORT);

  /* Set up the fake variables for the IPv6 test */
  memcpy(ec, old_ec, sizeof(extend_cell_t));
  NODE_CLEAR_IPV4(fake_node);
  NODE_SET_IPV6(fake_node, PUBLIC_IPV6, VALID_PORT);

  /* Do the IPv6 test */
  tt_int_op(circuit_extend_add_ipv6_helper(ec), OP_EQ, 0);
  /* The IPv6 should match */
  tt_int_op(tor_addr_compare(&ec->orport_ipv6.addr, &fake_node->ri->ipv6_addr,
            CMP_SEMANTIC), OP_EQ, 0);
  tt_int_op(ec->orport_ipv6.port, OP_EQ, VALID_PORT);

  /* Cleanup */
  mocked_node = NULL;

 done:
  UNMOCK(node_get_by_id);

  tor_free(ec);
  tor_free(old_ec);

  tor_free(ri);
  tor_free(fake_node);
}

static bool can_extend_over_ipv6_result = false;
static int mock_router_can_extend_over_ipv6_calls = 0;
static bool
mock_router_can_extend_over_ipv6(const or_options_t *options)
{
  (void)options;
  mock_router_can_extend_over_ipv6_calls++;
  return can_extend_over_ipv6_result;
}

/* Test the different cases in circuit_choose_ip_ap_for_extend(). */
static void
test_circuit_choose_ip_ap_for_extend(void *arg)
{
  (void)arg;
  tor_addr_port_t ipv4_ap;
  tor_addr_port_t ipv6_ap;

  /* Set up valid addresses */
  tor_addr_parse(&ipv4_ap.addr, PUBLIC_IPV4);
  ipv4_ap.port = VALID_PORT;
  tor_addr_parse(&ipv6_ap.addr, PUBLIC_IPV6);
  ipv6_ap.port = VALID_PORT;

  or_options_t *fake_options = options_new();
  MOCK(get_options, mock_get_options);
  mocked_options = fake_options;

  MOCK(router_can_extend_over_ipv6,
       mock_router_can_extend_over_ipv6);
  can_extend_over_ipv6_result = true;
  mock_router_can_extend_over_ipv6_calls = 0;

  /* No valid addresses */
  can_extend_over_ipv6_result = true;
  mock_router_can_extend_over_ipv6_calls = 0;
  tt_ptr_op(circuit_choose_ip_ap_for_extend(NULL, NULL), OP_EQ, NULL);
  tt_int_op(mock_router_can_extend_over_ipv6_calls, OP_EQ, 1);

  can_extend_over_ipv6_result = false;
  mock_router_can_extend_over_ipv6_calls = 0;
  tt_ptr_op(circuit_choose_ip_ap_for_extend(NULL, NULL), OP_EQ, NULL);
  tt_int_op(mock_router_can_extend_over_ipv6_calls, OP_EQ, 1);

  /* One valid address: IPv4 */
  can_extend_over_ipv6_result = true;
  mock_router_can_extend_over_ipv6_calls = 0;
  tt_ptr_op(circuit_choose_ip_ap_for_extend(&ipv4_ap, NULL), OP_EQ, &ipv4_ap);
  tt_int_op(mock_router_can_extend_over_ipv6_calls, OP_EQ, 1);

  can_extend_over_ipv6_result = false;
  mock_router_can_extend_over_ipv6_calls = 0;
  tt_ptr_op(circuit_choose_ip_ap_for_extend(&ipv4_ap, NULL), OP_EQ, &ipv4_ap);
  tt_int_op(mock_router_can_extend_over_ipv6_calls, OP_EQ, 1);

  /* One valid address: IPv6 */
  can_extend_over_ipv6_result = true;
  mock_router_can_extend_over_ipv6_calls = 0;
  tt_ptr_op(circuit_choose_ip_ap_for_extend(NULL, &ipv6_ap), OP_EQ, &ipv6_ap);
  tt_int_op(mock_router_can_extend_over_ipv6_calls, OP_EQ, 1);

  can_extend_over_ipv6_result = false;
  mock_router_can_extend_over_ipv6_calls = 0;
  tt_ptr_op(circuit_choose_ip_ap_for_extend(NULL, &ipv6_ap), OP_EQ, NULL);
  tt_int_op(mock_router_can_extend_over_ipv6_calls, OP_EQ, 1);

  /* Two valid addresses */
  const tor_addr_port_t *chosen_addr = NULL;

  can_extend_over_ipv6_result = true;
  mock_router_can_extend_over_ipv6_calls = 0;
  chosen_addr = circuit_choose_ip_ap_for_extend(&ipv4_ap, &ipv6_ap);
  tt_assert(chosen_addr == &ipv4_ap || chosen_addr == &ipv6_ap);
  tt_int_op(mock_router_can_extend_over_ipv6_calls, OP_EQ, 1);

  can_extend_over_ipv6_result = false;
  mock_router_can_extend_over_ipv6_calls = 0;
  tt_ptr_op(circuit_choose_ip_ap_for_extend(&ipv4_ap, &ipv6_ap),
            OP_EQ, &ipv4_ap);
  tt_int_op(mock_router_can_extend_over_ipv6_calls, OP_EQ, 1);

 done:
  UNMOCK(get_options);
  or_options_free(fake_options);
  mocked_options = NULL;

  UNMOCK(router_can_extend_over_ipv6);

  tor_free(fake_options);
}

static int mock_circuit_close_calls = 0;
static void
mock_circuit_mark_for_close_(circuit_t *circ, int reason,
                             int line, const char *cfile)
{
  (void)circ;
  (void)reason;
  (void)line;
  (void)cfile;
  mock_circuit_close_calls++;
}

static int mock_channel_connect_calls = 0;
static channel_t *mock_channel_connect_nchan = NULL;
static channel_t *
mock_channel_connect_for_circuit(const extend_info_t *ei)
{
  (void)ei;
  mock_channel_connect_calls++;
  return mock_channel_connect_nchan;
}

/* Test the different cases in circuit_open_connection_for_extend().
 * Chooses different IP addresses depending on the first character in arg:
 *  - 4: IPv4
 *  - 6: IPv6
 *  - d: IPv4 and IPv6 (dual-stack)
 */
static void
test_circuit_open_connection_for_extend(void *arg)
{
  const char ip_version = ((const char *)arg)[0];
  const bool use_ipv4 = (ip_version == '4' || ip_version == 'd');
  const bool use_ipv6 = (ip_version == '6' || ip_version == 'd');
  tor_assert(use_ipv4 || use_ipv6);

  extend_cell_t *ec = tor_malloc_zero(sizeof(extend_cell_t));
  circuit_t *circ = tor_malloc_zero(sizeof(circuit_t));
  channel_t *fake_n_chan = tor_malloc_zero(sizeof(channel_t));

  or_options_t *fake_options = options_new();
  MOCK(get_options, mock_get_options);
  mocked_options = fake_options;

  MOCK(circuit_mark_for_close_, mock_circuit_mark_for_close_);
  mock_circuit_close_calls = 0;
  MOCK(channel_connect_for_circuit, mock_channel_connect_for_circuit);
  mock_channel_connect_calls = 0;
  mock_channel_connect_nchan = NULL;

  MOCK(router_can_extend_over_ipv6,
       mock_router_can_extend_over_ipv6);
  can_extend_over_ipv6_result = true;

  setup_full_capture_of_logs(LOG_INFO);

#ifndef ALL_BUGS_ARE_FATAL
  /* Circuit must be non-NULL */
  mock_circuit_close_calls = 0;
  mock_channel_connect_calls = 0;
  tor_capture_bugs_(1);
  circuit_open_connection_for_extend(ec, NULL, 0);
  /* We can't close a NULL circuit */
  tt_int_op(mock_circuit_close_calls, OP_EQ, 0);
  tt_int_op(mock_channel_connect_calls, OP_EQ, 0);
  tt_int_op(smartlist_len(tor_get_captured_bug_log_()), OP_EQ, 1);
  tt_str_op(smartlist_get(tor_get_captured_bug_log_(), 0), OP_EQ,
            "!(ASSERT_PREDICT_UNLIKELY_(!circ))");
  tor_end_capture_bugs_();
  mock_clean_saved_logs();

  /* Extend cell must be non-NULL */
  mock_circuit_close_calls = 0;
  mock_channel_connect_calls = 0;
  tor_capture_bugs_(1);
  circuit_open_connection_for_extend(NULL, circ, 0);
  tt_int_op(mock_circuit_close_calls, OP_EQ, 1);
  tt_int_op(mock_channel_connect_calls, OP_EQ, 0);
  tt_int_op(smartlist_len(tor_get_captured_bug_log_()), OP_EQ, 1);
  tt_str_op(smartlist_get(tor_get_captured_bug_log_(), 0), OP_EQ,
            "!(ASSERT_PREDICT_UNLIKELY_(!ec))");
  tor_end_capture_bugs_();
  mock_clean_saved_logs();

  /* Extend cell and circuit must be non-NULL */
  mock_circuit_close_calls = 0;
  mock_channel_connect_calls = 0;
  tor_capture_bugs_(1);
  circuit_open_connection_for_extend(NULL, NULL, 0);
  /* We can't close a NULL circuit */
  tt_int_op(mock_circuit_close_calls, OP_EQ, 0);
  tt_int_op(mock_channel_connect_calls, OP_EQ, 0);
  /* Since we're using IF_BUG_ONCE(), we might not log any bugs */
  tt_int_op(smartlist_len(tor_get_captured_bug_log_()), OP_GE, 0);
  tt_int_op(smartlist_len(tor_get_captured_bug_log_()), OP_LE, 2);
  tor_end_capture_bugs_();
  mock_clean_saved_logs();

  /* Fail, because neither address is valid */
  mock_circuit_close_calls = 0;
  mock_channel_connect_calls = 0;
  tor_capture_bugs_(1);
  circuit_open_connection_for_extend(ec, circ, 0);
  /* Close the circuit, don't connect */
  tt_int_op(mock_circuit_close_calls, OP_EQ, 1);
  tt_int_op(mock_channel_connect_calls, OP_EQ, 0);
  /* Check state */
  tt_ptr_op(circ->n_hop, OP_EQ, NULL);
  tt_ptr_op(circ->n_chan_create_cell, OP_EQ, NULL);
  tt_int_op(circ->state, OP_EQ, 0);
  /* Cleanup */
  tor_end_capture_bugs_();
  mock_clean_saved_logs();
#endif /* !defined(ALL_BUGS_ARE_FATAL) */

  /* Set up valid addresses */
  if (use_ipv4) {
    tor_addr_parse(&ec->orport_ipv4.addr, PUBLIC_IPV4);
    ec->orport_ipv4.port = VALID_PORT;
  }
  if (use_ipv6) {
    tor_addr_parse(&ec->orport_ipv6.addr, PUBLIC_IPV6);
    ec->orport_ipv6.port = VALID_PORT;
  }

  /* Succeed, but don't try to open a connection */
  mock_circuit_close_calls = 0;
  mock_channel_connect_calls = 0;
  circuit_open_connection_for_extend(ec, circ, 0);
  /* If we haven't closed the circuit, that's success */
  tt_int_op(mock_circuit_close_calls, OP_EQ, 0);
  tt_int_op(mock_channel_connect_calls, OP_EQ, 0);
  /* Check state */
  tt_ptr_op(circ->n_hop, OP_NE, NULL);
  tt_ptr_op(circ->n_chan_create_cell, OP_NE, NULL);
  tt_int_op(circ->state, OP_EQ, CIRCUIT_STATE_CHAN_WAIT);
  /* Cleanup */
  mock_clean_saved_logs();
  tor_free(circ->n_hop);
  tor_free(circ->n_chan_create_cell);
  circ->state = 0;

  /* Try to open a connection, but fail with a NULL n_chan */
  mock_circuit_close_calls = 0;
  mock_channel_connect_calls = 0;
  circuit_open_connection_for_extend(ec, circ, 1);
  /* Try to connect, but fail, and close the circuit */
  tt_int_op(mock_circuit_close_calls, OP_EQ, 1);
  tt_int_op(mock_channel_connect_calls, OP_EQ, 1);
  expect_log_msg("Launching n_chan failed. Closing circuit.\n");
  /* Check state */
  tt_ptr_op(circ->n_hop, OP_NE, NULL);
  tt_ptr_op(circ->n_chan_create_cell, OP_NE, NULL);
  tt_int_op(circ->state, OP_EQ, CIRCUIT_STATE_CHAN_WAIT);
  /* Cleanup */
  mock_clean_saved_logs();
  tor_free(circ->n_hop);
  tor_free(circ->n_chan_create_cell);
  circ->state = 0;

  /* Try to open a connection, and succeed, because n_chan is not NULL */
  mock_channel_connect_nchan = fake_n_chan;
  mock_circuit_close_calls = 0;
  mock_channel_connect_calls = 0;
  circuit_open_connection_for_extend(ec, circ, 1);
  /* Connection attempt succeeded, leaving the circuit open */
  tt_int_op(mock_circuit_close_calls, OP_EQ, 0);
  tt_int_op(mock_channel_connect_calls, OP_EQ, 1);
  /* Check state */
  tt_ptr_op(circ->n_hop, OP_NE, NULL);
  tt_ptr_op(circ->n_chan_create_cell, OP_NE, NULL);
  tt_int_op(circ->state, OP_EQ, CIRCUIT_STATE_CHAN_WAIT);
  /* Cleanup */
  mock_clean_saved_logs();
  tor_free(circ->n_hop);
  tor_free(circ->n_chan_create_cell);
  circ->state = 0;
  mock_channel_connect_nchan = NULL;

 done:
  tor_end_capture_bugs_();
  teardown_capture_of_logs();

  UNMOCK(circuit_mark_for_close_);
  mock_circuit_close_calls = 0;
  UNMOCK(channel_connect_for_circuit);
  mock_channel_connect_calls = 0;

  UNMOCK(get_options);
  or_options_free(fake_options);
  mocked_options = NULL;

  UNMOCK(router_can_extend_over_ipv6);

  tor_free(ec);
  tor_free(circ->n_hop);
  tor_free(circ->n_chan_create_cell);
  tor_free(circ);
  tor_free(fake_n_chan);
}

/* Guaranteed to be initialised to zero. */
static extend_cell_t mock_extend_cell_parse_cell_out;
static int mock_extend_cell_parse_result = 0;
static int mock_extend_cell_parse_calls = 0;

static int
mock_extend_cell_parse(extend_cell_t *cell_out,
                       const uint8_t command,
                       const uint8_t *payload_in,
                       size_t payload_len)
{
  (void)command;
  (void)payload_in;
  (void)payload_len;

  mock_extend_cell_parse_calls++;
  memcpy(cell_out, &mock_extend_cell_parse_cell_out,
         sizeof(extend_cell_t));
  return mock_extend_cell_parse_result;
}

static int mock_channel_get_for_extend_calls = 0;
static int mock_channel_get_for_extend_launch_out = 0;
static channel_t *mock_channel_get_for_extend_nchan = NULL;
static channel_t *
mock_channel_get_for_extend(const char *rsa_id_digest,
                            const ed25519_public_key_t *ed_id,
                            const tor_addr_t *target_ipv4_addr,
                            const tor_addr_t *target_ipv6_addr,
                            bool for_origin_circ,
                            const char **msg_out,
                            int *launch_out)
{
  (void)rsa_id_digest;
  (void)ed_id;
  (void)target_ipv4_addr;
  (void)target_ipv6_addr;
  (void)for_origin_circ;

  /* channel_get_for_extend() requires non-NULL arguments */
  tt_ptr_op(msg_out, OP_NE, NULL);
  tt_ptr_op(launch_out, OP_NE, NULL);

  mock_channel_get_for_extend_calls++;
  *msg_out = NULL;
  *launch_out = mock_channel_get_for_extend_launch_out;
  return mock_channel_get_for_extend_nchan;

 done:
  return NULL;
}

static const char *
mock_channel_get_canonical_remote_descr(channel_t *chan)
{
  (void)chan;
  return "mock_channel_get_canonical_remote_descr()";
}

/* Should mock_circuit_deliver_create_cell() expect a direct connection? */
static bool mock_circuit_deliver_create_cell_expect_direct = false;
static int mock_circuit_deliver_create_cell_calls = 0;
static int mock_circuit_deliver_create_cell_result = 0;
static int
mock_circuit_deliver_create_cell(circuit_t *circ,
                                 const struct create_cell_t *create_cell,
                                 int relayed)
{
  (void)create_cell;

  /* circuit_deliver_create_cell() requires non-NULL arguments,
   * but we only check circ and circ->n_chan here. */
  tt_ptr_op(circ, OP_NE, NULL);
  /* We expect n_chan for relayed cells. But should we also expect it for
   * direct connections? */
  if (!mock_circuit_deliver_create_cell_expect_direct)
    tt_ptr_op(circ->n_chan, OP_NE, NULL);

  /* We should only ever get relayed cells from extends */
  tt_int_op(relayed, OP_EQ, !mock_circuit_deliver_create_cell_expect_direct);

  mock_circuit_deliver_create_cell_calls++;
  return mock_circuit_deliver_create_cell_result;

 done:
  return -1;
}

/* Test the different cases in circuit_extend(). */
static void
test_circuit_extend(void *arg)
{
  (void)arg;
  cell_t *cell = tor_malloc_zero(sizeof(cell_t));
  channel_t *p_chan = tor_malloc_zero(sizeof(channel_t));
  or_circuit_t *or_circ = tor_malloc_zero(sizeof(or_circuit_t));
  circuit_t *circ = TO_CIRCUIT(or_circ);
  channel_t *fake_n_chan = tor_malloc_zero(sizeof(channel_t));

  server = 0;
  MOCK(server_mode, mock_server_mode);

  /* Mock a debug function, but otherwise ignore it */
  MOCK(channel_describe_peer,
       mock_channel_get_canonical_remote_descr);

  setup_full_capture_of_logs(LOG_INFO);

#ifndef ALL_BUGS_ARE_FATAL
  /* Circuit must be non-NULL */
  tor_capture_bugs_(1);
  tt_int_op(circuit_extend(cell, NULL), OP_EQ, -1);
  tt_int_op(smartlist_len(tor_get_captured_bug_log_()), OP_EQ, 1);
  tt_str_op(smartlist_get(tor_get_captured_bug_log_(), 0), OP_EQ,
            "!(ASSERT_PREDICT_UNLIKELY_(!circ))");
  tor_end_capture_bugs_();
  mock_clean_saved_logs();

  /* Cell must be non-NULL */
  tor_capture_bugs_(1);
  tt_int_op(circuit_extend(NULL, circ), OP_EQ, -1);
  tt_int_op(smartlist_len(tor_get_captured_bug_log_()), OP_EQ, 1);
  tt_str_op(smartlist_get(tor_get_captured_bug_log_(), 0), OP_EQ,
            "!(ASSERT_PREDICT_UNLIKELY_(!cell))");
  tor_end_capture_bugs_();
  mock_clean_saved_logs();

  /* Extend cell and circuit must be non-NULL */
  tor_capture_bugs_(1);
  tt_int_op(circuit_extend(NULL, NULL), OP_EQ, -1);
  /* Since we're using IF_BUG_ONCE(), we might not log any bugs */
  tt_int_op(smartlist_len(tor_get_captured_bug_log_()), OP_GE, 0);
  tt_int_op(smartlist_len(tor_get_captured_bug_log_()), OP_LE, 2);
  tor_end_capture_bugs_();
  mock_clean_saved_logs();
#endif /* !defined(ALL_BUGS_ARE_FATAL) */

  /* Clients can't extend */
  server = 0;
  tt_int_op(circuit_extend(cell, circ), OP_EQ, -1);
  expect_log_msg("Got an extend cell, but running as a client. Closing.\n");
  mock_clean_saved_logs();

  /* But servers can. Unpack the cell, but fail parsing. */
  server = 1;
  tt_int_op(circuit_extend(cell, circ), OP_EQ, -1);
  expect_log_msg("Can't parse extend cell. Closing circuit.\n");
  mock_clean_saved_logs();

  /* Now mock parsing */
  MOCK(extend_cell_parse, mock_extend_cell_parse);

  /* And make parsing succeed, but fail on adding ed25519 */
  memset(&mock_extend_cell_parse_cell_out, 0,
             sizeof(mock_extend_cell_parse_cell_out));
  mock_extend_cell_parse_result = 0;
  mock_extend_cell_parse_calls = 0;

  tt_int_op(circuit_extend(cell, circ), OP_EQ, -1);
  tt_int_op(mock_extend_cell_parse_calls, OP_EQ, 1);
  expect_log_msg(
    "Client asked me to extend without specifying an id_digest.\n");
  mock_clean_saved_logs();
  mock_extend_cell_parse_calls = 0;

  /* Now add a node_id. Fail the lspec check because IPv4 and port are zero. */
  memset(&mock_extend_cell_parse_cell_out.node_id, 0xAA,
             sizeof(mock_extend_cell_parse_cell_out.node_id));

  tt_int_op(circuit_extend(cell, circ), OP_EQ, -1);
  tt_int_op(mock_extend_cell_parse_calls, OP_EQ, 1);
  expect_log_msg("Client asked me to extend to a zero destination port "
                 "or unspecified address '[scrubbed]'.\n");
  mock_clean_saved_logs();
  mock_extend_cell_parse_calls = 0;

  /* Now add a valid IPv4 and port. Fail the OR circuit magic check. */
  tor_addr_parse(&mock_extend_cell_parse_cell_out.orport_ipv4.addr,
                 PUBLIC_IPV4);
  mock_extend_cell_parse_cell_out.orport_ipv4.port = VALID_PORT;

#ifndef ALL_BUGS_ARE_FATAL
  tor_capture_bugs_(1);
  tt_int_op(circuit_extend(cell, circ), OP_EQ, -1);
  tt_int_op(mock_extend_cell_parse_calls, OP_EQ, 1);
  tt_int_op(smartlist_len(tor_get_captured_bug_log_()), OP_EQ, 1);
  tt_str_op(smartlist_get(tor_get_captured_bug_log_(), 0), OP_EQ,
            "!(ASSERT_PREDICT_UNLIKELY_(circ->magic != 0x98ABC04Fu))");
  tor_end_capture_bugs_();
  mock_clean_saved_logs();
  mock_extend_cell_parse_calls = 0;
#endif /* !defined(ALL_BUGS_ARE_FATAL) */

  /* Now add the right magic and a p_chan. */
  or_circ->base_.magic = OR_CIRCUIT_MAGIC;
  or_circ->p_chan = p_chan;

  /* Mock channel_get_for_extend(), so it doesn't crash. */
  mock_channel_get_for_extend_calls = 0;
  MOCK(channel_get_for_extend, mock_channel_get_for_extend);

  /* Test circuit not established, but don't launch another one */
  mock_channel_get_for_extend_launch_out = 0;
  mock_channel_get_for_extend_nchan = NULL;
  tt_int_op(circuit_extend(cell, circ), OP_EQ, 0);
  tt_int_op(mock_extend_cell_parse_calls, OP_EQ, 1);
  tt_int_op(mock_channel_get_for_extend_calls, OP_EQ, 1);

  /* cleanup */
  mock_clean_saved_logs();
  mock_extend_cell_parse_calls = 0;
  mock_channel_get_for_extend_calls = 0;
  /* circ and or_circ are the same object */
  tor_free(circ->n_hop);
  tor_free(circ->n_chan_create_cell);

  /* Mock channel_connect_for_circuit(), so we don't crash */
  mock_channel_connect_calls = 0;
  MOCK(channel_connect_for_circuit, mock_channel_connect_for_circuit);

  /* Test circuit not established, and successful launch of a channel */
  mock_channel_get_for_extend_launch_out = 1;
  mock_channel_get_for_extend_nchan = NULL;
  mock_channel_connect_nchan = fake_n_chan;
  tt_int_op(circuit_extend(cell, circ), OP_EQ, 0);
  tt_int_op(mock_extend_cell_parse_calls, OP_EQ, 1);
  tt_int_op(mock_channel_get_for_extend_calls, OP_EQ, 1);
  tt_int_op(mock_channel_connect_calls, OP_EQ, 1);

  /* cleanup */
  mock_clean_saved_logs();
  mock_extend_cell_parse_calls = 0;
  mock_channel_get_for_extend_calls = 0;
  mock_channel_connect_calls = 0;
  /* circ and or_circ are the same object */
  tor_free(circ->n_hop);
  tor_free(circ->n_chan_create_cell);

  /* Mock circuit_deliver_create_cell(), so it doesn't crash */
  mock_circuit_deliver_create_cell_calls = 0;
  mock_circuit_deliver_create_cell_expect_direct = false;
  MOCK(circuit_deliver_create_cell, mock_circuit_deliver_create_cell);

  /* Test circuit established, re-using channel, successful delivery */
  mock_channel_get_for_extend_launch_out = 0;
  mock_channel_get_for_extend_nchan = fake_n_chan;
  mock_channel_connect_nchan = NULL;
  mock_circuit_deliver_create_cell_result = 0;
  tt_int_op(circuit_extend(cell, circ), OP_EQ, 0);
  tt_int_op(mock_extend_cell_parse_calls, OP_EQ, 1);
  tt_int_op(mock_channel_get_for_extend_calls, OP_EQ, 1);
  tt_int_op(mock_channel_connect_calls, OP_EQ, 0);
  tt_int_op(mock_circuit_deliver_create_cell_calls, OP_EQ, 1);
  tt_ptr_op(circ->n_chan, OP_EQ, fake_n_chan);

  /* cleanup */
  circ->n_chan = NULL;
  mock_clean_saved_logs();
  mock_extend_cell_parse_calls = 0;
  mock_channel_get_for_extend_calls = 0;
  mock_channel_connect_calls = 0;
  mock_circuit_deliver_create_cell_calls = 0;
  /* circ and or_circ are the same object */
  tor_free(circ->n_hop);
  tor_free(circ->n_chan_create_cell);

  /* Test circuit established, re-using channel, failed delivery */
  mock_channel_get_for_extend_launch_out = 0;
  mock_channel_get_for_extend_nchan = fake_n_chan;
  mock_channel_connect_nchan = NULL;
  mock_circuit_deliver_create_cell_result = -1;
  tt_int_op(circuit_extend(cell, circ), OP_EQ, -1);
  tt_int_op(mock_extend_cell_parse_calls, OP_EQ, 1);
  tt_int_op(mock_channel_get_for_extend_calls, OP_EQ, 1);
  tt_int_op(mock_channel_connect_calls, OP_EQ, 0);
  tt_int_op(mock_circuit_deliver_create_cell_calls, OP_EQ, 1);
  tt_ptr_op(circ->n_chan, OP_EQ, fake_n_chan);

  /* cleanup */
  circ->n_chan = NULL;
  mock_clean_saved_logs();
  mock_extend_cell_parse_calls = 0;
  mock_channel_get_for_extend_calls = 0;
  mock_channel_connect_calls = 0;
  mock_circuit_deliver_create_cell_calls = 0;
  /* circ and or_circ are the same object */
  tor_free(circ->n_hop);
  tor_free(circ->n_chan_create_cell);

 done:
  tor_end_capture_bugs_();
  teardown_capture_of_logs();

  UNMOCK(server_mode);
  server = 0;

  UNMOCK(channel_describe_peer);

  UNMOCK(extend_cell_parse);
  memset(&mock_extend_cell_parse_cell_out, 0,
         sizeof(mock_extend_cell_parse_cell_out));
  mock_extend_cell_parse_result = 0;
  mock_extend_cell_parse_calls = 0;

  UNMOCK(channel_get_for_extend);
  mock_channel_get_for_extend_calls = 0;
  mock_channel_get_for_extend_launch_out = 0;
  mock_channel_get_for_extend_nchan = NULL;

  UNMOCK(channel_connect_for_circuit);
  mock_channel_connect_calls = 0;
  mock_channel_connect_nchan = NULL;

  UNMOCK(circuit_deliver_create_cell);
  mock_circuit_deliver_create_cell_calls = 0;
  mock_circuit_deliver_create_cell_result = 0;

  tor_free(cell);
  /* circ and or_circ are the same object */
  tor_free(circ->n_hop);
  tor_free(circ->n_chan_create_cell);
  tor_free(or_circ);
  tor_free(p_chan);
  tor_free(fake_n_chan);
}

/* Test the different cases in onionskin_answer(). */
static void
test_onionskin_answer(void *arg)
{
  (void)arg;
  created_cell_t *created_cell = tor_malloc_zero(sizeof(created_cell_t));
  or_circuit_t *or_circ = tor_malloc_zero(sizeof(or_circuit_t));
  char keys[CPATH_KEY_MATERIAL_LEN] = {0};
  uint8_t rend_circ_nonce[DIGEST_LEN] = {0};

  setup_full_capture_of_logs(LOG_INFO);

#ifndef ALL_BUGS_ARE_FATAL
  /* Circuit must be non-NULL */
  tor_capture_bugs_(1);
  tt_int_op(onionskin_answer(NULL, created_cell,
                             keys, CPATH_KEY_MATERIAL_LEN,
                             rend_circ_nonce), OP_EQ, -1);
  tt_int_op(smartlist_len(tor_get_captured_bug_log_()), OP_EQ, 1);
  tt_str_op(smartlist_get(tor_get_captured_bug_log_(), 0), OP_EQ,
            "!(ASSERT_PREDICT_UNLIKELY_(!circ))");
  tor_end_capture_bugs_();
  mock_clean_saved_logs();

  /* Created cell must be non-NULL */
  tor_capture_bugs_(1);
  tt_int_op(onionskin_answer(or_circ, NULL,
                             keys, CPATH_KEY_MATERIAL_LEN,
                             rend_circ_nonce), OP_EQ, -1);
  tt_int_op(smartlist_len(tor_get_captured_bug_log_()), OP_EQ, 1);
  tt_str_op(smartlist_get(tor_get_captured_bug_log_(), 0), OP_EQ,
            "!(ASSERT_PREDICT_UNLIKELY_(!created_cell))");
  tor_end_capture_bugs_();
  mock_clean_saved_logs();

  /* Keys must be non-NULL */
  tor_capture_bugs_(1);
  tt_int_op(onionskin_answer(or_circ, created_cell,
                             NULL, CPATH_KEY_MATERIAL_LEN,
                             rend_circ_nonce), OP_EQ, -1);
  tt_int_op(smartlist_len(tor_get_captured_bug_log_()), OP_EQ, 1);
  tt_str_op(smartlist_get(tor_get_captured_bug_log_(), 0), OP_EQ,
            "!(ASSERT_PREDICT_UNLIKELY_(!keys))");
  tor_end_capture_bugs_();
  mock_clean_saved_logs();

  /* The rend circuit nonce must be non-NULL */
  tor_capture_bugs_(1);
  tt_int_op(onionskin_answer(or_circ, created_cell,
                             keys, CPATH_KEY_MATERIAL_LEN,
                             NULL), OP_EQ, -1);
  tt_int_op(smartlist_len(tor_get_captured_bug_log_()), OP_EQ, 1);
  tt_str_op(smartlist_get(tor_get_captured_bug_log_(), 0), OP_EQ,
            "!(ASSERT_PREDICT_UNLIKELY_(!rend_circ_nonce))");
  tor_end_capture_bugs_();
  mock_clean_saved_logs();
#endif /* !defined(ALL_BUGS_ARE_FATAL) */

  /* Also, the keys length must be CPATH_KEY_MATERIAL_LEN, but we can't catch
   * asserts in unit tests. */

  /* Fail when formatting the created cell */
  tt_int_op(onionskin_answer(or_circ, created_cell,
                             keys, CPATH_KEY_MATERIAL_LEN,
                             rend_circ_nonce), OP_EQ, -1);
  expect_log_msg("couldn't format created cell (type=0, len=0).\n");
  mock_clean_saved_logs();

  /* TODO: test the rest of onionskin_answer(), see #33860 */
  /* TODO: mock created_cell_format for the next test  */

 done:
  tor_end_capture_bugs_();
  teardown_capture_of_logs();

  tor_free(created_cell);
  tor_free(or_circ);
}

/* Test the different cases in origin_circuit_init(). */
static void
test_origin_circuit_init(void *arg)
{
  (void)arg;
  origin_circuit_t *origin_circ = NULL;

  /* Init with 0 purpose and 0 flags */
  origin_circ = origin_circuit_init(0, 0);
  tt_int_op(origin_circ->base_.purpose, OP_EQ, 0);
  tt_int_op(origin_circ->base_.state, OP_EQ, CIRCUIT_STATE_CHAN_WAIT);
  tt_ptr_op(origin_circ->build_state, OP_NE, NULL);
  tt_int_op(origin_circ->build_state->is_internal, OP_EQ, 0);
  tt_int_op(origin_circ->build_state->is_ipv6_selftest, OP_EQ, 0);
  tt_int_op(origin_circ->build_state->need_capacity, OP_EQ, 0);
  tt_int_op(origin_circ->build_state->need_uptime, OP_EQ, 0);
  tt_int_op(origin_circ->build_state->onehop_tunnel, OP_EQ, 0);
  /* The circuits are automatically freed by the circuitlist. */

  /* Init with a purpose */
  origin_circ = origin_circuit_init(CIRCUIT_PURPOSE_C_GENERAL, 0);
  tt_int_op(origin_circ->base_.purpose, OP_EQ, CIRCUIT_PURPOSE_C_GENERAL);

  /* Init with each flag */
  origin_circ = origin_circuit_init(0, CIRCLAUNCH_IS_INTERNAL);
  tt_ptr_op(origin_circ->build_state, OP_NE, NULL);
  tt_int_op(origin_circ->build_state->is_internal, OP_EQ, 1);
  tt_int_op(origin_circ->build_state->is_ipv6_selftest, OP_EQ, 0);
  tt_int_op(origin_circ->build_state->need_capacity, OP_EQ, 0);
  tt_int_op(origin_circ->build_state->need_uptime, OP_EQ, 0);
  tt_int_op(origin_circ->build_state->onehop_tunnel, OP_EQ, 0);

  origin_circ = origin_circuit_init(0, CIRCLAUNCH_IS_IPV6_SELFTEST);
  tt_ptr_op(origin_circ->build_state, OP_NE, NULL);
  tt_int_op(origin_circ->build_state->is_internal, OP_EQ, 0);
  tt_int_op(origin_circ->build_state->is_ipv6_selftest, OP_EQ, 1);
  tt_int_op(origin_circ->build_state->need_capacity, OP_EQ, 0);
  tt_int_op(origin_circ->build_state->need_uptime, OP_EQ, 0);
  tt_int_op(origin_circ->build_state->onehop_tunnel, OP_EQ, 0);

  origin_circ = origin_circuit_init(0, CIRCLAUNCH_NEED_CAPACITY);
  tt_ptr_op(origin_circ->build_state, OP_NE, NULL);
  tt_int_op(origin_circ->build_state->is_internal, OP_EQ, 0);
  tt_int_op(origin_circ->build_state->is_ipv6_selftest, OP_EQ, 0);
  tt_int_op(origin_circ->build_state->need_capacity, OP_EQ, 1);
  tt_int_op(origin_circ->build_state->need_uptime, OP_EQ, 0);
  tt_int_op(origin_circ->build_state->onehop_tunnel, OP_EQ, 0);

  origin_circ = origin_circuit_init(0, CIRCLAUNCH_NEED_UPTIME);
  tt_ptr_op(origin_circ->build_state, OP_NE, NULL);
  tt_int_op(origin_circ->build_state->is_internal, OP_EQ, 0);
  tt_int_op(origin_circ->build_state->is_ipv6_selftest, OP_EQ, 0);
  tt_int_op(origin_circ->build_state->need_capacity, OP_EQ, 0);
  tt_int_op(origin_circ->build_state->need_uptime, OP_EQ, 1);
  tt_int_op(origin_circ->build_state->onehop_tunnel, OP_EQ, 0);

  origin_circ = origin_circuit_init(0, CIRCLAUNCH_ONEHOP_TUNNEL);
  tt_ptr_op(origin_circ->build_state, OP_NE, NULL);
  tt_int_op(origin_circ->build_state->is_internal, OP_EQ, 0);
  tt_int_op(origin_circ->build_state->is_ipv6_selftest, OP_EQ, 0);
  tt_int_op(origin_circ->build_state->need_capacity, OP_EQ, 0);
  tt_int_op(origin_circ->build_state->need_uptime, OP_EQ, 0);
  tt_int_op(origin_circ->build_state->onehop_tunnel, OP_EQ, 1);

 done:
  /* The circuits are automatically freed by the circuitlist. */
  ;
}

/* Test the different cases in circuit_send_next_onion_skin(). */
static void
test_circuit_send_next_onion_skin(void *arg)
{
  (void)arg;
  origin_circuit_t *origin_circ = NULL;
  struct timeval circ_start_time;
  memset(&circ_start_time, 0, sizeof(circ_start_time));

  extend_info_t fakehop;
  memset(&fakehop, 0, sizeof(fakehop));
  extend_info_t *single_fakehop = &fakehop;
  extend_info_t *multi_fakehop[DEFAULT_ROUTE_LEN] = {&fakehop,
                                                     &fakehop,
                                                     &fakehop};

  extend_info_t ipv6_hop;
  memset(&ipv6_hop, 0, sizeof(ipv6_hop));
  tor_addr_parse(&ipv6_hop.orports[0].addr, "1::2");
  extend_info_t *multi_ipv6_hop[DEFAULT_ROUTE_LEN] = {&ipv6_hop,
                                                      &ipv6_hop,
                                                      &ipv6_hop};

  extend_info_t ipv4_hop;
  memset(&ipv4_hop, 0, sizeof(ipv4_hop));
  tor_addr_from_ipv4h(&ipv4_hop.orports[0].addr, 0x20304050);
  extend_info_t *multi_ipv4_hop[DEFAULT_ROUTE_LEN] = {&ipv4_hop,
                                                      &ipv4_hop,
                                                      &ipv4_hop};

  mock_circuit_deliver_create_cell_expect_direct = false;
  MOCK(circuit_deliver_create_cell, mock_circuit_deliver_create_cell);
  server = 0;
  MOCK(server_mode, mock_server_mode);

  /* Try a direct connection, and succeed on a client */
  server = 0;
  origin_circ = new_test_origin_circuit(false,
                                        circ_start_time,
                                        1,
                                        &single_fakehop);
  tt_ptr_op(origin_circ, OP_NE, NULL);
  /* Skip some of the multi-hop checks */
  origin_circ->build_state->onehop_tunnel = 1;
  /* This is a direct connection */
  mock_circuit_deliver_create_cell_expect_direct = true;
  tt_int_op(circuit_send_next_onion_skin(origin_circ), OP_EQ, 0);
  /* The circuits are automatically freed by the circuitlist. */

  /* Try a direct connection, and succeed on a server */
  server = 1;
  origin_circ = new_test_origin_circuit(false,
                                        circ_start_time,
                                        1,
                                        &single_fakehop);
  tt_ptr_op(origin_circ, OP_NE, NULL);
  origin_circ->build_state->onehop_tunnel = 1;
  mock_circuit_deliver_create_cell_expect_direct = true;
  tt_int_op(circuit_send_next_onion_skin(origin_circ), OP_EQ, 0);

  /* Start capturing bugs */
  setup_full_capture_of_logs(LOG_WARN);
  tor_capture_bugs_(1);

  /* Try an extend, but fail the client valid address family check */
  server = 0;
  origin_circ = new_test_origin_circuit(true,
                                        circ_start_time,
                                        ARRAY_LENGTH(multi_fakehop),
                                        multi_fakehop);
  tt_ptr_op(origin_circ, OP_NE, NULL);
  /* Fix the state */
  origin_circ->base_.state = 0;
  /* This is an indirect connection */
  mock_circuit_deliver_create_cell_expect_direct = false;
  /* Fail because the address family is invalid */
  tt_int_op(circuit_send_next_onion_skin(origin_circ), OP_EQ,
            -END_CIRC_REASON_INTERNAL);
  expect_log_msg("No supported address family found in extend_info.\n");
  mock_clean_saved_logs();

  /* Try an extend, but fail the server valid address check */
  server = 1;
  origin_circ = new_test_origin_circuit(true,
                                        circ_start_time,
                                        ARRAY_LENGTH(multi_fakehop),
                                        multi_fakehop);
  tt_ptr_op(origin_circ, OP_NE, NULL);
  origin_circ->base_.state = 0;
  mock_circuit_deliver_create_cell_expect_direct = false;
  tt_int_op(circuit_send_next_onion_skin(origin_circ), OP_EQ,
            -END_CIRC_REASON_INTERNAL);
  expect_log_msg("No supported address family found in extend_info.\n");
  mock_clean_saved_logs();

  /* Try an extend, but fail in the client code, with an IPv6 address */
  server = 0;
  origin_circ = new_test_origin_circuit(true,
                                        circ_start_time,
                                        ARRAY_LENGTH(multi_ipv6_hop),
                                        multi_ipv6_hop);
  tt_ptr_op(origin_circ, OP_NE, NULL);
  origin_circ->base_.state = 0;
  mock_circuit_deliver_create_cell_expect_direct = false;
  tt_int_op(circuit_send_next_onion_skin(origin_circ), OP_EQ,
            -END_CIRC_REASON_INTERNAL);
  expect_log_msg("No supported address family found in extend_info.\n");
  mock_clean_saved_logs();

  /* Stop capturing bugs, but keep capturing logs */
  tor_end_capture_bugs_();

  /* Try an extend, pass the client IPv4 check, but fail later */
  server = 0;
  origin_circ = new_test_origin_circuit(true,
                                        circ_start_time,
                                        ARRAY_LENGTH(multi_ipv4_hop),
                                        multi_ipv4_hop);
  tt_ptr_op(origin_circ, OP_NE, NULL);
  origin_circ->base_.state = 0;
  mock_circuit_deliver_create_cell_expect_direct = false;
  /* Fail because the circuit data is invalid */
  tt_int_op(circuit_send_next_onion_skin(origin_circ), OP_EQ,
            -END_CIRC_REASON_INTERNAL);
  expect_log_msg("onion_skin_create failed.\n");
  mock_clean_saved_logs();

  /* Try an extend, pass the server IPv4 check, but fail later */
  server = 1;
  origin_circ = new_test_origin_circuit(true,
                                        circ_start_time,
                                        ARRAY_LENGTH(multi_ipv4_hop),
                                        multi_ipv4_hop);
  tt_ptr_op(origin_circ, OP_NE, NULL);
  origin_circ->base_.state = 0;
  mock_circuit_deliver_create_cell_expect_direct = false;
  tt_int_op(circuit_send_next_onion_skin(origin_circ), OP_EQ,
            -END_CIRC_REASON_INTERNAL);
  expect_log_msg("onion_skin_create failed.\n");
  mock_clean_saved_logs();

  /* Try an extend, pass the server IPv6 check, but fail later */
  server = 1;
  origin_circ = new_test_origin_circuit(true,
                                        circ_start_time,
                                        ARRAY_LENGTH(multi_ipv6_hop),
                                        multi_ipv6_hop);
  tt_ptr_op(origin_circ, OP_NE, NULL);
  origin_circ->base_.state = 0;
  mock_circuit_deliver_create_cell_expect_direct = false;
  tt_int_op(circuit_send_next_onion_skin(origin_circ), OP_EQ,
            -END_CIRC_REASON_INTERNAL);
  expect_log_msg("onion_skin_create failed.\n");
  mock_clean_saved_logs();

  /* Things we're not testing right now:
   * - the addresses in the extend cell inside
   *   circuit_send_intermediate_onion_skin() matches the address in the
   *   supplied extend_info.
   * - valid circuit data.
   * - actually extending the circuit to each hop. */

 done:
  tor_end_capture_bugs_();
  mock_clean_saved_logs();
  teardown_capture_of_logs();

  UNMOCK(circuit_deliver_create_cell);
  UNMOCK(server_mode);
  server = 0;

  /* The circuits are automatically freed by the circuitlist. */
}

/* Test the different cases in cpath_build_state_to_crn_flags(). */
static void
test_cpath_build_state_to_crn_flags(void *arg)
{
  (void)arg;

  cpath_build_state_t state;
  memset(&state, 0, sizeof(state));

  tt_int_op(cpath_build_state_to_crn_flags(&state), OP_EQ,
            0);

  memset(&state, 0, sizeof(state));
  state.need_uptime = 1;
  tt_int_op(cpath_build_state_to_crn_flags(&state), OP_EQ,
            CRN_NEED_UPTIME);

  memset(&state, 0, sizeof(state));
  state.need_capacity = 1;
  tt_int_op(cpath_build_state_to_crn_flags(&state), OP_EQ,
            CRN_NEED_CAPACITY);

  memset(&state, 0, sizeof(state));
  state.need_capacity = 1;
  state.need_uptime = 1;
  tt_int_op(cpath_build_state_to_crn_flags(&state), OP_EQ,
            CRN_NEED_CAPACITY | CRN_NEED_UPTIME);

  /* Check that no other flags are handled */
  memset(&state, 0xff, sizeof(state));
  tt_int_op(cpath_build_state_to_crn_flags(&state), OP_EQ,
            CRN_NEED_CAPACITY | CRN_NEED_UPTIME);

 done:
  ;
}

/* Test the different cases in cpath_build_state_to_crn_ipv6_extend_flag(). */
static void
test_cpath_build_state_to_crn_ipv6_extend_flag(void *arg)
{
  (void)arg;

  cpath_build_state_t state;

  memset(&state, 0, sizeof(state));
  state.desired_path_len = DEFAULT_ROUTE_LEN;
  tt_int_op(cpath_build_state_to_crn_ipv6_extend_flag(&state, 0), OP_EQ,
            0);

  /* Pass the state flag check, but not the length check */
  memset(&state, 0, sizeof(state));
  state.desired_path_len = DEFAULT_ROUTE_LEN;
  state.is_ipv6_selftest = 1;
  tt_int_op(cpath_build_state_to_crn_ipv6_extend_flag(&state, 0), OP_EQ,
            0);

  /* Pass the length check, but not the state flag check */
  memset(&state, 0, sizeof(state));
  state.desired_path_len = DEFAULT_ROUTE_LEN;
  tt_int_op(
      cpath_build_state_to_crn_ipv6_extend_flag(&state,
                                                DEFAULT_ROUTE_LEN - 2),
      OP_EQ, 0);

  /* Pass both checks */
  memset(&state, 0, sizeof(state));
  state.desired_path_len = DEFAULT_ROUTE_LEN;
  state.is_ipv6_selftest = 1;
  tt_int_op(
      cpath_build_state_to_crn_ipv6_extend_flag(&state,
                                                DEFAULT_ROUTE_LEN - 2),
      OP_EQ, CRN_INITIATE_IPV6_EXTEND);

  /* Check that no other flags are handled */
  memset(&state, 0xff, sizeof(state));
  state.desired_path_len = INT_MAX;
  tt_int_op(cpath_build_state_to_crn_ipv6_extend_flag(&state, INT_MAX), OP_EQ,
            0);

#ifndef ALL_BUGS_ARE_FATAL
  /* Start capturing bugs */
  setup_full_capture_of_logs(LOG_INFO);
  tor_capture_bugs_(1);

  /* Now test the single hop circuit case */
#define SINGLE_HOP_ROUTE_LEN 1
  memset(&state, 0, sizeof(state));
  state.desired_path_len = SINGLE_HOP_ROUTE_LEN;
  state.is_ipv6_selftest = 1;
  tt_int_op(
      cpath_build_state_to_crn_ipv6_extend_flag(&state,
                                                SINGLE_HOP_ROUTE_LEN - 2),
      OP_EQ, 0);
  tt_int_op(smartlist_len(tor_get_captured_bug_log_()), OP_EQ, 1);
  tt_str_op(smartlist_get(tor_get_captured_bug_log_(), 0), OP_EQ,
            "!(ASSERT_PREDICT_UNLIKELY_(state->desired_path_len < 2))");
  mock_clean_saved_logs();
#endif /* !defined(ALL_BUGS_ARE_FATAL) */

 done:
  tor_end_capture_bugs_();
  mock_clean_saved_logs();
  teardown_capture_of_logs();
}

#define TEST(name, flags, setup, cleanup) \
  { #name, test_ ## name, flags, setup, cleanup }

#define TEST_NEW_ROUTE_LEN(name, flags) \
  { #name, test_new_route_len_ ## name, flags, NULL, NULL }

#define TEST_CIRCUIT(name, flags) \
  { #name, test_circuit_ ## name, flags, NULL, NULL }

#define TEST_CPATH(name, flags) \
  { #name, test_cpath_ ## name, flags, NULL, NULL }

#ifndef COCCI
#define TEST_CIRCUIT_PASSTHROUGH(name, flags, arg) \
  { #name "/" arg, test_circuit_ ## name, flags, \
    &passthrough_setup, (void *)(arg) }
#endif

struct testcase_t circuitbuild_tests[] = {
  TEST_NEW_ROUTE_LEN(noexit, 0),
  TEST_NEW_ROUTE_LEN(safe_exit, 0),
  TEST_NEW_ROUTE_LEN(unsafe_exit, 0),
  TEST_NEW_ROUTE_LEN(unhandled_exit, 0),

  TEST(upgrade_from_guard_wait, TT_FORK, &helper_pubsub_setup, NULL),

  TEST_CIRCUIT(extend_state_valid, TT_FORK),
  TEST_CIRCUIT(extend_add_ed25519, TT_FORK),
  TEST_CIRCUIT(extend_lspec_valid, TT_FORK),
  TEST_CIRCUIT(extend_add_ip, TT_FORK),
  TEST_CIRCUIT(choose_ip_ap_for_extend, 0),

  TEST_CIRCUIT_PASSTHROUGH(open_connection_for_extend, TT_FORK, "4"),
  TEST_CIRCUIT_PASSTHROUGH(open_connection_for_extend, TT_FORK, "6"),
  TEST_CIRCUIT_PASSTHROUGH(open_connection_for_extend, TT_FORK, "dual-stack"),

  TEST_CIRCUIT(extend, TT_FORK),

  TEST(onionskin_answer, TT_FORK, NULL, NULL),

  TEST(origin_circuit_init, TT_FORK, NULL, NULL),
  TEST_CIRCUIT(send_next_onion_skin, TT_FORK),
  TEST_CPATH(build_state_to_crn_flags, 0),
  TEST_CPATH(build_state_to_crn_ipv6_extend_flag, TT_FORK),

  END_OF_TESTCASES
};
