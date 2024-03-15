/* Copyright (c) 2017-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file test_hs_common.c
 * \brief Test hidden service common functionalities.
 */

#define CONNECTION_EDGE_PRIVATE
#define HS_COMMON_PRIVATE
#define HS_CLIENT_PRIVATE
#define HS_SERVICE_PRIVATE
#define NODELIST_PRIVATE

#include "test/test.h"
#include "test/test_helpers.h"
#include "test/log_test_helpers.h"
#include "test/hs_test_helpers.h"

#include "core/or/connection_edge.h"
#include "lib/crypt_ops/crypto_format.h"
#include "lib/crypt_ops/crypto_rand.h"
#include "feature/hs/hs_common.h"
#include "feature/hs/hs_client.h"
#include "feature/hs/hs_service.h"
#include "app/config/config.h"
#include "feature/nodelist/networkstatus.h"
#include "feature/dirclient/dirclient.h"
#include "feature/dirauth/dirvote.h"
#include "feature/nodelist/nodelist.h"
#include "feature/nodelist/routerlist.h"
#include "app/config/statefile.h"
#include "core/or/circuitlist.h"
#include "feature/dirauth/shared_random.h"
#include "feature/dirauth/voting_schedule.h"

#include "feature/nodelist/microdesc_st.h"
#include "feature/nodelist/networkstatus_st.h"
#include "feature/nodelist/node_st.h"
#include "app/config/or_state_st.h"
#include "feature/nodelist/routerinfo_st.h"
#include "feature/nodelist/routerstatus_st.h"

/** Test the validation of HS v3 addresses */
static void
test_validate_address(void *arg)
{
  int ret;

  (void) arg;

  /* Address too short and too long. */
  setup_full_capture_of_logs(LOG_WARN);
  ret = hs_address_is_valid("blah");
  tt_int_op(ret, OP_EQ, 0);
  expect_log_msg_containing("Invalid length");
  teardown_capture_of_logs();

  setup_full_capture_of_logs(LOG_WARN);
  ret = hs_address_is_valid(
           "p3xnclpu4mu22dwaurjtsybyqk4xfjmcfz6z62yl24uwmhjatiwnlnadb");
  tt_int_op(ret, OP_EQ, 0);
  expect_log_msg_containing("Invalid length");
  teardown_capture_of_logs();

  /* Invalid checksum (taken from prop224) */
  setup_full_capture_of_logs(LOG_WARN);
  ret = hs_address_is_valid(
           "l5satjgud6gucryazcyvyvhuxhr74u6ygigiuyixe3a6ysis67ororad");
  tt_int_op(ret, OP_EQ, 0);
  expect_log_msg_containing("invalid checksum");
  teardown_capture_of_logs();

  setup_full_capture_of_logs(LOG_WARN);
  ret = hs_address_is_valid(
           "btojiu7nu5y5iwut64eufevogqdw4wmqzugnoluw232r4t3ecsfv37ad");
  tt_int_op(ret, OP_EQ, 0);
  expect_log_msg_containing("invalid checksum");
  teardown_capture_of_logs();

  /* Non base32 decodable string. */
  setup_full_capture_of_logs(LOG_WARN);
  ret = hs_address_is_valid(
           "????????????????????????????????????????????????????????");
  tt_int_op(ret, OP_EQ, 0);
  expect_log_msg_containing("Unable to base32 decode");
  teardown_capture_of_logs();

  /* Valid address. */
  ret = hs_address_is_valid(
           "25njqamcweflpvkl73j4szahhihoc4xt3ktcgjnpaingr5yhkenl5sid");
  tt_int_op(ret, OP_EQ, 1);

 done:
  ;
}

static int
mock_write_str_to_file(const char *path, const char *str, int bin)
{
  (void)bin;
  tt_str_op(path, OP_EQ, "/double/five"PATH_SEPARATOR"squared");
  tt_str_op(str, OP_EQ,
           "25njqamcweflpvkl73j4szahhihoc4xt3ktcgjnpaingr5yhkenl5sid.onion\n");

 done:
  return 0;
}

/** Test building HS v3 onion addresses. Uses test vectors from the
 *  ./hs_build_address.py script. */
static void
test_build_address(void *arg)
{
  int ret;
  char onion_addr[HS_SERVICE_ADDR_LEN_BASE32 + 1];
  ed25519_public_key_t pubkey;
  /* hex-encoded ed25519 pubkey used in hs_build_address.py */
  char pubkey_hex[] =
    "d75a980182b10ab7d54bfed3c964073a0ee172f3daa62325af021a68f707511a";
  hs_service_t *service = NULL;

  (void) arg;

  MOCK(write_str_to_file, mock_write_str_to_file);

  /* The following has been created with hs_build_address.py script that
   * follows proposal 224 specification to build an onion address. */
  static const char *test_addr =
    "25njqamcweflpvkl73j4szahhihoc4xt3ktcgjnpaingr5yhkenl5sid";

  /* Let's try to build the same onion address as the script */
  base16_decode((char*)pubkey.pubkey, sizeof(pubkey.pubkey),
                pubkey_hex, strlen(pubkey_hex));
  hs_build_address(&pubkey, HS_VERSION_THREE, onion_addr);
  tt_str_op(test_addr, OP_EQ, onion_addr);
  /* Validate that address. */
  ret = hs_address_is_valid(onion_addr);
  tt_int_op(ret, OP_EQ, 1);

  service = tor_malloc_zero(sizeof(hs_service_t));
  memcpy(service->onion_address, onion_addr, sizeof(service->onion_address));
  tor_asprintf(&service->config.directory_path, "/double/five");
  ret = write_address_to_file(service, "squared");
  tt_int_op(ret, OP_EQ, 0);

 done:
  hs_service_free(service);
}

/** Test that our HS time period calculation functions work properly */
static void
test_time_period(void *arg)
{
  (void) arg;
  uint64_t tn;
  int retval;
  time_t fake_time, correct_time, start_time;

  /* Let's do the example in prop224 section [TIME-PERIODS] */
  retval = parse_rfc1123_time("Wed, 13 Apr 2016 11:00:00 UTC",
                              &fake_time);
  tt_int_op(retval, OP_EQ, 0);

  /* Check that the time period number is right */
  tn = hs_get_time_period_num(fake_time);
  tt_u64_op(tn, OP_EQ, 16903);

  /* Increase current time to 11:59:59 UTC and check that the time period
     number is still the same */
  fake_time += 3599;
  tn = hs_get_time_period_num(fake_time);
  tt_u64_op(tn, OP_EQ, 16903);

  { /* Check start time of next time period */
    retval = parse_rfc1123_time("Wed, 13 Apr 2016 12:00:00 UTC",
                                &correct_time);
    tt_int_op(retval, OP_EQ, 0);

    start_time = hs_get_start_time_of_next_time_period(fake_time);
    tt_int_op(start_time, OP_EQ, correct_time);
  }

  /* Now take time to 12:00:00 UTC and check that the time period rotated */
  fake_time += 1;
  tn = hs_get_time_period_num(fake_time);
  tt_u64_op(tn, OP_EQ, 16904);

  /* Now also check our hs_get_next_time_period_num() function */
  tn = hs_get_next_time_period_num(fake_time);
  tt_u64_op(tn, OP_EQ, 16905);

  { /* Check start time of next time period again */
    retval = parse_rfc1123_time("Wed, 14 Apr 2016 12:00:00 UTC",
                                &correct_time);
    tt_int_op(retval, OP_EQ, 0);

    start_time = hs_get_start_time_of_next_time_period(fake_time);
    tt_int_op(start_time, OP_EQ, correct_time);
  }

  /* Now do another sanity check: The time period number at the start of the
   * next time period, must be the same time period number as the one returned
   * from hs_get_next_time_period_num() */
  {
    time_t next_tp_start = hs_get_start_time_of_next_time_period(fake_time);
    tt_u64_op(hs_get_time_period_num(next_tp_start), OP_EQ,
              hs_get_next_time_period_num(fake_time));
  }

 done:
  ;
}

/** Test that we can correctly find the start time of the next time period */
static void
test_start_time_of_next_time_period(void *arg)
{
  (void) arg;
  int retval;
  time_t fake_time;
  char tbuf[ISO_TIME_LEN + 1];
  time_t next_tp_start_time;

  /* Do some basic tests */
  retval = parse_rfc1123_time("Wed, 13 Apr 2016 11:00:00 UTC",
                              &fake_time);
  tt_int_op(retval, OP_EQ, 0);
  next_tp_start_time = hs_get_start_time_of_next_time_period(fake_time);
  /* Compare it with the correct result */
  format_iso_time(tbuf, next_tp_start_time);
  tt_str_op("2016-04-13 12:00:00", OP_EQ, tbuf);

  /* Another test with an edge-case time (start of TP) */
  retval = parse_rfc1123_time("Wed, 13 Apr 2016 12:00:00 UTC",
                              &fake_time);
  tt_int_op(retval, OP_EQ, 0);
  next_tp_start_time = hs_get_start_time_of_next_time_period(fake_time);
  format_iso_time(tbuf, next_tp_start_time);
  tt_str_op("2016-04-14 12:00:00", OP_EQ, tbuf);

  {
    /* Now pretend we are on a testing network and alter the voting schedule to
       be every 10 seconds. This means that a time period has length 10*24
       seconds (4 minutes). It also means that we apply a rotational offset of
       120 seconds to the time period, so that it starts at 00:02:00 instead of
       00:00:00. */
    or_options_t *options = get_options_mutable();
    options->TestingTorNetwork = 1;
    options->V3AuthVotingInterval = 10;
    options->TestingV3AuthInitialVotingInterval = 10;

    retval = parse_rfc1123_time("Wed, 13 Apr 2016 00:00:00 UTC",
                                &fake_time);
    tt_int_op(retval, OP_EQ, 0);
    next_tp_start_time = hs_get_start_time_of_next_time_period(fake_time);
    /* Compare it with the correct result */
    format_iso_time(tbuf, next_tp_start_time);
    tt_str_op("2016-04-13 00:02:00", OP_EQ, tbuf);

    retval = parse_rfc1123_time("Wed, 13 Apr 2016 00:02:00 UTC",
                                &fake_time);
    tt_int_op(retval, OP_EQ, 0);
    next_tp_start_time = hs_get_start_time_of_next_time_period(fake_time);
    /* Compare it with the correct result */
    format_iso_time(tbuf, next_tp_start_time);
    tt_str_op("2016-04-13 00:06:00", OP_EQ, tbuf);
  }

 done:
  ;
}

/* Cleanup the global nodelist. It also frees the "md" in the node_t because
 * we allocate the memory in helper_add_hsdir_to_networkstatus(). */
static void
cleanup_nodelist(void)
{
  const smartlist_t *nodelist = nodelist_get_list();
  SMARTLIST_FOREACH_BEGIN(nodelist, node_t *, node) {
    tor_free(node->md);
    node->md = NULL;
  } SMARTLIST_FOREACH_END(node);
  nodelist_free_all();
}

static void
helper_add_hsdir_to_networkstatus(networkstatus_t *ns,
                                  int identity_idx,
                                  const char *nickname,
                                  int is_hsdir)
{
  routerstatus_t *rs = tor_malloc_zero(sizeof(routerstatus_t));
  routerinfo_t *ri = tor_malloc_zero(sizeof(routerinfo_t));
  uint8_t identity[DIGEST_LEN];
  node_t *node = NULL;

  memset(identity, identity_idx, sizeof(identity));

  memcpy(rs->identity_digest, identity, DIGEST_LEN);
  rs->is_hs_dir = is_hsdir;
  rs->pv.supports_v3_hsdir = 1;
  strlcpy(rs->nickname, nickname, sizeof(rs->nickname));
  tor_addr_parse(&ri->ipv4_addr, "1.2.3.4");
  tor_addr_parse(&rs->ipv4_addr, "1.2.3.4");
  ri->nickname = tor_strdup(nickname);
  ri->protocol_list = tor_strdup("HSDir=1-2 LinkAuth=3");
  memcpy(ri->cache_info.identity_digest, identity, DIGEST_LEN);
  ri->cache_info.signing_key_cert = tor_malloc_zero(sizeof(tor_cert_t));
  /* Needed for the HSDir index computation. */
  memset(&ri->cache_info.signing_key_cert->signing_key,
         identity_idx, ED25519_PUBKEY_LEN);
  tt_assert(nodelist_set_routerinfo(ri, NULL));

  node = node_get_mutable_by_id(ri->cache_info.identity_digest);
  tt_assert(node);
  node->rs = rs;
  /* We need this to exist for node_has_preferred_descriptor() to return
   * true. */
  node->md = tor_malloc_zero(sizeof(microdesc_t));
  /* Do this now the nodelist_set_routerinfo() function needs a "rs" to set
   * the indexes which it doesn't have when it is called. */
  node_set_hsdir_index(node, ns);
  node->ri = NULL;
  smartlist_add(ns->routerstatus_list, rs);

 done:
  if (node == NULL)
    routerstatus_free(rs);

  routerinfo_free(ri);
}

static networkstatus_t *mock_ns = NULL;

static networkstatus_t *
mock_networkstatus_get_latest_consensus(void)
{
  time_t now = approx_time();

  /* If initialized, return it */
  if (mock_ns) {
    return mock_ns;
  }

  /* Initialize fake consensus */
  mock_ns = tor_malloc_zero(sizeof(networkstatus_t));

  /* This consensus is live */
  mock_ns->valid_after = now-1;
  mock_ns->fresh_until = now+1;
  mock_ns->valid_until = now+2;
  /* Create routerstatus list */
  mock_ns->routerstatus_list = smartlist_new();
  mock_ns->type = NS_TYPE_CONSENSUS;

  return mock_ns;
}

static networkstatus_t *
mock_networkstatus_get_reasonably_live_consensus(time_t now, int flavor)
{
  (void) now;
  (void) flavor;

  tt_assert(mock_ns);

 done:
  return mock_ns;
}

/** Test the responsible HSDirs calculation function */
static void
test_responsible_hsdirs(void *arg)
{
  smartlist_t *responsible_dirs = smartlist_new();
  networkstatus_t *ns = NULL;
  (void) arg;

  hs_init();

  MOCK(networkstatus_get_latest_consensus,
       mock_networkstatus_get_latest_consensus);
  MOCK(networkstatus_get_reasonably_live_consensus,
       mock_networkstatus_get_reasonably_live_consensus);

  ns = networkstatus_get_latest_consensus();

  { /* First router: HSdir */
    helper_add_hsdir_to_networkstatus(ns, 1, "igor", 1);
  }

  { /* Second HSDir */
    helper_add_hsdir_to_networkstatus(ns, 2, "victor", 1);
  }

  { /* Third relay but not HSDir */
    helper_add_hsdir_to_networkstatus(ns, 3, "spyro", 0);
  }

  /* Use a fixed time period and pub key so we always take the same path */
  ed25519_public_key_t pubkey;
  uint64_t time_period_num = 17653; // 2 May, 2018, 14:00.
  memset(&pubkey, 42, sizeof(pubkey));

  hs_get_responsible_hsdirs(&pubkey, time_period_num,
                            0, 0, responsible_dirs);

  /* Make sure that we only found 2 responsible HSDirs.
   * The third relay was not an hsdir! */
  tt_int_op(smartlist_len(responsible_dirs), OP_EQ, 2);

  /** TODO: Build a bigger network and do more tests here */

 done:
  SMARTLIST_FOREACH(ns->routerstatus_list,
                    routerstatus_t *, rs, routerstatus_free(rs));
  smartlist_free(responsible_dirs);
  smartlist_clear(ns->routerstatus_list);
  networkstatus_vote_free(mock_ns);
  cleanup_nodelist();

  UNMOCK(networkstatus_get_reasonably_live_consensus);
}

static void
mock_directory_initiate_request(directory_request_t *req)
{
  (void)req;
  return;
}

static int
mock_hs_desc_encode_descriptor(const hs_descriptor_t *desc,
                               const ed25519_keypair_t *signing_kp,
                               const uint8_t *descriptor_cookie,
                               char **encoded_out)
{
  (void)desc;
  (void)signing_kp;
  (void)descriptor_cookie;

  tor_asprintf(encoded_out, "lulu");
  return 0;
}

static or_state_t dummy_state;

/* Mock function to get fake or state (used for rev counters) */
static or_state_t *
get_or_state_replacement(void)
{
  return &dummy_state;
}

static int
mock_router_have_minimum_dir_info(void)
{
  return 1;
}

/** Test that we correctly detect when the HSDir hash ring changes so that we
 *  reupload our descriptor. */
static void
test_desc_reupload_logic(void *arg)
{
  networkstatus_t *ns = NULL;

  (void) arg;

  hs_init();

  MOCK(networkstatus_get_reasonably_live_consensus,
       mock_networkstatus_get_reasonably_live_consensus);
  MOCK(router_have_minimum_dir_info,
       mock_router_have_minimum_dir_info);
  MOCK(get_or_state,
       get_or_state_replacement);
  MOCK(networkstatus_get_latest_consensus,
       mock_networkstatus_get_latest_consensus);
  MOCK(directory_initiate_request,
       mock_directory_initiate_request);
  MOCK(hs_desc_encode_descriptor,
       mock_hs_desc_encode_descriptor);

  ns = networkstatus_get_latest_consensus();

  /** Test logic:
   *  1) Upload descriptor to HSDirs
   *     CHECK that previous_hsdirs list was populated.
   *  2) Then call router_dir_info_changed() without an HSDir set change.
   *     CHECK that no reupload occurs.
   *  3) Now change the HSDir set, and call dir_info_changed() again.
   *     CHECK that reupload occurs.
   *  4) Finally call service_desc_schedule_upload().
   *     CHECK that previous_hsdirs list was cleared.
   **/

  /* Let's start by building our descriptor and service */
  hs_service_descriptor_t *desc = service_descriptor_new();
  hs_service_t *service = NULL;
  /* hex-encoded ed25519 pubkey used in hs_build_address.py */
  char pubkey_hex[] =
    "d75a980182b10ab7d54bfed3c964073a0ee172f3daa62325af021a68f707511a";
  char onion_addr[HS_SERVICE_ADDR_LEN_BASE32 + 1];
  ed25519_public_key_t pubkey;
  base16_decode((char*)pubkey.pubkey, sizeof(pubkey.pubkey),
                pubkey_hex, strlen(pubkey_hex));
  hs_build_address(&pubkey, HS_VERSION_THREE, onion_addr);
  service = tor_malloc_zero(sizeof(hs_service_t));
  tt_assert(service);
  memcpy(service->onion_address, onion_addr, sizeof(service->onion_address));
  ed25519_secret_key_generate(&service->keys.identity_sk, 0);
  ed25519_public_key_generate(&service->keys.identity_pk,
                              &service->keys.identity_sk);
  service->desc_current = desc;
  /* Also add service to service map */
  hs_service_ht *service_map = get_hs_service_map();
  tt_assert(service_map);
  tt_int_op(hs_service_get_num_services(), OP_EQ, 0);
  register_service(service_map, service);
  tt_int_op(hs_service_get_num_services(), OP_EQ, 1);

  /* Now let's create our hash ring: */
  {
    helper_add_hsdir_to_networkstatus(ns, 1, "dingus", 1);
    helper_add_hsdir_to_networkstatus(ns, 2, "clive", 1);
    helper_add_hsdir_to_networkstatus(ns, 3, "aaron", 1);
    helper_add_hsdir_to_networkstatus(ns, 4, "lizzie", 1);
    helper_add_hsdir_to_networkstatus(ns, 5, "daewon", 1);
    helper_add_hsdir_to_networkstatus(ns, 6, "clarke", 1);
  }

  /* Now let's upload our desc to all hsdirs */
  upload_descriptor_to_all(service, desc);
  /* Check that previous hsdirs were populated */
  tt_int_op(smartlist_len(desc->previous_hsdirs), OP_EQ, 6);

  /* Poison next upload time so that we can see if it was changed by
   * router_dir_info_changed(). No changes in hash ring so far, so the upload
   * time should stay as is. */
  desc->next_upload_time = 42;
  router_dir_info_changed();
  tt_int_op(desc->next_upload_time, OP_EQ, 42);

  /* Now change the HSDir hash ring by swapping nora for aaron.
   * Start by clearing the hash ring */
  {
    SMARTLIST_FOREACH(ns->routerstatus_list,
                      routerstatus_t *, rs, routerstatus_free(rs));
    smartlist_clear(ns->routerstatus_list);
    cleanup_nodelist();
    routerlist_free_all();
  }

  { /* Now add back all the nodes */
    helper_add_hsdir_to_networkstatus(ns, 1, "dingus", 1);
    helper_add_hsdir_to_networkstatus(ns, 2, "clive", 1);
    helper_add_hsdir_to_networkstatus(ns, 4, "lizzie", 1);
    helper_add_hsdir_to_networkstatus(ns, 5, "daewon", 1);
    helper_add_hsdir_to_networkstatus(ns, 6, "clarke", 1);
    helper_add_hsdir_to_networkstatus(ns, 7, "nora", 1);
  }

  /* Now call service_desc_hsdirs_changed() and see that it detected the hash
     ring change */
  time_t now = approx_time();
  tt_assert(now);
  tt_int_op(service_desc_hsdirs_changed(service, desc), OP_EQ, 1);
  tt_int_op(smartlist_len(desc->previous_hsdirs), OP_EQ, 6);

  /* Now order another upload and see that we keep having 6 prev hsdirs */
  upload_descriptor_to_all(service, desc);
  /* Check that previous hsdirs were populated */
  tt_int_op(smartlist_len(desc->previous_hsdirs), OP_EQ, 6);

  /* Now restore the HSDir hash ring to its original state by swapping back
     aaron for nora */
  /* First clear up the hash ring */
  {
    SMARTLIST_FOREACH(ns->routerstatus_list,
                      routerstatus_t *, rs, routerstatus_free(rs));
    smartlist_clear(ns->routerstatus_list);
    cleanup_nodelist();
    routerlist_free_all();
  }

  { /* Now populate the hash ring again */
    helper_add_hsdir_to_networkstatus(ns, 1, "dingus", 1);
    helper_add_hsdir_to_networkstatus(ns, 2, "clive", 1);
    helper_add_hsdir_to_networkstatus(ns, 3, "aaron", 1);
    helper_add_hsdir_to_networkstatus(ns, 4, "lizzie", 1);
    helper_add_hsdir_to_networkstatus(ns, 5, "daewon", 1);
    helper_add_hsdir_to_networkstatus(ns, 6, "clarke", 1);
  }

  /* Check that our algorithm catches this change of hsdirs */
  tt_int_op(service_desc_hsdirs_changed(service, desc), OP_EQ, 1);

  /* Now pretend that the descriptor changed, and order a reupload to all
     HSDirs. Make sure that the set of previous HSDirs was cleared. */
  service_desc_schedule_upload(desc, now, 1);
  tt_int_op(smartlist_len(desc->previous_hsdirs), OP_EQ, 0);

  /* Now reupload again: see that the prev hsdir set got populated again. */
  upload_descriptor_to_all(service, desc);
  tt_int_op(smartlist_len(desc->previous_hsdirs), OP_EQ, 6);

 done:
  SMARTLIST_FOREACH(ns->routerstatus_list,
                    routerstatus_t *, rs, routerstatus_free(rs));
  smartlist_clear(ns->routerstatus_list);
  if (service) {
    remove_service(get_hs_service_map(), service);
    hs_service_free(service);
  }
  networkstatus_vote_free(ns);
  cleanup_nodelist();
  hs_free_all();
}

/** Test disaster SRV computation and caching */
static void
test_disaster_srv(void *arg)
{
  uint8_t *cached_disaster_srv_one = NULL;
  uint8_t *cached_disaster_srv_two = NULL;
  uint8_t srv_one[DIGEST256_LEN] = {0};
  uint8_t srv_two[DIGEST256_LEN] = {0};
  uint8_t srv_three[DIGEST256_LEN] = {0};
  uint8_t srv_four[DIGEST256_LEN] = {0};
  uint8_t srv_five[DIGEST256_LEN] = {0};

  (void) arg;

  /* Get the cached SRVs: we gonna use them later for verification */
  cached_disaster_srv_one = get_first_cached_disaster_srv();
  cached_disaster_srv_two = get_second_cached_disaster_srv();

  /* Compute some srvs */
  get_disaster_srv(1, srv_one);
  get_disaster_srv(2, srv_two);

  /* Check that the cached ones were updated */
  tt_mem_op(cached_disaster_srv_one, OP_EQ, srv_one, DIGEST256_LEN);
  tt_mem_op(cached_disaster_srv_two, OP_EQ, srv_two, DIGEST256_LEN);

  /* For at least one SRV, check that its result was as expected. */
  {
    uint8_t srv1_expected[32];
    crypto_digest256(
        (char*)srv1_expected,
        "shared-random-disaster\0\0\0\0\0\0\x05\xA0\0\0\0\0\0\0\0\1",
        strlen("shared-random-disaster")+16,
        DIGEST_SHA3_256);
    tt_mem_op(srv_one, OP_EQ, srv1_expected, DIGEST256_LEN);
    tt_str_op(hex_str((char*)srv_one, DIGEST256_LEN), OP_EQ,
        "F8A4948707653837FA44ABB5BBC75A12F6F101E7F8FAF699B9715F4965D3507D");
  }

  /* Ask for an SRV that has already been computed */
  get_disaster_srv(2, srv_two);
  /* and check that the cache entries have not changed */
  tt_mem_op(cached_disaster_srv_one, OP_EQ, srv_one, DIGEST256_LEN);
  tt_mem_op(cached_disaster_srv_two, OP_EQ, srv_two, DIGEST256_LEN);

  /* Ask for a new SRV */
  get_disaster_srv(3, srv_three);
  tt_mem_op(cached_disaster_srv_one, OP_EQ, srv_three, DIGEST256_LEN);
  tt_mem_op(cached_disaster_srv_two, OP_EQ, srv_two, DIGEST256_LEN);

  /* Ask for another SRV: none of the original SRVs should now be cached */
  get_disaster_srv(4, srv_four);
  tt_mem_op(cached_disaster_srv_one, OP_EQ, srv_three, DIGEST256_LEN);
  tt_mem_op(cached_disaster_srv_two, OP_EQ, srv_four, DIGEST256_LEN);

  /* Ask for yet another SRV */
  get_disaster_srv(5, srv_five);
  tt_mem_op(cached_disaster_srv_one, OP_EQ, srv_five, DIGEST256_LEN);
  tt_mem_op(cached_disaster_srv_two, OP_EQ, srv_four, DIGEST256_LEN);

 done:
  ;
}

/** Test our HS descriptor request tracker by making various requests and
 *  checking whether they get tracked properly. */
static void
test_hid_serv_request_tracker(void *arg)
{
  (void) arg;
  time_t retval;
  routerstatus_t *hsdir = NULL, *hsdir2 = NULL, *hsdir3 = NULL;
  time_t now = approx_time();

  const char *req_key_str_first =
 "vd4zb6zesaubtrjvdqcr2w7x7lhw2up4Xnw4526ThUNbL5o1go+EdUuEqlKxHkNbnK41pRzizzs";
  const char *req_key_str_second =
 "g53o7iavcd62oihswhr24u6czmqws5kpXnw4526ThUNbL5o1go+EdUuEqlKxHkNbnK41pRzizzs";
  const char *req_key_str_small = "ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ";

  /*************************** basic test *******************************/

  /* Get request tracker and make sure it's empty */
  strmap_t *request_tracker = get_last_hid_serv_requests();
  tt_int_op(strmap_size(request_tracker),OP_EQ, 0);

  /* Let's register a hid serv request */
  hsdir = tor_malloc_zero(sizeof(routerstatus_t));
  memset(hsdir->identity_digest, 'Z', DIGEST_LEN);
  retval = hs_lookup_last_hid_serv_request(hsdir, req_key_str_first,
                                           now, 1);
  tt_int_op(retval, OP_EQ, now);
  tt_int_op(strmap_size(request_tracker),OP_EQ, 1);

  /* Let's lookup a non-existent hidserv request */
  retval = hs_lookup_last_hid_serv_request(hsdir, req_key_str_second,
                                           now+1, 0);
  tt_int_op(retval, OP_EQ, 0);
  tt_int_op(strmap_size(request_tracker),OP_EQ, 1);

  /* Let's lookup a real hidserv request */
  retval = hs_lookup_last_hid_serv_request(hsdir, req_key_str_first,
                                           now+2, 0);
  tt_int_op(retval, OP_EQ, now); /* we got it */
  tt_int_op(strmap_size(request_tracker),OP_EQ, 1);

  /**********************************************************************/

  /* Let's add another request for the same HS but on a different HSDir. */
  hsdir2 = tor_malloc_zero(sizeof(routerstatus_t));
  memset(hsdir2->identity_digest, 2, DIGEST_LEN);
  retval = hs_lookup_last_hid_serv_request(hsdir2, req_key_str_first,
                                           now+3, 1);
  tt_int_op(retval, OP_EQ, now+3);
  tt_int_op(strmap_size(request_tracker),OP_EQ, 2);

  /* Check that we can clean the first request based on time */
  hs_clean_last_hid_serv_requests(now+3+REND_HID_SERV_DIR_REQUERY_PERIOD);
  tt_int_op(strmap_size(request_tracker),OP_EQ, 1);
  /* Check that it doesn't exist anymore */
  retval = hs_lookup_last_hid_serv_request(hsdir, req_key_str_first,
                                           now+2, 0);
  tt_int_op(retval, OP_EQ, 0);

  /* Now let's add a smaller req key str */
  hsdir3 = tor_malloc_zero(sizeof(routerstatus_t));
  memset(hsdir3->identity_digest, 3, DIGEST_LEN);
  retval = hs_lookup_last_hid_serv_request(hsdir3, req_key_str_small,
                                           now+4, 1);
  tt_int_op(retval, OP_EQ, now+4);
  tt_int_op(strmap_size(request_tracker),OP_EQ, 2);

  /*************************** deleting entries **************************/

  /* Add another request with very short key */
  retval = hs_lookup_last_hid_serv_request(hsdir, "l",  now, 1);
  tt_int_op(retval, OP_EQ, now);
  tt_int_op(strmap_size(request_tracker),OP_EQ, 3);

  /* Try deleting entries with a dummy key. Check that our previous requests
   * are still there */
  tor_capture_bugs_(1);
  hs_purge_hid_serv_from_last_hid_serv_requests("a");
  tt_int_op(strmap_size(request_tracker),OP_EQ, 3);
  tor_end_capture_bugs_();

  /* Try another dummy key. Check that requests are still there */
  {
    char dummy[2000];
    memset(dummy, 'Z', 2000);
    dummy[1999] = '\x00';
    hs_purge_hid_serv_from_last_hid_serv_requests(dummy);
    tt_int_op(strmap_size(request_tracker),OP_EQ, 3);
  }

  /* Another dummy key! */
  hs_purge_hid_serv_from_last_hid_serv_requests(req_key_str_second);
  tt_int_op(strmap_size(request_tracker),OP_EQ, 3);

  /* Now actually delete a request! */
  hs_purge_hid_serv_from_last_hid_serv_requests(req_key_str_first);
  tt_int_op(strmap_size(request_tracker),OP_EQ, 2);

  /* Purge it all! */
  hs_purge_last_hid_serv_requests();
  request_tracker = get_last_hid_serv_requests();
  tt_int_op(strmap_size(request_tracker),OP_EQ, 0);

 done:
  tor_free(hsdir);
  tor_free(hsdir2);
  tor_free(hsdir3);
}

static void
test_parse_extended_hostname(void *arg)
{
  (void) arg;
  hostname_type_t type;

  char address1[] = "fooaddress.onion";
  char address3[] = "fooaddress.exit";
  char address4[] = "www.torproject.org";
  char address5[] = "foo.abcdefghijklmnop.onion";
  char address6[] = "foo.bar.abcdefghijklmnop.onion";
  char address7[] = ".abcdefghijklmnop.onion";
  char address8[] =
    "www.25njqamcweflpvkl73j4szahhihoc4xt3ktcgjnpaingr5yhkenl5sid.onion";
  char address9[] =
    "www.15njqamcweflpvkl73j4szahhihoc4xt3ktcgjnpaingr5yhkenl5sid.onion";
  char address10[] =
    "15njqamcweflpvkl73j4szahhihoc4xt3ktcgjnpaingr5yhkenl5sid7jdl.onion";

  tt_assert(!parse_extended_hostname(address1, &type));
  tt_int_op(type, OP_EQ, BAD_HOSTNAME);

  tt_assert(parse_extended_hostname(address3, &type));
  tt_int_op(type, OP_EQ, EXIT_HOSTNAME);

  tt_assert(parse_extended_hostname(address4, &type));
  tt_int_op(type, OP_EQ, NORMAL_HOSTNAME);

  tt_assert(!parse_extended_hostname(address5, &type));
  tt_int_op(type, OP_EQ, BAD_HOSTNAME);

  tt_assert(!parse_extended_hostname(address6, &type));
  tt_int_op(type, OP_EQ, BAD_HOSTNAME);

  tt_assert(!parse_extended_hostname(address7, &type));
  tt_int_op(type, OP_EQ, BAD_HOSTNAME);

  tt_assert(parse_extended_hostname(address8, &type));
  tt_int_op(type, OP_EQ, ONION_V3_HOSTNAME);
  tt_str_op(address8, OP_EQ,
            "25njqamcweflpvkl73j4szahhihoc4xt3ktcgjnpaingr5yhkenl5sid");

  /* Invalid v3 address. */
  tt_assert(!parse_extended_hostname(address9, &type));
  tt_int_op(type, OP_EQ, BAD_HOSTNAME);

  /* Invalid v3 address: too long */
  tt_assert(!parse_extended_hostname(address10, &type));
  tt_int_op(type, OP_EQ, BAD_HOSTNAME);

 done: ;
}

static void
test_time_between_tp_and_srv(void *arg)
{
  int ret;
  networkstatus_t ns;
  (void) arg;

  /* This function should be returning true where "^" are:
   *
   *    +------------------------------------------------------------------+
   *    |                                                                  |
   *    | 00:00      12:00       00:00       12:00       00:00       12:00 |
   *    | SRV#1      TP#1        SRV#2       TP#2        SRV#3       TP#3  |
   *    |                                                                  |
   *    |  $==========|-----------$===========|-----------$===========|    |
   *    |             ^^^^^^^^^^^^            ^^^^^^^^^^^^                 |
   *    |                                                                  |
   *    +------------------------------------------------------------------+
   */

  ret = parse_rfc1123_time("Sat, 26 Oct 1985 00:00:00 UTC", &ns.valid_after);
  tt_int_op(ret, OP_EQ, 0);
  ret = parse_rfc1123_time("Sat, 26 Oct 1985 01:00:00 UTC", &ns.fresh_until);
  tt_int_op(ret, OP_EQ, 0);
  dirauth_sched_recalculate_timing(get_options(), ns.valid_after);
  ret = hs_in_period_between_tp_and_srv(&ns, 0);
  tt_int_op(ret, OP_EQ, 0);

  ret = parse_rfc1123_time("Sat, 26 Oct 1985 11:00:00 UTC", &ns.valid_after);
  tt_int_op(ret, OP_EQ, 0);
  ret = parse_rfc1123_time("Sat, 26 Oct 1985 12:00:00 UTC", &ns.fresh_until);
  tt_int_op(ret, OP_EQ, 0);
  dirauth_sched_recalculate_timing(get_options(), ns.valid_after);
  ret = hs_in_period_between_tp_and_srv(&ns, 0);
  tt_int_op(ret, OP_EQ, 0);

  ret = parse_rfc1123_time("Sat, 26 Oct 1985 12:00:00 UTC", &ns.valid_after);
  tt_int_op(ret, OP_EQ, 0);
  ret = parse_rfc1123_time("Sat, 26 Oct 1985 13:00:00 UTC", &ns.fresh_until);
  tt_int_op(ret, OP_EQ, 0);
  dirauth_sched_recalculate_timing(get_options(), ns.valid_after);
  ret = hs_in_period_between_tp_and_srv(&ns, 0);
  tt_int_op(ret, OP_EQ, 1);

  ret = parse_rfc1123_time("Sat, 26 Oct 1985 23:00:00 UTC", &ns.valid_after);
  tt_int_op(ret, OP_EQ, 0);
  ret = parse_rfc1123_time("Sat, 27 Oct 1985 00:00:00 UTC", &ns.fresh_until);
  tt_int_op(ret, OP_EQ, 0);
  dirauth_sched_recalculate_timing(get_options(), ns.valid_after);
  ret = hs_in_period_between_tp_and_srv(&ns, 0);
  tt_int_op(ret, OP_EQ, 1);

  ret = parse_rfc1123_time("Sat, 27 Oct 1985 00:00:00 UTC", &ns.valid_after);
  tt_int_op(ret, OP_EQ, 0);
  ret = parse_rfc1123_time("Sat, 27 Oct 1985 01:00:00 UTC", &ns.fresh_until);
  tt_int_op(ret, OP_EQ, 0);
  dirauth_sched_recalculate_timing(get_options(), ns.valid_after);
  ret = hs_in_period_between_tp_and_srv(&ns, 0);
  tt_int_op(ret, OP_EQ, 0);

 done:
  ;
}

/************ Reachability Test (it is huge) ****************/

/* Simulate different consensus for client and service. Used by the
 * reachability test. The SRV and responsible HSDir list are used by all
 * reachability tests so make them common to simplify setup and teardown. */
static networkstatus_t *mock_service_ns = NULL;
static networkstatus_t *mock_client_ns = NULL;
static sr_srv_t current_srv, previous_srv;
static smartlist_t *service_responsible_hsdirs = NULL;
static smartlist_t *client_responsible_hsdirs = NULL;

static networkstatus_t *
mock_networkstatus_get_reasonably_live_consensus_service(time_t now,
                                                         int flavor)
{
  (void) now;
  (void) flavor;

  if (mock_service_ns) {
    return mock_service_ns;
  }

  mock_service_ns = tor_malloc_zero(sizeof(networkstatus_t));
  mock_service_ns->routerstatus_list = smartlist_new();
  mock_service_ns->type = NS_TYPE_CONSENSUS;

  return mock_service_ns;
}

static networkstatus_t *
mock_networkstatus_get_latest_consensus_service(void)
{
  return mock_networkstatus_get_reasonably_live_consensus_service(0, 0);
}

static networkstatus_t *
mock_networkstatus_get_reasonably_live_consensus_client(time_t now, int flavor)
{
  (void) now;
  (void) flavor;

  if (mock_client_ns) {
    return mock_client_ns;
  }

  mock_client_ns = tor_malloc_zero(sizeof(networkstatus_t));
  mock_client_ns->routerstatus_list = smartlist_new();
  mock_client_ns->type = NS_TYPE_CONSENSUS;

  return mock_client_ns;
}

static networkstatus_t *
mock_networkstatus_get_latest_consensus_client(void)
{
  return mock_networkstatus_get_reasonably_live_consensus_client(0, 0);
}

/* Mock function because we are not trying to test the close circuit that does
 * an awful lot of checks on the circuit object. */
static void
mock_circuit_mark_for_close(circuit_t *circ, int reason, int line,
                            const char *file)
{
  (void) circ;
  (void) reason;
  (void) line;
  (void) file;
  return;
}

/* Initialize a big HSDir V3 hash ring. */
static void
helper_initialize_big_hash_ring(networkstatus_t *ns)
{
  int ret;

  /* Generate 250 hsdirs! :) */
  for (int counter = 1 ; counter < 251 ; counter++) {
    /* Let's generate random nickname for each hsdir... */
    char nickname_binary[8];
    char nickname_str[13] = {0};
    crypto_rand(nickname_binary, sizeof(nickname_binary));
    ret = base64_encode(nickname_str, sizeof(nickname_str),
                        nickname_binary, sizeof(nickname_binary), 0);
    tt_int_op(ret, OP_EQ, 12);
    helper_add_hsdir_to_networkstatus(ns, counter, nickname_str, 1);
  }

  /* Make sure we have 200 hsdirs in our list */
  tt_int_op(smartlist_len(ns->routerstatus_list), OP_EQ, 250);

 done:
  ;
}

/** Initialize service and publish its descriptor as needed. Return the newly
 *  allocated service object to the caller. */
static hs_service_t *
helper_init_service(time_t now)
{
  int retval;
  hs_service_t *service = hs_service_new(get_options());
  tt_assert(service);
  service->config.version = HS_VERSION_THREE;
  ed25519_secret_key_generate(&service->keys.identity_sk, 0);
  ed25519_public_key_generate(&service->keys.identity_pk,
                              &service->keys.identity_sk);
  /* Register service to global map. */
  retval = register_service(get_hs_service_map(), service);
  tt_int_op(retval, OP_EQ, 0);

  /* Initialize service descriptor */
  build_all_descriptors(now);
  tt_assert(service->desc_current);
  tt_assert(service->desc_next);

 done:
  return service;
}

/* Helper function to set the RFC 1123 time string into t. */
static void
set_consensus_times(const char *timestr, time_t *t)
{
  tt_assert(timestr);
  tt_assert(t);

  int ret = parse_rfc1123_time(timestr, t);
  tt_int_op(ret, OP_EQ, 0);

 done:
  return;
}

/* Helper function to cleanup the mock consensus (client and service) */
static void
cleanup_mock_ns(void)
{
  if (mock_service_ns) {
    SMARTLIST_FOREACH(mock_service_ns->routerstatus_list,
                      routerstatus_t *, rs, routerstatus_free(rs));
    smartlist_clear(mock_service_ns->routerstatus_list);
    mock_service_ns->sr_info.current_srv = NULL;
    mock_service_ns->sr_info.previous_srv = NULL;
    networkstatus_vote_free(mock_service_ns);
    mock_service_ns = NULL;
  }

  if (mock_client_ns) {
    SMARTLIST_FOREACH(mock_client_ns->routerstatus_list,
                      routerstatus_t *, rs, routerstatus_free(rs));
    smartlist_clear(mock_client_ns->routerstatus_list);
    mock_client_ns->sr_info.current_srv = NULL;
    mock_client_ns->sr_info.previous_srv = NULL;
    networkstatus_vote_free(mock_client_ns);
    mock_client_ns = NULL;
  }
}

/* Helper function to setup a reachability test. Once called, the
 * cleanup_reachability_test MUST be called at the end. */
static void
setup_reachability_test(void)
{
  MOCK(circuit_mark_for_close_, mock_circuit_mark_for_close);
  MOCK(get_or_state, get_or_state_replacement);

  hs_init();

  /* Baseline to start with. */
  memset(&current_srv, 0, sizeof(current_srv));
  memset(&previous_srv, 1, sizeof(previous_srv));

  /* Initialize the consensuses. */
  mock_networkstatus_get_latest_consensus_service();
  mock_networkstatus_get_latest_consensus_client();

  service_responsible_hsdirs = smartlist_new();
  client_responsible_hsdirs = smartlist_new();
}

/* Helper function to cleanup a reachability test initial setup. */
static void
cleanup_reachability_test(void)
{
  smartlist_free(service_responsible_hsdirs);
  service_responsible_hsdirs = NULL;
  smartlist_free(client_responsible_hsdirs);
  client_responsible_hsdirs = NULL;
  hs_free_all();
  cleanup_mock_ns();
  UNMOCK(get_or_state);
  UNMOCK(circuit_mark_for_close_);
}

/* A reachability test always check if the resulting service and client
 * responsible HSDir for the given parameters are equal.
 *
 * Return true iff the same exact nodes are in both list. */
static int
are_responsible_hsdirs_equal(void)
{
  int count = 0;
  tt_int_op(smartlist_len(client_responsible_hsdirs), OP_EQ, 6);
  tt_int_op(smartlist_len(service_responsible_hsdirs), OP_EQ, 8);

  SMARTLIST_FOREACH_BEGIN(client_responsible_hsdirs,
                          const routerstatus_t *, c_rs) {
    SMARTLIST_FOREACH_BEGIN(service_responsible_hsdirs,
                            const routerstatus_t *, s_rs) {
      if (tor_memeq(c_rs->identity_digest, s_rs->identity_digest,
                    DIGEST_LEN)) {
        count++;
        break;
      }
    } SMARTLIST_FOREACH_END(s_rs);
  } SMARTLIST_FOREACH_END(c_rs);

 done:
  return (count == 6);
}

/* Tor doesn't use such a function to get the previous HSDir, it is only used
 * in node_set_hsdir_index(). We need it here so we can test the reachability
 * scenario 6 that requires the previous time period to compute the list of
 * responsible HSDir because of the client state timing. */
static uint64_t
get_previous_time_period(time_t now)
{
  return hs_get_time_period_num(now) - 1;
}

/* Configuration of a reachability test scenario. */
typedef struct reachability_cfg_t {
  /* Consensus timings to be set. They have to be compliant with
   * RFC 1123 time format. */
  const char *service_valid_after;
  const char *service_valid_until;
  const char *client_valid_after;
  const char *client_valid_until;

  /* SRVs that the service and client should use. */
  sr_srv_t *service_current_srv;
  sr_srv_t *service_previous_srv;
  sr_srv_t *client_current_srv;
  sr_srv_t *client_previous_srv;

  /* A time period function for the service to use for this scenario. For a
   * successful reachability test, the client always use the current time
   * period thus why no client function. */
  uint64_t (*service_time_period_fn)(time_t);

  /* Is the client and service expected to be in a new time period. After
   * setting the consensus time, the reachability test checks
   * hs_in_period_between_tp_and_srv() and test the returned value against
   * this. */
  unsigned int service_in_new_tp;
  unsigned int client_in_new_tp;

  /* Some scenario requires a hint that the client, because of its consensus
   * time, will request the "next" service descriptor so this indicates if it
   * is the case or not. */
  unsigned int client_fetch_next_desc;
} reachability_cfg_t;

/* Some defines to help with semantic while reading a configuration below. */
#define NOT_IN_NEW_TP 0
#define IN_NEW_TP 1
#define DONT_NEED_NEXT_DESC 0
#define NEED_NEXT_DESC 1

static reachability_cfg_t reachability_scenarios[] = {
  /* Scenario 1
   *
   *  +------------------------------------------------------------------+
   *  |                                                                  |
   *  | 00:00      12:00       00:00       12:00       00:00       12:00 |
   *  | SRV#1      TP#1        SRV#2       TP#2        SRV#3       TP#3  |
   *  |                                                                  |
   *  |  $==========|-----------$===========|-----------$===========|    |
   *  |              ^ ^                                                 |
   *  |              S C                                                 |
   *  +------------------------------------------------------------------+
   *
   *  S: Service, C: Client
   *
   *  Service consensus valid_after time is set to 13:00 and client to 15:00,
   *  both are after TP#1 thus have access to SRV#1. Service and client should
   *  be using TP#1.
   */

  { "Sat, 26 Oct 1985 13:00:00 UTC", /* Service valid_after */
    "Sat, 26 Oct 1985 14:00:00 UTC", /* Service valid_until */
    "Sat, 26 Oct 1985 15:00:00 UTC", /* Client valid_after */
    "Sat, 26 Oct 1985 16:00:00 UTC", /* Client valid_until. */
    &current_srv, NULL, /* Service current and previous SRV */
    &current_srv, NULL, /* Client current and previous SRV */
    hs_get_time_period_num, /* Service time period function. */
    IN_NEW_TP, /* Is service in new TP? */
    IN_NEW_TP, /* Is client in new TP? */
    NEED_NEXT_DESC },

  /* Scenario 2
   *
   *  +------------------------------------------------------------------+
   *  |                                                                  |
   *  | 00:00      12:00       00:00       12:00       00:00       12:00 |
   *  | SRV#1      TP#1        SRV#2       TP#2        SRV#3       TP#3  |
   *  |                                                                  |
   *  |  $==========|-----------$===========|-----------$===========|    |
   *  |                        ^ ^                                       |
   *  |                        S C                                       |
   *  +------------------------------------------------------------------+
   *
   *  S: Service, C: Client
   *
   *  Service consensus valid_after time is set to 23:00 and client to 01:00,
   *  which makes the client after the SRV#2 and the service just before. The
   *  service should only be using TP#1. The client should be using TP#1.
   */

  { "Sat, 26 Oct 1985 23:00:00 UTC", /* Service valid_after */
    "Sat, 27 Oct 1985 00:00:00 UTC", /* Service valid_until */
    "Sat, 27 Oct 1985 01:00:00 UTC", /* Client valid_after */
    "Sat, 27 Oct 1985 02:00:00 UTC", /* Client valid_until. */
    &previous_srv, NULL, /* Service current and previous SRV */
    &current_srv, &previous_srv, /* Client current and previous SRV */
    hs_get_time_period_num, /* Service time period function. */
    IN_NEW_TP, /* Is service in new TP? */
    NOT_IN_NEW_TP, /* Is client in new TP? */
    NEED_NEXT_DESC },

  /* Scenario 3
   *
   *  +------------------------------------------------------------------+
   *  |                                                                  |
   *  | 00:00      12:00       00:00       12:00       00:00       12:00 |
   *  | SRV#1      TP#1        SRV#2       TP#2        SRV#3       TP#3  |
   *  |                                                                  |
   *  |  $==========|-----------$===========|----------$===========|     |
   *  |                            ^ ^                                   |
   *  |                            S C                                   |
   *  +------------------------------------------------------------------+
   *
   *  S: Service, C: Client
   *
   *  Service consensus valid_after time is set to 03:00 and client to 05:00,
   *  which makes both after SRV#2. The service should be using TP#1 as its
   *  current time period. The client should be using TP#1.
   */

  { "Sat, 27 Oct 1985 03:00:00 UTC", /* Service valid_after */
    "Sat, 27 Oct 1985 04:00:00 UTC", /* Service valid_until */
    "Sat, 27 Oct 1985 05:00:00 UTC", /* Client valid_after */
    "Sat, 27 Oct 1985 06:00:00 UTC", /* Client valid_until. */
    &current_srv, &previous_srv, /* Service current and previous SRV */
    &current_srv, &previous_srv, /* Client current and previous SRV */
    hs_get_time_period_num, /* Service time period function. */
    NOT_IN_NEW_TP, /* Is service in new TP? */
    NOT_IN_NEW_TP, /* Is client in new TP? */
    DONT_NEED_NEXT_DESC },

  /* Scenario 4
   *
   *  +------------------------------------------------------------------+
   *  |                                                                  |
   *  | 00:00      12:00       00:00       12:00       00:00       12:00 |
   *  | SRV#1      TP#1        SRV#2       TP#2        SRV#3       TP#3  |
   *  |                                                                  |
   *  |  $==========|-----------$===========|-----------$===========|    |
   *  |                                    ^ ^                           |
   *  |                                    S C                           |
   *  +------------------------------------------------------------------+
   *
   *  S: Service, C: Client
   *
   *  Service consensus valid_after time is set to 11:00 and client to 13:00,
   *  which makes the service before TP#2 and the client just after. The
   *  service should be using TP#1 as its current time period and TP#2 as the
   *  next. The client should be using TP#2 time period.
   */

  { "Sat, 27 Oct 1985 11:00:00 UTC", /* Service valid_after */
    "Sat, 27 Oct 1985 12:00:00 UTC", /* Service valid_until */
    "Sat, 27 Oct 1985 13:00:00 UTC", /* Client valid_after */
    "Sat, 27 Oct 1985 14:00:00 UTC", /* Client valid_until. */
    &current_srv, &previous_srv, /* Service current and previous SRV */
    &current_srv, &previous_srv, /* Client current and previous SRV */
    hs_get_next_time_period_num, /* Service time period function. */
    NOT_IN_NEW_TP, /* Is service in new TP? */
    IN_NEW_TP, /* Is client in new TP? */
    NEED_NEXT_DESC },

  /* Scenario 5
   *
   *  +------------------------------------------------------------------+
   *  |                                                                  |
   *  | 00:00      12:00       00:00       12:00       00:00       12:00 |
   *  | SRV#1      TP#1        SRV#2       TP#2        SRV#3       TP#3  |
   *  |                                                                  |
   *  |  $==========|-----------$===========|-----------$===========|    |
   *  |                        ^ ^                                       |
   *  |                        C S                                       |
   *  +------------------------------------------------------------------+
   *
   *  S: Service, C: Client
   *
   *  Service consensus valid_after time is set to 01:00 and client to 23:00,
   *  which makes the service after SRV#2 and the client just before. The
   *  service should be using TP#1 as its current time period and TP#2 as the
   *  next. The client should be using TP#1 time period.
   */

  { "Sat, 27 Oct 1985 01:00:00 UTC", /* Service valid_after */
    "Sat, 27 Oct 1985 02:00:00 UTC", /* Service valid_until */
    "Sat, 26 Oct 1985 23:00:00 UTC", /* Client valid_after */
    "Sat, 27 Oct 1985 00:00:00 UTC", /* Client valid_until. */
    &current_srv, &previous_srv, /* Service current and previous SRV */
    &previous_srv, NULL, /* Client current and previous SRV */
    hs_get_time_period_num, /* Service time period function. */
    NOT_IN_NEW_TP, /* Is service in new TP? */
    IN_NEW_TP, /* Is client in new TP? */
    DONT_NEED_NEXT_DESC },

  /* Scenario 6
   *
   *  +------------------------------------------------------------------+
   *  |                                                                  |
   *  | 00:00      12:00       00:00       12:00       00:00       12:00 |
   *  | SRV#1      TP#1        SRV#2       TP#2        SRV#3       TP#3  |
   *  |                                                                  |
   *  |  $==========|-----------$===========|-----------$===========|    |
   *  |                                    ^ ^                           |
   *  |                                    C S                           |
   *  +------------------------------------------------------------------+
   *
   *  S: Service, C: Client
   *
   *  Service consensus valid_after time is set to 13:00 and client to 11:00,
   *  which makes the service outside after TP#2 and the client just before.
   *  The service should be using TP#1 as its current time period and TP#2 as
   *  its next. The client should be using TP#1 time period.
   */

  { "Sat, 27 Oct 1985 13:00:00 UTC", /* Service valid_after */
    "Sat, 27 Oct 1985 14:00:00 UTC", /* Service valid_until */
    "Sat, 27 Oct 1985 11:00:00 UTC", /* Client valid_after */
    "Sat, 27 Oct 1985 12:00:00 UTC", /* Client valid_until. */
    &current_srv, &previous_srv, /* Service current and previous SRV */
    &current_srv, &previous_srv, /* Client current and previous SRV */
    get_previous_time_period, /* Service time period function. */
    IN_NEW_TP, /* Is service in new TP? */
    NOT_IN_NEW_TP, /* Is client in new TP? */
    DONT_NEED_NEXT_DESC },

  /* End marker. */
  { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0}
};

/* Run a single reachability scenario. num_scenario is the corresponding
 * scenario number from the documentation. It is used to log it in case of
 * failure so we know which scenario fails. */
static int
run_reachability_scenario(const reachability_cfg_t *cfg, int num_scenario)
{
  int ret = -1;
  hs_service_t *service;
  uint64_t service_tp, client_tp;
  ed25519_public_key_t service_blinded_pk, client_blinded_pk;

  setup_reachability_test();

  tt_assert(cfg);

  /* Set service consensus time. */
  set_consensus_times(cfg->service_valid_after,
                      &mock_service_ns->valid_after);
  set_consensus_times(cfg->service_valid_until,
                      &mock_service_ns->valid_until);
  set_consensus_times(cfg->service_valid_until,
                      &mock_service_ns->fresh_until);
  dirauth_sched_recalculate_timing(get_options(),
                                     mock_service_ns->valid_after);
  /* Check that service is in the right time period point */
  tt_int_op(hs_in_period_between_tp_and_srv(mock_service_ns, 0), OP_EQ,
            cfg->service_in_new_tp);

  /* Set client consensus time. */
  set_consensus_times(cfg->client_valid_after,
                      &mock_client_ns->valid_after);
  set_consensus_times(cfg->client_valid_until,
                      &mock_client_ns->valid_until);
  set_consensus_times(cfg->client_valid_until,
                      &mock_client_ns->fresh_until);
  dirauth_sched_recalculate_timing(get_options(),
                                     mock_client_ns->valid_after);
  /* Check that client is in the right time period point */
  tt_int_op(hs_in_period_between_tp_and_srv(mock_client_ns, 0), OP_EQ,
            cfg->client_in_new_tp);

  /* Set the SRVs for this scenario. */
  mock_client_ns->sr_info.current_srv = cfg->client_current_srv;
  mock_client_ns->sr_info.previous_srv = cfg->client_previous_srv;
  mock_service_ns->sr_info.current_srv = cfg->service_current_srv;
  mock_service_ns->sr_info.previous_srv = cfg->service_previous_srv;

  /* Initialize a service to get keys. */
  update_approx_time(mock_service_ns->valid_after);
  service = helper_init_service(mock_service_ns->valid_after+1);

  /*
   * === Client setup ===
   */

  MOCK(networkstatus_get_reasonably_live_consensus,
       mock_networkstatus_get_reasonably_live_consensus_client);
  MOCK(networkstatus_get_latest_consensus,
       mock_networkstatus_get_latest_consensus_client);

  /* Make networkstatus_is_live() happy. */
  update_approx_time(mock_client_ns->valid_after);
  /* Initialize a big hashring for this consensus with the hsdir index set. */
  helper_initialize_big_hash_ring(mock_client_ns);

  /* Client ONLY use the current time period. This is the whole point of these
   * reachability test that is to make sure the client can always reach the
   * service using only its current time period. */
  client_tp = hs_get_time_period_num(0);

  hs_build_blinded_pubkey(&service->keys.identity_pk, NULL, 0,
                          client_tp, &client_blinded_pk);
  hs_get_responsible_hsdirs(&client_blinded_pk, client_tp, 0, 1,
                            client_responsible_hsdirs);
  /* Cleanup the nodelist so we can let the service computes its own set of
   * node with its own hashring. */
  cleanup_nodelist();
  tt_int_op(smartlist_len(client_responsible_hsdirs), OP_EQ, 6);

  UNMOCK(networkstatus_get_latest_consensus);
  UNMOCK(networkstatus_get_reasonably_live_consensus);

  /*
   * === Service setup ===
   */

  MOCK(networkstatus_get_reasonably_live_consensus,
       mock_networkstatus_get_reasonably_live_consensus_service);
  MOCK(networkstatus_get_latest_consensus,
       mock_networkstatus_get_latest_consensus_service);

  /* Make networkstatus_is_live() happy. */
  update_approx_time(mock_service_ns->valid_after);
  /* Initialize a big hashring for this consensus with the hsdir index set. */
  helper_initialize_big_hash_ring(mock_service_ns);

  service_tp = cfg->service_time_period_fn(0);

  hs_build_blinded_pubkey(&service->keys.identity_pk, NULL, 0,
                          service_tp, &service_blinded_pk);

  /* A service builds two lists of responsible HSDir, for the current and the
   * next descriptor. Depending on the scenario, the client timing indicate if
   * it is fetching the current or the next descriptor so we use the
   * "client_fetch_next_desc" to know which one the client is trying to get to
   * confirm that the service computes the same hashring for the same blinded
   * key and service time period function. */
  hs_get_responsible_hsdirs(&service_blinded_pk, service_tp,
                            cfg->client_fetch_next_desc, 0,
                            service_responsible_hsdirs);
  cleanup_nodelist();
  tt_int_op(smartlist_len(service_responsible_hsdirs), OP_EQ, 8);

  UNMOCK(networkstatus_get_latest_consensus);
  UNMOCK(networkstatus_get_reasonably_live_consensus);

  /* Some testing of the values we just got from the client and service. */
  tt_mem_op(&client_blinded_pk, OP_EQ, &service_blinded_pk,
            ED25519_PUBKEY_LEN);
  tt_int_op(are_responsible_hsdirs_equal(), OP_EQ, 1);

  /* Everything went well. */
  ret = 0;

 done:
  cleanup_reachability_test();
  if (ret == -1) {
    /* Do this so we can know which scenario failed. */
    char msg[32];
    tor_snprintf(msg, sizeof(msg), "Scenario %d failed", num_scenario);
    tt_fail_msg(msg);
  }
  return ret;
}

static void
test_reachability(void *arg)
{
  (void) arg;

  /* NOTE: An important axiom to understand here is that SRV#N must only be
   * used with TP#N value. For example, SRV#2 with TP#1 should NEVER be used
   * together. The HSDir index computation is based on this axiom.*/

  for (int i = 0; reachability_scenarios[i].service_valid_after; ++i) {
    int ret = run_reachability_scenario(&reachability_scenarios[i], i + 1);
    if (ret < 0) {
      return;
    }
  }
}

static void
test_blinding_basics(void *arg)
{
  (void)arg;
  char *mem_op_hex_tmp = NULL;
  const uint64_t time_period = 1234;
  ed25519_keypair_t keypair;

  time_t instant;
  tt_int_op(0, OP_EQ, parse_iso_time("1973-05-20 01:50:33", &instant));
  tt_int_op(1440, OP_EQ, get_time_period_length()); // in minutes, remember.
  tt_int_op(time_period, OP_EQ, hs_get_time_period_num(instant));

  const char pubkey_hex[] =
    "833990B085C1A688C1D4C8B1F6B56AFAF5A2ECA674449E1D704F83765CCB7BC6";
  const char seckey_hex[] =
    "D8C7FF0E31295B66540D789AF3E3DF992038A9592EEA01D8B7CBA06D6E66D159"
    "4D6167696320576F7264733A20737065697373636F62616C742062697669756D";
  base16_decode((char*)keypair.pubkey.pubkey, sizeof(keypair.pubkey.pubkey),
                pubkey_hex, strlen(pubkey_hex));
  base16_decode((char*)keypair.seckey.seckey, sizeof(keypair.seckey.seckey),
                seckey_hex, strlen(seckey_hex));

  uint64_t period_len = get_time_period_length();
  tt_u64_op(period_len, OP_EQ, 1440);
  uint8_t params[32];
  build_blinded_key_param(&keypair.pubkey, NULL, 0,
                          time_period, 1440,
                          params);
  test_memeq_hex(params,
                 "379E50DB31FEE6775ABD0AF6FB7C371E"
                 "060308F4F847DB09FE4CFE13AF602287");

  ed25519_public_key_t blinded_public;
  hs_build_blinded_pubkey(&keypair.pubkey, NULL, 0, time_period,
                          &blinded_public);
  hs_subcredential_t subcred;
  hs_get_subcredential(&keypair.pubkey, &blinded_public, &subcred);

  test_memeq_hex(blinded_public.pubkey,
                 "3A50BF210E8F9EE955AE0014F7A6917F"
                 "B65EBF098A86305ABB508D1A7291B6D5");
  test_memeq_hex(subcred.subcred,
                 "635D55907816E8D76398A675A50B1C2F"
                 "3E36B42A5CA77BA3A0441285161AE07D");

  ed25519_keypair_t blinded_keypair;
  hs_build_blinded_keypair(&keypair, NULL, 0, time_period,
                           &blinded_keypair);
  tt_mem_op(blinded_public.pubkey, OP_EQ, blinded_keypair.pubkey.pubkey,
            ED25519_PUBKEY_LEN);
  test_memeq_hex(blinded_keypair.seckey.seckey,
                 "A958DC83AC885F6814C67035DE817A2C"
                 "604D5D2F715282079448F789B656350B"
                 "4540FE1F80AA3F7E91306B7BF7A8E367"
                 "293352B14A29FDCC8C19F3558075524B");

 done:
  tor_free(mem_op_hex_tmp);
}

/** Pick an HSDir for service with <b>onion_identity_pk</b> as a client. Put
 *  its identity digest in <b>hsdir_digest_out</b>. */
static void
helper_client_pick_hsdir(const ed25519_public_key_t *onion_identity_pk,
                        char *hsdir_digest_out)
{
  tt_assert(onion_identity_pk);

  routerstatus_t *client_hsdir = pick_hsdir_v3(onion_identity_pk);
  tt_assert(client_hsdir);
  digest_to_base64(hsdir_digest_out, client_hsdir->identity_digest);

 done:
  ;
}

static void
test_hs_indexes(void *arg)
{
  int ret;
  uint64_t period_num = 42;
  ed25519_public_key_t pubkey;

  (void) arg;

  /* Build the hs_index */
  {
    uint8_t hs_index[DIGEST256_LEN];
    const char *b32_test_vector =
      "37e5cbbd56a22823714f18f1623ece5983a0d64c78495a8cfab854245e5f9a8a";
    char test_vector[DIGEST256_LEN];
    ret = base16_decode(test_vector, sizeof(test_vector), b32_test_vector,
                        strlen(b32_test_vector));
    tt_int_op(ret, OP_EQ, sizeof(test_vector));
    /* Our test vector uses a public key set to 32 bytes of \x42. */
    memset(&pubkey, '\x42', sizeof(pubkey));
    hs_build_hs_index(1, &pubkey, period_num, hs_index);
    tt_mem_op(hs_index, OP_EQ, test_vector, sizeof(hs_index));
  }

  /* Build the hsdir_index */
  {
    uint8_t srv[DIGEST256_LEN];
    uint8_t hsdir_index[DIGEST256_LEN];
    const char *b32_test_vector =
      "db475361014a09965e7e5e4d4a25b8f8d4b8f16cb1d8a7e95eed50249cc1a2d5";
    char test_vector[DIGEST256_LEN];
    ret = base16_decode(test_vector, sizeof(test_vector), b32_test_vector,
                        strlen(b32_test_vector));
    tt_int_op(ret, OP_EQ, sizeof(test_vector));
    /* Our test vector uses a public key set to 32 bytes of \x42. */
    memset(&pubkey, '\x42', sizeof(pubkey));
    memset(srv, '\x43', sizeof(srv));
    hs_build_hsdir_index(&pubkey, srv, period_num, hsdir_index);
    tt_mem_op(hsdir_index, OP_EQ, test_vector, sizeof(hsdir_index));
  }

 done:
  ;
}

#define EARLY_IN_SRV_TO_TP 0
#define LATE_IN_SRV_TO_TP 1
#define EARLY_IN_TP_TO_SRV 2
#define LATE_IN_TP_TO_SRV 3

/** Set the consensus and system time based on <b>position</b>. See the
 *    following diagram for details:
 *
 *  +------------------------------------------------------------------+
 *  |                                                                  |
 *  | 00:00      12:00       00:00       12:00       00:00       12:00 |
 *  | SRV#1      TP#1        SRV#2       TP#2        SRV#3       TP#3  |
 *  |                                                                  |
 *  |  $==========|-----------$===========|----------$===========|     |
 *  |                                                                  |
 *  |                                                                  |
 *  +------------------------------------------------------------------+
 */
static time_t
helper_set_consensus_and_system_time(networkstatus_t *ns, int position)
{
  time_t real_time = 0;

  /* The period between SRV#N and TP#N is from 00:00 to 12:00 UTC. Consensus
   * valid_after is what matters here, the rest is just to specify the voting
   * period correctly. */
  if (position == LATE_IN_SRV_TO_TP) {
    parse_rfc1123_time("Wed, 13 Apr 2016 11:00:00 UTC", &ns->valid_after);
    parse_rfc1123_time("Wed, 13 Apr 2016 12:00:00 UTC", &ns->fresh_until);
    parse_rfc1123_time("Wed, 13 Apr 2016 14:00:00 UTC", &ns->valid_until);
  } else if (position == EARLY_IN_TP_TO_SRV) {
    parse_rfc1123_time("Wed, 13 Apr 2016 13:00:00 UTC", &ns->valid_after);
    parse_rfc1123_time("Wed, 13 Apr 2016 14:00:00 UTC", &ns->fresh_until);
    parse_rfc1123_time("Wed, 13 Apr 2016 16:00:00 UTC", &ns->valid_until);
  } else if (position == LATE_IN_TP_TO_SRV) {
    parse_rfc1123_time("Wed, 13 Apr 2016 23:00:00 UTC", &ns->valid_after);
    parse_rfc1123_time("Wed, 14 Apr 2016 00:00:00 UTC", &ns->fresh_until);
    parse_rfc1123_time("Wed, 14 Apr 2016 02:00:00 UTC", &ns->valid_until);
  } else if (position == EARLY_IN_SRV_TO_TP) {
    parse_rfc1123_time("Wed, 14 Apr 2016 01:00:00 UTC", &ns->valid_after);
    parse_rfc1123_time("Wed, 14 Apr 2016 02:00:00 UTC", &ns->fresh_until);
    parse_rfc1123_time("Wed, 14 Apr 2016 04:00:00 UTC", &ns->valid_until);
  } else {
    tt_assert(0);
  }
  dirauth_sched_recalculate_timing(get_options(), ns->valid_after);

  /* Set system time: pretend to be just 2 minutes before consensus expiry */
  real_time = ns->valid_until - 120;
  update_approx_time(real_time);

 done:
  return real_time;
}

/** Helper function that carries out the actual test for
 *  test_client_service_sync() */
static void
helper_test_hsdir_sync(networkstatus_t *ns,
                       int service_position, int client_position,
                       int client_fetches_next_desc)
{
  hs_service_descriptor_t *desc;
  int retval;

  /** Test logic:
   *   1) Initialize service time: consensus and system time.
   *   1.1) Initialize service hash ring
   *   2) Initialize service and publish descriptors.
   *   3) Initialize client time: consensus and system time.
   *   3.1) Initialize client hash ring
   *   4) Try to fetch descriptor as client, and CHECK that the HSDir picked by
   *      the client was also picked by service.
   */

  /* 1) Initialize service time: consensus and real time */
  time_t now = helper_set_consensus_and_system_time(ns, service_position);
  helper_initialize_big_hash_ring(ns);

  /* 2) Initialize service */
  hs_service_t *service = helper_init_service(now);
  desc = client_fetches_next_desc ? service->desc_next : service->desc_current;

  /* Now let's upload our desc to all hsdirs */
  upload_descriptor_to_all(service, desc);
  /* Cleanup right now so we don't memleak on error. */
  cleanup_nodelist();
  /* Check that previous hsdirs were populated */
  tt_int_op(smartlist_len(desc->previous_hsdirs), OP_EQ, 8);

  /* 3) Initialize client time */
  helper_set_consensus_and_system_time(ns, client_position);

  cleanup_nodelist();
  SMARTLIST_FOREACH(ns->routerstatus_list,
                    routerstatus_t *, rs, routerstatus_free(rs));
  smartlist_clear(ns->routerstatus_list);
  helper_initialize_big_hash_ring(ns);

  /* 4) Pick 6 HSDirs as a client and check that they were also chosen by the
        service. */
  for (int y = 0 ; y < 6 ; y++) {
    char client_hsdir_b64_digest[BASE64_DIGEST_LEN+1] = {0};
    helper_client_pick_hsdir(&service->keys.identity_pk,
                             client_hsdir_b64_digest);

    /* CHECK: Go through the hsdirs chosen by the service and make sure that it
     * contains the one picked by the client! */
    retval = smartlist_contains_string(desc->previous_hsdirs,
                                       client_hsdir_b64_digest);
    tt_int_op(retval, OP_EQ, 1);
  }

  /* Finally, try to pick a 7th hsdir and see that NULL is returned since we
   * exhausted all of them: */
  tt_assert(!pick_hsdir_v3(&service->keys.identity_pk));

 done:
  /* At the end: free all services and initialize the subsystem again, we will
   * need it for next scenario. */
  cleanup_nodelist();
  hs_service_free_all();
  hs_service_init();
  SMARTLIST_FOREACH(ns->routerstatus_list,
                    routerstatus_t *, rs, routerstatus_free(rs));
  smartlist_clear(ns->routerstatus_list);
}

/** This test ensures that client and service will pick the same HSDirs, under
 *  various timing scenarios:
 *  a) Scenario where both client and service are in the time segment between
 *     SRV#N and TP#N:
 *  b) Scenario where both client and service are in the time segment between
 *     TP#N and SRV#N+1.
 *  c) Scenario where service is between SRV#N and TP#N, but client is between
 *     TP#N and SRV#N+1.
 *  d) Scenario where service is between TP#N and SRV#N+1, but client is
 *     between SRV#N and TP#N.
 *
 * This test is important because it tests that upload_descriptor_to_all() is
 * in synch with pick_hsdir_v3(). That's not the case for the
 * test_reachability() test which only compares the responsible hsdir sets.
 */
static void
test_client_service_hsdir_set_sync(void *arg)
{
  networkstatus_t *ns = NULL;

  (void) arg;

  MOCK(networkstatus_get_latest_consensus,
       mock_networkstatus_get_latest_consensus);
  MOCK(networkstatus_get_reasonably_live_consensus,
       mock_networkstatus_get_reasonably_live_consensus);
  MOCK(get_or_state,
       get_or_state_replacement);
  MOCK(hs_desc_encode_descriptor,
       mock_hs_desc_encode_descriptor);
  MOCK(directory_initiate_request,
       mock_directory_initiate_request);

  hs_init();

  /* Initialize a big hash ring: we want it to be big so that client and
   * service cannot accidentally select the same HSDirs */
  ns = networkstatus_get_latest_consensus();
  tt_assert(ns);

  /** Now test the various synch scenarios. See the helper function for more
      details: */

  /*  a) Scenario where both client and service are in the time segment between
   *     SRV#N and TP#N. At this time the client fetches the first HS desc:
   *
   *  +------------------------------------------------------------------+
   *  |                                                                  |
   *  | 00:00      12:00       00:00       12:00       00:00       12:00 |
   *  | SRV#1      TP#1        SRV#2       TP#2        SRV#3       TP#3  |
   *  |                                                                  |
   *  |  $==========|-----------$===========|----------$===========|     |
   *  |                                  ^ ^                             |
   *  |                                  S C                             |
   *  +------------------------------------------------------------------+
   */
  helper_test_hsdir_sync(ns, LATE_IN_SRV_TO_TP, LATE_IN_SRV_TO_TP, 0);

  /*  b) Scenario where both client and service are in the time segment between
   *     TP#N and SRV#N+1. At this time the client fetches the second HS
   *     desc:
   *
   *  +------------------------------------------------------------------+
   *  |                                                                  |
   *  | 00:00      12:00       00:00       12:00       00:00       12:00 |
   *  | SRV#1      TP#1        SRV#2       TP#2        SRV#3       TP#3  |
   *  |                                                                  |
   *  |  $==========|-----------$===========|-----------$===========|    |
   *  |                      ^ ^                                         |
   *  |                      S C                                         |
   *  +------------------------------------------------------------------+
   */
  helper_test_hsdir_sync(ns, LATE_IN_TP_TO_SRV, LATE_IN_TP_TO_SRV, 1);

  /*  c) Scenario where service is between SRV#N and TP#N, but client is
   *     between TP#N and SRV#N+1. Client is forward in time so it fetches the
   *     second HS desc.
   *
   *  +------------------------------------------------------------------+
   *  |                                                                  |
   *  | 00:00      12:00       00:00       12:00       00:00       12:00 |
   *  | SRV#1      TP#1        SRV#2       TP#2        SRV#3       TP#3  |
   *  |                                                                  |
   *  |  $==========|-----------$===========|-----------$===========|    |
   *  |                                    ^ ^                           |
   *  |                                    S C                           |
   *  +------------------------------------------------------------------+
   */
  helper_test_hsdir_sync(ns, LATE_IN_SRV_TO_TP, EARLY_IN_TP_TO_SRV, 1);

  /*  d) Scenario where service is between TP#N and SRV#N+1, but client is
   *     between SRV#N and TP#N. Client is backwards in time so it fetches the
   *     first HS desc.
   *
   *  +------------------------------------------------------------------+
   *  |                                                                  |
   *  | 00:00      12:00       00:00       12:00       00:00       12:00 |
   *  | SRV#1      TP#1        SRV#2       TP#2        SRV#3       TP#3  |
   *  |                                                                  |
   *  |  $==========|-----------$===========|-----------$===========|    |
   *  |                                    ^ ^                           |
   *  |                                    C S                           |
   *  +------------------------------------------------------------------+
   */
  helper_test_hsdir_sync(ns, EARLY_IN_TP_TO_SRV, LATE_IN_SRV_TO_TP, 0);

  /*  e) Scenario where service is between SRV#N and TP#N, but client is
   *     between TP#N-1 and SRV#3. Client is backwards in time so it fetches
   *     the first HS desc.
   *
   *  +------------------------------------------------------------------+
   *  |                                                                  |
   *  | 00:00      12:00       00:00       12:00       00:00       12:00 |
   *  | SRV#1      TP#1        SRV#2       TP#2        SRV#3       TP#3  |
   *  |                                                                  |
   *  |  $==========|-----------$===========|-----------$===========|    |
   *  |                        ^ ^                                       |
   *  |                        C S                                       |
   *  +------------------------------------------------------------------+
   */
  helper_test_hsdir_sync(ns, EARLY_IN_SRV_TO_TP, LATE_IN_TP_TO_SRV, 0);

  /*  f) Scenario where service is between TP#N and SRV#N+1, but client is
   *     between SRV#N+1 and TP#N+1. Client is forward in time so it fetches
   *     the second HS desc.
   *
   *  +------------------------------------------------------------------+
   *  |                                                                  |
   *  | 00:00      12:00       00:00       12:00       00:00       12:00 |
   *  | SRV#1      TP#1        SRV#2       TP#2        SRV#3       TP#3  |
   *  |                                                                  |
   *  |  $==========|-----------$===========|-----------$===========|    |
   *  |                        ^ ^                                       |
   *  |                        S C                                       |
   *  +------------------------------------------------------------------+
   */
  helper_test_hsdir_sync(ns, LATE_IN_TP_TO_SRV, EARLY_IN_SRV_TO_TP, 1);

 done:
  networkstatus_vote_free(ns);
  nodelist_free_all();
  hs_free_all();
}

struct testcase_t hs_common_tests[] = {
  { "blinding_basics", test_blinding_basics, TT_FORK, NULL, NULL },
  { "build_address", test_build_address, TT_FORK,
    NULL, NULL },
  { "validate_address", test_validate_address, TT_FORK,
    NULL, NULL },
  { "time_period", test_time_period, TT_FORK,
    NULL, NULL },
  { "start_time_of_next_time_period", test_start_time_of_next_time_period,
    TT_FORK, NULL, NULL },
  { "responsible_hsdirs", test_responsible_hsdirs, TT_FORK,
    NULL, NULL },
  { "desc_reupload_logic", test_desc_reupload_logic, TT_FORK,
    NULL, NULL },
  { "disaster_srv", test_disaster_srv, TT_FORK,
    NULL, NULL },
  { "hid_serv_request_tracker", test_hid_serv_request_tracker, TT_FORK,
    NULL, NULL },
  { "parse_extended_hostname", test_parse_extended_hostname, TT_FORK,
    NULL, NULL },
  { "time_between_tp_and_srv", test_time_between_tp_and_srv, TT_FORK,
    NULL, NULL },
  { "reachability", test_reachability, TT_FORK,
    NULL, NULL },
  { "client_service_hsdir_set_sync", test_client_service_hsdir_set_sync,
    TT_FORK, NULL, NULL },
  { "hs_indexes", test_hs_indexes, TT_FORK,
    NULL, NULL },

  END_OF_TESTCASES
};
