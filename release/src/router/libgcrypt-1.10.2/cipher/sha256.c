/* sha256.c - SHA256 hash function
 * Copyright (C) 2003, 2006, 2008, 2009 Free Software Foundation, Inc.
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
    SHA224: 23097d22 3405d822 8642a477 bda255b3 2aadbce4 bda0b3f7 e36c9da7
    SHA256: ba7816bf 8f01cfea 414140de 5dae2223 b00361a3 96177a9c b410ff61 f20015ad

    "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq"
    SHA224: 75388b16 512776cc 5dba5da1 fd890150 b0c6455c b4f58b19 52522525
    SHA256: 248d6a61 d20638b8 e5c02693 0c3e6039 a33ce459 64ff2167 f6ecedd4 19db06c1

    "a" one million times
    SHA224: 20794655 980c91d8 bbb4c1ea 97618a4b f03f4258 1948b2ee 4ee7ad67
    SHA256: cdc76e5c 9914fb92 81a1c7e2 84d73e67 f1809a48 a497200e 046d39cc c7112cd0

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


/* USE_SSSE3 indicates whether to compile with Intel SSSE3 code. */
#undef USE_SSSE3
#if defined(__x86_64__) && defined(HAVE_GCC_INLINE_ASM_SSSE3) && \
    defined(HAVE_INTEL_SYNTAX_PLATFORM_AS) && \
    (defined(HAVE_COMPATIBLE_GCC_AMD64_PLATFORM_AS) || \
     defined(HAVE_COMPATIBLE_GCC_WIN64_PLATFORM_AS))
# define USE_SSSE3 1
#endif

/* USE_AVX indicates whether to compile with Intel AVX code. */
#undef USE_AVX
#if defined(__x86_64__) && defined(HAVE_GCC_INLINE_ASM_AVX) && \
    defined(HAVE_INTEL_SYNTAX_PLATFORM_AS) && \
    (defined(HAVE_COMPATIBLE_GCC_AMD64_PLATFORM_AS) || \
     defined(HAVE_COMPATIBLE_GCC_WIN64_PLATFORM_AS))
# define USE_AVX 1
#endif

/* USE_AVX2 indicates whether to compile with Intel AVX2/BMI2 code. */
#undef USE_AVX2
#if defined(__x86_64__) && defined(HAVE_GCC_INLINE_ASM_AVX2) && \
    defined(HAVE_GCC_INLINE_ASM_BMI2) && \
    defined(HAVE_INTEL_SYNTAX_PLATFORM_AS) && \
    (defined(HAVE_COMPATIBLE_GCC_AMD64_PLATFORM_AS) || \
     defined(HAVE_COMPATIBLE_GCC_WIN64_PLATFORM_AS))
# define USE_AVX2 1
#endif

/* USE_SHAEXT indicates whether to compile with Intel SHA Extension code. */
#undef USE_SHAEXT
#if defined(HAVE_GCC_INLINE_ASM_SHAEXT) && \
    defined(HAVE_GCC_INLINE_ASM_SSE41) && \
    defined(ENABLE_SHAEXT_SUPPORT)
# define USE_SHAEXT 1
#endif

/* USE_ARM_CE indicates whether to enable ARMv8 Crypto Extension assembly
 * code. */
#undef USE_ARM_CE
#ifdef ENABLE_ARM_CRYPTO_SUPPORT
# if defined(HAVE_ARM_ARCH_V6) && defined(__ARMEL__) \
     && defined(HAVE_COMPATIBLE_GCC_ARM_PLATFORM_AS) \
     && defined(HAVE_GCC_INLINE_ASM_AARCH32_CRYPTO)
#  define USE_ARM_CE 1
# elif defined(__AARCH64EL__) \
       && defined(HAVE_COMPATIBLE_GCC_AARCH64_PLATFORM_AS) \
       && defined(HAVE_GCC_INLINE_ASM_AARCH64_CRYPTO)
#  define USE_ARM_CE 1
# endif
#endif

/* USE_PPC_CRYPTO indicates whether to enable PowerPC vector crypto
 * accelerated code. */
#undef USE_PPC_CRYPTO
#ifdef ENABLE_PPC_CRYPTO_SUPPORT
# if defined(HAVE_COMPATIBLE_CC_PPC_ALTIVEC) && \
     defined(HAVE_GCC_INLINE_ASM_PPC_ALTIVEC)
#  if __GNUC__ >= 4
#   define USE_PPC_CRYPTO 1
#  endif
# endif
#endif

/* USE_S390X_CRYPTO indicates whether to enable zSeries code. */
#undef USE_S390X_CRYPTO
#if defined(HAVE_GCC_INLINE_ASM_S390X)
# define USE_S390X_CRYPTO 1
#endif /* USE_S390X_CRYPTO */


typedef struct {
  gcry_md_block_ctx_t bctx;
  u32  h[8];
#ifdef USE_S390X_CRYPTO
  u32  final_len_msb, final_len_lsb; /* needs to be right after h[7]. */
  int  use_s390x_crypto;
#endif
} SHA256_CONTEXT;


/* Assembly implementations use SystemV ABI, ABI conversion and additional
 * stack to store XMM6-XMM15 needed on Win64. */
#undef ASM_FUNC_ABI
#undef ASM_EXTRA_STACK
#if defined(USE_SSSE3) || defined(USE_AVX) || defined(USE_AVX2) || \
    defined(USE_SHAEXT)
# ifdef HAVE_COMPATIBLE_GCC_WIN64_PLATFORM_AS
#  define ASM_FUNC_ABI __attribute__((sysv_abi))
#  define ASM_EXTRA_STACK (10 * 16 + sizeof(void *) * 4)
# else
#  define ASM_FUNC_ABI
#  define ASM_EXTRA_STACK 0
# endif
#endif


#ifdef USE_SSSE3
unsigned int _gcry_sha256_transform_amd64_ssse3(const void *input_data,
                                                u32 state[8],
                                                size_t num_blks) ASM_FUNC_ABI;

static unsigned int
do_sha256_transform_amd64_ssse3(void *ctx, const unsigned char *data,
                                size_t nblks)
{
  SHA256_CONTEXT *hd = ctx;
  return _gcry_sha256_transform_amd64_ssse3 (data, hd->h, nblks)
         + ASM_EXTRA_STACK;
}
#endif

#ifdef USE_AVX
unsigned int _gcry_sha256_transform_amd64_avx(const void *input_data,
                                              u32 state[8],
                                              size_t num_blks) ASM_FUNC_ABI;

static unsigned int
do_sha256_transform_amd64_avx(void *ctx, const unsigned char *data,
                              size_t nblks)
{
  SHA256_CONTEXT *hd = ctx;
  return _gcry_sha256_transform_amd64_avx (data, hd->h, nblks)
         + ASM_EXTRA_STACK;
}
#endif

#ifdef USE_AVX2
unsigned int _gcry_sha256_transform_amd64_avx2(const void *input_data,
                                               u32 state[8],
                                               size_t num_blks) ASM_FUNC_ABI;

static unsigned int
do_sha256_transform_amd64_avx2(void *ctx, const unsigned char *data,
                               size_t nblks)
{
  SHA256_CONTEXT *hd = ctx;
  return _gcry_sha256_transform_amd64_avx2 (data, hd->h, nblks)
         + ASM_EXTRA_STACK;
}
#endif

#ifdef USE_SHAEXT
/* Does not need ASM_FUNC_ABI */
unsigned int
_gcry_sha256_transform_intel_shaext(u32 state[8],
                                    const unsigned char *input_data,
                                    size_t num_blks);

static unsigned int
do_sha256_transform_intel_shaext(void *ctx, const unsigned char *data,
                                 size_t nblks)
{
  SHA256_CONTEXT *hd = ctx;
  return _gcry_sha256_transform_intel_shaext (hd->h, data, nblks);
}
#endif

#ifdef USE_ARM_CE
unsigned int _gcry_sha256_transform_armv8_ce(u32 state[8],
                                             const void *input_data,
                                             size_t num_blks);

static unsigned int
do_sha256_transform_armv8_ce(void *ctx, const unsigned char *data,
                             size_t nblks)
{
  SHA256_CONTEXT *hd = ctx;
  return _gcry_sha256_transform_armv8_ce (hd->h, data, nblks);
}
#endif

#ifdef USE_PPC_CRYPTO
unsigned int _gcry_sha256_transform_ppc8(u32 state[8],
					 const unsigned char *input_data,
					 size_t num_blks);

unsigned int _gcry_sha256_transform_ppc9(u32 state[8],
					 const unsigned char *input_data,
					 size_t num_blks);

static unsigned int
do_sha256_transform_ppc8(void *ctx, const unsigned char *data, size_t nblks)
{
  SHA256_CONTEXT *hd = ctx;
  return _gcry_sha256_transform_ppc8 (hd->h, data, nblks);
}

static unsigned int
do_sha256_transform_ppc9(void *ctx, const unsigned char *data, size_t nblks)
{
  SHA256_CONTEXT *hd = ctx;
  return _gcry_sha256_transform_ppc9 (hd->h, data, nblks);
}
#endif

#ifdef USE_S390X_CRYPTO
#include "asm-inline-s390x.h"

static unsigned int
do_sha256_transform_s390x (void *ctx, const unsigned char *data, size_t nblks)
{
  SHA256_CONTEXT *hd = ctx;

  kimd_execute (KMID_FUNCTION_SHA256, hd->h, data, nblks * 64);
  return 0;
}

static unsigned int
do_sha256_final_s390x (void *ctx, const unsigned char *data, size_t datalen,
		       u32 len_msb, u32 len_lsb)
{
  SHA256_CONTEXT *hd = ctx;

  /* Make sure that 'final_len' is positioned at correct offset relative
   * to 'h[0]'. This is because we are passing 'h[0]' pointer as start of
   * parameter block to 'klmd' instruction. */

  gcry_assert (offsetof (SHA256_CONTEXT, final_len_msb)
	       - offsetof (SHA256_CONTEXT, h[0]) == 8 * sizeof(u32));
  gcry_assert (offsetof (SHA256_CONTEXT, final_len_lsb)
	       - offsetof (SHA256_CONTEXT, final_len_msb) == 1 * sizeof(u32));

  hd->final_len_msb = len_msb;
  hd->final_len_lsb = len_lsb;

  klmd_execute (KMID_FUNCTION_SHA256, hd->h, data, datalen);
  return 0;
}
#endif


static unsigned int
do_transform_generic (void *ctx, const unsigned char *data, size_t nblks);


static void
sha256_common_init (SHA256_CONTEXT *hd)
{
  unsigned int features = _gcry_get_hw_features ();

  hd->bctx.nblocks = 0;
  hd->bctx.nblocks_high = 0;
  hd->bctx.count = 0;
  hd->bctx.blocksize_shift = _gcry_ctz(64);

  /* Order of feature checks is important here; last match will be
   * selected.  Keep slower implementations at the top and faster at
   * the bottom.  */
  hd->bctx.bwrite = do_transform_generic;
#ifdef USE_SSSE3
  if ((features & HWF_INTEL_SSSE3) != 0)
    hd->bctx.bwrite = do_sha256_transform_amd64_ssse3;
#endif
#ifdef USE_AVX
  /* AVX implementation uses SHLD which is known to be slow on non-Intel CPUs.
   * Therefore use this implementation on Intel CPUs only. */
  if ((features & HWF_INTEL_AVX) && (features & HWF_INTEL_FAST_SHLD))
    hd->bctx.bwrite = do_sha256_transform_amd64_avx;
#endif
#ifdef USE_AVX2
  if ((features & HWF_INTEL_AVX2) && (features & HWF_INTEL_BMI2))
    hd->bctx.bwrite = do_sha256_transform_amd64_avx2;
#endif
#ifdef USE_SHAEXT
  if ((features & HWF_INTEL_SHAEXT) && (features & HWF_INTEL_SSE4_1))
    hd->bctx.bwrite = do_sha256_transform_intel_shaext;
#endif
#ifdef USE_ARM_CE
  if ((features & HWF_ARM_SHA2) != 0)
    hd->bctx.bwrite = do_sha256_transform_armv8_ce;
#endif
#ifdef USE_PPC_CRYPTO
  if ((features & HWF_PPC_VCRYPTO) != 0)
    hd->bctx.bwrite = do_sha256_transform_ppc8;
  if ((features & HWF_PPC_VCRYPTO) != 0 && (features & HWF_PPC_ARCH_3_00) != 0)
    hd->bctx.bwrite = do_sha256_transform_ppc9;
#endif
#ifdef USE_S390X_CRYPTO
  hd->use_s390x_crypto = 0;
  if ((features & HWF_S390X_MSA) != 0)
    {
      if ((kimd_query () & km_function_to_mask (KMID_FUNCTION_SHA256)) &&
	  (klmd_query () & km_function_to_mask (KMID_FUNCTION_SHA256)))
	{
	  hd->bctx.bwrite = do_sha256_transform_s390x;
	  hd->use_s390x_crypto = 1;
	}
    }
#endif
  (void)features;
}


static void
sha256_init (void *context, unsigned int flags)
{
  SHA256_CONTEXT *hd = context;

  (void)flags;

  hd->h[0] = 0x6a09e667;
  hd->h[1] = 0xbb67ae85;
  hd->h[2] = 0x3c6ef372;
  hd->h[3] = 0xa54ff53a;
  hd->h[4] = 0x510e527f;
  hd->h[5] = 0x9b05688c;
  hd->h[6] = 0x1f83d9ab;
  hd->h[7] = 0x5be0cd19;

  sha256_common_init (hd);
}


static void
sha224_init (void *context, unsigned int flags)
{
  SHA256_CONTEXT *hd = context;

  (void)flags;

  hd->h[0] = 0xc1059ed8;
  hd->h[1] = 0x367cd507;
  hd->h[2] = 0x3070dd17;
  hd->h[3] = 0xf70e5939;
  hd->h[4] = 0xffc00b31;
  hd->h[5] = 0x68581511;
  hd->h[6] = 0x64f98fa7;
  hd->h[7] = 0xbefa4fa4;

  sha256_common_init (hd);
}


/*
  Transform the message X which consists of 16 32-bit-words. See FIPS
  180-2 for details.  */
#define R(a,b,c,d,e,f,g,h,k,w) do                                 \
          {                                                       \
            t1 = (h) + Sum1((e)) + Cho((e),(f),(g)) + (k) + (w);  \
            t2 = Sum0((a)) + Maj((a),(b),(c));                    \
            d += t1;                                              \
            h  = t1 + t2;                                         \
          } while (0)

/* (4.2) same as SHA-1's F1.  */
#define Cho(x, y, z)  (z ^ (x & (y ^ z)))

/* (4.3) same as SHA-1's F3 */
#define Maj(x, y, z)  ((x & y) + (z & (x ^ y)))

/* (4.4) */
#define Sum0(x)       (ror (x, 2) ^ ror (x, 13) ^ ror (x, 22))

/* (4.5) */
#define Sum1(x)       (ror (x, 6) ^ ror (x, 11) ^ ror (x, 25))

/* Message expansion */
#define S0(x) (ror ((x), 7) ^ ror ((x), 18) ^ ((x) >> 3))       /* (4.6) */
#define S1(x) (ror ((x), 17) ^ ror ((x), 19) ^ ((x) >> 10))     /* (4.7) */
#define I(i) ( w[i] = buf_get_be32(data + i * 4) )
#define W(i) ( w[i&0x0f] =    S1(w[(i-2) &0x0f]) \
                            +    w[(i-7) &0x0f]  \
                            + S0(w[(i-15)&0x0f]) \
                            +    w[(i-16)&0x0f] )

static unsigned int
do_transform_generic (void *ctx, const unsigned char *data, size_t nblks)
{
  SHA256_CONTEXT *hd = ctx;
  static const u32 K[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
    0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
    0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
    0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
    0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
    0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
    0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
    0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
    0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
  };

  do
    {

      u32 a,b,c,d,e,f,g,h,t1,t2;
      u32 w[16];

      a = hd->h[0];
      b = hd->h[1];
      c = hd->h[2];
      d = hd->h[3];
      e = hd->h[4];
      f = hd->h[5];
      g = hd->h[6];
      h = hd->h[7];

      R(a, b, c, d, e, f, g, h, K[0], I(0));
      R(h, a, b, c, d, e, f, g, K[1], I(1));
      R(g, h, a, b, c, d, e, f, K[2], I(2));
      R(f, g, h, a, b, c, d, e, K[3], I(3));
      R(e, f, g, h, a, b, c, d, K[4], I(4));
      R(d, e, f, g, h, a, b, c, K[5], I(5));
      R(c, d, e, f, g, h, a, b, K[6], I(6));
      R(b, c, d, e, f, g, h, a, K[7], I(7));
      R(a, b, c, d, e, f, g, h, K[8], I(8));
      R(h, a, b, c, d, e, f, g, K[9], I(9));
      R(g, h, a, b, c, d, e, f, K[10], I(10));
      R(f, g, h, a, b, c, d, e, K[11], I(11));
      R(e, f, g, h, a, b, c, d, K[12], I(12));
      R(d, e, f, g, h, a, b, c, K[13], I(13));
      R(c, d, e, f, g, h, a, b, K[14], I(14));
      R(b, c, d, e, f, g, h, a, K[15], I(15));

      R(a, b, c, d, e, f, g, h, K[16], W(16));
      R(h, a, b, c, d, e, f, g, K[17], W(17));
      R(g, h, a, b, c, d, e, f, K[18], W(18));
      R(f, g, h, a, b, c, d, e, K[19], W(19));
      R(e, f, g, h, a, b, c, d, K[20], W(20));
      R(d, e, f, g, h, a, b, c, K[21], W(21));
      R(c, d, e, f, g, h, a, b, K[22], W(22));
      R(b, c, d, e, f, g, h, a, K[23], W(23));
      R(a, b, c, d, e, f, g, h, K[24], W(24));
      R(h, a, b, c, d, e, f, g, K[25], W(25));
      R(g, h, a, b, c, d, e, f, K[26], W(26));
      R(f, g, h, a, b, c, d, e, K[27], W(27));
      R(e, f, g, h, a, b, c, d, K[28], W(28));
      R(d, e, f, g, h, a, b, c, K[29], W(29));
      R(c, d, e, f, g, h, a, b, K[30], W(30));
      R(b, c, d, e, f, g, h, a, K[31], W(31));

      R(a, b, c, d, e, f, g, h, K[32], W(32));
      R(h, a, b, c, d, e, f, g, K[33], W(33));
      R(g, h, a, b, c, d, e, f, K[34], W(34));
      R(f, g, h, a, b, c, d, e, K[35], W(35));
      R(e, f, g, h, a, b, c, d, K[36], W(36));
      R(d, e, f, g, h, a, b, c, K[37], W(37));
      R(c, d, e, f, g, h, a, b, K[38], W(38));
      R(b, c, d, e, f, g, h, a, K[39], W(39));
      R(a, b, c, d, e, f, g, h, K[40], W(40));
      R(h, a, b, c, d, e, f, g, K[41], W(41));
      R(g, h, a, b, c, d, e, f, K[42], W(42));
      R(f, g, h, a, b, c, d, e, K[43], W(43));
      R(e, f, g, h, a, b, c, d, K[44], W(44));
      R(d, e, f, g, h, a, b, c, K[45], W(45));
      R(c, d, e, f, g, h, a, b, K[46], W(46));
      R(b, c, d, e, f, g, h, a, K[47], W(47));

      R(a, b, c, d, e, f, g, h, K[48], W(48));
      R(h, a, b, c, d, e, f, g, K[49], W(49));
      R(g, h, a, b, c, d, e, f, K[50], W(50));
      R(f, g, h, a, b, c, d, e, K[51], W(51));
      R(e, f, g, h, a, b, c, d, K[52], W(52));
      R(d, e, f, g, h, a, b, c, K[53], W(53));
      R(c, d, e, f, g, h, a, b, K[54], W(54));
      R(b, c, d, e, f, g, h, a, K[55], W(55));
      R(a, b, c, d, e, f, g, h, K[56], W(56));
      R(h, a, b, c, d, e, f, g, K[57], W(57));
      R(g, h, a, b, c, d, e, f, K[58], W(58));
      R(f, g, h, a, b, c, d, e, K[59], W(59));
      R(e, f, g, h, a, b, c, d, K[60], W(60));
      R(d, e, f, g, h, a, b, c, K[61], W(61));
      R(c, d, e, f, g, h, a, b, K[62], W(62));
      R(b, c, d, e, f, g, h, a, K[63], W(63));

      hd->h[0] += a;
      hd->h[1] += b;
      hd->h[2] += c;
      hd->h[3] += d;
      hd->h[4] += e;
      hd->h[5] += f;
      hd->h[6] += g;
      hd->h[7] += h;

      data += 64;
    }
  while (--nblks);

  return 26*4 + 32 + 3 * sizeof(void*);
}

#undef S0
#undef S1
#undef R


/*
   The routine finally terminates the computation and returns the
   digest.  The handle is prepared for a new cycle, but adding bytes
   to the handle will the destroy the returned buffer.  Returns: 32
   bytes with the message the digest.  */
static void
sha256_final(void *context)
{
  SHA256_CONTEXT *hd = context;
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

  if (0)
    { }
#ifdef USE_S390X_CRYPTO
  else if (hd->use_s390x_crypto)
    {
      burn = do_sha256_final_s390x (hd, hd->bctx.buf, hd->bctx.count, msb, lsb);
    }
#endif
  else if (hd->bctx.count < 56)  /* enough room */
    {
      hd->bctx.buf[hd->bctx.count++] = 0x80; /* pad */
      if (hd->bctx.count < 56)
	memset (&hd->bctx.buf[hd->bctx.count], 0, 56 - hd->bctx.count);

      /* append the 64 bit count */
      buf_put_be32(hd->bctx.buf + 56, msb);
      buf_put_be32(hd->bctx.buf + 60, lsb);
      burn = (*hd->bctx.bwrite) (hd, hd->bctx.buf, 1);
    }
  else  /* need one extra block */
    {
      hd->bctx.buf[hd->bctx.count++] = 0x80; /* pad character */
      /* fill pad and next block with zeroes */
      memset (&hd->bctx.buf[hd->bctx.count], 0, 64 - hd->bctx.count + 56);

      /* append the 64 bit count */
      buf_put_be32(hd->bctx.buf + 64 + 56, msb);
      buf_put_be32(hd->bctx.buf + 64 + 60, lsb);
      burn = (*hd->bctx.bwrite) (hd, hd->bctx.buf, 2);
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
sha256_read (void *context)
{
  SHA256_CONTEXT *hd = context;

  return hd->bctx.buf;
}


/* Shortcut functions which puts the hash value of the supplied buffer iov
 * into outbuf which must have a size of 32 bytes.  */
static void
_gcry_sha256_hash_buffers (void *outbuf, size_t nbytes,
			   const gcry_buffer_t *iov, int iovcnt)
{
  SHA256_CONTEXT hd;

  (void)nbytes;

  sha256_init (&hd, 0);
  for (;iovcnt > 0; iov++, iovcnt--)
    _gcry_md_block_write (&hd,
                          (const char*)iov[0].data + iov[0].off, iov[0].len);
  sha256_final (&hd);
  memcpy (outbuf, hd.bctx.buf, 32);
}


/* Shortcut functions which puts the hash value of the supplied buffer iov
 * into outbuf which must have a size of 28 bytes.  */
static void
_gcry_sha224_hash_buffers (void *outbuf, size_t nbytes,
			   const gcry_buffer_t *iov, int iovcnt)
{
  SHA256_CONTEXT hd;

  (void)nbytes;

  sha224_init (&hd, 0);
  for (;iovcnt > 0; iov++, iovcnt--)
    _gcry_md_block_write (&hd,
                          (const char*)iov[0].data + iov[0].off, iov[0].len);
  sha256_final (&hd);
  memcpy (outbuf, hd.bctx.buf, 28);
}



/*
     Self-test section.
 */


static gpg_err_code_t
selftests_sha224 (int extended, selftest_report_func_t report)
{
  const char *what;
  const char *errtxt;

  what = "short string";
  errtxt = _gcry_hash_selftest_check_one
    (GCRY_MD_SHA224, 0,
     "abc", 3,
     "\x23\x09\x7d\x22\x34\x05\xd8\x22\x86\x42\xa4\x77\xbd\xa2\x55\xb3"
     "\x2a\xad\xbc\xe4\xbd\xa0\xb3\xf7\xe3\x6c\x9d\xa7", 28);
  if (errtxt)
    goto failed;

  if (extended)
    {
      what = "long string";
      errtxt = _gcry_hash_selftest_check_one
        (GCRY_MD_SHA224, 0,
         "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq", 56,
         "\x75\x38\x8b\x16\x51\x27\x76\xcc\x5d\xba\x5d\xa1\xfd\x89\x01\x50"
         "\xb0\xc6\x45\x5c\xb4\xf5\x8b\x19\x52\x52\x25\x25", 28);
      if (errtxt)
        goto failed;

      what = "one million \"a\"";
      errtxt = _gcry_hash_selftest_check_one
        (GCRY_MD_SHA224, 1,
         NULL, 0,
         "\x20\x79\x46\x55\x98\x0c\x91\xd8\xbb\xb4\xc1\xea\x97\x61\x8a\x4b"
         "\xf0\x3f\x42\x58\x19\x48\xb2\xee\x4e\xe7\xad\x67", 28);
      if (errtxt)
        goto failed;
    }

  return 0; /* Succeeded. */

 failed:
  if (report)
    report ("digest", GCRY_MD_SHA224, what, errtxt);
  return GPG_ERR_SELFTEST_FAILED;
}

static gpg_err_code_t
selftests_sha256 (int extended, selftest_report_func_t report)
{
  const char *what;
  const char *errtxt;

  what = "short string";
  errtxt = _gcry_hash_selftest_check_one
    (GCRY_MD_SHA256, 0,
     "abc", 3,
     "\xba\x78\x16\xbf\x8f\x01\xcf\xea\x41\x41\x40\xde\x5d\xae\x22\x23"
     "\xb0\x03\x61\xa3\x96\x17\x7a\x9c\xb4\x10\xff\x61\xf2\x00\x15\xad", 32);
  if (errtxt)
    goto failed;

  if (extended)
    {
      what = "long string";
      errtxt = _gcry_hash_selftest_check_one
        (GCRY_MD_SHA256, 0,
         "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq", 56,
         "\x24\x8d\x6a\x61\xd2\x06\x38\xb8\xe5\xc0\x26\x93\x0c\x3e\x60\x39"
         "\xa3\x3c\xe4\x59\x64\xff\x21\x67\xf6\xec\xed\xd4\x19\xdb\x06\xc1",
         32);
      if (errtxt)
        goto failed;

      what = "one million \"a\"";
      errtxt = _gcry_hash_selftest_check_one
        (GCRY_MD_SHA256, 1,
         NULL, 0,
         "\xcd\xc7\x6e\x5c\x99\x14\xfb\x92\x81\xa1\xc7\xe2\x84\xd7\x3e\x67"
         "\xf1\x80\x9a\x48\xa4\x97\x20\x0e\x04\x6d\x39\xcc\xc7\x11\x2c\xd0",
         32);
      if (errtxt)
        goto failed;
    }

  return 0; /* Succeeded. */

 failed:
  if (report)
    report ("digest", GCRY_MD_SHA256, what, errtxt);
  return GPG_ERR_SELFTEST_FAILED;
}


/* Run a full self-test for ALGO and return 0 on success.  */
static gpg_err_code_t
run_selftests (int algo, int extended, selftest_report_func_t report)
{
  gpg_err_code_t ec;

  switch (algo)
    {
    case GCRY_MD_SHA224:
      ec = selftests_sha224 (extended, report);
      break;
    case GCRY_MD_SHA256:
      ec = selftests_sha256 (extended, report);
      break;
    default:
      ec = GPG_ERR_DIGEST_ALGO;
      break;

    }
  return ec;
}




static const byte asn224[19] = /* Object ID is 2.16.840.1.101.3.4.2.4 */
  { 0x30, 0x2D, 0x30, 0x0d, 0x06, 0x09, 0x60, 0x86, 0x48,
    0x01, 0x65, 0x03, 0x04, 0x02, 0x04, 0x05, 0x00, 0x04,
    0x1C
  };

static const gcry_md_oid_spec_t oid_spec_sha224[] =
  {
    /* From RFC3874, Section 4 */
    { "2.16.840.1.101.3.4.2.4" },
    /* ANSI X9.62  ecdsaWithSHA224 */
    { "1.2.840.10045.4.3.1" },
    { NULL },
  };

static const byte asn256[19] = /* Object ID is  2.16.840.1.101.3.4.2.1 */
  { 0x30, 0x31, 0x30, 0x0d, 0x06, 0x09, 0x60, 0x86,
    0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x01, 0x05,
    0x00, 0x04, 0x20 };

static const gcry_md_oid_spec_t oid_spec_sha256[] =
  {
    /* According to the OpenPGP draft rfc2440-bis06 */
    { "2.16.840.1.101.3.4.2.1" },
    /* PKCS#1 sha256WithRSAEncryption */
    { "1.2.840.113549.1.1.11" },
    /* ANSI X9.62  ecdsaWithSHA256 */
    { "1.2.840.10045.4.3.2" },

    { NULL },
  };

const gcry_md_spec_t _gcry_digest_spec_sha224 =
  {
    GCRY_MD_SHA224, {0, 1},
    "SHA224", asn224, DIM (asn224), oid_spec_sha224, 28,
    sha224_init, _gcry_md_block_write, sha256_final, sha256_read, NULL,
    _gcry_sha224_hash_buffers,
    sizeof (SHA256_CONTEXT),
    run_selftests
  };

const gcry_md_spec_t _gcry_digest_spec_sha256 =
  {
    GCRY_MD_SHA256, {0, 1},
    "SHA256", asn256, DIM (asn256), oid_spec_sha256, 32,
    sha256_init, _gcry_md_block_write, sha256_final, sha256_read, NULL,
    _gcry_sha256_hash_buffers,
    sizeof (SHA256_CONTEXT),
    run_selftests
  };
