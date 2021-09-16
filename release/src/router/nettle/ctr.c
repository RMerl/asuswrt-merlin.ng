/* ctr.c

   Cipher counter mode.

   Copyright (C) 2005 Niels Möller

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

#include "ctr.h"

#include "ctr-internal.h"
#include "macros.h"
#include "memxor.h"
#include "nettle-internal.h"

#define MIN(a,b) (((a) < (b)) ? (a) : (b))

static size_t
ctr_fill (size_t block_size, uint8_t *ctr, size_t length, uint8_t *buffer)
{
  size_t i;
  for (i = 0; i + block_size <= length; i += block_size)
    {
      memcpy (buffer + i, ctr, block_size);
      INCREMENT(block_size, ctr);
    }
  return i;
}

#if WORDS_BIGENDIAN
# define USE_CTR_CRYPT16 1
static nettle_fill16_func ctr_fill16;
static void
ctr_fill16(uint8_t *ctr, size_t blocks, union nettle_block16 *buffer)
{
  uint64_t hi, lo;
  size_t i;
  hi = READ_UINT64(ctr);
  lo = READ_UINT64(ctr + 8);

  for (i = 0; i < blocks; i++)
    {
      buffer[i].u64[0] = hi;
      buffer[i].u64[1] = lo;
      hi += !(++lo);
    }
  WRITE_UINT64(ctr, hi);
  WRITE_UINT64(ctr + 8, lo);
}
#else /* !WORDS_BIGENDIAN */
# if HAVE_BUILTIN_BSWAP64
#  define USE_CTR_CRYPT16 1
static nettle_fill16_func ctr_fill16;
static void
ctr_fill16(uint8_t *ctr, size_t blocks, union nettle_block16 *buffer)
{
  uint64_t hi, lo;
  size_t i;
  /* Read hi in native endianness */
  hi = LE_READ_UINT64(ctr);
  lo = READ_UINT64(ctr + 8);

  for (i = 0; i < blocks; i++)
    {
      buffer[i].u64[0] = hi;
      buffer[i].u64[1] = __builtin_bswap64(lo);
      if (!++lo)
	hi = __builtin_bswap64(__builtin_bswap64(hi) + 1);
    }
  LE_WRITE_UINT64(ctr, hi);
  WRITE_UINT64(ctr + 8, lo);
}
# else /* ! HAVE_BUILTIN_BSWAP64 */
#  define USE_CTR_CRYPT16 0
# endif
#endif /* !WORDS_BIGENDIAN */

void
ctr_crypt(const void *ctx, nettle_cipher_func *f,
	  size_t block_size, uint8_t *ctr,
	  size_t length, uint8_t *dst,
	  const uint8_t *src)
{
#if USE_CTR_CRYPT16
  if (block_size == 16)
    {
      _nettle_ctr_crypt16(ctx, f, ctr_fill16, ctr, length, dst, src);
      return;
    }
#endif

  if(src != dst)
    {
      size_t filled = ctr_fill (block_size, ctr, length, dst);

      f(ctx, filled, dst, dst);
      memxor(dst, src, filled);

      if (filled < length)
	{
	  TMP_DECL(block, uint8_t, NETTLE_MAX_CIPHER_BLOCK_SIZE);
	  TMP_ALLOC(block, block_size);

	  f(ctx, block_size, block, ctr);
	  INCREMENT(block_size, ctr);
	  memxor3(dst + filled, src + filled, block, length - filled);
	}
    }
  else
    {
      /* For in-place CTR, construct a buffer of consecutive counter
	 values, of size at most CTR_BUFFER_LIMIT. */
      TMP_DECL(buffer, uint8_t, CTR_BUFFER_LIMIT);

      size_t buffer_size;
      if (length < block_size)
	buffer_size = block_size;
      else if (length <= CTR_BUFFER_LIMIT)
	buffer_size = length;
      else
	buffer_size = CTR_BUFFER_LIMIT;

      TMP_ALLOC(buffer, buffer_size);

      while (length >= block_size)
	{
	  size_t filled
	    = ctr_fill (block_size, ctr, MIN(buffer_size, length), buffer);
	  assert (filled > 0);
	  f(ctx, filled, buffer, buffer);
	  memxor(dst, buffer, filled);
	  length -= filled;
	  dst += filled;
	}

      /* Final, possibly partial, block. */
      if (length > 0)
	{
	  f(ctx, block_size, buffer, ctr);
	  INCREMENT(block_size, ctr);
	  memxor(dst, buffer, length);
	}
    }
}
