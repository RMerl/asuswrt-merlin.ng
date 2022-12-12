/* nettle-internal.c

   Things that are used only by the testsuite and benchmark, and
   not included in the library.

   Copyright (C) 2002, 2014 Niels Möller

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

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "nettle-internal.h"
#include "arcfour.h"
#include "blowfish.h"
#include "cbc.h"
#include "chacha.h"
#include "des.h"
#include "salsa20.h"

/* Wrapper functions discarding the return value. Needed for the
   ciphers with weak keys. */
static void
des_set_key_wrapper (void *ctx, const uint8_t *key)
{
  des_set_key (ctx, key);
}

static void
des3_set_key_wrapper (void *ctx, const uint8_t *key)
{
  des3_set_key (ctx, key);
}

static void
blowfish128_set_key_wrapper (void *ctx, const uint8_t *key)
{
  blowfish128_set_key (ctx, key);
}

const struct nettle_cipher
nettle_des = {
  "des", sizeof(struct des_ctx),
  DES_BLOCK_SIZE, DES_KEY_SIZE,
  des_set_key_wrapper,
  des_set_key_wrapper,
  (nettle_cipher_func *) des_encrypt,
  (nettle_cipher_func *) des_decrypt
};

const struct nettle_cipher
nettle_des3 = {
 "des3", sizeof(struct des3_ctx),
 DES3_BLOCK_SIZE, DES3_KEY_SIZE,
 des3_set_key_wrapper,
 des3_set_key_wrapper,
 (nettle_cipher_func *) des3_encrypt,
 (nettle_cipher_func *) des3_decrypt
};

const struct nettle_cipher
nettle_blowfish128 =
  { "blowfish128", sizeof(struct blowfish_ctx),
    BLOWFISH_BLOCK_SIZE, BLOWFISH128_KEY_SIZE,
    blowfish128_set_key_wrapper,
    blowfish128_set_key_wrapper,
    (nettle_cipher_func *) blowfish_encrypt,
    (nettle_cipher_func *) blowfish_decrypt
  };

const struct nettle_aead
nettle_arcfour128 = {
  "arcfour128", sizeof(struct arcfour_ctx),
  1, ARCFOUR128_KEY_SIZE, 0, 0,
  (nettle_set_key_func *) arcfour128_set_key,
  (nettle_set_key_func *) arcfour128_set_key,
  NULL, NULL,
  (nettle_crypt_func *) arcfour_crypt,
  (nettle_crypt_func *) arcfour_crypt,
  NULL,  
};

const struct nettle_aead
nettle_chacha = {
  "chacha", sizeof(struct chacha_ctx),
  CHACHA_BLOCK_SIZE, CHACHA_KEY_SIZE,
  CHACHA_NONCE_SIZE, 0,
  (nettle_set_key_func *) chacha_set_key,
  (nettle_set_key_func *) chacha_set_key,
  (nettle_set_key_func *) chacha_set_nonce,
  NULL,
  (nettle_crypt_func *) chacha_crypt,
  (nettle_crypt_func *) chacha_crypt,
  NULL,
};

const struct nettle_aead
nettle_salsa20 = {
  "salsa20", sizeof(struct salsa20_ctx),
  SALSA20_BLOCK_SIZE, SALSA20_256_KEY_SIZE,
  SALSA20_NONCE_SIZE, 0,
  (nettle_set_key_func *) salsa20_256_set_key,
  (nettle_set_key_func *) salsa20_256_set_key,
  (nettle_set_key_func *) salsa20_set_nonce,
  NULL,
  (nettle_crypt_func *) salsa20_crypt,
  (nettle_crypt_func *) salsa20_crypt,
  NULL,
};

const struct nettle_aead
nettle_salsa20r12 = {
  "salsa20r12", sizeof(struct salsa20_ctx),
  SALSA20_BLOCK_SIZE, SALSA20_256_KEY_SIZE,
  SALSA20_NONCE_SIZE, 0,
  (nettle_set_key_func*) salsa20_256_set_key,
  (nettle_set_key_func*) salsa20_256_set_key,
  (nettle_set_key_func*) salsa20_set_nonce,
  NULL,
  (nettle_crypt_func *) salsa20r12_crypt,
  (nettle_crypt_func *) salsa20r12_crypt,
  NULL,
};

struct cbc_aes128_ctx CBC_CTX(struct aes128_ctx, AES_BLOCK_SIZE);
static void
cbc_aes128_set_encrypt_key(struct cbc_aes128_ctx *ctx, const uint8_t *key)
{
  aes128_set_encrypt_key(&ctx->ctx, key);
}
static void
cbc_aes128_set_iv(struct cbc_aes128_ctx *ctx, const uint8_t *iv)
{
  CBC_SET_IV(ctx, iv);
}
static void
cbc_aes128_encrypt_wrapper(struct cbc_aes128_ctx *ctx,
			   size_t length, uint8_t *dst,
			   const uint8_t *src)
{
  cbc_aes128_encrypt(&ctx->ctx, ctx->iv, length, dst, src);
}

const struct nettle_aead
nettle_cbc_aes128 = {
  "cbc_aes128", sizeof(struct cbc_aes128_ctx),
  AES_BLOCK_SIZE, AES128_KEY_SIZE,
  AES_BLOCK_SIZE, 0,
  (nettle_set_key_func*) cbc_aes128_set_encrypt_key,
  NULL,
  (nettle_set_key_func*) cbc_aes128_set_iv,
  NULL,
  (nettle_crypt_func *) cbc_aes128_encrypt_wrapper,
  NULL,
  NULL,
};

struct cbc_aes192_ctx CBC_CTX(struct aes192_ctx, AES_BLOCK_SIZE);
static void
cbc_aes192_set_encrypt_key(struct cbc_aes192_ctx *ctx, const uint8_t *key)
{
  aes192_set_encrypt_key(&ctx->ctx, key);
}
static void
cbc_aes192_set_iv(struct cbc_aes192_ctx *ctx, const uint8_t *iv)
{
  CBC_SET_IV(ctx, iv);
}
static void
cbc_aes192_encrypt_wrapper(struct cbc_aes192_ctx *ctx,
			   size_t length, uint8_t *dst,
			   const uint8_t *src)
{
  cbc_aes192_encrypt(&ctx->ctx, ctx->iv, length, dst, src);
}
const struct nettle_aead
nettle_cbc_aes192 = {
  "cbc_aes192", sizeof(struct cbc_aes192_ctx),
  AES_BLOCK_SIZE, AES192_KEY_SIZE,
  AES_BLOCK_SIZE, 0,
  (nettle_set_key_func*) cbc_aes192_set_encrypt_key,
  NULL,
  (nettle_set_key_func*) cbc_aes192_set_iv,
  NULL,
  (nettle_crypt_func *) cbc_aes192_encrypt_wrapper,
  NULL,
  NULL,
};

struct cbc_aes256_ctx CBC_CTX(struct aes256_ctx, AES_BLOCK_SIZE);
static void
cbc_aes256_set_encrypt_key(struct cbc_aes256_ctx *ctx, const uint8_t *key)
{
  aes256_set_encrypt_key(&ctx->ctx, key);
}
static void
cbc_aes256_set_iv(struct cbc_aes256_ctx *ctx, const uint8_t *iv)
{
  CBC_SET_IV(ctx, iv);
}
static void
cbc_aes256_encrypt_wrapper(struct cbc_aes256_ctx *ctx,
			   size_t length, uint8_t *dst,
			   const uint8_t *src)
{
  cbc_aes256_encrypt(&ctx->ctx, ctx->iv, length, dst, src);
}
const struct nettle_aead
nettle_cbc_aes256 = {
  "cbc_aes256", sizeof(struct cbc_aes256_ctx),
  AES_BLOCK_SIZE, AES256_KEY_SIZE,
  AES_BLOCK_SIZE, 0,
  (nettle_set_key_func*) cbc_aes256_set_encrypt_key,
  NULL,
  (nettle_set_key_func*) cbc_aes256_set_iv,
  NULL,
  (nettle_crypt_func *) cbc_aes256_encrypt_wrapper,
  NULL,
  NULL,
};
