/* bithelp.h  -  Some bit manipulation helpers
 *	Copyright (C) 1999, 2002 Free Software Foundation, Inc.
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
#ifndef GCRYPT_BITHELP_H
#define GCRYPT_BITHELP_H

#include "types.h"


/****************
 * Rotate the 32 bit unsigned integer X by N bits left/right
 */
static inline u32 rol(u32 x, int n)
{
	return ( (x << (n&(32-1))) | (x >> ((32-n)&(32-1))) );
}

static inline u32 ror(u32 x, int n)
{
	return ( (x >> (n&(32-1))) | (x << ((32-n)&(32-1))) );
}

static inline u64 rol64(u64 x, int n)
{
  return ( (x << (n&(64-1))) | (x >> ((64-n)&(64-1))) );
}

/* Byte swap for 32-bit and 64-bit integers.  If available, use compiler
   provided helpers.  */
#ifdef HAVE_BUILTIN_BSWAP32
# define _gcry_bswap32 __builtin_bswap32
#else
static inline u32
_gcry_bswap32(u32 x)
{
	return ((rol(x, 8) & 0x00ff00ffL) | (ror(x, 8) & 0xff00ff00L));
}
#endif

#ifdef HAVE_BUILTIN_BSWAP64
# define _gcry_bswap64 __builtin_bswap64
#else
static inline u64
_gcry_bswap64(u64 x)
{
	return ((u64)_gcry_bswap32(x) << 32) | (_gcry_bswap32(x >> 32));
}
#endif

/* Endian dependent byte swap operations.  */
#ifdef WORDS_BIGENDIAN
# define le_bswap32(x) _gcry_bswap32(x)
# define be_bswap32(x) ((u32)(x))
# define le_bswap64(x) _gcry_bswap64(x)
# define be_bswap64(x) ((u64)(x))
#else
# define le_bswap32(x) ((u32)(x))
# define be_bswap32(x) _gcry_bswap32(x)
# define le_bswap64(x) ((u64)(x))
# define be_bswap64(x) _gcry_bswap64(x)
#endif


/* Count trailing zero bits in an unsigend int.  We return an int
   because that is what gcc's builtin does.  Returns the number of
   bits in X if X is 0. */
static inline int
_gcry_ctz (unsigned int x)
{
#if defined (HAVE_BUILTIN_CTZ)
  return x ? __builtin_ctz (x) : 8 * sizeof (x);
#else
  /* See
   * http://graphics.stanford.edu/~seander/bithacks.html#ZerosOnRightModLookup
   */
  static const unsigned char mod37[] =
    {
      sizeof (unsigned int)*8,
          0,  1, 26,  2, 23, 27,  0,  3, 16, 24, 30, 28, 11,  0, 13,
      4,  7, 17,  0, 25, 22, 31, 15, 29, 10, 12,  6,  0, 21, 14,  9,
      5, 20,  8, 19, 18
    };
  return (int)mod37[(-x & x) % 37];
#endif
}


/* Count trailing zero bits in an u64.  We return an int because that
   is what gcc's builtin does.  Returns the number of bits in X if X
   is 0.  */
static inline int
_gcry_ctz64(u64 x)
{
#if defined (HAVE_BUILTIN_CTZL) && SIZEOF_UNSIGNED_LONG >= 8
  return x ? __builtin_ctzl (x) : 8 * sizeof (x);
#elif defined (HAVE_BUILTIN_CTZ) && SIZEOF_UNSIGNED_INT >= 8
#warning hello
  return x ? __builtin_ctz (x) : 8 * sizeof (x);
#else
  if ((x & 0xffffffff))
    return _gcry_ctz (x);
  else
    return 32 + _gcry_ctz (x >> 32);
#endif
}


#endif /*GCRYPT_BITHELP_H*/
