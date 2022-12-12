/* ecc-point.c

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

#include "ecc.h"
#include "ecc-internal.h"

void
ecc_point_init (struct ecc_point *p, const struct ecc_curve *ecc)
{
  p->ecc = ecc;
  p->p = gmp_alloc_limbs (2*ecc->p.size);
}

void
ecc_point_clear (struct ecc_point *p)
{
  gmp_free_limbs (p->p, 2*p->ecc->p.size);
}

int
ecc_point_set (struct ecc_point *p, const mpz_t x, const mpz_t y)
{
  mp_size_t size;  
  mpz_t m, lhs, rhs;
  mpz_t t;
  int res;

  size = p->ecc->p.size;
  mpz_roinit_n (m, p->ecc->p.m, size);
  
  if (mpz_sgn (x) < 0 || mpz_cmp (x, m) >= 0
      || mpz_sgn (y) < 0 || mpz_cmp (y, m) >= 0)
    return 0;

  mpz_init (lhs);
  mpz_init (rhs);

  mpz_mul (lhs, y, y);
  
  if (p->ecc->p.bit_size == 255)
    {
      /* ed25519 special case. FIXME: Do in some cleaner way? */
      mpz_t x2;
      mpz_init (x2);
      mpz_mul (x2, x, x);
      mpz_mul (rhs, x2, lhs);
      /* Check that -x^2 + y^2 = 1 - (121665/121666) x^2 y^2
	 or 121666 (1 + x^2 - y^2) = 121665 x^2 y^2 */
      mpz_sub (lhs, x2, lhs);
      mpz_add_ui (lhs, lhs, 1);
      mpz_mul_ui (lhs, lhs, 121666);
      mpz_mul_ui (rhs, rhs, 121665);
      mpz_clear (x2);
    }
  else if (p->ecc->p.bit_size == 448)
    {
      /* curve448 special case. FIXME: Do in some cleaner way? */
      mpz_t x2, d;
      mpz_init (x2);
      mpz_init_set_ui (d, 39081);
      mpz_mul (x2, x, x); /* x^2 */
      mpz_mul (d, d, x2); /* 39081 x^2 */
      mpz_set_ui (rhs, 1);
      mpz_submul (rhs, d, lhs); /* 1 - 39081 x^2 y^2 */
      /* Check that x^2 + y^2 = 1 - 39081 x^2 y^2 */
      mpz_add (lhs, x2, lhs);	/* x^2 + y^2 */
      mpz_clear (d);
      mpz_clear (x2);
    }
  else
    {
      /* Check that y^2 = x^3 - 3*x + b (mod p) */
      mpz_mul (rhs, x, x);
      mpz_sub_ui (rhs, rhs, 3);
      mpz_mul (rhs, rhs, x);
      mpz_add (rhs, rhs, mpz_roinit_n (t, p->ecc->b, size));
    }

  res = mpz_congruent_p (lhs, rhs, mpz_roinit_n (t, p->ecc->p.m, size));

  mpz_clear (lhs);
  mpz_clear (rhs);

  if (!res)
    return 0;

  mpz_limbs_copy (p->p, x, size);
  mpz_limbs_copy (p->p + size, y, size);

  return 1;
}

void
ecc_point_get (const struct ecc_point *p, mpz_t x, mpz_t y)
{
  mp_size_t size = p->ecc->p.size;
  if (x)
    mpz_set_n (x, p->p, size);
  if (y)
    mpz_set_n (y, p->p + size, size);
}
