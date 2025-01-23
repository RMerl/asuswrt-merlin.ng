/* ecc.c  -  Elliptic Curve Cryptography
 * Copyright (C) 2007, 2008, 2010, 2011 Free Software Foundation, Inc.
 * Copyright (C) 2013, 2015 g10 Code GmbH
 *
 * This file is part of Libgcrypt.
 *
 * Libgcrypt is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * Libgcrypt is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

/* This code is originally based on the Patch 0.1.6 for the gnupg
   1.4.x branch as retrieved on 2007-03-21 from
   http://www.calcurco.cat/eccGnuPG/src/gnupg-1.4.6-ecc0.2.0beta1.diff.bz2
   The original authors are:
     Written by
      Sergi Blanch i Torne <d4372211 at alumnes.eup.udl.es>,
      Ramiro Moreno Chiral <ramiro at eup.udl.es>
     Maintainers
      Sergi Blanch i Torne
      Ramiro Moreno Chiral
      Mikael Mylnikov (mmr)
  For use in Libgcrypt the code has been heavily modified and cleaned
  up. In fact there is not much left of the originally code except for
  some variable names and the text book implementaion of the sign and
  verification algorithms.  The arithmetic functions have entirely
  been rewritten and moved to mpi/ec.c.

  ECDH encrypt and decrypt code written by Andrey Jivsov.
*/


/* TODO:

  - In mpi/ec.c we use mpi_powm for x^2 mod p: Either implement a
    special case in mpi_powm or check whether mpi_mulm is faster.

*/


#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "g10lib.h"
#include "mpi.h"
#include "cipher.h"
#include "context.h"
#include "ec-context.h"
#include "pubkey-internal.h"
#include "ecc-common.h"


static const char *ecc_names[] =
  {
    "ecc",
    "ecdsa",
    "ecdh",
    "eddsa",
    "gost",
    "sm2",
    NULL,
  };


/* Sample NIST P-256 key from RFC 6979 A.2.5 */
static const char sample_public_key_secp256[] =
  "(public-key"
  " (ecc"
  "  (curve secp256r1)"
  "  (q #04"
  /**/  "60FED4BA255A9D31C961EB74C6356D68C049B8923B61FA6CE669622E60F29FB6"
  /**/  "7903FE1008B8BC99A41AE9E95628BC64F2F1B20C2D7E9F5177A3C294D4462299#)))";

static const char sample_secret_key_secp256[] =
  "(private-key"
  " (ecc"
  "  (curve secp256r1)"
  "  (d #C9AFA9D845BA75166B5C215767B1D6934E50C3DB36E89B127B8A622B120F6721#)"
  "  (q #04"
  /**/  "60FED4BA255A9D31C961EB74C6356D68C049B8923B61FA6CE669622E60F29FB6"
  /**/  "7903FE1008B8BC99A41AE9E95628BC64F2F1B20C2D7E9F5177A3C294D4462299#)))";


/* Registered progress function and its callback value. */
static void (*progress_cb) (void *, const char*, int, int, int);
static void *progress_cb_data;



/* Local prototypes. */
static void test_keys (mpi_ec_t ec, unsigned int nbits);
static int test_keys_fips (gcry_sexp_t skey);
static void test_ecdh_only_keys (mpi_ec_t ec, unsigned int nbits, int flags);
static unsigned int ecc_get_nbits (gcry_sexp_t parms);




void
_gcry_register_pk_ecc_progress (void (*cb) (void *, const char *,
                                            int, int, int),
                                void *cb_data)
{
  progress_cb = cb;
  progress_cb_data = cb_data;
}

/* static void */
/* progress (int c) */
/* { */
/*   if (progress_cb) */
/*     progress_cb (progress_cb_data, "pk_ecc", c, 0, 0); */
/* } */



/**
 * nist_generate_key - Standard version of the ECC key generation.
 * @ec: Elliptic curve computation context.
 * @flags: Flags controlling aspects of the creation.
 * @r_x: On success this receives an allocated MPI with the affine
 *       x-coordinate of the poblic key.  On error NULL is stored.
 * @r_y: Ditto for the y-coordinate.
 *
 * Return: An error code.
 *
 * The @flags bits used by this function are %PUBKEY_FLAG_TRANSIENT to
 * use a faster RNG, and %PUBKEY_FLAG_NO_KEYTEST to skip the assertion
 * that the key works as expected.
 *
 * FIXME: Check whether N is needed.
 */
static gpg_err_code_t
nist_generate_key (mpi_ec_t ec, int flags,
                   gcry_mpi_t *r_x, gcry_mpi_t *r_y)
{
  mpi_point_struct Q;
  gcry_random_level_t random_level;
  gcry_mpi_t x, y;
  const unsigned int pbits = ec->nbits;

  point_init (&Q);

  if ((flags & PUBKEY_FLAG_TRANSIENT_KEY))
    random_level = GCRY_STRONG_RANDOM;
  else
    random_level = GCRY_VERY_STRONG_RANDOM;

  /* Generate a secret.  */
  if (ec->dialect == ECC_DIALECT_ED25519
      || ec->dialect == ECC_DIALECT_SAFECURVE
      || (flags & PUBKEY_FLAG_DJB_TWEAK))
    {
      char *rndbuf;
      int len = (pbits+7)/8;

      rndbuf = _gcry_random_bytes_secure (len, random_level);
      if (ec->dialect == ECC_DIALECT_SAFECURVE)
        ec->d = mpi_set_opaque (NULL, rndbuf, len*8);
      else
        {
          ec->d = mpi_snew (pbits);
          if ((pbits % 8))
            rndbuf[0] &= (1 << (pbits % 8)) - 1;
          rndbuf[0] |= (1 << ((pbits + 7) % 8));
          rndbuf[len-1] &= (256 - ec->h);
          _gcry_mpi_set_buffer (ec->d, rndbuf, len, 0);
          xfree (rndbuf);
        }
    }
  else
    ec->d = _gcry_dsa_gen_k (ec->n, random_level);

  /* Compute Q.  */
  _gcry_mpi_ec_mul_point (&Q, ec->d, ec->G, ec);

  x = mpi_new (pbits);
  if (r_y == NULL)
    y = NULL;
  else
    y = mpi_new (pbits);
  if (_gcry_mpi_ec_get_affine (x, y, &Q, ec))
    log_fatal ("ecgen: Failed to get affine coordinates for %s\n", "Q");

  /* We want the Q=(x,y) be a "compliant key" in terms of the
   * http://tools.ietf.org/html/draft-jivsov-ecc-compact, which simply
   * means that we choose either Q=(x,y) or -Q=(x,p-y) such that we
   * end up with the min(y,p-y) as the y coordinate.  Such a public
   * key allows the most efficient compression: y can simply be
   * dropped because we know that it's a minimum of the two
   * possibilities without any loss of security.  Note that we don't
   * do that for Ed25519 so that we do not violate the special
   * construction of the secret key.  */
  if (r_y == NULL || ec->dialect == ECC_DIALECT_ED25519)
    ec->Q = mpi_point_set (NULL, Q.x, Q.y, Q.z);
  else
    {
      gcry_mpi_t negative;

      negative = mpi_new (pbits);

      if (ec->model == MPI_EC_WEIERSTRASS)
        mpi_sub (negative, ec->p, y);      /* negative = p - y */
      else
        mpi_sub (negative, ec->p, x);      /* negative = p - x */

      if (mpi_cmp (negative, y) < 0)   /* p - y < p */
        {
          /* We need to end up with -Q; this assures that new Q's y is
             the smallest one */
          if (ec->model == MPI_EC_WEIERSTRASS)
            {
              mpi_free (y);
              y = negative;
            }
          else
            {
              mpi_free (x);
              x = negative;
            }
          mpi_sub (ec->d, ec->n, ec->d);   /* d = order - d */
          ec->Q = mpi_point_set (NULL, x, y, mpi_const (MPI_C_ONE));

          if (DBG_CIPHER)
            log_debug ("ecgen converted Q to a compliant point\n");
        }
      else /* p - y >= p */
        {
          /* No change is needed exactly 50% of the time: just copy. */
          mpi_free (negative);
          ec->Q = mpi_point_set (NULL, Q.x, Q.y, Q.z);
          if (DBG_CIPHER)
            log_debug ("ecgen didn't need to convert Q to a compliant point\n");
        }
    }

  *r_x = x;
  if (r_y)
    *r_y = y;

  point_free (&Q);
  /* Now we can test our keys (this should never fail!).  */
  if ((flags & PUBKEY_FLAG_NO_KEYTEST))
    ; /* User requested to skip the test.  */
  else if (ec->model == MPI_EC_MONTGOMERY)
    test_ecdh_only_keys (ec, ec->nbits - 63, flags);
  else if (!fips_mode ())
    test_keys (ec, ec->nbits - 64);

  return 0;
}


/*
 * To verify correct skey it use a random information.
 * First, encrypt and decrypt this dummy value,
 * test if the information is recuperated.
 * Second, test with the sign and verify functions.
 */
static void
test_keys (mpi_ec_t ec, unsigned int nbits)
{
  gcry_mpi_t test = mpi_new (nbits);
  mpi_point_struct R_;
  gcry_mpi_t c = mpi_new (nbits);
  gcry_mpi_t out = mpi_new (nbits);
  gcry_mpi_t r = mpi_new (nbits);
  gcry_mpi_t s = mpi_new (nbits);

  if (DBG_CIPHER)
    log_debug ("Testing key.\n");

  point_init (&R_);

  _gcry_mpi_randomize (test, nbits, GCRY_WEAK_RANDOM);

  if (_gcry_ecc_ecdsa_sign (test, NULL, ec, r, s, 0, 0) )
    log_fatal ("ECDSA operation: sign failed\n");

  if (_gcry_ecc_ecdsa_verify (test, ec, r, s, 0, 0))
    {
      log_fatal ("ECDSA operation: sign, verify failed\n");
    }

  if (DBG_CIPHER)
    log_debug ("ECDSA operation: sign, verify ok.\n");

  point_free (&R_);
  mpi_free (s);
  mpi_free (r);
  mpi_free (out);
  mpi_free (c);
  mpi_free (test);
}

/* We should get here only with the NIST curves as they are the only ones
 * having the fips bit set in ecc_domain_parms_t struct so this is slightly
 * simpler than the whole ecc_generate function */
static int
test_keys_fips (gcry_sexp_t skey)
{
  int result = -1; /* Default to failure */
  gcry_md_hd_t hd = NULL;
  const char *data_tmpl = "(data (flags rfc6979) (hash %s %b))";
  gcry_sexp_t sig = NULL;
  char plaintext[128];
  int rc;

  /* Create a random plaintext.  */
  _gcry_randomize (plaintext, sizeof plaintext, GCRY_WEAK_RANDOM);

  /* Open MD context and feed the random data in */
  rc = _gcry_md_open (&hd, GCRY_MD_SHA256, 0);
  if (rc)
    {
      log_error ("ECDSA operation: failed to initialize MD context: %s\n", gpg_strerror (rc));
      goto leave;
    }
  _gcry_md_write (hd, plaintext, sizeof(plaintext));

  /* Sign the data */
  rc = _gcry_pk_sign_md (&sig, data_tmpl, hd, skey, NULL);
  if (rc)
    {
      log_error ("ECDSA operation: signing failed: %s\n", gpg_strerror (rc));
      goto leave;
    }

  /* Verify this signature.  */
  rc = _gcry_pk_verify_md (sig, data_tmpl, hd, skey, NULL);
  if (rc)
    {
      log_error ("ECDSA operation: verification failed: %s\n", gpg_strerror (rc));
      goto leave;
    }

  /* Modify the data and check that the signing fails.  */
  _gcry_md_reset(hd);
  plaintext[sizeof plaintext / 2] ^= 1;
  _gcry_md_write (hd, plaintext, sizeof(plaintext));
  rc = _gcry_pk_verify_md (sig, data_tmpl, hd, skey, NULL);
  if (rc != GPG_ERR_BAD_SIGNATURE)
    {
      log_error ("ECDSA operation: signature verification worked on modified data\n");
      goto leave;
    }

  result = 0;
leave:
  _gcry_md_close (hd);
  sexp_release (sig);
  return result;
}


static void
test_ecdh_only_keys (mpi_ec_t ec, unsigned int nbits, int flags)
{
  gcry_mpi_t test;
  mpi_point_struct R_;
  gcry_mpi_t x0, x1;

  if (DBG_CIPHER)
    log_debug ("Testing ECDH only key.\n");

  point_init (&R_);

  if (ec->dialect == ECC_DIALECT_SAFECURVE || (flags & PUBKEY_FLAG_DJB_TWEAK))
    {
      char *rndbuf;
      const unsigned int pbits = ec->nbits;
      int len = (pbits+7)/8;

      rndbuf = _gcry_random_bytes (len, GCRY_WEAK_RANDOM);
      if (ec->dialect == ECC_DIALECT_SAFECURVE)
        test = mpi_set_opaque (NULL, rndbuf, len*8);
      else
        {
          test = mpi_new (pbits);
          if ((pbits % 8))
            rndbuf[0] &= (1 << (pbits % 8)) - 1;
          rndbuf[0] |= (1 << ((pbits + 7) % 8));
          rndbuf[len-1] &= (256 - ec->h);
          _gcry_mpi_set_buffer (test, rndbuf, len, 0);
          xfree (rndbuf);
        }
    }
  else
    {
      test = mpi_new (nbits);
      _gcry_mpi_randomize (test, nbits, GCRY_WEAK_RANDOM);
    }

  x0 = mpi_new (0);
  x1 = mpi_new (0);

  /* R_ = hkQ  <=>  R_ = hkdG  */
  _gcry_mpi_ec_mul_point (&R_, test, ec->Q, ec);
  if (ec->dialect == ECC_DIALECT_STANDARD && !(flags & PUBKEY_FLAG_DJB_TWEAK))
    _gcry_mpi_ec_mul_point (&R_, _gcry_mpi_get_const (ec->h), &R_, ec);
  if (_gcry_mpi_ec_get_affine (x0, NULL, &R_, ec))
    log_fatal ("ecdh: Failed to get affine coordinates for hkQ\n");

  _gcry_mpi_ec_mul_point (&R_, test, ec->G, ec);
  _gcry_mpi_ec_mul_point (&R_, ec->d, &R_, ec);
  /* R_ = hdkG */
  if (ec->dialect == ECC_DIALECT_STANDARD && !(flags & PUBKEY_FLAG_DJB_TWEAK))
    _gcry_mpi_ec_mul_point (&R_, _gcry_mpi_get_const (ec->h), &R_, ec);

  if (_gcry_mpi_ec_get_affine (x1, NULL, &R_, ec))
    log_fatal ("ecdh: Failed to get affine coordinates for hdkG\n");

  if (mpi_cmp (x0, x1))
    {
      log_fatal ("ECDH test failed.\n");
    }

  mpi_free (x0);
  mpi_free (x1);

  point_free (&R_);
  mpi_free (test);
}


/*
 * To check the validity of the value, recalculate the correspondence
 * between the public value and the secret one.
 */
static int
check_secret_key (mpi_ec_t ec, int flags)
{
  int rc = 1;
  mpi_point_struct Q;
  gcry_mpi_t x1, y1;
  gcry_mpi_t x2 = NULL;
  gcry_mpi_t y2 = NULL;

  point_init (&Q);
  x1 = mpi_new (0);
  if (ec->model == MPI_EC_MONTGOMERY)
    y1 = NULL;
  else
    y1 = mpi_new (0);

  /* G in E(F_p) */
  if (!_gcry_mpi_ec_curve_point (ec->G, ec))
    {
      if (DBG_CIPHER)
        log_debug ("Bad check: Point 'G' does not belong to curve 'E'!\n");
      goto leave;
    }

  /* G != PaI */
  if (!mpi_cmp_ui (ec->G->z, 0))
    {
      if (DBG_CIPHER)
        log_debug ("Bad check: 'G' cannot be Point at Infinity!\n");
      goto leave;
    }

  /* Check order of curve.  */
  if (ec->dialect == ECC_DIALECT_STANDARD && !(flags & PUBKEY_FLAG_DJB_TWEAK))
    {
      _gcry_mpi_ec_mul_point (&Q, ec->n, ec->G, ec);
      if (mpi_cmp_ui (Q.z, 0))
        {
          if (DBG_CIPHER)
            log_debug ("check_secret_key: E is not a curve of order n\n");
          goto leave;
        }
    }

  /* Pubkey cannot be PaI */
  if (!mpi_cmp_ui (ec->Q->z, 0))
    {
      if (DBG_CIPHER)
        log_debug ("Bad check: Q can not be a Point at Infinity!\n");
      goto leave;
    }

  /* pubkey = [d]G over E */
  if (!_gcry_ecc_compute_public (&Q, ec))
    {
      if (DBG_CIPHER)
        log_debug ("Bad check: computation of dG failed\n");
      goto leave;
    }
  if (_gcry_mpi_ec_get_affine (x1, y1, &Q, ec))
    {
      if (DBG_CIPHER)
        log_debug ("Bad check: Q can not be a Point at Infinity!\n");
      goto leave;
    }

  if ((flags & PUBKEY_FLAG_EDDSA)
      || (ec->model == MPI_EC_EDWARDS && ec->dialect == ECC_DIALECT_SAFECURVE))
    ; /* Fixme: EdDSA is special.  */
  else if (!mpi_cmp_ui (ec->Q->z, 1))
    {
      /* Fast path if Q is already in affine coordinates.  */
      if (mpi_cmp (x1, ec->Q->x) || (y1 && mpi_cmp (y1, ec->Q->y)))
        {
          if (DBG_CIPHER)
            log_debug
              ("Bad check: There is NO correspondence between 'd' and 'Q'!\n");
          goto leave;
        }
    }
  else
    {
      x2 = mpi_new (0);
      y2 = mpi_new (0);
      if (_gcry_mpi_ec_get_affine (x2, y2, ec->Q, ec))
        {
          if (DBG_CIPHER)
            log_debug ("Bad check: Q can not be a Point at Infinity!\n");
          goto leave;
        }

      if (mpi_cmp (x1, x2) || mpi_cmp (y1, y2))
        {
          if (DBG_CIPHER)
            log_debug
              ("Bad check: There is NO correspondence between 'd' and 'Q'!\n");
          goto leave;
        }
    }
  rc = 0; /* Okay.  */

 leave:
  mpi_free (x2);
  mpi_free (x1);
  mpi_free (y1);
  mpi_free (y2);
  point_free (&Q);
  return rc;
}



/*********************************************
 **************  interface  ******************
 *********************************************/

static gcry_err_code_t
ecc_generate (const gcry_sexp_t genparms, gcry_sexp_t *r_skey)
{
  gpg_err_code_t rc;
  gcry_mpi_t Gx = NULL;
  gcry_mpi_t Gy = NULL;
  gcry_mpi_t Qx = NULL;
  gcry_mpi_t Qy = NULL;
  mpi_ec_t ec = NULL;
  gcry_sexp_t curve_info = NULL;
  gcry_sexp_t curve_flags = NULL;
  gcry_mpi_t base = NULL;
  gcry_mpi_t public = NULL;
  int flags = 0;

  rc = _gcry_mpi_ec_internal_new (&ec, &flags, "ecgen curve", genparms, NULL);
  if (rc)
    goto leave;

  if ((flags & PUBKEY_FLAG_EDDSA)
      || (ec->model == MPI_EC_EDWARDS && ec->dialect == ECC_DIALECT_SAFECURVE))
    rc = _gcry_ecc_eddsa_genkey (ec, flags);
  else if (ec->model == MPI_EC_MONTGOMERY)
    rc = nist_generate_key (ec, flags, &Qx, NULL);
  else
    rc = nist_generate_key (ec, flags, &Qx, &Qy);
  if (rc)
    goto leave;

  /* Copy data to the result.  */
  Gx = mpi_new (0);
  Gy = mpi_new (0);
  if (ec->model != MPI_EC_MONTGOMERY)
    {
      if (_gcry_mpi_ec_get_affine (Gx, Gy, ec->G, ec))
        log_fatal ("ecgen: Failed to get affine coordinates for %s\n", "G");
      base = _gcry_ecc_ec2os (Gx, Gy, ec->p);
    }
  if (((ec->dialect == ECC_DIALECT_SAFECURVE && ec->model == MPI_EC_EDWARDS)
       || ec->dialect == ECC_DIALECT_ED25519 || ec->model == MPI_EC_MONTGOMERY)
      && !(flags & PUBKEY_FLAG_NOCOMP))
    {
      unsigned char *encpk;
      unsigned int encpklen;

      if (ec->model == MPI_EC_MONTGOMERY)
        rc = _gcry_ecc_mont_encodepoint (Qx, ec->nbits,
                                         ec->dialect != ECC_DIALECT_SAFECURVE,
                                         &encpk, &encpklen);
      else
        /* (Gx and Gy are used as scratch variables)  */
        rc = _gcry_ecc_eddsa_encodepoint (ec->Q, ec, Gx, Gy,
                                          (ec->dialect != ECC_DIALECT_SAFECURVE
                                           && !!(flags & PUBKEY_FLAG_COMP)),
                                          &encpk, &encpklen);
      if (rc)
        goto leave;
      public = mpi_new (0);
      mpi_set_opaque (public, encpk, encpklen*8);
    }
  else
    {
      if (!Qx)
        {
          /* This is the case for a key from _gcry_ecc_eddsa_generate
             with no compression.  */
          Qx = mpi_new (0);
          Qy = mpi_new (0);
          if (_gcry_mpi_ec_get_affine (Qx, Qy, ec->Q, ec))
            log_fatal ("ecgen: Failed to get affine coordinates for %s\n", "Q");
        }
      public = _gcry_ecc_ec2os (Qx, Qy, ec->p);
    }
  if (ec->name)
    {
      rc = sexp_build (&curve_info, NULL, "(curve %s)", ec->name);
      if (rc)
        goto leave;
    }

  if ((flags & PUBKEY_FLAG_PARAM) || (flags & PUBKEY_FLAG_EDDSA)
      || (flags & PUBKEY_FLAG_DJB_TWEAK))
    {
      rc = sexp_build
        (&curve_flags, NULL,
         ((flags & PUBKEY_FLAG_PARAM) && (flags & PUBKEY_FLAG_EDDSA))?
         "(flags param eddsa)" :
         ((flags & PUBKEY_FLAG_PARAM) && (flags & PUBKEY_FLAG_DJB_TWEAK))?
         "(flags param djb-tweak)" :
         ((flags & PUBKEY_FLAG_PARAM))?
         "(flags param)" : ((flags & PUBKEY_FLAG_EDDSA))?
         "(flags eddsa)" : "(flags djb-tweak)" );
      if (rc)
        goto leave;
    }

  if ((flags & PUBKEY_FLAG_PARAM) && ec->name)
    rc = sexp_build (r_skey, NULL,
                     "(key-data"
                     " (public-key"
                     "  (ecc%S%S(p%m)(a%m)(b%m)(g%m)(n%m)(h%u)(q%m)))"
                     " (private-key"
                     "  (ecc%S%S(p%m)(a%m)(b%m)(g%m)(n%m)(h%u)(q%m)(d%m)))"
                     " )",
                     curve_info, curve_flags,
                     ec->p, ec->a, ec->b, base, ec->n, ec->h, public,
                     curve_info, curve_flags,
                     ec->p, ec->a, ec->b, base, ec->n, ec->h, public,
                     ec->d);
  else
    rc = sexp_build (r_skey, NULL,
                     "(key-data"
                     " (public-key"
                     "  (ecc%S%S(q%m)))"
                     " (private-key"
                     "  (ecc%S%S(q%m)(d%m)))"
                     " )",
                     curve_info, curve_flags,
                     public,
                     curve_info, curve_flags,
                     public, ec->d);
  if (rc)
    goto leave;

  if (DBG_CIPHER)
    {
      log_printmpi ("ecgen result  p", ec->p);
      log_printmpi ("ecgen result  a", ec->a);
      log_printmpi ("ecgen result  b", ec->b);
      log_printmpi ("ecgen result  G", base);
      log_printmpi ("ecgen result  n", ec->n);
      log_debug    ("ecgen result  h:+%02x\n", ec->h);
      log_printmpi ("ecgen result  Q", public);
      log_printmpi ("ecgen result  d", ec->d);
      if ((flags & PUBKEY_FLAG_EDDSA))
        log_debug ("ecgen result  using Ed25519+EdDSA\n");
    }

  if (fips_mode () && test_keys_fips (*r_skey))
    {
      sexp_release (*r_skey); r_skey = NULL;
      fips_signal_error ("self-test after key generation failed");
      rc = GPG_ERR_SELFTEST_FAILED;
    }

 leave:
  mpi_free (public);
  mpi_free (base);
  mpi_free (Gx);
  mpi_free (Gy);
  mpi_free (Qx);
  mpi_free (Qy);
  _gcry_mpi_ec_free (ec);
  sexp_release (curve_flags);
  sexp_release (curve_info);
  return rc;
}


static gcry_err_code_t
ecc_check_secret_key (gcry_sexp_t keyparms)
{
  gcry_err_code_t rc;
  int flags = 0;
  mpi_ec_t ec = NULL;

  /*
   * Extract the key.
   */
  rc = _gcry_mpi_ec_internal_new (&ec, &flags, "ecc_testkey", keyparms, NULL);
  if (rc)
    goto leave;
  if (!ec->p || !ec->a || !ec->b || !ec->G || !ec->n || !ec->Q || !ec->d)
    {
      rc = GPG_ERR_NO_OBJ;
      goto leave;
    }

  if (check_secret_key (ec, flags))
    rc = GPG_ERR_BAD_SECKEY;

 leave:
  _gcry_mpi_ec_free (ec);
  if (DBG_CIPHER)
    log_debug ("ecc_testkey    => %s\n", gpg_strerror (rc));
  return rc;
}


static gcry_err_code_t
ecc_sign (gcry_sexp_t *r_sig, gcry_sexp_t s_data, gcry_sexp_t keyparms)
{
  gcry_err_code_t rc;
  struct pk_encoding_ctx ctx;
  gcry_mpi_t data = NULL;
  gcry_mpi_t k = NULL;
  gcry_mpi_t sig_r = NULL;
  gcry_mpi_t sig_s = NULL;
  mpi_ec_t ec = NULL;
  int flags = 0;

  _gcry_pk_util_init_encoding_ctx (&ctx, PUBKEY_OP_SIGN, 0);

  /*
   * Extract the key.
   */
  rc = _gcry_mpi_ec_internal_new (&ec, &flags, "ecc_sign", keyparms, NULL);
  if (rc)
    goto leave;
  if (!ec->p || !ec->a || !ec->b || !ec->G || !ec->n || !ec->d)
    {
      rc = GPG_ERR_NO_OBJ;
      goto leave;
    }

  ctx.flags |= flags;
  if (ec->model == MPI_EC_EDWARDS && ec->dialect == ECC_DIALECT_SAFECURVE)
    ctx.flags |= PUBKEY_FLAG_EDDSA;
  /* Clear hash algo for EdDSA.  */
  if ((ctx.flags & PUBKEY_FLAG_EDDSA))
    ctx.hash_algo = GCRY_MD_NONE;

  /* Extract the data.  */
  rc = _gcry_pk_util_data_to_mpi (s_data, &data, &ctx);
  if (rc)
    goto leave;
  if (DBG_CIPHER)
    log_mpidump ("ecc_sign   data", data);

  if (ctx.label)
    rc = _gcry_mpi_scan (&k, GCRYMPI_FMT_USG, ctx.label, ctx.labellen, NULL);
  if (rc)
    goto leave;

  /* Hash algo is determined by curve in EdDSA.  Fill it if not specified.  */
  if ((ctx.flags & PUBKEY_FLAG_EDDSA) && !ctx.hash_algo)
    {
      if (ec->dialect == ECC_DIALECT_ED25519)
        ctx.hash_algo = GCRY_MD_SHA512;
      else if (ec->dialect == ECC_DIALECT_SAFECURVE)
        ctx.hash_algo = GCRY_MD_SHAKE256;
    }

  sig_r = mpi_new (0);
  sig_s = mpi_new (0);
  if ((ctx.flags & PUBKEY_FLAG_EDDSA))
    {
      /* EdDSA requires the public key.  */
      rc = _gcry_ecc_eddsa_sign (data, ec, sig_r, sig_s, &ctx);
      if (!rc)
        rc = sexp_build (r_sig, NULL,
                         "(sig-val(eddsa(r%M)(s%M)))", sig_r, sig_s);
    }
  else if ((ctx.flags & PUBKEY_FLAG_GOST))
    {
      rc = _gcry_ecc_gost_sign (data, ec, sig_r, sig_s);
      if (!rc)
        rc = sexp_build (r_sig, NULL,
                         "(sig-val(gost(r%M)(s%M)))", sig_r, sig_s);
    }
  else if ((ctx.flags & PUBKEY_FLAG_SM2))
    {
      rc = _gcry_ecc_sm2_sign (data, ec, sig_r, sig_s,
                               ctx.flags, ctx.hash_algo);
      if (!rc)
        rc = sexp_build (r_sig, NULL,
                         "(sig-val(sm2(r%M)(s%M)))", sig_r, sig_s);
    }
  else
    {
      rc = _gcry_ecc_ecdsa_sign (data, k, ec, sig_r, sig_s,
                                 ctx.flags, ctx.hash_algo);
      if (!rc)
        rc = sexp_build (r_sig, NULL,
                         "(sig-val(ecdsa(r%M)(s%M)))", sig_r, sig_s);
    }

 leave:
  _gcry_mpi_release (sig_r);
  _gcry_mpi_release (sig_s);
  _gcry_mpi_release (data);
  _gcry_mpi_release (k);
  _gcry_mpi_ec_free (ec);
  _gcry_pk_util_free_encoding_ctx (&ctx);
  if (DBG_CIPHER)
    log_debug ("ecc_sign      => %s\n", gpg_strerror (rc));
  return rc;
}


static gcry_err_code_t
ecc_verify (gcry_sexp_t s_sig, gcry_sexp_t s_data, gcry_sexp_t s_keyparms)
{
  gcry_err_code_t rc;
  struct pk_encoding_ctx ctx;
  gcry_sexp_t l1 = NULL;
  gcry_mpi_t sig_r = NULL;
  gcry_mpi_t sig_s = NULL;
  gcry_mpi_t data = NULL;
  int sigflags;
  mpi_ec_t ec = NULL;
  int flags = 0;

  _gcry_pk_util_init_encoding_ctx (&ctx, PUBKEY_OP_VERIFY,
                                   ecc_get_nbits (s_keyparms));

  /*
   * Extract the key.
   */
  rc = _gcry_mpi_ec_internal_new (&ec, &flags, "ecc_verify",
                                  s_keyparms, NULL);
  if (rc)
    goto leave;
  if (!ec->p || !ec->a || !ec->b || !ec->G || !ec->n || !ec->Q)
    {
      rc = GPG_ERR_NO_OBJ;
      goto leave;
    }

  if (ec->model == MPI_EC_MONTGOMERY)
    {
      if (DBG_CIPHER)
        log_debug ("ecc_verify: Can't use a Montgomery curve\n");
      rc = GPG_ERR_INTERNAL;
      goto leave;
    }

  ctx.flags |= flags;
  if (ec->model == MPI_EC_EDWARDS && ec->dialect == ECC_DIALECT_SAFECURVE)
    ctx.flags |= PUBKEY_FLAG_EDDSA;
  /* Clear hash algo for EdDSA.  */
  if ((ctx.flags & PUBKEY_FLAG_EDDSA))
    ctx.hash_algo = GCRY_MD_NONE;

  /* Extract the data.  */
  rc = _gcry_pk_util_data_to_mpi (s_data, &data, &ctx);
  if (rc)
    goto leave;
  if (DBG_CIPHER)
    log_mpidump ("ecc_verify data", data);

  /* Hash algo is determined by curve in EdDSA.  Fill it if not specified.  */
  if ((ctx.flags & PUBKEY_FLAG_EDDSA) && !ctx.hash_algo)
    {
      if (ec->dialect == ECC_DIALECT_ED25519)
        ctx.hash_algo = GCRY_MD_SHA512;
      else if (ec->dialect == ECC_DIALECT_SAFECURVE)
        ctx.hash_algo = GCRY_MD_SHAKE256;
    }

  /*
   * Extract the signature value.
   */
  rc = _gcry_pk_util_preparse_sigval (s_sig, ecc_names, &l1, &sigflags);
  if (rc)
    goto leave;
  rc = sexp_extract_param (l1, NULL, (sigflags & PUBKEY_FLAG_EDDSA)? "/rs":"rs",
                           &sig_r, &sig_s, NULL);
  if (rc)
    goto leave;
  if (DBG_CIPHER)
    {
      log_mpidump ("ecc_verify  s_r", sig_r);
      log_mpidump ("ecc_verify  s_s", sig_s);
    }
  if ((ctx.flags & PUBKEY_FLAG_EDDSA) ^ (sigflags & PUBKEY_FLAG_EDDSA))
    {
      rc = GPG_ERR_CONFLICT; /* Inconsistent use of flag/algoname.  */
      goto leave;
    }

  /*
   * Verify the signature.
   */
  if ((sigflags & PUBKEY_FLAG_EDDSA))
    {
      rc = _gcry_ecc_eddsa_verify (data, ec, sig_r, sig_s, &ctx);
    }
  else if ((sigflags & PUBKEY_FLAG_GOST))
    {
      rc = _gcry_ecc_gost_verify (data, ec, sig_r, sig_s);
    }
  else if ((sigflags & PUBKEY_FLAG_SM2))
    {
      rc = _gcry_ecc_sm2_verify (data, ec, sig_r, sig_s);
    }
  else
    {
      rc = _gcry_ecc_ecdsa_verify (data, ec, sig_r, sig_s,
                                   ctx.flags, ctx.hash_algo);
    }

 leave:
  _gcry_mpi_release (data);
  _gcry_mpi_release (sig_r);
  _gcry_mpi_release (sig_s);
  _gcry_mpi_ec_free (ec);
  sexp_release (l1);
  _gcry_pk_util_free_encoding_ctx (&ctx);
  if (DBG_CIPHER)
    log_debug ("ecc_verify    => %s\n", rc?gpg_strerror (rc):"Good");
  return rc;
}


/* ecdh raw is classic 2-round DH protocol published in 1976.
 *
 * Overview of ecc_encrypt_raw and ecc_decrypt_raw.
 *
 * As with any PK operation, encrypt version uses a public key and
 * decrypt -- private.
 *
 * Symbols used below:
 *     G - field generator point
 *     d - private long-term scalar
 *    dG - public long-term key
 *     k - ephemeral scalar
 *    kG - ephemeral public key
 *   dkG - shared secret
 *
 * ecc_encrypt_raw description:
 *   input:
 *     data[0] : private scalar (k)
 *   output: A new S-expression with the parameters:
 *     s : shared point (kdG)
 *     e : generated ephemeral public key (kG)
 *
 * ecc_decrypt_raw description:
 *   input:
 *     data[0] : a point kG (ephemeral public key)
 *   output:
 *     result[0] : shared point (kdG)
 */
static gcry_err_code_t
ecc_encrypt_raw (gcry_sexp_t *r_ciph, gcry_sexp_t s_data, gcry_sexp_t keyparms)
{
  unsigned int nbits;
  gcry_err_code_t rc;
  struct pk_encoding_ctx ctx;
  gcry_mpi_t mpi_s = NULL;
  gcry_mpi_t mpi_e = NULL;
  gcry_mpi_t data = NULL;
  mpi_ec_t ec = NULL;
  int flags = 0;
  int no_error_on_infinity;

  _gcry_pk_util_init_encoding_ctx (&ctx, PUBKEY_OP_ENCRYPT,
                                   (nbits = ecc_get_nbits (keyparms)));

  /*
   * Extract the key.
   */
  rc = _gcry_mpi_ec_internal_new (&ec, &flags, "ecc_encrypt", keyparms, NULL);
  if (rc)
    goto leave;

  if (ec->dialect == ECC_DIALECT_SAFECURVE)
    {
      ctx.flags |= PUBKEY_FLAG_RAW_FLAG;
      no_error_on_infinity = 1;
    }
  else if ((flags & PUBKEY_FLAG_DJB_TWEAK))
    no_error_on_infinity = 1;
  else
    no_error_on_infinity = 0;

  /*
   * Extract the data.
   */
  rc = _gcry_pk_util_data_to_mpi (s_data, &data, &ctx);
  if (rc)
    goto leave;

  /*
   * Tweak the scalar bits by cofactor and number of bits of the field.
   * It assumes the cofactor is a power of 2.
   */
  if ((flags & PUBKEY_FLAG_DJB_TWEAK))
    {
      int i;

      for (i = 0; (ec->h & (1 << i)) == 0; i++)
        mpi_clear_bit (data, i);
      mpi_set_highbit (data, ec->nbits - 1);
    }
  if (DBG_CIPHER)
    log_mpidump ("ecc_encrypt data", data);

  if (!ec->p || !ec->a || !ec->b || !ec->G || !ec->n || !ec->Q)
    {
      rc = GPG_ERR_NO_OBJ;
      goto leave;
    }

  if ((ctx.flags & PUBKEY_FLAG_SM2))
    {
      /* All encryption will be done, return it.  */
      rc = _gcry_ecc_sm2_encrypt (r_ciph, data, ec);
      goto leave;
    }

  /* The following is false: assert( mpi_cmp_ui( R.x, 1 )==0 );, so */
  {
    mpi_point_struct R;  /* Result that we return.  */
    gcry_mpi_t x, y;
    unsigned char *rawmpi;
    unsigned int rawmpilen;

    rc = 0;
    x = mpi_new (0);
    if (ec->model == MPI_EC_MONTGOMERY)
      y = NULL;
    else
      y = mpi_new (0);

    point_init (&R);

    /* R = kQ  <=>  R = kdG  */
    _gcry_mpi_ec_mul_point (&R, data, ec->Q, ec);

    if (_gcry_mpi_ec_get_affine (x, y, &R, ec))
      {
        /*
         * Here, X is 0.  In the X25519 computation on Curve25519, X0
         * function maps infinity to zero.  So, when PUBKEY_FLAG_DJB_TWEAK
         * is enabled, return the result of 0 not raising an error.
         *
         * This is a corner case.  It never occurs with properly
         * generated public keys, but it might happen with blindly
         * imported public key which might not follow the key
         * generation procedure.
         */
        if (!no_error_on_infinity)
          { /* It's not for X25519, then, the input data was simply wrong.  */
            rc = GPG_ERR_INV_DATA;
            goto leave_main;
          }
      }
    if (y)
      mpi_s = _gcry_ecc_ec2os (x, y, ec->p);
    else
      {
        rc = _gcry_ecc_mont_encodepoint (x, nbits,
                                         ec->dialect != ECC_DIALECT_SAFECURVE,
                                         &rawmpi, &rawmpilen);
        if (rc)
          goto leave_main;
        mpi_s = mpi_new (0);
        mpi_set_opaque (mpi_s, rawmpi, rawmpilen*8);
      }

    /* R = kG */
    _gcry_mpi_ec_mul_point (&R, data, ec->G, ec);

    if (_gcry_mpi_ec_get_affine (x, y, &R, ec))
      {
        rc = GPG_ERR_INV_DATA;
        goto leave_main;
      }
    if (y)
      mpi_e = _gcry_ecc_ec2os (x, y, ec->p);
    else
      {
        rc = _gcry_ecc_mont_encodepoint (x, nbits,
                                         ec->dialect != ECC_DIALECT_SAFECURVE,
                                         &rawmpi, &rawmpilen);
        if (!rc)
          {
            mpi_e = mpi_new (0);
            mpi_set_opaque (mpi_e, rawmpi, rawmpilen*8);
          }
      }

  leave_main:
    mpi_free (x);
    mpi_free (y);
    point_free (&R);
    if (rc)
      goto leave;
  }

  if (!rc)
    rc = sexp_build (r_ciph, NULL, "(enc-val(ecdh(s%m)(e%m)))", mpi_s, mpi_e);

 leave:
  _gcry_mpi_release (data);
  _gcry_mpi_release (mpi_s);
  _gcry_mpi_release (mpi_e);
  _gcry_mpi_ec_free (ec);
  _gcry_pk_util_free_encoding_ctx (&ctx);
  if (DBG_CIPHER)
    log_debug ("ecc_encrypt    => %s\n", gpg_strerror (rc));
  return rc;
}


/*  input:
 *     data[0] : a point kG (ephemeral public key)
 *   output:
 *     resaddr[0] : shared point kdG
 *
 *  see ecc_encrypt_raw for details.
 */
static gcry_err_code_t
ecc_decrypt_raw (gcry_sexp_t *r_plain, gcry_sexp_t s_data, gcry_sexp_t keyparms)
{
  unsigned int nbits;
  gpg_err_code_t rc;
  struct pk_encoding_ctx ctx;
  gcry_sexp_t l1 = NULL;
  gcry_mpi_t data_e = NULL;
  mpi_ec_t ec = NULL;
  mpi_point_struct kG;
  mpi_point_struct R;
  gcry_mpi_t r = NULL;
  int flags = 0;
  int enable_specific_point_validation;

  point_init (&kG);
  point_init (&R);

  _gcry_pk_util_init_encoding_ctx (&ctx, PUBKEY_OP_DECRYPT,
                                   (nbits = ecc_get_nbits (keyparms)));

  /*
   * Extract the key.
   */
  rc = _gcry_mpi_ec_internal_new (&ec, &flags, "ecc_decrypt", keyparms, NULL);
  if (rc)
    goto leave;

  if (!ec->p || !ec->a || !ec->b || !ec->G || !ec->n || !ec->d)
    {
      rc = GPG_ERR_NO_OBJ;
      goto leave;
    }

  /*
   * Extract the data.
   */
  rc = _gcry_pk_util_preparse_encval (s_data, ecc_names, &l1, &ctx);
  if (rc)
    goto leave;
  if ((ctx.flags & PUBKEY_FLAG_SM2))
    {
      /* All decryption will be done, return it.  */
      rc = _gcry_ecc_sm2_decrypt (r_plain, l1, ec);
      goto leave;
    }
  else
    {
      rc = sexp_extract_param (l1, NULL, "/e", &data_e, NULL);
      if (rc)
        goto leave;
      if (DBG_CIPHER)
        log_printmpi ("ecc_decrypt  d_e", data_e);
    }

  if (ec->dialect == ECC_DIALECT_SAFECURVE || (flags & PUBKEY_FLAG_DJB_TWEAK))
    enable_specific_point_validation = 1;
  else
    enable_specific_point_validation = 0;

  /*
   * Compute the plaintext.
   */
  if (ec->model == MPI_EC_MONTGOMERY)
    rc = _gcry_ecc_mont_decodepoint (data_e, ec, &kG);
  else
    rc = _gcry_ecc_sec_decodepoint (data_e, ec, &kG);
  if (rc)
    goto leave;

  if (DBG_CIPHER)
    log_printpnt ("ecc_decrypt    kG", &kG, NULL);

  if (enable_specific_point_validation)
    {
      /* For X25519, by its definition, validation should not be done.  */
      /* (Instead, we do output check.)
       *
       * However, to mitigate secret key leak from our implementation,
       * we also do input validation here.  For constant-time
       * implementation, we can remove this input validation.
       */
      if (_gcry_mpi_ec_bad_point (&kG, ec))
        {
          rc = GPG_ERR_INV_DATA;
          goto leave;
        }
    }
  else if (!_gcry_mpi_ec_curve_point (&kG, ec))
    {
      rc = GPG_ERR_INV_DATA;
      goto leave;
    }

  /* R = dkG */
  _gcry_mpi_ec_mul_point (&R, ec->d, &kG, ec);

  /* The following is false: assert( mpi_cmp_ui( R.x, 1 )==0 );, so:  */
  {
    gcry_mpi_t x, y;

    x = mpi_new (0);
    if (ec->model == MPI_EC_MONTGOMERY)
      y = NULL;
    else
      y = mpi_new (0);

    if (_gcry_mpi_ec_get_affine (x, y, &R, ec))
      {
        rc = GPG_ERR_INV_DATA;
        goto leave;
        /*
         * Note for X25519.
         *
         * By the definition of X25519, this is the case where X25519
         * returns 0, mapping infinity to zero.  However, we
         * deliberately let it return an error.
         *
         * For X25519 ECDH, comming here means that it might be
         * decrypted by anyone with the shared secret of 0 (the result
         * of this function could be always 0 by other scalar values,
         * other than the private key of D).
         *
         * So, it looks like an encrypted message but it can be
         * decrypted by anyone, or at least something wrong
         * happens.  Recipient should not proceed as if it were
         * properly encrypted message.
         *
         * This handling is needed for our major usage of GnuPG,
         * where it does the One-Pass Diffie-Hellman method,
         * C(1, 1, ECC CDH), with an ephemeral key.
         */
      }

    if (y)
      r = _gcry_ecc_ec2os (x, y, ec->p);
    else
      {

        unsigned char *rawmpi;
        unsigned int rawmpilen;

        rc = _gcry_ecc_mont_encodepoint (x, nbits,
                                         ec->dialect != ECC_DIALECT_SAFECURVE,
                                         &rawmpi, &rawmpilen);
        if (rc)
          goto leave;

        r = mpi_new (0);
        mpi_set_opaque (r, rawmpi, rawmpilen*8);
      }
    if (!r)
      rc = gpg_err_code_from_syserror ();
    else
      rc = 0;
    mpi_free (x);
    mpi_free (y);
  }
  if (DBG_CIPHER)
    log_printmpi ("ecc_decrypt  res", r);

  if (!rc)
    rc = sexp_build (r_plain, NULL, "(value %m)", r);

 leave:
  point_free (&R);
  point_free (&kG);
  _gcry_mpi_release (r);
  _gcry_mpi_release (data_e);
  sexp_release (l1);
  _gcry_mpi_ec_free (ec);
  _gcry_pk_util_free_encoding_ctx (&ctx);
  if (DBG_CIPHER)
    log_debug ("ecc_decrypt    => %s\n", gpg_strerror (rc));
  return rc;
}


/* Return the number of bits for the key described by PARMS.  On error
 * 0 is returned.  The format of PARMS starts with the algorithm name;
 * for example:
 *
 *   (ecc
 *     (curve <name>)
 *     (p <mpi>)
 *     (a <mpi>)
 *     (b <mpi>)
 *     (g <mpi>)
 *     (n <mpi>)
 *     (q <mpi>))
 *
 * More parameters may be given. Either P or CURVE is needed.
 */
static unsigned int
ecc_get_nbits (gcry_sexp_t parms)
{
  gcry_sexp_t l1;
  gcry_mpi_t p;
  unsigned int nbits = 0;
  char *curve;

  l1 = sexp_find_token (parms, "p", 1);
  if (!l1)
    { /* Parameter P not found - check whether we have "curve".  */
      l1 = sexp_find_token (parms, "curve", 5);
      if (!l1)
        return 0; /* Neither P nor CURVE found.  */

      curve = sexp_nth_string (l1, 1);
      sexp_release (l1);
      if (!curve)
        return 0;  /* No curve name given (or out of core). */

      if (_gcry_ecc_fill_in_curve (0, curve, NULL, &nbits))
        nbits = 0;
      xfree (curve);
    }
  else
    {
      p = sexp_nth_mpi (l1, 1, GCRYMPI_FMT_USG);
      sexp_release (l1);
      if (p)
        {
          nbits = mpi_get_nbits (p);
          _gcry_mpi_release (p);
        }
    }
  return nbits;
}


/* See rsa.c for a description of this function.  */
static gpg_err_code_t
compute_keygrip (gcry_md_hd_t md, gcry_sexp_t keyparms)
{
#define N_COMPONENTS 6
  static const char names[N_COMPONENTS] = "pabgnq";
  gpg_err_code_t rc;
  gcry_sexp_t l1;
  gcry_mpi_t values[N_COMPONENTS];
  int idx;
  char *curvename = NULL;
  int flags = 0;
  enum gcry_mpi_ec_models model = 0;
  enum ecc_dialects dialect = 0;
  const unsigned char *raw;
  unsigned int n;
  int maybe_uncompress;

  /* Clear the values first.  */
  for (idx=0; idx < N_COMPONENTS; idx++)
    values[idx] = NULL;


  /* Look for flags. */
  l1 = sexp_find_token (keyparms, "flags", 0);
  if (l1)
    {
      rc = _gcry_pk_util_parse_flaglist (l1, &flags, NULL);
      if (rc)
        goto leave;
    }

  /* Extract the parameters.  */
  if ((flags & PUBKEY_FLAG_PARAM))
    rc = sexp_extract_param (keyparms, NULL, "p?a?b?g?n?/q",
                             &values[0], &values[1], &values[2],
                             &values[3], &values[4], &values[5],
                             NULL);
  else
    rc = sexp_extract_param (keyparms, NULL, "/q", &values[5], NULL);
  if (rc)
    goto leave;

  /* Check whether a curve parameter is available and use that to fill
     in missing values.  */
  sexp_release (l1);
  l1 = sexp_find_token (keyparms, "curve", 5);
  if (l1)
    {
      curvename = sexp_nth_string (l1, 1);
      if (curvename)
        {
          rc = _gcry_ecc_update_curve_param (curvename,
                                             &model, &dialect,
                                             &values[0], &values[1], &values[2],
                                             &values[3], &values[4]);
          if (rc)
            goto leave;
        }
    }

  /* Guess required fields if a curve parameter has not been given.
     FIXME: This is a crude hacks.  We need to fix that.  */
  if (!curvename)
    {
      model = ((flags & PUBKEY_FLAG_EDDSA)
               ? MPI_EC_EDWARDS
               : MPI_EC_WEIERSTRASS);
      dialect = ((flags & PUBKEY_FLAG_EDDSA)
                 ? ECC_DIALECT_ED25519
                 : ECC_DIALECT_STANDARD);
    }

  /* Check that all parameters are known and normalize all MPIs (that
     should not be required but we use an internal function later and
     thus we better make 100% sure that they are normalized). */
  for (idx = 0; idx < N_COMPONENTS; idx++)
    if (!values[idx])
      {
        rc = GPG_ERR_NO_OBJ;
        goto leave;
      }
    else
      _gcry_mpi_normalize (values[idx]);

  /* Uncompress the public key with the exception of EdDSA where
     compression is the default and we thus compute the keygrip using
     the compressed version.  Because we don't support any non-eddsa
     compression, the only thing we need to do is to compress
     EdDSA.  */
  if ((flags & PUBKEY_FLAG_EDDSA) && dialect == ECC_DIALECT_ED25519)
    {
      const unsigned int pbits = mpi_get_nbits (values[0]);

      rc = _gcry_ecc_eddsa_ensure_compact (values[5], pbits);
      if (rc)
        goto leave;
      maybe_uncompress = 0;
    }
  else if ((flags & PUBKEY_FLAG_DJB_TWEAK))
    {
      /* Remove the prefix 0x40 for keygrip computation.  */
      raw = mpi_get_opaque (values[5], &n);
      if (raw)
        {
          n = (n + 7)/8;

          if (n > 1 && (n%2) && raw[0] == 0x40)
            if (!_gcry_mpi_set_opaque_copy (values[5], raw + 1, (n - 1)*8))
                rc = gpg_err_code_from_syserror ();
        }
      else
        {
          rc = GPG_ERR_INV_OBJ;
          goto leave;
        }
      maybe_uncompress = 0;
    }
  else
    maybe_uncompress = 1;

  /* Hash them all.  */
  for (idx = 0; idx < N_COMPONENTS; idx++)
    {
      char buf[30];
      unsigned char *rawbuffer;
      unsigned int rawlen;

      if (mpi_is_opaque (values[idx]))
        {
          rawbuffer = NULL;
          raw = mpi_get_opaque (values[idx], &rawlen);
          rawlen = (rawlen + 7)/8;
        }
      else
        {
          rawbuffer = _gcry_mpi_get_buffer (values[idx], 0, &rawlen, NULL);
          if (!rawbuffer)
            {
              rc = gpg_err_code_from_syserror ();
              goto leave;
            }
          raw = rawbuffer;
        }

      if (maybe_uncompress && idx == 5 && rawlen > 1
          && (*raw == 0x02 || *raw == 0x03))
        {
          /* This is a compressed Q - uncompress.  */
          mpi_ec_t ec = NULL;
          gcry_mpi_t x, y;
          gcry_mpi_t x3;
          gcry_mpi_t t;
          gcry_mpi_t p1_4;
          int y_bit = (*raw == 0x03);

          /* We need to get the curve parameters as MPIs so that we
           * can do computations.  We have them in VALUES but it is
           * possible that the caller provided them as opaque MPIs. */
          rc = _gcry_mpi_ec_internal_new (&ec, &flags, "ecc_keygrip",
                                          keyparms, NULL);
          if (rc)
            goto leave;
          if (!ec->p || !ec->a || !ec->b || !ec->G || !ec->n)
            {
              rc = GPG_ERR_NO_OBJ;
              _gcry_mpi_ec_free (ec);
              goto leave;
            }

          if (!mpi_test_bit (ec->p, 1))
            {
              /* No support for point compression for this curve.  */
              rc = GPG_ERR_NOT_IMPLEMENTED;
              _gcry_mpi_ec_free (ec);
              xfree (rawbuffer);
              goto leave;
            }

          raw++;
          rawlen--;
          rc = _gcry_mpi_scan (&x, GCRYMPI_FMT_USG, raw, rawlen, NULL);
          if (rc)
            {
              _gcry_mpi_ec_free (ec);
              xfree (rawbuffer);
              goto leave;
            }

          /*
           * Recover Y.  The Weierstrass curve: y^2 = x^3 + a*x + b
           */

          x3 = mpi_new (0);
          t = mpi_new (0);
          p1_4 = mpi_new (0);
          y = mpi_new (0);

          /* Compute right hand side.  */
          mpi_powm (x3, x, mpi_const (MPI_C_THREE), ec->p);
          mpi_mul (t, ec->a, x);
          mpi_mod (t, t, ec->p);
          mpi_add (t, t, ec->b);
          mpi_mod (t, t, ec->p);
          mpi_add (t, t, x3);
          mpi_mod (t, t, ec->p);

          /*
           * When p mod 4 = 3, modular square root of A can be computed by
           * A^((p+1)/4) mod p
           */

          /* Compute (p+1)/4 into p1_4 */
          mpi_rshift (p1_4, ec->p, 2);
          _gcry_mpi_add_ui (p1_4, p1_4, 1);

          mpi_powm (y, t, p1_4, ec->p);

          if (y_bit != mpi_test_bit (y, 0))
            mpi_sub (y, ec->p, y);

          mpi_free (p1_4);
          mpi_free (t);
          mpi_free (x3);

          xfree (rawbuffer);
          rawbuffer = _gcry_ecc_ec2os_buf (x, y, ec->p, &rawlen);
          raw = rawbuffer;

          mpi_free (x);
          mpi_free (y);
          _gcry_mpi_ec_free (ec);
        }

      snprintf (buf, sizeof buf, "(1:%c%u:", names[idx], rawlen);
      _gcry_md_write (md, buf, strlen (buf));
      _gcry_md_write (md, raw, rawlen);
      _gcry_md_write (md, ")", 1);
      xfree (rawbuffer);
    }

 leave:
  xfree (curvename);
  sexp_release (l1);
  for (idx = 0; idx < N_COMPONENTS; idx++)
    _gcry_mpi_release (values[idx]);

  return rc;
#undef N_COMPONENTS
}



/*
   Low-level API helper functions.
 */

/* This is the worker function for gcry_pubkey_get_sexp for ECC
   algorithms.  Note that the caller has already stored NULL at
   R_SEXP.  */
gpg_err_code_t
_gcry_pk_ecc_get_sexp (gcry_sexp_t *r_sexp, int mode, mpi_ec_t ec)
{
  gpg_err_code_t rc;
  gcry_mpi_t mpi_G = NULL;
  gcry_mpi_t mpi_Q = NULL;

  if (!ec->p || !ec->a || !ec->b || !ec->G || !ec->n)
    return GPG_ERR_BAD_CRYPT_CTX;

  if (mode == GCRY_PK_GET_SECKEY && !ec->d)
    return GPG_ERR_NO_SECKEY;

  /* Compute the public point if it is missing.  */
  if (!ec->Q && ec->d)
    ec->Q = _gcry_ecc_compute_public (NULL, ec);

  /* Encode G and Q.  */
  mpi_G = _gcry_mpi_ec_ec2os (ec->G, ec);
  if (!mpi_G)
    {
      rc = GPG_ERR_BROKEN_PUBKEY;
      goto leave;
    }
  if (!ec->Q)
    {
      rc = GPG_ERR_BAD_CRYPT_CTX;
      goto leave;
    }

  if (ec->dialect == ECC_DIALECT_ED25519)
    {
      unsigned char *encpk;
      unsigned int encpklen;

      rc = _gcry_ecc_eddsa_encodepoint (ec->Q, ec, NULL, NULL, 0,
                                        &encpk, &encpklen);
      if (rc)
        goto leave;
      mpi_Q = mpi_set_opaque (NULL, encpk, encpklen*8);
      encpk = NULL;
    }
  else if (ec->model == MPI_EC_MONTGOMERY)
    {
      unsigned char *encpk;
      unsigned int encpklen;

      rc = _gcry_ecc_mont_encodepoint (ec->Q->x, ec->nbits,
                                       ec->dialect != ECC_DIALECT_SAFECURVE,
                                       &encpk, &encpklen);
      if (rc)
        goto leave;
      mpi_Q = mpi_set_opaque (NULL, encpk, encpklen*8);
    }
  else
    {
      mpi_Q = _gcry_mpi_ec_ec2os (ec->Q, ec);
    }
  if (!mpi_Q)
    {
      rc = GPG_ERR_BROKEN_PUBKEY;
      goto leave;
    }

  /* Fixme: We should return a curve name instead of the parameters if
     if know that they match a curve.  */

  if (ec->d && (!mode || mode == GCRY_PK_GET_SECKEY))
    {
      /* Let's return a private key. */
      rc = sexp_build (r_sexp, NULL,
                       "(private-key(ecc(p%m)(a%m)(b%m)(g%m)(n%m)(h%u)(q%m)(d%m)))",
                       ec->p, ec->a, ec->b, mpi_G, ec->n, ec->h, mpi_Q, ec->d);
    }
  else if (ec->Q)
    {
      /* Let's return a public key.  */
      rc = sexp_build (r_sexp, NULL,
                       "(public-key(ecc(p%m)(a%m)(b%m)(g%m)(n%m)(h%u)(q%m)))",
                       ec->p, ec->a, ec->b, mpi_G, ec->n, ec->h, mpi_Q);
    }
  else
    rc = GPG_ERR_BAD_CRYPT_CTX;

 leave:
  mpi_free (mpi_Q);
  mpi_free (mpi_G);
  return rc;
}



/*
     Self-test section.
 */

static const char *
selftest_hash_sign (gcry_sexp_t pkey, gcry_sexp_t skey)
{
  int md_algo = GCRY_MD_SHA256;
  gcry_md_hd_t hd = NULL;
  const char *data_tmpl = "(data (flags rfc6979) (hash %s %b))";
  /* Sample data from RFC 6979 section A.2.5, hash is of message "sample" */
  static const char sample_data[] = "sample";
  static const char sample_data_bad[] = "sbmple";
  static const char signature_r[] =
    "efd48b2aacb6a8fd1140dd9cd45e81d69d2c877b56aaf991c34d0ea84eaf3716";
  static const char signature_s[] =
    "f7cb1c942d657c41d436c7a1b6e29f65f3e900dbb9aff4064dc4ab2f843acda8";

  const char *errtxt = NULL;
  gcry_error_t err;
  gcry_sexp_t sig = NULL;
  gcry_sexp_t l1 = NULL;
  gcry_sexp_t l2 = NULL;
  gcry_mpi_t r = NULL;
  gcry_mpi_t s = NULL;
  gcry_mpi_t calculated_r = NULL;
  gcry_mpi_t calculated_s = NULL;
  int cmp;

  err = _gcry_md_open (&hd, md_algo, 0);
  if (err)
    {
      errtxt = "gcry_md_open failed";
      goto leave;
    }

  _gcry_md_write (hd, sample_data, strlen(sample_data));

  err = _gcry_mpi_scan (&r, GCRYMPI_FMT_HEX, signature_r, 0, NULL);
  if (!err)
    err = _gcry_mpi_scan (&s, GCRYMPI_FMT_HEX, signature_s, 0, NULL);

  if (err)
    {
      errtxt = "converting data failed";
      goto leave;
    }

  err = _gcry_pk_sign_md (&sig, data_tmpl, hd, skey, NULL);
  if (err)
    {
      errtxt = "signing failed";
      goto leave;
    }

  /* check against known signature */
  errtxt = "signature validity failed";
  l1 = _gcry_sexp_find_token (sig, "sig-val", 0);
  if (!l1)
    goto leave;
  l2 = _gcry_sexp_find_token (l1, "ecdsa", 0);
  if (!l2)
    goto leave;

  sexp_release (l1);
  l1 = l2;

  l2 = _gcry_sexp_find_token (l1, "r", 0);
  if (!l2)
    goto leave;
  calculated_r = _gcry_sexp_nth_mpi (l2, 1, GCRYMPI_FMT_USG);
  if (!calculated_r)
    goto leave;

  sexp_release (l2);
  l2 = _gcry_sexp_find_token (l1, "s", 0);
  if (!l2)
    goto leave;
  calculated_s = _gcry_sexp_nth_mpi (l2, 1, GCRYMPI_FMT_USG);
  if (!calculated_s)
    goto leave;

  errtxt = "known sig check failed";

  cmp = _gcry_mpi_cmp (r, calculated_r);
  if (cmp)
    goto leave;
  cmp = _gcry_mpi_cmp (s, calculated_s);
  if (cmp)
    goto leave;

  errtxt = NULL;

  /* verify generated signature */
  err = _gcry_pk_verify_md (sig, data_tmpl, hd, pkey, NULL);
  if (err)
    {
      errtxt = "verify failed";
      goto leave;
    }

  _gcry_md_reset(hd);
  _gcry_md_write (hd, sample_data_bad, strlen(sample_data_bad));
  err = _gcry_pk_verify_md (sig, data_tmpl, hd, pkey, NULL);
  if (gcry_err_code (err) != GPG_ERR_BAD_SIGNATURE)
    {
      errtxt = "bad signature not detected";
      goto leave;
    }


 leave:
  _gcry_md_close (hd);
  sexp_release (sig);
  sexp_release (l1);
  sexp_release (l2);
  mpi_release (r);
  mpi_release (s);
  mpi_release (calculated_r);
  mpi_release (calculated_s);
  return errtxt;
}


static const char *
selftest_sign (gcry_sexp_t pkey, gcry_sexp_t skey)
{
  /* Sample data from RFC 6979 section A.2.5, hash is of message "sample" */
  static const char sample_data[] =
    "(data (flags rfc6979 prehash)"
    " (hash-algo sha256)"
    " (value 6:sample))";
  static const char sample_data_bad[] =
    "(data (flags rfc6979)"
    " (hash sha256 #bf2bdbe1aa9b6ec1e2ade1d694f41fc71a831d0268e98915"
    /**/           "62113d8a62add1bf#))";
  static const char signature_r[] =
    "efd48b2aacb6a8fd1140dd9cd45e81d69d2c877b56aaf991c34d0ea84eaf3716";
  static const char signature_s[] =
    "f7cb1c942d657c41d436c7a1b6e29f65f3e900dbb9aff4064dc4ab2f843acda8";

  const char *errtxt = NULL;
  gcry_error_t err;
  gcry_sexp_t data = NULL;
  gcry_sexp_t data_bad = NULL;
  gcry_sexp_t sig = NULL;
  gcry_sexp_t l1 = NULL;
  gcry_sexp_t l2 = NULL;
  gcry_mpi_t r = NULL;
  gcry_mpi_t s = NULL;
  gcry_mpi_t calculated_r = NULL;
  gcry_mpi_t calculated_s = NULL;
  int cmp;

  err = sexp_sscan (&data, NULL, sample_data, strlen (sample_data));
  if (!err)
    err = sexp_sscan (&data_bad, NULL,
                      sample_data_bad, strlen (sample_data_bad));
  if (!err)
    err = _gcry_mpi_scan (&r, GCRYMPI_FMT_HEX, signature_r, 0, NULL);
  if (!err)
    err = _gcry_mpi_scan (&s, GCRYMPI_FMT_HEX, signature_s, 0, NULL);

  if (err)
    {
      errtxt = "converting data failed";
      goto leave;
    }

  err = _gcry_pk_sign (&sig, data, skey);
  if (err)
    {
      errtxt = "signing failed";
      goto leave;
    }

  /* check against known signature */
  errtxt = "signature validity failed";
  l1 = _gcry_sexp_find_token (sig, "sig-val", 0);
  if (!l1)
    goto leave;
  l2 = _gcry_sexp_find_token (l1, "ecdsa", 0);
  if (!l2)
    goto leave;

  sexp_release (l1);
  l1 = l2;

  l2 = _gcry_sexp_find_token (l1, "r", 0);
  if (!l2)
    goto leave;
  calculated_r = _gcry_sexp_nth_mpi (l2, 1, GCRYMPI_FMT_USG);
  if (!calculated_r)
    goto leave;

  sexp_release (l2);
  l2 = _gcry_sexp_find_token (l1, "s", 0);
  if (!l2)
    goto leave;
  calculated_s = _gcry_sexp_nth_mpi (l2, 1, GCRYMPI_FMT_USG);
  if (!calculated_s)
    goto leave;

  errtxt = "known sig check failed";

  cmp = _gcry_mpi_cmp (r, calculated_r);
  if (cmp)
    goto leave;
  cmp = _gcry_mpi_cmp (s, calculated_s);
  if (cmp)
    goto leave;

  errtxt = NULL;

  /* verify generated signature */
  err = _gcry_pk_verify (sig, data, pkey);
  if (err)
    {
      errtxt = "verify failed";
      goto leave;
    }
  err = _gcry_pk_verify (sig, data_bad, pkey);
  if (gcry_err_code (err) != GPG_ERR_BAD_SIGNATURE)
    {
      errtxt = "bad signature not detected";
      goto leave;
    }


 leave:
  sexp_release (sig);
  sexp_release (data_bad);
  sexp_release (data);
  sexp_release (l1);
  sexp_release (l2);
  mpi_release (r);
  mpi_release (s);
  mpi_release (calculated_r);
  mpi_release (calculated_s);
  return errtxt;
}


static gpg_err_code_t
selftests_ecdsa (selftest_report_func_t report, int extended)
{
  const char *what;
  const char *errtxt;
  gcry_error_t err;
  gcry_sexp_t skey = NULL;
  gcry_sexp_t pkey = NULL;

  what = "convert";
  err = sexp_sscan (&skey, NULL, sample_secret_key_secp256,
                    strlen (sample_secret_key_secp256));
  if (!err)
    err = sexp_sscan (&pkey, NULL, sample_public_key_secp256,
                      strlen (sample_public_key_secp256));
  if (err)
    {
      errtxt = _gcry_strerror (err);
      goto failed;
    }

  what = "key consistency";
  err = ecc_check_secret_key(skey);
  if (err)
    {
      errtxt = _gcry_strerror (err);
      goto failed;
    }

  if (extended)
    {
      what = "sign";
      errtxt = selftest_sign (pkey, skey);
      if (errtxt)
        goto failed;
    }

  what = "digest sign";
  errtxt = selftest_hash_sign (pkey, skey);
  if (errtxt)
    goto failed;

  sexp_release(pkey);
  sexp_release(skey);
  return 0; /* Succeeded. */

 failed:
  sexp_release(pkey);
  sexp_release(skey);
  if (report)
    report ("pubkey", GCRY_PK_ECC, what, errtxt);
  return GPG_ERR_SELFTEST_FAILED;
}


/* Run a full self-test for ALGO and return 0 on success.  */
static gpg_err_code_t
run_selftests (int algo, int extended, selftest_report_func_t report)
{
  if (algo != GCRY_PK_ECC)
    return GPG_ERR_PUBKEY_ALGO;

  return selftests_ecdsa (report, extended);
}




gcry_pk_spec_t _gcry_pubkey_spec_ecc =
  {
    GCRY_PK_ECC, { 0, 1 },
    (GCRY_PK_USAGE_SIGN | GCRY_PK_USAGE_ENCR),
    "ECC", ecc_names,
    "pabgnhq", "pabgnhqd", "se", "rs", "pabgnhq",
    ecc_generate,
    ecc_check_secret_key,
    ecc_encrypt_raw,
    ecc_decrypt_raw,
    ecc_sign,
    ecc_verify,
    ecc_get_nbits,
    run_selftests,
    compute_keygrip,
    _gcry_ecc_get_curve,
    _gcry_ecc_get_param_sexp
  };
