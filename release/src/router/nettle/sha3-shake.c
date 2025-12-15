/* sha3-shake.c

   Copyright (C) 2017, 2024 Daiki Ueno
   Copyright (C) 2017 Red Hat, Inc.
   Copyright (C) 2024 Niels Möller

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
#include <string.h>

#include "sha3.h"
#include "sha3-internal.h"

#include "nettle-write.h"

void
_nettle_sha3_shake (struct sha3_state *state,
		    unsigned block_size, uint8_t *block,
		    unsigned index,
		    size_t length, uint8_t *dst)
{
  _sha3_pad_shake (state, block_size, block, index);

  while (length > block_size)
    {
      sha3_permute (state);
      _nettle_write_le64 (block_size, dst, state->a);
      length -= block_size;
      dst += block_size;
    }

  sha3_permute (state);
  _nettle_write_le64 (length, dst, state->a);
}

unsigned
_nettle_sha3_shake_output (struct sha3_state *state,
			   unsigned block_size, uint8_t *block,
			   unsigned index,
			   size_t length, uint8_t *dst)
{
  unsigned left;

  /* We use one's complement of the index value to indicate SHAKE is
     initialized. */
  if (index < block_size)
    {
      /* This is the first call of _shake_output.  */
      _sha3_pad_shake (state, block_size, block, index);
      /* Point at the end of block to trigger fill in of the buffer.  */
      index = block_size;
    }
  else
    index = ~index;

  assert (index <= block_size);

  /* Write remaining data from the buffer.  */
  left = block_size - index;
  if (length <= left)
    {
      memcpy (dst, block + index, length);
      return ~(index + length);
    }
  else
    {
      memcpy (dst, block + index, left);
      length -= left;
      dst += left;
    }

  /* Write full blocks.  */
  while (length > block_size)
    {
      sha3_permute (state);
      _nettle_write_le64 (block_size, dst, state->a);
      length -= block_size;
      dst += block_size;
    }

  sha3_permute (state);
  /* Fill in the buffer for next call.  */
  _nettle_write_le64 (block_size, block, state->a);
  memcpy (dst, block, length);
  return ~length;
}
