/* ecc-mul-m.c

   Point multiplication using Montgomery curve representation.

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

#include <assert.h>

#include "ecc.h"
#include "ecc-internal.h"

void
ecc_mul_m (const struct ecc_modulo *m,
	   mp_limb_t a24,
	   unsigned bit_low, unsigned bit_high,
	   mp_limb_t *qx, const uint8_t *n, const mp_limb_t *px,
	   mp_limb_t *scratch)
{
  unsigned i;
  mp_limb_t swap;

#define x2 (scratch)
#define z2 (scratch + m->size)
#define x3 (scratch + 2*m->size)
#define z3 (scratch + 3*m->size)

  /* Formulas from RFC 7748:

       A = x_2 + z_2
       AA = A^2
       B = x_2 - z_2
       BB = B^2
       E = AA - BB
       C = x_3 + z_3
       D = x_3 - z_3
       DA = D * A
       CB = C * B
       x_3 = (DA + CB)^2
       z_3 = x_1 * (DA - CB)^2
       x_2 = AA * BB
       z_2 = E * (AA + a24 * E)

     For pure doubling, we use:

       A = x_2 + z_2
       AA = A^2
       B = x_2 - z_2
       BB = B^2
       E = AA - BB
       x3 = AA * BB
       z3 =  E * (AA + a24 * E)
  */

#define A (scratch + 4*m->size)
#define AA A
#define D (scratch + 5*m->size)
#define DA D

#define tp (scratch + 6*m->size)

  /* For the doubling formulas. */
#define B D
#define BB D
#define E D

  /* Initialize, x2 = px, z2 = 1 */
  mpn_copyi (x2, px, m->size);
  z2[0] = 1;
  mpn_zero (z2+1, m->size - 1);

  /* Get x3, z3 from doubling. Since most significant bit is forced to 1. */
  ecc_mod_add (m, A, x2, z2);
  ecc_mod_sub (m, B, x2, z2);
  ecc_mod_sqr (m, AA, A, tp);
  ecc_mod_sqr (m, BB, B, tp);
  ecc_mod_mul (m, x3, AA, BB, tp);
  ecc_mod_sub (m, E, AA, BB);
  ecc_mod_addmul_1 (m, AA, E, a24);
  ecc_mod_mul (m, z3, E, AA, tp);

  for (i = bit_high, swap = 0; i >= bit_low; i--)
    {
      mp_limb_t bit = (n[i/8] >> (i & 7)) & 1;

      mpn_cnd_swap (swap ^ bit, x2, x3, 2*m->size);
      swap = bit;

      ecc_mod_add (m, A, x2, z2);
      ecc_mod_sub (m, D, x3, z3);
      ecc_mod_mul (m, DA, D, A, tp);
      ecc_mod_sqr (m, AA, A, tp);

      /* Store B, BB and E at z2 */
      ecc_mod_sub (m, z2, x2, z2);	/* B */
      /* Store C and CB at z3 */
      ecc_mod_add (m, z3, x3, z3);	/* C */
      ecc_mod_mul (m, z3, z3, z2, tp);	/* CB */
      ecc_mod_sqr (m, z2, z2, tp);	/* BB */

      /* Finish x2 */
      ecc_mod_mul (m, x2, AA, z2, tp);

      ecc_mod_sub (m, z2, AA, z2);	/* E */

      /* Finish z2 */
      ecc_mod_addmul_1 (m, AA, z2, a24);
      ecc_mod_mul (m, z2, z2, AA, tp);

      /* Finish x3 */
      ecc_mod_add (m, x3, DA, z3);
      ecc_mod_sqr (m, x3, x3, tp);

      /* Finish z3 */
      ecc_mod_sub (m, z3, DA, z3);	/* DA - CB */
      ecc_mod_sqr (m, z3, z3, tp);
      ecc_mod_mul (m, z3, z3, px, tp);
    }
  mpn_cnd_swap (swap, x2, x3, 2*m->size);

  /* Do the low zero bits, just duplicating x2 */
  for (i = 0; i < bit_low; i++)
    {
      ecc_mod_add (m, A, x2, z2);
      ecc_mod_sub (m, B, x2, z2);
      ecc_mod_sqr (m, AA, A, tp);
      ecc_mod_sqr (m, BB, B, tp);
      ecc_mod_mul (m, x2, AA, BB, tp);
      ecc_mod_sub (m, E, AA, BB);
      ecc_mod_addmul_1 (m, AA, E, a24);
      ecc_mod_mul (m, z2, E, AA, tp);
    }
  assert (m->invert_itch <= 7 * m->size);
  m->invert (m, x3, z2, z3 + m->size);
  ecc_mod_mul_canonical (m, qx, x2, x3, z3);
}
