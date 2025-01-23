/* crc-ppc.c - POWER8 vpmsum accelerated CRC implementation
 * Copyright (C) 2019-2020 Jussi Kivilinna <jussi.kivilinna@iki.fi>
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
 * License along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 *
 */

#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "g10lib.h"

#include "bithelp.h"
#include "bufhelp.h"


#if defined(ENABLE_PPC_CRYPTO_SUPPORT) && \
    defined(HAVE_COMPATIBLE_CC_PPC_ALTIVEC) && \
    defined(HAVE_GCC_INLINE_ASM_PPC_ALTIVEC) && \
    __GNUC__ >= 4

#include <altivec.h>
#include "bufhelp.h"


#define ALWAYS_INLINE inline __attribute__((always_inline))
#define NO_INLINE __attribute__((noinline))
#define NO_INSTRUMENT_FUNCTION __attribute__((no_instrument_function))

#define ASM_FUNC_ATTR          NO_INSTRUMENT_FUNCTION
#define ASM_FUNC_ATTR_INLINE   ASM_FUNC_ATTR ALWAYS_INLINE
#define ASM_FUNC_ATTR_NOINLINE ASM_FUNC_ATTR NO_INLINE

#define ALIGNED_64 __attribute__ ((aligned (64)))


typedef vector unsigned char vector16x_u8;
typedef vector unsigned int vector4x_u32;
typedef vector unsigned long long vector2x_u64;


/* Constants structure for generic reflected/non-reflected CRC32 PMULL
 * functions. */
struct crc32_consts_s
{
  /* k: { x^(32*17), x^(32*15), x^(32*5), x^(32*3), x^(32*2), 0 } mod P(x) */
  unsigned long long k[6];
  /* my_p: { floor(x^64 / P(x)), P(x) } */
  unsigned long long my_p[2];
};

/* PMULL constants for CRC32 and CRC32RFC1510. */
static const struct crc32_consts_s crc32_consts ALIGNED_64 =
{
  { /* k[6] = reverse_33bits( x^(32*y) mod P(x) ) */
    U64_C(0x154442bd4), U64_C(0x1c6e41596), /* y = { 17, 15 } */
    U64_C(0x1751997d0), U64_C(0x0ccaa009e), /* y = { 5, 3 } */
    U64_C(0x163cd6124), 0                   /* y = 2 */
  },
  { /* my_p[2] = reverse_33bits ( { floor(x^64 / P(x)), P(x) } ) */
    U64_C(0x1f7011641), U64_C(0x1db710641)
  }
};

/* PMULL constants for CRC24RFC2440 (polynomial multiplied with x⁸). */
static const struct crc32_consts_s crc24rfc2440_consts ALIGNED_64 =
{
  { /* k[6] = x^(32*y) mod P(x) << 32*/
    U64_C(0x08289a00) << 32, U64_C(0x74b44a00) << 32, /* y = { 17, 15 } */
    U64_C(0xc4b14d00) << 32, U64_C(0xfd7e0c00) << 32, /* y = { 5, 3 } */
    U64_C(0xd9fe8c00) << 32, 0                        /* y = 2 */
  },
  { /* my_p[2] = { floor(x^64 / P(x)), P(x) } */
    U64_C(0x1f845fe24), U64_C(0x1864cfb00)
  }
};


static ASM_FUNC_ATTR_INLINE vector2x_u64
asm_vpmsumd(vector2x_u64 a, vector2x_u64 b)
{
  __asm__("vpmsumd %0, %1, %2"
	  : "=v" (a)
	  : "v" (a), "v" (b));
  return a;
}


static ASM_FUNC_ATTR_INLINE vector2x_u64
asm_swap_u64(vector2x_u64 a)
{
  __asm__("xxswapd %x0, %x1"
	  : "=wa" (a)
	  : "wa" (a));
  return a;
}


static ASM_FUNC_ATTR_INLINE vector4x_u32
vec_sld_u32(vector4x_u32 a, vector4x_u32 b, unsigned int idx)
{
  return vec_sld (a, b, (4 * idx) & 15);
}


static const byte crc32_partial_fold_input_mask[16 + 16] ALIGNED_64 =
  {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  };
static const byte crc32_shuf_shift[3 * 16] ALIGNED_64 =
  {
    0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f,
    0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f,
    0x0f, 0x0e, 0x0d, 0x0c, 0x0b, 0x0a, 0x09, 0x08,
    0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01, 0x00,
    0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f,
    0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f,
  };
static const byte crc32_refl_shuf_shift[3 * 16] ALIGNED_64 =
  {
    0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f,
    0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f,
    0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f,
  };
static const vector16x_u8 bswap_const ALIGNED_64 =
  { 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0 };


#define CRC_VEC_SWAP(v) ({ vector2x_u64 __vecu64 = (v); \
                           vec_perm(__vecu64, __vecu64, bswap_const); })

#ifdef WORDS_BIGENDIAN
# define CRC_VEC_U64_DEF(lo, hi) { (hi), (lo) }
# define CRC_VEC_U64_LOAD(offs, ptr) \
	  asm_swap_u64(asm_vec_u64_load(offs, ptr))
# define CRC_VEC_U64_LOAD_LE(offs, ptr) \
	  CRC_VEC_SWAP(asm_vec_u64_load(offs, ptr))
# define CRC_VEC_U64_LOAD_BE(offs, ptr) \
	  asm_vec_u64_load(offs, ptr)
# define CRC_VEC_SWAP_TO_LE(v) CRC_VEC_SWAP(v)
# define CRC_VEC_SWAP_TO_BE(v) (v)
# define VEC_U64_LO 1
# define VEC_U64_HI 0

static ASM_FUNC_ATTR_INLINE vector2x_u64
asm_vec_u64_load(unsigned long offset, const void *ptr)
{
  vector2x_u64 vecu64;
#if __GNUC__ >= 4
  if (__builtin_constant_p (offset) && offset == 0)
    __asm__ volatile ("lxvd2x %x0,0,%1\n\t"
		      : "=wa" (vecu64)
		      : "r" ((uintptr_t)ptr)
		      : "memory");
  else
#endif
    __asm__ volatile ("lxvd2x %x0,%1,%2\n\t"
		      : "=wa" (vecu64)
		      : "r" (offset), "r" ((uintptr_t)ptr)
		      : "memory", "r0");
  return vecu64;
}
#else
# define CRC_VEC_U64_DEF(lo, hi) { (lo), (hi) }
# define CRC_VEC_U64_LOAD(offs, ptr) asm_vec_u64_load_le(offs, ptr)
# define CRC_VEC_U64_LOAD_LE(offs, ptr) asm_vec_u64_load_le(offs, ptr)
# define CRC_VEC_U64_LOAD_BE(offs, ptr) asm_vec_u64_load_be(offs, ptr)
# define CRC_VEC_SWAP_TO_LE(v) (v)
# define CRC_VEC_SWAP_TO_BE(v) CRC_VEC_SWAP(v)
# define VEC_U64_LO 0
# define VEC_U64_HI 1

static ASM_FUNC_ATTR_INLINE vector2x_u64
asm_vec_u64_load_le(unsigned long offset, const void *ptr)
{
  vector2x_u64 vecu64;
#if __GNUC__ >= 4
  if (__builtin_constant_p (offset) && offset == 0)
    __asm__ volatile ("lxvd2x %x0,0,%1\n\t"
		      : "=wa" (vecu64)
		      : "r" ((uintptr_t)ptr)
		      : "memory");
  else
#endif
    __asm__ volatile ("lxvd2x %x0,%1,%2\n\t"
		      : "=wa" (vecu64)
		      : "r" (offset), "r" ((uintptr_t)ptr)
		      : "memory", "r0");
  return asm_swap_u64(vecu64);
}

static ASM_FUNC_ATTR_INLINE vector2x_u64
asm_vec_u64_load_be(unsigned int offset, const void *ptr)
{
  static const vector16x_u8 vec_load_le_const =
    { ~7, ~6, ~5, ~4, ~3, ~2, ~1, ~0, ~15, ~14, ~13, ~12, ~11, ~10, ~9, ~8 };
  vector2x_u64 vecu64;

#if __GNUC__ >= 4
  if (__builtin_constant_p (offset) && offset == 0)
    __asm__ ("lxvd2x %%vs32,0,%1\n\t"
	     "vperm %0,%%v0,%%v0,%2\n\t"
	     : "=v" (vecu64)
	     : "r" ((uintptr_t)(ptr)), "v" (vec_load_le_const)
	     : "memory", "v0");
#endif
  else
    __asm__ ("lxvd2x %%vs32,%1,%2\n\t"
	     "vperm %0,%%v0,%%v0,%3\n\t"
	     : "=v" (vecu64)
	     : "r" (offset), "r" ((uintptr_t)(ptr)),
	       "v" (vec_load_le_const)
	     : "memory", "r0", "v0");

  return vecu64;
}
#endif


static ASM_FUNC_ATTR_INLINE void
crc32r_ppc8_ce_bulk (u32 *pcrc, const byte *inbuf, size_t inlen,
		     const struct crc32_consts_s *consts)
{
  vector4x_u32 zero = { 0, 0, 0, 0 };
  vector2x_u64 low_64bit_mask = CRC_VEC_U64_DEF((u64)-1, 0);
  vector2x_u64 low_32bit_mask = CRC_VEC_U64_DEF((u32)-1, 0);
  vector2x_u64 my_p = CRC_VEC_U64_LOAD(0, &consts->my_p[0]);
  vector2x_u64 k1k2 = CRC_VEC_U64_LOAD(0, &consts->k[1 - 1]);
  vector2x_u64 k3k4 = CRC_VEC_U64_LOAD(0, &consts->k[3 - 1]);
  vector2x_u64 k4lo = CRC_VEC_U64_DEF(k3k4[VEC_U64_HI], 0);
  vector2x_u64 k5lo = CRC_VEC_U64_LOAD(0, &consts->k[5 - 1]);
  vector2x_u64 crc = CRC_VEC_U64_DEF(*pcrc, 0);
  vector2x_u64 crc0, crc1, crc2, crc3;
  vector2x_u64 v0;

  if (inlen >= 8 * 16)
    {
      crc0 = CRC_VEC_U64_LOAD_LE(0 * 16, inbuf);
      crc0 ^= crc;
      crc1 = CRC_VEC_U64_LOAD_LE(1 * 16, inbuf);
      crc2 = CRC_VEC_U64_LOAD_LE(2 * 16, inbuf);
      crc3 = CRC_VEC_U64_LOAD_LE(3 * 16, inbuf);

      inbuf += 4 * 16;
      inlen -= 4 * 16;

      /* Fold by 4. */
      while (inlen >= 4 * 16)
	{
	  v0 = CRC_VEC_U64_LOAD_LE(0 * 16, inbuf);
	  crc0 = asm_vpmsumd(crc0, k1k2) ^ v0;

	  v0 = CRC_VEC_U64_LOAD_LE(1 * 16, inbuf);
	  crc1 = asm_vpmsumd(crc1, k1k2) ^ v0;

	  v0 = CRC_VEC_U64_LOAD_LE(2 * 16, inbuf);
	  crc2 = asm_vpmsumd(crc2, k1k2) ^ v0;

	  v0 = CRC_VEC_U64_LOAD_LE(3 * 16, inbuf);
	  crc3 = asm_vpmsumd(crc3, k1k2) ^ v0;

	  inbuf += 4 * 16;
	  inlen -= 4 * 16;
	}

      /* Fold 4 to 1. */
      crc1 ^= asm_vpmsumd(crc0, k3k4);
      crc2 ^= asm_vpmsumd(crc1, k3k4);
      crc3 ^= asm_vpmsumd(crc2, k3k4);
      crc = crc3;
    }
  else
    {
      v0 = CRC_VEC_U64_LOAD_LE(0, inbuf);
      crc ^= v0;

      inbuf += 16;
      inlen -= 16;
    }

  /* Fold by 1. */
  while (inlen >= 16)
    {
      v0 = CRC_VEC_U64_LOAD_LE(0, inbuf);
      crc = asm_vpmsumd(k3k4, crc);
      crc ^= v0;

      inbuf += 16;
      inlen -= 16;
    }

  /* Partial fold. */
  if (inlen)
    {
      /* Load last input and add padding zeros. */
      vector2x_u64 mask = CRC_VEC_U64_LOAD_LE(inlen, crc32_partial_fold_input_mask);
      vector2x_u64 shl_shuf = CRC_VEC_U64_LOAD_LE(inlen, crc32_refl_shuf_shift);
      vector2x_u64 shr_shuf = CRC_VEC_U64_LOAD_LE(inlen + 16, crc32_refl_shuf_shift);

      v0 = CRC_VEC_U64_LOAD_LE(inlen - 16, inbuf);
      v0 &= mask;

      crc = CRC_VEC_SWAP_TO_LE(crc);
      v0 |= (vector2x_u64)vec_perm((vector16x_u8)crc, (vector16x_u8)zero,
				   (vector16x_u8)shr_shuf);
      crc = (vector2x_u64)vec_perm((vector16x_u8)crc, (vector16x_u8)zero,
				   (vector16x_u8)shl_shuf);
      crc = asm_vpmsumd(k3k4, crc);
      crc ^= v0;

      inbuf += inlen;
      inlen -= inlen;
    }

  /* Final fold. */

  /* reduce 128-bits to 96-bits */
  v0 = asm_swap_u64(crc);
  v0 &= low_64bit_mask;
  crc = asm_vpmsumd(k4lo, crc);
  crc ^= v0;

  /* reduce 96-bits to 64-bits */
  v0 = (vector2x_u64)vec_sld_u32((vector4x_u32)crc,
				 (vector4x_u32)crc, 3);  /* [x0][x3][x2][x1] */
  v0 &= low_64bit_mask;                                  /* [00][00][x2][x1] */
  crc = crc & low_32bit_mask;                            /* [00][00][00][x0] */
  crc = v0 ^ asm_vpmsumd(k5lo, crc);                     /* [00][00][xx][xx] */

  /* barrett reduction */
  v0 = crc << 32;                                        /* [00][00][x0][00] */
  v0 = asm_vpmsumd(my_p, v0);
  v0 = asm_swap_u64(v0);
  v0 = asm_vpmsumd(my_p, v0);
  crc = (vector2x_u64)vec_sld_u32((vector4x_u32)crc,
				  zero, 1);              /* [00][x1][x0][00] */
  crc ^= v0;

  *pcrc = (u32)crc[VEC_U64_HI];
}


static ASM_FUNC_ATTR_INLINE u32
crc32r_ppc8_ce_reduction_4 (u32 data, u32 crc,
			    const struct crc32_consts_s *consts)
{
  vector4x_u32 zero = { 0, 0, 0, 0 };
  vector2x_u64 my_p = CRC_VEC_U64_LOAD(0, &consts->my_p[0]);
  vector2x_u64 v0 = CRC_VEC_U64_DEF((u64)data, 0);
  v0 = asm_vpmsumd(v0, my_p);                          /* [00][00][xx][xx] */
  v0 = (vector2x_u64)vec_sld_u32((vector4x_u32)v0,
				 zero, 3);             /* [x0][00][00][00] */
  v0 = (vector2x_u64)vec_sld_u32((vector4x_u32)v0,
				 (vector4x_u32)v0, 3); /* [00][x0][00][00] */
  v0 = asm_vpmsumd(v0, my_p);                          /* [00][00][xx][xx] */
  return (v0[VEC_U64_LO] >> 32) ^ crc;
}


static ASM_FUNC_ATTR_INLINE void
crc32r_less_than_16 (u32 *pcrc, const byte *inbuf, size_t inlen,
		     const struct crc32_consts_s *consts)
{
  u32 crc = *pcrc;
  u32 data;

  while (inlen >= 4)
    {
      data = buf_get_le32(inbuf);
      data ^= crc;

      inlen -= 4;
      inbuf += 4;

      crc = crc32r_ppc8_ce_reduction_4 (data, 0, consts);
    }

  switch (inlen)
    {
    case 0:
      break;
    case 1:
      data = inbuf[0];
      data ^= crc;
      data <<= 24;
      crc >>= 8;
      crc = crc32r_ppc8_ce_reduction_4 (data, crc, consts);
      break;
    case 2:
      data = inbuf[0] << 0;
      data |= inbuf[1] << 8;
      data ^= crc;
      data <<= 16;
      crc >>= 16;
      crc = crc32r_ppc8_ce_reduction_4 (data, crc, consts);
      break;
    case 3:
      data = inbuf[0] << 0;
      data |= inbuf[1] << 8;
      data |= inbuf[2] << 16;
      data ^= crc;
      data <<= 8;
      crc >>= 24;
      crc = crc32r_ppc8_ce_reduction_4 (data, crc, consts);
      break;
    }

  *pcrc = crc;
}


static ASM_FUNC_ATTR_INLINE void
crc32_ppc8_ce_bulk (u32 *pcrc, const byte *inbuf, size_t inlen,
		    const struct crc32_consts_s *consts)
{
  vector4x_u32 zero = { 0, 0, 0, 0 };
  vector2x_u64 low_96bit_mask = CRC_VEC_U64_DEF(~0, ~((u64)(u32)-1 << 32));
  vector2x_u64 p_my = asm_swap_u64(CRC_VEC_U64_LOAD(0, &consts->my_p[0]));
  vector2x_u64 p_my_lo, p_my_hi;
  vector2x_u64 k2k1 = asm_swap_u64(CRC_VEC_U64_LOAD(0, &consts->k[1 - 1]));
  vector2x_u64 k4k3 = asm_swap_u64(CRC_VEC_U64_LOAD(0, &consts->k[3 - 1]));
  vector2x_u64 k4hi = CRC_VEC_U64_DEF(0, consts->k[4 - 1]);
  vector2x_u64 k5hi = CRC_VEC_U64_DEF(0, consts->k[5 - 1]);
  vector2x_u64 crc = CRC_VEC_U64_DEF(0, _gcry_bswap64(*pcrc));
  vector2x_u64 crc0, crc1, crc2, crc3;
  vector2x_u64 v0;

  if (inlen >= 8 * 16)
    {
      crc0 = CRC_VEC_U64_LOAD_BE(0 * 16, inbuf);
      crc0 ^= crc;
      crc1 = CRC_VEC_U64_LOAD_BE(1 * 16, inbuf);
      crc2 = CRC_VEC_U64_LOAD_BE(2 * 16, inbuf);
      crc3 = CRC_VEC_U64_LOAD_BE(3 * 16, inbuf);

      inbuf += 4 * 16;
      inlen -= 4 * 16;

      /* Fold by 4. */
      while (inlen >= 4 * 16)
	{
	  v0 = CRC_VEC_U64_LOAD_BE(0 * 16, inbuf);
	  crc0 = asm_vpmsumd(crc0, k2k1) ^ v0;

	  v0 = CRC_VEC_U64_LOAD_BE(1 * 16, inbuf);
	  crc1 = asm_vpmsumd(crc1, k2k1) ^ v0;

	  v0 = CRC_VEC_U64_LOAD_BE(2 * 16, inbuf);
	  crc2 = asm_vpmsumd(crc2, k2k1) ^ v0;

	  v0 = CRC_VEC_U64_LOAD_BE(3 * 16, inbuf);
	  crc3 = asm_vpmsumd(crc3, k2k1) ^ v0;

	  inbuf += 4 * 16;
	  inlen -= 4 * 16;
	}

      /* Fold 4 to 1. */
      crc1 ^= asm_vpmsumd(crc0, k4k3);
      crc2 ^= asm_vpmsumd(crc1, k4k3);
      crc3 ^= asm_vpmsumd(crc2, k4k3);
      crc = crc3;
    }
  else
    {
      v0 = CRC_VEC_U64_LOAD_BE(0, inbuf);
      crc ^= v0;

      inbuf += 16;
      inlen -= 16;
    }

  /* Fold by 1. */
  while (inlen >= 16)
    {
      v0 = CRC_VEC_U64_LOAD_BE(0, inbuf);
      crc = asm_vpmsumd(k4k3, crc);
      crc ^= v0;

      inbuf += 16;
      inlen -= 16;
    }

  /* Partial fold. */
  if (inlen)
    {
      /* Load last input and add padding zeros. */
      vector2x_u64 mask = CRC_VEC_U64_LOAD_LE(inlen, crc32_partial_fold_input_mask);
      vector2x_u64 shl_shuf = CRC_VEC_U64_LOAD_LE(32 - inlen, crc32_refl_shuf_shift);
      vector2x_u64 shr_shuf = CRC_VEC_U64_LOAD_LE(inlen + 16, crc32_shuf_shift);

      v0 = CRC_VEC_U64_LOAD_LE(inlen - 16, inbuf);
      v0 &= mask;

      crc = CRC_VEC_SWAP_TO_LE(crc);
      crc2 = (vector2x_u64)vec_perm((vector16x_u8)crc, (vector16x_u8)zero,
				    (vector16x_u8)shr_shuf);
      v0 |= crc2;
      v0 = CRC_VEC_SWAP(v0);
      crc = (vector2x_u64)vec_perm((vector16x_u8)crc, (vector16x_u8)zero,
				   (vector16x_u8)shl_shuf);
      crc = asm_vpmsumd(k4k3, crc);
      crc ^= v0;

      inbuf += inlen;
      inlen -= inlen;
    }

  /* Final fold. */

  /* reduce 128-bits to 96-bits */
  v0 = (vector2x_u64)vec_sld_u32((vector4x_u32)crc,
				 (vector4x_u32)zero, 2);
  crc = asm_vpmsumd(k4hi, crc);
  crc ^= v0; /* bottom 32-bit are zero */

  /* reduce 96-bits to 64-bits */
  v0 = crc & low_96bit_mask;    /* [00][x2][x1][00] */
  crc >>= 32;                   /* [00][x3][00][x0] */
  crc = asm_vpmsumd(k5hi, crc); /* [00][xx][xx][00] */
  crc ^= v0;                    /* top and bottom 32-bit are zero */

  /* barrett reduction */
  p_my_hi = p_my;
  p_my_lo = p_my;
  p_my_hi[VEC_U64_LO] = 0;
  p_my_lo[VEC_U64_HI] = 0;
  v0 = crc >> 32;                                        /* [00][00][00][x1] */
  crc = asm_vpmsumd(p_my_hi, crc);                       /* [00][xx][xx][xx] */
  crc = (vector2x_u64)vec_sld_u32((vector4x_u32)crc,
				  (vector4x_u32)crc, 3); /* [x0][00][x2][x1] */
  crc = asm_vpmsumd(p_my_lo, crc);                       /* [00][xx][xx][xx] */
  crc ^= v0;

  *pcrc = _gcry_bswap32(crc[VEC_U64_LO]);
}


static ASM_FUNC_ATTR_INLINE u32
crc32_ppc8_ce_reduction_4 (u32 data, u32 crc,
			   const struct crc32_consts_s *consts)
{
  vector2x_u64 my_p = CRC_VEC_U64_LOAD(0, &consts->my_p[0]);
  vector2x_u64 v0 = CRC_VEC_U64_DEF((u64)data << 32, 0);
  v0 = asm_vpmsumd(v0, my_p); /* [00][x1][x0][00] */
  v0[VEC_U64_LO] = 0;         /* [00][x1][00][00] */
  v0 = asm_vpmsumd(v0, my_p); /* [00][00][xx][xx] */
  return _gcry_bswap32(v0[VEC_U64_LO]) ^ crc;
}


static ASM_FUNC_ATTR_INLINE void
crc32_less_than_16 (u32 *pcrc, const byte *inbuf, size_t inlen,
		    const struct crc32_consts_s *consts)
{
  u32 crc = *pcrc;
  u32 data;

  while (inlen >= 4)
    {
      data = buf_get_le32(inbuf);
      data ^= crc;
      data = _gcry_bswap32(data);

      inlen -= 4;
      inbuf += 4;

      crc = crc32_ppc8_ce_reduction_4 (data, 0, consts);
    }

  switch (inlen)
    {
    case 0:
      break;
    case 1:
      data = inbuf[0];
      data ^= crc;
      data = data & 0xffU;
      crc = crc >> 8;
      crc = crc32_ppc8_ce_reduction_4 (data, crc, consts);
      break;
    case 2:
      data = inbuf[0] << 0;
      data |= inbuf[1] << 8;
      data ^= crc;
      data = _gcry_bswap32(data << 16);
      crc = crc >> 16;
      crc = crc32_ppc8_ce_reduction_4 (data, crc, consts);
      break;
    case 3:
      data = inbuf[0] << 0;
      data |= inbuf[1] << 8;
      data |= inbuf[2] << 16;
      data ^= crc;
      data = _gcry_bswap32(data << 8);
      crc = crc >> 24;
      crc = crc32_ppc8_ce_reduction_4 (data, crc, consts);
      break;
    }

  *pcrc = crc;
}

void ASM_FUNC_ATTR
_gcry_crc32_ppc8_vpmsum (u32 *pcrc, const byte *inbuf, size_t inlen)
{
  const struct crc32_consts_s *consts = &crc32_consts;

  if (!inlen)
    return;

  if (inlen >= 16)
    crc32r_ppc8_ce_bulk (pcrc, inbuf, inlen, consts);
  else
    crc32r_less_than_16 (pcrc, inbuf, inlen, consts);
}

void ASM_FUNC_ATTR
_gcry_crc24rfc2440_ppc8_vpmsum (u32 *pcrc, const byte *inbuf, size_t inlen)
{
  const struct crc32_consts_s *consts = &crc24rfc2440_consts;

  if (!inlen)
    return;

  /* Note: *pcrc in input endian. */

  if (inlen >= 16)
    crc32_ppc8_ce_bulk (pcrc, inbuf, inlen, consts);
  else
    crc32_less_than_16 (pcrc, inbuf, inlen, consts);
}

#endif
