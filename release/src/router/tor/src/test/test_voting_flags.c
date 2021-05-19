/* Copyright (c) 2018-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

#include "orconfig.h"

#define VOTEFLAGS_PRIVATE

#include "core/or/or.h"

#include "feature/dirauth/voteflags.h"
#include "feature/dirauth/dirauth_options_st.h"
#include "feature/nodelist/node_st.h"
#include "feature/nodelist/routerstatus_st.h"
#include "feature/nodelist/routerinfo_st.h"

#include "app/config/config.h"

#include "test/test.h"
#include "test/opts_test_helpers.h"

typedef struct {
  time_t now;
  routerinfo_t ri;
  node_t node;

  routerstatus_t expected;
} flag_vote_test_cfg_t;

static void
setup_cfg(flag_vote_test_cfg_t *c)
{
  memset(c, 0, sizeof(*c));

  c->now = approx_time();

  c->ri.nickname = (char *) "testing100";
  strlcpy(c->expected.nickname, "testing100", sizeof(c->expected.nickname));

  memset(c->ri.cache_info.identity_digest, 0xff, DIGEST_LEN);
  memset(c->ri.cache_info.signed_descriptor_digest, 0xee, DIGEST_LEN);

  c->ri.cache_info.published_on = c->now - 100;
  c->expected.published_on = c->now - 100;

  tor_addr_from_ipv4h(&c->ri.ipv4_addr, 0x7f010105);
  tor_addr_from_ipv4h(&c->expected.ipv4_addr, 0x7f010105);
  c->ri.ipv4_orport = 9090;
  c->expected.ipv4_orport = 9090;

  tor_addr_make_null(&c->ri.ipv6_addr, AF_INET6);
  tor_addr_make_null(&c->expected.ipv6_addr, AF_INET6);

  // By default we have no loaded information about stability or speed,
  // so we'll default to voting "yeah sure." on these two.
  c->expected.is_fast = 1;
  c->expected.is_stable = 1;
}

static bool
check_result(flag_vote_test_cfg_t *c)
{
  bool result = false;
  routerstatus_t rs;
  memset(&rs, 0, sizeof(rs));
  dirauth_set_routerstatus_from_routerinfo(&rs, &c->node, &c->ri, c->now, 0);

  tt_i64_op(rs.published_on, OP_EQ, c->expected.published_on);
  tt_str_op(rs.nickname, OP_EQ, c->expected.nickname);

  // identity_digest and descriptor_digest are not set here.

  tt_assert(tor_addr_eq(&rs.ipv4_addr, &c->expected.ipv4_addr));
  tt_uint_op(rs.ipv4_orport, OP_EQ, c->expected.ipv4_orport);
  tt_uint_op(rs.ipv4_dirport, OP_EQ, c->expected.ipv4_dirport);

  tt_assert(tor_addr_eq(&rs.ipv6_addr, &c->expected.ipv6_addr));
  tt_uint_op(rs.ipv6_orport, OP_EQ, c->expected.ipv6_orport);

#define FLAG(flagname) \
  tt_uint_op(rs.flagname, OP_EQ, c->expected.flagname)

  FLAG(is_authority);
  FLAG(is_exit);
  FLAG(is_stable);
  FLAG(is_fast);
  FLAG(is_flagged_running);
  FLAG(is_named);
  FLAG(is_unnamed);
  FLAG(is_valid);
  FLAG(is_possible_guard);
  FLAG(is_bad_exit);
  FLAG(is_hs_dir);
  FLAG(is_v2_dir);
  FLAG(is_staledesc);
  FLAG(has_bandwidth);
  FLAG(has_exitsummary);
  FLAG(bw_is_unmeasured);

  result = true;

 done:
  return result;
}

static void
test_voting_flags_minimal(void *arg)
{
  flag_vote_test_cfg_t *cfg = arg;
  (void) check_result(cfg);
}

static void
test_voting_flags_ipv6(void *arg)
{
  flag_vote_test_cfg_t *cfg = arg;

  tt_assert(tor_addr_parse(&cfg->ri.ipv6_addr, "f00::b42") == AF_INET6);
  cfg->ri.ipv6_orport = 9091;
  // no change in expected results, since we aren't set up with ipv6
  // connectivity.
  if (!check_result(cfg))
    goto done;

  get_dirauth_options(get_options_mutable())->AuthDirHasIPv6Connectivity = 1;
  // no change in expected results, since last_reachable6 won't be set.
  if (!check_result(cfg))
    goto done;

  cfg->node.last_reachable6 = cfg->now - 10;
  // now that lastreachable6 is set, we expect to see the result.
  tt_assert(tor_addr_parse(&cfg->expected.ipv6_addr, "f00::b42") == AF_INET6);
  cfg->expected.ipv6_orport = 9091;
  if (!check_result(cfg))
    goto done;
 done:
  ;
}

static void
test_voting_flags_staledesc(void *arg)
{
  flag_vote_test_cfg_t *cfg = arg;
  time_t now = cfg->now;

  cfg->ri.cache_info.published_on = now - DESC_IS_STALE_INTERVAL + 10;
  cfg->expected.published_on = now - DESC_IS_STALE_INTERVAL + 10;
  // no change in expectations for is_staledesc
  if (!check_result(cfg))
    goto done;

  cfg->ri.cache_info.published_on = now - DESC_IS_STALE_INTERVAL - 10;
  cfg->expected.published_on = now - DESC_IS_STALE_INTERVAL - 10;
  cfg->expected.is_staledesc = 1;
  if (!check_result(cfg))
    goto done;

 done:
  ;
}

static void *
setup_voting_flags_test(const struct testcase_t *testcase)
{
  (void)testcase;
  flag_vote_test_cfg_t *cfg = tor_malloc_zero(sizeof(*cfg));
  setup_cfg(cfg);
  return cfg;
}

static int
teardown_voting_flags_test(const struct testcase_t *testcase, void *arg)
{
  (void)testcase;
  flag_vote_test_cfg_t *cfg = arg;
  tor_free(cfg);
  return 1;
}

static const struct testcase_setup_t voting_flags_setup = {
  .setup_fn = setup_voting_flags_test,
  .cleanup_fn = teardown_voting_flags_test,
};

#define T(name,flags)                                   \
  { #name, test_voting_flags_##name, (flags), &voting_flags_setup, NULL }

struct testcase_t voting_flags_tests[] = {
  T(minimal, 0),
  T(ipv6, TT_FORK),
  // TODO: Add more of these tests.
  T(staledesc, TT_FORK),
  END_OF_TESTCASES
};
