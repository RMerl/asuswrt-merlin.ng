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

#if GCM_TABLE_BITS < 7
# error Unsupported table size.
#endif

/* Implements a lookup table for processors without carryless-mul
   instruction. */
void
_ghash_set_key (struct gcm_key *ctx, const union nettle_block16 *key)
{
  /* Table elements hold the key, premultiplied by all needed powers
     of x. Element ordering follows the order bits are processed in
     _ghash_update, alternating u64[0] and u64[1] bits, starting from
     the least significant end. In the gcm bit order, bits (left to
     right) correspond to x powers (the numbers) like

       |0...7|8...15|...|56...63|64...71|72...79|...|120...127|

     where | indicates the byte boundaries. On little endian, these
     bits are in u64 like

       u64[0]: | 56...63   48...55   40...47  32...39  24...31 16...23  8...15  0...7|
       u64[1]: |120...127 112...129 104...111 96...103 88...95 80...87 72...79 64...71|

     With big-endian, we instead get

       u64[0]:  |0...63|
       u64[1]: |64...127|
  */
#if WORDS_BIGENDIAN
# define INDEX_PERMUTE 63
#else
# define INDEX_PERMUTE 7
#endif
  unsigned i;

  block16_set (&ctx->h[2*INDEX_PERMUTE], key);
  for (i = 1; i < 64; i++)
    block16_mulx_ghash(&ctx->h[2*(i ^ INDEX_PERMUTE)], &ctx->h[2*((i-1) ^ INDEX_PERMUTE)]);

  block16_mulx_ghash(&ctx->h[2*INDEX_PERMUTE + 1], &ctx->h[2*(63^INDEX_PERMUTE)]);
  for (i = 1; i < 64; i++)
    block16_mulx_ghash(&ctx->h[2*(i ^ INDEX_PERMUTE)+1], &ctx->h[2*((i-1) ^ INDEX_PERMUTE)+1]);
}
