/* Copyright (c) 2017-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file test_hs_ntor.c
 * \brief Test hidden service ntor functionality.
 */

#include "test/test.h"
#include "test/test_helpers.h"
#include "test/log_test_helpers.h"
#include "lib/crypt_ops/crypto_curve25519.h"
#include "lib/crypt_ops/crypto_ed25519.h"

#include "core/crypto/hs_ntor.h"

/* Test the HS ntor handshake. Simulate the sending of an encrypted INTRODUCE1
 * cell, and verify the proper derivation of decryption keys on the other end.
 * Then simulate the sending of an authenticated RENDEZVOUS1 cell and verify
 * the proper verification on the other end. */
static void
test_hs_ntor(void *arg)
{
  int retval;

  hs_subcredential_t subcredential;

  ed25519_keypair_t service_intro_auth_keypair;
  curve25519_keypair_t service_intro_enc_keypair;
  curve25519_keypair_t service_ephemeral_rend_keypair;

  curve25519_keypair_t client_ephemeral_enc_keypair;

  hs_ntor_intro_cell_keys_t client_hs_ntor_intro_cell_keys;
  hs_ntor_intro_cell_keys_t service_hs_ntor_intro_cell_keys;

  hs_ntor_rend_cell_keys_t service_hs_ntor_rend_cell_keys;
  hs_ntor_rend_cell_keys_t client_hs_ntor_rend_cell_keys;

  (void) arg;

  /* Generate fake data for this unittest */
  {
    /* Generate fake subcredential */
    memset(subcredential.subcred, 'Z', DIGEST256_LEN);

    /* service */
    curve25519_keypair_generate(&service_intro_enc_keypair, 0);
    ed25519_keypair_generate(&service_intro_auth_keypair, 0);
    curve25519_keypair_generate(&service_ephemeral_rend_keypair, 0);
    /* client */
    curve25519_keypair_generate(&client_ephemeral_enc_keypair, 0);
  }

  /* Client: Simulate the sending of an encrypted INTRODUCE1 cell */
  retval =
    hs_ntor_client_get_introduce1_keys(&service_intro_auth_keypair.pubkey,
                                       &service_intro_enc_keypair.pubkey,
                                       &client_ephemeral_enc_keypair,
                                       &subcredential,
                                       &client_hs_ntor_intro_cell_keys);
  tt_int_op(retval, OP_EQ, 0);

  /* Service: Simulate the decryption of the received INTRODUCE1 */
  retval =
    hs_ntor_service_get_introduce1_keys(&service_intro_auth_keypair.pubkey,
                                        &service_intro_enc_keypair,
                                        &client_ephemeral_enc_keypair.pubkey,
                                        &subcredential,
                                        &service_hs_ntor_intro_cell_keys);
  tt_int_op(retval, OP_EQ, 0);

  /* Test that the INTRODUCE1 encryption/mac keys match! */
  tt_mem_op(client_hs_ntor_intro_cell_keys.enc_key, OP_EQ,
            service_hs_ntor_intro_cell_keys.enc_key,
            CIPHER256_KEY_LEN);
  tt_mem_op(client_hs_ntor_intro_cell_keys.mac_key, OP_EQ,
            service_hs_ntor_intro_cell_keys.mac_key,
            DIGEST256_LEN);

  /* Service: Simulate creation of RENDEZVOUS1 key material. */
  retval =
    hs_ntor_service_get_rendezvous1_keys(&service_intro_auth_keypair.pubkey,
                                         &service_intro_enc_keypair,
                                         &service_ephemeral_rend_keypair,
                                         &client_ephemeral_enc_keypair.pubkey,
                                         &service_hs_ntor_rend_cell_keys);
  tt_int_op(retval, OP_EQ, 0);

  /* Client: Simulate the verification of a received RENDEZVOUS1 cell */
  retval =
    hs_ntor_client_get_rendezvous1_keys(&service_intro_auth_keypair.pubkey,
                                        &client_ephemeral_enc_keypair,
                                        &service_intro_enc_keypair.pubkey,
                                        &service_ephemeral_rend_keypair.pubkey,
                                        &client_hs_ntor_rend_cell_keys);
  tt_int_op(retval, OP_EQ, 0);

  /* Test that the RENDEZVOUS1 key material match! */
  tt_mem_op(client_hs_ntor_rend_cell_keys.rend_cell_auth_mac, OP_EQ,
            service_hs_ntor_rend_cell_keys.rend_cell_auth_mac,
            DIGEST256_LEN);
  tt_mem_op(client_hs_ntor_rend_cell_keys.ntor_key_seed, OP_EQ,
            service_hs_ntor_rend_cell_keys.ntor_key_seed,
            DIGEST256_LEN);
 done:
  ;
}

struct testcase_t hs_ntor_tests[] = {
  { "hs_ntor", test_hs_ntor, TT_FORK,
    NULL, NULL },

  END_OF_TESTCASES
};
