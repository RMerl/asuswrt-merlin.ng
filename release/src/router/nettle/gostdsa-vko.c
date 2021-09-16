/* gostdsa-vko.c

   Copyright (C) 2016 Dmitry Eremin-Solenikov

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

#include "ecc-internal.h"
#include "gostdsa.h"

/*
 * Shared key derivation/key agreement for GOST DSA algorithm.
 * It is defined in RFC 4357 Section 5.2 and RFC 7836 Section 4.3.1
 *
 * output is 2 * curve size:
 * 64 bytes for 256 bit curves and 128 bytes for 512 bit ones
 *
 * Basically shared key is equal to hash(cofactor * ukm * priv * pub). This
 * function does multiplication. Caller should do hashing on his own.
 *
 * UKM is not a secret value (consider it as a nonce).
 *
 * For supported GOST curves cofactor is equal to 1.
 */
void
gostdsa_vko (const struct ecc_scalar *priv,
		const struct ecc_point *pub,
		size_t ukm_length, const uint8_t *ukm,
		uint8_t *out)
{
  const struct ecc_curve *ecc = priv->ecc;
  unsigned bsize = (ecc_bit_size (ecc) + 7) / 8;
  mp_size_t size = ecc->p.size;
  mp_size_t itch = 4*size + ecc->mul_itch;
  mp_limb_t *scratch;

  if (itch < 5*size + ecc->h_to_a_itch)
      itch = 5*size + ecc->h_to_a_itch;

  assert (pub->ecc == ecc);
  assert (priv->ecc == ecc);
  assert (ukm_length <= bsize);

  scratch = gmp_alloc_limbs (itch);

#define UKM scratch
#define TEMP (scratch + 3*size)
#define XYZ scratch
#define TEMP_Y (scratch + 4*size)

  mpn_set_base256_le (UKM, size, ukm, ukm_length);

  /* If ukm is 0, set it to 1, otherwise the result will be allways equal to 0,
   * no matter what private and public keys are. See RFC 4357 referencing GOST
   * R 34.10-2001 (RFC 5832) Section 6.1 step 2. */
  if (mpn_zero_p (UKM, size))
    UKM[0] = 1;

  ecc_mod_mul_canonical (&ecc->q, TEMP, priv->p, UKM, TEMP); /* TEMP = UKM * priv */
  ecc->mul (ecc, XYZ, TEMP, pub->p, scratch + 4*size); /* XYZ = UKM * priv * pub */
  ecc->h_to_a (ecc, 0, TEMP, XYZ, scratch + 5*size); /* TEMP = XYZ */
  mpn_get_base256_le (out, bsize, TEMP, size);
  mpn_get_base256_le (out+bsize, bsize, TEMP_Y, size);
  gmp_free_limbs (scratch, itch);
}
