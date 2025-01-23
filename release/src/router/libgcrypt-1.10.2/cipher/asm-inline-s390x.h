/* asm-inline-s390x.h  -  Common macros for zSeries inline assembly
 *
 * Copyright (C) 2020 Jussi Kivilinna <jussi.kivilinna@iki.fi>
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

#ifndef GCRY_ASM_INLINE_S390X_H
#define GCRY_ASM_INLINE_S390X_H

#include <config.h>

#define ALWAYS_INLINE inline __attribute__((always_inline))

typedef unsigned int u128_t __attribute__ ((mode (TI)));

enum kmxx_functions_e
{
  KM_FUNCTION_AES_128 = 18,
  KM_FUNCTION_AES_192 = 19,
  KM_FUNCTION_AES_256 = 20,
  KM_FUNCTION_XTS_AES_128 = 50,
  KM_FUNCTION_XTS_AES_256 = 52,

  KMID_FUNCTION_SHA1 = 1,
  KMID_FUNCTION_SHA256 = 2,
  KMID_FUNCTION_SHA512 = 3,
  KMID_FUNCTION_SHA3_224 = 32,
  KMID_FUNCTION_SHA3_256 = 33,
  KMID_FUNCTION_SHA3_384 = 34,
  KMID_FUNCTION_SHA3_512 = 35,
  KMID_FUNCTION_SHAKE128 = 36,
  KMID_FUNCTION_SHAKE256 = 37,
  KMID_FUNCTION_GHASH = 65,

  PCC_FUNCTION_NIST_P256 = 64,
  PCC_FUNCTION_NIST_P384 = 65,
  PCC_FUNCTION_NIST_P521 = 66,
  PCC_FUNCTION_ED25519 = 72,
  PCC_FUNCTION_ED448 = 73,
  PCC_FUNCTION_X25519 = 80,
  PCC_FUNCTION_X448 = 81
};

enum kmxx_function_flags_e
{
  KM_ENCRYPT  = 0 << 7,
  KM_DECRYPT  = 1 << 7,

  KMF_LCFB_16 = 16 << 24,

  KMA_LPC     = 1 << 8,
  KMA_LAAD    = 1 << 9,
  KMA_HS      = 1 << 10,

  KLMD_PADDING_STATE = 1 << 8,
};

static ALWAYS_INLINE u128_t km_function_to_mask(enum kmxx_functions_e func)
{
  return (u128_t)1 << (127 - func);
}

static inline u128_t kimd_query(void)
{
  static u128_t function_codes = 0;
  static int initialized = 0;
  register unsigned long reg0 asm("0") = 0;
  register void *reg1 asm("1") = &function_codes;
  u128_t r1;

  if (initialized)
    return function_codes;

  asm volatile ("0: .insn rre,0xb93e << 16, 0, %[r1]\n\t"
		"   brc 1,0b\n\t"
		: [r1] "=a" (r1)
		: [reg0] "r" (reg0), [reg1] "r" (reg1)
		: "cc", "memory");

  initialized = 1;
  return function_codes;
}

static inline u128_t klmd_query(void)
{
  static u128_t function_codes = 0;
  static int initialized = 0;
  register unsigned long reg0 asm("0") = 0;
  register void *reg1 asm("1") = &function_codes;
  u128_t r1;

  if (initialized)
    return function_codes;

  asm volatile ("0: .insn rre,0xb93f << 16, 0, %[r1]\n\t"
		"   brc 1,0b\n\t"
		: [r1] "=a" (r1)
		: [reg0] "r" (reg0), [reg1] "r" (reg1)
		: "cc", "memory");

  initialized = 1;
  return function_codes;
}

static inline u128_t pcc_query(void)
{
  static u128_t function_codes = 0;
  static int initialized = 0;
  register unsigned long reg0 asm("0") = 0;
  register void *reg1 asm("1") = &function_codes;

  if (initialized)
    return function_codes;

  asm volatile ("0: .insn rre,0xb92c << 16, 0, 0\n\t"
		"   brc 1,0b\n\t"
		:
		: [reg0] "r" (reg0), [reg1] "r" (reg1)
		: "cc", "memory");

  initialized = 1;
  return function_codes;
}

static ALWAYS_INLINE void
kimd_execute(unsigned int func, void *param_block, const void *src,
	     size_t src_len)
{
  register unsigned long reg0 asm("0") = func;
  register byte *reg1 asm("1") = param_block;
  u128_t r1 = ((u128_t)(uintptr_t)src << 64) | (u64)src_len;

  asm volatile ("0: .insn rre,0xb93e << 16, 0, %[r1]\n\t"
		"   brc 1,0b\n\t"
		: [r1] "+a" (r1)
		: [func] "r" (reg0), [param_ptr] "r" (reg1)
		: "cc", "memory");
}

static ALWAYS_INLINE void
klmd_execute(unsigned int func, void *param_block, const void *src,
	     size_t src_len)
{
  register unsigned long reg0 asm("0") = func;
  register byte *reg1 asm("1") = param_block;
  u128_t r1 = ((u128_t)(uintptr_t)src << 64) | (u64)src_len;

  asm volatile ("0: .insn rre,0xb93f << 16, 0, %[r1]\n\t"
		"   brc 1,0b\n\t"
		: [func] "+r" (reg0), [r1] "+a" (r1)
		: [param_ptr] "r" (reg1)
		: "cc", "memory");
}

static ALWAYS_INLINE void
klmd_shake_execute(unsigned int func, void *param_block, void *dst,
		   size_t dst_len, const void *src, size_t src_len)
{
  register unsigned long reg0 asm("0") = func;
  register byte *reg1 asm("1") = param_block;
  u128_t r1 = ((u128_t)(uintptr_t)dst << 64) | (u64)dst_len;
  u128_t r2 = ((u128_t)(uintptr_t)src << 64) | (u64)src_len;

  asm volatile ("0: .insn rre,0xb93f << 16, %[r1], %[r2]\n\t"
		"   brc 1,0b\n\t"
		: [func] "+r" (reg0), [r1] "+a" (r1), [r2] "+a" (r2)
		: [param_ptr] "r" (reg1)
		: "cc", "memory");
}

static ALWAYS_INLINE unsigned int
pcc_scalar_multiply(unsigned int func, void *param_block)
{
  register unsigned long reg0 asm("0") = func;
  register byte *reg1 asm("1") = param_block;
  register unsigned long error = 0;

  asm volatile ("0: .insn rre,0xb92c << 16, 0, 0\n\t"
		"   brc 1,0b\n\t"
		"   brc 7,1f\n\t"
		"   j 2f\n\t"
		"1: lhi %[error], 1\n\t"
		"2:\n\t"
		: [func] "+r" (reg0), [error] "+r" (error)
		: [param_ptr] "r" (reg1)
		: "cc", "memory");

  return error;
}

#endif /* GCRY_ASM_INLINE_S390X_H */
