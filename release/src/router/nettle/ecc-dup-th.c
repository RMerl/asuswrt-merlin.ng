/* ecc-dup-th.c

   Copyright (C) 2014, 2019 Niels Möller

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

/* Double a point on a twisted Edwards curve, in homogeneous coordinates */
void
ecc_dup_th (const struct ecc_curve *ecc,
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
     http://www.hyperelliptic.org/EFD/g1p/auto-twisted-projective.html#doubling-dbl-2008-bbjlp):

     B = (X1+Y1)^2
     C = X1^2
     D = Y1^2
     (E = a*C = -C)
     F = E+D
     H = Z1^2
     J = F-2*H
     X3 = (B-C-D)*J
     Y3 = F*(E-D)
     Z3 = F*J         (-C+D)*(-C+D - 2Z1^2)

     In the formula for Y3, we have E - D = -(C+D). To avoid explicit
     negation, negate all of X3, Y3, Z3, and use

     Computation	Operation	Live variables

     B = (X1+Y1)^2	sqr		B
     C = X1^2		sqr		B, C
     D = Y1^2		sqr		B, C, D
     F = -C+D				B, C, D, F
     H = Z1^2		sqr		B, C, D, F, H
     J = 2*H - F			B, C, D, F, J
     X3 = (B-C-D)*J	mul		C, F, J  (Replace C <-- C+D)
     Y3 = F*(C+D)	mul		F, J
     Z3 = F*J		mul

     3M+4S
  */

#define C scratch
#define D (scratch + 1*ecc->p.size)
#define B (scratch + 2*ecc->p.size)

#define F C

  ecc_mod_sqr (&ecc->p, C, x1, C);	/* C */
  ecc_mod_sqr (&ecc->p, D, y1, D);	/* C, D */
  ecc_mod_add (&ecc->p, B, x1, y1);
  ecc_mod_sqr (&ecc->p, B, B, x2);	/* C, D, B */

  /* C+D stored at y' */
  ecc_mod_add (&ecc->p, y2, C, D);
  /* B - C - C stored at x' */
  ecc_mod_sub (&ecc->p, x2, B, y2);

  ecc_mod_sub (&ecc->p, F, D, C);	/* F */

  /* Use D as scratch for the following multiplies. */
  ecc_mod_mul (&ecc->p, y2, y2, F, D);

  /* H and J stored at z' */
  ecc_mod_sqr (&ecc->p, z2, z1, D);
  ecc_mod_add (&ecc->p, z2, z2, z2);
  ecc_mod_sub (&ecc->p, z2, z2, F);
  ecc_mod_mul (&ecc->p, x2, x2, z2, D);
  ecc_mod_mul (&ecc->p, z2, z2, F, D);
}
