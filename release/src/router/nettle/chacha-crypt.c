/* chacha-crypt.c

   The crypt function in the ChaCha stream cipher.
   Heavily based on the Salsa20 implementation in Nettle.

   Copyright (C) 2014 Niels Möller
   Copyright (C) 2013 Joachim Strömbergson
   Copyright (C) 2012 Simon Josefsson

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

/* Based on:
   chacha-ref.c version 2008.01.20.
   D. J. Bernstein
   Public domain.
*/

#if HAVE_CONFIG_H
# include "config.h"
#endif

#include <string.h>

#include "chacha.h"
#include "chacha-internal.h"

#include "macros.h"
#include "memxor.h"

#define CHACHA_ROUNDS 20

#if HAVE_NATIVE_chacha_4core
#define _nettle_chacha_crypt_4core chacha_crypt
#define _nettle_chacha_crypt32_4core chacha_crypt32
#elif HAVE_NATIVE_chacha_3core
#define _nettle_chacha_crypt_3core chacha_crypt
#define _nettle_chacha_crypt32_3core chacha_crypt32
#elif !(HAVE_NATIVE_fat_chacha_4core || HAVE_NATIVE_fat_chacha_3core)
#define _nettle_chacha_crypt_1core chacha_crypt
#define _nettle_chacha_crypt32_1core chacha_crypt32
#endif

#if HAVE_NATIVE_chacha_4core || HAVE_NATIVE_fat_chacha_4core
void
_nettle_chacha_crypt_4core(struct chacha_ctx *ctx,
			   size_t length,
			   uint8_t *dst,
			   const uint8_t *src)
{
  uint32_t x[4*_CHACHA_STATE_LENGTH];

  if (!length)
    return;

  while (length > 2*CHACHA_BLOCK_SIZE)
    {
      _nettle_chacha_4core (x, ctx->state, CHACHA_ROUNDS);
      if (length <= 4*CHACHA_BLOCK_SIZE)
	{
	  uint32_t incr = 3 + (length > 3*CHACHA_BLOCK_SIZE);
	  ctx->state[12] += incr;
	  ctx->state[13] += (ctx->state[12] < incr);
	  memxor3 (dst, src, x, length);
	  return;
	}
      ctx->state[12] += 4;
      ctx->state[13] += (ctx->state[12] < 4);
      memxor3 (dst, src, x, 4*CHACHA_BLOCK_SIZE);

      length -= 4*CHACHA_BLOCK_SIZE;
      dst += 4*CHACHA_BLOCK_SIZE;
      src += 4*CHACHA_BLOCK_SIZE;
    }
  if (length > CHACHA_BLOCK_SIZE)
    {
      _nettle_chacha_2core (x, ctx->state, CHACHA_ROUNDS);
      ctx->state[12] += 2;
      ctx->state[13] += (ctx->state[12] < 2);
    }
  else
    {
      _nettle_chacha_core (x, ctx->state, CHACHA_ROUNDS);
      ctx->state[13] += (++ctx->state[12] == 0);
    }
  memxor3 (dst, src, x, length);
}
#endif

#if HAVE_NATIVE_chacha_3core || HAVE_NATIVE_fat_chacha_3core
void
_nettle_chacha_crypt_3core(struct chacha_ctx *ctx,
			   size_t length,
			   uint8_t *dst,
			   const uint8_t *src)
{
  uint32_t x[3*_CHACHA_STATE_LENGTH];

  if (!length)
    return;

  while (length > 2*CHACHA_BLOCK_SIZE)
    {
      _nettle_chacha_3core (x, ctx->state, CHACHA_ROUNDS);
      ctx->state[12] += 3;
      ctx->state[13] += (ctx->state[12] < 3);
      if (length <= 3*CHACHA_BLOCK_SIZE)
	{
	  memxor3 (dst, src, x, length);
	  return;
	}
      memxor3 (dst, src, x, 3*CHACHA_BLOCK_SIZE);

      length -= 3*CHACHA_BLOCK_SIZE;
      dst += 3*CHACHA_BLOCK_SIZE;
      src += 3*CHACHA_BLOCK_SIZE;
    }
  if (length <= CHACHA_BLOCK_SIZE)
    {
      _nettle_chacha_core (x, ctx->state, CHACHA_ROUNDS);
      ctx->state[13] += (++ctx->state[12] == 0);
    }
  else
    {
      _nettle_chacha_3core (x, ctx->state, CHACHA_ROUNDS);
      ctx->state[12] += 2;
      ctx->state[13] += (ctx->state[12] < 2);
    }
  memxor3 (dst, src, x, length);
}
#endif

#if !(HAVE_NATIVE_chacha_4core || HAVE_NATIVE_chacha_3core)
void
_nettle_chacha_crypt_1core(struct chacha_ctx *ctx,
			   size_t length,
			   uint8_t *dst,
			   const uint8_t *src)
{
  if (!length)
    return;
  
  for (;;)
    {
      uint32_t x[_CHACHA_STATE_LENGTH];

      _nettle_chacha_core (x, ctx->state, CHACHA_ROUNDS);

      ctx->state[13] += (++ctx->state[12] == 0);

      /* stopping at 2^70 length per nonce is user's responsibility */
      
      if (length <= CHACHA_BLOCK_SIZE)
	{
	  memxor3 (dst, src, x, length);
	  return;
	}
      memxor3 (dst, src, x, CHACHA_BLOCK_SIZE);

      length -= CHACHA_BLOCK_SIZE;
      dst += CHACHA_BLOCK_SIZE;
      src += CHACHA_BLOCK_SIZE;
  }
}
#endif

#if HAVE_NATIVE_chacha_4core || HAVE_NATIVE_fat_chacha_4core
void
_nettle_chacha_crypt32_4core(struct chacha_ctx *ctx,
			     size_t length,
			     uint8_t *dst,
			     const uint8_t *src)
{
  uint32_t x[4*_CHACHA_STATE_LENGTH];

  if (!length)
    return;

  while (length > 2*CHACHA_BLOCK_SIZE)
    {
      _nettle_chacha_4core32 (x, ctx->state, CHACHA_ROUNDS);
      if (length <= 4*CHACHA_BLOCK_SIZE)
	{
	  ctx->state[12] += 3 + (length > 3*CHACHA_BLOCK_SIZE);
	  memxor3 (dst, src, x, length);
	  return;
	}
      ctx->state[12] += 4;
      memxor3 (dst, src, x, 4*CHACHA_BLOCK_SIZE);

      length -= 4*CHACHA_BLOCK_SIZE;
      dst += 4*CHACHA_BLOCK_SIZE;
      src += 4*CHACHA_BLOCK_SIZE;
    }
  if (length > CHACHA_BLOCK_SIZE)
    {
      _nettle_chacha_2core32 (x, ctx->state, CHACHA_ROUNDS);
      ctx->state[12] += 2;
    }
  else
    {
      _nettle_chacha_core (x, ctx->state, CHACHA_ROUNDS);
      ++ctx->state[12];
    }
  memxor3 (dst, src, x, length);
}
#endif

#if HAVE_NATIVE_chacha_3core || HAVE_NATIVE_fat_chacha_3core
void
_nettle_chacha_crypt32_3core(struct chacha_ctx *ctx,
			     size_t length,
			     uint8_t *dst,
			     const uint8_t *src)
{
  uint32_t x[3*_CHACHA_STATE_LENGTH];

  if (!length)
    return;

  while (length > 2*CHACHA_BLOCK_SIZE)
    {
      _nettle_chacha_3core32 (x, ctx->state, CHACHA_ROUNDS);
      ctx->state[12] += 3;
      if (length <= 3*CHACHA_BLOCK_SIZE)
	{
	  memxor3 (dst, src, x, length);
	  return;
	}
      memxor3 (dst, src, x, 3*CHACHA_BLOCK_SIZE);

      length -= 3*CHACHA_BLOCK_SIZE;
      dst += 3*CHACHA_BLOCK_SIZE;
      src += 3*CHACHA_BLOCK_SIZE;
    }
  if (length <= CHACHA_BLOCK_SIZE)
    {
      _nettle_chacha_core (x, ctx->state, CHACHA_ROUNDS);
      ++ctx->state[12];
    }
  else
    {
      _nettle_chacha_3core32 (x, ctx->state, CHACHA_ROUNDS);
      ctx->state[12] += 2;
    }
  memxor3 (dst, src, x, length);
}
#endif

#if !(HAVE_NATIVE_chacha_4core || HAVE_NATIVE_chacha_3core)
void
_nettle_chacha_crypt32_1core(struct chacha_ctx *ctx,
			     size_t length,
			     uint8_t *dst,
			     const uint8_t *src)
{
  if (!length)
    return;

  for (;;)
    {
      uint32_t x[_CHACHA_STATE_LENGTH];

      _nettle_chacha_core (x, ctx->state, CHACHA_ROUNDS);

      ++ctx->state[12];

      /* stopping at 2^38 length per nonce is user's responsibility */

      if (length <= CHACHA_BLOCK_SIZE)
	{
	  memxor3 (dst, src, x, length);
	  return;
	}
      memxor3 (dst, src, x, CHACHA_BLOCK_SIZE);

      length -= CHACHA_BLOCK_SIZE;
      dst += CHACHA_BLOCK_SIZE;
      src += CHACHA_BLOCK_SIZE;
  }
}
#endif
