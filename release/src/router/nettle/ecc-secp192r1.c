/* ecc-secp192r1.c

   Compile time constant (but machine dependent) tables.

   Copyright (C) 2013, 2014, 2019, 2021 Niels Möller
   Copyright (C) 2019 Wim Lewis

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

#include "ecc-secp192r1.h"

#if HAVE_NATIVE_ecc_secp192r1_modp

#define ecc_secp192r1_modp _nettle_ecc_secp192r1_modp
void
ecc_secp192r1_modp (const struct ecc_modulo *m, mp_limb_t *rp, mp_limb_t *xp);

/* Use that p = 2^{192} - 2^64 - 1, to eliminate 128 bits at a time. */

#elif GMP_NUMB_BITS == 32
/* p is 6 limbs, p = B^6 - B^2 - 1 */
static void
ecc_secp192r1_modp (const struct ecc_modulo *m UNUSED, mp_limb_t *rp, mp_limb_t *xp)
{
  mp_limb_t cy;

  /* Reduce from 12 to 9 limbs (top limb small)*/
  cy = mpn_add_n (xp + 2, xp + 2, xp + 8, 4);
  cy = sec_add_1 (xp + 6, xp + 6, 2, cy);
  cy += mpn_add_n (xp + 4, xp + 4, xp + 8, 4);
  assert (cy <= 2);

  xp[8] = cy;

  /* Reduce from 9 to 6 limbs */
  cy = mpn_add_n (xp, xp, xp + 6, 3);
  cy = sec_add_1 (xp + 3, xp + 3, 2, cy);
  cy += mpn_add_n (xp + 2, xp + 2, xp + 6, 3);
  cy = sec_add_1 (xp + 5, xp + 5, 1, cy);
  
  assert (cy <= 1);
  cy = mpn_cnd_add_n (cy, rp, xp, ecc_Bmodp, 6);
  assert (cy == 0);  
}
#elif GMP_NUMB_BITS == 64
/* p is 3 limbs, p = B^3 - B - 1 */
static void
ecc_secp192r1_modp (const struct ecc_modulo *m UNUSED, mp_limb_t *rp, mp_limb_t *xp)
{
  mp_limb_t cy;

  /* Reduce from 6 to 5 limbs (top limb small)*/
  cy = mpn_add_n (xp + 1, xp + 1, xp + 4, 2);
  cy = sec_add_1 (xp + 3, xp + 3, 1, cy);
  cy += mpn_add_n (xp + 2, xp + 2, xp + 4, 2);
  assert (cy <= 2);

  xp[4] = cy;

  /* Reduce from 5 to 4 limbs (high limb small) */
  cy = mpn_add_n (xp, xp, xp + 3, 2);
  cy = sec_add_1 (xp + 2, xp + 2, 1, cy);
  cy += mpn_add_n (xp + 1, xp + 1, xp + 3, 2);

  assert (cy <= 1);
  cy = mpn_cnd_add_n (cy, rp, xp, ecc_Bmodp, 3);
  assert (cy == 0);  
}
  
#else
#define ecc_secp192r1_modp ecc_mod
#endif

#define ECC_SECP192R1_INV_ITCH (4*ECC_LIMB_SIZE)

static void
ecc_secp192r1_inv (const struct ecc_modulo *p,
		   mp_limb_t *rp, const mp_limb_t *ap,
		   mp_limb_t *scratch)
{
#define a62m1 scratch
#define t0 (scratch + ECC_LIMB_SIZE)
#define tp (scratch + 2*ECC_LIMB_SIZE)

  /* Addition chain

       p - 2 = 2^{192} - 2^{64} - 3
	     = 1 + 2^{192} - 2^{64} - 4
	     = 1 + 2^2 (2^{190} - 2^{62} - 1)
	     = 1 + 2^2 (2^{62} - 1 + 2^{190} - 2^63)
	     = 1 + 2^2 (2^{62} - 1 + 2^{63}(2^{127} - 1))
	     = 1 + 2^2 (2^{62} - 1 + 2^{63}(1 + 2 (2^{126} - 1)))
	     = 1 + 2^2 (2^{62} - 1 + 2^{63}(1 + 2 (2^{63} + 1)(2^{63} - 1)))
	     = 1 + 2^2 (2^{62} - 1 + 2^{63}(1 + 2 (2^{63} + 1)(1 + 2(2^{62} - 1))))

       2^{62} - 1 = (2^{31}+1)(2^{31}-1)
		  = (2^{31}+1)(1 + 2(2^{30} - 1))
		  = (2^{31}+1)(1 + 2(2^{15}+1)(2^15-1))
		  = (2^{31}+1)(1 + 2(2^{15}+1)(1 + 2(1 + (2^{14}-1))))
		  = (2^{31}+1)(1 + 2(2^{15}+1)(1 + 2(1 + (2^7+1)(2^7-1))))
		  = (2^{31}+1)(1 + 2(2^{15}+1)(1 + 2(1 + (2^7+1)(1+2(2^3+1)(2^3-1)))))
		  = (2^{31}+1)(1 + 2(2^{15}+1)(1 + 2(1 + (2^7+1)(1+2(2^3+1)(1 + 2 (2+1))))))

       This addition chain needs 191 squarings and 14 multiplies.

       Could be improved sligthly as:

       a^7	   = 1 + 2 * (2 + 1)
       2^{62} - 1  = (2^{31}+1)(2^{31}-1)
		   = (2^{31}+1)(1 + 2(2^{30} - 1))
		   = (2^{31}+1)(1 + 2(2^{15}+1)(2^15-1))
		   = (2^{31}+1)(1 + 2(2^{15}+1)(1 + 2(1 + (2^{14}-1))))
		   = (2^{31}+1)(1 + 2(2^{15}+1)(1 + 2(1 + (2^7+1)(2^7-1))))
		   = (2^{31}+1)(1 + 2(2^{15}+1)(1 + 2(1 + (2^7+1)(1+2(2^3+1)(2^3-1)))))
       2^{65} - 1  = 2^3 (2^{62} - 1) + 2^3 - 1
       2^{127} - 1 = 2^{62} (2^{65} - 1) + 2^{62} - 1
       p - 2 = 1 + 2^2 (2^{62} - 1 + 2^{63}(2^{127} - 1))

       This needs 191 squarings and 13 multiplies, i.e., saving one
       multiply, at the cost of additional temporary storage for a^7.
  */

  ecc_mod_sqr (p, rp, ap, tp);	        /* a^2 */
  ecc_mod_mul (p, rp, rp, ap, tp);		/* a^3 */
  ecc_mod_sqr (p, rp, rp, tp);		/* a^6 */
  ecc_mod_mul (p, rp, rp, ap, tp);		/* a^{2^3-1} */
  ecc_mod_pow_2kp1 (p, t0, rp, 3, tp);	/* a^{2^6-1} */
  ecc_mod_sqr (p, rp, t0, tp);		/* a^{2^7-2} */
  ecc_mod_mul (p, rp, rp, ap, tp);	/* a^{2^7-1} */
  ecc_mod_pow_2kp1 (p, t0, rp, 7, tp);	/* a^{2^14-1} */
  ecc_mod_sqr (p, rp, t0, tp);		/* a^{2^15-2} */
  ecc_mod_mul (p, rp, ap, rp, tp);	/* a^{2^15-1} */
  ecc_mod_pow_2kp1 (p, t0, rp, 15, tp);	/* a^{2^30-1} */
  ecc_mod_sqr (p, rp, t0, tp);		/* a^{2^31-2} */
  ecc_mod_mul (p, rp, ap, rp, tp);	/* a^{2^31-1} */
  ecc_mod_pow_2kp1 (p, a62m1, rp, 31, tp);	/* a^{2^62-1} Overlaps t0 */

  ecc_mod_sqr (p, rp, a62m1, tp);	/* a^{2^63-2} */
  ecc_mod_mul (p, rp, rp, ap, tp);	/* a^{2^63-1} */
  ecc_mod_pow_2kp1 (p, t0, rp, 63, tp);	/* a^{2^126-1} */
  ecc_mod_sqr (p, rp, t0, tp);		/* a^{2^127-2} */
  ecc_mod_mul (p, rp, rp, ap, tp);		/* a^{2^127-1} Clobbers t1 */
  ecc_mod_pow_2k_mul (p, rp, rp, 63, a62m1, tp); /* a^{2^190 - 2^62 - 1} */
  ecc_mod_sqr (p, rp, rp, tp);		/* a^{2^191 - 2^63 - 2} */
  ecc_mod_sqr (p, rp, rp, tp);		/* a^{2^192 - 2^64 - 4} */
  ecc_mod_mul (p, rp, rp, ap, tp);

#undef a62m1
#undef t0
#undef tp
}

/* To guarantee that inputs to ecc_mod_zero_p are in the required range. */
#if ECC_LIMB_SIZE * GMP_NUMB_BITS != 192
#error Unsupported limb size
#endif

#define ECC_SECP192R1_SQRT_ITCH (3*ECC_LIMB_SIZE)

static int
ecc_secp192r1_sqrt (const struct ecc_modulo *p,
		    mp_limb_t *rp,
		    const mp_limb_t *cp,
		    mp_limb_t *scratch)
{
  /* This computes the square root modulo p192 using the identity:

     sqrt(c) = c^(2^190 - 2^62)  (mod P-192)

     which can be seen as a special case of Tonelli-Shanks with e=1.
  */

  /* We need one t0 (and use clobbering rp) and scratch space for mul and sqr. */

#define t0 scratch
#define tp (scratch + ECC_LIMB_SIZE)

  ecc_mod_sqr(p, rp, cp, tp);		/* c^2 */
  ecc_mod_mul(p, rp, rp, cp, tp);	/* c^3 */
  ecc_mod_pow_2kp1(p, t0, rp, 2, tp);	/* c^(2^4 - 1) */
  ecc_mod_pow_2kp1(p, rp, t0, 4, tp);	/* c^(2^8 - 1) */
  ecc_mod_pow_2kp1(p, t0, rp, 8,  tp);	/* c^(2^16 - 1) */
  ecc_mod_pow_2kp1(p, rp, t0, 16, tp);	/* c^(2^32 - 1) */
  ecc_mod_pow_2kp1(p, t0, rp, 32, tp);	/* c^(2^64 - 1) */
  ecc_mod_pow_2kp1(p, rp, t0, 64, tp);	/* c^(2^128 - 1) */

  ecc_mod_pow_2k    (p, rp, rp,     62, tp);   /* c^(2^190 - 2^62) */

  /* Check that input was a square, R^2 = C, for non-squares we'd get
     R^2 = -C. */
  ecc_mod_sqr(p, t0, rp, tp);
  ecc_mod_sub(p, t0, t0, cp);

  return ecc_mod_zero_p (p, t0);

#undef t0
#undef tp
}

const struct ecc_curve _nettle_secp_192r1 =
{
  {
    192,
    ECC_LIMB_SIZE,
    ECC_BMODP_SIZE,
    ECC_REDC_SIZE,
    ECC_SECP192R1_INV_ITCH,
    ECC_SECP192R1_SQRT_ITCH,
    0,

    ecc_p,
    ecc_Bmodp,
    ecc_Bmodp_shifted,    
    ecc_redc_ppm1,
    ecc_pp1h,

    ecc_secp192r1_modp,
    ecc_secp192r1_modp,
    ecc_secp192r1_inv,
    ecc_secp192r1_sqrt,
    NULL,
  },
  {
    192,
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
  ECC_J_TO_A_ITCH(ECC_LIMB_SIZE, ECC_SECP192R1_INV_ITCH),

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

const struct ecc_curve *nettle_get_secp_192r1(void)
{
  return &_nettle_secp_192r1;
}
