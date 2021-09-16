/* umac-internal.h

   UMAC message authentication code (RFC-4418).

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

#ifndef NETTLE_UMAC_INTERNAL_H_INCLUDED
#define NETTLE_UMAC_INTERNAL_H_INCLUDED

#include "nettle-types.h"

void
_nettle_umac_set_key (uint32_t *l1_key, uint32_t *l2_key,
		      uint64_t *l3_key1, uint32_t *l3_key2,
		      struct aes128_ctx *pad, const uint8_t *key, unsigned n);

uint64_t
_nettle_umac_nh (const uint32_t *key, unsigned length, const uint8_t *msg);

/* Equivalent to

   for (i = 0; i < n; i++)
     out[i] = _umac_nh (key + 4*i, length, msg);

   but processing input only once.
*/
void
_nettle_umac_nh_n (uint64_t *out, unsigned n, const uint32_t *key,
		   unsigned length, const uint8_t *msg);

/* Returns y*k + m (mod p), including "marker" processing. Return
   value is *not* in canonical representation, and must be normalized
   before the output is used. */
uint64_t
_nettle_umac_poly64 (uint32_t kh, uint32_t kl, uint64_t y, uint64_t m);

void
_nettle_umac_poly128 (const uint32_t *k, uint64_t *y, uint64_t mh, uint64_t ml);

void
_nettle_umac_l2_init (unsigned size, uint32_t *k);

void
_nettle_umac_l2(const uint32_t *key, uint64_t *state, unsigned n,
		uint64_t count, const uint64_t *m);

void
_nettle_umac_l2_final(const uint32_t *key, uint64_t *state, unsigned n,
		      uint64_t count);

void
_nettle_umac_l3_init (unsigned size, uint64_t *k);

uint32_t
_nettle_umac_l3 (const uint64_t *key, const uint64_t *m);

#endif /* NETTLE_UMAC_INTERNAL_H_INCLUDED */
