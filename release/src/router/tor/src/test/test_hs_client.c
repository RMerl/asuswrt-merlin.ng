/* Copyright (c) 2016-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file test_hs_client.c
 * \brief Test prop224 HS client functionality.
 */

#define CONFIG_PRIVATE
#define CRYPTO_PRIVATE
#define MAINLOOP_PRIVATE
#define HS_CLIENT_PRIVATE
#define CHANNEL_OBJECT_PRIVATE
#define CIRCUITBUILD_PRIVATE
#define CIRCUITLIST_PRIVATE
#define CONNECTION_PRIVATE
#define CRYPT_PATH_PRIVATE

#include "test/test.h"
#include "test/test_helpers.h"
#include "test/log_test_helpers.h"
#include "test/rend_test_helpers.h"
#include "test/hs_test_helpers.h"

#include "app/config/config.h"
#include "lib/crypt_ops/crypto_cipher.h"
#include "lib/crypt_ops/crypto_dh.h"
#include "lib/crypt_ops/crypto_rand.h"
#include "core/or/channeltls.h"
#include "feature/dircommon/directory.h"
#include "core/mainloop/mainloop.h"
#include "feature/nodelist/nodelist.h"
#include "feature/nodelist/routerset.h"

#include "feature/hs/hs_circuit.h"
#include "feature/hs/hs_circuitmap.h"
#include "feature/hs/hs_client.h"
#include "feature/hs/hs_config.h"
#include "feature/hs/hs_ident.h"
#include "feature/hs/hs_cache.h"
#include "feature/rend/rendcache.h"
#include "core/or/circuitlist.h"
#include "core/or/circuitbuild.h"
#include "core/or/extendinfo.h"
#include "core/mainloop/connection.h"
#include "core/or/connection_edge.h"
#include "feature/nodelist/networkstatus.h"

#include "core/or/cpath_build_state_st.h"
#include "core/or/crypt_path_st.h"
#include "core/or/crypt_path.h"
#include "feature/dircommon/dir_connection_st.h"
#include "core/or/entry_connection_st.h"
#include "core/or/extend_info_st.h"
#include "feature/nodelist/networkstatus_st.h"
#include "core/or/origin_circuit_st.h"
#include "core/or/socks_request_st.h"

static int
mock_connection_ap_handshake_send_begin(entry_connection_t *ap_conn)
{
  (void) ap_conn;
  return 0;
}

static networkstatus_t mock_ns;

/* Always return NULL. */
static networkstatus_t *
mock_networkstatus_get_reasonably_live_consensus_false(time_t now, int flavor)
{
  (void) now;
  (void) flavor;
  return NULL;
}

static networkstatus_t *
mock_networkstatus_get_reasonably_live_consensus(time_t now, int flavor)
{
  (void) now;
  (void) flavor;
  return &mock_ns;
}

static int
mock_write_str_to_file(const char *path, const char *str, int bin)
{
  (void) bin;
  (void) path;
  (void) str;
  return 0;
}

static or_options_t mocked_options;

static const or_options_t *
mock_get_options(void)
{
  return &mocked_options;
}

static int
helper_config_client(const char *conf, int validate_only)
{
  int ret = 0;
  or_options_t *options = NULL;
  tt_assert(conf);
  options = helper_parse_options(conf);
  tt_assert(options);
  ret = hs_config_client_auth_all(options, validate_only);
 done:
  or_options_free(options);
  return ret;
}

static void
helper_add_random_client_auth(const ed25519_public_key_t *service_pk)
{
  char *conf = NULL;
#define conf_fmt "ClientOnionAuthDir %s\n"
  tor_asprintf(&conf, conf_fmt, get_fname("auth_keys"));
#undef conf_fmt
  helper_config_client(conf, 0);
  tor_free(conf);

  digest256map_t *client_auths = get_hs_client_auths_map();
  hs_client_service_authorization_t *auth =
    tor_malloc_zero(sizeof(hs_client_service_authorization_t));
  curve25519_secret_key_generate(&auth->enc_seckey, 0);
  hs_build_address(service_pk, HS_VERSION_THREE, auth->onion_address);
  digest256map_set(client_auths, service_pk->pubkey, auth);
}

/* Test helper function: Setup a circuit and a stream with the same hidden
 * service destination, and put them in <b>circ_out</b> and
 * <b>conn_out</b>. Make the stream wait for circuits to be established to the
 * hidden service. */
static int
helper_get_circ_and_stream_for_test(origin_circuit_t **circ_out,
                                    connection_t **conn_out,
                                    int is_legacy)
{
  int retval;
  channel_tls_t *n_chan=NULL;
  rend_data_t *conn_rend_data = NULL;
  origin_circuit_t *or_circ = NULL;
  connection_t *conn = NULL;
  ed25519_public_key_t service_pk;

  /* Make a dummy connection stream and make it wait for our circuit */
  conn = test_conn_get_connection(AP_CONN_STATE_CIRCUIT_WAIT,
                                  CONN_TYPE_AP /* ??? */,
                                  0);
  if (is_legacy) {
    /* Legacy: Setup rend_data of stream */
    char service_id[REND_SERVICE_ID_LEN_BASE32+1] = {0};
    TO_EDGE_CONN(conn)->rend_data = mock_rend_data(service_id);
    conn_rend_data = TO_EDGE_CONN(conn)->rend_data;
  } else {
    /* prop224: Setup hs conn identifier on the stream */
    ed25519_secret_key_t sk;
    tt_int_op(0, OP_EQ, ed25519_secret_key_generate(&sk, 0));
    tt_int_op(0, OP_EQ, ed25519_public_key_generate(&service_pk, &sk));

    /* Setup hs_conn_identifier of stream */
    TO_EDGE_CONN(conn)->hs_ident = hs_ident_edge_conn_new(&service_pk);
  }

  /* Make it wait for circuit */
  connection_ap_mark_as_pending_circuit(TO_ENTRY_CONN(conn));

  /* This is needed to silence a BUG warning from
     connection_edge_update_circuit_isolation() */
  TO_ENTRY_CONN(conn)->original_dest_address =
    tor_strdup(TO_ENTRY_CONN(conn)->socks_request->address);

  /****************************************************/

  /* Now make dummy circuit */
  or_circ = origin_circuit_new();

  or_circ->base_.purpose = CIRCUIT_PURPOSE_C_REND_READY_INTRO_ACKED;

  or_circ->build_state = tor_malloc_zero(sizeof(cpath_build_state_t));
  or_circ->build_state->is_internal = 1;

  if (is_legacy) {
    /* Legacy: Setup rend data and final cpath */
    or_circ->build_state->pending_final_cpath =
      tor_malloc_zero(sizeof(crypt_path_t));
    or_circ->build_state->pending_final_cpath->magic = CRYPT_PATH_MAGIC;
    or_circ->build_state->pending_final_cpath->rend_dh_handshake_state =
      crypto_dh_new(DH_TYPE_REND);
    tt_assert(
         or_circ->build_state->pending_final_cpath->rend_dh_handshake_state);
    retval = crypto_dh_generate_public(
           or_circ->build_state->pending_final_cpath->rend_dh_handshake_state);
    tt_int_op(retval, OP_EQ, 0);
    or_circ->rend_data = rend_data_dup(conn_rend_data);
  } else {
    /* prop224: Setup hs ident on the circuit */
    or_circ->hs_ident = hs_ident_circuit_new(&service_pk);
  }

  TO_CIRCUIT(or_circ)->state = CIRCUIT_STATE_OPEN;

  /* fake n_chan */
  n_chan = tor_malloc_zero(sizeof(channel_tls_t));
  n_chan->base_.global_identifier = 1;
  or_circ->base_.n_chan = &(n_chan->base_);

  *circ_out = or_circ;
  *conn_out = conn;

  return 0;

 done:
  /* something failed */
  return -1;
}

/* Test: Ensure that setting up legacy e2e rendezvous circuits works
 * correctly. */
static void
test_e2e_rend_circuit_setup_legacy(void *arg)
{
  ssize_t retval;
  origin_circuit_t *or_circ = NULL;
  connection_t *conn = NULL;

  (void) arg;

  /** In this test we create a v2 legacy HS stream and a circuit with the same
   *  hidden service destination. We make the stream wait for circuits to be
   *  established to the hidden service, and then we complete the circuit using
   *  the hs_circuit_setup_e2e_rend_circ_legacy_client() function. We then
   *  check that the end-to-end cpath was setup correctly and that the stream
   *  was attached to the circuit as expected. */

  MOCK(connection_ap_handshake_send_begin,
       mock_connection_ap_handshake_send_begin);

  /* Setup */
  retval = helper_get_circ_and_stream_for_test( &or_circ, &conn, 1);
  tt_int_op(retval, OP_EQ, 0);
  tt_assert(or_circ);
  tt_assert(conn);

  /* Check number of hops */
  retval = cpath_get_n_hops(&or_circ->cpath);
  tt_int_op(retval, OP_EQ, 0);

  /* Check that our stream is not attached on any circuits */
  tt_ptr_op(TO_EDGE_CONN(conn)->on_circuit, OP_EQ, NULL);

  /********************************************** */

  /* Make a good RENDEZVOUS1 cell body because it needs to pass key exchange
   * digest verification... */
  uint8_t rend_cell_body[DH1024_KEY_LEN+DIGEST_LEN] = {2};
  {
    char keys[DIGEST_LEN+CPATH_KEY_MATERIAL_LEN];
    crypto_dh_t *dh_state =
      or_circ->build_state->pending_final_cpath->rend_dh_handshake_state;
    /* compute and overwrite digest of cell body with the right value */
    retval = crypto_dh_compute_secret(LOG_PROTOCOL_WARN, dh_state,
                                      (char*)rend_cell_body, DH1024_KEY_LEN,
                                      keys, DIGEST_LEN+CPATH_KEY_MATERIAL_LEN);
    tt_int_op(retval, OP_GT, 0);
    memcpy(rend_cell_body+DH1024_KEY_LEN, keys, DIGEST_LEN);
  }

  /* Setup the circuit */
  retval = hs_circuit_setup_e2e_rend_circ_legacy_client(or_circ,
                                                        rend_cell_body);
  tt_int_op(retval, OP_EQ, 0);

  /**********************************************/

  /* See that a hop was added to the circuit's cpath */
  retval = cpath_get_n_hops(&or_circ->cpath);
  tt_int_op(retval, OP_EQ, 1);

  /* Check the digest algo */
  tt_int_op(
         crypto_digest_get_algorithm(or_circ->cpath->pvt_crypto.f_digest),
            OP_EQ, DIGEST_SHA1);
  tt_int_op(
         crypto_digest_get_algorithm(or_circ->cpath->pvt_crypto.b_digest),
            OP_EQ, DIGEST_SHA1);
  tt_assert(or_circ->cpath->pvt_crypto.f_crypto);
  tt_assert(or_circ->cpath->pvt_crypto.b_crypto);

  /* Ensure that circ purpose was changed */
  tt_int_op(or_circ->base_.purpose, OP_EQ, CIRCUIT_PURPOSE_C_REND_JOINED);

  /* Test that stream got attached */
  tt_ptr_op(TO_EDGE_CONN(conn)->on_circuit, OP_EQ, TO_CIRCUIT(or_circ));

 done:
  connection_free_minimal(conn);
  if (or_circ)
    tor_free(TO_CIRCUIT(or_circ)->n_chan);
  circuit_free_(TO_CIRCUIT(or_circ));
}

/* Test: Ensure that setting up v3 rendezvous circuits works correctly. */
static void
test_e2e_rend_circuit_setup(void *arg)
{
  uint8_t ntor_key_seed[DIGEST256_LEN] = {0};
  origin_circuit_t *or_circ = NULL;
  int retval;
  connection_t *conn = NULL;

  (void) arg;

  /** In this test we create a prop224 v3 HS stream and a circuit with the same
   *  hidden service destination. We make the stream wait for circuits to be
   *  established to the hidden service, and then we complete the circuit using
   *  the hs_circuit_setup_e2e_rend_circ() function. We then check that the
   *  end-to-end cpath was setup correctly and that the stream was attached to
   *  the circuit as expected. */

  MOCK(connection_ap_handshake_send_begin,
       mock_connection_ap_handshake_send_begin);

  /* Setup */
  retval = helper_get_circ_and_stream_for_test(&or_circ, &conn, 0);
  tt_int_op(retval, OP_EQ, 0);
  tt_assert(or_circ);
  tt_assert(conn);

  /* Check number of hops: There should be no hops yet to this circ */
  retval = cpath_get_n_hops(&or_circ->cpath);
  tt_int_op(retval, OP_EQ, 0);
  tt_ptr_op(or_circ->cpath, OP_EQ, NULL);

  /* Check that our stream is not attached on any circuits */
  tt_ptr_op(TO_EDGE_CONN(conn)->on_circuit, OP_EQ, NULL);

  /**********************************************/

  /* Setup the circuit */
  retval = hs_circuit_setup_e2e_rend_circ(or_circ, ntor_key_seed,
                                          sizeof(ntor_key_seed), 0);
  tt_int_op(retval, OP_EQ, 0);

  /**********************************************/

  /* See that a hop was added to the circuit's cpath */
  retval = cpath_get_n_hops(&or_circ->cpath);
  tt_int_op(retval, OP_EQ, 1);

  /* Check that the crypt path has prop224 algorithm parameters */
  tt_int_op(crypto_digest_get_algorithm(or_circ->cpath->pvt_crypto.f_digest),
            OP_EQ, DIGEST_SHA3_256);
  tt_int_op(crypto_digest_get_algorithm(or_circ->cpath->pvt_crypto.b_digest),
            OP_EQ, DIGEST_SHA3_256);
  tt_assert(or_circ->cpath->pvt_crypto.f_crypto);
  tt_assert(or_circ->cpath->pvt_crypto.b_crypto);

  /* Ensure that circ purpose was changed */
  tt_int_op(or_circ->base_.purpose, OP_EQ, CIRCUIT_PURPOSE_C_REND_JOINED);

  /* Test that stream got attached */
  tt_ptr_op(TO_EDGE_CONN(conn)->on_circuit, OP_EQ, TO_CIRCUIT(or_circ));

 done:
  connection_free_minimal(conn);
  if (or_circ)
    tor_free(TO_CIRCUIT(or_circ)->n_chan);
  circuit_free_(TO_CIRCUIT(or_circ));
}

/** Test client logic for picking intro points from a descriptor. Also test how
 *  ExcludeNodes and intro point failures affect picking intro points. */
static void
test_client_pick_intro(void *arg)
{
  int ret;
  ed25519_keypair_t service_kp;
  hs_descriptor_t *desc = NULL;

  MOCK(networkstatus_get_reasonably_live_consensus,
       mock_networkstatus_get_reasonably_live_consensus);

  (void) arg;

  hs_init();

  /* Generate service keypair */
  tt_int_op(0, OP_EQ, ed25519_keypair_generate(&service_kp, 0));

  /* Set time */
  ret = parse_rfc1123_time("Sat, 26 Oct 1985 13:00:00 UTC",
                           &mock_ns.valid_after);
  tt_int_op(ret, OP_EQ, 0);
  ret = parse_rfc1123_time("Sat, 26 Oct 1985 14:00:00 UTC",
                           &mock_ns.fresh_until);
  tt_int_op(ret, OP_EQ, 0);

  update_approx_time(mock_ns.fresh_until-10);
  time_t now = approx_time();

  /* Test logic:
   *
   * 1) Add our desc with intro points to the HS cache.
   *
   * 2) Mark all descriptor intro points except _the chosen one_ as
   *    failed. Then query the desc to get a random intro: check that we got
   *    _the chosen one_. Then fail the chosen one as well, and see that no
   *    intros are returned.
   *
   * 3) Then clean the intro state cache and get an intro point.
   *
   * 4) Try fetching an intro with the wrong service key: shouldn't work
   *
   * 5) Set StrictNodes and put all our intro points in ExcludeNodes: see that
   *    nothing is returned.
   */

  /* 1) Add desc to HS cache */
  {
    char *encoded = NULL;
    desc = hs_helper_build_hs_desc_with_ip(&service_kp);
    ret = hs_desc_encode_descriptor(desc, &service_kp, NULL, &encoded);
    tt_int_op(ret, OP_EQ, 0);
    tt_assert(encoded);

    /* store it */
    ret = hs_cache_store_as_client(encoded, &service_kp.pubkey);
    tt_int_op(ret, OP_EQ, HS_DESC_DECODE_OK);

    /* fetch it to make sure it works */
    const hs_descriptor_t *fetched_desc =
      hs_cache_lookup_as_client(&service_kp.pubkey);
    tt_assert(fetched_desc);
    tt_mem_op(fetched_desc->subcredential.subcred,
              OP_EQ, desc->subcredential.subcred,
              SUBCRED_LEN);
    tt_assert(!fast_mem_is_zero((char*)fetched_desc->subcredential.subcred,
                               DIGEST256_LEN));
    tor_free(encoded);
  }

  /* 2) Mark all intro points except _the chosen one_ as failed. Then query the
   *   desc and get a random intro: check that we got _the chosen one_. */
  {
    /* Tell hs_get_extend_info_from_lspecs() to skip the private address check.
     */
    get_options_mutable()->ExtendAllowPrivateAddresses = 1;
    /* Pick the chosen intro point and get its ei */
    hs_desc_intro_point_t *chosen_intro_point =
      smartlist_get(desc->encrypted_data.intro_points, 0);
    extend_info_t *chosen_intro_ei =
      desc_intro_point_to_extend_info(chosen_intro_point);
    tt_assert(chosen_intro_point);
    tt_assert(chosen_intro_ei);

    /* Now mark all other intro points as failed */
    SMARTLIST_FOREACH_BEGIN(desc->encrypted_data.intro_points,
                            hs_desc_intro_point_t *, ip) {
      /* Skip the chosen intro point */
      if (ip == chosen_intro_point) {
        continue;
      }
      ed25519_public_key_t *intro_auth_key = &ip->auth_key_cert->signed_key;
      hs_cache_client_intro_state_note(&service_kp.pubkey,
                                       intro_auth_key,
                                       INTRO_POINT_FAILURE_GENERIC);
    } SMARTLIST_FOREACH_END(ip);

    /* Try to get a random intro: Should return the chosen one! */
    /* (We try several times, to make sure this behavior is consistent, and to
     * cover the different cases of client_get_random_intro().) */
    for (int i = 0; i < 64; ++i) {
      extend_info_t *ip = client_get_random_intro(&service_kp.pubkey);
      tor_assert(ip);
      tt_assert(!fast_mem_is_zero((char*)ip->identity_digest, DIGEST_LEN));
      tt_mem_op(ip->identity_digest, OP_EQ, chosen_intro_ei->identity_digest,
                DIGEST_LEN);
      extend_info_free(ip);
    }

    extend_info_free(chosen_intro_ei);

    /* Now also mark the chosen one as failed: See that we can't get any intro
       points anymore. */
    hs_cache_client_intro_state_note(&service_kp.pubkey,
                                &chosen_intro_point->auth_key_cert->signed_key,
                                     INTRO_POINT_FAILURE_TIMEOUT);
    extend_info_t *ip = client_get_random_intro(&service_kp.pubkey);
    tor_assert(!ip);
  }

  /* 3) Clean the intro state cache and get an intro point */
  {
    /* Pretend we are 5 mins in the future and order a cleanup of the intro
     * state. This should clean up the intro point failures and allow us to get
     * an intro. */
    hs_cache_client_intro_state_clean(now + 5*60);

    /* Get an intro. It should work! */
    extend_info_t *ip = client_get_random_intro(&service_kp.pubkey);
    tor_assert(ip);
    extend_info_free(ip);
  }

  /* 4) Try fetching an intro with the wrong service key: shouldn't work */
  {
    ed25519_keypair_t dummy_kp;
    tt_int_op(0, OP_EQ, ed25519_keypair_generate(&dummy_kp, 0));
    extend_info_t *ip = client_get_random_intro(&dummy_kp.pubkey);
    tor_assert(!ip);
  }

  /* 5) Set StrictNodes and put all our intro points in ExcludeNodes: see that
   *    nothing is returned. */
  {
    get_options_mutable()->ExcludeNodes = routerset_new();
    get_options_mutable()->StrictNodes = 1;
    SMARTLIST_FOREACH_BEGIN(desc->encrypted_data.intro_points,
                            hs_desc_intro_point_t *, ip) {
      extend_info_t *intro_ei = desc_intro_point_to_extend_info(ip);
      /* desc_intro_point_to_extend_info() doesn't return IPv6 intro points
       * yet, because we can't extend to them. See #24404, #24451, and #24181.
       */
      if (intro_ei == NULL) {
        /* Pretend we're making a direct connection, and that we can use IPv6
         */
        get_options_mutable()->ClientUseIPv6 = 1;
        intro_ei = hs_get_extend_info_from_lspecs(ip->link_specifiers,
                                                  &ip->onion_key, 1);
        tt_assert(tor_addr_family(&intro_ei->orports[0].addr) == AF_INET6);
      }
      tt_assert(intro_ei);
      if (intro_ei) {
        const char *ptr;
        char ip_addr[TOR_ADDR_BUF_LEN];
        /* We need to decorate in case it is an IPv6 else routerset_parse()
         * doesn't like it. */
        ptr = tor_addr_to_str(ip_addr, &intro_ei->orports[0].addr,
                              sizeof(ip_addr), 1);
        tt_assert(ptr == ip_addr);
        ret = routerset_parse(get_options_mutable()->ExcludeNodes,
                              ip_addr, "");
        tt_int_op(ret, OP_EQ, 0);
        extend_info_free(intro_ei);
      }
    } SMARTLIST_FOREACH_END(ip);

    extend_info_t *ip = client_get_random_intro(&service_kp.pubkey);
    tt_assert(!ip);
  }

 done:
  hs_descriptor_free(desc);
}

static int
mock_router_have_minimum_dir_info_false(void)
{
  return 0;
}
static int
mock_router_have_minimum_dir_info_true(void)
{
  return 1;
}

static hs_client_fetch_status_t
mock_fetch_v3_desc_error(const ed25519_public_key_t *key)
{
  (void) key;
  return HS_CLIENT_FETCH_ERROR;
}

static void
mock_connection_mark_unattached_ap_(entry_connection_t *conn, int endreason,
                                    int line, const char *file)
{
  (void) line;
  (void) file;
  conn->edge_.end_reason = endreason;
  /* This function ultimately will flag this so make sure we do also in the
   * MOCK one so we can assess closed connections vs open ones. */
  conn->edge_.base_.marked_for_close = 1;
}

static void
mock_connection_mark_unattached_ap_no_close(entry_connection_t *conn,
                                            int endreason, int line,
                                            const char *file)
{
  (void) conn;
  (void) endreason;
  (void) line;
  (void) file;
}

static void
test_descriptor_fetch(void *arg)
{
  int ret;
  entry_connection_t *ec = NULL;
  ed25519_public_key_t service_pk;
  ed25519_secret_key_t service_sk;

  (void) arg;

  hs_init();
  memset(&service_sk, 'A', sizeof(service_sk));
  ret = ed25519_public_key_generate(&service_pk, &service_sk);
  tt_int_op(ret, OP_EQ, 0);

  /* Initialize this so get_voting_interval() doesn't freak out. */
  ret = parse_rfc1123_time("Sat, 26 Oct 1985 13:00:00 UTC",
                           &mock_ns.valid_after);
  tt_int_op(ret, OP_EQ, 0);
  ret = parse_rfc1123_time("Sat, 26 Oct 1985 14:00:00 UTC",
                           &mock_ns.fresh_until);
  tt_int_op(ret, OP_EQ, 0);

  ec = entry_connection_new(CONN_TYPE_AP, AF_INET);
  tt_assert(ec);
  ENTRY_TO_EDGE_CONN(ec)->hs_ident = hs_ident_edge_conn_new(&service_pk);
  tt_assert(ENTRY_TO_EDGE_CONN(ec)->hs_ident);
  TO_CONN(ENTRY_TO_EDGE_CONN(ec))->state = AP_CONN_STATE_RENDDESC_WAIT;
  smartlist_add(get_connection_array(), &ec->edge_.base_);

  /* 1. FetchHidServDescriptors is false so we shouldn't be able to fetch. */
  get_options_mutable()->FetchHidServDescriptors = 0;
  ret = hs_client_refetch_hsdesc(&service_pk);
  tt_int_op(ret, OP_EQ, HS_CLIENT_FETCH_NOT_ALLOWED);
  get_options_mutable()->FetchHidServDescriptors = 1;

  /* 2. We don't have a live consensus. */
  MOCK(networkstatus_get_reasonably_live_consensus,
       mock_networkstatus_get_reasonably_live_consensus_false);
  ret = hs_client_refetch_hsdesc(&service_pk);
  UNMOCK(networkstatus_get_reasonably_live_consensus);
  tt_int_op(ret, OP_EQ, HS_CLIENT_FETCH_MISSING_INFO);

  /* From now on, return a live consensus. */
  MOCK(networkstatus_get_reasonably_live_consensus,
       mock_networkstatus_get_reasonably_live_consensus);

  /* 3. Not enough dir information. */
  MOCK(router_have_minimum_dir_info,
       mock_router_have_minimum_dir_info_false);
  ret = hs_client_refetch_hsdesc(&service_pk);
  UNMOCK(router_have_minimum_dir_info);
  tt_int_op(ret, OP_EQ, HS_CLIENT_FETCH_MISSING_INFO);

  /* From now on, we do have enough directory information. */
  MOCK(router_have_minimum_dir_info,
       mock_router_have_minimum_dir_info_true);

  /* 4. We do have a pending directory request. */
  {
    dir_connection_t *dir_conn = dir_connection_new(AF_INET);
    dir_conn->hs_ident = tor_malloc_zero(sizeof(hs_ident_dir_conn_t));
    TO_CONN(dir_conn)->purpose = DIR_PURPOSE_FETCH_HSDESC;
    ed25519_pubkey_copy(&dir_conn->hs_ident->identity_pk, &service_pk);
    smartlist_add(get_connection_array(), TO_CONN(dir_conn));
    ret = hs_client_refetch_hsdesc(&service_pk);
    smartlist_remove(get_connection_array(), TO_CONN(dir_conn));
    connection_free_minimal(TO_CONN(dir_conn));
    tt_int_op(ret, OP_EQ, HS_CLIENT_FETCH_PENDING);
  }

  /* 5. We'll trigger an error on the fetch_desc_v3 and force to close all
   *    pending SOCKS request. */
  MOCK(router_have_minimum_dir_info,
       mock_router_have_minimum_dir_info_true);
  MOCK(fetch_v3_desc, mock_fetch_v3_desc_error);
  MOCK(connection_mark_unattached_ap_,
       mock_connection_mark_unattached_ap_);
  ret = hs_client_refetch_hsdesc(&service_pk);
  UNMOCK(fetch_v3_desc);
  UNMOCK(connection_mark_unattached_ap_);
  tt_int_op(ret, OP_EQ, HS_CLIENT_FETCH_ERROR);
  /* The close waiting for descriptor function has been called. */
  tt_int_op(ec->edge_.end_reason, OP_EQ, END_STREAM_REASON_RESOLVEFAILED);

 done:
  connection_free_minimal(ENTRY_TO_CONN(ec));
  UNMOCK(networkstatus_get_reasonably_live_consensus);
  UNMOCK(router_have_minimum_dir_info);
  hs_free_all();
}

static void
test_auth_key_filename_is_valid(void *arg)
{
  (void) arg;

  /* Valid file name. */
  tt_assert(auth_key_filename_is_valid("a.auth_private"));
  /* Valid file name with special character. */
  tt_assert(auth_key_filename_is_valid("a-.auth_private"));
  /* Invalid extension. */
  tt_assert(!auth_key_filename_is_valid("a.ath_private"));
  /* Nothing before the extension. */
  tt_assert(!auth_key_filename_is_valid(".auth_private"));

 done:
  ;
}

static void
test_parse_auth_file_content(void *arg)
{
  hs_client_service_authorization_t *auth = NULL;

  (void) arg;

  /* Valid authorized client. */
  auth = parse_auth_file_content(
      "4acth47i6kxnvkewtm6q7ib2s3ufpo5sqbsnzjpbi7utijcltosqemad:descriptor:"
      "x25519:zdsyvn2jq534ugyiuzgjy4267jbtzcjbsgedhshzx5mforyxtryq");
  tt_assert(auth);

  /* Wrong number of fields. */
  tt_assert(!parse_auth_file_content("a:b"));
  /* Wrong auth type. */
  tt_assert(!parse_auth_file_content(
      "4acth47i6kxnvkewtm6q7ib2s3ufpo5sqbsnzjpbi7utijcltosqemad:x:"
      "x25519:zdsyvn2jq534ugyiuzgjy4267jbtzcjbsgedhshzx5mforyxtryq"));
  /* Wrong key type. */
  tt_assert(!parse_auth_file_content(
      "4acth47i6kxnvkewtm6q7ib2s3ufpo5sqbsnzjpbi7utijcltosqemad:descriptor:"
      "x:zdsyvn2jq534ugyiuzgjy4267jbtzcjbsgedhshzx5mforyxtryq"));
  /* Some malformed string. */
  tt_assert(!parse_auth_file_content("xx:descriptor:x25519:aa=="));
  /* Bigger key than it should be */
  tt_assert(!parse_auth_file_content("xx:descriptor:x25519:"
                     "vjqea4jbhwwc4hto7ekyvqfbeodghbaq6nxi45hz4wr3qvhqv3yqa"));
  /* All-zeroes key */
  tt_assert(!parse_auth_file_content("xx:descriptor:x25519:"
            "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"));

 done:
  tor_free(auth);
}

static char *
mock_read_file_to_str(const char *filename, int flags, struct stat *stat_out)
{
  char *ret = NULL;

  (void) flags;
  (void) stat_out;

  if (!strcmp(filename, get_fname("auth_keys" PATH_SEPARATOR
                                              "client1.auth_private"))) {
    ret = tor_strdup(
        "4acth47i6kxnvkewtm6q7ib2s3ufpo5sqbsnzjpbi7utijcltosqemad:descriptor:"
        "x25519:zdsyvn2jq534ugyiuzgjy4267jbtzcjbsgedhshzx5mforyxtryq");
    goto done;
  }

  if (!strcmp(filename, get_fname("auth_keys" PATH_SEPARATOR "dummy.xxx"))) {
    ret = tor_strdup(
        "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx:descriptor:"
        "x25519:xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx");
    goto done;
  }

  if (!strcmp(filename, get_fname("auth_keys" PATH_SEPARATOR
                                              "client2.auth_private"))) {
    ret = tor_strdup(
        "25njqamcweflpvkl73j4szahhihoc4xt3ktcgjnpaingr5yhkenl5sid:descriptor:"
        "x25519:fdreqzjqso7d2ac7qscrxfl5qfpamdvgy5d6cxejcgzc3hvhurmq");
    goto done;
  }

 done:
  return ret;
}

static int
mock_check_private_dir(const char *dirname, cpd_check_t check,
                       const char *effective_user)
{
  (void) dirname;
  (void) check;
  (void) effective_user;

  return 0;
}

static smartlist_t *
mock_tor_listdir(const char *dirname)
{
  smartlist_t *file_list = smartlist_new();

  (void) dirname;

  smartlist_add(file_list, tor_strdup("client1.auth_private"));
  smartlist_add(file_list, tor_strdup("dummy.xxx"));
  smartlist_add(file_list, tor_strdup("client2.auth_private"));

  return file_list;
}

static void
test_config_client_authorization(void *arg)
{
  int ret;
  char *conf = NULL;
  ed25519_public_key_t pk1, pk2;
  digest256map_t *global_map = NULL;
  char *key_dir = tor_strdup(get_fname("auth_keys"));

  (void) arg;

  MOCK(read_file_to_str, mock_read_file_to_str);
  MOCK(tor_listdir, mock_tor_listdir);
  MOCK(check_private_dir, mock_check_private_dir);

#define conf_fmt \
  "ClientOnionAuthDir %s\n"

  tor_asprintf(&conf, conf_fmt, key_dir);
  ret = helper_config_client(conf, 0);
  tor_free(conf);
  tt_int_op(ret, OP_EQ, 0);

#undef conf_fmt

  global_map = get_hs_client_auths_map();
  tt_int_op(digest256map_size(global_map), OP_EQ, 2);

  hs_parse_address("4acth47i6kxnvkewtm6q7ib2s3ufpo5sqbsnzjpbi7utijcltosqemad",
                   &pk1, NULL, NULL);
  hs_parse_address("25njqamcweflpvkl73j4szahhihoc4xt3ktcgjnpaingr5yhkenl5sid",
                   &pk2, NULL, NULL);

  tt_assert(digest256map_get(global_map, pk1.pubkey));
  tt_assert(digest256map_get(global_map, pk2.pubkey));

 done:
  tor_free(key_dir);
  hs_free_all();
  UNMOCK(read_file_to_str);
  UNMOCK(tor_listdir);
  UNMOCK(check_private_dir);
}

static entry_connection_t *
helper_build_socks_connection(const ed25519_public_key_t *service_pk,
                              int conn_state)
{
  entry_connection_t *socks = entry_connection_new(CONN_TYPE_AP, AF_INET);
  ENTRY_TO_EDGE_CONN(socks)->hs_ident = hs_ident_edge_conn_new(service_pk);
  TO_CONN(ENTRY_TO_EDGE_CONN(socks))->state = conn_state;
  smartlist_add(get_connection_array(), &socks->edge_.base_);
  return socks;
}

static void
test_desc_has_arrived_cleanup(void *arg)
{
  /* The goal of this test is to make sure we clean up everything in between
   * two descriptors from the same .onion. Because intro points can change
   * from one descriptor to another, once we received a new descriptor, we
   * need to cleanup the remaining circuits so they aren't used or selected
   * when establishing a connection with the newly stored descriptor.
   *
   * This test was created because of #27410. */

  int ret;
  char *desc_str = NULL;
  hs_descriptor_t *desc = NULL;
  const hs_descriptor_t *cached_desc;
  ed25519_keypair_t signing_kp;
  entry_connection_t *socks1 = NULL, *socks2 = NULL;
  hs_ident_dir_conn_t hs_dir_ident;
  dir_connection_t *dir_conn = NULL;

  (void) arg;

  hs_init();

  MOCK(networkstatus_get_reasonably_live_consensus,
       mock_networkstatus_get_reasonably_live_consensus);
  MOCK(connection_mark_unattached_ap_,
       mock_connection_mark_unattached_ap_);
  MOCK(router_have_minimum_dir_info,
       mock_router_have_minimum_dir_info_true);

  /* Set consensus time before our time so the cache lookup can always
   * validate that the entry is not expired. */
  parse_rfc1123_time("Sat, 26 Oct 1985 13:00:00 UTC", &mock_ns.valid_after);
  parse_rfc1123_time("Sat, 26 Oct 1985 14:00:00 UTC", &mock_ns.fresh_until);
  parse_rfc1123_time("Sat, 26 Oct 1985 16:00:00 UTC", &mock_ns.valid_until);

  /* Build a descriptor for a specific .onion. */
  ret = ed25519_keypair_generate(&signing_kp, 0);
  tt_int_op(ret, OP_EQ, 0);
  desc = hs_helper_build_hs_desc_with_ip(&signing_kp);
  tt_assert(desc);
  ret = hs_desc_encode_descriptor(desc, &signing_kp, NULL, &desc_str);
  tt_int_op(ret, OP_EQ, 0);

  /* Store in the client cache. */
  ret = hs_cache_store_as_client(desc_str, &signing_kp.pubkey);
  tt_int_op(ret, OP_EQ, HS_DESC_DECODE_OK);
  cached_desc = hs_cache_lookup_as_client(&signing_kp.pubkey);
  tt_assert(cached_desc);
  hs_helper_desc_equal(desc, cached_desc);

  /* Create two SOCKS connection for the same .onion both in the waiting for a
   * descriptor state. */
  socks1 = helper_build_socks_connection(&signing_kp.pubkey,
                                         AP_CONN_STATE_RENDDESC_WAIT);
  tt_assert(socks1);
  socks2 = helper_build_socks_connection(&signing_kp.pubkey,
                                         AP_CONN_STATE_RENDDESC_WAIT);
  tt_assert(socks2);

  /* Now, we'll make the intro points in the current descriptor unusable so
   * the hs_client_desc_has_arrived() will take the right code path that we
   * want to test that is the fetched descriptor has bad intro points. */
  SMARTLIST_FOREACH_BEGIN(desc->encrypted_data.intro_points,
                        hs_desc_intro_point_t *, ip) {
    hs_cache_client_intro_state_note(&signing_kp.pubkey,
                                     &ip->auth_key_cert->signed_key,
                                     INTRO_POINT_FAILURE_GENERIC);
  } SMARTLIST_FOREACH_END(ip);

  /* Simulate that a new descriptor just arrived. We should have both of our
   * SOCKS connection to be ended with a resolved failed. */
  hs_ident_dir_conn_init(&signing_kp.pubkey,
                         &desc->plaintext_data.blinded_pubkey, &hs_dir_ident);
  dir_conn = dir_connection_new(AF_INET);
  dir_conn->hs_ident = hs_ident_dir_conn_dup(&hs_dir_ident);
  hs_client_dir_fetch_done(dir_conn, "A reason", desc_str, 200);
  connection_free_minimal(TO_CONN(dir_conn));
  tt_int_op(socks1->edge_.end_reason, OP_EQ, END_STREAM_REASON_RESOLVEFAILED);
  tt_int_op(socks2->edge_.end_reason, OP_EQ, END_STREAM_REASON_RESOLVEFAILED);

  /* Now let say tor cleans up the intro state cache which resets all intro
   * point failure count. */
  hs_cache_client_intro_state_purge();

  /* Retrying all SOCKS which should basically do nothing since we don't have
   * any pending SOCKS connection in AP_CONN_STATE_RENDDESC_WAIT state. */
  retry_all_socks_conn_waiting_for_desc();

 done:
  connection_free_minimal(ENTRY_TO_CONN(socks1));
  connection_free_minimal(ENTRY_TO_CONN(socks2));
  hs_descriptor_free(desc);
  tor_free(desc_str);
  hs_free_all();

  UNMOCK(networkstatus_get_reasonably_live_consensus);
  UNMOCK(connection_mark_unattached_ap_);
  UNMOCK(router_have_minimum_dir_info);
}

static void
test_close_intro_circuits_new_desc(void *arg)
{
  int ret;
  ed25519_keypair_t service_kp;
  circuit_t *circ = NULL;
  origin_circuit_t *ocirc = NULL;
  hs_descriptor_t *desc1 = NULL, *desc2 = NULL;

  (void) arg;

  hs_init();
  rend_cache_init();

  /* This is needed because of the client cache expiration timestamp is based
   * on having a consensus. See cached_client_descriptor_has_expired(). */
  MOCK(networkstatus_get_reasonably_live_consensus,
       mock_networkstatus_get_reasonably_live_consensus);

  /* Set consensus time */
  parse_rfc1123_time("Sat, 26 Oct 1985 13:00:00 UTC",
                           &mock_ns.valid_after);
  parse_rfc1123_time("Sat, 26 Oct 1985 14:00:00 UTC",
                           &mock_ns.fresh_until);
  parse_rfc1123_time("Sat, 26 Oct 1985 16:00:00 UTC",
                           &mock_ns.valid_until);

  /* Generate service keypair */
  tt_int_op(0, OP_EQ, ed25519_keypair_generate(&service_kp, 0));

  /* Create and add to the global list a dummy client introduction circuits.
   * We'll then make sure the hs_ident is attached to a dummy descriptor. */
  circ = dummy_origin_circuit_new(0);
  tt_assert(circ);
  circ->purpose = CIRCUIT_PURPOSE_C_INTRODUCING;
  ocirc = TO_ORIGIN_CIRCUIT(circ);

  /* Build a descriptor _without_ client authorization and thus not
   * decryptable. Make sure the close circuit code path is not triggered. */
  {
    char *desc_encoded = NULL;
    uint8_t descriptor_cookie[HS_DESC_DESCRIPTOR_COOKIE_LEN];
    curve25519_keypair_t client_kp;
    hs_descriptor_t *desc = NULL;

    tt_int_op(0, OP_EQ, curve25519_keypair_generate(&client_kp, 0));
    crypto_rand((char *) descriptor_cookie, sizeof(descriptor_cookie));

    desc = hs_helper_build_hs_desc_with_client_auth(descriptor_cookie,
                                                    &client_kp.pubkey,
                                                    &service_kp);
    tt_assert(desc);
    ret = hs_desc_encode_descriptor(desc, &service_kp, descriptor_cookie,
                                    &desc_encoded);
    tt_int_op(ret, OP_EQ, 0);
    /* Associate descriptor intro key with the dummy circuit. */
    const hs_desc_intro_point_t *ip =
      smartlist_get(desc->encrypted_data.intro_points, 0);
    tt_assert(ip);
    ocirc->hs_ident = hs_ident_circuit_new(&service_kp.pubkey);
    ed25519_pubkey_copy(&ocirc->hs_ident->intro_auth_pk,
                        &ip->auth_key_cert->signed_key);
    hs_descriptor_free(desc);
    tt_assert(desc_encoded);
    /* Put it in the cache. Should not be decrypted since the client
     * authorization creds were not added to the global map. */
    ret = hs_cache_store_as_client(desc_encoded, &service_kp.pubkey);
    tor_free(desc_encoded);
    tt_int_op(ret, OP_EQ, HS_DESC_DECODE_NEED_CLIENT_AUTH);

    /* Clean cache with a future timestamp. It will trigger the clean up and
     * attempt to close the circuit but only if the descriptor is decryptable.
     * Cache object should be removed and circuit untouched. */
    hs_cache_clean_as_client(mock_ns.valid_after + (60 * 60 * 24));
    tt_assert(!hs_cache_lookup_as_client(&service_kp.pubkey));

    /* Make sure the circuit still there. */
    tt_assert(circuit_get_next_intro_circ(NULL, true));
    /* Get rid of the ident, it will be replaced in the next tests. */
    hs_ident_circuit_free(ocirc->hs_ident);
  }

  /* Build the first descriptor and cache it. */
  {
    char *encoded;
    desc1 = hs_helper_build_hs_desc_with_ip(&service_kp);
    tt_assert(desc1);
    ret = hs_desc_encode_descriptor(desc1, &service_kp, NULL, &encoded);
    tt_int_op(ret, OP_EQ, 0);
    tt_assert(encoded);

    /* Store it */
    ret = hs_cache_store_as_client(encoded, &service_kp.pubkey);
    tt_int_op(ret, OP_EQ, HS_DESC_DECODE_OK);
    tor_free(encoded);
    tt_assert(hs_cache_lookup_as_client(&service_kp.pubkey));
  }

  /* We'll pick one introduction point and associate it with the circuit. */
  {
    const hs_desc_intro_point_t *ip =
      smartlist_get(desc1->encrypted_data.intro_points, 0);
    tt_assert(ip);
    ocirc->hs_ident = hs_ident_circuit_new(&service_kp.pubkey);
    ed25519_pubkey_copy(&ocirc->hs_ident->intro_auth_pk,
                        &ip->auth_key_cert->signed_key);
  }

  /* Before we are about to clean up the intro circuits, make sure it is
   * actually there. */
  tt_assert(circuit_get_next_intro_circ(NULL, true));

  /* Build the second descriptor for the same service and cache it. */
  {
    char *encoded;
    desc2 = hs_helper_build_hs_desc_with_ip(&service_kp);
    tt_assert(desc2);
    tt_mem_op(&desc1->plaintext_data.signing_pubkey, OP_EQ,
              &desc2->plaintext_data.signing_pubkey, ED25519_PUBKEY_LEN);
    /* To replace the existing descriptor, the revision counter needs to be
     * bigger. */
    desc2->plaintext_data.revision_counter =
      desc1->plaintext_data.revision_counter + 1;

    ret = hs_desc_encode_descriptor(desc2, &service_kp, NULL, &encoded);
    tt_int_op(ret, OP_EQ, 0);
    tt_assert(encoded);

    ret = hs_cache_store_as_client(encoded, &service_kp.pubkey);
    tt_int_op(ret, OP_EQ, HS_DESC_DECODE_OK);
    tor_free(encoded);
    tt_assert(hs_cache_lookup_as_client(&service_kp.pubkey));
  }

  /* Once stored, our intro circuit should be closed because it is related to
   * an old introduction point that doesn't exists anymore. */
  tt_assert(!circuit_get_next_intro_circ(NULL, true));

 done:
  circuit_free(circ);
  hs_descriptor_free(desc1);
  hs_descriptor_free(desc2);
  hs_free_all();
  UNMOCK(networkstatus_get_reasonably_live_consensus);
}

static void
test_close_intro_circuits_cache_clean(void *arg)
{
  int ret;
  ed25519_keypair_t service_kp;
  circuit_t *circ = NULL;
  origin_circuit_t *ocirc = NULL;
  hs_descriptor_t *desc1 = NULL;

  (void) arg;

  hs_init();
  rend_cache_init();

  /* This is needed because of the client cache expiration timestamp is based
   * on having a consensus. See cached_client_descriptor_has_expired(). */
  MOCK(networkstatus_get_reasonably_live_consensus,
       mock_networkstatus_get_reasonably_live_consensus);

  /* Set consensus time */
  parse_rfc1123_time("Sat, 26 Oct 1985 13:00:00 UTC",
                     &mock_ns.valid_after);
  parse_rfc1123_time("Sat, 26 Oct 1985 14:00:00 UTC",
                     &mock_ns.fresh_until);
  parse_rfc1123_time("Sat, 26 Oct 1985 16:00:00 UTC",
                     &mock_ns.valid_until);

  /* Generate service keypair */
  tt_int_op(0, OP_EQ, ed25519_keypair_generate(&service_kp, 0));

  /* Create and add to the global list a dummy client introduction circuits.
   * We'll then make sure the hs_ident is attached to a dummy descriptor. */
  circ = dummy_origin_circuit_new(0);
  tt_assert(circ);
  circ->purpose = CIRCUIT_PURPOSE_C_INTRODUCING;
  ocirc = TO_ORIGIN_CIRCUIT(circ);

  /* Build the first descriptor and cache it. */
  {
    char *encoded;
    desc1 = hs_helper_build_hs_desc_with_ip(&service_kp);
    tt_assert(desc1);
    ret = hs_desc_encode_descriptor(desc1, &service_kp, NULL, &encoded);
    tt_int_op(ret, OP_EQ, 0);
    tt_assert(encoded);

    /* Store it */
    ret = hs_cache_store_as_client(encoded, &service_kp.pubkey);
    tt_int_op(ret, OP_EQ, 0);
    tor_free(encoded);
    tt_assert(hs_cache_lookup_as_client(&service_kp.pubkey));
  }

  /* We'll pick one introduction point and associate it with the circuit. */
  {
    const hs_desc_intro_point_t *ip =
      smartlist_get(desc1->encrypted_data.intro_points, 0);
    tt_assert(ip);
    ocirc->hs_ident = hs_ident_circuit_new(&service_kp.pubkey);
    ed25519_pubkey_copy(&ocirc->hs_ident->intro_auth_pk,
                        &ip->auth_key_cert->signed_key);
  }

  /* Before we are about to clean up the intro circuits, make sure it is
   * actually there. */
  tt_assert(circuit_get_next_intro_circ(NULL, true));

  /* Cleanup the client cache. The ns valid after time is what decides if the
   * descriptor has expired so put it in the future enough (72h) so we are
   * sure to always expire. */
  mock_ns.valid_after = approx_time() + (72 * 24 * 60 * 60);
  hs_cache_clean_as_client(0);

  /* Once stored, our intro circuit should be closed because it is related to
   * an old introduction point that doesn't exists anymore. */
  tt_assert(!circuit_get_next_intro_circ(NULL, true));

 done:
  circuit_free(circ);
  hs_descriptor_free(desc1);
  hs_free_all();
  rend_cache_free_all();
  UNMOCK(networkstatus_get_reasonably_live_consensus);
}

static void
test_socks_hs_errors(void *arg)
{
  int ret;
  char digest[DIGEST_LEN];
  char *desc_encoded = NULL;
  circuit_t *circ = NULL;
  origin_circuit_t *ocirc = NULL;
  tor_addr_t addr;
  ed25519_keypair_t service_kp;
  ed25519_keypair_t signing_kp;
  entry_connection_t *socks_conn = NULL;
  dir_connection_t *dir_conn = NULL;
  hs_descriptor_t *desc = NULL;
  uint8_t descriptor_cookie[HS_DESC_DESCRIPTOR_COOKIE_LEN];

  (void) arg;

  MOCK(networkstatus_get_reasonably_live_consensus,
       mock_networkstatus_get_reasonably_live_consensus);
  MOCK(connection_mark_unattached_ap_,
       mock_connection_mark_unattached_ap_no_close);
  MOCK(read_file_to_str, mock_read_file_to_str);
  MOCK(tor_listdir, mock_tor_listdir);
  MOCK(check_private_dir, mock_check_private_dir);

    /* Set consensus time */
  parse_rfc1123_time("Sat, 26 Oct 1985 13:00:00 UTC",
                           &mock_ns.valid_after);
  parse_rfc1123_time("Sat, 26 Oct 1985 14:00:00 UTC",
                           &mock_ns.fresh_until);
  parse_rfc1123_time("Sat, 26 Oct 1985 16:00:00 UTC",
                           &mock_ns.valid_until);

  hs_init();

  ret = ed25519_keypair_generate(&service_kp, 0);
  tt_int_op(ret, OP_EQ, 0);
  ret = ed25519_keypair_generate(&signing_kp, 0);
  tt_int_op(ret, OP_EQ, 0);

  socks_conn = helper_build_socks_connection(&service_kp.pubkey,
                                             AP_CONN_STATE_RENDDESC_WAIT);
  tt_assert(socks_conn);

  /* Create directory connection. */
  dir_conn = dir_connection_new(AF_INET);
  dir_conn->hs_ident = tor_malloc_zero(sizeof(hs_ident_dir_conn_t));
  TO_CONN(dir_conn)->purpose = DIR_PURPOSE_FETCH_HSDESC;
  ed25519_pubkey_copy(&dir_conn->hs_ident->identity_pk, &service_kp.pubkey);

  /* Encode descriptor so we can decode it. */
  desc = hs_helper_build_hs_desc_with_ip(&service_kp);
  tt_assert(desc);

  /* Before testing the client authentication error code, encode the
   * descriptor with no client auth. */
  ret = hs_desc_encode_descriptor(desc, &service_kp, NULL, &desc_encoded);
  tt_int_op(ret, OP_EQ, 0);
  tt_assert(desc_encoded);

  /*
   * Test the introduction failure codes (X'F2' and X'F7')
   */

  /* First, we have to put all the IPs in the failure cache. */
  SMARTLIST_FOREACH_BEGIN(desc->encrypted_data.intro_points,
                          hs_desc_intro_point_t *, ip) {
    hs_cache_client_intro_state_note(&service_kp.pubkey,
                                     &ip->auth_key_cert->signed_key,
                                     INTRO_POINT_FAILURE_GENERIC);
  } SMARTLIST_FOREACH_END(ip);

  hs_client_dir_fetch_done(dir_conn, "Reason", desc_encoded, 200);
  tt_int_op(socks_conn->socks_request->socks_extended_error_code, OP_EQ,
            SOCKS5_HS_INTRO_FAILED);

  /* Purge client cache of the descriptor so we can go again. */
  hs_cache_purge_as_client();

  /* Second, set all failures to be time outs. */
  SMARTLIST_FOREACH_BEGIN(desc->encrypted_data.intro_points,
                          hs_desc_intro_point_t *, ip) {
    hs_cache_client_intro_state_note(&service_kp.pubkey,
                                     &ip->auth_key_cert->signed_key,
                                     INTRO_POINT_FAILURE_TIMEOUT);
  } SMARTLIST_FOREACH_END(ip);

  hs_client_dir_fetch_done(dir_conn, "Reason", desc_encoded, 200);
  tt_int_op(socks_conn->socks_request->socks_extended_error_code, OP_EQ,
            SOCKS5_HS_INTRO_TIMEDOUT);

  /* Purge client cache of the descriptor so we can go again. */
  hs_cache_purge_as_client();

  /*
   * Test the rendezvous failure codes (X'F3')
   */

  circ = dummy_origin_circuit_new(0);
  tt_assert(circ);
  circ->purpose = CIRCUIT_PURPOSE_C_REND_READY;
  ocirc = TO_ORIGIN_CIRCUIT(circ);
  ocirc->hs_ident = hs_ident_circuit_new(&service_kp.pubkey);
  ocirc->build_state = tor_malloc_zero(sizeof(cpath_build_state_t));
  /* Code path will log this exit so build it. */
  ocirc->build_state->chosen_exit = extend_info_new("TestNickname", digest,
                                                    NULL, NULL, NULL, &addr,
                                                    4242);
  /* Attach socks connection to this rendezvous circuit. */
  ocirc->p_streams = ENTRY_TO_EDGE_CONN(socks_conn);
  /* Trigger the rendezvous failure. Timeout the circuit and free. */
  circuit_mark_for_close(circ, END_CIRC_REASON_TIMEOUT);

  tt_int_op(socks_conn->socks_request->socks_extended_error_code, OP_EQ,
            SOCKS5_HS_REND_FAILED);

  /*
   * Test client authorization codes.
   */

  tor_free(desc_encoded);
  crypto_rand((char *) descriptor_cookie, sizeof(descriptor_cookie));
  ret = hs_desc_encode_descriptor(desc, &service_kp, descriptor_cookie,
                                  &desc_encoded);
  tt_int_op(ret, OP_EQ, 0);
  tt_assert(desc_encoded);

  /* Try decoding. Point this to an existing descriptor. The following should
   * fail thus the desc_out should be set to NULL. */
  hs_descriptor_t *desc_out = desc;
  ret = hs_client_decode_descriptor(desc_encoded, &service_kp.pubkey,
                                    &desc_out);
  tt_int_op(ret, OP_EQ, HS_DESC_DECODE_NEED_CLIENT_AUTH);
  tt_assert(desc_out == NULL);

  /* The caching will fail to decrypt because the descriptor_cookie used above
   * is not known to the HS subsystem. This will lead to a missing client
   * auth. */
  hs_client_dir_fetch_done(dir_conn, "Reason", desc_encoded, 200);

  tt_int_op(socks_conn->socks_request->socks_extended_error_code, OP_EQ,
            SOCKS5_HS_MISSING_CLIENT_AUTH);

  /* Add in the global client auth list bad creds for this service. */
  helper_add_random_client_auth(&service_kp.pubkey);

  ret = hs_client_decode_descriptor(desc_encoded, &service_kp.pubkey,
                                    &desc_out);
  tt_int_op(ret, OP_EQ, HS_DESC_DECODE_BAD_CLIENT_AUTH);
  tt_assert(desc_out == NULL);

  /* Simmulate a fetch done again. This should replace the cached descriptor
   * and signal a bad client authorization. */
  hs_client_dir_fetch_done(dir_conn, "Reason", desc_encoded, 200);
  tt_int_op(socks_conn->socks_request->socks_extended_error_code, OP_EQ,
            SOCKS5_HS_BAD_CLIENT_AUTH);

 done:
  connection_free_minimal(ENTRY_TO_CONN(socks_conn));
  connection_free_minimal(TO_CONN(dir_conn));
  hs_descriptor_free(desc);
  tor_free(desc_encoded);
  circuit_free(circ);

  hs_free_all();

  UNMOCK(networkstatus_get_reasonably_live_consensus);
  UNMOCK(connection_mark_unattached_ap_);
  UNMOCK(read_file_to_str);
  UNMOCK(tor_listdir);
  UNMOCK(check_private_dir);
}

static void
test_close_intro_circuit_failure(void *arg)
{
  char digest[DIGEST_LEN];
  circuit_t *circ = NULL;
  ed25519_keypair_t service_kp, intro_kp;
  origin_circuit_t *ocirc = NULL;
  tor_addr_t addr;
  const hs_cache_intro_state_t *entry;

  (void) arg;

  hs_init();

  /* Generate service keypair */
  tt_int_op(0, OP_EQ, ed25519_keypair_generate(&service_kp, 0));
  tt_int_op(0, OP_EQ, ed25519_keypair_generate(&intro_kp, 0));

  /* Create and add to the global list a dummy client introduction circuit at
   * the ACK WAIT state. */
  circ = dummy_origin_circuit_new(0);
  tt_assert(circ);
  circ->purpose = CIRCUIT_PURPOSE_C_INTRODUCE_ACK_WAIT;
  ocirc = TO_ORIGIN_CIRCUIT(circ);
  ocirc->hs_ident = hs_ident_circuit_new(&service_kp.pubkey);
  ocirc->build_state = tor_malloc_zero(sizeof(cpath_build_state_t));
  /* Code path will log this exit so build it. */
  ocirc->build_state->chosen_exit = extend_info_new("TestNickname", digest,
                                                    NULL, NULL, NULL, &addr,
                                                    4242);
  ed25519_pubkey_copy(&ocirc->hs_ident->intro_auth_pk, &intro_kp.pubkey);

  /* We'll make for close the circuit for a timeout failure. It should _NOT_
   * end up in the failure cache just yet. We do that on free() only. */
  circuit_mark_for_close(circ, END_CIRC_REASON_TIMEOUT);
  tt_assert(!hs_cache_client_intro_state_find(&service_kp.pubkey,
                                              &intro_kp.pubkey));
  /* Time to free. It should get removed. */
  circuit_free(circ);
  entry = hs_cache_client_intro_state_find(&service_kp.pubkey,
                                           &intro_kp.pubkey);
  tt_assert(entry);
  tt_uint_op(entry->timed_out, OP_EQ, 1);
  hs_cache_client_intro_state_purge();

  /* Again, create and add to the global list a dummy client introduction
   * circuit at the INTRODUCING state. */
  circ = dummy_origin_circuit_new(0);
  tt_assert(circ);
  circ->purpose = CIRCUIT_PURPOSE_C_INTRODUCING;
  ocirc = TO_ORIGIN_CIRCUIT(circ);
  ocirc->hs_ident = hs_ident_circuit_new(&service_kp.pubkey);
  ocirc->build_state = tor_malloc_zero(sizeof(cpath_build_state_t));
  /* Code path will log this exit so build it. */
  ocirc->build_state->chosen_exit = extend_info_new("TestNickname", digest,
                                                    NULL, NULL, NULL, &addr,
                                                    4242);
  ed25519_pubkey_copy(&ocirc->hs_ident->intro_auth_pk, &intro_kp.pubkey);

  /* On free, we should get an unreachable failure. */
  circuit_free(circ);
  entry = hs_cache_client_intro_state_find(&service_kp.pubkey,
                                           &intro_kp.pubkey);
  tt_assert(entry);
  tt_uint_op(entry->unreachable_count, OP_EQ, 1);
  hs_cache_client_intro_state_purge();

  /* Again, create and add to the global list a dummy client introduction
   * circuit at the INTRODUCING state but we'll close it for timeout. It
   * should not be noted as a timeout failure. */
  circ = dummy_origin_circuit_new(0);
  tt_assert(circ);
  circ->purpose = CIRCUIT_PURPOSE_C_INTRODUCING;
  ocirc = TO_ORIGIN_CIRCUIT(circ);
  ocirc->hs_ident = hs_ident_circuit_new(&service_kp.pubkey);
  ocirc->build_state = tor_malloc_zero(sizeof(cpath_build_state_t));
  /* Code path will log this exit so build it. */
  ocirc->build_state->chosen_exit = extend_info_new("TestNickname", digest,
                                                    NULL, NULL, NULL, &addr,
                                                    4242);
  ed25519_pubkey_copy(&ocirc->hs_ident->intro_auth_pk, &intro_kp.pubkey);

  circuit_mark_for_close(circ, END_CIRC_REASON_TIMEOUT);
  circuit_free(circ);
  tt_assert(!hs_cache_client_intro_state_find(&service_kp.pubkey,
                                              &intro_kp.pubkey));

  /* Again, create and add to the global list a dummy client introduction
   * circuit at the INTRODUCING state but without a chosen_exit. In theory, it
   * can not happen but we'll make sure it doesn't end up in the failure cache
   * anyway. */
  circ = dummy_origin_circuit_new(0);
  tt_assert(circ);
  circ->purpose = CIRCUIT_PURPOSE_C_INTRODUCING;
  ocirc = TO_ORIGIN_CIRCUIT(circ);
  ocirc->hs_ident = hs_ident_circuit_new(&service_kp.pubkey);
  ed25519_pubkey_copy(&ocirc->hs_ident->intro_auth_pk, &intro_kp.pubkey);

  circuit_free(circ);
  tt_assert(!hs_cache_client_intro_state_find(&service_kp.pubkey,
                                              &intro_kp.pubkey));

 done:
  circuit_free(circ);
  hs_free_all();
}

static void
test_purge_ephemeral_client_auth(void *arg)
{
  ed25519_keypair_t service_kp;
  hs_client_service_authorization_t *auth = NULL;
  hs_client_register_auth_status_t status;

  (void) arg;

  /* We will try to write on disk client credentials. */
  MOCK(check_private_dir, mock_check_private_dir);
  MOCK(get_options, mock_get_options);
  MOCK(write_str_to_file, mock_write_str_to_file);

  /* Bogus directory so when we try to write the permanent client
   * authorization data to disk, we don't fail. See
   * store_permanent_client_auth_credentials() for more details. */
  mocked_options.ClientOnionAuthDir = tor_strdup("auth_dir");

  hs_init();

  /* Generate service keypair */
  tt_int_op(0, OP_EQ, ed25519_keypair_generate(&service_kp, 0));

  /* Generate a client authorization object. */
  auth = tor_malloc_zero(sizeof(hs_client_service_authorization_t));

  /* Set it up. No flags meaning it is ephemeral. */
  curve25519_secret_key_generate(&auth->enc_seckey, 0);
  hs_build_address(&service_kp.pubkey, HS_VERSION_THREE, auth->onion_address);
  auth->flags = 0;

  /* Confirm that there is nothing in the client auth map. It is unallocated
   * until we add the first entry. */
  tt_assert(!get_hs_client_auths_map());

  /* Add an entry to the client auth list. We loose ownership of the auth
   * object so nullify it. */
  status = hs_client_register_auth_credentials(auth);
  auth = NULL;
  tt_int_op(status, OP_EQ, REGISTER_SUCCESS);

  /* We should have the entry now. */
  digest256map_t *client_auths = get_hs_client_auths_map();
  tt_assert(client_auths);
  tt_int_op(digest256map_size(client_auths), OP_EQ, 1);

  /* Purge the cache that should remove all ephemeral values. */
  purge_ephemeral_client_auth();
  tt_int_op(digest256map_size(client_auths), OP_EQ, 0);

  /* Now add a new authorization object but permanent. */
  /* Generate a client authorization object. */
  auth = tor_malloc_zero(sizeof(hs_client_service_authorization_t));
  curve25519_secret_key_generate(&auth->enc_seckey, 0);
  hs_build_address(&service_kp.pubkey, HS_VERSION_THREE, auth->onion_address);
  auth->flags = CLIENT_AUTH_FLAG_IS_PERMANENT;

  /* Add an entry to the client auth list. We loose ownership of the auth
   * object so nullify it. */
  status = hs_client_register_auth_credentials(auth);
  auth = NULL;
  tt_int_op(status, OP_EQ, REGISTER_SUCCESS);
  tt_int_op(digest256map_size(client_auths), OP_EQ, 1);

  /* Purge again, the entry should still be there. */
  purge_ephemeral_client_auth();
  tt_int_op(digest256map_size(client_auths), OP_EQ, 1);

 done:
  client_service_authorization_free(auth);
  hs_free_all();
  tor_free(mocked_options.ClientOnionAuthDir);

  UNMOCK(check_private_dir);
  UNMOCK(get_options);
  UNMOCK(write_str_to_file);
}

struct testcase_t hs_client_tests[] = {
  { "e2e_rend_circuit_setup_legacy", test_e2e_rend_circuit_setup_legacy,
    TT_FORK, NULL, NULL },
  { "e2e_rend_circuit_setup", test_e2e_rend_circuit_setup,
    TT_FORK, NULL, NULL },
  { "client_pick_intro", test_client_pick_intro,
    TT_FORK, NULL, NULL },
  { "descriptor_fetch", test_descriptor_fetch,
    TT_FORK, NULL, NULL },
  { "auth_key_filename_is_valid", test_auth_key_filename_is_valid, TT_FORK,
    NULL, NULL },
  { "parse_auth_file_content", test_parse_auth_file_content, TT_FORK,
    NULL, NULL },
  { "config_client_authorization", test_config_client_authorization,
    TT_FORK, NULL, NULL },
  { "desc_has_arrived_cleanup", test_desc_has_arrived_cleanup,
    TT_FORK, NULL, NULL },
  { "close_intro_circuit_failure", test_close_intro_circuit_failure,
    TT_FORK, NULL, NULL },
  { "close_intro_circuits_new_desc", test_close_intro_circuits_new_desc,
    TT_FORK, NULL, NULL },
  { "close_intro_circuits_cache_clean", test_close_intro_circuits_cache_clean,
    TT_FORK, NULL, NULL },

  /* SOCKS5 Extended Error Code. */
  { "socks_hs_errors", test_socks_hs_errors, TT_FORK, NULL, NULL },

  /* Client authorization. */
  { "purge_ephemeral_client_auth", test_purge_ephemeral_client_auth, TT_FORK,
    NULL, NULL },

  END_OF_TESTCASES
};
