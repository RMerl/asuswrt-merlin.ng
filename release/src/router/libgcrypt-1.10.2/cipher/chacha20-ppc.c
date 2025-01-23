/* chacha20-ppc.c - PowerPC vector implementation of ChaCha20
 * Copyright (C) 2019 Jussi Kivilinna <jussi.kivilinna@iki.fi>
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

#include <config.h>

#if defined(ENABLE_PPC_CRYPTO_SUPPORT) && \
    defined(HAVE_COMPATIBLE_CC_PPC_ALTIVEC) && \
    defined(HAVE_GCC_INLINE_ASM_PPC_ALTIVEC) && \
    defined(USE_CHACHA20) && \
    __GNUC__ >= 4

#include <altivec.h>
#include "bufhelp.h"
#include "poly1305-internal.h"

#include "mpi-internal.h"
#include "longlong.h"


typedef vector unsigned char vector16x_u8;
typedef vector unsigned int vector4x_u32;
typedef vector unsigned long long vector2x_u64;


#define ALWAYS_INLINE inline __attribute__((always_inline))
#define NO_INLINE __attribute__((noinline))
#define NO_INSTRUMENT_FUNCTION __attribute__((no_instrument_function))

#define ASM_FUNC_ATTR          NO_INSTRUMENT_FUNCTION
#define ASM_FUNC_ATTR_INLINE   ASM_FUNC_ATTR ALWAYS_INLINE
#define ASM_FUNC_ATTR_NOINLINE ASM_FUNC_ATTR NO_INLINE


#ifdef WORDS_BIGENDIAN
static const vector16x_u8 le_bswap_const =
  { 3, 2, 1, 0, 7, 6, 5, 4, 11, 10, 9, 8, 15, 14, 13, 12 };
#endif


static ASM_FUNC_ATTR_INLINE vector4x_u32
vec_rol_elems(vector4x_u32 v, unsigned int idx)
{
#ifndef WORDS_BIGENDIAN
  return vec_sld (v, v, (16 - (4 * idx)) & 15);
#else
  return vec_sld (v, v, (4 * idx) & 15);
#endif
}


static ASM_FUNC_ATTR_INLINE vector4x_u32
vec_load_le(unsigned long offset, const unsigned char *ptr)
{
  vector4x_u32 vec;
  vec = vec_vsx_ld (offset, (const u32 *)ptr);
#ifdef WORDS_BIGENDIAN
  vec = (vector4x_u32)vec_perm((vector16x_u8)vec, (vector16x_u8)vec,
			       le_bswap_const);
#endif
  return vec;
}


static ASM_FUNC_ATTR_INLINE void
vec_store_le(vector4x_u32 vec, unsigned long offset, unsigned char *ptr)
{
#ifdef WORDS_BIGENDIAN
  vec = (vector4x_u32)vec_perm((vector16x_u8)vec, (vector16x_u8)vec,
			       le_bswap_const);
#endif
  vec_vsx_st (vec, offset, (u32 *)ptr);
}


static ASM_FUNC_ATTR_INLINE vector4x_u32
vec_add_ctr_u64(vector4x_u32 v, vector4x_u32 a)
{
#ifdef WORDS_BIGENDIAN
  static const vector16x_u8 swap32 =
    { 4, 5, 6, 7, 0, 1, 2, 3, 12, 13, 14, 15, 8, 9, 10, 11 };
  vector2x_u64 vec, add, sum;

  vec = (vector2x_u64)vec_perm((vector16x_u8)v, (vector16x_u8)v, swap32);
  add = (vector2x_u64)vec_perm((vector16x_u8)a, (vector16x_u8)a, swap32);
  sum = vec + add;
  return (vector4x_u32)vec_perm((vector16x_u8)sum, (vector16x_u8)sum, swap32);
#else
  return (vector4x_u32)((vector2x_u64)(v) + (vector2x_u64)(a));
#endif
}


/**********************************************************************
  2-way && 1-way chacha20
 **********************************************************************/

#define ROTATE(v1,rolv)			\
	__asm__ ("vrlw %0,%1,%2\n\t" : "=v" (v1) : "v" (v1), "v" (rolv))

#define WORD_ROL(v1,c)			\
	((v1) = vec_rol_elems((v1), (c)))

#define XOR(ds,s) \
	((ds) ^= (s))

#define PLUS(ds,s) \
	((ds) += (s))

#define QUARTERROUND4(x0,x1,x2,x3,rol_x1,rol_x2,rol_x3) \
	PLUS(x0, x1); XOR(x3, x0); ROTATE(x3, rotate_16); \
	PLUS(x2, x3); XOR(x1, x2); ROTATE(x1, rotate_12); \
	PLUS(x0, x1); XOR(x3, x0); ROTATE(x3, rotate_8); \
	PLUS(x2, x3); \
	  WORD_ROL(x3, rol_x3); \
		      XOR(x1, x2); \
	  WORD_ROL(x2, rol_x2); \
				   ROTATE(x1, rotate_7); \
	  WORD_ROL(x1, rol_x1);

#define ADD_U64(v,a) \
	(v = vec_add_ctr_u64(v, a))

unsigned int ASM_FUNC_ATTR
_gcry_chacha20_ppc8_blocks1(u32 *state, byte *dst, const byte *src,
			    size_t nblks)
{
  vector4x_u32 counter_1 = { 1, 0, 0, 0 };
  vector4x_u32 rotate_16 = { 16, 16, 16, 16 };
  vector4x_u32 rotate_12 = { 12, 12, 12, 12 };
  vector4x_u32 rotate_8 = { 8, 8, 8, 8 };
  vector4x_u32 rotate_7 = { 7, 7, 7, 7 };
  vector4x_u32 state0, state1, state2, state3;
  vector4x_u32 v0, v1, v2, v3;
  vector4x_u32 v4, v5, v6, v7;
  int i;

  /* force preload of constants to vector registers */
  __asm__ ("": "+v" (counter_1) :: "memory");
  __asm__ ("": "+v" (rotate_16) :: "memory");
  __asm__ ("": "+v" (rotate_12) :: "memory");
  __asm__ ("": "+v" (rotate_8) :: "memory");
  __asm__ ("": "+v" (rotate_7) :: "memory");

  state0 = vec_vsx_ld(0 * 16, state);
  state1 = vec_vsx_ld(1 * 16, state);
  state2 = vec_vsx_ld(2 * 16, state);
  state3 = vec_vsx_ld(3 * 16, state);

  while (nblks >= 2)
    {
      v0 = state0;
      v1 = state1;
      v2 = state2;
      v3 = state3;

      v4 = state0;
      v5 = state1;
      v6 = state2;
      v7 = state3;
      ADD_U64(v7, counter_1);

      for (i = 20; i > 0; i -= 2)
	{
	  QUARTERROUND4(v0, v1, v2, v3, 1, 2, 3);
	  QUARTERROUND4(v4, v5, v6, v7, 1, 2, 3);
	  QUARTERROUND4(v0, v1, v2, v3, 3, 2, 1);
	  QUARTERROUND4(v4, v5, v6, v7, 3, 2, 1);
	}

      v0 += state0;
      v1 += state1;
      v2 += state2;
      v3 += state3;
      ADD_U64(state3, counter_1); /* update counter */
      v4 += state0;
      v5 += state1;
      v6 += state2;
      v7 += state3;
      ADD_U64(state3, counter_1); /* update counter */

      v0 ^= vec_load_le(0 * 16, src);
      v1 ^= vec_load_le(1 * 16, src);
      v2 ^= vec_load_le(2 * 16, src);
      v3 ^= vec_load_le(3 * 16, src);
      vec_store_le(v0, 0 * 16, dst);
      vec_store_le(v1, 1 * 16, dst);
      vec_store_le(v2, 2 * 16, dst);
      vec_store_le(v3, 3 * 16, dst);
      src += 64;
      dst += 64;
      v4 ^= vec_load_le(0 * 16, src);
      v5 ^= vec_load_le(1 * 16, src);
      v6 ^= vec_load_le(2 * 16, src);
      v7 ^= vec_load_le(3 * 16, src);
      vec_store_le(v4, 0 * 16, dst);
      vec_store_le(v5, 1 * 16, dst);
      vec_store_le(v6, 2 * 16, dst);
      vec_store_le(v7, 3 * 16, dst);
      src += 64;
      dst += 64;

      nblks -= 2;
    }

  while (nblks)
    {
      v0 = state0;
      v1 = state1;
      v2 = state2;
      v3 = state3;

      for (i = 20; i > 0; i -= 2)
	{
	  QUARTERROUND4(v0, v1, v2, v3, 1, 2, 3);
	  QUARTERROUND4(v0, v1, v2, v3, 3, 2, 1);
	}

      v0 += state0;
      v1 += state1;
      v2 += state2;
      v3 += state3;
      ADD_U64(state3, counter_1); /* update counter */

      v0 ^= vec_load_le(0 * 16, src);
      v1 ^= vec_load_le(1 * 16, src);
      v2 ^= vec_load_le(2 * 16, src);
      v3 ^= vec_load_le(3 * 16, src);
      vec_store_le(v0, 0 * 16, dst);
      vec_store_le(v1, 1 * 16, dst);
      vec_store_le(v2, 2 * 16, dst);
      vec_store_le(v3, 3 * 16, dst);
      src += 64;
      dst += 64;

      nblks--;
    }

  vec_vsx_st(state3, 3 * 16, state); /* store counter */

  return 0;
}


/**********************************************************************
  4-way chacha20
 **********************************************************************/

/* 4x4 32-bit integer matrix transpose */
#define transpose_4x4(x0, x1, x2, x3) ({ \
	vector4x_u32 t1 = vec_mergeh(x0, x2); \
	vector4x_u32 t2 = vec_mergel(x0, x2); \
	vector4x_u32 t3 = vec_mergeh(x1, x3); \
	x3 = vec_mergel(x1, x3); \
	x0 = vec_mergeh(t1, t3); \
	x1 = vec_mergel(t1, t3); \
	x2 = vec_mergeh(t2, x3); \
	x3 = vec_mergel(t2, x3); \
      })

#define QUARTERROUND2(a1,b1,c1,d1,a2,b2,c2,d2)			\
	PLUS(a1,b1); PLUS(a2,b2); XOR(d1,a1); XOR(d2,a2);	\
	    ROTATE(d1, rotate_16); ROTATE(d2, rotate_16);	\
	PLUS(c1,d1); PLUS(c2,d2); XOR(b1,c1); XOR(b2,c2);	\
	    ROTATE(b1, rotate_12); ROTATE(b2, rotate_12);	\
	PLUS(a1,b1); PLUS(a2,b2); XOR(d1,a1); XOR(d2,a2);	\
	    ROTATE(d1, rotate_8); ROTATE(d2, rotate_8);		\
	PLUS(c1,d1); PLUS(c2,d2); XOR(b1,c1); XOR(b2,c2);	\
	    ROTATE(b1, rotate_7); ROTATE(b2, rotate_7);

unsigned int ASM_FUNC_ATTR
_gcry_chacha20_ppc8_blocks4(u32 *state, byte *dst, const byte *src,
			    size_t nblks)
{
  vector4x_u32 counters_0123 = { 0, 1, 2, 3 };
  vector4x_u32 counter_4 = { 4, 0, 0, 0 };
  vector4x_u32 rotate_16 = { 16, 16, 16, 16 };
  vector4x_u32 rotate_12 = { 12, 12, 12, 12 };
  vector4x_u32 rotate_8 = { 8, 8, 8, 8 };
  vector4x_u32 rotate_7 = { 7, 7, 7, 7 };
  vector4x_u32 state0, state1, state2, state3;
  vector4x_u32 v0, v1, v2, v3, v4, v5, v6, v7;
  vector4x_u32 v8, v9, v10, v11, v12, v13, v14, v15;
  vector4x_u32 tmp;
  int i;

  /* force preload of constants to vector registers */
  __asm__ ("": "+v" (counters_0123) :: "memory");
  __asm__ ("": "+v" (counter_4) :: "memory");
  __asm__ ("": "+v" (rotate_16) :: "memory");
  __asm__ ("": "+v" (rotate_12) :: "memory");
  __asm__ ("": "+v" (rotate_8) :: "memory");
  __asm__ ("": "+v" (rotate_7) :: "memory");

  state0 = vec_vsx_ld(0 * 16, state);
  state1 = vec_vsx_ld(1 * 16, state);
  state2 = vec_vsx_ld(2 * 16, state);
  state3 = vec_vsx_ld(3 * 16, state);

  do
    {
      v0 = vec_splat(state0, 0);
      v1 = vec_splat(state0, 1);
      v2 = vec_splat(state0, 2);
      v3 = vec_splat(state0, 3);
      v4 = vec_splat(state1, 0);
      v5 = vec_splat(state1, 1);
      v6 = vec_splat(state1, 2);
      v7 = vec_splat(state1, 3);
      v8 = vec_splat(state2, 0);
      v9 = vec_splat(state2, 1);
      v10 = vec_splat(state2, 2);
      v11 = vec_splat(state2, 3);
      v12 = vec_splat(state3, 0);
      v13 = vec_splat(state3, 1);
      v14 = vec_splat(state3, 2);
      v15 = vec_splat(state3, 3);

      v12 += counters_0123;
      v13 -= vec_cmplt(v12, counters_0123);

      for (i = 20; i > 0; i -= 2)
	{
	  QUARTERROUND2(v0, v4,  v8, v12,   v1, v5,  v9, v13)
	  QUARTERROUND2(v2, v6, v10, v14,   v3, v7, v11, v15)
	  QUARTERROUND2(v0, v5, v10, v15,   v1, v6, v11, v12)
	  QUARTERROUND2(v2, v7,  v8, v13,   v3, v4,  v9, v14)
	}

      v0 += vec_splat(state0, 0);
      v1 += vec_splat(state0, 1);
      v2 += vec_splat(state0, 2);
      v3 += vec_splat(state0, 3);
      v4 += vec_splat(state1, 0);
      v5 += vec_splat(state1, 1);
      v6 += vec_splat(state1, 2);
      v7 += vec_splat(state1, 3);
      v8 += vec_splat(state2, 0);
      v9 += vec_splat(state2, 1);
      v10 += vec_splat(state2, 2);
      v11 += vec_splat(state2, 3);
      tmp = vec_splat(state3, 0);
      tmp += counters_0123;
      v12 += tmp;
      v13 += vec_splat(state3, 1) - vec_cmplt(tmp, counters_0123);
      v14 += vec_splat(state3, 2);
      v15 += vec_splat(state3, 3);
      ADD_U64(state3, counter_4); /* update counter */

      transpose_4x4(v0, v1, v2, v3);
      transpose_4x4(v4, v5, v6, v7);
      transpose_4x4(v8, v9, v10, v11);
      transpose_4x4(v12, v13, v14, v15);

      v0 ^= vec_load_le((64 * 0 + 16 * 0), src);
      v1 ^= vec_load_le((64 * 1 + 16 * 0), src);
      v2 ^= vec_load_le((64 * 2 + 16 * 0), src);
      v3 ^= vec_load_le((64 * 3 + 16 * 0), src);

      v4 ^= vec_load_le((64 * 0 + 16 * 1), src);
      v5 ^= vec_load_le((64 * 1 + 16 * 1), src);
      v6 ^= vec_load_le((64 * 2 + 16 * 1), src);
      v7 ^= vec_load_le((64 * 3 + 16 * 1), src);

      v8 ^= vec_load_le((64 * 0 + 16 * 2), src);
      v9 ^= vec_load_le((64 * 1 + 16 * 2), src);
      v10 ^= vec_load_le((64 * 2 + 16 * 2), src);
      v11 ^= vec_load_le((64 * 3 + 16 * 2), src);

      v12 ^= vec_load_le((64 * 0 + 16 * 3), src);
      v13 ^= vec_load_le((64 * 1 + 16 * 3), src);
      v14 ^= vec_load_le((64 * 2 + 16 * 3), src);
      v15 ^= vec_load_le((64 * 3 + 16 * 3), src);

      vec_store_le(v0, (64 * 0 + 16 * 0), dst);
      vec_store_le(v1, (64 * 1 + 16 * 0), dst);
      vec_store_le(v2, (64 * 2 + 16 * 0), dst);
      vec_store_le(v3, (64 * 3 + 16 * 0), dst);

      vec_store_le(v4, (64 * 0 + 16 * 1), dst);
      vec_store_le(v5, (64 * 1 + 16 * 1), dst);
      vec_store_le(v6, (64 * 2 + 16 * 1), dst);
      vec_store_le(v7, (64 * 3 + 16 * 1), dst);

      vec_store_le(v8, (64 * 0 + 16 * 2), dst);
      vec_store_le(v9, (64 * 1 + 16 * 2), dst);
      vec_store_le(v10, (64 * 2 + 16 * 2), dst);
      vec_store_le(v11, (64 * 3 + 16 * 2), dst);

      vec_store_le(v12, (64 * 0 + 16 * 3), dst);
      vec_store_le(v13, (64 * 1 + 16 * 3), dst);
      vec_store_le(v14, (64 * 2 + 16 * 3), dst);
      vec_store_le(v15, (64 * 3 + 16 * 3), dst);

      src += 4*64;
      dst += 4*64;

      nblks -= 4;
    }
  while (nblks);

  vec_vsx_st(state3, 3 * 16, state); /* store counter */

  return 0;
}


#if SIZEOF_UNSIGNED_LONG == 8

/**********************************************************************
  4-way stitched chacha20-poly1305
 **********************************************************************/

#define ADD_1305_64(A2, A1, A0, B2, B1, B0) \
      __asm__ ("addc %0, %3, %0\n" \
	       "adde %1, %4, %1\n" \
	       "adde %2, %5, %2\n" \
	       : "+r" (A0), "+r" (A1), "+r" (A2) \
	       : "r" (B0), "r" (B1), "r" (B2) \
	       : "cc" )

#define MUL_MOD_1305_64_PART1(H2, H1, H0, R1, R0, R1_MULT5) do { \
    /* x = a * r (partial mod 2^130-5) */ \
    umul_ppmm(x0_hi, x0_lo, H0, R0);  /* h0 * r0 */ \
    umul_ppmm(x1_hi, x1_lo, H0, R1);  /* h0 * r1 */ \
    \
    umul_ppmm(t0_hi, t0_lo, H1, R1_MULT5); /* h1 * r1 mod 2^130-5 */ \
  } while (0)

#define MUL_MOD_1305_64_PART2(H2, H1, H0, R1, R0, R1_MULT5) do { \
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

#define POLY1305_BLOCK_PART1(in_pos) do { \
    m0 = buf_get_le64(poly1305_src + (in_pos) + 0); \
    m1 = buf_get_le64(poly1305_src + (in_pos) + 8); \
    /* a = h + m */ \
    ADD_1305_64(h2, h1, h0, m2, m1, m0); \
    /* h = a * r (partial mod 2^130-5) */ \
    MUL_MOD_1305_64_PART1(h2, h1, h0, r1, r0, r1_mult5); \
  } while (0)

#define POLY1305_BLOCK_PART2(in_pos) do { \
    MUL_MOD_1305_64_PART2(h2, h1, h0, r1, r0, r1_mult5); \
  } while (0)

unsigned int ASM_FUNC_ATTR
_gcry_chacha20_poly1305_ppc8_blocks4(u32 *state, byte *dst, const byte *src,
				     size_t nblks, POLY1305_STATE *st,
				     const byte *poly1305_src)
{
  vector4x_u32 counters_0123 = { 0, 1, 2, 3 };
  vector4x_u32 counter_4 = { 4, 0, 0, 0 };
  vector4x_u32 rotate_16 = { 16, 16, 16, 16 };
  vector4x_u32 rotate_12 = { 12, 12, 12, 12 };
  vector4x_u32 rotate_8 = { 8, 8, 8, 8 };
  vector4x_u32 rotate_7 = { 7, 7, 7, 7 };
  vector4x_u32 state0, state1, state2, state3;
  vector4x_u32 v0, v1, v2, v3, v4, v5, v6, v7;
  vector4x_u32 v8, v9, v10, v11, v12, v13, v14, v15;
  vector4x_u32 tmp;
  u64 r0, r1, r1_mult5;
  u64 h0, h1, h2;
  u64 m0, m1, m2;
  u64 x0_lo, x0_hi, x1_lo, x1_hi;
  u64 t0_lo, t0_hi, t1_lo, t1_hi;
  unsigned int i, o;

  /* load poly1305 state */
  m2 = 1;
  h0 = st->h[0] + ((u64)st->h[1] << 32);
  h1 = st->h[2] + ((u64)st->h[3] << 32);
  h2 = st->h[4];
  r0 = st->r[0] + ((u64)st->r[1] << 32);
  r1 = st->r[2] + ((u64)st->r[3] << 32);
  r1_mult5 = (r1 >> 2) + r1;

  /* force preload of constants to vector registers */
  __asm__ ("": "+v" (counters_0123) :: "memory");
  __asm__ ("": "+v" (counter_4) :: "memory");
  __asm__ ("": "+v" (rotate_16) :: "memory");
  __asm__ ("": "+v" (rotate_12) :: "memory");
  __asm__ ("": "+v" (rotate_8) :: "memory");
  __asm__ ("": "+v" (rotate_7) :: "memory");

  state0 = vec_vsx_ld(0 * 16, state);
  state1 = vec_vsx_ld(1 * 16, state);
  state2 = vec_vsx_ld(2 * 16, state);
  state3 = vec_vsx_ld(3 * 16, state);

  do
    {
      v0 = vec_splat(state0, 0);
      v1 = vec_splat(state0, 1);
      v2 = vec_splat(state0, 2);
      v3 = vec_splat(state0, 3);
      v4 = vec_splat(state1, 0);
      v5 = vec_splat(state1, 1);
      v6 = vec_splat(state1, 2);
      v7 = vec_splat(state1, 3);
      v8 = vec_splat(state2, 0);
      v9 = vec_splat(state2, 1);
      v10 = vec_splat(state2, 2);
      v11 = vec_splat(state2, 3);
      v12 = vec_splat(state3, 0);
      v13 = vec_splat(state3, 1);
      v14 = vec_splat(state3, 2);
      v15 = vec_splat(state3, 3);

      v12 += counters_0123;
      v13 -= vec_cmplt(v12, counters_0123);

      for (o = 20; o; o -= 10)
	{
	  for (i = 8; i; i -= 2)
	    {
	      POLY1305_BLOCK_PART1(0 * 16);
	      QUARTERROUND2(v0, v4,  v8, v12,   v1, v5,  v9, v13)
	      POLY1305_BLOCK_PART2();
	      QUARTERROUND2(v2, v6, v10, v14,   v3, v7, v11, v15)
	      POLY1305_BLOCK_PART1(1 * 16);
	      poly1305_src += 2 * 16;
	      QUARTERROUND2(v0, v5, v10, v15,   v1, v6, v11, v12)
	      POLY1305_BLOCK_PART2();
	      QUARTERROUND2(v2, v7,  v8, v13,   v3, v4,  v9, v14)
	    }

	  QUARTERROUND2(v0, v4,  v8, v12,   v1, v5,  v9, v13)
	  QUARTERROUND2(v2, v6, v10, v14,   v3, v7, v11, v15)
	  QUARTERROUND2(v0, v5, v10, v15,   v1, v6, v11, v12)
	  QUARTERROUND2(v2, v7,  v8, v13,   v3, v4,  v9, v14)
	}

      v0 += vec_splat(state0, 0);
      v1 += vec_splat(state0, 1);
      v2 += vec_splat(state0, 2);
      v3 += vec_splat(state0, 3);
      v4 += vec_splat(state1, 0);
      v5 += vec_splat(state1, 1);
      v6 += vec_splat(state1, 2);
      v7 += vec_splat(state1, 3);
      v8 += vec_splat(state2, 0);
      v9 += vec_splat(state2, 1);
      v10 += vec_splat(state2, 2);
      v11 += vec_splat(state2, 3);
      tmp = vec_splat(state3, 0);
      tmp += counters_0123;
      v12 += tmp;
      v13 += vec_splat(state3, 1) - vec_cmplt(tmp, counters_0123);
      v14 += vec_splat(state3, 2);
      v15 += vec_splat(state3, 3);
      ADD_U64(state3, counter_4); /* update counter */

      transpose_4x4(v0, v1, v2, v3);
      transpose_4x4(v4, v5, v6, v7);
      transpose_4x4(v8, v9, v10, v11);
      transpose_4x4(v12, v13, v14, v15);

      v0 ^= vec_load_le((64 * 0 + 16 * 0), src);
      v1 ^= vec_load_le((64 * 1 + 16 * 0), src);
      v2 ^= vec_load_le((64 * 2 + 16 * 0), src);
      v3 ^= vec_load_le((64 * 3 + 16 * 0), src);

      v4 ^= vec_load_le((64 * 0 + 16 * 1), src);
      v5 ^= vec_load_le((64 * 1 + 16 * 1), src);
      v6 ^= vec_load_le((64 * 2 + 16 * 1), src);
      v7 ^= vec_load_le((64 * 3 + 16 * 1), src);

      v8 ^= vec_load_le((64 * 0 + 16 * 2), src);
      v9 ^= vec_load_le((64 * 1 + 16 * 2), src);
      v10 ^= vec_load_le((64 * 2 + 16 * 2), src);
      v11 ^= vec_load_le((64 * 3 + 16 * 2), src);

      v12 ^= vec_load_le((64 * 0 + 16 * 3), src);
      v13 ^= vec_load_le((64 * 1 + 16 * 3), src);
      v14 ^= vec_load_le((64 * 2 + 16 * 3), src);
      v15 ^= vec_load_le((64 * 3 + 16 * 3), src);

      vec_store_le(v0, (64 * 0 + 16 * 0), dst);
      vec_store_le(v1, (64 * 1 + 16 * 0), dst);
      vec_store_le(v2, (64 * 2 + 16 * 0), dst);
      vec_store_le(v3, (64 * 3 + 16 * 0), dst);

      vec_store_le(v4, (64 * 0 + 16 * 1), dst);
      vec_store_le(v5, (64 * 1 + 16 * 1), dst);
      vec_store_le(v6, (64 * 2 + 16 * 1), dst);
      vec_store_le(v7, (64 * 3 + 16 * 1), dst);

      vec_store_le(v8, (64 * 0 + 16 * 2), dst);
      vec_store_le(v9, (64 * 1 + 16 * 2), dst);
      vec_store_le(v10, (64 * 2 + 16 * 2), dst);
      vec_store_le(v11, (64 * 3 + 16 * 2), dst);

      vec_store_le(v12, (64 * 0 + 16 * 3), dst);
      vec_store_le(v13, (64 * 1 + 16 * 3), dst);
      vec_store_le(v14, (64 * 2 + 16 * 3), dst);
      vec_store_le(v15, (64 * 3 + 16 * 3), dst);

      src += 4*64;
      dst += 4*64;

      nblks -= 4;
    }
  while (nblks);

  vec_vsx_st(state3, 3 * 16, state); /* store counter */

  /* store poly1305 state */
  st->h[0] = h0;
  st->h[1] = h0 >> 32;
  st->h[2] = h1;
  st->h[3] = h1 >> 32;
  st->h[4] = h2;

  return 0;
}

#endif /* SIZEOF_UNSIGNED_LONG == 8 */

#endif /* ENABLE_PPC_CRYPTO_SUPPORT */
