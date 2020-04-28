/* typemax.h -- encapsulate max values for long, long long, etc. */

/* Copyright (C) 2001 Free Software Foundation, Inc.

   This file is part of GNU Bash, the Bourne Again SHell.

   Bash is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Bash is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Bash.  If not, see <http://www.gnu.org/licenses/>.
*/

/*
 * NOTE:  This should be included after config.h, limits.h, stdint.h, and
 *	  inttypes.h
 */

#ifndef _SH_TYPEMAX_H
#define _SH_TYPEMAX_H

#ifndef CHAR_BIT
#  define CHAR_BIT 8
#endif

/* Nonzero if the integer type T is signed.  */
#ifndef TYPE_SIGNED
#  define TYPE_SIGNED(t)	(! ((t) 0 < (t) -1))
#endif

#ifndef TYPE_MINIMUM
#  define TYPE_MINIMUM(t) ((t) (TYPE_SIGNED (t) \
				? ~ (t) 0 << (sizeof (t) * CHAR_BIT - 1) \
				: (t) 0))
#endif

#ifndef TYPE_MAXIMUM
#  define TYPE_MAXIMUM(t) ((t) (~ (t) 0 - TYPE_MINIMUM (t)))
#endif

#ifdef HAVE_LONG_LONG
#  ifndef LLONG_MAX
#    define LLONG_MAX   TYPE_MAXIMUM(long long int)
#    define LLONG_MIN	TYPE_MINIMUM(long long int)
#  endif
#  ifndef ULLONG_MAX
#    define ULLONG_MAX  TYPE_MAXIMUM(unsigned long long int)
#  endif
#endif

#ifndef ULONG_MAX
#  define ULONG_MAX	((unsigned long) ~(unsigned long) 0)
#endif

#ifndef LONG_MAX
#  define LONG_MAX	((long int) (ULONG_MAX >> 1))
#  define LONG_MIN	((long int) (-LONG_MAX - 1L))
#endif

#ifndef INT_MAX		/* ouch */
#  define INT_MAX	TYPE_MAXIMUM(int)
#  define INT_MIN	TYPE_MINIMUM(int)
#  define UINT_MAX	((unsigned int) ~(unsigned int)0)
#endif

/* workaround for gcc bug in versions < 2.7 */
#if defined (HAVE_LONG_LONG) && __GNUC__ == 2 && __GNUC_MINOR__ < 7
static const unsigned long long int maxquad = ULLONG_MAX;
#  undef ULLONG_MAX
#  define ULLONG_MAX maxquad
#endif

#if !defined (INTMAX_MAX) || !defined (INTMAX_MIN)

#if SIZEOF_INTMAX_T == SIZEOF_LONG_LONG
#  define INTMAX_MAX	LLONG_MAX
#  define INTMAX_MIN	LLONG_MIN
#elif SIZEOF_INTMAX_T == SIZEOF_LONG
#  define INTMAX_MAX	LONG_MAX
#  define INTMAX_MIN	LONG_MIN
#else
#  define INTMAX_MAX	INT_MAX
#  define INTMAX_MIN	INT_MIN
#endif

#endif

#ifndef SSIZE_MAX
#  define SSIZE_MAX	32767		/* POSIX minimum max */
#endif

#ifndef SIZE_MAX
#  define SIZE_MAX	65535		/* POSIX minimum max */
#endif

#endif /* _SH_TYPEMAX_H */
