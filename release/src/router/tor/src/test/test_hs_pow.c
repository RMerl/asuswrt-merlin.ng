/* Copyright (c) 2020-2023, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file test_hs_pow.c
 * \brief Tests for service proof-of-work verification and wire protocol.
 */

#define HS_SERVICE_PRIVATE
#define HS_CIRCUIT_PRIVATE

#include "lib/cc/compat_compiler.h"
#include "lib/cc/torint.h"

#include "test/hs_test_helpers.h"
#include "test/log_test_helpers.h"
#include "test/test_helpers.h"
#include "test/test.h"

#include "app/config/config.h"
#include "core/or/circuitbuild.h"
#include "core/or/circuitlist.h"
#include "core/or/relay.h"
#include "feature/hs/hs_cell.h"
#include "feature/hs/hs_circuit.h"
#include "feature/hs/hs_metrics.h"
#include "feature/hs/hs_pow.h"
#include "feature/hs/hs_service.h"
#include "feature/nodelist/nodelist.h"

#include "core/or/crypt_path_st.h"
#include "core/or/origin_circuit_st.h"
#include "feature/nodelist/node_st.h"
#include "feature/nodelist/routerinfo_st.h"

#include "trunnel/hs/cell_introduce1.h"

static int test_rend_launch_count;
static uint32_t test_rend_launch_expect_effort;

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

  tt_int_op(test_rend_launch_expect_effort, OP_EQ, rdv_data->pow_effort);
  test_rend_launch_count++;

done:
  ;
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

static size_t relay_payload_len;
static uint8_t relay_payload[RELAY_PAYLOAD_SIZE];

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

typedef struct testing_hs_pow_service_t {
  hs_service_t service;
  hs_subcredential_t subcred;
  hs_service_intro_point_t *service_ip;
  hs_desc_intro_point_t *desc_ip;
  hs_ntor_intro_cell_keys_t intro_keys;
  origin_circuit_t *intro_circ;
  origin_circuit_t *rend_circ;
} testing_hs_pow_service_t;

/* Common test setup */
static testing_hs_pow_service_t *
testing_hs_pow_service_new(void)
{
  MOCK(build_state_get_exit_node, mock_build_state_get_exit_node);
  MOCK(relay_send_command_from_edge_, mock_relay_send_command_from_edge);
  MOCK(launch_rendezvous_point_circuit, mock_launch_rendezvous_point_circuit);
  MOCK(node_get_link_specifier_smartlist,
       mock_node_get_link_specifier_smartlist);

  testing_hs_pow_service_t *tsvc = tor_malloc_zero(sizeof *tsvc);
  hs_metrics_service_init(&tsvc->service);

  ed25519_keypair_t identity_keypair;
  ed25519_keypair_generate(&identity_keypair, 0);
  hs_helper_get_subcred_from_identity_keypair(&identity_keypair,
                                              &tsvc->subcred);

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

  hs_service_intro_point_t *svc_ip = service_intro_point_new(&intro_node);
  const ed25519_public_key_t *ip_auth_pubkey = &svc_ip->auth_key_kp.pubkey;
  const curve25519_public_key_t *ip_enc_pubkey = &svc_ip->enc_key_kp.pubkey;
  tsvc->service_ip = svc_ip;

  ed25519_keypair_t signing_kp;
  ed25519_keypair_generate(&signing_kp, 0);
  tsvc->desc_ip = hs_helper_build_intro_point(&signing_kp, 0, "1.2.3.4", 0,
                                              &svc_ip->auth_key_kp,
                                              &svc_ip->enc_key_kp);

  tsvc->intro_circ = origin_circuit_new();
  tsvc->rend_circ = origin_circuit_new();

  tsvc->intro_circ->cpath = tor_malloc_zero(sizeof(crypt_path_t));

  struct hs_ident_circuit_t *hs_ident = tor_malloc_zero(sizeof *hs_ident);
  tsvc->rend_circ->hs_ident = hs_ident;
  tsvc->intro_circ->hs_ident = hs_ident;
  curve25519_keypair_generate(&hs_ident->rendezvous_client_kp, 0);
  tt_int_op(0, OP_EQ,
            hs_ntor_client_get_introduce1_keys(ip_auth_pubkey,
                                               ip_enc_pubkey,
                                               &hs_ident->rendezvous_client_kp,
                                               &tsvc->subcred,
                                               &tsvc->intro_keys));
 done:
  return tsvc;
}

static void
testing_hs_pow_service_free(testing_hs_pow_service_t *tsvc)
{
  hs_metrics_service_free(&tsvc->service);
  service_intro_point_free(tsvc->service_ip);
  hs_desc_intro_point_free(tsvc->desc_ip);
  hs_pow_free_service_state(tsvc->service.state.pow_state);
  tor_free(tsvc);

  if (fake_node) {
    tor_free(fake_node->ri->onion_curve25519_pkey);
    tor_free(fake_node->ri);
    tor_free(fake_node);
  }

  UNMOCK(build_state_get_exit_node);
  UNMOCK(relay_send_command_from_edge_);
  UNMOCK(launch_rendezvous_point_circuit);
  UNMOCK(node_get_link_specifier_smartlist);
}

/* Make sure we can send a PoW extension to a service without PoW enabled */
static void
test_hs_pow_unsolicited(void *arg)
{
  (void)arg;

  testing_hs_pow_service_t *tsvc = testing_hs_pow_service_new();

  /* Try this twice, changing only the presence or lack of PoW solution */
  for (int test_variant = 0; test_variant < 2; test_variant++) {

    relay_payload_len = 0;
    test_rend_launch_count = 0;
    test_rend_launch_expect_effort = 0;
    memset(relay_payload, 0, sizeof relay_payload);

    hs_pow_solution_t solution = { 0 };
    int retval;

    retval = hs_circ_send_introduce1(tsvc->intro_circ, tsvc->rend_circ,
                                     tsvc->desc_ip, &tsvc->subcred,
                                     test_variant == 0 ? &solution : NULL);

    tt_int_op(retval, OP_EQ, 0);
    tt_assert(!fast_mem_is_zero((const char*)relay_payload,
                                sizeof relay_payload));
    tt_int_op(relay_payload_len, OP_NE, 0);

    retval = hs_circ_handle_introduce2(&tsvc->service, tsvc->intro_circ,
                                       tsvc->service_ip, &tsvc->subcred,
                                       relay_payload,
                                       relay_payload_len);

    tt_int_op(retval, OP_EQ, test_variant == 0 ? -1 : 0);
    tt_int_op(test_rend_launch_count, OP_EQ, test_variant == 0 ? 0 : 1);
  }

 done:
  testing_hs_pow_service_free(tsvc);
}

static void
test_hs_pow_vectors(void *arg)
{
  (void)arg;

  /* This covers encoding, wire protocol, and verification for PoW-extended
   * introduction cells. The solutions here can be generated using the
   * setup in test_hs_pow_slow.
   */
  static const struct {
    uint32_t claimed_effort;
    uint32_t validated_effort;
    int expected_retval;
    const char *seed_hex;
    const char *service_blinded_id_hex;
    const char *nonce_hex;
    const char *sol_hex;
    const char *encoded_hex;
  } vectors[] = {
    {
      /* All zero, expect invalid */
      1, 0, -1,
      "0000000000000000000000000000000000000000000000000000000000000000",
      "1111111111111111111111111111111111111111111111111111111111111111",
      "00000000000000000000000000000000", "00000000000000000000000000000000",
      "01"
      "00000000000000000000000000000000"
      "00000001" "00000000"
      "00000000000000000000000000000000"
    },
    {
      /* Valid zero-effort solution */
      0, 0, 0,
      "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
      "1111111111111111111111111111111111111111111111111111111111111111",
      "55555555555555555555555555555555", "4312f87ceab844c78e1c793a913812d7",
      "01"
      "55555555555555555555555555555555"
      "00000000" "aaaaaaaa"
      "4312f87ceab844c78e1c793a913812d7"
    },
    {
      /* Valid high-effort solution */
      1000000, 1000000, 0,
      "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
      "1111111111111111111111111111111111111111111111111111111111111111",
      "59217255555555555555555555555555", "0f3db97b9cac20c1771680a1a34848d3",
      "01"
      "59217255555555555555555555555555"
      "000f4240" "aaaaaaaa"
      "0f3db97b9cac20c1771680a1a34848d3"
    },
    {
      /* Reject replays */
      1000000, 0, -1,
      "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
      "1111111111111111111111111111111111111111111111111111111111111111",
      "59217255555555555555555555555555", "0f3db97b9cac20c1771680a1a34848d3",
      "01"
      "59217255555555555555555555555555"
      "000f4240" "aaaaaaaa"
      "0f3db97b9cac20c1771680a1a34848d3"
    },
    {
      /* The claimed effort must exactly match what's in the challenge */
      99999, 0, -1,
      "86fb0acf4932cda44dbb451282f415479462dd10cb97ff5e7e8e2a53c3767a7f",
      "bfd298428562e530c52bdb36d81a0e293ef4a0e94d787f0f8c0c611f4f9e78ed",
      "2eff9fdbc34326d9d2f18ed277469c63", "400cb091139f86b352119f6e131802d6",
      "01"
      "2eff9fdbc34326d9d2f18ed277469c63"
      "0001869f" "86fb0acf"
      "400cb091139f86b352119f6e131802d6"
    },
    {
      /* Otherwise good solution but with a corrupted nonce */
      100000, 0, -1,
      "86fb0acf4932cda44dbb451282f415479462dd10cb97ff5e7e8e2a53c3767a7f",
      "bfd298428562e530c52bdb36d81a0e293ef4a0e94d787f0f8c0c611f4f9e78ed",
      "2eff9fdbc34326d9a2f18ed277469c63", "400cb091139f86b352119f6e131802d6",
      "01"
      "2eff9fdbc34326d9a2f18ed277469c63"
      "000186a0" "86fb0acf"
      "400cb091139f86b352119f6e131802d6"
    },
    {
      /* Corrected version of above */
      100000, 100000, 0,
      "86fb0acf4932cda44dbb451282f415479462dd10cb97ff5e7e8e2a53c3767a7f",
      "bfd298428562e530c52bdb36d81a0e293ef4a0e94d787f0f8c0c611f4f9e78ed",
      "2eff9fdbc34326d9d2f18ed277469c63", "400cb091139f86b352119f6e131802d6",
      "01"
      "2eff9fdbc34326d9d2f18ed277469c63"
      "000186a0" "86fb0acf"
      "400cb091139f86b352119f6e131802d6"
    }
  };

  testing_hs_pow_service_t *tsvc = testing_hs_pow_service_new();
  hs_pow_service_state_t *pow_state = tor_malloc_zero(sizeof *pow_state);
  tsvc->service.state.pow_state = pow_state;
  tsvc->service.desc_current = service_descriptor_new();
  pow_state->rend_request_pqueue = smartlist_new();

  char *mem_op_hex_tmp = NULL;
  uint8_t *decrypted = NULL;
  trn_cell_introduce_encrypted_t *enc_cell = NULL;
  trn_cell_introduce1_t *cell = NULL;

  const unsigned num_vectors = sizeof vectors / sizeof vectors[0];
  for (unsigned vec_i = 0; vec_i < num_vectors; vec_i++) {
    const int expected_retval = vectors[vec_i].expected_retval;
    const char *service_blinded_id_hex = vectors[vec_i].service_blinded_id_hex;
    const char *seed_hex = vectors[vec_i].seed_hex;
    const char *nonce_hex = vectors[vec_i].nonce_hex;
    const char *sol_hex = vectors[vec_i].sol_hex;
    const char *encoded_hex = vectors[vec_i].encoded_hex;

    relay_payload_len = 0;
    test_rend_launch_count = 0;
    test_rend_launch_expect_effort = vectors[vec_i].validated_effort;
    memset(relay_payload, 0, sizeof relay_payload);

    hs_pow_solution_t solution = {
      .effort = vectors[vec_i].claimed_effort,
    };
    int retval;

    tt_int_op(strlen(service_blinded_id_hex), OP_EQ, 2 * HS_POW_ID_LEN);
    tt_int_op(strlen(seed_hex), OP_EQ, 2 * HS_POW_SEED_LEN);
    tt_int_op(strlen(nonce_hex), OP_EQ, 2 * sizeof solution.nonce);
    tt_int_op(strlen(sol_hex), OP_EQ, 2 * sizeof solution.equix_solution);

    tt_assert(tsvc->service.desc_current);
    ed25519_public_key_t *desc_blinded_pubkey =
        &tsvc->service.desc_current->desc->plaintext_data.blinded_pubkey;

    tt_int_op(base16_decode((char*)desc_blinded_pubkey->pubkey,
                            HS_POW_ID_LEN, service_blinded_id_hex,
                            2 * HS_POW_ID_LEN),
                            OP_EQ, HS_POW_ID_LEN);
    tt_int_op(base16_decode((char*)pow_state->seed_previous, HS_POW_SEED_LEN,
                            seed_hex, 2 * HS_POW_SEED_LEN),
                            OP_EQ, HS_POW_SEED_LEN);
    tt_int_op(base16_decode((char*)solution.nonce, HS_POW_NONCE_LEN,
                            nonce_hex, 2 * HS_POW_NONCE_LEN),
                            OP_EQ, HS_POW_NONCE_LEN);
    tt_int_op(base16_decode((char*)solution.equix_solution, HS_POW_EQX_SOL_LEN,
                            sol_hex, 2 * HS_POW_EQX_SOL_LEN),
                            OP_EQ, HS_POW_EQX_SOL_LEN);

    ed25519_pubkey_copy(&tsvc->service_ip->blinded_id, desc_blinded_pubkey);
    memcpy(solution.seed_head, pow_state->seed_previous, HS_POW_SEED_HEAD_LEN);

    /* Try to encode 'solution' into a relay cell */

    retval = hs_circ_send_introduce1(tsvc->intro_circ, tsvc->rend_circ,
                                     tsvc->desc_ip, &tsvc->subcred,
                                     &solution);

    tt_int_op(retval, OP_EQ, 0);
    tt_assert(!fast_mem_is_zero((const char*)relay_payload,
                                sizeof relay_payload));
    tt_int_op(relay_payload_len, OP_NE, 0);

    /* Check the service's response to this introduction */

    retval = hs_circ_handle_introduce2(&tsvc->service, tsvc->intro_circ,
                                       tsvc->service_ip, &tsvc->subcred,
                                       relay_payload,
                                       relay_payload_len);
    tt_int_op(retval, OP_EQ, expected_retval);
    tt_int_op(test_rend_launch_count, OP_EQ, expected_retval == 0 ? 1 : 0);

    /* Start unpacking the cell ourselves so we can check the PoW data */

    trn_cell_introduce1_free(cell);
    cell = NULL;
    tt_int_op(trn_cell_introduce1_parse(&cell, relay_payload,
                                        relay_payload_len), OP_GT, 0);

    size_t encrypted_section_len;
    const uint8_t *encrypted_section;
    encrypted_section = trn_cell_introduce1_getconstarray_encrypted(cell);
    encrypted_section_len = trn_cell_introduce1_getlen_encrypted(cell);
    tt_int_op(encrypted_section_len, OP_GT,
              DIGEST256_LEN + CURVE25519_PUBKEY_LEN);

    /* Decrypt the encrypted portion of the INTRODUCE1 */

    crypto_cipher_t *cipher = NULL;
    cipher = crypto_cipher_new_with_bits((char *) tsvc->intro_keys.enc_key,
                                         CURVE25519_PUBKEY_LEN * 8);
    tt_ptr_op(cipher, OP_NE, NULL);

    size_t decrypted_len = encrypted_section_len
                             - DIGEST256_LEN - CURVE25519_PUBKEY_LEN;
    tor_free(decrypted);
    decrypted = tor_malloc_zero(decrypted_len);
    retval = crypto_cipher_decrypt(cipher, (char *) decrypted,
                                   (const char *) encrypted_section
                                     + CURVE25519_PUBKEY_LEN,
                                   decrypted_len);
    crypto_cipher_free(cipher);
    tt_int_op(retval, OP_EQ, 0);

    /* Parse the outer layer of the encrypted payload */

    trn_cell_introduce_encrypted_free(enc_cell);
    enc_cell = NULL;
    tt_int_op(trn_cell_introduce_encrypted_parse(&enc_cell, decrypted,
                                                 decrypted_len), OP_GT, 0);

    /* Check for the expected single extension */

    const trn_extension_t *extensions =
      trn_cell_introduce_encrypted_get_extensions(enc_cell);
    tt_int_op(trn_extension_get_num(extensions), OP_EQ, 1);

    const trn_extension_field_t *field =
      trn_extension_getconst_fields(extensions, 0);
    tt_int_op(trn_extension_field_get_field_type(field),
              OP_EQ, TRUNNEL_EXT_TYPE_POW);

    const uint8_t *field_data = trn_extension_field_getconstarray_field(field);
    size_t field_len = trn_extension_field_getlen_field(field);

    /* Our test vectors cover the packed data in the single extension */

    tt_int_op(field_len * 2, OP_EQ, strlen(encoded_hex));
    test_memeq_hex(field_data, encoded_hex);
  }

 done:
  tor_free(mem_op_hex_tmp);
  tor_free(decrypted);
  trn_cell_introduce1_free(cell);
  trn_cell_introduce_encrypted_free(enc_cell);
  service_descriptor_free(tsvc->service.desc_current);
  testing_hs_pow_service_free(tsvc);
  hs_pow_remove_seed_from_cache(NULL);
}

struct testcase_t hs_pow_tests[] = {
  { "unsolicited", test_hs_pow_unsolicited, TT_FORK, NULL, NULL },
  { "vectors", test_hs_pow_vectors, TT_FORK, NULL, NULL },
  END_OF_TESTCASES
};
