/* cfb.c

   Cipher feedback mode.

   Copyright (C) 2015, 2017 Dmitry Eremin-Solenikov
   Copyright (C) 2001, 2011 Niels Möller

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
#include <stdlib.h>
#include <string.h>

#include "cfb.h"

#include "memxor.h"
#include "nettle-internal.h"

void
cfb_encrypt(const void *ctx, nettle_cipher_func *f,
	    size_t block_size, uint8_t *iv,
	    size_t length, uint8_t *dst,
	    const uint8_t *src)
{
  uint8_t *p;
  TMP_DECL(buffer, uint8_t, NETTLE_MAX_CIPHER_BLOCK_SIZE);

  TMP_ALLOC(buffer, block_size);

  if (src != dst)
    {
      for (p = iv; length >= block_size; p = dst, dst += block_size, src += block_size, length -= block_size)
	{
	  f(ctx, block_size, dst, p);
	  memxor(dst, src, block_size);
	}
    }
  else
    {
      for (p = iv; length >= block_size; p = dst, dst += block_size, src += block_size, length -= block_size)
	{
	  f(ctx, block_size, buffer, p);
	  memxor(dst, buffer, block_size);
	}
    }

  if (p != iv)
    memcpy(iv, p, block_size);

  if (length)
    {
      f(ctx, block_size, buffer, iv);
      memxor3(dst, buffer, src, length);
      /* We do not care about updating IV here. This is the last call in
       * message sequence and one has to set IV afterwards anyway */
    }
}

/* Don't allocate any more space than this on the stack */
#define CFB_BUFFER_LIMIT 512

void
cfb_decrypt(const void *ctx, nettle_cipher_func *f,
	    size_t block_size, uint8_t *iv,
	    size_t length, uint8_t *dst,
	    const uint8_t *src)
{
  if (src != dst)
    {
      size_t left = length % block_size;

      length -= left;
      if (length > 0)
	{
	  /* Decrypt in ECB mode */
	  f(ctx, block_size, dst, iv);
	  f(ctx, length - block_size, dst + block_size, src);
	  memcpy(iv, src + length - block_size, block_size);
	  memxor(dst, src, length);
	}

      if (left > 0)
	{
	  TMP_DECL(buffer, uint8_t, NETTLE_MAX_CIPHER_BLOCK_SIZE);
	  TMP_ALLOC(buffer, block_size);

	  f(ctx, block_size, buffer, iv);
	  memxor3(dst + length, src + length, buffer, left);
	}
    }
  else
    {
      /* For in-place CFB, we decrypt into a temporary buffer of size
       * at most CFB_BUFFER_LIMIT, and process that amount of data at
       * a time. */

      /* NOTE: We assume that block_size <= CFB_BUFFER_LIMIT */

      TMP_DECL(buffer, uint8_t, CFB_BUFFER_LIMIT);
      TMP_DECL(initial_iv, uint8_t, NETTLE_MAX_CIPHER_BLOCK_SIZE);

      size_t buffer_size;
      size_t left;

      buffer_size = CFB_BUFFER_LIMIT - (CFB_BUFFER_LIMIT % block_size);

      TMP_ALLOC(buffer, buffer_size);
      TMP_ALLOC(initial_iv, block_size);

      left = length % block_size;
      length -= left;

      while (length > 0)
	{
	  size_t part = length > buffer_size ? buffer_size : length;

	  /* length is greater that zero and is divided by block_size, so it is
	   * not less than block_size. So does part */

	  f(ctx, block_size, buffer, iv);
	  f(ctx, part - block_size, buffer + block_size, src);
	  memcpy(iv, src + part - block_size, block_size);
	  memxor(dst, buffer, part);

	  length -= part;
	  src += part;
	  dst += part;
	}

      if (left > 0)
	{
	  f(ctx, block_size, buffer, iv);
	  memxor(dst, buffer, left);
	}
    }
}
