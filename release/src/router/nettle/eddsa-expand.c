/* eddsa-expand.c

   Copyright (C) 2014 Niels Möller

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

#include "eddsa.h"
#include "eddsa-internal.h"

#include "ecc.h"
#include "ecc-internal.h"

/* Expands a private key, generating the secret scalar K2 and leaving
   the key K1 for nonce generation, at the end of the digest. */
void
_eddsa_expand_key (const struct ecc_curve *ecc,
		   const struct ecc_eddsa *eddsa,
		   void *ctx,
		   const uint8_t *key,
		   uint8_t *digest,
		   mp_limb_t *k2)
{
  size_t nbytes = 1 + ecc->p.bit_size / 8;

  eddsa->update (ctx, nbytes, key);
  eddsa->digest (ctx, 2*nbytes, digest);

  /* For ed448, ignores the most significant byte. */
  mpn_set_base256_le (k2, ecc->p.size, digest, (ecc->p.bit_size + 7) / 8);

  /* Clear low c bits */
  k2[0] &= eddsa->low_mask;

  /* Clear higher bits. */
  k2[ecc->p.size - 1] &= eddsa->high_bit - 1;

  /* Set bit number bit_size - 1 (bit 254 for curve25519, bit 447 for
     curve448) */
  k2[ecc->p.size - 1] |= eddsa->high_bit;
}
