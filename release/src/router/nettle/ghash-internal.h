/* ghash-internal.h

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

#ifndef NETTLE_GHASH_INTERNAL_H_INCLUDED
#define NETTLE_GHASH_INTERNAL_H_INCLUDED

#include "nettle-types.h"
#include "gcm.h"

/* Name mangling */
#define _ghash_set_key _nettle_ghash_set_key
#define _ghash_update _nettle_ghash_update

#ifdef __cplusplus
extern "C" {
#endif

/* The CTX a struct gcm_key (even if struct ghash_key might be a more
 * appropriate name). An array of blocks, exact contents depends on
 * the implementation. STATE is only a single block. */

/* Expands KEY as needed, for corresponding _ghash_update */
void
_ghash_set_key (struct gcm_key *ctx, const union nettle_block16 *key);

/* Updates STATE by hashing DATA, which must be an integral number of
   blocks. For convenience, returns a pointer to the end of the
   data. */
const uint8_t *
_ghash_update (const struct gcm_key *ctx, union nettle_block16 *state,
	       size_t blocks, const uint8_t *data);

#ifdef __cplusplus
}
#endif

#endif /* NETTLE_GHASH_INTERNAL_H_INCLUDED */
