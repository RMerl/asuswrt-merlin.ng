/* Copyright (c) 2018-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file test_bwmgt.c
 * \brief tests for bandwidth management / token bucket functions
 */

#define CONFIG_PRIVATE
#define CONNECTION_PRIVATE
#define DIRAUTH_SYS_PRIVATE
#define TOKEN_BUCKET_PRIVATE

#include "core/or/or.h"

#include "app/config/config.h"
#include "core/mainloop/connection.h"
#include "feature/dirauth/dirauth_sys.h"
#include "feature/dircommon/directory.h"
#include "feature/nodelist/microdesc.h"
#include "feature/nodelist/networkstatus.h"
#include "feature/nodelist/nodelist.h"
#include "feature/nodelist/routerlist.h"
#include "lib/crypt_ops/crypto_rand.h"
#include "lib/evloop/token_bucket.h"
#include "test/test.h"
#include "test/test_helpers.h"

#include "app/config/or_options_st.h"
#include "core/or/connection_st.h"
#include "feature/dirauth/dirauth_options_st.h"
#include "feature/nodelist/microdesc_st.h"
#include "feature/nodelist/networkstatus_st.h"
#include "feature/nodelist/routerinfo_st.h"
#include "feature/nodelist/routerstatus_st.h"

// an imaginary time, in timestamp units. Chosen so it will roll over.
static const uint32_t START_TS = UINT32_MAX-10;
static const int32_t KB = 1024;
static const uint32_t GB = (UINT64_C(1) << 30);

static or_options_t mock_options;

static const or_options_t *
mock_get_options(void)
{
  return &mock_options;
}

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

static void
test_bwmgt_token_buf_init(void *arg)
{
  (void)arg;
  token_bucket_rw_t b;

  token_bucket_rw_init(&b, 16*KB, 64*KB, START_TS);
  // Burst is correct
  tt_uint_op(b.cfg.burst, OP_EQ, 64*KB);
  // Rate is correct, within 1 percent.
  {
    uint32_t ticks_per_sec =
      (uint32_t) monotime_msec_to_approx_coarse_stamp_units(1000);
    uint32_t rate_per_sec = (b.cfg.rate * ticks_per_sec / TICKS_PER_STEP);

    tt_uint_op(rate_per_sec, OP_GT, 16*KB-160);
    tt_uint_op(rate_per_sec, OP_LT, 16*KB+160);
  }
  // Bucket starts out full:
  tt_uint_op(b.last_refilled_at_timestamp, OP_EQ, START_TS);
  tt_int_op(b.read_bucket.bucket, OP_EQ, 64*KB);

 done:
  ;
}

static void
test_bwmgt_token_buf_adjust(void *arg)
{
  (void)arg;
  token_bucket_rw_t b;

  token_bucket_rw_init(&b, 16*KB, 64*KB, START_TS);

  uint32_t rate_orig = b.cfg.rate;
  // Increasing burst
  token_bucket_rw_adjust(&b, 16*KB, 128*KB);
  tt_uint_op(b.cfg.rate, OP_EQ, rate_orig);
  tt_uint_op(b.read_bucket.bucket, OP_EQ, 64*KB);
  tt_uint_op(b.cfg.burst, OP_EQ, 128*KB);

  // Decreasing burst but staying above bucket
  token_bucket_rw_adjust(&b, 16*KB, 96*KB);
  tt_uint_op(b.cfg.rate, OP_EQ, rate_orig);
  tt_uint_op(b.read_bucket.bucket, OP_EQ, 64*KB);
  tt_uint_op(b.cfg.burst, OP_EQ, 96*KB);

  // Decreasing burst below bucket,
  token_bucket_rw_adjust(&b, 16*KB, 48*KB);
  tt_uint_op(b.cfg.rate, OP_EQ, rate_orig);
  tt_uint_op(b.read_bucket.bucket, OP_EQ, 48*KB);
  tt_uint_op(b.cfg.burst, OP_EQ, 48*KB);

  // Changing rate.
  token_bucket_rw_adjust(&b, 32*KB, 48*KB);
  tt_uint_op(b.cfg.rate, OP_GE, rate_orig*2 - 10);
  tt_uint_op(b.cfg.rate, OP_LE, rate_orig*2 + 10);
  tt_uint_op(b.read_bucket.bucket, OP_EQ, 48*KB);
  tt_uint_op(b.cfg.burst, OP_EQ, 48*KB);

 done:
  ;
}

static void
test_bwmgt_token_buf_dec(void *arg)
{
  (void)arg;
  token_bucket_rw_t b;
  token_bucket_rw_init(&b, 16*KB, 64*KB, START_TS);

  // full-to-not-full.
  tt_int_op(0, OP_EQ, token_bucket_rw_dec_read(&b, KB));
  tt_int_op(b.read_bucket.bucket, OP_EQ, 63*KB);

  // Full to almost-not-full
  tt_int_op(0, OP_EQ, token_bucket_rw_dec_read(&b, 63*KB - 1));
  tt_int_op(b.read_bucket.bucket, OP_EQ, 1);

  // almost-not-full to empty.
  tt_int_op(1, OP_EQ, token_bucket_rw_dec_read(&b, 1));
  tt_int_op(b.read_bucket.bucket, OP_EQ, 0);

  // reset bucket, try full-to-empty
  token_bucket_rw_init(&b, 16*KB, 64*KB, START_TS);
  tt_int_op(1, OP_EQ, token_bucket_rw_dec_read(&b, 64*KB));
  tt_int_op(b.read_bucket.bucket, OP_EQ, 0);

  // reset bucket, try underflow.
  token_bucket_rw_init(&b, 16*KB, 64*KB, START_TS);
  tt_int_op(1, OP_EQ, token_bucket_rw_dec_read(&b, 64*KB + 1));
  tt_int_op(b.read_bucket.bucket, OP_EQ, -1);

  // A second underflow does not make the bucket empty.
  tt_int_op(0, OP_EQ, token_bucket_rw_dec_read(&b, 1000));
  tt_int_op(b.read_bucket.bucket, OP_EQ, -1001);

 done:
  ;
}

static void
test_bwmgt_token_buf_refill(void *arg)
{
  (void)arg;
  token_bucket_rw_t b;
  const uint32_t BW_SEC =
    (uint32_t)monotime_msec_to_approx_coarse_stamp_units(1000);
  token_bucket_rw_init(&b, 16*KB, 64*KB, START_TS);

  /* Make the buffer much emptier, then let one second elapse. */
  token_bucket_rw_dec_read(&b, 48*KB);
  tt_int_op(b.read_bucket.bucket, OP_EQ, 16*KB);
  tt_int_op(0, OP_EQ, token_bucket_rw_refill(&b, START_TS + BW_SEC));
  tt_int_op(b.read_bucket.bucket, OP_GT, 32*KB - 300);
  tt_int_op(b.read_bucket.bucket, OP_LT, 32*KB + 300);

  /* Another half second. */
  tt_int_op(0, OP_EQ, token_bucket_rw_refill(&b, START_TS + BW_SEC*3/2));
  tt_int_op(b.read_bucket.bucket, OP_GT, 40*KB - 400);
  tt_int_op(b.read_bucket.bucket, OP_LT, 40*KB + 400);
  tt_uint_op(b.last_refilled_at_timestamp, OP_EQ, START_TS + BW_SEC*3/2);

  /* No time: nothing happens. */
  {
    const uint32_t bucket_orig = b.read_bucket.bucket;
    tt_int_op(0, OP_EQ, token_bucket_rw_refill(&b, START_TS + BW_SEC*3/2));
    tt_int_op(b.read_bucket.bucket, OP_EQ, bucket_orig);
  }

  /* Another 30 seconds: fill the bucket. */
  tt_int_op(0, OP_EQ, token_bucket_rw_refill(&b,
                                     START_TS + BW_SEC*3/2 + BW_SEC*30));
  tt_int_op(b.read_bucket.bucket, OP_EQ, b.cfg.burst);
  tt_uint_op(b.last_refilled_at_timestamp, OP_EQ,
             START_TS + BW_SEC*3/2 + BW_SEC*30);

  /* Another 30 seconds: nothing happens. */
  tt_int_op(0, OP_EQ, token_bucket_rw_refill(&b,
                                     START_TS + BW_SEC*3/2 + BW_SEC*60));
  tt_int_op(b.read_bucket.bucket, OP_EQ, b.cfg.burst);
  tt_uint_op(b.last_refilled_at_timestamp, OP_EQ,
             START_TS + BW_SEC*3/2 + BW_SEC*60);

  /* Empty the bucket, let two seconds pass, and make sure that a refill is
   * noticed. */
  tt_int_op(1, OP_EQ, token_bucket_rw_dec_read(&b, b.cfg.burst));
  tt_int_op(0, OP_EQ, b.read_bucket.bucket);
  tt_int_op(1, OP_EQ, token_bucket_rw_refill(&b,
                                     START_TS + BW_SEC*3/2 + BW_SEC*61));
  tt_int_op(0, OP_EQ, token_bucket_rw_refill(&b,
                                     START_TS + BW_SEC*3/2 + BW_SEC*62));
  tt_int_op(b.read_bucket.bucket, OP_GT, 32*KB-400);
  tt_int_op(b.read_bucket.bucket, OP_LT, 32*KB+400);

  /* Underflow the bucket, make sure we detect when it has tokens again. */
  tt_int_op(1, OP_EQ,
            token_bucket_rw_dec_read(&b, b.read_bucket.bucket+16*KB));
  tt_int_op(-16*KB, OP_EQ, b.read_bucket.bucket);
  // half a second passes...
  tt_int_op(0, OP_EQ, token_bucket_rw_refill(&b, START_TS + BW_SEC*64));
  tt_int_op(b.read_bucket.bucket, OP_GT, -8*KB-300);
  tt_int_op(b.read_bucket.bucket, OP_LT, -8*KB+300);
  // a second passes
  tt_int_op(1, OP_EQ, token_bucket_rw_refill(&b, START_TS + BW_SEC*65));
  tt_int_op(b.read_bucket.bucket, OP_GT, 8*KB-400);
  tt_int_op(b.read_bucket.bucket, OP_LT, 8*KB+400);

  // We step a second backwards, and nothing happens.
  tt_int_op(0, OP_EQ, token_bucket_rw_refill(&b, START_TS + BW_SEC*64));
  tt_int_op(b.read_bucket.bucket, OP_GT, 8*KB-400);
  tt_int_op(b.read_bucket.bucket, OP_LT, 8*KB+400);

  /* A large amount of time passes, but less than the threshold at which
   * we start detecting an assumed rollover event. This might be about 20
   * days on a system with stamp units equal to 1ms. */
  uint32_t ts_stamp = START_TS + UINT32_MAX / 5;
  tt_int_op(0, OP_EQ, token_bucket_rw_refill(&b, ts_stamp));
  tt_int_op(b.read_bucket.bucket, OP_EQ, b.cfg.burst);

  /* Fully empty the bucket and make sure it's filling once again */
  token_bucket_rw_dec_read(&b, b.cfg.burst);
  tt_int_op(b.read_bucket.bucket, OP_EQ, 0);
  tt_int_op(1, OP_EQ, token_bucket_rw_refill(&b, ts_stamp += BW_SEC));
  tt_int_op(b.read_bucket.bucket, OP_GT, 16*KB - 300);
  tt_int_op(b.read_bucket.bucket, OP_LT, 16*KB + 300);

  /* An even larger amount of time passes, which we take to be a 32-bit
   * rollover event. The individual update is ignored, but the timestamp
   * is still updated and the very next update should be accounted properly. */
  tt_int_op(0, OP_EQ, token_bucket_rw_refill(&b, ts_stamp += UINT32_MAX/2));
  tt_int_op(b.read_bucket.bucket, OP_GT, 16*KB - 600);
  tt_int_op(b.read_bucket.bucket, OP_LT, 16*KB + 600);
  tt_int_op(0, OP_EQ, token_bucket_rw_refill(&b, ts_stamp += BW_SEC));
  tt_int_op(b.read_bucket.bucket, OP_GT, 32*KB - 600);
  tt_int_op(b.read_bucket.bucket, OP_LT, 32*KB + 600);

 done:
  ;
}

/* Test some helper functions we use within the token bucket interface. */
static void
test_bwmgt_token_buf_helpers(void *arg)
{
  uint32_t ret;

  (void) arg;

  /* The returned value will be OS specific but in any case, it should be
   * greater than 1 since we are passing 1GB/sec rate. */
  ret = rate_per_sec_to_rate_per_step(1 * GB);
  tt_u64_op(ret, OP_GT, 1);

  /* We default to 1 in case rate is 0. */
  ret = rate_per_sec_to_rate_per_step(0);
  tt_u64_op(ret, OP_EQ, 1);

 done:
  ;
}

static void
test_bwmgt_dir_conn_global_write_low(void *arg)
{
  bool ret;
  int addr_family;
  connection_t *conn = NULL;
  routerstatus_t *rs = NULL; microdesc_t *md = NULL; routerinfo_t *ri = NULL;
  tor_addr_t relay_addr;
  dirauth_options_t *dirauth_opts = NULL;

  (void) arg;

  memset(&mock_options, 0, sizeof(or_options_t));
  MOCK(networkstatus_get_latest_consensus,
       mock_networkstatus_get_latest_consensus);
  MOCK(networkstatus_get_latest_consensus_by_flavor,
       mock_networkstatus_get_latest_consensus_by_flavor);
  MOCK(get_estimated_address_per_node,
       mock_get_estimated_address_per_node);

  /*
   * The following is rather complex but that is what it takes to add a dummy
   * consensus with a valid routerlist which will populate our node address
   * set that we need to lookup to test the known relay code path.
   *
   * We MUST do that before we MOCK(get_options) else it is another world of
   * complexity.
   */

  /* This will be the address of our relay. */
  tor_addr_parse(&relay_addr, "1.2.3.4");

  /* We'll now add a relay into our routerlist and see if we let it. */
  dummy_ns = tor_malloc_zero(sizeof(*dummy_ns));
  dummy_ns->flavor = FLAV_MICRODESC;
  dummy_ns->routerstatus_list = smartlist_new();

  md = tor_malloc_zero(sizeof(*md));
  ri = tor_malloc_zero(sizeof(*ri));
  rs = tor_malloc_zero(sizeof(*rs));
  crypto_rand(rs->identity_digest, sizeof(rs->identity_digest));
  crypto_rand(md->digest, sizeof(md->digest));
  memcpy(rs->descriptor_digest, md->digest, DIGEST256_LEN);

  /* Set IP address. */
  tor_addr_copy(&rs->ipv4_addr, &relay_addr);
  tor_addr_copy(&ri->ipv4_addr, &rs->ipv4_addr);
  /* Add the rs to the consensus becoming a node_t. */
  smartlist_add(dummy_ns->routerstatus_list, rs);

  /* Add all configured authorities (hardcoded) before we set the consensus so
   * the address set exists. */
  ret = consider_adding_dir_servers(&mock_options, &mock_options);
  tt_int_op(ret, OP_EQ, 0);

  /* This will make the nodelist bloom filter very large
   * (the_nodelist->node_addrs) so we will fail the contain test rarely. */
  addr_per_node = 1024;

  nodelist_set_consensus(dummy_ns);

  dirauth_opts = tor_malloc_zero(sizeof(dirauth_options_t));
  dirauth_opts->AuthDirRejectRequestsUnderLoad = 0;
  dirauth_set_options(dirauth_opts);

  /* Ok, now time to control which options we use. */
  MOCK(get_options, mock_get_options);

  /* Set ourselves as an authoritative dir. */
  mock_options.AuthoritativeDir = 1;
  mock_options.V3AuthoritativeDir = 1;
  mock_options.UseDefaultFallbackDirs = 0;

  /* This will set our global bucket to 1 byte and thus we will hit the
   * banwdith limit in our test. */
  mock_options.BandwidthRate = 1;
  mock_options.BandwidthBurst = 1;

  /* Else an IPv4 address screams. */
  mock_options.ClientUseIPv4 = 1;
  mock_options.ClientUseIPv6 = 1;

  /* Initialize the global buckets. */
  connection_bucket_init();

  /* The address "127.0.0.1" is set with this helper. */
  conn = test_conn_get_connection(DIR_CONN_STATE_MIN_, CONN_TYPE_DIR,
                                  DIR_PURPOSE_MIN_);
  tt_assert(conn);

  /* First try a non authority non relay IP thus a client but we are not
   * configured to reject requests under load so we should get a false value
   * that our limit is _not_ low. */
  addr_family = tor_addr_parse(&conn->addr, "1.1.1.1");
  tt_int_op(addr_family, OP_EQ, AF_INET);
  ret = connection_dir_is_global_write_low(conn, INT_MAX);
  tt_int_op(ret, OP_EQ, 0);

  /* Now, we will reject requests under load so try again a non authority non
   * relay IP thus a client. We should get a warning that our limit is too
   * low. */
  dirauth_opts->AuthDirRejectRequestsUnderLoad = 1;

  addr_family = tor_addr_parse(&conn->addr, "1.1.1.1");
  tt_int_op(addr_family, OP_EQ, AF_INET);
  ret = connection_dir_is_global_write_low(conn, INT_MAX);
  tt_int_op(ret, OP_EQ, 1);

  /* Now, lets try with a connection address from moria1. It should always
   * pass even though our limit is too low. */
  addr_family = tor_addr_parse(&conn->addr, "128.31.0.39");
  tt_int_op(addr_family, OP_EQ, AF_INET);
  ret = connection_dir_is_global_write_low(conn, INT_MAX);
  tt_int_op(ret, OP_EQ, 0);

  /* IPv6 testing of gabelmoo. */
  addr_family = tor_addr_parse(&conn->addr, "[2001:638:a000:4140::ffff:189]");
  tt_int_op(addr_family, OP_EQ, AF_INET6);
  ret = connection_dir_is_global_write_low(conn, INT_MAX);
  tt_int_op(ret, OP_EQ, 0);

  /* Lets retry with a known relay address. It should pass. Possible due to
   * our consensus setting above. */
  memcpy(&conn->addr, &relay_addr, sizeof(tor_addr_t));
  ret = connection_dir_is_global_write_low(conn, INT_MAX);
  tt_int_op(ret, OP_EQ, 0);

  /* Lets retry with a random IP that is not an authority nor a relay. */
  addr_family = tor_addr_parse(&conn->addr, "1.2.3.4");
  tt_int_op(addr_family, OP_EQ, AF_INET);
  ret = connection_dir_is_global_write_low(conn, INT_MAX);
  tt_int_op(ret, OP_EQ, 0);

  /* Finally, just make sure it still denies an IP if we are _not_ a v3
   * directory authority. */
  mock_options.V3AuthoritativeDir = 0;
  addr_family = tor_addr_parse(&conn->addr, "1.2.3.4");
  tt_int_op(addr_family, OP_EQ, AF_INET);
  ret = connection_dir_is_global_write_low(conn, INT_MAX);
  tt_int_op(ret, OP_EQ, 1);

  /* Random IPv6 should not be allowed. */
  addr_family = tor_addr_parse(&conn->addr, "[CAFE::ACAB]");
  tt_int_op(addr_family, OP_EQ, AF_INET6);
  ret = connection_dir_is_global_write_low(conn, INT_MAX);
  tt_int_op(ret, OP_EQ, 1);

 done:
  connection_free_minimal(conn);
  routerstatus_free(rs); routerinfo_free(ri); microdesc_free(md);
  smartlist_clear(dummy_ns->routerstatus_list);
  networkstatus_vote_free(dummy_ns);

  UNMOCK(get_estimated_address_per_node);
  UNMOCK(networkstatus_get_latest_consensus);
  UNMOCK(networkstatus_get_latest_consensus_by_flavor);
  UNMOCK(get_options);
}

#define BWMGT(name)                                          \
  { #name, test_bwmgt_ ## name , TT_FORK, NULL, NULL }

struct testcase_t bwmgt_tests[] = {
  BWMGT(token_buf_init),
  BWMGT(token_buf_adjust),
  BWMGT(token_buf_dec),
  BWMGT(token_buf_refill),
  BWMGT(token_buf_helpers),

  BWMGT(dir_conn_global_write_low),
  END_OF_TESTCASES
};
