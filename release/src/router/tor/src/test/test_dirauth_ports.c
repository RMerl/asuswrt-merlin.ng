/* Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

#include "orconfig.h"
#define CONFIG_PRIVATE

#include "core/or/or.h"
#include "feature/dirclient/dir_server_st.h"
#include "feature/nodelist/dirlist.h"
#include "app/config/config.h"
#include "test/test.h"
#include "test/log_test_helpers.h"

static void
test_dirauth_port_parsing(void *arg)
{
  (void)arg;

  // This one is okay.
  int rv = parse_dir_authority_line(
    "moria1 orport=9101 "
    "v3ident=D586D18309DED4CD6D57C18FDB97EFA96D330566 "
    "upload=http://128.31.0.39:9131/ "
    "download=http://128.31.0.39:9131 "
    "vote=http://128.31.0.39:9131/ "
    "128.31.0.39:9131 9695 DFC3 5FFE B861 329B 9F1A B04C 4639 7020 CE31",
    NO_DIRINFO, 1);
  tt_int_op(rv,OP_EQ,0);

  // These have bad syntax.
  setup_capture_of_logs(LOG_WARN);
  rv = parse_dir_authority_line(
    "moria1 orport=9101 "
    "v3ident=D586D18309DED4CD6D57C18FDB97EFA96D330566 "
    "uploadx=http://128.31.0.39:9131/ "
    "128.31.0.39:9131 9695 DFC3 5FFE B861 329B 9F1A B04C 4639 7020 CE31",
    NO_DIRINFO, 1);
  tt_int_op(rv,OP_EQ,0);
  expect_log_msg_containing("Unrecognized flag");
  mock_clean_saved_logs();

  rv = parse_dir_authority_line(
    "moria1 orport=9101 "
    "v3ident=D586D18309DED4CD6D57C18FDB97EFA96D330566 "
    "upload=https://128.31.0.39:9131/ " // https is not recognized
    "128.31.0.39:9131 9695 DFC3 5FFE B861 329B 9F1A B04C 4639 7020 CE31",
    NO_DIRINFO, 1);
  tt_int_op(rv,OP_EQ,-1);
  expect_log_msg_containing("Unsupported URL scheme");
  mock_clean_saved_logs();

  rv = parse_dir_authority_line(
    "moria1 orport=9101 "
    "v3ident=D586D18309DED4CD6D57C18FDB97EFA96D330566 "
    "upload=http://128.31.0.39:9131/tor " // suffix is not supported
    "128.31.0.39:9131 9695 DFC3 5FFE B861 329B 9F1A B04C 4639 7020 CE31",
    NO_DIRINFO, 1);
  tt_int_op(rv,OP_EQ,-1);
  expect_log_msg_containing("Unsupported URL prefix");
  mock_clean_saved_logs();

  rv = parse_dir_authority_line(
    "moria1 orport=9101 "
    "v3ident=D586D18309DED4CD6D57C18FDB97EFA96D330566 "
    "upload=http://128.31.0.256:9131/ " // "256" is not ipv4.
    "128.31.0.39:9131 9695 DFC3 5FFE B861 329B 9F1A B04C 4639 7020 CE31",
    NO_DIRINFO, 1);
  tt_int_op(rv,OP_EQ,-1);
  expect_log_msg_containing("Unable to parse address");

  rv = parse_dir_authority_line(
    "moria1 orport=9101 "
    "v3ident=D586D18309DED4CD6D57C18FDB97EFA96D330566 "
    "upload=http://xyz.example.com/ " // hostnames not supported.
    "128.31.0.39:9131 9695 DFC3 5FFE B861 329B 9F1A B04C 4639 7020 CE31",
    NO_DIRINFO, 1);
  tt_int_op(rv,OP_EQ,-1);
  expect_log_msg_containing("Unable to parse address");

 done:
  teardown_capture_of_logs();
}

static void
test_dirauth_port_lookup(void *arg)
{
  (void)arg;

  clear_dir_servers();

  int rv = parse_dir_authority_line(
    "moria1 orport=9101 "
    "v3ident=D586D18309DED4CD6D57C18FDB97EFA96D330566 "
    "upload=http://128.31.0.40:9132/ "
    "download=http://128.31.0.41:9133 "
    "vote=http://128.31.0.42:9134/ "
    "128.31.0.39:9131 9695 DFC3 5FFE B861 329B 9F1A B04C 4639 7020 CE31",
    NO_DIRINFO, 0);
  tt_int_op(rv,OP_EQ,0);

  rv = parse_dir_authority_line(
    "morgoth orport=9101 "
    "v3ident=D586D18309DED4CDFFFFFFFFDB97EFA96D330566 "
    "upload=http://128.31.0.43:9140/ "
    "128.31.0.44:9131 9695 DFC3 5FFE B861 329B 9F1A B04C 4639 7020 CE31",
    NO_DIRINFO, 0);
  tt_int_op(rv,OP_EQ,0);

  const smartlist_t *servers = router_get_trusted_dir_servers();
  tt_assert(servers);
  tt_int_op(smartlist_len(servers), OP_EQ, 2);
  const dir_server_t *moria = smartlist_get(servers, 0);
  const dir_server_t *morgoth = smartlist_get(servers, 1);
  tt_str_op(moria->nickname, OP_EQ, "moria1");
  tt_str_op(morgoth->nickname, OP_EQ, "morgoth");

  const tor_addr_port_t *dirport;

  dirport = trusted_dir_server_get_dirport(moria,
                                           AUTH_USAGE_UPLOAD, AF_INET);
  tt_int_op(dirport->port, OP_EQ, 9132);
  dirport = trusted_dir_server_get_dirport(moria,
                                           AUTH_USAGE_DOWNLOAD, AF_INET);
  tt_int_op(dirport->port, OP_EQ, 9133);
  dirport = trusted_dir_server_get_dirport(moria,
                                           AUTH_USAGE_VOTING, AF_INET);
  tt_int_op(dirport->port, OP_EQ, 9134);

  dirport = trusted_dir_server_get_dirport(morgoth,
                                           AUTH_USAGE_UPLOAD, AF_INET);
  tt_int_op(dirport->port, OP_EQ, 9140);
  dirport = trusted_dir_server_get_dirport(morgoth,
                                           AUTH_USAGE_DOWNLOAD, AF_INET);
  tt_int_op(dirport->port, OP_EQ, 9131); // fallback
  dirport = trusted_dir_server_get_dirport(morgoth,
                                           AUTH_USAGE_VOTING, AF_INET);
  tt_int_op(dirport->port, OP_EQ, 9131); // fallback

 done:
  ;
}

#define T(name) \
  { #name, test_dirauth_port_ ## name, TT_FORK, NULL, NULL }

struct testcase_t dirauth_port_tests[] = {
  T(parsing),
  T(lookup),
  END_OF_TESTCASES
};
