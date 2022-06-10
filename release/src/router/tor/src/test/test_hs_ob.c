/* Copyright (c) 2020-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file test_hs_ob.c
 * \brief Test hidden service onion balance functionality.
 */

#define CONFIG_PRIVATE
#define HS_SERVICE_PRIVATE
#define HS_OB_PRIVATE

#include "test/test.h"
#include "test/test_helpers.h"
#include "test/log_test_helpers.h"

#include "app/config/config.h"
#include "feature/hs/hs_config.h"
#include "feature/hs/hs_ob.h"
#include "feature/hs/hs_service.h"
#include "feature/nodelist/networkstatus.h"
#include "feature/nodelist/networkstatus_st.h"

static ed25519_keypair_t onion_addr_kp_1;
static char onion_addr_1[HS_SERVICE_ADDR_LEN_BASE32 + 1];

static ed25519_keypair_t onion_addr_kp_2;
static char onion_addr_2[HS_SERVICE_ADDR_LEN_BASE32 + 1];

static bool config_is_good = true;

static int
helper_tor_config(const char *conf)
{
  int ret = -1;
  or_options_t *options = helper_parse_options(conf);
  tt_assert(options);
  ret = hs_config_service_all(options, 0);
 done:
  or_options_free(options);
  return ret;
}

static networkstatus_t mock_ns;

static networkstatus_t *
mock_networkstatus_get_live_consensus(time_t now)
{
  (void) now;
  return &mock_ns;
}

static char *
mock_read_file_to_str(const char *filename, int flags, struct stat *stat_out)
{
  char *ret = NULL;

  (void) flags;
  (void) stat_out;

  if (!strcmp(filename, get_fname("hs3" PATH_SEPARATOR "ob_config"))) {
    if (config_is_good) {
      tor_asprintf(&ret, "MasterOnionAddress %s.onion\n"
                         "MasterOnionAddress %s.onion\n",
                         onion_addr_1, onion_addr_2);
    } else {
      tor_asprintf(&ret, "MasterOnionAddress JUNKJUNKJUNK.onion\n"
                         "UnknownOption BLAH\n");
    }
    goto done;
  }

 done:
  return ret;
}

static void
test_parse_config_file(void *arg)
{
  int ret;
  char *conf = NULL;
  const ed25519_public_key_t *pkey;

  (void) arg;

  hs_init();

  MOCK(read_file_to_str, mock_read_file_to_str);

#define fmt_conf            \
  "HiddenServiceDir %s\n"   \
  "HiddenServicePort 22\n"  \
  "HiddenServiceOnionBalanceInstance 1\n"
  tor_asprintf(&conf, fmt_conf, get_fname("hs3"));
#undef fmt_conf

  /* Build the OB frontend onion addresses. */
  ed25519_keypair_generate(&onion_addr_kp_1, 0);
  hs_build_address(&onion_addr_kp_1.pubkey, HS_VERSION_THREE, onion_addr_1);
  ed25519_keypair_generate(&onion_addr_kp_2, 0);
  hs_build_address(&onion_addr_kp_2.pubkey, HS_VERSION_THREE, onion_addr_2);

  ret = helper_tor_config(conf);
  tor_free(conf);
  tt_int_op(ret, OP_EQ, 0);

  /* Load the keys for the service. After that, the v3 service should be
   * registered in the global map and we'll be able to access it. */
  tt_int_op(get_hs_service_staging_list_size(), OP_EQ, 1);
  hs_service_load_all_keys();
  tt_int_op(get_hs_service_map_size(), OP_EQ, 1);
  const hs_service_t *s = get_first_service();
  tt_assert(s);
  tt_assert(s->config.ob_master_pubkeys);
  tt_assert(hs_ob_service_is_instance(s));
  tt_assert(smartlist_len(s->config.ob_master_pubkeys) == 2);

  /* Test the public keys we've added. */
  pkey = smartlist_get(s->config.ob_master_pubkeys, 0);
  tt_mem_op(&onion_addr_kp_1.pubkey, OP_EQ, pkey, ED25519_PUBKEY_LEN);
  pkey = smartlist_get(s->config.ob_master_pubkeys, 1);
  tt_mem_op(&onion_addr_kp_2.pubkey, OP_EQ, pkey, ED25519_PUBKEY_LEN);

 done:
  hs_free_all();

  UNMOCK(read_file_to_str);
}

static void
test_parse_config_file_bad(void *arg)
{
  int ret;
  char *conf = NULL;

  (void) arg;

  hs_init();

  MOCK(read_file_to_str, mock_read_file_to_str);

  /* Indicate mock_read_file_to_str() to use the bad config. */
  config_is_good = false;

#define fmt_conf            \
  "HiddenServiceDir %s\n"   \
  "HiddenServicePort 22\n"  \
  "HiddenServiceOnionBalanceInstance 1\n"
  tor_asprintf(&conf, fmt_conf, get_fname("hs3"));
#undef fmt_conf

  setup_full_capture_of_logs(LOG_INFO);
  ret = helper_tor_config(conf);
  tor_free(conf);
  tt_int_op(ret, OP_EQ, -1);
  expect_log_msg_containing("OnionBalance: MasterOnionAddress "
                            "JUNKJUNKJUNK.onion is invalid");
  expect_log_msg_containing("Found unrecognized option \'UnknownOption\'; "
                            "saving it.");
  teardown_capture_of_logs();

 done:
  hs_free_all();

  UNMOCK(read_file_to_str);
}

static void
test_get_subcredentials(void *arg)
{
  int ret;
  hs_service_t *service = NULL;
  hs_service_config_t config;
  hs_subcredential_t *subcreds = NULL;

  (void) arg;
  memset(&config, 0, sizeof(config));

  MOCK(networkstatus_get_live_consensus,
       mock_networkstatus_get_live_consensus);

  /* Setup consensus with proper time so we can compute the time period. */
  ret = parse_rfc1123_time("Sat, 26 Oct 1985 13:00:00 UTC",
                           &mock_ns.valid_after);
  tt_int_op(ret, OP_EQ, 0);
  ret = parse_rfc1123_time("Sat, 26 Oct 1985 14:00:00 UTC",
                           &mock_ns.fresh_until);
  tt_int_op(ret, OP_EQ, 0);

  config.ob_master_pubkeys = smartlist_new();
  tt_assert(config.ob_master_pubkeys);

  /* Set up an instance */
  service = tor_malloc_zero(sizeof(hs_service_t));
  service->config = config;
  /* Setup the service descriptors */
  service->desc_current = service_descriptor_new();
  service->desc_next = service_descriptor_new();

  /* First try to compute subcredentials but with no OB keys. Make sure that
   * subcreds get NULLed. To do this check we first poison subcreds. */
  subcreds = (void*)999;
  tt_ptr_op(subcreds, OP_NE, NULL);
  size_t num = compute_subcredentials(service, &subcreds);
  tt_ptr_op(subcreds, OP_EQ, NULL);

  /* Generate a keypair to add to the OB keys list. */
  ed25519_keypair_generate(&onion_addr_kp_1, 0);
  smartlist_add(config.ob_master_pubkeys, &onion_addr_kp_1.pubkey);

  /* Set up the instance subcredentials */
  char current_subcred[SUBCRED_LEN];
  char next_subcred[SUBCRED_LEN];
  memset(current_subcred, 'C', SUBCRED_LEN);
  memset(next_subcred, 'N', SUBCRED_LEN);
  memcpy(service->desc_current->desc->subcredential.subcred, current_subcred,
         SUBCRED_LEN);
  memcpy(service->desc_next->desc->subcredential.subcred, next_subcred,
         SUBCRED_LEN);

  /* See that subcreds are computed properly */
  num = compute_subcredentials(service, &subcreds);
  /* 5 subcredentials: 3 for the frontend, 2 for the instance */
  tt_uint_op(num, OP_EQ, 5);
  tt_ptr_op(subcreds, OP_NE, NULL);

  /* Validate the subcredentials we just got. We'll build them oursevles with
   * the right time period steps and compare. */
  const uint64_t tp = hs_get_time_period_num(0);
  const int steps[3] = {0, -1, 1};

  unsigned int i;
  for (i = 0; i < 3; i++) {
    hs_subcredential_t subcredential;
    ed25519_public_key_t blinded_pubkey;
    hs_build_blinded_pubkey(&onion_addr_kp_1.pubkey, NULL, 0, tp + steps[i],
                            &blinded_pubkey);
    hs_get_subcredential(&onion_addr_kp_1.pubkey, &blinded_pubkey,
                         &subcredential);
    tt_mem_op(subcreds[i].subcred, OP_EQ, subcredential.subcred,
              SUBCRED_LEN);
  }

  tt_mem_op(subcreds[i++].subcred, OP_EQ, current_subcred, SUBCRED_LEN);
  tt_mem_op(subcreds[i++].subcred, OP_EQ, next_subcred, SUBCRED_LEN);

 done:
  tor_free(subcreds);

  smartlist_free(config.ob_master_pubkeys);
  if (service) {
    memset(&service->config, 0, sizeof(hs_service_config_t));
    hs_service_free(service);
  }

  UNMOCK(networkstatus_get_live_consensus);
}

struct testcase_t hs_ob_tests[] = {
  { "parse_config_file", test_parse_config_file, TT_FORK,
    NULL, NULL },
  { "parse_config_file_bad", test_parse_config_file_bad, TT_FORK,
    NULL, NULL },

  { "get_subcredentials", test_get_subcredentials, TT_FORK,
    NULL, NULL },

  END_OF_TESTCASES
};
