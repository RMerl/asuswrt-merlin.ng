/* bswap-internal.h

   Copyright (C) 2022 Niels Möller

   This file is part of GNU Nettle.

   GNU Nettle is free software: you can redistribute it and/or
   modify it under the terms of either:

     * the GNU Lesser General Public License as published by the Free
       Software Foundation; either version 3 of the License, or (at your
       option) any later version.

   or

     * the GNU General Public License as published by the Free
       Software Foundation; either version 2 of the License, or (at your
       option) any later version.

   or both in parallel, as here.

   GNU Nettle is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received copies of the GNU General Public License and
   the GNU Lesser General Public License along with this program.  If
   not, see http://www.gnu.org/licenses/.
*/

#ifndef NETTLE_BSWAP_INTERNAL_H_INCLUDED
#define NETTLE_BSWAP_INTERNAL_H_INCLUDED

#include "nettle-types.h"

/* Note that these definitions depend config.h, which should be
   included first. */

#if HAVE_BUILTIN_BSWAP64
#define nettle_bswap64 __builtin_bswap64
/* Assume bswap32 is also available. */
#define nettle_bswap32 __builtin_bswap32
#else
static inline uint64_t
nettle_bswap64 (uint64_t x)
{
  x = (x >> 32) | (x << 32);
  x = ((x >> 16) & UINT64_C (0xffff0000ffff))
    | ((x & UINT64_C (0xffff0000ffff)) << 16);
  x = ((x >> 8) & UINT64_C (0xff00ff00ff00ff))
    | ((x & UINT64_C (0xff00ff00ff00ff)) << 8);
  return x;
}

static inline uint32_t
nettle_bswap32 (uint32_t x)
{
  x = (x << 16) | (x >> 16);
  x = ((x & 0x00FF00FF) << 8) | ((x >> 8) & 0x00FF00FF);
  return x;
}
#endif

static inline void
nettle_bswap32_n (unsigned n, uint32_t *x)
{
  unsigned i;
  for (i = 0; i < n; i++)
    x[i] = nettle_bswap32 (x[i]);
}

#if WORDS_BIGENDIAN
#define bswap64_if_be nettle_bswap64
#define bswap32_if_be nettle_bswap32
#define bswap64_if_le(x) (x)
#define bswap32_if_le(x) (x)
#define bswap32_n_if_le(n, x)
#else
#define bswap64_if_be(x) (x)
#define bswap32_if_be(x) (x)
#define bswap64_if_le nettle_bswap64
#define bswap32_if_le nettle_bswap32
#define bswap32_n_if_le nettle_bswap32_n
#endif

#endif /* NETTLE_BSWAP_INTERNAL_H_INCLUDED */
