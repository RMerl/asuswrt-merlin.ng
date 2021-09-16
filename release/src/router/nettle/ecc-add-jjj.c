/* ecc-add-jjj.c

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
ecc_add_jjj (const struct ecc_curve *ecc,
	     mp_limb_t *r, const mp_limb_t *p, const mp_limb_t *q,
	     mp_limb_t *scratch)
{
#define x1 p
#define y1 (p + ecc->p.size)
#define z1 (p + 2*ecc->p.size)

#define x2 q
#define y2 (q + ecc->p.size)
#define z2 (q + 2*ecc->p.size)

#define x3 r
#define y3 (r + ecc->p.size)
#define z3 (r + 2*ecc->p.size)
  /* Formulas, from djb,
     http://www.hyperelliptic.org/EFD/g1p/auto-shortw-jacobian-3.html#addition-add-2007-bl:

     Computation		Operation	Live variables

      Z1Z1 = Z1^2		sqr		Z1Z1
      Z2Z2 = Z2^2		sqr		Z1Z1, Z2Z2
      U1 = X1*Z2Z2		mul		Z1Z1, Z2Z2, U1
      U2 = X2*Z1Z1		mul		Z1Z1, Z2Z2, U1, U2
      H = U2-U1					Z1Z1, Z2Z2, U1, H
      Z3 = ((Z1+Z2)^2-Z1Z1-Z2Z2)*H sqr, mul	Z1Z1, Z2Z2, U1, H
      S1 = Y1*Z2*Z2Z2		mul, mul	Z1Z1, U1, H, S1
      S2 = Y2*Z1*Z1Z1		mul, mul	U1, H, S1, S2
      W = 2*(S2-S1)	(djb: r)		U1, H, S1, W
      I = (2*H)^2		sqr		U1, H, S1, W, I
      J = H*I			mul		U1, S1, W, J, V
      V = U1*I			mul		S1, W, J, V
      X3 = W^2-J-2*V		sqr		S1, W, J, V
      Y3 = W*(V-X3)-2*S1*J	mul, mul
  */

#define h scratch
#define z1z1 (scratch + ecc->p.size)
#define z2z2 z1z1
#define z1z2 (scratch + 2*ecc->p.size)

#define w (scratch + ecc->p.size)
#define i (scratch + 2*ecc->p.size)
#define j h
#define v i

#define tp  (scratch + 3*ecc->p.size)

  ecc_mod_sqr (&ecc->p, z2z2, z2, tp);		/* z2z2 */
  /* Store u1 at x3 */
  ecc_mod_mul (&ecc->p, x3, x1, z2z2, tp);	/* z2z2 */

  ecc_mod_add (&ecc->p, z1z2, z1, z2);		/* z2z2, z1z2 */
  ecc_mod_sqr (&ecc->p, z1z2, z1z2, tp);
  ecc_mod_sub (&ecc->p, z1z2, z1z2, z2z2);	/* z2z2, z1z2 */

  /* Do s1 early, store at y3 */
  ecc_mod_mul (&ecc->p, z2z2, z2z2, z2, tp);	/* z2z2, z1z2 */
  ecc_mod_mul (&ecc->p, y3, z2z2, y1, tp);	/* z1z2 */

  ecc_mod_sqr (&ecc->p, z1z1, z1, tp);		/* z1z1, z1z2 */
  ecc_mod_sub (&ecc->p, z1z2, z1z2, z1z1);
  ecc_mod_mul (&ecc->p, h, x2, z1z1, tp);	/* z1z1, z1z2, h */
  ecc_mod_sub (&ecc->p, h, h, x3);

  /* z1^3 */
  ecc_mod_mul (&ecc->p, z1z1, z1z1, z1, tp);

  /* z3 <-- h z1 z2 delayed until now, since that may clobber z1. */
  ecc_mod_mul (&ecc->p, z3, z1z2, h, tp);	/* z1z1, h */
  /* w = 2 (s2 - s1) */
  ecc_mod_mul (&ecc->p, w, z1z1, y2, tp);	/* h, w */
  ecc_mod_sub (&ecc->p, w, w, y3);
  ecc_mod_add (&ecc->p, w, w, w);

  /* i = (2h)^2 */
  ecc_mod_add (&ecc->p, i, h, h);		/* h, w, i */
  ecc_mod_sqr (&ecc->p, i, i, tp);

  /* j and h can overlap */
  ecc_mod_mul (&ecc->p, j, h, i, tp);		/* j, w, i */

  /* v and i can overlap */
  ecc_mod_mul (&ecc->p, v, x3, i, tp);		/* j, w, v */

  /* x3 <-- w^2 - j - 2v */
  ecc_mod_sqr (&ecc->p, x3, w, tp);
  ecc_mod_sub (&ecc->p, x3, x3, j);
  ecc_mod_submul_1 (&ecc->p, x3, v, 2);

  /* y3 <-- w (v - x3) - 2 s1 j */
  ecc_mod_mul (&ecc->p, j, j, y3, tp);
  ecc_mod_sub (&ecc->p, v, v, x3);
  ecc_mod_mul (&ecc->p, y3, v, w, tp);
  ecc_mod_submul_1 (&ecc->p, y3, j, 2);
}
