/* ecc-gostdsa-sign.c

   Copyright (C) 2015 Dmitry Eremin-Solenikov
   Copyright (C) 2013, 2014 Niels Möller

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

#include "gostdsa.h"
#include "ecc-internal.h"

/* Low-level GOST DSA signing */

mp_size_t
ecc_gostdsa_sign_itch (const struct ecc_curve *ecc)
{
  /* Needs 3*ecc->p.size + scratch for ecc->mul_g. Currently same for
     ecc_mul_g. */
  return ECC_GOSTDSA_SIGN_ITCH (ecc->p.size);
}

/* NOTE: Caller should check if r or s is zero. */
void
ecc_gostdsa_sign (const struct ecc_curve *ecc,
		const mp_limb_t *zp,
		const mp_limb_t *kp,
		size_t length, const uint8_t *digest,
		mp_limb_t *rp, mp_limb_t *sp,
		mp_limb_t *scratch)
{
#define P	    scratch
#define hp	    (scratch + 4*ecc->p.size)
#define tp	    (scratch + 2*ecc->p.size)
#define t2p	    scratch
  /* Procedure, according to GOST 34.10. q denotes the group
     order.

     1. k <-- uniformly random, 0 < k < q

     2. C <-- (c_x, c_y) = k g

     3. r <-- c_x mod q

     4. s <-- (r*z + k*h) mod q.
  */

  ecc->mul_g (ecc, P, kp, P + 3*ecc->p.size);
  /* x coordinate only, modulo q */
  ecc->h_to_a (ecc, 2, rp, P, P + 3*ecc->p.size);

  /* Process hash digest */
  gost_hash (&ecc->q, hp, length, digest);
  if (mpn_zero_p (hp, ecc->p.size))
    mpn_add_1 (hp, hp, ecc->p.size, 1);

  ecc_mod_mul (&ecc->q, tp, rp, zp, tp);
  ecc_mod_mul (&ecc->q, t2p, kp, hp, t2p);
  ecc_mod_add (&ecc->q, sp, tp, t2p);

  /* Also reduce mod ecc->q. It should already be < 2*ecc->q,
   * so one subtraction should suffice. */

  *scratch = mpn_sub_n (tp, sp, ecc->q.m, ecc->p.size);
  cnd_copy (*scratch == 0, sp, tp, ecc->p.size);

#undef P
#undef hp
#undef tp
#undef t2p
}
