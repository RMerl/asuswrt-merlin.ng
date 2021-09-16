/* salsa20-internal.h

   The Salsa20 stream cipher.

   Copyright (C) 2012 Simon Josefsson
   Copyright (C) 2001 Niels Möller

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

#ifndef NETTLE_SALSA20_INTERNAL_H_INCLUDED
#define NETTLE_SALSA20_INTERNAL_H_INCLUDED

#include "nettle-types.h"
#include "salsa20.h"

void
_nettle_salsa20_core(uint32_t *dst, const uint32_t *src, unsigned rounds);

void
_nettle_salsa20_crypt(struct salsa20_ctx *ctx, unsigned rounds,
		      size_t length, uint8_t *dst,
		      const uint8_t *src);

/* Functions available only in some configurations */
void
_nettle_salsa20_2core(uint32_t *dst, const uint32_t *src, unsigned rounds);

void
_nettle_salsa20_crypt_1core(struct salsa20_ctx *ctx, unsigned rounds,
			    size_t length, uint8_t *dst,
			    const uint8_t *src);
void
_nettle_salsa20_crypt_2core(struct salsa20_ctx *ctx, unsigned rounds,
			    size_t length, uint8_t *dst,
			    const uint8_t *src);

#endif /* NETTLE_SALSA20_INTERNAL_H_INCLUDED */
