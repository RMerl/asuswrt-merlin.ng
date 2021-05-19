/* Copyright (c) 2018-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information     */

/**
 * \file test_bridges.c
 * \brief Unittests for code in bridges.c
 **/

#define TOR_BRIDGES_PRIVATE
#define PT_PRIVATE /* Only needed for the mock_* items below */

#include <stdbool.h>

#include "core/or/or.h"
#include "lib/net/address.h"
#include "feature/client/bridges.h"
#include "app/config/config.h"
#include "feature/client/transports.h"
#include "feature/nodelist/node_st.h"
#include "feature/nodelist/routerinfo_st.h"
#include "feature/nodelist/routerstatus_st.h"
#include "feature/nodelist/microdesc_st.h"

/* Test suite stuff */
#include "test/test.h"

/**
 * A mocked transport_t, constructed via mock_transport_get_by_name().
 */
static transport_t *mock_transport = NULL;

/**
 * Mock transport_get_by_name() to simply return a transport_t for the
 * transport name that was input to it.
 */
static transport_t *
mock_transport_get_by_name(const char *name)
{
  tor_addr_t *addr = tor_malloc(sizeof(tor_addr_t));
  uint16_t port = 9999;
  int socksv = 9;
  char *args = tor_strdup("foo=bar");

  if (!mock_transport) {
    tor_addr_parse(addr, "99.99.99.99");
    mock_transport = transport_new(addr, port, name, socksv, args);
  }

  tor_free(addr);
  tor_free(args);

  return mock_transport;
}

#undef PT_PRIVATE /* defined(PT_PRIVATE) */

/**
 * Test helper: Add a variety of bridges to our global bridgelist.
 */
static void
helper_add_bridges_to_bridgelist(void *arg)
{
  /* Note: the two bridges which do not have specified fingerprints will be
   * internally stored as both having the same fingerprint of all-zero bytes.
   */

  (void)arg;
  char *bridge0 = tor_strdup("6.6.6.6:6666");
  char *bridge1 = tor_strdup("6.6.6.7:6667 "
                             "A10C4F666D27364036B562823E5830BC448E046A");
  char *bridge2 = tor_strdup("obfs4 198.245.60.51:443 "
                             "752CF7825B3B9EA6A98C83AC41F7099D67007EA5 "
                  "cert=xpmQtKUqQ/6v5X7ijgYE/f03+l2/EuQ1dexjyUhh16wQlu/"
                             "cpXUGalmhDIlhuiQPNEKmKw iat-mode=0");
  char *bridge3 = tor_strdup("banana 5.5.5.5:5555 "
                             "9D6AE1BD4FDF39721CE908966E79E16F9BFCCF2F");
  char *bridge4 = tor_strdup("obfs4 1.2.3.4:1234 "
                             "foo=abcdefghijklmnopqrstuvwxyz");
  char *bridge5 = tor_strdup("apple 4.4.4.4:4444 "
                             "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA "
                             "foo=abcdefghijklmnopqrstuvwxyz");
  char *bridge6 = tor_strdup("[2001:0db8:85a3:0000:0000:8a2e:0370:7334]:6666");

  mark_bridge_list();

#define ADD_BRIDGE(bridge)                                          \
  bridge_line_t *bridge_line_ ##bridge = parse_bridge_line(bridge); \
  if (!bridge_line_ ##bridge) {                                     \
    printf("Unparseable bridge line: '%s'", #bridge);               \
  } else {                                                          \
    bridge_add_from_config(bridge_line_ ##bridge);                  \
  }                                                                 \
  tor_free(bridge);

  ADD_BRIDGE(bridge0);
  ADD_BRIDGE(bridge1);
  ADD_BRIDGE(bridge2);
  ADD_BRIDGE(bridge3);
  ADD_BRIDGE(bridge4);
  ADD_BRIDGE(bridge5);
  ADD_BRIDGE(bridge6);
#undef ADD_BRIDGES

  sweep_bridge_list();
}

/**
 * Make sure our test helper works too.
 */
static void
test_bridges_helper_func_add_bridges_to_bridgelist(void *arg)
{
  helper_add_bridges_to_bridgelist(arg);
  tt_finished();

 done:
  mark_bridge_list();
  sweep_bridge_list();
}

/**
 * Calling bridge_list_get() should create a new bridgelist if we
 * didn't have one before.
 */
static void
test_bridges_bridge_list_get_creates_new_bridgelist(void *arg)
{
  const smartlist_t *bridgelist = bridge_list_get();

  (void)arg;

  tt_ptr_op(bridgelist, OP_NE, NULL);

 done:
  return;
}

/**
 * Calling clear_bridge_list() should remove all bridges from the bridgelist.
 */
static void
test_bridges_clear_bridge_list(void *arg)
{
  const smartlist_t *bridgelist;
  const smartlist_t *bridgelist_after;
  const bridge_info_t *bridge;

  helper_add_bridges_to_bridgelist(arg);
  bridgelist = bridge_list_get();
  tt_ptr_op(bridgelist, OP_NE, NULL);

  bridge = smartlist_get(bridgelist, 0);
  tt_ptr_op(bridge, OP_NE, NULL);

  clear_bridge_list();
  bridgelist_after = bridge_list_get();
  tt_ptr_op(bridgelist_after, OP_NE, NULL);
  tt_int_op(smartlist_len(bridgelist_after), OP_EQ, 0);

 done:
  return;
}

/**
 * Calling bridge_get_addrport() should give me the address and port
 * of the bridge.  In this case, we sort the smartlist of bridges on
 * fingerprints and choose the first one.
 */
static void
test_bridges_bridge_get_addrport(void *arg)
{
  smartlist_t *bridgelist;
  const bridge_info_t *bridge;
  const tor_addr_port_t *addrport;

  helper_add_bridges_to_bridgelist(arg);
  bridgelist = (smartlist_t*)bridge_list_get();
  tt_ptr_op(bridgelist, OP_NE, NULL);

  // This should be the bridge at 6.6.6.6:6666 with fingerprint
  // 0000000000000000000000000000000000000000
  bridge = smartlist_get(bridgelist, 0);
  tt_ptr_op(bridge, OP_NE, NULL);

  addrport = bridge_get_addr_port(bridge);
  tt_int_op(addrport->port, OP_EQ, 6666);

 done:
  mark_bridge_list();
  sweep_bridge_list();
}

/**
 * Calling get_configured_bridge_by_orports_digest() with two
 * configured bridge orports and an invalid digest should return the
 * bridge of the first addrport in the list.
 */
static void
test_bridges_get_configured_bridge_by_orports_digest(void *arg)
{
  smartlist_t *orports = NULL;
  const smartlist_t *bridgelist;
  const bridge_info_t *bridge1;
  const bridge_info_t *bridge2;
  const bridge_info_t *ret;
  tor_addr_port_t *addrport1;
  tor_addr_port_t *addrport2;
  const char *digest;

  helper_add_bridges_to_bridgelist(arg);
  bridgelist = bridge_list_get();
  tt_ptr_op(bridgelist, OP_NE, NULL);

  // This should be the bridge at 6.6.6.6:6666 with fingerprint
  // 0000000000000000000000000000000000000000
  bridge1 = smartlist_get(bridgelist, 0);
  tt_ptr_op(bridge1, OP_NE, NULL);
  // This should be the bridge at 6.6.6.7:6667 with fingerprint
  // A10C4F666D27364036B562823E5830BC448E046A
  bridge2 = smartlist_get(bridgelist, 1);
  tt_ptr_op(bridge2, OP_NE, NULL);

  addrport1 = (tor_addr_port_t*)bridge_get_addr_port(bridge1);
  tt_int_op(addrport1->port, OP_EQ, 6666);
  addrport2 = (tor_addr_port_t*)bridge_get_addr_port(bridge2);
  tt_int_op(addrport2->port, OP_EQ, 6667);

  orports = smartlist_new();
  smartlist_add(orports, addrport1);
  smartlist_add(orports, addrport2);

  digest = "zzzzzzzzzzzzzzzz";

  ret = get_configured_bridge_by_orports_digest(digest, orports);
  tt_ptr_op(ret, OP_NE, NULL);

  tt_assert(tor_addr_port_eq(addrport1, bridge_get_addr_port(ret)));

 done:
  smartlist_free(orports);

  mark_bridge_list();
  sweep_bridge_list();
}

/**
 * Calling get_configured_bridge_by_addr_port_digest() with a digest that we do
 * have and an addr:port pair we don't should return the bridge for that
 * digest.
 */
static void
test_bridges_get_configured_bridge_by_addr_port_digest_digest_only(void *arg)
{
  char digest[DIGEST_LEN];
  bridge_info_t *bridge;
  const char fingerprint[HEX_DIGEST_LEN] =
    "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";
  tor_addr_t *addr = tor_malloc(sizeof(tor_addr_t));
  char ret_addr[16];
  uint16_t port = 11111;
  int ret;

  helper_add_bridges_to_bridgelist(arg);

  // We don't actually have a bridge with this addr:port pair
  base16_decode(digest, DIGEST_LEN, fingerprint, HEX_DIGEST_LEN);
  ret = tor_addr_parse(addr, "111.111.111.111");
  tt_int_op(ret, OP_EQ, 2); // it returns the address family on success

  bridge = get_configured_bridge_by_addr_port_digest(addr, port, digest);
  tt_ptr_op(bridge, OP_NE, NULL);

  tor_addr_to_str(ret_addr, &bridge_get_addr_port(bridge)->addr, 16, 0);
  tt_str_op("4.4.4.4", OP_EQ, ret_addr);

 done:
  tor_free(addr);

  mark_bridge_list();
  sweep_bridge_list();
}

/**
 * Calling get_configured_bridge_by_addr_port_digest() with only an
 * addr:port (i.e. digest set to NULL) should return the bridge for
 * that digest when there is such a bridge.
 */
static void
test_bridges_get_configured_bridge_by_addr_port_digest_address_only(void *arg)
{
  bridge_info_t *bridge;
  tor_addr_t *addr = tor_malloc(sizeof(tor_addr_t));
  char ret_addr[16];
  uint16_t port = 6666;
  int ret;

  helper_add_bridges_to_bridgelist(arg);

  ret = tor_addr_parse(addr, "6.6.6.6");
  tt_int_op(ret, OP_EQ, 2); // it returns the address family on success

  bridge = get_configured_bridge_by_addr_port_digest(addr, port, NULL);
  tt_ptr_op(bridge, OP_NE, NULL);

  tor_addr_to_str(ret_addr, &bridge_get_addr_port(bridge)->addr, 16, 0);
  tt_str_op("6.6.6.6", OP_EQ, ret_addr);

 done:
  tor_free(addr);

  mark_bridge_list();
  sweep_bridge_list();
}

/**
 * Calling get_configured_bridge_by_exact_addr_port_digest() with a digest that
 * we do have, and an addr:port pair we don't have, should return NULL.
 */
static void
test_bridges_get_configured_bridge_by_exact_addr_port_digest_donly(void *arg)
{
  char digest[DIGEST_LEN];
  bridge_info_t *bridge;
  const char fingerprint[HEX_DIGEST_LEN] =
    "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";
  tor_addr_t *addr = tor_malloc(sizeof(tor_addr_t));
  uint16_t port = 11111;
  int ret;

  helper_add_bridges_to_bridgelist(arg);

  // We don't actually have a bridge with this addr:port pair
  base16_decode(digest, DIGEST_LEN, fingerprint, HEX_DIGEST_LEN);
  ret = tor_addr_parse(addr, "111.111.111.111");
  tt_int_op(ret, OP_EQ, 2); // it returns the address family on success

  bridge = get_configured_bridge_by_exact_addr_port_digest(addr, port, digest);
  tt_ptr_op(bridge, OP_EQ, NULL);

 done:
  tor_free(addr);

  mark_bridge_list();
  sweep_bridge_list();
}

/**
 * Calling get_configured_bridge_by_exact_addr_port_digest() with a digest that
 * we do have, and an addr:port pair we do have, should return the bridge.
 */
static void
test_bridges_get_configured_bridge_by_exact_addr_port_digest_both(void *arg)
{
  char digest[DIGEST_LEN];
  bridge_info_t *bridge;
  const char fingerprint[HEX_DIGEST_LEN] =
    "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";
  tor_addr_t *addr = tor_malloc(sizeof(tor_addr_t));
  uint16_t port = 4444;
  char ret_addr[16];
  int ret;

  helper_add_bridges_to_bridgelist(arg);

  base16_decode(digest, DIGEST_LEN, fingerprint, HEX_DIGEST_LEN);
  ret = tor_addr_parse(addr, "4.4.4.4");
  tt_int_op(ret, OP_EQ, 2); // it returns the address family on success

  bridge = get_configured_bridge_by_exact_addr_port_digest(addr, port, digest);
  tt_ptr_op(bridge, OP_NE, NULL);

  tor_addr_to_str(ret_addr, &bridge_get_addr_port(bridge)->addr, 16, 0);
  tt_str_op("4.4.4.4", OP_EQ, ret_addr);

 done:
  tor_free(addr);

  mark_bridge_list();
  sweep_bridge_list();
}

/**
 * Calling get_configured_bridge_by_exact_addr_port_digest() with no digest,
 * and an addr:port pair we do have, should return the bridge.
 */
static void
test_bridges_get_configured_bridge_by_exact_addr_port_digest_aonly(void *arg)
{
  bridge_info_t *bridge;
  tor_addr_t *addr = tor_malloc(sizeof(tor_addr_t));
  uint16_t port = 4444;
  char ret_addr[16];
  int ret;

  helper_add_bridges_to_bridgelist(arg);

  ret = tor_addr_parse(addr, "4.4.4.4");
  tt_int_op(ret, OP_EQ, 2); // it returns the address family on success

  bridge = get_configured_bridge_by_exact_addr_port_digest(addr, port, NULL);
  tt_ptr_op(bridge, OP_NE, NULL);

  tor_addr_to_str(ret_addr, &bridge_get_addr_port(bridge)->addr, 16, 0);
  tt_str_op("4.4.4.4", OP_EQ, ret_addr);

 done:
  tor_free(addr);

  mark_bridge_list();
  sweep_bridge_list();
}

/**
 * Calling find_bridge_by_digest() when we have a bridge with a known
 * identity digest should return the bridge's information.
 */
static void
test_bridges_find_bridge_by_digest_known(void *arg)
{
  char digest1[DIGEST_LEN];
  bridge_info_t *bridge;
  const char fingerprint[HEX_DIGEST_LEN] =
    "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";

  helper_add_bridges_to_bridgelist(arg);

  base16_decode(digest1, DIGEST_LEN, fingerprint, HEX_DIGEST_LEN);
  bridge = find_bridge_by_digest(digest1);

  tt_ptr_op(bridge, OP_NE, NULL);

  /* We have to call bridge_get_rsa_id_digest() here because the bridge_info_t
   * struct is opaquely defined in bridges.h. */
  const uint8_t *digest2 = bridge_get_rsa_id_digest(bridge);

  tt_mem_op((char*)digest2, OP_EQ, digest1, DIGEST_LEN);

 done:
  mark_bridge_list();
  sweep_bridge_list();
}

/**
 * Calling find_bridge_by_digest() when we do NOT have a bridge with that
 * identity digest should return NULL.
 */
static void
test_bridges_find_bridge_by_digest_unknown(void *arg)
{
  const char *fingerprint = "cccccccccccccccccccccccccccccccccccccccc";
  bridge_info_t *bridge;

  helper_add_bridges_to_bridgelist(arg);

  bridge = find_bridge_by_digest(fingerprint);

  tt_ptr_op(bridge, OP_EQ, NULL);

 done:
  mark_bridge_list();
  sweep_bridge_list();
}

/**
 * Calling bridge_resolve_conflicts() with an identical bridge to one we've
 * already configure should mark the pre-configured bridge for removal.
 */
static void
test_bridges_bridge_resolve_conflicts(void *arg)
{
  tor_addr_t *addr = tor_malloc(sizeof(tor_addr_t));
  uint16_t port = 4444;
  const char *digest = "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA";
  const char *transport = "apple";
  int ret;

  helper_add_bridges_to_bridgelist(arg);

  ret = tor_addr_parse(addr, "4.4.4.4");
  tt_int_op(ret, OP_EQ, 2); // it returns the address family on success

  bridge_resolve_conflicts((const tor_addr_t*)addr, port, digest, transport);

  /* The bridge should now be marked for removal, and removed when we sweep the
   * bridge_list */
  sweep_bridge_list();
  ret = addr_is_a_configured_bridge((const tor_addr_t*)addr, port, digest);
  tt_int_op(ret, OP_EQ, 0);

 done:
  tor_free(addr);

  mark_bridge_list();
  sweep_bridge_list();
}

/**
 * Calling transport_is_needed() with a transport we do need ("obfs4") and a
 * bogus transport that we don't need should return 1 and 0, respectively.
 */
static void
test_bridges_transport_is_needed(void *arg)
{
  int ret;

  helper_add_bridges_to_bridgelist(arg);

  ret = transport_is_needed("obfs4");
  tt_int_op(ret, OP_EQ, 1);

  ret = transport_is_needed("apowefjaoewpaief");
  tt_int_op(ret, OP_EQ, 0);

 done:
  mark_bridge_list();
  sweep_bridge_list();
}

/**
 * Calling get_transport_by_bridge_addrport() with the address and port of a
 * configured bridge which uses a pluggable transport when there is no global
 * transport_list should return -1 and the transport_t should be NULL.
 */
static void
test_bridges_get_transport_by_bridge_addrport_no_ptlist(void *arg)
{
  transport_t *transport = NULL;
  tor_addr_t *addr = tor_malloc(sizeof(tor_addr_t));
  uint16_t port = 1234;
  int ret;

  helper_add_bridges_to_bridgelist(arg);

  ret = tor_addr_parse(addr, "1.2.3.4");
  tt_int_op(ret, OP_EQ, 2); // it returns the address family on success?

  /* This will fail because the global transport_list has nothing in it, and so
   * transport_get_by_name() has nothing to return, even the the bridge *did*
   * say it had an obfs4 transport.
   */
  ret = get_transport_by_bridge_addrport((const tor_addr_t*)addr, port,
                                         (const transport_t**)&transport);
  tt_int_op(ret, OP_EQ, -1); // returns -1 on failure
  tt_ptr_op(transport, OP_EQ, NULL);

 done:
  tor_free(addr);

  mark_bridge_list();
  sweep_bridge_list();
}

/**
 * Calling get_transport_by_bridge_addrport() with the address and port of a
 * configured bridge which uses a pluggable transport should return 0 and set
 * appropriate transport_t.
 */
static void
test_bridges_get_transport_by_bridge_addrport(void *arg)
{
  transport_t *transport = NULL;
  tor_addr_t *addr = tor_malloc(sizeof(tor_addr_t));
  uint16_t port = 1234;
  int ret;

  helper_add_bridges_to_bridgelist(arg);
  mark_transport_list(); // Also initialise our transport_list

  ret = tor_addr_parse(addr, "1.2.3.4");
  tt_int_op(ret, OP_EQ, 2); // it returns the address family on success?

  /* After we mock transport_get_by_name() to return a bogus transport_t with
   * the name it was asked for, the call should succeed.
   */
  MOCK(transport_get_by_name, mock_transport_get_by_name);
  ret = get_transport_by_bridge_addrport((const tor_addr_t*)addr, port,
                                         (const transport_t**)&transport);
  tt_int_op(ret, OP_EQ, 0); // returns 0 on success
  tt_ptr_op(transport, OP_NE, NULL);
  tt_str_op(transport->name, OP_EQ, "obfs4");

 done:
  UNMOCK(transport_get_by_name);

  tor_free(addr);
  transport_free(transport);

  mark_bridge_list();
  sweep_bridge_list();
}

static void
test_bridges_node_is_a_configured_bridge(void *arg)
{

  routerinfo_t ri_ipv4 = { .ipv4_orport = 6666 };
  tor_addr_parse(&ri_ipv4.ipv4_addr, "6.6.6.6");

  routerstatus_t rs_ipv4 = { .ipv4_orport = 6666 };
  tor_addr_parse(&rs_ipv4.ipv4_addr, "6.6.6.6");

  routerinfo_t ri_ipv6 = { .ipv6_orport = 6666 };
  tor_addr_parse(&(ri_ipv6.ipv6_addr),
                 "2001:0db8:85a3:0000:0000:8a2e:0370:7334");

  routerstatus_t rs_ipv6 = { .ipv6_orport = 6666 };
  tor_addr_parse(&(rs_ipv6.ipv6_addr),
                 "2001:0db8:85a3:0000:0000:8a2e:0370:7334");

  microdesc_t md_ipv6 = { .ipv6_orport = 6666 };
  tor_addr_parse(&(md_ipv6.ipv6_addr),
                 "2001:0db8:85a3:0000:0000:8a2e:0370:7334");

  helper_add_bridges_to_bridgelist(arg);

  node_t node_with_digest;
  memset(&node_with_digest, 0, sizeof(node_with_digest));

  const char fingerprint[HEX_DIGEST_LEN] =
    "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";

  const char fingerprint2[HEX_DIGEST_LEN] =
    "ffffffffffffffffffffffffffffffffffffffff";

  base16_decode(node_with_digest.identity, DIGEST_LEN,
                fingerprint, HEX_DIGEST_LEN);

  node_t node_ri_ipv4 = { .ri = &ri_ipv4 };
  base16_decode(node_ri_ipv4.identity, DIGEST_LEN,
                fingerprint2, HEX_DIGEST_LEN);
  tt_assert(node_is_a_configured_bridge(&node_ri_ipv4));

  /* This will still match bridge0, since bridge0 has no digest set. */
  memset(node_ri_ipv4.identity, 0x3f, DIGEST_LEN);
  tt_assert(node_is_a_configured_bridge(&node_ri_ipv4));

  /* It won't match bridge1, though, since bridge1 has a digest, and this
     isn't it! */
  tor_addr_parse(&node_ri_ipv4.ri->ipv4_addr, "6.6.6.7");
  node_ri_ipv4.ri->ipv4_orport = 6667;
  tt_assert(! node_is_a_configured_bridge(&node_ri_ipv4));
  /* If we set the fingerprint right, though, it will match. */
  base16_decode(node_ri_ipv4.identity, DIGEST_LEN,
                "A10C4F666D27364036B562823E5830BC448E046A", HEX_DIGEST_LEN);
  tt_assert(node_is_a_configured_bridge(&node_ri_ipv4));

  node_t node_rs_ipv4 = { .rs = &rs_ipv4 };
  base16_decode(node_rs_ipv4.identity, DIGEST_LEN,
                fingerprint2, HEX_DIGEST_LEN);
  tt_assert(node_is_a_configured_bridge(&node_rs_ipv4));

  node_t node_ri_ipv6 = { .ri = &ri_ipv6 };
  base16_decode(node_ri_ipv6.identity, DIGEST_LEN,
                fingerprint2, HEX_DIGEST_LEN);
  tt_assert(node_is_a_configured_bridge(&node_ri_ipv6));

  node_t node_rs_ipv6 = { .rs = &rs_ipv6 };
  base16_decode(node_rs_ipv6.identity, DIGEST_LEN,
                fingerprint2, HEX_DIGEST_LEN);
  tt_assert(node_is_a_configured_bridge(&node_rs_ipv6));

  node_t node_md_ipv6 = { .md = &md_ipv6 };
  base16_decode(node_md_ipv6.identity, DIGEST_LEN,
                fingerprint2, HEX_DIGEST_LEN);
  tt_assert(node_is_a_configured_bridge(&node_md_ipv6));

  mark_bridge_list();
  sweep_bridge_list();

  tt_assert(!node_is_a_configured_bridge(&node_with_digest));
  tt_assert(!node_is_a_configured_bridge(&node_ri_ipv4));
  tt_assert(!node_is_a_configured_bridge(&node_ri_ipv6));
  tt_assert(!node_is_a_configured_bridge(&node_rs_ipv4));
  tt_assert(!node_is_a_configured_bridge(&node_rs_ipv6));
  tt_assert(!node_is_a_configured_bridge(&node_md_ipv6));

 done:
  mark_bridge_list();
  sweep_bridge_list();
}

#undef PT_PRIVATE /* defined(PT_PRIVATE) */

#define B_TEST(name, flags) \
  { #name, test_bridges_ ##name, (flags), NULL, NULL }

struct testcase_t bridges_tests[] = {
  B_TEST(helper_func_add_bridges_to_bridgelist, 0),
  B_TEST(bridge_list_get_creates_new_bridgelist, 0),
  B_TEST(clear_bridge_list, 0),
  B_TEST(bridge_get_addrport, 0),
  B_TEST(get_configured_bridge_by_orports_digest, 0),
  B_TEST(get_configured_bridge_by_addr_port_digest_digest_only, 0),
  B_TEST(get_configured_bridge_by_addr_port_digest_address_only, 0),
  B_TEST(get_configured_bridge_by_exact_addr_port_digest_donly, 0),
  B_TEST(get_configured_bridge_by_exact_addr_port_digest_both, 0),
  B_TEST(get_configured_bridge_by_exact_addr_port_digest_aonly, 0),
  B_TEST(find_bridge_by_digest_known, 0),
  B_TEST(find_bridge_by_digest_unknown, 0),
  B_TEST(bridge_resolve_conflicts, 0),
  B_TEST(get_transport_by_bridge_addrport_no_ptlist, 0),
  B_TEST(get_transport_by_bridge_addrport, 0),
  B_TEST(transport_is_needed, 0),
  B_TEST(node_is_a_configured_bridge, 0),
  END_OF_TESTCASES
};
