/* rsa.c - RSA implementation
 * Copyright (C) 1997, 1998, 1999 by Werner Koch (dd9jn)
 * Copyright (C) 2000, 2001, 2002, 2003, 2008 Free Software Foundation, Inc.
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

/* This code uses an algorithm protected by U.S. Patent #4,405,829
   which expired on September 20, 2000.  The patent holder placed that
   patent into the public domain on Sep 6th, 2000.
*/

#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "g10lib.h"
#include "mpi.h"
#include "cipher.h"
#include "pubkey-internal.h"


typedef struct
{
  gcry_mpi_t n;	    /* modulus */
  gcry_mpi_t e;	    /* exponent */
} RSA_public_key;


typedef struct
{
  gcry_mpi_t n;	    /* public modulus */
  gcry_mpi_t e;	    /* public exponent */
  gcry_mpi_t d;	    /* exponent */
  gcry_mpi_t p;	    /* prime  p. */
  gcry_mpi_t q;	    /* prime  q. */
  gcry_mpi_t u;	    /* inverse of p mod q. */
} RSA_secret_key;


static const char *rsa_names[] =
  {
    "rsa",
    "openpgp-rsa",
    "oid.1.2.840.113549.1.1.1",
    NULL,
  };


/* A sample 2048 bit RSA key used for the selftests.  */
static const char sample_secret_key[] =
" (private-key"
"  (rsa"
"  (n #009F56231A3D82E3E7D613D59D53E9AB921BEF9F08A782AED0B6E46ADBC853EC"
"      7C71C422435A3CD8FA0DB9EFD55CD3295BADC4E8E2E2B94E15AE82866AB8ADE8"
"      7E469FAE76DC3577DE87F1F419C4EB41123DFAF8D16922D5EDBAD6E9076D5A1C"
"      958106F0AE5E2E9193C6B49124C64C2A241C4075D4AF16299EB87A6585BAE917"
"      DEF27FCDD165764D069BC18D16527B29DAAB549F7BBED4A7C6A842D203ED6613"
"      6E2411744E432CD26D940132F25874483DCAEECDFD95744819CBCF1EA810681C"
"      42907EBCB1C7EAFBE75C87EC32C5413EA10476545D3FC7B2ADB1B66B7F200918"
"      664B0E5261C2895AA28B0DE321E921B3F877172CCCAB81F43EF98002916156F6CB#)"
"   (e #010001#)"
"   (d #07EF82500C403899934FE993AC5A36F14FF2DF38CF1EF315F205EE4C83EDAA19"
"       8890FC23DE9AA933CAFB37B6A8A8DBA675411958337287310D3FF2F1DDC0CB93"
"       7E70F57F75F833C021852B631D2B9A520E4431A03C5C3FCB5742DCD841D9FB12"
"       771AA1620DCEC3F1583426066ED9DC3F7028C5B59202C88FDF20396E2FA0EC4F"
"       5A22D9008F3043673931BC14A5046D6327398327900867E39CC61B2D1AFE2F48"
"       EC8E1E3861C68D257D7425F4E6F99ABD77D61F10CA100EFC14389071831B33DD"
"       69CC8EABEF860D1DC2AAA84ABEAE5DFC91BC124DAF0F4C8EF5BBEA436751DE84"
"       3A8063E827A024466F44C28614F93B0732A100D4A0D86D532FE1E22C7725E401#)"
"   (p #00C29D438F115825779631CD665A5739367F3E128ADC29766483A46CA80897E0"
"       79B32881860B8F9A6A04C2614A904F6F2578DAE13EA67CD60AE3D0AA00A1FF9B"
"       441485E44B2DC3D0B60260FBFE073B5AC72FAF67964DE15C8212C389D20DB9CF"
"       54AF6AEF5C4196EAA56495DD30CF709F499D5AB30CA35E086C2A1589D6283F1783#)"
"   (q #00D1984135231CB243FE959C0CBEF551EDD986AD7BEDF71EDF447BE3DA27AF46"
"       79C974A6FA69E4D52FE796650623DE70622862713932AA2FD9F2EC856EAEAA77"
"       88B4EA6084DC81C902F014829B18EA8B2666EC41586818E0589E18876065F97E"
"       8D22CE2DA53A05951EC132DCEF41E70A9C35F4ACC268FFAC2ADF54FA1DA110B919#)"
"   (u #67CF0FD7635205DD80FA814EE9E9C267C17376BF3209FB5D1BC42890D2822A04"
"       479DAF4D5B6ED69D0F8D1AF94164D07F8CD52ECEFE880641FA0F41DDAB1785E4"
"       A37A32F997A516480B4CD4F6482B9466A1765093ED95023CA32D5EDC1E34CEE9"
"       AF595BC51FE43C4BF810FA225AF697FB473B83815966188A4312C048B885E3F7#)))";

/* A sample 2048 bit RSA key used for the selftests (public only).  */
static const char sample_public_key[] =
" (public-key"
"  (rsa"
"   (n #009F56231A3D82E3E7D613D59D53E9AB921BEF9F08A782AED0B6E46ADBC853EC"
"       7C71C422435A3CD8FA0DB9EFD55CD3295BADC4E8E2E2B94E15AE82866AB8ADE8"
"       7E469FAE76DC3577DE87F1F419C4EB41123DFAF8D16922D5EDBAD6E9076D5A1C"
"       958106F0AE5E2E9193C6B49124C64C2A241C4075D4AF16299EB87A6585BAE917"
"       DEF27FCDD165764D069BC18D16527B29DAAB549F7BBED4A7C6A842D203ED6613"
"       6E2411744E432CD26D940132F25874483DCAEECDFD95744819CBCF1EA810681C"
"       42907EBCB1C7EAFBE75C87EC32C5413EA10476545D3FC7B2ADB1B66B7F200918"
"       664B0E5261C2895AA28B0DE321E921B3F877172CCCAB81F43EF98002916156F6CB#)"
"   (e #010001#)))";


static int test_keys (RSA_secret_key *sk, unsigned nbits);
static int  check_secret_key (RSA_secret_key *sk);
static void public (gcry_mpi_t output, gcry_mpi_t input, RSA_public_key *skey);
static void secret (gcry_mpi_t output, gcry_mpi_t input, RSA_secret_key *skey);
static unsigned int rsa_get_nbits (gcry_sexp_t parms);


/* Check that a freshly generated key actually works.  Returns 0 on success. */
static int
test_keys (RSA_secret_key *sk, unsigned int nbits)
{
  int result = -1; /* Default to failure.  */
  RSA_public_key pk;
  gcry_mpi_t plaintext = mpi_new (nbits);
  gcry_mpi_t ciphertext = mpi_new (nbits);
  gcry_mpi_t decr_plaintext = mpi_new (nbits);
  gcry_mpi_t signature = mpi_new (nbits);

  /* Put the relevant parameters into a public key structure.  */
  pk.n = sk->n;
  pk.e = sk->e;

  /* Create a random plaintext.  */
  _gcry_mpi_randomize (plaintext, nbits, GCRY_WEAK_RANDOM);

  /* Encrypt using the public key.  */
  public (ciphertext, plaintext, &pk);

  /* Check that the cipher text does not match the plaintext.  */
  if (!mpi_cmp (ciphertext, plaintext))
    goto leave; /* Ciphertext is identical to the plaintext.  */

  /* Decrypt using the secret key.  */
  secret (decr_plaintext, ciphertext, sk);

  /* Check that the decrypted plaintext matches the original plaintext.  */
  if (mpi_cmp (decr_plaintext, plaintext))
    goto leave; /* Plaintext does not match.  */

  /* Create another random plaintext as data for signature checking.  */
  _gcry_mpi_randomize (plaintext, nbits, GCRY_WEAK_RANDOM);

  /* Use the RSA secret function to create a signature of the plaintext.  */
  secret (signature, plaintext, sk);

  /* Use the RSA public function to verify this signature.  */
  public (decr_plaintext, signature, &pk);
  if (mpi_cmp (decr_plaintext, plaintext))
    goto leave; /* Signature does not match.  */

  /* Modify the signature and check that the signing fails.  */
  mpi_add_ui (signature, signature, 1);
  public (decr_plaintext, signature, &pk);
  if (!mpi_cmp (decr_plaintext, plaintext))
    goto leave; /* Signature matches but should not.  */

  result = 0; /* All tests succeeded.  */

 leave:
  _gcry_mpi_release (signature);
  _gcry_mpi_release (decr_plaintext);
  _gcry_mpi_release (ciphertext);
  _gcry_mpi_release (plaintext);
  return result;
}

static int
test_keys_fips (gcry_sexp_t skey)
{
  int result = -1; /* Default to failure.  */
  char plaintext[128];
  gcry_sexp_t sig = NULL;
  const char *data_tmpl = "(data (flags pkcs1) (hash %s %b))";
  gcry_md_hd_t hd = NULL;
  int ec;

  /* Create a random plaintext.  */
  _gcry_randomize (plaintext, sizeof plaintext, GCRY_WEAK_RANDOM);

  /* Open MD context and feed the random data in */
  ec = _gcry_md_open (&hd, GCRY_MD_SHA256, 0);
  if (ec)
    goto leave;
  _gcry_md_write (hd, plaintext, sizeof(plaintext));

  /* Use the RSA secret function to create a signature of the plaintext.  */
  ec = _gcry_pk_sign_md (&sig, data_tmpl, hd, skey, NULL);
  if (ec)
    goto leave;

  /* Use the RSA public function to verify this signature.  */
  ec = _gcry_pk_verify_md (sig, data_tmpl, hd, skey, NULL);
  if (ec)
    goto leave;

  /* Modify the data and check that the signing fails.  */
  _gcry_md_reset(hd);
  plaintext[sizeof plaintext / 2] ^= 1;
  _gcry_md_write (hd, plaintext, sizeof(plaintext));
  ec = _gcry_pk_verify_md (sig, data_tmpl, hd, skey, NULL);
  if (ec != GPG_ERR_BAD_SIGNATURE)
    goto leave; /* Signature verification worked on modified data  */

  result = 0; /* All tests succeeded.  */
 leave:
  sexp_release (sig);
  _gcry_md_close (hd);
  return result;
}

/* Callback used by the prime generation to test whether the exponent
   is suitable. Returns 0 if the test has been passed. */
static int
check_exponent (void *arg, gcry_mpi_t a)
{
  gcry_mpi_t e = arg;
  gcry_mpi_t tmp;
  int result;

  mpi_sub_ui (a, a, 1);
  tmp = _gcry_mpi_alloc_like (a);
  result = !mpi_gcd(tmp, e, a); /* GCD is not 1. */
  _gcry_mpi_release (tmp);
  mpi_add_ui (a, a, 1);
  return result;
}

/****************
 * Generate a key pair with a key of size NBITS.
 * USE_E = 0 let Libcgrypt decide what exponent to use.
 *       = 1 request the use of a "secure" exponent; this is required by some
 *           specification to be 65537.
 *       > 2 Use this public exponent.  If the given exponent
 *           is not odd one is internally added to it.
 * TRANSIENT_KEY:  If true, generate the primes using the standard RNG.
 * Returns: 2 structures filled with all needed values
 */
static gpg_err_code_t
generate_std (RSA_secret_key *sk, unsigned int nbits, unsigned long use_e,
              int transient_key)
{
  gcry_mpi_t p, q; /* the two primes */
  gcry_mpi_t d;    /* the private key */
  gcry_mpi_t u;
  gcry_mpi_t t1, t2;
  gcry_mpi_t n;    /* the public key */
  gcry_mpi_t e;    /* the exponent */
  gcry_mpi_t phi;  /* helper: (p-1)(q-1) */
  gcry_mpi_t g;
  gcry_mpi_t f;
  gcry_random_level_t random_level;

  /* The random quality depends on the transient_key flag.  */
  random_level = transient_key ? GCRY_STRONG_RANDOM : GCRY_VERY_STRONG_RANDOM;

  /* Make sure that nbits is even so that we generate p, q of equal size. */
  if ( (nbits&1) )
    nbits++;

  if (use_e == 1)   /* Alias for a secure value */
    use_e = 65537;  /* as demanded by Sphinx. */

  /* Public exponent:
     In general we use 41 as this is quite fast and more secure than the
     commonly used 17.  Benchmarking the RSA verify function
     with a 1024 bit key yields (2001-11-08):
     e=17    0.54 ms
     e=41    0.75 ms
     e=257   0.95 ms
     e=65537 1.80 ms
  */
  e = mpi_alloc( (32+BITS_PER_MPI_LIMB-1)/BITS_PER_MPI_LIMB );
  if (!use_e)
    mpi_set_ui (e, 41);     /* This is a reasonable secure and fast value */
  else
    {
      use_e |= 1; /* make sure this is odd */
      mpi_set_ui (e, use_e);
    }

  n = mpi_new (nbits);

  p = q = NULL;
  do
    {
      /* select two (very secret) primes */
      if (p)
        _gcry_mpi_release (p);
      if (q)
        _gcry_mpi_release (q);
      if (use_e)
        { /* Do an extra test to ensure that the given exponent is
             suitable. */
          p = _gcry_generate_secret_prime (nbits/2, random_level,
                                           check_exponent, e);
          q = _gcry_generate_secret_prime (nbits/2, random_level,
                                           check_exponent, e);
        }
      else
        { /* We check the exponent later. */
          p = _gcry_generate_secret_prime (nbits/2, random_level, NULL, NULL);
          q = _gcry_generate_secret_prime (nbits/2, random_level, NULL, NULL);
        }
      if (mpi_cmp (p, q) > 0 ) /* p shall be smaller than q (for calc of u)*/
        mpi_swap(p,q);
      /* calculate the modulus */
      mpi_mul( n, p, q );
    }
  while ( mpi_get_nbits(n) != nbits );

  /* calculate Euler totient: phi = (p-1)(q-1) */
  t1 = mpi_alloc_secure( mpi_get_nlimbs(p) );
  t2 = mpi_alloc_secure( mpi_get_nlimbs(p) );
  phi   = mpi_snew ( nbits );
  g	= mpi_snew ( nbits );
  f	= mpi_snew ( nbits );
  mpi_sub_ui( t1, p, 1 );
  mpi_sub_ui( t2, q, 1 );
  mpi_mul( phi, t1, t2 );
  mpi_gcd (g, t1, t2);
  mpi_fdiv_q(f, phi, g);

  while (!mpi_gcd(t1, e, phi)) /* (while gcd is not 1) */
    {
      if (use_e)
        BUG (); /* The prime generator already made sure that we
                   never can get to here. */
      mpi_add_ui (e, e, 2);
    }

  /* calculate the secret key d = e^-1 mod phi */
  d = mpi_snew ( nbits );
  mpi_invm (d, e, f );
  /* calculate the inverse of p and q (used for chinese remainder theorem)*/
  u = mpi_snew ( nbits );
  mpi_invm(u, p, q );

  if( DBG_CIPHER )
    {
      log_mpidump("  p= ", p );
      log_mpidump("  q= ", q );
      log_mpidump("phi= ", phi );
      log_mpidump("  g= ", g );
      log_mpidump("  f= ", f );
      log_mpidump("  n= ", n );
      log_mpidump("  e= ", e );
      log_mpidump("  d= ", d );
      log_mpidump("  u= ", u );
    }

  _gcry_mpi_release (t1);
  _gcry_mpi_release (t2);
  _gcry_mpi_release (phi);
  _gcry_mpi_release (f);
  _gcry_mpi_release (g);

  sk->n = n;
  sk->e = e;
  sk->p = p;
  sk->q = q;
  sk->d = d;
  sk->u = u;

  /* Now we can test our keys. */
  if (test_keys (sk, nbits - 64))
    {
      _gcry_mpi_release (sk->n); sk->n = NULL;
      _gcry_mpi_release (sk->e); sk->e = NULL;
      _gcry_mpi_release (sk->p); sk->p = NULL;
      _gcry_mpi_release (sk->q); sk->q = NULL;
      _gcry_mpi_release (sk->d); sk->d = NULL;
      _gcry_mpi_release (sk->u); sk->u = NULL;
      fips_signal_error ("self-test after key generation failed");
      return GPG_ERR_SELFTEST_FAILED;
    }

  return 0;
}


/* Check the RSA key length is acceptable for key generation or usage */
static gpg_err_code_t
rsa_check_keysize (unsigned int nbits)
{
  if (fips_mode () && nbits < 2048)
    return GPG_ERR_INV_VALUE;

  return GPG_ERR_NO_ERROR;
}


/* Check the RSA key length is acceptable for signature verification
 *
 * FIPS allows signature verification with RSA keys of size
 * 1024, 1280, 1536 and 1792 in legacy mode, but this is up to the
 * calling application to decide if the signature is legacy and
 * should be accepted.
 */
static gpg_err_code_t
rsa_check_verify_keysize (unsigned int nbits)
{
  if (fips_mode ())
    {
      if ((nbits >= 1024 && (nbits % 256) == 0) || nbits >= 2048)
        return GPG_ERR_NO_ERROR;

      return GPG_ERR_INV_VALUE;
    }

  return GPG_ERR_NO_ERROR;
}


/****************
 * Generate a key pair with a key of size NBITS.
 * USE_E = 0 let Libcgrypt decide what exponent to use.
 *       = 1 request the use of a "secure" exponent; this is required by some
 *           specification to be 65537.
 *       > 2 Use this public exponent.  If the given exponent
 *           is not odd one is internally added to it.
 * TESTPARMS: If set, do not generate but test whether the p,q is probably prime
 *            Returns key with zeroes to not break code calling this function.
 * TRANSIENT_KEY:  If true, generate the primes using the standard RNG.
 * Returns: 2 structures filled with all needed values
 */
static gpg_err_code_t
generate_fips (RSA_secret_key *sk, unsigned int nbits, unsigned long use_e,
               gcry_sexp_t testparms, int transient_key)
{
  gcry_mpi_t p, q; /* the two primes */
  gcry_mpi_t d;    /* the private key */
  gcry_mpi_t u;
  gcry_mpi_t p1, q1;
  gcry_mpi_t n;    /* the public key */
  gcry_mpi_t e;    /* the exponent */
  gcry_mpi_t g;
  gcry_mpi_t minp;
  gcry_mpi_t diff, mindiff;
  gcry_random_level_t random_level;
  unsigned int pbits = nbits/2;
  unsigned int i;
  int pqswitch;
  gpg_err_code_t ec;

  if (nbits <= 1024 || (nbits & 0x1FF))
    return GPG_ERR_INV_VALUE;
  ec = rsa_check_keysize (nbits);
  if (ec)
    return ec;

  /* Set default error code.  */
  ec = GPG_ERR_NO_PRIME;

  /* The random quality depends on the transient_key flag.  */
  random_level = transient_key ? GCRY_STRONG_RANDOM : GCRY_VERY_STRONG_RANDOM;

  if (testparms)
    {
      /* Parameters to derive the key are given.  */
      /* Note that we explicitly need to setup the values of tbl
         because some compilers (e.g. OpenWatcom, IRIX) don't allow to
         initialize a structure with automatic variables.  */
      struct { const char *name; gcry_mpi_t *value; } tbl[] = {
        { "e" },
        { "p" },
        { "q" },
        { NULL }
      };
      int idx;
      gcry_sexp_t oneparm;

      tbl[0].value = &e;
      tbl[1].value = &p;
      tbl[2].value = &q;

      for (idx=0; tbl[idx].name; idx++)
        {
          oneparm = sexp_find_token (testparms, tbl[idx].name, 0);
          if (oneparm)
            {
              *tbl[idx].value = sexp_nth_mpi (oneparm, 1, GCRYMPI_FMT_USG);
              sexp_release (oneparm);
            }
        }
      for (idx=0; tbl[idx].name; idx++)
        if (!*tbl[idx].value)
          break;
      if (tbl[idx].name)
        {
          /* At least one parameter is missing.  */
          for (idx=0; tbl[idx].name; idx++)
            _gcry_mpi_release (*tbl[idx].value);
          return GPG_ERR_MISSING_VALUE;
        }
    }
  else
    {
      if (use_e < 65537)
        use_e = 65537;  /* This is the smallest value allowed by FIPS */

      e = mpi_alloc ((32+BITS_PER_MPI_LIMB-1)/BITS_PER_MPI_LIMB);

      use_e |= 1; /* make sure this is odd */
      mpi_set_ui (e, use_e);

      p = mpi_snew (pbits);
      q = mpi_snew (pbits);
    }

  n = mpi_new (nbits);
  d = mpi_snew (nbits);
  u = mpi_snew (nbits);

  /* prepare approximate minimum p and q */
  minp = mpi_new (pbits);
  mpi_set_ui (minp, 0xB504F334);
  mpi_lshift (minp, minp, pbits - 32);

  /* prepare minimum p and q difference */
  diff = mpi_new (pbits);
  mindiff = mpi_new (pbits - 99);
  mpi_set_ui (mindiff, 1);
  mpi_lshift (mindiff, mindiff, pbits - 100);

  p1 = mpi_snew (pbits);
  q1 = mpi_snew (pbits);
  g  = mpi_snew (pbits);

 retry:
  /* generate p and q */
  for (i = 0; i < 10 * pbits; i++)
    {
    ploop:
      if (!testparms)
        {
          _gcry_mpi_randomize (p, pbits, random_level);
          mpi_set_bit (p, 0);
        }
      if (mpi_cmp (p, minp) < 0)
        {
          if (testparms)
            goto err;
          goto ploop;
        }

      mpi_sub_ui (p1, p, 1);
      if (mpi_gcd (g, p1, e))
        {
          if (_gcry_fips186_4_prime_check (p, pbits) != GPG_ERR_NO_ERROR)
            {
              /* not a prime */
              if (testparms)
                goto err;
            }
          else
            break;
        }
      else if (testparms)
        goto err;
    }
  if (i >= 10 * pbits)
    goto err;

  for (i = 0; i < 20 * pbits; i++)
    {
    qloop:
      if (!testparms)
        {
          _gcry_mpi_randomize (q, pbits, random_level);
          mpi_set_bit (q, 0);
        }
      if (mpi_cmp (q, minp) < 0)
        {
          if (testparms)
            goto err;
          goto qloop;
        }
      if (mpi_cmp (p, q) > 0)
        {
          pqswitch = 1;
          mpi_sub (diff, p, q);
        }
      else
        {
          pqswitch = 0;
          mpi_sub (diff, q, p);
        }
      if (mpi_cmp (diff, mindiff) < 0)
        {
          if (testparms)
            goto err;
          goto qloop;
        }

      mpi_sub_ui (q1, q, 1);
      if (mpi_gcd (g, q1, e))
        {
          if (_gcry_fips186_4_prime_check (q, pbits) != GPG_ERR_NO_ERROR)
            {
              /* not a prime */
              if (testparms)
                goto err;
            }
          else
            break;
        }
      else if (testparms)
        goto err;
    }
  if (i >= 20 * pbits)
    goto err;

  if (testparms)
    {
      mpi_clear (p);
      mpi_clear (q);
    }
  else
    {
      gcry_mpi_t f;

      if (pqswitch)
        {
          gcry_mpi_t tmp;

          tmp = p;
          p = q;
          q = tmp;
        }

      f = mpi_snew (nbits);

      /* calculate the modulus */
      mpi_mul (n, p, q);

      /* calculate the secret key d = e^1 mod phi */
      mpi_gcd (g, p1, q1);
      mpi_fdiv_q (f, p1, g);
      mpi_mul (f, f, q1);

      mpi_invm (d, e, f);

      _gcry_mpi_release (f);

      if (mpi_get_nbits (d) < pbits)
        goto retry;

      /* calculate the inverse of p and q (used for chinese remainder theorem)*/
      mpi_invm (u, p, q );
    }

  ec = 0; /* Success.  */

  if (DBG_CIPHER)
    {
      log_mpidump("  p= ", p );
      log_mpidump("  q= ", q );
      log_mpidump("  n= ", n );
      log_mpidump("  e= ", e );
      log_mpidump("  d= ", d );
      log_mpidump("  u= ", u );
    }

 err:

  _gcry_mpi_release (p1);
  _gcry_mpi_release (q1);
  _gcry_mpi_release (g);
  _gcry_mpi_release (minp);
  _gcry_mpi_release (mindiff);
  _gcry_mpi_release (diff);

  sk->n = n;
  sk->e = e;
  sk->p = p;
  sk->q = q;
  sk->d = d;
  sk->u = u;

  if (ec)
    {
      _gcry_mpi_release (sk->n); sk->n = NULL;
      _gcry_mpi_release (sk->e); sk->e = NULL;
      _gcry_mpi_release (sk->p); sk->p = NULL;
      _gcry_mpi_release (sk->q); sk->q = NULL;
      _gcry_mpi_release (sk->d); sk->d = NULL;
      _gcry_mpi_release (sk->u); sk->u = NULL;
    }

  return ec;
}


/* Helper for generate_x931.  */
static gcry_mpi_t
gen_x931_parm_xp (unsigned int nbits)
{
  gcry_mpi_t xp;

  xp = mpi_snew (nbits);
  _gcry_mpi_randomize (xp, nbits, GCRY_VERY_STRONG_RANDOM);

  /* The requirement for Xp is:

       sqrt{2}*2^{nbits-1} <= xp <= 2^{nbits} - 1

     We set the two high order bits to 1 to satisfy the lower bound.
     By using mpi_set_highbit we make sure that the upper bound is
     satisfied as well.  */
  mpi_set_highbit (xp, nbits-1);
  mpi_set_bit (xp, nbits-2);
  gcry_assert ( mpi_get_nbits (xp) == nbits );

  return xp;
}


/* Helper for generate_x931.  */
static gcry_mpi_t
gen_x931_parm_xi (void)
{
  gcry_mpi_t xi;

  xi = mpi_snew (101);
  _gcry_mpi_randomize (xi, 101, GCRY_VERY_STRONG_RANDOM);
  mpi_set_highbit (xi, 100);
  gcry_assert ( mpi_get_nbits (xi) == 101 );

  return xi;
}



/* Variant of the standard key generation code using the algorithm
   from X9.31.  Using this algorithm has the advantage that the
   generation can be made deterministic which is required for CAVS
   testing.  */
static gpg_err_code_t
generate_x931 (RSA_secret_key *sk, unsigned int nbits, unsigned long e_value,
               gcry_sexp_t deriveparms, int *swapped)
{
  gcry_mpi_t p, q; /* The two primes.  */
  gcry_mpi_t e;    /* The public exponent.  */
  gcry_mpi_t n;    /* The public key.  */
  gcry_mpi_t d;    /* The private key */
  gcry_mpi_t u;    /* The inverse of p and q.  */
  gcry_mpi_t pm1;  /* p - 1  */
  gcry_mpi_t qm1;  /* q - 1  */
  gcry_mpi_t phi;  /* Euler totient.  */
  gcry_mpi_t f, g; /* Helper.  */

  *swapped = 0;

  if (e_value == 0)        /* 65537 is the libgcrypt's selection. */
    e_value = 65537;
  else if (e_value == 1)   /* Alias for a secure value. */
    e_value = 65537;

  /* Point 1 of section 4.1:  k = 1024 + 256s with S >= 0  */
  if (nbits < 1024 || (nbits % 256))
    return GPG_ERR_INV_VALUE;

  /* Point 2:  2 <= bitlength(e) < 2^{k-2}
     Note that we do not need to check the upper bound because we use
     an unsigned long for E and thus there is no way for E to reach
     that limit.  */
  if (e_value < 3)
    return GPG_ERR_INV_VALUE;

  /* Our implementation requires E to be odd.  */
  if (!(e_value & 1))
    return GPG_ERR_INV_VALUE;

  /* Point 3:  e > 0 or e 0 if it is to be randomly generated.
     We support only a fixed E and thus there is no need for an extra test.  */


  /* Compute or extract the derive parameters.  */
  {
    gcry_mpi_t xp1 = NULL;
    gcry_mpi_t xp2 = NULL;
    gcry_mpi_t xp  = NULL;
    gcry_mpi_t xq1 = NULL;
    gcry_mpi_t xq2 = NULL;
    gcry_mpi_t xq  = NULL;
    gcry_mpi_t tmpval;

    if (!deriveparms)
      {
        /* Not given: Generate them.  */
        xp = gen_x931_parm_xp (nbits/2);
        /* Make sure that |xp - xq| > 2^{nbits - 100} holds.  */
        tmpval = mpi_snew (nbits/2);
        do
          {
            _gcry_mpi_release (xq);
            xq = gen_x931_parm_xp (nbits/2);
            mpi_sub (tmpval, xp, xq);
          }
        while (mpi_get_nbits (tmpval) <= (nbits/2 - 100));
        _gcry_mpi_release (tmpval);

        xp1 = gen_x931_parm_xi ();
        xp2 = gen_x931_parm_xi ();
        xq1 = gen_x931_parm_xi ();
        xq2 = gen_x931_parm_xi ();

      }
    else
      {
        /* Parameters to derive the key are given.  */
        /* Note that we explicitly need to setup the values of tbl
           because some compilers (e.g. OpenWatcom, IRIX) don't allow
           to initialize a structure with automatic variables.  */
        struct { const char *name; gcry_mpi_t *value; } tbl[] = {
          { "Xp1" },
          { "Xp2" },
          { "Xp"  },
          { "Xq1" },
          { "Xq2" },
          { "Xq"  },
          { NULL }
        };
        int idx;
        gcry_sexp_t oneparm;

        tbl[0].value = &xp1;
        tbl[1].value = &xp2;
        tbl[2].value = &xp;
        tbl[3].value = &xq1;
        tbl[4].value = &xq2;
        tbl[5].value = &xq;

        for (idx=0; tbl[idx].name; idx++)
          {
            oneparm = sexp_find_token (deriveparms, tbl[idx].name, 0);
            if (oneparm)
              {
                *tbl[idx].value = sexp_nth_mpi (oneparm, 1, GCRYMPI_FMT_USG);
                sexp_release (oneparm);
              }
          }
        for (idx=0; tbl[idx].name; idx++)
          if (!*tbl[idx].value)
            break;
        if (tbl[idx].name)
          {
            /* At least one parameter is missing.  */
            for (idx=0; tbl[idx].name; idx++)
              _gcry_mpi_release (*tbl[idx].value);
            return GPG_ERR_MISSING_VALUE;
          }
      }

    e = mpi_alloc_set_ui (e_value);

    /* Find two prime numbers.  */
    p = _gcry_derive_x931_prime (xp, xp1, xp2, e, NULL, NULL);
    q = _gcry_derive_x931_prime (xq, xq1, xq2, e, NULL, NULL);
    _gcry_mpi_release (xp);  xp  = NULL;
    _gcry_mpi_release (xp1); xp1 = NULL;
    _gcry_mpi_release (xp2); xp2 = NULL;
    _gcry_mpi_release (xq);  xq  = NULL;
    _gcry_mpi_release (xq1); xq1 = NULL;
    _gcry_mpi_release (xq2); xq2 = NULL;
    if (!p || !q)
      {
        _gcry_mpi_release (p);
        _gcry_mpi_release (q);
        _gcry_mpi_release (e);
        return GPG_ERR_NO_PRIME;
      }
  }


  /* Compute the public modulus.  We make sure that p is smaller than
     q to allow the use of the CRT.  */
  if (mpi_cmp (p, q) > 0 )
    {
      mpi_swap (p, q);
      *swapped = 1;
    }
  n = mpi_new (nbits);
  mpi_mul (n, p, q);

  /* Compute the Euler totient:  phi = (p-1)(q-1)  */
  pm1 = mpi_snew (nbits/2);
  qm1 = mpi_snew (nbits/2);
  phi = mpi_snew (nbits);
  mpi_sub_ui (pm1, p, 1);
  mpi_sub_ui (qm1, q, 1);
  mpi_mul (phi, pm1, qm1);

  g = mpi_snew (nbits);
  gcry_assert (mpi_gcd (g, e, phi));

  /* Compute: f = lcm(p-1,q-1) = phi / gcd(p-1,q-1) */
  mpi_gcd (g, pm1, qm1);
  f = pm1; pm1 = NULL;
  _gcry_mpi_release (qm1); qm1 = NULL;
  mpi_fdiv_q (f, phi, g);
  _gcry_mpi_release (phi); phi = NULL;
  d = g; g = NULL;
  /* Compute the secret key:  d = e^{-1} mod lcm(p-1,q-1) */
  mpi_invm (d, e, f);

  /* Compute the inverse of p and q.  */
  u = f; f = NULL;
  mpi_invm (u, p, q );

  if( DBG_CIPHER )
    {
      if (*swapped)
        log_debug ("p and q are swapped\n");
      log_mpidump("  p", p );
      log_mpidump("  q", q );
      log_mpidump("  n", n );
      log_mpidump("  e", e );
      log_mpidump("  d", d );
      log_mpidump("  u", u );
    }


  sk->n = n;
  sk->e = e;
  sk->p = p;
  sk->q = q;
  sk->d = d;
  sk->u = u;

  /* Now we can test our keys. */
  if (test_keys (sk, nbits - 64))
    {
      _gcry_mpi_release (sk->n); sk->n = NULL;
      _gcry_mpi_release (sk->e); sk->e = NULL;
      _gcry_mpi_release (sk->p); sk->p = NULL;
      _gcry_mpi_release (sk->q); sk->q = NULL;
      _gcry_mpi_release (sk->d); sk->d = NULL;
      _gcry_mpi_release (sk->u); sk->u = NULL;
      fips_signal_error ("self-test after key generation failed");
      return GPG_ERR_SELFTEST_FAILED;
    }

  return 0;
}


/****************
 * Test whether the secret key is valid.
 * Returns: true if this is a valid key.
 */
static int
check_secret_key( RSA_secret_key *sk )
{
  int rc;
  gcry_mpi_t temp = mpi_alloc( mpi_get_nlimbs(sk->p)*2 );

  mpi_mul(temp, sk->p, sk->q );
  rc = mpi_cmp( temp, sk->n );
  mpi_free(temp);
  return !rc;
}



/****************
 * Public key operation. Encrypt INPUT with PKEY and put result into OUTPUT.
 *
 *	c = m^e mod n
 *
 * Where c is OUTPUT, m is INPUT and e,n are elements of PKEY.
 */
static void
public(gcry_mpi_t output, gcry_mpi_t input, RSA_public_key *pkey )
{
  if( output == input )  /* powm doesn't like output and input the same */
    {
      gcry_mpi_t x = mpi_alloc( mpi_get_nlimbs(input)*2 );
      mpi_powm( x, input, pkey->e, pkey->n );
      mpi_set(output, x);
      mpi_free(x);
    }
  else
    mpi_powm( output, input, pkey->e, pkey->n );
}

#if 0
static void
stronger_key_check ( RSA_secret_key *skey )
{
  gcry_mpi_t t = mpi_alloc_secure ( 0 );
  gcry_mpi_t t1 = mpi_alloc_secure ( 0 );
  gcry_mpi_t t2 = mpi_alloc_secure ( 0 );
  gcry_mpi_t phi = mpi_alloc_secure ( 0 );

  /* check that n == p * q */
  mpi_mul( t, skey->p, skey->q);
  if (mpi_cmp( t, skey->n) )
    log_info ( "RSA Oops: n != p * q\n" );

  /* check that p is less than q */
  if( mpi_cmp( skey->p, skey->q ) > 0 )
    {
      log_info ("RSA Oops: p >= q - fixed\n");
      _gcry_mpi_swap ( skey->p, skey->q);
    }

    /* check that e divides neither p-1 nor q-1 */
    mpi_sub_ui(t, skey->p, 1 );
    mpi_fdiv_r(t, t, skey->e );
    if ( !mpi_cmp_ui( t, 0) )
        log_info ( "RSA Oops: e divides p-1\n" );
    mpi_sub_ui(t, skey->q, 1 );
    mpi_fdiv_r(t, t, skey->e );
    if ( !mpi_cmp_ui( t, 0) )
        log_info ( "RSA Oops: e divides q-1\n" );

    /* check that d is correct */
    mpi_sub_ui( t1, skey->p, 1 );
    mpi_sub_ui( t2, skey->q, 1 );
    mpi_mul( phi, t1, t2 );
    gcry_mpi_gcd(t, t1, t2);
    mpi_fdiv_q(t, phi, t);
    mpi_invm(t, skey->e, t );
    if ( mpi_cmp(t, skey->d ) )
      {
        log_info ( "RSA Oops: d is wrong - fixed\n");
        mpi_set (skey->d, t);
        log_printmpi ("  fixed d", skey->d);
      }

    /* check for correctness of u */
    mpi_invm(t, skey->p, skey->q );
    if ( mpi_cmp(t, skey->u ) )
      {
        log_info ( "RSA Oops: u is wrong - fixed\n");
        mpi_set (skey->u, t);
        log_printmpi ("  fixed u", skey->u);
      }

    log_info ( "RSA secret key check finished\n");

    mpi_free (t);
    mpi_free (t1);
    mpi_free (t2);
    mpi_free (phi);
}
#endif



/* Secret key operation - standard version.
 *
 *	m = c^d mod n
 */
static void
secret_core_std (gcry_mpi_t M, gcry_mpi_t C,
                 gcry_mpi_t D, gcry_mpi_t N)
{
  mpi_powm (M, C, D, N);
}


/* Secret key operation - using the CRT.
 *
 *      m1 = c ^ (d mod (p-1)) mod p
 *      m2 = c ^ (d mod (q-1)) mod q
 *      h = u * (m2 - m1) mod q
 *      m = m1 + h * p
 */
static void
secret_core_crt (gcry_mpi_t M, gcry_mpi_t C,
                 gcry_mpi_t D, unsigned int Nlimbs,
                 gcry_mpi_t P, gcry_mpi_t Q, gcry_mpi_t U)
{
  gcry_mpi_t m1 = mpi_alloc_secure ( Nlimbs + 1 );
  gcry_mpi_t m2 = mpi_alloc_secure ( Nlimbs + 1 );
  gcry_mpi_t h  = mpi_alloc_secure ( Nlimbs + 1 );
  gcry_mpi_t D_blind = mpi_alloc_secure ( Nlimbs + 1 );
  gcry_mpi_t r;
  unsigned int r_nbits;

  r_nbits = mpi_get_nbits (P) / 4;
  if (r_nbits < 96)
    r_nbits = 96;
  r = mpi_secure_new (r_nbits);

  /* d_blind = (d mod (p-1)) + (p-1) * r            */
  /* m1 = c ^ d_blind mod p */
  _gcry_mpi_randomize (r, r_nbits, GCRY_WEAK_RANDOM);
  mpi_set_highbit (r, r_nbits - 1);
  mpi_sub_ui ( h, P, 1 );
  mpi_mul ( D_blind, h, r );
  mpi_fdiv_r ( h, D, h );
  mpi_add ( D_blind, D_blind, h );
  mpi_powm ( m1, C, D_blind, P );

  /* d_blind = (d mod (q-1)) + (q-1) * r            */
  /* m2 = c ^ d_blind mod q */
  _gcry_mpi_randomize (r, r_nbits, GCRY_WEAK_RANDOM);
  mpi_set_highbit (r, r_nbits - 1);
  mpi_sub_ui ( h, Q, 1  );
  mpi_mul ( D_blind, h, r );
  mpi_fdiv_r ( h, D, h );
  mpi_add ( D_blind, D_blind, h );
  mpi_powm ( m2, C, D_blind, Q );

  mpi_free ( r );
  mpi_free ( D_blind );

  /* h = u * ( m2 - m1 ) mod q */
  mpi_sub ( h, m2, m1 );
  if ( mpi_has_sign ( h ) )
    mpi_add ( h, h, Q );
  mpi_mulm ( h, U, h, Q );

  /* m = m1 + h * p */
  mpi_mul ( h, h, P );
  mpi_add ( M, m1, h );

  mpi_free ( h );
  mpi_free ( m1 );
  mpi_free ( m2 );
}


/* Secret key operation.
 * Encrypt INPUT with SKEY and put result into
 * OUTPUT.  SKEY has the secret key parameters.
 */
static void
secret (gcry_mpi_t output, gcry_mpi_t input, RSA_secret_key *skey )
{
  /* Remove superfluous leading zeroes from INPUT.  */
  mpi_normalize (input);

  if (!skey->p || !skey->q || !skey->u)
    {
      secret_core_std (output, input, skey->d, skey->n);
    }
  else
    {
      secret_core_crt (output, input, skey->d, mpi_get_nlimbs (skey->n),
                       skey->p, skey->q, skey->u);
    }
}


static void
secret_blinded (gcry_mpi_t output, gcry_mpi_t input,
                RSA_secret_key *sk, unsigned int nbits)
{
  gcry_mpi_t r;	           /* Random number needed for blinding.  */
  gcry_mpi_t ri;	   /* Modular multiplicative inverse of r.  */
  gcry_mpi_t bldata;       /* Blinded data to decrypt.  */

  /* First, we need a random number r between 0 and n - 1, which is
   * relatively prime to n (i.e. it is neither p nor q).  The random
   * number needs to be only unpredictable, thus we employ the
   * gcry_create_nonce function by using GCRY_WEAK_RANDOM with
   * gcry_mpi_randomize.  */
  r  = mpi_snew (nbits);
  ri = mpi_snew (nbits);
  bldata = mpi_snew (nbits);

  do
    {
      _gcry_mpi_randomize (r, nbits, GCRY_WEAK_RANDOM);
      mpi_mod (r, r, sk->n);
    }
  while (!mpi_invm (ri, r, sk->n));

  /* Do blinding.  We calculate: y = (x * r^e) mod n, where r is the
   * random number, e is the public exponent, x is the non-blinded
   * input data and n is the RSA modulus.  */
  mpi_powm (bldata, r, sk->e, sk->n);
  mpi_mulm (bldata, bldata, input, sk->n);

  /* Perform decryption.  */
  secret (output, bldata, sk);
  _gcry_mpi_release (bldata);

  /* Undo blinding.  Here we calculate: y = (x * r^-1) mod n, where x
   * is the blinded decrypted data, ri is the modular multiplicative
   * inverse of r and n is the RSA modulus.  */
  mpi_mulm (output, output, ri, sk->n);

  _gcry_mpi_release (r);
  _gcry_mpi_release (ri);
}


/*********************************************
 **************  interface  ******************
 *********************************************/

static gcry_err_code_t
rsa_generate (const gcry_sexp_t genparms, gcry_sexp_t *r_skey)
{
  gpg_err_code_t ec;
  unsigned int nbits;
  unsigned long evalue;
  RSA_secret_key sk;
  gcry_sexp_t deriveparms;
  int flags = 0;
  gcry_sexp_t l1;
  gcry_sexp_t swap_info = NULL;
  int testparms = 0;

  memset (&sk, 0, sizeof sk);

  ec = _gcry_pk_util_get_nbits (genparms, &nbits);
  if (ec)
    return ec;

  ec = _gcry_pk_util_get_rsa_use_e (genparms, &evalue);
  if (ec)
    return ec;

  /* Parse the optional flags list.  */
  l1 = sexp_find_token (genparms, "flags", 0);
  if (l1)
    {
      ec = _gcry_pk_util_parse_flaglist (l1, &flags, NULL);
      sexp_release (l1);
      if (ec)
        return ec;
    }

  deriveparms = (genparms?
                 sexp_find_token (genparms, "derive-parms", 0) : NULL);
  if (!deriveparms)
    {
      /* Parse the optional "use-x931" flag. */
      l1 = sexp_find_token (genparms, "use-x931", 0);
      if (l1)
        {
          flags |= PUBKEY_FLAG_USE_X931;
          sexp_release (l1);
        }
    }

  if (deriveparms || (flags & PUBKEY_FLAG_USE_X931))
    {
      int swapped;
      if (fips_mode ())
        {
          sexp_release (deriveparms);
          return GPG_ERR_INV_SEXP;
        }
      ec = generate_x931 (&sk, nbits, evalue, deriveparms, &swapped);
      sexp_release (deriveparms);
      if (!ec && swapped)
        ec = sexp_new (&swap_info, "(misc-key-info(p-q-swapped))", 0, 1);
    }
  else
    {
      /* Parse the optional "transient-key" flag. */
      if (!(flags & PUBKEY_FLAG_TRANSIENT_KEY))
        {
          l1 = sexp_find_token (genparms, "transient-key", 0);
          if (l1)
            {
              flags |= PUBKEY_FLAG_TRANSIENT_KEY;
              sexp_release (l1);
            }
        }
      deriveparms = (genparms? sexp_find_token (genparms, "test-parms", 0)
                     /**/    : NULL);
      if (deriveparms)
        testparms = 1;

      /* Generate.  */
      if (deriveparms || fips_mode ())
        {
          ec = generate_fips (&sk, nbits, evalue, deriveparms,
                              !!(flags & PUBKEY_FLAG_TRANSIENT_KEY));
        }
      else
        {
          ec = generate_std (&sk, nbits, evalue,
                             !!(flags & PUBKEY_FLAG_TRANSIENT_KEY));
        }
      sexp_release (deriveparms);
    }

  if (!ec)
    {
      ec = sexp_build (r_skey, NULL,
                       "(key-data"
                       " (public-key"
                       "  (rsa(n%m)(e%m)))"
                       " (private-key"
                       "  (rsa(n%m)(e%m)(d%m)(p%m)(q%m)(u%m)))"
                       " %S)",
                       sk.n, sk.e,
                       sk.n, sk.e, sk.d, sk.p, sk.q, sk.u,
                       swap_info);
    }

  mpi_free (sk.n);
  mpi_free (sk.e);
  mpi_free (sk.p);
  mpi_free (sk.q);
  mpi_free (sk.d);
  mpi_free (sk.u);
  sexp_release (swap_info);

  if (!ec && !testparms && fips_mode () && test_keys_fips (*r_skey))
    {
      sexp_release (*r_skey); *r_skey = NULL;
      fips_signal_error ("self-test after key generation failed");
      return GPG_ERR_SELFTEST_FAILED;
    }

  return ec;
}


static gcry_err_code_t
rsa_check_secret_key (gcry_sexp_t keyparms)
{
  gcry_err_code_t rc;
  RSA_secret_key sk = {NULL, NULL, NULL, NULL, NULL, NULL};

  /* To check the key we need the optional parameters. */
  rc = sexp_extract_param (keyparms, NULL, "nedpqu",
                           &sk.n, &sk.e, &sk.d, &sk.p, &sk.q, &sk.u,
                           NULL);
  if (rc)
    goto leave;

  if (!check_secret_key (&sk))
    rc = GPG_ERR_BAD_SECKEY;

 leave:
  _gcry_mpi_release (sk.n);
  _gcry_mpi_release (sk.e);
  _gcry_mpi_release (sk.d);
  _gcry_mpi_release (sk.p);
  _gcry_mpi_release (sk.q);
  _gcry_mpi_release (sk.u);
  if (DBG_CIPHER)
    log_debug ("rsa_testkey    => %s\n", gpg_strerror (rc));
  return rc;
}


static gcry_err_code_t
rsa_encrypt (gcry_sexp_t *r_ciph, gcry_sexp_t s_data, gcry_sexp_t keyparms)
{
  gcry_err_code_t rc;
  struct pk_encoding_ctx ctx;
  gcry_mpi_t data = NULL;
  RSA_public_key pk = {NULL, NULL};
  gcry_mpi_t ciph = NULL;
  unsigned int nbits = rsa_get_nbits (keyparms);

  rc = rsa_check_keysize (nbits);
  if (rc)
    return rc;

  _gcry_pk_util_init_encoding_ctx (&ctx, PUBKEY_OP_ENCRYPT, nbits);

  /* Extract the data.  */
  rc = _gcry_pk_util_data_to_mpi (s_data, &data, &ctx);
  if (rc)
    goto leave;
  if (DBG_CIPHER)
    log_mpidump ("rsa_encrypt data", data);
  if (!data || mpi_is_opaque (data))
    {
      rc = GPG_ERR_INV_DATA;
      goto leave;
    }

  /* Extract the key.  */
  rc = sexp_extract_param (keyparms, NULL, "ne", &pk.n, &pk.e, NULL);
  if (rc)
    goto leave;
  if (DBG_CIPHER)
    {
      log_mpidump ("rsa_encrypt    n", pk.n);
      log_mpidump ("rsa_encrypt    e", pk.e);
    }

  /* Do RSA computation and build result.  */
  ciph = mpi_new (0);
  public (ciph, data, &pk);
  if (DBG_CIPHER)
    log_mpidump ("rsa_encrypt  res", ciph);
  if ((ctx.flags & PUBKEY_FLAG_FIXEDLEN))
    {
      /* We need to make sure to return the correct length to avoid
         problems with missing leading zeroes.  */
      unsigned char *em;
      size_t emlen = (mpi_get_nbits (pk.n)+7)/8;

      rc = _gcry_mpi_to_octet_string (&em, NULL, ciph, emlen);
      if (!rc)
        {
          rc = sexp_build (r_ciph, NULL, "(enc-val(rsa(a%b)))", (int)emlen, em);
          xfree (em);
        }
    }
  else
    rc = sexp_build (r_ciph, NULL, "(enc-val(rsa(a%m)))", ciph);

 leave:
  _gcry_mpi_release (ciph);
  _gcry_mpi_release (pk.n);
  _gcry_mpi_release (pk.e);
  _gcry_mpi_release (data);
  _gcry_pk_util_free_encoding_ctx (&ctx);
  if (DBG_CIPHER)
    log_debug ("rsa_encrypt    => %s\n", gpg_strerror (rc));
  return rc;
}


static gcry_err_code_t
rsa_decrypt (gcry_sexp_t *r_plain, gcry_sexp_t s_data, gcry_sexp_t keyparms)

{
  gpg_err_code_t rc;
  struct pk_encoding_ctx ctx;
  gcry_sexp_t l1 = NULL;
  gcry_mpi_t data = NULL;
  RSA_secret_key sk = {NULL, NULL, NULL, NULL, NULL, NULL};
  gcry_mpi_t plain = NULL;
  unsigned char *unpad = NULL;
  size_t unpadlen = 0;
  unsigned int nbits = rsa_get_nbits (keyparms);

  rc = rsa_check_keysize (nbits);
  if (rc)
    return rc;

  _gcry_pk_util_init_encoding_ctx (&ctx, PUBKEY_OP_DECRYPT, nbits);

  /* Extract the data.  */
  rc = _gcry_pk_util_preparse_encval (s_data, rsa_names, &l1, &ctx);
  if (rc)
    goto leave;
  rc = sexp_extract_param (l1, NULL, "a", &data, NULL);
  if (rc)
    goto leave;
  if (DBG_CIPHER)
    log_printmpi ("rsa_decrypt data", data);
  if (mpi_is_opaque (data))
    {
      rc = GPG_ERR_INV_DATA;
      goto leave;
    }
  if (fips_mode () && (ctx.encoding == PUBKEY_ENC_PKCS1 ||
                       ctx.encoding == PUBKEY_ENC_OAEP))
    {
      rc = GPG_ERR_INV_FLAG;
      goto leave;
    }

  /* Extract the key.  */
  rc = sexp_extract_param (keyparms, NULL, "nedp?q?u?",
                           &sk.n, &sk.e, &sk.d, &sk.p, &sk.q, &sk.u,
                           NULL);
  if (rc)
    goto leave;
  if (DBG_CIPHER)
    {
      log_printmpi ("rsa_decrypt    n", sk.n);
      log_printmpi ("rsa_decrypt    e", sk.e);
      if (!fips_mode ())
        {
          log_printmpi ("rsa_decrypt    d", sk.d);
          log_printmpi ("rsa_decrypt    p", sk.p);
          log_printmpi ("rsa_decrypt    q", sk.q);
          log_printmpi ("rsa_decrypt    u", sk.u);
        }
    }

  /* Better make sure that there are no superfluous leading zeroes in
     the input and it has not been "padded" using multiples of N.
     This mitigates side-channel attacks (CVE-2013-4576).  */
  mpi_normalize (data);
  mpi_fdiv_r (data, data, sk.n);

  /* Allocate MPI for the plaintext.  */
  plain = mpi_snew (nbits);

  /* We use blinding by default to mitigate timing attacks which can
     be practically mounted over the network as shown by Brumley and
     Boney in 2003.  */
  if ((ctx.flags & PUBKEY_FLAG_NO_BLINDING))
    secret (plain, data, &sk);
  else
    secret_blinded (plain, data, &sk, nbits);

  if (DBG_CIPHER)
    log_printmpi ("rsa_decrypt  res", plain);

  /* Reverse the encoding and build the s-expression.  */
  switch (ctx.encoding)
    {
    case PUBKEY_ENC_PKCS1:
      rc = _gcry_rsa_pkcs1_decode_for_enc (&unpad, &unpadlen, nbits, plain);
      mpi_free (plain);
      plain = NULL;
      if (!rc)
        rc = sexp_build (r_plain, NULL, "(value %b)", (int)unpadlen, unpad);
      break;

    case PUBKEY_ENC_OAEP:
      rc = _gcry_rsa_oaep_decode (&unpad, &unpadlen,
                                  nbits, ctx.hash_algo,
                                  plain, ctx.label, ctx.labellen);
      mpi_free (plain);
      plain = NULL;
      if (!rc)
        rc = sexp_build (r_plain, NULL, "(value %b)", (int)unpadlen, unpad);
      break;

    default:
      /* Raw format.  For backward compatibility we need to assume a
         signed mpi by using the sexp format string "%m".  */
      rc = sexp_build (r_plain, NULL,
                       (ctx.flags & PUBKEY_FLAG_LEGACYRESULT)
                       ? "%m":"(value %m)", plain);
      break;
    }

 leave:
  xfree (unpad);
  _gcry_mpi_release (plain);
  _gcry_mpi_release (sk.n);
  _gcry_mpi_release (sk.e);
  _gcry_mpi_release (sk.d);
  _gcry_mpi_release (sk.p);
  _gcry_mpi_release (sk.q);
  _gcry_mpi_release (sk.u);
  _gcry_mpi_release (data);
  sexp_release (l1);
  _gcry_pk_util_free_encoding_ctx (&ctx);
  if (DBG_CIPHER)
    log_debug ("rsa_decrypt    => %s\n", gpg_strerror (rc));
  return rc;
}


static gcry_err_code_t
rsa_sign (gcry_sexp_t *r_sig, gcry_sexp_t s_data, gcry_sexp_t keyparms)
{
  gpg_err_code_t rc;
  struct pk_encoding_ctx ctx;
  gcry_mpi_t data = NULL;
  RSA_secret_key sk = {NULL, NULL, NULL, NULL, NULL, NULL};
  RSA_public_key pk;
  gcry_mpi_t sig = NULL;
  gcry_mpi_t result = NULL;
  unsigned int nbits = rsa_get_nbits (keyparms);

  rc = rsa_check_keysize (nbits);
  if (rc)
    return rc;

  _gcry_pk_util_init_encoding_ctx (&ctx, PUBKEY_OP_SIGN, nbits);

  /* Extract the data.  */
  rc = _gcry_pk_util_data_to_mpi (s_data, &data, &ctx);
  if (rc)
    goto leave;
  if (DBG_CIPHER)
    log_printmpi ("rsa_sign   data", data);
  if (mpi_is_opaque (data))
    {
      rc = GPG_ERR_INV_DATA;
      goto leave;
    }

  /* Extract the key.  */
  rc = sexp_extract_param (keyparms, NULL, "nedp?q?u?",
                           &sk.n, &sk.e, &sk.d, &sk.p, &sk.q, &sk.u,
                           NULL);
  if (rc)
    goto leave;
  if (DBG_CIPHER)
    {
      log_printmpi ("rsa_sign      n", sk.n);
      log_printmpi ("rsa_sign      e", sk.e);
      if (!fips_mode ())
        {
          log_printmpi ("rsa_sign      d", sk.d);
          log_printmpi ("rsa_sign      p", sk.p);
          log_printmpi ("rsa_sign      q", sk.q);
          log_printmpi ("rsa_sign      u", sk.u);
        }
    }

  /* Do RSA computation.  */
  sig = mpi_new (0);
  if ((ctx.flags & PUBKEY_FLAG_NO_BLINDING))
    secret (sig, data, &sk);
  else
    secret_blinded (sig, data, &sk, nbits);
  if (DBG_CIPHER)
    log_printmpi ("rsa_sign    res", sig);

  /* Check that the created signature is good.  This detects a failure
     of the CRT algorithm  (Lenstra's attack on RSA's use of the CRT).  */
  result = mpi_new (0);
  pk.n = sk.n;
  pk.e = sk.e;
  public (result, sig, &pk);
  if (mpi_cmp (result, data))
    {
      rc = GPG_ERR_BAD_SIGNATURE;
      goto leave;
    }

  /* Convert the result.  */
  if ((ctx.flags & PUBKEY_FLAG_FIXEDLEN))
    {
      /* We need to make sure to return the correct length to avoid
         problems with missing leading zeroes.  */
      unsigned char *em;
      size_t emlen = (mpi_get_nbits (sk.n)+7)/8;

      rc = _gcry_mpi_to_octet_string (&em, NULL, sig, emlen);
      if (!rc)
        {
          rc = sexp_build (r_sig, NULL, "(sig-val(rsa(s%b)))", (int)emlen, em);
          xfree (em);
        }
    }
  else
    rc = sexp_build (r_sig, NULL, "(sig-val(rsa(s%M)))", sig);


 leave:
  _gcry_mpi_release (result);
  _gcry_mpi_release (sig);
  _gcry_mpi_release (sk.n);
  _gcry_mpi_release (sk.e);
  _gcry_mpi_release (sk.d);
  _gcry_mpi_release (sk.p);
  _gcry_mpi_release (sk.q);
  _gcry_mpi_release (sk.u);
  _gcry_mpi_release (data);
  _gcry_pk_util_free_encoding_ctx (&ctx);
  if (DBG_CIPHER)
    log_debug ("rsa_sign      => %s\n", gpg_strerror (rc));
  return rc;
}


static gcry_err_code_t
rsa_verify (gcry_sexp_t s_sig, gcry_sexp_t s_data, gcry_sexp_t keyparms)
{
  gcry_err_code_t rc;
  struct pk_encoding_ctx ctx;
  gcry_sexp_t l1 = NULL;
  gcry_mpi_t sig = NULL;
  gcry_mpi_t data = NULL;
  RSA_public_key pk = { NULL, NULL };
  gcry_mpi_t result = NULL;
  unsigned int nbits = rsa_get_nbits (keyparms);

  rc = rsa_check_verify_keysize (nbits);
  if (rc)
    return rc;

  _gcry_pk_util_init_encoding_ctx (&ctx, PUBKEY_OP_VERIFY, nbits);

  /* Extract the data.  */
  rc = _gcry_pk_util_data_to_mpi (s_data, &data, &ctx);
  if (rc)
    goto leave;
  if (DBG_CIPHER)
    log_printmpi ("rsa_verify data", data);
  if (ctx.encoding != PUBKEY_ENC_PSS && mpi_is_opaque (data))
    {
      rc = GPG_ERR_INV_DATA;
      goto leave;
    }

  /* Extract the signature value.  */
  rc = _gcry_pk_util_preparse_sigval (s_sig, rsa_names, &l1, NULL);
  if (rc)
    goto leave;
  rc = sexp_extract_param (l1, NULL, "s", &sig, NULL);
  if (rc)
    goto leave;
  if (DBG_CIPHER)
    log_printmpi ("rsa_verify  sig", sig);

  /* Extract the key.  */
  rc = sexp_extract_param (keyparms, NULL, "ne", &pk.n, &pk.e, NULL);
  if (rc)
    goto leave;
  if (DBG_CIPHER)
    {
      log_printmpi ("rsa_verify    n", pk.n);
      log_printmpi ("rsa_verify    e", pk.e);
    }

  /* Do RSA computation and compare.  */
  result = mpi_new (0);
  public (result, sig, &pk);
  if (DBG_CIPHER)
    log_printmpi ("rsa_verify  cmp", result);
  if (ctx.verify_cmp)
    rc = ctx.verify_cmp (&ctx, result);
  else
    rc = mpi_cmp (result, data) ? GPG_ERR_BAD_SIGNATURE : 0;

 leave:
  _gcry_mpi_release (result);
  _gcry_mpi_release (pk.n);
  _gcry_mpi_release (pk.e);
  _gcry_mpi_release (data);
  _gcry_mpi_release (sig);
  sexp_release (l1);
  _gcry_pk_util_free_encoding_ctx (&ctx);
  if (DBG_CIPHER)
    log_debug ("rsa_verify    => %s\n", rc?gpg_strerror (rc):"Good");
  return rc;
}



/* Return the number of bits for the key described by PARMS.  On error
 * 0 is returned.  The format of PARMS starts with the algorithm name;
 * for example:
 *
 *   (rsa
 *     (n <mpi>)
 *     (e <mpi>))
 *
 * More parameters may be given but we only need N here.
 */
static unsigned int
rsa_get_nbits (gcry_sexp_t parms)
{
  gcry_sexp_t l1;
  gcry_mpi_t n;
  unsigned int nbits;

  l1 = sexp_find_token (parms, "n", 1);
  if (!l1)
    return 0; /* Parameter N not found.  */

  n = sexp_nth_mpi (l1, 1, GCRYMPI_FMT_USG);
  sexp_release (l1);
  nbits = n? mpi_get_nbits (n) : 0;
  _gcry_mpi_release (n);
  return nbits;
}


/* Compute a keygrip.  MD is the hash context which we are going to
   update.  KEYPARAM is an S-expression with the key parameters, this
   is usually a public key but may also be a secret key.  An example
   of such an S-expression is:

      (rsa
        (n #00B...#)
        (e #010001#))

   PKCS-15 says that for RSA only the modulus should be hashed -
   however, it is not clear whether this is meant to use the raw bytes
   (assuming this is an unsigned integer) or whether the DER required
   0 should be prefixed.  We hash the raw bytes.  */
static gpg_err_code_t
compute_keygrip (gcry_md_hd_t md, gcry_sexp_t keyparam)
{
  gcry_sexp_t l1;
  const char *data;
  size_t datalen;

  l1 = sexp_find_token (keyparam, "n", 1);
  if (!l1)
    return GPG_ERR_NO_OBJ;

  data = sexp_nth_data (l1, 1, &datalen);
  if (!data)
    {
      sexp_release (l1);
      return GPG_ERR_NO_OBJ;
    }

  _gcry_md_write (md, data, datalen);
  sexp_release (l1);

  return 0;
}




/*
     Self-test section.
 */

static const char *
selftest_hash_sign_2048 (gcry_sexp_t pkey, gcry_sexp_t skey)
{
  int md_algo = GCRY_MD_SHA256;
  gcry_md_hd_t hd = NULL;
  const char *data_tmpl = "(data (flags pkcs1) (hash %s %b))";
  static const char sample_data[] =
    "11223344556677889900aabbccddeeff"
    "102030405060708090a0b0c0d0f01121";
  static const char sample_data_bad[] =
    "11223344556677889900aabbccddeeff"
    "802030405060708090a0b0c0d0f01121";

  const char *errtxt = NULL;
  gcry_error_t err;
  gcry_sexp_t sig = NULL;
  /* raw signature data reference */
  const char ref_data[] =
    "518f41dea3ad884e93eefff8d7ca68a6f4c30d923632e35673651d675cebd652"
    "a44ed66f6879b18f3d48b2d235b1dd78f6189be1440352cc94231a55c1f93109"
    "84616b2841c42fe9a6e37be34cd188207209bd028e2fa93e721fbac40c31a068"
    "1253b312d4e07addb9c7f3d508fa89f218ea7c7f7b9f6a9b1e522c19fa1cd839"
    "93f9d4ca2f16c3d0b9abafe5e63e848152afc72ce7ee19ea45353116f85209ea"
    "b9de42129dbccdac8faa461e8e8cc2ae801101cc6add4ba76ccb752030b0e827"
    "7352b11cdecebae9cdc9a626c4701cd9c85cd287618888c5fae8b4d0ba48915d"
    "e5cc64e3aee2ba2862d04348ea71f65454f74f9fd1e3108005cc367ca41585a4";
  gcry_mpi_t ref_mpi = NULL;
  gcry_mpi_t sig_mpi = NULL;

  err = _gcry_md_open (&hd, md_algo, 0);
  if (err)
    {
      errtxt = "gcry_md_open failed";
      goto leave;
    }

  _gcry_md_write (hd, sample_data, sizeof(sample_data));

  err = _gcry_pk_sign_md (&sig, data_tmpl, hd, skey, NULL);
  if (err)
    {
      errtxt = "signing failed";
      goto leave;
    }

  err = _gcry_mpi_scan(&ref_mpi, GCRYMPI_FMT_HEX, ref_data, 0, NULL);
  if (err)
    {
      errtxt = "converting ref_data to mpi failed";
      goto leave;
    }

  err = _gcry_sexp_extract_param(sig, "sig-val!rsa", "s", &sig_mpi, NULL);
  if (err)
    {
      errtxt = "extracting signature data failed";
      goto leave;
    }

  if (mpi_cmp (sig_mpi, ref_mpi))
    {
      errtxt = "signature does not match reference data";
      goto leave;
    }

  err = _gcry_pk_verify_md (sig, data_tmpl, hd, pkey, NULL);
  if (err)
    {
      errtxt = "verify failed";
      goto leave;
    }

  _gcry_md_reset(hd);
  _gcry_md_write (hd, sample_data_bad, sizeof(sample_data_bad));
  err = _gcry_pk_verify_md (sig, data_tmpl, hd, pkey, NULL);
  if (gcry_err_code (err) != GPG_ERR_BAD_SIGNATURE)
    {
      errtxt = "bad signature not detected";
      goto leave;
    }


 leave:
  sexp_release (sig);
  _gcry_md_close (hd);
  _gcry_mpi_release (ref_mpi);
  _gcry_mpi_release (sig_mpi);
  return errtxt;
}

static const char *
selftest_sign_2048 (gcry_sexp_t pkey, gcry_sexp_t skey)
{
  static const char sample_data[] =
    "(data (flags pkcs1)"
    " (hash sha256 #11223344556677889900aabbccddeeff"
    /**/           "102030405060708090a0b0c0d0f01121#))";
  static const char sample_data_bad[] =
    "(data (flags pkcs1)"
    " (hash sha256 #11223344556677889900aabbccddeeff"
    /**/           "802030405060708090a0b0c0d0f01121#))";

  const char *errtxt = NULL;
  gcry_error_t err;
  gcry_sexp_t data = NULL;
  gcry_sexp_t data_bad = NULL;
  gcry_sexp_t sig = NULL;
  /* raw signature data reference */
  const char ref_data[] =
    "6252a19a11e1d5155ed9376036277193d644fa239397fff03e9b92d6f86415d6"
    "d30da9273775f290e580d038295ff8ff89522becccfa6ae870bf76b76df402a8"
    "54f69347e3db3de8e1e7d4dada281ec556810c7a8ecd0b5f51f9b1c0e7aa7557"
    "61aa2b8ba5f811304acc6af0eca41fe49baf33bf34eddaf44e21e036ac7f0b68"
    "03cdef1c60021fb7b5b97ebacdd88ab755ce29af568dbc5728cc6e6eff42618d"
    "62a0386ca8beed46402bdeeef29b6a3feded906bace411a06a39192bf516ae10"
    "67e4320fa8ea113968525f4574d022a3ceeaafdc41079efe1f22cc94bf59d8d3"
    "328085da9674857db56de5978a62394aab48aa3b72e23a1b16260cfd9daafe65";
  gcry_mpi_t ref_mpi = NULL;
  gcry_mpi_t sig_mpi = NULL;

  err = sexp_sscan (&data, NULL, sample_data, strlen (sample_data));
  if (!err)
    err = sexp_sscan (&data_bad, NULL,
                      sample_data_bad, strlen (sample_data_bad));
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

  err = _gcry_mpi_scan(&ref_mpi, GCRYMPI_FMT_HEX, ref_data, 0, NULL);
  if (err)
    {
      errtxt = "converting ref_data to mpi failed";
      goto leave;
    }

  err = _gcry_sexp_extract_param(sig, "sig-val!rsa", "s", &sig_mpi, NULL);
  if (err)
    {
      errtxt = "extracting signature data failed";
      goto leave;
    }

  if (mpi_cmp (sig_mpi, ref_mpi))
    {
      errtxt = "signature does not match reference data";
      goto leave;
    }

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
  _gcry_mpi_release (ref_mpi);
  _gcry_mpi_release (sig_mpi);
  return errtxt;
}



/* Given an S-expression ENCR_DATA of the form:

   (enc-val
    (rsa
     (a a-value)))

   as returned by gcry_pk_decrypt, return the the A-VALUE.  On error,
   return NULL.  */
static gcry_mpi_t
extract_a_from_sexp (gcry_sexp_t encr_data)
{
  gcry_sexp_t l1, l2, l3;
  gcry_mpi_t a_value;

  l1 = sexp_find_token (encr_data, "enc-val", 0);
  if (!l1)
    return NULL;
  l2 = sexp_find_token (l1, "rsa", 0);
  sexp_release (l1);
  if (!l2)
    return NULL;
  l3 = sexp_find_token (l2, "a", 0);
  sexp_release (l2);
  if (!l3)
    return NULL;
  a_value = sexp_nth_mpi (l3, 1, 0);
  sexp_release (l3);

  return a_value;
}


static const char *
selftest_encr_2048 (gcry_sexp_t pkey, gcry_sexp_t skey)
{
  const char *errtxt = NULL;
  gcry_error_t err;
  static const char plaintext[] =
    "Jim quickly realized that the beautiful gowns are expensive.";
  gcry_sexp_t plain = NULL;
  gcry_sexp_t encr  = NULL;
  gcry_mpi_t  ciphertext = NULL;
  gcry_sexp_t decr  = NULL;
  char *decr_plaintext = NULL;
  gcry_sexp_t tmplist = NULL;
  /* expected result of encrypting the plaintext with sample_secret_key */
  static const char ref_data[] =
    "18022e2593a402a737caaa93b4c7e750e20ca265452980e1d6b7710fbd3e"
    "7dce72be5c2110fb47691cb38f42170ee3b4a37f2498d4a51567d762585e"
    "4cb81d04fbc7df4144f8e5eac2d4b8688521b64011f11d7ad53f4c874004"
    "819856f2e2a6f83d1c9c4e73ac26089789c14482b0b8d44139133c88c4a5"
    "2dba9dd6d6ffc622666b7d129168333d999706af30a2d7d272db7734e5ed"
    "fb8c64ea3018af3ad20f4a013a5060cb0f5e72753967bebe294280a6ed0d"
    "dbd3c4f11d0a8696e9d32a0dc03deb0b5e49b2cbd1503392642d4e1211f3"
    "e8e2ee38abaa3671ccd57fcde8ca76e85fd2cb77c35706a970a213a27352"
    "cec92a9604d543ddb5fc478ff50e0622";
  gcry_mpi_t ref_mpi = NULL;

  /* Put the plaintext into an S-expression.  */
  err = sexp_build (&plain, NULL, "(data (flags raw) (value %s))", plaintext);
  if (err)
    {
      errtxt = "converting data failed";
      goto leave;
    }

  /* Encrypt.  */
  err = _gcry_pk_encrypt (&encr, plain, pkey);
  if (err)
    {
      errtxt = "encrypt failed";
      goto leave;
    }

  err = _gcry_mpi_scan(&ref_mpi, GCRYMPI_FMT_HEX, ref_data, 0, NULL);
  if (err)
    {
      errtxt = "converting encrydata to mpi failed";
      goto leave;
    }

  /* Extraxt the ciphertext from the returned S-expression.  */
  /*sexp_dump (encr);*/
  ciphertext = extract_a_from_sexp (encr);
  if (!ciphertext)
    {
      errtxt = "gcry_pk_encrypt returned garbage";
      goto leave;
    }

  /* Check that the ciphertext does no match the plaintext.  */
  /* _gcry_log_printmpi ("plaintext", plaintext); */
  /* _gcry_log_printmpi ("ciphertxt", ciphertext); */
  if (mpi_cmp (ref_mpi, ciphertext))
    {
      errtxt = "ciphertext doesn't match reference data";
      goto leave;
    }

  /* Decrypt.  */
  err = _gcry_pk_decrypt (&decr, encr, skey);
  if (err)
    {
      errtxt = "decrypt failed";
      goto leave;
    }

  /* Extract the decrypted data from the S-expression.  Note that the
     output of gcry_pk_decrypt depends on whether a flags lists occurs
     in its input data.  Because we passed the output of
     gcry_pk_encrypt directly to gcry_pk_decrypt, such a flag value
     won't be there as of today.  To be prepared for future changes we
     take care of it anyway.  */
  tmplist = sexp_find_token (decr, "value", 0);
  if (tmplist)
    decr_plaintext = sexp_nth_string (tmplist, 1);
  else
    decr_plaintext = sexp_nth_string (decr, 0);
  if (!decr_plaintext)
    {
      errtxt = "decrypt returned no plaintext";
      goto leave;
    }

  /* Check that the decrypted plaintext matches the original  plaintext.  */
  if (strcmp (plaintext, decr_plaintext))
    {
      errtxt = "mismatch";
      goto leave;
    }

 leave:
  sexp_release (tmplist);
  xfree (decr_plaintext);
  sexp_release (decr);
  _gcry_mpi_release (ciphertext);
  _gcry_mpi_release (ref_mpi);
  sexp_release (encr);
  sexp_release (plain);
  return errtxt;
}


static gpg_err_code_t
selftests_rsa (selftest_report_func_t report, int extended)
{
  const char *what;
  const char *errtxt;
  gcry_error_t err;
  gcry_sexp_t skey = NULL;
  gcry_sexp_t pkey = NULL;

  /* Convert the S-expressions into the internal representation.  */
  what = "convert";
  err = sexp_sscan (&skey, NULL, sample_secret_key, strlen (sample_secret_key));
  if (!err)
    err = sexp_sscan (&pkey, NULL,
                      sample_public_key, strlen (sample_public_key));
  if (err)
    {
      errtxt = _gcry_strerror (err);
      goto failed;
    }

  what = "key consistency";
  err = _gcry_pk_testkey (skey);
  if (err)
    {
      errtxt = _gcry_strerror (err);
      goto failed;
    }

  if (extended)
    {
      what = "sign";
      errtxt = selftest_sign_2048 (pkey, skey);
      if (errtxt)
        goto failed;
    }

  what = "digest sign";
  errtxt = selftest_hash_sign_2048 (pkey, skey);
  if (errtxt)
    goto failed;

  what = "encrypt";
  errtxt = selftest_encr_2048 (pkey, skey);
  if (errtxt)
    goto failed;

  sexp_release (pkey);
  sexp_release (skey);
  return 0; /* Succeeded. */

 failed:
  sexp_release (pkey);
  sexp_release (skey);
  if (report)
    report ("pubkey", GCRY_PK_RSA, what, errtxt);
  return GPG_ERR_SELFTEST_FAILED;
}


/* Run a full self-test for ALGO and return 0 on success.  */
static gpg_err_code_t
run_selftests (int algo, int extended, selftest_report_func_t report)
{
  gpg_err_code_t ec;

  switch (algo)
    {
    case GCRY_PK_RSA:
      ec = selftests_rsa (report, extended);
      break;
    default:
      ec = GPG_ERR_PUBKEY_ALGO;
      break;

    }
  return ec;
}




gcry_pk_spec_t _gcry_pubkey_spec_rsa =
  {
    GCRY_PK_RSA, { 0, 1 },
    (GCRY_PK_USAGE_SIGN | GCRY_PK_USAGE_ENCR),
    "RSA", rsa_names,
    "ne", "nedpqu", "a", "s", "n",
    rsa_generate,
    rsa_check_secret_key,
    rsa_encrypt,
    rsa_decrypt,
    rsa_sign,
    rsa_verify,
    rsa_get_nbits,
    run_selftests,
    compute_keygrip
  };
