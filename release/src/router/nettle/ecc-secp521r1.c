/* ecc-secp521r1.c

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

#include "ecc-internal.h"

#define USE_REDC 0

#include "ecc-secp521r1.h"

#define B_SHIFT (521 % GMP_NUMB_BITS)

#if HAVE_NATIVE_ecc_secp521r1_modp
#define ecc_secp521r1_modp _nettle_ecc_secp521r1_modp
void
ecc_secp521r1_modp (const struct ecc_modulo *m, mp_limb_t *rp, mp_limb_t *xp);

#else

#define BMODP_SHIFT (GMP_NUMB_BITS - B_SHIFT)
#define BMODP ((mp_limb_t) 1 << BMODP_SHIFT)

/* Result may be *slightly* larger than 2^521 */
static void
ecc_secp521r1_modp (const struct ecc_modulo *m UNUSED, mp_limb_t *rp, mp_limb_t *xp)
{
  /* FIXME: Should use mpn_addlsh_n_ip1 */
  mp_limb_t hi;
  /* Reduce from 2*ECC_LIMB_SIZE to ECC_LIMB_SIZE + 1 */
  xp[ECC_LIMB_SIZE]
    = mpn_addmul_1 (xp, xp + ECC_LIMB_SIZE, ECC_LIMB_SIZE, BMODP);
  hi = mpn_addmul_1 (xp, xp + ECC_LIMB_SIZE, 1, BMODP);
  hi = sec_add_1 (xp + 1, xp + 1, ECC_LIMB_SIZE - 1, hi);

  /* Combine hi with top bits, and add in. */
  hi = (hi << BMODP_SHIFT) | (xp[ECC_LIMB_SIZE-1] >> B_SHIFT);
  rp[ECC_LIMB_SIZE-1] = (xp[ECC_LIMB_SIZE-1]
			 & (((mp_limb_t) 1 << B_SHIFT)-1))
    + sec_add_1 (rp, xp, ECC_LIMB_SIZE - 1, hi);
}
#endif

#define ECC_SECP521R1_INV_ITCH (3*ECC_LIMB_SIZE)

static void
ecc_secp521r1_inv (const struct ecc_modulo *p,
		   mp_limb_t *rp, const mp_limb_t *ap,
		   mp_limb_t *scratch)
{
#define t0 scratch
#define tp (scratch + ECC_LIMB_SIZE)

  /* Addition chain for p - 2:

     2^{521} - 3
     = 1 + 2^2(2^519 - 1)
     = 1 + 2^2(1 + 2 (2^518 - 1)
     = 1 + 2^2(1 + 2 (2^259 + 1) (1 + 2(2^258 - 1)))
     = 1 + 2^2(1 + 2 (2^259 + 1) (1 + 2(2^129 + 1) (2^129 - 1)))
     = 1 + 2^2(1 + 2 (2^259 + 1) (1 + 2(2^129 + 1) (1 + 2 (2^128 - 1))))

     where

     2^{128} - 1 = (2^64 + 1) (2^32+1) (2^16 + 1) (2^8 + 1) (2^4 + 1) (2^2 + 1) (2 + 1)

     This addition chain needs 520 squarings and 13 multiplies.
  */

  ecc_mod_sqr (p, rp, ap, tp);	        /* a^2 */
  ecc_mod_mul (p, rp, ap, rp, tp);	/* a^3 = a^{2^2 - 1} */
  ecc_mod_pow_2kp1 (p, t0, rp, 2, tp);	/* a^15 = a^{2^4 - 1} */
  ecc_mod_pow_2kp1 (p, rp, t0, 4, tp);	/* a^{2^8 - 1} */
  ecc_mod_pow_2kp1 (p, t0, rp, 8, tp);	/* a^{2^16 - 1} */
  ecc_mod_pow_2kp1 (p, rp, t0, 16, tp);	/* a^{2^32 - 1} */
  ecc_mod_pow_2kp1 (p, t0, rp, 32, tp);	/* a^{2^64 - 1} */
  ecc_mod_pow_2kp1 (p, rp, t0, 64, tp);	/* a^{2^128 - 1} */
  ecc_mod_sqr (p, rp, rp, tp);		/* a^{2^129 - 2} */
  ecc_mod_mul (p, rp, rp, ap, tp);	/* a^{2^129 - 1} */
  ecc_mod_pow_2kp1 (p, t0, rp, 129, tp);/* a^{2^258 - 1} */
  ecc_mod_sqr (p, rp, t0, tp);		/* a^{2^259 - 2} */
  ecc_mod_mul (p, rp, rp, ap, tp);	/* a^{2^259 - 1} */
  ecc_mod_pow_2kp1 (p, t0, rp, 259, tp);/* a^{2^518 - 1} */
  ecc_mod_sqr (p, rp, t0, tp);		/* a^{2^519 - 2} */
  ecc_mod_mul (p, rp, rp, ap, tp);	/* a^{2^519 - 1} */
  ecc_mod_sqr (p, rp, rp, tp);		/* a^{2^520 - 2} */
  ecc_mod_sqr (p, rp, rp, tp);		/* a^{2^521 - 4} */
  ecc_mod_mul (p, rp, rp, ap, tp);	/* a^{2^521 - 3} */
}

#define ECC_SECP521R1_SQRT_ITCH (2*ECC_LIMB_SIZE)

static int
ecc_secp521r1_sqrt (const struct ecc_modulo *m,
		    mp_limb_t *rp,
		    const mp_limb_t *cp,
		    mp_limb_t *scratch)
{
  mp_limb_t hi;

  /* This computes the square root modulo p256 using the identity:

     sqrt(c) = c^(2^519) (mod P-521)

     which can be seen as a special case of Tonelli-Shanks with e=1.
  */

  ecc_mod_pow_2k (m, rp, cp, 519, scratch);

  /* Check result. */
  ecc_mod_sqr (m, scratch, rp, scratch);
  ecc_mod_sub (m, scratch, scratch, cp);

  /* Reduce top bits, since ecc_mod_zero_p requires input < 2p */
  hi = scratch[ECC_LIMB_SIZE-1] >> B_SHIFT;
  scratch[ECC_LIMB_SIZE-1] = (scratch[ECC_LIMB_SIZE-1]
			      & (((mp_limb_t) 1 << B_SHIFT)-1))
    + sec_add_1 (scratch, scratch, ECC_LIMB_SIZE - 1, hi);

  return ecc_mod_zero_p (m, scratch);
}


const struct ecc_curve _nettle_secp_521r1 =
{
  {
    521,
    ECC_LIMB_SIZE,    
    ECC_BMODP_SIZE,
    ECC_REDC_SIZE,
    ECC_SECP521R1_INV_ITCH,
    ECC_SECP521R1_SQRT_ITCH,
    0,

    ecc_p,
    ecc_Bmodp,
    ecc_Bmodp_shifted,
    ecc_redc_ppm1,
    ecc_pp1h,

    ecc_secp521r1_modp,
    ecc_secp521r1_modp,
    ecc_secp521r1_inv,
    ecc_secp521r1_sqrt,
    NULL,
  },
  {
    521,
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
  ECC_J_TO_A_ITCH(ECC_LIMB_SIZE, ECC_SECP521R1_INV_ITCH),

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

const struct ecc_curve *nettle_get_secp_521r1(void)
{
  return &_nettle_secp_521r1;
}
