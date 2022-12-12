/* sm3.c

   The SM3 hash function.

   Copyright (C) 2017 Jia Zhang
   Copyright (C) 2021 Tianjia Zhang <tianjia.zhang@linux.alibaba.com>

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
#include <string.h>

#include "sm3.h"

#include "macros.h"
#include "nettle-write.h"

static const uint32_t K[64] =
{
  0x79cc4519UL, 0xf3988a32UL, 0xe7311465UL, 0xce6228cbUL,
  0x9cc45197UL, 0x3988a32fUL, 0x7311465eUL, 0xe6228cbcUL,
  0xcc451979UL, 0x988a32f3UL, 0x311465e7UL, 0x6228cbceUL,
  0xc451979cUL, 0x88a32f39UL, 0x11465e73UL, 0x228cbce6UL,
  0x9d8a7a87UL, 0x3b14f50fUL, 0x7629ea1eUL, 0xec53d43cUL,
  0xd8a7a879UL, 0xb14f50f3UL, 0x629ea1e7UL, 0xc53d43ceUL,
  0x8a7a879dUL, 0x14f50f3bUL, 0x29ea1e76UL, 0x53d43cecUL,
  0xa7a879d8UL, 0x4f50f3b1UL, 0x9ea1e762UL, 0x3d43cec5UL,
  0x7a879d8aUL, 0xf50f3b14UL, 0xea1e7629UL, 0xd43cec53UL,
  0xa879d8a7UL, 0x50f3b14fUL, 0xa1e7629eUL, 0x43cec53dUL,
  0x879d8a7aUL, 0x0f3b14f5UL, 0x1e7629eaUL, 0x3cec53d4UL,
  0x79d8a7a8UL, 0xf3b14f50UL, 0xe7629ea1UL, 0xcec53d43UL,
  0x9d8a7a87UL, 0x3b14f50fUL, 0x7629ea1eUL, 0xec53d43cUL,
  0xd8a7a879UL, 0xb14f50f3UL, 0x629ea1e7UL, 0xc53d43ceUL,
  0x8a7a879dUL, 0x14f50f3bUL, 0x29ea1e76UL, 0x53d43cecUL,
  0xa7a879d8UL, 0x4f50f3b1UL, 0x9ea1e762UL, 0x3d43cec5UL
};

/*
  Transform the message X which consists of 16 32-bit-words. See
  GM/T 004-2012 for details. */
#define R(i, a, b, c, d, e, f, g, h, t, w1, w2)     \
  do {                                              \
    ss1 = ROTL32(7, (ROTL32(12, (a)) + (e) + (t))); \
    ss2 = ss1 ^ ROTL32(12, (a));                    \
    d += FF ## i(a, b, c) + ss2 + ((w1) ^ (w2));    \
    h += GG ## i(e, f, g) + ss1 + (w1);             \
    b = ROTL32(9, (b));                             \
    f = ROTL32(19, (f));                            \
    h = P0((h));                                    \
  } while (0)

#define R1(a,b,c,d,e,f,g,h,t,w1,w2) R(1,a,b,c,d,e,f,g,h,t,w1,w2)
#define R2(a,b,c,d,e,f,g,h,t,w1,w2) R(2,a,b,c,d,e,f,g,h,t,w1,w2)

#define FF1(x, y, z)  (x ^ y ^ z)
#define FF2(x, y, z)  ((x & y) | (x & z) | (y & z))

#define GG1(x, y, z)  (x ^ y ^ z)
#define GG2(x, y, z)  ((x & y) | ( ~x & z))

/* Message expansion */
#define P0(x) ((x) ^ ROTL32(9, (x)) ^ ROTL32(17, (x)))
#define P1(x) ((x) ^ ROTL32(15, (x)) ^ ROTL32(23, (x)))
#define I(i)  (w[i] = READ_UINT32(input + i * 4))
#define W1(i) (w[i & 0x0f])
#define W2(i) (w[i & 0x0f] =                                            \
        P1(w[i & 0x0f] ^ w[(i-9) & 0x0f] ^ ROTL32(15, w[(i-3) & 0x0f])) \
        ^ ROTL32(7, w[(i-13) & 0x0f])                                   \
        ^ w[(i-6) & 0x0f])


static void
sm3_compress(uint32_t *state, const uint8_t *input)
{
  uint32_t a, b, c, d, e, f, g, h, ss1, ss2;
  uint32_t w[16];

  a = state[0];
  b = state[1];
  c = state[2];
  d = state[3];
  e = state[4];
  f = state[5];
  g = state[6];
  h = state[7];

  R1(a, b, c, d, e, f, g, h, K[0], I(0), I(4));
  R1(d, a, b, c, h, e, f, g, K[1], I(1), I(5));
  R1(c, d, a, b, g, h, e, f, K[2], I(2), I(6));
  R1(b, c, d, a, f, g, h, e, K[3], I(3), I(7));
  R1(a, b, c, d, e, f, g, h, K[4], W1(4), I(8));
  R1(d, a, b, c, h, e, f, g, K[5], W1(5), I(9));
  R1(c, d, a, b, g, h, e, f, K[6], W1(6), I(10));
  R1(b, c, d, a, f, g, h, e, K[7], W1(7), I(11));
  R1(a, b, c, d, e, f, g, h, K[8], W1(8), I(12));
  R1(d, a, b, c, h, e, f, g, K[9], W1(9), I(13));
  R1(c, d, a, b, g, h, e, f, K[10], W1(10), I(14));
  R1(b, c, d, a, f, g, h, e, K[11], W1(11), I(15));
  R1(a, b, c, d, e, f, g, h, K[12], W1(12), W2(16));
  R1(d, a, b, c, h, e, f, g, K[13], W1(13), W2(17));
  R1(c, d, a, b, g, h, e, f, K[14], W1(14), W2(18));
  R1(b, c, d, a, f, g, h, e, K[15], W1(15), W2(19));

  R2(a, b, c, d, e, f, g, h, K[16], W1(16), W2(20));
  R2(d, a, b, c, h, e, f, g, K[17], W1(17), W2(21));
  R2(c, d, a, b, g, h, e, f, K[18], W1(18), W2(22));
  R2(b, c, d, a, f, g, h, e, K[19], W1(19), W2(23));
  R2(a, b, c, d, e, f, g, h, K[20], W1(20), W2(24));
  R2(d, a, b, c, h, e, f, g, K[21], W1(21), W2(25));
  R2(c, d, a, b, g, h, e, f, K[22], W1(22), W2(26));
  R2(b, c, d, a, f, g, h, e, K[23], W1(23), W2(27));
  R2(a, b, c, d, e, f, g, h, K[24], W1(24), W2(28));
  R2(d, a, b, c, h, e, f, g, K[25], W1(25), W2(29));
  R2(c, d, a, b, g, h, e, f, K[26], W1(26), W2(30));
  R2(b, c, d, a, f, g, h, e, K[27], W1(27), W2(31));
  R2(a, b, c, d, e, f, g, h, K[28], W1(28), W2(32));
  R2(d, a, b, c, h, e, f, g, K[29], W1(29), W2(33));
  R2(c, d, a, b, g, h, e, f, K[30], W1(30), W2(34));
  R2(b, c, d, a, f, g, h, e, K[31], W1(31), W2(35));

  R2(a, b, c, d, e, f, g, h, K[32], W1(32), W2(36));
  R2(d, a, b, c, h, e, f, g, K[33], W1(33), W2(37));
  R2(c, d, a, b, g, h, e, f, K[34], W1(34), W2(38));
  R2(b, c, d, a, f, g, h, e, K[35], W1(35), W2(39));
  R2(a, b, c, d, e, f, g, h, K[36], W1(36), W2(40));
  R2(d, a, b, c, h, e, f, g, K[37], W1(37), W2(41));
  R2(c, d, a, b, g, h, e, f, K[38], W1(38), W2(42));
  R2(b, c, d, a, f, g, h, e, K[39], W1(39), W2(43));
  R2(a, b, c, d, e, f, g, h, K[40], W1(40), W2(44));
  R2(d, a, b, c, h, e, f, g, K[41], W1(41), W2(45));
  R2(c, d, a, b, g, h, e, f, K[42], W1(42), W2(46));
  R2(b, c, d, a, f, g, h, e, K[43], W1(43), W2(47));
  R2(a, b, c, d, e, f, g, h, K[44], W1(44), W2(48));
  R2(d, a, b, c, h, e, f, g, K[45], W1(45), W2(49));
  R2(c, d, a, b, g, h, e, f, K[46], W1(46), W2(50));
  R2(b, c, d, a, f, g, h, e, K[47], W1(47), W2(51));

  R2(a, b, c, d, e, f, g, h, K[48], W1(48), W2(52));
  R2(d, a, b, c, h, e, f, g, K[49], W1(49), W2(53));
  R2(c, d, a, b, g, h, e, f, K[50], W1(50), W2(54));
  R2(b, c, d, a, f, g, h, e, K[51], W1(51), W2(55));
  R2(a, b, c, d, e, f, g, h, K[52], W1(52), W2(56));
  R2(d, a, b, c, h, e, f, g, K[53], W1(53), W2(57));
  R2(c, d, a, b, g, h, e, f, K[54], W1(54), W2(58));
  R2(b, c, d, a, f, g, h, e, K[55], W1(55), W2(59));
  R2(a, b, c, d, e, f, g, h, K[56], W1(56), W2(60));
  R2(d, a, b, c, h, e, f, g, K[57], W1(57), W2(61));
  R2(c, d, a, b, g, h, e, f, K[58], W1(58), W2(62));
  R2(b, c, d, a, f, g, h, e, K[59], W1(59), W2(63));
  R2(a, b, c, d, e, f, g, h, K[60], W1(60), W2(64));
  R2(d, a, b, c, h, e, f, g, K[61], W1(61), W2(65));
  R2(c, d, a, b, g, h, e, f, K[62], W1(62), W2(66));
  R2(b, c, d, a, f, g, h, e, K[63], W1(63), W2(67));

  state[0] ^= a;
  state[1] ^= b;
  state[2] ^= c;
  state[3] ^= d;
  state[4] ^= e;
  state[5] ^= f;
  state[6] ^= g;
  state[7] ^= h;
}

void
sm3_init(struct sm3_ctx *ctx)
{
  static const uint32_t H0[_SM3_DIGEST_LENGTH] =
  {
    0x7380166fUL, 0x4914b2b9UL, 0x172442d7UL, 0xda8a0600UL,
    0xa96f30bcUL, 0x163138aaUL, 0xe38dee4dUL, 0xb0fb0e4eUL
  };

  memcpy(ctx->state, H0, sizeof(H0));

  /* Initialize bit count */
  ctx->count = 0;

  /* Initialize buffer */
  ctx->index = 0;
}

#define COMPRESS(ctx, data) (sm3_compress((ctx)->state, (data)))

void
sm3_update(struct sm3_ctx *ctx,
	   size_t length, const uint8_t *data)
{
  MD_UPDATE(ctx, length, data, COMPRESS, ctx->count++);
}

static void
sm3_write_digest(struct sm3_ctx *ctx,
		 size_t length,
		 uint8_t *digest)
{
  uint64_t bit_count;

  assert(length <= SM3_DIGEST_SIZE);

  MD_PAD(ctx, 8, COMPRESS);

  /* There are 512 = 2^9 bits in one block */
  bit_count = (ctx->count << 9) | (ctx->index << 3);

  /* This is slightly inefficient, as the numbers are converted to
     big-endian format, and will be converted back by the compression
     function. It's probably not worth the effort to fix this. */
  WRITE_UINT64(ctx->block + (SM3_BLOCK_SIZE - 8), bit_count);
  COMPRESS(ctx, ctx->block);

  _nettle_write_be32(length, digest, ctx->state);
}

void
sm3_digest(struct sm3_ctx *ctx,
	   size_t length,
	   uint8_t *digest)
{
  sm3_write_digest(ctx, length, digest);
  sm3_init(ctx);
}
