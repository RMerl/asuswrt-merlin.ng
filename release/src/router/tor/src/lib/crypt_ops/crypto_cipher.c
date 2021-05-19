/* Copyright (c) 2001, Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file crypto_cipher.c
 * \brief Symmetric cryptography (low-level) with AES.
 **/

#include "orconfig.h"

#include "lib/crypt_ops/crypto_cipher.h"
#include "lib/crypt_ops/crypto_rand.h"
#include "lib/crypt_ops/crypto_util.h"

#include "lib/log/log.h"
#include "lib/log/util_bug.h"
#include "lib/cc/torint.h"
#include "lib/crypt_ops/aes.h"

#include <string.h>

/** Allocate and return a new symmetric cipher using the provided key and iv.
 * The key is <b>bits</b> bits long; the IV is CIPHER_IV_LEN bytes.  Both
 * must be provided. Key length must be 128, 192, or 256 */
crypto_cipher_t *
crypto_cipher_new_with_iv_and_bits(const uint8_t *key,
                                   const uint8_t *iv,
                                   int bits)
{
  tor_assert(key);
  tor_assert(iv);

  return aes_new_cipher((const uint8_t*)key, (const uint8_t*)iv, bits);
}

/** Allocate and return a new symmetric cipher using the provided key and iv.
 * The key is CIPHER_KEY_LEN bytes; the IV is CIPHER_IV_LEN bytes.  Both
 * must be provided.
 */
crypto_cipher_t *
crypto_cipher_new_with_iv(const char *key, const char *iv)
{
  return crypto_cipher_new_with_iv_and_bits((uint8_t*)key, (uint8_t*)iv,
                                            128);
}

/** Return a new crypto_cipher_t with the provided <b>key</b> and an IV of all
 * zero bytes and key length <b>bits</b>.  Key length must be 128, 192, or
 * 256. */
crypto_cipher_t *
crypto_cipher_new_with_bits(const char *key, int bits)
{
  char zeroiv[CIPHER_IV_LEN];
  memset(zeroiv, 0, sizeof(zeroiv));
  return crypto_cipher_new_with_iv_and_bits((uint8_t*)key, (uint8_t*)zeroiv,
                                            bits);
}

/** Return a new crypto_cipher_t with the provided <b>key</b> (of
 * CIPHER_KEY_LEN bytes) and an IV of all zero bytes.  */
crypto_cipher_t *
crypto_cipher_new(const char *key)
{
  return crypto_cipher_new_with_bits(key, 128);
}

/** Free a symmetric cipher.
 */
void
crypto_cipher_free_(crypto_cipher_t *env)
{
  if (!env)
    return;

  aes_cipher_free(env);
}

/* symmetric crypto */

/** Encrypt <b>fromlen</b> bytes from <b>from</b> using the cipher
 * <b>env</b>; on success, store the result to <b>to</b> and return 0.
 * Does not check for failure.
 */
int
crypto_cipher_encrypt(crypto_cipher_t *env, char *to,
                      const char *from, size_t fromlen)
{
  tor_assert(env);
  tor_assert(env);
  tor_assert(from);
  tor_assert(fromlen);
  tor_assert(to);
  tor_assert(fromlen < SIZE_T_CEILING);

  memcpy(to, from, fromlen);
  aes_crypt_inplace(env, to, fromlen);
  return 0;
}

/** Decrypt <b>fromlen</b> bytes from <b>from</b> using the cipher
 * <b>env</b>; on success, store the result to <b>to</b> and return 0.
 * Does not check for failure.
 */
int
crypto_cipher_decrypt(crypto_cipher_t *env, char *to,
                      const char *from, size_t fromlen)
{
  tor_assert(env);
  tor_assert(from);
  tor_assert(to);
  tor_assert(fromlen < SIZE_T_CEILING);

  memcpy(to, from, fromlen);
  aes_crypt_inplace(env, to, fromlen);
  return 0;
}

/** Encrypt <b>len</b> bytes on <b>from</b> using the cipher in <b>env</b>;
 * on success. Does not check for failure.
 */
void
crypto_cipher_crypt_inplace(crypto_cipher_t *env, char *buf, size_t len)
{
  tor_assert(len < SIZE_T_CEILING);
  aes_crypt_inplace(env, buf, len);
}

/** Encrypt <b>fromlen</b> bytes (at least 1) from <b>from</b> with the key in
 * <b>key</b> to the buffer in <b>to</b> of length
 * <b>tolen</b>. <b>tolen</b> must be at least <b>fromlen</b> plus
 * CIPHER_IV_LEN bytes for the initialization vector. On success, return the
 * number of bytes written, on failure, return -1.
 */
int
crypto_cipher_encrypt_with_iv(const char *key,
                              char *to, size_t tolen,
                              const char *from, size_t fromlen)
{
  crypto_cipher_t *cipher;
  tor_assert(from);
  tor_assert(to);
  tor_assert(fromlen < INT_MAX);

  if (fromlen < 1)
    return -1;
  if (tolen < fromlen + CIPHER_IV_LEN)
    return -1;

  char iv[CIPHER_IV_LEN];
  crypto_rand(iv, sizeof(iv));
  cipher = crypto_cipher_new_with_iv(key, iv);

  memcpy(to, iv, CIPHER_IV_LEN);
  crypto_cipher_encrypt(cipher, to+CIPHER_IV_LEN, from, fromlen);
  crypto_cipher_free(cipher);
  memwipe(iv, 0, sizeof(iv));
  return (int)(fromlen + CIPHER_IV_LEN);
}

/** Decrypt <b>fromlen</b> bytes (at least 1+CIPHER_IV_LEN) from <b>from</b>
 * with the key in <b>key</b> to the buffer in <b>to</b> of length
 * <b>tolen</b>. <b>tolen</b> must be at least <b>fromlen</b> minus
 * CIPHER_IV_LEN bytes for the initialization vector. On success, return the
 * number of bytes written, on failure, return -1.
 */
int
crypto_cipher_decrypt_with_iv(const char *key,
                              char *to, size_t tolen,
                              const char *from, size_t fromlen)
{
  crypto_cipher_t *cipher;
  tor_assert(key);
  tor_assert(from);
  tor_assert(to);
  tor_assert(fromlen < INT_MAX);

  if (fromlen <= CIPHER_IV_LEN)
    return -1;
  if (tolen < fromlen - CIPHER_IV_LEN)
    return -1;

  cipher = crypto_cipher_new_with_iv(key, from);

  crypto_cipher_encrypt(cipher, to, from+CIPHER_IV_LEN, fromlen-CIPHER_IV_LEN);
  crypto_cipher_free(cipher);
  return (int)(fromlen - CIPHER_IV_LEN);
}
