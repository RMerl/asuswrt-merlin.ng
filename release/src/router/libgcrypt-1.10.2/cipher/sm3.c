/* sm3.c - SM3 hash function
 * Copyright (C) 2017 Jia Zhang
 *
 * This file is part of Libgcrypt.
 *
 * Libgcrypt is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
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


/*  Test vectors:

    "abc"
    SM3: 66c7f0f4 62eeedd9 d1f2d46b dc10e4e2 4167c487 5cf2f7a2 297da02b 8f4ba8e0

    "abcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcd"
    SM3: debe9ff9 2275b8a1 38604889 c18e5a4d 6fdb70e5 387e5765 293dcba3 9c0c5732

    "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq"
    SM3: 639b6cc5 e64d9e37 a390b192 df4fa1ea 0720ab74 7ff692b9 f38c4e66 ad7b8c05

    "a" one million times
    SM3: c8aaf894 29554029 e231941a 2acc0ad6 1ff2a5ac d8fadd25 847a3a73 2b3b02c3

 */


#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "g10lib.h"
#include "bithelp.h"
#include "bufhelp.h"
#include "cipher.h"
#include "hash-common.h"


/* USE_AVX_BMI2 indicates whether to compile with Intel AVX/BMI2 code. */
#undef USE_AVX_BMI2
#if defined(__x86_64__) && defined(HAVE_GCC_INLINE_ASM_AVX) && \
    defined(HAVE_GCC_INLINE_ASM_BMI2) && \
    (defined(HAVE_COMPATIBLE_GCC_AMD64_PLATFORM_AS) || \
     defined(HAVE_COMPATIBLE_GCC_WIN64_PLATFORM_AS))
# define USE_AVX_BMI2 1
#endif

/* USE_AARCH64_SIMD indicates whether to enable ARMv8 SIMD assembly
 * code. */
#undef USE_AARCH64_SIMD
#ifdef ENABLE_NEON_SUPPORT
# if defined(__AARCH64EL__) \
       && defined(HAVE_COMPATIBLE_GCC_AARCH64_PLATFORM_AS) \
       && defined(HAVE_GCC_INLINE_ASM_AARCH64_NEON)
#  define USE_AARCH64_SIMD 1
# endif
#endif


typedef struct {
  gcry_md_block_ctx_t bctx;
  u32 h[8];
} SM3_CONTEXT;


/* AMD64 assembly implementations use SystemV ABI, ABI conversion and additional
 * stack to store XMM6-XMM15 needed on Win64. */
#undef ASM_FUNC_ABI
#undef ASM_EXTRA_STACK
#if defined(USE_AVX_BMI2)
# ifdef HAVE_COMPATIBLE_GCC_WIN64_PLATFORM_AS
#  define ASM_FUNC_ABI __attribute__((sysv_abi))
#  define ASM_EXTRA_STACK (10 * 16 + 4 * sizeof(void *))
# else
#  define ASM_FUNC_ABI
#  define ASM_EXTRA_STACK 0
# endif
#endif


#ifdef USE_AVX_BMI2
unsigned int _gcry_sm3_transform_amd64_avx_bmi2(void *state,
                                                const void *input_data,
                                                size_t num_blks) ASM_FUNC_ABI;

static unsigned int
do_sm3_transform_amd64_avx_bmi2(void *context, const unsigned char *data,
                                size_t nblks)
{
  SM3_CONTEXT *hd = context;
  unsigned int nburn = _gcry_sm3_transform_amd64_avx_bmi2 (hd->h, data, nblks);
  nburn += nburn ? ASM_EXTRA_STACK : 0;
  return nburn;
}
#endif /* USE_AVX_BMI2 */

#ifdef USE_AARCH64_SIMD
unsigned int _gcry_sm3_transform_aarch64(void *state, const void *input_data,
                                         size_t num_blks);

static unsigned int
do_sm3_transform_aarch64(void *context, const unsigned char *data, size_t nblks)
{
  SM3_CONTEXT *hd = context;
  return _gcry_sm3_transform_aarch64 (hd->h, data, nblks);
}
#endif /* USE_AARCH64_SIMD */


static unsigned int
transform (void *c, const unsigned char *data, size_t nblks);


static void
sm3_init (void *context, unsigned int flags)
{
  SM3_CONTEXT *hd = context;
  unsigned int features = _gcry_get_hw_features ();

  (void)flags;

  hd->h[0] = 0x7380166f;
  hd->h[1] = 0x4914b2b9;
  hd->h[2] = 0x172442d7;
  hd->h[3] = 0xda8a0600;
  hd->h[4] = 0xa96f30bc;
  hd->h[5] = 0x163138aa;
  hd->h[6] = 0xe38dee4d;
  hd->h[7] = 0xb0fb0e4e;

  hd->bctx.nblocks = 0;
  hd->bctx.nblocks_high = 0;
  hd->bctx.count = 0;
  hd->bctx.blocksize_shift = _gcry_ctz(64);
  hd->bctx.bwrite = transform;

#ifdef USE_AVX_BMI2
  if ((features & HWF_INTEL_AVX2) && (features & HWF_INTEL_BMI2))
    hd->bctx.bwrite = do_sm3_transform_amd64_avx_bmi2;
#endif
#ifdef USE_AARCH64_SIMD
  if (features & HWF_ARM_NEON)
    hd->bctx.bwrite = do_sm3_transform_aarch64;
#endif

  (void)features;
}


/*
  Transform the message X which consists of 16 32-bit-words. See
  GM/T 004-2012 for details.  */
#define R(i,a,b,c,d,e,f,g,h,t,w1,w2) do                               \
          {                                                           \
            ss1 = rol ((rol ((a), 12) + (e) + (t)), 7);               \
            ss2 = ss1 ^ rol ((a), 12);                                \
            d += FF##i(a,b,c) + ss2 + ((w1) ^ (w2));                  \
            h += GG##i(e,f,g) + ss1 + (w1);                           \
            b = rol ((b), 9);                                         \
            f = rol ((f), 19);                                        \
            h = P0 ((h));                                             \
          } while (0)

#define R1(a,b,c,d,e,f,g,h,t,w1,w2) R(1,a,b,c,d,e,f,g,h,t,w1,w2)
#define R2(a,b,c,d,e,f,g,h,t,w1,w2) R(2,a,b,c,d,e,f,g,h,t,w1,w2)

#define FF1(x, y, z)  (x ^ y ^ z)

#define FF2(x, y, z)  ((x & y) | (x & z) | (y & z))

#define GG1(x, y, z)  (x ^ y ^ z)

#define GG2(x, y, z)  ((x & y) | ( ~x & z))

/* Message expansion */
#define P0(x) ((x) ^ rol ((x), 9) ^ rol ((x), 17))
#define P1(x) ((x) ^ rol ((x), 15) ^ rol ((x), 23))
#define I(i)  ( w[i] = buf_get_be32(data + i * 4) )
#define W1(i) ( w[i&0x0f] )
#define W2(i) ( w[i&0x0f] =   P1(w[i    &0x0f] \
                               ^ w[(i-9)&0x0f] \
                               ^ rol (w[(i-3)&0x0f], 15)) \
                            ^ rol (w[(i-13)&0x0f], 7) \
                            ^ w[(i-6)&0x0f] )

static unsigned int
transform_blk (void *ctx, const unsigned char *data)
{
  SM3_CONTEXT *hd = ctx;
  static const u32 K[64] = {
    0x79cc4519, 0xf3988a32, 0xe7311465, 0xce6228cb,
    0x9cc45197, 0x3988a32f, 0x7311465e, 0xe6228cbc,
    0xcc451979, 0x988a32f3, 0x311465e7, 0x6228cbce,
    0xc451979c, 0x88a32f39, 0x11465e73, 0x228cbce6,
    0x9d8a7a87, 0x3b14f50f, 0x7629ea1e, 0xec53d43c,
    0xd8a7a879, 0xb14f50f3, 0x629ea1e7, 0xc53d43ce,
    0x8a7a879d, 0x14f50f3b, 0x29ea1e76, 0x53d43cec,
    0xa7a879d8, 0x4f50f3b1, 0x9ea1e762, 0x3d43cec5,
    0x7a879d8a, 0xf50f3b14, 0xea1e7629, 0xd43cec53,
    0xa879d8a7, 0x50f3b14f, 0xa1e7629e, 0x43cec53d,
    0x879d8a7a, 0x0f3b14f5, 0x1e7629ea, 0x3cec53d4,
    0x79d8a7a8, 0xf3b14f50, 0xe7629ea1, 0xcec53d43,
    0x9d8a7a87, 0x3b14f50f, 0x7629ea1e, 0xec53d43c,
    0xd8a7a879, 0xb14f50f3, 0x629ea1e7, 0xc53d43ce,
    0x8a7a879d, 0x14f50f3b, 0x29ea1e76, 0x53d43cec,
    0xa7a879d8, 0x4f50f3b1, 0x9ea1e762, 0x3d43cec5
  };

  u32 a,b,c,d,e,f,g,h,ss1,ss2;
  u32 w[16];

  a = hd->h[0];
  b = hd->h[1];
  c = hd->h[2];
  d = hd->h[3];
  e = hd->h[4];
  f = hd->h[5];
  g = hd->h[6];
  h = hd->h[7];

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

  hd->h[0] ^= a;
  hd->h[1] ^= b;
  hd->h[2] ^= c;
  hd->h[3] ^= d;
  hd->h[4] ^= e;
  hd->h[5] ^= f;
  hd->h[6] ^= g;
  hd->h[7] ^= h;

  return /*burn_stack*/ 26*4+32;
}
#undef P0
#undef P1
#undef R
#undef R1
#undef R2

static unsigned int
transform (void *ctx, const unsigned char *data, size_t nblks)
{
  SM3_CONTEXT *hd = ctx;
  unsigned int burn;

  do
    {
      burn = transform_blk (hd, data);
      data += 64;
    }
  while (--nblks);

  return burn;
}


/*
   The routine finally terminates the computation and returns the
   digest.  The handle is prepared for a new cycle, but adding bytes
   to the handle will the destroy the returned buffer.  Returns: 32
   bytes with the message the digest.  */
static void
sm3_final(void *context)
{
  SM3_CONTEXT *hd = context;
  u32 t, th, msb, lsb;
  byte *p;
  unsigned int burn;

  t = hd->bctx.nblocks;
  if (sizeof t == sizeof hd->bctx.nblocks)
    th = hd->bctx.nblocks_high;
  else
    th = hd->bctx.nblocks >> 32;

  /* multiply by 64 to make a byte count */
  lsb = t << 6;
  msb = (th << 6) | (t >> 26);
  /* add the count */
  t = lsb;
  if ((lsb += hd->bctx.count) < t)
    msb++;
  /* multiply by 8 to make a bit count */
  t = lsb;
  lsb <<= 3;
  msb <<= 3;
  msb |= t >> 29;

  if (hd->bctx.count < 56)  /* enough room */
    {
      hd->bctx.buf[hd->bctx.count++] = 0x80; /* pad */
      if (hd->bctx.count < 56)
	memset (&hd->bctx.buf[hd->bctx.count], 0, 56 - hd->bctx.count);

      /* append the 64 bit count */
      buf_put_be32(hd->bctx.buf + 56, msb);
      buf_put_be32(hd->bctx.buf + 60, lsb);
      burn = (*hd->bctx.bwrite) ( hd, hd->bctx.buf, 1 );
    }
  else  /* need one extra block */
    {
      hd->bctx.buf[hd->bctx.count++] = 0x80; /* pad character */
      /* fill pad and next block with zeroes */
      memset (&hd->bctx.buf[hd->bctx.count], 0, 64 - hd->bctx.count + 56);

      /* append the 64 bit count */
      buf_put_be32(hd->bctx.buf + 64 + 56, msb);
      buf_put_be32(hd->bctx.buf + 64 + 60, lsb);
      burn = (*hd->bctx.bwrite) ( hd, hd->bctx.buf, 2 );
    }

  p = hd->bctx.buf;
#define X(a) do { buf_put_be32(p, hd->h[a]); p += 4; } while(0)
  X(0);
  X(1);
  X(2);
  X(3);
  X(4);
  X(5);
  X(6);
  X(7);
#undef X

  hd->bctx.count = 0;

  _gcry_burn_stack (burn);
}

static byte *
sm3_read (void *context)
{
  SM3_CONTEXT *hd = context;

  return hd->bctx.buf;
}


/* Shortcut functions which puts the hash value of the supplied buffer iov
 * into outbuf which must have a size of 32 bytes.  */
static void
_gcry_sm3_hash_buffers (void *outbuf, size_t nbytes,
			const gcry_buffer_t *iov, int iovcnt)
{
  SM3_CONTEXT hd;

  (void)nbytes;

  sm3_init (&hd, 0);
  for (;iovcnt > 0; iov++, iovcnt--)
    _gcry_md_block_write (&hd,
                          (const char*)iov[0].data + iov[0].off, iov[0].len);
  sm3_final (&hd);
  memcpy (outbuf, hd.bctx.buf, 32);
}



/*
     Self-test section.
 */


static gpg_err_code_t
selftests_sm3 (int extended, selftest_report_func_t report)
{
  const char *what;
  const char *errtxt;

  what = "short string (spec example 1)";
  errtxt = _gcry_hash_selftest_check_one
    (GCRY_MD_SM3, 0,
     "abc", 3,
     "\x66\xc7\xf0\xf4\x62\xee\xed\xd9\xd1\xf2\xd4\x6b\xdc\x10\xe4\xe2"
     "\x41\x67\xc4\x87\x5c\xf2\xf7\xa2\x29\x7d\xa0\x2b\x8f\x4b\xa8\xe0", 32);
  if (errtxt)
    goto failed;

  if (extended)
    {
      what = "long string (spec example 2)";
      errtxt = _gcry_hash_selftest_check_one
        (GCRY_MD_SM3, 0,
         "abcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcd", 64,
         "\xde\xbe\x9f\xf9\x22\x75\xb8\xa1\x38\x60\x48\x89\xc1\x8e\x5a\x4d"
         "\x6f\xdb\x70\xe5\x38\x7e\x57\x65\x29\x3d\xcb\xa3\x9c\x0c\x57\x32",
         32);
      if (errtxt)
        goto failed;

      what = "long string";
      errtxt = _gcry_hash_selftest_check_one
        (GCRY_MD_SM3, 0,
         "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq", 56,
         "\x63\x9b\x6c\xc5\xe6\x4d\x9e\x37\xa3\x90\xb1\x92\xdf\x4f\xa1\xea"
         "\x07\x20\xab\x74\x7f\xf6\x92\xb9\xf3\x8c\x4e\x66\xad\x7b\x8c\x05",
         32);
      if (errtxt)
        goto failed;

      what = "one million \"a\"";
      errtxt = _gcry_hash_selftest_check_one
        (GCRY_MD_SM3, 1,
         NULL, 0,
         "\xc8\xaa\xf8\x94\x29\x55\x40\x29\xe2\x31\x94\x1a\x2a\xcc\x0a\xd6"
         "\x1f\xf2\xa5\xac\xd8\xfa\xdd\x25\x84\x7a\x3a\x73\x2b\x3b\x02\xc3",
         32);
      if (errtxt)
        goto failed;
    }

  return 0; /* Succeeded. */

 failed:
  if (report)
    report ("digest", GCRY_MD_SM3, what, errtxt);
  return GPG_ERR_SELFTEST_FAILED;
}


/* Run a full self-test for ALGO and return 0 on success.  */
static gpg_err_code_t
run_selftests (int algo, int extended, selftest_report_func_t report)
{
  gpg_err_code_t ec;

  switch (algo)
    {
    case GCRY_MD_SM3:
      ec = selftests_sm3 (extended, report);
      break;
    default:
      ec = GPG_ERR_DIGEST_ALGO;
      break;

    }
  return ec;
}

static const byte asn_sm3[] = /* Object ID is 1.2.156.10197.401 */
  { 0x30, 0x2F, 0x30, 0x0B, 0x06, 0x07, 0x2A, 0x81,
    0x1C, 0xCF, 0x55, 0x83, 0x11, 0x05, 0x00, 0x04,
    0x20 };

static const gcry_md_oid_spec_t oid_spec_sm3[] =
  {
    /* China Electronics Standardization Instutute,
       OID White paper (2015), Table 6 */
    { "1.2.156.10197.401" },
    { NULL },
  };

const gcry_md_spec_t _gcry_digest_spec_sm3 =
  {
    GCRY_MD_SM3, {0, 0},
    "SM3", asn_sm3, DIM (asn_sm3), oid_spec_sm3, 32,
    sm3_init, _gcry_md_block_write, sm3_final, sm3_read, NULL,
    _gcry_sm3_hash_buffers,
    sizeof (SM3_CONTEXT),
    run_selftests
  };
