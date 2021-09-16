/* salsa20-crypt-internal.c

   The Salsa20 stream cipher.

   Copyright (C) 2012 Simon Josefsson
   Copyright (C) 2020 Niels Möller

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

#include "salsa20.h"
#include "salsa20-internal.h"

#include "macros.h"
#include "memxor.h"

#if HAVE_NATIVE_salsa20_2core
#define _nettle_salsa20_crypt_2core _nettle_salsa20_crypt
#elif !HAVE_NATIVE_fat_salsa20_2core
#define _nettle_salsa20_crypt_1core _nettle_salsa20_crypt
#endif

#if HAVE_NATIVE_salsa20_2core || HAVE_NATIVE_fat_salsa20_2core
void
_nettle_salsa20_crypt_2core(struct salsa20_ctx *ctx, unsigned rounds,
			    size_t length, uint8_t *dst,
			    const uint8_t *src)
{
  uint32_t x[2*_SALSA20_INPUT_LENGTH];
  while (length > SALSA20_BLOCK_SIZE)
    {
      _nettle_salsa20_2core (x, ctx->input, rounds);
      ctx->input[8] += 2;
      ctx->input[9] += (ctx->input[8] < 2);
      if (length <= 2 * SALSA20_BLOCK_SIZE)
	{
	  memxor3 (dst, src, x, length);
	  return;
	}
      memxor3 (dst, src, x, 2*SALSA20_BLOCK_SIZE);

      length -= 2*SALSA20_BLOCK_SIZE;
      dst += 2*SALSA20_BLOCK_SIZE;
      src += 2*SALSA20_BLOCK_SIZE;
    }
  _nettle_salsa20_core (x, ctx->input, rounds);
  ctx->input[9] += (++ctx->input[8] == 0);
  memxor3 (dst, src, x, length);
}
#endif

#if !HAVE_NATIVE_salsa20_2core
void
_nettle_salsa20_crypt_1core(struct salsa20_ctx *ctx, unsigned rounds,
			    size_t length,
			    uint8_t *dst,
			    const uint8_t *src)
{
  for (;;)
    {
      uint32_t x[_SALSA20_INPUT_LENGTH];

      _nettle_salsa20_core (x, ctx->input, rounds);

      ctx->input[9] += (++ctx->input[8] == 0);

      /* stopping at 2^70 length per nonce is user's responsibility */

      if (length <= SALSA20_BLOCK_SIZE)
	{
	  memxor3 (dst, src, x, length);
	  return;
	}
      memxor3 (dst, src, x, SALSA20_BLOCK_SIZE);

      length -= SALSA20_BLOCK_SIZE;
      dst += SALSA20_BLOCK_SIZE;
      src += SALSA20_BLOCK_SIZE;
    }
}
#endif
