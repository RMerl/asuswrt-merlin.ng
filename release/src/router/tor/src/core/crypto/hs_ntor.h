/* Copyright (c) 2017-2019, The Tor Project, Inc. */
/* See LICENSE for licensing information */

#ifndef TOR_HS_NTOR_H
#define TOR_HS_NTOR_H

#include "core/or/or.h"
struct ed25519_public_key_t;
struct curve25519_public_key_t;
struct curve25519_keypair_t;

/* Output length of KDF for key expansion */
#define HS_NTOR_KEY_EXPANSION_KDF_OUT_LEN \
  (DIGEST256_LEN*2 + CIPHER256_KEY_LEN*2)

/* Key material needed to encode/decode INTRODUCE1 cells */
typedef struct {
  /* Key used for encryption of encrypted INTRODUCE1 blob */
  uint8_t enc_key[CIPHER256_KEY_LEN];
  /* MAC key used to protect encrypted INTRODUCE1 blob */
  uint8_t mac_key[DIGEST256_LEN];
} hs_ntor_intro_cell_keys_t;

/* Key material needed to encode/decode RENDEZVOUS1 cells */
typedef struct {
  /* This is the MAC of the HANDSHAKE_INFO field */
  uint8_t rend_cell_auth_mac[DIGEST256_LEN];
  /* This is the key seed used to derive further rendezvous crypto keys as
   * detailed in section 4.2.1 of rend-spec-ng.txt. */
  uint8_t ntor_key_seed[DIGEST256_LEN];
} hs_ntor_rend_cell_keys_t;

int hs_ntor_client_get_introduce1_keys(
              const struct ed25519_public_key_t *intro_auth_pubkey,
              const struct curve25519_public_key_t *intro_enc_pubkey,
              const struct curve25519_keypair_t *client_ephemeral_enc_keypair,
              const uint8_t *subcredential,
              hs_ntor_intro_cell_keys_t *hs_ntor_intro_cell_keys_out);

int hs_ntor_client_get_rendezvous1_keys(
          const struct ed25519_public_key_t *intro_auth_pubkey,
          const struct curve25519_keypair_t *client_ephemeral_enc_keypair,
          const struct curve25519_public_key_t *intro_enc_pubkey,
          const struct curve25519_public_key_t *service_ephemeral_rend_pubkey,
          hs_ntor_rend_cell_keys_t *hs_ntor_rend_cell_keys_out);

int hs_ntor_service_get_introduce1_keys(
            const struct ed25519_public_key_t *intro_auth_pubkey,
            const struct curve25519_keypair_t *intro_enc_keypair,
            const struct curve25519_public_key_t *client_ephemeral_enc_pubkey,
            const uint8_t *subcredential,
            hs_ntor_intro_cell_keys_t *hs_ntor_intro_cell_keys_out);

int hs_ntor_service_get_rendezvous1_keys(
            const struct ed25519_public_key_t *intro_auth_pubkey,
            const struct curve25519_keypair_t *intro_enc_keypair,
            const struct curve25519_keypair_t *service_ephemeral_rend_keypair,
            const struct curve25519_public_key_t *client_ephemeral_enc_pubkey,
            hs_ntor_rend_cell_keys_t *hs_ntor_rend_cell_keys_out);

int hs_ntor_circuit_key_expansion(const uint8_t *ntor_key_seed,
                                  size_t seed_len,
                                  uint8_t *keys_out, size_t keys_out_len);

int hs_ntor_client_rendezvous2_mac_is_good(
                        const hs_ntor_rend_cell_keys_t *hs_ntor_rend_cell_keys,
                        const uint8_t *rcvd_mac);

#endif /* !defined(TOR_HS_NTOR_H) */
