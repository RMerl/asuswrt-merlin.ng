/* Copyright (c) 2017-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/** \file hs_ntor.c
 *  \brief Implements the ntor variant used in Tor hidden services.
 *
 *  \details
 *  This module handles the variant of the ntor handshake that is documented in
 *  section [NTOR-WITH-EXTRA-DATA] of rend-spec-ng.txt .
 *
 *  The functions in this file provide an API that should be used when sending
 *  or receiving INTRODUCE1/RENDEZVOUS1 cells to generate the various key
 *  material required to create and handle those cells.
 *
 *  In the case of INTRODUCE1 it provides encryption and MAC keys to
 *  encode/decode the encrypted blob (see hs_ntor_intro_cell_keys_t). The
 *  relevant pub functions are hs_ntor_{client,service}_get_introduce1_keys().
 *
 *  In the case of RENDEZVOUS1 it calculates the MAC required to authenticate
 *  the cell, and also provides the key seed that is used to derive the crypto
 *  material for rendezvous encryption (see hs_ntor_rend_cell_keys_t). The
 *  relevant pub functions are hs_ntor_{client,service}_get_rendezvous1_keys().
 *  It also provides a function (hs_ntor_circuit_key_expansion()) that does the
 *  rendezvous key expansion to setup end-to-end rend circuit keys.
 */

#include "core/or/or.h"
#include "lib/crypt_ops/crypto_util.h"
#include "lib/crypt_ops/crypto_curve25519.h"
#include "lib/crypt_ops/crypto_ed25519.h"
#include "core/crypto/hs_ntor.h"

/* String constants used by the ntor HS protocol */
#define PROTOID "tor-hs-ntor-curve25519-sha3-256-1"
#define PROTOID_LEN (sizeof(PROTOID) - 1)
#define SERVER_STR "Server"
#define SERVER_STR_LEN (sizeof(SERVER_STR) - 1)

/* Protocol-specific tweaks to our crypto inputs */
#define T_HSENC PROTOID ":hs_key_extract"
#define T_HSENC_LEN (sizeof(T_HSENC) - 1)
#define T_HSVERIFY PROTOID ":hs_verify"
#define T_HSMAC PROTOID ":hs_mac"
#define M_HSEXPAND PROTOID ":hs_key_expand"
#define M_HSEXPAND_LEN (sizeof(M_HSEXPAND) - 1)

/************************* Helper functions: *******************************/

/** Helper macro: copy <b>len</b> bytes from <b>inp</b> to <b>ptr</b> and
 *advance <b>ptr</b> by the number of bytes copied. Stolen from onion_ntor.c */
#define APPEND(ptr, inp, len)                   \
  STMT_BEGIN {                                  \
    memcpy(ptr, (inp), (len));                  \
    ptr += len;                                 \
  } STMT_END

/* Length of EXP(X,y) | EXP(X,b) | AUTH_KEY | B | X | Y | PROTOID */
#define REND_SECRET_HS_INPUT_LEN (CURVE25519_OUTPUT_LEN * 2 + \
  ED25519_PUBKEY_LEN + CURVE25519_PUBKEY_LEN * 3 + PROTOID_LEN)
/* Length of auth_input = verify | AUTH_KEY | B | Y | X | PROTOID | "Server" */
#define REND_AUTH_INPUT_LEN (DIGEST256_LEN + ED25519_PUBKEY_LEN + \
  CURVE25519_PUBKEY_LEN * 3 + PROTOID_LEN + SERVER_STR_LEN)

/** Helper function: Compute the last part of the HS ntor handshake which
 *  derives key material necessary to create and handle RENDEZVOUS1
 *  cells. Function used by both client and service. The actual calculations is
 *  as follows:
 *
 *    NTOR_KEY_SEED = MAC(rend_secret_hs_input, t_hsenc)
 *    verify = MAC(rend_secret_hs_input, t_hsverify)
 *    auth_input = verify | AUTH_KEY | B | Y | X | PROTOID | "Server"
 *    auth_input_mac = MAC(auth_input, t_hsmac)
 *
 *  where in the above, AUTH_KEY is <b>intro_auth_pubkey</b>, B is
 *  <b>intro_enc_pubkey</b>, Y is <b>service_ephemeral_rend_pubkey</b>, and X
 *  is <b>client_ephemeral_enc_pubkey</b>. The provided
 *  <b>rend_secret_hs_input</b> is of size REND_SECRET_HS_INPUT_LEN.
 *
 *  The final results of NTOR_KEY_SEED and auth_input_mac are placed in
 *  <b>hs_ntor_rend_cell_keys_out</b>. Return 0 if everything went fine. */
static int
get_rendezvous1_key_material(const uint8_t *rend_secret_hs_input,
                  const ed25519_public_key_t *intro_auth_pubkey,
                  const curve25519_public_key_t *intro_enc_pubkey,
                  const curve25519_public_key_t *service_ephemeral_rend_pubkey,
                  const curve25519_public_key_t *client_ephemeral_enc_pubkey,
                  hs_ntor_rend_cell_keys_t *hs_ntor_rend_cell_keys_out)
{
  int bad = 0;
  uint8_t ntor_key_seed[DIGEST256_LEN];
  uint8_t ntor_verify[DIGEST256_LEN];
  uint8_t rend_auth_input[REND_AUTH_INPUT_LEN];
  uint8_t rend_cell_auth[DIGEST256_LEN];
  uint8_t *ptr;

  /* Let's build NTOR_KEY_SEED */
  crypto_mac_sha3_256(ntor_key_seed, sizeof(ntor_key_seed),
                      rend_secret_hs_input, REND_SECRET_HS_INPUT_LEN,
                      (const uint8_t *)T_HSENC, strlen(T_HSENC));
  bad |= safe_mem_is_zero(ntor_key_seed, DIGEST256_LEN);

  /* Let's build ntor_verify */
  crypto_mac_sha3_256(ntor_verify, sizeof(ntor_verify),
                      rend_secret_hs_input, REND_SECRET_HS_INPUT_LEN,
                      (const uint8_t *)T_HSVERIFY, strlen(T_HSVERIFY));
  bad |= safe_mem_is_zero(ntor_verify, DIGEST256_LEN);

  /* Let's build auth_input: */
  ptr = rend_auth_input;
  /* Append ntor_verify */
  APPEND(ptr, ntor_verify, sizeof(ntor_verify));
  /* Append AUTH_KEY */
  APPEND(ptr, intro_auth_pubkey->pubkey, ED25519_PUBKEY_LEN);
  /* Append B */
  APPEND(ptr, intro_enc_pubkey->public_key, CURVE25519_PUBKEY_LEN);
  /* Append Y */
  APPEND(ptr,
         service_ephemeral_rend_pubkey->public_key, CURVE25519_PUBKEY_LEN);
  /* Append X */
  APPEND(ptr,
         client_ephemeral_enc_pubkey->public_key, CURVE25519_PUBKEY_LEN);
  /* Append PROTOID */
  APPEND(ptr, PROTOID, strlen(PROTOID));
  /* Append "Server" */
  APPEND(ptr, SERVER_STR, strlen(SERVER_STR));
  tor_assert(ptr == rend_auth_input + sizeof(rend_auth_input));

  /* Let's build auth_input_mac that goes in RENDEZVOUS1 cell */
  crypto_mac_sha3_256(rend_cell_auth, sizeof(rend_cell_auth),
                      rend_auth_input, sizeof(rend_auth_input),
                      (const uint8_t *)T_HSMAC, strlen(T_HSMAC));
  bad |= safe_mem_is_zero(ntor_verify, DIGEST256_LEN);

  { /* Get the computed RENDEZVOUS1 material! */
    memcpy(&hs_ntor_rend_cell_keys_out->rend_cell_auth_mac,
           rend_cell_auth, DIGEST256_LEN);
    memcpy(&hs_ntor_rend_cell_keys_out->ntor_key_seed,
           ntor_key_seed, DIGEST256_LEN);
  }

  memwipe(rend_cell_auth, 0, sizeof(rend_cell_auth));
  memwipe(rend_auth_input, 0, sizeof(rend_auth_input));
  memwipe(ntor_key_seed, 0, sizeof(ntor_key_seed));

  return bad;
}

/** Length of secret_input = EXP(B,x) | AUTH_KEY | X | B | PROTOID */
#define INTRO_SECRET_HS_INPUT_LEN (CURVE25519_OUTPUT_LEN +ED25519_PUBKEY_LEN +\
  CURVE25519_PUBKEY_LEN + CURVE25519_PUBKEY_LEN + PROTOID_LEN)
/* Length of info = m_hsexpand | subcredential */
#define INFO_BLOB_LEN (M_HSEXPAND_LEN + DIGEST256_LEN)
/* Length of KDF input = intro_secret_hs_input | t_hsenc | info */
#define KDF_INPUT_LEN (INTRO_SECRET_HS_INPUT_LEN + T_HSENC_LEN + INFO_BLOB_LEN)

/** Helper function: Compute the part of the HS ntor handshake that generates
 *  key material for creating and handling INTRODUCE1 cells. Function used
 *  by both client and service. Specifically, calculate the following:
 *
 *     info = m_hsexpand | subcredential
 *     hs_keys = KDF(intro_secret_hs_input | t_hsenc | info, S_KEY_LEN+MAC_LEN)
 *     ENC_KEY = hs_keys[0:S_KEY_LEN]
 *     MAC_KEY = hs_keys[S_KEY_LEN:S_KEY_LEN+MAC_KEY_LEN]
 *
 *  where intro_secret_hs_input is <b>secret_input</b> (of size
 *  INTRO_SECRET_HS_INPUT_LEN), and <b>subcredential</b> is of size
 *  DIGEST256_LEN.
 *
 * If everything went well, fill <b>hs_ntor_intro_cell_keys_out</b> with the
 * necessary key material, and return 0. */
static void
get_introduce1_key_material(const uint8_t *secret_input,
                        const hs_subcredential_t *subcredential,
                        hs_ntor_intro_cell_keys_t *hs_ntor_intro_cell_keys_out)
{
  uint8_t keystream[CIPHER256_KEY_LEN + DIGEST256_LEN];
  uint8_t info_blob[INFO_BLOB_LEN];
  uint8_t kdf_input[KDF_INPUT_LEN];
  uint8_t *ptr;

  /* Let's build info */
  ptr = info_blob;
  APPEND(ptr, M_HSEXPAND, strlen(M_HSEXPAND));
  APPEND(ptr, subcredential->subcred, SUBCRED_LEN);
  tor_assert(ptr == info_blob + sizeof(info_blob));

  /* Let's build the input to the KDF */
  ptr = kdf_input;
  APPEND(ptr, secret_input, INTRO_SECRET_HS_INPUT_LEN);
  APPEND(ptr, T_HSENC, strlen(T_HSENC));
  APPEND(ptr, info_blob, sizeof(info_blob));
  tor_assert(ptr == kdf_input + sizeof(kdf_input));

  /* Now we need to run kdf_input over SHAKE-256 */
  crypto_xof(keystream, sizeof(keystream),
             kdf_input, sizeof(kdf_input));

  { /* Get the keys */
    memcpy(&hs_ntor_intro_cell_keys_out->enc_key, keystream,CIPHER256_KEY_LEN);
    memcpy(&hs_ntor_intro_cell_keys_out->mac_key,
           keystream+CIPHER256_KEY_LEN, DIGEST256_LEN);
  }

  memwipe(keystream,  0, sizeof(keystream));
  memwipe(kdf_input,  0, sizeof(kdf_input));
}

/** Helper function: Calculate the 'intro_secret_hs_input' element used by the
 * HS ntor handshake and place it in <b>secret_input_out</b>. This function is
 * used by both client and service code.
 *
 * For the client-side it looks like this:
 *
 *         intro_secret_hs_input = EXP(B,x) | AUTH_KEY | X | B | PROTOID
 *
 * whereas for the service-side it looks like this:
 *
 *         intro_secret_hs_input = EXP(X,b) | AUTH_KEY | X | B | PROTOID
 *
 * In this function, <b>dh_result</b> carries the EXP() result (and has size
 * CURVE25519_OUTPUT_LEN) <b>intro_auth_pubkey</b> is AUTH_KEY,
 * <b>client_ephemeral_enc_pubkey</b> is X, and <b>intro_enc_pubkey</b> is B.
 */
static void
get_intro_secret_hs_input(const uint8_t *dh_result,
                    const ed25519_public_key_t *intro_auth_pubkey,
                    const curve25519_public_key_t *client_ephemeral_enc_pubkey,
                    const curve25519_public_key_t *intro_enc_pubkey,
                    uint8_t *secret_input_out)
{
  uint8_t *ptr;

  /* Append EXP() */
  ptr = secret_input_out;
  APPEND(ptr, dh_result, CURVE25519_OUTPUT_LEN);
  /* Append AUTH_KEY */
  APPEND(ptr, intro_auth_pubkey->pubkey, ED25519_PUBKEY_LEN);
  /* Append X */
  APPEND(ptr, client_ephemeral_enc_pubkey->public_key, CURVE25519_PUBKEY_LEN);
  /* Append B */
  APPEND(ptr, intro_enc_pubkey->public_key, CURVE25519_PUBKEY_LEN);
  /* Append PROTOID */
  APPEND(ptr, PROTOID, strlen(PROTOID));
  tor_assert(ptr == secret_input_out + INTRO_SECRET_HS_INPUT_LEN);
}

/** Calculate the 'rend_secret_hs_input' element used by the HS ntor handshake
 *  and place it in <b>rend_secret_hs_input_out</b>. This function is used by
 *  both client and service code.
 *
 * The computation on the client side is:
 *  rend_secret_hs_input = EXP(X,y) | EXP(X,b) | AUTH_KEY | B | X | Y | PROTOID
 * whereas on the service side it is:
 *  rend_secret_hs_input = EXP(Y,x) | EXP(B,x) | AUTH_KEY | B | X | Y | PROTOID
 *
 * where:
 * <b>dh_result1</b> and <b>dh_result2</b> carry the two EXP() results (of size
 * CURVE25519_OUTPUT_LEN)
 * <b>intro_auth_pubkey</b> is AUTH_KEY,
 * <b>intro_enc_pubkey</b> is B,
 * <b>client_ephemeral_enc_pubkey</b> is X, and
 * <b>service_ephemeral_rend_pubkey</b> is Y.
 */
static void
get_rend_secret_hs_input(const uint8_t *dh_result1, const uint8_t *dh_result2,
                  const ed25519_public_key_t *intro_auth_pubkey,
                  const curve25519_public_key_t *intro_enc_pubkey,
                  const curve25519_public_key_t *client_ephemeral_enc_pubkey,
                  const curve25519_public_key_t *service_ephemeral_rend_pubkey,
                  uint8_t *rend_secret_hs_input_out)
{
  uint8_t *ptr;

  ptr = rend_secret_hs_input_out;
  /* Append the first EXP() */
  APPEND(ptr, dh_result1, CURVE25519_OUTPUT_LEN);
  /* Append the other EXP() */
  APPEND(ptr, dh_result2, CURVE25519_OUTPUT_LEN);
  /* Append AUTH_KEY */
  APPEND(ptr, intro_auth_pubkey->pubkey, ED25519_PUBKEY_LEN);
  /* Append B */
  APPEND(ptr, intro_enc_pubkey->public_key, CURVE25519_PUBKEY_LEN);
  /* Append X */
  APPEND(ptr,
         client_ephemeral_enc_pubkey->public_key, CURVE25519_PUBKEY_LEN);
  /* Append Y */
  APPEND(ptr,
         service_ephemeral_rend_pubkey->public_key, CURVE25519_PUBKEY_LEN);
  /* Append PROTOID */
  APPEND(ptr, PROTOID, strlen(PROTOID));
  tor_assert(ptr == rend_secret_hs_input_out + REND_SECRET_HS_INPUT_LEN);
}

/************************* Public functions: *******************************/

/* Public function: Do the appropriate ntor calculations and derive the keys
 * needed to encrypt and authenticate INTRODUCE1 cells. Return 0 and place the
 * final key material in <b>hs_ntor_intro_cell_keys_out</b> if everything went
 * well, otherwise return -1;
 *
 * The relevant calculations are as follows:
 *
 *     intro_secret_hs_input = EXP(B,x) | AUTH_KEY | X | B | PROTOID
 *     info = m_hsexpand | subcredential
 *     hs_keys = KDF(intro_secret_hs_input | t_hsenc | info, S_KEY_LEN+MAC_LEN)
 *     ENC_KEY = hs_keys[0:S_KEY_LEN]
 *     MAC_KEY = hs_keys[S_KEY_LEN:S_KEY_LEN+MAC_KEY_LEN]
 *
 * where:
 * <b>intro_auth_pubkey</b> is AUTH_KEY (found in HS descriptor),
 * <b>intro_enc_pubkey</b> is B (also found in HS descriptor),
 * <b>client_ephemeral_enc_keypair</b> is freshly generated keypair (x,X)
 * <b>subcredential</b> is the hidden service subcredential (of size
 * DIGEST256_LEN). */
int
hs_ntor_client_get_introduce1_keys(
                      const ed25519_public_key_t *intro_auth_pubkey,
                      const curve25519_public_key_t *intro_enc_pubkey,
                      const curve25519_keypair_t *client_ephemeral_enc_keypair,
                      const hs_subcredential_t *subcredential,
                      hs_ntor_intro_cell_keys_t *hs_ntor_intro_cell_keys_out)
{
  int bad = 0;
  uint8_t secret_input[INTRO_SECRET_HS_INPUT_LEN];
  uint8_t dh_result[CURVE25519_OUTPUT_LEN];

  tor_assert(intro_auth_pubkey);
  tor_assert(intro_enc_pubkey);
  tor_assert(client_ephemeral_enc_keypair);
  tor_assert(subcredential);
  tor_assert(hs_ntor_intro_cell_keys_out);

  /* Calculate EXP(B,x) */
  curve25519_handshake(dh_result,
                       &client_ephemeral_enc_keypair->seckey,
                       intro_enc_pubkey);
  bad |= safe_mem_is_zero(dh_result, CURVE25519_OUTPUT_LEN);

  /* Get intro_secret_hs_input */
  get_intro_secret_hs_input(dh_result, intro_auth_pubkey,
                            &client_ephemeral_enc_keypair->pubkey,
                            intro_enc_pubkey, secret_input);
  bad |= safe_mem_is_zero(secret_input, CURVE25519_OUTPUT_LEN);

  /* Get ENC_KEY and MAC_KEY! */
  get_introduce1_key_material(secret_input, subcredential,
                              hs_ntor_intro_cell_keys_out);

  /* Cleanup */
  memwipe(secret_input,  0, sizeof(secret_input));
  if (bad) {
    memwipe(hs_ntor_intro_cell_keys_out, 0, sizeof(hs_ntor_intro_cell_keys_t));
  }

  return bad ? -1 : 0;
}

/* Public function: Do the appropriate ntor calculations and derive the keys
 * needed to verify RENDEZVOUS1 cells and encrypt further rendezvous
 * traffic. Return 0 and place the final key material in
 * <b>hs_ntor_rend_cell_keys_out</b> if everything went well, else return -1.
 *
 * The relevant calculations are as follows:
 *
 *  rend_secret_hs_input = EXP(Y,x) | EXP(B,x) | AUTH_KEY | B | X | Y | PROTOID
 *  NTOR_KEY_SEED = MAC(rend_secret_hs_input, t_hsenc)
 *  verify = MAC(rend_secret_hs_input, t_hsverify)
 *  auth_input = verify | AUTH_KEY | B | Y | X | PROTOID | "Server"
 *  auth_input_mac = MAC(auth_input, t_hsmac)
 *
 * where:
 * <b>intro_auth_pubkey</b> is AUTH_KEY (found in HS descriptor),
 * <b>client_ephemeral_enc_keypair</b> is freshly generated keypair (x,X)
 * <b>intro_enc_pubkey</b> is B (also found in HS descriptor),
 * <b>service_ephemeral_rend_pubkey</b> is Y (SERVER_PK in RENDEZVOUS1 cell) */
int
hs_ntor_client_get_rendezvous1_keys(
                  const ed25519_public_key_t *intro_auth_pubkey,
                  const curve25519_keypair_t *client_ephemeral_enc_keypair,
                  const curve25519_public_key_t *intro_enc_pubkey,
                  const curve25519_public_key_t *service_ephemeral_rend_pubkey,
                  hs_ntor_rend_cell_keys_t *hs_ntor_rend_cell_keys_out)
{
  int bad = 0;
  uint8_t rend_secret_hs_input[REND_SECRET_HS_INPUT_LEN];
  uint8_t dh_result1[CURVE25519_OUTPUT_LEN];
  uint8_t dh_result2[CURVE25519_OUTPUT_LEN];

  tor_assert(intro_auth_pubkey);
  tor_assert(client_ephemeral_enc_keypair);
  tor_assert(intro_enc_pubkey);
  tor_assert(service_ephemeral_rend_pubkey);
  tor_assert(hs_ntor_rend_cell_keys_out);

  /* Compute EXP(Y, x) */
  curve25519_handshake(dh_result1,
                       &client_ephemeral_enc_keypair->seckey,
                       service_ephemeral_rend_pubkey);
  bad |= safe_mem_is_zero(dh_result1, CURVE25519_OUTPUT_LEN);

  /* Compute EXP(B, x) */
  curve25519_handshake(dh_result2,
                       &client_ephemeral_enc_keypair->seckey,
                       intro_enc_pubkey);
  bad |= safe_mem_is_zero(dh_result2, CURVE25519_OUTPUT_LEN);

  /* Get rend_secret_hs_input */
  get_rend_secret_hs_input(dh_result1, dh_result2,
                           intro_auth_pubkey, intro_enc_pubkey,
                           &client_ephemeral_enc_keypair->pubkey,
                           service_ephemeral_rend_pubkey,
                           rend_secret_hs_input);

  /* Get NTOR_KEY_SEED and the auth_input MAC */
  bad |= get_rendezvous1_key_material(rend_secret_hs_input,
                                      intro_auth_pubkey,
                                      intro_enc_pubkey,
                                      service_ephemeral_rend_pubkey,
                                      &client_ephemeral_enc_keypair->pubkey,
                                      hs_ntor_rend_cell_keys_out);

  memwipe(rend_secret_hs_input, 0, sizeof(rend_secret_hs_input));
  if (bad) {
    memwipe(hs_ntor_rend_cell_keys_out, 0, sizeof(hs_ntor_rend_cell_keys_t));
  }

  return bad ? -1 : 0;
}

/* Public function: Do the appropriate ntor calculations and derive the keys
 * needed to decrypt and verify INTRODUCE1 cells. Return 0 and place the final
 * key material in <b>hs_ntor_intro_cell_keys_out</b> if everything went well,
 * otherwise return -1;
 *
 * The relevant calculations are as follows:
 *
 *    intro_secret_hs_input = EXP(X,b) | AUTH_KEY | X | B | PROTOID
 *    info = m_hsexpand | subcredential
 *    hs_keys = KDF(intro_secret_hs_input | t_hsenc | info, S_KEY_LEN+MAC_LEN)
 *    HS_DEC_KEY = hs_keys[0:S_KEY_LEN]
 *    HS_MAC_KEY = hs_keys[S_KEY_LEN:S_KEY_LEN+MAC_KEY_LEN]
 *
 * where:
 * <b>intro_auth_pubkey</b> is AUTH_KEY (introduction point auth key),
 * <b>intro_enc_keypair</b> is (b,B) (introduction point encryption keypair),
 * <b>client_ephemeral_enc_pubkey</b> is X (CLIENT_PK in INTRODUCE2 cell),
 * <b>subcredential</b> is the HS subcredential (of size DIGEST256_LEN) */
int
hs_ntor_service_get_introduce1_keys(
                    const ed25519_public_key_t *intro_auth_pubkey,
                    const curve25519_keypair_t *intro_enc_keypair,
                    const curve25519_public_key_t *client_ephemeral_enc_pubkey,
                    const hs_subcredential_t *subcredential,
                    hs_ntor_intro_cell_keys_t *hs_ntor_intro_cell_keys_out)
{
  return hs_ntor_service_get_introduce1_keys_multi(
                             intro_auth_pubkey,
                             intro_enc_keypair,
                             client_ephemeral_enc_pubkey,
                             1,
                             subcredential,
                             hs_ntor_intro_cell_keys_out);
}

/**
 * As hs_ntor_service_get_introduce1_keys(), but take multiple subcredentials
 * as input, and yield multiple sets of keys as output.
 **/
int
hs_ntor_service_get_introduce1_keys_multi(
            const struct ed25519_public_key_t *intro_auth_pubkey,
            const struct curve25519_keypair_t *intro_enc_keypair,
            const struct curve25519_public_key_t *client_ephemeral_enc_pubkey,
            size_t n_subcredentials,
            const hs_subcredential_t *subcredentials,
            hs_ntor_intro_cell_keys_t *hs_ntor_intro_cell_keys_out)
{
  int bad = 0;
  uint8_t secret_input[INTRO_SECRET_HS_INPUT_LEN];
  uint8_t dh_result[CURVE25519_OUTPUT_LEN];

  tor_assert(intro_auth_pubkey);
  tor_assert(intro_enc_keypair);
  tor_assert(client_ephemeral_enc_pubkey);
  tor_assert(n_subcredentials >= 1);
  tor_assert(subcredentials);
  tor_assert(hs_ntor_intro_cell_keys_out);

  /* Compute EXP(X, b) */
  curve25519_handshake(dh_result,
                       &intro_enc_keypair->seckey,
                       client_ephemeral_enc_pubkey);
  bad |= safe_mem_is_zero(dh_result, CURVE25519_OUTPUT_LEN);

  /* Get intro_secret_hs_input */
  get_intro_secret_hs_input(dh_result, intro_auth_pubkey,
                            client_ephemeral_enc_pubkey,
                            &intro_enc_keypair->pubkey,
                            secret_input);
  bad |= safe_mem_is_zero(secret_input, CURVE25519_OUTPUT_LEN);

  for (unsigned i = 0; i < n_subcredentials; ++i) {
    /* Get ENC_KEY and MAC_KEY! */
    get_introduce1_key_material(secret_input, &subcredentials[i],
                                &hs_ntor_intro_cell_keys_out[i]);
  }

  memwipe(secret_input,  0, sizeof(secret_input));
  if (bad) {
    memwipe(hs_ntor_intro_cell_keys_out, 0,
            sizeof(hs_ntor_intro_cell_keys_t) * n_subcredentials);
  }

  return bad ? -1 : 0;
}

/* Public function: Do the appropriate ntor calculations and derive the keys
 * needed to create and authenticate RENDEZVOUS1 cells. Return 0 and place the
 * final key material in <b>hs_ntor_rend_cell_keys_out</b> if all went fine,
 * return -1 if error happened.
 *
 * The relevant calculations are as follows:
 *
 *  rend_secret_hs_input = EXP(X,y) | EXP(X,b) | AUTH_KEY | B | X | Y | PROTOID
 *  NTOR_KEY_SEED = MAC(rend_secret_hs_input, t_hsenc)
 *  verify = MAC(rend_secret_hs_input, t_hsverify)
 *  auth_input = verify | AUTH_KEY | B | Y | X | PROTOID | "Server"
 *  auth_input_mac = MAC(auth_input, t_hsmac)
 *
 * where:
 * <b>intro_auth_pubkey</b> is AUTH_KEY (intro point auth key),
 * <b>intro_enc_keypair</b> is (b,B) (intro point enc keypair)
 * <b>service_ephemeral_rend_keypair</b> is a fresh (y,Y) keypair
 * <b>client_ephemeral_enc_pubkey</b> is X (CLIENT_PK in INTRODUCE2 cell) */
int
hs_ntor_service_get_rendezvous1_keys(
                    const ed25519_public_key_t *intro_auth_pubkey,
                    const curve25519_keypair_t *intro_enc_keypair,
                    const curve25519_keypair_t *service_ephemeral_rend_keypair,
                    const curve25519_public_key_t *client_ephemeral_enc_pubkey,
                    hs_ntor_rend_cell_keys_t *hs_ntor_rend_cell_keys_out)
{
  int bad = 0;
  uint8_t rend_secret_hs_input[REND_SECRET_HS_INPUT_LEN];
  uint8_t dh_result1[CURVE25519_OUTPUT_LEN];
  uint8_t dh_result2[CURVE25519_OUTPUT_LEN];

  tor_assert(intro_auth_pubkey);
  tor_assert(intro_enc_keypair);
  tor_assert(service_ephemeral_rend_keypair);
  tor_assert(client_ephemeral_enc_pubkey);
  tor_assert(hs_ntor_rend_cell_keys_out);

  /* Compute EXP(X, y) */
  curve25519_handshake(dh_result1,
                       &service_ephemeral_rend_keypair->seckey,
                       client_ephemeral_enc_pubkey);
  bad |= safe_mem_is_zero(dh_result1, CURVE25519_OUTPUT_LEN);

  /* Compute EXP(X, b) */
  curve25519_handshake(dh_result2,
                       &intro_enc_keypair->seckey,
                       client_ephemeral_enc_pubkey);
  bad |= safe_mem_is_zero(dh_result2, CURVE25519_OUTPUT_LEN);

  /* Get rend_secret_hs_input */
  get_rend_secret_hs_input(dh_result1, dh_result2,
                           intro_auth_pubkey,
                           &intro_enc_keypair->pubkey,
                           client_ephemeral_enc_pubkey,
                           &service_ephemeral_rend_keypair->pubkey,
                           rend_secret_hs_input);

  /* Get NTOR_KEY_SEED and AUTH_INPUT_MAC! */
  bad |= get_rendezvous1_key_material(rend_secret_hs_input,
                                      intro_auth_pubkey,
                                      &intro_enc_keypair->pubkey,
                                      &service_ephemeral_rend_keypair->pubkey,
                                      client_ephemeral_enc_pubkey,
                                      hs_ntor_rend_cell_keys_out);

  memwipe(rend_secret_hs_input, 0, sizeof(rend_secret_hs_input));
  if (bad) {
    memwipe(hs_ntor_rend_cell_keys_out, 0, sizeof(hs_ntor_rend_cell_keys_t));
  }

  return bad ? -1 : 0;
}

/** Given a received RENDEZVOUS2 MAC in <b>mac</b> (of length DIGEST256_LEN),
 *  and the RENDEZVOUS1 key material in <b>hs_ntor_rend_cell_keys</b>, return 1
 *  if the MAC is good, otherwise return 0. */
int
hs_ntor_client_rendezvous2_mac_is_good(
                        const hs_ntor_rend_cell_keys_t *hs_ntor_rend_cell_keys,
                        const uint8_t *rcvd_mac)
{
  tor_assert(rcvd_mac);
  tor_assert(hs_ntor_rend_cell_keys);

  return tor_memeq(hs_ntor_rend_cell_keys->rend_cell_auth_mac,
                   rcvd_mac, DIGEST256_LEN);
}

/* Input length to KDF for key expansion */
#define NTOR_KEY_EXPANSION_KDF_INPUT_LEN (DIGEST256_LEN + M_HSEXPAND_LEN)

/** Given the rendezvous key seed in <b>ntor_key_seed</b> (of size
 *  DIGEST256_LEN), do the circuit key expansion as specified by section
 *  '4.2.1. Key expansion' and place the keys in <b>keys_out</b> (which must be
 *  of size HS_NTOR_KEY_EXPANSION_KDF_OUT_LEN).
 *
 * Return 0 if things went well, else return -1. */
int
hs_ntor_circuit_key_expansion(const uint8_t *ntor_key_seed, size_t seed_len,
                              uint8_t *keys_out, size_t keys_out_len)
{
  uint8_t *ptr;
  uint8_t kdf_input[NTOR_KEY_EXPANSION_KDF_INPUT_LEN];

  /* Sanity checks on lengths to make sure we are good */
  if (BUG(seed_len != DIGEST256_LEN)) {
    return -1;
  }
  if (BUG(keys_out_len != HS_NTOR_KEY_EXPANSION_KDF_OUT_LEN)) {
    return -1;
  }

  /* Let's build the input to the KDF */
  ptr = kdf_input;
  APPEND(ptr, ntor_key_seed, DIGEST256_LEN);
  APPEND(ptr, M_HSEXPAND, strlen(M_HSEXPAND));
  tor_assert(ptr == kdf_input + sizeof(kdf_input));

  /* Generate the keys */
  crypto_xof(keys_out, HS_NTOR_KEY_EXPANSION_KDF_OUT_LEN,
             kdf_input, sizeof(kdf_input));

  return 0;
}
