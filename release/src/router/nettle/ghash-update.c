/* ghash-update.c

   Galois counter mode, specified by NIST,
   http://csrc.nist.gov/publications/nistpubs/800-38D/SP-800-38D.pdf

   See also the gcm paper at
   http://www.cryptobarn.com/papers/gcm-spec.pdf.

   Copyright (C) 2011 Katholieke Universiteit Leuven
   Copyright (C) 2011, 2013, 2018, 2022 Niels Möller
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

#include "ghash-internal.h"
#include "block-internal.h"

#if GCM_TABLE_BITS < 7
# error Unsupported table size.
#endif

/* For fat builds */
#if HAVE_NATIVE_ghash_update
const uint8_t *
_nettle_ghash_update_c (const struct gcm_key *ctx, union nettle_block16 *state,
			size_t blocks, const uint8_t *data);
#define _nettle_ghash_update _nettle_ghash_update_c
#endif

static void
gcm_gf_mul (union nettle_block16 *x, const union nettle_block16 *table)
{
  uint64_t x0 = x->u64[0];
  uint64_t x1 = x->u64[1];
  uint64_t r0 = 0;
  uint64_t r1 = 0;
  unsigned i;
  for (i = 0; i < 64; i++, x0 >>= 1, x1 >>= 1)
    {
      uint64_t m0 = -(x0 & 1);
      uint64_t m1 = -(x1 & 1);
      r0 ^= m0 & table[2*i].u64[0];
      r1 ^= m0 & table[2*i].u64[1];
      r0 ^= m1 & table[2*i+1].u64[0];
      r1 ^= m1 & table[2*i+1].u64[1];
    }
  x->u64[0] = r0; x->u64[1] = r1;
}

const uint8_t *
_ghash_update (const struct gcm_key *ctx, union nettle_block16 *state,
	       size_t blocks, const uint8_t *data)
{
  for (; blocks-- > 0; data += GCM_BLOCK_SIZE)
    {
      memxor (state->b, data, GCM_BLOCK_SIZE);
      gcm_gf_mul (state, ctx->h);
    }
  return data;
}
