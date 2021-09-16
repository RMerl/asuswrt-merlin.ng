/* gosthash94.c - an implementation of GOST Hash Function
 *
 * based on the Russian Standard GOST R 34.11-94.
 * English description in RFC 5831.
 * See also RFC 4357.
 *
 * Copyright: 2009-2012 Aleksey Kravchenko <rhash.admin@gmail.com>
 * Copyright: 2019 Dmitry Eremin-Solenikov <dbaryshkov@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/*
 * Ported to nettle by Nikos Mavrogiannopoulos.
 */

#if HAVE_CONFIG_H
#include "config.h"
#endif

#include <assert.h>
#include <string.h>

#include "macros.h"
#include "nettle-write.h"
#include "gosthash94.h"
#include "gost28147-internal.h"

/**
 * Initialize algorithm context before calculating hash
 * with test parameters set.
 *
 * @param ctx context to initalize
 */
void
gosthash94_init (struct gosthash94_ctx *ctx)
{
    memset (ctx, 0, sizeof (struct gosthash94_ctx));
}

/**
 * The core transformation. Process a 512-bit block.
 *
 * @param hash intermediate message hash
 * @param block the message block to process
 */
static void
gost_block_compress (struct gosthash94_ctx *ctx, const uint32_t *block,
		     const uint32_t sbox[4][256])
{
    unsigned i;
    uint32_t key[8], u[8], v[8], w[8], s[8];

    /* u := hash, v := <256-bit message block> */
    memcpy (u, ctx->hash, sizeof (u));
    memcpy (v, block, sizeof (v));

    /* w := u xor v */
    w[0] = u[0] ^ v[0], w[1] = u[1] ^ v[1];
    w[2] = u[2] ^ v[2], w[3] = u[3] ^ v[3];
    w[4] = u[4] ^ v[4], w[5] = u[5] ^ v[5];
    w[6] = u[6] ^ v[6], w[7] = u[7] ^ v[7];

    /* calculate keys, encrypt hash and store result to the s[] array */
    for (i = 0;; i += 2)
      {
          /* key generation: key_i := P(w) */
          key[0] =
              (w[0] & 0x000000ff) | ((w[2] & 0x000000ff) << 8) |
              ((w[4] & 0x000000ff) << 16) | ((w[6] & 0x000000ff) << 24);
          key[1] =
              ((w[0] & 0x0000ff00) >> 8) | (w[2] & 0x0000ff00) |
              ((w[4] & 0x0000ff00) << 8) | ((w[6] & 0x0000ff00) << 16);
          key[2] =
              ((w[0] & 0x00ff0000) >> 16) | ((w[2] & 0x00ff0000) >> 8) |
              (w[4] & 0x00ff0000) | ((w[6] & 0x00ff0000) << 8);
          key[3] =
              ((w[0] & 0xff000000) >> 24) | ((w[2] & 0xff000000) >> 16) |
              ((w[4] & 0xff000000) >> 8) | (w[6] & 0xff000000);
          key[4] =
              (w[1] & 0x000000ff) | ((w[3] & 0x000000ff) << 8) |
              ((w[5] & 0x000000ff) << 16) | ((w[7] & 0x000000ff) << 24);
          key[5] =
              ((w[1] & 0x0000ff00) >> 8) | (w[3] & 0x0000ff00) |
              ((w[5] & 0x0000ff00) << 8) | ((w[7] & 0x0000ff00) << 16);
          key[6] =
              ((w[1] & 0x00ff0000) >> 16) | ((w[3] & 0x00ff0000) >> 8) |
              (w[5] & 0x00ff0000) | ((w[7] & 0x00ff0000) << 8);
          key[7] =
              ((w[1] & 0xff000000) >> 24) | ((w[3] & 0xff000000) >> 16) |
              ((w[5] & 0xff000000) >> 8) | (w[7] & 0xff000000);

          /* encryption: s_i := E_{key_i} (h_i) */
          _nettle_gost28147_encrypt_block (key, sbox, &ctx->hash[i], &s[i]);

          if (i == 0)
            {
                /* w:= A(u) ^ A^2(v) */
                w[0] = u[2] ^ v[4], w[1] = u[3] ^ v[5];
                w[2] = u[4] ^ v[6], w[3] = u[5] ^ v[7];
                w[4] = u[6] ^ (v[0] ^= v[2]);
                w[5] = u[7] ^ (v[1] ^= v[3]);
                w[6] = (u[0] ^= u[2]) ^ (v[2] ^= v[4]);
                w[7] = (u[1] ^= u[3]) ^ (v[3] ^= v[5]);
            }
          else if ((i & 2) != 0)
            {
                if (i == 6)
                    break;

                /* w := A^2(u) xor A^4(v) xor C_3; u := A(u) xor C_3 */
                /* C_3=0xff00ffff000000ffff0000ff00ffff0000ff00ff00ff00ffff00ff00ff00ff00 */
                u[2] ^= u[4] ^ 0x000000ff;
                u[3] ^= u[5] ^ 0xff00ffff;
                u[4] ^= 0xff00ff00;
                u[5] ^= 0xff00ff00;
                u[6] ^= 0x00ff00ff;
                u[7] ^= 0x00ff00ff;
                u[0] ^= 0x00ffff00;
                u[1] ^= 0xff0000ff;

                w[0] = u[4] ^ v[0];
                w[2] = u[6] ^ v[2];
                w[4] = u[0] ^ (v[4] ^= v[6]);
                w[6] = u[2] ^ (v[6] ^= v[0]);
                w[1] = u[5] ^ v[1];
                w[3] = u[7] ^ v[3];
                w[5] = u[1] ^ (v[5] ^= v[7]);
                w[7] = u[3] ^ (v[7] ^= v[1]);
            }
          else
            {
                /* i==4 here */
                /* w:= A( A^2(u) xor C_3 ) xor A^6(v) */
                w[0] = u[6] ^ v[4], w[1] = u[7] ^ v[5];
                w[2] = u[0] ^ v[6], w[3] = u[1] ^ v[7];
                w[4] = u[2] ^ (v[0] ^= v[2]);
                w[5] = u[3] ^ (v[1] ^= v[3]);
                w[6] = (u[4] ^= u[6]) ^ (v[2] ^= v[4]);
                w[7] = (u[5] ^= u[7]) ^ (v[3] ^= v[5]);
            }
      }

    /* step hash function: x(block, hash) := psi^61(hash xor psi(block xor psi^12(S))) */

    /* 12 rounds of the LFSR and xor in <message block> */
    u[0] = block[0] ^ s[6];
    u[1] = block[1] ^ s[7];
    u[2] =
        block[2] ^ (s[0] << 16) ^ (s[0] >> 16) ^ (s[0] & 0xffff) ^ (s[1] &
                                                                    0xffff)
        ^ (s[1] >> 16) ^ (s[2] << 16) ^ s[6] ^ (s[6] << 16) ^ (s[7] &
                                                               0xffff0000)
        ^ (s[7] >> 16);
    u[3] =
        block[3] ^ (s[0] & 0xffff) ^ (s[0] << 16) ^ (s[1] & 0xffff) ^ (s[1]
                                                                       <<
                                                                       16)
        ^ (s[1] >> 16) ^ (s[2] << 16) ^ (s[2] >> 16) ^ (s[3] << 16) ^ s[6]
        ^ (s[6] << 16) ^ (s[6] >> 16) ^ (s[7] & 0xffff) ^ (s[7] << 16) ^
        (s[7] >> 16);
    u[4] =
        block[4] ^ (s[0] & 0xffff0000) ^ (s[0] << 16) ^ (s[0] >> 16) ^
        (s[1] & 0xffff0000) ^ (s[1] >> 16) ^ (s[2] << 16) ^ (s[2] >> 16) ^
        (s[3] << 16) ^ (s[3] >> 16) ^ (s[4] << 16) ^ (s[6] << 16) ^ (s[6]
                                                                     >> 16)
        ^ (s[7] & 0xffff) ^ (s[7] << 16) ^ (s[7] >> 16);
    u[5] =
        block[5] ^ (s[0] << 16) ^ (s[0] >> 16) ^ (s[0] & 0xffff0000) ^
        (s[1] & 0xffff) ^ s[2] ^ (s[2] >> 16) ^ (s[3] << 16) ^ (s[3] >> 16)
        ^ (s[4] << 16) ^ (s[4] >> 16) ^ (s[5] << 16) ^ (s[6] << 16) ^ (s[6]
                                                                       >>
                                                                       16)
        ^ (s[7] & 0xffff0000) ^ (s[7] << 16) ^ (s[7] >> 16);
    u[6] =
        block[6] ^ s[0] ^ (s[1] >> 16) ^ (s[2] << 16) ^ s[3] ^ (s[3] >> 16)
        ^ (s[4] << 16) ^ (s[4] >> 16) ^ (s[5] << 16) ^ (s[5] >> 16) ^ s[6]
        ^ (s[6] << 16) ^ (s[6] >> 16) ^ (s[7] << 16);
    u[7] =
        block[7] ^ (s[0] & 0xffff0000) ^ (s[0] << 16) ^ (s[1] & 0xffff) ^
        (s[1] << 16) ^ (s[2] >> 16) ^ (s[3] << 16) ^ s[4] ^ (s[4] >> 16) ^
        (s[5] << 16) ^ (s[5] >> 16) ^ (s[6] >> 16) ^ (s[7] & 0xffff) ^
        (s[7] << 16) ^ (s[7] >> 16);

    /* 1 round of the LFSR (a mixing transformation) and xor with <hash> */
    v[0] = ctx->hash[0] ^ (u[1] << 16) ^ (u[0] >> 16);
    v[1] = ctx->hash[1] ^ (u[2] << 16) ^ (u[1] >> 16);
    v[2] = ctx->hash[2] ^ (u[3] << 16) ^ (u[2] >> 16);
    v[3] = ctx->hash[3] ^ (u[4] << 16) ^ (u[3] >> 16);
    v[4] = ctx->hash[4] ^ (u[5] << 16) ^ (u[4] >> 16);
    v[5] = ctx->hash[5] ^ (u[6] << 16) ^ (u[5] >> 16);
    v[6] = ctx->hash[6] ^ (u[7] << 16) ^ (u[6] >> 16);
    v[7] =
        ctx->
        hash[7] ^ (u[0] & 0xffff0000) ^ (u[0] << 16) ^ (u[1] & 0xffff0000)
        ^ (u[1] << 16) ^ (u[6] << 16) ^ (u[7] & 0xffff0000) ^ (u[7] >> 16);

    /* 61 rounds of LFSR, mixing up hash */
    ctx->hash[0] = (v[0] & 0xffff0000) ^ (v[0] << 16) ^ (v[0] >> 16) ^
        (v[1] >> 16) ^ (v[1] & 0xffff0000) ^ (v[2] << 16) ^
        (v[3] >> 16) ^ (v[4] << 16) ^ (v[5] >> 16) ^ v[5] ^
        (v[6] >> 16) ^ (v[7] << 16) ^ (v[7] >> 16) ^ (v[7] & 0xffff);
    ctx->hash[1] = (v[0] << 16) ^ (v[0] >> 16) ^ (v[0] & 0xffff0000) ^
        (v[1] & 0xffff) ^ v[2] ^ (v[2] >> 16) ^ (v[3] << 16) ^
        (v[4] >> 16) ^ (v[5] << 16) ^ (v[6] << 16) ^ v[6] ^
        (v[7] & 0xffff0000) ^ (v[7] >> 16);
    ctx->hash[2] = (v[0] & 0xffff) ^ (v[0] << 16) ^ (v[1] << 16) ^
        (v[1] >> 16) ^ (v[1] & 0xffff0000) ^ (v[2] << 16) ^ (v[3] >> 16) ^
        v[3] ^ (v[4] << 16) ^ (v[5] >> 16) ^ v[6] ^ (v[6] >> 16) ^
        (v[7] & 0xffff) ^ (v[7] << 16) ^ (v[7] >> 16);
    ctx->hash[3] = (v[0] << 16) ^ (v[0] >> 16) ^ (v[0] & 0xffff0000) ^
        (v[1] & 0xffff0000) ^ (v[1] >> 16) ^ (v[2] << 16) ^
        (v[2] >> 16) ^ v[2] ^ (v[3] << 16) ^ (v[4] >> 16) ^ v[4] ^
        (v[5] << 16) ^ (v[6] << 16) ^ (v[7] & 0xffff) ^ (v[7] >> 16);
    ctx->hash[4] =
        (v[0] >> 16) ^ (v[1] << 16) ^ v[1] ^ (v[2] >> 16) ^ v[2] ^ (v[3] <<
                                                                    16) ^
        (v[3] >> 16) ^ v[3] ^ (v[4] << 16) ^ (v[5] >> 16) ^ v[5] ^ (v[6] <<
                                                                    16) ^
        (v[6] >> 16) ^ (v[7] << 16);
    ctx->hash[5] =
        (v[0] << 16) ^ (v[0] & 0xffff0000) ^ (v[1] << 16) ^ (v[1] >> 16) ^
        (v[1] & 0xffff0000) ^ (v[2] << 16) ^ v[2] ^ (v[3] >> 16) ^ v[3] ^
        (v[4] << 16) ^ (v[4] >> 16) ^ v[4] ^ (v[5] << 16) ^ (v[6] << 16) ^
        (v[6] >> 16) ^ v[6] ^ (v[7] << 16) ^ (v[7] >> 16) ^ (v[7] &
                                                             0xffff0000);
    ctx->hash[6] =
        v[0] ^ v[2] ^ (v[2] >> 16) ^ v[3] ^ (v[3] << 16) ^ v[4] ^ (v[4] >>
                                                                   16) ^
        (v[5] << 16) ^ (v[5] >> 16) ^ v[5] ^ (v[6] << 16) ^ (v[6] >> 16) ^
        v[6] ^ (v[7] << 16) ^ v[7];
    ctx->hash[7] =
        v[0] ^ (v[0] >> 16) ^ (v[1] << 16) ^ (v[1] >> 16) ^ (v[2] << 16) ^
        (v[3] >> 16) ^ v[3] ^ (v[4] << 16) ^ v[4] ^ (v[5] >> 16) ^ v[5] ^
        (v[6] << 16) ^ (v[6] >> 16) ^ (v[7] << 16) ^ v[7];
}

/**
 * This function calculates hash value by 256-bit blocks.
 * It updates 256-bit check sum as follows:
 *    *(uint256_t)(ctx->sum) += *(uint256_t*)block;
 * and then updates intermediate hash value ctx->hash 
 * by calling gost_block_compress().
 *
 * @param ctx algorithm context
 * @param block the 256-bit message block to process
 */
static void
gost_compute_sum_and_hash (struct gosthash94_ctx *ctx, const uint8_t *block,
			   const uint32_t sbox[4][256])
{
    uint32_t block_le[8];
    unsigned i, carry;

    /* compute the 256-bit sum */
    for (i = carry = 0; i < 8; i++, block += 4)
      {
	  block_le[i] = LE_READ_UINT32(block);
          ctx->sum[i] += carry;
	  carry = (ctx->sum[i] < carry);
          ctx->sum[i] += block_le[i];
          carry += (ctx->sum[i] < block_le[i]);
      }

    /* update message hash */
    gost_block_compress (ctx, block_le, sbox);
}

#define COMPRESS(ctx, block) gost_compute_sum_and_hash((ctx), (block), sbox);

/**
 * Calculate message hash.
 * Can be called repeatedly with chunks of the message to be hashed.
 *
 * @param ctx the algorithm context containing current hashing state
 * @param msg message chunk
 * @param size length of the message chunk
 */
static void
gosthash94_update_int (struct gosthash94_ctx *ctx,
		       size_t length, const uint8_t *msg,
		       const uint32_t sbox[4][256])
{
    MD_UPDATE(ctx, length, msg, COMPRESS, ctx->count++);
}

/**
 * Calculate message hash.
 * Can be called repeatedly with chunks of the message to be hashed.
 *
 * @param ctx the algorithm context containing current hashing state
 * @param msg message chunk
 * @param size length of the message chunk
 */
void
gosthash94_update (struct gosthash94_ctx *ctx,
		   size_t length, const uint8_t *msg)
{
  gosthash94_update_int (ctx, length, msg,
			 _nettle_gost28147_param_test_3411.sbox);
}

/**
 * Calculate message hash.
 * Can be called repeatedly with chunks of the message to be hashed.
 *
 * @param ctx the algorithm context containing current hashing state
 * @param msg message chunk
 * @param size length of the message chunk
 */
void
gosthash94cp_update (struct gosthash94_ctx *ctx,
		     size_t length, const uint8_t *msg)
{
  gosthash94_update_int (ctx, length, msg,
			 _nettle_gost28147_param_CryptoPro_3411.sbox);
}

/**
 * Finish hashing and store message digest into given array.
 *
 * @param ctx the algorithm context containing current hashing state
 * @param result calculated hash in binary form
 */
static void
gosthash94_write_digest (struct gosthash94_ctx *ctx,
			 size_t length, uint8_t *result,
			 const uint32_t sbox[4][256])
{
    uint32_t msg32[GOSTHASH94_BLOCK_SIZE / 4];

    assert(length <= GOSTHASH94_DIGEST_SIZE);

    /* pad the last block with zeroes and hash it */
    if (ctx->index > 0)
      {
          memset (ctx->block + ctx->index, 0, GOSTHASH94_BLOCK_SIZE - ctx->index);
          gost_compute_sum_and_hash (ctx, ctx->block, sbox);
      }

    /* hash the message length and the sum */
    msg32[0] = (ctx->count << 8) | (ctx->index << 3);
    msg32[1] = ctx->count >> 24;
    memset (msg32 + 2, 0, sizeof (uint32_t) * 6);

    gost_block_compress (ctx, msg32, sbox);
    gost_block_compress (ctx, ctx->sum, sbox);

    /* convert hash state to result bytes */
    _nettle_write_le32(length, result, ctx->hash);
    gosthash94_init (ctx);
}

void
gosthash94_digest (struct gosthash94_ctx *ctx,
		   size_t length, uint8_t *result)
{
  gosthash94_write_digest (ctx, length, result,
			   _nettle_gost28147_param_test_3411.sbox);
}

void
gosthash94cp_digest (struct gosthash94_ctx *ctx,
		     size_t length, uint8_t *result)
{
  gosthash94_write_digest (ctx, length, result,
			   _nettle_gost28147_param_CryptoPro_3411.sbox);
}
