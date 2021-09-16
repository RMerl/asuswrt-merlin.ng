/* sha2-internal.h

   The sha2 family of hash functions.

   Copyright (C) 2001, 2012 Niels Möller

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

#ifndef NETTLE_SHA2_INTERNAL_H_INCLUDED
#define NETTLE_SHA2_INTERNAL_H_INCLUDED

#include "nettle-types.h"

/* Internal compression function. STATE points to 8 uint32_t words,
   DATA points to 64 bytes of input data, possibly unaligned, and K
   points to the table of constants. */
void
_nettle_sha256_compress(uint32_t *state, const uint8_t *data, const uint32_t *k);

/* Internal compression function. STATE points to 8 uint64_t words,
   DATA points to 128 bytes of input data, possibly unaligned, and K
   points to the table of constants. */
void
_nettle_sha512_compress(uint64_t *state, const uint8_t *data, const uint64_t *k);


#endif /* NETTLE_SHA2_INTERNAL_H_INCLUDED */
