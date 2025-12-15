/* ocb.c

   OCB AEAD mode, RFC 7253

   Copyright (C) 2021 Niels Möller

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

#include <string.h>

#include "ocb.h"
#include "block-internal.h"
#include "bswap-internal.h"
#include "memops.h"

#define OCB_MAX_BLOCKS 16

/* Returns 64 bits from the concatenation (u0, u1), starting from bit offset. */
static inline uint64_t
extract(uint64_t u0, uint64_t u1, unsigned offset)
{
  if (offset == 0)
    return u0;
  u0 = bswap64_if_le(u0);
  u1 = bswap64_if_le(u1);
  return bswap64_if_le((u0 << offset) | (u1 >> (64 - offset)));
}

void
ocb_set_key (struct ocb_key *key, const void *cipher, nettle_cipher_func *f)
{
  static const union nettle_block16 zero_block;
  f (cipher, OCB_BLOCK_SIZE, key->L[0].b, zero_block.b);
  block16_mulx_be (&key->L[1], &key->L[0]);
  block16_mulx_be (&key->L[2], &key->L[1]);
}

/* Add x^k L[2], where k is the number of trailing zero bits in i. */
static void
update_offset(const struct ocb_key *key,
	      union nettle_block16 *offset, size_t i)
{
  if (i & 1)
    block16_xor (offset, &key->L[2]);
  else
    {
      assert (i > 0);
      union nettle_block16 diff;
      block16_mulx_be (&diff, &key->L[2]);
      for (i >>= 1; !(i&1); i >>= 1)
	block16_mulx_be (&diff, &diff);

      block16_xor (offset, &diff);
    }
}

static void
pad_block (union nettle_block16 *block, size_t length, const uint8_t *data)
{
  memcpy (block->b, data, length);
  block->b[length] = 0x80;
  memset (block->b + length + 1, 0, OCB_BLOCK_SIZE - 1 - length);
}

void
ocb_set_nonce (struct ocb_ctx *ctx,
	       const void *cipher, nettle_cipher_func *f,
	       size_t tag_length,
	       size_t nonce_length, const uint8_t *nonce)
{
  union nettle_block16 top;
  uint64_t stretch;

  unsigned bottom;
  assert (nonce_length < 16);
  assert (tag_length > 0);
  assert (tag_length <= 16);

  /* Bit size, or zero for tag_length == 16 */
  top.b[0] = (tag_length & 15) << 4;
  memset (top.b + 1, 0, 15 - nonce_length);
  top.b[15 - nonce_length] |= 1;
  memcpy (top.b + 16 - nonce_length, nonce, nonce_length);
  bottom = top.b[15] & 0x3f;
  top.b[15] &= 0xc0;

  f (cipher, OCB_BLOCK_SIZE, top.b, top.b);

  stretch = top.u64[0];
#if WORDS_BIGENDIAN
  stretch ^= (top.u64[0] << 8) | (top.u64[1] >> 56);
#else
  stretch ^= (top.u64[0] >> 8) | (top.u64[1] << 56);
#endif

  ctx->initial.u64[0] = extract(top.u64[0], top.u64[1], bottom);
  ctx->initial.u64[1] = extract(top.u64[1], stretch, bottom);
  ctx->sum.u64[0] = ctx->sum.u64[1] = 0;
  ctx->checksum.u64[0] = ctx->checksum.u64[1] = 0;

  ctx->data_count = ctx->message_count = 0;
}

static void
ocb_fill_n (const struct ocb_key *key,
	    union nettle_block16 *offset, size_t count,
	    size_t n, union nettle_block16 *o)
{
  assert (n > 0);
  union nettle_block16 *prev;
  if (count & 1)
    prev = offset;
  else
    {
      /* Do a single block to align block count. */
      count++; /* Always odd. */
      block16_xor (offset, &key->L[2]);
      block16_set (&o[0], offset);
      prev = o;
      n--; o++;
    }

  for (; n >= 2; n -= 2, o += 2)
    {
      size_t i;
      count += 2; /* Always odd. */

      /* Based on trailing zeros of ctx->message_count - 1, the
         initial shift below discards a one bit. */
      block16_mulx_be (&o[0], &key->L[2]);
      for (i = count >> 1; !(i&1); i >>= 1)
	block16_mulx_be (&o[0], &o[0]);

      block16_xor (&o[0], prev);
      block16_xor3 (&o[1], &o[0], &key->L[2]);
      prev = &o[1];
    }
  block16_set(offset, prev);

  if (n > 0)
    {
      update_offset (key, offset, ++count);
      block16_set (o, offset);
    }
}

void
ocb_update (struct ocb_ctx *ctx, const struct ocb_key *key,
	    const void *cipher, nettle_cipher_func *f,
	    size_t length, const uint8_t *data)
{
  union nettle_block16 block[OCB_MAX_BLOCKS];
  size_t n = length / OCB_BLOCK_SIZE;
  assert (ctx->message_count == 0);

  if (ctx->data_count == 0)
    ctx->offset.u64[0] = ctx->offset.u64[1] = 0;

  while (n > 0)
    {
      size_t size, i;
      size_t blocks = (n <= OCB_MAX_BLOCKS) ? n
	: OCB_MAX_BLOCKS - 1 + (ctx->data_count & 1);

      ocb_fill_n (key, &ctx->offset, ctx->data_count, blocks, block);
      ctx->data_count += blocks;

      size = blocks * OCB_BLOCK_SIZE;
      memxor (block[0].b, data, size);
      f (cipher, size, block[0].b, block[0].b);
      for (i = 0; i < blocks; i++)
	block16_xor(&ctx->sum, &block[i]);

      n -= blocks; data += size;
    }

  length &= 15;
  if (length > 0)
    {
      union nettle_block16 block;
      pad_block (&block, length, data);
      block16_xor (&ctx->offset, &key->L[0]);
      block16_xor (&block, &ctx->offset);

      f (cipher, OCB_BLOCK_SIZE, block.b, block.b);
      block16_xor (&ctx->sum, &block);
    }
}

static void
ocb_crypt_n (struct ocb_ctx *ctx, const struct ocb_key *key,
	     const void *cipher, nettle_cipher_func *f,
	     size_t n, uint8_t *dst, const uint8_t *src)
{
  union nettle_block16 o[OCB_MAX_BLOCKS], block[OCB_MAX_BLOCKS];
  size_t size;

  while (n > 0)
    {
      size_t blocks = (n <= OCB_MAX_BLOCKS) ? n
	: OCB_MAX_BLOCKS - 1 + (ctx->message_count & 1);

      ocb_fill_n (key, &ctx->offset, ctx->message_count, blocks, o);
      ctx->message_count += blocks;

      size = blocks * OCB_BLOCK_SIZE;
      memxor3 (block[0].b, o[0].b, src, size);
      f (cipher, size, block[0].b, block[0].b);
      memxor3 (dst, block[0].b, o[0].b, size);

      n -= blocks; src += size; dst += size;
    }
}

/* Rotate bytes c positions to the right, in memory order. */
#if WORDS_BIGENDIAN
# define MEM_ROTATE_RIGHT(c, s0, s1) do {				\
    uint64_t __rotate_t = ((s0) >> (8*(c))) | ((s1) << (64-8*(c)));	\
    (s1) = ((s1) >> (8*(c))) | ((s0) << (64-8*(c)));			\
    (s0) = __rotate_t;							\
  } while (0)
#else
# define MEM_ROTATE_RIGHT(c, s0, s1) do {				\
    uint64_t __rotate_t = ((s0) << (8*(c))) | ((s1) >> (64-8*(c)));	\
    (s1) = ((s1) << (8*(c))) | ((s0) >> (64-8*(c)));			\
    (s0) = __rotate_t;							\
  } while (0)
#endif

/* Mask for the first c bytes in memory */
#if WORDS_BIGENDIAN
# define MEM_MASK(c) (-((uint64_t) 1 << (64 - 8*(c))))
#else
# define MEM_MASK(c) (((uint64_t) 1 << (8*(c))) - 1)
#endif

/* Checksum of n complete blocks. */
static void
ocb_checksum_n (union nettle_block16 *checksum,
		size_t n, const uint8_t *src)
{
  unsigned initial;
  uint64_t edge_word = 0;
  uint64_t s0, s1;

  if (n == 1)
    {
      memxor (checksum->b, src, OCB_BLOCK_SIZE);
      return;
    }

  /* Initial unaligned bytes. */
  initial = -(uintptr_t) src & 7;

  if (initial > 0)
    {
      /* Input not 64-bit aligned. Read initial bytes. */
      unsigned i;
      /* Edge word is read in big-endian order */
      for (i = initial; i > 0; i--)
	edge_word = (edge_word << 8) + *src++;
      n--;
    }

  /* Now src is 64-bit aligned, so do 64-bit reads. */
  for (s0 = s1 = 0 ; n > 0; n--, src += OCB_BLOCK_SIZE)
    {
      s0 ^= ((const uint64_t *) src)[0];
      s1 ^= ((const uint64_t *) src)[1];
    }
  if (initial > 0)
    {
      unsigned i;
      uint64_t mask;
      s0 ^= ((const uint64_t *) src)[0];
      for (i = 8 - initial, src += 8; i > 0; i--)
	edge_word = (edge_word << 8) + *src++;

      /* Rotate [s0, s1] right initial bytes. */
      MEM_ROTATE_RIGHT(initial, s0, s1);
      /* Add in the edge bytes.  */
      mask = MEM_MASK(initial);
      edge_word = bswap64_if_le (edge_word);
      s0 ^= (edge_word & mask);
      s1 ^= (edge_word & ~mask);
    }
  checksum->u64[0] ^= s0;
  checksum->u64[1] ^= s1;
}

void
ocb_encrypt (struct ocb_ctx *ctx, const struct ocb_key *key,
	     const void *cipher, nettle_cipher_func *f,
	     size_t length, uint8_t *dst, const uint8_t *src)
{
  size_t n = length / OCB_BLOCK_SIZE;

  if (ctx->message_count == 0)
    ctx->offset = ctx->initial;

  if (n > 0)
    {
      ocb_checksum_n (&ctx->checksum, n, src);
      ocb_crypt_n (ctx, key, cipher, f, n, dst, src);
      length &= 15;
    }
  if (length > 0)
    {
      union nettle_block16 block;

      src += n*OCB_BLOCK_SIZE; dst += n*OCB_BLOCK_SIZE;

      pad_block (&block, length, src);
      block16_xor (&ctx->checksum, &block);

      block16_xor (&ctx->offset, &key->L[0]);
      f (cipher, OCB_BLOCK_SIZE, block.b, ctx->offset.b);
      memxor3 (dst, block.b, src, length);
      ctx->message_count++;
    }
}

void
ocb_decrypt (struct ocb_ctx *ctx, const struct ocb_key *key,
	     const void *encrypt_ctx, nettle_cipher_func *encrypt,
	     const void *decrypt_ctx, nettle_cipher_func *decrypt,
	     size_t length, uint8_t *dst, const uint8_t *src)
{
  size_t n = length / OCB_BLOCK_SIZE;

  if (ctx->message_count == 0)
    ctx->offset = ctx->initial;

  if (n > 0)
    {
      ocb_crypt_n (ctx, key, decrypt_ctx, decrypt, n, dst, src);
      ocb_checksum_n (&ctx->checksum, n, dst);
      length &= 15;
    }
  if (length > 0)
    {
      union nettle_block16 block;

      src += n*OCB_BLOCK_SIZE; dst += n*OCB_BLOCK_SIZE;

      block16_xor (&ctx->offset, &key->L[0]);
      encrypt (encrypt_ctx, OCB_BLOCK_SIZE, block.b, ctx->offset.b);
      memxor3 (dst, block.b, src, length);

      pad_block (&block, length, dst);
      block16_xor (&ctx->checksum, &block);
      ctx->message_count++;
    }
}

void
ocb_digest (const struct ocb_ctx *ctx, const struct ocb_key *key,
	    const void *cipher, nettle_cipher_func *f,
	    size_t length, uint8_t *digest)
{
  union nettle_block16 block;
  assert (length <= OCB_DIGEST_SIZE);
  block16_xor3 (&block,  &key->L[1],
		(ctx->message_count > 0) ? &ctx->offset : &ctx->initial);
  block16_xor (&block, &ctx->checksum);
  f (cipher, OCB_BLOCK_SIZE, block.b, block.b);
  memxor3 (digest, block.b, ctx->sum.b, length);
}

void
ocb_encrypt_message (const struct ocb_key *key,
		     const void *cipher, nettle_cipher_func *f,
		     size_t nlength, const uint8_t *nonce,
		     size_t alength, const uint8_t *adata,
		     size_t tlength,
		     size_t clength, uint8_t *dst, const uint8_t *src)
{
  struct ocb_ctx ctx;
  assert (clength >= tlength);
  ocb_set_nonce (&ctx, cipher, f, tlength, nlength, nonce);
  ocb_update (&ctx, key, cipher, f, alength, adata);
  ocb_encrypt (&ctx, key, cipher, f,  clength - tlength, dst, src);
  ocb_digest (&ctx, key, cipher, f, tlength, dst + clength - tlength);
}

int
ocb_decrypt_message (const struct ocb_key *key,
		     const void *encrypt_ctx, nettle_cipher_func *encrypt,
		     const void *decrypt_ctx, nettle_cipher_func *decrypt,
		     size_t nlength, const uint8_t *nonce,
		     size_t alength, const uint8_t *adata,
		     size_t tlength,
		     size_t mlength, uint8_t *dst, const uint8_t *src)
{
  struct ocb_ctx ctx;
  union nettle_block16 digest;
  ocb_set_nonce (&ctx, encrypt_ctx, encrypt, tlength, nlength, nonce);
  ocb_update (&ctx, key, encrypt_ctx, encrypt, alength, adata);
  ocb_decrypt (&ctx, key, encrypt_ctx, encrypt, decrypt_ctx, decrypt,
	       mlength, dst, src);
  ocb_digest (&ctx, key, encrypt_ctx, encrypt, tlength, digest.b);
  return memeql_sec(digest.b, src + mlength, tlength);
}
