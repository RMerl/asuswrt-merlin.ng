/* Copyright (c) 2017-2021, The Tor Project, Inc. */
/* Copyright (c) 2017, isis agora lovecruft  */
/* See LICENSE for licensing information     */

/**
 * \file test_router.c
 * \brief Unittests for code in router.c
 **/

#define CONFIG_PRIVATE
#define CONNECTION_PRIVATE
#define ROUTER_PRIVATE

#include "core/or/or.h"
#include "app/config/config.h"
#include "core/mainloop/mainloop.h"
#include "core/mainloop/connection.h"
#include "feature/hibernate/hibernate.h"
#include "feature/nodelist/networkstatus.h"
#include "feature/nodelist/networkstatus_st.h"
#include "feature/nodelist/node_st.h"
#include "feature/nodelist/nodelist.h"
#include "feature/nodelist/routerinfo_st.h"
#include "feature/nodelist/routerlist.h"
#include "feature/nodelist/routerstatus_st.h"
#include "feature/nodelist/torcert.h"
#include "feature/relay/router.h"
#include "feature/stats/bwhist.h"
#include "lib/crypt_ops/crypto_curve25519.h"
#include "lib/crypt_ops/crypto_ed25519.h"
#include "lib/encoding/confline.h"

#include "core/or/listener_connection_st.h"

/* Test suite stuff */
#include "test/test.h"
#include "test/log_test_helpers.h"

static routerinfo_t *
rtr_tests_gen_routerinfo(crypto_pk_t *ident_key, crypto_pk_t *tap_key)
{
  time_t now;

  routerinfo_t *mock_routerinfo;

  /* Mock the published timestamp, otherwise router_dump_router_to_string()
   * will poop its pants. */
  time(&now);

  /* We'll need keys, or router_dump_router_to_string() would return NULL. */
  tor_assert(ident_key != NULL);
  tor_assert(tap_key != NULL);

  mock_routerinfo = tor_malloc_zero(sizeof(routerinfo_t));
  mock_routerinfo->nickname = tor_strdup("ConlonNancarrow");
  tor_addr_from_ipv4h(&mock_routerinfo->ipv4_addr, 123456789);
  mock_routerinfo->ipv4_orport = 443;
  mock_routerinfo->platform = tor_strdup("unittest");
  mock_routerinfo->cache_info.published_on = now;
  mock_routerinfo->identity_pkey = crypto_pk_dup_key(ident_key);
  mock_routerinfo->protocol_list =
    tor_strdup("Cons=1-2 Desc=1-2 DirCache=1-2");
  router_set_rsa_onion_pkey(tap_key, &mock_routerinfo->onion_pkey,
                            &mock_routerinfo->onion_pkey_len);
  mock_routerinfo->bandwidthrate = 9001;
  mock_routerinfo->bandwidthburst = 9002;

  return mock_routerinfo;
}

/* If no distribution option was set, then check_bridge_distribution_setting()
 * should have set it to "any". */
static void
test_router_dump_router_to_string_no_bridge_distribution_method(void *arg)
{
  const char* needle = "bridge-distribution-request any";
  or_options_t* options = get_options_mutable();
  routerinfo_t* router = NULL;
  curve25519_keypair_t ntor_keypair;
  ed25519_keypair_t signing_keypair;
  ed25519_keypair_t identity_keypair;
  char* desc = NULL;
  char* found = NULL;
  (void)arg;
  crypto_pk_t *ident_key = pk_generate(0);
  crypto_pk_t *tap_key = pk_generate(0);

  options->ORPort_set = 1;
  options->BridgeRelay = 1;

  /* Generate keys which router_dump_router_to_string() expects to exist. */
  tt_int_op(0, OP_EQ, curve25519_keypair_generate(&ntor_keypair, 0));
  tt_int_op(0, OP_EQ, ed25519_keypair_generate(&signing_keypair, 0));
  tt_int_op(0, OP_EQ, ed25519_keypair_generate(&identity_keypair, 0));

  /* Set up part of our routerinfo_t so that we don't trigger any other
   * assertions in router_dump_router_to_string(). */
  router = rtr_tests_gen_routerinfo(ident_key, tap_key);
  tt_ptr_op(router, OP_NE, NULL);

  router->cache_info.signing_key_cert =
    tor_cert_create_ed25519(&identity_keypair,
                            CERT_TYPE_ID_SIGNING,
                            &signing_keypair.pubkey,
                            time(NULL),
                            86400,
                            CERT_FLAG_INCLUDE_SIGNING_KEY);

  /* The real router_get_my_routerinfo() looks up onion_curve25519_pkey using
   * get_current_curve25519_keypair(), but we don't initialise static data in
   * this test. */
  router->onion_curve25519_pkey = &ntor_keypair.pubkey;

  /* Generate our server descriptor and ensure that the substring
   * "bridge-distribution-request any" occurs somewhere within it. */
  desc = router_dump_router_to_string(router,
                                      ident_key,
                                      tap_key,
                                      &ntor_keypair,
                                      &signing_keypair);
  tt_ptr_op(desc, OP_NE, NULL);
  found = strstr(desc, needle);
  tt_ptr_op(found, OP_NE, NULL);

 done:
  if (router)
    router->onion_curve25519_pkey = NULL; // avoid double-free
  routerinfo_free(router);
  tor_free(desc);
  crypto_pk_free(ident_key);
  crypto_pk_free(tap_key);
}

static routerinfo_t *mock_router_get_my_routerinfo_result = NULL;

static const routerinfo_t *
mock_router_get_my_routerinfo(void)
{
  return mock_router_get_my_routerinfo_result;
}

static long
mock_get_uptime_3h(void)
{
  return 3*60*60;
}

static long
mock_get_uptime_1d(void)
{
  return 24*60*60;
}

static int
mock_rep_hist_bandwidth_assess(void)
{
  return 20001;
}

static int
mock_we_are_not_hibernating(void)
{
  return 0;
}

static int
mock_we_are_hibernating(void)
{
  return 0;
}

static void
test_router_check_descriptor_bandwidth_changed(void *arg)
{
  (void)arg;
  routerinfo_t routerinfo;
  memset(&routerinfo, 0, sizeof(routerinfo));
  mock_router_get_my_routerinfo_result = NULL;

  MOCK(we_are_hibernating, mock_we_are_not_hibernating);
  MOCK(router_get_my_routerinfo, mock_router_get_my_routerinfo);
  mock_router_get_my_routerinfo_result = &routerinfo;

  /* When uptime is less than 24h, no previous bandwidth, no last_changed
   * Uptime: 10800, last_changed: 0, Previous bw: 0, Current bw: 0 */
  routerinfo.bandwidthcapacity = 0;
  MOCK(get_uptime, mock_get_uptime_3h);
  setup_full_capture_of_logs(LOG_INFO);
  check_descriptor_bandwidth_changed(time(NULL));
  expect_log_msg_not_containing(
     "Measured bandwidth has changed; rebuilding descriptor.");
  teardown_capture_of_logs();

  /* When uptime is less than 24h, previous bandwidth,
   * last_changed more than 3h ago
   * Uptime: 10800, last_changed: 0, Previous bw: 10000, Current bw: 0 */
  routerinfo.bandwidthcapacity = 10000;
  setup_full_capture_of_logs(LOG_INFO);
  check_descriptor_bandwidth_changed(time(NULL));
  expect_log_msg_containing(
     "Measured bandwidth has changed; rebuilding descriptor.");
  teardown_capture_of_logs();

  /* When uptime is less than 24h, previous bandwidth,
   * last_changed more than 3h ago, and hibernating
   * Uptime: 10800, last_changed: 0, Previous bw: 10000, Current bw: 0 */

  UNMOCK(we_are_hibernating);
  MOCK(we_are_hibernating, mock_we_are_hibernating);
  routerinfo.bandwidthcapacity = 10000;
  setup_full_capture_of_logs(LOG_INFO);
  check_descriptor_bandwidth_changed(time(NULL));
  expect_log_msg_not_containing(
     "Measured bandwidth has changed; rebuilding descriptor.");
  teardown_capture_of_logs();
  UNMOCK(we_are_hibernating);
  MOCK(we_are_hibernating, mock_we_are_not_hibernating);

  /* When uptime is less than 24h, last_changed is not more than 3h ago
   * Uptime: 10800, last_changed: x, Previous bw: 10000, Current bw: 0 */
  setup_full_capture_of_logs(LOG_INFO);
  check_descriptor_bandwidth_changed(time(NULL));
  expect_log_msg_not_containing(
     "Measured bandwidth has changed; rebuilding descriptor.");
  teardown_capture_of_logs();

  /* When uptime is less than 24h and bandwidthcapacity does not change
   * Uptime: 10800, last_changed: x, Previous bw: 10000, Current bw: 20001 */
  MOCK(bwhist_bandwidth_assess, mock_rep_hist_bandwidth_assess);
  setup_full_capture_of_logs(LOG_INFO);
  check_descriptor_bandwidth_changed(time(NULL) + 6*60*60 + 1);
  expect_log_msg_containing(
     "Measured bandwidth has changed; rebuilding descriptor.");
  UNMOCK(get_uptime);
  UNMOCK(bwhist_bandwidth_assess);
  teardown_capture_of_logs();

  /* When uptime is more than 24h */
  MOCK(get_uptime, mock_get_uptime_1d);
  setup_full_capture_of_logs(LOG_INFO);
  check_descriptor_bandwidth_changed(time(NULL));
  expect_log_msg_not_containing(
     "Measured bandwidth has changed; rebuilding descriptor.");
  teardown_capture_of_logs();

 done:
  UNMOCK(get_uptime);
  UNMOCK(router_get_my_routerinfo);
  UNMOCK(we_are_hibernating);
}

static networkstatus_t *mock_ns = NULL;
static networkstatus_t *
mock_networkstatus_get_live_consensus(time_t now)
{
  (void)now;
  return mock_ns;
}

static routerstatus_t *mock_rs = NULL;
static const routerstatus_t *
mock_networkstatus_vote_find_entry(networkstatus_t *ns, const char *digest)
{
  (void)ns;
  (void)digest;
  return mock_rs;
}

static void
test_router_mark_if_too_old(void *arg)
{
  (void)arg;
  time_t now = approx_time();
  MOCK(networkstatus_get_live_consensus,
       mock_networkstatus_get_live_consensus);
  MOCK(networkstatus_vote_find_entry, mock_networkstatus_vote_find_entry);

  routerstatus_t rs;
  networkstatus_t ns;
  memset(&rs, 0, sizeof(rs));
  memset(&ns, 0, sizeof(ns));
  mock_ns = &ns;
  mock_ns->valid_after = now-3600;
  mock_rs = &rs;

  // no reason to mark this time.
  desc_clean_since = now-10;
  desc_dirty_reason = NULL;
  mark_my_descriptor_dirty_if_too_old(now);
  tt_i64_op(desc_clean_since, OP_EQ, now-10);

  // Doesn't appear in consensus?  Still don't mark it.
  mock_ns = NULL;
  mark_my_descriptor_dirty_if_too_old(now);
  tt_i64_op(desc_clean_since, OP_EQ, now-10);
  mock_ns = &ns;

  // No new descriptor in a long time?  Mark it.
  desc_clean_since = now - 3600 * 96;
  mark_my_descriptor_dirty_if_too_old(now);
  tt_i64_op(desc_clean_since, OP_EQ, 0);
  tt_str_op(desc_dirty_reason, OP_EQ, "time for new descriptor");

  desc_clean_since = now - 10;
  desc_dirty_reason = NULL;
  mark_my_descriptor_dirty_if_too_old(now);
  tt_i64_op(desc_clean_since, OP_EQ, now - 10);

  // Version in consensus marked as stale?  We'll mark it.
  desc_clean_since = now - 2 * 3600;
  desc_dirty_reason = NULL;
  mock_rs->is_staledesc = 1;
  mark_my_descriptor_dirty_if_too_old(now);
  tt_i64_op(desc_clean_since, OP_EQ, 0);
  tt_str_op(desc_dirty_reason, OP_EQ,
            "listed as stale in consensus");

  // same deal if we're absent from the consensus.
  desc_clean_since = now - 2 * 3600;
  desc_dirty_reason = NULL;
  mock_rs = NULL;
  mark_my_descriptor_dirty_if_too_old(now);
  tt_i64_op(desc_clean_since, OP_EQ, 0);
  tt_str_op(desc_dirty_reason, OP_EQ,
            "not listed in consensus");

 done:
  UNMOCK(networkstatus_get_live_consensus);
  UNMOCK(networkstatus_vote_find_entry);
}

static node_t fake_node;
static const node_t *
mock_node_get_by_nickname(const char *name, unsigned flags)
{
  (void)flags;
  if (!strcasecmp(name, "crumpet"))
    return &fake_node;
  else
    return NULL;
}

static void
test_router_get_my_family(void *arg)
{
  (void)arg;
  or_options_t *options = options_new();
  smartlist_t *sl = NULL;
  char *join = NULL;
  // Overwrite the result of router_get_my_identity_digest().  This
  // happens to be okay, but only for testing.
  set_server_identity_key_digest_testing(
                                   (const uint8_t*)"holeinthebottomofthe");

  setup_capture_of_logs(LOG_WARN);

  // No family listed -- so there's no list.
  sl = get_my_declared_family(options);
  tt_ptr_op(sl, OP_EQ, NULL);
  expect_no_log_entry();

#define CLEAR() do {                                    \
    if (sl) {                                           \
      SMARTLIST_FOREACH(sl, char *, cp, tor_free(cp));  \
      smartlist_free(sl);                               \
    }                                                   \
    tor_free(join);                                     \
    mock_clean_saved_logs();                            \
  } while (0)

  // Add a single nice friendly hex member.  This should be enough
  // to have our own ID added.
  tt_ptr_op(options->MyFamily, OP_EQ, NULL);
  config_line_append(&options->MyFamily, "MyFamily",
                     "$AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA");

  sl = get_my_declared_family(options);
  tt_ptr_op(sl, OP_NE, NULL);
  tt_int_op(smartlist_len(sl), OP_EQ, 2);
  join = smartlist_join_strings(sl, " ", 0, NULL);
  tt_str_op(join, OP_EQ,
            "$686F6C65696E746865626F74746F6D6F66746865 "
            "$AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA");
  expect_no_log_entry();
  CLEAR();

  // Add a hex member with a ~.  The ~ part should get removed.
  config_line_append(&options->MyFamily, "MyFamily",
                     "$0123456789abcdef0123456789abcdef01234567~Muffin");
  sl = get_my_declared_family(options);
  tt_ptr_op(sl, OP_NE, NULL);
  tt_int_op(smartlist_len(sl), OP_EQ, 3);
  join = smartlist_join_strings(sl, " ", 0, NULL);
  tt_str_op(join, OP_EQ,
            "$0123456789ABCDEF0123456789ABCDEF01234567 "
            "$686F6C65696E746865626F74746F6D6F66746865 "
            "$AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA");
  expect_no_log_entry();
  CLEAR();

  // Nickname lookup will fail, so a nickname will appear verbatim.
  config_line_append(&options->MyFamily, "MyFamily",
                     "BAGEL");
  sl = get_my_declared_family(options);
  tt_ptr_op(sl, OP_NE, NULL);
  tt_int_op(smartlist_len(sl), OP_EQ, 4);
  join = smartlist_join_strings(sl, " ", 0, NULL);
  tt_str_op(join, OP_EQ,
            "$0123456789ABCDEF0123456789ABCDEF01234567 "
            "$686F6C65696E746865626F74746F6D6F66746865 "
            "$AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA "
            "bagel");
  expect_single_log_msg_containing(
           "There is a router named \"BAGEL\" in my declared family, but "
           "I have no descriptor for it.");
  CLEAR();

  // A bogus digest should fail entirely.
  config_line_append(&options->MyFamily, "MyFamily",
                     "$painauchocolat");
  sl = get_my_declared_family(options);
  tt_ptr_op(sl, OP_NE, NULL);
  tt_int_op(smartlist_len(sl), OP_EQ, 4);
  join = smartlist_join_strings(sl, " ", 0, NULL);
  tt_str_op(join, OP_EQ,
            "$0123456789ABCDEF0123456789ABCDEF01234567 "
            "$686F6C65696E746865626F74746F6D6F66746865 "
            "$AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA "
            "bagel");
  // "BAGEL" is still there, but it won't make a warning, because we already
  // warned about it.
  expect_single_log_msg_containing(
           "There is a router named \"$painauchocolat\" in my declared "
           "family, but that isn't a legal digest or nickname. Skipping it.");
  CLEAR();

  // Let's introduce a node we can look up by nickname
  memset(&fake_node, 0, sizeof(fake_node));
  memcpy(fake_node.identity, "whydoyouasknonononon", DIGEST_LEN);
  MOCK(node_get_by_nickname, mock_node_get_by_nickname);

  config_line_append(&options->MyFamily, "MyFamily",
                     "CRUmpeT");
  sl = get_my_declared_family(options);
  tt_ptr_op(sl, OP_NE, NULL);
  tt_int_op(smartlist_len(sl), OP_EQ, 5);
  join = smartlist_join_strings(sl, " ", 0, NULL);
  tt_str_op(join, OP_EQ,
            "$0123456789ABCDEF0123456789ABCDEF01234567 "
            "$686F6C65696E746865626F74746F6D6F66746865 "
            "$776879646F796F7561736B6E6F6E6F6E6F6E6F6E "
            "$AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA "
            "bagel");
  // "BAGEL" is still there, but it won't make a warning, because we already
  // warned about it.  Some with "$painauchocolat".
  expect_single_log_msg_containing(
           "There is a router named \"CRUmpeT\" in my declared "
           "family, but it wasn't listed by digest. Please consider saying "
           "$776879646F796F7561736B6E6F6E6F6E6F6E6F6E instead, if that's "
           "what you meant.");
  CLEAR();
  UNMOCK(node_get_by_nickname);

  // Try a singleton list containing only us: It should give us NULL.
  config_free_lines(options->MyFamily);
  config_line_append(&options->MyFamily, "MyFamily",
                     "$686F6C65696E746865626F74746F6D6F66746865");
  sl = get_my_declared_family(options);
  tt_ptr_op(sl, OP_EQ, NULL);
  expect_no_log_entry();

 done:
  or_options_free(options);
  teardown_capture_of_logs();
  CLEAR();
  UNMOCK(node_get_by_nickname);

#undef CLEAR
}

static smartlist_t *fake_connection_array = NULL;
static smartlist_t *
mock_get_connection_array(void)
{
  return fake_connection_array;
}

static void
test_router_get_advertised_or_port(void *arg)
{
  (void)arg;
  int r, w=0, n=0;
  char *msg=NULL;
  or_options_t *opts = options_new();
  listener_connection_t *listener = NULL;
  tor_addr_port_t ipv6;

  // Test one failing case of routerconf_find_ipv6_or_ap().
  routerconf_find_ipv6_or_ap(opts, &ipv6);
  tt_str_op(fmt_addrport(&ipv6.addr, ipv6.port), OP_EQ, "[::]:0");

  // And one failing case of routerconf_find_or_port().
  tt_int_op(0, OP_EQ, routerconf_find_or_port(opts, AF_INET));

  // Set up a couple of configured ports.
  config_line_append(&opts->ORPort_lines, "ORPort", "[1234::5678]:auto");
  config_line_append(&opts->ORPort_lines, "ORPort", "5.6.7.8:9999");
  r = parse_ports(opts, 0, &msg, &n, &w);
  tt_assert(r == 0);

  // There are no listeners, so the "auto" case will turn up no results.
  tt_int_op(0, OP_EQ, routerconf_find_or_port(opts, AF_INET6));
  routerconf_find_ipv6_or_ap(opts, &ipv6);
  tt_str_op(fmt_addrport(&ipv6.addr, ipv6.port), OP_EQ, "[::]:0");

  // This will return the matching value from the configured port.
  tt_int_op(9999, OP_EQ, routerconf_find_or_port(opts, AF_INET));

  // Now set up a dummy listener.
  MOCK(get_connection_array, mock_get_connection_array);
  fake_connection_array = smartlist_new();
  listener = listener_connection_new(CONN_TYPE_OR_LISTENER, AF_INET6);
  TO_CONN(listener)->port = 54321;
  smartlist_add(fake_connection_array, TO_CONN(listener));

  // We should get a port this time.
  tt_int_op(54321, OP_EQ, routerconf_find_or_port(opts, AF_INET6));

  // Test one succeeding case of routerconf_find_ipv6_or_ap().
  routerconf_find_ipv6_or_ap(opts, &ipv6);
  tt_str_op(fmt_addrport(&ipv6.addr, ipv6.port), OP_EQ,
            "[1234::5678]:54321");

  // This will return the matching value from the configured port.
  tt_int_op(9999, OP_EQ, routerconf_find_or_port(opts, AF_INET));

 done:
  or_options_free(opts);
  config_free_all();
  smartlist_free(fake_connection_array);
  connection_free_minimal(TO_CONN(listener));
  UNMOCK(get_connection_array);
}

static void
test_router_get_advertised_or_port_localhost(void *arg)
{
  (void)arg;
  int r, w=0, n=0;
  char *msg=NULL;
  or_options_t *opts = options_new();
  tor_addr_port_t ipv6;

  // Set up a couple of configured ports on localhost.
  config_line_append(&opts->ORPort_lines, "ORPort", "[::1]:9999");
  config_line_append(&opts->ORPort_lines, "ORPort", "127.0.0.1:8888");
  r = parse_ports(opts, 0, &msg, &n, &w);
  tt_assert(r == 0);

  // We should refuse to advertise them, since we have default dirauths.
  routerconf_find_ipv6_or_ap(opts, &ipv6);
  tt_str_op(fmt_addrport(&ipv6.addr, ipv6.port), OP_EQ, "[::]:0");
  // But the lower-level function should still report the correct value
  tt_int_op(9999, OP_EQ, routerconf_find_or_port(opts, AF_INET6));

  // The IPv4 checks are done in resolve_my_address(), which doesn't use
  // ORPorts so we can't test them here. (See #33681.) Both these lower-level
  // functions should still report the correct value.
  tt_int_op(8888, OP_EQ, routerconf_find_or_port(opts, AF_INET));

  // Now try with a fake authority set up.
  config_line_append(&opts->DirAuthorities, "DirAuthority",
                     "127.0.0.1:1066 "
                     "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA");

  tt_int_op(9999, OP_EQ, routerconf_find_or_port(opts, AF_INET6));
  routerconf_find_ipv6_or_ap(opts, &ipv6);
  tt_str_op(fmt_addrport(&ipv6.addr, ipv6.port), OP_EQ, "[::1]:9999");

  tt_int_op(8888, OP_EQ, routerconf_find_or_port(opts, AF_INET));

 done:
  or_options_free(opts);
  config_free_all();
}

#define ROUTER_TEST(name, flags)                          \
  { #name, test_router_ ## name, flags, NULL, NULL }

struct testcase_t router_tests[] = {
  ROUTER_TEST(check_descriptor_bandwidth_changed, TT_FORK),
  ROUTER_TEST(dump_router_to_string_no_bridge_distribution_method, TT_FORK),
  ROUTER_TEST(mark_if_too_old, TT_FORK),
  ROUTER_TEST(get_my_family, TT_FORK),
  ROUTER_TEST(get_advertised_or_port, TT_FORK),
  ROUTER_TEST(get_advertised_or_port_localhost, TT_FORK),
  END_OF_TESTCASES
};
