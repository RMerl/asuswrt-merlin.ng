/* ecc-curve25519.c

   Arithmetic and tables for curve25519,

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

#include "ecc-internal.h"

#define USE_REDC 0

#include "ecc-curve25519.h"

#define PHIGH_BITS (GMP_NUMB_BITS * ECC_LIMB_SIZE - 255)

#if HAVE_NATIVE_ecc_curve25519_modp

#define ecc_curve25519_modp _nettle_ecc_curve25519_modp
void
ecc_curve25519_modp (const struct ecc_modulo *m, mp_limb_t *rp, mp_limb_t *xp);
#else

#if PHIGH_BITS == 0
#error Unsupported limb size */
#endif

static void
ecc_curve25519_modp(const struct ecc_modulo *m UNUSED, mp_limb_t *rp, mp_limb_t *xp)
{
  mp_limb_t hi, cy;

  cy = mpn_addmul_1 (xp, xp + ECC_LIMB_SIZE, ECC_LIMB_SIZE,
		     (mp_limb_t) 19 << PHIGH_BITS);
  hi = xp[ECC_LIMB_SIZE-1];
  cy = (cy << PHIGH_BITS) + (hi >> (GMP_NUMB_BITS - PHIGH_BITS));
  rp[ECC_LIMB_SIZE-1] = (hi & (GMP_NUMB_MASK >> PHIGH_BITS))
    + sec_add_1 (rp, xp, ECC_LIMB_SIZE - 1, 19 * cy);
}
#endif /* HAVE_NATIVE_ecc_curve25519_modp */

#define QHIGH_BITS (GMP_NUMB_BITS * ECC_LIMB_SIZE - 252)

#if QHIGH_BITS == 0
#error Unsupported limb size */
#endif

static void
ecc_curve25519_modq (const struct ecc_modulo *q, mp_limb_t *rp, mp_limb_t *xp)
{
  mp_size_t n;
  mp_limb_t cy;

  /* n is the offset where we add in the next term */
  for (n = ECC_LIMB_SIZE; n-- > 0;)
    {
      cy = mpn_submul_1 (xp + n,
			 q->B_shifted, ECC_LIMB_SIZE,
			 xp[n + ECC_LIMB_SIZE]);
      /* Top limb of mBmodq_shifted is zero, so we get cy == 0 or 1 */
      assert (cy < 2);
      mpn_cnd_add_n (cy, xp+n, xp+n, q->m, ECC_LIMB_SIZE);
    }

  cy = mpn_submul_1 (xp, q->m, ECC_LIMB_SIZE,
		     xp[ECC_LIMB_SIZE-1] >> (GMP_NUMB_BITS - QHIGH_BITS));
  assert (cy < 2);
  mpn_cnd_add_n (cy, rp, xp, q->m, ECC_LIMB_SIZE);
}

/* Computes a^{(p-5)/8} = a^{2^{252}-3} mod m. Needs 4 * n scratch
   space. */
static void
ecc_mod_pow_252m3 (const struct ecc_modulo *m,
		   mp_limb_t *rp, const mp_limb_t *ap, mp_limb_t *scratch)
{
#define a7 scratch
#define t0 (scratch + ECC_LIMB_SIZE)
#define tp (scratch + 2*ECC_LIMB_SIZE)

  /* a^{2^252 - 3} = a^{(p-5)/8}, using the addition chain
     2^252 - 3
     = 1 + (2^252-4)
     = 1 + 4 (2^250-1)
     = 1 + 4 (2^125+1)(2^125-1)
     = 1 + 4 (2^125+1)(1+2(2^124-1))
     = 1 + 4 (2^125+1)(1+2(2^62+1)(2^62-1))
     = 1 + 4 (2^125+1)(1+2(2^62+1)(2^31+1)(2^31-1))
     = 1 + 4 (2^125+1)(1+2(2^62+1)(2^31+1)(7+8(2^28-1)))
     = 1 + 4 (2^125+1)(1+2(2^62+1)(2^31+1)(7+8(2^14+1)(2^14-1)))
     = 1 + 4 (2^125+1)(1+2(2^62+1)(2^31+1)(7+8(2^14+1)(2^7+1)(2^7-1)))
     = 1 + 4 (2^125+1)(1+2(2^62+1)(2^31+1)(7+8(2^14+1)(2^7+1)(1+2(2^6-1))))
     = 1 + 4 (2^125+1)(1+2(2^62+1)(2^31+1)(7+8(2^14+1)(2^7+1)(1+2(2^3+1)*7)))
  */ 
     
  ecc_mod_pow_2kp1 (m, a7, ap, 1, tp);	/* a^3 */
  ecc_mod_sqr (m, a7, a7, tp);		/* a^6 */
  ecc_mod_mul (m, a7, a7, ap, tp);	/* a^7 */
  ecc_mod_pow_2kp1 (m, rp, a7, 3, tp);	/* a^63 = a^{2^6-1} */
  ecc_mod_sqr (m, rp, rp, tp);		/* a^{2^7-2} */
  ecc_mod_mul (m, rp, rp, ap, tp);	/* a^{2^7-1} */
  ecc_mod_pow_2kp1 (m, t0, rp, 7, tp);	/* a^{2^14-1}*/
  ecc_mod_pow_2kp1 (m, rp, t0, 14, tp);	/* a^{2^28-1} */
  ecc_mod_sqr (m, rp, rp, tp);		/* a^{2^29-2} */
  ecc_mod_sqr (m, rp, rp, tp);		/* a^{2^30-4} */
  ecc_mod_sqr (m, rp, rp, tp);		/* a^{2^31-8} */
  ecc_mod_mul (m, rp, rp, a7, tp);	/* a^{2^31-1} */
  ecc_mod_pow_2kp1 (m, t0, rp, 31, tp);	/* a^{2^62-1} */
  ecc_mod_pow_2kp1 (m, rp, t0, 62, tp);	/* a^{2^124-1}*/
  ecc_mod_sqr (m, rp, rp, tp);		/* a^{2^125-2} */
  ecc_mod_mul (m, rp, rp, ap, tp);	/* a^{2^125-1} */
  ecc_mod_pow_2kp1 (m, t0, rp, 125, tp);/* a^{2^250-1} */
  ecc_mod_sqr (m, rp, t0, tp);		/* a^{2^251-2} */
  ecc_mod_sqr (m, rp, rp, tp);		/* a^{2^252-4} */
  ecc_mod_mul (m, rp, rp, ap, tp);    	/* a^{2^252-3} */
#undef a7
#undef t0
#undef tp
}

/* Scratch as for ecc_mod_pow_252m3 above. */
#define ECC_25519_INV_ITCH (4*ECC_LIMB_SIZE)

static void
ecc_curve25519_inv (const struct ecc_modulo *p,
		    mp_limb_t *rp, const mp_limb_t *ap,
		    mp_limb_t *scratch)
{
  /* Addition chain

       p - 2 = 2^{255} - 21
             = 1 + 2 (1 + 4 (2^{252}-3))
  */
  ecc_mod_pow_252m3 (p, rp, ap, scratch);
  ecc_mod_sqr (p, rp, rp, scratch);
  ecc_mod_sqr (p, rp, rp, scratch);
  ecc_mod_mul (p, rp, ap, rp, scratch);
  ecc_mod_sqr (p, rp, rp, scratch);
  ecc_mod_mul (p, rp, ap, rp, scratch);
}

static int
ecc_curve25519_zero_p (const struct ecc_modulo *p, mp_limb_t *xp)
{
/* First, reduce to < 2p. */
#if PHIGH_BITS > 0
  mp_limb_t hi = xp[ECC_LIMB_SIZE-1];
  xp[ECC_LIMB_SIZE-1] = (hi & (GMP_NUMB_MASK >> PHIGH_BITS))
    + sec_add_1 (xp, xp, ECC_LIMB_SIZE - 1, 19 * (hi >> (GMP_NUMB_BITS - PHIGH_BITS)));
#endif

  return ecc_mod_zero_p (p, xp);
}

/* Compute x such that x^2 = u/v (mod p). Returns one on success, zero
   on failure. We use the e = 2 special case of the Shanks-Tonelli
   algorithm (see http://www.math.vt.edu/people/brown/doc/sqrts.pdf,
   or Henri Cohen, Computational Algebraic Number Theory, 1.5.1).

   To avoid a separate inversion, we also use a trick of djb's, to
   compute the candidate root as

     x = (u/v)^{(p+3)/8} = u v^3 (u v^7)^{(p-5)/8}.
*/
#if ECC_SQRT_E != 2
#error Broken curve25519 parameters
#endif

/* Needs 2*n space + scratch for ecc_mod_pow_252m3. */
#define ECC_25519_SQRT_RATIO_ITCH (6*ECC_LIMB_SIZE)

static int
ecc_curve25519_sqrt_ratio(const struct ecc_modulo *p, mp_limb_t *rp,
			  const mp_limb_t *up, const mp_limb_t *vp,
			  mp_limb_t *scratch)
{
  int pos, neg;

#define uv3 scratch
#define uv7 (scratch + ECC_LIMB_SIZE)

#define v2 uv7
#define uv uv3
#define v4 uv7

#define scratch_out (scratch + 2 * ECC_LIMB_SIZE)

#define x2 scratch
#define vx2 (scratch + ECC_LIMB_SIZE)
#define t0 (scratch + 2*ECC_LIMB_SIZE)

						/* Live values */
  ecc_mod_sqr (p, v2, vp, scratch_out);		/* v2 */
  ecc_mod_mul (p, uv, up, vp, scratch_out);	/* uv, v2 */
  ecc_mod_mul (p, uv3, uv, v2, scratch_out);	/* uv3, v2 */
  ecc_mod_sqr (p, v4, v2, scratch_out);		/* uv3, v4 */
  ecc_mod_mul (p, uv7, uv3, v4, scratch_out);	/* uv7 */
  ecc_mod_pow_252m3 (p, rp, uv7, scratch_out);	/* uv3, uv7p */
  ecc_mod_mul (p, rp, rp, uv3, scratch_out);	/* none */

  /* Check sign. If square root exists, have v x^2 = ±u */
  ecc_mod_sqr (p, x2, rp, t0);
  ecc_mod_mul (p, vx2, x2, vp, t0);
  ecc_mod_add (p, t0, vx2, up);
  neg = ecc_curve25519_zero_p (p, t0);
  ecc_mod_sub (p, t0, up, vx2);
  pos = ecc_curve25519_zero_p (p, t0);

  ecc_mod_mul (p, t0, rp, ecc_sqrt_z, t0);
  cnd_copy (neg, rp, t0, ECC_LIMB_SIZE);
  return pos | neg;

#undef uv3
#undef uv7
#undef v2
#undef uv
#undef v4
#undef scratch_out
#undef x2
#undef vx2
#undef t0
}

const struct ecc_curve _nettle_curve25519 =
{
  {
    255,
    ECC_LIMB_SIZE,
    ECC_BMODP_SIZE,
    0,
    ECC_25519_INV_ITCH,
    0,
    ECC_25519_SQRT_RATIO_ITCH,

    ecc_p,
    ecc_Bmodp,
    ecc_Bmodp_shifted,
    NULL,
    ecc_pp1h,

    ecc_curve25519_modp,
    ecc_curve25519_modp,
    ecc_curve25519_inv,
    NULL,
    ecc_curve25519_sqrt_ratio,
  },
  {
    253,
    ECC_LIMB_SIZE,
    ECC_BMODQ_SIZE,
    0,
    ECC_MOD_INV_ITCH (ECC_LIMB_SIZE),
    0,
    0,

    ecc_q,
    ecc_Bmodq,  
    ecc_mBmodq_shifted, /* Use q - 2^{252} instead. */
    NULL,
    ecc_qp1h,

    ecc_curve25519_modq,
    ecc_curve25519_modq,
    ecc_mod_inv,
    NULL,
    NULL,
  },

  0, /* No redc */
  ECC_PIPPENGER_K,
  ECC_PIPPENGER_C,

  ECC_ADD_TH_ITCH (ECC_LIMB_SIZE),
  ECC_ADD_THH_ITCH (ECC_LIMB_SIZE),
  ECC_DUP_TH_ITCH (ECC_LIMB_SIZE),
  ECC_MUL_A_EH_ITCH (ECC_LIMB_SIZE),
  ECC_MUL_G_EH_ITCH (ECC_LIMB_SIZE),
  ECC_EH_TO_A_ITCH (ECC_LIMB_SIZE, ECC_25519_INV_ITCH),

  ecc_add_th,
  ecc_add_thh,
  ecc_dup_th,
  ecc_mul_a_eh,
  ecc_mul_g_eh,
  ecc_eh_to_a,

  ecc_b, /* Edwards curve constant. */
  ecc_unit,
  ecc_table
};
