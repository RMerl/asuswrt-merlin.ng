/* ripemd160-internal.h

   RIPEMD-160 hash function.

   Copyright (C) 2011 Andres Mejia

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

#ifndef NETTLE_RIPEMD160_INTERNAL_H_INCLUDED
#define NETTLE_RIPEMD160_INTERNAL_H_INCLUDED


/* Internal compression function. STATE points to 5 uint32_t words,
   and DATA points to 64 bytes of input data, possibly unaligned. */
void
_nettle_ripemd160_compress(uint32_t *state, const uint8_t *data);

#endif /* NETTLE_RIPEMD160_INTERNAL_H_INCLUDED */
