/* umac-l3.c

   Copyright (C) 2013 Niels Möller

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

#if HAVE_CONFIG_H
# include "config.h"
#endif

#include "umac.h"
#include "umac-internal.h"

#include "bswap-internal.h"

/* 2^36 - 5 */
#define P 0x0000000FFFFFFFFBULL

void
_nettle_umac_l3_init (unsigned size, uint64_t *k)
{
  unsigned i;
  for (i = 0; i < size; i++)
    {
      uint64_t w = k[i];
      w = bswap64_if_le (w);
      k[i] = w % P;
    }
}

static uint64_t
umac_l3_word (const uint64_t *k, uint64_t w)
{
  unsigned i;
  uint64_t y;

  /* Since it's easiest to process the input word from the low end,
   * loop over keys in reverse order. */

  for (i = y = 0; i < 4; i++, w >>= 16)
    y += (w & 0xffff) * k[3-i];

  return y;
}

uint32_t
_nettle_umac_l3 (const uint64_t *key, const uint64_t *m)
{
  uint32_t y = (umac_l3_word (key, m[0])
		+ umac_l3_word (key + 4, m[1])) % P;

  return bswap32_if_le (y);
}
