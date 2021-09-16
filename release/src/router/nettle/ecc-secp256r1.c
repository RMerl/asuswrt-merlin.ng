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

#include "ecc.h"
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
  mp_limb_t u1, u0;
  mp_size_t n;

  n = 2*p->size;
  u1 = xp[--n];
  u0 = xp[n-1];

  /* This is not particularly fast, but should work well with assembly implementation. */
  for (; n >= p->size; n--)
    {
      mp_limb_t q2, q1, q0, t, cy;

      /* <q2, q1, q0> = v * u1 + <u1,u0>, with v = 2^32 - 1:

	   +---+---+
	   | u1| u0|
	   +---+---+
	       |-u1|
	     +-+-+-+
	     | u1|
       +---+-+-+-+-+
       | q2| q1| q0|
       +---+---+---+
      */
      q1 = u1 - (u1 > u0);
      q0 = u0 - u1;
      t = u1 << 32;
      q0 += t;
      t = (u1 >> 32) + (q0 < t) + 1;
      q1 += t;
      q2 = q1 < t;

      /* Compute candidate remainder */
      u1 = u0 + (q1 << 32) - q1;
      t = -(mp_limb_t) (u1 > q0);
      u1 -= t & 0xffffffff;
      q1 += t;
      q2 += t + (q1 < t);

      assert (q2 < 2);

      /*
	 n-1 n-2 n-3 n-4
	+---+---+---+---+
	| u1| u0| u low |
	+---+---+---+---+
	  - | q1(2^96-1)|
	    +-------+---+
	    |q2(2^.)|
	    +-------+

	 We multiply by two low limbs of p, 2^96 - 1, so we could use
	 shifts rather than mul.
      */
      t = mpn_submul_1 (xp + n - 4, p->m, 2, q1);
      t += mpn_cnd_sub_n (q2, xp + n - 3, xp + n - 3, p->m, 1);
      t += (-q2) & 0xffffffff;

      u0 = xp[n-2];
      cy = (u0 < t);
      u0 -= t;
      t = (u1 < cy);
      u1 -= cy;

      cy = mpn_cnd_add_n (t, xp + n - 4, xp + n - 4, p->m, 2);
      u0 += cy;
      u1 += (u0 < cy);
      u1 -= (-t) & 0xffffffff;
    }
  rp[0] = xp[0];
  rp[1] = xp[1];
  rp[2] = u0;
  rp[3] = u1;
}

static void
ecc_secp256r1_modq (const struct ecc_modulo *q, mp_limb_t *rp, mp_limb_t *xp)
{
  mp_limb_t u2, u1, u0;
  mp_size_t n;

  n = 2*q->size;
  u2 = xp[--n];
  u1 = xp[n-1];

  /* This is not particularly fast, but should work well with assembly implementation. */
  for (; n >= q->size; n--)
    {
      mp_limb_t q2, q1, q0, t, c1, c0;

      u0 = xp[n-2];

      /* <q2, q1, q0> = v * u2 + <u2,u1>, same method as above.

	   +---+---+
	   | u2| u1|
	   +---+---+
	       |-u2|
	     +-+-+-+
	     | u2|
       +---+-+-+-+-+
       | q2| q1| q0|
       +---+---+---+
      */
      q1 = u2 - (u2 > u1);
      q0 = u1 - u2;
      t = u2 << 32;
      q0 += t;
      t = (u2 >> 32) + (q0 < t) + 1;
      q1 += t;
      q2 = q1 < t;

      /* Compute candidate remainder, <u1, u0> - <q2, q1> * (2^128 - 2^96 + 2^64 - 1)
	 <u1, u0> + 2^64 q2 + (2^96 - 2^64 + 1) q1 (mod 2^128)

	   +---+---+
	   | u1| u0|
	   +---+---+
	   | q2| q1|
	   +---+---+
	   |-q1|
	 +-+-+-+
	 | q1|
       --+-+-+-+---+
	   | u2| u1|
	   +---+---+
      */
      u2 = u1 + q2 - q1;
      u1 = u0 + q1;
      u2 += (u1 < q1);
      u2 += (q1 << 32);

      t = -(mp_limb_t) (u2 >= q0);
      q1 += t;
      q2 += t + (q1 < t);
      u1 += t;
      u2 += (t << 32) + (u1 < t);

      assert (q2 < 2);

      c0 = mpn_cnd_sub_n (q2, xp + n - 3, xp + n - 3, q->m, 1);
      c0 += (-q2) & q->m[1];
      t = mpn_submul_1 (xp + n - 4, q->m, 2, q1);
      c0 += t;
      c1 = c0 < t;

      /* Construct underflow condition. */
      c1 += (u1 < c0);
      t = - (mp_limb_t) (u2 < c1);

      u1 -= c0;
      u2 -= c1;

      /* Conditional add of p */
      u1 += t;
      u2 += (t<<32) + (u1 < t);

      t = mpn_cnd_add_n (t, xp + n - 4, xp + n - 4, q->m, 2);
      u1 += t;
      u2 += (u1 < t);
    }
  rp[0] = xp[0];
  rp[1] = xp[1];
  rp[2] = u1;
  rp[3] = u2;
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
}

const struct ecc_curve _nettle_secp_256r1 =
{
  {
    256,
    ECC_LIMB_SIZE,
    ECC_BMODP_SIZE,
    ECC_REDC_SIZE,
    ECC_SECP256R1_INV_ITCH,
    0,

    ecc_p,
    ecc_Bmodp,
    ecc_Bmodp_shifted,
    ecc_redc_ppm1,
    ecc_pp1h,

    ecc_secp256r1_modp,
    USE_REDC ? ecc_secp256r1_redc : ecc_secp256r1_modp,
    ecc_secp256r1_inv,
    NULL,
  },
  {
    256,
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

    ecc_secp256r1_modq,
    ecc_secp256r1_modq,
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
