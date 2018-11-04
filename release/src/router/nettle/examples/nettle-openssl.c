/* nettle-openssl.c

   Glue that's used only by the benchmark, and subject to change.

   Copyright (C) 2002, 2017 Niels Möller
   Copyright (C) 2017 Red Hat, Inc.

   This file is part of GNU Nettle.

   GNU Nettle is free software: you can redistribute it and/or
   modify it under the terms of either:

     * the GNU Lesser General Public License as published by the Free
       Software Foundation; either version 3 of the License, or (at your
       option) any later version.

   or

     * the GNU General Public License as published by the Free
       Software Foundation; either version 2 of the License, or (at your
       option) any later version.

   or both in parallel, as here.

   GNU Nettle is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received copies of the GNU General Public License and
   the GNU Lesser General Public License along with this program.  If
   not, see http://www.gnu.org/licenses/.
*/

#if HAVE_CONFIG_H
# include "config.h"
#endif

/* Openssl glue, for comparative benchmarking only */

#if WITH_OPENSSL

/* No ancient ssleay compatibility */
#define NCOMPAT
#define OPENSSL_DISABLE_OLD_DES_SUPPORT

#include <assert.h>

#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>

#include <openssl/md5.h>
#include <openssl/sha.h>

#include "nettle-internal.h"

/* We use Openssl's EVP api for all openssl ciphers. This API selects
   platform-specific implementations if appropriate, e.g., using x86
   AES-NI instructions. */
struct openssl_cipher_ctx {
  EVP_CIPHER_CTX *evp;
};

void
nettle_openssl_init(void)
{
  ERR_load_crypto_strings();
  OpenSSL_add_all_algorithms();
#if OPENSSL_VERSION_NUMBER >= 0x1010000
  CONF_modules_load_file(NULL, NULL, 0);
#else
  OPENSSL_config(NULL);
#endif
}

static void
openssl_evp_set_encrypt_key(void *p, const uint8_t *key,
			    const EVP_CIPHER *cipher)
{
  struct openssl_cipher_ctx *ctx = p;
  ctx->evp = EVP_CIPHER_CTX_new();
  assert(EVP_EncryptInit_ex(ctx->evp, cipher, NULL, key, NULL) == 1);
  EVP_CIPHER_CTX_set_padding(ctx->evp, 0);
}
static void
openssl_evp_set_decrypt_key(void *p, const uint8_t *key,
			    const EVP_CIPHER *cipher)
{
  struct openssl_cipher_ctx *ctx = p;
  ctx->evp = EVP_CIPHER_CTX_new();
  assert(EVP_DecryptInit_ex(ctx->evp, cipher, NULL, key, NULL) == 1);
  EVP_CIPHER_CTX_set_padding(ctx->evp, 0);
}

static void
openssl_evp_encrypt(const void *p, size_t length,
		    uint8_t *dst, const uint8_t *src)
{
  const struct openssl_cipher_ctx *ctx = p;
  int len;
  assert(EVP_EncryptUpdate(ctx->evp, dst, &len, src, length) == 1);
}
static void
openssl_evp_decrypt(const void *p, size_t length,
		    uint8_t *dst, const uint8_t *src)
{
  const struct openssl_cipher_ctx *ctx = p;
  int len;
  assert(EVP_DecryptUpdate(ctx->evp, dst, &len, src, length) == 1);
}

/* AES */
static nettle_set_key_func openssl_aes128_set_encrypt_key;
static nettle_set_key_func openssl_aes128_set_decrypt_key;
static nettle_set_key_func openssl_aes192_set_encrypt_key;
static nettle_set_key_func openssl_aes192_set_decrypt_key;
static nettle_set_key_func openssl_aes256_set_encrypt_key;
static nettle_set_key_func openssl_aes256_set_decrypt_key;

static void
openssl_aes128_set_encrypt_key(void *ctx, const uint8_t *key)
{
  openssl_evp_set_encrypt_key(ctx, key, EVP_aes_128_ecb());
}
static void
openssl_aes128_set_decrypt_key(void *ctx, const uint8_t *key)
{
  openssl_evp_set_decrypt_key(ctx, key, EVP_aes_128_ecb());
}

static void
openssl_aes192_set_encrypt_key(void *ctx, const uint8_t *key)
{
  openssl_evp_set_encrypt_key(ctx, key, EVP_aes_192_ecb());
}
static void
openssl_aes192_set_decrypt_key(void *ctx, const uint8_t *key)
{
  openssl_evp_set_decrypt_key(ctx, key, EVP_aes_192_ecb());
}

static void
openssl_aes256_set_encrypt_key(void *ctx, const uint8_t *key)
{
  openssl_evp_set_encrypt_key(ctx, key, EVP_aes_256_ecb());
}
static void
openssl_aes256_set_decrypt_key(void *ctx, const uint8_t *key)
{
  openssl_evp_set_decrypt_key(ctx, key, EVP_aes_256_ecb());
}

const struct nettle_cipher
nettle_openssl_aes128 = {
  "openssl aes128", sizeof(struct openssl_cipher_ctx),
  16, 16,
  openssl_aes128_set_encrypt_key, openssl_aes128_set_decrypt_key,
  openssl_evp_encrypt, openssl_evp_decrypt
};

const struct nettle_cipher
nettle_openssl_aes192 = {
  "openssl aes192", sizeof(struct openssl_cipher_ctx),
  16, 24,
  openssl_aes192_set_encrypt_key, openssl_aes192_set_decrypt_key,
  openssl_evp_encrypt, openssl_evp_decrypt
};

const struct nettle_cipher
nettle_openssl_aes256 = {
  "openssl aes256", sizeof(struct openssl_cipher_ctx),
  16, 32,
  openssl_aes256_set_encrypt_key, openssl_aes256_set_decrypt_key,
  openssl_evp_encrypt, openssl_evp_decrypt
};

/* Arcfour */
static void
openssl_arcfour128_set_encrypt_key(void *ctx, const uint8_t *key)
{
  openssl_evp_set_encrypt_key(ctx, key, EVP_rc4());
}

static void
openssl_arcfour128_set_decrypt_key(void *ctx, const uint8_t *key)
{
  openssl_evp_set_decrypt_key(ctx, key, EVP_rc4());
}

const struct nettle_aead
nettle_openssl_arcfour128 = {
  "openssl arcfour128", sizeof(struct openssl_cipher_ctx),
  1, 16, 0, 0,
  openssl_arcfour128_set_encrypt_key,
  openssl_arcfour128_set_decrypt_key,
  NULL, NULL,
  (nettle_crypt_func *)openssl_evp_encrypt,
  (nettle_crypt_func *)openssl_evp_decrypt,
  NULL,  
};

/* Blowfish */
static void
openssl_bf128_set_encrypt_key(void *ctx, const uint8_t *key)
{
  openssl_evp_set_encrypt_key(ctx, key, EVP_bf_ecb());
}

static void
openssl_bf128_set_decrypt_key(void *ctx, const uint8_t *key)
{
  openssl_evp_set_decrypt_key(ctx, key, EVP_bf_ecb());
}

const struct nettle_cipher
nettle_openssl_blowfish128 = {
  "openssl bf128", sizeof(struct openssl_cipher_ctx),
  8, 16,
  openssl_bf128_set_encrypt_key, openssl_bf128_set_decrypt_key,
  openssl_evp_encrypt, openssl_evp_decrypt
};


/* DES */
static void
openssl_des_set_encrypt_key(void *ctx, const uint8_t *key)
{
  openssl_evp_set_encrypt_key(ctx, key, EVP_des_ecb());
}

static void
openssl_des_set_decrypt_key(void *ctx, const uint8_t *key)
{
  openssl_evp_set_decrypt_key(ctx, key, EVP_des_ecb());
}

const struct nettle_cipher
nettle_openssl_des = {
  "openssl des", sizeof(struct openssl_cipher_ctx),
  8, 8,
  openssl_des_set_encrypt_key, openssl_des_set_decrypt_key,
  openssl_evp_encrypt, openssl_evp_decrypt
};


/* Cast128 */
static void
openssl_cast128_set_encrypt_key(void *ctx, const uint8_t *key)
{
  openssl_evp_set_encrypt_key(ctx, key, EVP_cast5_ecb());
}

static void
openssl_cast128_set_decrypt_key(void *ctx, const uint8_t *key)
{
  openssl_evp_set_decrypt_key(ctx, key, EVP_cast5_ecb());
}

const struct nettle_cipher
nettle_openssl_cast128 = {
  "openssl cast128", sizeof(struct openssl_cipher_ctx),
  8, 16,
  openssl_cast128_set_encrypt_key, openssl_cast128_set_decrypt_key,
  openssl_evp_encrypt, openssl_evp_decrypt
};

/* Hash functions */

/* md5 */
static nettle_hash_init_func openssl_md5_init;
static void
openssl_md5_init(void *ctx)
{
  MD5_Init(ctx);
}

static nettle_hash_update_func openssl_md5_update;
static void
openssl_md5_update(void *ctx,
		   size_t length,
		   const uint8_t *src)
{
  MD5_Update(ctx, src, length);
}

static nettle_hash_digest_func openssl_md5_digest;
static void
openssl_md5_digest(void *ctx,
		   size_t length, uint8_t *dst)
{
  assert(length == SHA_DIGEST_LENGTH);
  MD5_Final(dst, ctx);
  MD5_Init(ctx);
}

const struct nettle_hash
nettle_openssl_md5 = {
  "openssl md5", sizeof(SHA_CTX),
  SHA_DIGEST_LENGTH, SHA_CBLOCK,
  openssl_md5_init,
  openssl_md5_update,
  openssl_md5_digest
};

/* sha1 */
static nettle_hash_init_func openssl_sha1_init;
static void
openssl_sha1_init(void *ctx)
{
  SHA1_Init(ctx);
}

static nettle_hash_update_func openssl_sha1_update;
static void
openssl_sha1_update(void *ctx,
		    size_t length,
		    const uint8_t *src)
{
  SHA1_Update(ctx, src, length);
}

static nettle_hash_digest_func openssl_sha1_digest;
static void
openssl_sha1_digest(void *ctx,
		    size_t length, uint8_t *dst)
{
  assert(length == SHA_DIGEST_LENGTH);
  SHA1_Final(dst, ctx);
  SHA1_Init(ctx);
}

const struct nettle_hash
nettle_openssl_sha1 = {
  "openssl sha1", sizeof(SHA_CTX),
  SHA_DIGEST_LENGTH, SHA_CBLOCK,
  openssl_sha1_init,
  openssl_sha1_update,
  openssl_sha1_digest
};
  
#endif /* WITH_OPENSSL */
