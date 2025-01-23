/* blake2.c - BLAKE2b and BLAKE2s hash functions (RFC 7693)
 * Copyright (C) 2017  Jussi Kivilinna <jussi.kivilinna@iki.fi>
 *
 * This file is part of Libgcrypt.
 *
 * Libgcrypt is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser general Public License as
 * published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * Libgcrypt is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

/* The code is based on public-domain/CC0 BLAKE2 reference implementation
 * by Samual Neves, at https://github.com/BLAKE2/BLAKE2/tree/master/ref
 * Copyright 2012, Samuel Neves <sneves@dei.uc.pt>
 */

#include <config.h>
#include <string.h>
#include "g10lib.h"
#include "bithelp.h"
#include "bufhelp.h"
#include "cipher.h"
#include "hash-common.h"

/* USE_AVX indicates whether to compile with Intel AVX code. */
#undef USE_AVX
#if defined(__x86_64__) && defined(HAVE_GCC_INLINE_ASM_AVX) && \
    (defined(HAVE_COMPATIBLE_GCC_AMD64_PLATFORM_AS) || \
     defined(HAVE_COMPATIBLE_GCC_WIN64_PLATFORM_AS))
# define USE_AVX 1
#endif

/* USE_AVX2 indicates whether to compile with Intel AVX2 code. */
#undef USE_AVX2
#if defined(__x86_64__) && defined(HAVE_GCC_INLINE_ASM_AVX2) && \
    (defined(HAVE_COMPATIBLE_GCC_AMD64_PLATFORM_AS) || \
     defined(HAVE_COMPATIBLE_GCC_WIN64_PLATFORM_AS))
# define USE_AVX2 1
#endif

/* AMD64 assembly implementations use SystemV ABI, ABI conversion and additional
 * stack to store XMM6-XMM15 needed on Win64. */
#undef ASM_FUNC_ABI
#undef ASM_EXTRA_STACK
#if defined(USE_AVX2) && defined(HAVE_COMPATIBLE_GCC_WIN64_PLATFORM_AS)
# define ASM_FUNC_ABI __attribute__((sysv_abi))
# define ASM_EXTRA_STACK (10 * 16)
#else
# define ASM_FUNC_ABI
# define ASM_EXTRA_STACK 0
#endif

#define BLAKE2B_BLOCKBYTES 128
#define BLAKE2B_OUTBYTES 64
#define BLAKE2B_KEYBYTES 64

#define BLAKE2S_BLOCKBYTES 64
#define BLAKE2S_OUTBYTES 32
#define BLAKE2S_KEYBYTES 32

typedef struct
{
  u64 h[8];
  u64 t[2];
  u64 f[2];
} BLAKE2B_STATE;

struct blake2b_param_s
{
  byte digest_length;
  byte key_length;
  byte fanout;
  byte depth;
  byte leaf_length[4];
  byte node_offset[4];
  byte xof_length[4];
  byte node_depth;
  byte inner_length;
  byte reserved[14];
  byte salt[16];
  byte personal[16];
};

typedef struct BLAKE2B_CONTEXT_S
{
  BLAKE2B_STATE state;
  byte buf[BLAKE2B_BLOCKBYTES];
  size_t buflen;
  size_t outlen;
#ifdef USE_AVX2
  unsigned int use_avx2:1;
#endif
} BLAKE2B_CONTEXT;

typedef struct
{
  u32 h[8];
  u32 t[2];
  u32 f[2];
} BLAKE2S_STATE;

struct blake2s_param_s
{
  byte digest_length;
  byte key_length;
  byte fanout;
  byte depth;
  byte leaf_length[4];
  byte node_offset[4];
  byte xof_length[2];
  byte node_depth;
  byte inner_length;
  /* byte reserved[0]; */
  byte salt[8];
  byte personal[8];
};

typedef struct BLAKE2S_CONTEXT_S
{
  BLAKE2S_STATE state;
  byte buf[BLAKE2S_BLOCKBYTES];
  size_t buflen;
  size_t outlen;
#ifdef USE_AVX
  unsigned int use_avx:1;
#endif
} BLAKE2S_CONTEXT;

typedef unsigned int (*blake2_transform_t)(void *S, const void *inblk,
					   size_t nblks);


static const u64 blake2b_IV[8] =
{
  U64_C(0x6a09e667f3bcc908), U64_C(0xbb67ae8584caa73b),
  U64_C(0x3c6ef372fe94f82b), U64_C(0xa54ff53a5f1d36f1),
  U64_C(0x510e527fade682d1), U64_C(0x9b05688c2b3e6c1f),
  U64_C(0x1f83d9abfb41bd6b), U64_C(0x5be0cd19137e2179)
};

static const u32 blake2s_IV[8] =
{
  0x6A09E667UL, 0xBB67AE85UL, 0x3C6EF372UL, 0xA54FF53AUL,
  0x510E527FUL, 0x9B05688CUL, 0x1F83D9ABUL, 0x5BE0CD19UL
};

static byte zero_block[BLAKE2B_BLOCKBYTES] = { 0, };


static void blake2_write(void *S, const void *inbuf, size_t inlen,
			 byte *tmpbuf, size_t *tmpbuflen, size_t blkbytes,
			 blake2_transform_t transform_fn)
{
  const byte* in = inbuf;
  unsigned int burn = 0;

  if (inlen > 0)
    {
      size_t left = *tmpbuflen;
      size_t fill = blkbytes - left;
      size_t nblks;

      if (inlen > fill)
	{
	  if (fill > 0)
	    buf_cpy (tmpbuf + left, in, fill); /* Fill buffer */
	  left = 0;

	  burn = transform_fn (S, tmpbuf, 1); /* Increment counter + Compress */

	  in += fill;
	  inlen -= fill;

	  nblks = inlen / blkbytes - !(inlen % blkbytes);
	  if (nblks)
	    {
	      burn = transform_fn(S, in, nblks);
	      in += blkbytes * nblks;
	      inlen -= blkbytes * nblks;
	    }
	}

      gcry_assert (inlen > 0);

      buf_cpy (tmpbuf + left, in, inlen);
      *tmpbuflen = left + inlen;
    }

  if (burn)
    _gcry_burn_stack (burn);

  return;
}


static inline void blake2b_set_lastblock(BLAKE2B_STATE *S)
{
  S->f[0] = U64_C(0xffffffffffffffff);
}

static inline int blake2b_is_lastblock(const BLAKE2B_STATE *S)
{
  return S->f[0] != 0;
}

static inline void blake2b_increment_counter(BLAKE2B_STATE *S, const int inc)
{
  S->t[0] += (u64)inc;
  S->t[1] += (S->t[0] < (u64)inc) - (inc < 0);
}

static inline u64 rotr64(u64 x, u64 n)
{
  return ((x >> (n & 63)) | (x << ((64 - n) & 63)));
}

static unsigned int blake2b_transform_generic(BLAKE2B_STATE *S,
                                              const void *inblks,
                                              size_t nblks)
{
  static const byte blake2b_sigma[12][16] =
  {
    {  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15 },
    { 14, 10,  4,  8,  9, 15, 13,  6,  1, 12,  0,  2, 11,  7,  5,  3 },
    { 11,  8, 12,  0,  5,  2, 15, 13, 10, 14,  3,  6,  7,  1,  9,  4 },
    {  7,  9,  3,  1, 13, 12, 11, 14,  2,  6,  5, 10,  4,  0, 15,  8 },
    {  9,  0,  5,  7,  2,  4, 10, 15, 14,  1, 11, 12,  6,  8,  3, 13 },
    {  2, 12,  6, 10,  0, 11,  8,  3,  4, 13,  7,  5, 15, 14,  1,  9 },
    { 12,  5,  1, 15, 14, 13,  4, 10,  0,  7,  6,  3,  9,  2,  8, 11 },
    { 13, 11,  7, 14, 12,  1,  3,  9,  5,  0, 15,  4,  8,  6,  2, 10 },
    {  6, 15, 14,  9, 11,  3,  0,  8, 12,  2, 13,  7,  1,  4, 10,  5 },
    { 10,  2,  8,  4,  7,  6,  1,  5, 15, 11,  9, 14,  3, 12, 13 , 0 },
    {  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15 },
    { 14, 10,  4,  8,  9, 15, 13,  6,  1, 12,  0,  2, 11,  7,  5,  3 }
  };
  const byte* in = inblks;
  u64 m[16];
  u64 v[16];

  while (nblks--)
    {
      /* Increment counter */
      blake2b_increment_counter (S, BLAKE2B_BLOCKBYTES);

      /* Compress */
      m[0] = buf_get_le64 (in + 0 * sizeof(m[0]));
      m[1] = buf_get_le64 (in + 1 * sizeof(m[0]));
      m[2] = buf_get_le64 (in + 2 * sizeof(m[0]));
      m[3] = buf_get_le64 (in + 3 * sizeof(m[0]));
      m[4] = buf_get_le64 (in + 4 * sizeof(m[0]));
      m[5] = buf_get_le64 (in + 5 * sizeof(m[0]));
      m[6] = buf_get_le64 (in + 6 * sizeof(m[0]));
      m[7] = buf_get_le64 (in + 7 * sizeof(m[0]));
      m[8] = buf_get_le64 (in + 8 * sizeof(m[0]));
      m[9] = buf_get_le64 (in + 9 * sizeof(m[0]));
      m[10] = buf_get_le64 (in + 10 * sizeof(m[0]));
      m[11] = buf_get_le64 (in + 11 * sizeof(m[0]));
      m[12] = buf_get_le64 (in + 12 * sizeof(m[0]));
      m[13] = buf_get_le64 (in + 13 * sizeof(m[0]));
      m[14] = buf_get_le64 (in + 14 * sizeof(m[0]));
      m[15] = buf_get_le64 (in + 15 * sizeof(m[0]));

      v[ 0] = S->h[0];
      v[ 1] = S->h[1];
      v[ 2] = S->h[2];
      v[ 3] = S->h[3];
      v[ 4] = S->h[4];
      v[ 5] = S->h[5];
      v[ 6] = S->h[6];
      v[ 7] = S->h[7];
      v[ 8] = blake2b_IV[0];
      v[ 9] = blake2b_IV[1];
      v[10] = blake2b_IV[2];
      v[11] = blake2b_IV[3];
      v[12] = blake2b_IV[4] ^ S->t[0];
      v[13] = blake2b_IV[5] ^ S->t[1];
      v[14] = blake2b_IV[6] ^ S->f[0];
      v[15] = blake2b_IV[7] ^ S->f[1];

#define G(r,i,a,b,c,d)                      \
  do {                                      \
    a = a + b + m[blake2b_sigma[r][2*i+0]]; \
    d = rotr64(d ^ a, 32);                  \
    c = c + d;                              \
    b = rotr64(b ^ c, 24);                  \
    a = a + b + m[blake2b_sigma[r][2*i+1]]; \
    d = rotr64(d ^ a, 16);                  \
    c = c + d;                              \
    b = rotr64(b ^ c, 63);                  \
  } while(0)

#define ROUND(r)                    \
  do {                              \
    G(r,0,v[ 0],v[ 4],v[ 8],v[12]); \
    G(r,1,v[ 1],v[ 5],v[ 9],v[13]); \
    G(r,2,v[ 2],v[ 6],v[10],v[14]); \
    G(r,3,v[ 3],v[ 7],v[11],v[15]); \
    G(r,4,v[ 0],v[ 5],v[10],v[15]); \
    G(r,5,v[ 1],v[ 6],v[11],v[12]); \
    G(r,6,v[ 2],v[ 7],v[ 8],v[13]); \
    G(r,7,v[ 3],v[ 4],v[ 9],v[14]); \
  } while(0)

      ROUND(0);
      ROUND(1);
      ROUND(2);
      ROUND(3);
      ROUND(4);
      ROUND(5);
      ROUND(6);
      ROUND(7);
      ROUND(8);
      ROUND(9);
      ROUND(10);
      ROUND(11);

#undef G
#undef ROUND

      S->h[0] = S->h[0] ^ v[0] ^ v[0 + 8];
      S->h[1] = S->h[1] ^ v[1] ^ v[1 + 8];
      S->h[2] = S->h[2] ^ v[2] ^ v[2 + 8];
      S->h[3] = S->h[3] ^ v[3] ^ v[3 + 8];
      S->h[4] = S->h[4] ^ v[4] ^ v[4 + 8];
      S->h[5] = S->h[5] ^ v[5] ^ v[5 + 8];
      S->h[6] = S->h[6] ^ v[6] ^ v[6 + 8];
      S->h[7] = S->h[7] ^ v[7] ^ v[7 + 8];

      in += BLAKE2B_BLOCKBYTES;
    }

  return sizeof(void *) * 4 + sizeof(u64) * 16 * 2;
}

#ifdef USE_AVX2
unsigned int _gcry_blake2b_transform_amd64_avx2(BLAKE2B_STATE *S,
                                                const void *inblks,
                                                size_t nblks) ASM_FUNC_ABI;
#endif

static unsigned int blake2b_transform(void *ctx, const void *inblks,
                                      size_t nblks)
{
  BLAKE2B_CONTEXT *c = ctx;
  unsigned int nburn;

  if (0)
    {}
#ifdef USE_AVX2
  if (c->use_avx2)
    nburn = _gcry_blake2b_transform_amd64_avx2(&c->state, inblks, nblks);
#endif
  else
    nburn = blake2b_transform_generic(&c->state, inblks, nblks);

  if (nburn)
    nburn += ASM_EXTRA_STACK;

  return nburn;
}

static void blake2b_final(void *ctx)
{
  BLAKE2B_CONTEXT *c = ctx;
  BLAKE2B_STATE *S = &c->state;
  unsigned int burn;
  size_t i;

  gcry_assert (sizeof(c->buf) >= c->outlen);
  if (blake2b_is_lastblock(S))
    return;

  if (c->buflen < BLAKE2B_BLOCKBYTES)
    memset (c->buf + c->buflen, 0, BLAKE2B_BLOCKBYTES - c->buflen); /* Padding */
  blake2b_set_lastblock (S);
  blake2b_increment_counter (S, (int)c->buflen - BLAKE2B_BLOCKBYTES);
  burn = blake2b_transform (ctx, c->buf, 1);

  /* Output full hash to buffer */
  for (i = 0; i < 8; ++i)
    buf_put_le64 (c->buf + sizeof(S->h[i]) * i, S->h[i]);

  /* Zero out extra buffer bytes. */
  if (c->outlen < sizeof(c->buf))
    memset (c->buf + c->outlen, 0, sizeof(c->buf) - c->outlen);

  if (burn)
    _gcry_burn_stack (burn);
}

static byte *blake2b_read(void *ctx)
{
  BLAKE2B_CONTEXT *c = ctx;
  return c->buf;
}

static void blake2b_write(void *ctx, const void *inbuf, size_t inlen)
{
  BLAKE2B_CONTEXT *c = ctx;
  BLAKE2B_STATE *S = &c->state;
  blake2_write(S, inbuf, inlen, c->buf, &c->buflen, BLAKE2B_BLOCKBYTES,
	       blake2b_transform);
}

static inline void blake2b_init_param(BLAKE2B_STATE *S,
				      const struct blake2b_param_s *P)
{
  const byte *p = (const byte *)P;
  size_t i;

  /* init xors IV with input parameter block */

  /* IV XOR ParamBlock */
  for (i = 0; i < 8; ++i)
    S->h[i] = blake2b_IV[i] ^ buf_get_le64(p + sizeof(S->h[i]) * i);
}

static inline gcry_err_code_t blake2b_init(BLAKE2B_CONTEXT *ctx,
					   const byte *key, size_t keylen)
{
  struct blake2b_param_s P[1] = { { 0, } };
  BLAKE2B_STATE *S = &ctx->state;

  if (!ctx->outlen || ctx->outlen > BLAKE2B_OUTBYTES)
    return GPG_ERR_INV_ARG;
  if (sizeof(P[0]) != sizeof(u64) * 8)
    return GPG_ERR_INTERNAL;
  if (keylen && (!key || keylen > BLAKE2B_KEYBYTES))
    return GPG_ERR_INV_KEYLEN;

  P->digest_length = ctx->outlen;
  P->key_length = keylen;
  P->fanout = 1;
  P->depth = 1;

  blake2b_init_param (S, P);
  wipememory (P, sizeof(P));

  if (key)
    {
      blake2b_write (ctx, key, keylen);
      blake2b_write (ctx, zero_block, BLAKE2B_BLOCKBYTES - keylen);
    }

  return 0;
}

static gcry_err_code_t blake2b_init_ctx(void *ctx, unsigned int flags,
					const byte *key, size_t keylen,
					unsigned int dbits)
{
  BLAKE2B_CONTEXT *c = ctx;
  unsigned int features = _gcry_get_hw_features ();

  (void)features;
  (void)flags;

  memset (c, 0, sizeof (*c));

#ifdef USE_AVX2
  c->use_avx2 = !!(features & HWF_INTEL_AVX2);
#endif

  c->outlen = dbits / 8;
  c->buflen = 0;
  return blake2b_init(c, key, keylen);
}

/* Variable-length Hash Function H'.  */
gcry_err_code_t
blake2b_vl_hash (const void *in, size_t inlen, size_t outputlen, void *output)
{
  gcry_err_code_t ec;
  BLAKE2B_CONTEXT ctx;
  unsigned char buf[4];

  ec = blake2b_init_ctx (&ctx, 0, NULL, 0,
                         (outputlen < 64 ? outputlen: 64)*8);
  if (ec)
    return ec;

  buf_put_le32 (buf, outputlen);
  blake2b_write (&ctx, buf, 4);
  blake2b_write (&ctx, in, inlen);
  blake2b_final (&ctx);

  if (outputlen <= 64)
    memcpy (output, ctx.buf, outputlen);
  else
    {
      int r = (outputlen-1)/32 - 1;
      unsigned int remained = outputlen - 32*r;
      int i;
      unsigned char d[64];

      i = 0;
      while (1)
        {
          memcpy (d, ctx.buf, 64);
          memcpy ((unsigned char *)output+i*32, d, 32);

          if (++i >= r)
            break;

          ec = blake2b_init_ctx (&ctx, 0, NULL, 0, 64*8);
          if (ec)
            return ec;

          blake2b_write (&ctx, d, 64);
          blake2b_final (&ctx);
        }

      ec = blake2b_init_ctx (&ctx, 0, NULL, 0, remained*8);
      if (ec)
        return ec;

      blake2b_write (&ctx, d, 64);
      blake2b_final (&ctx);

      memcpy ((unsigned char *)output+r*32, ctx.buf, remained);
    }

  wipememory (buf, sizeof (buf));
  wipememory (&ctx, sizeof (ctx));
  return 0;
}

static inline void blake2s_set_lastblock(BLAKE2S_STATE *S)
{
  S->f[0] = 0xFFFFFFFFUL;
}

static inline int blake2s_is_lastblock(BLAKE2S_STATE *S)
{
  return S->f[0] != 0;
}

static inline void blake2s_increment_counter(BLAKE2S_STATE *S, const int inc)
{
  S->t[0] += (u32)inc;
  S->t[1] += (S->t[0] < (u32)inc) - (inc < 0);
}

static unsigned int blake2s_transform_generic(BLAKE2S_STATE *S,
                                              const void *inblks,
                                              size_t nblks)
{
  static const byte blake2s_sigma[10][16] =
  {
    {  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15 },
    { 14, 10,  4,  8,  9, 15, 13,  6,  1, 12,  0,  2, 11,  7,  5,  3 },
    { 11,  8, 12,  0,  5,  2, 15, 13, 10, 14,  3,  6,  7,  1,  9,  4 },
    {  7,  9,  3,  1, 13, 12, 11, 14,  2,  6,  5, 10,  4,  0, 15,  8 },
    {  9,  0,  5,  7,  2,  4, 10, 15, 14,  1, 11, 12,  6,  8,  3, 13 },
    {  2, 12,  6, 10,  0, 11,  8,  3,  4, 13,  7,  5, 15, 14,  1,  9 },
    { 12,  5,  1, 15, 14, 13,  4, 10,  0,  7,  6,  3,  9,  2,  8, 11 },
    { 13, 11,  7, 14, 12,  1,  3,  9,  5,  0, 15,  4,  8,  6,  2, 10 },
    {  6, 15, 14,  9, 11,  3,  0,  8, 12,  2, 13,  7,  1,  4, 10,  5 },
    { 10,  2,  8,  4,  7,  6,  1,  5, 15, 11,  9, 14,  3, 12, 13 , 0 },
  };
  unsigned int burn = 0;
  const byte* in = inblks;
  u32 m[16];
  u32 v[16];

  while (nblks--)
    {
      /* Increment counter */
      blake2s_increment_counter (S, BLAKE2S_BLOCKBYTES);

      /* Compress */
      m[0] = buf_get_le32 (in + 0 * sizeof(m[0]));
      m[1] = buf_get_le32 (in + 1 * sizeof(m[0]));
      m[2] = buf_get_le32 (in + 2 * sizeof(m[0]));
      m[3] = buf_get_le32 (in + 3 * sizeof(m[0]));
      m[4] = buf_get_le32 (in + 4 * sizeof(m[0]));
      m[5] = buf_get_le32 (in + 5 * sizeof(m[0]));
      m[6] = buf_get_le32 (in + 6 * sizeof(m[0]));
      m[7] = buf_get_le32 (in + 7 * sizeof(m[0]));
      m[8] = buf_get_le32 (in + 8 * sizeof(m[0]));
      m[9] = buf_get_le32 (in + 9 * sizeof(m[0]));
      m[10] = buf_get_le32 (in + 10 * sizeof(m[0]));
      m[11] = buf_get_le32 (in + 11 * sizeof(m[0]));
      m[12] = buf_get_le32 (in + 12 * sizeof(m[0]));
      m[13] = buf_get_le32 (in + 13 * sizeof(m[0]));
      m[14] = buf_get_le32 (in + 14 * sizeof(m[0]));
      m[15] = buf_get_le32 (in + 15 * sizeof(m[0]));

      v[ 0] = S->h[0];
      v[ 1] = S->h[1];
      v[ 2] = S->h[2];
      v[ 3] = S->h[3];
      v[ 4] = S->h[4];
      v[ 5] = S->h[5];
      v[ 6] = S->h[6];
      v[ 7] = S->h[7];
      v[ 8] = blake2s_IV[0];
      v[ 9] = blake2s_IV[1];
      v[10] = blake2s_IV[2];
      v[11] = blake2s_IV[3];
      v[12] = S->t[0] ^ blake2s_IV[4];
      v[13] = S->t[1] ^ blake2s_IV[5];
      v[14] = S->f[0] ^ blake2s_IV[6];
      v[15] = S->f[1] ^ blake2s_IV[7];

#define G(r,i,a,b,c,d)                      \
  do {                                      \
    a = a + b + m[blake2s_sigma[r][2*i+0]]; \
    d = ror(d ^ a, 16);                     \
    c = c + d;                              \
    b = ror(b ^ c, 12);                     \
    a = a + b + m[blake2s_sigma[r][2*i+1]]; \
    d = ror(d ^ a, 8);                      \
    c = c + d;                              \
    b = ror(b ^ c, 7);                      \
  } while(0)

#define ROUND(r)                    \
  do {                              \
    G(r,0,v[ 0],v[ 4],v[ 8],v[12]); \
    G(r,1,v[ 1],v[ 5],v[ 9],v[13]); \
    G(r,2,v[ 2],v[ 6],v[10],v[14]); \
    G(r,3,v[ 3],v[ 7],v[11],v[15]); \
    G(r,4,v[ 0],v[ 5],v[10],v[15]); \
    G(r,5,v[ 1],v[ 6],v[11],v[12]); \
    G(r,6,v[ 2],v[ 7],v[ 8],v[13]); \
    G(r,7,v[ 3],v[ 4],v[ 9],v[14]); \
  } while(0)

      ROUND(0);
      ROUND(1);
      ROUND(2);
      ROUND(3);
      ROUND(4);
      ROUND(5);
      ROUND(6);
      ROUND(7);
      ROUND(8);
      ROUND(9);

#undef G
#undef ROUND

      S->h[0] = S->h[0] ^ v[0] ^ v[0 + 8];
      S->h[1] = S->h[1] ^ v[1] ^ v[1 + 8];
      S->h[2] = S->h[2] ^ v[2] ^ v[2 + 8];
      S->h[3] = S->h[3] ^ v[3] ^ v[3 + 8];
      S->h[4] = S->h[4] ^ v[4] ^ v[4 + 8];
      S->h[5] = S->h[5] ^ v[5] ^ v[5 + 8];
      S->h[6] = S->h[6] ^ v[6] ^ v[6 + 8];
      S->h[7] = S->h[7] ^ v[7] ^ v[7 + 8];

      in += BLAKE2S_BLOCKBYTES;
    }

  return burn;
}

#ifdef USE_AVX
unsigned int _gcry_blake2s_transform_amd64_avx(BLAKE2S_STATE *S,
                                               const void *inblks,
                                               size_t nblks) ASM_FUNC_ABI;
#endif

static unsigned int blake2s_transform(void *ctx, const void *inblks,
                                      size_t nblks)
{
  BLAKE2S_CONTEXT *c = ctx;
  unsigned int nburn;

  if (0)
    {}
#ifdef USE_AVX
  if (c->use_avx)
    nburn = _gcry_blake2s_transform_amd64_avx(&c->state, inblks, nblks);
#endif
  else
    nburn = blake2s_transform_generic(&c->state, inblks, nblks);

  if (nburn)
    nburn += ASM_EXTRA_STACK;

  return nburn;
}

static void blake2s_final(void *ctx)
{
  BLAKE2S_CONTEXT *c = ctx;
  BLAKE2S_STATE *S = &c->state;
  unsigned int burn;
  size_t i;

  gcry_assert (sizeof(c->buf) >= c->outlen);
  if (blake2s_is_lastblock(S))
    return;

  if (c->buflen < BLAKE2S_BLOCKBYTES)
    memset (c->buf + c->buflen, 0, BLAKE2S_BLOCKBYTES - c->buflen); /* Padding */
  blake2s_set_lastblock (S);
  blake2s_increment_counter (S, (int)c->buflen - BLAKE2S_BLOCKBYTES);
  burn = blake2s_transform (ctx, c->buf, 1);

  /* Output full hash to buffer */
  for (i = 0; i < 8; ++i)
    buf_put_le32 (c->buf + sizeof(S->h[i]) * i, S->h[i]);

  /* Zero out extra buffer bytes. */
  if (c->outlen < sizeof(c->buf))
    memset (c->buf + c->outlen, 0, sizeof(c->buf) - c->outlen);

  if (burn)
    _gcry_burn_stack (burn);
}

static byte *blake2s_read(void *ctx)
{
  BLAKE2S_CONTEXT *c = ctx;
  return c->buf;
}

static void blake2s_write(void *ctx, const void *inbuf, size_t inlen)
{
  BLAKE2S_CONTEXT *c = ctx;
  BLAKE2S_STATE *S = &c->state;
  blake2_write(S, inbuf, inlen, c->buf, &c->buflen, BLAKE2S_BLOCKBYTES,
	       blake2s_transform);
}

static inline void blake2s_init_param(BLAKE2S_STATE *S,
				      const struct blake2s_param_s *P)
{
  const byte *p = (const byte *)P;
  size_t i;

  /* init2 xors IV with input parameter block */

  /* IV XOR ParamBlock */
  for (i = 0; i < 8; ++i)
    S->h[i] ^= blake2s_IV[i] ^ buf_get_le32(&p[i * 4]);
}

static inline gcry_err_code_t blake2s_init(BLAKE2S_CONTEXT *ctx,
					   const byte *key, size_t keylen)
{
  struct blake2s_param_s P[1] = { { 0, } };
  BLAKE2S_STATE *S = &ctx->state;

  if (!ctx->outlen || ctx->outlen > BLAKE2S_OUTBYTES)
    return GPG_ERR_INV_ARG;
  if (sizeof(P[0]) != sizeof(u32) * 8)
    return GPG_ERR_INTERNAL;
  if (keylen && (!key || keylen > BLAKE2S_KEYBYTES))
    return GPG_ERR_INV_KEYLEN;

  P->digest_length = ctx->outlen;
  P->key_length = keylen;
  P->fanout = 1;
  P->depth = 1;

  blake2s_init_param (S, P);
  wipememory (P, sizeof(P));

  if (key)
    {
      blake2s_write (ctx, key, keylen);
      blake2s_write (ctx, zero_block, BLAKE2S_BLOCKBYTES - keylen);
    }

  return 0;
}

static gcry_err_code_t blake2s_init_ctx(void *ctx, unsigned int flags,
					const byte *key, size_t keylen,
					unsigned int dbits)
{
  BLAKE2S_CONTEXT *c = ctx;
  unsigned int features = _gcry_get_hw_features ();

  (void)features;
  (void)flags;

  memset (c, 0, sizeof (*c));

#ifdef USE_AVX
  c->use_avx = !!(features & HWF_INTEL_AVX);
#endif

  c->outlen = dbits / 8;
  c->buflen = 0;
  return blake2s_init(c, key, keylen);
}

/* Selftests from "RFC 7693, Appendix E. BLAKE2b and BLAKE2s Self-Test
 * Module C Source". */
static void selftest_seq(byte *out, size_t len, u32 seed)
{
  size_t i;
  u32 t, a, b;

  a = 0xDEAD4BAD * seed;
  b = 1;

  for (i = 0; i < len; i++)
    {
      t = a + b;
      a = b;
      b = t;
      out[i] = (t >> 24) & 0xFF;
    }
}

static gpg_err_code_t
selftests_blake2b (int algo, int extended, selftest_report_func_t report)
{
  static const byte blake2b_res[32] =
  {
    0xC2, 0x3A, 0x78, 0x00, 0xD9, 0x81, 0x23, 0xBD,
    0x10, 0xF5, 0x06, 0xC6, 0x1E, 0x29, 0xDA, 0x56,
    0x03, 0xD7, 0x63, 0xB8, 0xBB, 0xAD, 0x2E, 0x73,
    0x7F, 0x5E, 0x76, 0x5A, 0x7B, 0xCC, 0xD4, 0x75
  };
  static const size_t b2b_md_len[4] = { 20, 32, 48, 64 };
  static const size_t b2b_in_len[6] = { 0, 3, 128, 129, 255, 1024 };
  size_t i, j, outlen, inlen;
  byte in[1024], key[64];
  BLAKE2B_CONTEXT ctx;
  BLAKE2B_CONTEXT ctx2;
  const char *what;
  const char *errtxt;

  (void)extended;

  what = "rfc7693 BLAKE2b selftest";

  /* 256-bit hash for testing */
  if (blake2b_init_ctx(&ctx, 0, NULL, 0, 32 * 8))
    {
      errtxt = "init failed";
      goto failed;
    }

  for (i = 0; i < 4; i++)
    {
      outlen = b2b_md_len[i];
      for (j = 0; j < 6; j++)
	{
	  inlen = b2b_in_len[j];

	  selftest_seq(in, inlen, inlen); /* unkeyed hash */
	  blake2b_init_ctx(&ctx2, 0, NULL, 0, outlen * 8);
	  blake2b_write(&ctx2, in, inlen);
	  blake2b_final(&ctx2);
	  blake2b_write(&ctx, ctx2.buf, outlen); /* hash the hash */

	  selftest_seq(key, outlen, outlen); /* keyed hash */
	  blake2b_init_ctx(&ctx2, 0, key, outlen, outlen * 8);
	  blake2b_write(&ctx2, in, inlen);
	  blake2b_final(&ctx2);
	  blake2b_write(&ctx, ctx2.buf, outlen); /* hash the hash */
	}
    }

  /* compute and compare the hash of hashes */
  blake2b_final(&ctx);
  for (i = 0; i < 32; i++)
    {
      if (ctx.buf[i] != blake2b_res[i])
	{
	  errtxt = "digest mismatch";
	  goto failed;
	}
    }

  return 0;

failed:
  if (report)
    report ("digest", algo, what, errtxt);
  return GPG_ERR_SELFTEST_FAILED;
}

static gpg_err_code_t
selftests_blake2s (int algo, int extended, selftest_report_func_t report)
{
  static const byte blake2s_res[32] =
  {
    0x6A, 0x41, 0x1F, 0x08, 0xCE, 0x25, 0xAD, 0xCD,
    0xFB, 0x02, 0xAB, 0xA6, 0x41, 0x45, 0x1C, 0xEC,
    0x53, 0xC5, 0x98, 0xB2, 0x4F, 0x4F, 0xC7, 0x87,
    0xFB, 0xDC, 0x88, 0x79, 0x7F, 0x4C, 0x1D, 0xFE
  };
  static const size_t b2s_md_len[4] = { 16, 20, 28, 32 };
  static const size_t b2s_in_len[6] = { 0, 3, 64, 65, 255, 1024 };
  size_t i, j, outlen, inlen;
  byte in[1024], key[32];
  BLAKE2S_CONTEXT ctx;
  BLAKE2S_CONTEXT ctx2;
  const char *what;
  const char *errtxt;

  (void)extended;

  what = "rfc7693 BLAKE2s selftest";

  /* 256-bit hash for testing */
  if (blake2s_init_ctx(&ctx, 0, NULL, 0, 32 * 8))
    {
      errtxt = "init failed";
      goto failed;
    }

  for (i = 0; i < 4; i++)
    {
      outlen = b2s_md_len[i];
      for (j = 0; j < 6; j++)
	{
	  inlen = b2s_in_len[j];

	  selftest_seq(in, inlen, inlen); /* unkeyed hash */
	  blake2s_init_ctx(&ctx2, 0, NULL, 0, outlen * 8);
	  blake2s_write(&ctx2, in, inlen);
	  blake2s_final(&ctx2);
	  blake2s_write(&ctx, ctx2.buf, outlen); /* hash the hash */

	  selftest_seq(key, outlen, outlen); /* keyed hash */
	  blake2s_init_ctx(&ctx2, 0, key, outlen, outlen * 8);
	  blake2s_write(&ctx2, in, inlen);
	  blake2s_final(&ctx2);
	  blake2s_write(&ctx, ctx2.buf, outlen); /* hash the hash */
	}
    }

  /* compute and compare the hash of hashes */
  blake2s_final(&ctx);
  for (i = 0; i < 32; i++)
    {
      if (ctx.buf[i] != blake2s_res[i])
	{
	  errtxt = "digest mismatch";
	  goto failed;
	}
    }

  return 0;

failed:
  if (report)
    report ("digest", algo, what, errtxt);
  return GPG_ERR_SELFTEST_FAILED;
}


gcry_err_code_t _gcry_blake2_init_with_key(void *ctx, unsigned int flags,
					   const unsigned char *key,
					   size_t keylen, int algo)
{
  gcry_err_code_t rc;
  switch (algo)
    {
    case GCRY_MD_BLAKE2B_512:
      rc = blake2b_init_ctx (ctx, flags, key, keylen, 512);
      break;
    case GCRY_MD_BLAKE2B_384:
      rc = blake2b_init_ctx (ctx, flags, key, keylen, 384);
      break;
    case GCRY_MD_BLAKE2B_256:
      rc = blake2b_init_ctx (ctx, flags, key, keylen, 256);
      break;
    case GCRY_MD_BLAKE2B_160:
      rc = blake2b_init_ctx (ctx, flags, key, keylen, 160);
      break;
    case GCRY_MD_BLAKE2S_256:
      rc = blake2s_init_ctx (ctx, flags, key, keylen, 256);
      break;
    case GCRY_MD_BLAKE2S_224:
      rc = blake2s_init_ctx (ctx, flags, key, keylen, 224);
      break;
    case GCRY_MD_BLAKE2S_160:
      rc = blake2s_init_ctx (ctx, flags, key, keylen, 160);
      break;
    case GCRY_MD_BLAKE2S_128:
      rc = blake2s_init_ctx (ctx, flags, key, keylen, 128);
      break;
    default:
      rc = GPG_ERR_DIGEST_ALGO;
      break;
    }

  return rc;
}


#define DEFINE_BLAKE2_VARIANT(bs, BS, dbits, oid_branch) \
  static void blake2##bs##_##dbits##_init(void *ctx, unsigned int flags) \
  { \
    int err = blake2##bs##_init_ctx (ctx, flags, NULL, 0, dbits); \
    gcry_assert (err == 0); \
  } \
  static void \
  _gcry_blake2##bs##_##dbits##_hash_buffers(void *outbuf, size_t nbytes, \
        const gcry_buffer_t *iov, int iovcnt) \
  { \
    BLAKE2##BS##_CONTEXT hd; \
    (void)nbytes; \
    blake2##bs##_##dbits##_init (&hd, 0); \
    for (;iovcnt > 0; iov++, iovcnt--) \
      blake2##bs##_write (&hd, (const char*)iov[0].data + iov[0].off, \
                          iov[0].len); \
    blake2##bs##_final (&hd); \
    memcpy (outbuf, blake2##bs##_read (&hd), dbits / 8); \
  } \
  static const byte blake2##bs##_##dbits##_asn[] = { 0x30 }; \
  static const gcry_md_oid_spec_t oid_spec_blake2##bs##_##dbits[] = \
    { \
      { " 1.3.6.1.4.1.1722.12.2." oid_branch }, \
      { NULL } \
    }; \
  const gcry_md_spec_t _gcry_digest_spec_blake2##bs##_##dbits = \
    { \
      GCRY_MD_BLAKE2##BS##_##dbits, {0, 0}, \
      "BLAKE2" #BS "_" #dbits, blake2##bs##_##dbits##_asn, \
      DIM (blake2##bs##_##dbits##_asn), oid_spec_blake2##bs##_##dbits, \
      dbits / 8, blake2##bs##_##dbits##_init, blake2##bs##_write, \
      blake2##bs##_final, blake2##bs##_read, NULL, \
      _gcry_blake2##bs##_##dbits##_hash_buffers, \
      sizeof (BLAKE2##BS##_CONTEXT), selftests_blake2##bs \
    };

DEFINE_BLAKE2_VARIANT(b, B, 512, "1.16")
DEFINE_BLAKE2_VARIANT(b, B, 384, "1.12")
DEFINE_BLAKE2_VARIANT(b, B, 256, "1.8")
DEFINE_BLAKE2_VARIANT(b, B, 160, "1.5")

DEFINE_BLAKE2_VARIANT(s, S, 256, "2.8")
DEFINE_BLAKE2_VARIANT(s, S, 224, "2.7")
DEFINE_BLAKE2_VARIANT(s, S, 160, "2.5")
DEFINE_BLAKE2_VARIANT(s, S, 128, "2.4")
