/* ghash-set-key.c

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

/* For fat builds */
#if HAVE_NATIVE_ghash_set_key
void
_nettle_ghash_set_key_c (struct gcm_key *ctx, const union nettle_block16 *key);
#define _nettle_ghash_set_key _nettle_ghash_set_key_c
#endif

/* Implements a lookup table for processors without carryless-mul
   instruction. */
void
_ghash_set_key (struct gcm_key *ctx, const union nettle_block16 *key)
{
  /* Middle element if GCM_TABLE_BITS > 0, otherwise the first
     element */
  unsigned i = (1<<GCM_TABLE_BITS)/2;
  block16_zero (&ctx->h[0]);
  ctx->h[i] = *key;

  /* Algorithm 3 from the gcm paper. First do powers of two, then do
     the rest by adding. */
  while (i /= 2)
    block16_mulx_ghash (&ctx->h[i], &ctx->h[2*i]);
  for (i = 2; i < 1<<GCM_TABLE_BITS; i *= 2)
    {
      unsigned j;
      for (j = 1; j < i; j++)
	block16_xor3 (&ctx->h[i+j], &ctx->h[i], &ctx->h[j]);
    }
}
