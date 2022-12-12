/* ecc-secp256r1.c

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

#if HAVE_NATIVE_ecc_secp256r1_redc
# define USE_REDC 1
#else
# define USE_REDC (ECC_REDC_SIZE != 0)
#endif

#include "ecc-secp256r1.h"

#if HAVE_NATIVE_ecc_secp256r1_redc
# define ecc_secp256r1_redc _nettle_ecc_secp256r1_redc
void
ecc_secp256r1_redc (const struct ecc_modulo *p, mp_limb_t *rp, mp_limb_t *xp);
#else /* !HAVE_NATIVE_ecc_secp256r1_redc */
# if ECC_REDC_SIZE > 0
#   define ecc_secp256r1_redc ecc_pp1_redc
# elif ECC_REDC_SIZE == 0
#   define ecc_secp256r1_redc NULL
# else
#  error Configuration error
# endif
#endif /* !HAVE_NATIVE_ecc_secp256r1_redc */

#if ECC_BMODP_SIZE < ECC_LIMB_SIZE
#define ecc_secp256r1_modp ecc_mod
#define ecc_secp256r1_modq ecc_mod
#elif GMP_NUMB_BITS == 64

static void
ecc_secp256r1_modp (const struct ecc_modulo *p, mp_limb_t *rp, mp_limb_t *xp)
{
  mp_limb_t d1, u1, cy;
  mp_size_t n;

  /* Reduce to < B^4 p up front, to avoid first quotient overflowing a limb. */
  cy = mpn_sub_n (xp + 4, xp + 4, p->m, p->size);
  mpn_cnd_add_n (cy, xp + 4, xp + 4, p->m, p->size);

  d1 = UINT64_C(0xffffffff00000001);
  for (n = 2*p->size, u1 = xp[--n] ;; n--)
    {
      mp_limb_t u0, q1, q0, qmax, r, t, mask;
      u0 = xp[n-1];

      /* Since d0 == 0, 2/1 division gives a good enough quotient
	 approximation.

	 <q1, q0> = v * u1 + <u1,u0>, with v = 2^32 - 1:

	   +---+---+
	   | u1| u0|
	   +---+---+
	       |-u1|
	     +-+-+-+
	     | u1|
           +-+-+-+-+
           | q1| q0|
           +---+---+
      */
      q1 = u1 - (u1 > u0);
      q0 = u0 - u1;
      t = u1 << 32;
      q0 += t;
      q1 += (u1 >> 32) + (q0 < t) + 1;

      /* Force q = B-1 when u1 == d1 */
      qmax = - (mp_limb_t) (u1 >= d1);

      /* Candidate remainder r = u0 - q d1 (mod B), and 2/1 division
	 adjustments. */
      r = u0 + (q1 << 32) - q1;
      mask = - (mp_limb_t) (r > q0);
      q1 += mask;
      r += (mask & d1);
      mask = - (mp_limb_t) (r >= d1);
      q1 -= mask;
      r -= (mask & d1);

      /* In the case that u1 == d1, we get q1 == 0, r == 0 here (and
	 correct 2/1 quotient would be B). Replace with q1 = B-1, r =
	 d1. */
      q1 |= qmax;
      r += d1 & qmax;

      cy = mpn_submul_1 (xp + n - 4, p->m, 3, q1);
      mask = - (mp_limb_t) (r < cy);
      if (n == p->size)
	{
	  rp[3] = r - cy + (mask & d1) + mpn_cnd_add_n (mask, rp, xp, p->m, 3);
	  return;
	}
      u1 = r - cy + (mask & d1) + mpn_cnd_add_n (mask, xp + n - 4, xp + n- 4, p->m, 3);
    }
}

static void
ecc_secp256r1_modq (const struct ecc_modulo *q, mp_limb_t *rp, mp_limb_t *xp)
{
  mp_limb_t d1, cy;
  mp_size_t n;

  /* Reduce to < B^4 p up front, to avoid first quotient overflowing a limb. */
  cy = mpn_sub_n (xp + 4, xp + 4, q->m, q->size);
  mpn_cnd_add_n (cy, xp + 4, xp + 4, q->m, q->size);

  d1 = UINT64_C(0xffffffff00000000);
  n = 2*q->size;
  for (;;)
    {
      mp_limb_t u1, u0, q1, q0, r, t, qmax, mask;
      u1 = xp[--n];
      u0 = xp[n-1];

      /* divappr2, specialized for d1 = 2^64 - 2^32, d0 = 2^64-1.

	 <q1, q0> = v * u1 + <u1,u0>, with v = 2^32 - 1:

	   +---+---+
	   | u1| u0|
	   +---+---+
	       |-u1|
	     +-+-+-+
	     | u1|
           +-+-+-+-+
           | q1| q0|
           +---+---+
      */
      q1 = u1 - (u1 > u0);
      q0 = u0 - u1;
      t = u1 << 32;
      q0 += t;
      q1 += (q0 < t);
      t = u1 >> 32;
      /* The divappr2 algorithm handles only q < B - 1. If we check
	 for u1 >= d1 = 2^{64}-2^{32}, we cover all cases where q =
	 2^64-1, and some when q = 2^64-2. The latter case is
	 corrected by the final adjustment. */
      qmax = - (mp_limb_t) (t == 0xffffffff);
      q1 += t + 1;

      /* Candidate remainder r = u0 - q (d1 + 1) (mod B), and divappr2
	 adjustments.

	 For general divappr2, the expression is

	   r = u_0 - q1 d1 - floor(q1 d0 / B) - 1

	 but in our case floor(q1 d0 / B) simplifies to q1 - 1.
      */
      r = u0 + (q1 << 32) - q1;
      mask = - (mp_limb_t) (r >= q0);
      q1 += mask;
      r += (mask & (d1 + 1));
      q1 += (r >= d1 - 1);

      /* Replace by qmax, when that is needed */
      q1 |= qmax;

      /* Subtract, may underflow. */
      cy = mpn_submul_1 (xp + n - 4, q->m, 4, q1);
      if (n == q->size)
	{
	  mpn_cnd_add_n (cy > u1, rp, xp, q->m, 4);
	  return;
	}
      mpn_cnd_add_n (cy > u1, xp + n - 4, xp + n- 4, q->m, 4);
    }
}

#else
#error Unsupported parameters
#endif

#define ECC_SECP256R1_INV_ITCH (4*ECC_LIMB_SIZE)

static void
ecc_secp256r1_inv (const struct ecc_modulo *p,
		   mp_limb_t *rp, const mp_limb_t *ap,
		   mp_limb_t *scratch)
{
#define a5m1 scratch
#define t0 (scratch + ECC_LIMB_SIZE)
#define a15m1 t0
#define a32m1 a5m1
#define tp (scratch + 2*ECC_LIMB_SIZE)
/*
   Addition chain for p - 2 = 2^{256} - 2^{224} + 2^{192} + 2^{96} - 3

    2^5 - 1 = 1 + 2 (2^4 - 1) = 1 + 2 (2^2+1)(2 + 1)    4 S + 3 M
    2^{15} - 1 = (2^5 - 1) (1 + 2^5 (1 + 2^5)          10 S + 2 M
    2^{16} - 1 = 1 + 2 (2^{15} - 1)                       S +   M
    2^{32} - 1 = (2^{16} + 1) (2^{16} - 1)             16 S +   M
    2^{64} - 2^{32} + 1 = 2^{32} (2^{32} - 1) + 1      32 S +   M
    2^{192} - 2^{160} + 2^{128} + 2^{32} - 1
        = 2^{128} (2^{64} - 2^{32} + 1) + 2^{32} - 1  128 S +   M
    2^{224} - 2^{192} + 2^{160} + 2^{64} - 1
        = 2^{32} (...) + 2^{32} - 1                    32 S +   M
    2^{239} - 2^{207} + 2^{175} + 2^{79} - 1
        = 2^{15} (...) + 2^{15} - 1                    15 S +   M
    2^{254} - 2^{222} + 2^{190} + 2^{94} - 1
        = 2^{15} (...) + 2^{15} - 1                    15 S +   M
    p - 2 = 2^2 (...) + 1                               2 S     M
                                                   ---------------
						      255 S + 13 M
 */
  ecc_mod_sqr (p, rp, ap, tp);			/* a^2 */
  ecc_mod_mul (p, rp, rp, ap, tp);		/* a^3 */
  ecc_mod_pow_2kp1 (p, t0, rp, 2, tp);		/* a^{2^4 - 1} */
  ecc_mod_sqr (p, rp, t0, tp);			/* a^{2^5 - 2} */
  ecc_mod_mul (p, a5m1, rp, ap, tp);		/* a^{2^5 - 1}, a5m1 */

  ecc_mod_pow_2kp1 (p, rp, a5m1, 5, tp);	/* a^{2^{10} - 1, a5m1*/
  ecc_mod_pow_2k_mul (p, a15m1, rp, 5, a5m1, tp); /* a^{2^{15} - 1}, a5m1 a15m1 */
  ecc_mod_sqr (p, rp, a15m1, tp);		/* a^{2^{16} - 2}, a15m1 */
  ecc_mod_mul (p, rp, rp, ap, tp);		/* a^{2^{16} - 1}, a15m1 */
  ecc_mod_pow_2kp1 (p, a32m1, rp, 16, tp);	/* a^{2^{32} - 1}, a15m1, a32m1 */

  ecc_mod_pow_2k_mul (p, rp, a32m1, 32, ap, tp);/* a^{2^{64} - 2^{32} + 1 */
  ecc_mod_pow_2k_mul (p, rp, rp, 128, a32m1, tp); /* a^{2^{192} - 2^{160} + 2^{128} + 2^{32} - 1} */
  ecc_mod_pow_2k_mul (p, rp, rp, 32, a32m1, tp);/* a^{2^{224} - 2^{192} + 2^{160} + 2^{64} - 1} */
  ecc_mod_pow_2k_mul (p, rp, rp, 15, a15m1, tp);/* a^{2^{239} - 2^{207} + 2^{175} + 2^{79} - 1} */
  ecc_mod_pow_2k_mul (p, rp, rp, 15, a15m1, tp);/* a^{2^{254} - 2^{222} + 2^{190} + 2^{94} - 1} */
  ecc_mod_pow_2k_mul (p, rp, rp, 2, ap, tp); 	/* a^{2^{256} - 2^{224} + 2^{192} + 2^{96} - 3} */

#undef a5m1
#undef t0
#undef a15m1
#undef a32m1
#undef tp
}

/* To guarantee that inputs to ecc_mod_zero_p are in the required range. */
#if ECC_LIMB_SIZE * GMP_NUMB_BITS != 256
#error Unsupported limb size
#endif

#define ECC_SECP256R1_SQRT_ITCH (3*ECC_LIMB_SIZE)

static int
ecc_secp256r1_sqrt (const struct ecc_modulo *m,
		    mp_limb_t *rp,
		    const mp_limb_t *cp,
		    mp_limb_t *scratch)
{
  /* This computes the square root modulo p256 using the identity:

     sqrt(c) = c^(2^254 − 2^222 + 2^190 + 2^94)  (mod P-256)

     which can be seen as a special case of Tonelli-Shanks with e=1.

     It would be nice to share part of the addition chain between inverse and sqrt.

     We need

       p-2 = 2^{256} - 2^{224} + 2^{192} + 2^{96} - 3 (inverse)

     and

       (p+1)/4 = 2^{254} − 2^{222} + 2^{190} + 2^{94} (sqrt)

     which we can both get conveniently from

       (p-3)/4 = 2^{254} − 2^{222} + 2^{190} + 2^{94} - 1

     But addition chain for 2^{94} - 1 appears to cost a few more mul
     operations than the current, separate, chains. */

#define t0 scratch
#define tp (scratch + ECC_LIMB_SIZE)

  ecc_mod_sqr        (m, rp, cp, tp);		/* c^2 */
  ecc_mod_mul        (m, t0, rp, cp, tp);	/* c^3 */
  ecc_mod_pow_2kp1   (m, rp, t0, 2, tp);	/* c^(2^4 - 1) */
  ecc_mod_pow_2kp1   (m, t0, rp, 4, tp);	/* c^(2^8 - 1) */
  ecc_mod_pow_2kp1   (m, rp, t0, 8, tp);	/* c^(2^16 - 1) */
  ecc_mod_pow_2kp1   (m, t0, rp, 16, tp);	/* c^(2^32 - 1) */
  ecc_mod_pow_2k_mul (m, rp, t0, 32, cp, tp);	/* c^(2^64 - 2^32 + 1) */
  ecc_mod_pow_2k_mul (m, t0, rp, 96, cp, tp);	/* c^(2^160 - 2^128 + 2^96 + 1) */
  ecc_mod_pow_2k     (m, rp, t0, 94,     tp);	/* c^(2^254 - 2^222 + 2^190 + 2^94) */

  ecc_mod_sqr (m, t0, rp, tp);
  ecc_mod_sub (m, t0, t0, cp);

  return ecc_mod_zero_p (m, t0);
#undef t0
#undef tp

}

const struct ecc_curve _nettle_secp_256r1 =
{
  {
    256,
    ECC_LIMB_SIZE,
    ECC_BMODP_SIZE,
    ECC_REDC_SIZE,
    ECC_SECP256R1_INV_ITCH,
    ECC_SECP256R1_SQRT_ITCH,
    0,

    ecc_p,
    ecc_Bmodp,
    ecc_Bmodp_shifted,
    ecc_redc_ppm1,
    ecc_pp1h,

    ecc_secp256r1_modp,
    USE_REDC ? ecc_secp256r1_redc : ecc_secp256r1_modp,
    ecc_secp256r1_inv,
    ecc_secp256r1_sqrt,
    NULL,
  },
  {
    256,
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

    ecc_secp256r1_modq,
    ecc_secp256r1_modq,
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
  ECC_J_TO_A_ITCH(ECC_LIMB_SIZE, ECC_SECP256R1_INV_ITCH),

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

const struct ecc_curve *nettle_get_secp_256r1(void)
{
  return &_nettle_secp_256r1;
}
