/* ecc-j-to-a.c

   Copyright (C) 2013 Niels Möller

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

/* Development of Nettle's ECC support was funded by the .SE Internet Fund. */

#if HAVE_CONFIG_H
# include "config.h"
#endif

#include "ecc.h"
#include "ecc-internal.h"

void
ecc_j_to_a (const struct ecc_curve *ecc,
	    int op,
	    mp_limb_t *r, const mp_limb_t *p,
	    mp_limb_t *scratch)
{
#define izp   scratch
#define iz2p (scratch + ecc->p.size)
#define iz3p (scratch + 2*ecc->p.size)
#define tp    scratch

  ecc->p.invert (&ecc->p, izp, p+2*ecc->p.size, izp + ecc->p.size);
  ecc_mod_sqr (&ecc->p, iz2p, izp, iz2p);

  if (ecc->use_redc)
    {
      /* Divide this common factor by B, instead of applying redc to
	 both x and y outputs. */
      mpn_zero (iz2p + ecc->p.size, ecc->p.size);
      ecc->p.reduce (&ecc->p, iz2p, iz2p);
    }

  /* r_x <-- x / z^2 */
  ecc_mod_mul_canonical (&ecc->p, r, iz2p, p, iz3p);
  if (op)
    {
      /* Skip y coordinate */
      if (op > 1)
	{
	  mp_limb_t cy;
	  /* Also reduce the x coordinate mod ecc->q. It should
	     already be < 2*ecc->q, so one subtraction should
	     suffice. */
	  cy = mpn_sub_n (scratch, r, ecc->q.m, ecc->p.size);
	  cnd_copy (cy == 0, r, scratch, ecc->p.size);
	}
      return;
    }
  ecc_mod_mul (&ecc->p, iz3p, iz2p, izp, iz3p);
  ecc_mod_mul_canonical (&ecc->p, r + ecc->p.size, iz3p, p + ecc->p.size, tp);

#undef izp
#undef iz2p
#undef iz3p
#undef tp
}
