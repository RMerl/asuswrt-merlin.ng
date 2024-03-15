/* Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

#include "orconfig.h"
#define ONION_NTOR_V3_PRIVATE
#include "core/or/or.h"
#include "test/test.h"
#include "lib/crypt_ops/crypto_curve25519.h"
#include "lib/crypt_ops/crypto_ed25519.h"
#include "core/crypto/onion_ntor_v3.h"
#include "core/crypto/onion_crypto.h"
#include "core/or/extend_info_st.h"
#include "core/or/crypt_path_st.h"
#define TOR_CONGESTION_CONTROL_PRIVATE
#define TOR_CONGESTION_CONTROL_COMMON_PRIVATE
#include "core/or/congestion_control_st.h"
#include "core/or/congestion_control_common.h"
#include "app/config/config.h"

#define unhex(arry, s)                                                  \
  { tt_int_op(sizeof(arry), OP_EQ,                                      \
              base16_decode((char*)arry, sizeof(arry), s, strlen(s)));  \
  }

static void
test_ntor3_testvecs(void *arg)
{
  (void)arg;
  char *mem_op_hex_tmp = NULL; // temp val to make test_memeq_hex work.

  ntor3_server_handshake_state_t *relay_state = NULL;
  uint8_t *onion_skin = NULL;
  size_t onion_skin_len;
  ntor3_handshake_state_t *client_state = NULL;
  uint8_t *cm = NULL, *sm = NULL;
  size_t cm_len, sm_len;
  di_digest256_map_t *private_keys = NULL;
  uint8_t *server_handshake = NULL;
  size_t server_handshake_len;

  // Test vectors from python implementation, confirmed with rust
  // implementation.
  curve25519_keypair_t relay_keypair_b;
  curve25519_keypair_t client_keypair_x;
  curve25519_keypair_t relay_keypair_y;
  ed25519_public_key_t relay_id;

  unhex(relay_keypair_b.seckey.secret_key,
        "4051daa5921cfa2a1c27b08451324919538e79e788a81b38cbed097a5dff454a");
  unhex(relay_keypair_b.pubkey.public_key,
        "f8307a2bc1870b00b828bb74dbb8fd88e632a6375ab3bcd1ae706aaa8b6cdd1d");
  unhex(relay_id.pubkey,
        "9fad2af287ef942632833d21f946c6260c33fae6172b60006e86e4a6911753a2");
  unhex(client_keypair_x.seckey.secret_key,
        "b825a3719147bcbe5fb1d0b0fcb9c09e51948048e2e3283d2ab7b45b5ef38b49");
  unhex(client_keypair_x.pubkey.public_key,
        "252fe9ae91264c91d4ecb8501f79d0387e34ad8ca0f7c995184f7d11d5da4f46");
  unhex(relay_keypair_y.seckey.secret_key,
        "4865a5b7689dafd978f529291c7171bc159be076b92186405d13220b80e2a053");
  unhex(relay_keypair_y.pubkey.public_key,
        "4bf4814326fdab45ad5184f5518bd7fae25dc59374062698201a50a22954246d");

  uint8_t client_message[11];
  uint8_t verification[5];
  unhex(client_message, "68656c6c6f20776f726c64");
  unhex(verification, "78797a7a79");

  // ========= Client handshake 1.

  onion_skin_ntor3_create_nokeygen(
                    &client_keypair_x,
                    &relay_id,
                    &relay_keypair_b.pubkey,
                    verification,
                    sizeof(verification),
                    client_message,
                    sizeof(client_message),
                    &client_state,
                    &onion_skin,
                    &onion_skin_len);

  const char expect_client_handshake[] = "9fad2af287ef942632833d21f946c6260c"
    "33fae6172b60006e86e4a6911753a2f8307a2bc1870b00b828bb74dbb8fd88e632a6375"
    "ab3bcd1ae706aaa8b6cdd1d252fe9ae91264c91d4ecb8501f79d0387e34ad8ca0f7c995"
    "184f7d11d5da4f463bebd9151fd3b47c180abc9e044d53565f04d82bbb3bebed3d06cea"
    "65db8be9c72b68cd461942088502f67";

  tt_int_op(onion_skin_len, OP_EQ, strlen(expect_client_handshake)/2);
  test_memeq_hex(onion_skin, expect_client_handshake);

  // ========= Relay handshake.

  dimap_add_entry(&private_keys,
                  relay_keypair_b.pubkey.public_key,
                  &relay_keypair_b);

  int r = onion_skin_ntor3_server_handshake_part1(
                    private_keys,
                    &client_keypair_x,
                    &relay_id,
                    onion_skin,
                    onion_skin_len,
                    verification,
                    sizeof(verification),
                    &cm,
                    &cm_len,
                    &relay_state);
  tt_int_op(r, OP_EQ, 0);
  tt_int_op(cm_len, OP_EQ, sizeof(client_message));
  tt_mem_op(cm, OP_EQ, client_message, cm_len);

  uint8_t server_message[10];
  unhex(server_message, "486f6c61204d756e646f");

  uint8_t server_keys[256];
  onion_skin_ntor3_server_handshake_part2_nokeygen(
                    &relay_keypair_y,
                    relay_state,
                    verification,
                    sizeof(verification),
                    server_message,
                    sizeof(server_message),
                    &server_handshake,
                    &server_handshake_len,
                    server_keys,
                    sizeof(server_keys));

  const char expect_server_handshake[] = "4bf4814326fdab45ad5184f5518bd7fae25"
    "dc59374062698201a50a22954246d2fc5f8773ca824542bc6cf6f57c7c29bbf4e5476461"
    "ab130c5b18ab0a91276651202c3e1e87c0d32054c";
  tt_int_op(server_handshake_len, OP_EQ, strlen(expect_server_handshake)/2);
  test_memeq_hex(server_handshake, expect_server_handshake);

  uint8_t expect_keys[256];
  unhex(expect_keys, "9c19b631fd94ed86a817e01f6c80b0743a43f5faebd39cfaa8b00f"
        "a8bcc65c3bfeaa403d91acbd68a821bf6ee8504602b094a254392a07737d5662768"
        "c7a9fb1b2814bb34780eaee6e867c773e28c212ead563e98a1cd5d5b4576f5ee61c"
        "59bde025ff2851bb19b721421694f263818e3531e43a9e4e3e2c661e2ad547d8984"
        "caa28ebecd3e4525452299be26b9185a20a90ce1eac20a91f2832d731b54502b097"
        "49b5a2a2949292f8cfcbeffb790c7790ed935a9d251e7e336148ea83b063a5618fc"
        "ff674a44581585fd22077ca0e52c59a24347a38d1a1ceebddbf238541f226b8f88d"
        "0fb9c07a1bcd2ea764bbbb5dacdaf5312a14c0b9e4f06309b0333b4a");
  tt_mem_op(server_keys, OP_EQ, expect_keys, 256);

  // ===== Client handshake 2

  uint8_t client_keys[256];
  r = onion_ntor3_client_handshake(
                  client_state,
                  server_handshake,
                  server_handshake_len,
                  verification,
                  sizeof(verification),
                  client_keys,
                  sizeof(client_keys),
                  &sm,
                  &sm_len);

  tt_int_op(r, OP_EQ, 0);
  tt_int_op(sm_len, OP_EQ, sizeof(server_message));
  tt_mem_op(sm, OP_EQ, server_message, sizeof(server_message));
  tt_mem_op(client_keys, OP_EQ, server_keys, 256);

 done:
  tor_free(onion_skin);
  tor_free(server_handshake);
  tor_free(mem_op_hex_tmp);
  ntor3_handshake_state_free(client_state);
  ntor3_server_handshake_state_free(relay_state);
  tor_free(cm);
  tor_free(sm);
  dimap_free(private_keys, NULL);
}

static void
run_full_handshake(circuit_params_t *serv_params_in,
                   circuit_params_t *client_params_out,
                   circuit_params_t *serv_params_out)
{
  extend_info_t info = {0};
  uint8_t onionskin[CELL_PAYLOAD_SIZE];
  int onionskin_len = 0;
  int reply_len = 0;
  onion_handshake_state_t handshake_state = {0};
  server_onion_keys_t server_keys = {0};
  curve25519_keypair_t relay_onion_key;
  uint8_t serv_reply[CELL_PAYLOAD_SIZE];
  uint8_t serv_keys[100];
  uint8_t rend_nonce[DIGEST_LEN];
  uint8_t client_keys[CELL_PAYLOAD_SIZE];
  uint8_t rend_auth[DIGEST_LEN];

  info.exit_supports_congestion_control = 1;

  unhex(relay_onion_key.seckey.secret_key,
        "4051daa5921cfa2a1c27b08451324919538e79e788a81b38cbed097a5dff454a");
  unhex(relay_onion_key.pubkey.public_key,
        "f8307a2bc1870b00b828bb74dbb8fd88e632a6375ab3bcd1ae706aaa8b6cdd1d");

  memcpy(&info.curve25519_onion_key,
         &relay_onion_key.pubkey, sizeof(info.curve25519_onion_key));
  unhex(info.ed_identity.pubkey,
        "9fad2af287ef942632833d21f946c6260c33fae6172b60006e86e4a6911753a2");

  memcpy(&server_keys.my_ed_identity, &info.ed_identity,
         sizeof(server_keys.my_ed_identity));

  dimap_add_entry(&server_keys.curve25519_key_map,
                  relay_onion_key.pubkey.public_key,
                  &relay_onion_key);

  onionskin_len = onion_skin_create(ONION_HANDSHAKE_TYPE_NTOR_V3, &info,
                    &handshake_state, onionskin,
                    sizeof(onionskin));
  tt_int_op(onionskin_len, OP_NE, -1);

  server_keys.junk_keypair = &handshake_state.u.ntor3->client_keypair;

  reply_len = onion_skin_server_handshake(ONION_HANDSHAKE_TYPE_NTOR_V3,
                              onionskin, onionskin_len,
                              &server_keys, serv_params_in,
                              serv_reply, sizeof(serv_reply),
                              serv_keys, sizeof(serv_keys),
                              rend_nonce, serv_params_out);
  tt_int_op(reply_len, OP_NE, -1);

  tt_int_op(onion_skin_client_handshake(ONION_HANDSHAKE_TYPE_NTOR_V3,
                              &handshake_state,
                              serv_reply, reply_len,
                              client_keys, sizeof(client_keys),
                              rend_auth, client_params_out,
                              NULL), OP_EQ, 0);

 done:
  dimap_free(server_keys.curve25519_key_map, NULL);
  ntor3_handshake_state_free(handshake_state.u.ntor3);

  return;
}

/**
 * Test congestion control negotiation logic.
 *
 * This tests that congestion control is only enabled when both
 * client and server agree, via consensus param or torrc.
 *
 * It also tests that when they agree, they agree on the server's
 * version of sendme_inc.
 */
static void
test_ntor3_handshake(void *arg)
{
  (void)arg;
  circuit_params_t client_params, serv_params, serv_ns_params;

  serv_ns_params.sendme_inc_cells = congestion_control_sendme_inc();

  /* client off, serv off -> off */
  serv_ns_params.cc_enabled = 0;
  run_full_handshake(&serv_ns_params, &client_params, &serv_params);
  tt_int_op(client_params.cc_enabled, OP_EQ, 0);
  tt_int_op(serv_params.cc_enabled, OP_EQ, 0);

  /* client off, serv on -> off */
  congestion_control_set_cc_disabled();
  serv_ns_params.cc_enabled = 1;
  run_full_handshake(&serv_ns_params, &client_params, &serv_params);
  tt_int_op(client_params.cc_enabled, OP_EQ, 0);
  tt_int_op(serv_params.cc_enabled, OP_EQ, 0);

  /* client off + param, serv on -> on */
  serv_ns_params.cc_enabled = 1;
  get_options_mutable()->AlwaysCongestionControl = 1;
  run_full_handshake(&serv_ns_params, &client_params, &serv_params);
  tt_int_op(client_params.cc_enabled, OP_EQ, 1);
  tt_int_op(serv_params.cc_enabled, OP_EQ, 1);

  /* client on, serv off -> off */
  serv_ns_params.cc_enabled = 0;
  congestion_control_set_cc_enabled();
  run_full_handshake(&serv_ns_params, &client_params, &serv_params);
  tt_int_op(client_params.cc_enabled, OP_EQ, 0);
  tt_int_op(serv_params.cc_enabled, OP_EQ, 0);

  /* client on, serv on -> on */
  serv_ns_params.cc_enabled = 1;
  run_full_handshake(&serv_ns_params, &client_params, &serv_params);
  tt_int_op(client_params.cc_enabled, OP_EQ, 1);
  tt_int_op(serv_params.cc_enabled, OP_EQ, 1);

  /* client on, serv on, sendme_inc diff -> serv sendme_inc */
  serv_ns_params.cc_enabled = 1;
  serv_ns_params.sendme_inc_cells += 1;
  run_full_handshake(&serv_ns_params, &client_params, &serv_params);
  tt_int_op(client_params.cc_enabled, OP_EQ, 1);
  tt_int_op(serv_params.cc_enabled, OP_EQ, 1);
  tt_int_op(serv_params.sendme_inc_cells, OP_EQ,
            client_params.sendme_inc_cells);
  tt_int_op(client_params.sendme_inc_cells, OP_EQ,
            serv_ns_params.sendme_inc_cells);
  tt_int_op(client_params.sendme_inc_cells, OP_NE,
            congestion_control_sendme_inc());

 done:
  return;
}

struct testcase_t ntor_v3_tests[] = {
  { "testvecs", test_ntor3_testvecs, 0, NULL, NULL, },
  { "handshake_negtotiation", test_ntor3_handshake, 0, NULL, NULL, },
  END_OF_TESTCASES,
};
