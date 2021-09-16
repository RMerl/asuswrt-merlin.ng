/* ecc-dup-jj.c

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

/* NOTE: Behaviour for corner cases:

   + p = 0  ==>  r = 0, correct!
*/
void
ecc_dup_jj (const struct ecc_curve *ecc,
	    mp_limb_t *r, const mp_limb_t *p,
	    mp_limb_t *scratch)
{
#define x1 p
#define y1 (p + ecc->p.size)
#define z1 (p + 2*ecc->p.size)

#define x2 r
#define y2 (r + ecc->p.size)
#define z2 (r + 2*ecc->p.size)

  /* Formulas (from djb,
     http://www.hyperelliptic.org/EFD/g1p/auto-shortw-jacobian-3.html#doubling-dbl-2001-b):

     Computation			Operation	Live variables
     delta = z^2			sqr		delta
     gamma = y^2			sqr		delta, gamma
     z' = (y+z)^2-gamma-delta		sqr		delta, gamma
     alpha = 3*(x-delta)*(x+delta)	mul		gamma, beta, alpha
     beta = x*gamma			mul		gamma, beta, alpha
     x' = alpha^2-8*beta		sqr		gamma, beta, alpha
     y' = alpha*(4*beta-x')-8*gamma^2	mul, sqr
  */

#define gamma scratch
#define delta (scratch + ecc->p.size)
#define alpha delta

#define beta (scratch + 2*ecc->p.size)
#define sum  (scratch + 3*ecc->p.size)

  ecc_mod_sqr (&ecc->p, gamma, y1, gamma);	/* x, y, z, gamma */
  ecc_mod_sqr (&ecc->p, delta, z1, delta);	/* x, y, z, gamma, delta */

  ecc_mod_add (&ecc->p, sum, z1, y1);		/* x, gamma, delta, s */
  ecc_mod_sqr (&ecc->p, sum, sum, y2);		/* Can use y-z as scratch */
  ecc_mod_sub (&ecc->p, z2, sum, delta);	/* x, z, gamma, delta */
  ecc_mod_sub (&ecc->p, z2, z2, gamma);

  ecc_mod_mul (&ecc->p, beta, x1, gamma, beta);	/* x, z, gamma, delta, beta */

  ecc_mod_add (&ecc->p, sum, x1, delta);	/* x, sum, z', gamma, delta, beta */
  ecc_mod_sub (&ecc->p, delta, x1, delta);	/* sum, z', gamma, delta, beta */
  /* This multiplication peaks the storage need; can use x-y for scratch. */
  ecc_mod_mul (&ecc->p, alpha, sum, delta, x2);	/* z', gamma, alpha, beta */
  ecc_mod_mul_1 (&ecc->p, alpha, alpha, 3);

  ecc_mod_mul_1 (&ecc->p, y2, beta, 4);

  /* From now on, can use beta as scratch. */
  ecc_mod_sqr (&ecc->p, x2, alpha, beta);	/* alpha^2 */
  ecc_mod_submul_1 (&ecc->p, x2, y2, 2);	/*  alpha^2 - 8 beta */

  ecc_mod_sub (&ecc->p, y2, y2, x2);		/* 4 beta - x' */
  ecc_mod_mul (&ecc->p, y2, y2, alpha, beta);
  ecc_mod_sqr (&ecc->p, gamma, gamma, beta);
  ecc_mod_submul_1 (&ecc->p, y2, gamma, 8);
}
