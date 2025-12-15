/* sha3.c

   The sha3 hash function.

   Copyright (C) 2012 Niels Möller

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

#include "macros.h"
#include "md-internal.h"
#include "memxor.h"

#if WORDS_BIGENDIAN
static void
sha3_xor_block (struct sha3_state *state, unsigned length, const uint8_t *data)
{
  assert ( (length & 7) == 0);
  {    
    uint64_t *p;
    for (p = state->a; length > 0; p++, length -= 8, data += 8)
      *p ^= LE_READ_UINT64 (data);
  }
}
#else /* !WORDS_BIGENDIAN */
#define sha3_xor_block(state, length, data) memxor (state->a, data, length)
#endif

static void
sha3_absorb (struct sha3_state *state, unsigned length, const uint8_t *data)
{
  sha3_xor_block (state, length, data);
  sha3_permute (state);
}

unsigned
_nettle_sha3_update (struct sha3_state *state,
		     unsigned block_size, uint8_t *block,
		     unsigned pos,
		     size_t length, const uint8_t *data)
{
  assert (pos < block_size);

  if (!length)
    return pos;

  if (pos > 0)
    {
      MD_FILL_OR_RETURN_INDEX (block_size, block, pos, length, data);
      sha3_absorb (state, block_size, block);
    }
  for (; length >= block_size; length -= block_size, data += block_size)
    sha3_absorb (state, block_size, data);

  memcpy (block, data, length);
  return length;
}

void
_nettle_sha3_pad (struct sha3_state *state,
		  unsigned block_size, uint8_t *block, unsigned pos, uint8_t magic)
{
  assert (pos < block_size);
  block[pos++] = magic;

  memset (block + pos, 0, block_size - pos);
  block[block_size - 1] |= 0x80;

  sha3_xor_block (state, block_size, block);
}
