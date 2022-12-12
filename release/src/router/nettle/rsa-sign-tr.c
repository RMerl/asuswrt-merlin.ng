/* rsa-sign-tr.c

   Creating RSA signatures, with some additional checks.

   Copyright (C) 2001, 2015 Niels Möller
   Copyright (C) 2012 Nikos Mavrogiannopoulos
   Copyright (C) 2018 Red Hat Inc.

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

#include "gmp-glue.h"
#include "rsa.h"
#include "rsa-internal.h"

#define MAX(a, b) ((a) > (b) ? (a) : (b))

#if NETTLE_USE_MINI_GMP
/* Blinds m, by computing c = m r^e (mod n), for a random r. Also
   returns the inverse (ri), for use by rsa_unblind. */
static void
rsa_blind (const struct rsa_public_key *pub,
	   void *random_ctx, nettle_random_func *random,
	   mpz_t c, mpz_t ri, const mpz_t m)
{
  mpz_t r;

  mpz_init(r);

  /* c = m*(r^e)
   * ri = r^(-1)
   */
  do
    {
      nettle_mpz_random(r, random_ctx, random, pub->n);
      /* invert r */
    }
  while (!mpz_invert (ri, r, pub->n));

  /* c = c*(r^e) mod n */
  mpz_powm_sec(r, r, pub->e, pub->n);
  mpz_mul(c, m, r);
  mpz_fdiv_r(c, c, pub->n);

  mpz_clear(r);
}

/* m = c ri mod n */
static void
rsa_unblind (const struct rsa_public_key *pub,
	     mpz_t m, const mpz_t ri, const mpz_t c)
{
  mpz_mul(m, c, ri);
  mpz_fdiv_r(m, m, pub->n);
}

/* Checks for any errors done in the RSA computation. That avoids
 * attacks which rely on faults on hardware, or even software MPI
 * implementation. */
int
rsa_compute_root_tr(const struct rsa_public_key *pub,
		    const struct rsa_private_key *key,
		    void *random_ctx, nettle_random_func *random,
		    mpz_t x, const mpz_t m)
{
  int res;
  mpz_t t, mb, xb, ri;

  /* mpz_powm_sec handles only odd moduli. If p, q or n is even, the
     key is invalid and rejected by rsa_private_key_prepare. However,
     some applications, notably gnutls, don't use this function, and
     we don't want an invalid key to lead to a crash down inside
     mpz_powm_sec. So do an additional check here. */
  if (mpz_even_p (pub->n) || mpz_even_p (key->p) || mpz_even_p (key->q))
    return 0;

  mpz_init (mb);
  mpz_init (xb);
  mpz_init (ri);
  mpz_init (t);

  rsa_blind (pub, random_ctx, random, mb, ri, m);

  rsa_compute_root (key, xb, mb);

  mpz_powm_sec(t, xb, pub->e, pub->n);
  res = (mpz_cmp(mb, t) == 0);

  if (res)
    rsa_unblind (pub, x, ri, xb);

  mpz_clear (mb);
  mpz_clear (xb);
  mpz_clear (ri);
  mpz_clear (t);

  return res;
}

int
_rsa_sec_compute_root_tr(const struct rsa_public_key *pub,
			 const struct rsa_private_key *key,
			 void *random_ctx, nettle_random_func *random,
			 mp_limb_t *x, const mp_limb_t *m)
{
  mp_size_t nn;
  mpz_t mz;
  mpz_t xz;
  int res;

  mpz_init(xz);

  nn = mpz_size (pub->n);

  res = rsa_compute_root_tr(pub, key, random_ctx, random, xz,
			    mpz_roinit_n(mz, m, nn));

  if (res)
    mpz_limbs_copy(x, xz, nn);

  mpz_clear(xz);
  return res;
}
#else
/* Blinds m, by computing c = m r^e (mod n), for a random r. Also
   returns the inverse (ri), for use by rsa_unblind. Must have c != m,
   no in-place operation.*/
static void
rsa_sec_blind (const struct rsa_public_key *pub,
               void *random_ctx, nettle_random_func *random,
               mp_limb_t *c, mp_limb_t *ri, const mp_limb_t *m)
{
  const mp_limb_t *ep = mpz_limbs_read (pub->e);
  const mp_limb_t *np = mpz_limbs_read (pub->n);
  mp_bitcnt_t ebn = mpz_sizeinbase (pub->e, 2);
  mp_size_t nn = mpz_size (pub->n);
  size_t itch;
  size_t i2;
  mp_limb_t *scratch;
  TMP_GMP_DECL (tp, mp_limb_t);
  TMP_GMP_DECL (rp, mp_limb_t);
  TMP_GMP_DECL (r, uint8_t);

  TMP_GMP_ALLOC (rp, nn);
  TMP_GMP_ALLOC (r, nn * sizeof(mp_limb_t));

  /* c = m*(r^e) mod n */
  itch = mpn_sec_powm_itch(nn, ebn, nn);
  i2 = mpn_sec_mul_itch(nn, nn);
  itch = MAX(itch, i2);
  i2 = mpn_sec_div_r_itch(2*nn, nn);
  itch = MAX(itch, i2);
  i2 = mpn_sec_invert_itch(nn);
  itch = MAX(itch, i2);

  TMP_GMP_ALLOC (tp, 2*nn  + itch);
  scratch = tp + 2*nn;

  /* ri = r^(-1) */
  do
    {
      random(random_ctx, nn * sizeof(mp_limb_t), (uint8_t *)r);
      mpn_set_base256(rp, nn, r, nn * sizeof(mp_limb_t));
      mpn_copyi(tp, rp, nn);
      /* invert r */
    }
  while (!mpn_sec_invert (ri, tp, np, nn, 2 * nn * GMP_NUMB_BITS, scratch));

  mpn_sec_powm (c, rp, nn, ep, ebn, np, nn, scratch);
  mpn_sec_mul (tp, c, nn, m, nn, scratch);
  mpn_sec_div_r (tp, 2*nn, np, nn, scratch);
  mpn_copyi(c, tp, nn);

  TMP_GMP_FREE (r);
  TMP_GMP_FREE (rp);
  TMP_GMP_FREE (tp);
}

/* m = c ri mod n. Allows x == c. */
static void
rsa_sec_unblind (const struct rsa_public_key *pub,
                 mp_limb_t *x, mp_limb_t *ri, const mp_limb_t *c)
{
  const mp_limb_t *np = mpz_limbs_read (pub->n);
  mp_size_t nn = mpz_size (pub->n);

  size_t itch;
  size_t i2;
  mp_limb_t *scratch;
  TMP_GMP_DECL(tp, mp_limb_t);

  itch = mpn_sec_mul_itch(nn, nn);
  i2 = mpn_sec_div_r_itch(nn + nn, nn);
  itch = MAX(itch, i2);

  TMP_GMP_ALLOC (tp, nn + nn + itch);
  scratch = tp + nn + nn;

  mpn_sec_mul (tp, c, nn, ri, nn, scratch);
  mpn_sec_div_r (tp, nn + nn, np, nn, scratch);
  mpn_copyi(x, tp, nn);

  TMP_GMP_FREE (tp);
}

static int
sec_equal(const mp_limb_t *a, const mp_limb_t *b, size_t limbs)
{
  volatile mp_limb_t z = 0;
  size_t i;

  for (i = 0; i < limbs; i++)
    {
      z |= (a[i] ^ b[i]);
    }

  return z == 0;
}

static int
rsa_sec_check_root(const struct rsa_public_key *pub,
                   const mp_limb_t *x, const mp_limb_t *m)
{
  mp_size_t nn = mpz_size (pub->n);
  mp_size_t ebn = mpz_sizeinbase (pub->e, 2);
  const mp_limb_t *np = mpz_limbs_read (pub->n);
  const mp_limb_t *ep = mpz_limbs_read (pub->e);
  int ret;

  mp_size_t itch;

  mp_limb_t *scratch;
  TMP_GMP_DECL(tp, mp_limb_t);

  itch = mpn_sec_powm_itch (nn, ebn, nn);
  TMP_GMP_ALLOC (tp, nn + itch);
  scratch = tp + nn;

  mpn_sec_powm(tp, x, nn, ep, ebn, np, nn, scratch);
  ret = sec_equal(tp, m, nn);

  TMP_GMP_FREE (tp);
  return ret;
}

static void
cnd_mpn_zero (int cnd, volatile mp_ptr rp, mp_size_t n)
{
  volatile mp_limb_t c;
  volatile mp_limb_t mask = (mp_limb_t) cnd - 1;

  while (--n >= 0)
    {
      c = rp[n];
      c &= mask;
      rp[n] = c;
    }
}

/* Checks for any errors done in the RSA computation. That avoids
 * attacks which rely on faults on hardware, or even software MPI
 * implementation.
 * This version is side-channel silent even in case of error,
 * the destination buffer is always overwritten */
int
_rsa_sec_compute_root_tr(const struct rsa_public_key *pub,
			 const struct rsa_private_key *key,
			 void *random_ctx, nettle_random_func *random,
			 mp_limb_t *x, const mp_limb_t *m)
{
  TMP_GMP_DECL (c, mp_limb_t);
  TMP_GMP_DECL (ri, mp_limb_t);
  TMP_GMP_DECL (scratch, mp_limb_t);
  size_t key_limb_size;
  int ret;

  key_limb_size = mpz_size(pub->n);

  /* mpz_powm_sec handles only odd moduli. If p, q or n is even, the
     key is invalid and rejected by rsa_private_key_prepare. However,
     some applications, notably gnutls, don't use this function, and
     we don't want an invalid key to lead to a crash down inside
     mpz_powm_sec. So do an additional check here. */
  if (mpz_even_p (pub->n) || mpz_even_p (key->p) || mpz_even_p (key->q))
    {
      mpn_zero(x, key_limb_size);
      return 0;
    }

  assert(mpz_size(pub->n) == key_limb_size);

  TMP_GMP_ALLOC (c, key_limb_size);
  TMP_GMP_ALLOC (ri, key_limb_size);
  TMP_GMP_ALLOC (scratch, _rsa_sec_compute_root_itch(key));

  rsa_sec_blind (pub, random_ctx, random, c, ri, m);

  _rsa_sec_compute_root(key, x, c, scratch);

  ret = rsa_sec_check_root(pub, x, c);

  rsa_sec_unblind(pub, x, ri, x);

  cnd_mpn_zero(1 - ret, x, key_limb_size);

  TMP_GMP_FREE (scratch);
  TMP_GMP_FREE (ri);
  TMP_GMP_FREE (c);
  return ret;
}

/* Checks for any errors done in the RSA computation. That avoids
 * attacks which rely on faults on hardware, or even software MPI
 * implementation.
 * This version is maintained for API compatibility reasons. It
 * is not completely side-channel silent. There are conditionals
 * in buffer copying both in case of success or error.
 */
int
rsa_compute_root_tr(const struct rsa_public_key *pub,
		    const struct rsa_private_key *key,
		    void *random_ctx, nettle_random_func *random,
		    mpz_t x, const mpz_t m)
{
  TMP_GMP_DECL (l, mp_limb_t);
  mp_size_t nn = mpz_size(pub->n);
  int res;

  TMP_GMP_ALLOC (l, nn);
  mpz_limbs_copy(l, m, nn);

  res = _rsa_sec_compute_root_tr (pub, key, random_ctx, random, l, l);
  if (res) {
    mp_limb_t *xp = mpz_limbs_write (x, nn);
    mpn_copyi (xp, l, nn);
    mpz_limbs_finish (x, nn);
  }

  TMP_GMP_FREE (l);
  return res;
}
#endif
