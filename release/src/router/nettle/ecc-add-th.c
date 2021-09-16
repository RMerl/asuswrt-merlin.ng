/* ecc-add-th.c

   Copyright (C) 2014, 2017 Niels Möller

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

#include "ecc.h"
#include "ecc-internal.h"

/* Add two points on a twisted Edwards curve, with result and first point in
   homogeneous coordinates. */
void
ecc_add_th (const struct ecc_curve *ecc,
	    mp_limb_t *r, const mp_limb_t *p, const mp_limb_t *q,
	    mp_limb_t *scratch)
{
#define x1 p
#define y1 (p + ecc->p.size)
#define z1 (p + 2*ecc->p.size)

#define x2 q
#define y2 (q + ecc->p.size)

#define x3 r
#define y3 (r + ecc->p.size)
#define z3 (r + 2*ecc->p.size)

  /* Formulas (from djb,
     http://www.hyperelliptic.org/EFD/g1p/auto-twisted-projective.html#addition-madd-2008-bbjlp

     Computation	Operation	Live variables

     C = x1*x2		mul		C
     D = y1*y2		mul		C, D
     T = (x1+y1)*(x2+y2) mul		C, D, T
         - C - D
     E = b*C*D          2 mul		C, E, T  (Replace C <-- D+C)
     B = z1^2		sqr		B, C, E, T
     F = B - E				B, C, E, F, T
     G = B + E     			C, F, G, T
     x3 = z1 * F * T    2 mul		C, F, G, T
     y3 = z1*G*(D+C)	2 mul		F, G
     z3 = F*G		mul

     10M + 1S

     We have different sign for E, hence swapping F and G, because our
     ecc->b corresponds to -b above.
  */
#define T scratch
#define E (scratch + 1*ecc->p.size)
#define F E
#define C (scratch + 2*ecc->p.size)
#define D (scratch + 3*ecc->p.size)
#define B D

  /* Use T as scratch, clobber E */
  ecc_mod_mul (&ecc->p, C, x1, x2, T);	/* C */
  ecc_mod_mul (&ecc->p, D, y1, y2, T);	/* C, D */
  ecc_mod_add (&ecc->p, x3, x1, y1);
  ecc_mod_add (&ecc->p, y3, x2, y2);
  ecc_mod_mul (&ecc->p, T, x3, y3, T);	/* C, D, T */
  ecc_mod_sub (&ecc->p, T, T, C);
  ecc_mod_sub (&ecc->p, T, T, D);
  /* Can now use x3 as scratch, without breaking in-place operation. */
  ecc_mod_mul (&ecc->p, T, T, z1, x3);

  ecc_mod_mul (&ecc->p, E, C, D, x3);	/* C, D, T, E */
  ecc_mod_mul (&ecc->p, E, E, ecc->b, x3);

  ecc_mod_add (&ecc->p, C, D, C);	/* C, T, E */
  ecc_mod_mul (&ecc->p, C, C, z1, x3);

  ecc_mod_sqr (&ecc->p, B, z1, x3);	/* C, T, E, B */
  ecc_mod_add (&ecc->p, x3, B, E);
  ecc_mod_sub (&ecc->p, F, B, E);	/* C, T, F */

  /* Can now use y3 as scratch, without breaking in-place operation. */
  ecc_mod_mul (&ecc->p, y3, C, F, y3);	/* T G */

  /* Can use C--D as scratch */
  ecc_mod_mul (&ecc->p, z3, x3, F, C);	/* T */
  ecc_mod_mul (&ecc->p, x3, x3, T, C);
}
