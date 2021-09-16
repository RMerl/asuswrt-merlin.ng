/* ecc-secp384r1.c

   Compile time constant (but machine dependent) tables.

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

#include "ecc.h"
#include "ecc-internal.h"

#define USE_REDC 0

#include "ecc-secp384r1.h"

#if HAVE_NATIVE_ecc_secp384r1_modp
#define ecc_secp384r1_modp _nettle_ecc_secp384r1_modp
void
ecc_secp384r1_modp (const struct ecc_modulo *m, mp_limb_t *rp, mp_limb_t *xp);
#elif GMP_NUMB_BITS == 32

/* Use that 2^{384} = 2^{128} + 2^{96} - 2^{32} + 1, and eliminate 256
   bits at a time.

   We can get carry == 2 in the first iteration, and I think *only* in
   the first iteration. */

/* p is 12 limbs, and B^12 - p = B^4 + B^3 - B + 1. We can eliminate
   almost 8 at a time. Do only 7, to avoid additional carry
   propagation, followed by 5. */
static void
ecc_secp384r1_modp (const struct ecc_modulo *p, mp_limb_t *rp, mp_limb_t *xp)
{
  mp_limb_t cy, bw;

  /* Reduce from 24 to 17 limbs. */
  cy = mpn_add_n (xp + 4, xp + 4, xp + 16, 8);
  cy = sec_add_1 (xp + 12, xp + 12, 3, cy);

  bw = mpn_sub_n (xp + 5, xp + 5, xp + 16, 8);
  bw = sec_sub_1 (xp + 13, xp + 13, 3, bw);

  cy += mpn_add_n (xp + 7, xp + 7, xp + 16, 8);
  cy = sec_add_1 (xp + 15, xp + 15, 1, cy);

  cy += mpn_add_n (xp + 8, xp + 8, xp + 16, 8);
  assert (bw <= cy);
  cy -= bw;

  assert (cy <= 2);  
  xp[16] = cy;

  /* Reduce from 17 to 12 limbs */
  cy = mpn_add_n (xp, xp, xp + 12, 5);
  cy = sec_add_1 (xp + 5, xp + 5, 3, cy);
  
  bw = mpn_sub_n (xp + 1, xp + 1, xp + 12, 5);
  bw = sec_sub_1 (xp + 6, xp + 6, 6, bw);
  
  cy += mpn_add_n (xp + 3, xp + 3, xp + 12, 5);
  cy = sec_add_1 (xp + 8, xp + 8, 1, cy);

  cy += mpn_add_n (xp + 4, xp + 4, xp + 12, 5);
  cy = sec_add_1 (xp + 9, xp + 9, 3, cy);

  assert (cy >= bw);
  cy -= bw;
  assert (cy <= 1);
  cy = mpn_cnd_add_n (cy, rp, xp, p->B, ECC_LIMB_SIZE);
  assert (cy == 0);
}
#elif GMP_NUMB_BITS == 64
/* p is 6 limbs, and B^6 - p = B^2 + 2^32 (B - 1) + 1. Eliminate 3
   (almost 4) limbs at a time. */
static void
ecc_secp384r1_modp (const struct ecc_modulo *p, mp_limb_t *rp, mp_limb_t *xp)
{
  mp_limb_t tp[6];
  mp_limb_t cy;

  /* Reduce from 12 to 9 limbs */
  tp[0] = 0; /* FIXME: Could use mpn_sub_nc */
  mpn_copyi (tp + 1, xp + 8, 3);
  tp[4] = xp[11] - mpn_sub_n (tp, tp, xp + 8, 4);
  tp[5] = mpn_lshift (tp, tp, 5, 32);

  cy = mpn_add_n (xp + 2, xp + 2, xp + 8, 4);
  cy = sec_add_1 (xp + 6, xp + 6, 2, cy);

  cy += mpn_add_n (xp + 2, xp + 2, tp, 6);
  cy += mpn_add_n (xp + 4, xp + 4, xp + 8, 4);

  assert (cy <= 2);
  xp[8] = cy;

  /* Reduce from 9 to 6 limbs */
  tp[0] = 0;
  mpn_copyi (tp + 1, xp + 6, 2);
  tp[3] = xp[8] - mpn_sub_n (tp, tp, xp + 6, 3);
  tp[4] = mpn_lshift (tp, tp, 4, 32);

  cy = mpn_add_n (xp, xp, xp + 6, 3);
  cy = sec_add_1 (xp + 3, xp + 3, 2, cy);
  cy += mpn_add_n (xp, xp, tp, 5);
  cy += mpn_add_n (xp + 2, xp + 2, xp + 6, 3);

  cy = sec_add_1 (xp + 5, xp + 5, 1, cy);
  assert (cy <= 1);

  cy = mpn_cnd_add_n (cy, xp, xp, p->B, ECC_LIMB_SIZE);
  assert (cy == 0);
  mpn_copyi (rp, xp, ECC_LIMB_SIZE);
}
#else
#define ecc_secp384r1_modp ecc_mod
#endif

#define ECC_SECP384R1_INV_ITCH (6*ECC_LIMB_SIZE)

static void
ecc_secp384r1_inv (const struct ecc_modulo *p,
		   mp_limb_t *rp, const mp_limb_t *ap,
		   mp_limb_t *scratch)
{
#define a3 scratch
#define a5m1 (scratch + ECC_LIMB_SIZE)
#define a15m1 (scratch + 2*ECC_LIMB_SIZE)
#define a30m1 a5m1
#define t0 (scratch + 3*ECC_LIMB_SIZE)
#define tp (scratch + 4*ECC_LIMB_SIZE)
  /*
    Addition chain for

    p - 2 = 2^{384} - 2^{128} - 2^{96} + 2^{32} - 3

    3 = 1 + 2
    2^4 - 1 = 15 = (2^2 + 1) * 3
    2^5 - 1 = 1 + 2(2^4 - 1)
    2^{15} - 1 = (1 + 2^5(1 + 2^5)) (2^5-1)
    2^{30} - 1 = (2^{15} + 1) (2^{15} - 1)
    2^{60} - 1 = (2^{30} + 1) (2^{30} - 1)
    2^{120} - 1 = (2^{60} + 1) (2^{60} - 1)
    2^{240} - 1 = (2^{120} + 1)(2^{120} - 1)
    2^{255} - 1 = 2^{15} (2^{240} - 1) + 2^{15} - 1
    2^{286} - 2^{30} - 1 = 2^{31} (2^{255} - 1) + 2^{30} - 1

    2^{288} - 2^{32} - 1 = 2^2 (2^{286} - 2^{30} - 1) + 3
    2^{382} - 2^{126} - 2^{94} + 2^{30} - 1
         = 2^{94} (2^{288} - 2^{32} - 1) + 2^{30} - 1

    This addition chain needs 383 squarings and 14 multiplies.

  */
  ecc_mod_sqr (p, rp, ap, tp);		/* a^2 */
  ecc_mod_mul (p, a3, rp, ap, tp);	/* a^3 */
  ecc_mod_pow_2kp1 (p, rp, a3, 2, tp);	/* a^{2^4 - 1}, a3 */
  ecc_mod_sqr (p, rp, rp, tp);		/* a^{2^5 - 2} */
  ecc_mod_mul (p, a5m1, rp, ap, tp);	/* a^{2^5 - 1}, a3 a5m1 */

  ecc_mod_pow_2kp1 (p, rp, a5m1, 5, tp); /* a^{2^{10} - 1, a3, a5m1*/
  ecc_mod_pow_2k_mul (p, a15m1, rp, 5, a5m1, tp); /* a^{2^{15} - 1}, a3, a5m1 a15m1 */
  ecc_mod_pow_2kp1 (p, a30m1, a15m1, 15, tp);  /* a^{2^{30} - 1}, a3 a15m1 a30m1 */
  
  ecc_mod_pow_2kp1 (p, rp, a30m1, 30, tp); /* a^{2^{60} - 1, a3 a15m1 a30m1 */
  ecc_mod_pow_2kp1 (p, t0, rp, 60, tp); /* a^{2^{120} - 1, a3 a15m1 a30m1 */
  ecc_mod_pow_2kp1 (p, rp, t0, 120, tp); /* a^{2^{240} - 1, a3 a15m1 a30m1 */
  ecc_mod_pow_2k_mul (p, rp, rp, 15, a15m1, tp); /* a^{2^{255} - 1, a3 a30m1 */
  ecc_mod_pow_2k_mul (p, rp, rp, 31, a30m1, tp); /* a^{2^{286} - 2^{30} - 1}, a3 a30m1 */

  ecc_mod_pow_2k_mul (p, rp, rp, 2, a3, tp); /* a^{2^{288} - 2^{32} - 1, a30m1 */
  ecc_mod_pow_2k_mul (p, rp, rp, 94, a30m1, tp); /* a^{2^{392} - 2^{126} - 2^{94} + 2^{30} - 1 */
  ecc_mod_pow_2k_mul (p, rp, rp, 2, ap, tp);
}

const struct ecc_curve _nettle_secp_384r1 =
{
  {
    384,
    ECC_LIMB_SIZE,    
    ECC_BMODP_SIZE,
    ECC_REDC_SIZE,
    ECC_SECP384R1_INV_ITCH,
    0,

    ecc_p,
    ecc_Bmodp,
    ecc_Bmodp_shifted,
    ecc_redc_ppm1,
    ecc_pp1h,

    ecc_secp384r1_modp,
    ecc_secp384r1_modp,
    ecc_secp384r1_inv,
    NULL,
  },
  {
    384,
    ECC_LIMB_SIZE,    
    ECC_BMODQ_SIZE,
    0,
    ECC_MOD_INV_ITCH (ECC_LIMB_SIZE),
    0,

    ecc_q,
    ecc_Bmodq,
    ecc_Bmodq_shifted,
    NULL,
    ecc_qp1h,

    ecc_mod,
    ecc_mod,
    ecc_mod_inv,
    NULL,
  },

  USE_REDC,
  ECC_PIPPENGER_K,
  ECC_PIPPENGER_C,

  ECC_ADD_JJA_ITCH (ECC_LIMB_SIZE),
  ECC_ADD_JJJ_ITCH (ECC_LIMB_SIZE),
  ECC_DUP_JJ_ITCH (ECC_LIMB_SIZE),
  ECC_MUL_A_ITCH (ECC_LIMB_SIZE),
  ECC_MUL_G_ITCH (ECC_LIMB_SIZE),
  ECC_J_TO_A_ITCH(ECC_LIMB_SIZE, ECC_SECP384R1_INV_ITCH),

  ecc_add_jja,
  ecc_add_jjj,
  ecc_dup_jj,
  ecc_mul_a,
  ecc_mul_g,
  ecc_j_to_a,

  ecc_b,
  ecc_unit,
  ecc_table
};

const struct ecc_curve *nettle_get_secp_384r1(void)
{
  return &_nettle_secp_384r1;
}
