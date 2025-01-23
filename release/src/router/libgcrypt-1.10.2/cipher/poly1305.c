/* poly1305.c  -  Poly1305 internals and generic implementation
 * Copyright (C) 2014,2017,2018 Jussi Kivilinna <jussi.kivilinna@iki.fi>
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

#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "types.h"
#include "g10lib.h"
#include "cipher.h"
#include "bufhelp.h"
#include "poly1305-internal.h"

#include "mpi-internal.h"
#include "longlong.h"


static const char *selftest (void);


#undef HAVE_ASM_POLY1305_BLOCKS


#undef USE_MPI_64BIT
#undef USE_MPI_32BIT
#if BYTES_PER_MPI_LIMB == 8 && defined(HAVE_TYPE_U64)
# define USE_MPI_64BIT 1
#elif BYTES_PER_MPI_LIMB == 4
# define USE_MPI_32BIT 1
#else
# error please implement for this limb size.
#endif


/* USE_S390X_ASM indicates whether to enable zSeries code. */
#undef USE_S390X_ASM
#if BYTES_PER_MPI_LIMB == 8
# if defined (__s390x__) && __GNUC__ >= 4 && __ARCH__ >= 9
#  if defined(HAVE_GCC_INLINE_ASM_S390X)
#   define USE_S390X_ASM 1
#  endif /* USE_S390X_ASM */
# endif
#endif


#ifdef USE_S390X_ASM

#define HAVE_ASM_POLY1305_BLOCKS 1

extern unsigned int _gcry_poly1305_s390x_blocks1(void *state,
						 const byte *buf, size_t len,
						 byte high_pad);

static unsigned int
poly1305_blocks (poly1305_context_t *ctx, const byte *buf, size_t len,
		 byte high_pad)
{
  return _gcry_poly1305_s390x_blocks1(&ctx->state, buf, len, high_pad);
}

#endif /* USE_S390X_ASM */


static void poly1305_init (poly1305_context_t *ctx,
			   const byte key[POLY1305_KEYLEN])
{
  POLY1305_STATE *st = &ctx->state;

  ctx->leftover = 0;

  st->h[0] = 0;
  st->h[1] = 0;
  st->h[2] = 0;
  st->h[3] = 0;
  st->h[4] = 0;

  st->r[0] = buf_get_le32(key + 0)  & 0x0fffffff;
  st->r[1] = buf_get_le32(key + 4)  & 0x0ffffffc;
  st->r[2] = buf_get_le32(key + 8)  & 0x0ffffffc;
  st->r[3] = buf_get_le32(key + 12) & 0x0ffffffc;

  st->k[0] = buf_get_le32(key + 16);
  st->k[1] = buf_get_le32(key + 20);
  st->k[2] = buf_get_le32(key + 24);
  st->k[3] = buf_get_le32(key + 28);
}


#ifdef USE_MPI_64BIT

#if defined (__aarch64__) && defined(HAVE_CPU_ARCH_ARM) && __GNUC__ >= 4

/* A += B (armv8/aarch64) */
#define ADD_1305_64(A2, A1, A0, B2, B1, B0) \
      __asm__ ("adds %0, %3, %0\n" \
	       "adcs %1, %4, %1\n" \
	       "adc  %2, %5, %2\n" \
	       : "+r" (A0), "+r" (A1), "+r" (A2) \
	       : "r" (B0), "r" (B1), "r" (B2) \
	       : "cc" )

#endif /* __aarch64__ */

#if defined (__x86_64__) && defined(HAVE_CPU_ARCH_X86) && __GNUC__ >= 4

/* A += B (x86-64) */
#define ADD_1305_64(A2, A1, A0, B2, B1, B0) \
      __asm__ ("addq %3, %0\n" \
	       "adcq %4, %1\n" \
	       "adcq %5, %2\n" \
	       : "+r" (A0), "+r" (A1), "+r" (A2) \
	       : "g" (B0), "g" (B1), "g" (B2) \
	       : "cc" )

#endif /* __x86_64__ */

#if defined (__powerpc__) && defined(HAVE_CPU_ARCH_PPC) && __GNUC__ >= 4

/* A += B (ppc64) */
#define ADD_1305_64(A2, A1, A0, B2, B1, B0) \
      __asm__ ("addc %0, %3, %0\n" \
	       "adde %1, %4, %1\n" \
	       "adde %2, %5, %2\n" \
	       : "+r" (A0), "+r" (A1), "+r" (A2) \
	       : "r" (B0), "r" (B1), "r" (B2) \
	       : "cc" )

#endif /* __powerpc__ */

#ifndef ADD_1305_64
/* A += B (generic, mpi) */
#  define ADD_1305_64(A2, A1, A0, B2, B1, B0) do { \
    u64 carry; \
    add_ssaaaa(carry, A0, 0, A0, 0, B0); \
    add_ssaaaa(A2, A1, A2, A1, B2, B1); \
    add_ssaaaa(A2, A1, A2, A1, 0, carry); \
  } while (0)
#endif

/* H = H * R mod 2¹³⁰-5 */
#define MUL_MOD_1305_64(H2, H1, H0, R1, R0, R1_MULT5) do { \
    u64 x0_lo, x0_hi, x1_lo, x1_hi; \
    u64 t0_lo, t0_hi, t1_lo, t1_hi; \
    \
    /* x = a * r (partial mod 2^130-5) */ \
    umul_ppmm(x0_hi, x0_lo, H0, R0);  /* h0 * r0 */ \
    umul_ppmm(x1_hi, x1_lo, H0, R1);  /* h0 * r1 */ \
    \
    umul_ppmm(t0_hi, t0_lo, H1, R1_MULT5); /* h1 * r1 mod 2^130-5 */ \
    add_ssaaaa(x0_hi, x0_lo, x0_hi, x0_lo, t0_hi, t0_lo); \
    umul_ppmm(t1_hi, t1_lo, H1, R0);       /* h1 * r0 */ \
    add_ssaaaa(x1_hi, x1_lo, x1_hi, x1_lo, t1_hi, t1_lo); \
    \
    t1_lo = H2 * R1_MULT5; /* h2 * r1 mod 2^130-5 */ \
    t1_hi = H2 * R0;       /* h2 * r0 */ \
    add_ssaaaa(H0, H1, x1_hi, x1_lo, t1_hi, t1_lo); \
    \
    /* carry propagation */ \
    H2 = H0 & 3; \
    H0 = (H0 >> 2) * 5; /* msb mod 2^130-5 */ \
    ADD_1305_64(H2, H1, H0, (u64)0, x0_hi, x0_lo); \
  } while (0)

#ifndef HAVE_ASM_POLY1305_BLOCKS

static unsigned int
poly1305_blocks (poly1305_context_t *ctx, const byte *buf, size_t len,
		 byte high_pad)
{
  POLY1305_STATE *st = &ctx->state;
  u64 r0, r1, r1_mult5;
  u64 h0, h1, h2;
  u64 m0, m1, m2;

  m2 = high_pad;

  h0 = st->h[0] + ((u64)st->h[1] << 32);
  h1 = st->h[2] + ((u64)st->h[3] << 32);
  h2 = st->h[4];

  r0 = st->r[0] + ((u64)st->r[1] << 32);
  r1 = st->r[2] + ((u64)st->r[3] << 32);

  r1_mult5 = (r1 >> 2) + r1;

  m0 = buf_get_le64(buf + 0);
  m1 = buf_get_le64(buf + 8);
  buf += POLY1305_BLOCKSIZE;
  len -= POLY1305_BLOCKSIZE;

  while (len >= POLY1305_BLOCKSIZE)
    {
      /* a = h + m */
      ADD_1305_64(h2, h1, h0, m2, m1, m0);

      m0 = buf_get_le64(buf + 0);
      m1 = buf_get_le64(buf + 8);

      /* h = a * r (partial mod 2^130-5) */
      MUL_MOD_1305_64(h2, h1, h0, r1, r0, r1_mult5);

      buf += POLY1305_BLOCKSIZE;
      len -= POLY1305_BLOCKSIZE;
    }

  /* a = h + m */
  ADD_1305_64(h2, h1, h0, m2, m1, m0);

  /* h = a * r (partial mod 2^130-5) */
  MUL_MOD_1305_64(h2, h1, h0, r1, r0, r1_mult5);

  st->h[0] = h0;
  st->h[1] = h0 >> 32;
  st->h[2] = h1;
  st->h[3] = h1 >> 32;
  st->h[4] = h2;

  return 6 * sizeof (void *) + 18 * sizeof (u64);
}

#endif /* !HAVE_ASM_POLY1305_BLOCKS */

static unsigned int poly1305_final (poly1305_context_t *ctx,
				    byte mac[POLY1305_TAGLEN])
{
  POLY1305_STATE *st = &ctx->state;
  unsigned int burn = 0;
  u64 u, carry;
  u64 k0, k1;
  u64 h0, h1;
  u64 h2;

  /* process the remaining block */
  if (ctx->leftover)
    {
      ctx->buffer[ctx->leftover++] = 1;
      if (ctx->leftover < POLY1305_BLOCKSIZE)
	{
	  memset (&ctx->buffer[ctx->leftover], 0,
		  POLY1305_BLOCKSIZE - ctx->leftover);
	  ctx->leftover = POLY1305_BLOCKSIZE;
	}
      burn = poly1305_blocks (ctx, ctx->buffer, POLY1305_BLOCKSIZE, 0);
    }

  h0 = st->h[0] + ((u64)st->h[1] << 32);
  h1 = st->h[2] + ((u64)st->h[3] << 32);
  h2 = st->h[4];

  k0 = st->k[0] + ((u64)st->k[1] << 32);
  k1 = st->k[2] + ((u64)st->k[3] << 32);

  /* check if h is more than 2^130-5, by adding 5. */
  add_ssaaaa(carry, u, 0, h0, 0, 5);
  add_ssaaaa(carry, u, 0, carry, 0, h1);
  u = (carry + h2) >> 2; /* u == 0 or 1 */

  /* minus 2^130-5 ... (+5) */
  u = (-u) & 5;
  add_ssaaaa(h1, h0, h1, h0, 0, u);

  /* add high part of key + h */
  add_ssaaaa(h1, h0, h1, h0, k1, k0);
  buf_put_le64(mac + 0, h0);
  buf_put_le64(mac + 8, h1);

  /* burn_stack */
  return 4 * sizeof (void *) + 7 * sizeof (u64) + burn;
}

#endif /* USE_MPI_64BIT */

#ifdef USE_MPI_32BIT

#ifdef HAVE_COMPATIBLE_GCC_ARM_PLATFORM_AS

/* HI:LO += A * B (arm) */
#define UMUL_ADD_32(HI, LO, A, B) \
      __asm__ ("umlal %1, %0, %4, %5" \
	       : "=r" (HI), "=r" (LO) \
	       : "0" (HI), "1" (LO), "r" (A), "r" (B) )

/* A += B (arm) */
#ifdef __GCC_ASM_FLAG_OUTPUTS__
#  define ADD_1305_32(A4, A3, A2, A1, A0, B4, B3, B2, B1, B0) do { \
      u32 __carry; \
      __asm__ ("adds %0, %0, %5\n" \
	       "adcs %1, %1, %6\n" \
	       "adcs %2, %2, %7\n" \
	       "adcs %3, %3, %8\n" \
	       : "+r" (A0), "+r" (A1), "+r" (A2), "+r" (A3), \
	         "=@cccs" (__carry) \
	       : "r" (B0), "r" (B1), "r" (B2), "r" (B3) \
	       : ); \
      (A4) += (B4) + __carry; \
    } while (0)
#else
#  define ADD_1305_32(A4, A3, A2, A1, A0, B4, B3, B2, B1, B0) do { \
      u32 __carry = (B0); \
      __asm__ ("adds %0, %0, %2\n" \
	       "adcs %1, %1, %3\n" \
	       "rrx %2, %2\n" /* carry to 31th bit */ \
	       : "+r" (A0), "+r" (A1), "+r" (__carry) \
	       : "r" (B1), "r" (0) \
	       : "cc" ); \
      __asm__ ("lsls %0, %0, #1\n" /* carry from 31th bit */ \
	       "adcs %1, %1, %4\n" \
	       "adcs %2, %2, %5\n" \
	       "adc  %3, %3, %6\n" \
	       : "+r" (__carry), "+r" (A2), "+r" (A3), "+r" (A4) \
	       : "r" (B2), "r" (B3), "r" (B4) \
	       : "cc" ); \
    } while (0)
#endif

#endif /* HAVE_COMPATIBLE_GCC_ARM_PLATFORM_AS */

#if defined (__i386__) && defined(HAVE_CPU_ARCH_X86) && __GNUC__ >= 5
/* Note: ADD_1305_32 below does not compile on GCC-4.7 */

/* A += B (i386) */
#define ADD_1305_32(A4, A3, A2, A1, A0, B4, B3, B2, B1, B0) \
      __asm__ ("addl %5, %0\n" \
	       "adcl %6, %1\n" \
	       "adcl %7, %2\n" \
	       "adcl %8, %3\n" \
	       "adcl %9, %4\n" \
	       : "+r" (A0), "+r" (A1), "+r" (A2), "+r" (A3), "+r" (A4) \
	       : "g" (B0), "g" (B1), "g" (B2), "g" (B3), "g" (B4) \
	       : "cc" )

#endif /* __i386__ */

#ifndef UMUL_ADD_32
/* HI:LO += A * B (generic, mpi) */
#  define UMUL_ADD_32(HI, LO, A, B) do { \
    u32 t_lo, t_hi; \
    umul_ppmm(t_hi, t_lo, A, B); \
    add_ssaaaa(HI, LO, HI, LO, t_hi, t_lo); \
  } while (0)
#endif

#ifndef ADD_1305_32
/* A += B (generic, mpi) */
#  define ADD_1305_32(A4, A3, A2, A1, A0, B4, B3, B2, B1, B0) do { \
    u32 carry0, carry1, carry2; \
    add_ssaaaa(carry0, A0, 0, A0, 0, B0); \
    add_ssaaaa(carry1, A1, 0, A1, 0, B1); \
    add_ssaaaa(carry1, A1, carry1, A1, 0, carry0); \
    add_ssaaaa(carry2, A2, 0, A2, 0, B2); \
    add_ssaaaa(carry2, A2, carry2, A2, 0, carry1); \
    add_ssaaaa(A4, A3, A4, A3, B4, B3); \
    add_ssaaaa(A4, A3, A4, A3, 0, carry2); \
  } while (0)
#endif

/* H = H * R mod 2¹³⁰-5 */
#define MUL_MOD_1305_32(H4, H3, H2, H1, H0, R3, R2, R1, R0, \
                        R3_MULT5, R2_MULT5, R1_MULT5) do { \
    u32 x0_lo, x0_hi, x1_lo, x1_hi, x2_lo, x2_hi, x3_lo, x3_hi; \
    u32 t0_lo, t0_hi; \
    \
    /* x = a * r (partial mod 2^130-5) */ \
    umul_ppmm(x0_hi, x0_lo, H0, R0);  /* h0 * r0 */ \
    umul_ppmm(x1_hi, x1_lo, H0, R1);  /* h0 * r1 */ \
    umul_ppmm(x2_hi, x2_lo, H0, R2);  /* h0 * r2 */ \
    umul_ppmm(x3_hi, x3_lo, H0, R3);  /* h0 * r3 */ \
    \
    UMUL_ADD_32(x0_hi, x0_lo, H1, R3_MULT5); /* h1 * r3 mod 2^130-5 */ \
    UMUL_ADD_32(x1_hi, x1_lo, H1, R0);       /* h1 * r0 */ \
    UMUL_ADD_32(x2_hi, x2_lo, H1, R1);       /* h1 * r1 */ \
    UMUL_ADD_32(x3_hi, x3_lo, H1, R2);       /* h1 * r2 */ \
    \
    UMUL_ADD_32(x0_hi, x0_lo, H2, R2_MULT5); /* h2 * r2 mod 2^130-5 */ \
    UMUL_ADD_32(x1_hi, x1_lo, H2, R3_MULT5); /* h2 * r3 mod 2^130-5 */ \
    UMUL_ADD_32(x2_hi, x2_lo, H2, R0);       /* h2 * r0 */ \
    UMUL_ADD_32(x3_hi, x3_lo, H2, R1);       /* h2 * r1 */ \
    \
    UMUL_ADD_32(x0_hi, x0_lo, H3, R1_MULT5); /* h3 * r1 mod 2^130-5 */ \
    H1 = x0_hi; \
    UMUL_ADD_32(x1_hi, x1_lo, H3, R2_MULT5); /* h3 * r2 mod 2^130-5 */ \
    UMUL_ADD_32(x2_hi, x2_lo, H3, R3_MULT5); /* h3 * r3 mod 2^130-5 */ \
    UMUL_ADD_32(x3_hi, x3_lo, H3, R0);       /* h3 * r0 */ \
    \
    t0_lo = H4 * R1_MULT5; /* h4 * r1 mod 2^130-5 */ \
    t0_hi = H4 * R2_MULT5; /* h4 * r2 mod 2^130-5 */ \
    add_ssaaaa(H2, x1_lo, x1_hi, x1_lo, 0, t0_lo); \
    add_ssaaaa(H3, x2_lo, x2_hi, x2_lo, 0, t0_hi); \
    t0_lo = H4 * R3_MULT5; /* h4 * r3 mod 2^130-5 */ \
    t0_hi = H4 * R0;       /* h4 * r0 */ \
    add_ssaaaa(H4, x3_lo, x3_hi, x3_lo, t0_hi, t0_lo); \
    \
    /* carry propagation */ \
    H0 = (H4 >> 2) * 5; /* msb mod 2^130-5 */ \
    H4 = H4 & 3; \
    ADD_1305_32(H4, H3, H2, H1, H0, 0, x3_lo, x2_lo, x1_lo, x0_lo); \
  } while (0)

#ifndef HAVE_ASM_POLY1305_BLOCKS

static unsigned int
poly1305_blocks (poly1305_context_t *ctx, const byte *buf, size_t len,
		 byte high_pad)
{
  POLY1305_STATE *st = &ctx->state;
  u32 r1_mult5, r2_mult5, r3_mult5;
  u32 h0, h1, h2, h3, h4;
  u32 m0, m1, m2, m3, m4;

  m4 = high_pad;

  h0 = st->h[0];
  h1 = st->h[1];
  h2 = st->h[2];
  h3 = st->h[3];
  h4 = st->h[4];

  r1_mult5 = (st->r[1] >> 2) + st->r[1];
  r2_mult5 = (st->r[2] >> 2) + st->r[2];
  r3_mult5 = (st->r[3] >> 2) + st->r[3];

  while (len >= POLY1305_BLOCKSIZE)
    {
      m0 = buf_get_le32(buf + 0);
      m1 = buf_get_le32(buf + 4);
      m2 = buf_get_le32(buf + 8);
      m3 = buf_get_le32(buf + 12);

      /* a = h + m */
      ADD_1305_32(h4, h3, h2, h1, h0, m4, m3, m2, m1, m0);

      /* h = a * r (partial mod 2^130-5) */
      MUL_MOD_1305_32(h4, h3, h2, h1, h0,
		      st->r[3], st->r[2], st->r[1], st->r[0],
		      r3_mult5, r2_mult5, r1_mult5);

      buf += POLY1305_BLOCKSIZE;
      len -= POLY1305_BLOCKSIZE;
    }

  st->h[0] = h0;
  st->h[1] = h1;
  st->h[2] = h2;
  st->h[3] = h3;
  st->h[4] = h4;

  return 6 * sizeof (void *) + 28 * sizeof (u32);
}

#endif /* !HAVE_ASM_POLY1305_BLOCKS */

static unsigned int poly1305_final (poly1305_context_t *ctx,
				    byte mac[POLY1305_TAGLEN])
{
  POLY1305_STATE *st = &ctx->state;
  unsigned int burn = 0;
  u32 carry, tmp0, tmp1, tmp2, u;
  u32 h4, h3, h2, h1, h0;

  /* process the remaining block */
  if (ctx->leftover)
    {
      ctx->buffer[ctx->leftover++] = 1;
      if (ctx->leftover < POLY1305_BLOCKSIZE)
	{
	  memset (&ctx->buffer[ctx->leftover], 0,
		  POLY1305_BLOCKSIZE - ctx->leftover);
	  ctx->leftover = POLY1305_BLOCKSIZE;
	}
      burn = poly1305_blocks (ctx, ctx->buffer, POLY1305_BLOCKSIZE, 0);
    }

  h0 = st->h[0];
  h1 = st->h[1];
  h2 = st->h[2];
  h3 = st->h[3];
  h4 = st->h[4];

  /* check if h is more than 2^130-5, by adding 5. */
  add_ssaaaa(carry, tmp0, 0, h0, 0, 5);
  add_ssaaaa(carry, tmp0, 0, carry, 0, h1);
  add_ssaaaa(carry, tmp0, 0, carry, 0, h2);
  add_ssaaaa(carry, tmp0, 0, carry, 0, h3);
  u = (carry + h4) >> 2; /* u == 0 or 1 */

  /* minus 2^130-5 ... (+5) */
  u = (-u) & 5;
  add_ssaaaa(carry, h0, 0, h0, 0, u);
  add_ssaaaa(carry, h1, 0, h1, 0, carry);
  add_ssaaaa(carry, h2, 0, h2, 0, carry);
  add_ssaaaa(carry, h3, 0, h3, 0, carry);

  /* add high part of key + h */
  add_ssaaaa(tmp0, h0, 0, h0, 0, st->k[0]);
  add_ssaaaa(tmp1, h1, 0, h1, 0, st->k[1]);
  add_ssaaaa(tmp1, h1, tmp1, h1, 0, tmp0);
  add_ssaaaa(tmp2, h2, 0, h2, 0, st->k[2]);
  add_ssaaaa(tmp2, h2, tmp2, h2, 0, tmp1);
  add_ssaaaa(carry, h3, 0, h3, 0, st->k[3]);
  h3 += tmp2;

  buf_put_le32(mac + 0, h0);
  buf_put_le32(mac + 4, h1);
  buf_put_le32(mac + 8, h2);
  buf_put_le32(mac + 12, h3);

  /* burn_stack */
  return 4 * sizeof (void *) + 10 * sizeof (u32) + burn;
}

#endif /* USE_MPI_32BIT */


unsigned int
_gcry_poly1305_update_burn (poly1305_context_t *ctx, const byte *m,
			    size_t bytes)
{
  unsigned int burn = 0;

  /* handle leftover */
  if (ctx->leftover)
    {
      size_t want = (POLY1305_BLOCKSIZE - ctx->leftover);
      if (want > bytes)
	want = bytes;
      buf_cpy (ctx->buffer + ctx->leftover, m, want);
      bytes -= want;
      m += want;
      ctx->leftover += want;
      if (ctx->leftover < POLY1305_BLOCKSIZE)
	return 0;
      burn = poly1305_blocks (ctx, ctx->buffer, POLY1305_BLOCKSIZE, 1);
      ctx->leftover = 0;
    }

  /* process full blocks */
  if (bytes >= POLY1305_BLOCKSIZE)
    {
      size_t nblks = bytes / POLY1305_BLOCKSIZE;
      burn = poly1305_blocks (ctx, m, nblks * POLY1305_BLOCKSIZE, 1);
      m += nblks * POLY1305_BLOCKSIZE;
      bytes -= nblks * POLY1305_BLOCKSIZE;
    }

  /* store leftover */
  if (bytes)
    {
      buf_cpy (ctx->buffer + ctx->leftover, m, bytes);
      ctx->leftover += bytes;
    }

  return burn;
}


void
_gcry_poly1305_update (poly1305_context_t *ctx, const byte *m, size_t bytes)
{
  unsigned int burn;

  burn = _gcry_poly1305_update_burn (ctx, m, bytes);

  if (burn)
    _gcry_burn_stack (burn);
}


void
_gcry_poly1305_finish (poly1305_context_t *ctx, byte mac[POLY1305_TAGLEN])
{
  unsigned int burn;

  burn = poly1305_final (ctx, mac);

  _gcry_burn_stack (burn);
}


gcry_err_code_t
_gcry_poly1305_init (poly1305_context_t * ctx, const byte * key,
		     size_t keylen)
{
  static int initialized;
  static const char *selftest_failed;

  if (!initialized)
    {
      initialized = 1;
      selftest_failed = selftest ();
      if (selftest_failed)
	log_error ("Poly1305 selftest failed (%s)\n", selftest_failed);
    }

  if (keylen != POLY1305_KEYLEN)
    return GPG_ERR_INV_KEYLEN;

  if (selftest_failed)
    return GPG_ERR_SELFTEST_FAILED;

  poly1305_init (ctx, key);

  return 0;
}


static void
poly1305_auth (byte mac[POLY1305_TAGLEN], const byte * m, size_t bytes,
	       const byte * key)
{
  poly1305_context_t ctx;

  memset (&ctx, 0, sizeof (ctx));

  _gcry_poly1305_init (&ctx, key, POLY1305_KEYLEN);
  _gcry_poly1305_update (&ctx, m, bytes);
  _gcry_poly1305_finish (&ctx, mac);

  wipememory (&ctx, sizeof (ctx));
}


static const char *
selftest (void)
{
  /* example from nacl */
  static const byte nacl_key[POLY1305_KEYLEN] = {
    0xee, 0xa6, 0xa7, 0x25, 0x1c, 0x1e, 0x72, 0x91,
    0x6d, 0x11, 0xc2, 0xcb, 0x21, 0x4d, 0x3c, 0x25,
    0x25, 0x39, 0x12, 0x1d, 0x8e, 0x23, 0x4e, 0x65,
    0x2d, 0x65, 0x1f, 0xa4, 0xc8, 0xcf, 0xf8, 0x80,
  };

  static const byte nacl_msg[131] = {
    0x8e, 0x99, 0x3b, 0x9f, 0x48, 0x68, 0x12, 0x73,
    0xc2, 0x96, 0x50, 0xba, 0x32, 0xfc, 0x76, 0xce,
    0x48, 0x33, 0x2e, 0xa7, 0x16, 0x4d, 0x96, 0xa4,
    0x47, 0x6f, 0xb8, 0xc5, 0x31, 0xa1, 0x18, 0x6a,
    0xc0, 0xdf, 0xc1, 0x7c, 0x98, 0xdc, 0xe8, 0x7b,
    0x4d, 0xa7, 0xf0, 0x11, 0xec, 0x48, 0xc9, 0x72,
    0x71, 0xd2, 0xc2, 0x0f, 0x9b, 0x92, 0x8f, 0xe2,
    0x27, 0x0d, 0x6f, 0xb8, 0x63, 0xd5, 0x17, 0x38,
    0xb4, 0x8e, 0xee, 0xe3, 0x14, 0xa7, 0xcc, 0x8a,
    0xb9, 0x32, 0x16, 0x45, 0x48, 0xe5, 0x26, 0xae,
    0x90, 0x22, 0x43, 0x68, 0x51, 0x7a, 0xcf, 0xea,
    0xbd, 0x6b, 0xb3, 0x73, 0x2b, 0xc0, 0xe9, 0xda,
    0x99, 0x83, 0x2b, 0x61, 0xca, 0x01, 0xb6, 0xde,
    0x56, 0x24, 0x4a, 0x9e, 0x88, 0xd5, 0xf9, 0xb3,
    0x79, 0x73, 0xf6, 0x22, 0xa4, 0x3d, 0x14, 0xa6,
    0x59, 0x9b, 0x1f, 0x65, 0x4c, 0xb4, 0x5a, 0x74,
    0xe3, 0x55, 0xa5
  };

  static const byte nacl_mac[16] = {
    0xf3, 0xff, 0xc7, 0x70, 0x3f, 0x94, 0x00, 0xe5,
    0x2a, 0x7d, 0xfb, 0x4b, 0x3d, 0x33, 0x05, 0xd9
  };

  /* generates a final value of (2^130 - 2) == 3 */
  static const byte wrap_key[POLY1305_KEYLEN] = {
    0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  };

  static const byte wrap_msg[16] = {
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
  };

  static const byte wrap_mac[16] = {
    0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  };

  /* mac of the macs of messages of length 0 to 256, where the key and messages
   * have all their values set to the length
   */
  static const byte total_key[POLY1305_KEYLEN] = {
    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0xff, 0xfe, 0xfd, 0xfc, 0xfb, 0xfa, 0xf9,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
  };

  static const byte total_mac[16] = {
    0x64, 0xaf, 0xe2, 0xe8, 0xd6, 0xad, 0x7b, 0xbd,
    0xd2, 0x87, 0xf9, 0x7c, 0x44, 0x62, 0x3d, 0x39
  };

  poly1305_context_t ctx;
  poly1305_context_t total_ctx;
  byte all_key[POLY1305_KEYLEN];
  byte all_msg[256];
  byte mac[16];
  size_t i, j;

  memset (&ctx, 0, sizeof (ctx));
  memset (&total_ctx, 0, sizeof (total_ctx));

  memset (mac, 0, sizeof (mac));
  poly1305_auth (mac, nacl_msg, sizeof (nacl_msg), nacl_key);
  if (memcmp (nacl_mac, mac, sizeof (nacl_mac)) != 0)
    return "Poly1305 test 1 failed.";

  /* SSE2/AVX have a 32 byte block size, but also support 64 byte blocks, so
   * make sure everything still works varying between them */
  memset (mac, 0, sizeof (mac));
  _gcry_poly1305_init (&ctx, nacl_key, POLY1305_KEYLEN);
  _gcry_poly1305_update (&ctx, nacl_msg + 0, 32);
  _gcry_poly1305_update (&ctx, nacl_msg + 32, 64);
  _gcry_poly1305_update (&ctx, nacl_msg + 96, 16);
  _gcry_poly1305_update (&ctx, nacl_msg + 112, 8);
  _gcry_poly1305_update (&ctx, nacl_msg + 120, 4);
  _gcry_poly1305_update (&ctx, nacl_msg + 124, 2);
  _gcry_poly1305_update (&ctx, nacl_msg + 126, 1);
  _gcry_poly1305_update (&ctx, nacl_msg + 127, 1);
  _gcry_poly1305_update (&ctx, nacl_msg + 128, 1);
  _gcry_poly1305_update (&ctx, nacl_msg + 129, 1);
  _gcry_poly1305_update (&ctx, nacl_msg + 130, 1);
  _gcry_poly1305_finish (&ctx, mac);
  if (memcmp (nacl_mac, mac, sizeof (nacl_mac)) != 0)
    return "Poly1305 test 2 failed.";

  memset (mac, 0, sizeof (mac));
  poly1305_auth (mac, wrap_msg, sizeof (wrap_msg), wrap_key);
  if (memcmp (wrap_mac, mac, sizeof (nacl_mac)) != 0)
    return "Poly1305 test 3 failed.";

  _gcry_poly1305_init (&total_ctx, total_key, POLY1305_KEYLEN);
  for (i = 0; i < 256; i++)
    {
      /* set key and message to 'i,i,i..' */
      for (j = 0; j < sizeof (all_key); j++)
	all_key[j] = i;
      for (j = 0; j < i; j++)
	all_msg[j] = i;
      poly1305_auth (mac, all_msg, i, all_key);
      _gcry_poly1305_update (&total_ctx, mac, 16);
    }
  _gcry_poly1305_finish (&total_ctx, mac);
  if (memcmp (total_mac, mac, sizeof (total_mac)) != 0)
    return "Poly1305 test 4 failed.";

  return NULL;
}
