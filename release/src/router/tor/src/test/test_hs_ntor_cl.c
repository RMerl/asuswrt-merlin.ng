/* Copyright (c) 2017-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/** This is a wrapper over the little-t-tor HS ntor functions. The wrapper is
 *  used by src/test/hs_ntor_ref.py to conduct the HS ntor integration
 *  tests.
 *
 *  The logic of this wrapper is basically copied from src/test/test_ntor_cl.c
 */

#include "orconfig.h"
#include <stdio.h>
#include <stdlib.h>

#define ONION_NTOR_PRIVATE
#include "core/or/or.h"
#include "lib/crypt_ops/crypto_cipher.h"
#include "lib/crypt_ops/crypto_curve25519.h"
#include "lib/crypt_ops/crypto_ed25519.h"
#include "lib/crypt_ops/crypto_format.h"
#include "lib/crypt_ops/crypto_init.h"
#include "core/crypto/hs_ntor.h"
#include "core/crypto/onion_ntor.h"

#define N_ARGS(n) STMT_BEGIN {                                  \
    if (argc < (n)) {                                           \
      fprintf(stderr, "%s needs %d arguments.\n",argv[1],n);    \
      return 1;                                                 \
    }                                                           \
  } STMT_END
#define BASE16(idx, var, n) STMT_BEGIN {                                \
    const char *s = argv[(idx)];                                        \
    if (base16_decode((char*)var, n, s, strlen(s)) < (int)n ) {              \
      fprintf(stderr, "couldn't decode argument %d (%s)\n",idx,s);      \
      return 1;                                                         \
    }                                                                   \
  } STMT_END
#define INT(idx, var) STMT_BEGIN {                                      \
    var = atoi(argv[(idx)]);                                            \
    if (var <= 0) {                                                     \
      fprintf(stderr, "bad integer argument %d (%s)\n",idx,argv[(idx)]); \
    }                                                                   \
  } STMT_END

/** The first part of the HS ntor protocol. The client-side computes all
    necessary key material and sends the appropriate message to the service. */
static int
client1(int argc, char **argv)
{
  int retval;

  /* Inputs */
  curve25519_public_key_t intro_enc_pubkey;
  ed25519_public_key_t intro_auth_pubkey;
  curve25519_keypair_t client_ephemeral_enc_keypair;
  hs_subcredential_t subcredential;

  /* Output */
  hs_ntor_intro_cell_keys_t hs_ntor_intro_cell_keys;

  char buf[256];

  N_ARGS(6);
  BASE16(2, intro_auth_pubkey.pubkey, ED25519_PUBKEY_LEN);
  BASE16(3, intro_enc_pubkey.public_key, CURVE25519_PUBKEY_LEN);
  BASE16(4, client_ephemeral_enc_keypair.seckey.secret_key,
         CURVE25519_SECKEY_LEN);
  BASE16(5, subcredential.subcred, DIGEST256_LEN);

  /* Generate keypair */
  curve25519_public_key_generate(&client_ephemeral_enc_keypair.pubkey,
                                 &client_ephemeral_enc_keypair.seckey);

  retval = hs_ntor_client_get_introduce1_keys(&intro_auth_pubkey,
                                              &intro_enc_pubkey,
                                              &client_ephemeral_enc_keypair,
                                              &subcredential,
                                              &hs_ntor_intro_cell_keys);
  if (retval < 0) {
    goto done;
  }

  /* Send ENC_KEY */
  base16_encode(buf, sizeof(buf),
                (const char*)hs_ntor_intro_cell_keys.enc_key,
                sizeof(hs_ntor_intro_cell_keys.enc_key));
  printf("%s\n", buf);
  /* Send MAC_KEY */
  base16_encode(buf, sizeof(buf),
                (const char*)hs_ntor_intro_cell_keys.mac_key,
                sizeof(hs_ntor_intro_cell_keys.mac_key));
  printf("%s\n", buf);

 done:
  return retval;
}

/** The second part of the HS ntor protocol. The service-side computes all
    necessary key material and sends the appropriate message to the client */
static int
server1(int argc, char **argv)
{
  int retval;

  /* Inputs */
  curve25519_keypair_t intro_enc_keypair;
  ed25519_public_key_t intro_auth_pubkey;
  curve25519_public_key_t client_ephemeral_enc_pubkey;
  hs_subcredential_t subcredential;

  /* Output */
  hs_ntor_intro_cell_keys_t hs_ntor_intro_cell_keys;
  hs_ntor_rend_cell_keys_t hs_ntor_rend_cell_keys;
  curve25519_keypair_t service_ephemeral_rend_keypair;

  char buf[256];

  N_ARGS(6);
  BASE16(2, intro_auth_pubkey.pubkey, ED25519_PUBKEY_LEN);
  BASE16(3, intro_enc_keypair.seckey.secret_key, CURVE25519_SECKEY_LEN);
  BASE16(4, client_ephemeral_enc_pubkey.public_key, CURVE25519_PUBKEY_LEN);
  BASE16(5, subcredential.subcred, DIGEST256_LEN);

  /* Generate keypair */
  curve25519_public_key_generate(&intro_enc_keypair.pubkey,
                                 &intro_enc_keypair.seckey);
  curve25519_keypair_generate(&service_ephemeral_rend_keypair, 0);

  /* Get INTRODUCE1 keys */
  retval = hs_ntor_service_get_introduce1_keys(&intro_auth_pubkey,
                                               &intro_enc_keypair,
                                               &client_ephemeral_enc_pubkey,
                                               &subcredential,
                                               &hs_ntor_intro_cell_keys);
  if (retval < 0) {
    goto done;
  }

  /* Get RENDEZVOUS1 keys */
  retval = hs_ntor_service_get_rendezvous1_keys(&intro_auth_pubkey,
                                               &intro_enc_keypair,
                                               &service_ephemeral_rend_keypair,
                                               &client_ephemeral_enc_pubkey,
                                               &hs_ntor_rend_cell_keys);
  if (retval < 0) {
    goto done;
  }

  /* Send ENC_KEY */
  base16_encode(buf, sizeof(buf),
                (const char*)hs_ntor_intro_cell_keys.enc_key,
                sizeof(hs_ntor_intro_cell_keys.enc_key));
  printf("%s\n", buf);
  /* Send MAC_KEY */
  base16_encode(buf, sizeof(buf),
                (const char*)hs_ntor_intro_cell_keys.mac_key,
                sizeof(hs_ntor_intro_cell_keys.mac_key));
  printf("%s\n", buf);
  /* Send AUTH_MAC */
  base16_encode(buf, sizeof(buf),
                (const char*)hs_ntor_rend_cell_keys.rend_cell_auth_mac,
                sizeof(hs_ntor_rend_cell_keys.rend_cell_auth_mac));
  printf("%s\n", buf);
  /* Send NTOR_KEY_SEED */
  base16_encode(buf, sizeof(buf),
                (const char*)hs_ntor_rend_cell_keys.ntor_key_seed,
                sizeof(hs_ntor_rend_cell_keys.ntor_key_seed));
  printf("%s\n", buf);
  /* Send service ephemeral pubkey (Y) */
  base16_encode(buf, sizeof(buf),
                (const char*)service_ephemeral_rend_keypair.pubkey.public_key,
                sizeof(service_ephemeral_rend_keypair.pubkey.public_key));
  printf("%s\n", buf);

 done:
  return retval;
}

/** The final step of the ntor protocol, the client computes and returns the
 *  rendezvous key material. */
static int
client2(int argc, char **argv)
{
  int retval;

  /* Inputs */
  curve25519_public_key_t intro_enc_pubkey;
  ed25519_public_key_t intro_auth_pubkey;
  curve25519_keypair_t client_ephemeral_enc_keypair;
  curve25519_public_key_t service_ephemeral_rend_pubkey;
  hs_subcredential_t subcredential;

  /* Output */
  hs_ntor_rend_cell_keys_t hs_ntor_rend_cell_keys;

  char buf[256];

  N_ARGS(7);
  BASE16(2, intro_auth_pubkey.pubkey, ED25519_PUBKEY_LEN);
  BASE16(3, client_ephemeral_enc_keypair.seckey.secret_key,
         CURVE25519_SECKEY_LEN);
  BASE16(4, intro_enc_pubkey.public_key, CURVE25519_PUBKEY_LEN);
  BASE16(5, service_ephemeral_rend_pubkey.public_key, CURVE25519_PUBKEY_LEN);
  BASE16(6, subcredential.subcred, DIGEST256_LEN);

  /* Generate keypair */
  curve25519_public_key_generate(&client_ephemeral_enc_keypair.pubkey,
                                 &client_ephemeral_enc_keypair.seckey);

  /* Get RENDEZVOUS1 keys */
  retval = hs_ntor_client_get_rendezvous1_keys(&intro_auth_pubkey,
                                               &client_ephemeral_enc_keypair,
                                               &intro_enc_pubkey,
                                               &service_ephemeral_rend_pubkey,
                                               &hs_ntor_rend_cell_keys);
  if (retval < 0) {
    goto done;
  }

  /* Send AUTH_MAC */
  base16_encode(buf, sizeof(buf),
                (const char*)hs_ntor_rend_cell_keys.rend_cell_auth_mac,
                sizeof(hs_ntor_rend_cell_keys.rend_cell_auth_mac));
  printf("%s\n", buf);
  /* Send NTOR_KEY_SEED */
  base16_encode(buf, sizeof(buf),
                (const char*)hs_ntor_rend_cell_keys.ntor_key_seed,
                sizeof(hs_ntor_rend_cell_keys.ntor_key_seed));
  printf("%s\n", buf);

 done:
  return 1;
}

/** Perform a different part of the protocol depdning on the argv used. */
int
main(int argc, char **argv)
{
  if (argc < 2) {
    fprintf(stderr, "I need arguments. Read source for more info.\n");
    return 1;
  }

  init_logging(1);
  curve25519_init();
  if (crypto_global_init(0, NULL, NULL) < 0)
    return 1;

  if (!strcmp(argv[1], "client1")) {
    return client1(argc, argv);
  } else if (!strcmp(argv[1], "server1")) {
    return server1(argc, argv);
  } else if (!strcmp(argv[1], "client2")) {
    return client2(argc, argv);
  } else {
    fprintf(stderr, "What's a %s?\n", argv[1]);
    return 1;
  }
}
