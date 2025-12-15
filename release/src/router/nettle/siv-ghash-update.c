/* siv-ghash-update.c

   POLYVAL implementation for AES-GCM-SIV, based on GHASH

   Copyright (C) 2011 Katholieke Universiteit Leuven
   Copyright (C) 2011, 2013, 2018, 2022 Niels Möller
   Copyright (C) 2018, 2022 Red Hat, Inc.

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

#include "ghash-internal.h"
#include "block-internal.h"
#include "macros.h"

const uint8_t *
_siv_ghash_update (const struct gcm_key *ctx, union nettle_block16 *state,
		 size_t blocks, const uint8_t *data)
{
  for (; blocks-- > 0; data += GCM_BLOCK_SIZE)
    {
      union nettle_block16 b;

#if WORDS_BIGENDIAN
      b.u64[1] = LE_READ_UINT64(data);
      b.u64[0] = LE_READ_UINT64(data + 8);
#else
      b.u64[1] = READ_UINT64(data);
      b.u64[0] = READ_UINT64(data + 8);
#endif

      _ghash_update (ctx, state, 1, b.b);
    }

  return data;
}

