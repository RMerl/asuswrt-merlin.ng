/* Copyright (c) 2016-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file hs_descriptor.c
 * \brief Handle hidden service descriptor encoding/decoding.
 *
 * \details
 * Here is a graphical depiction of an HS descriptor and its layers:
 *
 *      +------------------------------------------------------+
 *      |DESCRIPTOR HEADER:                                    |
 *      |  hs-descriptor 3                                     |
 *      |  descriptor-lifetime 180                             |
 *      |  ...                                                 |
 *      |  superencrypted                                      |
 *      |+---------------------------------------------------+ |
 *      ||SUPERENCRYPTED LAYER (aka OUTER ENCRYPTED LAYER):  | |
 *      ||  desc-auth-type x25519                            | |
 *      ||  desc-auth-ephemeral-key                          | |
 *      ||  auth-client                                      | |
 *      ||  auth-client                                      | |
 *      ||  ...                                              | |
 *      ||  encrypted                                        | |
 *      ||+-------------------------------------------------+| |
 *      |||ENCRYPTED LAYER (aka INNER ENCRYPTED LAYER):     || |
 *      |||  create2-formats                                || |
 *      |||  intro-auth-required                            || |
 *      |||  introduction-point                             || |
 *      |||  introduction-point                             || |
 *      |||  ...                                            || |
 *      ||+-------------------------------------------------+| |
 *      |+---------------------------------------------------+ |
 *      +------------------------------------------------------+
 *
 * The DESCRIPTOR HEADER section is completely unencrypted and contains generic
 * descriptor metadata.
 *
 * The SUPERENCRYPTED LAYER section is the first layer of encryption, and it's
 * encrypted using the blinded public key of the hidden service to protect
 * against entities who don't know its onion address. The clients of the hidden
 * service know its onion address and blinded public key, whereas third-parties
 * (like HSDirs) don't know it (except if it's a public hidden service).
 *
 * The ENCRYPTED LAYER section is the second layer of encryption, and it's
 * encrypted using the client authorization key material (if those exist). When
 * client authorization is enabled, this second layer of encryption protects
 * the descriptor content from unauthorized entities. If client authorization
 * is disabled, this second layer of encryption does not provide any extra
 * security but is still present. The plaintext of this layer contains all the
 * information required to connect to the hidden service like its list of
 * introduction points.
 **/

/* For unit tests.*/
#define HS_DESCRIPTOR_PRIVATE

#include <stdbool.h>
#include "core/or/or.h"
#include "app/config/config.h"
#include "trunnel/ed25519_cert.h" /* Trunnel interface. */
#include "feature/hs/hs_descriptor.h"
#include "core/or/circuitbuild.h"
#include "core/or/congestion_control_common.h"
#include "core/or/protover.h"
#include "lib/crypt_ops/crypto_rand.h"
#include "lib/crypt_ops/crypto_util.h"
#include "feature/dirparse/parsecommon.h"
#include "feature/hs/hs_cache.h"
#include "feature/hs/hs_config.h"
#include "feature/hs/hs_pow.h"
#include "feature/nodelist/torcert.h" /* tor_cert_encode_ed22519() */
#include "lib/memarea/memarea.h"
#include "lib/crypt_ops/crypto_format.h"
#include "core/or/versions.h"

#include "core/or/extend_info_st.h"

/* Constant string value used for the descriptor format. */
#define str_hs_desc "hs-descriptor"
#define str_desc_cert "descriptor-signing-key-cert"
#define str_rev_counter "revision-counter"
#define str_superencrypted "superencrypted"
#define str_encrypted "encrypted"
#define str_signature "signature"
#define str_lifetime "descriptor-lifetime"
/* Constant string value for the encrypted part of the descriptor. */
#define str_create2_formats "create2-formats"
#define str_intro_auth_required "intro-auth-required"
#define str_single_onion "single-onion-service"
#define str_intro_point "introduction-point"
#define str_ip_onion_key "onion-key"
#define str_ip_auth_key "auth-key"
#define str_ip_enc_key "enc-key"
#define str_ip_enc_key_cert "enc-key-cert"
#define str_ip_legacy_key "legacy-key"
#define str_ip_legacy_key_cert "legacy-key-cert"
#define str_intro_point_start "\n" str_intro_point " "
#define str_flow_control "flow-control"
#define str_pow_params "pow-params"
/* Constant string value for the construction to encrypt the encrypted data
 * section. */
#define str_enc_const_superencryption "hsdir-superencrypted-data"
#define str_enc_const_encryption "hsdir-encrypted-data"
/* Prefix required to compute/verify HS desc signatures */
#define str_desc_sig_prefix "Tor onion service descriptor sig v3"
#define str_desc_auth_type "desc-auth-type"
#define str_desc_auth_key "desc-auth-ephemeral-key"
#define str_desc_auth_client "auth-client"
#define str_encrypted "encrypted"

/** Authentication supported types. */
static const struct {
  hs_desc_auth_type_t type;
  const char *identifier;
} intro_auth_types[] = {
  { HS_DESC_AUTH_ED25519, "ed25519" },
  /* Indicate end of array. */
  { 0, NULL }
};

/** PoW supported types. */
static const struct {
  hs_pow_desc_type_t type;
  const char *identifier;
} pow_types[] = {
  { HS_POW_DESC_V1, "v1"},
  /* Indicate end of array. */
  { 0, NULL }
};

/** Descriptor ruleset. */
static token_rule_t hs_desc_v3_token_table[] = {
  T1_START(str_hs_desc, R_HS_DESCRIPTOR, EQ(1), NO_OBJ),
  T1(str_lifetime, R3_DESC_LIFETIME, EQ(1), NO_OBJ),
  T1(str_desc_cert, R3_DESC_SIGNING_CERT, NO_ARGS, NEED_OBJ),
  T1(str_rev_counter, R3_REVISION_COUNTER, EQ(1), NO_OBJ),
  T1(str_superencrypted, R3_SUPERENCRYPTED, NO_ARGS, NEED_OBJ),
  T1_END(str_signature, R3_SIGNATURE, EQ(1), NO_OBJ),
  END_OF_TABLE
};

/** Descriptor ruleset for the superencrypted section. */
static token_rule_t hs_desc_superencrypted_v3_token_table[] = {
  T1_START(str_desc_auth_type, R3_DESC_AUTH_TYPE, GE(1), NO_OBJ),
  T1(str_desc_auth_key, R3_DESC_AUTH_KEY, GE(1), NO_OBJ),
  T1N(str_desc_auth_client, R3_DESC_AUTH_CLIENT, GE(3), NO_OBJ),
  T1(str_encrypted, R3_ENCRYPTED, NO_ARGS, NEED_OBJ),
  END_OF_TABLE
};

/** Descriptor ruleset for the encrypted section. */
static token_rule_t hs_desc_encrypted_v3_token_table[] = {
  T1_START(str_create2_formats, R3_CREATE2_FORMATS, CONCAT_ARGS, NO_OBJ),
  T01(str_intro_auth_required, R3_INTRO_AUTH_REQUIRED, GE(1), NO_OBJ),
  T01(str_single_onion, R3_SINGLE_ONION_SERVICE, ARGS, NO_OBJ),
  T01(str_flow_control, R3_FLOW_CONTROL, GE(2), NO_OBJ),
  T01(str_pow_params, R3_POW_PARAMS, GE(4), NO_OBJ),
  END_OF_TABLE
};

/** Descriptor ruleset for the introduction points section. */
static token_rule_t hs_desc_intro_point_v3_token_table[] = {
  T1_START(str_intro_point, R3_INTRODUCTION_POINT, EQ(1), NO_OBJ),
  T1N(str_ip_onion_key, R3_INTRO_ONION_KEY, GE(2), OBJ_OK),
  T1(str_ip_auth_key, R3_INTRO_AUTH_KEY, NO_ARGS, NEED_OBJ),
  T1(str_ip_enc_key, R3_INTRO_ENC_KEY, GE(2), OBJ_OK),
  T1(str_ip_enc_key_cert, R3_INTRO_ENC_KEY_CERT, ARGS, OBJ_OK),
  T01(str_ip_legacy_key, R3_INTRO_LEGACY_KEY, ARGS, NEED_KEY_1024),
  T01(str_ip_legacy_key_cert, R3_INTRO_LEGACY_KEY_CERT, ARGS, OBJ_OK),
  END_OF_TABLE
};

/** Using a key, salt and encrypted payload, build a MAC and put it in mac_out.
 * We use SHA3-256 for the MAC computation.
 * This function can't fail. */
static void
build_mac(const uint8_t *mac_key, size_t mac_key_len,
          const uint8_t *salt, size_t salt_len,
          const uint8_t *encrypted, size_t encrypted_len,
          uint8_t *mac_out, size_t mac_len)
{
  crypto_digest_t *digest;

  const uint64_t mac_len_netorder = tor_htonll(mac_key_len);
  const uint64_t salt_len_netorder = tor_htonll(salt_len);

  tor_assert(mac_key);
  tor_assert(salt);
  tor_assert(encrypted);
  tor_assert(mac_out);

  digest = crypto_digest256_new(DIGEST_SHA3_256);
  /* As specified in section 2.5 of proposal 224, first add the mac key
   * then add the salt first and then the encrypted section. */

  crypto_digest_add_bytes(digest, (const char *) &mac_len_netorder, 8);
  crypto_digest_add_bytes(digest, (const char *) mac_key, mac_key_len);
  crypto_digest_add_bytes(digest, (const char *) &salt_len_netorder, 8);
  crypto_digest_add_bytes(digest, (const char *) salt, salt_len);
  crypto_digest_add_bytes(digest, (const char *) encrypted, encrypted_len);
  crypto_digest_get_digest(digest, (char *) mac_out, mac_len);
  crypto_digest_free(digest);
}

/** Using a secret data and a given descriptor object, build the secret
 * input needed for the KDF.
 *
 * secret_input = SECRET_DATA | subcredential | INT_8(revision_counter)
 *
 * Then, set the newly allocated buffer in secret_input_out and return the
 * length of the buffer. */
static size_t
build_secret_input(const hs_descriptor_t *desc,
                   const uint8_t *secret_data,
                   size_t secret_data_len,
                   uint8_t **secret_input_out)
{
  size_t offset = 0;
  size_t secret_input_len = secret_data_len + DIGEST256_LEN + sizeof(uint64_t);
  uint8_t *secret_input = NULL;

  tor_assert(desc);
  tor_assert(secret_data);
  tor_assert(secret_input_out);

  secret_input = tor_malloc_zero(secret_input_len);

  /* Copy the secret data. */
  memcpy(secret_input, secret_data, secret_data_len);
  offset += secret_data_len;
  /* Copy subcredential. */
  memcpy(secret_input + offset, desc->subcredential.subcred, DIGEST256_LEN);
  offset += DIGEST256_LEN;
  /* Copy revision counter value. */
  set_uint64(secret_input + offset,
             tor_htonll(desc->plaintext_data.revision_counter));
  offset += sizeof(uint64_t);
  tor_assert(secret_input_len == offset);

  *secret_input_out = secret_input;

  return secret_input_len;
}

/** Do the KDF construction and put the resulting data in key_out which is of
 * key_out_len length. It uses SHAKE-256 as specified in the spec. */
static void
build_kdf_key(const hs_descriptor_t *desc,
              const uint8_t *secret_data,
              size_t secret_data_len,
              const uint8_t *salt, size_t salt_len,
              uint8_t *key_out, size_t key_out_len,
              int is_superencrypted_layer)
{
  uint8_t *secret_input = NULL;
  size_t secret_input_len;
  crypto_xof_t *xof;

  tor_assert(desc);
  tor_assert(secret_data);
  tor_assert(salt);
  tor_assert(key_out);

  /* Build the secret input for the KDF computation. */
  secret_input_len = build_secret_input(desc, secret_data,
                                        secret_data_len, &secret_input);

  xof = crypto_xof_new();
  /* Feed our KDF. [SHAKE it like a polaroid picture --Yawning]. */
  crypto_xof_add_bytes(xof, secret_input, secret_input_len);
  crypto_xof_add_bytes(xof, salt, salt_len);

  /* Feed in the right string constant based on the desc layer */
  if (is_superencrypted_layer) {
    crypto_xof_add_bytes(xof, (const uint8_t *) str_enc_const_superencryption,
                         strlen(str_enc_const_superencryption));
  } else {
    crypto_xof_add_bytes(xof, (const uint8_t *) str_enc_const_encryption,
                         strlen(str_enc_const_encryption));
  }

  /* Eat from our KDF. */
  crypto_xof_squeeze_bytes(xof, key_out, key_out_len);
  crypto_xof_free(xof);
  memwipe(secret_input,  0, secret_input_len);

  tor_free(secret_input);
}

/** Using the given descriptor, secret data, and salt, run it through our
 * KDF function and then extract a secret key in key_out, the IV in iv_out
 * and MAC in mac_out. This function can't fail. */
static void
build_secret_key_iv_mac(const hs_descriptor_t *desc,
                        const uint8_t *secret_data,
                        size_t secret_data_len,
                        const uint8_t *salt, size_t salt_len,
                        uint8_t *key_out, size_t key_len,
                        uint8_t *iv_out, size_t iv_len,
                        uint8_t *mac_out, size_t mac_len,
                        int is_superencrypted_layer)
{
  size_t offset = 0;
  uint8_t kdf_key[HS_DESC_ENCRYPTED_KDF_OUTPUT_LEN];

  tor_assert(desc);
  tor_assert(secret_data);
  tor_assert(salt);
  tor_assert(key_out);
  tor_assert(iv_out);
  tor_assert(mac_out);

  build_kdf_key(desc, secret_data, secret_data_len,
                salt, salt_len, kdf_key, sizeof(kdf_key),
                is_superencrypted_layer);
  /* Copy the bytes we need for both the secret key and IV. */
  memcpy(key_out, kdf_key, key_len);
  offset += key_len;
  memcpy(iv_out, kdf_key + offset, iv_len);
  offset += iv_len;
  memcpy(mac_out, kdf_key + offset, mac_len);
  /* Extra precaution to make sure we are not out of bound. */
  tor_assert((offset + mac_len) == sizeof(kdf_key));
  memwipe(kdf_key, 0, sizeof(kdf_key));
}

/* === ENCODING === */

/** Encode the given link specifier objects into a newly allocated string.
 * This can't fail so caller can always assume a valid string being
 * returned. */
STATIC char *
encode_link_specifiers(const smartlist_t *specs)
{
  char *encoded_b64 = NULL;
  link_specifier_list_t *lslist = link_specifier_list_new();

  tor_assert(specs);
  /* No link specifiers is a code flow error, can't happen. */
  tor_assert(smartlist_len(specs) > 0);
  tor_assert(smartlist_len(specs) <= UINT8_MAX);

  link_specifier_list_set_n_spec(lslist, smartlist_len(specs));

  SMARTLIST_FOREACH_BEGIN(specs, const link_specifier_t *,
                          spec) {
    link_specifier_t *ls = link_specifier_dup(spec);
    tor_assert(ls);
    link_specifier_list_add_spec(lslist, ls);
  } SMARTLIST_FOREACH_END(spec);

  {
    uint8_t *encoded;
    ssize_t encoded_len, encoded_b64_len, ret;

    encoded_len = link_specifier_list_encoded_len(lslist);
    tor_assert(encoded_len > 0);
    encoded = tor_malloc_zero(encoded_len);
    ret = link_specifier_list_encode(encoded, encoded_len, lslist);
    tor_assert(ret == encoded_len);

    /* Base64 encode our binary format. Add extra NUL byte for the base64
     * encoded value. */
    encoded_b64_len = base64_encode_size(encoded_len, 0) + 1;
    encoded_b64 = tor_malloc_zero(encoded_b64_len);
    ret = base64_encode(encoded_b64, encoded_b64_len, (const char *) encoded,
                        encoded_len, 0);
    tor_assert(ret == (encoded_b64_len - 1));
    tor_free(encoded);
  }

  link_specifier_list_free(lslist);
  return encoded_b64;
}

/** Encode an introduction point legacy key and certificate. Return a newly
 * allocated string with it. On failure, return NULL. */
static char *
encode_legacy_key(const hs_desc_intro_point_t *ip)
{
  char *key_str, b64_cert[256], *encoded = NULL;
  size_t key_str_len;

  tor_assert(ip);

  /* Encode cross cert. */
  if (base64_encode(b64_cert, sizeof(b64_cert),
                    (const char *) ip->legacy.cert.encoded,
                    ip->legacy.cert.len, BASE64_ENCODE_MULTILINE) < 0) {
    log_warn(LD_REND, "Unable to encode legacy crosscert.");
    goto done;
  }
  /* Convert the encryption key to PEM format NUL terminated. */
  if (crypto_pk_write_public_key_to_string(ip->legacy.key, &key_str,
                                           &key_str_len) < 0) {
    log_warn(LD_REND, "Unable to encode legacy encryption key.");
    goto done;
  }
  tor_asprintf(&encoded,
               "%s \n%s"  /* Newline is added by the call above. */
               "%s\n"
               "-----BEGIN CROSSCERT-----\n"
               "%s"
               "-----END CROSSCERT-----",
               str_ip_legacy_key, key_str,
               str_ip_legacy_key_cert, b64_cert);
  tor_free(key_str);

 done:
  return encoded;
}

/** Encode an introduction point encryption key and certificate. Return a newly
 * allocated string with it. On failure, return NULL. */
static char *
encode_enc_key(const hs_desc_intro_point_t *ip)
{
  char *encoded = NULL, *encoded_cert;
  char key_b64[CURVE25519_BASE64_PADDED_LEN + 1];

  tor_assert(ip);

  /* Base64 encode the encryption key for the "enc-key" field. */
  curve25519_public_to_base64(key_b64, &ip->enc_key, true);
  if (tor_cert_encode_ed22519(ip->enc_key_cert, &encoded_cert) < 0) {
    goto done;
  }
  tor_asprintf(&encoded,
               "%s ntor %s\n"
               "%s\n%s",
               str_ip_enc_key, key_b64,
               str_ip_enc_key_cert, encoded_cert);
  tor_free(encoded_cert);

 done:
  return encoded;
}

/** Encode an introduction point onion key. Return a newly allocated string
 * with it. Can not fail. */
static char *
encode_onion_key(const hs_desc_intro_point_t *ip)
{
  char *encoded = NULL;
  char key_b64[CURVE25519_BASE64_PADDED_LEN + 1];

  tor_assert(ip);

  /* Base64 encode the encryption key for the "onion-key" field. */
  curve25519_public_to_base64(key_b64, &ip->onion_key, true);
  tor_asprintf(&encoded, "%s ntor %s", str_ip_onion_key, key_b64);

  return encoded;
}

/** Encode an introduction point object and return a newly allocated string
 * with it. On failure, return NULL. */
static char *
encode_intro_point(const ed25519_public_key_t *sig_key,
                   const hs_desc_intro_point_t *ip)
{
  char *encoded_ip = NULL;
  smartlist_t *lines = smartlist_new();

  tor_assert(ip);
  tor_assert(sig_key);

  /* Encode link specifier. */
  {
    char *ls_str = encode_link_specifiers(ip->link_specifiers);
    smartlist_add_asprintf(lines, "%s %s", str_intro_point, ls_str);
    tor_free(ls_str);
  }

  /* Onion key encoding. */
  {
    char *encoded_onion_key = encode_onion_key(ip);
    if (encoded_onion_key == NULL) {
      goto err;
    }
    smartlist_add_asprintf(lines, "%s", encoded_onion_key);
    tor_free(encoded_onion_key);
  }

  /* Authentication key encoding. */
  {
    char *encoded_cert;
    if (tor_cert_encode_ed22519(ip->auth_key_cert, &encoded_cert) < 0) {
      goto err;
    }
    smartlist_add_asprintf(lines, "%s\n%s", str_ip_auth_key, encoded_cert);
    tor_free(encoded_cert);
  }

  /* Encryption key encoding. */
  {
    char *encoded_enc_key = encode_enc_key(ip);
    if (encoded_enc_key == NULL) {
      goto err;
    }
    smartlist_add_asprintf(lines, "%s", encoded_enc_key);
    tor_free(encoded_enc_key);
  }

  /* Legacy key if any. */
  if (ip->legacy.key != NULL) {
    /* Strong requirement else the IP creation was badly done. */
    tor_assert(ip->legacy.cert.encoded);
    char *encoded_legacy_key = encode_legacy_key(ip);
    if (encoded_legacy_key == NULL) {
      goto err;
    }
    smartlist_add_asprintf(lines, "%s", encoded_legacy_key);
    tor_free(encoded_legacy_key);
  }

  /* Join them all in one blob of text. */
  encoded_ip = smartlist_join_strings(lines, "\n", 1, NULL);

 err:
  SMARTLIST_FOREACH(lines, char *, l, tor_free(l));
  smartlist_free(lines);
  return encoded_ip;
}

/** Given a source length, return the new size including padding for the
 * plaintext encryption. */
static size_t
compute_padded_plaintext_length(size_t plaintext_len)
{
  size_t plaintext_padded_len;
  const int padding_block_length = HS_DESC_SUPERENC_PLAINTEXT_PAD_MULTIPLE;

  /* Make sure we won't overflow. */
  tor_assert(plaintext_len <= (SIZE_T_CEILING - padding_block_length));

  /* Get the extra length we need to add. For example, if srclen is 10200
   * bytes, this will expand to (2 * 10k) == 20k thus an extra 9800 bytes. */
  plaintext_padded_len = CEIL_DIV(plaintext_len, padding_block_length) *
                         padding_block_length;
  /* Can never be extra careful. Make sure we are _really_ padded. */
  tor_assert(!(plaintext_padded_len % padding_block_length));
  return plaintext_padded_len;
}

/** Given a buffer, pad it up to the encrypted section padding requirement. Set
 * the newly allocated string in padded_out and return the length of the
 * padded buffer. */
STATIC size_t
build_plaintext_padding(const char *plaintext, size_t plaintext_len,
                        uint8_t **padded_out)
{
  size_t padded_len;
  uint8_t *padded;

  tor_assert(plaintext);
  tor_assert(padded_out);

  /* Allocate the final length including padding. */
  padded_len = compute_padded_plaintext_length(plaintext_len);
  tor_assert(padded_len >= plaintext_len);
  padded = tor_malloc_zero(padded_len);

  memcpy(padded, plaintext, plaintext_len);
  *padded_out = padded;
  return padded_len;
}

/** Using a key, IV and plaintext data of length plaintext_len, create the
 * encrypted section by encrypting it and setting encrypted_out with the
 * data. Return size of the encrypted data buffer. */
static size_t
build_encrypted(const uint8_t *key, const uint8_t *iv, const char *plaintext,
                size_t plaintext_len, uint8_t **encrypted_out,
                int is_superencrypted_layer)
{
  size_t encrypted_len;
  uint8_t *padded_plaintext, *encrypted;
  crypto_cipher_t *cipher;

  tor_assert(key);
  tor_assert(iv);
  tor_assert(plaintext);
  tor_assert(encrypted_out);

  /* If we are encrypting the middle layer of the descriptor, we need to first
     pad the plaintext */
  if (is_superencrypted_layer) {
    encrypted_len = build_plaintext_padding(plaintext, plaintext_len,
                                            &padded_plaintext);
    /* Extra precautions that we have a valid padding length. */
    tor_assert(!(encrypted_len % HS_DESC_SUPERENC_PLAINTEXT_PAD_MULTIPLE));
  } else { /* No padding required for inner layers */
    padded_plaintext = tor_memdup(plaintext, plaintext_len);
    encrypted_len = plaintext_len;
  }

  /* This creates a cipher for AES. It can't fail. */
  cipher = crypto_cipher_new_with_iv_and_bits(key, iv,
                                              HS_DESC_ENCRYPTED_BIT_SIZE);
  /* We use a stream cipher so the encrypted length will be the same as the
   * plaintext padded length. */
  encrypted = tor_malloc_zero(encrypted_len);
  /* This can't fail. */
  crypto_cipher_encrypt(cipher, (char *) encrypted,
                        (const char *) padded_plaintext, encrypted_len);
  *encrypted_out = encrypted;
  /* Cleanup. */
  crypto_cipher_free(cipher);
  tor_free(padded_plaintext);
  return encrypted_len;
}

/** Encrypt the given <b>plaintext</b> buffer using <b>desc</b> and
 * <b>secret_data</b> to get the keys. Set encrypted_out with the encrypted
 * data and return the length of it. <b>is_superencrypted_layer</b> is set
 * if this is the outer encrypted layer of the descriptor. */
static size_t
encrypt_descriptor_data(const hs_descriptor_t *desc,
                        const uint8_t *secret_data,
                        size_t secret_data_len,
                        const char *plaintext,
                        char **encrypted_out, int is_superencrypted_layer)
{
  char *final_blob;
  size_t encrypted_len, final_blob_len, offset = 0;
  uint8_t *encrypted;
  uint8_t salt[HS_DESC_ENCRYPTED_SALT_LEN];
  uint8_t secret_key[HS_DESC_ENCRYPTED_KEY_LEN], secret_iv[CIPHER_IV_LEN];
  uint8_t mac_key[DIGEST256_LEN], mac[DIGEST256_LEN];

  tor_assert(desc);
  tor_assert(secret_data);
  tor_assert(plaintext);
  tor_assert(encrypted_out);

  /* Get our salt. The returned bytes are already hashed. */
  crypto_strongest_rand(salt, sizeof(salt));

  /* KDF construction resulting in a key from which the secret key, IV and MAC
   * key are extracted which is what we need for the encryption. */
  build_secret_key_iv_mac(desc, secret_data, secret_data_len,
                          salt, sizeof(salt),
                          secret_key, sizeof(secret_key),
                          secret_iv, sizeof(secret_iv),
                          mac_key, sizeof(mac_key),
                          is_superencrypted_layer);

  /* Build the encrypted part that is do the actual encryption. */
  encrypted_len = build_encrypted(secret_key, secret_iv, plaintext,
                                  strlen(plaintext), &encrypted,
                                  is_superencrypted_layer);
  memwipe(secret_key, 0, sizeof(secret_key));
  memwipe(secret_iv, 0, sizeof(secret_iv));
  /* This construction is specified in section 2.5 of proposal 224. */
  final_blob_len = sizeof(salt) + encrypted_len + DIGEST256_LEN;
  final_blob = tor_malloc_zero(final_blob_len);

  /* Build the MAC. */
  build_mac(mac_key, sizeof(mac_key), salt, sizeof(salt),
            encrypted, encrypted_len, mac, sizeof(mac));
  memwipe(mac_key, 0, sizeof(mac_key));

  /* The salt is the first value. */
  memcpy(final_blob, salt, sizeof(salt));
  offset = sizeof(salt);
  /* Second value is the encrypted data. */
  memcpy(final_blob + offset, encrypted, encrypted_len);
  offset += encrypted_len;
  /* Third value is the MAC. */
  memcpy(final_blob + offset, mac, sizeof(mac));
  offset += sizeof(mac);
  /* Cleanup the buffers. */
  memwipe(salt, 0, sizeof(salt));
  memwipe(encrypted, 0, encrypted_len);
  tor_free(encrypted);
  /* Extra precaution. */
  tor_assert(offset == final_blob_len);

  *encrypted_out = final_blob;
  return final_blob_len;
}

/** Create and return a string containing a client-auth entry. It's the
 * responsibility of the caller to free the returned string. This function
 * will never fail. */
static char *
get_auth_client_str(const hs_desc_authorized_client_t *client)
{
  int ret;
  char *auth_client_str = NULL;
  /* We are gonna fill these arrays with base64 data. They are all double
   * the size of their binary representation to fit the base64 overhead. */
  char client_id_b64[HS_DESC_CLIENT_ID_LEN * 2];
  char iv_b64[CIPHER_IV_LEN * 2];
  char encrypted_cookie_b64[HS_DESC_ENCRYPED_COOKIE_LEN * 2];

#define ASSERT_AND_BASE64(field) STMT_BEGIN                        \
  tor_assert(!fast_mem_is_zero((char *) client->field,              \
                              sizeof(client->field)));             \
  ret = base64_encode_nopad(field##_b64, sizeof(field##_b64),      \
                            client->field, sizeof(client->field)); \
  tor_assert(ret > 0);                                             \
  STMT_END

  ASSERT_AND_BASE64(client_id);
  ASSERT_AND_BASE64(iv);
  ASSERT_AND_BASE64(encrypted_cookie);

  /* Build the final string */
  tor_asprintf(&auth_client_str, "%s %s %s %s", str_desc_auth_client,
               client_id_b64, iv_b64, encrypted_cookie_b64);

#undef ASSERT_AND_BASE64

  return auth_client_str;
}

/** Create the "client-auth" part of the descriptor and return a
 *  newly-allocated string with it. It's the responsibility of the caller to
 *  free the returned string. */
static char *
get_all_auth_client_lines(const hs_descriptor_t *desc)
{
  smartlist_t *auth_client_lines = smartlist_new();
  char *auth_client_lines_str = NULL;

  tor_assert(desc);
  tor_assert(desc->superencrypted_data.clients);
  tor_assert(smartlist_len(desc->superencrypted_data.clients) != 0);
  tor_assert(smartlist_len(desc->superencrypted_data.clients)
                                 % HS_DESC_AUTH_CLIENT_MULTIPLE == 0);

  /* Make a line for each client */
  SMARTLIST_FOREACH_BEGIN(desc->superencrypted_data.clients,
                          const hs_desc_authorized_client_t *, client) {
    char *auth_client_str = NULL;

    auth_client_str = get_auth_client_str(client);

    smartlist_add(auth_client_lines, auth_client_str);
  } SMARTLIST_FOREACH_END(client);

  /* Join all lines together to form final string */
  auth_client_lines_str = smartlist_join_strings(auth_client_lines,
                                                 "\n", 1, NULL);
  /* Cleanup the mess */
  SMARTLIST_FOREACH(auth_client_lines, char *, a, tor_free(a));
  smartlist_free(auth_client_lines);

  return auth_client_lines_str;
}

/** Create the inner layer of the descriptor (which includes the intro points,
 * etc.). Return a newly-allocated string with the layer plaintext, or NULL if
 * an error occurred. It's the responsibility of the caller to free the
 * returned string. */
static char *
get_inner_encrypted_layer_plaintext(const hs_descriptor_t *desc)
{
  char *encoded_str = NULL;
  smartlist_t *lines = smartlist_new();

  /* Build the start of the section prior to the introduction points. */
  {
    if (!desc->encrypted_data.create2_ntor) {
      log_err(LD_BUG, "HS desc doesn't have recognized handshake type.");
      goto err;
    }
    smartlist_add_asprintf(lines, "%s %d\n", str_create2_formats,
                           ONION_HANDSHAKE_TYPE_NTOR);

#ifdef TOR_UNIT_TESTS
    if (desc->encrypted_data.test_extra_plaintext) {
      smartlist_add(lines,
                    tor_strdup(desc->encrypted_data.test_extra_plaintext));
    }
#endif

    if (desc->encrypted_data.intro_auth_types &&
        smartlist_len(desc->encrypted_data.intro_auth_types)) {
      /* Put the authentication-required line. */
      char *buf = smartlist_join_strings(desc->encrypted_data.intro_auth_types,
                                         " ", 0, NULL);
      smartlist_add_asprintf(lines, "%s %s\n", str_intro_auth_required, buf);
      tor_free(buf);
    }

    if (desc->encrypted_data.single_onion_service) {
      smartlist_add_asprintf(lines, "%s\n", str_single_onion);
    }

    if (congestion_control_enabled()) {
      /* Add flow control line into the descriptor. */
      smartlist_add_asprintf(lines, "%s %s %u\n", str_flow_control,
                             protover_get_supported(PRT_FLOWCTRL),
                             congestion_control_sendme_inc());
    }

    /* Add PoW parameters if present. */
    if (desc->encrypted_data.pow_params) {
      /* Base64 the seed */
      size_t seed_b64_len = base64_encode_size(HS_POW_SEED_LEN, 0) + 1;
      char *seed_b64 = tor_malloc_zero(seed_b64_len);
      int ret = base64_encode(seed_b64, seed_b64_len,
                              (char *)desc->encrypted_data.pow_params->seed,
                              HS_POW_SEED_LEN, 0);
      /* Return length doesn't count the NUL byte. */
      tor_assert((size_t) ret == (seed_b64_len - 1));

      /* Convert the expiration time to space-less ISO format. */
      char time_buf[ISO_TIME_LEN + 1];
      format_iso_time_nospace(time_buf,
                 desc->encrypted_data.pow_params->expiration_time);

      /* Add "pow-params" line to descriptor encoding. */
      smartlist_add_asprintf(lines, "%s %s %s %u %s\n", str_pow_params,
                pow_types[desc->encrypted_data.pow_params->type].identifier,
                seed_b64,
                desc->encrypted_data.pow_params->suggested_effort,
                time_buf);
      tor_free(seed_b64);
    }
  }

  /* Build the introduction point(s) section. */
  SMARTLIST_FOREACH_BEGIN(desc->encrypted_data.intro_points,
                          const hs_desc_intro_point_t *, ip) {
    char *encoded_ip = encode_intro_point(&desc->plaintext_data.signing_pubkey,
                                          ip);
    if (encoded_ip == NULL) {
      log_err(LD_BUG, "HS desc intro point is malformed.");
      goto err;
    }
    smartlist_add(lines, encoded_ip);
  } SMARTLIST_FOREACH_END(ip);

  /* Build the entire encrypted data section into one encoded plaintext and
   * then encrypt it. */
  encoded_str = smartlist_join_strings(lines, "", 0, NULL);

 err:
  SMARTLIST_FOREACH(lines, char *, l, tor_free(l));
  smartlist_free(lines);

  return encoded_str;
}

/** Create the middle layer of the descriptor, which includes the client auth
 * data and the encrypted inner layer (provided as a base64 string at
 * <b>layer2_b64_ciphertext</b>). Return a newly-allocated string with the
 * layer plaintext. It's the responsibility of the caller to free the returned
 * string. Can not fail. */
static char *
get_outer_encrypted_layer_plaintext(const hs_descriptor_t *desc,
                                    const char *layer2_b64_ciphertext)
{
  char *layer1_str = NULL;
  smartlist_t *lines = smartlist_new();

  /* Specify auth type */
  smartlist_add_asprintf(lines, "%s %s\n", str_desc_auth_type, "x25519");

  {  /* Print ephemeral x25519 key */
    char ephemeral_key_base64[CURVE25519_BASE64_PADDED_LEN + 1];
    const curve25519_public_key_t *ephemeral_pubkey;

    ephemeral_pubkey = &desc->superencrypted_data.auth_ephemeral_pubkey;
    tor_assert(!fast_mem_is_zero((char *) ephemeral_pubkey->public_key,
                                CURVE25519_PUBKEY_LEN));

    curve25519_public_to_base64(ephemeral_key_base64, ephemeral_pubkey, true);
    smartlist_add_asprintf(lines, "%s %s\n",
                           str_desc_auth_key, ephemeral_key_base64);

    memwipe(ephemeral_key_base64, 0, sizeof(ephemeral_key_base64));
  }

  {  /* Create auth-client lines. */
    char *auth_client_lines = get_all_auth_client_lines(desc);
    tor_assert(auth_client_lines);
    smartlist_add(lines, auth_client_lines);
  }

  /* create encrypted section */
  {
    smartlist_add_asprintf(lines,
                           "%s\n"
                           "-----BEGIN MESSAGE-----\n"
                           "%s"
                           "-----END MESSAGE-----",
                           str_encrypted, layer2_b64_ciphertext);
  }

  layer1_str = smartlist_join_strings(lines, "", 0, NULL);

  /* We need to memwipe all lines because it contains the ephemeral key */
  SMARTLIST_FOREACH(lines, char *, a, memwipe(a, 0, strlen(a)));
  SMARTLIST_FOREACH(lines, char *, a, tor_free(a));
  smartlist_free(lines);

  return layer1_str;
}

/** Encrypt <b>encoded_str</b> into an encrypted blob and then base64 it before
 * returning it. <b>desc</b> is provided to derive the encryption
 * keys. <b>secret_data</b> is also proved to derive the encryption keys.
 * <b>is_superencrypted_layer</b> is set if <b>encoded_str</b> is the
 * middle (superencrypted) layer of the descriptor. It's the responsibility of
 * the caller to free the returned string. */
static char *
encrypt_desc_data_and_base64(const hs_descriptor_t *desc,
                             const uint8_t *secret_data,
                             size_t secret_data_len,
                             const char *encoded_str,
                             int is_superencrypted_layer)
{
  char *enc_b64;
  ssize_t enc_b64_len, ret_len, enc_len;
  char *encrypted_blob = NULL;

  enc_len = encrypt_descriptor_data(desc, secret_data, secret_data_len,
                                    encoded_str, &encrypted_blob,
                                    is_superencrypted_layer);
  /* Get the encoded size plus a NUL terminating byte. */
  enc_b64_len = base64_encode_size(enc_len, BASE64_ENCODE_MULTILINE) + 1;
  enc_b64 = tor_malloc_zero(enc_b64_len);
  /* Base64 the encrypted blob before returning it. */
  ret_len = base64_encode(enc_b64, enc_b64_len, encrypted_blob, enc_len,
                          BASE64_ENCODE_MULTILINE);
  /* Return length doesn't count the NUL byte. */
  tor_assert(ret_len == (enc_b64_len - 1));
  tor_free(encrypted_blob);

  return enc_b64;
}

/** Generate the secret data which is used to encrypt/decrypt the descriptor.
 *
 * SECRET_DATA = blinded-public-key
 * SECRET_DATA = blinded-public-key | descriptor_cookie
 *
 * The descriptor_cookie is optional but if it exists, it must be at least
 * HS_DESC_DESCRIPTOR_COOKIE_LEN bytes long.
 *
 * A newly allocated secret data is put in secret_data_out. Return the
 * length of the secret data. This function cannot fail. */
static size_t
build_secret_data(const ed25519_public_key_t *blinded_pubkey,
                  const uint8_t *descriptor_cookie,
                  uint8_t **secret_data_out)
{
  size_t secret_data_len;
  uint8_t *secret_data;

  tor_assert(blinded_pubkey);
  tor_assert(secret_data_out);

  if (descriptor_cookie) {
    /* If the descriptor cookie is present, we need both the blinded
     * pubkey and the descriptor cookie as a secret data. */
    secret_data_len = ED25519_PUBKEY_LEN + HS_DESC_DESCRIPTOR_COOKIE_LEN;
    secret_data = tor_malloc(secret_data_len);

    memcpy(secret_data,
           blinded_pubkey->pubkey,
           ED25519_PUBKEY_LEN);
    memcpy(secret_data + ED25519_PUBKEY_LEN,
           descriptor_cookie,
           HS_DESC_DESCRIPTOR_COOKIE_LEN);
  } else {
    /* If the descriptor cookie is not present, we need only the blinded
     * pubkey as a secret data. */
    secret_data_len = ED25519_PUBKEY_LEN;
    secret_data = tor_malloc(secret_data_len);
    memcpy(secret_data,
           blinded_pubkey->pubkey,
           ED25519_PUBKEY_LEN);
  }

  *secret_data_out = secret_data;
  return secret_data_len;
}

/** Generate and encode the superencrypted portion of <b>desc</b>. This also
 * involves generating the encrypted portion of the descriptor, and performing
 * the superencryption. A newly allocated NUL-terminated string pointer
 * containing the encrypted encoded blob is put in encrypted_blob_out. Return 0
 * on success else a negative value. */
static int
encode_superencrypted_data(const hs_descriptor_t *desc,
                           const uint8_t *descriptor_cookie,
                           char **encrypted_blob_out)
{
  int ret = -1;
  uint8_t *secret_data = NULL;
  size_t secret_data_len = 0;
  char *layer2_str = NULL;
  char *layer2_b64_ciphertext = NULL;
  char *layer1_str = NULL;
  char *layer1_b64_ciphertext = NULL;

  tor_assert(desc);
  tor_assert(encrypted_blob_out);

  /* Func logic: We first create the inner layer of the descriptor (layer2).
   * We then encrypt it and use it to create the middle layer of the descriptor
   * (layer1).  Finally we superencrypt the middle layer and return it to our
   * caller. */

  /* Create inner descriptor layer */
  layer2_str = get_inner_encrypted_layer_plaintext(desc);
  if (!layer2_str) {
    goto err;
  }

  secret_data_len = build_secret_data(&desc->plaintext_data.blinded_pubkey,
                                      descriptor_cookie,
                                      &secret_data);

  /* Encrypt and b64 the inner layer */
  layer2_b64_ciphertext =
    encrypt_desc_data_and_base64(desc, secret_data, secret_data_len,
                                 layer2_str, 0);
  if (!layer2_b64_ciphertext) {
    goto err;
  }

  /* Now create middle descriptor layer given the inner layer */
  layer1_str = get_outer_encrypted_layer_plaintext(desc,layer2_b64_ciphertext);
  if (!layer1_str) {
    goto err;
  }

  /* Encrypt and base64 the middle layer */
  layer1_b64_ciphertext =
    encrypt_desc_data_and_base64(desc,
                                 desc->plaintext_data.blinded_pubkey.pubkey,
                                 ED25519_PUBKEY_LEN,
                                 layer1_str, 1);
  if (!layer1_b64_ciphertext) {
    goto err;
  }

  /* Success! */
  ret = 0;

 err:
  memwipe(secret_data, 0, secret_data_len);
  tor_free(secret_data);
  tor_free(layer1_str);
  tor_free(layer2_str);
  tor_free(layer2_b64_ciphertext);

  *encrypted_blob_out = layer1_b64_ciphertext;
  return ret;
}

/** Encode a v3 HS descriptor. Return 0 on success and set encoded_out to the
 * newly allocated string of the encoded descriptor. On error, -1 is returned
 * and encoded_out is untouched. */
static int
desc_encode_v3(const hs_descriptor_t *desc,
               const ed25519_keypair_t *signing_kp,
               const uint8_t *descriptor_cookie,
               char **encoded_out)
{
  int ret = -1;
  char *encoded_str = NULL;
  size_t encoded_len;
  smartlist_t *lines = smartlist_new();

  tor_assert(desc);
  tor_assert(signing_kp);
  tor_assert(encoded_out);
  tor_assert(desc->plaintext_data.version == 3);

  /* Build the non-encrypted values. */
  {
    char *encoded_cert;
    /* Encode certificate then create the first line of the descriptor. */
    if (desc->plaintext_data.signing_key_cert->cert_type
        != CERT_TYPE_SIGNING_HS_DESC) {
      log_err(LD_BUG, "HS descriptor signing key has an unexpected cert type "
              "(%d)", (int) desc->plaintext_data.signing_key_cert->cert_type);
      goto err;
    }
    if (tor_cert_encode_ed22519(desc->plaintext_data.signing_key_cert,
                                &encoded_cert) < 0) {
      /* The function will print error logs. */
      goto err;
    }
    /* Create the hs descriptor line. */
    smartlist_add_asprintf(lines, "%s %" PRIu32, str_hs_desc,
                           desc->plaintext_data.version);
    /* Add the descriptor lifetime line (in minutes). */
    smartlist_add_asprintf(lines, "%s %" PRIu32, str_lifetime,
                           desc->plaintext_data.lifetime_sec / 60);
    /* Create the descriptor certificate line. */
    smartlist_add_asprintf(lines, "%s\n%s", str_desc_cert, encoded_cert);
    tor_free(encoded_cert);
    /* Create the revision counter line. */
    smartlist_add_asprintf(lines, "%s %" PRIu64, str_rev_counter,
                           desc->plaintext_data.revision_counter);
  }

  /* Build the superencrypted data section. */
  {
    char *enc_b64_blob=NULL;
    if (encode_superencrypted_data(desc, descriptor_cookie,
                                   &enc_b64_blob) < 0) {
      goto err;
    }
    smartlist_add_asprintf(lines,
                           "%s\n"
                           "-----BEGIN MESSAGE-----\n"
                           "%s"
                           "-----END MESSAGE-----",
                           str_superencrypted, enc_b64_blob);
    tor_free(enc_b64_blob);
  }

  /* Join all lines in one string so we can generate a signature and append
   * it to the descriptor. */
  encoded_str = smartlist_join_strings(lines, "\n", 1, &encoded_len);

  /* Sign all fields of the descriptor with our short term signing key. */
  {
    ed25519_signature_t sig;
    char ed_sig_b64[ED25519_SIG_BASE64_LEN + 1];
    if (ed25519_sign_prefixed(&sig,
                              (const uint8_t *) encoded_str, encoded_len,
                              str_desc_sig_prefix, signing_kp) < 0) {
      log_warn(LD_BUG, "Can't sign encoded HS descriptor!");
      tor_free(encoded_str);
      goto err;
    }
    ed25519_signature_to_base64(ed_sig_b64, &sig);
    /* Create the signature line. */
    smartlist_add_asprintf(lines, "%s %s", str_signature, ed_sig_b64);
  }
  /* Free previous string that we used so compute the signature. */
  tor_free(encoded_str);
  encoded_str = smartlist_join_strings(lines, "\n", 1, NULL);
  *encoded_out = encoded_str;

  if (strlen(encoded_str) >= hs_cache_get_max_descriptor_size()) {
    log_warn(LD_GENERAL, "We just made an HS descriptor that's too big (%d)."
             "Failing.", (int)strlen(encoded_str));
    tor_free(encoded_str);
    goto err;
  }

  /* XXX: Trigger a control port event. */

  /* Success! */
  ret = 0;

 err:
  SMARTLIST_FOREACH(lines, char *, l, tor_free(l));
  smartlist_free(lines);
  return ret;
}

/* === DECODING === */

/** Given the token tok for an auth client, decode it as
 * hs_desc_authorized_client_t. tok->args MUST contain at least 3 elements
 * Return 0 on success else -1 on failure. */
static int
decode_auth_client(const directory_token_t *tok,
                   hs_desc_authorized_client_t *client)
{
  int ret = -1;

  tor_assert(tok);
  tor_assert(tok->n_args >= 3);
  tor_assert(client);

  if (base64_decode((char *) client->client_id, sizeof(client->client_id),
                    tok->args[0], strlen(tok->args[0])) !=
      sizeof(client->client_id)) {
    goto done;
  }
  if (base64_decode((char *) client->iv, sizeof(client->iv),
                    tok->args[1], strlen(tok->args[1])) !=
      sizeof(client->iv)) {
    goto done;
  }
  if (base64_decode((char *) client->encrypted_cookie,
                    sizeof(client->encrypted_cookie),
                    tok->args[2], strlen(tok->args[2])) !=
      sizeof(client->encrypted_cookie)) {
    goto done;
  }

  /* Success. */
  ret = 0;
 done:
  return ret;
}

/** Given an encoded string of the link specifiers, return a newly allocated
 * list of decoded link specifiers. Return NULL on error. */
STATIC smartlist_t *
decode_link_specifiers(const char *encoded)
{
  int decoded_len;
  size_t encoded_len, i;
  uint8_t *decoded;
  smartlist_t *results = NULL;
  link_specifier_list_t *specs = NULL;

  tor_assert(encoded);

  encoded_len = strlen(encoded);
  decoded = tor_malloc(encoded_len);
  decoded_len = base64_decode((char *) decoded, encoded_len, encoded,
                              encoded_len);
  if (decoded_len < 0) {
    goto err;
  }

  if (link_specifier_list_parse(&specs, decoded,
                                (size_t) decoded_len) < decoded_len) {
    goto err;
  }
  tor_assert(specs);
  results = smartlist_new();

  for (i = 0; i < link_specifier_list_getlen_spec(specs); i++) {
    link_specifier_t *ls = link_specifier_list_get_spec(specs, i);
    if (BUG(!ls)) {
      goto err;
    }
    link_specifier_t *ls_dup = link_specifier_dup(ls);
    if (BUG(!ls_dup)) {
      goto err;
    }
    smartlist_add(results, ls_dup);
  }

  goto done;
 err:
  if (results) {
    SMARTLIST_FOREACH(results, link_specifier_t *, s,
                      link_specifier_free(s));
    smartlist_free(results);
    results = NULL;
  }
 done:
  link_specifier_list_free(specs);
  tor_free(decoded);
  return results;
}

/** Given a list of authentication types, decode it and put it in the encrypted
 * data section. Return 1 if we at least know one of the type or 0 if we know
 * none of them. */
static int
decode_auth_type(hs_desc_encrypted_data_t *desc, const char *list)
{
  int match = 0;

  tor_assert(desc);
  tor_assert(list);

  desc->intro_auth_types = smartlist_new();
  smartlist_split_string(desc->intro_auth_types, list, " ", 0, 0);

  /* Validate the types that we at least know about one. */
  SMARTLIST_FOREACH_BEGIN(desc->intro_auth_types, const char *, auth) {
    for (int idx = 0; intro_auth_types[idx].identifier; idx++) {
      if (!strncmp(auth, intro_auth_types[idx].identifier,
                   strlen(intro_auth_types[idx].identifier))) {
        match = 1;
        break;
      }
    }
  } SMARTLIST_FOREACH_END(auth);

  return match;
}

/** Parse a space-delimited list of integers representing CREATE2 formats into
 * the bitfield in hs_desc_encrypted_data_t. Ignore unrecognized values. */
static void
decode_create2_list(hs_desc_encrypted_data_t *desc, const char *list)
{
  smartlist_t *tokens;

  tor_assert(desc);
  tor_assert(list);

  tokens = smartlist_new();
  smartlist_split_string(tokens, list, " ", 0, 0);

  SMARTLIST_FOREACH_BEGIN(tokens, char *, s) {
    int ok;
    unsigned long type = tor_parse_ulong(s, 10, 1, UINT16_MAX, &ok, NULL);
    if (!ok) {
      log_warn(LD_REND, "Unparseable value %s in create2 list", escaped(s));
      continue;
    }
    switch (type) {
    case ONION_HANDSHAKE_TYPE_NTOR:
      desc->create2_ntor = 1;
      break;
    default:
      /* We deliberately ignore unsupported handshake types */
      continue;
    }
  } SMARTLIST_FOREACH_END(s);

  SMARTLIST_FOREACH(tokens, char *, s, tor_free(s));
  smartlist_free(tokens);
}

/** Given a certificate, validate the certificate for certain conditions which
 * are if the given type matches the cert's one, if the signing key is
 * included and if the that key was actually used to sign the certificate.
 *
 * Return 1 iff if all conditions pass or 0 if one of them fails. */
STATIC int
cert_is_valid(tor_cert_t *cert, uint8_t type, const char *log_obj_type)
{
  tor_assert(log_obj_type);

  if (cert == NULL) {
    log_warn(LD_REND, "Certificate for %s couldn't be parsed.", log_obj_type);
    goto err;
  }
  if (cert->cert_type != type) {
    log_warn(LD_REND, "Invalid cert type %02x for %s.", cert->cert_type,
             log_obj_type);
    goto err;
  }
  /* All certificate must have its signing key included. */
  if (!cert->signing_key_included) {
    log_warn(LD_REND, "Signing key is NOT included for %s.", log_obj_type);
    goto err;
  }

  /* The following will not only check if the signature matches but also the
   * expiration date and overall validity. */
  if (tor_cert_checksig(cert, &cert->signing_key, approx_time()) < 0) {
    if (cert->cert_expired) {
      char expiration_str[ISO_TIME_LEN+1];
      format_iso_time(expiration_str, cert->valid_until);
      log_fn(LOG_PROTOCOL_WARN, LD_REND, "Invalid signature for %s: %s (%s)",
             log_obj_type, tor_cert_describe_signature_status(cert),
             expiration_str);
    } else {
      log_warn(LD_REND, "Invalid signature for %s: %s",
               log_obj_type, tor_cert_describe_signature_status(cert));
    }
    goto err;
  }

  return 1;
 err:
  return 0;
}

/** Given some binary data, try to parse it to get a certificate object. If we
 * have a valid cert, validate it using the given wanted type. On error, print
 * a log using the err_msg has the certificate identifier adding semantic to
 * the log and cert_out is set to NULL. On success, 0 is returned and cert_out
 * points to a newly allocated certificate object. */
static int
cert_parse_and_validate(tor_cert_t **cert_out, const char *data,
                        size_t data_len, unsigned int cert_type_wanted,
                        const char *err_msg)
{
  tor_cert_t *cert;

  tor_assert(cert_out);
  tor_assert(data);
  tor_assert(err_msg);

  /* Parse certificate. */
  cert = tor_cert_parse((const uint8_t *) data, data_len);
  if (!cert) {
    log_warn(LD_REND, "Certificate for %s couldn't be parsed.", err_msg);
    goto err;
  }

  /* Validate certificate. */
  if (!cert_is_valid(cert, cert_type_wanted, err_msg)) {
    goto err;
  }

  *cert_out = cert;
  return 0;

 err:
  tor_cert_free(cert);
  *cert_out = NULL;
  return -1;
}

/** Return true iff the given length of the encrypted data of a descriptor
 * passes validation. */
STATIC int
encrypted_data_length_is_valid(size_t len)
{
  /* Make sure there is enough data for the salt and the mac. The equality is
     there to ensure that there is at least one byte of encrypted data. */
  if (len <= HS_DESC_ENCRYPTED_SALT_LEN + DIGEST256_LEN) {
    log_warn(LD_REND, "Length of descriptor's encrypted data is too small. "
                      "Got %lu but minimum value is %d",
             (unsigned long)len, HS_DESC_ENCRYPTED_SALT_LEN + DIGEST256_LEN);
    goto err;
  }

  return 1;
 err:
  return 0;
}

/** Build the KEYS component for the authorized client computation. The format
 * of the construction is:
 *
 *    SECRET_SEED = x25519(sk, pk)
 *    KEYS = KDF(subcredential | SECRET_SEED, 40)
 *
 * Set the <b>keys_out</b> argument to point to the buffer containing the KEYS,
 * and return the buffer's length. The caller should wipe and free its content
 * once done with it. This function can't fail. */
static size_t
build_descriptor_cookie_keys(const hs_subcredential_t *subcredential,
                             const curve25519_secret_key_t *sk,
                             const curve25519_public_key_t *pk,
                             uint8_t **keys_out)
{
  uint8_t secret_seed[CURVE25519_OUTPUT_LEN];
  uint8_t *keystream;
  size_t keystream_len = HS_DESC_CLIENT_ID_LEN + HS_DESC_COOKIE_KEY_LEN;
  crypto_xof_t *xof;

  tor_assert(subcredential);
  tor_assert(sk);
  tor_assert(pk);
  tor_assert(keys_out);

  keystream = tor_malloc_zero(keystream_len);

  /* Calculate x25519(sk, pk) to get the secret seed. */
  curve25519_handshake(secret_seed, sk, pk);

  /* Calculate KEYS = KDF(subcredential | SECRET_SEED, 40) */
  xof = crypto_xof_new();
  crypto_xof_add_bytes(xof, subcredential->subcred, SUBCRED_LEN);
  crypto_xof_add_bytes(xof, secret_seed, sizeof(secret_seed));
  crypto_xof_squeeze_bytes(xof, keystream, keystream_len);
  crypto_xof_free(xof);

  memwipe(secret_seed, 0, sizeof(secret_seed));

  *keys_out = keystream;
  return keystream_len;
}

/** Decrypt the descriptor cookie given the descriptor, the auth client,
 * and the client secret key. On success, return 0 and a newly allocated
 * descriptor cookie descriptor_cookie_out. On error or if the client id
 * is invalid, return -1 and descriptor_cookie_out is set to
 * NULL. */
static int
decrypt_descriptor_cookie(const hs_descriptor_t *desc,
                          const hs_desc_authorized_client_t *client,
                          const curve25519_secret_key_t *client_auth_sk,
                          uint8_t **descriptor_cookie_out)
{
  int ret = -1;
  uint8_t *keystream = NULL;
  size_t keystream_length = 0;
  uint8_t *descriptor_cookie = NULL;
  const uint8_t *cookie_key = NULL;
  crypto_cipher_t *cipher = NULL;

  tor_assert(desc);
  tor_assert(client);
  tor_assert(client_auth_sk);
  tor_assert(!fast_mem_is_zero(
        (char *) &desc->superencrypted_data.auth_ephemeral_pubkey,
        sizeof(desc->superencrypted_data.auth_ephemeral_pubkey)));
  tor_assert(!fast_mem_is_zero((char *) desc->subcredential.subcred,
                               DIGEST256_LEN));

  /* Catch potential code-flow cases of an uninitialized private key sneaking
   * into this function. */
  if (BUG(fast_mem_is_zero((char *)client_auth_sk, sizeof(*client_auth_sk)))) {
    goto done;
  }

  /* Get the KEYS component to derive the CLIENT-ID and COOKIE-KEY. */
  keystream_length =
    build_descriptor_cookie_keys(&desc->subcredential,
                             client_auth_sk,
                             &desc->superencrypted_data.auth_ephemeral_pubkey,
                             &keystream);
  tor_assert(keystream_length > 0);

  /* If the client id of auth client is not the same as the calculcated
   * client id, it means that this auth client is invalid according to the
   * client secret key client_auth_sk. */
  if (tor_memneq(client->client_id, keystream, HS_DESC_CLIENT_ID_LEN)) {
    goto done;
  }
  cookie_key = keystream + HS_DESC_CLIENT_ID_LEN;

  /* This creates a cipher for AES. It can't fail. */
  cipher = crypto_cipher_new_with_iv_and_bits(cookie_key, client->iv,
                                              HS_DESC_COOKIE_KEY_BIT_SIZE);
  descriptor_cookie = tor_malloc_zero(HS_DESC_DESCRIPTOR_COOKIE_LEN);
  /* This can't fail. */
  crypto_cipher_decrypt(cipher, (char *) descriptor_cookie,
                        (const char *) client->encrypted_cookie,
                        sizeof(client->encrypted_cookie));

  /* Success. */
  ret = 0;
 done:
  *descriptor_cookie_out = descriptor_cookie;
  if (cipher) {
    crypto_cipher_free(cipher);
  }
  memwipe(keystream, 0, keystream_length);
  tor_free(keystream);
  return ret;
}

/** Decrypt an encrypted descriptor layer at <b>encrypted_blob</b> of size
 *  <b>encrypted_blob_size</b>. The descriptor cookie is optional. Use
 *  the descriptor object <b>desc</b> and <b>descriptor_cookie</b>
 *  to generate the right decryption keys; set <b>decrypted_out</b> to
 *  the plaintext. If <b>is_superencrypted_layer</b> is set, this is
 *  the outer encrypted layer of the descriptor.
 *
 * On any error case, including an empty output, return 0 and set
 * *<b>decrypted_out</b> to NULL.
 */
MOCK_IMPL(STATIC size_t,
decrypt_desc_layer,(const hs_descriptor_t *desc,
                    const uint8_t *descriptor_cookie,
                    bool is_superencrypted_layer,
                    char **decrypted_out))
{
  uint8_t *decrypted = NULL;
  uint8_t secret_key[HS_DESC_ENCRYPTED_KEY_LEN], secret_iv[CIPHER_IV_LEN];
  uint8_t *secret_data = NULL;
  size_t secret_data_len = 0;
  uint8_t mac_key[DIGEST256_LEN], our_mac[DIGEST256_LEN];
  const uint8_t *salt, *encrypted, *desc_mac;
  size_t encrypted_len, result_len = 0;
  const uint8_t *encrypted_blob = (is_superencrypted_layer)
    ? desc->plaintext_data.superencrypted_blob
    : desc->superencrypted_data.encrypted_blob;
  size_t encrypted_blob_size = (is_superencrypted_layer)
    ? desc->plaintext_data.superencrypted_blob_size
    : desc->superencrypted_data.encrypted_blob_size;

  tor_assert(decrypted_out);
  tor_assert(desc);
  tor_assert(encrypted_blob);

  /* Construction is as follow: SALT | ENCRYPTED_DATA | MAC .
   * Make sure we have enough space for all these things. */
  if (!encrypted_data_length_is_valid(encrypted_blob_size)) {
    goto err;
  }

  /* Start of the blob thus the salt. */
  salt = encrypted_blob;

  /* Next is the encrypted data. */
  encrypted = encrypted_blob + HS_DESC_ENCRYPTED_SALT_LEN;
  encrypted_len = encrypted_blob_size -
    (HS_DESC_ENCRYPTED_SALT_LEN + DIGEST256_LEN);
  tor_assert(encrypted_len > 0); /* guaranteed by the check above */

  /* And last comes the MAC. */
  desc_mac = encrypted_blob + encrypted_blob_size - DIGEST256_LEN;

  /* Build secret data to be used in the decryption. */
  secret_data_len = build_secret_data(&desc->plaintext_data.blinded_pubkey,
                                      descriptor_cookie,
                                      &secret_data);

  /* KDF construction resulting in a key from which the secret key, IV and MAC
   * key are extracted which is what we need for the decryption. */
  build_secret_key_iv_mac(desc, secret_data, secret_data_len,
                          salt, HS_DESC_ENCRYPTED_SALT_LEN,
                          secret_key, sizeof(secret_key),
                          secret_iv, sizeof(secret_iv),
                          mac_key, sizeof(mac_key),
                          is_superencrypted_layer);

  /* Build MAC. */
  build_mac(mac_key, sizeof(mac_key), salt, HS_DESC_ENCRYPTED_SALT_LEN,
            encrypted, encrypted_len, our_mac, sizeof(our_mac));
  memwipe(mac_key, 0, sizeof(mac_key));
  /* Verify MAC; MAC is H(mac_key || salt || encrypted)
   *
   * This is a critical check that is making sure the computed MAC matches the
   * one in the descriptor. */
  if (!tor_memeq(our_mac, desc_mac, sizeof(our_mac))) {
    log_info(LD_REND, "Encrypted service descriptor MAC check failed");
    goto err;
  }

  {
    /* Decrypt. Here we are assured that the encrypted length is valid for
     * decryption. */
    crypto_cipher_t *cipher;

    cipher = crypto_cipher_new_with_iv_and_bits(secret_key, secret_iv,
                                                HS_DESC_ENCRYPTED_BIT_SIZE);
    /* Extra byte for the NUL terminated byte. */
    decrypted = tor_malloc_zero(encrypted_len + 1);
    crypto_cipher_decrypt(cipher, (char *) decrypted,
                          (const char *) encrypted, encrypted_len);
    crypto_cipher_free(cipher);
  }

  {
    /* Adjust length to remove NUL padding bytes */
    uint8_t *end = memchr(decrypted, 0, encrypted_len);
    result_len = encrypted_len;
    if (end) {
      result_len = end - decrypted;
    }
  }

  if (result_len == 0) {
    /* Treat this as an error, so that somebody will free the output. */
    goto err;
  }

  /* Make sure to NUL terminate the string. */
  decrypted[encrypted_len] = '\0';
  *decrypted_out = (char *) decrypted;
  goto done;

 err:
  if (decrypted) {
    tor_free(decrypted);
  }
  *decrypted_out = NULL;
  result_len = 0;

 done:
  memwipe(secret_data, 0, secret_data_len);
  memwipe(secret_key, 0, sizeof(secret_key));
  memwipe(secret_iv, 0, sizeof(secret_iv));
  tor_free(secret_data);
  return result_len;
}

/** Decrypt the superencrypted section of the descriptor using the given
 * descriptor object <b>desc</b>. A newly allocated NUL terminated string is
 * put in decrypted_out which contains the superencrypted layer of the
 * descriptor. Return the length of decrypted_out on success else 0 is
 * returned and decrypted_out is set to NULL. */
MOCK_IMPL(STATIC size_t,
desc_decrypt_superencrypted,(const hs_descriptor_t *desc,char **decrypted_out))
{
  size_t superencrypted_len = 0;
  char *superencrypted_plaintext = NULL;

  tor_assert(desc);
  tor_assert(decrypted_out);

  superencrypted_len = decrypt_desc_layer(desc,
                                          NULL,
                                          true, &superencrypted_plaintext);

  if (!superencrypted_len) {
    log_warn(LD_REND, "Decrypting superencrypted desc failed.");
    goto done;
  }
  tor_assert(superencrypted_plaintext);

 done:
  /* In case of error, superencrypted_plaintext is already NULL, so the
   * following line makes sense. */
  *decrypted_out = superencrypted_plaintext;
  /* This makes sense too, because, in case of error, this is zero. */
  return superencrypted_len;
}

/** Decrypt the encrypted section of the descriptor using the given descriptor
 * object <b>desc</b>. A newly allocated NUL terminated string is put in
 * decrypted_out which contains the encrypted layer of the descriptor.
 * Return the length of decrypted_out on success else 0 is returned and
 * decrypted_out is set to NULL. */
MOCK_IMPL(STATIC size_t,
desc_decrypt_encrypted,(const hs_descriptor_t *desc,
                        const curve25519_secret_key_t *client_auth_sk,
                        char **decrypted_out))
{
  size_t encrypted_len = 0;
  char *encrypted_plaintext = NULL;
  uint8_t *descriptor_cookie = NULL;

  tor_assert(desc);
  tor_assert(desc->superencrypted_data.clients);
  tor_assert(decrypted_out);

  /* If the client secret key is provided, try to find a valid descriptor
   * cookie. Otherwise, leave it NULL. */
  if (client_auth_sk) {
    SMARTLIST_FOREACH_BEGIN(desc->superencrypted_data.clients,
                            hs_desc_authorized_client_t *, client) {
      /* If we can decrypt the descriptor cookie successfully, we will use that
       * descriptor cookie and break from the loop. */
      if (!decrypt_descriptor_cookie(desc, client, client_auth_sk,
                                     &descriptor_cookie)) {
        break;
      }
    } SMARTLIST_FOREACH_END(client);
  }

  encrypted_len = decrypt_desc_layer(desc,
                                     descriptor_cookie,
                                     false, &encrypted_plaintext);

  if (!encrypted_len) {
    goto err;
  }
  tor_assert(encrypted_plaintext);

 err:
  /* In case of error, encrypted_plaintext is already NULL, so the
   * following line makes sense. */
  *decrypted_out = encrypted_plaintext;
  if (descriptor_cookie) {
    memwipe(descriptor_cookie, 0, HS_DESC_DESCRIPTOR_COOKIE_LEN);
  }
  tor_free(descriptor_cookie);
  /* This makes sense too, because, in case of error, this is zero. */
  return encrypted_len;
}

/** Given the token tok for an intro point legacy key, the list of tokens, the
 * introduction point ip being decoded and the descriptor desc from which it
 * comes from, decode the legacy key and set the intro point object. Return 0
 * on success else -1 on failure. */
static int
decode_intro_legacy_key(const directory_token_t *tok,
                        smartlist_t *tokens,
                        hs_desc_intro_point_t *ip,
                        const hs_descriptor_t *desc)
{
  tor_assert(tok);
  tor_assert(tokens);
  tor_assert(ip);
  tor_assert(desc);

  if (!crypto_pk_public_exponent_ok(tok->key)) {
    log_warn(LD_REND, "Introduction point legacy key is invalid");
    goto err;
  }
  ip->legacy.key = crypto_pk_dup_key(tok->key);
  /* Extract the legacy cross certification cert which MUST be present if we
   * have a legacy key. */
  tok = find_opt_by_keyword(tokens, R3_INTRO_LEGACY_KEY_CERT);
  if (!tok) {
    log_warn(LD_REND, "Introduction point legacy key cert is missing");
    goto err;
  }
  tor_assert(tok->object_body);
  if (strcmp(tok->object_type, "CROSSCERT")) {
    /* Info level because this might be an unknown field that we should
     * ignore. */
    log_info(LD_REND, "Introduction point legacy encryption key "
                      "cross-certification has an unknown format.");
    goto err;
  }
  /* Keep a copy of the certificate. */
  ip->legacy.cert.encoded = tor_memdup(tok->object_body, tok->object_size);
  ip->legacy.cert.len = tok->object_size;
  /* The check on the expiration date is for the entire lifetime of a
   * certificate which is 24 hours. However, a descriptor has a maximum
   * lifetime of 12 hours meaning we have a 12h difference between the two
   * which ultimately accommodate the clock skewed client. */
  if (rsa_ed25519_crosscert_check(ip->legacy.cert.encoded,
                                  ip->legacy.cert.len, ip->legacy.key,
                                  &desc->plaintext_data.signing_pubkey,
                                  approx_time() - HS_DESC_CERT_LIFETIME)) {
    log_warn(LD_REND, "Unable to check cross-certification on the "
                      "introduction point legacy encryption key.");
    ip->cross_certified = 0;
    goto err;
  }

  /* Success. */
  return 0;
 err:
  return -1;
}

/** Dig into the descriptor <b>tokens</b> to find the onion key we should use
 * for this intro point, and set it into <b>onion_key_out</b>. Return 0 if it
 * was found and well-formed, otherwise return -1 in case of errors. */
static int
set_intro_point_onion_key(curve25519_public_key_t *onion_key_out,
                          const smartlist_t *tokens)
{
  int retval = -1;
  smartlist_t *onion_keys = NULL;

  tor_assert(onion_key_out);

  onion_keys = find_all_by_keyword(tokens, R3_INTRO_ONION_KEY);
  if (!onion_keys) {
    log_warn(LD_REND, "Descriptor did not contain intro onion keys");
    goto err;
  }

  SMARTLIST_FOREACH_BEGIN(onion_keys, directory_token_t *, tok) {
    /* This field is using GE(2) so for possible forward compatibility, we
     * accept more fields but must be at least 2. */
    tor_assert(tok->n_args >= 2);

    /* Try to find an ntor key, it's the only recognized type right now */
    if (!strcmp(tok->args[0], "ntor")) {
      if (curve25519_public_from_base64(onion_key_out, tok->args[1]) < 0) {
        log_warn(LD_REND, "Introduction point ntor onion-key is invalid");
        goto err;
      }
      /* Got the onion key! Set the appropriate retval */
      retval = 0;
    }
  } SMARTLIST_FOREACH_END(tok);

  /* Log an error if we didn't find it :( */
  if (retval < 0) {
    log_warn(LD_REND, "Descriptor did not contain ntor onion keys");
  }

 err:
  smartlist_free(onion_keys);
  return retval;
}

/** Given the start of a section and the end of it, decode a single
 * introduction point from that section. Return a newly allocated introduction
 * point object containing the decoded data. Return NULL if the section can't
 * be decoded. */
STATIC hs_desc_intro_point_t *
decode_introduction_point(const hs_descriptor_t *desc, const char *start)
{
  hs_desc_intro_point_t *ip = NULL;
  memarea_t *area = NULL;
  smartlist_t *tokens = NULL;
  const directory_token_t *tok;

  tor_assert(desc);
  tor_assert(start);

  area = memarea_new();
  tokens = smartlist_new();
  if (tokenize_string(area, start, start + strlen(start),
                      tokens, hs_desc_intro_point_v3_token_table, 0) < 0) {
    log_warn(LD_REND, "Introduction point is not parseable");
    goto err;
  }

  /* Ok we seem to have a well formed section containing enough tokens to
   * parse. Allocate our IP object and try to populate it. */
  ip = hs_desc_intro_point_new();

  /* "introduction-point" SP link-specifiers NL */
  tok = find_by_keyword(tokens, R3_INTRODUCTION_POINT);
  tor_assert(tok->n_args == 1);
  /* Our constructor creates this list by default so free it. */
  smartlist_free(ip->link_specifiers);
  ip->link_specifiers = decode_link_specifiers(tok->args[0]);
  if (!ip->link_specifiers) {
    log_warn(LD_REND, "Introduction point has invalid link specifiers");
    goto err;
  }

  /* "onion-key" SP ntor SP key NL */
  if (set_intro_point_onion_key(&ip->onion_key, tokens) < 0) {
    goto err;
  }

  /* "auth-key" NL certificate NL */
  tok = find_by_keyword(tokens, R3_INTRO_AUTH_KEY);
  tor_assert(tok->object_body);
  if (strcmp(tok->object_type, "ED25519 CERT")) {
    log_warn(LD_REND, "Unexpected object type for introduction auth key");
    goto err;
  }
  /* Parse cert and do some validation. */
  if (cert_parse_and_validate(&ip->auth_key_cert, tok->object_body,
                              tok->object_size, CERT_TYPE_AUTH_HS_IP_KEY,
                              "introduction point auth-key") < 0) {
    goto err;
  }
  /* Validate authentication certificate with descriptor signing key. */
  if (tor_cert_checksig(ip->auth_key_cert,
                        &desc->plaintext_data.signing_pubkey, 0) < 0) {
    log_warn(LD_REND, "Invalid authentication key signature: %s",
             tor_cert_describe_signature_status(ip->auth_key_cert));
    goto err;
  }

  /* Exactly one "enc-key" SP "ntor" SP key NL */
  tok = find_by_keyword(tokens, R3_INTRO_ENC_KEY);
  if (!strcmp(tok->args[0], "ntor")) {
    /* This field is using GE(2) so for possible forward compatibility, we
     * accept more fields but must be at least 2. */
    tor_assert(tok->n_args >= 2);

    if (curve25519_public_from_base64(&ip->enc_key, tok->args[1]) < 0) {
      log_warn(LD_REND, "Introduction point ntor enc-key is invalid");
      goto err;
    }
  } else {
    /* Unknown key type so we can't use that introduction point. */
    log_warn(LD_REND, "Introduction point encryption key is unrecognized.");
    goto err;
  }

  /* Exactly once "enc-key-cert" NL certificate NL */
  tok = find_by_keyword(tokens, R3_INTRO_ENC_KEY_CERT);
  tor_assert(tok->object_body);
  /* Do the cross certification. */
  if (strcmp(tok->object_type, "ED25519 CERT")) {
      log_warn(LD_REND, "Introduction point ntor encryption key "
                        "cross-certification has an unknown format.");
      goto err;
  }
  if (cert_parse_and_validate(&ip->enc_key_cert, tok->object_body,
                              tok->object_size, CERT_TYPE_CROSS_HS_IP_KEYS,
                              "introduction point enc-key-cert") < 0) {
    goto err;
  }
  if (tor_cert_checksig(ip->enc_key_cert,
                        &desc->plaintext_data.signing_pubkey, 0) < 0) {
    log_warn(LD_REND, "Invalid encryption key signature: %s",
             tor_cert_describe_signature_status(ip->enc_key_cert));
    goto err;
  }
  /* It is successfully cross certified. Flag the object. */
  ip->cross_certified = 1;

  /* Do we have a "legacy-key" SP key NL ?*/
  tok = find_opt_by_keyword(tokens, R3_INTRO_LEGACY_KEY);
  if (tok) {
    if (decode_intro_legacy_key(tok, tokens, ip, desc) < 0) {
      goto err;
    }
  }

  /* Introduction point has been parsed successfully. */
  goto done;

 err:
  hs_desc_intro_point_free(ip);
  ip = NULL;

 done:
  SMARTLIST_FOREACH(tokens, directory_token_t *, t, token_clear(t));
  smartlist_free(tokens);
  if (area) {
    memarea_drop_all(area);
  }

  return ip;
}

/** Given a descriptor string at <b>data</b>, decode all possible introduction
 * points that we can find. Add the introduction point object to desc_enc as we
 * find them. This function can't fail and it is possible that zero
 * introduction points can be decoded. */
static void
decode_intro_points(const hs_descriptor_t *desc,
                    hs_desc_encrypted_data_t *desc_enc,
                    const char *data)
{
  smartlist_t *chunked_desc = smartlist_new();
  smartlist_t *intro_points = smartlist_new();

  tor_assert(desc);
  tor_assert(desc_enc);
  tor_assert(data);
  tor_assert(desc_enc->intro_points);

  /* Take the desc string, and extract the intro point substrings out of it */
  {
    /* Split the descriptor string using the intro point header as delimiter */
    smartlist_split_string(chunked_desc, data, str_intro_point_start, 0, 0);

    /* Check if there are actually any intro points included. The first chunk
     * should be other descriptor fields (e.g. create2-formats), so it's not an
     * intro point. */
    if (smartlist_len(chunked_desc) < 2) {
      goto done;
    }
  }

  /* Take the intro point substrings, and prepare them for parsing */
  {
    int i = 0;
    /* Prepend the introduction-point header to all the chunks, since
       smartlist_split_string() devoured it. */
    SMARTLIST_FOREACH_BEGIN(chunked_desc, char *, chunk) {
      /* Ignore first chunk. It's other descriptor fields. */
      if (i++ == 0) {
        continue;
      }

      smartlist_add_asprintf(intro_points, "%s %s", str_intro_point, chunk);
    } SMARTLIST_FOREACH_END(chunk);
  }

  /* Parse the intro points! */
  SMARTLIST_FOREACH_BEGIN(intro_points, const char *, intro_point) {
    hs_desc_intro_point_t *ip = decode_introduction_point(desc, intro_point);
    if (!ip) {
      /* Malformed introduction point section. We'll ignore this introduction
       * point and continue parsing. New or unknown fields are possible for
       * forward compatibility. */
      continue;
    }
    smartlist_add(desc_enc->intro_points, ip);
  } SMARTLIST_FOREACH_END(intro_point);

 done:
  SMARTLIST_FOREACH(chunked_desc, char *, a, tor_free(a));
  smartlist_free(chunked_desc);
  SMARTLIST_FOREACH(intro_points, char *, a, tor_free(a));
  smartlist_free(intro_points);
}

/** Return 1 iff the given base64 encoded signature in b64_sig from the encoded
 * descriptor in encoded_desc validates the descriptor content. */
STATIC int
desc_sig_is_valid(const char *b64_sig,
                  const ed25519_public_key_t *signing_pubkey,
                  const char *encoded_desc, size_t encoded_len)
{
  int ret = 0;
  ed25519_signature_t sig;
  const char *sig_start;

  tor_assert(b64_sig);
  tor_assert(signing_pubkey);
  tor_assert(encoded_desc);
  /* Verifying nothing won't end well :). */
  tor_assert(encoded_len > 0);

  /* Signature length check. */
  if (strlen(b64_sig) != ED25519_SIG_BASE64_LEN) {
    log_warn(LD_REND, "Service descriptor has an invalid signature length."
                      "Expected %d but got %lu",
             ED25519_SIG_BASE64_LEN, (unsigned long) strlen(b64_sig));
    goto err;
  }

  /* First, convert base64 blob to an ed25519 signature. */
  if (ed25519_signature_from_base64(&sig, b64_sig) != 0) {
    log_warn(LD_REND, "Service descriptor does not contain a valid "
                      "signature");
    goto err;
  }

  /* Find the start of signature. */
  sig_start = tor_memstr(encoded_desc, encoded_len, "\n" str_signature " ");
  /* Getting here means the token parsing worked for the signature so if we
   * can't find the start of the signature, we have a code flow issue. */
  if (!sig_start) {
    log_warn(LD_GENERAL, "Malformed signature line. Rejecting.");
    goto err;
  }
  /* Skip newline, it has to go in the signature check. */
  sig_start++;

  /* Validate signature with the full body of the descriptor. */
  if (ed25519_checksig_prefixed(&sig,
                                (const uint8_t *) encoded_desc,
                                sig_start - encoded_desc,
                                str_desc_sig_prefix,
                                signing_pubkey) != 0) {
    log_warn(LD_REND, "Invalid signature on service descriptor");
    goto err;
  }
  /* Valid signature! All is good. */
  ret = 1;

 err:
  return ret;
}

/** Given the token tok for PoW params, decode it as hs_pow_desc_params_t.
 * tok->args MUST contain at least 4 elements Return 0 on success else -1 on
 * failure. */
static int
decode_pow_params(const directory_token_t *tok,
                  hs_pow_desc_params_t *pow_params)
{
  int ret = -1;

  tor_assert(tok);
  tor_assert(tok->n_args >= 4);
  tor_assert(pow_params);

  /* Find the type of PoW system being used. */
  int match = 0;
  for (int idx = 0; pow_types[idx].identifier; idx++) {
    if (!strncmp(tok->args[0], pow_types[idx].identifier,
                 strlen(pow_types[idx].identifier))) {
      pow_params->type = pow_types[idx].type;
      match = 1;
      break;
    }
  }
  if (!match) {
    log_warn(LD_REND, "Unknown PoW type from descriptor.");
    goto done;
  }

  if (base64_decode((char *)pow_params->seed, sizeof(pow_params->seed),
                    tok->args[1], strlen(tok->args[1])) !=
      sizeof(pow_params->seed)) {
    log_warn(LD_REND, "Unparseable seed %s in PoW params",
             escaped(tok->args[1]));
    goto done;
  }

  int ok;
  unsigned long effort =
      tor_parse_ulong(tok->args[2], 10, 0, UINT32_MAX, &ok, NULL);
  if (!ok) {
    log_warn(LD_REND, "Unparseable suggested effort %s in PoW params",
             escaped(tok->args[2]));
    goto done;
  }
  pow_params->suggested_effort = (uint32_t)effort;

  /* Parse the expiration time of the PoW params. */
  time_t expiration_time = 0;
  if (parse_iso_time_nospace(tok->args[3], &expiration_time)) {
    log_warn(LD_REND, "Unparseable expiration time %s in PoW params",
             escaped(tok->args[3]));
    goto done;
  }
  /* Validation of this time is done in client_desc_has_arrived() so we can
   * trigger a fetch if expired. */
  pow_params->expiration_time = expiration_time;

  /* Success. */
  ret = 0;

 done:
  return ret;
}

/** Decode descriptor plaintext data for version 3. Given a list of tokens, an
 * allocated plaintext object that will be populated and the encoded
 * descriptor with its length. The last one is needed for signature
 * verification. Unknown tokens are simply ignored so this won't error on
 * unknowns but requires that all v3 token be present and valid.
 *
 * Return 0 on success else a negative value. */
static hs_desc_decode_status_t
desc_decode_plaintext_v3(smartlist_t *tokens,
                         hs_desc_plaintext_data_t *desc,
                         const char *encoded_desc, size_t encoded_len)
{
  int ok;
  directory_token_t *tok;

  tor_assert(tokens);
  tor_assert(desc);
  /* Version higher could still use this function to decode most of the
   * descriptor and then they decode the extra part. */
  tor_assert(desc->version >= 3);

  /* Descriptor lifetime parsing. */
  tok = find_by_keyword(tokens, R3_DESC_LIFETIME);
  tor_assert(tok->n_args == 1);
  desc->lifetime_sec = (uint32_t) tor_parse_ulong(tok->args[0], 10, 0,
                                                  UINT32_MAX, &ok, NULL);
  if (!ok) {
    log_warn(LD_REND, "Service descriptor lifetime value is invalid");
    goto err;
  }
  /* Put it from minute to second. */
  desc->lifetime_sec *= 60;
  if (desc->lifetime_sec > HS_DESC_MAX_LIFETIME) {
    log_warn(LD_REND, "Service descriptor lifetime is too big. "
                      "Got %" PRIu32 " but max is %d",
             desc->lifetime_sec, HS_DESC_MAX_LIFETIME);
    goto err;
  }

  /* Descriptor signing certificate. */
  tok = find_by_keyword(tokens, R3_DESC_SIGNING_CERT);
  tor_assert(tok->object_body);
  /* Expecting a prop220 cert with the signing key extension, which contains
   * the blinded public key. */
  if (strcmp(tok->object_type, "ED25519 CERT") != 0) {
    log_warn(LD_REND, "Service descriptor signing cert wrong type (%s)",
             escaped(tok->object_type));
    goto err;
  }
  if (cert_parse_and_validate(&desc->signing_key_cert, tok->object_body,
                              tok->object_size, CERT_TYPE_SIGNING_HS_DESC,
                              "service descriptor signing key") < 0) {
    goto err;
  }

  /* Copy the public keys into signing_pubkey and blinded_pubkey */
  memcpy(&desc->signing_pubkey, &desc->signing_key_cert->signed_key,
         sizeof(ed25519_public_key_t));
  memcpy(&desc->blinded_pubkey, &desc->signing_key_cert->signing_key,
         sizeof(ed25519_public_key_t));

  /* Extract revision counter value. */
  tok = find_by_keyword(tokens, R3_REVISION_COUNTER);
  tor_assert(tok->n_args == 1);
  desc->revision_counter = tor_parse_uint64(tok->args[0], 10, 0,
                                            UINT64_MAX, &ok, NULL);
  if (!ok) {
    log_warn(LD_REND, "Service descriptor revision-counter is invalid");
    goto err;
  }

  /* Extract the superencrypted data section. */
  tok = find_by_keyword(tokens, R3_SUPERENCRYPTED);
  tor_assert(tok->object_body);
  if (strcmp(tok->object_type, "MESSAGE") != 0) {
    log_warn(LD_REND, "Desc superencrypted data section is invalid");
    goto err;
  }
  /* Make sure the length of the superencrypted blob is valid. */
  if (!encrypted_data_length_is_valid(tok->object_size)) {
    goto err;
  }

  /* Copy the superencrypted blob to the descriptor object so we can handle it
   * latter if needed. */
  desc->superencrypted_blob = tor_memdup(tok->object_body, tok->object_size);
  desc->superencrypted_blob_size = tok->object_size;

  /* Extract signature and verify it. */
  tok = find_by_keyword(tokens, R3_SIGNATURE);
  tor_assert(tok->n_args == 1);
  /* First arg here is the actual encoded signature. */
  if (!desc_sig_is_valid(tok->args[0], &desc->signing_pubkey,
                         encoded_desc, encoded_len)) {
    goto err;
  }

  return HS_DESC_DECODE_OK;
 err:
  return HS_DESC_DECODE_PLAINTEXT_ERROR;
}

/** Decode the version 3 superencrypted section of the given descriptor desc.
 * The desc_superencrypted_out will be populated with the decoded data. */
STATIC hs_desc_decode_status_t
desc_decode_superencrypted_v3(const hs_descriptor_t *desc,
                              hs_desc_superencrypted_data_t *
                              desc_superencrypted_out)
{
  hs_desc_decode_status_t ret = HS_DESC_DECODE_SUPERENC_ERROR;
  char *message = NULL;
  size_t message_len;
  memarea_t *area = NULL;
  directory_token_t *tok;
  smartlist_t *tokens = NULL;
  /* Rename the parameter because it is too long. */
  hs_desc_superencrypted_data_t *superencrypted = desc_superencrypted_out;

  tor_assert(desc);
  tor_assert(desc_superencrypted_out);

  /* Decrypt the superencrypted data that is located in the plaintext section
   * in the descriptor as a blob of bytes. */
  message_len = desc_decrypt_superencrypted(desc, &message);
  if (!message_len) {
    log_warn(LD_REND, "Service descriptor decryption failed.");
    goto err;
  }
  tor_assert(message);

  area = memarea_new();
  tokens = smartlist_new();
  if (tokenize_string(area, message, message + message_len,
                      tokens, hs_desc_superencrypted_v3_token_table, 0) < 0) {
    log_warn(LD_REND, "Superencrypted service descriptor is not parseable.");
    goto err;
  }

  /* Verify desc auth type */
  tok = find_by_keyword(tokens, R3_DESC_AUTH_TYPE);
  tor_assert(tok->n_args >= 1);
  if (strcmp(tok->args[0], "x25519")) {
    log_warn(LD_DIR, "Unrecognized desc auth type");
    goto err;
  }

  /* Extract desc auth ephemeral key */
  tok = find_by_keyword(tokens, R3_DESC_AUTH_KEY);
  tor_assert(tok->n_args >= 1);
  if (curve25519_public_from_base64(&superencrypted->auth_ephemeral_pubkey,
                                    tok->args[0]) < 0) {
    log_warn(LD_DIR, "Bogus desc auth ephemeral key in HS desc");
    goto err;
  }

  /* Extract desc auth client items */
  if (!superencrypted->clients) {
    superencrypted->clients = smartlist_new();
  }
  SMARTLIST_FOREACH_BEGIN(tokens, const directory_token_t *, token) {
    if (token->tp == R3_DESC_AUTH_CLIENT) {
      tor_assert(token->n_args >= 3);

      hs_desc_authorized_client_t *client =
        tor_malloc_zero(sizeof(hs_desc_authorized_client_t));

      if (decode_auth_client(token, client) < 0) {
        log_warn(LD_REND, "Descriptor client authorization section can't "
                          "be decoded.");
        tor_free(client);
        goto err;
      }
      smartlist_add(superencrypted->clients, client);
    }
  } SMARTLIST_FOREACH_END(token);

  /* Extract the encrypted data section. */
  tok = find_by_keyword(tokens, R3_ENCRYPTED);
  tor_assert(tok->object_body);
  if (strcmp(tok->object_type, "MESSAGE") != 0) {
    log_warn(LD_REND, "Desc encrypted data section is invalid");
    goto err;
  }
  /* Make sure the length of the encrypted blob is valid. */
  if (!encrypted_data_length_is_valid(tok->object_size)) {
    goto err;
  }

  /* Copy the encrypted blob to the descriptor object so we can handle it
   * latter if needed. */
  tor_assert(tok->object_size <= INT_MAX);
  superencrypted->encrypted_blob = tor_memdup(tok->object_body,
                                              tok->object_size);
  superencrypted->encrypted_blob_size = tok->object_size;

  ret = HS_DESC_DECODE_OK;
  goto done;

 err:
  tor_assert(ret < HS_DESC_DECODE_OK);
  hs_desc_superencrypted_data_free_contents(desc_superencrypted_out);

 done:
  if (tokens) {
    SMARTLIST_FOREACH(tokens, directory_token_t *, t, token_clear(t));
    smartlist_free(tokens);
  }
  if (area) {
    memarea_drop_all(area);
  }
  if (message) {
    tor_free(message);
  }
  return ret;
}

/** Decode the version 3 encrypted section of the given descriptor desc. The
 * desc_encrypted_out will be populated with the decoded data. */
STATIC hs_desc_decode_status_t
desc_decode_encrypted_v3(const hs_descriptor_t *desc,
                         const curve25519_secret_key_t *client_auth_sk,
                         hs_desc_encrypted_data_t *desc_encrypted_out)
{
  hs_desc_decode_status_t ret = HS_DESC_DECODE_ENCRYPTED_ERROR;
  char *message = NULL;
  size_t message_len;
  memarea_t *area = NULL;
  directory_token_t *tok;
  smartlist_t *tokens = NULL;

  tor_assert(desc);
  tor_assert(desc_encrypted_out);

  /* Decrypt the encrypted data that is located in the superencrypted section
   * in the descriptor as a blob of bytes. */
  message_len = desc_decrypt_encrypted(desc, client_auth_sk, &message);
  if (!message_len) {
    /* Two possible situation here. Either we have a client authorization
     * configured that didn't work or we do not have any configured for this
     * onion address so likely the descriptor is for authorized client only,
     * we are not. */
    if (client_auth_sk) {
      /* At warning level so the client can notice that its client
       * authorization is failing. */
      log_warn(LD_REND, "Client authorization for requested onion address "
                        "is invalid. Can't decrypt the descriptor.");
      ret = HS_DESC_DECODE_BAD_CLIENT_AUTH;
    } else {
      /* Inform at notice level that the onion address requested can't be
       * reached without client authorization most likely. */
      log_notice(LD_REND, "Fail to decrypt descriptor for requested onion "
                        "address. It is likely requiring client "
                        "authorization.");
      ret = HS_DESC_DECODE_NEED_CLIENT_AUTH;
    }
    goto err;
  }
  tor_assert(message);

  area = memarea_new();
  tokens = smartlist_new();
  if (tokenize_string(area, message, message + message_len,
                      tokens, hs_desc_encrypted_v3_token_table, 0) < 0) {
    log_warn(LD_REND, "Encrypted service descriptor is not parseable.");
    goto err;
  }

  /* CREATE2 supported cell format. It's mandatory. */
  tok = find_by_keyword(tokens, R3_CREATE2_FORMATS);
  tor_assert(tok);
  decode_create2_list(desc_encrypted_out, tok->args[0]);
  /* Must support ntor according to the specification */
  if (!desc_encrypted_out->create2_ntor) {
    log_warn(LD_REND, "Service create2-formats does not include ntor.");
    goto err;
  }

  /* Authentication type. It's optional but only once. */
  tok = find_opt_by_keyword(tokens, R3_INTRO_AUTH_REQUIRED);
  if (tok) {
    tor_assert(tok->n_args >= 1);
    if (!decode_auth_type(desc_encrypted_out, tok->args[0])) {
      log_warn(LD_REND, "Service descriptor authentication type has "
                        "invalid entry(ies).");
      goto err;
    }
  }

  /* Is this service a single onion service? */
  tok = find_opt_by_keyword(tokens, R3_SINGLE_ONION_SERVICE);
  if (tok) {
    desc_encrypted_out->single_onion_service = 1;
  }

  /* Get flow control if any. */
  tok = find_opt_by_keyword(tokens, R3_FLOW_CONTROL);
  if (tok) {
    int ok;

    tor_asprintf(&desc_encrypted_out->flow_control_pv, "FlowCtrl=%s",
                 tok->args[0]);
    uint8_t sendme_inc =
      (uint8_t) tor_parse_uint64(tok->args[1], 10, 0, UINT8_MAX, &ok, NULL);
    if (!ok || !congestion_control_validate_sendme_increment(sendme_inc)) {
      log_warn(LD_REND, "Service descriptor flow control sendme "
                        "value is invalid");
      goto err;
    }
    desc_encrypted_out->sendme_inc = sendme_inc;
  }

  /* Get PoW if any. */
  tok = find_opt_by_keyword(tokens, R3_POW_PARAMS);
  if (tok) {
    hs_pow_desc_params_t *pow_params =
      tor_malloc_zero(sizeof(hs_pow_desc_params_t));
    if (decode_pow_params(tok, pow_params)) {
      tor_free(pow_params);
      goto err;
    }
    desc_encrypted_out->pow_params = pow_params;
  }

  /* Initialize the descriptor's introduction point list before we start
   * decoding. Having 0 intro point is valid. Then decode them all. */
  desc_encrypted_out->intro_points = smartlist_new();
  decode_intro_points(desc, desc_encrypted_out, message);

  /* Validation of maximum introduction points allowed. */
  if (smartlist_len(desc_encrypted_out->intro_points) >
      HS_CONFIG_V3_MAX_INTRO_POINTS) {
    log_warn(LD_REND, "Service descriptor contains too many introduction "
                      "points. Maximum allowed is %d but we have %d",
             HS_CONFIG_V3_MAX_INTRO_POINTS,
             smartlist_len(desc_encrypted_out->intro_points));
    goto err;
  }

  /* NOTE: Unknown fields are allowed because this function could be used to
   * decode other descriptor version. */

  ret = HS_DESC_DECODE_OK;
  goto done;

 err:
  tor_assert(ret < HS_DESC_DECODE_OK);
  hs_desc_encrypted_data_free_contents(desc_encrypted_out);

 done:
  if (tokens) {
    SMARTLIST_FOREACH(tokens, directory_token_t *, t, token_clear(t));
    smartlist_free(tokens);
  }
  if (area) {
    memarea_drop_all(area);
  }
  if (message) {
    tor_free(message);
  }
  return ret;
}

/** Table of encrypted decode function version specific. The function are
 * indexed by the version number so v3 callback is at index 3 in the array. */
static hs_desc_decode_status_t
  (*decode_encrypted_handlers[])(
      const hs_descriptor_t *desc,
      const curve25519_secret_key_t *client_auth_sk,
      hs_desc_encrypted_data_t *desc_encrypted) =
{
  /* v0 */ NULL, /* v1 */ NULL, /* v2 */ NULL,
  desc_decode_encrypted_v3,
};

/** Decode the encrypted data section of the given descriptor and store the
 * data in the given encrypted data object. Return 0 on success else a
 * negative value on error. */
hs_desc_decode_status_t
hs_desc_decode_encrypted(const hs_descriptor_t *desc,
                         const curve25519_secret_key_t *client_auth_sk,
                         hs_desc_encrypted_data_t *desc_encrypted)
{
  hs_desc_decode_status_t ret = HS_DESC_DECODE_ENCRYPTED_ERROR;
  uint32_t version;

  tor_assert(desc);
  /* Ease our life a bit. */
  version = desc->plaintext_data.version;
  tor_assert(desc_encrypted);
  /* Calling this function without an encrypted blob to parse is a code flow
   * error. The superencrypted parsing should never succeed in the first place
   * without an encrypted section. */
  tor_assert(desc->superencrypted_data.encrypted_blob);
  /* Let's make sure we have a supported version as well. By correctly parsing
   * the plaintext, this should not fail. */
  if (BUG(!hs_desc_is_supported_version(version))) {
    goto err;
  }
  /* Extra precaution. Having no handler for the supported version should
   * never happened else we forgot to add it but we bumped the version. */
  tor_assert(ARRAY_LENGTH(decode_encrypted_handlers) >= version);
  tor_assert(decode_encrypted_handlers[version]);

  /* Run the version specific plaintext decoder. */
  ret = decode_encrypted_handlers[version](desc, client_auth_sk,
                                           desc_encrypted);
  if (ret < 0) {
    goto err;
  }

 err:
  return ret;
}

/** Table of superencrypted decode function version specific. The function are
 * indexed by the version number so v3 callback is at index 3 in the array. */
static hs_desc_decode_status_t
  (*decode_superencrypted_handlers[])(
      const hs_descriptor_t *desc,
      hs_desc_superencrypted_data_t *desc_superencrypted) =
{
  /* v0 */ NULL, /* v1 */ NULL, /* v2 */ NULL,
  desc_decode_superencrypted_v3,
};

/** Decode the superencrypted data section of the given descriptor and store
 * the data in the given superencrypted data object. */
hs_desc_decode_status_t
hs_desc_decode_superencrypted(const hs_descriptor_t *desc,
                              hs_desc_superencrypted_data_t *
                              desc_superencrypted)
{
  hs_desc_decode_status_t ret = HS_DESC_DECODE_SUPERENC_ERROR;
  uint32_t version;

  tor_assert(desc);
  /* Ease our life a bit. */
  version = desc->plaintext_data.version;
  tor_assert(desc_superencrypted);
  /* Calling this function without an superencrypted blob to parse is
   * a code flow error. The plaintext parsing should never succeed in
   * the first place without an superencrypted section. */
  tor_assert(desc->plaintext_data.superencrypted_blob);
  /* Let's make sure we have a supported version as well. By correctly parsing
   * the plaintext, this should not fail. */
  if (BUG(!hs_desc_is_supported_version(version))) {
    goto err;
  }
  /* Extra precaution. Having no handler for the supported version should
   * never happened else we forgot to add it but we bumped the version. */
  tor_assert(ARRAY_LENGTH(decode_superencrypted_handlers) >= version);
  tor_assert(decode_superencrypted_handlers[version]);

  /* Run the version specific plaintext decoder. */
  ret = decode_superencrypted_handlers[version](desc, desc_superencrypted);
  if (ret < 0) {
    goto err;
  }

 err:
  return ret;
}

/** Table of plaintext decode function version specific. The function are
 * indexed by the version number so v3 callback is at index 3 in the array. */
static hs_desc_decode_status_t
  (*decode_plaintext_handlers[])(
      smartlist_t *tokens,
      hs_desc_plaintext_data_t *desc,
      const char *encoded_desc,
      size_t encoded_len) =
{
  /* v0 */ NULL, /* v1 */ NULL, /* v2 */ NULL,
  desc_decode_plaintext_v3,
};

/** Fully decode the given descriptor plaintext and store the data in the
 * plaintext data object. */
hs_desc_decode_status_t
hs_desc_decode_plaintext(const char *encoded,
                         hs_desc_plaintext_data_t *plaintext)
{
  int ok = 0;
  hs_desc_decode_status_t ret = HS_DESC_DECODE_PLAINTEXT_ERROR;
  memarea_t *area = NULL;
  smartlist_t *tokens = NULL;
  size_t encoded_len;
  directory_token_t *tok;

  tor_assert(encoded);
  tor_assert(plaintext);

  /* Check that descriptor is within size limits. */
  encoded_len = strlen(encoded);
  if (encoded_len >= hs_cache_get_max_descriptor_size()) {
    log_warn(LD_REND, "Service descriptor is too big (%lu bytes)",
             (unsigned long) encoded_len);
    goto err;
  }

  area = memarea_new();
  tokens = smartlist_new();
  /* Tokenize the descriptor so we can start to parse it. */
  if (tokenize_string(area, encoded, encoded + encoded_len, tokens,
                      hs_desc_v3_token_table, 0) < 0) {
    log_warn(LD_REND, "Service descriptor is not parseable");
    goto err;
  }

  /* Get the version of the descriptor which is the first mandatory field of
   * the descriptor. From there, we'll decode the right descriptor version. */
  tok = find_by_keyword(tokens, R_HS_DESCRIPTOR);
  tor_assert(tok->n_args == 1);
  plaintext->version = (uint32_t) tor_parse_ulong(tok->args[0], 10, 0,
                                                  UINT32_MAX, &ok, NULL);
  if (!ok) {
    log_warn(LD_REND, "Service descriptor has unparseable version %s",
             escaped(tok->args[0]));
    goto err;
  }
  if (!hs_desc_is_supported_version(plaintext->version)) {
    log_warn(LD_REND, "Service descriptor has unsupported version %" PRIu32,
             plaintext->version);
    goto err;
  }
  /* Extra precaution. Having no handler for the supported version should
   * never happened else we forgot to add it but we bumped the version. */
  tor_assert(ARRAY_LENGTH(decode_plaintext_handlers) >= plaintext->version);
  tor_assert(decode_plaintext_handlers[plaintext->version]);

  /* Run the version specific plaintext decoder. */
  ret = decode_plaintext_handlers[plaintext->version](tokens, plaintext,
                                                      encoded, encoded_len);
  if (ret != HS_DESC_DECODE_OK) {
    goto err;
  }
  /* Success. Descriptor has been populated with the data. */
  ret = HS_DESC_DECODE_OK;

 err:
  if (tokens) {
    SMARTLIST_FOREACH(tokens, directory_token_t *, t, token_clear(t));
    smartlist_free(tokens);
  }
  if (area) {
    memarea_drop_all(area);
  }
  return ret;
}

/** Fully decode an encoded descriptor and set a newly allocated descriptor
 * object in desc_out.  Client secret key is used to decrypt the "encrypted"
 * section if not NULL else it's ignored.
 *
 * Return 0 on success. A negative value is returned on error and desc_out is
 * set to NULL. */
hs_desc_decode_status_t
hs_desc_decode_descriptor(const char *encoded,
                          const hs_subcredential_t *subcredential,
                          const curve25519_secret_key_t *client_auth_sk,
                          hs_descriptor_t **desc_out)
{
  hs_desc_decode_status_t ret = HS_DESC_DECODE_GENERIC_ERROR;
  hs_descriptor_t *desc;

  tor_assert(encoded);

  desc = tor_malloc_zero(sizeof(hs_descriptor_t));

  /* Subcredentials are not optional. */
  if (BUG(!subcredential ||
          fast_mem_is_zero((char*)subcredential, DIGEST256_LEN))) {
    log_warn(LD_GENERAL, "Tried to decrypt without subcred. Impossible!");
    goto err;
  }

  memcpy(&desc->subcredential, subcredential, sizeof(desc->subcredential));

  ret = hs_desc_decode_plaintext(encoded, &desc->plaintext_data);
  if (ret != HS_DESC_DECODE_OK) {
    goto err;
  }

  ret = hs_desc_decode_superencrypted(desc, &desc->superencrypted_data);
  if (ret != HS_DESC_DECODE_OK) {
    goto err;
  }

  ret = hs_desc_decode_encrypted(desc, client_auth_sk, &desc->encrypted_data);
  if (ret != HS_DESC_DECODE_OK) {
    goto err;
  }

  if (desc_out) {
    *desc_out = desc;
  } else {
    hs_descriptor_free(desc);
  }
  return ret;

 err:
  hs_descriptor_free(desc);
  if (desc_out) {
    *desc_out = NULL;
  }

  tor_assert(ret < 0);
  return ret;
}

/** Table of encode function version specific. The functions are indexed by the
 * version number so v3 callback is at index 3 in the array. */
static int
  (*encode_handlers[])(
      const hs_descriptor_t *desc,
      const ed25519_keypair_t *signing_kp,
      const uint8_t *descriptor_cookie,
      char **encoded_out) =
{
  /* v0 */ NULL, /* v1 */ NULL, /* v2 */ NULL,
  desc_encode_v3,
};

/** Encode the given descriptor desc including signing with the given key pair
 * signing_kp and encrypting with the given descriptor cookie.
 *
 * If the client authorization is enabled, descriptor_cookie must be the same
 * as the one used to build hs_desc_authorized_client_t in the descriptor.
 * Otherwise, it must be NULL.  On success, encoded_out points to a newly
 * allocated NUL terminated string that contains the encoded descriptor as
 * a string.
 *
 * Return 0 on success and encoded_out is a valid pointer. On error, -1 is
 * returned and encoded_out is set to NULL. */
MOCK_IMPL(int,
hs_desc_encode_descriptor,(const hs_descriptor_t *desc,
                           const ed25519_keypair_t *signing_kp,
                           const uint8_t *descriptor_cookie,
                           char **encoded_out))
{
  int ret = -1;
  uint32_t version;

  tor_assert(desc);
  tor_assert(encoded_out);

  /* Make sure we support the version of the descriptor format. */
  version = desc->plaintext_data.version;
  if (!hs_desc_is_supported_version(version)) {
    goto err;
  }
  /* Extra precaution. Having no handler for the supported version should
   * never happened else we forgot to add it but we bumped the version. */
  tor_assert(ARRAY_LENGTH(encode_handlers) >= version);
  tor_assert(encode_handlers[version]);

  ret = encode_handlers[version](desc, signing_kp,
                                 descriptor_cookie, encoded_out);
  if (ret < 0) {
    goto err;
  }

  /* Try to decode what we just encoded. Symmetry is nice!, but it is
   * symmetric only if the client auth is disabled (That is, the descriptor
   * cookie will be NULL) and the test-only mock plaintext isn't in use. */
  bool do_round_trip_test = !descriptor_cookie;
#ifdef TOR_UNIT_TESTS
  if (desc->encrypted_data.test_extra_plaintext) {
    do_round_trip_test = false;
  }
#endif
  if (do_round_trip_test) {
    ret = hs_desc_decode_descriptor(*encoded_out, &desc->subcredential,
                                    NULL, NULL);
    if (BUG(ret != HS_DESC_DECODE_OK)) {
      ret = -1;
      goto err;
    }
  }

  return 0;

 err:
  *encoded_out = NULL;
  return ret;
}

/** Free the content of the plaintext section of a descriptor. */
void
hs_desc_plaintext_data_free_contents(hs_desc_plaintext_data_t *desc)
{
  if (!desc) {
    return;
  }

  if (desc->superencrypted_blob) {
    tor_free(desc->superencrypted_blob);
  }
  tor_cert_free(desc->signing_key_cert);

  memwipe(desc, 0, sizeof(*desc));
}

/** Free the content of the superencrypted section of a descriptor. */
void
hs_desc_superencrypted_data_free_contents(hs_desc_superencrypted_data_t *desc)
{
  if (!desc) {
    return;
  }

  if (desc->encrypted_blob) {
    tor_free(desc->encrypted_blob);
  }
  if (desc->clients) {
    SMARTLIST_FOREACH(desc->clients, hs_desc_authorized_client_t *, client,
                      hs_desc_authorized_client_free(client));
    smartlist_free(desc->clients);
  }

  memwipe(desc, 0, sizeof(*desc));
}

/** Free the content of the encrypted section of a descriptor. */
void
hs_desc_encrypted_data_free_contents(hs_desc_encrypted_data_t *desc)
{
  if (!desc) {
    return;
  }

  if (desc->intro_auth_types) {
    SMARTLIST_FOREACH(desc->intro_auth_types, char *, a, tor_free(a));
    smartlist_free(desc->intro_auth_types);
  }
  if (desc->intro_points) {
    SMARTLIST_FOREACH(desc->intro_points, hs_desc_intro_point_t *, ip,
                      hs_desc_intro_point_free(ip));
    smartlist_free(desc->intro_points);
  }
  tor_free(desc->flow_control_pv);
  tor_free(desc->pow_params);
  memwipe(desc, 0, sizeof(*desc));
}

/** Free the descriptor plaintext data object. */
void
hs_desc_plaintext_data_free_(hs_desc_plaintext_data_t *desc)
{
  hs_desc_plaintext_data_free_contents(desc);
  tor_free(desc);
}

/** Free the descriptor plaintext data object. */
void
hs_desc_superencrypted_data_free_(hs_desc_superencrypted_data_t *desc)
{
  hs_desc_superencrypted_data_free_contents(desc);
  tor_free(desc);
}

/** Free the descriptor encrypted data object. */
void
hs_desc_encrypted_data_free_(hs_desc_encrypted_data_t *desc)
{
  hs_desc_encrypted_data_free_contents(desc);
  tor_free(desc);
}

/** Free the given descriptor object. */
void
hs_descriptor_free_(hs_descriptor_t *desc)
{
  if (!desc) {
    return;
  }

  hs_desc_plaintext_data_free_contents(&desc->plaintext_data);
  hs_desc_superencrypted_data_free_contents(&desc->superencrypted_data);
  hs_desc_encrypted_data_free_contents(&desc->encrypted_data);
  tor_free(desc);
}

/** Return the size in bytes of the given plaintext data object. A sizeof() is
 * not enough because the object contains pointers and the encrypted blob.
 * This is particularly useful for our OOM subsystem that tracks the HSDir
 * cache size for instance. */
size_t
hs_desc_plaintext_obj_size(const hs_desc_plaintext_data_t *data)
{
  tor_assert(data);
  return (sizeof(*data) + sizeof(*data->signing_key_cert) +
          data->superencrypted_blob_size);
}

/** Return the size in bytes of the given encrypted data object. Used by OOM
 * subsystem. */
static size_t
hs_desc_encrypted_obj_size(const hs_desc_encrypted_data_t *data)
{
  tor_assert(data);
  size_t intro_size = 0;
  if (data->intro_auth_types) {
    intro_size +=
      smartlist_len(data->intro_auth_types) * sizeof(intro_auth_types);
  }
  if (data->intro_points) {
    /* XXX could follow pointers here and get more accurate size */
    intro_size +=
      smartlist_len(data->intro_points) * sizeof(hs_desc_intro_point_t);
  }

  return sizeof(*data) + intro_size;
}

/** Return the size in bytes of the given descriptor object. Used by OOM
 * subsystem. */
  size_t
hs_desc_obj_size(const hs_descriptor_t *data)
{
  if (data == NULL) {
    return 0;
  }
  return (hs_desc_plaintext_obj_size(&data->plaintext_data) +
          hs_desc_encrypted_obj_size(&data->encrypted_data) +
          sizeof(data->subcredential));
}

/** Return a newly allocated descriptor intro point. */
hs_desc_intro_point_t *
hs_desc_intro_point_new(void)
{
  hs_desc_intro_point_t *ip = tor_malloc_zero(sizeof(*ip));
  ip->link_specifiers = smartlist_new();
  return ip;
}

/** Free a descriptor intro point object. */
void
hs_desc_intro_point_free_(hs_desc_intro_point_t *ip)
{
  if (ip == NULL) {
    return;
  }
  if (ip->link_specifiers) {
    SMARTLIST_FOREACH(ip->link_specifiers, link_specifier_t *,
                      ls, link_specifier_free(ls));
    smartlist_free(ip->link_specifiers);
  }
  tor_cert_free(ip->auth_key_cert);
  tor_cert_free(ip->enc_key_cert);
  crypto_pk_free(ip->legacy.key);
  tor_free(ip->legacy.cert.encoded);
  tor_free(ip);
}

/** Allocate and build a new fake client info for the descriptor. Return a
 * newly allocated object. This can't fail. */
hs_desc_authorized_client_t *
hs_desc_build_fake_authorized_client(void)
{
  hs_desc_authorized_client_t *client_auth =
    tor_malloc_zero(sizeof(*client_auth));

  crypto_rand((char *) client_auth->client_id,
              sizeof(client_auth->client_id));
  crypto_rand((char *) client_auth->iv,
              sizeof(client_auth->iv));
  crypto_rand((char *) client_auth->encrypted_cookie,
              sizeof(client_auth->encrypted_cookie));

  return client_auth;
}

/** Using the service's subcredential, client public key, auth ephemeral secret
 * key, and descriptor cookie, build the auth client so we can then encode the
 * descriptor for publication. client_out must be already allocated. */
void
hs_desc_build_authorized_client(const hs_subcredential_t *subcredential,
                                const curve25519_public_key_t *client_auth_pk,
                                const curve25519_secret_key_t *
                                auth_ephemeral_sk,
                                const uint8_t *descriptor_cookie,
                                hs_desc_authorized_client_t *client_out)
{
  uint8_t *keystream = NULL;
  size_t keystream_length = 0;
  const uint8_t *cookie_key;
  crypto_cipher_t *cipher;

  tor_assert(client_auth_pk);
  tor_assert(auth_ephemeral_sk);
  tor_assert(descriptor_cookie);
  tor_assert(client_out);
  tor_assert(subcredential);
  tor_assert(!fast_mem_is_zero((char *) auth_ephemeral_sk,
                              sizeof(*auth_ephemeral_sk)));
  tor_assert(!fast_mem_is_zero((char *) client_auth_pk,
                              sizeof(*client_auth_pk)));
  tor_assert(!fast_mem_is_zero((char *) descriptor_cookie,
                              HS_DESC_DESCRIPTOR_COOKIE_LEN));
  tor_assert(!fast_mem_is_zero((char *) subcredential,
                              DIGEST256_LEN));

  /* Get the KEYS part so we can derive the CLIENT-ID and COOKIE-KEY. */
  keystream_length =
    build_descriptor_cookie_keys(subcredential,
                                 auth_ephemeral_sk, client_auth_pk,
                                 &keystream);
  tor_assert(keystream_length > 0);

  /* Extract the CLIENT-ID and COOKIE-KEY from the KEYS. */
  memcpy(client_out->client_id, keystream, HS_DESC_CLIENT_ID_LEN);
  cookie_key = keystream + HS_DESC_CLIENT_ID_LEN;

  /* Random IV */
  crypto_strongest_rand(client_out->iv, sizeof(client_out->iv));

  /* This creates a cipher for AES. It can't fail. */
  cipher = crypto_cipher_new_with_iv_and_bits(cookie_key, client_out->iv,
                                              HS_DESC_COOKIE_KEY_BIT_SIZE);
  /* This can't fail. */
  crypto_cipher_encrypt(cipher, (char *) client_out->encrypted_cookie,
                        (const char *) descriptor_cookie,
                        HS_DESC_DESCRIPTOR_COOKIE_LEN);

  memwipe(keystream, 0, keystream_length);
  tor_free(keystream);

  crypto_cipher_free(cipher);
}

/** Free an authoriezd client object. */
void
hs_desc_authorized_client_free_(hs_desc_authorized_client_t *client)
{
  tor_free(client);
}

/** From the given descriptor, remove and free every introduction point. */
void
hs_descriptor_clear_intro_points(hs_descriptor_t *desc)
{
  smartlist_t *ips;

  tor_assert(desc);

  ips = desc->encrypted_data.intro_points;
  if (ips) {
    SMARTLIST_FOREACH(ips, hs_desc_intro_point_t *,
                      ip, hs_desc_intro_point_free(ip));
    smartlist_clear(ips);
  }
}

/** Return true iff we support the given descriptor congestion control
 * parameters. */
bool
hs_desc_supports_congestion_control(const hs_descriptor_t *desc)
{
  tor_assert(desc);

  /* Validate that we support the protocol version in the descriptor. */
  return desc->encrypted_data.flow_control_pv &&
         protocol_list_supports_protocol(desc->encrypted_data.flow_control_pv,
                                         PRT_FLOWCTRL, PROTOVER_FLOWCTRL_CC);
}
