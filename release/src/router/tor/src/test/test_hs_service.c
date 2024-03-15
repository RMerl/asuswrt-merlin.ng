/* Copyright (c) 2016-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file test_hs_service.c
 * \brief Test hidden service functionality.
 */

#define CIRCUITBUILD_PRIVATE
#define CIRCUITLIST_PRIVATE
#define CONFIG_PRIVATE
#define CONNECTION_PRIVATE
#define CONNECTION_EDGE_PRIVATE
#define CRYPTO_PRIVATE
#define HS_COMMON_PRIVATE
#define HS_SERVICE_PRIVATE
#define HS_INTROPOINT_PRIVATE
#define HS_CIRCUIT_PRIVATE
#define MAINLOOP_PRIVATE
#define NETWORKSTATUS_PRIVATE
#define STATEFILE_PRIVATE
#define CHANNEL_OBJECT_PRIVATE
#define HS_CLIENT_PRIVATE
#define CRYPT_PATH_PRIVATE

#include "test/test.h"
#include "test/test_helpers.h"
#include "test/log_test_helpers.h"
#include "test/hs_test_helpers.h"

#include "core/or/or.h"
#include "app/config/config.h"
#include "app/config/statefile.h"
#include "core/crypto/hs_ntor.h"
#include "core/mainloop/connection.h"
#include "core/mainloop/mainloop.h"
#include "core/or/circuitbuild.h"
#include "core/or/circuitlist.h"
#include "core/or/circuituse.h"
#include "core/or/connection_edge.h"
#include "core/or/edge_connection_st.h"
#include "core/or/relay.h"
#include "core/or/versions.h"
#include "feature/dirauth/dirvote.h"
#include "feature/dirauth/shared_random_state.h"
#include "feature/dirauth/voting_schedule.h"
#include "feature/hs/hs_circuit.h"
#include "feature/hs/hs_circuitmap.h"
#include "feature/hs/hs_client.h"
#include "feature/hs/hs_common.h"
#include "feature/hs/hs_config.h"
#include "feature/hs/hs_ident.h"
#include "feature/hs/hs_ob.h"
#include "feature/hs/hs_cell.h"
#include "feature/hs/hs_intropoint.h"
#include "feature/hs/hs_metrics.h"
#include "feature/hs/hs_service.h"
#include "feature/nodelist/networkstatus.h"
#include "feature/nodelist/nodelist.h"
#include "lib/crypt_ops/crypto_rand.h"
#include "lib/fs/dir.h"

#include "core/or/cpath_build_state_st.h"
#include "core/or/crypt_path_st.h"
#include "core/or/crypt_path.h"
#include "feature/nodelist/networkstatus_st.h"
#include "feature/nodelist/node_st.h"
#include "core/or/origin_circuit_st.h"
#include "app/config/or_state_st.h"
#include "feature/nodelist/routerinfo_st.h"

/* Trunnel */
#include "trunnel/hs/cell_establish_intro.h"

#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

static networkstatus_t mock_ns;

static networkstatus_t *
mock_networkstatus_get_reasonably_live_consensus(time_t now, int flavor)
{
  (void) now;
  (void) flavor;
  return &mock_ns;
}

static networkstatus_t *
mock_networkstatus_get_reasonably_live_consensus_null(time_t now, int flavor)
{
  (void) now;
  (void) flavor;
  return NULL;
}

static or_state_t *dummy_state = NULL;

/* Mock function to get fake or state (used for rev counters) */
static or_state_t *
get_or_state_replacement(void)
{
  return dummy_state;
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

static size_t relay_payload_len;
static char relay_payload[RELAY_PAYLOAD_SIZE];

static int
mock_relay_send_command_from_edge(streamid_t stream_id, circuit_t *circ,
                                  uint8_t relay_command, const char *payload,
                                  size_t payload_len,
                                  crypt_path_t *cpath_layer,
                                  const char *filename, int lineno)
{
  (void) stream_id;
  (void) circ;
  (void) relay_command;
  (void) payload;
  (void) payload_len;
  (void) cpath_layer;
  (void) filename;
  (void) lineno;

  memcpy(relay_payload, payload, payload_len);
  relay_payload_len = payload_len;

  return 0;
}

static unsigned int num_intro_points = 0;
static unsigned int
mock_count_desc_circuit_established(const hs_service_descriptor_t *desc)
{
  (void) desc;
  return num_intro_points;
}

static int
mock_router_have_minimum_dir_info_false(void)
{
  return 0;
}

/* Helper: from a set of options in conf, configure a service which will add
 * it to the staging list of the HS subsystem. */
static int
helper_config_service(const char *conf)
{
  int ret = 0;
  or_options_t *options = NULL;
  tt_assert(conf);
  options = helper_parse_options(conf);
  tt_assert(options);
  ret = hs_config_service_all(options, 0);
 done:
  or_options_free(options);
  return ret;
}

/* Test: Ensure that setting up rendezvous circuits works correctly. */
static void
test_e2e_rend_circuit_setup(void *arg)
{
  ed25519_public_key_t service_pk;
  origin_circuit_t *or_circ;
  int retval;

  /** In this test we create a v3 prop224 service-side rendezvous circuit.
   *  We simulate an HS ntor key exchange with a client, and check that
   *  the circuit was setup correctly and is ready to accept rendezvous data */

  (void) arg;

  /* Now make dummy circuit */
  {
    or_circ = origin_circuit_new();

    or_circ->base_.purpose = CIRCUIT_PURPOSE_S_CONNECT_REND;

    or_circ->build_state = tor_malloc_zero(sizeof(cpath_build_state_t));
    or_circ->build_state->is_internal = 1;

    /* prop224: Setup hs conn identifier on the stream */
    ed25519_secret_key_t sk;
    tt_int_op(0, OP_EQ, ed25519_secret_key_generate(&sk, 0));
    tt_int_op(0, OP_EQ, ed25519_public_key_generate(&service_pk, &sk));

    or_circ->hs_ident = hs_ident_circuit_new(&service_pk);

    TO_CIRCUIT(or_circ)->state = CIRCUIT_STATE_OPEN;
  }

  /* Check number of hops */
  retval = cpath_get_n_hops(&or_circ->cpath);
  tt_int_op(retval, OP_EQ, 0);

  /* Setup the circuit: do the ntor key exchange */
  {
    uint8_t ntor_key_seed[DIGEST256_LEN] = {2};
    retval = hs_circuit_setup_e2e_rend_circ(or_circ, ntor_key_seed,
                                            sizeof(ntor_key_seed), 1);
    tt_int_op(retval, OP_EQ, 0);
  }

  /* See that a hop was added to the circuit's cpath */
  retval = cpath_get_n_hops(&or_circ->cpath);
  tt_int_op(retval, OP_EQ, 1);

  /* Check the digest algo */
  tt_int_op(crypto_digest_get_algorithm(or_circ->cpath->pvt_crypto.f_digest),
            OP_EQ, DIGEST_SHA3_256);
  tt_int_op(crypto_digest_get_algorithm(or_circ->cpath->pvt_crypto.b_digest),
            OP_EQ, DIGEST_SHA3_256);
  tt_assert(or_circ->cpath->pvt_crypto.f_crypto);
  tt_assert(or_circ->cpath->pvt_crypto.b_crypto);

  /* Ensure that circ purpose was changed */
  tt_int_op(or_circ->base_.purpose, OP_EQ, CIRCUIT_PURPOSE_S_REND_JOINED);

 done:
  circuit_free_(TO_CIRCUIT(or_circ));
}

/* Helper: Return a newly allocated and initialized origin circuit with
 * purpose and flags. A default HS identifier is set to an ed25519
 * authentication key for introduction point. */
static origin_circuit_t *
helper_create_origin_circuit(int purpose, int flags)
{
  origin_circuit_t *circ = NULL;

  circ = origin_circuit_init(purpose, flags);
  tor_assert(circ);
  circ->cpath = tor_malloc_zero(sizeof(crypt_path_t));
  circ->cpath->magic = CRYPT_PATH_MAGIC;
  circ->cpath->state = CPATH_STATE_OPEN;
  circ->cpath->package_window = circuit_initial_package_window();
  circ->cpath->deliver_window = CIRCWINDOW_START;
  circ->cpath->prev = circ->cpath;
  /* Random nonce. */
  crypto_rand(circ->cpath->prev->rend_circ_nonce, DIGEST_LEN);
  /* Create a default HS identifier. */
  circ->hs_ident = tor_malloc_zero(sizeof(hs_ident_circuit_t));

  return circ;
}

/* Helper: Return a newly allocated authorized client object with
 * and a newly generated public key. */
static hs_service_authorized_client_t *
helper_create_authorized_client(void)
{
  int ret;
  hs_service_authorized_client_t *client;
  curve25519_secret_key_t seckey;
  client = tor_malloc_zero(sizeof(hs_service_authorized_client_t));

  ret = curve25519_secret_key_generate(&seckey, 0);
  tt_int_op(ret, OP_EQ, 0);
  curve25519_public_key_generate(&client->client_pk, &seckey);

 done:
  return client;
}

/* Helper: Return a newly allocated authorized client object with the
 * same client name and the same public key as the given client. */
static hs_service_authorized_client_t *
helper_clone_authorized_client(const hs_service_authorized_client_t *client)
{
  hs_service_authorized_client_t *client_out;

  tor_assert(client);

  client_out = tor_malloc_zero(sizeof(hs_service_authorized_client_t));
  memcpy(client_out->client_pk.public_key,
         client->client_pk.public_key, CURVE25519_PUBKEY_LEN);

  return client_out;
}

/* Helper: Return a newly allocated service object with the identity keypair
 * sets and the current descriptor. Then register it to the global map.
 * Caller should use hs_free_all() to free this service or remove it from the
 * global map before freeing. */
static hs_service_t *
helper_create_service(void)
{
  /* Set a service for this circuit. */
  hs_service_t *service = hs_service_new(get_options());
  tor_assert(service);
  service->config.version = HS_VERSION_THREE;
  ed25519_secret_key_generate(&service->keys.identity_sk, 0);
  ed25519_public_key_generate(&service->keys.identity_pk,
                              &service->keys.identity_sk);
  service->desc_current = service_descriptor_new();
  tt_assert(service->desc_current);
  /* Register service to global map. */
  int ret = register_service(get_hs_service_map(), service);
  tt_int_op(ret, OP_EQ, 0);

 done:
  return service;
}

/* Helper: Deallocate a given service object, its child objects and
 * remove it from onion service map.
 * */
static void
helper_destroy_service(hs_service_t *service)
{
  if (!service)
    return;

  remove_service(get_hs_service_map(), service);

  hs_service_free(service);
}

/* Helper: Return a newly allocated service object with clients. */
static hs_service_t *
helper_create_service_with_clients(int num_clients)
{
  int i;
  hs_service_t *service = helper_create_service();
  tt_assert(service);
  service->config.clients = smartlist_new();

  for (i = 0; i < num_clients; i++) {
    hs_service_authorized_client_t *client;
    client = helper_create_authorized_client();
    smartlist_add(service->config.clients, client);
  }

 done:
  return service;
}

/* Helper: Return a newly allocated service intro point with two link
 * specifiers, one IPv4 and one legacy ID set to As. */
static hs_service_intro_point_t *
helper_create_service_ip(void)
{
  link_specifier_t *ls;
  hs_service_intro_point_t *ip = service_intro_point_new(NULL);
  tor_assert(ip);
  /* Add a first unused link specifier. */
  ls = link_specifier_new();
  link_specifier_set_ls_type(ls, LS_IPV4);
  smartlist_add(ip->base.link_specifiers, ls);
  /* Add a second link specifier used by a test. */
  ls = link_specifier_new();
  link_specifier_set_ls_type(ls, LS_LEGACY_ID);
  memset(link_specifier_getarray_un_legacy_id(ls), 'A',
         link_specifier_getlen_un_legacy_id(ls));
  smartlist_add(ip->base.link_specifiers, ls);

  return ip;
}

static void
test_load_keys(void *arg)
{
  int ret;
  char *conf = NULL;
  char *hsdir_v3 = tor_strdup(get_fname("hs3"));
  char addr[HS_SERVICE_ADDR_LEN_BASE32 + 1];

  (void) arg;

  /* We'll register one service then we'll load keys and validate that both
   * are in a correct state. */

  hs_init();

#define conf_fmt \
  "HiddenServiceDir %s\n" \
  "HiddenServiceVersion %d\n" \
  "HiddenServicePort 65535\n"

  /* v3 service. */
  tor_asprintf(&conf, conf_fmt, hsdir_v3, HS_VERSION_THREE);
  ret = helper_config_service(conf);
  tor_free(conf);
  tt_int_op(ret, OP_EQ, 0);
  /* It's in staging? */
  tt_int_op(get_hs_service_staging_list_size(), OP_EQ, 1);

#undef conf_fmt

  /* Load the keys for these. After that, the v3 service should be registered
   * in the global map. */
  hs_service_load_all_keys();
  tt_int_op(get_hs_service_map_size(), OP_EQ, 1);
  hs_service_t *s = get_first_service();
  tt_assert(s);

  /* Ok we have the service object. Validate few things. */
  tt_assert(!fast_mem_is_zero(s->onion_address, sizeof(s->onion_address)));
  tt_int_op(hs_address_is_valid(s->onion_address), OP_EQ, 1);
  tt_assert(!fast_mem_is_zero((char *) s->keys.identity_sk.seckey,
                             ED25519_SECKEY_LEN));
  tt_assert(!fast_mem_is_zero((char *) s->keys.identity_pk.pubkey,
                             ED25519_PUBKEY_LEN));
  /* Check onion address from identity key. */
  hs_build_address(&s->keys.identity_pk, s->config.version, addr);
  tt_int_op(hs_address_is_valid(addr), OP_EQ, 1);
  tt_str_op(addr, OP_EQ, s->onion_address);

 done:
  tor_free(hsdir_v3);
  hs_free_all();
}

static void
test_client_filename_is_valid(void *arg)
{
  (void) arg;

  /* Valid file name. */
  tt_assert(client_filename_is_valid("a.auth"));
  /* Valid file name with special character. */
  tt_assert(client_filename_is_valid("a-.auth"));
  /* Invalid extension. */
  tt_assert(!client_filename_is_valid("a.ath"));
  /* Nothing before the extension. */
  tt_assert(!client_filename_is_valid(".auth"));

 done:
  ;
}

static void
test_parse_authorized_client(void *arg)
{
  hs_service_authorized_client_t *client = NULL;

  (void) arg;

  /* Valid authorized client. */
  client = parse_authorized_client(
    "descriptor:x25519:dz4q5xqlb4ldnbs72iarrml4ephk3du4i7o2cgiva5lwr6wkquja");
  tt_assert(client);

  /* Wrong number of fields. */
  tt_assert(!parse_authorized_client("a:b:c:d:e"));
  /* Wrong auth type. */
  tt_assert(!parse_authorized_client(
    "x:x25519:dz4q5xqlb4ldnbs72iarrml4ephk3du4i7o2cgiva5lwr6wkquja"));
  /* Wrong key type. */
  tt_assert(!parse_authorized_client(
    "descriptor:x:dz4q5xqlb4ldnbs72iarrml4ephk3du4i7o2cgiva5lwr6wkquja"));
  /* Some malformed string. */
  tt_assert(!parse_authorized_client("descriptor:x25519:aa=="));
  tt_assert(!parse_authorized_client("descriptor:"));
  tt_assert(!parse_authorized_client("descriptor:x25519"));
  tt_assert(!parse_authorized_client("descriptor:x25519:"));
  tt_assert(!parse_authorized_client(""));

 done:
  service_authorized_client_free(client);
}

static char *
mock_read_file_to_str(const char *filename, int flags, struct stat *stat_out)
{
  char *ret = NULL;

  (void) flags;
  (void) stat_out;

  if (!strcmp(filename, get_fname("hs3" PATH_SEPARATOR
                                  "authorized_clients" PATH_SEPARATOR
                                  "client1.auth"))) {
    ret = tor_strdup("descriptor:x25519:"
                  "dz4q5xqlb4ldnbs72iarrml4ephk3du4i7o2cgiva5lwr6wkquja");
    goto done;
  }

  if (!strcmp(filename, get_fname("hs3" PATH_SEPARATOR
                                  "authorized_clients" PATH_SEPARATOR
                                  "dummy.xxx"))) {
    ret = tor_strdup("descriptor:x25519:"
                  "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx");
    goto done;
  }

  if (!strcmp(filename, get_fname("hs3" PATH_SEPARATOR
                                  "authorized_clients" PATH_SEPARATOR
                                  "client2.auth"))) {
    ret = tor_strdup("descriptor:x25519:"
                  "okoi2gml3wd6x7jganlk5d66xxyjgg24sxw4y7javx4giqr66zta");
    goto done;
  }

 done:
  return ret;
}

static smartlist_t *
mock_tor_listdir(const char *dirname)
{
  smartlist_t *file_list = smartlist_new();

  (void) dirname;

  smartlist_add(file_list, tor_strdup("client1.auth"));
  smartlist_add(file_list, tor_strdup("dummy.xxx"));
  smartlist_add(file_list, tor_strdup("client2.auth"));

  return file_list;
}

static void
test_load_keys_with_client_auth(void *arg)
{
  int ret;
  char *conf = NULL;
  smartlist_t *pubkey_b32_list = smartlist_new();
  char *hsdir_v3 = tor_strdup(get_fname("hs3"));
  hs_service_t *service;

  (void) arg;

  hs_init();
  smartlist_add(pubkey_b32_list, tor_strdup(
                "dz4q5xqlb4ldnbs72iarrml4ephk3du4i7o2cgiva5lwr6wkquja"));
  smartlist_add(pubkey_b32_list, tor_strdup(
                "okoi2gml3wd6x7jganlk5d66xxyjgg24sxw4y7javx4giqr66zta"));

#define conf_fmt \
  "HiddenServiceDir %s\n" \
  "HiddenServiceVersion %d\n" \
  "HiddenServicePort 65534\n"

  tor_asprintf(&conf, conf_fmt, hsdir_v3, HS_VERSION_THREE);
  ret = helper_config_service(conf);
  tor_free(conf);
  tt_int_op(ret, OP_EQ, 0);
  /* It's in staging? */
  tt_int_op(get_hs_service_staging_list_size(), OP_EQ, 1);

#undef conf_fmt

  MOCK(read_file_to_str, mock_read_file_to_str);
  MOCK(tor_listdir, mock_tor_listdir);

  /* Load the keys for these. After that, the v3 service should be registered
   * in the global map. */
  hs_service_load_all_keys();
  tt_int_op(get_hs_service_map_size(), OP_EQ, 1);

  service = get_first_service();
  tt_assert(service);
  tt_assert(service->config.clients);
  tt_int_op(smartlist_len(service->config.clients), OP_EQ,
            smartlist_len(pubkey_b32_list));

  /* Test that the keys in clients are correct. */
  SMARTLIST_FOREACH_BEGIN(pubkey_b32_list, char *, pubkey_b32) {

    curve25519_public_key_t pubkey;
    /* This flag will be set if the key is found in clients. */
    int is_found = 0;
    base32_decode((char *) pubkey.public_key, sizeof(pubkey.public_key),
                  pubkey_b32, strlen(pubkey_b32));

    SMARTLIST_FOREACH_BEGIN(service->config.clients,
                            hs_service_authorized_client_t *, client) {
      if (tor_memeq(&pubkey, &client->client_pk, sizeof(pubkey))) {
        is_found = 1;
        break;
      }
    } SMARTLIST_FOREACH_END(client);

    tt_assert(is_found);

  } SMARTLIST_FOREACH_END(pubkey_b32);

 done:
  SMARTLIST_FOREACH(pubkey_b32_list, char *, s, tor_free(s));
  smartlist_free(pubkey_b32_list);
  tor_free(hsdir_v3);
  hs_free_all();
  UNMOCK(read_file_to_str);
  UNMOCK(tor_listdir);
}

static void
test_access_service(void *arg)
{
  int ret;
  char *conf = NULL;
  char *hsdir_v3 = tor_strdup(get_fname("hs3"));
  hs_service_ht *global_map;
  hs_service_t *s = NULL;

  (void) arg;

  /* We'll register one service then we'll load keys and validate that both
   * are in a correct state. */

  hs_init();

#define conf_fmt \
  "HiddenServiceDir %s\n" \
  "HiddenServiceVersion %d\n" \
  "HiddenServicePort 65535\n"

  /* v3 service. */
  tor_asprintf(&conf, conf_fmt, hsdir_v3, HS_VERSION_THREE);
  ret = helper_config_service(conf);
  tor_free(conf);
  tt_int_op(ret, OP_EQ, 0);
  /* It's in staging? */
  tt_int_op(get_hs_service_staging_list_size(), OP_EQ, 1);

  /* Load the keys for these. After that, the v3 service should be registered
   * in the global map. */
  hs_service_load_all_keys();
  tt_int_op(get_hs_service_map_size(), OP_EQ, 1);
  s = get_first_service();
  tt_assert(s);
  global_map = get_hs_service_map();
  tt_assert(global_map);

  /* From here, we'll try the service accessors. */
  hs_service_t *query = find_service(global_map, &s->keys.identity_pk);
  tt_assert(query);
  tt_mem_op(query, OP_EQ, s, sizeof(hs_service_t));
  /* Remove service, check if it actually works and then put it back. */
  remove_service(global_map, s);
  hs_metrics_service_free(s);
  tt_int_op(get_hs_service_map_size(), OP_EQ, 0);
  query = find_service(global_map, &s->keys.identity_pk);
  tt_ptr_op(query, OP_EQ, NULL);

  /* Register back the service in the map. */
  ret = register_service(global_map, s);
  tt_int_op(ret, OP_EQ, 0);
  tt_int_op(get_hs_service_map_size(), OP_EQ, 1);
  /* Twice should fail. */
  hs_metrics_service_free(s); /* Avoid BUG() on metrics init. */
  ret = register_service(global_map, s);
  tt_int_op(ret, OP_EQ, -1);
  /* Remove service from map so we don't double free on cleanup. */
  remove_service(global_map, s);
  tt_int_op(get_hs_service_map_size(), OP_EQ, 0);
  query = find_service(global_map, &s->keys.identity_pk);
  tt_ptr_op(query, OP_EQ, NULL);
  /* Let's try to remove twice for fun. */
  setup_full_capture_of_logs(LOG_WARN);
  remove_service(global_map, s);
  expect_log_msg_containing("Could not find service in the global map");
  teardown_capture_of_logs();

 done:
  hs_service_free(s);
  tor_free(hsdir_v3);
  hs_free_all();
}

/** Test that we can create intro point objects, index them and find them */
static void
test_service_intro_point(void *arg)
{
  hs_service_t *service = NULL;
  hs_service_intro_point_t *ip = NULL;

  (void) arg;

  update_approx_time(1481621834);

  /* Test simple creation of an object. */
  {
    time_t now = approx_time();
    ip = helper_create_service_ip();
    tt_assert(ip);
    /* Make sure the authentication keypair is not zeroes. */
    tt_int_op(fast_mem_is_zero((const char *) &ip->auth_key_kp,
                              sizeof(ed25519_keypair_t)), OP_EQ, 0);
    /* The introduce2_max MUST be in that range. */
    tt_u64_op(ip->introduce2_max, OP_GE,
              INTRO_POINT_MIN_LIFETIME_INTRODUCTIONS);
    tt_u64_op(ip->introduce2_max, OP_LE,
              INTRO_POINT_MAX_LIFETIME_INTRODUCTIONS);
    /* Time to expire MUST also be in that range. We subtract 500 seconds
     * because there could be a gap between setting now and the time taken in
     * service_intro_point_new. On ARM and other older CPUs, it can be
     * surprisingly slow... */
    tt_u64_op(ip->time_to_expire, OP_GE,
              now + INTRO_POINT_LIFETIME_MIN_SECONDS - 500);
    /* We add 500 seconds, because this time we're testing against the
     * maximum allowed time. */
    tt_u64_op(ip->time_to_expire, OP_LE,
              now + INTRO_POINT_LIFETIME_MAX_SECONDS + 500);
    tt_assert(ip->replay_cache);
    tt_assert(ip->base.link_specifiers);
    /* By default, this is NOT a legacy object. */
    tt_int_op(ip->base.is_only_legacy, OP_EQ, 0);
  }

  /* Test functions that uses a service intropoints map with that previously
   * created object (non legacy). */
  {
    ed25519_public_key_t garbage = { {0} };
    hs_service_intro_point_t *query;

    service = hs_service_new(get_options());
    tt_assert(service);
    service->desc_current = service_descriptor_new();
    tt_assert(service->desc_current);
    /* Add intropoint to descriptor map. */
    service_intro_point_add(service->desc_current->intro_points.map, ip);
    query = service_intro_point_find(service, &ip->auth_key_kp.pubkey);
    tt_mem_op(query, OP_EQ, ip, sizeof(hs_service_intro_point_t));
    query = service_intro_point_find(service, &garbage);
    tt_ptr_op(query, OP_EQ, NULL);

    /* While at it, can I find the descriptor with the intro point? */
    hs_service_descriptor_t *desc_lookup =
      service_desc_find_by_intro(service, ip);
    tt_mem_op(service->desc_current, OP_EQ, desc_lookup,
              sizeof(hs_service_descriptor_t));

    /* Remove object from service descriptor and make sure it is out. */
    service_intro_point_remove(service, ip);
    query = service_intro_point_find(service, &ip->auth_key_kp.pubkey);
    tt_ptr_op(query, OP_EQ, NULL);
  }

 done:
  /* If the test succeed, this object is no longer referenced in the service
   * so we can free it without use after free. Else, it might explode because
   * it's still in the service descriptor map. */
  service_intro_point_free(ip);
  hs_service_free(service);
}

static node_t mock_node;
static const node_t *
mock_node_get_by_id(const char *digest)
{
  (void) digest;
  memset(mock_node.identity, 'A', DIGEST_LEN);
  /* Only return the matching identity of As */
  if (!tor_memcmp(mock_node.identity, digest, DIGEST_LEN)) {
    return &mock_node;
  }
  return NULL;
}

static void
test_helper_functions(void *arg)
{
  int ret;
  hs_service_t *service = NULL;
  hs_service_intro_point_t *ip = NULL;
  hs_ident_circuit_t ident;

  (void) arg;

  MOCK(node_get_by_id, mock_node_get_by_id);

  hs_service_init();
  time_t now = time(NULL);
  update_approx_time(now);

  service = helper_create_service();

  ip = helper_create_service_ip();
  /* Immediately add the intro point to the service so the free service at the
   * end cleans it as well. */
  service_intro_point_add(service->desc_current->intro_points.map, ip);

  /* Setup the circuit identifier. */
  ed25519_pubkey_copy(&ident.intro_auth_pk, &ip->auth_key_kp.pubkey);
  ed25519_pubkey_copy(&ident.identity_pk, &service->keys.identity_pk);

  /* Testing get_objects_from_ident(). */
  {
    hs_service_t *s_lookup = NULL;
    hs_service_intro_point_t *ip_lookup = NULL;
    hs_service_descriptor_t *desc_lookup = NULL;

    get_objects_from_ident(&ident, &s_lookup, &ip_lookup, &desc_lookup);
    tt_mem_op(s_lookup, OP_EQ, service, sizeof(hs_service_t));
    tt_mem_op(ip_lookup, OP_EQ, ip, sizeof(hs_service_intro_point_t));
    tt_mem_op(desc_lookup, OP_EQ, service->desc_current,
              sizeof(hs_service_descriptor_t));
    /* Reset */
    s_lookup = NULL; ip_lookup = NULL; desc_lookup = NULL;

    /* NULL parameter should work. */
    get_objects_from_ident(&ident, NULL, &ip_lookup, &desc_lookup);
    tt_mem_op(ip_lookup, OP_EQ, ip, sizeof(hs_service_intro_point_t));
    tt_mem_op(desc_lookup, OP_EQ, service->desc_current,
              sizeof(hs_service_descriptor_t));
    /* Reset. */
    s_lookup = NULL; ip_lookup = NULL; desc_lookup = NULL;

    /* Break the ident and we should find nothing. */
    memset(&ident, 0, sizeof(ident));
    get_objects_from_ident(&ident, &s_lookup, &ip_lookup, &desc_lookup);
    tt_ptr_op(s_lookup, OP_EQ, NULL);
    tt_ptr_op(ip_lookup, OP_EQ, NULL);
    tt_ptr_op(desc_lookup, OP_EQ, NULL);
  }

  /* Testing get_node_from_intro_point() */
  {
    const node_t *node = get_node_from_intro_point(ip);
    tt_ptr_op(node, OP_EQ, &mock_node);
    SMARTLIST_FOREACH_BEGIN(ip->base.link_specifiers,
                            link_specifier_t *, ls) {
      if (link_specifier_get_ls_type(ls) == LS_LEGACY_ID) {
        /* Change legacy id in link specifier which is not the mock node. */
        memset(link_specifier_getarray_un_legacy_id(ls), 'B',
               link_specifier_getlen_un_legacy_id(ls));
      }
    } SMARTLIST_FOREACH_END(ls);
    node = get_node_from_intro_point(ip);
    tt_ptr_op(node, OP_EQ, NULL);
  }

  /* Testing can_service_launch_intro_circuit() */
  {
    /* Put the start of the retry period back in time, we should be allowed.
     * to launch intro circuit. */
    service->state.num_intro_circ_launched = 2;
    service->state.intro_circ_retry_started_time =
      (now - INTRO_CIRC_RETRY_PERIOD - 1);
    ret = can_service_launch_intro_circuit(service, now);
    tt_int_op(ret, OP_EQ, 1);
    tt_u64_op(service->state.intro_circ_retry_started_time, OP_EQ, now);
    tt_u64_op(service->state.num_intro_circ_launched, OP_EQ, 0);
    /* Call it again, we should still be allowed because we are under
     * MAX_INTRO_CIRCS_PER_PERIOD which been set to 0 previously. */
    ret = can_service_launch_intro_circuit(service, now);
    tt_int_op(ret, OP_EQ, 1);
    tt_u64_op(service->state.intro_circ_retry_started_time, OP_EQ, now);
    tt_u64_op(service->state.num_intro_circ_launched, OP_EQ, 0);
    /* Too many intro circuit launched means we are not allowed. */
    service->state.num_intro_circ_launched = 20;
    ret = can_service_launch_intro_circuit(service, now);
    tt_int_op(ret, OP_EQ, 0);
  }

  /* Testing intro_point_should_expire(). */
  {
    /* Just some basic test of the current state. */
    tt_u64_op(ip->introduce2_max, OP_GE,
              INTRO_POINT_MIN_LIFETIME_INTRODUCTIONS);
    tt_u64_op(ip->introduce2_max, OP_LE,
              INTRO_POINT_MAX_LIFETIME_INTRODUCTIONS);
    tt_u64_op(ip->time_to_expire, OP_GE,
              now + INTRO_POINT_LIFETIME_MIN_SECONDS);
    tt_u64_op(ip->time_to_expire, OP_LE,
              now + INTRO_POINT_LIFETIME_MAX_SECONDS);

    /* This newly created IP from above shouldn't expire now. */
    ret = intro_point_should_expire(ip, now);
    tt_int_op(ret, OP_EQ, 0);
    /* Maximum number of INTRODUCE2 cell reached, it should expire. */
    ip->introduce2_count = INTRO_POINT_MAX_LIFETIME_INTRODUCTIONS + 1;
    ret = intro_point_should_expire(ip, now);
    tt_int_op(ret, OP_EQ, 1);
    ip->introduce2_count = 0;
    /* It should expire if time to expire has been reached. */
    ip->time_to_expire = now - 1000;
    ret = intro_point_should_expire(ip, now);
    tt_int_op(ret, OP_EQ, 1);
  }

 done:
  /* This will free the service and all objects associated to it. */
  if (service) {
    remove_service(get_hs_service_map(), service);
    hs_service_free(service);
  }
  hs_service_free_all();
  UNMOCK(node_get_by_id);
}

/** Test that we do the right operations when an intro circuit opens */
static void
test_intro_circuit_opened(void *arg)
{
  int flags = CIRCLAUNCH_NEED_UPTIME | CIRCLAUNCH_IS_INTERNAL;
  hs_service_t *service = NULL;
  origin_circuit_t *circ = NULL;

  (void) arg;

  hs_init();
  MOCK(circuit_mark_for_close_, mock_circuit_mark_for_close);
  MOCK(relay_send_command_from_edge_, mock_relay_send_command_from_edge);

  circ = helper_create_origin_circuit(CIRCUIT_PURPOSE_S_ESTABLISH_INTRO,
                                      flags);

  /* No service associated with this circuit. */
  setup_full_capture_of_logs(LOG_WARN);
  hs_service_circuit_has_opened(circ);
  expect_log_msg_containing("Unknown service identity key");
  teardown_capture_of_logs();

  /* Set a service for this circuit. */
  {
    service = helper_create_service();
    ed25519_pubkey_copy(&circ->hs_ident->identity_pk,
                        &service->keys.identity_pk);

    /* No intro point associated with this circuit. */
    setup_full_capture_of_logs(LOG_WARN);
    hs_service_circuit_has_opened(circ);
    expect_log_msg_containing("Unknown introduction point auth key");
    teardown_capture_of_logs();
  }

  /* Set an IP object now for this circuit. */
  {
    hs_service_intro_point_t *ip = helper_create_service_ip();
    service_intro_point_add(service->desc_current->intro_points.map, ip);
    /* Update ident to contain the intro point auth key. */
    ed25519_pubkey_copy(&circ->hs_ident->intro_auth_pk,
                        &ip->auth_key_kp.pubkey);
  }

  /* This one should go all the way. */
  setup_full_capture_of_logs(LOG_INFO);
  hs_service_circuit_has_opened(circ);
  expect_log_msg_containing("Introduction circuit 0 established for service");
  teardown_capture_of_logs();

 done:
  circuit_free_(TO_CIRCUIT(circ));
  if (service) {
    remove_service(get_hs_service_map(), service);
    hs_service_free(service);
  }
  hs_free_all();
  UNMOCK(circuit_mark_for_close_);
  UNMOCK(relay_send_command_from_edge_);
}

/** Test the operations we do on a circuit after we learn that we successfully
 *  established an intro point on it */
static void
test_intro_established(void *arg)
{
  int ret;
  int flags = CIRCLAUNCH_NEED_UPTIME | CIRCLAUNCH_IS_INTERNAL;
  uint8_t payload[RELAY_PAYLOAD_SIZE] = {0};
  origin_circuit_t *circ = NULL;
  hs_service_t *service = NULL;
  hs_service_intro_point_t *ip = NULL;

  (void) arg;

  hs_init();
  MOCK(circuit_mark_for_close_, mock_circuit_mark_for_close);

  circ = helper_create_origin_circuit(CIRCUIT_PURPOSE_S_ESTABLISH_INTRO,
                                      flags);
  tt_assert(circ);

  /* Test a wrong purpose. */
  TO_CIRCUIT(circ)->purpose = CIRCUIT_PURPOSE_S_INTRO;
  setup_full_capture_of_logs(LOG_WARN);
  ret = hs_service_receive_intro_established(circ, payload, sizeof(payload));
  tt_int_op(ret, OP_EQ, -1);
  expect_log_msg_containing("Received an INTRO_ESTABLISHED cell on a "
                            "non introduction circuit of purpose");
  teardown_capture_of_logs();

  /* Back to normal. */
  TO_CIRCUIT(circ)->purpose = CIRCUIT_PURPOSE_S_ESTABLISH_INTRO;

  /* No service associated to it. */
  setup_full_capture_of_logs(LOG_WARN);
  ret = hs_service_receive_intro_established(circ, payload, sizeof(payload));
  tt_int_op(ret, OP_EQ, -1);
  expect_log_msg_containing("Unknown service identity key");
  teardown_capture_of_logs();

  /* Set a service for this circuit. */
  service = helper_create_service();
  ed25519_pubkey_copy(&circ->hs_ident->identity_pk,
                      &service->keys.identity_pk);
  /* No introduction point associated to it. */
  setup_full_capture_of_logs(LOG_WARN);
  ret = hs_service_receive_intro_established(circ, payload, sizeof(payload));
  tt_int_op(ret, OP_EQ, -1);
  expect_log_msg_containing("Introduction circuit established without an "
                            "intro point object on circuit");
  teardown_capture_of_logs();

  /* Set an IP object now for this circuit. */
  {
    ip = helper_create_service_ip();
    service_intro_point_add(service->desc_current->intro_points.map, ip);
    /* Update ident to contain the intro point auth key. */
    ed25519_pubkey_copy(&circ->hs_ident->intro_auth_pk,
                        &ip->auth_key_kp.pubkey);
  }

  /* Send an empty payload. INTRO_ESTABLISHED cells are basically zeroes. */
  ret = hs_service_receive_intro_established(circ, payload, sizeof(payload));
  tt_int_op(ret, OP_EQ, 0);
  tt_int_op(TO_CIRCUIT(circ)->purpose, OP_EQ, CIRCUIT_PURPOSE_S_INTRO);

 done:
  if (circ)
    circuit_free_(TO_CIRCUIT(circ));
  if (service) {
    remove_service(get_hs_service_map(), service);
    hs_service_free(service);
  }
  hs_free_all();
  UNMOCK(circuit_mark_for_close_);
}

/** Check the operations we do on a rendezvous circuit after we learn it's
 *  open */
static void
test_rdv_circuit_opened(void *arg)
{
  int flags = CIRCLAUNCH_NEED_UPTIME | CIRCLAUNCH_IS_INTERNAL;
  origin_circuit_t *circ = NULL;
  hs_service_t *service = NULL;

  (void) arg;

  hs_init();
  MOCK(circuit_mark_for_close_, mock_circuit_mark_for_close);
  MOCK(relay_send_command_from_edge_, mock_relay_send_command_from_edge);

  circ = helper_create_origin_circuit(CIRCUIT_PURPOSE_S_CONNECT_REND, flags);
  crypto_rand((char *) circ->hs_ident->rendezvous_cookie, REND_COOKIE_LEN);
  crypto_rand((char *) circ->hs_ident->rendezvous_handshake_info,
              sizeof(circ->hs_ident->rendezvous_handshake_info));

  /* No service associated with this circuit. */
  setup_full_capture_of_logs(LOG_WARN);
  hs_service_circuit_has_opened(circ);
  expect_log_msg_containing("Unknown service identity key");
  teardown_capture_of_logs();
  /* This should be set to a non zero timestamp. */
  tt_u64_op(TO_CIRCUIT(circ)->timestamp_dirty, OP_NE, 0);

  /* Set a service for this circuit. */
  service = helper_create_service();
  ed25519_pubkey_copy(&circ->hs_ident->identity_pk,
                      &service->keys.identity_pk);
  /* Should be all good. */
  hs_service_circuit_has_opened(circ);
  tt_int_op(TO_CIRCUIT(circ)->purpose, OP_EQ, CIRCUIT_PURPOSE_S_REND_JOINED);

 done:
  circuit_free_(TO_CIRCUIT(circ));
  if (service) {
    remove_service(get_hs_service_map(), service);
    hs_service_free(service);
  }
  hs_free_all();
  UNMOCK(circuit_mark_for_close_);
  UNMOCK(relay_send_command_from_edge_);
}

static void
mock_assert_circuit_ok(const circuit_t *c)
{
  (void) c;
  return;
}

/** Test for the general mechanism for closing intro circs.
 *  Also a way to identify that #23603 has been fixed. */
static void
test_closing_intro_circs(void *arg)
{
  hs_service_t *service = NULL;
  hs_service_intro_point_t *ip = NULL, *entry = NULL;
  origin_circuit_t *intro_circ = NULL, *tmp_circ;
  int flags = CIRCLAUNCH_NEED_UPTIME | CIRCLAUNCH_IS_INTERNAL;

  (void) arg;

  MOCK(assert_circuit_ok, mock_assert_circuit_ok);

  hs_init();

  /* Initialize service */
  service = helper_create_service();
  /* Initialize intro point */
  ip = helper_create_service_ip();
  tt_assert(ip);
  service_intro_point_add(service->desc_current->intro_points.map, ip);

  /* Initialize intro circuit */
  intro_circ = origin_circuit_init(CIRCUIT_PURPOSE_S_ESTABLISH_INTRO, flags);
  intro_circ->hs_ident = hs_ident_circuit_new(&service->keys.identity_pk);
  /* Register circuit in the circuitmap . */
  hs_circuitmap_register_intro_circ_v3_service_side(intro_circ,
                                                    &ip->auth_key_kp.pubkey);
  tmp_circ =
    hs_circuitmap_get_intro_circ_v3_service_side(&ip->auth_key_kp.pubkey);
  tt_ptr_op(tmp_circ, OP_EQ, intro_circ);

  /* Pretend that intro point has failed too much */
  ip->circuit_retries = MAX_INTRO_POINT_CIRCUIT_RETRIES+1;

  /* Now pretend we are freeing this intro circuit. We want to see that our
   * destructor is not gonna kill our intro point structure since that's the
   * job of the cleanup routine. */
  circuit_free_(TO_CIRCUIT(intro_circ));
  intro_circ = NULL;
  entry = service_intro_point_find(service, &ip->auth_key_kp.pubkey);
  tt_assert(entry);
  /* The free should also remove the circuit from the circuitmap. */
  tmp_circ =
    hs_circuitmap_get_intro_circ_v3_service_side(&ip->auth_key_kp.pubkey);
  tt_assert(!tmp_circ);

  /* Now pretend that a new intro point circ was launched and opened. Check
   * that the intro point will be established correctly. */
  intro_circ = origin_circuit_init(CIRCUIT_PURPOSE_S_ESTABLISH_INTRO, flags);
  intro_circ->hs_ident = hs_ident_circuit_new(&service->keys.identity_pk);
  ed25519_pubkey_copy(&intro_circ->hs_ident->intro_auth_pk,
                      &ip->auth_key_kp.pubkey);
  /* Register circuit in the circuitmap . */
  hs_circuitmap_register_intro_circ_v3_service_side(intro_circ,
                                                    &ip->auth_key_kp.pubkey);
  tmp_circ =
    hs_circuitmap_get_intro_circ_v3_service_side(&ip->auth_key_kp.pubkey);
  tt_ptr_op(tmp_circ, OP_EQ, intro_circ);
  tt_int_op(TO_CIRCUIT(intro_circ)->marked_for_close, OP_EQ, 0);
  circuit_mark_for_close(TO_CIRCUIT(intro_circ), END_CIRC_REASON_INTERNAL);
  tt_int_op(TO_CIRCUIT(intro_circ)->marked_for_close, OP_NE, 0);
  /* At this point, we should not be able to find it in the circuitmap. */
  tmp_circ =
    hs_circuitmap_get_intro_circ_v3_service_side(&ip->auth_key_kp.pubkey);
  tt_assert(!tmp_circ);

 done:
  if (intro_circ) {
    circuit_free_(TO_CIRCUIT(intro_circ));
  }
  /* Frees the service object. */
  if (service) {
    remove_service(get_hs_service_map(), service);
    hs_service_free(service);
  }
  hs_free_all();
  UNMOCK(assert_circuit_ok);
}

/** Test sending and receiving introduce2 cells */
static void
test_bad_introduce2(void *arg)
{
  int ret;
  int flags = CIRCLAUNCH_NEED_UPTIME | CIRCLAUNCH_IS_INTERNAL;
  uint8_t payload[RELAY_PAYLOAD_SIZE] = {0};
  origin_circuit_t *circ = NULL;
  hs_service_t *service = NULL;
  hs_service_intro_point_t *ip = NULL;
  const smartlist_t *entries = NULL;
  const metrics_store_entry_t *entry = NULL;

  (void) arg;

  hs_init();
  MOCK(circuit_mark_for_close_, mock_circuit_mark_for_close);
  MOCK(get_or_state,
       get_or_state_replacement);

  dummy_state = or_state_new();

  circ = helper_create_origin_circuit(CIRCUIT_PURPOSE_S_INTRO, flags);
  tt_assert(circ);

  /* Test a wrong purpose. */
  TO_CIRCUIT(circ)->purpose = CIRCUIT_PURPOSE_S_ESTABLISH_INTRO;
  setup_full_capture_of_logs(LOG_WARN);
  ret = hs_service_receive_introduce2(circ, payload, sizeof(payload));
  tt_int_op(ret, OP_EQ, -1);
  expect_log_msg_containing("Received an INTRODUCE2 cell on a "
                            "non introduction circuit of purpose");
  teardown_capture_of_logs();

  /* Back to normal. */
  TO_CIRCUIT(circ)->purpose = CIRCUIT_PURPOSE_S_INTRO;

  /* No service associated to it. */
  setup_full_capture_of_logs(LOG_WARN);
  ret = hs_service_receive_introduce2(circ, payload, sizeof(payload));
  tt_int_op(ret, OP_EQ, -1);
  expect_log_msg_containing("Unknown service identity key");
  teardown_capture_of_logs();

  /* Set a service for this circuit. */
  service = helper_create_service();
  ed25519_pubkey_copy(&circ->hs_ident->identity_pk,
                      &service->keys.identity_pk);
  /* No introduction point associated to it. */
  setup_full_capture_of_logs(LOG_WARN);
  ret = hs_service_receive_introduce2(circ, payload, sizeof(payload));
  tt_int_op(ret, OP_EQ, -1);
  expect_log_msg_containing("Unknown introduction auth key when handling "
                            "an INTRODUCE2 cell on circuit");
  teardown_capture_of_logs();

  entries = metrics_store_get_all(service->metrics.store,
                                  "tor_hs_intro_rejected_intro_req_count");

  tt_assert(entries);
  /* There are `hs_metrics_intro_req_size` entries (one for each
   * possible `reason` label value). */
  tt_int_op(smartlist_len(entries), OP_EQ,
            hs_metrics_intro_req_error_reasons_size);

  /* Make sure the tor_hs_intro_rejected_intro_req_count metric was
   * only incremented for reason HS_METRICS_ERR_INTRO_REQ_BAD_AUTH_KEY. */
  for (size_t i = 0; i < hs_metrics_intro_req_error_reasons_size; ++i) {
    const char *reason = hs_metrics_intro_req_error_reasons[i];

    if (!strcmp(reason, HS_METRICS_ERR_INTRO_REQ_BAD_AUTH_KEY)) {
      continue;
    }

    entry = metrics_store_find_entry_with_label(
        entries,
        metrics_format_label("reason", reason));
    tt_assert(entry);
    tt_int_op(metrics_store_entry_get_value(entry), OP_EQ, 0);
  }

  entry = metrics_store_find_entry_with_label(
      entries,
      metrics_format_label("reason", HS_METRICS_ERR_INTRO_REQ_BAD_AUTH_KEY));
  tt_assert(entry);
  tt_int_op(metrics_store_entry_get_value(entry), OP_EQ, 1);

  /* Set an IP object now for this circuit. */
  {
    ip = helper_create_service_ip();
    service_intro_point_add(service->desc_current->intro_points.map, ip);
    /* Update ident to contain the intro point auth key. */
    ed25519_pubkey_copy(&circ->hs_ident->intro_auth_pk,
                        &ip->auth_key_kp.pubkey);
  }

  /* This will fail because receiving an INTRODUCE2 cell implies a valid cell
   * and then launching circuits so let's not do that and instead test that
   * behaviour differently. */
  ret = hs_service_receive_introduce2(circ, payload, sizeof(payload));
  tt_int_op(ret, OP_EQ, -1);
  tt_u64_op(ip->introduce2_count, OP_EQ, 0);

  /* Make sure the tor_hs_intro_rejected_intro_req_count metric was incremented
   * a second time, this time, with reason="invalid_introduce2_cell". */
  entry = metrics_store_find_entry_with_label(
      entries,
      metrics_format_label("reason", HS_METRICS_ERR_INTRO_REQ_INTRODUCE2));
  tt_assert(entry);
  tt_int_op(metrics_store_entry_get_value(entry), OP_EQ, 1);

  /* The metric entries with other reason labels are unaffected */
  entry = metrics_store_find_entry_with_label(
      entries,
      metrics_format_label("reason", HS_METRICS_ERR_INTRO_REQ_SUBCREDENTIAL));
  tt_assert(entry);
  tt_int_op(metrics_store_entry_get_value(entry), OP_EQ, 0);

  entry = metrics_store_find_entry_with_label(
      entries, metrics_format_label(
                   "reason", HS_METRICS_ERR_INTRO_REQ_INTRODUCE2_REPLAY));
  tt_assert(entry);
  tt_int_op(metrics_store_entry_get_value(entry), OP_EQ, 0);

  entry = metrics_store_find_entry_with_label(
      entries,
      metrics_format_label("reason", HS_METRICS_ERR_INTRO_REQ_BAD_AUTH_KEY));
  tt_assert(entry);
  tt_int_op(metrics_store_entry_get_value(entry), OP_EQ, 1);

 done:
  or_state_free(dummy_state);
  dummy_state = NULL;
  if (circ)
    circuit_free_(TO_CIRCUIT(circ));
  if (service) {
    remove_service(get_hs_service_map(), service);
    hs_service_free(service);
  }
  hs_free_all();
  UNMOCK(circuit_mark_for_close_);
}

/** Test basic hidden service housekeeping operations (maintaining intro
 *  points, etc) */
static void
test_service_event(void *arg)
{
  int flags = CIRCLAUNCH_NEED_UPTIME | CIRCLAUNCH_IS_INTERNAL;
  time_t now = time(NULL);
  hs_service_t *service;
  origin_circuit_t *circ = NULL;

  (void) arg;

  hs_init();
  MOCK(circuit_mark_for_close_, mock_circuit_mark_for_close);

  circ = helper_create_origin_circuit(CIRCUIT_PURPOSE_S_INTRO, flags);

  /* Set a service for this circuit. */
  service = helper_create_service();
  tt_assert(service);
  ed25519_pubkey_copy(&circ->hs_ident->identity_pk,
                      &service->keys.identity_pk);

  /* Currently this consists of cleaning invalid intro points. So adding IPs
   * here that should get cleaned up. */
  {
    hs_service_intro_point_t *ip = helper_create_service_ip();
    service_intro_point_add(service->desc_current->intro_points.map, ip);
    /* This run will remove the IP because we have no circuits nor node_t
     * associated with it. */
    run_housekeeping_event(now);
    tt_int_op(digest256map_size(service->desc_current->intro_points.map),
              OP_EQ, 0);
    /* We'll trigger a removal because we've reached our maximum amount of
     * times we should retry a circuit. For this, we need to have a node_t
     * that matches the identity of this IP. */
    routerinfo_t ri;
    memset(&ri, 0, sizeof(ri));
    ip = helper_create_service_ip();
    service_intro_point_add(service->desc_current->intro_points.map, ip);
    memset(ri.cache_info.identity_digest, 'A', DIGEST_LEN);
    /* This triggers a node_t creation. */
    tt_assert(nodelist_set_routerinfo(&ri, NULL));
    ip->circuit_retries = MAX_INTRO_POINT_CIRCUIT_RETRIES + 1;
    run_housekeeping_event(now);
    tt_int_op(digest256map_size(service->desc_current->intro_points.map),
              OP_EQ, 0);
    /* No removal but no circuit so this means the IP object will stay in the
     * descriptor map so we can retry it. */
    ip = helper_create_service_ip();
    service_intro_point_add(service->desc_current->intro_points.map, ip);
    run_housekeeping_event(now);
    tt_int_op(digest256map_size(service->desc_current->intro_points.map),
              OP_EQ, 1);
    /* Remove the IP object at once for the next test. */
    ip->circuit_retries = MAX_INTRO_POINT_CIRCUIT_RETRIES + 1;
    run_housekeeping_event(now);
    tt_int_op(digest256map_size(service->desc_current->intro_points.map),
              OP_EQ, 0);
    /* Now, we'll create an IP with a registered circuit. The IP object
     * shouldn't go away. */
    ip = helper_create_service_ip();
    service_intro_point_add(service->desc_current->intro_points.map, ip);
    ed25519_pubkey_copy(&circ->hs_ident->intro_auth_pk,
                        &ip->auth_key_kp.pubkey);
    hs_circuitmap_register_intro_circ_v3_service_side(
                                         circ, &ip->auth_key_kp.pubkey);
    run_housekeeping_event(now);
    tt_int_op(digest256map_size(service->desc_current->intro_points.map),
              OP_EQ, 1);
    /* We'll mangle the IP object to expire. */
    ip->time_to_expire = now;
    run_housekeeping_event(now);
    tt_int_op(digest256map_size(service->desc_current->intro_points.map),
              OP_EQ, 0);
  }

 done:
  hs_circuitmap_remove_circuit(TO_CIRCUIT(circ));
  circuit_free_(TO_CIRCUIT(circ));
  if (service) {
    remove_service(get_hs_service_map(), service);
    hs_service_free(service);
  }
  hs_free_all();
  UNMOCK(circuit_mark_for_close_);
}

/** Test that we rotate descriptors correctly. */
static void
test_rotate_descriptors(void *arg)
{
  int ret;
  time_t next_rotation_time, now;
  hs_service_t *service = NULL;
  hs_service_descriptor_t *desc_next;

  (void) arg;

  dummy_state = or_state_new();

  hs_init();
  MOCK(get_or_state, get_or_state_replacement);
  MOCK(circuit_mark_for_close_, mock_circuit_mark_for_close);
  MOCK(networkstatus_get_reasonably_live_consensus,
       mock_networkstatus_get_reasonably_live_consensus);

  /* Descriptor rotation happens with a consensus with a new SRV. */

  ret = parse_rfc1123_time("Sat, 26 Oct 1985 13:00:00 UTC",
                           &mock_ns.valid_after);
  tt_int_op(ret, OP_EQ, 0);
  ret = parse_rfc1123_time("Sat, 26 Oct 1985 14:00:00 UTC",
                           &mock_ns.fresh_until);
  tt_int_op(ret, OP_EQ, 0);
  dirauth_sched_recalculate_timing(get_options(), mock_ns.valid_after);

  update_approx_time(mock_ns.valid_after+1);
  now = mock_ns.valid_after+1;

  /* Create a service with a default descriptor and state. It's added to the
   * global map. */
  service = helper_create_service();
  service_descriptor_free(service->desc_current);
  service->desc_current = NULL;
  /* This triggers a build for both descriptors. The time now is only used in
   * the descriptor certificate which is important to be now else the decoding
   * will complain that the cert has expired if we use valid_after. */
  build_all_descriptors(now);
  tt_assert(service->desc_current);
  tt_assert(service->desc_next);

  /* Tweak our service next rotation time so we can use a custom time. */
  service->state.next_rotation_time = next_rotation_time =
    mock_ns.valid_after + (11 * 60 * 60);

  /* Nothing should happen, we are not at a new SRV. Our next rotation time
   * should be untouched. */
  rotate_all_descriptors(mock_ns.valid_after);
  tt_u64_op(service->state.next_rotation_time, OP_EQ, next_rotation_time);
  tt_assert(service->desc_current);
  tt_assert(service->desc_next);
  tt_u64_op(service->desc_current->time_period_num, OP_EQ,
            hs_get_previous_time_period_num(0));
  tt_u64_op(service->desc_next->time_period_num, OP_EQ,
            hs_get_time_period_num(0));
  /* Keep a reference so we can compare it after rotation to the current. */
  desc_next = service->desc_next;

  /* Going right after a new SRV. */
  ret = parse_rfc1123_time("Sat, 27 Oct 1985 01:00:00 UTC",
                           &mock_ns.valid_after);
  tt_int_op(ret, OP_EQ, 0);
  ret = parse_rfc1123_time("Sat, 27 Oct 1985 02:00:00 UTC",
                           &mock_ns.fresh_until);
  tt_int_op(ret, OP_EQ, 0);
  dirauth_sched_recalculate_timing(get_options(), mock_ns.valid_after);

  update_approx_time(mock_ns.valid_after+1);
  now = mock_ns.valid_after+1;

  /* Note down what to expect for the next rotation time which is 01:00 + 23h
   * meaning 00:00:00. */
  next_rotation_time = mock_ns.valid_after + (23 * 60 * 60);
  /* We should have our next rotation time modified, our current descriptor
   * cleaned up and the next descriptor becoming the current. */
  rotate_all_descriptors(mock_ns.valid_after);
  tt_u64_op(service->state.next_rotation_time, OP_EQ, next_rotation_time);
  tt_mem_op(service->desc_current, OP_EQ, desc_next, sizeof(*desc_next));
  tt_assert(service->desc_next == NULL);

  /* A second time should do nothing. */
  rotate_all_descriptors(mock_ns.valid_after);
  tt_u64_op(service->state.next_rotation_time, OP_EQ, next_rotation_time);
  tt_mem_op(service->desc_current, OP_EQ, desc_next, sizeof(*desc_next));
  tt_assert(service->desc_next == NULL);

  build_all_descriptors(now);
  tt_mem_op(service->desc_current, OP_EQ, desc_next, sizeof(*desc_next));
  tt_u64_op(service->desc_current->time_period_num, OP_EQ,
            hs_get_time_period_num(0));
  tt_u64_op(service->desc_next->time_period_num, OP_EQ,
            hs_get_next_time_period_num(0));
  tt_assert(service->desc_next);

 done:
  if (service) {
    remove_service(get_hs_service_map(), service);
    hs_service_free(service);
  }
  hs_free_all();
  UNMOCK(get_or_state);
  UNMOCK(circuit_mark_for_close_);
  UNMOCK(networkstatus_get_reasonably_live_consensus);
}

/** Test building descriptors: picking intro points, setting up their link
 *  specifiers, etc. */
static void
test_build_update_descriptors(void *arg)
{
  int ret;
  node_t *node;
  hs_service_t *service = NULL;
  hs_service_intro_point_t *ip_cur, *ip_next;
  routerinfo_t ri;

  (void) arg;

  hs_init();

  MOCK(get_or_state,
       get_or_state_replacement);
  MOCK(networkstatus_get_reasonably_live_consensus,
       mock_networkstatus_get_reasonably_live_consensus);

  dummy_state = or_state_new();

  ret = parse_rfc1123_time("Sat, 26 Oct 1985 03:00:00 UTC",
                           &mock_ns.valid_after);
  tt_int_op(ret, OP_EQ, 0);
  ret = parse_rfc1123_time("Sat, 26 Oct 1985 04:00:00 UTC",
                           &mock_ns.fresh_until);
  tt_int_op(ret, OP_EQ, 0);
  dirauth_sched_recalculate_timing(get_options(), mock_ns.valid_after);

  update_approx_time(mock_ns.valid_after+1);

  time_t now = mock_ns.valid_after+1;

  /* Create a service without a current descriptor to trigger a build. */
  service = helper_create_service();
  tt_assert(service);
  /* Unfortunately, the helper creates a dummy descriptor so get rid of it. */
  service_descriptor_free(service->desc_current);
  service->desc_current = NULL;

  /* We have a fresh service so this should trigger a build for both
   * descriptors for specific time period that we'll test. */
  build_all_descriptors(now);
  /* Check *current* descriptor. */
  tt_assert(service->desc_current);
  tt_assert(service->desc_current->desc);
  tt_assert(service->desc_current->intro_points.map);
  /* The current time period is the one expected when starting at 03:00. */
  tt_u64_op(service->desc_current->time_period_num, OP_EQ,
            hs_get_time_period_num(0));
  /* This should be untouched, the update descriptor process changes it. */
  tt_u64_op(service->desc_current->next_upload_time, OP_EQ, 0);

  /* Check *next* descriptor. */
  tt_assert(service->desc_next);
  tt_assert(service->desc_next->desc);
  tt_assert(service->desc_next->intro_points.map);
  tt_assert(service->desc_current != service->desc_next);
  tt_u64_op(service->desc_next->time_period_num, OP_EQ,
            hs_get_next_time_period_num(0));
  /* This should be untouched, the update descriptor process changes it. */
  tt_u64_op(service->desc_next->next_upload_time, OP_EQ, 0);

  /* Time to test the update of those descriptors. At first, we have no node
   * in the routerlist so this will find NO suitable node for the IPs. */
  setup_full_capture_of_logs(LOG_INFO);
  update_all_descriptors_intro_points(now);
  expect_log_msg_containing("Unable to find a suitable node to be an "
                            "introduction point for service");
  teardown_capture_of_logs();
  tt_int_op(digest256map_size(service->desc_current->intro_points.map),
            OP_EQ, 0);
  tt_int_op(digest256map_size(service->desc_next->intro_points.map),
            OP_EQ, 0);

  /* Now, we'll setup a node_t. */
  {
    curve25519_secret_key_t curve25519_secret_key;

    memset(&ri, 0, sizeof(routerinfo_t));

    tor_addr_parse(&ri.ipv4_addr, "127.0.0.1");
    ri.ipv4_orport = 1337;
    ri.purpose = ROUTER_PURPOSE_GENERAL;
    /* Ugly yes but we never free the "ri" object so this just makes things
     * easier. */
    ri.protocol_list = (char *) "HSDir=1-2 LinkAuth=3";
    summarize_protover_flags(&ri.pv, ri.protocol_list, NULL);
    ret = curve25519_secret_key_generate(&curve25519_secret_key, 0);
    tt_int_op(ret, OP_EQ, 0);
    ri.onion_curve25519_pkey =
      tor_malloc_zero(sizeof(curve25519_public_key_t));
    ri.onion_pkey = tor_malloc_zero(140);
    curve25519_public_key_generate(ri.onion_curve25519_pkey,
                                   &curve25519_secret_key);
    memset(ri.cache_info.identity_digest, 'A', DIGEST_LEN);
    /* Setup ed25519 identity */
    ed25519_keypair_t kp1;
    ed25519_keypair_generate(&kp1, 0);
    ri.cache_info.signing_key_cert = tor_malloc_zero(sizeof(tor_cert_t));
    tt_assert(ri.cache_info.signing_key_cert);
    ed25519_pubkey_copy(&ri.cache_info.signing_key_cert->signing_key,
                        &kp1.pubkey);
    nodelist_set_routerinfo(&ri, NULL);
    node = node_get_mutable_by_id(ri.cache_info.identity_digest);
    tt_assert(node);
    node->is_running = node->is_valid = node->is_fast = node->is_stable = 1;
  }

  /* We have to set this, or the lack of microdescriptors for these
   * nodes will make them unusable. */
  get_options_mutable()->UseMicrodescriptors = 0;

  /* We expect to pick only one intro point from the node above. */
  setup_full_capture_of_logs(LOG_INFO);
  update_all_descriptors_intro_points(now);
  tor_free(node->ri->onion_curve25519_pkey); /* Avoid memleak. */
  tor_free(node->ri->cache_info.signing_key_cert);
  tor_free(node->ri->onion_pkey);
  expect_log_msg_containing("just picked 1 intro points and wanted 3 for next "
                            "descriptor. It currently has 0 intro points. "
                            "Launching ESTABLISH_INTRO circuit shortly.");
  teardown_capture_of_logs();
  tt_int_op(digest256map_size(service->desc_current->intro_points.map),
            OP_EQ, 1);
  tt_int_op(digest256map_size(service->desc_next->intro_points.map),
            OP_EQ, 1);
  /* Get the IP object. Because we don't have the auth key of the IP, we can't
   * query it so get the first element in the map. */
  {
    void *obj = NULL;
    const uint8_t *key;
    digest256map_iter_t *iter =
      digest256map_iter_init(service->desc_current->intro_points.map);
    digest256map_iter_get(iter, &key, &obj);
    tt_assert(obj);
    ip_cur = obj;
    /* Get also the IP from the next descriptor. We'll make sure it's not the
     * same object as in the current descriptor. */
    iter = digest256map_iter_init(service->desc_next->intro_points.map);
    digest256map_iter_get(iter, &key, &obj);
    tt_assert(obj);
    ip_next = obj;
  }
  tt_mem_op(ip_cur, OP_NE, ip_next, sizeof(hs_desc_intro_point_t));

  /* We won't test the service IP object because there is a specific test
   * already for this but we'll make sure that the state is coherent.*/

  /* Three link specifiers are mandatory so make sure we do have them. */
  tt_int_op(smartlist_len(ip_cur->base.link_specifiers), OP_EQ, 3);
  /* Make sure we have a valid encryption keypair generated when we pick an
   * intro point in the update process. */
  tt_assert(!fast_mem_is_zero((char *) ip_cur->enc_key_kp.seckey.secret_key,
                             CURVE25519_SECKEY_LEN));
  tt_assert(!fast_mem_is_zero((char *) ip_cur->enc_key_kp.pubkey.public_key,
                             CURVE25519_PUBKEY_LEN));
  tt_u64_op(ip_cur->time_to_expire, OP_GE, now +
            INTRO_POINT_LIFETIME_MIN_SECONDS);
  tt_u64_op(ip_cur->time_to_expire, OP_LE, now +
            INTRO_POINT_LIFETIME_MAX_SECONDS);

  /* Now, we will try to set up a service after a new time period has started
   * and see if it behaves as expected. */

  ret = parse_rfc1123_time("Sat, 26 Oct 1985 13:00:00 UTC",
                           &mock_ns.valid_after);
  tt_int_op(ret, OP_EQ, 0);
  ret = parse_rfc1123_time("Sat, 26 Oct 1985 14:00:00 UTC",
                           &mock_ns.fresh_until);
  tt_int_op(ret, OP_EQ, 0);

  update_approx_time(mock_ns.valid_after+1);
  now = mock_ns.valid_after+1;

  /* Create a service without a current descriptor to trigger a build. */
  service = helper_create_service();
  tt_assert(service);
  /* Unfortunately, the helper creates a dummy descriptor so get rid of it. */
  service_descriptor_free(service->desc_current);
  service->desc_current = NULL;

  /* We have a fresh service so this should trigger a build for both
   * descriptors for specific time period that we'll test. */
  build_all_descriptors(now);
  /* Check *current* descriptor. */
  tt_assert(service->desc_current);
  tt_assert(service->desc_current->desc);
  tt_assert(service->desc_current->intro_points.map);
  /* This should be for the previous time period. */
  tt_u64_op(service->desc_current->time_period_num, OP_EQ,
            hs_get_previous_time_period_num(0));
  /* This should be untouched, the update descriptor process changes it. */
  tt_u64_op(service->desc_current->next_upload_time, OP_EQ, 0);

  /* Check *next* descriptor. */
  tt_assert(service->desc_next);
  tt_assert(service->desc_next->desc);
  tt_assert(service->desc_next->intro_points.map);
  tt_assert(service->desc_current != service->desc_next);
  tt_u64_op(service->desc_next->time_period_num, OP_EQ,
            hs_get_time_period_num(0));
  /* This should be untouched, the update descriptor process changes it. */
  tt_u64_op(service->desc_next->next_upload_time, OP_EQ, 0);

  /* Let's remove the next descriptor to simulate a rotation. */
  service_descriptor_free(service->desc_next);
  service->desc_next = NULL;

  build_all_descriptors(now);
  /* Check *next* descriptor. */
  tt_assert(service->desc_next);
  tt_assert(service->desc_next->desc);
  tt_assert(service->desc_next->intro_points.map);
  tt_assert(service->desc_current != service->desc_next);
  tt_u64_op(service->desc_next->time_period_num, OP_EQ,
            hs_get_next_time_period_num(0));
  /* This should be untouched, the update descriptor process changes it. */
  tt_u64_op(service->desc_next->next_upload_time, OP_EQ, 0);

 done:
  if (service) {
    remove_service(get_hs_service_map(), service);
    hs_service_free(service);
  }
  hs_free_all();
  nodelist_free_all();
}

/** Test building descriptors. We use this separate function instead of
 *  using test_build_update_descriptors because that function is too complex
 *  and also too interactive. */
static void
test_build_descriptors(void *arg)
{
  int ret;
  time_t now = time(NULL);
  hs_service_t *last_service = NULL;

  (void) arg;

  hs_init();

  MOCK(get_or_state,
       get_or_state_replacement);
  MOCK(networkstatus_get_reasonably_live_consensus,
       mock_networkstatus_get_reasonably_live_consensus);

  dummy_state = or_state_new();

  ret = parse_rfc1123_time("Sat, 26 Oct 1985 03:00:00 UTC",
                           &mock_ns.valid_after);
  tt_int_op(ret, OP_EQ, 0);
  ret = parse_rfc1123_time("Sat, 26 Oct 1985 04:00:00 UTC",
                           &mock_ns.fresh_until);
  tt_int_op(ret, OP_EQ, 0);
  dirauth_sched_recalculate_timing(get_options(), mock_ns.valid_after);

  /* Generate a valid number of fake auth clients when a client authorization
   * is disabled. */
  {
    hs_service_t *service = helper_create_service();
    last_service = service;
    service_descriptor_free(service->desc_current);
    service->desc_current = NULL;

    build_all_descriptors(now);
    tt_assert(service->desc_current);
    tt_assert(service->desc_current->desc);

    hs_desc_superencrypted_data_t *superencrypted;
    superencrypted = &service->desc_current->desc->superencrypted_data;
    tt_int_op(smartlist_len(superencrypted->clients), OP_EQ, 16);

    helper_destroy_service(service);
    last_service = NULL;
  }

  /* Generate a valid number of fake auth clients when the number of
   * clients is zero. */
  {
    hs_service_t *service = helper_create_service_with_clients(0);
    last_service = service;
    service_descriptor_free(service->desc_current);
    service->desc_current = NULL;

    build_all_descriptors(now);
    hs_desc_superencrypted_data_t *superencrypted;
    superencrypted = &service->desc_current->desc->superencrypted_data;
    tt_int_op(smartlist_len(superencrypted->clients), OP_EQ, 16);

    helper_destroy_service(service);
    last_service = NULL;
  }

  /* Generate a valid number of fake auth clients when the number of
   * clients is not a multiple of 16. */
  {
    hs_service_t *service = helper_create_service_with_clients(20);
    last_service = service;
    service_descriptor_free(service->desc_current);
    service->desc_current = NULL;

    build_all_descriptors(now);
    hs_desc_superencrypted_data_t *superencrypted;
    superencrypted = &service->desc_current->desc->superencrypted_data;
    tt_int_op(smartlist_len(superencrypted->clients), OP_EQ, 32);

    helper_destroy_service(service);
    last_service = NULL;
  }

  /* Do not generate any fake desc client when the number of clients is
   * a multiple of 16 but not zero. */
  {
    hs_service_t *service = helper_create_service_with_clients(32);
    last_service = service;
    service_descriptor_free(service->desc_current);
    service->desc_current = NULL;

    build_all_descriptors(now);
    hs_desc_superencrypted_data_t *superencrypted;
    superencrypted = &service->desc_current->desc->superencrypted_data;
    tt_int_op(smartlist_len(superencrypted->clients), OP_EQ, 32);

    helper_destroy_service(service);
    last_service = NULL;
  }

 done:
  helper_destroy_service(last_service);
  hs_free_all();
}

static void
test_upload_descriptors(void *arg)
{
  int ret;
  time_t now;
  hs_service_t *service;

  (void) arg;

  hs_init();
  MOCK(get_or_state,
       get_or_state_replacement);
  MOCK(networkstatus_get_reasonably_live_consensus,
       mock_networkstatus_get_reasonably_live_consensus);

  dummy_state = or_state_new();

  ret = parse_rfc1123_time("Sat, 26 Oct 1985 13:00:00 UTC",
                           &mock_ns.valid_after);
  tt_int_op(ret, OP_EQ, 0);
  ret = parse_rfc1123_time("Sat, 26 Oct 1985 14:00:00 UTC",
                           &mock_ns.fresh_until);
  tt_int_op(ret, OP_EQ, 0);
  dirauth_sched_recalculate_timing(get_options(), mock_ns.valid_after);

  update_approx_time(mock_ns.valid_after+1);
  now = mock_ns.valid_after+1;

  /* Create a service with no descriptor. It's added to the global map. */
  service = hs_service_new(get_options());
  tt_assert(service);
  service->config.version = HS_VERSION_THREE;
  ed25519_secret_key_generate(&service->keys.identity_sk, 0);
  ed25519_public_key_generate(&service->keys.identity_pk,
                              &service->keys.identity_sk);
  /* Register service to global map. */
  ret = register_service(get_hs_service_map(), service);
  tt_int_op(ret, OP_EQ, 0);
  /* But first, build our descriptor. */
  build_all_descriptors(now);

  /* Nothing should happen because we have 0 introduction circuit established
   * and we want (by default) 3 intro points. */
  run_upload_descriptor_event(now);
  /* If no upload happened, this should be untouched. */
  tt_u64_op(service->desc_current->next_upload_time, OP_EQ, 0);
  /* We'll simulate that we've opened our intro point circuit and that we only
   * want one intro point. */
  service->config.num_intro_points = 1;

  /* Set our next upload time after now which will skip the upload. */
  service->desc_current->next_upload_time = now + 1000;
  run_upload_descriptor_event(now);
  /* If no upload happened, this should be untouched. */
  tt_u64_op(service->desc_current->next_upload_time, OP_EQ, now + 1000);

 done:
  hs_free_all();
  UNMOCK(get_or_state);
}

/** Global vars used by test_rendezvous1_parsing() */
static char rend1_payload[RELAY_PAYLOAD_SIZE];
static size_t rend1_payload_len = 0;

/** Mock for relay_send_command_from_edge() to send a RENDEZVOUS1 cell. Instead
 *  of sending it to the network, instead save it to the global `rend1_payload`
 *  variable so that we can inspect it in the test_rendezvous1_parsing()
 *  test. */
static int
mock_relay_send_rendezvous1(streamid_t stream_id, circuit_t *circ,
                            uint8_t relay_command, const char *payload,
                            size_t payload_len,
                            crypt_path_t *cpath_layer,
                            const char *filename, int lineno)
{
  (void) stream_id;
  (void) circ;
  (void) relay_command;
  (void) cpath_layer;
  (void) filename;
  (void) lineno;

  memcpy(rend1_payload, payload, payload_len);
  rend1_payload_len = payload_len;

  return 0;
}

/** Send a RENDEZVOUS1 as a service, and parse it as a client. */
static void
test_rendezvous1_parsing(void *arg)
{
  int retval;
  static const char *test_addr =
    "4acth47i6kxnvkewtm6q7ib2s3ufpo5sqbsnzjpbi7utijcltosqemad.onion";
  hs_service_t *service = NULL;
  origin_circuit_t *service_circ = NULL;
  origin_circuit_t *client_circ = NULL;
  ed25519_keypair_t ip_auth_kp;
  curve25519_keypair_t ephemeral_kp;
  curve25519_keypair_t client_kp;
  curve25519_keypair_t ip_enc_kp;
  int flags = CIRCLAUNCH_NEED_UPTIME | CIRCLAUNCH_IS_INTERNAL;

  (void) arg;

  MOCK(relay_send_command_from_edge_, mock_relay_send_rendezvous1);

  {
    /* Let's start by setting up the service that will start the rend */
    service = tor_malloc_zero(sizeof(hs_service_t));
    ed25519_secret_key_generate(&service->keys.identity_sk, 0);
    ed25519_public_key_generate(&service->keys.identity_pk,
                                &service->keys.identity_sk);
    memcpy(service->onion_address, test_addr, sizeof(service->onion_address));
    tt_assert(service);
  }

  {
    /* Now let's set up the service rendezvous circuit and its keys. */
    service_circ = helper_create_origin_circuit(CIRCUIT_PURPOSE_S_CONNECT_REND,
                                                flags);
    tor_free(service_circ->hs_ident);
    hs_ntor_rend_cell_keys_t hs_ntor_rend_cell_keys;
    uint8_t rendezvous_cookie[HS_REND_COOKIE_LEN];
    curve25519_keypair_generate(&ip_enc_kp, 0);
    curve25519_keypair_generate(&ephemeral_kp, 0);
    curve25519_keypair_generate(&client_kp, 0);
    ed25519_keypair_generate(&ip_auth_kp, 0);
    retval = hs_ntor_service_get_rendezvous1_keys(&ip_auth_kp.pubkey,
                                                  &ip_enc_kp,
                                                  &ephemeral_kp,
                                                  &client_kp.pubkey,
                                                  &hs_ntor_rend_cell_keys);
    tt_int_op(retval, OP_EQ, 0);

    memset(rendezvous_cookie, 2, sizeof(rendezvous_cookie));
    service_circ->hs_ident =
      create_rp_circuit_identifier(service, rendezvous_cookie,
                                   &ephemeral_kp.pubkey,
                                   &hs_ntor_rend_cell_keys);
  }

  /* Send out the RENDEZVOUS1 and make sure that our mock func worked */
  tt_assert(fast_mem_is_zero(rend1_payload, 32));
  hs_circ_service_rp_has_opened(service, service_circ);
  tt_assert(!fast_mem_is_zero(rend1_payload, 32));
  tt_int_op(rend1_payload_len, OP_EQ, HS_LEGACY_RENDEZVOUS_CELL_SIZE);

  /******************************/

  /** Now let's create the client rendezvous circuit */
  client_circ =
    helper_create_origin_circuit(CIRCUIT_PURPOSE_C_REND_READY_INTRO_ACKED,
                                 flags);
  /* fix up its circ ident */
  ed25519_pubkey_copy(&client_circ->hs_ident->intro_auth_pk,
                      &ip_auth_kp.pubkey);
  memcpy(&client_circ->hs_ident->rendezvous_client_kp,
         &client_kp, sizeof(client_circ->hs_ident->rendezvous_client_kp));
  memcpy(&client_circ->hs_ident->intro_enc_pk.public_key,
         &ip_enc_kp.pubkey.public_key,
         sizeof(client_circ->hs_ident->intro_enc_pk.public_key));

  /* Now parse the rendezvous2 circuit and make sure it was fine. We are
   * skipping 20 bytes off its payload, since that's the rendezvous cookie
   * which is only present in REND1. */
  retval = handle_rendezvous2(client_circ,
                              (uint8_t*)rend1_payload+20,
                              rend1_payload_len-20);
  tt_int_op(retval, OP_EQ, 0);

  /* TODO: We are only simulating client/service here. We could also simulate
   * the rendezvous point by plugging in rend_mid_establish_rendezvous(). We
   * would need an extra circuit and some more stuff but it's doable. */

 done:
  circuit_free_(TO_CIRCUIT(service_circ));
  circuit_free_(TO_CIRCUIT(client_circ));
  hs_service_free(service);
  hs_free_all();
  UNMOCK(relay_send_command_from_edge_);
}

static void
test_authorized_client_config_equal(void *arg)
{
  int ret;
  hs_service_config_t *config1, *config2;

  (void) arg;

  config1 = tor_malloc_zero(sizeof(*config1));
  config2 = tor_malloc_zero(sizeof(*config2));

  /* Both configs are empty. */
  {
    config1->clients = smartlist_new();
    config2->clients = smartlist_new();

    ret = service_authorized_client_config_equal(config1, config2);
    tt_int_op(ret, OP_EQ, 1);

    service_clear_config(config1);
    service_clear_config(config2);
  }

  /* Both configs have exactly the same client config. */
  {
    config1->clients = smartlist_new();
    config2->clients = smartlist_new();

    hs_service_authorized_client_t *client1, *client2;
    client1 = helper_create_authorized_client();
    client2 = helper_create_authorized_client();

    smartlist_add(config1->clients, client1);
    smartlist_add(config1->clients, client2);

    /* We should swap the order of clients here to test that the order
     * does not matter. */
    smartlist_add(config2->clients, helper_clone_authorized_client(client2));
    smartlist_add(config2->clients, helper_clone_authorized_client(client1));

    ret = service_authorized_client_config_equal(config1, config2);
    tt_int_op(ret, OP_EQ, 1);

    service_clear_config(config1);
    service_clear_config(config2);
  }

  /* The numbers of clients in both configs are not equal. */
  {
    config1->clients = smartlist_new();
    config2->clients = smartlist_new();

    hs_service_authorized_client_t *client1, *client2;
    client1 = helper_create_authorized_client();
    client2 = helper_create_authorized_client();

    smartlist_add(config1->clients, client1);
    smartlist_add(config1->clients, client2);

    smartlist_add(config2->clients, helper_clone_authorized_client(client1));

    ret = service_authorized_client_config_equal(config1, config2);
    tt_int_op(ret, OP_EQ, 0);

    service_clear_config(config1);
    service_clear_config(config2);
  }

  /* The first config has two distinct clients while the second config
   * has two clients but they are duplicate. */
  {
    config1->clients = smartlist_new();
    config2->clients = smartlist_new();

    hs_service_authorized_client_t *client1, *client2;
    client1 = helper_create_authorized_client();
    client2 = helper_create_authorized_client();

    smartlist_add(config1->clients, client1);
    smartlist_add(config1->clients, client2);

    smartlist_add(config2->clients, helper_clone_authorized_client(client1));
    smartlist_add(config2->clients, helper_clone_authorized_client(client1));

    ret = service_authorized_client_config_equal(config1, config2);
    tt_int_op(ret, OP_EQ, 0);

    service_clear_config(config1);
    service_clear_config(config2);
  }

  /* Both configs have totally distinct clients. */
  {
    config1->clients = smartlist_new();
    config2->clients = smartlist_new();

    hs_service_authorized_client_t *client1, *client2, *client3, *client4;
    client1 = helper_create_authorized_client();
    client2 = helper_create_authorized_client();
    client3 = helper_create_authorized_client();
    client4 = helper_create_authorized_client();

    smartlist_add(config1->clients, client1);
    smartlist_add(config1->clients, client2);

    smartlist_add(config2->clients, client3);
    smartlist_add(config2->clients, client4);

    ret = service_authorized_client_config_equal(config1, config2);
    tt_int_op(ret, OP_EQ, 0);

    service_clear_config(config1);
    service_clear_config(config2);
  }

 done:
  tor_free(config1);
  tor_free(config2);
}

/** Test that client circuit ID gets correctly exported */
static void
test_export_client_circuit_id(void *arg)
{
  origin_circuit_t *or_circ = NULL;
  size_t sz;
  char *cp1=NULL, *cp2=NULL;
  connection_t *conn = NULL;

  (void) arg;

  MOCK(connection_write_to_buf_impl_, connection_write_to_buf_mock);

  hs_service_init();

  /* Create service */
  hs_service_t *service = helper_create_service();
  /* Check that export circuit ID detection works */
  service->config.circuit_id_protocol = HS_CIRCUIT_ID_PROTOCOL_NONE;
  tt_int_op(0, OP_EQ,
            hs_service_exports_circuit_id(&service->keys.identity_pk));
  service->config.circuit_id_protocol = HS_CIRCUIT_ID_PROTOCOL_HAPROXY;
  tt_int_op(1, OP_EQ,
            hs_service_exports_circuit_id(&service->keys.identity_pk));

  /* Create client connection */
  conn = test_conn_get_connection(AP_CONN_STATE_CIRCUIT_WAIT, CONN_TYPE_AP, 0);

  /* Create client edge conn hs_ident */
  edge_connection_t *edge_conn = TO_EDGE_CONN(conn);
  edge_conn->hs_ident = hs_ident_edge_conn_new(&service->keys.identity_pk);
  edge_conn->hs_ident->orig_virtual_port = 42;

  /* Create rend circuit */
  or_circ = origin_circuit_new();
  or_circ->base_.purpose = CIRCUIT_PURPOSE_C_REND_JOINED;
  edge_conn->on_circuit = TO_CIRCUIT(or_circ);
  or_circ->global_identifier = 666;

  /* Export circuit ID */
  export_hs_client_circuit_id(edge_conn, service->config.circuit_id_protocol);

  /* Check contents */
  cp1 = buf_get_contents(conn->outbuf, &sz);
  tt_str_op(cp1, OP_EQ,
            "PROXY TCP6 fc00:dead:beef:4dad::0:29a ::1 666 42\r\n");

  /* Change circ GID and see that the reported circuit ID also changes */
  or_circ->global_identifier = 22;

  /* check changes */
  export_hs_client_circuit_id(edge_conn, service->config.circuit_id_protocol);
  cp2 = buf_get_contents(conn->outbuf, &sz);
  tt_str_op(cp1, OP_NE, cp2);
  tor_free(cp1);

  /* Check that GID with UINT32_MAX works. */
  or_circ->global_identifier = UINT32_MAX;

  export_hs_client_circuit_id(edge_conn, service->config.circuit_id_protocol);
  cp1 = buf_get_contents(conn->outbuf, &sz);
  tt_str_op(cp1, OP_EQ,
            "PROXY TCP6 fc00:dead:beef:4dad::ffff:ffff ::1 65535 42\r\n");
  tor_free(cp1);

  /* Check that GID with UINT16_MAX works. */
  or_circ->global_identifier = UINT16_MAX;

  export_hs_client_circuit_id(edge_conn, service->config.circuit_id_protocol);
  cp1 = buf_get_contents(conn->outbuf, &sz);
  tt_str_op(cp1, OP_EQ,
            "PROXY TCP6 fc00:dead:beef:4dad::0:ffff ::1 65535 42\r\n");
  tor_free(cp1);

  /* Check that GID with UINT16_MAX + 7 works. */
  or_circ->global_identifier = UINT16_MAX + 7;

  export_hs_client_circuit_id(edge_conn, service->config.circuit_id_protocol);
  cp1 = buf_get_contents(conn->outbuf, &sz);
  tt_str_op(cp1, OP_EQ, "PROXY TCP6 fc00:dead:beef:4dad::1:6 ::1 6 42\r\n");

 done:
  UNMOCK(connection_write_to_buf_impl_);
  circuit_free_(TO_CIRCUIT(or_circ));
  connection_free_minimal(conn);
  hs_service_free(service);
  tor_free(cp1);
  tor_free(cp2);
}

static smartlist_t *
mock_node_get_link_specifier_smartlist(const node_t *node, bool direct_conn)
{
  (void) node;
  (void) direct_conn;

  smartlist_t *lspecs = smartlist_new();
  link_specifier_t *ls_legacy = link_specifier_new();
  smartlist_add(lspecs, ls_legacy);

  return lspecs;
}

static node_t *fake_node = NULL;

static const node_t *
mock_build_state_get_exit_node(cpath_build_state_t *state)
{
  (void) state;

  if (!fake_node) {
    curve25519_secret_key_t seckey;
    curve25519_secret_key_generate(&seckey, 0);

    fake_node = tor_malloc_zero(sizeof(node_t));
    fake_node->ri = tor_malloc_zero(sizeof(routerinfo_t));
    fake_node->ri->onion_curve25519_pkey =
      tor_malloc_zero(sizeof(curve25519_public_key_t));
    curve25519_public_key_generate(fake_node->ri->onion_curve25519_pkey,
                                   &seckey);
  }

  return fake_node;
}

static void
mock_launch_rendezvous_point_circuit(const hs_service_t *service,
                             const ed25519_public_key_t *ip_auth_pubkey,
                             const curve25519_keypair_t *ip_enc_key_kp,
                             const hs_cell_intro_rdv_data_t *rdv_data,
                             time_t now)
{
  (void) service;
  (void) ip_auth_pubkey;
  (void) ip_enc_key_kp;
  (void) rdv_data;
  (void) now;
  return;
}

/**
 *  Test that INTRO2 cells are handled well by onion services in the normal
 *  case and also when onionbalance is enabled.
 */
static void
test_intro2_handling(void *arg)
{
  (void)arg;

  MOCK(build_state_get_exit_node, mock_build_state_get_exit_node);
  MOCK(relay_send_command_from_edge_, mock_relay_send_command_from_edge);
  MOCK(node_get_link_specifier_smartlist,
       mock_node_get_link_specifier_smartlist);
  MOCK(launch_rendezvous_point_circuit, mock_launch_rendezvous_point_circuit);

  memset(relay_payload, 0, sizeof(relay_payload));

  int retval;
  time_t now = 0101010101;
  update_approx_time(now);

  /** OK this is the play:
   *
   *  In Act I, we have a standalone onion service X (without onionbalance
   *  enabled). We test that X can properly handle INTRO2 cells sent by a
   *  client Alice.
   *
   *  In Act II, we create an onionbalance setup with frontend being Z which
   *  includes instances X and Y. We then setup onionbalance on X and test that
   *  Alice who addresses Z can communicate with X through INTRO2 cells.
   *
   *  In Act III, we test that Alice can also communicate with X
   *  directly even tho onionbalance is enabled.
   *
   *  And finally in Act IV, we check various cases where the INTRO2 cell
   *  should not go through because the subcredentials don't line up
   *  (e.g. Alice sends INTRO2 to X using Y's subcredential).
   */

  /** Let's start with some setup! Create the instances and the frontend
      service, create Alice, etc:  */

  /* Create instance X */
  hs_service_t x_service;
  memset(&x_service, 0, sizeof(hs_service_t));
  /* Disable onionbalance */
  x_service.config.ob_master_pubkeys = NULL;
  x_service.state.replay_cache_rend_cookie = replaycache_new(0,0);
  /* Initialize the metrics store */
  hs_metrics_service_init(&x_service);

  /* Create subcredential for x: */
  ed25519_keypair_t x_identity_keypair;
  hs_subcredential_t x_subcred;
  ed25519_keypair_generate(&x_identity_keypair, 0);
  hs_helper_get_subcred_from_identity_keypair(&x_identity_keypair,
                                              &x_subcred);

  /* Create the x instance's intro point */
  hs_service_intro_point_t *x_ip = NULL;
  {
    curve25519_secret_key_t seckey;
    curve25519_public_key_t pkey;
    curve25519_secret_key_generate(&seckey, 0);
    curve25519_public_key_generate(&pkey, &seckey);

    node_t intro_node;
    memset(&intro_node, 0, sizeof(intro_node));
    routerinfo_t ri;
    memset(&ri, 0, sizeof(routerinfo_t));
    ri.onion_curve25519_pkey = &pkey;
    intro_node.ri = &ri;

    x_ip = service_intro_point_new(&intro_node);
  }

  /* Create z frontend's subcredential */
  ed25519_keypair_t z_identity_keypair;
  hs_subcredential_t z_subcred;
  ed25519_keypair_generate(&z_identity_keypair, 0);
  hs_helper_get_subcred_from_identity_keypair(&z_identity_keypair,
                                              &z_subcred);

  /* Create y instance's subcredential */
  ed25519_keypair_t y_identity_keypair;
  hs_subcredential_t y_subcred;
  ed25519_keypair_generate(&y_identity_keypair, 0);
  hs_helper_get_subcred_from_identity_keypair(&y_identity_keypair,
                                              &y_subcred);

  /* Create Alice's intro point */
  hs_desc_intro_point_t *alice_ip;
  ed25519_keypair_t signing_kp;
  ed25519_keypair_generate(&signing_kp, 0);
  alice_ip = hs_helper_build_intro_point(&signing_kp, now, "1.2.3.4", 0,
                                         &x_ip->auth_key_kp,
                                         &x_ip->enc_key_kp);

  /* Create Alice's intro and rend circuits */
  origin_circuit_t *intro_circ = origin_circuit_new();
  intro_circ->cpath = tor_malloc_zero(sizeof(crypt_path_t));
  intro_circ->cpath->prev = intro_circ->cpath;
  intro_circ->hs_ident = tor_malloc_zero(sizeof(*intro_circ->hs_ident));
  origin_circuit_t rend_circ;
  TO_CIRCUIT(&rend_circ)->ccontrol = NULL;
  rend_circ.hs_ident = tor_malloc_zero(sizeof(*rend_circ.hs_ident));
  curve25519_keypair_generate(&rend_circ.hs_ident->rendezvous_client_kp, 0);
  memset(rend_circ.hs_ident->rendezvous_cookie, 'r', HS_REND_COOKIE_LEN);

  /* ************************************************************ */

  /* Act I:
   *
   * Where Alice connects to X without onionbalance in the picture */

  /* Create INTRODUCE1 */
  tt_assert(fast_mem_is_zero(relay_payload, sizeof(relay_payload)));
  retval = hs_circ_send_introduce1(intro_circ, &rend_circ,
                                   alice_ip, &x_subcred, NULL);

  /* Check that the payload was written successfully */
  tt_int_op(retval, OP_EQ, 0);
  tt_assert(!fast_mem_is_zero(relay_payload, sizeof(relay_payload)));
  tt_int_op(relay_payload_len, OP_NE, 0);

  /* Handle the cell */
  retval = hs_circ_handle_introduce2(&x_service,
                                    intro_circ, x_ip,
                                    &x_subcred,
                                    (uint8_t*)relay_payload,relay_payload_len);
  tt_int_op(retval, OP_EQ, 0);

  /* ************************************************************ */

  /* Act II:
   *
   * We now create an onionbalance setup with Z being the frontend and X and Y
   * being the backend instances. Make sure that Alice can talk with the
   * backend instance X even tho she thinks she is talking to the frontend Z.
   */

  /* Now configure the X instance to do onionbalance with Z as the frontend */
  x_service.config.ob_master_pubkeys = smartlist_new();
  smartlist_add(x_service.config.ob_master_pubkeys,
                &z_identity_keypair.pubkey);

  /* Create descriptors for x and load next descriptor with the x's
   * subcredential so that it can accept connections for itself. */
  x_service.desc_current = service_descriptor_new();
  memset(x_service.desc_current->desc->subcredential.subcred, 'C',SUBCRED_LEN);
  x_service.desc_next = service_descriptor_new();
  memcpy(&x_service.desc_next->desc->subcredential, &x_subcred, SUBCRED_LEN);

  /* Refresh OB keys */
  hs_ob_refresh_keys(&x_service);

  /* Create INTRODUCE1 from Alice to X through Z */
  memset(relay_payload, 0, sizeof(relay_payload));
  retval = hs_circ_send_introduce1(intro_circ, &rend_circ,
                                   alice_ip, &z_subcred, NULL);

  /* Check that the payload was written successfully */
  tt_int_op(retval, OP_EQ, 0);
  tt_assert(!fast_mem_is_zero(relay_payload, sizeof(relay_payload)));
  tt_int_op(relay_payload_len, OP_NE, 0);

  /* Deliver INTRODUCE1 to X even tho it carries Z's subcredential */
  replaycache_free(x_service.state.replay_cache_rend_cookie);
  x_service.state.replay_cache_rend_cookie = replaycache_new(0, 0);

  retval = hs_circ_handle_introduce2(&x_service,
                                   intro_circ, x_ip,
                                   &z_subcred,
                                   (uint8_t*)relay_payload, relay_payload_len);
  tt_int_op(retval, OP_EQ, 0);

  replaycache_free(x_ip->replay_cache);
  x_ip->replay_cache = replaycache_new(0, 0);

  replaycache_free(x_service.state.replay_cache_rend_cookie);
  x_service.state.replay_cache_rend_cookie = replaycache_new(0, 0);

  /* ************************************************************ */

  /* Act III:
   *
   * Now send a direct INTRODUCE cell from Alice to X using X's subcredential
   * and check that it succeeds even with onionbalance enabled.
   */

  /* Refresh OB keys (just to check for memleaks) */
  hs_ob_refresh_keys(&x_service);

  /* Create INTRODUCE1 from Alice to X using X's subcred. */
  memset(relay_payload, 0, sizeof(relay_payload));
  retval = hs_circ_send_introduce1(intro_circ, &rend_circ,
                                   alice_ip, &x_subcred, NULL);

  /* Check that the payload was written successfully */
  tt_int_op(retval, OP_EQ, 0);
  tt_assert(!fast_mem_is_zero(relay_payload, sizeof(relay_payload)));
  tt_int_op(relay_payload_len, OP_NE, 0);

  /* Send INTRODUCE1 to X with X's subcredential (should succeed) */
  replaycache_free(x_service.state.replay_cache_rend_cookie);
  x_service.state.replay_cache_rend_cookie = replaycache_new(0, 0);

  retval = hs_circ_handle_introduce2(&x_service,
                                   intro_circ, x_ip,
                                   &x_subcred,
                                   (uint8_t*)relay_payload, relay_payload_len);
  tt_int_op(retval, OP_EQ, 0);

  /* We haven't encountered any errors yet, so all the introduction request
   * error metrics should be 0 */
  const smartlist_t *entries = metrics_store_get_all(
      x_service.metrics.store, "tor_hs_intro_rejected_intro_req_count");
  const metrics_store_entry_t *entry = NULL;

  for (size_t i = 0; i < hs_metrics_intro_req_error_reasons_size; ++i) {
    entry = metrics_store_find_entry_with_label(
        entries,
        metrics_format_label("reason", hs_metrics_intro_req_error_reasons[i]));
    tt_assert(entry);
    tt_int_op(metrics_store_entry_get_value(entry), OP_EQ, 0);
  }

  /* ************************************************************ */

  /* Act IV:
   *
   * Test cases where the INTRO2 cell should not be able to decode.
   */

  /* Try sending the exact same INTRODUCE2 cell again and see that the intro
   * point replay cache triggers: */
  setup_full_capture_of_logs(LOG_WARN);
  retval = hs_circ_handle_introduce2(&x_service,
                                   intro_circ, x_ip,
                                   &x_subcred,
                                   (uint8_t*)relay_payload, relay_payload_len);
  tt_int_op(retval, OP_EQ, -1);
  expect_log_msg_containing("with the same ENCRYPTED section");
  teardown_capture_of_logs();
  entry = metrics_store_find_entry_with_label(
      entries,
      metrics_format_label("reason", HS_METRICS_ERR_INTRO_REQ_INTRODUCE2));
  tt_assert(entry);
  tt_int_op(metrics_store_entry_get_value(entry), OP_EQ, 1);

  /* Now cleanup the intro point replay cache but not the service replay cache
     and see that this one triggers this time. */
  replaycache_free(x_ip->replay_cache);
  x_ip->replay_cache = replaycache_new(0, 0);
  setup_full_capture_of_logs(LOG_INFO);
  retval = hs_circ_handle_introduce2(&x_service,
                                   intro_circ, x_ip,
                                   &x_subcred,
                                   (uint8_t*)relay_payload, relay_payload_len);
  tt_int_op(retval, OP_EQ, -1);
  expect_log_msg_containing("with same REND_COOKIE");
  teardown_capture_of_logs();

  entry = metrics_store_find_entry_with_label(
      entries, metrics_format_label(
                   "reason", HS_METRICS_ERR_INTRO_REQ_INTRODUCE2_REPLAY));
  tt_assert(entry);
  tt_int_op(metrics_store_entry_get_value(entry), OP_EQ, 1);

  /* Now just to make sure cleanup both replay caches and make sure that the
     cell gets through */
  replaycache_free(x_ip->replay_cache);
  x_ip->replay_cache = replaycache_new(0, 0);
  replaycache_free(x_service.state.replay_cache_rend_cookie);
  x_service.state.replay_cache_rend_cookie = replaycache_new(0, 0);
  retval = hs_circ_handle_introduce2(&x_service,
                                   intro_circ, x_ip,
                                   &x_subcred,
                                   (uint8_t*)relay_payload, relay_payload_len);
  tt_int_op(retval, OP_EQ, 0);

  /* This time, the error metric was *not* incremented */
  tt_int_op(metrics_store_entry_get_value(entry), OP_EQ, 1);

  /* As a final thing, create an INTRODUCE1 cell from Alice to X using Y's
   * subcred (should fail since Y is just another instance and not the frontend
   * service!) */
  memset(relay_payload, 0, sizeof(relay_payload));
  retval = hs_circ_send_introduce1(intro_circ, &rend_circ,
                                   alice_ip, &y_subcred, NULL);
  tt_int_op(retval, OP_EQ, 0);

  /* Check that the payload was written successfully */
  tt_assert(!fast_mem_is_zero(relay_payload, sizeof(relay_payload)));
  tt_int_op(relay_payload_len, OP_NE, 0);

  retval = hs_circ_handle_introduce2(&x_service,
                                   intro_circ, x_ip,
                                   &y_subcred,
                                   (uint8_t*)relay_payload, relay_payload_len);

  tt_int_op(retval, OP_EQ, -1);
  entry = metrics_store_find_entry_with_label(
      entries,
      metrics_format_label("reason", HS_METRICS_ERR_INTRO_REQ_INTRODUCE2));
  tt_assert(entry);
  tt_int_op(metrics_store_entry_get_value(entry), OP_EQ, 2);

 done:
  /* Start cleaning up X */
  replaycache_free(x_service.state.replay_cache_rend_cookie);
  smartlist_free(x_service.config.ob_master_pubkeys);
  tor_free(x_service.state.ob_subcreds);
  service_descriptor_free(x_service.desc_current);
  service_descriptor_free(x_service.desc_next);
  hs_metrics_service_free(&x_service);
  service_intro_point_free(x_ip);

  /* Clean up Alice */
  hs_desc_intro_point_free(alice_ip);
  tor_free(rend_circ.hs_ident);

  if (fake_node) {
    tor_free(fake_node->ri->onion_curve25519_pkey);
    tor_free(fake_node->ri);
    tor_free(fake_node);
  }

  UNMOCK(build_state_get_exit_node);
  UNMOCK(relay_send_command_from_edge_);
  UNMOCK(node_get_link_specifier_smartlist);
  UNMOCK(launch_rendezvous_point_circuit);
}

static void
test_cannot_upload_descriptors(void *arg)
{
  int ret;
  time_t now;
  hs_service_t *service;

  (void) arg;

  hs_init();
  MOCK(get_or_state,
       get_or_state_replacement);
  MOCK(networkstatus_get_reasonably_live_consensus,
       mock_networkstatus_get_reasonably_live_consensus);

  dummy_state = or_state_new();

  ret = parse_rfc1123_time("Sat, 26 Oct 1985 13:00:00 UTC",
                           &mock_ns.valid_after);
  tt_int_op(ret, OP_EQ, 0);
  ret = parse_rfc1123_time("Sat, 26 Oct 1985 14:00:00 UTC",
                           &mock_ns.fresh_until);
  tt_int_op(ret, OP_EQ, 0);
  dirauth_sched_recalculate_timing(get_options(), mock_ns.valid_after);

  update_approx_time(mock_ns.valid_after + 1);
  now = mock_ns.valid_after + 1;

  /* Create a service with no descriptor. It's added to the global map. */
  service = hs_service_new(get_options());
  tt_assert(service);
  service->config.version = HS_VERSION_THREE;
  ed25519_secret_key_generate(&service->keys.identity_sk, 0);
  ed25519_public_key_generate(&service->keys.identity_pk,
                              &service->keys.identity_sk);
  /* Register service to global map. */
  ret = register_service(get_hs_service_map(), service);
  tt_int_op(ret, OP_EQ, 0);
  /* But first, build our descriptor. */
  build_all_descriptors(now);

  /* 1. Testing missing intro points reason. */
  {
    digest256map_t *cur = service->desc_current->intro_points.map;
    digest256map_t *tmp = digest256map_new();
    service->desc_current->intro_points.map = tmp;
    service->desc_current->missing_intro_points = 1;
    setup_full_capture_of_logs(LOG_INFO);
    run_upload_descriptor_event(now);
    digest256map_free(tmp, tor_free_);
    service->desc_current->intro_points.map = cur;
    expect_log_msg_containing(
      "Service [scrubbed] can't upload its current descriptor: "
      "Missing intro points");
    teardown_capture_of_logs();
    /* Reset. */
    service->desc_current->missing_intro_points = 0;
  }

  /* 2. Testing non established intro points. */
  {
    setup_full_capture_of_logs(LOG_INFO);
    run_upload_descriptor_event(now);
    expect_log_msg_containing(
      "Service [scrubbed] can't upload its current descriptor: "
      "Intro circuits aren't yet all established (0/3).");
    teardown_capture_of_logs();
  }

  /* We need to pass the established circuit tests and thus from now on, we
   * MOCK this to return 3 intro points. */
  MOCK(count_desc_circuit_established, mock_count_desc_circuit_established);
  num_intro_points = 3;

  /* 3. Testing non established intro points. */
  {
    service->desc_current->next_upload_time = now + 1000;
    setup_full_capture_of_logs(LOG_INFO);
    run_upload_descriptor_event(now);
    expect_log_msg_containing(
      "Service [scrubbed] can't upload its current descriptor: "
      "Next upload time is");
    teardown_capture_of_logs();
    /* Reset. */
    service->desc_current->next_upload_time = 0;
  }

  /* 4. Testing missing live consensus. */
  {
    MOCK(networkstatus_get_reasonably_live_consensus,
         mock_networkstatus_get_reasonably_live_consensus_null);
    setup_full_capture_of_logs(LOG_INFO);
    run_upload_descriptor_event(now);
    expect_log_msg_containing(
      "Service [scrubbed] can't upload its current descriptor: "
      "No reasonably live consensus");
    teardown_capture_of_logs();
    /* Reset. */
    MOCK(networkstatus_get_reasonably_live_consensus,
         mock_networkstatus_get_reasonably_live_consensus);
  }

  /* 5. Test missing minimum directory information. */
  {
    MOCK(router_have_minimum_dir_info,
         mock_router_have_minimum_dir_info_false);
    setup_full_capture_of_logs(LOG_INFO);
    run_upload_descriptor_event(now);
    expect_log_msg_containing(
      "Service [scrubbed] can't upload its current descriptor: "
      "Not enough directory information");
    teardown_capture_of_logs();

    /* Running it again shouldn't trigger anything due to rate limitation. */
    setup_full_capture_of_logs(LOG_INFO);
    run_upload_descriptor_event(now);
    expect_no_log_entry();
    teardown_capture_of_logs();
    UNMOCK(router_have_minimum_dir_info);
  }

  /* Increase time and redo test (5) in order to test the rate limiting. */
  update_approx_time(mock_ns.valid_after + 61);
  {
    MOCK(router_have_minimum_dir_info,
         mock_router_have_minimum_dir_info_false);
    setup_full_capture_of_logs(LOG_INFO);
    run_upload_descriptor_event(now);
    expect_log_msg_containing(
      "Service [scrubbed] can't upload its current descriptor: "
      "Not enough directory information");
    teardown_capture_of_logs();
    UNMOCK(router_have_minimum_dir_info);
  }

 done:
  hs_free_all();
  UNMOCK(count_desc_circuit_established);
  UNMOCK(networkstatus_get_reasonably_live_consensus);
  UNMOCK(get_or_state);
}

struct testcase_t hs_service_tests[] = {
  { "e2e_rend_circuit_setup", test_e2e_rend_circuit_setup, TT_FORK,
    NULL, NULL },
  { "load_keys", test_load_keys, TT_FORK,
    NULL, NULL },
  { "client_filename_is_valid", test_client_filename_is_valid, TT_FORK,
    NULL, NULL },
  { "parse_authorized_client", test_parse_authorized_client, TT_FORK,
    NULL, NULL },
  { "load_keys_with_client_auth", test_load_keys_with_client_auth, TT_FORK,
    NULL, NULL },
  { "access_service", test_access_service, TT_FORK,
    NULL, NULL },
  { "service_intro_point", test_service_intro_point, TT_FORK,
    NULL, NULL },
  { "helper_functions", test_helper_functions, TT_FORK,
    NULL, NULL },
  { "intro_circuit_opened", test_intro_circuit_opened, TT_FORK,
    NULL, NULL },
  { "intro_established", test_intro_established, TT_FORK,
    NULL, NULL },
  { "closing_intro_circs", test_closing_intro_circs, TT_FORK,
    NULL, NULL },
  { "rdv_circuit_opened", test_rdv_circuit_opened, TT_FORK,
    NULL, NULL },
  { "bad_introduce2", test_bad_introduce2, TT_FORK,
    NULL, NULL },
  { "service_event", test_service_event, TT_FORK,
    NULL, NULL },
  { "rotate_descriptors", test_rotate_descriptors, TT_FORK,
    NULL, NULL },
  { "build_update_descriptors", test_build_update_descriptors, TT_FORK,
    NULL, NULL },
  { "build_descriptors", test_build_descriptors, TT_FORK,
    NULL, NULL },
  { "upload_descriptors", test_upload_descriptors, TT_FORK,
    NULL, NULL },
  { "cannot_upload_descriptors", test_cannot_upload_descriptors, TT_FORK,
    NULL, NULL },
  { "rendezvous1_parsing", test_rendezvous1_parsing, TT_FORK,
    NULL, NULL },
  { "authorized_client_config_equal", test_authorized_client_config_equal,
    TT_FORK, NULL, NULL },
  { "export_client_circuit_id", test_export_client_circuit_id, TT_FORK,
    NULL, NULL },
  { "intro2_handling", test_intro2_handling, TT_FORK, NULL, NULL },

  END_OF_TESTCASES
};
