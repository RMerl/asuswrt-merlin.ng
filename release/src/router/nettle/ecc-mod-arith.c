/* ecc-mod-arith.c

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

/* Development of Nettle's ECC support was funded by the .SE Internet Fund. */

#if HAVE_CONFIG_H
# include "config.h"
#endif

#include <assert.h>

#include "ecc-internal.h"

/* Routines for modp arithmetic. All values are ecc->size limbs, but
   not necessarily < p. */

int
ecc_mod_zero_p (const struct ecc_modulo *m, const mp_limb_t *xp_in)
{
  volatile mp_limb_t is_non_zero, is_not_p;
  const volatile mp_limb_t *xp;
  mp_size_t i;

  for (xp = xp_in, i = 0, is_non_zero = is_not_p = 0; i < m->size; i++)
    {
      is_non_zero |= xp[i];
      is_not_p |= (xp[i] ^ m->m[i]);
    }

  return (is_non_zero == 0) | (is_not_p == 0);
}

int
ecc_mod_equal_p (const struct ecc_modulo *m, const mp_limb_t *a,
		 const mp_limb_t *ref, mp_limb_t *scratch)
{
  mp_limb_t cy;
  cy = mpn_sub_n (scratch, a, ref, m->size);
  /* If cy > 0, i.e., a < ref, then they can't be equal mod m. */
  return (cy == 0) & ecc_mod_zero_p (m, scratch);
}

void
ecc_mod_add (const struct ecc_modulo *m, mp_limb_t *rp,
	     const mp_limb_t *ap, const mp_limb_t *bp)
{
  mp_limb_t cy;
  cy = mpn_add_n (rp, ap, bp, m->size);
  cy = mpn_cnd_add_n (cy, rp, rp, m->B, m->size);
  cy = mpn_cnd_add_n (cy, rp, rp, m->B, m->size);
  assert (cy == 0);  
}

void
ecc_mod_sub (const struct ecc_modulo *m, mp_limb_t *rp,
	     const mp_limb_t *ap, const mp_limb_t *bp)
{
  mp_limb_t cy;
  cy = mpn_sub_n (rp, ap, bp, m->size);
  cy = mpn_cnd_sub_n (cy, rp, rp, m->B, m->size);
  cy = mpn_cnd_sub_n (cy, rp, rp, m->B, m->size);
  assert (cy == 0);  
}

void
ecc_mod_mul_1 (const struct ecc_modulo *m, mp_limb_t *rp,
	       const mp_limb_t *ap, mp_limb_t b)
{
  mp_limb_t hi;

  assert (b <= 0xffffffff);
  hi = mpn_mul_1 (rp, ap, m->size, b);
  hi = mpn_addmul_1 (rp, m->B, m->size, hi);
  assert (hi <= 1);
  hi = mpn_cnd_add_n (hi, rp, rp, m->B, m->size);
  /* Sufficient if b < B^size / p */
  assert (hi == 0);
}

void
ecc_mod_addmul_1 (const struct ecc_modulo *m, mp_limb_t *rp,
		  const mp_limb_t *ap, mp_limb_t b)
{
  mp_limb_t hi;

  assert (b <= 0xffffffff);
  hi = mpn_addmul_1 (rp, ap, m->size, b);
  hi = mpn_addmul_1 (rp, m->B, m->size, hi);
  assert (hi <= 1);
  hi = mpn_cnd_add_n (hi, rp, rp, m->B, m->size);
  /* Sufficient roughly if b < B^size / p */
  assert (hi == 0);
}
  
void
ecc_mod_submul_1 (const struct ecc_modulo *m, mp_limb_t *rp,
		  const mp_limb_t *ap, mp_limb_t b)
{
  mp_limb_t hi;

  assert (b <= 0xffffffff);
  hi = mpn_submul_1 (rp, ap, m->size, b);
  hi = mpn_submul_1 (rp, m->B, m->size, hi);
  assert (hi <= 1);
  hi = mpn_cnd_sub_n (hi, rp, rp, m->B, m->size);
  /* Sufficient roughly if b < B^size / p */
  assert (hi == 0);
}

void
ecc_mod_mul (const struct ecc_modulo *m, mp_limb_t *rp,
	     const mp_limb_t *ap, const mp_limb_t *bp, mp_limb_t *tp)
{
  mpn_mul_n (tp, ap, bp, m->size);
  m->reduce (m, rp, tp);
}

void
ecc_mod_sqr (const struct ecc_modulo *m, mp_limb_t *rp,
	     const mp_limb_t *ap, mp_limb_t *tp)
{
  mpn_sqr (tp, ap, m->size);
  m->reduce (m, rp, tp);
}

void
ecc_mod_mul_canonical (const struct ecc_modulo *m, mp_limb_t *rp,
		       const mp_limb_t *ap, const mp_limb_t *bp, mp_limb_t *tp)
{
  mp_limb_t cy;
  mpn_mul_n (tp, ap, bp, m->size);
  m->reduce (m, tp + m->size, tp);

  cy = mpn_sub_n (rp, tp + m->size, m->m, m->size);
  cnd_copy (cy, rp, tp + m->size, m->size);
}

void
ecc_mod_sqr_canonical (const struct ecc_modulo *m, mp_limb_t *rp,
		       const mp_limb_t *ap, mp_limb_t *tp)
{
  mp_limb_t cy;
  mpn_sqr (tp, ap, m->size);
  m->reduce (m, tp + m->size, tp);

  cy = mpn_sub_n (rp, tp + m->size, m->m, m->size);
  cnd_copy (cy, rp, tp + m->size, m->size);
}

void
ecc_mod_pow_2k (const struct ecc_modulo *m,
		mp_limb_t *rp, const mp_limb_t *xp,
		unsigned k, mp_limb_t *tp)
{
  ecc_mod_sqr (m, rp, xp, tp);
  while (--k > 0)
    ecc_mod_sqr (m, rp, rp, tp);
}

void
ecc_mod_pow_2k_mul (const struct ecc_modulo *m,
		    mp_limb_t *rp, const mp_limb_t *xp,
		    unsigned k, const mp_limb_t *yp,
		    mp_limb_t *tp)
{
  ecc_mod_pow_2k (m, rp, xp, k, tp);
  ecc_mod_mul (m, rp, rp, yp, tp);
}
