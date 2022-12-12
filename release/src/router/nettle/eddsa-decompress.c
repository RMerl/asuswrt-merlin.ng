/* eddsa-decompress.c

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

#include "eddsa.h"
#include "eddsa-internal.h"

#include "ecc-internal.h"
#include "gmp-glue.h"

mp_size_t
_eddsa_decompress_itch (const struct ecc_curve *ecc)
{
  return 4*ecc->p.size + ecc->p.sqrt_ratio_itch;
}

int
_eddsa_decompress (const struct ecc_curve *ecc, mp_limb_t *p,
		   const uint8_t *cp,
		   mp_limb_t *scratch)
{
  mp_limb_t sign, cy;
  mp_size_t nlimbs;
  size_t nbytes;
  int res;

#define xp p
#define yp (p + ecc->p.size)

#define y2 scratch
#define vp (scratch + ecc->p.size)
#define up scratch
#define tp (scratch + 2*ecc->p.size)
#define scratch_out (scratch + 4*ecc->p.size)

  nbytes = 1 + ecc->p.bit_size / 8;
  /* By RFC 8032, sign bit is always the most significant bit of the
     last byte. */
  sign = cp[nbytes-1] >> 7;

  /* May need an extra limb. */
  nlimbs = (nbytes * 8 + GMP_NUMB_BITS - 1) / GMP_NUMB_BITS;
  assert (nlimbs <= ecc->p.size + 1);
  mpn_set_base256_le (scratch, nlimbs, cp, nbytes);

  /* Clear out the sign bit */
  scratch[nlimbs - 1] &=
    ((mp_limb_t) 1 << ((nbytes * 8 - 1) % GMP_NUMB_BITS)) - 1;
  mpn_copyi (yp, scratch, ecc->p.size);

  /* Check range. */
  if (nlimbs > ecc->p.size)
    res = (scratch[nlimbs - 1] == 0);
  else
    res = 1;

  /* For a valid input, y < p, so subtraction should underflow. */
  res &= mpn_sub_n (scratch, scratch, ecc->p.m, ecc->p.size);

  ecc_mod_sqr (&ecc->p, y2, yp, y2);
  ecc_mod_mul (&ecc->p, vp, y2, ecc->b, vp);
  ecc_mod_sub (&ecc->p, vp, vp, ecc->unit);
  /* The sign is different between curve25519 and curve448.  */
  if (ecc->p.bit_size == 255)
    ecc_mod_sub (&ecc->p, up, ecc->unit, y2);
  else
    ecc_mod_sub (&ecc->p, up, y2, ecc->unit);
  res &= ecc->p.sqrt_ratio (&ecc->p, tp, up, vp, scratch_out);

  cy = mpn_sub_n (xp, tp, ecc->p.m, ecc->p.size);
  cnd_copy (cy, xp, tp, ecc->p.size);
  sign ^= xp[0] & 1;
  mpn_sub_n (tp, ecc->p.m, xp, ecc->p.size);
  cnd_copy (sign, xp, tp, ecc->p.size);
  /* Fails if the square root is zero but (original) sign was 1 */
  res &= mpn_sub_n (tp, xp, ecc->p.m, ecc->p.size);
  return res;
}
