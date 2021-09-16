/* rsa-sec-compute-root.c

   Side-channel silent RSA root computation.

   Copyright (C) 2018 Niels Möller
   Copyright (C) 2018 Red Hat, Inc

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

#include "rsa.h"
#include "rsa-internal.h"
#include "gmp-glue.h"

#if !NETTLE_USE_MINI_GMP
#define MAX(a, b) ((a) > (b) ? (a) : (b))

/* Like mpn_sec_mul_itch, monotonously increasing in operand sizes. */
static mp_size_t
sec_mul_itch (mp_size_t an, mp_size_t bn)
{
  if (an >= bn)
    return mpn_sec_mul_itch (an, bn);
  else
    return mpn_sec_mul_itch (bn, an);
}

/* Writes an + bn limbs to the rp area */
static void
sec_mul (mp_limb_t *rp,
	 const mp_limb_t *ap, mp_size_t an,
	 const mp_limb_t *bp, mp_size_t bn, mp_limb_t *scratch)
{
  if (an >= bn)
    mpn_sec_mul (rp, ap, an, bp, bn, scratch);
  else
    mpn_sec_mul (rp, bp, bn, ap, an, scratch);
}

static mp_size_t
sec_mod_mul_itch (mp_size_t an, mp_size_t bn, mp_size_t mn)
{
  mp_size_t mul_itch = sec_mul_itch (an, bn);
  mp_size_t mod_itch = mpn_sec_div_r_itch (an + bn, mn);
  return MAX(mul_itch, mod_itch);
}

/* Sets r <-- a b % m. Needs space for an + bn limbs at rp. It is
   required than an + bn >= mn. */
static void
sec_mod_mul (mp_limb_t *rp,
	     const mp_limb_t *ap, mp_size_t an,
	     const mp_limb_t *bp, mp_size_t bn,
	     const mp_limb_t *mp, mp_size_t mn,
	     mp_limb_t *scratch)
{
  assert (an + bn >= mn);
  sec_mul (rp, ap, an, bp, bn, scratch);
  mpn_sec_div_r (rp, an + bn, mp, mn, scratch);
}

static mp_size_t
sec_powm_itch (mp_size_t bn, mp_size_t en, mp_size_t mn)
{
  mp_size_t mod_itch = bn + mpn_sec_div_r_itch (bn, mn);
  mp_size_t pow_itch = mn + mpn_sec_powm_itch (mn, en * GMP_NUMB_BITS, mn);
  return MAX (mod_itch, pow_itch);
}

/* Sets r <-- b ^ e % m. Performs an initial reduction b mod m, and
   requires bn >= mn. */
static void
sec_powm (mp_limb_t *rp,
	  const mp_limb_t *bp, mp_size_t bn,
	  const mp_limb_t *ep, mp_size_t en,
	  const mp_limb_t *mp, mp_size_t mn, mp_limb_t *scratch)
{
  assert (bn >= mn);
  assert (en <= mn);
  mpn_copyi (scratch, bp, bn);
  mpn_sec_div_r (scratch, bn, mp, mn, scratch + bn);
  mpn_sec_powm (rp, scratch, mn, ep, en * GMP_NUMB_BITS, mp, mn,
		scratch + mn);
}

mp_size_t
_rsa_sec_compute_root_itch (const struct rsa_private_key *key)
{
  mp_size_t nn = NETTLE_OCTET_SIZE_TO_LIMB_SIZE (key->size);
  mp_size_t pn = mpz_size (key->p);
  mp_size_t qn = mpz_size (key->q);
  mp_size_t an = mpz_size (key->a);
  mp_size_t bn = mpz_size (key->b);
  mp_size_t cn = mpz_size (key->c);

  mp_size_t powm_p_itch = sec_powm_itch (nn, an, pn);
  mp_size_t powm_q_itch = sec_powm_itch (nn, bn, qn);
  mp_size_t mod_mul_itch = cn + MAX(pn, qn) 
    + sec_mod_mul_itch (MAX(pn, qn), cn, pn);

  mp_size_t mul_itch = sec_mul_itch (qn, pn);
  mp_size_t add_1_itch = mpn_sec_add_1_itch (nn - qn);

  /* pn + qn for the product q * r_mod_p' */
  mp_size_t itch = pn + qn + MAX (mul_itch, add_1_itch);

  itch = MAX (itch, powm_p_itch);
  itch = MAX (itch, powm_q_itch);
  itch = MAX (itch, mod_mul_itch);

  /* pn + qn for the r_mod_p and r_mod_q temporaries. */
  return pn + qn + itch;
}

void
_rsa_sec_compute_root (const struct rsa_private_key *key,
		       mp_limb_t *rp, const mp_limb_t *mp,
		       mp_limb_t *scratch)
{
  mp_size_t nn = NETTLE_OCTET_SIZE_TO_LIMB_SIZE (key->size);

  /* The common case is pn = qn. This function would be simpler if we
   * could require that pn >= qn. */
  const mp_limb_t *pp = mpz_limbs_read (key->p);
  const mp_limb_t *qp = mpz_limbs_read (key->q);

  mp_size_t pn = mpz_size (key->p);
  mp_size_t qn = mpz_size (key->q);
  mp_size_t an = mpz_size (key->a);
  mp_size_t bn = mpz_size (key->b);
  mp_size_t cn = mpz_size (key->c);

  mp_limb_t *r_mod_p = scratch;
  mp_limb_t *r_mod_q = scratch + pn;
  mp_limb_t *scratch_out = r_mod_q + qn;
  mp_limb_t cy;

  assert (pn <= nn);
  assert (qn <= nn);
  assert (an <= pn);
  assert (bn <= qn);
  assert (cn <= pn);

  /* Compute r_mod_p = m^d % p = (m%p)^a % p */
  sec_powm (r_mod_p, mp, nn, mpz_limbs_read (key->a), an, pp, pn, scratch_out);
  /* Compute r_mod_q = m^d % q = (m%q)^b % q */
  sec_powm (r_mod_q, mp, nn, mpz_limbs_read (key->b), bn, qp, qn, scratch_out);

  /* Set r_mod_p' = r_mod_p * c % p - r_mod_q * c % p . */
  sec_mod_mul (scratch_out, r_mod_p, pn, mpz_limbs_read (key->c), cn, pp, pn,
	       scratch_out + cn + pn);
  mpn_copyi (r_mod_p, scratch_out, pn);

  sec_mod_mul (scratch_out, r_mod_q, qn, mpz_limbs_read (key->c), cn, pp, pn,
	       scratch_out + cn + qn);
  cy = mpn_sub_n (r_mod_p, r_mod_p, scratch_out, pn);
  mpn_cnd_add_n (cy, r_mod_p, r_mod_p, pp, pn);

  /* Finally, compute x = r_mod_q + q r_mod_p' */
  sec_mul (scratch_out, qp, qn, r_mod_p, pn, scratch_out + pn + qn);

  cy = mpn_add_n (rp, scratch_out, r_mod_q, qn);
  mpn_sec_add_1 (rp + qn, scratch_out + qn, nn - qn, cy, scratch_out + pn + qn);
}
#endif
