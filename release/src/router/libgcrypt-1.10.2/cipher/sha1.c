/* sha1.c - SHA1 hash function
 * Copyright (C) 1998, 2001, 2002, 2003, 2008 Free Software Foundation, Inc.
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
 *
 *  "abc"
 *  A999 3E36 4706 816A BA3E  2571 7850 C26C 9CD0 D89D
 *
 *  "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq"
 *  8498 3E44 1C3B D26E BAAE  4AA1 F951 29E5 E546 70F1
 */


#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef HAVE_STDINT_H
# include <stdint.h>
#endif

#include "g10lib.h"
#include "bithelp.h"
#include "bufhelp.h"
#include "cipher.h"
#include "sha1.h"


/* USE_SSSE3 indicates whether to compile with Intel SSSE3 code. */
#undef USE_SSSE3
#if defined(__x86_64__) && defined(HAVE_GCC_INLINE_ASM_SSSE3) && \
    (defined(HAVE_COMPATIBLE_GCC_AMD64_PLATFORM_AS) || \
     defined(HAVE_COMPATIBLE_GCC_WIN64_PLATFORM_AS))
# define USE_SSSE3 1
#endif

/* USE_AVX indicates whether to compile with Intel AVX code. */
#undef USE_AVX
#if defined(__x86_64__) && defined(HAVE_GCC_INLINE_ASM_AVX) && \
    (defined(HAVE_COMPATIBLE_GCC_AMD64_PLATFORM_AS) || \
     defined(HAVE_COMPATIBLE_GCC_WIN64_PLATFORM_AS))
# define USE_AVX 1
#endif

/* USE_BMI2 indicates whether to compile with Intel AVX/BMI2 code. */
#undef USE_BMI2
#if defined(__x86_64__) && defined(HAVE_GCC_INLINE_ASM_AVX) && \
    defined(HAVE_GCC_INLINE_ASM_BMI2) && \
    (defined(HAVE_COMPATIBLE_GCC_AMD64_PLATFORM_AS) || \
     defined(HAVE_COMPATIBLE_GCC_WIN64_PLATFORM_AS))
# define USE_BMI2 1
#endif

/* USE_AVX2 indicates whether to compile with Intel AVX2/BMI2 code. */
#undef USE_AVX2
#if defined(USE_BMI2) && defined(HAVE_GCC_INLINE_ASM_AVX2)
# define USE_AVX2 1
#endif

/* USE_SHAEXT indicates whether to compile with Intel SHA Extension code. */
#undef USE_SHAEXT
#if defined(HAVE_GCC_INLINE_ASM_SHAEXT) && \
    defined(HAVE_GCC_INLINE_ASM_SSE41) && \
    defined(ENABLE_SHAEXT_SUPPORT)
# define USE_SHAEXT 1
#endif

/* USE_NEON indicates whether to enable ARM NEON assembly code. */
#undef USE_NEON
#ifdef ENABLE_NEON_SUPPORT
# if defined(HAVE_ARM_ARCH_V6) && defined(__ARMEL__) \
     && defined(HAVE_COMPATIBLE_GCC_ARM_PLATFORM_AS) \
     && defined(HAVE_GCC_INLINE_ASM_NEON)
#  define USE_NEON 1
# endif
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


/* A macro to test whether P is properly aligned for an u32 type.
   Note that config.h provides a suitable replacement for uintptr_t if
   it does not exist in stdint.h.  */
/* #if __GNUC__ >= 2 */
/* # define U32_ALIGNED_P(p) (!(((uintptr_t)p) % __alignof__ (u32))) */
/* #else */
/* # define U32_ALIGNED_P(p) (!(((uintptr_t)p) % sizeof (u32))) */
/* #endif */



/* Assembly implementations use SystemV ABI, ABI conversion and additional
 * stack to store XMM6-XMM15 needed on Win64. */
#undef ASM_FUNC_ABI
#undef ASM_EXTRA_STACK
#if defined(USE_SSSE3) || defined(USE_AVX) || defined(USE_BMI2) || \
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
unsigned int
_gcry_sha1_transform_amd64_ssse3 (void *state, const unsigned char *data,
                                  size_t nblks) ASM_FUNC_ABI;

static unsigned int
do_sha1_transform_amd64_ssse3 (void *ctx, const unsigned char *data,
                               size_t nblks)
{
  SHA1_CONTEXT *hd = ctx;
  return _gcry_sha1_transform_amd64_ssse3 (&hd->h0, data, nblks)
         + ASM_EXTRA_STACK;
}
#endif

#ifdef USE_AVX
unsigned int
_gcry_sha1_transform_amd64_avx (void *state, const unsigned char *data,
                                 size_t nblks) ASM_FUNC_ABI;

static unsigned int
do_sha1_transform_amd64_avx (void *ctx, const unsigned char *data,
                             size_t nblks)
{
  SHA1_CONTEXT *hd = ctx;
  return _gcry_sha1_transform_amd64_avx (&hd->h0, data, nblks)
         + ASM_EXTRA_STACK;
}
#endif

#ifdef USE_BMI2
unsigned int
_gcry_sha1_transform_amd64_avx_bmi2 (void *state, const unsigned char *data,
                                     size_t nblks) ASM_FUNC_ABI;

static unsigned int
do_sha1_transform_amd64_avx_bmi2 (void *ctx, const unsigned char *data,
                                  size_t nblks)
{
  SHA1_CONTEXT *hd = ctx;
  return _gcry_sha1_transform_amd64_avx_bmi2 (&hd->h0, data, nblks)
         + ASM_EXTRA_STACK;
}

#ifdef USE_AVX2
unsigned int
_gcry_sha1_transform_amd64_avx2_bmi2 (void *state, const unsigned char *data,
                                      size_t nblks) ASM_FUNC_ABI;

static unsigned int
do_sha1_transform_amd64_avx2_bmi2 (void *ctx, const unsigned char *data,
                                   size_t nblks)
{
  SHA1_CONTEXT *hd = ctx;

  /* AVX2/BMI2 function only handles pair of blocks so nblks needs to be
   * multiple of 2 and function does not handle zero nblks. Use AVX/BMI2
   * code to handle these cases. */

  if (nblks <= 1)
    return do_sha1_transform_amd64_avx_bmi2 (ctx, data, nblks);

  if (nblks & 1)
    {
      (void)_gcry_sha1_transform_amd64_avx_bmi2 (&hd->h0, data, 1);
      nblks--;
      data += 64;
    }

  return _gcry_sha1_transform_amd64_avx2_bmi2 (&hd->h0, data, nblks)
         + ASM_EXTRA_STACK;
}
#endif /* USE_AVX2 */
#endif /* USE_BMI2 */

#ifdef USE_SHAEXT
/* Does not need ASM_FUNC_ABI */
unsigned int
_gcry_sha1_transform_intel_shaext (void *state, const unsigned char *data,
                                   size_t nblks);

static unsigned int
do_sha1_transform_intel_shaext (void *ctx, const unsigned char *data,
                                size_t nblks)
{
  SHA1_CONTEXT *hd = ctx;
  return _gcry_sha1_transform_intel_shaext (&hd->h0, data, nblks);
}
#endif

#ifdef USE_NEON
unsigned int
_gcry_sha1_transform_armv7_neon (void *state, const unsigned char *data,
                                 size_t nblks);

static unsigned int
do_sha1_transform_armv7_neon (void *ctx, const unsigned char *data,
                              size_t nblks)
{
  SHA1_CONTEXT *hd = ctx;
  return _gcry_sha1_transform_armv7_neon (&hd->h0, data, nblks);
}
#endif

#ifdef USE_ARM_CE
unsigned int
_gcry_sha1_transform_armv8_ce (void *state, const unsigned char *data,
                               size_t nblks);

static unsigned int
do_sha1_transform_armv8_ce (void *ctx, const unsigned char *data,
                            size_t nblks)
{
  SHA1_CONTEXT *hd = ctx;
  return _gcry_sha1_transform_armv8_ce (&hd->h0, data, nblks);
}
#endif

#ifdef SHA1_USE_S390X_CRYPTO
#include "asm-inline-s390x.h"

static unsigned int
do_sha1_transform_s390x (void *ctx, const unsigned char *data, size_t nblks)
{
  SHA1_CONTEXT *hd = ctx;

  kimd_execute (KMID_FUNCTION_SHA1, &hd->h0, data, nblks * 64);
  return 0;
}

static unsigned int
do_sha1_final_s390x (void *ctx, const unsigned char *data, size_t datalen,
		     u32 len_msb, u32 len_lsb)
{
  SHA1_CONTEXT *hd = ctx;

  /* Make sure that 'final_len' is positioned at correct offset relative
   * to 'h0'. This is because we are passing 'h0' pointer as start of
   * parameter block to 'klmd' instruction. */

  gcry_assert (offsetof (SHA1_CONTEXT, final_len_msb)
	       - offsetof (SHA1_CONTEXT, h0) == 5 * sizeof(u32));
  gcry_assert (offsetof (SHA1_CONTEXT, final_len_lsb)
	       - offsetof (SHA1_CONTEXT, final_len_msb) == 1 * sizeof(u32));

  hd->final_len_msb = len_msb;
  hd->final_len_lsb = len_lsb;

  klmd_execute (KMID_FUNCTION_SHA1, &hd->h0, data, datalen);
  return 0;
}
#endif


static unsigned int
do_transform_generic (void *c, const unsigned char *data, size_t nblks);


static void
sha1_init (void *context, unsigned int flags)
{
  SHA1_CONTEXT *hd = context;
  unsigned int features = _gcry_get_hw_features ();

  (void)flags;

  hd->h0 = 0x67452301;
  hd->h1 = 0xefcdab89;
  hd->h2 = 0x98badcfe;
  hd->h3 = 0x10325476;
  hd->h4 = 0xc3d2e1f0;

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
    hd->bctx.bwrite = do_sha1_transform_amd64_ssse3;
#endif
#ifdef USE_AVX
  /* AVX implementation uses SHLD which is known to be slow on non-Intel CPUs.
   * Therefore use this implementation on Intel CPUs only. */
  if ((features & HWF_INTEL_AVX) && (features & HWF_INTEL_FAST_SHLD))
    hd->bctx.bwrite = do_sha1_transform_amd64_avx;
#endif
#ifdef USE_BMI2
  if ((features & HWF_INTEL_AVX) && (features & HWF_INTEL_BMI2))
    hd->bctx.bwrite = do_sha1_transform_amd64_avx_bmi2;
#endif
#ifdef USE_AVX2
  if ((features & HWF_INTEL_AVX2) && (features & HWF_INTEL_AVX) &&
      (features & HWF_INTEL_BMI2))
    hd->bctx.bwrite = do_sha1_transform_amd64_avx2_bmi2;
#endif
#ifdef USE_SHAEXT
  if ((features & HWF_INTEL_SHAEXT) && (features & HWF_INTEL_SSE4_1))
    hd->bctx.bwrite = do_sha1_transform_intel_shaext;
#endif
#ifdef USE_NEON
  if ((features & HWF_ARM_NEON) != 0)
    hd->bctx.bwrite = do_sha1_transform_armv7_neon;
#endif
#ifdef USE_ARM_CE
  if ((features & HWF_ARM_SHA1) != 0)
    hd->bctx.bwrite = do_sha1_transform_armv8_ce;
#endif
#ifdef SHA1_USE_S390X_CRYPTO
  hd->use_s390x_crypto = 0;
  if ((features & HWF_S390X_MSA) != 0)
    {
      if ((kimd_query () & km_function_to_mask (KMID_FUNCTION_SHA1)) &&
	  (klmd_query () & km_function_to_mask (KMID_FUNCTION_SHA1)))
	{
	  hd->bctx.bwrite = do_sha1_transform_s390x;
	  hd->use_s390x_crypto = 1;
	}
    }
#endif

  (void)features;
}

/*
 * Initialize the context HD. This is used to prepare the use of
 * _gcry_sha1_mixblock.  WARNING: This is a special purpose function
 * for exclusive use by random-csprng.c.
 */
void
_gcry_sha1_mixblock_init (SHA1_CONTEXT *hd)
{
  sha1_init (hd, 0);
}


/* Round function macros. */
#define K1  0x5A827999L
#define K2  0x6ED9EBA1L
#define K3  0x8F1BBCDCL
#define K4  0xCA62C1D6L
#define F1(x,y,z)   ( z ^ ( x & ( y ^ z ) ) )
#define F2(x,y,z)   ( x ^ y ^ z )
#define F3(x,y,z)   ( ( x & y ) | ( z & ( x | y ) ) )
#define F4(x,y,z)   ( x ^ y ^ z )
#define M(i) ( tm =    x[ i    &0x0f]  \
                     ^ x[(i-14)&0x0f]  \
	 	     ^ x[(i-8) &0x0f]  \
                     ^ x[(i-3) &0x0f], \
                     (x[i&0x0f] = rol(tm, 1)))
#define R(a,b,c,d,e,f,k,m)  do { e += rol( a, 5 )     \
	                              + f( b, c, d )  \
		 		      + k	      \
			 	      + m;	      \
				 b = rol( b, 30 );    \
			       } while(0)

/*
 * Transform NBLOCKS of each 64 bytes (16 32-bit words) at DATA.
 */
static unsigned int
do_transform_generic (void *ctx, const unsigned char *data, size_t nblks)
{
  SHA1_CONTEXT *hd = ctx;

  do
    {
      const u32 *idata = (const void *)data;
      u32 a, b, c, d, e; /* Local copies of the chaining variables.  */
      u32 tm;            /* Helper.  */
      u32 x[16];         /* The array we work on. */

#define I(i) (x[i] = buf_get_be32(idata + i))

      /* Get the values of the chaining variables. */
      a = hd->h0;
      b = hd->h1;
      c = hd->h2;
      d = hd->h3;
      e = hd->h4;

      /* Transform. */
      R( a, b, c, d, e, F1, K1, I( 0) );
      R( e, a, b, c, d, F1, K1, I( 1) );
      R( d, e, a, b, c, F1, K1, I( 2) );
      R( c, d, e, a, b, F1, K1, I( 3) );
      R( b, c, d, e, a, F1, K1, I( 4) );
      R( a, b, c, d, e, F1, K1, I( 5) );
      R( e, a, b, c, d, F1, K1, I( 6) );
      R( d, e, a, b, c, F1, K1, I( 7) );
      R( c, d, e, a, b, F1, K1, I( 8) );
      R( b, c, d, e, a, F1, K1, I( 9) );
      R( a, b, c, d, e, F1, K1, I(10) );
      R( e, a, b, c, d, F1, K1, I(11) );
      R( d, e, a, b, c, F1, K1, I(12) );
      R( c, d, e, a, b, F1, K1, I(13) );
      R( b, c, d, e, a, F1, K1, I(14) );
      R( a, b, c, d, e, F1, K1, I(15) );
      R( e, a, b, c, d, F1, K1, M(16) );
      R( d, e, a, b, c, F1, K1, M(17) );
      R( c, d, e, a, b, F1, K1, M(18) );
      R( b, c, d, e, a, F1, K1, M(19) );
      R( a, b, c, d, e, F2, K2, M(20) );
      R( e, a, b, c, d, F2, K2, M(21) );
      R( d, e, a, b, c, F2, K2, M(22) );
      R( c, d, e, a, b, F2, K2, M(23) );
      R( b, c, d, e, a, F2, K2, M(24) );
      R( a, b, c, d, e, F2, K2, M(25) );
      R( e, a, b, c, d, F2, K2, M(26) );
      R( d, e, a, b, c, F2, K2, M(27) );
      R( c, d, e, a, b, F2, K2, M(28) );
      R( b, c, d, e, a, F2, K2, M(29) );
      R( a, b, c, d, e, F2, K2, M(30) );
      R( e, a, b, c, d, F2, K2, M(31) );
      R( d, e, a, b, c, F2, K2, M(32) );
      R( c, d, e, a, b, F2, K2, M(33) );
      R( b, c, d, e, a, F2, K2, M(34) );
      R( a, b, c, d, e, F2, K2, M(35) );
      R( e, a, b, c, d, F2, K2, M(36) );
      R( d, e, a, b, c, F2, K2, M(37) );
      R( c, d, e, a, b, F2, K2, M(38) );
      R( b, c, d, e, a, F2, K2, M(39) );
      R( a, b, c, d, e, F3, K3, M(40) );
      R( e, a, b, c, d, F3, K3, M(41) );
      R( d, e, a, b, c, F3, K3, M(42) );
      R( c, d, e, a, b, F3, K3, M(43) );
      R( b, c, d, e, a, F3, K3, M(44) );
      R( a, b, c, d, e, F3, K3, M(45) );
      R( e, a, b, c, d, F3, K3, M(46) );
      R( d, e, a, b, c, F3, K3, M(47) );
      R( c, d, e, a, b, F3, K3, M(48) );
      R( b, c, d, e, a, F3, K3, M(49) );
      R( a, b, c, d, e, F3, K3, M(50) );
      R( e, a, b, c, d, F3, K3, M(51) );
      R( d, e, a, b, c, F3, K3, M(52) );
      R( c, d, e, a, b, F3, K3, M(53) );
      R( b, c, d, e, a, F3, K3, M(54) );
      R( a, b, c, d, e, F3, K3, M(55) );
      R( e, a, b, c, d, F3, K3, M(56) );
      R( d, e, a, b, c, F3, K3, M(57) );
      R( c, d, e, a, b, F3, K3, M(58) );
      R( b, c, d, e, a, F3, K3, M(59) );
      R( a, b, c, d, e, F4, K4, M(60) );
      R( e, a, b, c, d, F4, K4, M(61) );
      R( d, e, a, b, c, F4, K4, M(62) );
      R( c, d, e, a, b, F4, K4, M(63) );
      R( b, c, d, e, a, F4, K4, M(64) );
      R( a, b, c, d, e, F4, K4, M(65) );
      R( e, a, b, c, d, F4, K4, M(66) );
      R( d, e, a, b, c, F4, K4, M(67) );
      R( c, d, e, a, b, F4, K4, M(68) );
      R( b, c, d, e, a, F4, K4, M(69) );
      R( a, b, c, d, e, F4, K4, M(70) );
      R( e, a, b, c, d, F4, K4, M(71) );
      R( d, e, a, b, c, F4, K4, M(72) );
      R( c, d, e, a, b, F4, K4, M(73) );
      R( b, c, d, e, a, F4, K4, M(74) );
      R( a, b, c, d, e, F4, K4, M(75) );
      R( e, a, b, c, d, F4, K4, M(76) );
      R( d, e, a, b, c, F4, K4, M(77) );
      R( c, d, e, a, b, F4, K4, M(78) );
      R( b, c, d, e, a, F4, K4, M(79) );

      /* Update the chaining variables. */
      hd->h0 += a;
      hd->h1 += b;
      hd->h2 += c;
      hd->h3 += d;
      hd->h4 += e;

      data += 64;
    }
  while (--nblks);

  return 88+4*sizeof(void*);
}


/*
 * Apply the SHA-1 transform function on the buffer BLOCKOF64BYTE
 * which must have a length 64 bytes.  BLOCKOF64BYTE must be 32-bit
 * aligned.  Updates the 20 bytes in BLOCKOF64BYTE with its mixed
 * content.  Returns the number of bytes which should be burned on the
 * stack.  You need to use _gcry_sha1_mixblock_init to initialize the
 * context.
 * WARNING: This is a special purpose function for exclusive use by
 * random-csprng.c.
 */
unsigned int
_gcry_sha1_mixblock (SHA1_CONTEXT *hd, void *blockof64byte)
{
  u32 *p = blockof64byte;
  unsigned int nburn;

  nburn = (*hd->bctx.bwrite) (hd, blockof64byte, 1);
  p[0] = hd->h0;
  p[1] = hd->h1;
  p[2] = hd->h2;
  p[3] = hd->h3;
  p[4] = hd->h4;

  return nburn;
}


/* The routine final terminates the computation and
 * returns the digest.
 * The handle is prepared for a new cycle, but adding bytes to the
 * handle will the destroy the returned buffer.
 * Returns: 20 bytes representing the digest.
 */

static void
sha1_final(void *context)
{
  SHA1_CONTEXT *hd = context;
  u32 t, th, msb, lsb;
  unsigned char *p;
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
  if( (lsb += hd->bctx.count) < t )
    msb++;
  /* multiply by 8 to make a bit count */
  t = lsb;
  lsb <<= 3;
  msb <<= 3;
  msb |= t >> 29;

  if (0)
    { }
#ifdef SHA1_USE_S390X_CRYPTO
  else if (hd->use_s390x_crypto)
    {
      burn = do_sha1_final_s390x (hd, hd->bctx.buf, hd->bctx.count, msb, lsb);
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
#define X(a) do { buf_put_be32(p, hd->h##a); p += 4; } while(0)
  X(0);
  X(1);
  X(2);
  X(3);
  X(4);
#undef X

  hd->bctx.count = 0;

  _gcry_burn_stack (burn);
}

static unsigned char *
sha1_read( void *context )
{
  SHA1_CONTEXT *hd = context;

  return hd->bctx.buf;
}


/****************
 * Shortcut functions which puts the hash value of the supplied buffer iov
 * into outbuf which must have a size of 20 bytes.
 */
static void
_gcry_sha1_hash_buffers (void *outbuf, size_t nbytes,
			 const gcry_buffer_t *iov, int iovcnt)
{
  SHA1_CONTEXT hd;

  (void)nbytes;

  sha1_init (&hd, 0);
  for (;iovcnt > 0; iov++, iovcnt--)
    _gcry_md_block_write (&hd,
                          (const char*)iov[0].data + iov[0].off, iov[0].len);
  sha1_final (&hd);
  memcpy (outbuf, hd.bctx.buf, 20);
}

/* Variant of the above shortcut function using a single buffer.  */
void
_gcry_sha1_hash_buffer (void *outbuf, const void *buffer, size_t length)
{
  gcry_buffer_t iov = { 0 };

  iov.data = (void *)buffer;
  iov.len = length;

  _gcry_sha1_hash_buffers (outbuf, 20, &iov, 1);
}



/*
     Self-test section.
 */


static gpg_err_code_t
selftests_sha1 (int extended, selftest_report_func_t report)
{
  const char *what;
  const char *errtxt;

  what = "short string";
  errtxt = _gcry_hash_selftest_check_one
    (GCRY_MD_SHA1, 0,
     "abc", 3,
     "\xA9\x99\x3E\x36\x47\x06\x81\x6A\xBA\x3E"
     "\x25\x71\x78\x50\xC2\x6C\x9C\xD0\xD8\x9D", 20);
  if (errtxt)
    goto failed;

  if (extended)
    {
      what = "long string";
      errtxt = _gcry_hash_selftest_check_one
        (GCRY_MD_SHA1, 0,
         "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq", 56,
         "\x84\x98\x3E\x44\x1C\x3B\xD2\x6E\xBA\xAE"
         "\x4A\xA1\xF9\x51\x29\xE5\xE5\x46\x70\xF1", 20);
      if (errtxt)
        goto failed;

      what = "one million \"a\"";
      errtxt = _gcry_hash_selftest_check_one
        (GCRY_MD_SHA1, 1,
         NULL, 0,
         "\x34\xAA\x97\x3C\xD4\xC4\xDA\xA4\xF6\x1E"
         "\xEB\x2B\xDB\xAD\x27\x31\x65\x34\x01\x6F", 20);
      if (errtxt)
        goto failed;
    }

  return 0; /* Succeeded. */

 failed:
  if (report)
    report ("digest", GCRY_MD_SHA1, what, errtxt);
  return GPG_ERR_SELFTEST_FAILED;
}


/* Run a full self-test for ALGO and return 0 on success.  */
static gpg_err_code_t
run_selftests (int algo, int extended, selftest_report_func_t report)
{
  gpg_err_code_t ec;

  switch (algo)
    {
    case GCRY_MD_SHA1:
      ec = selftests_sha1 (extended, report);
      break;
    default:
      ec = GPG_ERR_DIGEST_ALGO;
      break;

    }
  return ec;
}




static const unsigned char asn[15] = /* Object ID is 1.3.14.3.2.26 */
  { 0x30, 0x21, 0x30, 0x09, 0x06, 0x05, 0x2b, 0x0e, 0x03,
    0x02, 0x1a, 0x05, 0x00, 0x04, 0x14 };

static const gcry_md_oid_spec_t oid_spec_sha1[] =
  {
    /* iso.member-body.us.rsadsi.pkcs.pkcs-1.5 (sha1WithRSAEncryption) */
    { "1.2.840.113549.1.1.5" },
    /* iso.member-body.us.x9-57.x9cm.3 (dsaWithSha1)*/
    { "1.2.840.10040.4.3" },
    /* from NIST's OIW  (sha1) */
    { "1.3.14.3.2.26" },
    /* from NIST OIW (sha-1WithRSAEncryption) */
    { "1.3.14.3.2.29" },
    /* iso.member-body.us.ansi-x9-62.signatures.ecdsa-with-sha1 */
    { "1.2.840.10045.4.1" },
    { NULL },
  };

const gcry_md_spec_t _gcry_digest_spec_sha1 =
  {
    GCRY_MD_SHA1, {0, 1},
    "SHA1", asn, DIM (asn), oid_spec_sha1, 20,
    sha1_init, _gcry_md_block_write, sha1_final, sha1_read, NULL,
    _gcry_sha1_hash_buffers,
    sizeof (SHA1_CONTEXT),
    run_selftests
  };
