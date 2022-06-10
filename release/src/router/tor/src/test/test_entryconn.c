/* Copyright (c) 2014-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

#include "orconfig.h"

#define CONNECTION_PRIVATE
#define CONNECTION_EDGE_PRIVATE

#include "core/or/or.h"
#include "test/test.h"

#include "feature/client/addressmap.h"
#include "app/config/config.h"
#include "lib/confmgt/confmgt.h"
#include "core/mainloop/connection.h"
#include "core/or/connection_edge.h"
#include "feature/nodelist/nodelist.h"

#include "feature/hs/hs_cache.h"

#include "core/or/entry_connection_st.h"
#include "core/or/socks_request_st.h"

#include "lib/encoding/confline.h"

static void *
entryconn_rewrite_setup(const struct testcase_t *tc)
{
  (void)tc;
  entry_connection_t *ec = entry_connection_new(CONN_TYPE_AP, AF_INET);
  addressmap_init();
  return ec;
}

static int
entryconn_rewrite_teardown(const struct testcase_t *tc, void *arg)
{
  (void)tc;
  entry_connection_t *ec = arg;
  if (ec)
    connection_free_minimal(ENTRY_TO_CONN(ec));
  addressmap_free_all();
  return 1;
}

static struct testcase_setup_t test_rewrite_setup = {
  entryconn_rewrite_setup, entryconn_rewrite_teardown
};

/* Simple rewrite: no changes needed */
static void
test_entryconn_rewrite_basic(void *arg)
{
  entry_connection_t *ec = arg;
  rewrite_result_t rr;

  tt_assert(ec->socks_request);
  strlcpy(ec->socks_request->address, "www.TORproject.org",
          sizeof(ec->socks_request->address));
  ec->socks_request->command = SOCKS_COMMAND_CONNECT;
  connection_ap_handshake_rewrite(ec, &rr);

  tt_int_op(rr.should_close, OP_EQ, 0);
  tt_int_op(rr.end_reason, OP_EQ, 0);
  tt_int_op(rr.automap, OP_EQ, 0);
  tt_i64_op(rr.map_expires, OP_EQ, TIME_MAX);
  tt_int_op(rr.exit_source, OP_EQ, ADDRMAPSRC_NONE);
  tt_str_op(rr.orig_address, OP_EQ, "www.torproject.org");
  tt_str_op(ec->socks_request->address, OP_EQ, "www.torproject.org");
  tt_str_op(ec->original_dest_address, OP_EQ, "www.torproject.org");

 done:
  ;
}

/* Rewrite but reject because of disallowed .exit */
static void
test_entryconn_rewrite_bad_dotexit(void *arg)
{
  entry_connection_t *ec = arg;
  rewrite_result_t rr;

  tt_assert(ec->socks_request);
  strlcpy(ec->socks_request->address, "www.TORproject.org.foo.exit",
          sizeof(ec->socks_request->address));
  ec->socks_request->command = SOCKS_COMMAND_CONNECT;
  connection_ap_handshake_rewrite(ec, &rr);

  tt_int_op(rr.should_close, OP_EQ, 1);
  tt_int_op(rr.end_reason, OP_EQ, END_STREAM_REASON_TORPROTOCOL);

 done:
  ;
}

/* Automap on resolve, connect to automapped address, resolve again and get
 * same answer. (IPv4) */
static void
test_entryconn_rewrite_automap_ipv4(void *arg)
{
  entry_connection_t *ec = arg;
  entry_connection_t *ec2=NULL, *ec3=NULL;
  rewrite_result_t rr;
  char *msg = NULL;

  ec2 = entry_connection_new(CONN_TYPE_AP, AF_INET);
  ec3 = entry_connection_new(CONN_TYPE_AP, AF_INET);

  get_options_mutable()->AutomapHostsOnResolve = 1;
  smartlist_add_strdup(get_options_mutable()->AutomapHostsSuffixes, ".");
  parse_virtual_addr_network("127.202.0.0/16", AF_INET, 0, &msg);

  /* Automap this on resolve. */
  strlcpy(ec->socks_request->address, "WWW.MIT.EDU",
          sizeof(ec->socks_request->address));
  ec->socks_request->command = SOCKS_COMMAND_RESOLVE;
  connection_ap_handshake_rewrite(ec, &rr);

  tt_int_op(rr.automap, OP_EQ, 1);
  tt_int_op(rr.should_close, OP_EQ, 0);
  tt_int_op(rr.end_reason, OP_EQ, 0);
  tt_i64_op(rr.map_expires, OP_EQ, TIME_MAX);
  tt_int_op(rr.exit_source, OP_EQ, ADDRMAPSRC_NONE);
  tt_str_op(rr.orig_address, OP_EQ, "www.mit.edu");
  tt_str_op(ec->original_dest_address, OP_EQ, "www.mit.edu");

  tt_assert(!strcmpstart(ec->socks_request->address,"127.202."));

  /* Connect to it and make sure we get the original address back. */
  strlcpy(ec2->socks_request->address, ec->socks_request->address,
          sizeof(ec2->socks_request->address));

  ec2->socks_request->command = SOCKS_COMMAND_CONNECT;
  connection_ap_handshake_rewrite(ec2, &rr);

  tt_int_op(rr.automap, OP_EQ, 0);
  tt_int_op(rr.should_close, OP_EQ, 0);
  tt_int_op(rr.end_reason, OP_EQ, 0);
  tt_i64_op(rr.map_expires, OP_EQ, TIME_MAX);
  tt_int_op(rr.exit_source, OP_EQ, ADDRMAPSRC_NONE);
  tt_str_op(rr.orig_address, OP_EQ, ec->socks_request->address);
  tt_str_op(ec2->original_dest_address, OP_EQ, ec->socks_request->address);
  tt_str_op(ec2->socks_request->address, OP_EQ, "www.mit.edu");

  /* Resolve it again, make sure the answer is the same. */
  strlcpy(ec3->socks_request->address, "www.MIT.EDU",
          sizeof(ec3->socks_request->address));
  ec3->socks_request->command = SOCKS_COMMAND_RESOLVE;
  connection_ap_handshake_rewrite(ec3, &rr);

  tt_int_op(rr.automap, OP_EQ, 1);
  tt_int_op(rr.should_close, OP_EQ, 0);
  tt_int_op(rr.end_reason, OP_EQ, 0);
  tt_i64_op(rr.map_expires, OP_EQ, TIME_MAX);
  tt_int_op(rr.exit_source, OP_EQ, ADDRMAPSRC_NONE);
  tt_str_op(rr.orig_address, OP_EQ, "www.mit.edu");
  tt_str_op(ec3->original_dest_address, OP_EQ, "www.mit.edu");

  tt_str_op(ec3->socks_request->address, OP_EQ,
            ec->socks_request->address);

 done:
  connection_free_minimal(ENTRY_TO_CONN(ec2));
  connection_free_minimal(ENTRY_TO_CONN(ec3));
}

/* Automap on resolve, connect to automapped address, resolve again and get
 * same answer. (IPv6) */
static void
test_entryconn_rewrite_automap_ipv6(void *arg)
{
  (void)arg;
  entry_connection_t *ec =NULL;
  entry_connection_t *ec2=NULL, *ec3=NULL;
  rewrite_result_t rr;
  char *msg = NULL;

  ec = entry_connection_new(CONN_TYPE_AP, AF_INET6);
  ec2 = entry_connection_new(CONN_TYPE_AP, AF_INET6);
  ec3 = entry_connection_new(CONN_TYPE_AP, AF_INET6);

  get_options_mutable()->AutomapHostsOnResolve = 1;
  smartlist_add_strdup(get_options_mutable()->AutomapHostsSuffixes, ".");
  parse_virtual_addr_network("FE80::/32", AF_INET6, 0, &msg);

  /* Automap this on resolve. */
  strlcpy(ec->socks_request->address, "WWW.MIT.EDU",
          sizeof(ec->socks_request->address));
  ec->socks_request->command = SOCKS_COMMAND_RESOLVE;
  connection_ap_handshake_rewrite(ec, &rr);

  tt_int_op(rr.automap, OP_EQ, 1);
  tt_int_op(rr.should_close, OP_EQ, 0);
  tt_int_op(rr.end_reason, OP_EQ, 0);
  tt_i64_op(rr.map_expires, OP_EQ, TIME_MAX);
  tt_int_op(rr.exit_source, OP_EQ, ADDRMAPSRC_NONE);
  tt_str_op(rr.orig_address, OP_EQ, "www.mit.edu");
  tt_str_op(ec->original_dest_address, OP_EQ, "www.mit.edu");

  /* Yes, this [ should be here. */
  tt_assert(!strcmpstart(ec->socks_request->address,"[fe80:"));

  /* Connect to it and make sure we get the original address back. */
  strlcpy(ec2->socks_request->address, ec->socks_request->address,
          sizeof(ec2->socks_request->address));

  ec2->socks_request->command = SOCKS_COMMAND_CONNECT;
  connection_ap_handshake_rewrite(ec2, &rr);

  tt_int_op(rr.automap, OP_EQ, 0);
  tt_int_op(rr.should_close, OP_EQ, 0);
  tt_int_op(rr.end_reason, OP_EQ, 0);
  tt_i64_op(rr.map_expires, OP_EQ, TIME_MAX);
  tt_int_op(rr.exit_source, OP_EQ, ADDRMAPSRC_NONE);
  tt_str_op(rr.orig_address, OP_EQ, ec->socks_request->address);
  tt_str_op(ec2->original_dest_address, OP_EQ, ec->socks_request->address);
  tt_str_op(ec2->socks_request->address, OP_EQ, "www.mit.edu");

  /* Resolve it again, make sure the answer is the same. */
  strlcpy(ec3->socks_request->address, "www.MIT.EDU",
          sizeof(ec3->socks_request->address));
  ec3->socks_request->command = SOCKS_COMMAND_RESOLVE;
  connection_ap_handshake_rewrite(ec3, &rr);

  tt_int_op(rr.automap, OP_EQ, 1);
  tt_int_op(rr.should_close, OP_EQ, 0);
  tt_int_op(rr.end_reason, OP_EQ, 0);
  tt_i64_op(rr.map_expires, OP_EQ, TIME_MAX);
  tt_int_op(rr.exit_source, OP_EQ, ADDRMAPSRC_NONE);
  tt_str_op(rr.orig_address, OP_EQ, "www.mit.edu");
  tt_str_op(ec3->original_dest_address, OP_EQ, "www.mit.edu");

  tt_str_op(ec3->socks_request->address, OP_EQ,
            ec->socks_request->address);

 done:
  connection_free_minimal(ENTRY_TO_CONN(ec));
  connection_free_minimal(ENTRY_TO_CONN(ec2));
  connection_free_minimal(ENTRY_TO_CONN(ec3));
}

#if 0
/* FFFF not actually supported. */
/* automap on resolve, reverse lookup. */
static void
test_entryconn_rewrite_automap_reverse(void *arg)
{
  entry_connection_t *ec = arg;
  entry_connection_t *ec2=NULL;
  rewrite_result_t rr;
  char *msg = NULL;

  ec2 = entry_connection_new(CONN_TYPE_AP, AF_INET);

  get_options_mutable()->AutomapHostsOnResolve = 1;
  get_options_mutable()->SafeLogging_ = SAFELOG_SCRUB_NONE;
  smartlist_add(get_options_mutable()->AutomapHostsSuffixes,
                tor_strdup(".bloom"));
  parse_virtual_addr_network("127.80.0.0/16", AF_INET, 0, &msg);

  /* Automap this on resolve. */
  strlcpy(ec->socks_request->address, "www.poldy.BLOOM",
          sizeof(ec->socks_request->address));
  ec->socks_request->command = SOCKS_COMMAND_RESOLVE;
  connection_ap_handshake_rewrite(ec, &rr);

  tt_int_op(rr.automap, OP_EQ, 1);
  tt_int_op(rr.should_close, OP_EQ, 0);
  tt_int_op(rr.end_reason, OP_EQ, 0);
  tt_i64_op(rr.map_expires, OP_EQ, TIME_MAX);
  tt_int_op(rr.exit_source, OP_EQ, ADDRMAPSRC_NONE);
  tt_str_op(rr.orig_address, OP_EQ, "www.poldy.bloom");
  tt_str_op(ec->original_dest_address, OP_EQ, "www.poldy.bloom");

  tt_assert(!strcmpstart(ec->socks_request->address,"127.80."));

  strlcpy(ec2->socks_request->address, ec->socks_request->address,
          sizeof(ec2->socks_request->address));
  ec2->socks_request->command = SOCKS_COMMAND_RESOLVE_PTR;
  connection_ap_handshake_rewrite(ec2, &rr);

  tt_int_op(rr.automap, OP_EQ, 0);
  tt_int_op(rr.should_close, OP_EQ, 1);
  tt_int_op(rr.end_reason, OP_EQ,
          END_STREAM_REASON_DONE|END_STREAM_REASON_FLAG_ALREADY_SOCKS_REPLIED);
  tt_i64_op(rr.map_expires, OP_EQ, TIME_MAX);
  tt_int_op(rr.exit_source, OP_EQ, ADDRMAPSRC_NONE);

 done:
  connection_free_minimal(ENTRY_TO_CONN(ec2));
}
#endif /* 0 */

/* Rewrite because of cached DNS entry. */
static void
test_entryconn_rewrite_cached_dns_ipv4(void *arg)
{
  entry_connection_t *ec = arg;
  rewrite_result_t rr;
  time_t expires = time(NULL) + 3600;
  entry_connection_t *ec2=NULL;

  ec2 = entry_connection_new(CONN_TYPE_AP, AF_INET);

  addressmap_register("www.friendly.example.com",
                      tor_strdup("240.240.241.241"),
                      expires,
                      ADDRMAPSRC_DNS,
                      0, 0, 0);

  strlcpy(ec->socks_request->address, "www.friendly.example.com",
          sizeof(ec->socks_request->address));
  strlcpy(ec2->socks_request->address, "www.friendly.example.com",
          sizeof(ec2->socks_request->address));

  ec->socks_request->command = SOCKS_COMMAND_CONNECT;
  ec2->socks_request->command = SOCKS_COMMAND_CONNECT;

  ec2->entry_cfg.use_cached_ipv4_answers = 1; /* only ec2 gets this flag */
  connection_ap_handshake_rewrite(ec, &rr);

  tt_int_op(rr.automap, OP_EQ, 0);
  tt_int_op(rr.should_close, OP_EQ, 0);
  tt_int_op(rr.end_reason, OP_EQ, 0);
  tt_i64_op(rr.map_expires, OP_EQ, TIME_MAX);
  tt_int_op(rr.exit_source, OP_EQ, ADDRMAPSRC_NONE);
  tt_str_op(rr.orig_address, OP_EQ, "www.friendly.example.com");
  tt_str_op(ec->socks_request->address, OP_EQ, "www.friendly.example.com");

  connection_ap_handshake_rewrite(ec2, &rr);
  tt_int_op(rr.automap, OP_EQ, 0);
  tt_int_op(rr.should_close, OP_EQ, 0);
  tt_int_op(rr.end_reason, OP_EQ, 0);
  tt_i64_op(rr.map_expires, OP_EQ, expires);
  tt_int_op(rr.exit_source, OP_EQ, ADDRMAPSRC_NONE);
  tt_str_op(rr.orig_address, OP_EQ, "www.friendly.example.com");
  tt_str_op(ec2->socks_request->address, OP_EQ, "240.240.241.241");

 done:
  connection_free_minimal(ENTRY_TO_CONN(ec2));
}

/* Rewrite because of cached DNS entry. */
static void
test_entryconn_rewrite_cached_dns_ipv6(void *arg)
{
  entry_connection_t *ec = NULL;
  rewrite_result_t rr;
  time_t expires = time(NULL) + 3600;
  entry_connection_t *ec2=NULL;

  (void)arg;

  ec  = entry_connection_new(CONN_TYPE_AP, AF_INET6);
  ec2 = entry_connection_new(CONN_TYPE_AP, AF_INET6);

  addressmap_register("www.friendly.example.com",
                      tor_strdup("[::f00f]"),
                      expires,
                      ADDRMAPSRC_DNS,
                      0, 0, 0);

  strlcpy(ec->socks_request->address, "www.friendly.example.com",
          sizeof(ec->socks_request->address));
  strlcpy(ec2->socks_request->address, "www.friendly.example.com",
          sizeof(ec2->socks_request->address));

  ec->socks_request->command = SOCKS_COMMAND_CONNECT;
  ec2->socks_request->command = SOCKS_COMMAND_CONNECT;

  ec2->entry_cfg.use_cached_ipv6_answers = 1; /* only ec2 gets this flag */
  connection_ap_handshake_rewrite(ec, &rr);

  tt_int_op(rr.automap, OP_EQ, 0);
  tt_int_op(rr.should_close, OP_EQ, 0);
  tt_int_op(rr.end_reason, OP_EQ, 0);
  tt_i64_op(rr.map_expires, OP_EQ, TIME_MAX);
  tt_int_op(rr.exit_source, OP_EQ, ADDRMAPSRC_NONE);
  tt_str_op(rr.orig_address, OP_EQ, "www.friendly.example.com");
  tt_str_op(ec->socks_request->address, OP_EQ, "www.friendly.example.com");

  connection_ap_handshake_rewrite(ec2, &rr);
  tt_int_op(rr.automap, OP_EQ, 0);
  tt_int_op(rr.should_close, OP_EQ, 0);
  tt_int_op(rr.end_reason, OP_EQ, 0);
  tt_i64_op(rr.map_expires, OP_EQ, expires);
  tt_int_op(rr.exit_source, OP_EQ, ADDRMAPSRC_NONE);
  tt_str_op(rr.orig_address, OP_EQ, "www.friendly.example.com");
  tt_str_op(ec2->socks_request->address, OP_EQ, "[::f00f]");

 done:
  connection_free_minimal(ENTRY_TO_CONN(ec));
  connection_free_minimal(ENTRY_TO_CONN(ec2));
}

/* Fail to connect to unmapped address in virtual range. */
static void
test_entryconn_rewrite_unmapped_virtual(void *arg)
{
  entry_connection_t *ec = arg;
  rewrite_result_t rr;
  entry_connection_t *ec2 = NULL;
  char *msg = NULL;

  ec2 = entry_connection_new(CONN_TYPE_AP, AF_INET6);

  parse_virtual_addr_network("18.202.0.0/16", AF_INET, 0, &msg);
  parse_virtual_addr_network("[ABCD::]/16", AF_INET6, 0, &msg);

  strlcpy(ec->socks_request->address, "18.202.5.5",
          sizeof(ec->socks_request->address));
  ec->socks_request->command = SOCKS_COMMAND_CONNECT;
  connection_ap_handshake_rewrite(ec, &rr);

  tt_int_op(rr.should_close, OP_EQ, 1);
  tt_int_op(rr.end_reason, OP_EQ, END_STREAM_REASON_INTERNAL);
  tt_int_op(rr.automap, OP_EQ, 0);
  tt_i64_op(rr.map_expires, OP_EQ, TIME_MAX);
  tt_int_op(rr.exit_source, OP_EQ, ADDRMAPSRC_NONE);

  strlcpy(ec2->socks_request->address, "[ABCD:9::5314:9543]",
          sizeof(ec2->socks_request->address));
  ec2->socks_request->command = SOCKS_COMMAND_CONNECT;
  connection_ap_handshake_rewrite(ec2, &rr);

  tt_int_op(rr.should_close, OP_EQ, 1);
  tt_int_op(rr.end_reason, OP_EQ, END_STREAM_REASON_INTERNAL);
  tt_int_op(rr.automap, OP_EQ, 0);
  tt_i64_op(rr.map_expires, OP_EQ, TIME_MAX);
  tt_int_op(rr.exit_source, OP_EQ, ADDRMAPSRC_NONE);

 done:
  connection_free_minimal(ENTRY_TO_CONN(ec2));
}

/* Rewrite because of mapaddress option */
static void
test_entryconn_rewrite_mapaddress(void *arg)
{
  entry_connection_t *ec = arg;
  rewrite_result_t rr;

  config_line_append(&get_options_mutable()->AddressMap,
                     "MapAddress", "meta metaobjects.example");
  config_register_addressmaps(get_options());

  strlcpy(ec->socks_request->address, "meta",
          sizeof(ec->socks_request->address));
  ec->socks_request->command = SOCKS_COMMAND_CONNECT;
  connection_ap_handshake_rewrite(ec, &rr);

  tt_int_op(rr.should_close, OP_EQ, 0);
  tt_int_op(rr.end_reason, OP_EQ, 0);
  tt_int_op(rr.automap, OP_EQ, 0);
  tt_i64_op(rr.map_expires, OP_EQ, TIME_MAX);
  tt_int_op(rr.exit_source, OP_EQ, ADDRMAPSRC_NONE);
  tt_str_op(ec->socks_request->address, OP_EQ, "metaobjects.example");

 done:
  ;
}

/* Reject reverse lookups of internal address. */
static void
test_entryconn_rewrite_reject_internal_reverse(void *arg)
{
  entry_connection_t *ec = arg;
  rewrite_result_t rr;

  strlcpy(ec->socks_request->address, "10.0.0.1",
          sizeof(ec->socks_request->address));
  ec->socks_request->command = SOCKS_COMMAND_RESOLVE_PTR;
  connection_ap_handshake_rewrite(ec, &rr);

  tt_int_op(rr.should_close, OP_EQ, 1);
  tt_int_op(rr.end_reason, OP_EQ, END_STREAM_REASON_SOCKSPROTOCOL |
            END_STREAM_REASON_FLAG_ALREADY_SOCKS_REPLIED);
  tt_int_op(rr.automap, OP_EQ, 0);
  tt_i64_op(rr.map_expires, OP_EQ, TIME_MAX);
  tt_int_op(rr.exit_source, OP_EQ, ADDRMAPSRC_NONE);

 done:
  ;
}

/* Rewrite into .exit because of virtual address mapping.  */
static void
test_entryconn_rewrite_automap_exit(void *arg)
{
  entry_connection_t *ec = arg;
  entry_connection_t *ec2=NULL;
  rewrite_result_t rr;
  char *msg = NULL;

  ec2 = entry_connection_new(CONN_TYPE_AP, AF_INET);

  smartlist_add_strdup(get_options_mutable()->AutomapHostsSuffixes,
                ".EXIT");
  parse_virtual_addr_network("127.1.0.0/16", AF_INET, 0, &msg);

  /* Try to automap this on resolve. */
  strlcpy(ec->socks_request->address, "website.example.exit",
          sizeof(ec->socks_request->address));
  ec->socks_request->command = SOCKS_COMMAND_RESOLVE;
  connection_ap_handshake_rewrite(ec, &rr);

  /* Make sure it isn't allowed -- there is no longer an AllowDotExit
   * option. */
  tt_int_op(rr.automap, OP_EQ, 0);
  tt_int_op(rr.should_close, OP_EQ, 1);
  tt_int_op(rr.end_reason, OP_EQ, END_STREAM_REASON_TORPROTOCOL);

 done:
  connection_free_minimal(ENTRY_TO_CONN(ec2));
}

/* Rewrite into .exit because of mapaddress */
static void
test_entryconn_rewrite_mapaddress_exit(void *arg)
{
  entry_connection_t *ec = arg;
  rewrite_result_t rr;

  config_line_append(&get_options_mutable()->AddressMap,
                     "MapAddress", "*.example.com  *.example.com.abc.exit");
  config_register_addressmaps(get_options());

  /* Automap this on resolve. */
  strlcpy(ec->socks_request->address, "abc.example.com",
          sizeof(ec->socks_request->address));
  ec->socks_request->command = SOCKS_COMMAND_CONNECT;
  connection_ap_handshake_rewrite(ec, &rr);

  tt_int_op(rr.automap, OP_EQ, 0);
  tt_int_op(rr.should_close, OP_EQ, 0);
  tt_int_op(rr.end_reason, OP_EQ, 0);
  tt_i64_op(rr.map_expires, OP_EQ, TIME_MAX);
  tt_int_op(rr.exit_source, OP_EQ, ADDRMAPSRC_TORRC);
  tt_str_op(rr.orig_address, OP_EQ, "abc.example.com");
  tt_str_op(ec->socks_request->address, OP_EQ, "abc.example.com.abc.exit");
 done:
  ;
}

/* Map foo.onion to longthing.onion, and also automap. */
static void
test_entryconn_rewrite_mapaddress_automap_onion(void *arg)
{
  entry_connection_t *ec = arg;
  entry_connection_t *ec2 = NULL;
  entry_connection_t *ec3 = NULL;
  entry_connection_t *ec4 = NULL;
  rewrite_result_t rr;
  char *msg = NULL;

  ec2 = entry_connection_new(CONN_TYPE_AP, AF_INET);
  ec3 = entry_connection_new(CONN_TYPE_AP, AF_INET);
  ec4 = entry_connection_new(CONN_TYPE_AP, AF_INET);

  get_options_mutable()->AutomapHostsOnResolve = 1;
  smartlist_add_strdup(get_options_mutable()->AutomapHostsSuffixes,
                ".onion");
  parse_virtual_addr_network("192.168.0.0/16", AF_INET, 0, &msg);
  config_line_append(&get_options_mutable()->AddressMap,
                     "MapAddress", "foo.onion abcdefghijklmnop.onion");
  config_register_addressmaps(get_options());

  /* Connect to foo.onion. */
  strlcpy(ec->socks_request->address, "foo.onion",
          sizeof(ec->socks_request->address));
  ec->socks_request->command = SOCKS_COMMAND_CONNECT;
  connection_ap_handshake_rewrite(ec, &rr);

  tt_int_op(rr.automap, OP_EQ, 0);
  tt_int_op(rr.should_close, OP_EQ, 0);
  tt_int_op(rr.end_reason, OP_EQ, 0);
  tt_i64_op(rr.map_expires, OP_EQ, TIME_MAX);
  tt_int_op(rr.exit_source, OP_EQ, ADDRMAPSRC_NONE);
  tt_str_op(rr.orig_address, OP_EQ, "foo.onion");
  tt_str_op(ec->socks_request->address, OP_EQ, "abcdefghijklmnop.onion");

  /* Okay, resolve foo.onion */
  strlcpy(ec2->socks_request->address, "foo.onion",
          sizeof(ec2->socks_request->address));
  ec2->socks_request->command = SOCKS_COMMAND_RESOLVE;
  connection_ap_handshake_rewrite(ec2, &rr);

  tt_int_op(rr.automap, OP_EQ, 1);
  tt_int_op(rr.should_close, OP_EQ, 0);
  tt_int_op(rr.end_reason, OP_EQ, 0);
  tt_i64_op(rr.map_expires, OP_EQ, TIME_MAX);
  tt_int_op(rr.exit_source, OP_EQ, ADDRMAPSRC_NONE);
  tt_str_op(rr.orig_address, OP_EQ, "foo.onion");
  tt_assert(!strcmpstart(ec2->socks_request->address, "192.168."));

  /* Now connect */
  strlcpy(ec3->socks_request->address, ec2->socks_request->address,
          sizeof(ec3->socks_request->address));
  ec3->socks_request->command = SOCKS_COMMAND_CONNECT;
  connection_ap_handshake_rewrite(ec3, &rr);
  tt_int_op(rr.automap, OP_EQ, 0);
  tt_int_op(rr.should_close, OP_EQ, 0);
  tt_int_op(rr.end_reason, OP_EQ, 0);
  tt_assert(!strcmpstart(ec3->socks_request->address,
                         "abcdefghijklmnop.onion"));

  /* Now resolve abcefghijklmnop.onion. */
  strlcpy(ec4->socks_request->address, "abcdefghijklmnop.onion",
          sizeof(ec4->socks_request->address));
  ec4->socks_request->command = SOCKS_COMMAND_RESOLVE;
  connection_ap_handshake_rewrite(ec4, &rr);

  tt_int_op(rr.automap, OP_EQ, 1);
  tt_int_op(rr.should_close, OP_EQ, 0);
  tt_int_op(rr.end_reason, OP_EQ, 0);
  tt_i64_op(rr.map_expires, OP_EQ, TIME_MAX);
  tt_int_op(rr.exit_source, OP_EQ, ADDRMAPSRC_NONE);
  tt_str_op(rr.orig_address, OP_EQ, "abcdefghijklmnop.onion");
  tt_assert(!strcmpstart(ec4->socks_request->address, "192.168."));
  /* XXXX doesn't work
   tt_str_op(ec4->socks_request->address, OP_EQ, ec2->socks_request->address);
  */

 done:
  connection_free_minimal(ENTRY_TO_CONN(ec2));
  connection_free_minimal(ENTRY_TO_CONN(ec3));
  connection_free_minimal(ENTRY_TO_CONN(ec4));
}

static void
test_entryconn_rewrite_mapaddress_automap_onion_common(entry_connection_t *ec,
                                                       int map_to_onion,
                                                       int map_to_address)
{
  entry_connection_t *ec2 = NULL;
  entry_connection_t *ec3 = NULL;
  rewrite_result_t rr;

  ec2 = entry_connection_new(CONN_TYPE_AP, AF_INET);
  ec3 = entry_connection_new(CONN_TYPE_AP, AF_INET);

  /* Connect to irc.example.com */
  strlcpy(ec->socks_request->address, "irc.example.com",
          sizeof(ec->socks_request->address));
  ec->socks_request->command = SOCKS_COMMAND_CONNECT;
  connection_ap_handshake_rewrite(ec, &rr);

  tt_int_op(rr.automap, OP_EQ, 0);
  tt_int_op(rr.should_close, OP_EQ, 0);
  tt_int_op(rr.end_reason, OP_EQ, 0);
  tt_i64_op(rr.map_expires, OP_EQ, TIME_MAX);
  tt_int_op(rr.exit_source, OP_EQ, ADDRMAPSRC_NONE);
  tt_str_op(rr.orig_address, OP_EQ, "irc.example.com");
  tt_str_op(ec->socks_request->address, OP_EQ,
            map_to_onion ? "abcdefghijklmnop.onion" : "irc.example.com");

  /* Okay, resolve irc.example.com */
  strlcpy(ec2->socks_request->address, "irc.example.com",
          sizeof(ec2->socks_request->address));
  ec2->socks_request->command = SOCKS_COMMAND_RESOLVE;
  connection_ap_handshake_rewrite(ec2, &rr);

  tt_int_op(rr.automap, OP_EQ, map_to_onion && map_to_address);
  tt_int_op(rr.should_close, OP_EQ, 0);
  tt_int_op(rr.end_reason, OP_EQ, 0);
  tt_i64_op(rr.map_expires, OP_EQ, TIME_MAX);
  tt_int_op(rr.exit_source, OP_EQ, ADDRMAPSRC_NONE);
  tt_str_op(rr.orig_address, OP_EQ, "irc.example.com");
  if (map_to_onion && map_to_address)
    tt_assert(!strcmpstart(ec2->socks_request->address, "192.168."));

  /* Now connect */
  strlcpy(ec3->socks_request->address, ec2->socks_request->address,
          sizeof(ec3->socks_request->address));
  ec3->socks_request->command = SOCKS_COMMAND_CONNECT;
  connection_ap_handshake_rewrite(ec3, &rr);
  tt_int_op(rr.automap, OP_EQ, 0);
  tt_int_op(rr.should_close, OP_EQ, 0);
  tt_int_op(rr.end_reason, OP_EQ, 0);
  if (map_to_onion)
    tt_assert(!strcmpstart(ec3->socks_request->address,
                           "abcdefghijklmnop.onion"));

 done:
  connection_free_minimal(ENTRY_TO_CONN(ec2));
  connection_free_minimal(ENTRY_TO_CONN(ec3));
}

/* This time is the same, but we start with a mapping from a non-onion
 * address. */
static void
test_entryconn_rewrite_mapaddress_automap_onion2(void *arg)
{
  char *msg = NULL;
  get_options_mutable()->AutomapHostsOnResolve = 1;
  smartlist_add_strdup(get_options_mutable()->AutomapHostsSuffixes,
                ".onion");
  parse_virtual_addr_network("192.168.0.0/16", AF_INET, 0, &msg);
  config_line_append(&get_options_mutable()->AddressMap,
                     "MapAddress", "irc.example.com abcdefghijklmnop.onion");
  config_register_addressmaps(get_options());

  test_entryconn_rewrite_mapaddress_automap_onion_common(arg, 1, 1);
}

/* Same as above, with automapped turned off */
static void
test_entryconn_rewrite_mapaddress_automap_onion3(void *arg)
{
  config_line_append(&get_options_mutable()->AddressMap,
                     "MapAddress", "irc.example.com abcdefghijklmnop.onion");
  config_register_addressmaps(get_options());

  test_entryconn_rewrite_mapaddress_automap_onion_common(arg, 1, 0);
}

/* As above, with no mapping. */
static void
test_entryconn_rewrite_mapaddress_automap_onion4(void *arg)
{
  char *msg = NULL;
  get_options_mutable()->AutomapHostsOnResolve = 1;
  smartlist_add_strdup(get_options_mutable()->AutomapHostsSuffixes,
                ".onion");
  parse_virtual_addr_network("192.168.0.0/16", AF_INET, 0, &msg);

  test_entryconn_rewrite_mapaddress_automap_onion_common(arg, 0, 1);
}

/** Test that rewrite functions can handle v3 onion addresses */
static void
test_entryconn_rewrite_onion_v3(void *arg)
{
  int retval;
  entry_connection_t *conn = arg;

  (void) arg;

  hs_cache_init();

  /* Make a SOCKS request */
  conn->socks_request->command = SOCKS_COMMAND_CONNECT;
  strlcpy(conn->socks_request->address,
          "git.25njqamcweflpvkl73j4szahhihoc4xt3ktcgjnpaingr5yhkenl5sid.onion",
          sizeof(conn->socks_request->address));

  /* Make an onion connection using the SOCKS request */
  conn->entry_cfg.onion_traffic = 1;
  ENTRY_TO_CONN(conn)->state = AP_CONN_STATE_SOCKS_WAIT;
  tt_assert(!ENTRY_TO_EDGE_CONN(conn)->hs_ident);

  /* Handle SOCKS and rewrite! */
  retval = connection_ap_handshake_rewrite_and_attach(conn, NULL, NULL);
  tt_int_op(retval, OP_EQ, 0);

  /* Check connection state after rewrite. It should be in waiting for
   * descriptor state. */
  tt_int_op(ENTRY_TO_CONN(conn)->state, OP_EQ, AP_CONN_STATE_RENDDESC_WAIT);
  /* check that the address got rewritten */
  tt_str_op(conn->socks_request->address, OP_EQ,
            "25njqamcweflpvkl73j4szahhihoc4xt3ktcgjnpaingr5yhkenl5sid");
  /* check that HS information got attached to the connection */
  tt_assert(ENTRY_TO_EDGE_CONN(conn)->hs_ident);

 done:
  hs_free_all();
  /* 'conn' is cleaned by handler */
}

#define REWRITE(name)                           \
  { #name, test_entryconn_##name, TT_FORK, &test_rewrite_setup, NULL }

struct testcase_t entryconn_tests[] = {
  REWRITE(rewrite_basic),
  REWRITE(rewrite_bad_dotexit),
  REWRITE(rewrite_automap_ipv4),
  REWRITE(rewrite_automap_ipv6),
  // REWRITE(rewrite_automap_reverse),
  REWRITE(rewrite_cached_dns_ipv4),
  REWRITE(rewrite_cached_dns_ipv6),
  REWRITE(rewrite_unmapped_virtual),
  REWRITE(rewrite_mapaddress),
  REWRITE(rewrite_reject_internal_reverse),
  REWRITE(rewrite_automap_exit),
  REWRITE(rewrite_mapaddress_exit),
  REWRITE(rewrite_mapaddress_automap_onion),
  REWRITE(rewrite_mapaddress_automap_onion2),
  REWRITE(rewrite_mapaddress_automap_onion3),
  REWRITE(rewrite_mapaddress_automap_onion4),
  REWRITE(rewrite_onion_v3),

  END_OF_TESTCASES
};
