/* ecc-curve448.c

   Arithmetic and tables for curve448,

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

#include <assert.h>

#include "ecc-internal.h"

#define USE_REDC 0

#include "ecc-curve448.h"

#if HAVE_NATIVE_ecc_curve448_modp
#define ecc_curve448_modp _nettle_ecc_curve448_modp
void
ecc_curve448_modp (const struct ecc_modulo *m, mp_limb_t *rp, mp_limb_t *xp);
#elif GMP_NUMB_BITS == 64
static void
ecc_curve448_modp(const struct ecc_modulo *m, mp_limb_t *rp, mp_limb_t *xp)
{
  /* Let B = 2^64, b = 2^32 = sqrt(B).
     p = B^7 - b B^3 - 1 ==> B^7 = b B^3 + 1

     We use this to reduce

     {r_{13}, ..., r_0} =
       {r_6,...,r_0}
     + {r_{10},...,r_7}
     + 2 {r_{13},r_{12}, r_{11}} B^4
     + b {r_{10},...,r_7,r_{13},r_{12},r_{11} (mod p)

     or

             +----+----+----+----+----+----+----+
             |r_6 |r_5 |r_4 |r_3 |r_2 |r_1 |r_0 |
             +----+----+----+----+----+----+----+
                            |r_10|r_9 |r_8 |r_7 |
             +----+----+----+----+----+----+----+
         2 * |r_13|r_12|r_11|
             +----+----+----+----+----+----+----+
      +  b * |r_10|r_9 |r_8 |r_7 |r_13|r_12|r_11|
      -------+----+----+----+----+----+----+----+
         c_7 |r_6 |r_5 |r_4 |r_3 |r_2 |r_1 |r_0 |
             +----+----+----+----+----+----+----+
  */
  mp_limb_t c3, c4, c7;
  mp_limb_t *tp = xp + 7;

  c4 = mpn_add_n (xp, xp, xp + 7, 4);
  c7 = mpn_addmul_1 (xp + 4, xp + 11, 3, 2);
  c3 = mpn_addmul_1 (xp, xp + 11, 3, (mp_limb_t) 1 << 32);
  c7 += mpn_addmul_1 (xp + 3, xp + 7, 4, (mp_limb_t) 1 << 32);
  tp[0] = c7;
  tp[1] = tp[2] = 0;
  tp[3] = c3 + (c7 << 32);
  tp[4] = c4 + (c7 >> 32) + (tp[3] < c3);
  tp[5] = tp[6] = 0;
  c7 = mpn_add_n (rp, xp, tp, 7);
  c7 = mpn_cnd_add_n (c7, rp, rp, m->B, 7);
  assert (c7 == 0);
}
#else
#define ecc_curve448_modp ecc_mod
#endif

/* Computes a^{(p-3)/4} = a^{2^446-2^222-1} mod m. Needs 4 * n scratch
   space. */
static void
ecc_mod_pow_446m224m1 (const struct ecc_modulo *p,
		       mp_limb_t *rp, const mp_limb_t *ap,
		       mp_limb_t *scratch)
{
/* Note overlap: operations writing to t0 clobber t1. */
#define t0 scratch
#define t1 (scratch + ECC_LIMB_SIZE)
#define tp (scratch + 2*ECC_LIMB_SIZE)

  /* Set t0 = a^7 */
  ecc_mod_sqr (p, t0, ap, tp);		/* a^2 */
  ecc_mod_mul (p, t0, ap, t0, tp);	/* a^3 */
  ecc_mod_sqr (p, t0, t0, tp);		/* a^6 */
  ecc_mod_mul (p, t0, ap, t0, tp);	/* a^{2^3-1} */

  /* Set t0 = a^{2^18-1} */
  ecc_mod_pow_2kp1 (p, rp, t0, 3, tp);	/* a^{2^6-1} */
  ecc_mod_pow_2k (p, rp, rp, 3, tp);	/* a^{2^9-2^3} */
  ecc_mod_mul (p, rp, rp, t0, tp);	/* a^{2^9-1} */
  ecc_mod_pow_2kp1 (p, t0, rp, 9, tp);	/* a^{2^18-1} */

  /* Set t0 = a^{2^37-1} */
  ecc_mod_sqr (p, rp, t0, tp);		/* a^{2^19-2} */
  ecc_mod_mul (p, rp, ap, rp, tp);	/* a^{2^19-1} */
  ecc_mod_pow_2k (p, t1, rp, 18, tp);	/* a^{2^37-2^18} */
  ecc_mod_mul (p, t0, t0, t1, tp);	/* a^{2^37-1} */

  /* Set t0 = a^{2^222-1} */
  ecc_mod_pow_2kp1 (p, rp, t0, 37, tp);	/* a^{2^74-1} */
  ecc_mod_pow_2k (p, t1, rp, 37, tp);	/* a^{2^111-2^37} */
  ecc_mod_mul (p, t1, t1, t0, tp);	/* a^{2^111-1} */
  ecc_mod_pow_2kp1 (p, t0, t1, 111, tp);/* a^{2^222-1} */

  ecc_mod_sqr (p, rp, t0, tp);		/* a^{2^223-2} */
  ecc_mod_mul (p, rp, rp, ap, tp);	/* a^{2^223-1} */
  ecc_mod_pow_2k (p, t1, rp, 223, tp);	/* a^{2^446-2^223} */
  ecc_mod_mul (p, rp, t1, t0, tp);	/* a^{2^446-2^222-1} */
#undef t0
#undef t1
#undef tp
}

#define ECC_CURVE448_INV_ITCH (4*ECC_LIMB_SIZE)

static void ecc_curve448_inv (const struct ecc_modulo *p,
			 mp_limb_t *rp, const mp_limb_t *ap,
			 mp_limb_t *tp)
{
  ecc_mod_pow_446m224m1 (p, rp, ap, tp);/* a^{2^446-2^222-1} */
  ecc_mod_sqr (p, rp, rp, tp);		/* a^{2^447-2^223-2} */
  ecc_mod_sqr (p, rp, rp, tp);		/* a^{2^448-2^224-4} */
  ecc_mod_mul (p, rp, ap, rp, tp);	/* a^{2^448-2^224-3} */
}

/* To guarantee that inputs to ecc_mod_zero_p are in the required range. */
#if ECC_LIMB_SIZE * GMP_NUMB_BITS != 448
#error Unsupported limb size
#endif

/* Compute x such that x^2 = u/v (mod p). Returns one on success, zero
   on failure.

   To avoid a separate inversion, we use a trick of djb's, to
   compute the candidate root as

     x = (u/v)^{(p+1)/4} = u^3 v (u^5 v^3)^{(p-3)/4}.
*/

/* Needs 2*n space + scratch for ecc_mod_pow_446m224m1. */
#define ECC_CURVE448_SQRT_RATIO_ITCH (6*ECC_LIMB_SIZE)

static int
ecc_curve448_sqrt_ratio(const struct ecc_modulo *p, mp_limb_t *rp,
			const mp_limb_t *up, const mp_limb_t *vp,
			mp_limb_t *scratch)
{
#define uv scratch
#define u3v (scratch + ECC_LIMB_SIZE)
#define u5v3 uv

#define t0 scratch
#define scratch_out (scratch + 2*ECC_LIMB_SIZE)
						/* Live values */
  ecc_mod_mul (p, uv, up, vp, scratch_out);	/* uv */
  ecc_mod_sqr (p, u3v, up, scratch_out);	/* uv, u3v */
  ecc_mod_mul (p, u3v, u3v, uv, scratch_out);	/* uv, u3v */

  ecc_mod_sqr (p, u5v3, uv, scratch_out);	/* u5v3, u3v */
  ecc_mod_mul (p, u5v3, u5v3, u3v, scratch_out);/* u5v3, u3v */

  ecc_mod_pow_446m224m1 (p, rp, u5v3, scratch_out); /* u3v */
  ecc_mod_mul (p, rp, rp, u3v, scratch_out);

  /* If square root exists, have v x^2 = u */
  ecc_mod_sqr (p, t0, rp, scratch_out);		/* x^2 */
  ecc_mod_mul (p, t0, t0, vp, scratch_out);	/* v x^2 */
  ecc_mod_sub (p, t0, t0, up);

  return ecc_mod_zero_p (p, t0);
#undef uv
#undef u3v
#undef u5v3
#undef t0
#undef scratch_out
}

const struct ecc_curve _nettle_curve448 =
{
  {
    448,
    ECC_LIMB_SIZE,
    ECC_BMODP_SIZE,
    0,
    ECC_CURVE448_INV_ITCH,
    0,
    ECC_CURVE448_SQRT_RATIO_ITCH,

    ecc_p,
    ecc_Bmodp,
    ecc_Bmodp_shifted,
    NULL,
    ecc_pp1h,

    ecc_curve448_modp,
    ecc_curve448_modp,
    ecc_curve448_inv,
    NULL,
    ecc_curve448_sqrt_ratio,
  },
  {
    446,
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

  0, /* No redc */
  ECC_PIPPENGER_K,
  ECC_PIPPENGER_C,

  ECC_ADD_EH_ITCH (ECC_LIMB_SIZE),
  ECC_ADD_EHH_ITCH (ECC_LIMB_SIZE),
  ECC_DUP_EH_ITCH (ECC_LIMB_SIZE),
  ECC_MUL_A_EH_ITCH (ECC_LIMB_SIZE),
  ECC_MUL_G_EH_ITCH (ECC_LIMB_SIZE),
  ECC_EH_TO_A_ITCH (ECC_LIMB_SIZE, ECC_CURVE448_INV_ITCH),

  ecc_add_eh,
  ecc_add_ehh,
  ecc_dup_eh,
  ecc_mul_a_eh,
  ecc_mul_g_eh,
  ecc_eh_to_a,

  ecc_b,
  ecc_unit,
  ecc_table
};
