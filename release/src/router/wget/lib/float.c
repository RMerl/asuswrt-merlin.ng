/* Auxiliary definitions for <float.h>.
   Copyright (C) 2011-2024 Free Software Foundation, Inc.
   Written by Bruno Haible <bruno@clisp.org>, 2011.

   This file is free software: you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   This file is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

#include <config.h>

/* Specification.  */
#include <float.h>

#if GNULIB_defined_long_double_union
# if (defined _ARCH_PPC || defined _POWER) && (defined _AIX || defined __linux__) && (LDBL_MANT_DIG == 106) && defined __GNUC__
const union gl_long_double_union gl_LDBL_MAX =
  { { DBL_MAX, DBL_MAX / (double)134217728UL / (double)134217728UL } };
# elif defined __i386__
const union gl_long_double_union gl_LDBL_MAX =
  { { 0xFFFFFFFF, 0xFFFFFFFF, 32766 } };
# endif
# if defined __i386__ && (defined __FreeBSD__ || defined __DragonFly__)
/* We can't even simply evaluate the formula (LDBL_MIN / 9223372036854775808.0L)
   at run time, because it would require BEGIN_LONG_DOUBLE_ROUNDING /
   END_LONG_DOUBLE_ROUNDING invocations.  It simpler to just write down the
   representation of LDBL_TRUE_MIN, based on
   <https://en.wikipedia.org/wiki/Extended_precision#x86_extended_precision_format>.  */
const union gl_long_double_union gl_LDBL_TRUE_MIN =
  { { 0x00000001, 0x00000000, 0 } };
# endif
#endif

#if GNULIB_defined_FLT_SNAN
/* Define like memory_positive_SNaNf(), see signed-snan.h and snan.h,
   or like setpayloadsigf() with an arbitrary payload.  */
gl_FLT_SNAN_t gl_FLT_SNAN =
# if FLT_MANT_DIG == 24
#  if defined __hppa || (defined __mips__ && !MIPS_NAN2008_FLOAT) || defined __sh__
  /* sign bit: 0, 8 exponent bits: all 1, next bit: 1, payload: 0b10...0 */
  { { 0x7FE00000U } }
#  else
  /* sign bit: 0, 8 exponent bits: all 1, next bit: 0, payload: 0b10...0 */
  { { 0x7FA00000U } }
#  endif
# endif
  ;
#endif

#if GNULIB_defined_DBL_SNAN
/* Define like memory_positive_SNaNd(), see signed-snan.h and snan.h,
   or like setpayloadsig() with an arbitrary payload.  */
gl_DBL_SNAN_t gl_DBL_SNAN =
# if DBL_MANT_DIG == 53
#  if defined __hppa || (defined __mips__ && !MIPS_NAN2008_FLOAT) || defined __sh__
  /* sign bit: 0, 11 exponent bits: all 1, next bit: 1, payload: 0b10...0 */
  { { 0x7FFC000000000000ULL } }
#  else
  /* sign bit: 0, 11 exponent bits: all 1, next bit: 0, payload: 0b10...0 */
  { { 0x7FF4000000000000ULL } }
#  endif
# endif
  ;
#endif

#if GNULIB_defined_LDBL_SNAN
# ifdef WORDS_BIGENDIAN
#  define TWO(hi,lo) { hi, lo }
# else
#  define TWO(hi,lo) { lo, hi }
# endif
/* Define like memory_positive_SNaNl(), see signed-snan.h and snan.h,
   or like setpayloadsigl() with an arbitrary payload.  */
gl_LDBL_SNAN_t gl_LDBL_SNAN =
# if LDBL_MANT_DIG == 53 /* on arm, hppa, mips, sh, but also MSVC */
#  if defined __hppa || (defined __mips__ && !MIPS_NAN2008_FLOAT) || defined __sh__
  /* sign bit: 0, 11 exponent bits: all 1, next bit: 1, payload: 0b10...0 */
  { { 0x7FFC000000000000ULL } }
#  else
  /* sign bit: 0, 11 exponent bits: all 1, next bit: 0, payload: 0b10...0 */
  { { 0x7FF4000000000000ULL } }
#  endif
# elif LDBL_MANT_DIG == 64 /* on i386, x86_64, ia64, m68k */
#  if defined __m68k__
  /* sign bit: 0, 15 exponent bits: all 1, 16 gap bits: all 0,
     always=1 bit: 1, next bit: 0, payload: 0b10...0 */
  { { 0x7FFF0000ULL, 0xA0000000ULL, 0x00000000ULL } }
#  else
  /* sign bit: 0, 15 exponent bits: all 1, always=1 bit: 1, next bit: 0, payload: 0b10...0
     (see <https://en.wikipedia.org/wiki/Extended_precision#x86_extended_precision_format>) */
  { TWO (0x00007FFFULL, 0xA000000000000000ULL) }
#  endif
# elif LDBL_MANT_DIG == 106 /* on powerpc, powerpc64, powerpc64le */
  /* most-significant double:
     sign bit: 0, 11 exponent bits: all 1, next bit: 0, payload: 0b10...0,
     least-significant double: 0.0 */
  { { 0x7FF4000000000000ULL, 0ULL } }
# elif LDBL_MANT_DIG == 113 /* on alpha, arm64, loongarch64, mips64, riscv64, s390x, sparc64 */
#  if (defined __mips__ && !MIPS_NAN2008_FLOAT)
  /* sign bit: 0, 15 exponent bits: all 1, next bit: 1, payload: 0b10...0 */
  { TWO (0x7FFFC00000000000ULL, 0ULL) }
#  else
  /* sign bit: 0, 15 exponent bits: all 1, next bit: 0, payload: 0b10...0 */
  { TWO (0x7FFF400000000000ULL, 0ULL) }
#  endif
# endif
  ;
#endif

/* This declaration is solely to ensure that after preprocessing
   this file is never empty.  */
typedef int dummy;
