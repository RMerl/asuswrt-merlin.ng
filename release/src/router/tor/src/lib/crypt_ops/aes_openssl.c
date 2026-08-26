/* Copyright (c) 2001, Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file aes_openssl.c
 * \brief Use OpenSSL to implement AES_CTR.
 **/

#define USE_AES_RAW
#define TOR_AES_PRIVATE

#include "orconfig.h"
#include "lib/crypt_ops/aes.h"
#include "lib/crypt_ops/crypto_util.h"
#include "lib/log/util_bug.h"
#include "lib/arch/bytes.h"

#ifdef _WIN32 /*wrkard for dtls1.h >= 0.9.8m of "#include <winsock.h>"*/
  #include <winsock2.h>
  #include <ws2tcpip.h>
#endif

#include "lib/crypt_ops/compat_openssl.h"
#include <openssl/opensslv.h>
#include "lib/crypt_ops/crypto_openssl_mgt.h"

DISABLE_GCC_WARNING("-Wredundant-decls")

#include <stdlib.h>
#include <string.h>
#include <openssl/aes.h>
#include <openssl/evp.h>
#include <openssl/engine.h>
#include <openssl/modes.h>

ENABLE_GCC_WARNING("-Wredundant-decls")

#include "lib/log/log.h"
#include "lib/ctime/di_ops.h"

/* Cached values of our EVP_CIPHER items.  If we don't pre-fetch them,
 * then EVP_CipherInit calls EVP_CIPHER_fetch itself,
 * which is surprisingly expensive.
 */
static const EVP_CIPHER *aes128ctr = NULL;
static const EVP_CIPHER *aes192ctr = NULL;
static const EVP_CIPHER *aes256ctr = NULL;
static const EVP_CIPHER *aes128ecb = NULL;
static const EVP_CIPHER *aes192ecb = NULL;
static const EVP_CIPHER *aes256ecb = NULL;

#if OPENSSL_VERSION_NUMBER >= OPENSSL_V_NOPATCH(3,0,0) \
  && !defined(LIBRESSL_VERSION_NUMBER)
#define RESOLVE_CIPHER(c) \
  EVP_CIPHER_fetch(NULL, OBJ_nid2sn(EVP_CIPHER_get_nid(c)), "")
#else
#define RESOLVE_CIPHER(c) (c)
#endif

/**
 * Pre-fetch the versions of every AES cipher with its associated provider.
 */
static void
init_ciphers(void)
{
  aes128ctr = RESOLVE_CIPHER(EVP_aes_128_ctr());
  aes192ctr = RESOLVE_CIPHER(EVP_aes_192_ctr());
  aes256ctr = RESOLVE_CIPHER(EVP_aes_256_ctr());
  aes128ecb = RESOLVE_CIPHER(EVP_aes_128_ecb());
  aes192ecb = RESOLVE_CIPHER(EVP_aes_192_ecb());
  aes256ecb = RESOLVE_CIPHER(EVP_aes_256_ecb());
}
#define INIT_CIPHERS() STMT_BEGIN { \
    if (PREDICT_UNLIKELY(NULL == aes128ctr)) {  \
      init_ciphers();                           \
    }                                           \
  } STMT_END

/* We have 2 strategies for getting the AES block cipher: Via OpenSSL's
 * AES_encrypt function, or via OpenSSL's EVP_EncryptUpdate function.
 *
 * If there's any hardware acceleration in play, we want to be using EVP_* so
 * we can get it.  Otherwise, we'll want AES_*, which seems to be about 5%
 * faster than indirecting through the EVP layer.
 */

/* We have 2 strategies for getting a plug-in counter mode: use our own, or
 * use OpenSSL's.
 *
 * Here we have a counter mode that's faster than the one shipping with
 * OpenSSL pre-1.0 (by about 10%!).  But OpenSSL 1.0.0 added a counter mode
 * implementation faster than the one here (by about 7%).  So we pick which
 * one to used based on the Openssl version above.  (OpenSSL 1.0.0a fixed a
 * critical bug in that counter mode implementation, so we need to test to
 * make sure that we have a fixed version.)
 */

/* We don't actually define the struct here. */

aes_cnt_cipher_t *
aes_new_cipher(const uint8_t *key, const uint8_t *iv, int key_bits)
{
  INIT_CIPHERS();
  EVP_CIPHER_CTX *cipher = EVP_CIPHER_CTX_new();
  const EVP_CIPHER *c = NULL;
  switch (key_bits) {
    case 128: c = aes128ctr; break;
    case 192: c = aes192ctr; break;
    case 256: c = aes256ctr; break;
    default: tor_assert_unreached(); // LCOV_EXCL_LINE
  }
  EVP_EncryptInit(cipher, c, key, iv);
  return (aes_cnt_cipher_t *) cipher;
}
void
aes_cipher_free_(aes_cnt_cipher_t *cipher_)
{
  if (!cipher_)
    return;
  EVP_CIPHER_CTX *cipher = (EVP_CIPHER_CTX *) cipher_;
  EVP_CIPHER_CTX_reset(cipher);
  EVP_CIPHER_CTX_free(cipher);
}

/** Changes the key of the cipher;
 * sets the IV to 0.
 */
void
aes_cipher_set_key(aes_cnt_cipher_t *cipher_, const uint8_t *key, int key_bits)
{
  EVP_CIPHER_CTX *cipher = (EVP_CIPHER_CTX *) cipher_;
  uint8_t iv[16] = {0};
  const EVP_CIPHER *c = NULL;
  switch (key_bits) {
    case 128: c = aes128ctr; break;
    case 192: c = aes192ctr; break;
    case 256: c = aes256ctr; break;
    default: tor_assert_unreached(); // LCOV_EXCL_LINE
  }

  // No need to call EVP_CIPHER_CTX_Reset here; EncryptInit already
  // does it for us.
  EVP_EncryptInit(cipher, c, key, iv);
}
/** Change the IV of this stream cipher without changing the key.
 *
 * Requires that the cipher stream position is at an even multiple of 16 bytes.
 */
void
aes_cipher_set_iv_aligned(aes_cnt_cipher_t *cipher_, const uint8_t *iv)
{
  EVP_CIPHER_CTX *cipher = (EVP_CIPHER_CTX *) cipher_;
#ifdef LIBRESSL_VERSION_NUMBER
  EVP_CIPHER_CTX_set_iv(cipher, iv, 16);
#else
  // We would have to do this if the cipher's position were not aligned:
  // EVP_CIPHER_CTX_set_num(cipher, 0);

  memcpy(EVP_CIPHER_CTX_iv_noconst(cipher), iv, 16);
#endif
}
void
aes_crypt_inplace(aes_cnt_cipher_t *cipher_, char *data, size_t len)
{
  int outl;
  EVP_CIPHER_CTX *cipher = (EVP_CIPHER_CTX *) cipher_;

  tor_assert(len < INT_MAX);

  EVP_EncryptUpdate(cipher, (unsigned char*)data,
                    &outl, (unsigned char*)data, (int)len);
}

/* ========
 * Functions for "raw" (ECB) AES.
 *
 * I'm choosing the name "raw" here because ECB is not a mode;
 * it's a disaster.  The only way to use this safely is
 * within a real construction.
 */

/**
 * Create a new instance of AES using a key of length 'key_bits'
 * for raw block encryption.
 *
 * This is even more low-level than counter-mode, and you should
 * only use it with extreme caution.
 */
aes_raw_t *
aes_raw_new(const uint8_t *key, int key_bits, bool encrypt)
{
  INIT_CIPHERS();
  EVP_CIPHER_CTX *cipher = EVP_CIPHER_CTX_new();
  tor_assert(cipher);
  const EVP_CIPHER *c = NULL;
  switch (key_bits) {
    case 128: c = aes128ecb; break;
    case 192: c = aes192ecb; break;
    case 256: c = aes256ecb; break;
    default: tor_assert_unreached();
  }

  // No need to call EVP_CIPHER_CTX_Reset here; EncryptInit already
  // does it for us.
  int r = EVP_CipherInit(cipher, c, key, NULL, encrypt);
  tor_assert(r == 1);
  EVP_CIPHER_CTX_set_padding(cipher, 0);
  return (aes_raw_t *)cipher;
}
/**
 * Replace the key on an existing aes_raw_t.
 *
 * This may be faster than freeing and reallocating.
 */
void
aes_raw_set_key(aes_raw_t **cipher_, const uint8_t *key,
                int key_bits, bool encrypt)
{
  const EVP_CIPHER *c = *(EVP_CIPHER**) cipher_;
  switch (key_bits) {
    case 128: c = aes128ecb; break;
    case 192: c = aes192ecb; break;
    case 256: c = aes256ecb; break;
    default: tor_assert_unreached();
  }
  aes_raw_t *cipherp = *cipher_;
  EVP_CIPHER_CTX *cipher = (EVP_CIPHER_CTX *)cipherp;
  int r = EVP_CipherInit(cipher, c, key, NULL, encrypt);
  tor_assert(r == 1);
  EVP_CIPHER_CTX_set_padding(cipher, 0);
}

/**
 * Release storage held by 'cipher'.
 */
void
aes_raw_free_(aes_raw_t *cipher_)
{
  if (!cipher_)
    return;
  EVP_CIPHER_CTX *cipher = (EVP_CIPHER_CTX *)cipher_;
#ifdef OPENSSL_1_1_API
  EVP_CIPHER_CTX_reset(cipher);
#else
  EVP_CIPHER_CTX_cleanup(cipher);
#endif
  EVP_CIPHER_CTX_free(cipher);
}
#define aes_raw_free(cipher) \
  FREE_AND_NULL(aes_raw_t, aes_raw_free_, (cipher))
/**
 * Encrypt a single 16-byte block with 'cipher',
 * which must have been initialized for encryption.
 */
void
aes_raw_encrypt(const aes_raw_t *cipher, uint8_t *block)
{
  int outl = 16;
  int r = EVP_EncryptUpdate((EVP_CIPHER_CTX *)cipher, block, &outl, block, 16);
  tor_assert(r == 1);
  tor_assert(outl == 16);
}
/**
 * Decrypt a single 16-byte block with 'cipher',
 * which must have been initialized for decryption.
 */
void
aes_raw_decrypt(const aes_raw_t *cipher, uint8_t *block)
{
  int outl = 16;
  int r = EVP_DecryptUpdate((EVP_CIPHER_CTX *)cipher, block, &outl, block, 16);
  tor_assert(r == 1);
  tor_assert(outl == 16);
}
