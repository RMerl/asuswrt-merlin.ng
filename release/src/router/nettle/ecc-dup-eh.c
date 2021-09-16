/* ecc-dup-eh.c

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

#include "ecc.h"
#include "ecc-internal.h"

/* Double a point on an Edwards curve, in homogeneous coordinates */
void
ecc_dup_eh (const struct ecc_curve *ecc,
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
     http://www.hyperelliptic.org/EFD/g1p/auto-edwards-projective.html#doubling-dbl-2007-bl):

     Computation	Operation	Live variables

     b = (x+y)^2	sqr		b
     c = x^2		sqr		b, c
     d = y^2		sqr		b, c, d
     e = c+d				b, c, d, e
     h = z^2		sqr		b, c, d, e, h
     j = e-2*h				b, c, d, e, j
     x' = (b-e)*j	mul		c, d, e, j
     y' = e*(c-d)	mul		e, j
     z' = e*j		mul
  */
#define C scratch
#define D (scratch + 1*ecc->p.size)
#define B (scratch + 2*ecc->p.size)

#define E C

  ecc_mod_sqr (&ecc->p, C, x1, C);	/* C */
  ecc_mod_sqr (&ecc->p, D, y1, D);	/* C, D */
  ecc_mod_add (&ecc->p, B, x1, y1);
  ecc_mod_sqr (&ecc->p, B, B, x2);	/* C, D, B */

  /* c-d stored at y' */
  ecc_mod_sub (&ecc->p, y2, C, D);
  ecc_mod_add (&ecc->p, E, C, D);	/* B, E */
  /* b-e stored at x' */
  ecc_mod_sub (&ecc->p, x2, B, E);	/* E */

  /* Use D as scratch for the following multiplies. */
  ecc_mod_mul (&ecc->p, y2, y2, E, D);

  /* h and j stored at z' */
  ecc_mod_sqr (&ecc->p, z2, z1, D);
  ecc_mod_add (&ecc->p, z2, z2, z2);
  ecc_mod_sub (&ecc->p, z2, E, z2);
  ecc_mod_mul (&ecc->p, x2, x2, z2, D);
  ecc_mod_mul (&ecc->p, z2, z2, E, D);
}
