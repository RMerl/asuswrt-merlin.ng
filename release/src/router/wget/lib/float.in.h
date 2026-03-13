/* A correct <float.h>.

   Copyright (C) 2007-2024 Free Software Foundation, Inc.

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

#ifndef _@GUARD_PREFIX@_FLOAT_H

#if __GNUC__ >= 3
@PRAGMA_SYSTEM_HEADER@
#endif
@PRAGMA_COLUMNS@

/* The include_next requires a split double-inclusion guard.  */
#@INCLUDE_NEXT@ @NEXT_FLOAT_H@

#ifndef _@GUARD_PREFIX@_FLOAT_H
#define _@GUARD_PREFIX@_FLOAT_H

/* ============================ ISO C99 support ============================ */

/* 'long double' properties.  */

#if defined __i386__ && (defined __BEOS__ || defined __OpenBSD__)
/* Number of mantissa units, in base FLT_RADIX.  */
# undef LDBL_MANT_DIG
# define LDBL_MANT_DIG   64
/* Number of decimal digits that is sufficient for representing a number.  */
# undef LDBL_DIG
# define LDBL_DIG        18
/* x-1 where x is the smallest representable number > 1.  */
# undef LDBL_EPSILON
# define LDBL_EPSILON    1.0842021724855044340E-19L
/* Minimum e such that FLT_RADIX^(e-1) is a normalized number.  */
# undef LDBL_MIN_EXP
# define LDBL_MIN_EXP    (-16381)
/* Maximum e such that FLT_RADIX^(e-1) is a representable finite number.  */
# undef LDBL_MAX_EXP
# define LDBL_MAX_EXP    16384
/* Minimum positive normalized number.  */
# undef LDBL_MIN
# define LDBL_MIN        3.3621031431120935063E-4932L
/* Maximum representable finite number.  */
# undef LDBL_MAX
# define LDBL_MAX        1.1897314953572317650E+4932L
/* Minimum e such that 10^e is in the range of normalized numbers.  */
# undef LDBL_MIN_10_EXP
# define LDBL_MIN_10_EXP (-4931)
/* Maximum e such that 10^e is in the range of representable finite numbers.  */
# undef LDBL_MAX_10_EXP
# define LDBL_MAX_10_EXP 4932
#endif

/* On FreeBSD/x86 6.4, the 'long double' type really has only 53 bits of
   precision in the compiler but 64 bits of precision at runtime.  See
   <https://lists.gnu.org/r/bug-gnulib/2008-07/msg00063.html>.  */
#if defined __i386__ && (defined __FreeBSD__ || defined __DragonFly__)
/* Number of mantissa units, in base FLT_RADIX.  */
# undef LDBL_MANT_DIG
# define LDBL_MANT_DIG   64
/* Number of decimal digits that is sufficient for representing a number.  */
# undef LDBL_DIG
# define LDBL_DIG        18
/* x-1 where x is the smallest representable number > 1.  */
# undef LDBL_EPSILON
# define LDBL_EPSILON 1.084202172485504434007452800869941711426e-19L /* 2^-63 */
/* Minimum e such that FLT_RADIX^(e-1) is a normalized number.  */
# undef LDBL_MIN_EXP
# define LDBL_MIN_EXP    (-16381)
/* Maximum e such that FLT_RADIX^(e-1) is a representable finite number.  */
# undef LDBL_MAX_EXP
# define LDBL_MAX_EXP    16384
/* Minimum positive normalized number.  */
# undef LDBL_MIN
# define LDBL_MIN        3.362103143112093506262677817321752E-4932L /* = 0x1p-16382L */
/* Maximum representable finite number.  */
# undef LDBL_MAX
/* LDBL_MAX is represented as { 0xFFFFFFFF, 0xFFFFFFFF, 32766 }.
   But the largest literal that GCC allows us to write is
   0x0.fffffffffffff8p16384L = { 0xFFFFF800, 0xFFFFFFFF, 32766 }.
   So, define it like this through a reference to an external variable

     const unsigned int LDBL_MAX[3] = { 0xFFFFFFFF, 0xFFFFFFFF, 32766 };
     extern const long double LDBL_MAX;

   Unfortunately, this is not a constant expression.  */
# if !GNULIB_defined_long_double_union
union gl_long_double_union
  {
    struct { unsigned int lo; unsigned int hi; unsigned int exponent; } xd;
    long double ld;
  };
#  define GNULIB_defined_long_double_union 1
# endif
extern const union gl_long_double_union gl_LDBL_MAX;
# define LDBL_MAX (gl_LDBL_MAX.ld)
/* Minimum e such that 10^e is in the range of normalized numbers.  */
# undef LDBL_MIN_10_EXP
# define LDBL_MIN_10_EXP (-4931)
/* Maximum e such that 10^e is in the range of representable finite numbers.  */
# undef LDBL_MAX_10_EXP
# define LDBL_MAX_10_EXP 4932
#endif

/* On AIX 7.1 with gcc 4.2, the values of LDBL_MIN_EXP, LDBL_MIN, LDBL_MAX are
   wrong.
   On Linux/PowerPC with gcc 4.4, the value of LDBL_MAX is wrong.  */
#if (defined _ARCH_PPC || defined _POWER) && defined _AIX && (LDBL_MANT_DIG == 106) && defined __GNUC__
# undef LDBL_MIN_EXP
# define LDBL_MIN_EXP DBL_MIN_EXP
# undef LDBL_MIN_10_EXP
# define LDBL_MIN_10_EXP DBL_MIN_10_EXP
# undef LDBL_MIN
# define LDBL_MIN 2.22507385850720138309023271733240406422e-308L /* DBL_MIN = 2^-1022 */
#endif
#if (defined _ARCH_PPC || defined _POWER) && (defined _AIX || defined __linux__) && (LDBL_MANT_DIG == 106) && defined __GNUC__
# undef LDBL_MAX
/* LDBL_MAX is represented as { 0x7FEFFFFF, 0xFFFFFFFF, 0x7C8FFFFF, 0xFFFFFFFF }.
   It is not easy to define:
     #define LDBL_MAX 1.79769313486231580793728971405302307166e308L
   is too small, whereas
     #define LDBL_MAX 1.79769313486231580793728971405302307167e308L
   is too large.  Apparently a bug in GCC decimal-to-binary conversion.
   Also, I can't get values larger than
     #define LDBL63 ((long double) (1ULL << 63))
     #define LDBL882 (LDBL63 * LDBL63 * LDBL63 * LDBL63 * LDBL63 * LDBL63 * LDBL63 * LDBL63 * LDBL63 * LDBL63 * LDBL63 * LDBL63 * LDBL63 * LDBL63)
     #define LDBL945 (LDBL63 * LDBL63 * LDBL63 * LDBL63 * LDBL63 * LDBL63 * LDBL63 * LDBL63 * LDBL63 * LDBL63 * LDBL63 * LDBL63 * LDBL63 * LDBL63 * LDBL63)
     #define LDBL1008 (LDBL63 * LDBL63 * LDBL63 * LDBL63 * LDBL63 * LDBL63 * LDBL63 * LDBL63 * LDBL63 * LDBL63 * LDBL63 * LDBL63 * LDBL63 * LDBL63 * LDBL63 * LDBL63)
     #define LDBL_MAX (LDBL1008 * 65535.0L + LDBL945 * (long double) 9223372036821221375ULL + LDBL882 * (long double) 4611686018427387904ULL)
   which is represented as { 0x7FEFFFFF, 0xFFFFFFFF, 0x7C8FFFFF, 0xF8000000 }.
   So, define it like this through a reference to an external variable

     const double LDBL_MAX[2] = { DBL_MAX, DBL_MAX / (double)134217728UL / (double)134217728UL };
     extern const long double LDBL_MAX;

   or through a pointer cast

     #define LDBL_MAX \
       (*(const long double *) (double[]) { DBL_MAX, DBL_MAX / (double)134217728UL / (double)134217728UL })

   Unfortunately, this is not a constant expression, and the latter expression
   does not work well when GCC is optimizing..  */
# if !GNULIB_defined_long_double_union
union gl_long_double_union
  {
    struct { double hi; double lo; } dd;
    long double ld;
  };
#  define GNULIB_defined_long_double_union 1
# endif
extern const union gl_long_double_union gl_LDBL_MAX;
# define LDBL_MAX (gl_LDBL_MAX.ld)
#endif

/* On IRIX 6.5, with cc, the value of LDBL_MANT_DIG is wrong.
   On IRIX 6.5, with gcc 4.2, the values of LDBL_MIN_EXP, LDBL_MIN, LDBL_EPSILON
   are wrong.  */
#if defined __sgi && (LDBL_MANT_DIG >= 106)
# undef LDBL_MANT_DIG
# define LDBL_MANT_DIG 106
# if defined __GNUC__
#  undef LDBL_MIN_EXP
#  define LDBL_MIN_EXP DBL_MIN_EXP
#  undef LDBL_MIN_10_EXP
#  define LDBL_MIN_10_EXP DBL_MIN_10_EXP
#  undef LDBL_MIN
#  define LDBL_MIN 2.22507385850720138309023271733240406422e-308L /* DBL_MIN = 2^-1022 */
#  undef LDBL_EPSILON
#  define LDBL_EPSILON 2.46519032881566189191165176650870696773e-32L /* 2^-105 */
# endif
#endif

/* ============================ ISO C11 support ============================ */

/* 'float' properties */

#ifndef FLT_HAS_SUBNORM
# define FLT_HAS_SUBNORM 1
#endif
#ifndef FLT_DECIMAL_DIG
/* FLT_MANT_DIG = 24 => FLT_DECIMAL_DIG = 9 */
# define FLT_DECIMAL_DIG ((int)(FLT_MANT_DIG * 0.3010299956639812 + 2))
#endif
#if defined _AIX && !defined __GNUC__
/* On AIX, the value of FLT_TRUE_MIN in /usr/include/float.h is a 'double',
   not a 'float'.  */
# undef FLT_TRUE_MIN
#endif
#ifndef FLT_TRUE_MIN
/* FLT_MIN / 2^(FLT_MANT_DIG-1) */
# define FLT_TRUE_MIN (FLT_MIN / 8388608.0f)
#endif

/* 'double' properties */

#ifndef DBL_HAS_SUBNORM
# define DBL_HAS_SUBNORM 1
#endif
#ifndef DBL_DECIMAL_DIG
/* DBL_MANT_DIG = 53 => DBL_DECIMAL_DIG = 17 */
# define DBL_DECIMAL_DIG ((int)(DBL_MANT_DIG * 0.3010299956639812 + 2))
#endif
#ifndef DBL_TRUE_MIN
/* DBL_MIN / 2^(DBL_MANT_DIG-1) */
# define DBL_TRUE_MIN (DBL_MIN / 4503599627370496.0)
#endif

/* 'long double' properties */

#ifndef LDBL_HAS_SUBNORM
# define LDBL_HAS_SUBNORM 1
#endif
#ifndef LDBL_DECIMAL_DIG
/* LDBL_MANT_DIG = 53 => LDBL_DECIMAL_DIG = 17 */
/* LDBL_MANT_DIG = 64 => LDBL_DECIMAL_DIG = 21 */
/* LDBL_MANT_DIG = 106 => LDBL_DECIMAL_DIG = 33 */
/* LDBL_MANT_DIG = 113 => LDBL_DECIMAL_DIG = 36 */
# define LDBL_DECIMAL_DIG ((int)(LDBL_MANT_DIG * 0.3010299956639812 + 2))
#endif
#ifndef LDBL_TRUE_MIN
/* LDBL_MIN / 2^(LDBL_MANT_DIG-1) */
# if LDBL_MANT_DIG == 53
#  define LDBL_TRUE_MIN (LDBL_MIN / 4503599627370496.0L)
# elif LDBL_MANT_DIG == 64
#  if defined __i386__ && (defined __FreeBSD__ || defined __DragonFly__)
/* Work around FreeBSD/x86 problem mentioned above.  */
extern const union gl_long_double_union gl_LDBL_TRUE_MIN;
#   define LDBL_TRUE_MIN (gl_LDBL_TRUE_MIN.ld)
#  else
#   define LDBL_TRUE_MIN (LDBL_MIN / 9223372036854775808.0L)
#  endif
# elif LDBL_MANT_DIG == 106
#  define LDBL_TRUE_MIN (LDBL_MIN / 40564819207303340847894502572032.0L)
# elif LDBL_MANT_DIG == 113
#  define LDBL_TRUE_MIN (LDBL_MIN / 5192296858534827628530496329220096.0L)
# endif
#endif

/* ============================ ISO C23 support ============================ */

/* 'float' properties */

#ifndef FLT_IS_IEC_60559
# if defined __m68k__
#  define FLT_IS_IEC_60559 0
# else
#  define FLT_IS_IEC_60559 1
# endif
#endif
#ifndef FLT_NORM_MAX
# define FLT_NORM_MAX FLT_MAX
#endif
#ifndef FLT_SNAN
/* For sh, beware of <https://gcc.gnu.org/bugzilla/show_bug.cgi?id=111814>.  */
# if ((__GNUC__ + (__GNUC_MINOR__ >= 3) > 3) || defined __clang__) && !defined __sh__
#  define FLT_SNAN __builtin_nansf ("")
# else
typedef union { unsigned int word[1]; float value; } gl_FLT_SNAN_t;
extern gl_FLT_SNAN_t gl_FLT_SNAN;
#  define FLT_SNAN (gl_FLT_SNAN.value)
#  define GNULIB_defined_FLT_SNAN 1
# endif
#endif

/* 'double' properties */

#ifndef DBL_IS_IEC_60559
# if defined __m68k__
#  define DBL_IS_IEC_60559 0
# else
#  define DBL_IS_IEC_60559 1
# endif
#endif
#ifndef DBL_NORM_MAX
# define DBL_NORM_MAX DBL_MAX
#endif
#ifndef DBL_SNAN
/* For sh, beware of <https://gcc.gnu.org/bugzilla/show_bug.cgi?id=111814>.  */
# if ((__GNUC__ + (__GNUC_MINOR__ >= 3) > 3) || defined __clang__) && !defined __sh__
#  define DBL_SNAN __builtin_nans ("")
# else
typedef union { unsigned long long word[1]; double value; } gl_DBL_SNAN_t;
extern gl_DBL_SNAN_t gl_DBL_SNAN;
#  define DBL_SNAN (gl_DBL_SNAN.value)
#  define GNULIB_defined_DBL_SNAN 1
# endif
#endif

/* 'long double' properties */

#ifndef LDBL_IS_IEC_60559
# if defined __m68k__
#  define LDBL_IS_IEC_60559 0
# elif LDBL_MANT_DIG == 53 || LDBL_MANT_DIG == 113
#  define LDBL_IS_IEC_60559 1
# else
#  define LDBL_IS_IEC_60559 0
# endif
#endif
#ifndef LDBL_NORM_MAX
# define LDBL_NORM_MAX LDBL_MAX
#endif
#ifndef LDBL_SNAN
/* For sh, beware of <https://gcc.gnu.org/bugzilla/show_bug.cgi?id=111814>.  */
# if ((__GNUC__ + (__GNUC_MINOR__ >= 3) > 3) || defined __clang__) && !defined __sh__
#  define LDBL_SNAN __builtin_nansl ("")
# else
#  if LDBL_MANT_DIG == 53
typedef union { unsigned long long word[1]; long double value; } gl_LDBL_SNAN_t;
#  elif defined __m68k__
typedef union { unsigned int word[3]; long double value; } gl_LDBL_SNAN_t;
#  else
typedef union { unsigned long long word[2]; long double value; } gl_LDBL_SNAN_t;
#  endif
extern gl_LDBL_SNAN_t gl_LDBL_SNAN;
#  define LDBL_SNAN (gl_LDBL_SNAN.value)
#  define GNULIB_defined_LDBL_SNAN 1
# endif
#endif

/* ================================= Other ================================= */

#if @REPLACE_ITOLD@
/* Pull in a function that fixes the 'int' to 'long double' conversion
   of glibc 2.7.  */
extern
# ifdef __cplusplus
"C"
# endif
void _Qp_itoq (long double *, int);
static void (*_gl_float_fix_itold) (long double *, int) = _Qp_itoq;
#endif

#endif /* _@GUARD_PREFIX@_FLOAT_H */
#endif /* _@GUARD_PREFIX@_FLOAT_H */
