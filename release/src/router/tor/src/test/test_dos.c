/* Copyright (c) 2018-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

#define DOS_PRIVATE
#define CHANNEL_OBJECT_PRIVATE
#define CIRCUITLIST_PRIVATE

#include "core/or/or.h"
#include "core/or/dos.h"
#include "core/or/circuitlist.h"
#include "lib/crypt_ops/crypto_rand.h"
#include "feature/stats/geoip_stats.h"
#include "core/or/channel.h"
#include "feature/nodelist/microdesc.h"
#include "feature/nodelist/networkstatus.h"
#include "feature/nodelist/nodelist.h"
#include "feature/nodelist/routerlist.h"

#include "feature/nodelist/networkstatus_st.h"
#include "core/or/or_connection_st.h"
#include "feature/nodelist/routerstatus_st.h"

#include "test/test.h"
#include "test/log_test_helpers.h"

static networkstatus_t *dummy_ns = NULL;
static networkstatus_t *
mock_networkstatus_get_latest_consensus(void)
{
  return dummy_ns;
}

static networkstatus_t *
mock_networkstatus_get_latest_consensus_by_flavor(consensus_flavor_t f)
{
  tor_assert(f == FLAV_MICRODESC);
  return dummy_ns;
}

/* Number of address a single node_t can have. Default to the production
 * value. This is to control the size of the bloom filter. */
static int addr_per_node = 2;
static int
mock_get_estimated_address_per_node(void)
{
  return addr_per_node;
}

static unsigned int
mock_enable_dos_protection(const networkstatus_t *ns)
{
  (void) ns;
  return 1;
}

/** Test that the connection tracker of the DoS subsystem will block clients
 *  who try to establish too many connections */
static void
test_dos_conn_creation(void *arg)
{
  (void) arg;

  MOCK(get_param_cc_enabled, mock_enable_dos_protection);
  MOCK(get_param_conn_enabled, mock_enable_dos_protection);

  /* Initialize test data */
  or_connection_t or_conn;
  time_t now = 1281533250; /* 2010-08-11 13:27:30 UTC */
  tt_int_op(AF_INET,OP_EQ, tor_addr_parse(&TO_CONN(&or_conn)->addr,
                                          "18.0.0.1"));
  tor_addr_t *addr = &TO_CONN(&or_conn)->addr;

  /* Get DoS subsystem limits */
  dos_init();
  uint32_t max_concurrent_conns = get_param_conn_max_concurrent_count(NULL);

  /* Introduce new client */
  geoip_note_client_seen(GEOIP_CLIENT_CONNECT, addr, NULL, now);
  { /* Register many conns from this client but not enough to get it blocked */
    unsigned int i;
    for (i = 0; i < max_concurrent_conns; i++) {
      dos_new_client_conn(&or_conn, NULL);
    }
  }

  /* Check that new conns are still permitted */
  tt_int_op(DOS_CONN_DEFENSE_NONE, OP_EQ,
            dos_conn_addr_get_defense_type(addr));

  /* Register another conn and check that new conns are not allowed anymore */
  dos_new_client_conn(&or_conn, NULL);
  tt_int_op(DOS_CONN_DEFENSE_CLOSE, OP_EQ,
            dos_conn_addr_get_defense_type(addr));

  /* Close a client conn and see that a new conn will be permitted again */
  dos_close_client_conn(&or_conn);
  tt_int_op(DOS_CONN_DEFENSE_NONE, OP_EQ,
            dos_conn_addr_get_defense_type(addr));

  /* Register another conn and see that defense measures get reactivated */
  dos_new_client_conn(&or_conn, NULL);
  tt_int_op(DOS_CONN_DEFENSE_CLOSE, OP_EQ,
            dos_conn_addr_get_defense_type(addr));

 done:
  dos_free_all();
}

/** Helper mock: Place a fake IP addr for this channel in <b>addr_out</b> */
static int
mock_channel_get_addr_if_possible(const channel_t *chan, tor_addr_t *addr_out)
{
  (void)chan;
  tt_int_op(AF_INET,OP_EQ, tor_addr_parse(addr_out, "18.0.0.1"));
  return 1;

 done:
  return 0;
}

/** Test that the circuit tracker of the DoS subsystem will block clients who
 *  try to establish too many circuits. */
static void
test_dos_circuit_creation(void *arg)
{
  (void) arg;
  unsigned int i;

  MOCK(get_param_cc_enabled, mock_enable_dos_protection);
  MOCK(get_param_conn_enabled, mock_enable_dos_protection);
  MOCK(channel_get_addr_if_possible,
       mock_channel_get_addr_if_possible);

  /* Initialize channels/conns/circs that will be used */
  channel_t *chan = tor_malloc_zero(sizeof(channel_t));
  channel_init(chan);
  chan->is_client = 1;

  /* Initialize test data */
  or_connection_t or_conn;
  time_t now = 1281533250; /* 2010-08-11 13:27:30 UTC */
  tt_int_op(AF_INET,OP_EQ, tor_addr_parse(&TO_CONN(&or_conn)->addr,
                                          "18.0.0.1"));
  tor_addr_t *addr = &TO_CONN(&or_conn)->addr;

  /* Get DoS subsystem limits */
  dos_init();
  uint32_t max_circuit_count = get_param_cc_circuit_burst(NULL);
  uint32_t min_conc_conns_for_cc =
    get_param_cc_min_concurrent_connection(NULL);

  /* Introduce new client and establish enough connections to activate the
   * circuit counting subsystem */
  geoip_note_client_seen(GEOIP_CLIENT_CONNECT, addr, NULL, now);
  for (i = 0; i < min_conc_conns_for_cc ; i++) {
    dos_new_client_conn(&or_conn, NULL);
  }

  /* Register new circuits for this client and conn, but not enough to get
   * detected as dos */
  for (i=0; i < max_circuit_count-1; i++) {
    dos_cc_new_create_cell(chan);
  }
  /* see that we didn't get detected for dosing */
  tt_int_op(DOS_CC_DEFENSE_NONE, OP_EQ, dos_cc_get_defense_type(chan));

  /* Register another CREATE cell that will push us over the limit. Check that
   * the cell gets refused. */
  dos_cc_new_create_cell(chan);
  tt_int_op(DOS_CC_DEFENSE_REFUSE_CELL, OP_EQ, dos_cc_get_defense_type(chan));

  /* TODO: Wait a few seconds before sending the cell, and check that the
     buckets got refilled properly. */
  /* TODO: Actually send a Tor cell (instead of calling the DoS function) and
   * check that it will get refused */

 done:
  tor_free(chan);
  dos_free_all();
}

/** Test that the DoS subsystem properly refills the circuit token buckets. */
static void
test_dos_bucket_refill(void *arg)
{
  (void) arg;
  int i;
  /* For this test, this variable is set to the current circ count of the token
   * bucket. */
  uint32_t current_circ_count;

  MOCK(get_param_cc_enabled, mock_enable_dos_protection);
  MOCK(get_param_conn_enabled, mock_enable_dos_protection);
  MOCK(channel_get_addr_if_possible,
       mock_channel_get_addr_if_possible);

  time_t now = 1281533250; /* 2010-08-11 13:27:30 UTC */
  update_approx_time(now);

  /* Initialize channels/conns/circs that will be used */
  channel_t *chan = tor_malloc_zero(sizeof(channel_t));
  channel_init(chan);
  chan->is_client = 1;
  or_connection_t or_conn;
  tt_int_op(AF_INET,OP_EQ, tor_addr_parse(&TO_CONN(&or_conn)->addr,
                                          "18.0.0.1"));
  tor_addr_t *addr = &TO_CONN(&or_conn)->addr;

  /* Initialize DoS subsystem and get relevant limits */
  dos_init();
  uint32_t max_circuit_count = get_param_cc_circuit_burst(NULL);
  uint64_t circ_rate = get_circuit_rate_per_second();
  /* Check that the circuit rate is a positive number and smaller than the max
   * circuit count */
  tt_u64_op(circ_rate, OP_GT, 1);
  tt_u64_op(circ_rate, OP_LT, max_circuit_count);

  /* Register this client */
  geoip_note_client_seen(GEOIP_CLIENT_CONNECT, addr, NULL, now);
  dos_new_client_conn(&or_conn, NULL);

  /* Fetch this client from the geoip cache and get its DoS structs */
  clientmap_entry_t *entry = geoip_lookup_client(addr, NULL,
                                                 GEOIP_CLIENT_CONNECT);
  tt_assert(entry);
  dos_client_stats_t* dos_stats = &entry->dos_stats;
  /* Check that the circuit bucket is still uninitialized */
  tt_uint_op(dos_stats->cc_stats.circuit_bucket, OP_EQ, 0);

  /* Send a create cell: then check that the circ token bucket got initialized
   * and one circ was subtracted. */
  dos_cc_new_create_cell(chan);
  current_circ_count = max_circuit_count - 1;
  tt_uint_op(dos_stats->cc_stats.circuit_bucket, OP_EQ, current_circ_count);

  /* Now send 29 more CREATEs and ensure that the bucket is missing 30
   * tokens */
  for (i=0; i < 29; i++) {
   dos_cc_new_create_cell(chan);
   current_circ_count--;
  }
  tt_uint_op(dos_stats->cc_stats.circuit_bucket, OP_EQ, current_circ_count);

  /* OK! Progress time forward one sec, refill the bucket and check that the
   * refill happened correctly. */
  now += 1;
  update_approx_time(now);
  cc_stats_refill_bucket(&dos_stats->cc_stats, addr);
  /* check refill */
  current_circ_count += circ_rate;
  tt_uint_op(dos_stats->cc_stats.circuit_bucket, OP_EQ, current_circ_count);

  /* Now send as many CREATE cells as needed to deplete our token bucket
   * completely */
  for (; current_circ_count != 0; current_circ_count--) {
   dos_cc_new_create_cell(chan);
  }
  tt_uint_op(current_circ_count, OP_EQ, 0);
  tt_uint_op(dos_stats->cc_stats.circuit_bucket, OP_EQ, current_circ_count);

  /* Now progress time a week forward, and check that the token bucket does not
   * have more than max_circs allowance, even tho we let it simmer for so
   * long. */
  now += 604800; /* a week */
  update_approx_time(now);
  cc_stats_refill_bucket(&dos_stats->cc_stats, addr);
  current_circ_count += max_circuit_count;
  tt_uint_op(dos_stats->cc_stats.circuit_bucket, OP_EQ, current_circ_count);

  /* Now send as many CREATE cells as needed to deplete our token bucket
   * completely */
  for (; current_circ_count != 0; current_circ_count--) {
    dos_cc_new_create_cell(chan);
  }
  tt_uint_op(current_circ_count, OP_EQ, 0);
  tt_uint_op(dos_stats->cc_stats.circuit_bucket, OP_EQ, current_circ_count);

  /* Now use a very large time, and check that the token bucket does not have
   * more than max_circs allowance, even tho we let it simmer for so long. */
  now = INT32_MAX; /* 2038? */
  update_approx_time(now);
  cc_stats_refill_bucket(&dos_stats->cc_stats, addr);
  current_circ_count += max_circuit_count;
  tt_uint_op(dos_stats->cc_stats.circuit_bucket, OP_EQ, current_circ_count);

  /* Now send as many CREATE cells as needed to deplete our token bucket
   * completely */
  for (; current_circ_count != 0; current_circ_count--) {
    dos_cc_new_create_cell(chan);
  }
  tt_uint_op(current_circ_count, OP_EQ, 0);
  tt_uint_op(dos_stats->cc_stats.circuit_bucket, OP_EQ, current_circ_count);

  /* Now use a very small time, and check that the token bucket has exactly
   * the max_circs allowance, because backward clock jumps are rare. */
  now = INT32_MIN; /* 19?? */
  update_approx_time(now);
  cc_stats_refill_bucket(&dos_stats->cc_stats, addr);
  current_circ_count += max_circuit_count;
  tt_uint_op(dos_stats->cc_stats.circuit_bucket, OP_EQ, current_circ_count);

  /* Now send as many CREATE cells as needed to deplete our token bucket
   * completely */
  for (; current_circ_count != 0; current_circ_count--) {
    dos_cc_new_create_cell(chan);
  }
  tt_uint_op(current_circ_count, OP_EQ, 0);
  tt_uint_op(dos_stats->cc_stats.circuit_bucket, OP_EQ, current_circ_count);

  /* Progress time forward one sec again, refill the bucket and check that the
   * refill happened correctly. */
  now += 1;
  update_approx_time(now);
  cc_stats_refill_bucket(&dos_stats->cc_stats, addr);
  /* check refill */
  current_circ_count += circ_rate;
  tt_uint_op(dos_stats->cc_stats.circuit_bucket, OP_EQ, current_circ_count);

  /* Now send as many CREATE cells as needed to deplete our token bucket
   * completely */
  for (; current_circ_count != 0; current_circ_count--) {
    dos_cc_new_create_cell(chan);
  }
  tt_uint_op(current_circ_count, OP_EQ, 0);
  tt_uint_op(dos_stats->cc_stats.circuit_bucket, OP_EQ, current_circ_count);

  /* Now use a very large time (again), and check that the token bucket does
   * not have more than max_circs allowance, even tho we let it simmer for so
   * long. */
  now = INT32_MAX; /* 2038? */
  update_approx_time(now);
  cc_stats_refill_bucket(&dos_stats->cc_stats, addr);
  current_circ_count += max_circuit_count;
  tt_uint_op(dos_stats->cc_stats.circuit_bucket, OP_EQ, current_circ_count);

  /* Now send as many CREATE cells as needed to deplete our token bucket
   * completely */
  for (; current_circ_count != 0; current_circ_count--) {
    dos_cc_new_create_cell(chan);
  }
  tt_uint_op(current_circ_count, OP_EQ, 0);
  tt_uint_op(dos_stats->cc_stats.circuit_bucket, OP_EQ, current_circ_count);

  /* This code resets the time to zero with 32-bit time_t, which triggers the
   * code that initialises the bucket. */
#if SIZEOF_TIME_T == 8
  /* Now use a very very small time, and check that the token bucket has
   * exactly the max_circs allowance, because backward clock jumps are rare.
   */
  now = (time_t)INT64_MIN; /* ???? */
  update_approx_time(now);
  cc_stats_refill_bucket(&dos_stats->cc_stats, addr);
  current_circ_count += max_circuit_count;
  tt_uint_op(dos_stats->cc_stats.circuit_bucket, OP_EQ, current_circ_count);

  /* Now send as many CREATE cells as needed to deplete our token bucket
   * completely */
  for (; current_circ_count != 0; current_circ_count--) {
    dos_cc_new_create_cell(chan);
  }
  tt_uint_op(current_circ_count, OP_EQ, 0);
  tt_uint_op(dos_stats->cc_stats.circuit_bucket, OP_EQ, current_circ_count);

  /* Progress time forward one sec again, refill the bucket and check that the
   * refill happened correctly. */
  now += 1;
  update_approx_time(now);
  cc_stats_refill_bucket(&dos_stats->cc_stats, addr);
  /* check refill */
  current_circ_count += circ_rate;
  tt_uint_op(dos_stats->cc_stats.circuit_bucket, OP_EQ, current_circ_count);

  /* Now send as many CREATE cells as needed to deplete our token bucket
   * completely */
  for (; current_circ_count != 0; current_circ_count--) {
    dos_cc_new_create_cell(chan);
  }
  tt_uint_op(current_circ_count, OP_EQ, 0);
  tt_uint_op(dos_stats->cc_stats.circuit_bucket, OP_EQ, current_circ_count);

  /* Now use a very very small time, and check that the token bucket has
   * exactly the max_circs allowance, because backward clock jumps are rare.
   */
  now = (time_t)INT64_MIN; /* ???? */
  update_approx_time(now);
  cc_stats_refill_bucket(&dos_stats->cc_stats, addr);
  current_circ_count += max_circuit_count;
  tt_uint_op(dos_stats->cc_stats.circuit_bucket, OP_EQ, current_circ_count);

  /* Now send as many CREATE cells as needed to deplete our token bucket
   * completely */
  for (; current_circ_count != 0; current_circ_count--) {
    dos_cc_new_create_cell(chan);
  }
  tt_uint_op(current_circ_count, OP_EQ, 0);
  tt_uint_op(dos_stats->cc_stats.circuit_bucket, OP_EQ, current_circ_count);

  /* Now use a very very large time, and check that the token bucket does not
   * have more than max_circs allowance, even tho we let it simmer for so
   * long. */
  now = (time_t)INT64_MAX; /* ???? */
  update_approx_time(now);
  cc_stats_refill_bucket(&dos_stats->cc_stats, addr);
  current_circ_count += max_circuit_count;
  tt_uint_op(dos_stats->cc_stats.circuit_bucket, OP_EQ, current_circ_count);

  /* Now send as many CREATE cells as needed to deplete our token bucket
   * completely */
  for (; current_circ_count != 0; current_circ_count--) {
    dos_cc_new_create_cell(chan);
  }
  tt_uint_op(current_circ_count, OP_EQ, 0);
  tt_uint_op(dos_stats->cc_stats.circuit_bucket, OP_EQ, current_circ_count);
#endif /* SIZEOF_TIME_T == 8 */

 done:
  tor_free(chan);
  dos_free_all();
}

/* Test if we avoid counting a known relay. */
static void
test_known_relay(void *arg)
{
  clientmap_entry_t *entry = NULL;
  routerstatus_t *rs = NULL; microdesc_t *md = NULL; routerinfo_t *ri = NULL;

  (void) arg;

  MOCK(networkstatus_get_latest_consensus,
       mock_networkstatus_get_latest_consensus);
  MOCK(networkstatus_get_latest_consensus_by_flavor,
       mock_networkstatus_get_latest_consensus_by_flavor);
  MOCK(get_estimated_address_per_node,
       mock_get_estimated_address_per_node);
  MOCK(get_param_cc_enabled, mock_enable_dos_protection);

  dos_init();

  dummy_ns = tor_malloc_zero(sizeof(*dummy_ns));
  dummy_ns->flavor = FLAV_MICRODESC;
  dummy_ns->routerstatus_list = smartlist_new();

  /* Setup an OR conn so we can pass it to the DoS subsystem. */
  or_connection_t or_conn;
  tor_addr_parse(&TO_CONN(&or_conn)->addr, "42.42.42.42");

  rs = tor_malloc_zero(sizeof(*rs));
  tor_addr_copy(&rs->ipv4_addr, &TO_CONN(&or_conn)->addr);
  crypto_rand(rs->identity_digest, sizeof(rs->identity_digest));
  smartlist_add(dummy_ns->routerstatus_list, rs);

  /* This will make the nodelist bloom filter very large
   * (the_nodelist->node_addrs) so we will fail the contain test rarely. */
  addr_per_node = 1024;
  nodelist_set_consensus(dummy_ns);

  /* We have now a node in our list so we'll make sure we don't count it as a
   * client connection. */
  geoip_note_client_seen(GEOIP_CLIENT_CONNECT, &TO_CONN(&or_conn)->addr,
                         NULL, 0);
  /* Suppose we have 5 connections in rapid succession, the counter should
   * always be 0 because we should ignore this. */
  dos_new_client_conn(&or_conn, NULL);
  dos_new_client_conn(&or_conn, NULL);
  dos_new_client_conn(&or_conn, NULL);
  dos_new_client_conn(&or_conn, NULL);
  dos_new_client_conn(&or_conn, NULL);
  entry = geoip_lookup_client(&TO_CONN(&or_conn)->addr, NULL,
                              GEOIP_CLIENT_CONNECT);
  tt_assert(entry);
  /* We should have a count of 0. */
  tt_uint_op(entry->dos_stats.concurrent_count, OP_EQ, 0);

  /* To make sure that his is working properly, make a unknown client
   * connection and see if we do get it. */
  tor_addr_parse(&TO_CONN(&or_conn)->addr, "42.42.42.43");
  geoip_note_client_seen(GEOIP_CLIENT_CONNECT, &TO_CONN(&or_conn)->addr,
                         NULL, 0);
  dos_new_client_conn(&or_conn, NULL);
  dos_new_client_conn(&or_conn, NULL);
  entry = geoip_lookup_client(&TO_CONN(&or_conn)->addr, NULL,
                              GEOIP_CLIENT_CONNECT);
  tt_assert(entry);
  /* We should have a count of 2. */
  tt_uint_op(entry->dos_stats.concurrent_count, OP_EQ, 2);

 done:
  routerstatus_free(rs); routerinfo_free(ri); microdesc_free(md);
  smartlist_clear(dummy_ns->routerstatus_list);
  networkstatus_vote_free(dummy_ns);
  dos_free_all();
  UNMOCK(networkstatus_get_latest_consensus);
  UNMOCK(networkstatus_get_latest_consensus_by_flavor);
  UNMOCK(get_estimated_address_per_node);
  UNMOCK(get_param_cc_enabled);
}

struct testcase_t dos_tests[] = {
  { "conn_creation", test_dos_conn_creation, TT_FORK, NULL, NULL },
  { "circuit_creation", test_dos_circuit_creation, TT_FORK, NULL, NULL },
  { "bucket_refill", test_dos_bucket_refill, TT_FORK, NULL, NULL },
  { "known_relay" , test_known_relay, TT_FORK,
    NULL, NULL },
  END_OF_TESTCASES
};
