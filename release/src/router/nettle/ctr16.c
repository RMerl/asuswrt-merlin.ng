/* ctr16.c

   Cipher counter mode, optimized for 16-byte blocks.

   Copyright (C) 2005-2018 Niels Möller
   Copyright (C) 2018 Red Hat, Inc.

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

#include <assert.h>

#include "ctr.h"

#include "ctr-internal.h"
#include "memxor.h"
#include "nettle-internal.h"

#define MIN(a,b) (((a) < (b)) ? (a) : (b))

void
_nettle_ctr_crypt16(const void *ctx, nettle_cipher_func *f,
		    nettle_fill16_func *fill, uint8_t *ctr,
		    size_t length, uint8_t *dst,
		    const uint8_t *src)
{
  if (dst != src && !((uintptr_t) dst % sizeof(uint64_t)))
    {
      size_t blocks = length / 16u;
      size_t done;
      fill (ctr, blocks, (union nettle_block16 *) dst);

      done = blocks * 16;
      f(ctx, done, dst, dst);
      memxor (dst, src, done);

      length -= done;
      if (length > 0)
	{ /* Left-over partial block */
	  union nettle_block16 block;
	  dst += done;
	  src += done;
	  assert (length < 16);
	  /* Use fill, to update ctr value in the same way in all cases. */
	  fill (ctr, 1, &block);
	  f (ctx, 16, block.b, block.b);
	  memxor3 (dst, src, block.b, length);
	}
    }
  else
    {
      /* Construct an aligned buffer of consecutive counter values, of
	 size at most CTR_BUFFER_LIMIT. */
      TMP_DECL(buffer, union nettle_block16, CTR_BUFFER_LIMIT / 16);
      size_t blocks = (length + 15) / 16u;
      size_t i;
      TMP_ALLOC(buffer, MIN(blocks, CTR_BUFFER_LIMIT / 16));

      for (i = 0; blocks >= CTR_BUFFER_LIMIT / 16;
	   i += CTR_BUFFER_LIMIT, blocks -= CTR_BUFFER_LIMIT / 16)
	{
	  fill (ctr, CTR_BUFFER_LIMIT / 16, buffer);
	  f(ctx, CTR_BUFFER_LIMIT, buffer->b, buffer->b);
	  if (length - i < CTR_BUFFER_LIMIT)
	    goto done;
	  memxor3 (dst + i, src + i, buffer->b, CTR_BUFFER_LIMIT);
	}

      if (blocks > 0)
	{
	  assert (length - i < CTR_BUFFER_LIMIT);
	  fill (ctr, blocks, buffer);
	  f(ctx, blocks * 16, buffer->b, buffer->b);
	done:
	  memxor3 (dst + i, src + i, buffer->b, length - i);
	}
    }
}
