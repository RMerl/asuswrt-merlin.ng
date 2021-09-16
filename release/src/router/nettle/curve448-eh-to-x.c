/* curve448-eh-to-x.c

   Copyright (C) 2017 Daiki Ueno
   Copyright (C) 2017 Red Hat, Inc.

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

#include <string.h>

#include "curve448.h"

#include "ecc.h"
#include "ecc-internal.h"

/* Transform a point on the edwards448 Edwards curve to the curve448
   Montgomery curve, and return the x coordinate. */
void
curve448_eh_to_x (mp_limb_t *xp, const mp_limb_t *p, mp_limb_t *scratch)
{
#define up p
#define vp (p + ecc->p.size)
#define t0 scratch
#define tp (scratch + ecc->p.size)

  const struct ecc_curve *ecc = &_nettle_curve448;

  /* If u = U/W and v = V/W are the coordinates of the point on
     edwards448 we get the curve448 x coordinate as

     x = v^2 / u^2 = (V/W)^2 / (U/W)^2 = (V/U)^2
  */
  /* Needs a total of 5*size storage. */
  ecc->p.invert (&ecc->p, t0, up, tp);
  ecc_mod_mul (&ecc->p, t0, t0, vp, tp);
  ecc_mod_sqr_canonical (&ecc->p, xp, t0, tp);

#undef up
#undef vp
#undef t0
#undef tp
}
