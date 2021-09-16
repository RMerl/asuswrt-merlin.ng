/* poly1305-internal.h

   Poly1305 message authentication code.

   Copyright (C) 2013 Nikos Mavrogiannopoulos
   Copyright (C) 2013, 2014 Niels Möller

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

#ifndef NETTLE_POLY1305_INTERNAL_H_INCLUDED
#define NETTLE_POLY1305_INTERNAL_H_INCLUDED

#include "poly1305.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Low level functions/macros for the poly1305 construction. */

#define POLY1305_DIGEST_SIZE 16
#define POLY1305_KEY_SIZE 16

/* Low-level internal interface. */
void _nettle_poly1305_set_key(struct poly1305_ctx *ctx, const uint8_t key[POLY1305_KEY_SIZE]);
/* Extracts digest, and adds it to s, the encrypted nonce. */
void _nettle_poly1305_digest (struct poly1305_ctx *ctx, union nettle_block16 *s);
/* Process one block. */
void _nettle_poly1305_block (struct poly1305_ctx *ctx, const uint8_t *m,
			     unsigned high);

#ifdef __cplusplus
}
#endif

#endif /* NETTLE_POLY1305_INTERNAL_H_INCLUDED */
