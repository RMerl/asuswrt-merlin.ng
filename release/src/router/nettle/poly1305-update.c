/* poly1305-update.c

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

#if HAVE_CONFIG_H
#include "config.h"
#endif

#include "poly1305.h"
#include "poly1305-internal.h"
#include "md-internal.h"

#if HAVE_NATIVE_fat_poly1305_blocks
const uint8_t *
_nettle_poly1305_blocks_c(struct poly1305_ctx *ctx,
			   size_t blocks, const uint8_t *m);

const uint8_t *
_nettle_poly1305_blocks_c(struct poly1305_ctx *ctx,
			   size_t blocks, const uint8_t *m)
{
  for (; blocks; blocks--, m += POLY1305_BLOCK_SIZE)
    _nettle_poly1305_block(ctx, m, 1);
  return m;
}
#endif

unsigned
_nettle_poly1305_update (struct poly1305_ctx *ctx,
			 uint8_t *block, unsigned index,
			 size_t length, const uint8_t *m)
{
  if (!length)
    return index;

  if (index > 0)
    {
      /* Try to fill partial block */
      MD_FILL_OR_RETURN_INDEX (POLY1305_BLOCK_SIZE, block, index,
			       length, m);
      _nettle_poly1305_block(ctx, block, 1);
    }
#if HAVE_NATIVE_poly1305_blocks
  m = _nettle_poly1305_blocks (ctx, length >> 4, m);
  length &= 15;
#else
  for (; length >= POLY1305_BLOCK_SIZE;
       length -= POLY1305_BLOCK_SIZE, m += POLY1305_BLOCK_SIZE)
    _nettle_poly1305_block (ctx, m, 1);
#endif

  memcpy (block, m, length);
  return length;
}
