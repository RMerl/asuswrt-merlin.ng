/* shake128.c

   The SHAKE128 hash function, arbitrary length output.

   Copyright (C) 2017 Daiki Ueno
   Copyright (C) 2017 Red Hat, Inc.

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

#include <string.h>

#include "sha3.h"
#include "sha3-internal.h"

void
sha3_128_init (struct sha3_128_ctx *ctx)
{
  memset (ctx, 0, offsetof (struct sha3_128_ctx, block));
}

void
sha3_128_update (struct sha3_128_ctx *ctx,
		 size_t length,
		 const uint8_t *data)
{
  ctx->index = _nettle_sha3_update (&ctx->state,
				    SHA3_128_BLOCK_SIZE, ctx->block,
				    ctx->index, length, data);
}

void
sha3_128_shake (struct sha3_128_ctx *ctx,
		size_t length, uint8_t *dst)
{
  _nettle_sha3_shake (&ctx->state, sizeof (ctx->block), ctx->block, ctx->index, length, dst);
  sha3_128_init (ctx);
}

void
sha3_128_shake_output (struct sha3_128_ctx *ctx,
		       size_t length, uint8_t *digest)
{
  ctx->index =
    _nettle_sha3_shake_output (&ctx->state,
			       sizeof (ctx->block), ctx->block, ctx->index,
			       length, digest);
}
