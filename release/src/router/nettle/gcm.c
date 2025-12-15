/* gcm.c

   Galois counter mode, specified by NIST,
   http://csrc.nist.gov/publications/nistpubs/800-38D/SP-800-38D.pdf

   See also the gcm paper at
   http://www.cryptobarn.com/papers/gcm-spec.pdf.

   Copyright (C) 2011 Katholieke Universiteit Leuven
   Copyright (C) 2011, 2013, 2018 Niels Möller
   Copyright (C) 2018 Red Hat, Inc.
   
   Contributed by Nikos Mavrogiannopoulos

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

#include "gcm.h"

#include "ghash-internal.h"
#include "memxor.h"
#include "nettle-internal.h"
#include "macros.h"
#include "ctr-internal.h"
#include "block-internal.h"
#include "bswap-internal.h"

/* Initialization of GCM.
 * @ctx: The context of GCM
 * @cipher: The context of the underlying block cipher
 * @f: The underlying cipher encryption function
 */
void
gcm_set_key(struct gcm_key *key,
	    const void *cipher, nettle_cipher_func *f)
{
  static const union nettle_block16 zero_block;
  union nettle_block16 key_block;
  f (cipher, GCM_BLOCK_SIZE, key_block.b, zero_block.b);

  _ghash_set_key (key, &key_block);
}

/* Call _ghash_update, with zero padding of any partial final block. */
static void
gcm_hash (const struct gcm_key *key, union nettle_block16 *x,
	  size_t length, const uint8_t *data) {
  data = _ghash_update (key, x, length / GCM_BLOCK_SIZE, data);
  length &= (GCM_BLOCK_SIZE - 1);
  if (length > 0)
    {
      union nettle_block16 block;
      block16_zero (&block);
      memcpy (block.b, data, length);
      _ghash_update (key, x, 1, block.b);
    }
}

static void
gcm_hash_sizes(const struct gcm_key *key, union nettle_block16 *x,
	       uint64_t auth_size, uint64_t data_size)
{
  union nettle_block16 buffer;

  data_size *= 8;
  auth_size *= 8;

  buffer.u64[0] = bswap64_if_le (auth_size);
  buffer.u64[1] = bswap64_if_le (data_size);

  _ghash_update (key, x, 1, buffer.b);
}

/* NOTE: The key is needed only if length != GCM_IV_SIZE */
void
gcm_set_iv(struct gcm_ctx *ctx, const struct gcm_key *key,
	   size_t length, const uint8_t *iv)
{
  if (length == GCM_IV_SIZE)
    {
      memcpy (ctx->iv.b, iv, GCM_BLOCK_SIZE - 4);
      ctx->iv.b[GCM_BLOCK_SIZE - 4] = 0;
      ctx->iv.b[GCM_BLOCK_SIZE - 3] = 0;
      ctx->iv.b[GCM_BLOCK_SIZE - 2] = 0;
      ctx->iv.b[GCM_BLOCK_SIZE - 1] = 1;
    }
  else
    {
      block16_zero(&ctx->iv);
      gcm_hash(key, &ctx->iv, length, iv);
      gcm_hash_sizes(key, &ctx->iv, 0, length);
    }

  ctx->ctr = ctx->iv;
  /* Increment the rightmost 32 bits. */
  INCREMENT (4, ctx->ctr.b + GCM_BLOCK_SIZE - 4);

  /* Reset the rest of the message-dependent state. */
  block16_zero(&ctx->x);
  ctx->auth_size = ctx->data_size = 0;
}

void
gcm_update(struct gcm_ctx *ctx, const struct gcm_key *key,
	   size_t length, const uint8_t *data)
{
  assert(ctx->auth_size % GCM_BLOCK_SIZE == 0);
  assert(ctx->data_size == 0);

  gcm_hash(key, &ctx->x, length, data);

  ctx->auth_size += length;
}

static nettle_fill16_func gcm_fill;
#if WORDS_BIGENDIAN
static void
gcm_fill(uint8_t *ctr, size_t blocks, union nettle_block16 *buffer)
{
  uint64_t hi, mid;
  uint32_t lo;
  size_t i;
  hi = READ_UINT64(ctr);
  mid = (uint64_t) READ_UINT32(ctr + 8) << 32;
  lo = READ_UINT32(ctr + 12);

  for (i = 0; i < blocks; i++)
    {
      buffer[i].u64[0] = hi;
      buffer[i].u64[1] = mid + lo++;
    }
  WRITE_UINT32(ctr + 12, lo);

}
#elif HAVE_BUILTIN_BSWAP64
/* Assume __builtin_bswap32 is also available */
static void
gcm_fill(uint8_t *ctr, size_t blocks, union nettle_block16 *buffer)
{
  uint64_t hi, mid;
  uint32_t lo;
  size_t i;
  hi = LE_READ_UINT64(ctr);
  mid = LE_READ_UINT32(ctr + 8);
  lo = READ_UINT32(ctr + 12);

  for (i = 0; i < blocks; i++)
    {
      buffer[i].u64[0] = hi;
      buffer[i].u64[1] = mid + ((uint64_t)__builtin_bswap32(lo) << 32);
      lo++;
    }
  WRITE_UINT32(ctr + 12, lo);
}
#else
static void
gcm_fill(uint8_t *ctr, size_t blocks, union nettle_block16 *buffer)
{
  uint32_t c;

  c = READ_UINT32(ctr + GCM_BLOCK_SIZE - 4);

  for (; blocks-- > 0; buffer++, c++)
    {
      memcpy(buffer->b, ctr, GCM_BLOCK_SIZE - 4);
      WRITE_UINT32(buffer->b + GCM_BLOCK_SIZE - 4, c);
    }

  WRITE_UINT32(ctr + GCM_BLOCK_SIZE - 4, c);
}
#endif

void
gcm_encrypt (struct gcm_ctx *ctx, const struct gcm_key *key,
	     const void *cipher, nettle_cipher_func *f,
	     size_t length, uint8_t *dst, const uint8_t *src)
{
  assert(ctx->data_size % GCM_BLOCK_SIZE == 0);

  _nettle_ctr_crypt16(cipher, f, gcm_fill, ctx->ctr.b, length, dst, src);
  gcm_hash(key, &ctx->x, length, dst);

  ctx->data_size += length;
}

void
gcm_decrypt(struct gcm_ctx *ctx, const struct gcm_key *key,
	    const void *cipher, nettle_cipher_func *f,
	    size_t length, uint8_t *dst, const uint8_t *src)
{
  assert(ctx->data_size % GCM_BLOCK_SIZE == 0);

  gcm_hash(key, &ctx->x, length, src);
  _nettle_ctr_crypt16(cipher, f, gcm_fill, ctx->ctr.b, length, dst, src);

  ctx->data_size += length;
}

void
gcm_digest(struct gcm_ctx *ctx, const struct gcm_key *key,
	   const void *cipher, nettle_cipher_func *f,
	   size_t length, uint8_t *digest)
{
  union nettle_block16 buffer;

  assert (length <= GCM_BLOCK_SIZE);

  gcm_hash_sizes(key, &ctx->x, ctx->auth_size, ctx->data_size);

  f (cipher, GCM_BLOCK_SIZE, buffer.b, ctx->iv.b);
  block16_xor (&buffer, &ctx->x);
  memcpy (digest, buffer.b, length);

  return;
}
