/* ecc-mod.c

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

#include <assert.h>

#include "ecc-internal.h"

/* Computes r <-- x mod m, input 2*m->size, output m->size. It's
 * allowed to have rp == xp or rp == xp + m->size, but no other kind
 * of overlap is allowed. */
void
ecc_mod (const struct ecc_modulo *m, mp_limb_t *rp, mp_limb_t *xp)
{
  mp_limb_t hi;
  mp_size_t mn = m->size;
  mp_size_t bn = m->B_size;
  mp_size_t sn = mn - bn;
  mp_size_t rn = 2*mn;
  mp_size_t i;
  unsigned shift;

  assert (bn < mn);

  /* FIXME: Could use mpn_addmul_2. */
  /* Eliminate sn limbs at a time */
  if (m->B[bn-1] < ((mp_limb_t) 1 << (GMP_NUMB_BITS - 1)))
    {
      /* Multiply sn + 1 limbs at a time, so we get a mn+1 limb
	 product. Then we can absorb the carry in the high limb */
      while (rn > 2 * mn - bn)
	{
	  rn -= sn;

	  for (i = 0; i <= sn; i++)
	    xp[rn+i-1] = mpn_addmul_1 (xp + rn - mn - 1 + i, m->B, bn, xp[rn+i-1]);
	  xp[rn-1] = xp[rn+sn-1]
	    + mpn_add_n (xp + rn - sn - 1, xp + rn - sn - 1, xp + rn - 1, sn);
	}
    }
  else
    {
      while (rn > 2 * mn - bn)
	{
	  rn -= sn;

	  for (i = 0; i < sn; i++)
	    xp[rn+i] = mpn_addmul_1 (xp + rn - mn + i, m->B, bn, xp[rn+i]);
				     
	  hi = mpn_add_n (xp + rn - sn, xp + rn - sn, xp + rn, sn);
	  hi = mpn_cnd_add_n (hi, xp + rn - mn, xp + rn - mn, m->B, mn);
	  assert (hi == 0);
	}
    }

  assert (rn > mn);
  rn -= mn;
  assert (rn <= sn);

  for (i = 0; i < rn; i++)
    xp[mn+i] = mpn_addmul_1 (xp + i, m->B, bn, xp[mn+i]);

  hi = mpn_add_n (xp + bn, xp + bn, xp + mn, rn);
  if (rn < sn)
    hi = sec_add_1 (xp + bn + rn, xp + bn + rn, sn - rn, hi);

  shift = m->size * GMP_NUMB_BITS - m->bit_size;
  if (shift > 0)
    {
      /* Combine hi with top bits, add in */
      hi = (hi << shift) | (xp[mn-1] >> (GMP_NUMB_BITS - shift));
      xp[mn-1] = (xp[mn-1] & (((mp_limb_t) 1 << (GMP_NUMB_BITS - shift)) - 1))
	+ mpn_addmul_1 (xp, m->B_shifted, mn-1, hi);
      /* FIXME: Can this copying be eliminated? */
      if (rp != xp)
	mpn_copyi (rp, xp, mn);
    }
  else
    {
      hi = mpn_cnd_add_n (hi, rp, xp, m->B, mn);
      assert (hi == 0);
    }
}
