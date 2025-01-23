/* ec-inline.h - EC inline addition/substraction helpers
 * Copyright (C) 2021 Jussi Kivilinna <jussi.kivilinna@iki.fi>
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

#ifndef GCRY_EC_INLINE_H
#define GCRY_EC_INLINE_H

#include "mpi-internal.h"
#include "longlong.h"
#include "ec-context.h"
#include "../cipher/bithelp.h"
#include "../cipher/bufhelp.h"


#if BYTES_PER_MPI_LIMB == 8

/* 64-bit limb definitions for 64-bit architectures.  */

#define LIMBS_PER_LIMB64 1
#define LOAD64(x, pos) ((x)[pos])
#define STORE64(x, pos, v) ((x)[pos] = (mpi_limb_t)(v))
#define LIMB_TO64(v) ((mpi_limb_t)(v))
#define LIMB_FROM64(v) ((mpi_limb_t)(v))
#define HIBIT_LIMB64(v) ((mpi_limb_t)(v) >> (BITS_PER_MPI_LIMB - 1))
#define HI32_LIMB64(v) (u32)((mpi_limb_t)(v) >> (BITS_PER_MPI_LIMB - 32))
#define LO32_LIMB64(v) ((u32)(v))
#define LIMB64_C(hi, lo) (((mpi_limb_t)(u32)(hi) << 32) | (u32)(lo))
#define MASK_AND64(mask, val) ((mask) & (val))
#define LIMB_OR64(val1, val2) ((val1) | (val2))
#define STORE64_COND(x, pos, mask1, val1, mask2, val2) \
    ((x)[(pos)] = ((mask1) & (val1)) | ((mask2) & (val2)))

typedef mpi_limb_t mpi_limb64_t;

static inline u32
LOAD32(mpi_ptr_t x, unsigned int pos)
{
  unsigned int shr = (pos % 2) * 32;
  return (x[pos / 2] >> shr);
}

static inline mpi_limb64_t
LIMB64_HILO(u32 hi, u32 lo)
{
  mpi_limb64_t v = hi;
  return (v << 32) | lo;
}


/* x86-64 addition/subtraction helpers.  */
#if defined (__x86_64__) && defined(HAVE_CPU_ARCH_X86) && __GNUC__ >= 4

#define ADD3_LIMB64(A2, A1, A0, B2, B1, B0, C2, C1, C0) \
  __asm__ ("addq %8, %2\n" \
	   "adcq %7, %1\n" \
	   "adcq %6, %0\n" \
	   : "=r" (A2), \
	     "=&r" (A1), \
	     "=&r" (A0) \
	   : "0" ((mpi_limb_t)(B2)), \
	     "1" ((mpi_limb_t)(B1)), \
	     "2" ((mpi_limb_t)(B0)), \
	     "rme" ((mpi_limb_t)(C2)), \
	     "rme" ((mpi_limb_t)(C1)), \
	     "rme" ((mpi_limb_t)(C0)) \
	   : "cc")

#define SUB3_LIMB64(A3, A2, A1, A0, B2, B1, B0, C2, C1, C0) \
  __asm__ ("subq %8, %2\n" \
	   "sbbq %7, %1\n" \
	   "sbbq %6, %0\n" \
	   : "=r" (A2), \
	     "=&r" (A1), \
	     "=&r" (A0) \
	   : "0" ((mpi_limb_t)(B2)), \
	     "1" ((mpi_limb_t)(B1)), \
	     "2" ((mpi_limb_t)(B0)), \
	     "rme" ((mpi_limb_t)(C2)), \
	     "rme" ((mpi_limb_t)(C1)), \
	     "rme" ((mpi_limb_t)(C0)) \
	   : "cc")

#define ADD4_LIMB64(A3, A2, A1, A0, B3, B2, B1, B0, C3, C2, C1, C0) \
  __asm__ ("addq %11, %3\n" \
	   "adcq %10, %2\n" \
	   "adcq %9, %1\n" \
	   "adcq %8, %0\n" \
	   : "=r" (A3), \
	     "=&r" (A2), \
	     "=&r" (A1), \
	     "=&r" (A0) \
	   : "0" ((mpi_limb_t)(B3)), \
	     "1" ((mpi_limb_t)(B2)), \
	     "2" ((mpi_limb_t)(B1)), \
	     "3" ((mpi_limb_t)(B0)), \
	     "rme" ((mpi_limb_t)(C3)), \
	     "rme" ((mpi_limb_t)(C2)), \
	     "rme" ((mpi_limb_t)(C1)), \
	     "rme" ((mpi_limb_t)(C0)) \
	   : "cc")

#define SUB4_LIMB64(A3, A2, A1, A0, B3, B2, B1, B0, C3, C2, C1, C0) \
  __asm__ ("subq %11, %3\n" \
	   "sbbq %10, %2\n" \
	   "sbbq %9, %1\n" \
	   "sbbq %8, %0\n" \
	   : "=r" (A3), \
	     "=&r" (A2), \
	     "=&r" (A1), \
	     "=&r" (A0) \
	   : "0" ((mpi_limb_t)(B3)), \
	     "1" ((mpi_limb_t)(B2)), \
	     "2" ((mpi_limb_t)(B1)), \
	     "3" ((mpi_limb_t)(B0)), \
	     "rme" ((mpi_limb_t)(C3)), \
	     "rme" ((mpi_limb_t)(C2)), \
	     "rme" ((mpi_limb_t)(C1)), \
	     "rme" ((mpi_limb_t)(C0)) \
	   : "cc")

#define ADD5_LIMB64(A4, A3, A2, A1, A0, B4, B3, B2, B1, B0, \
                    C4, C3, C2, C1, C0) \
  __asm__ ("addq %14, %4\n" \
	   "adcq %13, %3\n" \
	   "adcq %12, %2\n" \
	   "adcq %11, %1\n" \
	   "adcq %10, %0\n" \
	   : "=r" (A4), \
	     "=&r" (A3), \
	     "=&r" (A2), \
	     "=&r" (A1), \
	     "=&r" (A0) \
	   : "0" ((mpi_limb_t)(B4)), \
	     "1" ((mpi_limb_t)(B3)), \
	     "2" ((mpi_limb_t)(B2)), \
	     "3" ((mpi_limb_t)(B1)), \
	     "4" ((mpi_limb_t)(B0)), \
	     "rme" ((mpi_limb_t)(C4)), \
	     "rme" ((mpi_limb_t)(C3)), \
	     "rme" ((mpi_limb_t)(C2)), \
	     "rme" ((mpi_limb_t)(C1)), \
	     "rme" ((mpi_limb_t)(C0)) \
	   : "cc")

#define SUB5_LIMB64(A4, A3, A2, A1, A0, B4, B3, B2, B1, B0, \
                    C4, C3, C2, C1, C0) \
  __asm__ ("subq %14, %4\n" \
	   "sbbq %13, %3\n" \
	   "sbbq %12, %2\n" \
	   "sbbq %11, %1\n" \
	   "sbbq %10, %0\n" \
	   : "=r" (A4), \
	     "=&r" (A3), \
	     "=&r" (A2), \
	     "=&r" (A1), \
	     "=&r" (A0) \
	   : "0" ((mpi_limb_t)(B4)), \
	     "1" ((mpi_limb_t)(B3)), \
	     "2" ((mpi_limb_t)(B2)), \
	     "3" ((mpi_limb_t)(B1)), \
	     "4" ((mpi_limb_t)(B0)), \
	     "rme" ((mpi_limb_t)(C4)), \
	     "rme" ((mpi_limb_t)(C3)), \
	     "rme" ((mpi_limb_t)(C2)), \
	     "rme" ((mpi_limb_t)(C1)), \
	     "rme" ((mpi_limb_t)(C0)) \
	   : "cc")

#endif /* __x86_64__ */


/* ARM AArch64 addition/subtraction helpers.  */
#if defined (__aarch64__) && defined(HAVE_CPU_ARCH_ARM) && __GNUC__ >= 4

#define ADD3_LIMB64(A2, A1, A0, B2, B1, B0, C2, C1, C0) \
  __asm__ ("adds %2, %5, %8\n" \
	   "adcs %1, %4, %7\n" \
	   "adc  %0, %3, %6\n" \
	   : "=r" (A2), \
	     "=&r" (A1), \
	     "=&r" (A0) \
	   : "r" ((mpi_limb_t)(B2)), \
	     "r" ((mpi_limb_t)(B1)), \
	     "r" ((mpi_limb_t)(B0)), \
	     "r" ((mpi_limb_t)(C2)), \
	     "r" ((mpi_limb_t)(C1)), \
	     "r" ((mpi_limb_t)(C0)) \
	   : "cc")

#define SUB3_LIMB64(A2, A1, A0, B2, B1, B0, C2, C1, C0) \
  __asm__ ("subs %2, %5, %8\n" \
	   "sbcs %1, %4, %7\n" \
	   "sbc  %0, %3, %6\n" \
	   : "=r" (A2), \
	     "=&r" (A1), \
	     "=&r" (A0) \
	   : "r" ((mpi_limb_t)(B2)), \
	     "r" ((mpi_limb_t)(B1)), \
	     "r" ((mpi_limb_t)(B0)), \
	     "r" ((mpi_limb_t)(C2)), \
	     "r" ((mpi_limb_t)(C1)), \
	     "r" ((mpi_limb_t)(C0)) \
	   : "cc")

#define ADD4_LIMB64(A3, A2, A1, A0, B3, B2, B1, B0, C3, C2, C1, C0) \
  __asm__ ("adds %3, %7, %11\n" \
	   "adcs %2, %6, %10\n" \
	   "adcs %1, %5, %9\n" \
	   "adc  %0, %4, %8\n" \
	   : "=r" (A3), \
	     "=&r" (A2), \
	     "=&r" (A1), \
	     "=&r" (A0) \
	   : "r" ((mpi_limb_t)(B3)), \
	     "r" ((mpi_limb_t)(B2)), \
	     "r" ((mpi_limb_t)(B1)), \
	     "r" ((mpi_limb_t)(B0)), \
	     "r" ((mpi_limb_t)(C3)), \
	     "r" ((mpi_limb_t)(C2)), \
	     "r" ((mpi_limb_t)(C1)), \
	     "r" ((mpi_limb_t)(C0)) \
	   : "cc")

#define SUB4_LIMB64(A3, A2, A1, A0, B3, B2, B1, B0, C3, C2, C1, C0) \
  __asm__ ("subs %3, %7, %11\n" \
	   "sbcs %2, %6, %10\n" \
	   "sbcs %1, %5, %9\n" \
	   "sbc  %0, %4, %8\n" \
	   : "=r" (A3), \
	     "=&r" (A2), \
	     "=&r" (A1), \
	     "=&r" (A0) \
	   : "r" ((mpi_limb_t)(B3)), \
	     "r" ((mpi_limb_t)(B2)), \
	     "r" ((mpi_limb_t)(B1)), \
	     "r" ((mpi_limb_t)(B0)), \
	     "r" ((mpi_limb_t)(C3)), \
	     "r" ((mpi_limb_t)(C2)), \
	     "r" ((mpi_limb_t)(C1)), \
	     "r" ((mpi_limb_t)(C0)) \
	   : "cc")

#define ADD5_LIMB64(A4, A3, A2, A1, A0, B4, B3, B2, B1, B0, \
                    C4, C3, C2, C1, C0) \
  __asm__ ("adds %4, %9, %14\n" \
	   "adcs %3, %8, %13\n" \
	   "adcs %2, %7, %12\n" \
	   "adcs %1, %6, %11\n" \
	   "adc  %0, %5, %10\n" \
	   : "=r" (A4), \
	     "=&r" (A3), \
	     "=&r" (A2), \
	     "=&r" (A1), \
	     "=&r" (A0) \
	   : "r" ((mpi_limb_t)(B4)), \
	     "r" ((mpi_limb_t)(B3)), \
	     "r" ((mpi_limb_t)(B2)), \
	     "r" ((mpi_limb_t)(B1)), \
	     "r" ((mpi_limb_t)(B0)), \
	     "r" ((mpi_limb_t)(C4)), \
	     "r" ((mpi_limb_t)(C3)), \
	     "r" ((mpi_limb_t)(C2)), \
	     "r" ((mpi_limb_t)(C1)), \
	     "r" ((mpi_limb_t)(C0)) \
	   : "cc")

#define SUB5_LIMB64(A4, A3, A2, A1, A0, B4, B3, B2, B1, B0, \
                    C4, C3, C2, C1, C0) \
  __asm__ ("subs %4, %9, %14\n" \
	   "sbcs %3, %8, %13\n" \
	   "sbcs %2, %7, %12\n" \
	   "sbcs %1, %6, %11\n" \
	   "sbc  %0, %5, %10\n" \
	   : "=r" (A4), \
	     "=&r" (A3), \
	     "=&r" (A2), \
	     "=&r" (A1), \
	     "=&r" (A0) \
	   : "r" ((mpi_limb_t)(B4)), \
	     "r" ((mpi_limb_t)(B3)), \
	     "r" ((mpi_limb_t)(B2)), \
	     "r" ((mpi_limb_t)(B1)), \
	     "r" ((mpi_limb_t)(B0)), \
	     "r" ((mpi_limb_t)(C4)), \
	     "r" ((mpi_limb_t)(C3)), \
	     "r" ((mpi_limb_t)(C2)), \
	     "r" ((mpi_limb_t)(C1)), \
	     "r" ((mpi_limb_t)(C0)) \
	   : "cc")

#endif /* __aarch64__ */


/* PowerPC64 addition/subtraction helpers.  */
#if defined (__powerpc__) && defined(HAVE_CPU_ARCH_PPC) && __GNUC__ >= 4

#define ADD3_LIMB64(A2, A1, A0, B2, B1, B0, C2, C1, C0) \
  __asm__ ("addc %2, %8, %5\n" \
	   "adde %1, %7, %4\n" \
	   "adde %0, %6, %3\n" \
	   : "=r" (A2), \
	     "=&r" (A1), \
	     "=&r" (A0) \
	   : "r" ((mpi_limb_t)(B2)), \
	     "r" ((mpi_limb_t)(B1)), \
	     "r" ((mpi_limb_t)(B0)), \
	     "r" ((mpi_limb_t)(C2)), \
	     "r" ((mpi_limb_t)(C1)), \
	     "r" ((mpi_limb_t)(C0)) \
	   : "cc", "r0")

#define SUB3_LIMB64(A2, A1, A0, B2, B1, B0, C2, C1, C0) \
  __asm__ ("subfc %2, %8, %5\n" \
	   "subfe %1, %7, %4\n" \
	   "subfe %0, %6, %3\n" \
	   : "=r" (A2), \
	     "=&r" (A1), \
	     "=&r" (A0) \
	   : "r" ((mpi_limb_t)(B2)), \
	     "r" ((mpi_limb_t)(B1)), \
	     "r" ((mpi_limb_t)(B0)), \
	     "r" ((mpi_limb_t)(C2)), \
	     "r" ((mpi_limb_t)(C1)), \
	     "r" ((mpi_limb_t)(C0)) \
	   : "cc", "r0")

#define ADD4_LIMB64(A3, A2, A1, A0, B3, B2, B1, B0, C3, C2, C1, C0) \
  __asm__ ("addc %3, %11, %7\n" \
	   "adde %2, %10, %6\n" \
	   "adde %1, %9, %5\n" \
	   "adde %0, %8, %4\n" \
	   : "=r" (A3), \
	     "=&r" (A2), \
	     "=&r" (A1), \
	     "=&r" (A0) \
	   : "r" ((mpi_limb_t)(B3)), \
	     "r" ((mpi_limb_t)(B2)), \
	     "r" ((mpi_limb_t)(B1)), \
	     "r" ((mpi_limb_t)(B0)), \
	     "r" ((mpi_limb_t)(C3)), \
	     "r" ((mpi_limb_t)(C2)), \
	     "r" ((mpi_limb_t)(C1)), \
	     "r" ((mpi_limb_t)(C0)) \
	   : "cc")

#define SUB4_LIMB64(A3, A2, A1, A0, B3, B2, B1, B0, C3, C2, C1, C0) \
  __asm__ ("subfc %3, %11, %7\n" \
	   "subfe %2, %10, %6\n" \
	   "subfe %1, %9, %5\n" \
	   "subfe %0, %8, %4\n" \
	   : "=r" (A3), \
	     "=&r" (A2), \
	     "=&r" (A1), \
	     "=&r" (A0) \
	   : "r" ((mpi_limb_t)(B3)), \
	     "r" ((mpi_limb_t)(B2)), \
	     "r" ((mpi_limb_t)(B1)), \
	     "r" ((mpi_limb_t)(B0)), \
	     "r" ((mpi_limb_t)(C3)), \
	     "r" ((mpi_limb_t)(C2)), \
	     "r" ((mpi_limb_t)(C1)), \
	     "r" ((mpi_limb_t)(C0)) \
	   : "cc")

#define ADD5_LIMB64(A4, A3, A2, A1, A0, B4, B3, B2, B1, B0, \
	                    C4, C3, C2, C1, C0) \
  __asm__ ("addc %4, %14, %9\n" \
	   "adde %3, %13, %8\n" \
	   "adde %2, %12, %7\n" \
	   "adde %1, %11, %6\n" \
	   "adde %0, %10, %5\n" \
	   : "=r" (A4), \
	     "=&r" (A3), \
	     "=&r" (A2), \
	     "=&r" (A1), \
	     "=&r" (A0) \
	   : "r" ((mpi_limb_t)(B4)), \
	     "r" ((mpi_limb_t)(B3)), \
	     "r" ((mpi_limb_t)(B2)), \
	     "r" ((mpi_limb_t)(B1)), \
	     "r" ((mpi_limb_t)(B0)), \
	     "r" ((mpi_limb_t)(C4)), \
	     "r" ((mpi_limb_t)(C3)), \
	     "r" ((mpi_limb_t)(C2)), \
	     "r" ((mpi_limb_t)(C1)), \
	     "r" ((mpi_limb_t)(C0)) \
	   : "cc")

#define SUB5_LIMB64(A4, A3, A2, A1, A0, B4, B3, B2, B1, B0, \
	                    C4, C3, C2, C1, C0) \
  __asm__ ("subfc %4, %14, %9\n" \
	   "subfe %3, %13, %8\n" \
	   "subfe %2, %12, %7\n" \
	   "subfe %1, %11, %6\n" \
	   "subfe %0, %10, %5\n" \
	   : "=r" (A4), \
	     "=&r" (A3), \
	     "=&r" (A2), \
	     "=&r" (A1), \
	     "=&r" (A0) \
	   : "r" ((mpi_limb_t)(B4)), \
	     "r" ((mpi_limb_t)(B3)), \
	     "r" ((mpi_limb_t)(B2)), \
	     "r" ((mpi_limb_t)(B1)), \
	     "r" ((mpi_limb_t)(B0)), \
	     "r" ((mpi_limb_t)(C4)), \
	     "r" ((mpi_limb_t)(C3)), \
	     "r" ((mpi_limb_t)(C2)), \
	     "r" ((mpi_limb_t)(C1)), \
	     "r" ((mpi_limb_t)(C0)) \
	   : "cc")

#endif /* __powerpc__ */


/* s390x/zSeries addition/subtraction helpers.  */
#if defined (__s390x__) && defined(HAVE_CPU_ARCH_S390X) && __GNUC__ >= 4

#define ADD3_LIMB64(A2, A1, A0, B2, B1, B0, C2, C1, C0) \
  __asm__ ("algr %2, %8\n" \
	   "alcgr %1, %7\n" \
	   "alcgr %0, %6\n" \
	   : "=r" (A2), \
	     "=&r" (A1), \
	     "=&r" (A0) \
	   : "0" ((mpi_limb_t)(B2)), \
	     "1" ((mpi_limb_t)(B1)), \
	     "2" ((mpi_limb_t)(B0)), \
	     "r" ((mpi_limb_t)(C2)), \
	     "r" ((mpi_limb_t)(C1)), \
	     "r" ((mpi_limb_t)(C0)) \
	   : "cc")

#define SUB3_LIMB64(A3, A2, A1, A0, B2, B1, B0, C2, C1, C0) \
  __asm__ ("slgr %2, %8\n" \
	   "slbgr %1, %7\n" \
	   "slbgr %0, %6\n" \
	   : "=r" (A2), \
	     "=&r" (A1), \
	     "=&r" (A0) \
	   : "0" ((mpi_limb_t)(B2)), \
	     "1" ((mpi_limb_t)(B1)), \
	     "2" ((mpi_limb_t)(B0)), \
	     "r" ((mpi_limb_t)(C2)), \
	     "r" ((mpi_limb_t)(C1)), \
	     "r" ((mpi_limb_t)(C0)) \
	   : "cc")

#define ADD4_LIMB64(A3, A2, A1, A0, B3, B2, B1, B0, C3, C2, C1, C0) \
  __asm__ ("algr %3, %11\n" \
	   "alcgr %2, %10\n" \
	   "alcgr %1, %9\n" \
	   "alcgr %0, %8\n" \
	   : "=r" (A3), \
	     "=&r" (A2), \
	     "=&r" (A1), \
	     "=&r" (A0) \
	   : "0" ((mpi_limb_t)(B3)), \
	     "1" ((mpi_limb_t)(B2)), \
	     "2" ((mpi_limb_t)(B1)), \
	     "3" ((mpi_limb_t)(B0)), \
	     "r" ((mpi_limb_t)(C3)), \
	     "r" ((mpi_limb_t)(C2)), \
	     "r" ((mpi_limb_t)(C1)), \
	     "r" ((mpi_limb_t)(C0)) \
	   : "cc")

#define SUB4_LIMB64(A3, A2, A1, A0, B3, B2, B1, B0, C3, C2, C1, C0) \
  __asm__ ("slgr %3, %11\n" \
	   "slbgr %2, %10\n" \
	   "slbgr %1, %9\n" \
	   "slbgr %0, %8\n" \
	   : "=r" (A3), \
	     "=&r" (A2), \
	     "=&r" (A1), \
	     "=&r" (A0) \
	   : "0" ((mpi_limb_t)(B3)), \
	     "1" ((mpi_limb_t)(B2)), \
	     "2" ((mpi_limb_t)(B1)), \
	     "3" ((mpi_limb_t)(B0)), \
	     "r" ((mpi_limb_t)(C3)), \
	     "r" ((mpi_limb_t)(C2)), \
	     "r" ((mpi_limb_t)(C1)), \
	     "r" ((mpi_limb_t)(C0)) \
	   : "cc")

#define ADD5_LIMB64(A4, A3, A2, A1, A0, B4, B3, B2, B1, B0, \
                    C4, C3, C2, C1, C0) \
  __asm__ ("algr %4, %14\n" \
	   "alcgr %3, %13\n" \
	   "alcgr %2, %12\n" \
	   "alcgr %1, %11\n" \
	   "alcgr %0, %10\n" \
	   : "=r" (A4), \
	     "=&r" (A3), \
	     "=&r" (A2), \
	     "=&r" (A1), \
	     "=&r" (A0) \
	   : "0" ((mpi_limb_t)(B4)), \
	     "1" ((mpi_limb_t)(B3)), \
	     "2" ((mpi_limb_t)(B2)), \
	     "3" ((mpi_limb_t)(B1)), \
	     "4" ((mpi_limb_t)(B0)), \
	     "r" ((mpi_limb_t)(C4)), \
	     "r" ((mpi_limb_t)(C3)), \
	     "r" ((mpi_limb_t)(C2)), \
	     "r" ((mpi_limb_t)(C1)), \
	     "r" ((mpi_limb_t)(C0)) \
	   : "cc")

#define SUB5_LIMB64(A4, A3, A2, A1, A0, B4, B3, B2, B1, B0, \
                    C4, C3, C2, C1, C0) \
  __asm__ ("slgr %4, %14\n" \
	   "slbgr %3, %13\n" \
	   "slbgr %2, %12\n" \
	   "slbgr %1, %11\n" \
	   "slbgr %0, %10\n" \
	   : "=r" (A4), \
	     "=&r" (A3), \
	     "=&r" (A2), \
	     "=&r" (A1), \
	     "=&r" (A0) \
	   : "0" ((mpi_limb_t)(B4)), \
	     "1" ((mpi_limb_t)(B3)), \
	     "2" ((mpi_limb_t)(B2)), \
	     "3" ((mpi_limb_t)(B1)), \
	     "4" ((mpi_limb_t)(B0)), \
	     "r" ((mpi_limb_t)(C4)), \
	     "r" ((mpi_limb_t)(C3)), \
	     "r" ((mpi_limb_t)(C2)), \
	     "r" ((mpi_limb_t)(C1)), \
	     "r" ((mpi_limb_t)(C0)) \
	   : "cc")

#endif /* __s390x__ */


/* Common 64-bit arch addition/subtraction macros.  */

#define ADD2_LIMB64(A1, A0, B1, B0, C1, C0) \
  add_ssaaaa(A1, A0, B1, B0, C1, C0)

#define SUB2_LIMB64(A1, A0, B1, B0, C1, C0) \
  sub_ddmmss(A1, A0, B1, B0, C1, C0)

#endif /* BYTES_PER_MPI_LIMB == 8 */


#if BYTES_PER_MPI_LIMB == 4

/* 64-bit limb definitions for 32-bit architectures.  */

#define LIMBS_PER_LIMB64 2
#define LIMB_FROM64(v) ((v).lo)
#define HIBIT_LIMB64(v) ((v).hi >> (BITS_PER_MPI_LIMB - 1))
#define HI32_LIMB64(v) ((v).hi)
#define LO32_LIMB64(v) ((v).lo)
#define LOAD32(x, pos) ((x)[pos])
#define LIMB64_C(hi, lo) { (lo), (hi) }

typedef struct
{
  mpi_limb_t lo;
  mpi_limb_t hi;
} mpi_limb64_t;

static inline mpi_limb64_t
LOAD64(const mpi_ptr_t x, unsigned int pos)
{
  mpi_limb64_t v;
  v.lo = x[pos * 2 + 0];
  v.hi = x[pos * 2 + 1];
  return v;
}

static inline void
STORE64(mpi_ptr_t x, unsigned int pos, mpi_limb64_t v)
{
  x[pos * 2 + 0] = v.lo;
  x[pos * 2 + 1] = v.hi;
}

static inline mpi_limb64_t
MASK_AND64(mpi_limb_t mask, mpi_limb64_t val)
{
  val.lo &= mask;
  val.hi &= mask;
  return val;
}

static inline mpi_limb64_t
LIMB_OR64(mpi_limb64_t val1, mpi_limb64_t val2)
{
  val1.lo |= val2.lo;
  val1.hi |= val2.hi;
  return val1;
}

static inline void
STORE64_COND(mpi_ptr_t x, unsigned int pos, mpi_limb_t mask1,
	     mpi_limb64_t val1, mpi_limb_t mask2, mpi_limb64_t val2)
{
  x[pos * 2 + 0] = (mask1 & val1.lo) | (mask2 & val2.lo);
  x[pos * 2 + 1] = (mask1 & val1.hi) | (mask2 & val2.hi);
}

static inline mpi_limb64_t
LIMB_TO64(mpi_limb_t x)
{
  mpi_limb64_t v;
  v.lo = x;
  v.hi = 0;
  return v;
}

static inline mpi_limb64_t
LIMB64_HILO(mpi_limb_t hi, mpi_limb_t lo)
{
  mpi_limb64_t v;
  v.lo = lo;
  v.hi = hi;
  return v;
}


/* i386 addition/subtraction helpers.  */
#if defined (__i386__) && defined(HAVE_CPU_ARCH_X86) && __GNUC__ >= 4

#define ADD4_LIMB32(a3, a2, a1, a0, b3, b2, b1, b0, c3, c2, c1, c0) \
  __asm__ ("addl %11, %3\n" \
	   "adcl %10, %2\n" \
	   "adcl %9, %1\n" \
	   "adcl %8, %0\n" \
	   : "=r" (a3), \
	     "=&r" (a2), \
	     "=&r" (a1), \
	     "=&r" (a0) \
	   : "0" ((mpi_limb_t)(b3)), \
	     "1" ((mpi_limb_t)(b2)), \
	     "2" ((mpi_limb_t)(b1)), \
	     "3" ((mpi_limb_t)(b0)), \
	     "g" ((mpi_limb_t)(c3)), \
	     "g" ((mpi_limb_t)(c2)), \
	     "g" ((mpi_limb_t)(c1)), \
	     "g" ((mpi_limb_t)(c0)) \
	   : "cc")

#define ADD6_LIMB32(a5, a4, a3, a2, a1, a0, b5, b4, b3, b2, b1, b0, \
		    c5, c4, c3, c2, c1, c0) do { \
    mpi_limb_t __carry6_32; \
    __asm__ ("addl %10, %3\n" \
	     "adcl %9, %2\n" \
	     "adcl %8, %1\n" \
	     "sbbl %0, %0\n" \
	     : "=r" (__carry6_32), \
	       "=&r" (a2), \
	       "=&r" (a1), \
	       "=&r" (a0) \
	     : "0" ((mpi_limb_t)(0)), \
	       "1" ((mpi_limb_t)(b2)), \
	       "2" ((mpi_limb_t)(b1)), \
	       "3" ((mpi_limb_t)(b0)), \
	       "g" ((mpi_limb_t)(c2)), \
	       "g" ((mpi_limb_t)(c1)), \
	       "g" ((mpi_limb_t)(c0)) \
	     : "cc"); \
    __asm__ ("addl $1, %3\n" \
	     "adcl %10, %2\n" \
	     "adcl %9, %1\n" \
	     "adcl %8, %0\n" \
	     : "=r" (a5), \
	       "=&r" (a4), \
	       "=&r" (a3), \
	       "=&r" (__carry6_32) \
	     : "0" ((mpi_limb_t)(b5)), \
	       "1" ((mpi_limb_t)(b4)), \
	       "2" ((mpi_limb_t)(b3)), \
	       "3" ((mpi_limb_t)(__carry6_32)), \
	       "g" ((mpi_limb_t)(c5)), \
	       "g" ((mpi_limb_t)(c4)), \
	       "g" ((mpi_limb_t)(c3)) \
	   : "cc"); \
  } while (0)

#define SUB4_LIMB32(a3, a2, a1, a0, b3, b2, b1, b0, c3, c2, c1, c0) \
  __asm__ ("subl %11, %3\n" \
	   "sbbl %10, %2\n" \
	   "sbbl %9, %1\n" \
	   "sbbl %8, %0\n" \
	   : "=r" (a3), \
	     "=&r" (a2), \
	     "=&r" (a1), \
	     "=&r" (a0) \
	   : "0" ((mpi_limb_t)(b3)), \
	     "1" ((mpi_limb_t)(b2)), \
	     "2" ((mpi_limb_t)(b1)), \
	     "3" ((mpi_limb_t)(b0)), \
	     "g" ((mpi_limb_t)(c3)), \
	     "g" ((mpi_limb_t)(c2)), \
	     "g" ((mpi_limb_t)(c1)), \
	     "g" ((mpi_limb_t)(c0)) \
	   : "cc")

#define SUB6_LIMB32(a5, a4, a3, a2, a1, a0, b5, b4, b3, b2, b1, b0, \
		    c5, c4, c3, c2, c1, c0) do { \
    mpi_limb_t __borrow6_32; \
    __asm__ ("subl %10, %3\n" \
	     "sbbl %9, %2\n" \
	     "sbbl %8, %1\n" \
	     "sbbl %0, %0\n" \
	     : "=r" (__borrow6_32), \
	       "=&r" (a2), \
	       "=&r" (a1), \
	       "=&r" (a0) \
	     : "0" ((mpi_limb_t)(0)), \
	       "1" ((mpi_limb_t)(b2)), \
	       "2" ((mpi_limb_t)(b1)), \
	       "3" ((mpi_limb_t)(b0)), \
	       "g" ((mpi_limb_t)(c2)), \
	       "g" ((mpi_limb_t)(c1)), \
	       "g" ((mpi_limb_t)(c0)) \
	     : "cc"); \
    __asm__ ("addl $1, %3\n" \
	     "sbbl %10, %2\n" \
	     "sbbl %9, %1\n" \
	     "sbbl %8, %0\n" \
	     : "=r" (a5), \
	       "=&r" (a4), \
	       "=&r" (a3), \
	       "=&r" (__borrow6_32) \
	     : "0" ((mpi_limb_t)(b5)), \
	       "1" ((mpi_limb_t)(b4)), \
	       "2" ((mpi_limb_t)(b3)), \
	       "3" ((mpi_limb_t)(__borrow6_32)), \
	       "g" ((mpi_limb_t)(c5)), \
	       "g" ((mpi_limb_t)(c4)), \
	       "g" ((mpi_limb_t)(c3)) \
	   : "cc"); \
  } while (0)

#endif /* __i386__ */


/* ARM addition/subtraction helpers.  */
#ifdef HAVE_COMPATIBLE_GCC_ARM_PLATFORM_AS

#define ADD4_LIMB32(A3, A2, A1, A0, B3, B2, B1, B0, C3, C2, C1, C0) \
  __asm__ ("adds %3, %7, %11\n" \
	   "adcs %2, %6, %10\n" \
	   "adcs %1, %5, %9\n" \
	   "adc  %0, %4, %8\n" \
	   : "=r" (A3), \
	     "=&r" (A2), \
	     "=&r" (A1), \
	     "=&r" (A0) \
	   : "r" ((mpi_limb_t)(B3)), \
	     "r" ((mpi_limb_t)(B2)), \
	     "r" ((mpi_limb_t)(B1)), \
	     "r" ((mpi_limb_t)(B0)), \
	     "Ir" ((mpi_limb_t)(C3)), \
	     "Ir" ((mpi_limb_t)(C2)), \
	     "Ir" ((mpi_limb_t)(C1)), \
	     "Ir" ((mpi_limb_t)(C0)) \
	   : "cc")

#define ADD6_LIMB32(A5, A4, A3, A2, A1, A0, B5, B4, B3, B2, B1, B0, \
		    C5, C4, C3, C2, C1, C0) do { \
    mpi_limb_t __carry6_32; \
    __asm__ ("adds %3, %7, %10\n" \
	     "adcs %2, %6, %9\n" \
	     "adcs %1, %5, %8\n" \
	     "adc  %0, %4, %4\n" \
	     : "=r" (__carry6_32), \
	       "=&r" (A2), \
	       "=&r" (A1), \
	       "=&r" (A0) \
	     : "r" ((mpi_limb_t)(0)), \
	       "r" ((mpi_limb_t)(B2)), \
	       "r" ((mpi_limb_t)(B1)), \
	       "r" ((mpi_limb_t)(B0)), \
	       "Ir" ((mpi_limb_t)(C2)), \
	       "Ir" ((mpi_limb_t)(C1)), \
	       "Ir" ((mpi_limb_t)(C0)) \
	     : "cc"); \
    ADD4_LIMB32(A5, A4, A3, __carry6_32, B5, B4, B3, __carry6_32, \
		C5, C4, C3, 0xffffffffU); \
  } while (0)

#define SUB4_LIMB32(A3, A2, A1, A0, B3, B2, B1, B0, C3, C2, C1, C0) \
  __asm__ ("subs %3, %7, %11\n" \
	   "sbcs %2, %6, %10\n" \
	   "sbcs %1, %5, %9\n" \
	   "sbc  %0, %4, %8\n" \
	   : "=r" (A3), \
	     "=&r" (A2), \
	     "=&r" (A1), \
	     "=&r" (A0) \
	   : "r" ((mpi_limb_t)(B3)), \
	     "r" ((mpi_limb_t)(B2)), \
	     "r" ((mpi_limb_t)(B1)), \
	     "r" ((mpi_limb_t)(B0)), \
	     "Ir" ((mpi_limb_t)(C3)), \
	     "Ir" ((mpi_limb_t)(C2)), \
	     "Ir" ((mpi_limb_t)(C1)), \
	     "Ir" ((mpi_limb_t)(C0)) \
	   : "cc")


#define SUB6_LIMB32(A5, A4, A3, A2, A1, A0, B5, B4, B3, B2, B1, B0, \
		    C5, C4, C3, C2, C1, C0) do { \
    mpi_limb_t __borrow6_32; \
    __asm__ ("subs %3, %7, %10\n" \
	     "sbcs %2, %6, %9\n" \
	     "sbcs %1, %5, %8\n" \
	     "sbc  %0, %4, %4\n" \
	     : "=r" (__borrow6_32), \
	       "=&r" (A2), \
	       "=&r" (A1), \
	       "=&r" (A0) \
	     : "r" ((mpi_limb_t)(0)), \
	       "r" ((mpi_limb_t)(B2)), \
	       "r" ((mpi_limb_t)(B1)), \
	       "r" ((mpi_limb_t)(B0)), \
	       "Ir" ((mpi_limb_t)(C2)), \
	       "Ir" ((mpi_limb_t)(C1)), \
	       "Ir" ((mpi_limb_t)(C0)) \
	     : "cc"); \
    SUB4_LIMB32(A5, A4, A3, __borrow6_32, B5, B4, B3, 0, \
		C5, C4, C3, -__borrow6_32); \
  } while (0)

#endif /* HAVE_COMPATIBLE_GCC_ARM_PLATFORM_AS */


/* Common 32-bit arch addition/subtraction macros.  */

#if defined(ADD4_LIMB32)
/* A[0..1] = B[0..1] + C[0..1] */
#define ADD2_LIMB64(A1, A0, B1, B0, C1, C0) \
	ADD4_LIMB32(A1.hi, A1.lo, A0.hi, A0.lo, \
		    B1.hi, B1.lo, B0.hi, B0.lo, \
		    C1.hi, C1.lo, C0.hi, C0.lo)
#else
/* A[0..1] = B[0..1] + C[0..1] */
#define ADD2_LIMB64(A1, A0, B1, B0, C1, C0) do { \
    mpi_limb_t __carry2_0, __carry2_1; \
    add_ssaaaa(__carry2_0, A0.lo, 0, B0.lo, 0, C0.lo); \
    add_ssaaaa(__carry2_1, A0.hi, 0, B0.hi, 0, C0.hi); \
    add_ssaaaa(__carry2_1, A0.hi, __carry2_1, A0.hi, 0, __carry2_0); \
    add_ssaaaa(A1.hi, A1.lo, B1.hi, B1.lo, C1.hi, C1.lo); \
    add_ssaaaa(A1.hi, A1.lo, A1.hi, A1.lo, 0, __carry2_1); \
  } while (0)
#endif

#if defined(ADD6_LIMB32)
/* A[0..2] = B[0..2] + C[0..2] */
#define ADD3_LIMB64(A2, A1, A0, B2, B1, B0, C2, C1, C0) \
	ADD6_LIMB32(A2.hi, A2.lo, A1.hi, A1.lo, A0.hi, A0.lo, \
		    B2.hi, B2.lo, B1.hi, B1.lo, B0.hi, B0.lo, \
		    C2.hi, C2.lo, C1.hi, C1.lo, C0.hi, C0.lo)
#endif

#if defined(ADD6_LIMB32)
/* A[0..3] = B[0..3] + C[0..3] */
#define ADD4_LIMB64(A3, A2, A1, A0, B3, B2, B1, B0, C3, C2, C1, C0) do { \
    mpi_limb_t __carry4; \
    ADD6_LIMB32(__carry4, A2.lo, A1.hi, A1.lo, A0.hi, A0.lo, \
		0, B2.lo, B1.hi, B1.lo, B0.hi, B0.lo, \
		0, C2.lo, C1.hi, C1.lo, C0.hi, C0.lo); \
    ADD4_LIMB32(A3.hi, A3.lo, A2.hi, __carry4, \
		B3.hi, B3.lo, B2.hi, __carry4, \
		C3.hi, C3.lo, C2.hi, 0xffffffffU); \
  } while (0)
#endif

#if defined(SUB4_LIMB32)
/* A[0..1] = B[0..1] - C[0..1] */
#define SUB2_LIMB64(A1, A0, B1, B0, C1, C0) \
	SUB4_LIMB32(A1.hi, A1.lo, A0.hi, A0.lo, \
		    B1.hi, B1.lo, B0.hi, B0.lo, \
		    C1.hi, C1.lo, C0.hi, C0.lo)
#else
/* A[0..1] = B[0..1] - C[0..1] */
#define SUB2_LIMB64(A1, A0, B1, B0, C1, C0) do { \
    mpi_limb_t __borrow2_0, __borrow2_1; \
    sub_ddmmss(__borrow2_0, A0.lo, 0, B0.lo, 0, C0.lo); \
    sub_ddmmss(__borrow2_1, A0.hi, 0, B0.hi, 0, C0.hi); \
    sub_ddmmss(__borrow2_1, A0.hi, __borrow2_1, A0.hi, 0, -__borrow2_0); \
    sub_ddmmss(A1.hi, A1.lo, B1.hi, B1.lo, C1.hi, C1.lo); \
    sub_ddmmss(A1.hi, A1.lo, A1.hi, A1.lo, 0, -__borrow2_1); \
  } while (0)
#endif

#if defined(SUB6_LIMB32)
/* A[0..2] = B[0..2] - C[0..2] */
#define SUB3_LIMB64(A2, A1, A0, B2, B1, B0, C2, C1, C0) \
	SUB6_LIMB32(A2.hi, A2.lo, A1.hi, A1.lo, A0.hi, A0.lo, \
		    B2.hi, B2.lo, B1.hi, B1.lo, B0.hi, B0.lo, \
		    C2.hi, C2.lo, C1.hi, C1.lo, C0.hi, C0.lo)
#endif

#if defined(SUB6_LIMB32)
/* A[0..3] = B[0..3] - C[0..3] */
#define SUB4_LIMB64(A3, A2, A1, A0, B3, B2, B1, B0, C3, C2, C1, C0) do { \
    mpi_limb_t __borrow4; \
    SUB6_LIMB32(__borrow4, A2.lo, A1.hi, A1.lo, A0.hi, A0.lo, \
		0, B2.lo, B1.hi, B1.lo, B0.hi, B0.lo, \
		0, C2.lo, C1.hi, C1.lo, C0.hi, C0.lo); \
    SUB4_LIMB32(A3.hi, A3.lo, A2.hi, __borrow4, \
		B3.hi, B3.lo, B2.hi, 0, \
		C3.hi, C3.lo, C2.hi, -__borrow4); \
  } while (0)
#endif

#endif /* BYTES_PER_MPI_LIMB == 4 */


/* Common definitions.  */
#define BITS_PER_MPI_LIMB64 (BITS_PER_MPI_LIMB * LIMBS_PER_LIMB64)
#define BYTES_PER_MPI_LIMB64 (BYTES_PER_MPI_LIMB * LIMBS_PER_LIMB64)


/* Common addition/subtraction macros.  */

#ifndef ADD3_LIMB64
/* A[0..2] = B[0..2] + C[0..2] */
#define ADD3_LIMB64(A2, A1, A0, B2, B1, B0, C2, C1, C0) do { \
    mpi_limb64_t __carry3; \
    ADD2_LIMB64(__carry3, A0, zero, B0, zero, C0); \
    ADD2_LIMB64(A2, A1, B2, B1, C2, C1); \
    ADD2_LIMB64(A2, A1, A2, A1, zero, __carry3); \
  } while (0)
#endif

#ifndef ADD4_LIMB64
/* A[0..3] = B[0..3] + C[0..3] */
#define ADD4_LIMB64(A3, A2, A1, A0, B3, B2, B1, B0, C3, C2, C1, C0) do { \
    mpi_limb64_t __carry4; \
    ADD3_LIMB64(__carry4, A1, A0, zero, B1, B0, zero, C1, C0); \
    ADD2_LIMB64(A3, A2, B3, B2, C3, C2); \
    ADD2_LIMB64(A3, A2, A3, A2, zero, __carry4); \
  } while (0)
#endif

#ifndef ADD5_LIMB64
/* A[0..4] = B[0..4] + C[0..4] */
#define ADD5_LIMB64(A4, A3, A2, A1, A0, B4, B3, B2, B1, B0, \
                    C4, C3, C2, C1, C0) do { \
    mpi_limb64_t __carry5; \
    ADD4_LIMB64(__carry5, A2, A1, A0, zero, B2, B1, B0, zero, C2, C1, C0); \
    ADD2_LIMB64(A4, A3, B4, B3, C4, C3); \
    ADD2_LIMB64(A4, A3, A4, A3, zero, __carry5); \
  } while (0)
#endif

#ifndef ADD7_LIMB64
/* A[0..6] = B[0..6] + C[0..6] */
#define ADD7_LIMB64(A6, A5, A4, A3, A2, A1, A0, B6, B5, B4, B3, B2, B1, B0, \
                    C6, C5, C4, C3, C2, C1, C0) do { \
    mpi_limb64_t __carry7; \
    ADD4_LIMB64(__carry7, A2, A1, A0, zero, B2, B1, B0, \
		zero, C2, C1, C0); \
    ADD5_LIMB64(A6, A5, A4, A3, __carry7, B6, B5, B4, B3, \
		__carry7, C6, C5, C4, C3, LIMB64_HILO(-1, -1)); \
  } while (0)
#endif

#ifndef SUB3_LIMB64
/* A[0..2] = B[0..2] - C[0..2] */
#define SUB3_LIMB64(A2, A1, A0, B2, B1, B0, C2, C1, C0) do { \
    mpi_limb64_t __borrow3; \
    SUB2_LIMB64(__borrow3, A0, zero, B0, zero, C0); \
    SUB2_LIMB64(A2, A1, B2, B1, C2, C1); \
    SUB2_LIMB64(A2, A1, A2, A1, zero, LIMB_TO64(-LIMB_FROM64(__borrow3))); \
  } while (0)
#endif

#ifndef SUB4_LIMB64
/* A[0..3] = B[0..3] - C[0..3] */
#define SUB4_LIMB64(A3, A2, A1, A0, B3, B2, B1, B0, C3, C2, C1, C0) do { \
    mpi_limb64_t __borrow4; \
    SUB3_LIMB64(__borrow4, A1, A0, zero, B1, B0, zero, C1, C0); \
    SUB2_LIMB64(A3, A2, B3, B2, C3, C2); \
    SUB2_LIMB64(A3, A2, A3, A2, zero, LIMB_TO64(-LIMB_FROM64(__borrow4))); \
  } while (0)
#endif

#ifndef SUB5_LIMB64
/* A[0..4] = B[0..4] - C[0..4] */
#define SUB5_LIMB64(A4, A3, A2, A1, A0, B4, B3, B2, B1, B0, \
                    C4, C3, C2, C1, C0) do { \
    mpi_limb64_t __borrow5; \
    SUB4_LIMB64(__borrow5, A2, A1, A0, zero, B2, B1, B0, zero, C2, C1, C0); \
    SUB2_LIMB64(A4, A3, B4, B3, C4, C3); \
    SUB2_LIMB64(A4, A3, A4, A3, zero, LIMB_TO64(-LIMB_FROM64(__borrow5))); \
  } while (0)
#endif

#ifndef SUB7_LIMB64
/* A[0..6] = B[0..6] - C[0..6] */
#define SUB7_LIMB64(A6, A5, A4, A3, A2, A1, A0, B6, B5, B4, B3, B2, B1, B0, \
                    C6, C5, C4, C3, C2, C1, C0) do { \
    mpi_limb64_t __borrow7; \
    SUB4_LIMB64(__borrow7, A2, A1, A0, zero, B2, B1, B0, \
		zero, C2, C1, C0); \
    SUB5_LIMB64(A6, A5, A4, A3, __borrow7, B6, B5, B4, B3, zero, \
		C6, C5, C4, C3, LIMB_TO64(-LIMB_FROM64(__borrow7))); \
  } while (0)
#endif


#if defined(WORDS_BIGENDIAN) || (BITS_PER_MPI_LIMB64 != BITS_PER_MPI_LIMB)
#define LOAD64_UNALIGNED(x, pos) \
  LIMB64_HILO(LOAD32(x, 2 * (pos) + 2), LOAD32(x, 2 * (pos) + 1))
#else
#define LOAD64_UNALIGNED(x, pos) \
  buf_get_le64((const byte *)(&(x)[pos]) + 4)
#endif


/* Helper functions.  */

static inline int
mpi_nbits_more_than (gcry_mpi_t w, unsigned int nbits)
{
  unsigned int nbits_nlimbs;
  mpi_limb_t wlimb;
  unsigned int n;

  nbits_nlimbs = (nbits + BITS_PER_MPI_LIMB - 1) / BITS_PER_MPI_LIMB;

  /* Note: Assumes that 'w' is normalized. */

  if (w->nlimbs > nbits_nlimbs)
    return 1;
  if (w->nlimbs < nbits_nlimbs)
    return 0;
  if ((nbits % BITS_PER_MPI_LIMB) == 0)
    return 0;

  wlimb = w->d[nbits_nlimbs - 1];
  if (wlimb == 0)
    log_bug ("mpi_nbits_more_than: input mpi not normalized\n");

  count_leading_zeros (n, wlimb);

  return (BITS_PER_MPI_LIMB - n) > (nbits % BITS_PER_MPI_LIMB);
}

#endif /* GCRY_EC_INLINE_H */
