/* ctr-internal.h

   Copyright (C) 2018 Niels Möller

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

#ifndef NETTLE_CTR_INTERNAL_H_INCLUDED
#define NETTLE_CTR_INTERNAL_H_INCLUDED

#include "nettle-types.h"

/* Size limit for temporary stack buffers. */
#define CTR_BUFFER_LIMIT 512

/* Fill BUFFER (n blocks) with incrementing CTR values. It would be
   nice if CTR was always 64-bit aligned, but it isn't when called
   from ctr_crypt. */
typedef void
nettle_fill16_func(uint8_t *ctr, size_t n, union nettle_block16 *buffer);

void
_nettle_ctr_crypt16(const void *ctx, nettle_cipher_func *f,
		    nettle_fill16_func *fill, uint8_t *ctr,
		    size_t length, uint8_t *dst,
		    const uint8_t *src);


#endif /* NETTLE_CTR_INTERNAL_H_INCLUDED */
