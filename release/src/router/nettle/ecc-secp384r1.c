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

/* Computes a^{2^{288} -2^{32} - 1} mod m. Also produces the
   intermediate value a^{2^{30} - 1}. Needs 5*ECC_LIMB_SIZE
   scratch. */
static void
ecc_mod_pow_288m32m1 (const struct ecc_modulo *m,
		      mp_limb_t *rp, mp_limb_t *a30m1,
		      const mp_limb_t *ap, mp_limb_t *scratch)
{
  /*
    Addition chain for 2^{288} - 2^{32} - 1:

    2^2 - 1 = 1 + 2
    2^4 - 1 = (2^2 + 1) * (2^2 - 1)
    2^5 - 1 = 1 + 2(2^4 - 1)
    2^{10} - 1 = (2^5 + 1) (2^5 - 1)
    2^{15} - 1 = 2^5 (2^{10} - 1) + (2^5-1)
    2^{30} - 1 = (2^{15} + 1) (2^{15} - 1)
    2^{32} - 4 = 2^2 (2^{30} - 1)
    2^{32} - 1 = (2^{32} - 4) + 3
    2^{60} - 1 = 2^{28}(2^{32} - 4) + (2^{30} - 1)
    2^{120} - 1 = (2^{60} + 1) (2^{60} - 1)
    2^{240} - 1 = (2^{120} + 1)(2^{120} - 1)
    2^{255} - 1 = 2^{15} (2^{240} - 1) + 2^{15} - 1
    2^{288} - 2^{32} - 1 = 2^{33} (2^{255} - 1) + 2^{32} - 1

    Total 287 squarings, and 12 multiplies.

    The specific sqr/mul schedule is from Routine 3.2.12 of
    "Mathematical routines for the NIST prime elliptic curves", April
    5, 2010, author unknown.
  */

#define t0 scratch
#define a3 (scratch + ECC_LIMB_SIZE)
#define a5m1 a30m1
#define a15m1 (scratch + 2*ECC_LIMB_SIZE)
#define a32m1 a3
#define tp (scratch + 3*ECC_LIMB_SIZE)

  ecc_mod_sqr        (m, t0, ap, tp);			/* a^2 */
  ecc_mod_mul        (m, a3, t0, ap, tp);		/* a^3 */
  ecc_mod_pow_2kp1   (m, rp, a3, 2, tp);		/* a^(2^4 - 1) */
  ecc_mod_sqr        (m, t0, rp, tp);			/* a^(2^5 - 2) */
  ecc_mod_mul        (m, a5m1, t0, ap, tp);		/* a^(2^5 - 1) */
  ecc_mod_pow_2kp1   (m, t0, a5m1, 5, tp);		/* a^(2^10 - 1) */
  ecc_mod_pow_2k_mul (m, a15m1, t0, 5, a5m1, tp);	/* a^(2^15 - 1) a5m1*/
  ecc_mod_pow_2kp1   (m, a30m1, a15m1, 15, tp);		/* a^(2^30 - 1) */
  ecc_mod_pow_2k     (m, t0, a30m1, 2, tp);		/* a^(2^32 - 4) */
  ecc_mod_mul        (m, a32m1, t0, a3, tp);		/* a^(2^32 - 1) a3 */
  ecc_mod_pow_2k_mul (m, rp, t0, 28, a30m1, tp);	/* a^(2^60 - 1) a32m4 */
  ecc_mod_pow_2kp1   (m, t0, rp, 60, tp);		/* a^(2^120 - 1) */
  ecc_mod_pow_2kp1   (m, rp, t0, 120, tp);		/* a^(2^240 - 1) */
  ecc_mod_pow_2k_mul (m, t0, rp, 15, a15m1, tp);	/* a^(2^255 - 1) a15m1 */
  ecc_mod_pow_2k_mul (m, rp, t0, 33, a32m1, tp);	/* a^(2^288 - 2^32 - 1) a32m1 */

#undef t0
#undef a3
#undef a5m1
#undef a15m1
#undef a32m1
#undef tp
}

#define ECC_SECP384R1_INV_ITCH (6*ECC_LIMB_SIZE)

static void
ecc_secp384r1_inv (const struct ecc_modulo *p,
		   mp_limb_t *rp, const mp_limb_t *ap,
		   mp_limb_t *scratch)
{
  /*
    Addition chain for

    p - 2 = 2^{384} - 2^{128} - 2^{96} + 2^{32} - 3

    Start with

      a^{2^{288} - 2^{32} - 1}

    and then use

      2^{382} - 2^{126} - 2^{94} + 2^{30} - 1
         = 2^{94} (2^{288} - 2^{32} - 1) + 2^{30} - 1

      2^{384} - 2^{128} - 2^{96} + 2^{32} - 3
         = 2^2 (2^{382} - 2^{126} - 2^{94} + 2^{30} - 1) + 1

    This addition chain needs 96 additional squarings and 2
    multiplies, for a total of 383 squarings and 14 multiplies.
  */

#define a30m1 scratch
#define tp (scratch + ECC_LIMB_SIZE)

  ecc_mod_pow_288m32m1 (p, rp, a30m1, ap, tp);
  ecc_mod_pow_2k_mul (p, rp, rp, 94, a30m1, tp); /* a^{2^{382} - 2^{126} - 2^{94} + 2^{30} - 1 */
  ecc_mod_pow_2k_mul (p, rp, rp, 2, ap, tp);

#undef a30m1
#undef tp
}

/* To guarantee that inputs to ecc_mod_zero_p are in the required range. */
#if ECC_LIMB_SIZE * GMP_NUMB_BITS != 384
#error Unsupported limb size
#endif

#define ECC_SECP384R1_SQRT_ITCH (6*ECC_LIMB_SIZE)

static int
ecc_secp384r1_sqrt (const struct ecc_modulo *m,
		    mp_limb_t *rp,
		    const mp_limb_t *cp,
		    mp_limb_t *scratch)
{
  /* This computes the square root modulo p384 using the identity:

     sqrt(c) = c^(2^382 − 2^126 - 2^94 + 2^30)  (mod P-384)

     which can be seen as a special case of Tonelli-Shanks with e=1.

     Starting with

       a^{2^{288} - 2^{32} - 1}

     and then use

       2^352 - 2^96 - 2^64 + 1
         = 2^64 (2^{288} - 2^{32} - 1) + 1
       2^382 − 2^126 - 2^94 + 2^30
         = 2^30 (2^352 - 2^96 - 2^64 + 1)

     An additional 94 squarings and 2 multiplies, for a total of for a
     total of 381 squarings and 14 multiplies.
  */

#define t0 scratch
#define tp (scratch + ECC_LIMB_SIZE)

  ecc_mod_pow_288m32m1 (m, rp, t0, cp, tp);
  ecc_mod_pow_2k_mul (m, t0, rp, 64, cp, tp);		/* c^(2^352 - 2^96 - 2^64 + 1) */
  ecc_mod_pow_2k     (m, rp, t0, 30, tp);		/* c^(2^382 - 2^126 - 2^94 + 2^30) */

  ecc_mod_sqr (m, t0, rp, tp);
  ecc_mod_sub (m, t0, t0, cp);

  return ecc_mod_zero_p (m, t0);

#undef t0
#undef tp
}


const struct ecc_curve _nettle_secp_384r1 =
{
  {
    384,
    ECC_LIMB_SIZE,    
    ECC_BMODP_SIZE,
    ECC_REDC_SIZE,
    ECC_SECP384R1_INV_ITCH,
    ECC_SECP384R1_SQRT_ITCH,
    0,

    ecc_p,
    ecc_Bmodp,
    ecc_Bmodp_shifted,
    ecc_redc_ppm1,
    ecc_pp1h,

    ecc_secp384r1_modp,
    ecc_secp384r1_modp,
    ecc_secp384r1_inv,
    ecc_secp384r1_sqrt,
    NULL,
  },
  {
    384,
    ECC_LIMB_SIZE,    
    ECC_BMODQ_SIZE,
    0,
    ECC_MOD_INV_ITCH (ECC_LIMB_SIZE),
    0,
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
