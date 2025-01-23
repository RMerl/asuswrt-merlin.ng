/* chacha20.c  -  Bernstein's ChaCha20 cipher
 * Copyright (C) 2014,2017-2019 Jussi Kivilinna <jussi.kivilinna@iki.fi>
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
 *
 * For a description of the algorithm, see:
 *   http://cr.yp.to/chacha.html
 */

/*
 * Based on D. J. Bernstein reference implementation at
 * http://cr.yp.to/chacha.html:
 *
 * chacha-regs.c version 20080118
 * D. J. Bernstein
 * Public domain.
 */

#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "g10lib.h"
#include "cipher.h"
#include "cipher-internal.h"
#include "bufhelp.h"


#define CHACHA20_MIN_KEY_SIZE 16        /* Bytes.  */
#define CHACHA20_MAX_KEY_SIZE 32        /* Bytes.  */
#define CHACHA20_BLOCK_SIZE   64        /* Bytes.  */
#define CHACHA20_MIN_IV_SIZE   8        /* Bytes.  */
#define CHACHA20_MAX_IV_SIZE  12        /* Bytes.  */
#define CHACHA20_CTR_SIZE     16        /* Bytes.  */


/* USE_SSSE3 indicates whether to compile with Intel SSSE3 code. */
#undef USE_SSSE3
#if defined(__x86_64__) && defined(HAVE_GCC_INLINE_ASM_SSSE3) && \
   (defined(HAVE_COMPATIBLE_GCC_AMD64_PLATFORM_AS) || \
    defined(HAVE_COMPATIBLE_GCC_WIN64_PLATFORM_AS))
# define USE_SSSE3 1
#endif

/* USE_AVX2 indicates whether to compile with Intel AVX2 code. */
#undef USE_AVX2
#if defined(__x86_64__) && defined(HAVE_GCC_INLINE_ASM_AVX2) && \
    (defined(HAVE_COMPATIBLE_GCC_AMD64_PLATFORM_AS) || \
     defined(HAVE_COMPATIBLE_GCC_WIN64_PLATFORM_AS))
# define USE_AVX2 1
#endif

/* USE_ARMV7_NEON indicates whether to enable ARMv7 NEON assembly code. */
#undef USE_ARMV7_NEON
#ifdef ENABLE_NEON_SUPPORT
# if defined(HAVE_ARM_ARCH_V6) && defined(__ARMEL__) \
     && defined(HAVE_COMPATIBLE_GCC_ARM_PLATFORM_AS) \
     && defined(HAVE_GCC_INLINE_ASM_NEON)
#  define USE_ARMV7_NEON 1
# endif
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

/* USE_PPC_VEC indicates whether to enable PowerPC vector
 * accelerated code. */
#undef USE_PPC_VEC
#ifdef ENABLE_PPC_CRYPTO_SUPPORT
# if defined(HAVE_COMPATIBLE_CC_PPC_ALTIVEC) && \
     defined(HAVE_GCC_INLINE_ASM_PPC_ALTIVEC)
#  if __GNUC__ >= 4
#   define USE_PPC_VEC 1
#  endif
# endif
#endif

/* USE_S390X_VX indicates whether to enable zSeries code. */
#undef USE_S390X_VX
#if defined (__s390x__) && __GNUC__ >= 4 && __ARCH__ >= 9
# if defined(HAVE_GCC_INLINE_ASM_S390X_VX)
#  define USE_S390X_VX 1
# endif /* USE_S390X_VX */
#endif

/* Assembly implementations use SystemV ABI, ABI conversion and additional
 * stack to store XMM6-XMM15 needed on Win64. */
#undef ASM_FUNC_ABI
#undef ASM_EXTRA_STACK
#if defined(HAVE_COMPATIBLE_GCC_WIN64_PLATFORM_AS)
# define ASM_FUNC_ABI __attribute__((sysv_abi))
#else
# define ASM_FUNC_ABI
#endif


typedef struct CHACHA20_context_s
{
  u32 input[16];
  unsigned char pad[CHACHA20_BLOCK_SIZE];
  unsigned int unused; /* bytes in the pad.  */
  unsigned int use_ssse3:1;
  unsigned int use_avx2:1;
  unsigned int use_neon:1;
  unsigned int use_ppc:1;
  unsigned int use_s390x:1;
} CHACHA20_context_t;


#ifdef USE_SSSE3

unsigned int _gcry_chacha20_amd64_ssse3_blocks4(u32 *state, byte *dst,
						const byte *src,
						size_t nblks) ASM_FUNC_ABI;

unsigned int _gcry_chacha20_amd64_ssse3_blocks1(u32 *state, byte *dst,
						const byte *src,
						size_t nblks) ASM_FUNC_ABI;

unsigned int _gcry_chacha20_poly1305_amd64_ssse3_blocks4(
		u32 *state, byte *dst, const byte *src, size_t nblks,
		void *poly1305_state, const byte *poly1305_src) ASM_FUNC_ABI;

unsigned int _gcry_chacha20_poly1305_amd64_ssse3_blocks1(
		u32 *state, byte *dst, const byte *src, size_t nblks,
		void *poly1305_state, const byte *poly1305_src) ASM_FUNC_ABI;

#endif /* USE_SSSE3 */

#ifdef USE_AVX2

unsigned int _gcry_chacha20_amd64_avx2_blocks8(u32 *state, byte *dst,
					       const byte *src,
					       size_t nblks) ASM_FUNC_ABI;

unsigned int _gcry_chacha20_poly1305_amd64_avx2_blocks8(
		u32 *state, byte *dst, const byte *src, size_t nblks,
		void *poly1305_state, const byte *poly1305_src) ASM_FUNC_ABI;

#endif /* USE_AVX2 */

#ifdef USE_PPC_VEC

unsigned int _gcry_chacha20_ppc8_blocks4(u32 *state, byte *dst,
					 const byte *src,
					 size_t nblks);

unsigned int _gcry_chacha20_ppc8_blocks1(u32 *state, byte *dst,
					 const byte *src,
					 size_t nblks);

#undef USE_PPC_VEC_POLY1305
#if SIZEOF_UNSIGNED_LONG == 8
#define USE_PPC_VEC_POLY1305 1
unsigned int _gcry_chacha20_poly1305_ppc8_blocks4(
		u32 *state, byte *dst, const byte *src, size_t nblks,
		POLY1305_STATE *st, const byte *poly1305_src);
#endif /* SIZEOF_UNSIGNED_LONG == 8 */

#endif /* USE_PPC_VEC */

#ifdef USE_S390X_VX

unsigned int _gcry_chacha20_s390x_vx_blocks8(u32 *state, byte *dst,
					     const byte *src, size_t nblks);

unsigned int _gcry_chacha20_s390x_vx_blocks4_2_1(u32 *state, byte *dst,
						 const byte *src, size_t nblks);

#undef USE_S390X_VX_POLY1305
#if SIZEOF_UNSIGNED_LONG == 8
#define USE_S390X_VX_POLY1305 1
unsigned int _gcry_chacha20_poly1305_s390x_vx_blocks8(
		u32 *state, byte *dst, const byte *src, size_t nblks,
		POLY1305_STATE *st, const byte *poly1305_src);

unsigned int _gcry_chacha20_poly1305_s390x_vx_blocks4_2_1(
		u32 *state, byte *dst, const byte *src, size_t nblks,
		POLY1305_STATE *st, const byte *poly1305_src);
#endif /* SIZEOF_UNSIGNED_LONG == 8 */

#endif /* USE_S390X_VX */

#ifdef USE_ARMV7_NEON

unsigned int _gcry_chacha20_armv7_neon_blocks4(u32 *state, byte *dst,
					       const byte *src,
					       size_t nblks);

#endif /* USE_ARMV7_NEON */

#ifdef USE_AARCH64_SIMD

unsigned int _gcry_chacha20_aarch64_blocks4(u32 *state, byte *dst,
					    const byte *src, size_t nblks);

unsigned int _gcry_chacha20_poly1305_aarch64_blocks4(
		u32 *state, byte *dst, const byte *src, size_t nblks,
		void *poly1305_state, const byte *poly1305_src);

#endif /* USE_AARCH64_SIMD */


static const char *selftest (void);


#define ROTATE(v,c)	(rol(v,c))
#define XOR(v,w)	((v) ^ (w))
#define PLUS(v,w)	((u32)((v) + (w)))
#define PLUSONE(v)	(PLUS((v),1))

#define QUARTERROUND(a,b,c,d) \
  a = PLUS(a,b); d = ROTATE(XOR(d,a),16); \
  c = PLUS(c,d); b = ROTATE(XOR(b,c),12); \
  a = PLUS(a,b); d = ROTATE(XOR(d,a), 8); \
  c = PLUS(c,d); b = ROTATE(XOR(b,c), 7);

#define BUF_XOR_LE32(dst, src, offset, x) \
  buf_put_le32((dst) + (offset), buf_get_le32((src) + (offset)) ^ (x))

static unsigned int
do_chacha20_blocks (u32 *input, byte *dst, const byte *src, size_t nblks)
{
  u32 x0, x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, x11, x12, x13, x14, x15;
  unsigned int i;

  while (nblks)
    {
      x0 = input[0];
      x1 = input[1];
      x2 = input[2];
      x3 = input[3];
      x4 = input[4];
      x5 = input[5];
      x6 = input[6];
      x7 = input[7];
      x8 = input[8];
      x9 = input[9];
      x10 = input[10];
      x11 = input[11];
      x12 = input[12];
      x13 = input[13];
      x14 = input[14];
      x15 = input[15];

      for (i = 20; i > 0; i -= 2)
	{
	  QUARTERROUND(x0, x4,  x8, x12)
	  QUARTERROUND(x1, x5,  x9, x13)
	  QUARTERROUND(x2, x6, x10, x14)
	  QUARTERROUND(x3, x7, x11, x15)
	  QUARTERROUND(x0, x5, x10, x15)
	  QUARTERROUND(x1, x6, x11, x12)
	  QUARTERROUND(x2, x7,  x8, x13)
	  QUARTERROUND(x3, x4,  x9, x14)
	}

      x0 = PLUS(x0, input[0]);
      x1 = PLUS(x1, input[1]);
      x2 = PLUS(x2, input[2]);
      x3 = PLUS(x3, input[3]);
      x4 = PLUS(x4, input[4]);
      x5 = PLUS(x5, input[5]);
      x6 = PLUS(x6, input[6]);
      x7 = PLUS(x7, input[7]);
      x8 = PLUS(x8, input[8]);
      x9 = PLUS(x9, input[9]);
      x10 = PLUS(x10, input[10]);
      x11 = PLUS(x11, input[11]);
      x12 = PLUS(x12, input[12]);
      x13 = PLUS(x13, input[13]);
      x14 = PLUS(x14, input[14]);
      x15 = PLUS(x15, input[15]);

      input[12] = PLUSONE(input[12]);
      input[13] = PLUS(input[13], !input[12]);

      BUF_XOR_LE32(dst, src, 0, x0);
      BUF_XOR_LE32(dst, src, 4, x1);
      BUF_XOR_LE32(dst, src, 8, x2);
      BUF_XOR_LE32(dst, src, 12, x3);
      BUF_XOR_LE32(dst, src, 16, x4);
      BUF_XOR_LE32(dst, src, 20, x5);
      BUF_XOR_LE32(dst, src, 24, x6);
      BUF_XOR_LE32(dst, src, 28, x7);
      BUF_XOR_LE32(dst, src, 32, x8);
      BUF_XOR_LE32(dst, src, 36, x9);
      BUF_XOR_LE32(dst, src, 40, x10);
      BUF_XOR_LE32(dst, src, 44, x11);
      BUF_XOR_LE32(dst, src, 48, x12);
      BUF_XOR_LE32(dst, src, 52, x13);
      BUF_XOR_LE32(dst, src, 56, x14);
      BUF_XOR_LE32(dst, src, 60, x15);

      src += CHACHA20_BLOCK_SIZE;
      dst += CHACHA20_BLOCK_SIZE;
      nblks--;
    }

  /* burn_stack */
  return (17 * sizeof(u32) + 6 * sizeof(void *));
}


static unsigned int
chacha20_blocks (CHACHA20_context_t *ctx, byte *dst, const byte *src,
		 size_t nblks)
{
#ifdef USE_SSSE3
  if (ctx->use_ssse3)
    {
      return _gcry_chacha20_amd64_ssse3_blocks1(ctx->input, dst, src, nblks);
    }
#endif

#ifdef USE_PPC_VEC
  if (ctx->use_ppc)
    {
      return _gcry_chacha20_ppc8_blocks1(ctx->input, dst, src, nblks);
    }
#endif

#ifdef USE_S390X_VX
  if (ctx->use_s390x)
    {
      return _gcry_chacha20_s390x_vx_blocks4_2_1(ctx->input, dst, src, nblks);
    }
#endif

  return do_chacha20_blocks (ctx->input, dst, src, nblks);
}


static void
chacha20_keysetup (CHACHA20_context_t *ctx, const byte *key,
                   unsigned int keylen)
{
  static const char sigma[16] = "expand 32-byte k";
  static const char tau[16] = "expand 16-byte k";
  const char *constants;

  ctx->input[4] = buf_get_le32(key + 0);
  ctx->input[5] = buf_get_le32(key + 4);
  ctx->input[6] = buf_get_le32(key + 8);
  ctx->input[7] = buf_get_le32(key + 12);
  if (keylen == CHACHA20_MAX_KEY_SIZE) /* 256 bits */
    {
      key += 16;
      constants = sigma;
    }
  else /* 128 bits */
    {
      constants = tau;
    }
  ctx->input[8] = buf_get_le32(key + 0);
  ctx->input[9] = buf_get_le32(key + 4);
  ctx->input[10] = buf_get_le32(key + 8);
  ctx->input[11] = buf_get_le32(key + 12);
  ctx->input[0] = buf_get_le32(constants + 0);
  ctx->input[1] = buf_get_le32(constants + 4);
  ctx->input[2] = buf_get_le32(constants + 8);
  ctx->input[3] = buf_get_le32(constants + 12);
}


static void
chacha20_ivsetup (CHACHA20_context_t * ctx, const byte *iv, size_t ivlen)
{
  if (ivlen == CHACHA20_CTR_SIZE)
    {
      ctx->input[12] = buf_get_le32 (iv + 0);
      ctx->input[13] = buf_get_le32 (iv + 4);
      ctx->input[14] = buf_get_le32 (iv + 8);
      ctx->input[15] = buf_get_le32 (iv + 12);
    }
  else if (ivlen == CHACHA20_MAX_IV_SIZE)
    {
      ctx->input[12] = 0;
      ctx->input[13] = buf_get_le32 (iv + 0);
      ctx->input[14] = buf_get_le32 (iv + 4);
      ctx->input[15] = buf_get_le32 (iv + 8);
    }
  else if (ivlen == CHACHA20_MIN_IV_SIZE)
    {
      ctx->input[12] = 0;
      ctx->input[13] = 0;
      ctx->input[14] = buf_get_le32 (iv + 0);
      ctx->input[15] = buf_get_le32 (iv + 4);
    }
  else
    {
      ctx->input[12] = 0;
      ctx->input[13] = 0;
      ctx->input[14] = 0;
      ctx->input[15] = 0;
    }
}


static void
chacha20_setiv (void *context, const byte *iv, size_t ivlen)
{
  CHACHA20_context_t *ctx = (CHACHA20_context_t *) context;

  /* draft-nir-cfrg-chacha20-poly1305-02 defines 96-bit and 64-bit nonce. */
  if (iv && ivlen != CHACHA20_MAX_IV_SIZE && ivlen != CHACHA20_MIN_IV_SIZE
      && ivlen != CHACHA20_CTR_SIZE)
    log_info ("WARNING: chacha20_setiv: bad ivlen=%u\n", (u32) ivlen);

  if (iv && (ivlen == CHACHA20_MAX_IV_SIZE || ivlen == CHACHA20_MIN_IV_SIZE
             || ivlen == CHACHA20_CTR_SIZE))
    chacha20_ivsetup (ctx, iv, ivlen);
  else
    chacha20_ivsetup (ctx, NULL, 0);

  /* Reset the unused pad bytes counter.  */
  ctx->unused = 0;
}


static gcry_err_code_t
chacha20_do_setkey (CHACHA20_context_t *ctx,
                    const byte *key, unsigned int keylen)
{
  static int initialized;
  static const char *selftest_failed;
  unsigned int features = _gcry_get_hw_features ();

  if (!initialized)
    {
      initialized = 1;
      selftest_failed = selftest ();
      if (selftest_failed)
        log_error ("CHACHA20 selftest failed (%s)\n", selftest_failed);
    }
  if (selftest_failed)
    return GPG_ERR_SELFTEST_FAILED;

  if (keylen != CHACHA20_MAX_KEY_SIZE && keylen != CHACHA20_MIN_KEY_SIZE)
    return GPG_ERR_INV_KEYLEN;

#ifdef USE_SSSE3
  ctx->use_ssse3 = (features & HWF_INTEL_SSSE3) != 0;
#endif
#ifdef USE_AVX2
  ctx->use_avx2 = (features & HWF_INTEL_AVX2) != 0;
#endif
#ifdef USE_ARMV7_NEON
  ctx->use_neon = (features & HWF_ARM_NEON) != 0;
#endif
#ifdef USE_AARCH64_SIMD
  ctx->use_neon = (features & HWF_ARM_NEON) != 0;
#endif
#ifdef USE_PPC_VEC
  ctx->use_ppc = (features & HWF_PPC_ARCH_2_07) != 0;
#endif
#ifdef USE_S390X_VX
  ctx->use_s390x = (features & HWF_S390X_VX) != 0;
#endif

  (void)features;

  chacha20_keysetup (ctx, key, keylen);

  /* We default to a zero nonce.  */
  chacha20_setiv (ctx, NULL, 0);

  return 0;
}


static gcry_err_code_t
chacha20_setkey (void *context, const byte *key, unsigned int keylen,
                 cipher_bulk_ops_t *bulk_ops)
{
  CHACHA20_context_t *ctx = (CHACHA20_context_t *) context;
  gcry_err_code_t rc = chacha20_do_setkey (ctx, key, keylen);
  (void)bulk_ops;
  _gcry_burn_stack (4 + sizeof (void *) + 4 * sizeof (void *));
  return rc;
}


static unsigned int
do_chacha20_encrypt_stream_tail (CHACHA20_context_t *ctx, byte *outbuf,
				 const byte *inbuf, size_t length)
{
  static const unsigned char zero_pad[CHACHA20_BLOCK_SIZE] = { 0, };
  unsigned int nburn, burn = 0;

#ifdef USE_AVX2
  if (ctx->use_avx2 && length >= CHACHA20_BLOCK_SIZE * 8)
    {
      size_t nblocks = length / CHACHA20_BLOCK_SIZE;
      nblocks -= nblocks % 8;
      nburn = _gcry_chacha20_amd64_avx2_blocks8(ctx->input, outbuf, inbuf,
						nblocks);
      burn = nburn > burn ? nburn : burn;
      length -= nblocks * CHACHA20_BLOCK_SIZE;
      outbuf += nblocks * CHACHA20_BLOCK_SIZE;
      inbuf  += nblocks * CHACHA20_BLOCK_SIZE;
    }
#endif

#ifdef USE_SSSE3
  if (ctx->use_ssse3 && length >= CHACHA20_BLOCK_SIZE * 4)
    {
      size_t nblocks = length / CHACHA20_BLOCK_SIZE;
      nblocks -= nblocks % 4;
      nburn = _gcry_chacha20_amd64_ssse3_blocks4(ctx->input, outbuf, inbuf,
						 nblocks);
      burn = nburn > burn ? nburn : burn;
      length -= nblocks * CHACHA20_BLOCK_SIZE;
      outbuf += nblocks * CHACHA20_BLOCK_SIZE;
      inbuf  += nblocks * CHACHA20_BLOCK_SIZE;
    }
#endif

#ifdef USE_ARMV7_NEON
  if (ctx->use_neon && length >= CHACHA20_BLOCK_SIZE * 4)
    {
      size_t nblocks = length / CHACHA20_BLOCK_SIZE;
      nblocks -= nblocks % 4;
      nburn = _gcry_chacha20_armv7_neon_blocks4(ctx->input, outbuf, inbuf,
						nblocks);
      burn = nburn > burn ? nburn : burn;
      length -= nblocks * CHACHA20_BLOCK_SIZE;
      outbuf += nblocks * CHACHA20_BLOCK_SIZE;
      inbuf  += nblocks * CHACHA20_BLOCK_SIZE;
    }
#endif

#ifdef USE_AARCH64_SIMD
  if (ctx->use_neon && length >= CHACHA20_BLOCK_SIZE * 4)
    {
      size_t nblocks = length / CHACHA20_BLOCK_SIZE;
      nblocks -= nblocks % 4;
      nburn = _gcry_chacha20_aarch64_blocks4(ctx->input, outbuf, inbuf,
					     nblocks);
      burn = nburn > burn ? nburn : burn;
      length -= nblocks * CHACHA20_BLOCK_SIZE;
      outbuf += nblocks * CHACHA20_BLOCK_SIZE;
      inbuf  += nblocks * CHACHA20_BLOCK_SIZE;
    }
#endif

#ifdef USE_PPC_VEC
  if (ctx->use_ppc && length >= CHACHA20_BLOCK_SIZE * 4)
    {
      size_t nblocks = length / CHACHA20_BLOCK_SIZE;
      nblocks -= nblocks % 4;
      nburn = _gcry_chacha20_ppc8_blocks4(ctx->input, outbuf, inbuf, nblocks);
      burn = nburn > burn ? nburn : burn;
      length -= nblocks * CHACHA20_BLOCK_SIZE;
      outbuf += nblocks * CHACHA20_BLOCK_SIZE;
      inbuf  += nblocks * CHACHA20_BLOCK_SIZE;
    }
#endif

#ifdef USE_S390X_VX
  if (ctx->use_s390x && length >= CHACHA20_BLOCK_SIZE * 8)
    {
      size_t nblocks = length / CHACHA20_BLOCK_SIZE;
      nblocks -= nblocks % 8;
      nburn = _gcry_chacha20_s390x_vx_blocks8(ctx->input, outbuf, inbuf,
					      nblocks);
      burn = nburn > burn ? nburn : burn;
      length -= nblocks * CHACHA20_BLOCK_SIZE;
      outbuf += nblocks * CHACHA20_BLOCK_SIZE;
      inbuf  += nblocks * CHACHA20_BLOCK_SIZE;
    }
#endif

  if (length >= CHACHA20_BLOCK_SIZE)
    {
      size_t nblocks = length / CHACHA20_BLOCK_SIZE;
      nburn = chacha20_blocks(ctx, outbuf, inbuf, nblocks);
      burn = nburn > burn ? nburn : burn;
      length -= nblocks * CHACHA20_BLOCK_SIZE;
      outbuf += nblocks * CHACHA20_BLOCK_SIZE;
      inbuf  += nblocks * CHACHA20_BLOCK_SIZE;
    }

  if (length > 0)
    {
      nburn = chacha20_blocks(ctx, ctx->pad, zero_pad, 1);
      burn = nburn > burn ? nburn : burn;

      buf_xor (outbuf, inbuf, ctx->pad, length);
      ctx->unused = CHACHA20_BLOCK_SIZE - length;
    }

  if (burn)
    burn += 5 * sizeof(void *);

  return burn;
}


static void
chacha20_encrypt_stream (void *context, byte *outbuf, const byte *inbuf,
                         size_t length)
{
  CHACHA20_context_t *ctx = (CHACHA20_context_t *) context;
  unsigned int nburn, burn = 0;

  if (!length)
    return;

  if (ctx->unused)
    {
      unsigned char *p = ctx->pad;
      size_t n;

      gcry_assert (ctx->unused < CHACHA20_BLOCK_SIZE);

      n = ctx->unused;
      if (n > length)
        n = length;

      buf_xor (outbuf, inbuf, p + CHACHA20_BLOCK_SIZE - ctx->unused, n);
      length -= n;
      outbuf += n;
      inbuf += n;
      ctx->unused -= n;

      if (!length)
        return;
      gcry_assert (!ctx->unused);
    }

  nburn = do_chacha20_encrypt_stream_tail (ctx, outbuf, inbuf, length);
  burn = nburn > burn ? nburn : burn;

  if (burn)
    _gcry_burn_stack (burn);
}


gcry_err_code_t
_gcry_chacha20_poly1305_encrypt(gcry_cipher_hd_t c, byte *outbuf,
				const byte *inbuf, size_t length)
{
  CHACHA20_context_t *ctx = (void *) &c->context.c;
  unsigned int nburn, burn = 0;
  byte *authptr = NULL;

  if (!length)
    return 0;

  if (ctx->unused)
    {
      unsigned char *p = ctx->pad;
      size_t n;

      gcry_assert (ctx->unused < CHACHA20_BLOCK_SIZE);

      n = ctx->unused;
      if (n > length)
        n = length;

      buf_xor (outbuf, inbuf, p + CHACHA20_BLOCK_SIZE - ctx->unused, n);
      nburn = _gcry_poly1305_update_burn (&c->u_mode.poly1305.ctx, outbuf, n);
      burn = nburn > burn ? nburn : burn;
      length -= n;
      outbuf += n;
      inbuf += n;
      ctx->unused -= n;

      if (!length)
	{
	  if (burn)
	    _gcry_burn_stack (burn);

	  return 0;
	}
      gcry_assert (!ctx->unused);
    }

  gcry_assert (c->u_mode.poly1305.ctx.leftover == 0);

  if (0)
    { }
#ifdef USE_AVX2
  else if (ctx->use_avx2 && length >= CHACHA20_BLOCK_SIZE * 8)
    {
      nburn = _gcry_chacha20_amd64_avx2_blocks8(ctx->input, outbuf, inbuf, 8);
      burn = nburn > burn ? nburn : burn;

      authptr = outbuf;
      length -= 8 * CHACHA20_BLOCK_SIZE;
      outbuf += 8 * CHACHA20_BLOCK_SIZE;
      inbuf  += 8 * CHACHA20_BLOCK_SIZE;
    }
#endif
#ifdef USE_SSSE3
  else if (ctx->use_ssse3 && length >= CHACHA20_BLOCK_SIZE * 4)
    {
      nburn = _gcry_chacha20_amd64_ssse3_blocks4(ctx->input, outbuf, inbuf, 4);
      burn = nburn > burn ? nburn : burn;

      authptr = outbuf;
      length -= 4 * CHACHA20_BLOCK_SIZE;
      outbuf += 4 * CHACHA20_BLOCK_SIZE;
      inbuf  += 4 * CHACHA20_BLOCK_SIZE;
    }
  else if (ctx->use_ssse3 && length >= CHACHA20_BLOCK_SIZE * 2)
    {
      nburn = _gcry_chacha20_amd64_ssse3_blocks1(ctx->input, outbuf, inbuf, 2);
      burn = nburn > burn ? nburn : burn;

      authptr = outbuf;
      length -= 2 * CHACHA20_BLOCK_SIZE;
      outbuf += 2 * CHACHA20_BLOCK_SIZE;
      inbuf  += 2 * CHACHA20_BLOCK_SIZE;
    }
  else if (ctx->use_ssse3 && length >= CHACHA20_BLOCK_SIZE)
    {
      nburn = _gcry_chacha20_amd64_ssse3_blocks1(ctx->input, outbuf, inbuf, 1);
      burn = nburn > burn ? nburn : burn;

      authptr = outbuf;
      length -= 1 * CHACHA20_BLOCK_SIZE;
      outbuf += 1 * CHACHA20_BLOCK_SIZE;
      inbuf  += 1 * CHACHA20_BLOCK_SIZE;
    }
#endif
#ifdef USE_AARCH64_SIMD
  else if (ctx->use_neon && length >= CHACHA20_BLOCK_SIZE * 4)
    {
      nburn = _gcry_chacha20_aarch64_blocks4(ctx->input, outbuf, inbuf, 4);
      burn = nburn > burn ? nburn : burn;

      authptr = outbuf;
      length -= 4 * CHACHA20_BLOCK_SIZE;
      outbuf += 4 * CHACHA20_BLOCK_SIZE;
      inbuf  += 4 * CHACHA20_BLOCK_SIZE;
    }
#endif
#ifdef USE_PPC_VEC_POLY1305
  else if (ctx->use_ppc && length >= CHACHA20_BLOCK_SIZE * 4)
    {
      nburn = _gcry_chacha20_ppc8_blocks4(ctx->input, outbuf, inbuf, 4);
      burn = nburn > burn ? nburn : burn;

      authptr = outbuf;
      length -= 4 * CHACHA20_BLOCK_SIZE;
      outbuf += 4 * CHACHA20_BLOCK_SIZE;
      inbuf  += 4 * CHACHA20_BLOCK_SIZE;
    }
#endif
#ifdef USE_S390X_VX_POLY1305
  else if (ctx->use_s390x && length >= 2 * CHACHA20_BLOCK_SIZE * 8)
    {
      nburn = _gcry_chacha20_s390x_vx_blocks8(ctx->input, outbuf, inbuf, 8);
      burn = nburn > burn ? nburn : burn;

      authptr = outbuf;
      length -= 8 * CHACHA20_BLOCK_SIZE;
      outbuf += 8 * CHACHA20_BLOCK_SIZE;
      inbuf  += 8 * CHACHA20_BLOCK_SIZE;
    }
  else if (ctx->use_s390x && length >= CHACHA20_BLOCK_SIZE * 4)
    {
      nburn = _gcry_chacha20_s390x_vx_blocks4_2_1(ctx->input, outbuf, inbuf, 4);
      burn = nburn > burn ? nburn : burn;

      authptr = outbuf;
      length -= 4 * CHACHA20_BLOCK_SIZE;
      outbuf += 4 * CHACHA20_BLOCK_SIZE;
      inbuf  += 4 * CHACHA20_BLOCK_SIZE;
    }
  else if (ctx->use_s390x && length >= CHACHA20_BLOCK_SIZE * 2)
    {
      nburn = _gcry_chacha20_s390x_vx_blocks4_2_1(ctx->input, outbuf, inbuf, 2);
      burn = nburn > burn ? nburn : burn;

      authptr = outbuf;
      length -= 2 * CHACHA20_BLOCK_SIZE;
      outbuf += 2 * CHACHA20_BLOCK_SIZE;
      inbuf  += 2 * CHACHA20_BLOCK_SIZE;
    }
  else if (ctx->use_s390x && length >= CHACHA20_BLOCK_SIZE)
    {
      nburn = _gcry_chacha20_s390x_vx_blocks4_2_1(ctx->input, outbuf, inbuf, 1);
      burn = nburn > burn ? nburn : burn;

      authptr = outbuf;
      length -= 1 * CHACHA20_BLOCK_SIZE;
      outbuf += 1 * CHACHA20_BLOCK_SIZE;
      inbuf  += 1 * CHACHA20_BLOCK_SIZE;
    }
#endif

  if (authptr)
    {
      size_t authoffset = outbuf - authptr;

#ifdef USE_AVX2
      if (ctx->use_avx2 &&
	  length >= 8 * CHACHA20_BLOCK_SIZE &&
	  authoffset >= 8 * CHACHA20_BLOCK_SIZE)
	{
	  size_t nblocks = length / CHACHA20_BLOCK_SIZE;
	  nblocks -= nblocks % 8;

	  nburn = _gcry_chacha20_poly1305_amd64_avx2_blocks8(
		      ctx->input, outbuf, inbuf, nblocks,
		      &c->u_mode.poly1305.ctx.state, authptr);
	  burn = nburn > burn ? nburn : burn;

	  length  -= nblocks * CHACHA20_BLOCK_SIZE;
	  outbuf  += nblocks * CHACHA20_BLOCK_SIZE;
	  inbuf   += nblocks * CHACHA20_BLOCK_SIZE;
	  authptr += nblocks * CHACHA20_BLOCK_SIZE;
	}
#endif

#ifdef USE_SSSE3
      if (ctx->use_ssse3)
	{
	  if (length >= 4 * CHACHA20_BLOCK_SIZE &&
	      authoffset >= 4 * CHACHA20_BLOCK_SIZE)
	    {
	      size_t nblocks = length / CHACHA20_BLOCK_SIZE;
	      nblocks -= nblocks % 4;

	      nburn = _gcry_chacha20_poly1305_amd64_ssse3_blocks4(
			  ctx->input, outbuf, inbuf, nblocks,
			  &c->u_mode.poly1305.ctx.state, authptr);
	      burn = nburn > burn ? nburn : burn;

	      length  -= nblocks * CHACHA20_BLOCK_SIZE;
	      outbuf  += nblocks * CHACHA20_BLOCK_SIZE;
	      inbuf   += nblocks * CHACHA20_BLOCK_SIZE;
	      authptr += nblocks * CHACHA20_BLOCK_SIZE;
	    }

	  if (length >= CHACHA20_BLOCK_SIZE &&
	      authoffset >= CHACHA20_BLOCK_SIZE)
	    {
	      size_t nblocks = length / CHACHA20_BLOCK_SIZE;

	      nburn = _gcry_chacha20_poly1305_amd64_ssse3_blocks1(
			  ctx->input, outbuf, inbuf, nblocks,
			  &c->u_mode.poly1305.ctx.state, authptr);
	      burn = nburn > burn ? nburn : burn;

	      length  -= nblocks * CHACHA20_BLOCK_SIZE;
	      outbuf  += nblocks * CHACHA20_BLOCK_SIZE;
	      inbuf   += nblocks * CHACHA20_BLOCK_SIZE;
	      authptr += nblocks * CHACHA20_BLOCK_SIZE;
	    }
	}
#endif

#ifdef USE_AARCH64_SIMD
      if (ctx->use_neon &&
	  length >= 4 * CHACHA20_BLOCK_SIZE &&
	  authoffset >= 4 * CHACHA20_BLOCK_SIZE)
	{
	  size_t nblocks = length / CHACHA20_BLOCK_SIZE;
	  nblocks -= nblocks % 4;

	  nburn = _gcry_chacha20_poly1305_aarch64_blocks4(
		      ctx->input, outbuf, inbuf, nblocks,
		      &c->u_mode.poly1305.ctx.state, authptr);
	  burn = nburn > burn ? nburn : burn;

	  length  -= nblocks * CHACHA20_BLOCK_SIZE;
	  outbuf  += nblocks * CHACHA20_BLOCK_SIZE;
	  inbuf   += nblocks * CHACHA20_BLOCK_SIZE;
	  authptr += nblocks * CHACHA20_BLOCK_SIZE;
	}
#endif

#ifdef USE_PPC_VEC_POLY1305
      if (ctx->use_ppc &&
	  length >= 4 * CHACHA20_BLOCK_SIZE &&
	  authoffset >= 4 * CHACHA20_BLOCK_SIZE)
	{
	  size_t nblocks = length / CHACHA20_BLOCK_SIZE;
	  nblocks -= nblocks % 4;

	  nburn = _gcry_chacha20_poly1305_ppc8_blocks4(
		      ctx->input, outbuf, inbuf, nblocks,
		      &c->u_mode.poly1305.ctx.state, authptr);
	  burn = nburn > burn ? nburn : burn;

	  length  -= nblocks * CHACHA20_BLOCK_SIZE;
	  outbuf  += nblocks * CHACHA20_BLOCK_SIZE;
	  inbuf   += nblocks * CHACHA20_BLOCK_SIZE;
	  authptr += nblocks * CHACHA20_BLOCK_SIZE;
	}
#endif

#ifdef USE_S390X_VX_POLY1305
      if (ctx->use_s390x)
	{
	  if (length >= 8 * CHACHA20_BLOCK_SIZE &&
	      authoffset >= 8 * CHACHA20_BLOCK_SIZE)
	    {
	      size_t nblocks = length / CHACHA20_BLOCK_SIZE;
	      nblocks -= nblocks % 8;

	      burn = _gcry_chacha20_poly1305_s390x_vx_blocks8(
			  ctx->input, outbuf, inbuf, nblocks,
			  &c->u_mode.poly1305.ctx.state, authptr);
	      burn = nburn > burn ? nburn : burn;

	      length  -= nblocks * CHACHA20_BLOCK_SIZE;
	      outbuf  += nblocks * CHACHA20_BLOCK_SIZE;
	      inbuf   += nblocks * CHACHA20_BLOCK_SIZE;
	      authptr += nblocks * CHACHA20_BLOCK_SIZE;
	    }

	  if (length >= CHACHA20_BLOCK_SIZE &&
	      authoffset >= CHACHA20_BLOCK_SIZE)
	    {
	      size_t nblocks = length / CHACHA20_BLOCK_SIZE;

	      burn = _gcry_chacha20_poly1305_s390x_vx_blocks4_2_1(
			  ctx->input, outbuf, inbuf, nblocks,
			  &c->u_mode.poly1305.ctx.state, authptr);
	      burn = nburn > burn ? nburn : burn;

	      length  -= nblocks * CHACHA20_BLOCK_SIZE;
	      outbuf  += nblocks * CHACHA20_BLOCK_SIZE;
	      inbuf   += nblocks * CHACHA20_BLOCK_SIZE;
	      authptr += nblocks * CHACHA20_BLOCK_SIZE;
	    }
	}
#endif

      if (authoffset > 0)
	{
	  _gcry_poly1305_update (&c->u_mode.poly1305.ctx, authptr, authoffset);
	  authptr += authoffset;
	  authoffset = 0;
	}

      gcry_assert(authptr == outbuf);
    }

  while (length)
    {
      size_t currlen = length;

      /* Since checksumming is done after encryption, process input in 24KiB
       * chunks to keep data loaded in L1 cache for checksumming. */
      if (currlen > 24 * 1024)
	currlen = 24 * 1024;

      nburn = do_chacha20_encrypt_stream_tail (ctx, outbuf, inbuf, currlen);
      burn = nburn > burn ? nburn : burn;

      nburn = _gcry_poly1305_update_burn (&c->u_mode.poly1305.ctx, outbuf,
					  currlen);
      burn = nburn > burn ? nburn : burn;

      outbuf += currlen;
      inbuf += currlen;
      length -= currlen;
    }

  if (burn)
    _gcry_burn_stack (burn);

  return 0;
}


gcry_err_code_t
_gcry_chacha20_poly1305_decrypt(gcry_cipher_hd_t c, byte *outbuf,
				const byte *inbuf, size_t length)
{
  CHACHA20_context_t *ctx = (void *) &c->context.c;
  unsigned int nburn, burn = 0;

  if (!length)
    return 0;

  if (ctx->unused)
    {
      unsigned char *p = ctx->pad;
      size_t n;

      gcry_assert (ctx->unused < CHACHA20_BLOCK_SIZE);

      n = ctx->unused;
      if (n > length)
        n = length;

      nburn = _gcry_poly1305_update_burn (&c->u_mode.poly1305.ctx, inbuf, n);
      burn = nburn > burn ? nburn : burn;
      buf_xor (outbuf, inbuf, p + CHACHA20_BLOCK_SIZE - ctx->unused, n);
      length -= n;
      outbuf += n;
      inbuf += n;
      ctx->unused -= n;

      if (!length)
	{
	  if (burn)
	    _gcry_burn_stack (burn);

	  return 0;
	}
      gcry_assert (!ctx->unused);
    }

  gcry_assert (c->u_mode.poly1305.ctx.leftover == 0);

#ifdef USE_AVX2
  if (ctx->use_avx2 && length >= 8 * CHACHA20_BLOCK_SIZE)
    {
      size_t nblocks = length / CHACHA20_BLOCK_SIZE;
      nblocks -= nblocks % 8;

      nburn = _gcry_chacha20_poly1305_amd64_avx2_blocks8(
			ctx->input, outbuf, inbuf, nblocks,
			&c->u_mode.poly1305.ctx.state, inbuf);
      burn = nburn > burn ? nburn : burn;

      length -= nblocks * CHACHA20_BLOCK_SIZE;
      outbuf += nblocks * CHACHA20_BLOCK_SIZE;
      inbuf  += nblocks * CHACHA20_BLOCK_SIZE;
    }
#endif

#ifdef USE_SSSE3
  if (ctx->use_ssse3)
    {
      if (length >= 4 * CHACHA20_BLOCK_SIZE)
	{
	  size_t nblocks = length / CHACHA20_BLOCK_SIZE;
	  nblocks -= nblocks % 4;

	  nburn = _gcry_chacha20_poly1305_amd64_ssse3_blocks4(
			    ctx->input, outbuf, inbuf, nblocks,
			    &c->u_mode.poly1305.ctx.state, inbuf);
	  burn = nburn > burn ? nburn : burn;

	  length -= nblocks * CHACHA20_BLOCK_SIZE;
	  outbuf += nblocks * CHACHA20_BLOCK_SIZE;
	  inbuf  += nblocks * CHACHA20_BLOCK_SIZE;
	}

      if (length >= CHACHA20_BLOCK_SIZE)
	{
	  size_t nblocks = length / CHACHA20_BLOCK_SIZE;

	  nburn = _gcry_chacha20_poly1305_amd64_ssse3_blocks1(
			    ctx->input, outbuf, inbuf, nblocks,
			    &c->u_mode.poly1305.ctx.state, inbuf);
	  burn = nburn > burn ? nburn : burn;

	  length -= nblocks * CHACHA20_BLOCK_SIZE;
	  outbuf += nblocks * CHACHA20_BLOCK_SIZE;
	  inbuf  += nblocks * CHACHA20_BLOCK_SIZE;
	}
    }
#endif

#ifdef USE_AARCH64_SIMD
  if (ctx->use_neon && length >= 4 * CHACHA20_BLOCK_SIZE)
    {
      size_t nblocks = length / CHACHA20_BLOCK_SIZE;
      nblocks -= nblocks % 4;

      nburn = _gcry_chacha20_poly1305_aarch64_blocks4(
			ctx->input, outbuf, inbuf, nblocks,
			&c->u_mode.poly1305.ctx.state, inbuf);
      burn = nburn > burn ? nburn : burn;

      length -= nblocks * CHACHA20_BLOCK_SIZE;
      outbuf += nblocks * CHACHA20_BLOCK_SIZE;
      inbuf  += nblocks * CHACHA20_BLOCK_SIZE;
    }
#endif

#ifdef USE_PPC_VEC_POLY1305
  if (ctx->use_ppc && length >= 4 * CHACHA20_BLOCK_SIZE)
    {
      size_t nblocks = length / CHACHA20_BLOCK_SIZE;
      nblocks -= nblocks % 4;

      nburn = _gcry_chacha20_poly1305_ppc8_blocks4(
			ctx->input, outbuf, inbuf, nblocks,
			&c->u_mode.poly1305.ctx.state, inbuf);
      burn = nburn > burn ? nburn : burn;

      length -= nblocks * CHACHA20_BLOCK_SIZE;
      outbuf += nblocks * CHACHA20_BLOCK_SIZE;
      inbuf  += nblocks * CHACHA20_BLOCK_SIZE;
    }
#endif

#ifdef USE_S390X_VX_POLY1305
  if (ctx->use_s390x)
    {
      if (length >= 8 * CHACHA20_BLOCK_SIZE)
	{
	  size_t nblocks = length / CHACHA20_BLOCK_SIZE;
	  nblocks -= nblocks % 8;

	  nburn = _gcry_chacha20_poly1305_s390x_vx_blocks8(
			    ctx->input, outbuf, inbuf, nblocks,
			    &c->u_mode.poly1305.ctx.state, inbuf);
	  burn = nburn > burn ? nburn : burn;

	  length -= nblocks * CHACHA20_BLOCK_SIZE;
	  outbuf += nblocks * CHACHA20_BLOCK_SIZE;
	  inbuf  += nblocks * CHACHA20_BLOCK_SIZE;
	}

      if (length >= CHACHA20_BLOCK_SIZE)
	{
	  size_t nblocks = length / CHACHA20_BLOCK_SIZE;

	  nburn = _gcry_chacha20_poly1305_s390x_vx_blocks4_2_1(
			    ctx->input, outbuf, inbuf, nblocks,
			    &c->u_mode.poly1305.ctx.state, inbuf);
	  burn = nburn > burn ? nburn : burn;

	  length -= nblocks * CHACHA20_BLOCK_SIZE;
	  outbuf += nblocks * CHACHA20_BLOCK_SIZE;
	  inbuf  += nblocks * CHACHA20_BLOCK_SIZE;
	}
    }
#endif

  while (length)
    {
      size_t currlen = length;

      /* Since checksumming is done before decryption, process input in 24KiB
       * chunks to keep data loaded in L1 cache for decryption. */
      if (currlen > 24 * 1024)
	currlen = 24 * 1024;

      nburn = _gcry_poly1305_update_burn (&c->u_mode.poly1305.ctx, inbuf,
					  currlen);
      burn = nburn > burn ? nburn : burn;

      nburn = do_chacha20_encrypt_stream_tail (ctx, outbuf, inbuf, currlen);
      burn = nburn > burn ? nburn : burn;

      outbuf += currlen;
      inbuf += currlen;
      length -= currlen;
    }

  if (burn)
    _gcry_burn_stack (burn);

  return 0;
}


static const char *
selftest (void)
{
  byte ctxbuf[sizeof(CHACHA20_context_t) + 15];
  CHACHA20_context_t *ctx;
  byte scratch[127 + 1];
  byte buf[512 + 64 + 4];
  int i;

  /* From draft-strombergson-chacha-test-vectors */
  static byte key_1[] = {
    0xc4, 0x6e, 0xc1, 0xb1, 0x8c, 0xe8, 0xa8, 0x78,
    0x72, 0x5a, 0x37, 0xe7, 0x80, 0xdf, 0xb7, 0x35,
    0x1f, 0x68, 0xed, 0x2e, 0x19, 0x4c, 0x79, 0xfb,
    0xc6, 0xae, 0xbe, 0xe1, 0xa6, 0x67, 0x97, 0x5d
  };
  static const byte nonce_1[] =
    { 0x1a, 0xda, 0x31, 0xd5, 0xcf, 0x68, 0x82, 0x21 };
  static const byte plaintext_1[127] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  };
  static const byte ciphertext_1[127] = {
    0xf6, 0x3a, 0x89, 0xb7, 0x5c, 0x22, 0x71, 0xf9,
    0x36, 0x88, 0x16, 0x54, 0x2b, 0xa5, 0x2f, 0x06,
    0xed, 0x49, 0x24, 0x17, 0x92, 0x30, 0x2b, 0x00,
    0xb5, 0xe8, 0xf8, 0x0a, 0xe9, 0xa4, 0x73, 0xaf,
    0xc2, 0x5b, 0x21, 0x8f, 0x51, 0x9a, 0xf0, 0xfd,
    0xd4, 0x06, 0x36, 0x2e, 0x8d, 0x69, 0xde, 0x7f,
    0x54, 0xc6, 0x04, 0xa6, 0xe0, 0x0f, 0x35, 0x3f,
    0x11, 0x0f, 0x77, 0x1b, 0xdc, 0xa8, 0xab, 0x92,
    0xe5, 0xfb, 0xc3, 0x4e, 0x60, 0xa1, 0xd9, 0xa9,
    0xdb, 0x17, 0x34, 0x5b, 0x0a, 0x40, 0x27, 0x36,
    0x85, 0x3b, 0xf9, 0x10, 0xb0, 0x60, 0xbd, 0xf1,
    0xf8, 0x97, 0xb6, 0x29, 0x0f, 0x01, 0xd1, 0x38,
    0xae, 0x2c, 0x4c, 0x90, 0x22, 0x5b, 0xa9, 0xea,
    0x14, 0xd5, 0x18, 0xf5, 0x59, 0x29, 0xde, 0xa0,
    0x98, 0xca, 0x7a, 0x6c, 0xcf, 0xe6, 0x12, 0x27,
    0x05, 0x3c, 0x84, 0xe4, 0x9a, 0x4a, 0x33
  };

  /* 16-byte alignment required for amd64 implementation. */
  ctx = (CHACHA20_context_t *)((uintptr_t)(ctxbuf + 15) & ~(uintptr_t)15);

  chacha20_setkey (ctx, key_1, sizeof key_1, NULL);
  chacha20_setiv (ctx, nonce_1, sizeof nonce_1);
  scratch[sizeof (scratch) - 1] = 0;
  chacha20_encrypt_stream (ctx, scratch, plaintext_1, sizeof plaintext_1);
  if (memcmp (scratch, ciphertext_1, sizeof ciphertext_1))
    return "ChaCha20 encryption test 1 failed.";
  if (scratch[sizeof (scratch) - 1])
    return "ChaCha20 wrote too much.";
  chacha20_setkey (ctx, key_1, sizeof (key_1), NULL);
  chacha20_setiv (ctx, nonce_1, sizeof nonce_1);
  chacha20_encrypt_stream (ctx, scratch, scratch, sizeof plaintext_1);
  if (memcmp (scratch, plaintext_1, sizeof plaintext_1))
    return "ChaCha20 decryption test 1 failed.";

  for (i = 0; i < sizeof buf; i++)
    buf[i] = i;
  chacha20_setkey (ctx, key_1, sizeof key_1, NULL);
  chacha20_setiv (ctx, nonce_1, sizeof nonce_1);
  /*encrypt */
  chacha20_encrypt_stream (ctx, buf, buf, sizeof buf);
  /*decrypt */
  chacha20_setkey (ctx, key_1, sizeof key_1, NULL);
  chacha20_setiv (ctx, nonce_1, sizeof nonce_1);
  chacha20_encrypt_stream (ctx, buf, buf, 1);
  chacha20_encrypt_stream (ctx, buf + 1, buf + 1, (sizeof buf) - 1 - 1);
  chacha20_encrypt_stream (ctx, buf + (sizeof buf) - 1,
                           buf + (sizeof buf) - 1, 1);
  for (i = 0; i < sizeof buf; i++)
    if (buf[i] != (byte) i)
      return "ChaCha20 encryption test 2 failed.";

  chacha20_setkey (ctx, key_1, sizeof key_1, NULL);
  chacha20_setiv (ctx, nonce_1, sizeof nonce_1);
  /* encrypt */
  for (i = 0; i < sizeof buf; i++)
    chacha20_encrypt_stream (ctx, &buf[i], &buf[i], 1);
  /* decrypt */
  chacha20_setkey (ctx, key_1, sizeof key_1, NULL);
  chacha20_setiv (ctx, nonce_1, sizeof nonce_1);
  chacha20_encrypt_stream (ctx, buf, buf, sizeof buf);
  for (i = 0; i < sizeof buf; i++)
    if (buf[i] != (byte) i)
      return "ChaCha20 encryption test 3 failed.";

  return NULL;
}


gcry_cipher_spec_t _gcry_cipher_spec_chacha20 = {
  GCRY_CIPHER_CHACHA20,
  {0, 0},                       /* flags */
  "CHACHA20",                   /* name */
  NULL,                         /* aliases */
  NULL,                         /* oids */
  1,                            /* blocksize in bytes. */
  CHACHA20_MAX_KEY_SIZE * 8,    /* standard key length in bits. */
  sizeof (CHACHA20_context_t),
  chacha20_setkey,
  NULL,
  NULL,
  chacha20_encrypt_stream,
  chacha20_encrypt_stream,
  NULL,
  NULL,
  chacha20_setiv
};
