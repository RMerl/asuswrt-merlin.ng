/* shake256.c

   The SHAKE256 hash function, arbitrary length output.

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

#include <stddef.h>
#include <string.h>

#include "sha3.h"
#include "sha3-internal.h"

#include "nettle-write.h"

void
sha3_256_shake (struct sha3_256_ctx *ctx,
		size_t length,
		uint8_t *dst)
{
  _sha3_pad_shake (&ctx->state, SHA3_256_BLOCK_SIZE, ctx->block, ctx->index);
  while (length > SHA3_256_BLOCK_SIZE)
    {
      _nettle_write_le64 (SHA3_256_BLOCK_SIZE, dst, ctx->state.a);
      length -= SHA3_256_BLOCK_SIZE;
      dst += SHA3_256_BLOCK_SIZE;
      sha3_permute (&ctx->state);
    }
  _nettle_write_le64 (length, dst, ctx->state.a);

  sha3_256_init (ctx);
}
