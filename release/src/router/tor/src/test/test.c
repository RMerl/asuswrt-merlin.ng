/* Copyright (c) 2001-2004, Roger Dingledine.
->a * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file test.c
 * \brief Unit tests for many pieces of the lower level Tor modules.
 **/

#include "orconfig.h"
#include "lib/crypt_ops/crypto_dh.h"
#include "lib/crypt_ops/crypto_rand.h"
#include "app/config/or_state_st.h"
#include "test/rng_test_helpers.h"

#include <stdio.h>
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif

#ifdef _WIN32
/* For mkdir() */
#include <direct.h>
#else
#include <dirent.h>
#endif /* defined(_WIN32) */

#include <math.h>

/* These macros pull in declarations for some functions and structures that
 * are typically file-private. */
#define ROUTER_PRIVATE
#define CIRCUITSTATS_PRIVATE
#define CIRCUITLIST_PRIVATE
#define MAINLOOP_PRIVATE
#define STATEFILE_PRIVATE

#include "core/or/or.h"
#include "lib/err/backtrace.h"
#include "lib/buf/buffers.h"
#include "core/or/circuitlist.h"
#include "core/or/circuitstats.h"
#include "lib/compress/compress.h"
#include "app/config/config.h"
#include "core/or/connection_edge.h"
#include "core/or/extendinfo.h"
#include "feature/rend/rendcommon.h"
#include "feature/rend/rendcache.h"
#include "feature/rend/rendparse.h"
#include "test/test.h"
#include "core/mainloop/mainloop.h"
#include "lib/memarea/memarea.h"
#include "core/or/onion.h"
#include "core/crypto/onion_ntor.h"
#include "core/crypto/onion_fast.h"
#include "core/crypto/onion_tap.h"
#include "core/or/policies.h"
#include "app/config/statefile.h"
#include "lib/crypt_ops/crypto_curve25519.h"

#include "core/or/extend_info_st.h"
#include "core/or/or_circuit_st.h"
#include "feature/rend/rend_encoded_v2_service_descriptor_st.h"
#include "feature/rend/rend_intro_point_st.h"
#include "feature/rend/rend_service_descriptor_st.h"
#include "feature/relay/onion_queue.h"

/** Run unit tests for the onion handshake code. */
static void
test_onion_handshake(void *arg)
{
  /* client-side */
  crypto_dh_t *c_dh = NULL;
  char c_buf[TAP_ONIONSKIN_CHALLENGE_LEN];
  char c_keys[40];
  /* server-side */
  char s_buf[TAP_ONIONSKIN_REPLY_LEN];
  char s_keys[40];
  int i;
  /* shared */
  crypto_pk_t *pk = NULL, *pk2 = NULL;

  (void)arg;
  pk = pk_generate(0);
  pk2 = pk_generate(1);

  /* client handshake 1. */
  memset(c_buf, 0, TAP_ONIONSKIN_CHALLENGE_LEN);
  tt_assert(! onion_skin_TAP_create(pk, &c_dh, c_buf));

  for (i = 1; i <= 3; ++i) {
    crypto_pk_t *k1, *k2;
    if (i==1) {
      /* server handshake: only one key known. */
      k1 = pk;  k2 = NULL;
    } else if (i==2) {
      /* server handshake: try the right key first. */
      k1 = pk;  k2 = pk2;
    } else {
      /* server handshake: try the right key second. */
      k1 = pk2; k2 = pk;
    }

    memset(s_buf, 0, TAP_ONIONSKIN_REPLY_LEN);
    memset(s_keys, 0, 40);
    tt_assert(! onion_skin_TAP_server_handshake(c_buf, k1, k2,
                                                  s_buf, s_keys, 40));

    /* client handshake 2 */
    memset(c_keys, 0, 40);
    tt_assert(! onion_skin_TAP_client_handshake(c_dh, s_buf, c_keys,
                                                40, NULL));

    tt_mem_op(c_keys,OP_EQ, s_keys, 40);
    memset(s_buf, 0, 40);
    tt_mem_op(c_keys,OP_NE, s_buf, 40);
  }
 done:
  crypto_dh_free(c_dh);
  crypto_pk_free(pk);
  crypto_pk_free(pk2);
}

static void
test_bad_onion_handshake(void *arg)
{
  char junk_buf[TAP_ONIONSKIN_CHALLENGE_LEN];
  char junk_buf2[TAP_ONIONSKIN_CHALLENGE_LEN];
  /* client-side */
  crypto_dh_t *c_dh = NULL;
  char c_buf[TAP_ONIONSKIN_CHALLENGE_LEN];
  char c_keys[40];
  /* server-side */
  char s_buf[TAP_ONIONSKIN_REPLY_LEN];
  char s_keys[40];
  /* shared */
  crypto_pk_t *pk = NULL, *pk2 = NULL;

  (void)arg;

  pk = pk_generate(0);
  pk2 = pk_generate(1);

  /* Server: Case 1: the encrypted data is degenerate. */
  memset(junk_buf, 0, sizeof(junk_buf));
  crypto_pk_obsolete_public_hybrid_encrypt(pk,
                               junk_buf2, TAP_ONIONSKIN_CHALLENGE_LEN,
                               junk_buf, DH1024_KEY_LEN,
                               PK_PKCS1_OAEP_PADDING, 1);
  tt_int_op(-1, OP_EQ,
            onion_skin_TAP_server_handshake(junk_buf2, pk, NULL,
                                            s_buf, s_keys, 40));

  /* Server: Case 2: the encrypted data is not long enough. */
  memset(junk_buf, 0, sizeof(junk_buf));
  memset(junk_buf2, 0, sizeof(junk_buf2));
  crypto_pk_public_encrypt(pk, junk_buf2, sizeof(junk_buf2),
                               junk_buf, 48, PK_PKCS1_OAEP_PADDING);
  tt_int_op(-1, OP_EQ,
            onion_skin_TAP_server_handshake(junk_buf2, pk, NULL,
                                            s_buf, s_keys, 40));

  /* client handshake 1: do it straight. */
  memset(c_buf, 0, TAP_ONIONSKIN_CHALLENGE_LEN);
  tt_assert(! onion_skin_TAP_create(pk, &c_dh, c_buf));

  /* Server: Case 3: we just don't have the right key. */
  tt_int_op(-1, OP_EQ,
            onion_skin_TAP_server_handshake(c_buf, pk2, NULL,
                                            s_buf, s_keys, 40));

  /* Server: Case 4: The RSA-encrypted portion is corrupt. */
  c_buf[64] ^= 33;
  tt_int_op(-1, OP_EQ,
            onion_skin_TAP_server_handshake(c_buf, pk, NULL,
                                            s_buf, s_keys, 40));
  c_buf[64] ^= 33;

  /* (Let the server proceed) */
  tt_int_op(0, OP_EQ,
            onion_skin_TAP_server_handshake(c_buf, pk, NULL,
                                            s_buf, s_keys, 40));

  /* Client: Case 1: The server sent back junk. */
  const char *msg = NULL;
  s_buf[64] ^= 33;
  tt_int_op(-1, OP_EQ,
            onion_skin_TAP_client_handshake(c_dh, s_buf, c_keys, 40, &msg));
  s_buf[64] ^= 33;
  tt_str_op(msg, OP_EQ, "Digest DOES NOT MATCH on onion handshake. "
            "Bug or attack.");

  /* Let the client finish; make sure it can. */
  msg = NULL;
  tt_int_op(0, OP_EQ,
            onion_skin_TAP_client_handshake(c_dh, s_buf, c_keys, 40, &msg));
  tt_mem_op(s_keys,OP_EQ, c_keys, 40);
  tt_ptr_op(msg, OP_EQ, NULL);

  /* Client: Case 2: The server sent back a degenerate DH. */
  memset(s_buf, 0, sizeof(s_buf));
  tt_int_op(-1, OP_EQ,
            onion_skin_TAP_client_handshake(c_dh, s_buf, c_keys, 40, &msg));
  tt_str_op(msg, OP_EQ, "DH computation failed.");

 done:
  crypto_dh_free(c_dh);
  crypto_pk_free(pk);
  crypto_pk_free(pk2);
}

static void
test_ntor_handshake(void *arg)
{
  /* client-side */
  ntor_handshake_state_t *c_state = NULL;
  uint8_t c_buf[NTOR_ONIONSKIN_LEN];
  uint8_t c_keys[400];

  /* server-side */
  di_digest256_map_t *s_keymap=NULL;
  curve25519_keypair_t s_keypair;
  uint8_t s_buf[NTOR_REPLY_LEN];
  uint8_t s_keys[400];

  /* shared */
  const curve25519_public_key_t *server_pubkey;
  uint8_t node_id[20] = "abcdefghijklmnopqrst";

  (void) arg;

  /* Make the server some keys */
  curve25519_secret_key_generate(&s_keypair.seckey, 0);
  curve25519_public_key_generate(&s_keypair.pubkey, &s_keypair.seckey);
  dimap_add_entry(&s_keymap, s_keypair.pubkey.public_key, &s_keypair);
  server_pubkey = &s_keypair.pubkey;

  /* client handshake 1. */
  memset(c_buf, 0, NTOR_ONIONSKIN_LEN);
  tt_int_op(0, OP_EQ, onion_skin_ntor_create(node_id, server_pubkey,
                                          &c_state, c_buf));

  /* server handshake */
  memset(s_buf, 0, NTOR_REPLY_LEN);
  memset(s_keys, 0, 40);
  tt_int_op(0, OP_EQ, onion_skin_ntor_server_handshake(c_buf, s_keymap, NULL,
                                                    node_id,
                                                    s_buf, s_keys, 400));

  /* client handshake 2 */
  memset(c_keys, 0, 40);
  tt_int_op(0, OP_EQ, onion_skin_ntor_client_handshake(c_state, s_buf,
                                                       c_keys, 400, NULL));

  tt_mem_op(c_keys,OP_EQ, s_keys, 400);
  memset(s_buf, 0, 40);
  tt_mem_op(c_keys,OP_NE, s_buf, 40);

  /* Now try with a bogus server response. Zero input should trigger
   * All The Problems. */
  memset(c_keys, 0, 400);
  memset(s_buf, 0, NTOR_REPLY_LEN);
  const char *msg = NULL;
  tt_int_op(-1, OP_EQ, onion_skin_ntor_client_handshake(c_state, s_buf,
                                                        c_keys, 400, &msg));
  tt_str_op(msg, OP_EQ, "Zero output from curve25519 handshake");

 done:
  ntor_handshake_state_free(c_state);
  dimap_free(s_keymap, NULL);
}

static void
test_fast_handshake(void *arg)
{
  /* tests for the obsolete "CREATE_FAST" handshake. */
  (void) arg;
  fast_handshake_state_t *state = NULL;
  uint8_t client_handshake[CREATE_FAST_LEN];
  uint8_t server_handshake[CREATED_FAST_LEN];
  uint8_t s_keys[100], c_keys[100];

  /* First, test an entire handshake. */
  memset(client_handshake, 0, sizeof(client_handshake));
  tt_int_op(0, OP_EQ, fast_onionskin_create(&state, client_handshake));
  tt_assert(! fast_mem_is_zero((char*)client_handshake,
                              sizeof(client_handshake)));

  tt_int_op(0, OP_EQ,
            fast_server_handshake(client_handshake, server_handshake,
                                  s_keys, 100));
  const char *msg = NULL;
  tt_int_op(0, OP_EQ,
            fast_client_handshake(state, server_handshake, c_keys, 100, &msg));
  tt_ptr_op(msg, OP_EQ, NULL);
  tt_mem_op(s_keys, OP_EQ, c_keys, 100);

  /* Now test a failing handshake. */
  server_handshake[0] ^= 3;
  tt_int_op(-1, OP_EQ,
            fast_client_handshake(state, server_handshake, c_keys, 100, &msg));
  tt_str_op(msg, OP_EQ, "Digest DOES NOT MATCH on fast handshake. "
            "Bug or attack.");

 done:
  fast_handshake_state_free(state);
}

/** Run unit tests for the onion queues. */
static void
test_onion_queues(void *arg)
{
  uint8_t buf1[TAP_ONIONSKIN_CHALLENGE_LEN] = {0};
  uint8_t buf2[NTOR_ONIONSKIN_LEN] = {0};

  or_circuit_t *circ1 = or_circuit_new(0, NULL);
  or_circuit_t *circ2 = or_circuit_new(0, NULL);

  create_cell_t *onionskin = NULL, *create2_ptr;
  create_cell_t *create1 = tor_malloc_zero(sizeof(create_cell_t));
  create_cell_t *create2 = tor_malloc_zero(sizeof(create_cell_t));
  (void)arg;
  create2_ptr = create2; /* remember, but do not free */

  create_cell_init(create1, CELL_CREATE, ONION_HANDSHAKE_TYPE_TAP,
                   TAP_ONIONSKIN_CHALLENGE_LEN, buf1);
  create_cell_init(create2, CELL_CREATE, ONION_HANDSHAKE_TYPE_NTOR,
                   NTOR_ONIONSKIN_LEN, buf2);

  tt_int_op(0,OP_EQ, onion_num_pending(ONION_HANDSHAKE_TYPE_TAP));
  tt_int_op(0,OP_EQ, onion_pending_add(circ1, create1));
  create1 = NULL;
  tt_int_op(1,OP_EQ, onion_num_pending(ONION_HANDSHAKE_TYPE_TAP));

  tt_int_op(0,OP_EQ, onion_num_pending(ONION_HANDSHAKE_TYPE_NTOR));
  tt_int_op(0,OP_EQ, onion_pending_add(circ2, create2));
  create2 = NULL;
  tt_int_op(1,OP_EQ, onion_num_pending(ONION_HANDSHAKE_TYPE_NTOR));

  tt_ptr_op(circ2,OP_EQ, onion_next_task(&onionskin));
  tt_int_op(1,OP_EQ, onion_num_pending(ONION_HANDSHAKE_TYPE_TAP));
  tt_int_op(0,OP_EQ, onion_num_pending(ONION_HANDSHAKE_TYPE_NTOR));
  tt_ptr_op(onionskin, OP_EQ, create2_ptr);

  clear_pending_onions();
  tt_int_op(0,OP_EQ, onion_num_pending(ONION_HANDSHAKE_TYPE_TAP));
  tt_int_op(0,OP_EQ, onion_num_pending(ONION_HANDSHAKE_TYPE_NTOR));

 done:
  circuit_free_(TO_CIRCUIT(circ1));
  circuit_free_(TO_CIRCUIT(circ2));
  tor_free(create1);
  tor_free(create2);
  tor_free(onionskin);
}

static void
test_circuit_timeout(void *arg)
{
  /* Plan:
   *  1. Generate 1000 samples
   *  2. Estimate parameters
   *  3. If difference, repeat
   *  4. Save state
   *  5. load state
   *  6. Estimate parameters
   *  7. compare differences
   */
  circuit_build_times_t initial;
  circuit_build_times_t estimate;
  circuit_build_times_t final;
  double timeout1, timeout2;
  or_state_t *state=NULL;
  int i, runs;
  double close_ms;
  (void)arg;

  initialize_periodic_events();

  circuit_build_times_init(&initial);
  circuit_build_times_init(&estimate);
  circuit_build_times_init(&final);

  state = or_state_new();

  // Use a deterministic RNG here, or else we'll get nondeterministic
  // coverage in some of the circuitstats functions.
  testing_enable_deterministic_rng();

  circuitbuild_running_unit_tests();
#define timeout0 (build_time_t)(30*1000.0)
  initial.Xm = 3000;
  circuit_build_times_initial_alpha(&initial,
                                    CBT_DEFAULT_QUANTILE_CUTOFF/100.0,
                                    timeout0);
  close_ms = MAX(circuit_build_times_calculate_timeout(&initial,
                             CBT_DEFAULT_CLOSE_QUANTILE/100.0),
                 CBT_DEFAULT_TIMEOUT_INITIAL_VALUE);
  do {
    for (i=0; i < CBT_DEFAULT_MIN_CIRCUITS_TO_OBSERVE; i++) {
      build_time_t sample = circuit_build_times_generate_sample(&initial,0,1);

      if (sample > close_ms) {
        circuit_build_times_add_time(&estimate, CBT_BUILD_ABANDONED);
      } else {
        circuit_build_times_add_time(&estimate, sample);
      }
    }
    circuit_build_times_update_alpha(&estimate);
    timeout1 = circuit_build_times_calculate_timeout(&estimate,
                                  CBT_DEFAULT_QUANTILE_CUTOFF/100.0);
    circuit_build_times_set_timeout(&estimate);
    log_notice(LD_CIRC, "Timeout1 is %f, Xm is %d", timeout1, estimate.Xm);
           /* 2% error */
  } while (fabs(circuit_build_times_cdf(&initial, timeout0) -
                circuit_build_times_cdf(&initial, timeout1)) > 0.02);

  tt_int_op(estimate.total_build_times, OP_LE, CBT_NCIRCUITS_TO_OBSERVE);

  circuit_build_times_update_state(&estimate, state);
  circuit_build_times_free_timeouts(&final);
  tt_int_op(circuit_build_times_parse_state(&final, state), OP_EQ, 0);

  circuit_build_times_update_alpha(&final);
  timeout2 = circuit_build_times_calculate_timeout(&final,
                                 CBT_DEFAULT_QUANTILE_CUTOFF/100.0);

  circuit_build_times_set_timeout(&final);
  log_notice(LD_CIRC, "Timeout2 is %f, Xm is %d", timeout2, final.Xm);

  /* 5% here because some accuracy is lost due to histogram conversion */
  tt_assert(fabs(circuit_build_times_cdf(&initial, timeout0) -
                   circuit_build_times_cdf(&initial, timeout2)) < 0.05);

  for (runs = 0; runs < 50; runs++) {
    int build_times_idx = 0;
    int total_build_times = 0;

    final.close_ms = final.timeout_ms = CBT_DEFAULT_TIMEOUT_INITIAL_VALUE;
    estimate.close_ms = estimate.timeout_ms
                      = CBT_DEFAULT_TIMEOUT_INITIAL_VALUE;

    for (i = 0; i < CBT_DEFAULT_RECENT_CIRCUITS*2; i++) {
      circuit_build_times_network_circ_success(&estimate);
      circuit_build_times_add_time(&estimate,
            circuit_build_times_generate_sample(&estimate, 0,
                CBT_DEFAULT_QUANTILE_CUTOFF/100.0));

      circuit_build_times_network_circ_success(&estimate);
      circuit_build_times_add_time(&final,
            circuit_build_times_generate_sample(&final, 0,
                CBT_DEFAULT_QUANTILE_CUTOFF/100.0));
    }

    tt_assert(!circuit_build_times_network_check_changed(&estimate));
    tt_assert(!circuit_build_times_network_check_changed(&final));

    /* Reset liveness to be non-live */
    final.liveness.network_last_live = 0;
    estimate.liveness.network_last_live = 0;

    build_times_idx = estimate.build_times_idx;
    total_build_times = estimate.total_build_times;

    tt_assert(circuit_build_times_network_check_live(&estimate));
    tt_assert(circuit_build_times_network_check_live(&final));

    circuit_build_times_count_close(&estimate, 0,
            (time_t)(approx_time()-estimate.close_ms/1000.0-1));
    circuit_build_times_count_close(&final, 0,
            (time_t)(approx_time()-final.close_ms/1000.0-1));

    tt_assert(!circuit_build_times_network_check_live(&estimate));
    tt_assert(!circuit_build_times_network_check_live(&final));

    log_info(LD_CIRC, "idx: %d %d, tot: %d %d",
             build_times_idx, estimate.build_times_idx,
             total_build_times, estimate.total_build_times);

    /* Check rollback index. Should match top of loop. */
    tt_assert(build_times_idx == estimate.build_times_idx);
    // This can fail if estimate.total_build_times == 1000, because
    // in that case, rewind actually causes us to lose timeouts
    if (total_build_times != CBT_NCIRCUITS_TO_OBSERVE)
      tt_assert(total_build_times == estimate.total_build_times);

    /* Now simulate that the network has become live and we need
     * a change */
    circuit_build_times_network_is_live(&estimate);
    circuit_build_times_network_is_live(&final);

    for (i = 0; i < CBT_DEFAULT_MAX_RECENT_TIMEOUT_COUNT; i++) {
      circuit_build_times_count_timeout(&estimate, 1);

      if (i < CBT_DEFAULT_MAX_RECENT_TIMEOUT_COUNT-1) {
        circuit_build_times_count_timeout(&final, 1);
      }
    }

    tt_int_op(estimate.liveness.after_firsthop_idx, OP_EQ, 0);
    tt_assert(final.liveness.after_firsthop_idx ==
                CBT_DEFAULT_MAX_RECENT_TIMEOUT_COUNT-1);

    tt_assert(circuit_build_times_network_check_live(&estimate));
    tt_assert(circuit_build_times_network_check_live(&final));

    circuit_build_times_count_timeout(&final, 1);

    /* Ensure return value for degenerate cases are clamped correctly */
    initial.alpha = INT32_MAX;
    tt_assert(circuit_build_times_calculate_timeout(&initial, .99999999) <=
              INT32_MAX);
    initial.alpha = 0;
    tt_assert(circuit_build_times_calculate_timeout(&initial, .5) <=
              INT32_MAX);
  }

 done:
  circuit_build_times_free_timeouts(&initial);
  circuit_build_times_free_timeouts(&estimate);
  circuit_build_times_free_timeouts(&final);
  or_state_free(state);
  teardown_periodic_events();

  testing_disable_deterministic_rng();
}

/** Test encoding and parsing of rendezvous service descriptors. */
static void
test_rend_fns(void *arg)
{
  rend_service_descriptor_t *generated = NULL, *parsed = NULL;
  char service_id[DIGEST_LEN];
  char service_id_base32[REND_SERVICE_ID_LEN_BASE32+1];
  const char *next_desc;
  smartlist_t *descs = smartlist_new();
  char computed_desc_id[DIGEST_LEN];
  char parsed_desc_id[DIGEST_LEN];
  crypto_pk_t *pk1 = NULL, *pk2 = NULL;
  time_t now;
  char *intro_points_encrypted = NULL;
  size_t intro_points_size;
  size_t encoded_size;
  int i;

  (void)arg;

  /* Initialize the service cache. */
  rend_cache_init();

  pk1 = pk_generate(0);
  pk2 = pk_generate(1);
  generated = tor_malloc_zero(sizeof(rend_service_descriptor_t));
  generated->pk = crypto_pk_dup_key(pk1);
  crypto_pk_get_digest(generated->pk, service_id);
  base32_encode(service_id_base32, REND_SERVICE_ID_LEN_BASE32+1,
                service_id, REND_SERVICE_ID_LEN);
  now = time(NULL);
  generated->timestamp = now;
  generated->version = 2;
  generated->protocols = 42;
  generated->intro_nodes = smartlist_new();

  for (i = 0; i < 3; i++) {
    rend_intro_point_t *intro = tor_malloc_zero(sizeof(rend_intro_point_t));
    crypto_pk_t *okey = pk_generate(2 + i);
    intro->extend_info =
      extend_info_new(NULL, NULL, NULL, NULL, NULL, NULL, 0);
    intro->extend_info->onion_key = okey;
    crypto_pk_get_digest(intro->extend_info->onion_key,
                         intro->extend_info->identity_digest);
    //crypto_rand(info->identity_digest, DIGEST_LEN); /* Would this work? */
    intro->extend_info->nickname[0] = '$';
    base16_encode(intro->extend_info->nickname + 1,
                  sizeof(intro->extend_info->nickname) - 1,
                  intro->extend_info->identity_digest, DIGEST_LEN);
    tor_addr_t addr;
    uint16_t port;
    /* Does not cover all IP addresses. */
    tor_addr_from_ipv4h(&addr, crypto_rand_int(65536) + 1);
    port = 1 + crypto_rand_int(65535);
    extend_info_add_orport(intro->extend_info, &addr, port);
    intro->intro_key = crypto_pk_dup_key(pk2);
    smartlist_add(generated->intro_nodes, intro);
  }
  int rv = rend_encode_v2_descriptors(descs, generated, now, 0,
                                      REND_NO_AUTH, NULL, NULL);
  tt_int_op(rv, OP_GT, 0);
  rv = rend_compute_v2_desc_id(computed_desc_id, service_id_base32, NULL,
                               now, 0);
  tt_int_op(rv, OP_EQ, 0);
  tt_mem_op(((rend_encoded_v2_service_descriptor_t *)
             smartlist_get(descs, 0))->desc_id, OP_EQ,
            computed_desc_id, DIGEST_LEN);
  rv = rend_parse_v2_service_descriptor(&parsed, parsed_desc_id,
               &intro_points_encrypted, &intro_points_size, &encoded_size,
               &next_desc,
          ((rend_encoded_v2_service_descriptor_t *)smartlist_get(descs, 0))
                                        ->desc_str, 1);
  tt_int_op(rv, OP_EQ, 0);
  tt_assert(parsed);
  tt_mem_op(((rend_encoded_v2_service_descriptor_t *)
         smartlist_get(descs, 0))->desc_id,OP_EQ, parsed_desc_id, DIGEST_LEN);
  tt_int_op(rend_parse_introduction_points(parsed, intro_points_encrypted,
                                         intro_points_size),OP_EQ, 3);
  tt_assert(!crypto_pk_cmp_keys(generated->pk, parsed->pk));
  tt_int_op(parsed->timestamp,OP_EQ, now);
  tt_int_op(parsed->version,OP_EQ, 2);
  tt_int_op(parsed->protocols,OP_EQ, 42);
  tt_int_op(smartlist_len(parsed->intro_nodes),OP_EQ, 3);
  for (i = 0; i < smartlist_len(parsed->intro_nodes); i++) {
    rend_intro_point_t *par_intro = smartlist_get(parsed->intro_nodes, i),
      *gen_intro = smartlist_get(generated->intro_nodes, i);
    extend_info_t *par_info = par_intro->extend_info;
    extend_info_t *gen_info = gen_intro->extend_info;
    tt_assert(!crypto_pk_cmp_keys(gen_info->onion_key, par_info->onion_key));
    tt_mem_op(gen_info->identity_digest,OP_EQ, par_info->identity_digest,
               DIGEST_LEN);
    tt_str_op(gen_info->nickname,OP_EQ, par_info->nickname);
    const tor_addr_port_t *a1, *a2;
    a1 = extend_info_get_orport(gen_info, AF_INET);
    a2 = extend_info_get_orport(par_info, AF_INET);
    tt_assert(a1 && a2);
    tt_assert(tor_addr_eq(&a1->addr, &a2->addr));
    tt_int_op(a2->port,OP_EQ, a2->port);
  }

  rend_service_descriptor_free(parsed);
  rend_service_descriptor_free(generated);
  parsed = generated = NULL;

 done:
  if (descs) {
    for (i = 0; i < smartlist_len(descs); i++)
      rend_encoded_v2_service_descriptor_free_(smartlist_get(descs, i));
    smartlist_free(descs);
  }
  if (parsed)
    rend_service_descriptor_free(parsed);
  if (generated)
    rend_service_descriptor_free(generated);
  if (pk1)
    crypto_pk_free(pk1);
  if (pk2)
    crypto_pk_free(pk2);
  tor_free(intro_points_encrypted);
}

#define ENT(name)                                                       \
  { #name, test_ ## name , 0, NULL, NULL }
#define FORK(name)                                                      \
  { #name, test_ ## name , TT_FORK, NULL, NULL }

static struct testcase_t test_array[] = {
  ENT(onion_handshake),
  { "bad_onion_handshake", test_bad_onion_handshake, 0, NULL, NULL },
  ENT(onion_queues),
  { "ntor_handshake", test_ntor_handshake, 0, NULL, NULL },
  { "fast_handshake", test_fast_handshake, 0, NULL, NULL },
  FORK(circuit_timeout),
  FORK(rend_fns),

  END_OF_TESTCASES
};

struct testgroup_t testgroups[] = {
  { "", test_array },
  { "accounting/", accounting_tests },
  { "addr/", addr_tests },
  { "address/", address_tests },
  { "address_set/", address_set_tests },
  { "bridges/", bridges_tests },
  { "buffer/", buffer_tests },
  { "bwmgt/", bwmgt_tests },
  { "cellfmt/", cell_format_tests },
  { "cellqueue/", cell_queue_tests },
  { "channel/", channel_tests },
  { "channelpadding/", channelpadding_tests },
  { "channeltls/", channeltls_tests },
  { "checkdir/", checkdir_tests },
  { "circuitbuild/", circuitbuild_tests },
  { "circuitpadding/", circuitpadding_tests },
  { "circuitlist/", circuitlist_tests },
  { "circuitmux/", circuitmux_tests },
  { "circuitmux_ewma/", circuitmux_ewma_tests },
  { "circuitstats/", circuitstats_tests },
  { "circuituse/", circuituse_tests },
  { "compat/libevent/", compat_libevent_tests },
  { "config/", config_tests },
  { "config/mgr/", confmgr_tests },
  { "config/parse/", confparse_tests },
  { "connection/", connection_tests },
  { "conscache/", conscache_tests },
  { "consdiff/", consdiff_tests },
  { "consdiffmgr/", consdiffmgr_tests },
  { "container/", container_tests },
  { "container/namemap/", namemap_tests },
  { "control/", controller_tests },
  { "control/btrack/", btrack_tests },
  { "control/event/", controller_event_tests },
  { "crypto/", crypto_tests },
  { "crypto/ope/", crypto_ope_tests },
#ifdef ENABLE_OPENSSL
  { "crypto/openssl/", crypto_openssl_tests },
#endif
  { "crypto/pem/", pem_tests },
  { "crypto/rng/", crypto_rng_tests },
  { "dir/", dir_tests },
  { "dir/auth/process_descs/", process_descs_tests },
  { "dir/md/", microdesc_tests },
  { "dirauth/dirvote/", dirvote_tests},
  { "dir/voting/flags/", voting_flags_tests },
  { "dir/voting/schedule/", voting_schedule_tests },
  { "dir_handle_get/", dir_handle_get_tests },
  { "dispatch/", dispatch_tests, },
  { "dns/", dns_tests },
  { "dos/", dos_tests },
  { "entryconn/", entryconn_tests },
  { "entrynodes/", entrynodes_tests },
  { "extorport/", extorport_tests },
  { "geoip/", geoip_tests },
  { "guardfraction/", guardfraction_tests },
  { "hs_cache/", hs_cache },
  { "hs_cell/", hs_cell_tests },
  { "hs_client/", hs_client_tests },
  { "hs_common/", hs_common_tests },
  { "hs_config/", hs_config_tests },
  { "hs_control/", hs_control_tests },
  { "hs_descriptor/", hs_descriptor },
  { "hs_dos/", hs_dos_tests },
  { "hs_intropoint/", hs_intropoint_tests },
  { "hs_metrics/", hs_metrics_tests },
  { "hs_ntor/", hs_ntor_tests },
  { "hs_ob/", hs_ob_tests },
  { "hs_service/", hs_service_tests },
  { "introduce/", introduce_tests },
  { "keypin/", keypin_tests },
  { "legacy_hs/", hs_tests },
  { "link-handshake/", link_handshake_tests },
  { "mainloop/", mainloop_tests },
  { "metrics/", metrics_tests },
  { "netinfo/", netinfo_tests },
  { "nodelist/", nodelist_tests },
  { "oom/", oom_tests },
  { "oos/", oos_tests },
  { "options/", options_tests },
  { "options/act/", options_act_tests },
  { "parsecommon/", parsecommon_tests },
  { "periodic-event/" , periodic_event_tests },
  { "policy/" , policy_tests },
  { "prob_distr/", prob_distr_tests },
  { "procmon/", procmon_tests },
  { "process/", process_tests },
  { "proto/haproxy/", proto_haproxy_tests },
  { "proto/http/", proto_http_tests },
  { "proto/misc/", proto_misc_tests },
  { "protover/", protover_tests },
  { "pt/", pt_tests },
  { "pubsub/build/", pubsub_build_tests },
  { "pubsub/msg/", pubsub_msg_tests },
  { "relay/" , relay_tests },
  { "relaycell/", relaycell_tests },
  { "relaycrypt/", relaycrypt_tests },
  { "rend_cache/", rend_cache_tests },
  { "replaycache/", replaycache_tests },
  { "router/", router_tests },
  { "routerkeys/", routerkeys_tests },
  { "routerlist/", routerlist_tests },
  { "routerset/" , routerset_tests },
  { "scheduler/", scheduler_tests },
  { "sendme/", sendme_tests },
  { "shared-random/", sr_tests },
  { "socks/", socks_tests },
  { "statefile/", statefile_tests },
  { "stats/", stats_tests },
  { "status/" , status_tests },
  { "storagedir/", storagedir_tests },
  { "token_bucket/", token_bucket_tests },
  { "tortls/", tortls_tests },
#ifndef ENABLE_NSS
  { "tortls/openssl/", tortls_openssl_tests },
#endif
  { "tortls/x509/", x509_tests },
  { "util/", util_tests },
  { "util/format/", util_format_tests },
  { "util/handle/", handle_tests },
  { "util/logging/", logging_tests },
  { "util/process/", util_process_tests },
  { "util/thread/", thread_tests },
  END_OF_GROUPS
};
