/* Copyright (c) 2017-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

#define HS_DESCRIPTOR_PRIVATE

#include "core/or/or.h"
#include "trunnel/ed25519_cert.h" /* Trunnel interface. */
#include "lib/crypt_ops/crypto_ed25519.h"
#include "feature/hs/hs_descriptor.h"
#include "feature/dirparse/unparseable.h"

#include "test/fuzz/fuzzing.h"

static void
mock_dump_desc__nodump(const char *desc, const char *type)
{
  (void)desc;
  (void)type;
}

static int
mock_rsa_ed25519_crosscert_check(const uint8_t *crosscert,
                                 const size_t crosscert_len,
                                 const crypto_pk_t *rsa_id_key,
                                 const ed25519_public_key_t *master_key,
                                 const time_t reject_if_expired_before)
{
  (void) crosscert;
  (void) crosscert_len;
  (void) rsa_id_key;
  (void) master_key;
  (void) reject_if_expired_before;
  return 0;
}

static size_t
mock_decrypt_desc_layer(const hs_descriptor_t *desc,
                        const uint8_t *descriptor_cookie,
                        bool is_superencrypted_layer,
                        char **decrypted_out)
{
  (void)is_superencrypted_layer;
  (void)desc;
  (void)descriptor_cookie;
  const size_t overhead = HS_DESC_ENCRYPTED_SALT_LEN + DIGEST256_LEN;
  const uint8_t *encrypted_blob = (is_superencrypted_layer)
    ? desc->plaintext_data.superencrypted_blob
    : desc->superencrypted_data.encrypted_blob;
  size_t encrypted_blob_size = (is_superencrypted_layer)
    ? desc->plaintext_data.superencrypted_blob_size
    : desc->superencrypted_data.encrypted_blob_size;

  if (encrypted_blob_size < overhead)
    return 0;
  *decrypted_out = tor_memdup_nulterm(
                   encrypted_blob + HS_DESC_ENCRYPTED_SALT_LEN,
                   encrypted_blob_size - overhead);
  size_t result = strlen(*decrypted_out);
  if (result) {
    return result;
  } else {
    tor_free(*decrypted_out);
    return 0;
  }
}

static const uint8_t *decrypted_data = NULL;
static size_t decrypted_len = 0;
static size_t
mock_desc_decrypt_encrypted(const hs_descriptor_t *desc,
                        const curve25519_secret_key_t *client_auth_sk,
                        char **decrypted_out)
{
  (void)desc;
  (void)client_auth_sk;
  *decrypted_out = (char*)tor_memdup_nulterm(decrypted_data, decrypted_len);
  return decrypted_len;
}

int
fuzz_init(void)
{
  disable_signature_checking();
  MOCK(dump_desc, mock_dump_desc__nodump);
  MOCK(rsa_ed25519_crosscert_check, mock_rsa_ed25519_crosscert_check);
  MOCK(decrypt_desc_layer, mock_decrypt_desc_layer);
  MOCK(desc_decrypt_encrypted, mock_desc_decrypt_encrypted);
  ed25519_init();
  return 0;
}

int
fuzz_cleanup(void)
{
  return 0;
}

int
fuzz_main(const uint8_t *data, size_t sz)
{
  decrypted_data = data;
  decrypted_len = sz;

  hs_descriptor_t *desc = tor_malloc_zero(sizeof(hs_descriptor_t));
  hs_desc_encrypted_data_t *output = tor_malloc_zero(sizeof(*output));
  curve25519_secret_key_t *client_auth_sk = NULL;
  hs_desc_decode_status_t status;

  status = desc_decode_encrypted_v3(desc, client_auth_sk, output);
  if (status == HS_DESC_DECODE_OK) {
    log_debug(LD_GENERAL, "Decoding okay");
  } else {
    log_debug(LD_GENERAL, "Decoding failed");
  }

  hs_descriptor_free(desc);
  hs_desc_encrypted_data_free(output);
  return 0;
}
