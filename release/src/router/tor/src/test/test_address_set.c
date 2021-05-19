/* Copyright (c) 2017-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

#include "core/or/or.h"
#include "lib/crypt_ops/crypto_rand.h"
#include "core/or/address_set.h"
#include "feature/nodelist/dirlist.h"
#include "feature/nodelist/microdesc.h"
#include "feature/nodelist/networkstatus.h"
#include "feature/nodelist/nodelist.h"
#include "feature/nodelist/routerlist.h"
#include "feature/nodelist/torcert.h"

#include "feature/nodelist/microdesc_st.h"
#include "feature/nodelist/networkstatus_st.h"
#include "feature/nodelist/routerinfo_st.h"
#include "feature/nodelist/routerstatus_st.h"

#include "test/test.h"

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

static void
mock_dirlist_add_trusted_dir_addresses(void)
{
  return;
}

/* Number of address a single node_t can have. Default to the production
 * value. This is to control the size of the bloom filter. */
static int addr_per_node = 2;
static int
mock_get_estimated_address_per_node(void)
{
  return addr_per_node;
}

static void
test_contains(void *arg)
{
  int ret;
  address_set_t *set = NULL;

  (void) arg;

  /* Setup an IPv4 and IPv6 addresses. */
  tor_addr_t addr_v6;
  tor_addr_parse(&addr_v6, "1:2:3:4::");
  tor_addr_t addr_v4;
  tor_addr_parse(&addr_v4, "42.42.42.42");
  uint32_t ipv4h = tor_addr_to_ipv4h(&addr_v4);

  /* Make it very big so the chance of failing the contain test will be
   * extremely rare. */
  set = address_set_new(1024);
  tt_assert(set);

  /* Add and lookup IPv6. */
  address_set_add(set, &addr_v6);
  ret = address_set_probably_contains(set, &addr_v6);
  tt_int_op(ret, OP_EQ, 1);

  /* Add and lookup IPv4. */
  address_set_add_ipv4h(set, ipv4h);
  ret = address_set_probably_contains(set, &addr_v4);
  tt_int_op(ret, OP_EQ, 1);

  /* Try a lookup of rubbish. */
  tor_addr_t dummy_addr;
  memset(&dummy_addr, 'A', sizeof(dummy_addr));
  dummy_addr.family = AF_INET;
  ret = address_set_probably_contains(set, &dummy_addr);
  tt_int_op(ret, OP_EQ, 0);
  dummy_addr.family = AF_INET6;
  ret = address_set_probably_contains(set, &dummy_addr);
  tt_int_op(ret, OP_EQ, 0);

 done:
  address_set_free(set);
}

static void
test_nodelist(void *arg)
{
  int ret;
  routerstatus_t *rs = NULL; microdesc_t *md = NULL; routerinfo_t *ri = NULL;

  (void) arg;

  MOCK(networkstatus_get_latest_consensus,
       mock_networkstatus_get_latest_consensus);
  MOCK(networkstatus_get_latest_consensus_by_flavor,
       mock_networkstatus_get_latest_consensus_by_flavor);
  MOCK(get_estimated_address_per_node,
       mock_get_estimated_address_per_node);
  MOCK(dirlist_add_trusted_dir_addresses,
       mock_dirlist_add_trusted_dir_addresses);

  dummy_ns = tor_malloc_zero(sizeof(*dummy_ns));
  dummy_ns->flavor = FLAV_MICRODESC;
  dummy_ns->routerstatus_list = smartlist_new();

  tor_addr_t addr_v4, addr_v6, dummy_addr;
  tor_addr_parse(&addr_v4, "42.42.42.42");
  tor_addr_parse(&addr_v6, "1:2:3:4::");
  memset(&dummy_addr, 'A', sizeof(dummy_addr));

  /* This will make the nodelist bloom filter very large
   * (the_nodelist->node_addrs) so we will fail the contain test rarely. */
  addr_per_node = 1024;

  /* No node no nothing. The lookups should be empty. We've mocked the
   * dirlist_add_trusted_dir_addresses in order for _no_ authorities to be
   * added to the filter else it makes this test to trigger many false
   * positive. */
  nodelist_set_consensus(dummy_ns);

  /* The address set should be empty. */
  ret = nodelist_probably_contains_address(&addr_v4);
  tt_int_op(ret, OP_EQ, 0);
  ret = nodelist_probably_contains_address(&addr_v6);
  tt_int_op(ret, OP_EQ, 0);
  dummy_addr.family = AF_INET;
  ret = nodelist_probably_contains_address(&dummy_addr);
  tt_int_op(ret, OP_EQ, 0);
  dummy_addr.family = AF_INET6;
  ret = nodelist_probably_contains_address(&dummy_addr);
  tt_int_op(ret, OP_EQ, 0);

  md = tor_malloc_zero(sizeof(*md));
  ri = tor_malloc_zero(sizeof(*ri));
  rs = tor_malloc_zero(sizeof(*rs));
  crypto_rand(rs->identity_digest, sizeof(rs->identity_digest));
  crypto_rand(md->digest, sizeof(md->digest));
  memcpy(rs->descriptor_digest, md->digest, DIGEST256_LEN);

  /* Setup the rs, ri and md addresses. */
  tor_addr_copy(&rs->ipv4_addr, &addr_v4);
  tor_addr_parse(&rs->ipv6_addr, "1:2:3:4::");
  tor_addr_copy(&ri->ipv4_addr, &addr_v4);
  tor_addr_parse(&ri->ipv6_addr, "1:2:3:4::");
  tor_addr_parse(&md->ipv6_addr, "1:2:3:4::");

  /* Add the rs to the consensus becoming a node_t. */
  smartlist_add(dummy_ns->routerstatus_list, rs);
  nodelist_set_consensus(dummy_ns);

  /* At this point, the address set should be initialized in the nodelist and
   * we should be able to lookup. */
  ret = nodelist_probably_contains_address(&addr_v4);
  tt_int_op(ret, OP_EQ, 1);
  ret = nodelist_probably_contains_address(&addr_v6);
  tt_int_op(ret, OP_EQ, 1);
  /* Lookup unknown address. */
  dummy_addr.family = AF_INET;
  ret = nodelist_probably_contains_address(&dummy_addr);
  tt_int_op(ret, OP_EQ, 0);
  dummy_addr.family = AF_INET6;
  ret = nodelist_probably_contains_address(&dummy_addr);
  tt_int_op(ret, OP_EQ, 0);

 done:
  routerstatus_free(rs); routerinfo_free(ri); microdesc_free(md);
  smartlist_clear(dummy_ns->routerstatus_list);
  networkstatus_vote_free(dummy_ns);
  UNMOCK(networkstatus_get_latest_consensus);
  UNMOCK(networkstatus_get_latest_consensus_by_flavor);
  UNMOCK(get_estimated_address_per_node);
  UNMOCK(dirlist_add_trusted_dir_addresses);
}

/** Test that the no-reentry exit filter works as intended */
static void
test_exit_no_reentry(void *arg)
{
  routerstatus_t *rs = NULL; microdesc_t *md = NULL; routerinfo_t *ri = NULL;
  (void) arg;

  MOCK(networkstatus_get_latest_consensus,
       mock_networkstatus_get_latest_consensus);
  MOCK(networkstatus_get_latest_consensus_by_flavor,
       mock_networkstatus_get_latest_consensus_by_flavor);
  MOCK(get_estimated_address_per_node,
       mock_get_estimated_address_per_node);
  MOCK(dirlist_add_trusted_dir_addresses,
       mock_dirlist_add_trusted_dir_addresses);

  dummy_ns = tor_malloc_zero(sizeof(*dummy_ns));
  dummy_ns->flavor = FLAV_MICRODESC;
  dummy_ns->routerstatus_list = smartlist_new();

  tor_addr_t addr_v4, addr_v6, dummy_addr;
  tor_addr_parse(&addr_v4, "42.42.42.42");
  tor_addr_parse(&addr_v6, "1:2:3:4::");
  memset(&dummy_addr, 'A', sizeof(dummy_addr));

  /* This will make the nodelist bloom filter very large
   * (the_nodelist->node_addrs) so we will fail the contain test rarely. */
  addr_per_node = 1024;

  /* After this point the nodelist is populated with the directory authorities
   * address and ports */
  nodelist_set_consensus(dummy_ns);

  /* The address set is empty. Try it anyway */
  tt_assert(!nodelist_reentry_contains(&addr_v4, 244));
  tt_assert(!nodelist_reentry_contains(&addr_v6, 244));

  /* Now let's populate the network */
  md = tor_malloc_zero(sizeof(*md));
  ri = tor_malloc_zero(sizeof(*ri));
  rs = tor_malloc_zero(sizeof(*rs));
  crypto_rand(rs->identity_digest, sizeof(rs->identity_digest));
  crypto_rand(md->digest, sizeof(md->digest));
  memcpy(rs->descriptor_digest, md->digest, DIGEST256_LEN);

  /* Setup the rs, ri and md addresses. */
  tor_addr_copy(&rs->ipv4_addr, &addr_v4);
  rs->ipv4_orport = 444;
  tor_addr_parse(&rs->ipv6_addr, "1:2:3:4::");
  rs->ipv6_orport = 666;
  tor_addr_copy(&ri->ipv4_addr, &addr_v4);
  tor_addr_parse(&ri->ipv6_addr, "1:2:3:4::");
  tor_addr_parse(&md->ipv6_addr, "1:2:3:4::");

  /* Add the rs to the consensus becoming a node_t. */
  smartlist_add(dummy_ns->routerstatus_list, rs);
  nodelist_set_consensus(dummy_ns);

  /* Now that the nodelist is populated let's do some retry attempts */

  /* First let's try an address that is on the no-reentry list, but with a
     different port */
  tt_assert(!nodelist_reentry_contains(&addr_v4, 666));
  tt_assert(!nodelist_reentry_contains(&addr_v6, 444));

  /* OK now let's try with the right address and right port */
  tt_assert(nodelist_reentry_contains(&addr_v4, 444));
  tt_assert(nodelist_reentry_contains(&addr_v6, 666));

 done:
  routerstatus_free(rs); routerinfo_free(ri); microdesc_free(md);
  smartlist_clear(dummy_ns->routerstatus_list);
  networkstatus_vote_free(dummy_ns);
  UNMOCK(networkstatus_get_latest_consensus);
  UNMOCK(networkstatus_get_latest_consensus_by_flavor);
  UNMOCK(get_estimated_address_per_node);
  UNMOCK(dirlist_add_trusted_dir_addresses);
}

struct testcase_t address_set_tests[] = {
  { "contains", test_contains, TT_FORK,
    NULL, NULL },
  { "nodelist", test_nodelist, TT_FORK,
    NULL, NULL },
  { "exit_no_reentry", test_exit_no_reentry, TT_FORK, NULL, NULL },

  END_OF_TESTCASES
};

