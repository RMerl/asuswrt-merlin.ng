/* Copyright (c) 2001, Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file crypto_hkdf.c
 * \brief Block of functions related with HKDF utilities and operations.
 **/

#include "lib/crypt_ops/crypto_hkdf.h"
#include "lib/crypt_ops/crypto_util.h"
#include "lib/crypt_ops/crypto_digest.h"

#include "lib/crypt_ops/crypto_openssl_mgt.h"
#include "lib/intmath/cmp.h"
#include "lib/log/util_bug.h"

#ifdef ENABLE_OPENSSL
#include <openssl/evp.h>
#include <openssl/opensslv.h>

#if defined(HAVE_ERR_LOAD_KDF_STRINGS)
#include <openssl/kdf.h>
#define HAVE_OPENSSL_HKDF 1
#endif
#endif /* defined(ENABLE_OPENSSL) */

#include <string.h>

/** Given <b>key_in_len</b> bytes of negotiated randomness in <b>key_in</b>
 * ("K"), expand it into <b>key_out_len</b> bytes of negotiated key material in
 * <b>key_out</b> by taking the first <b>key_out_len</b> bytes of
 *    H(K | [00]) | H(K | [01]) | ....
 *
 * This is the key expansion algorithm used in the "TAP" circuit extension
 * mechanism; it shouldn't be used for new protocols.
 *
 * Return 0 on success, -1 on failure.
 */
int
crypto_expand_key_material_TAP(const uint8_t *key_in, size_t key_in_len,
                               uint8_t *key_out, size_t key_out_len)
{
  int i, r = -1;
  uint8_t *cp, *tmp = tor_malloc(key_in_len+1);
  uint8_t digest[DIGEST_LEN];

  /* If we try to get more than this amount of key data, we'll repeat blocks.*/
  tor_assert(key_out_len <= DIGEST_LEN*256);

  memcpy(tmp, key_in, key_in_len);
  for (cp = key_out, i=0; cp < key_out+key_out_len;
       ++i, cp += DIGEST_LEN) {
    tmp[key_in_len] = i;
    if (crypto_digest((char*)digest, (const char *)tmp, key_in_len+1) < 0)
      goto exit;
    memcpy(cp, digest, MIN(DIGEST_LEN, key_out_len-(cp-key_out)));
  }

  r = 0;
 exit:
  memwipe(tmp, 0, key_in_len+1);
  tor_free(tmp);
  memwipe(digest, 0, sizeof(digest));
  return r;
}

#ifdef HAVE_OPENSSL_HKDF
/**
 * Perform RFC5869 HKDF computation using OpenSSL (only to be called from
 * crypto_expand_key_material_rfc5869_sha256_openssl). Note that OpenSSL
 * requires input key to be nonempty and salt length to be equal or less
 * than 1024.
 */
static int
crypto_expand_key_material_rfc5869_sha256_openssl(
                                    const uint8_t *key_in, size_t key_in_len,
                                    const uint8_t *salt_in, size_t salt_in_len,
                                    const uint8_t *info_in, size_t info_in_len,
                                    uint8_t *key_out, size_t key_out_len)
{
  int r;
  EVP_PKEY_CTX *evp_pkey_ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_HKDF, NULL);
  tor_assert(evp_pkey_ctx);
  tor_assert(key_in_len != 0);
  tor_assert(salt_in_len <= 1024);

  r = EVP_PKEY_derive_init(evp_pkey_ctx);
  tor_assert(r == 1);

  r = EVP_PKEY_CTX_set_hkdf_md(evp_pkey_ctx, EVP_sha256());
  tor_assert(r == 1);

  r = EVP_PKEY_CTX_set1_hkdf_salt(evp_pkey_ctx, salt_in, (int)salt_in_len);
  tor_assert(r == 1);

  r = EVP_PKEY_CTX_set1_hkdf_key(evp_pkey_ctx, key_in, (int)key_in_len);
  tor_assert(r == 1);

  r = EVP_PKEY_CTX_add1_hkdf_info(evp_pkey_ctx, info_in, (int)info_in_len);
  tor_assert(r == 1);

  r = EVP_PKEY_derive(evp_pkey_ctx, key_out, &key_out_len);
  tor_assert(r == 1);

  EVP_PKEY_CTX_free(evp_pkey_ctx);
  return 0;
}

#else /* !defined(HAVE_OPENSSL_HKDF) */

/**
 * Perform RFC5869 HKDF computation using our own legacy implementation.
 * Only to be called from crypto_expand_key_material_rfc5869_sha256_openssl.
 */
static int
crypto_expand_key_material_rfc5869_sha256_legacy(
                                    const uint8_t *key_in, size_t key_in_len,
                                    const uint8_t *salt_in, size_t salt_in_len,
                                    const uint8_t *info_in, size_t info_in_len,
                                    uint8_t *key_out, size_t key_out_len)
{
  uint8_t prk[DIGEST256_LEN];
  uint8_t tmp[DIGEST256_LEN + 128 + 1];
  uint8_t mac[DIGEST256_LEN];
  int i;
  uint8_t *outp;
  size_t tmp_len;

  crypto_hmac_sha256((char*)prk,
                     (const char*)salt_in, salt_in_len,
                     (const char*)key_in, key_in_len);

  /* If we try to get more than this amount of key data, we'll repeat blocks.*/
  tor_assert(key_out_len <= DIGEST256_LEN * 256);
  tor_assert(info_in_len <= 128);
  memset(tmp, 0, sizeof(tmp));
  outp = key_out;
  i = 1;

  while (key_out_len) {
    size_t n;
    if (i > 1) {
      memcpy(tmp, mac, DIGEST256_LEN);
      memcpy(tmp+DIGEST256_LEN, info_in, info_in_len);
      tmp[DIGEST256_LEN+info_in_len] = i;
      tmp_len = DIGEST256_LEN + info_in_len + 1;
    } else {
      memcpy(tmp, info_in, info_in_len);
      tmp[info_in_len] = i;
      tmp_len = info_in_len + 1;
    }
    crypto_hmac_sha256((char*)mac,
                       (const char*)prk, DIGEST256_LEN,
                       (const char*)tmp, tmp_len);
    n = key_out_len < DIGEST256_LEN ? key_out_len : DIGEST256_LEN;
    memcpy(outp, mac, n);
    key_out_len -= n;
    outp += n;
    ++i;
  }

  memwipe(tmp, 0, sizeof(tmp));
  memwipe(mac, 0, sizeof(mac));
  return 0;
}
#endif /* defined(HAVE_OPENSSL_HKDF) */

/** Expand some secret key material according to RFC5869, using SHA256 as the
 * underlying hash.  The <b>key_in_len</b> bytes at <b>key_in</b> are the
 * secret key material; the <b>salt_in_len</b> bytes at <b>salt_in</b> and the
 * <b>info_in_len</b> bytes in <b>info_in_len</b> are the algorithm's "salt"
 * and "info" parameters respectively.  On success, write <b>key_out_len</b>
 * bytes to <b>key_out</b> and return 0.  Assert on failure.
 */
int
crypto_expand_key_material_rfc5869_sha256(
                                    const uint8_t *key_in, size_t key_in_len,
                                    const uint8_t *salt_in, size_t salt_in_len,
                                    const uint8_t *info_in, size_t info_in_len,
                                    uint8_t *key_out, size_t key_out_len)
{
  tor_assert(key_in);
  tor_assert(key_in_len > 0);

#ifdef HAVE_OPENSSL_HKDF
  return crypto_expand_key_material_rfc5869_sha256_openssl(key_in,
                                             key_in_len, salt_in,
                                             salt_in_len, info_in,
                                             info_in_len,
                                             key_out, key_out_len);
#else /* !defined(HAVE_OPENSSL_HKDF) */
  return crypto_expand_key_material_rfc5869_sha256_legacy(key_in,
                                               key_in_len, salt_in,
                                               salt_in_len, info_in,
                                               info_in_len,
                                               key_out, key_out_len);
#endif /* defined(HAVE_OPENSSL_HKDF) */
}
