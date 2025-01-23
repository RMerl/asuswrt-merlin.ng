/* Elgamal.c  -  Elgamal Public Key encryption
 * Copyright (C) 1998, 2000, 2001, 2002, 2003,
 *               2008  Free Software Foundation, Inc.
 * Copyright (C) 2013 g10 Code GmbH
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
 *
 * For a description of the algorithm, see:
 *   Bruce Schneier: Applied Cryptography. John Wiley & Sons, 1996.
 *   ISBN 0-471-11709-9. Pages 476 ff.
 */

#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "g10lib.h"
#include "mpi.h"
#include "cipher.h"
#include "pubkey-internal.h"


/* Blinding is used to mitigate side-channel attacks.  You may undef
   this to speed up the operation in case the system is secured
   against physical and network mounted side-channel attacks.  */
#define USE_BLINDING 1


typedef struct
{
  gcry_mpi_t p;	    /* prime */
  gcry_mpi_t g;	    /* group generator */
  gcry_mpi_t y;	    /* g^x mod p */
} ELG_public_key;


typedef struct
{
  gcry_mpi_t p;	    /* prime */
  gcry_mpi_t g;	    /* group generator */
  gcry_mpi_t y;	    /* g^x mod p */
  gcry_mpi_t x;	    /* secret exponent */
} ELG_secret_key;


static const char *elg_names[] =
  {
    "elg",
    "openpgp-elg",
    "openpgp-elg-sig",
    NULL,
  };


static int test_keys (ELG_secret_key *sk, unsigned int nbits, int nodie);
static gcry_mpi_t gen_k (gcry_mpi_t p);
static gcry_err_code_t generate (ELG_secret_key *sk, unsigned nbits,
                                 gcry_mpi_t **factors);
static int  check_secret_key (ELG_secret_key *sk);
static void do_encrypt (gcry_mpi_t a, gcry_mpi_t b, gcry_mpi_t input,
                        ELG_public_key *pkey);
static void decrypt (gcry_mpi_t output, gcry_mpi_t a, gcry_mpi_t b,
                     ELG_secret_key *skey);
static void sign (gcry_mpi_t a, gcry_mpi_t b, gcry_mpi_t input,
                  ELG_secret_key *skey);
static int  verify (gcry_mpi_t a, gcry_mpi_t b, gcry_mpi_t input,
                    ELG_public_key *pkey);
static unsigned int elg_get_nbits (gcry_sexp_t parms);


static void (*progress_cb) (void *, const char *, int, int, int);
static void *progress_cb_data;

void
_gcry_register_pk_elg_progress (void (*cb) (void *, const char *,
                                            int, int, int),
				void *cb_data)
{
  progress_cb = cb;
  progress_cb_data = cb_data;
}


static void
progress (int c)
{
  if (progress_cb)
    progress_cb (progress_cb_data, "pk_elg", c, 0, 0);
}


/****************
 * Michael Wiener's table on subgroup sizes to match field sizes.
 * (floating around somewhere, probably based on the paper from
 * Eurocrypt 96, page 332)
 */
static unsigned int
wiener_map( unsigned int n )
{
  static struct { unsigned int p_n, q_n; } t[] =
    { /*   p	  q	 attack cost */
      {  512, 119 },	/* 9 x 10^17 */
      {  768, 145 },	/* 6 x 10^21 */
      { 1024, 165 },	/* 7 x 10^24 */
      { 1280, 183 },	/* 3 x 10^27 */
      { 1536, 198 },	/* 7 x 10^29 */
      { 1792, 212 },	/* 9 x 10^31 */
      { 2048, 225 },	/* 8 x 10^33 */
      { 2304, 237 },	/* 5 x 10^35 */
      { 2560, 249 },	/* 3 x 10^37 */
      { 2816, 259 },	/* 1 x 10^39 */
      { 3072, 269 },	/* 3 x 10^40 */
      { 3328, 279 },	/* 8 x 10^41 */
      { 3584, 288 },	/* 2 x 10^43 */
      { 3840, 296 },	/* 4 x 10^44 */
      { 4096, 305 },	/* 7 x 10^45 */
      { 4352, 313 },	/* 1 x 10^47 */
      { 4608, 320 },	/* 2 x 10^48 */
      { 4864, 328 },	/* 2 x 10^49 */
      { 5120, 335 },	/* 3 x 10^50 */
      { 0, 0 }
    };
  int i;

  for(i=0; t[i].p_n; i++ )
    {
      if( n <= t[i].p_n )
        return t[i].q_n;
    }
  /* Not in table - use an arbitrary high number. */
  return  n / 8 + 200;
}

static int
test_keys ( ELG_secret_key *sk, unsigned int nbits, int nodie )
{
  ELG_public_key pk;
  gcry_mpi_t test   = mpi_new ( 0 );
  gcry_mpi_t out1_a = mpi_new ( nbits );
  gcry_mpi_t out1_b = mpi_new ( nbits );
  gcry_mpi_t out2   = mpi_new ( nbits );
  int failed = 0;

  pk.p = sk->p;
  pk.g = sk->g;
  pk.y = sk->y;

  _gcry_mpi_randomize ( test, nbits, GCRY_WEAK_RANDOM );

  do_encrypt ( out1_a, out1_b, test, &pk );
  decrypt ( out2, out1_a, out1_b, sk );
  if ( mpi_cmp( test, out2 ) )
    failed |= 1;

  sign ( out1_a, out1_b, test, sk );
  if ( !verify( out1_a, out1_b, test, &pk ) )
    failed |= 2;

  _gcry_mpi_release ( test );
  _gcry_mpi_release ( out1_a );
  _gcry_mpi_release ( out1_b );
  _gcry_mpi_release ( out2 );

  if (failed && !nodie)
    log_fatal ("Elgamal test key for %s %s failed\n",
               (failed & 1)? "encrypt+decrypt":"",
               (failed & 2)? "sign+verify":"");
  if (failed && DBG_CIPHER)
    log_debug ("Elgamal test key for %s %s failed\n",
               (failed & 1)? "encrypt+decrypt":"",
               (failed & 2)? "sign+verify":"");

  return failed;
}


/****************
 * Generate a random secret exponent k from prime p, so that k is
 * relatively prime to p-1.
 */
static gcry_mpi_t
gen_k( gcry_mpi_t p )
{
  gcry_mpi_t k = mpi_alloc_secure( 0 );
  gcry_mpi_t temp = mpi_alloc( mpi_get_nlimbs(p) );
  gcry_mpi_t p_1 = mpi_copy(p);
  unsigned int orig_nbits = mpi_get_nbits(p);
  unsigned int nbits, nbytes;
  char *rndbuf = NULL;

  nbits = orig_nbits;

  nbytes = (nbits+7)/8;
  if( DBG_CIPHER )
    log_debug("choosing a random k\n");
  mpi_sub_ui( p_1, p, 1);
  for(;;)
    {
      if( !rndbuf || nbits < 32 )
        {
          xfree(rndbuf);
          rndbuf = _gcry_random_bytes_secure( nbytes, GCRY_STRONG_RANDOM );
        }
      else
        {
          /* Change only some of the higher bits.  We could improve
             this by directly requesting more memory at the first call
             to get_random_bytes() and use this the here maybe it is
             easier to do this directly in random.c Anyway, it is
             highly inlikely that we will ever reach this code. */
          char *pp = _gcry_random_bytes_secure( 4, GCRY_STRONG_RANDOM );
          memcpy( rndbuf, pp, 4 );
          xfree(pp);
	}
      _gcry_mpi_set_buffer( k, rndbuf, nbytes, 0 );

      for(;;)
        {
          if( !(mpi_cmp( k, p_1 ) < 0) )  /* check: k < (p-1) */
            {
              if( DBG_CIPHER )
                progress('+');
              break; /* no  */
            }
          if( !(mpi_cmp_ui( k, 0 ) > 0) )  /* check: k > 0 */
            {
              if( DBG_CIPHER )
                progress('-');
              break; /* no */
            }
          if (mpi_gcd( temp, k, p_1 ))
            goto found;  /* okay, k is relative prime to (p-1) */
          mpi_add_ui( k, k, 1 );
          if( DBG_CIPHER )
            progress('.');
	}
    }
 found:
  xfree (rndbuf);
  if( DBG_CIPHER )
    progress('\n');
  mpi_free(p_1);
  mpi_free(temp);

  return k;
}

/****************
 * Generate a key pair with a key of size NBITS
 * Returns: 2 structures filled with all needed values
 *	    and an array with n-1 factors of (p-1)
 */
static gcry_err_code_t
generate ( ELG_secret_key *sk, unsigned int nbits, gcry_mpi_t **ret_factors )
{
  gcry_err_code_t rc;
  gcry_mpi_t p;    /* the prime */
  gcry_mpi_t p_min1;
  gcry_mpi_t g;
  gcry_mpi_t x;    /* the secret exponent */
  gcry_mpi_t y;
  unsigned int qbits;
  unsigned int xbits;
  byte *rndbuf;

  p_min1 = mpi_new ( nbits );
  qbits = wiener_map( nbits );
  if( qbits & 1 ) /* better have a even one */
    qbits++;
  g = mpi_alloc(1);
  rc = _gcry_generate_elg_prime (0, nbits, qbits, g, &p, ret_factors);
  if (rc)
    {
      mpi_free (p_min1);
      mpi_free (g);
      return rc;
    }
  mpi_sub_ui(p_min1, p, 1);


  /* Select a random number which has these properties:
   *	 0 < x < p-1
   * This must be a very good random number because this is the
   * secret part.  The prime is public and may be shared anyway,
   * so a random generator level of 1 is used for the prime.
   *
   * I don't see a reason to have a x of about the same size
   * as the p.  It should be sufficient to have one about the size
   * of q or the later used k plus a large safety margin. Decryption
   * will be much faster with such an x.
   */
  xbits = qbits * 3 / 2;
  if( xbits >= nbits )
    BUG();
  x = mpi_snew ( xbits );
  if( DBG_CIPHER )
    log_debug("choosing a random x of size %u\n", xbits );
  rndbuf = NULL;
  do
    {
      if( DBG_CIPHER )
        progress('.');
      if( rndbuf )
        { /* Change only some of the higher bits */
          if( xbits < 16 ) /* should never happen ... */
            {
              xfree(rndbuf);
              rndbuf = _gcry_random_bytes_secure ((xbits+7)/8,
                                                  GCRY_VERY_STRONG_RANDOM);
            }
          else
            {
              char *r = _gcry_random_bytes_secure (2, GCRY_VERY_STRONG_RANDOM);
              memcpy(rndbuf, r, 2 );
              xfree (r);
            }
	}
      else
        {
          rndbuf = _gcry_random_bytes_secure ((xbits+7)/8,
                                              GCRY_VERY_STRONG_RANDOM );
	}
      _gcry_mpi_set_buffer( x, rndbuf, (xbits+7)/8, 0 );
      mpi_clear_highbit( x, xbits+1 );
    }
  while( !( mpi_cmp_ui( x, 0 )>0 && mpi_cmp( x, p_min1 )<0 ) );
  xfree(rndbuf);

  y = mpi_new (nbits);
  mpi_powm( y, g, x, p );

  if( DBG_CIPHER )
    {
      progress ('\n');
      log_mpidump ("elg  p", p );
      log_mpidump ("elg  g", g );
      log_mpidump ("elg  y", y );
      log_mpidump ("elg  x", x );
    }

  /* Copy the stuff to the key structures */
  sk->p = p;
  sk->g = g;
  sk->y = y;
  sk->x = x;

  _gcry_mpi_release ( p_min1 );

  /* Now we can test our keys (this should never fail!) */
  test_keys ( sk, nbits - 64, 0 );

  return 0;
}


/* Generate a key pair with a key of size NBITS not using a random
   value for the secret key but the one given as X.  This is useful to
   implement a passphrase based decryption for a public key based
   encryption.  It has appliactions in backup systems.

   Returns: A structure filled with all needed values and an array
 	    with n-1 factors of (p-1).  */
static gcry_err_code_t
generate_using_x (ELG_secret_key *sk, unsigned int nbits, gcry_mpi_t x,
                  gcry_mpi_t **ret_factors )
{
  gcry_err_code_t rc;
  gcry_mpi_t p;      /* The prime.  */
  gcry_mpi_t p_min1; /* The prime minus 1.  */
  gcry_mpi_t g;      /* The generator.  */
  gcry_mpi_t y;      /* g^x mod p.  */
  unsigned int qbits;
  unsigned int xbits;

  sk->p = NULL;
  sk->g = NULL;
  sk->y = NULL;
  sk->x = NULL;

  /* Do a quick check to see whether X is suitable.  */
  xbits = mpi_get_nbits (x);
  if ( xbits < 64 || xbits >= nbits )
    return GPG_ERR_INV_VALUE;

  p_min1 = mpi_new ( nbits );
  qbits  = wiener_map ( nbits );
  if ( (qbits & 1) ) /* Better have an even one.  */
    qbits++;
  g = mpi_alloc (1);
  rc = _gcry_generate_elg_prime (0, nbits, qbits, g, &p, ret_factors );
  if (rc)
    {
      mpi_free (p_min1);
      mpi_free (g);
      return rc;
    }
  mpi_sub_ui (p_min1, p, 1);

  if (DBG_CIPHER)
    log_debug ("using a supplied x of size %u", xbits );
  if ( !(mpi_cmp_ui ( x, 0 ) > 0 && mpi_cmp ( x, p_min1 ) <0 ) )
    {
      _gcry_mpi_release ( p_min1 );
      _gcry_mpi_release ( p );
      _gcry_mpi_release ( g );
      return GPG_ERR_INV_VALUE;
    }

  y = mpi_new (nbits);
  mpi_powm ( y, g, x, p );

  if ( DBG_CIPHER )
    {
      progress ('\n');
      log_mpidump ("elg  p", p );
      log_mpidump ("elg  g", g );
      log_mpidump ("elg  y", y );
      log_mpidump ("elg  x", x );
    }

  /* Copy the stuff to the key structures */
  sk->p = p;
  sk->g = g;
  sk->y = y;
  sk->x = mpi_copy (x);

  _gcry_mpi_release ( p_min1 );

  /* Now we can test our keys. */
  if ( test_keys ( sk, nbits - 64, 1 ) )
    {
      _gcry_mpi_release ( sk->p ); sk->p = NULL;
      _gcry_mpi_release ( sk->g ); sk->g = NULL;
      _gcry_mpi_release ( sk->y ); sk->y = NULL;
      _gcry_mpi_release ( sk->x ); sk->x = NULL;
      return GPG_ERR_BAD_SECKEY;
    }

  return 0;
}


/****************
 * Test whether the secret key is valid.
 * Returns: if this is a valid key.
 */
static int
check_secret_key( ELG_secret_key *sk )
{
  int rc;
  gcry_mpi_t y = mpi_alloc( mpi_get_nlimbs(sk->y) );

  mpi_powm (y, sk->g, sk->x, sk->p);
  rc = !mpi_cmp( y, sk->y );
  mpi_free( y );
  return rc;
}


static void
do_encrypt(gcry_mpi_t a, gcry_mpi_t b, gcry_mpi_t input, ELG_public_key *pkey )
{
  gcry_mpi_t k;

  /* Note: maybe we should change the interface, so that it
   * is possible to check that input is < p and return an
   * error code.
   */

  k = gen_k( pkey->p );
  mpi_powm (a, pkey->g, k, pkey->p);

  /* b = (y^k * input) mod p
   *	 = ((y^k mod p) * (input mod p)) mod p
   * and because input is < p
   *	 = ((y^k mod p) * input) mod p
   */
  mpi_powm (b, pkey->y, k, pkey->p);
  mpi_mulm (b, b, input, pkey->p);
#if 0
  if( DBG_CIPHER )
    {
      log_mpidump("elg encrypted y", pkey->y);
      log_mpidump("elg encrypted p", pkey->p);
      log_mpidump("elg encrypted k", k);
      log_mpidump("elg encrypted M", input);
      log_mpidump("elg encrypted a", a);
      log_mpidump("elg encrypted b", b);
    }
#endif
  mpi_free(k);
}




static void
decrypt (gcry_mpi_t output, gcry_mpi_t a, gcry_mpi_t b, ELG_secret_key *skey )
{
  gcry_mpi_t t1, t2, r, r1, h;
  unsigned int nbits = mpi_get_nbits (skey->p);
  gcry_mpi_t x_blind;

  mpi_normalize (a);
  mpi_normalize (b);

  t1 = mpi_snew (nbits);

#ifdef USE_BLINDING

  t2 = mpi_snew (nbits);
  r  = mpi_new (nbits);
  r1 = mpi_new (nbits);
  h  = mpi_new (nbits);
  x_blind = mpi_snew (nbits);

  /* We need a random number of about the prime size.  The random
     number merely needs to be unpredictable; thus we use level 0.  */
  _gcry_mpi_randomize (r, nbits, GCRY_WEAK_RANDOM);

  /* Also, exponent blinding: x_blind = x + (p-1)*r1 */
  _gcry_mpi_randomize (r1, nbits, GCRY_WEAK_RANDOM);
  mpi_set_highbit (r1, nbits - 1);
  mpi_sub_ui (h, skey->p, 1);
  mpi_mul (x_blind, h, r1);
  mpi_add (x_blind, skey->x, x_blind);

  /* t1 = r^x mod p */
  mpi_powm (t1, r, x_blind, skey->p);
  /* t2 = (a * r)^-x mod p */
  mpi_mulm (t2, a, r, skey->p);
  mpi_powm (t2, t2, x_blind, skey->p);
  mpi_invm (t2, t2, skey->p);
  /* t1 = (t1 * t2) mod p*/
  mpi_mulm (t1, t1, t2, skey->p);

  mpi_free (x_blind);
  mpi_free (h);
  mpi_free (r1);
  mpi_free (r);
  mpi_free (t2);

#else /*!USE_BLINDING*/

  /* output = b/(a^x) mod p */
  mpi_powm (t1, a, skey->x, skey->p);
  mpi_invm (t1, t1, skey->p);

#endif /*!USE_BLINDING*/

  mpi_mulm (output, b, t1, skey->p);

#if 0
  if( DBG_CIPHER )
    {
      log_mpidump ("elg decrypted x", skey->x);
      log_mpidump ("elg decrypted p", skey->p);
      log_mpidump ("elg decrypted a", a);
      log_mpidump ("elg decrypted b", b);
      log_mpidump ("elg decrypted M", output);
    }
#endif
  mpi_free (t1);
}


/****************
 * Make an Elgamal signature out of INPUT
 */

static void
sign(gcry_mpi_t a, gcry_mpi_t b, gcry_mpi_t input, ELG_secret_key *skey )
{
    gcry_mpi_t k;
    gcry_mpi_t t   = mpi_alloc( mpi_get_nlimbs(a) );
    gcry_mpi_t inv = mpi_alloc( mpi_get_nlimbs(a) );
    gcry_mpi_t p_1 = mpi_copy(skey->p);

   /*
    * b = (t * inv) mod (p-1)
    * b = (t * inv(k,(p-1),(p-1)) mod (p-1)
    * b = (((M-x*a) mod (p-1)) * inv(k,(p-1),(p-1))) mod (p-1)
    *
    */
    mpi_sub_ui(p_1, p_1, 1);
    k = gen_k( skey->p );
    mpi_powm( a, skey->g, k, skey->p );
    mpi_mul(t, skey->x, a );
    mpi_subm(t, input, t, p_1 );
    mpi_invm(inv, k, p_1 );
    mpi_mulm(b, t, inv, p_1 );

#if 0
    if( DBG_CIPHER )
      {
	log_mpidump ("elg sign p", skey->p);
	log_mpidump ("elg sign g", skey->g);
	log_mpidump ("elg sign y", skey->y);
	log_mpidump ("elg sign x", skey->x);
	log_mpidump ("elg sign k", k);
	log_mpidump ("elg sign M", input);
	log_mpidump ("elg sign a", a);
	log_mpidump ("elg sign b", b);
      }
#endif
    mpi_free(k);
    mpi_free(t);
    mpi_free(inv);
    mpi_free(p_1);
}


/****************
 * Returns true if the signature composed of A and B is valid.
 */
static int
verify(gcry_mpi_t a, gcry_mpi_t b, gcry_mpi_t input, ELG_public_key *pkey )
{
  int rc;
  gcry_mpi_t t1;
  gcry_mpi_t t2;
  gcry_mpi_t base[4];
  gcry_mpi_t ex[4];

  if( !(mpi_cmp_ui( a, 0 ) > 0 && mpi_cmp( a, pkey->p ) < 0) )
    return 0; /* assertion	0 < a < p  failed */

  t1 = mpi_alloc( mpi_get_nlimbs(a) );
  t2 = mpi_alloc( mpi_get_nlimbs(a) );

#if 0
  /* t1 = (y^a mod p) * (a^b mod p) mod p */
  gcry_mpi_powm( t1, pkey->y, a, pkey->p );
  gcry_mpi_powm( t2, a, b, pkey->p );
  mpi_mulm( t1, t1, t2, pkey->p );

  /* t2 = g ^ input mod p */
  gcry_mpi_powm( t2, pkey->g, input, pkey->p );

  rc = !mpi_cmp( t1, t2 );
#elif 0
  /* t1 = (y^a mod p) * (a^b mod p) mod p */
  base[0] = pkey->y; ex[0] = a;
  base[1] = a;       ex[1] = b;
  base[2] = NULL;    ex[2] = NULL;
  mpi_mulpowm( t1, base, ex, pkey->p );

  /* t2 = g ^ input mod p */
  gcry_mpi_powm( t2, pkey->g, input, pkey->p );

  rc = !mpi_cmp( t1, t2 );
#else
  /* t1 = g ^ - input * y ^ a * a ^ b  mod p */
  mpi_invm(t2, pkey->g, pkey->p );
  base[0] = t2     ; ex[0] = input;
  base[1] = pkey->y; ex[1] = a;
  base[2] = a;       ex[2] = b;
  base[3] = NULL;    ex[3] = NULL;
  mpi_mulpowm( t1, base, ex, pkey->p );
  rc = !mpi_cmp_ui( t1, 1 );

#endif

  mpi_free(t1);
  mpi_free(t2);
  return rc;
}

/*********************************************
 **************  interface  ******************
 *********************************************/

static gpg_err_code_t
elg_generate (const gcry_sexp_t genparms, gcry_sexp_t *r_skey)
{
  gpg_err_code_t rc;
  unsigned int nbits;
  ELG_secret_key sk;
  gcry_mpi_t xvalue = NULL;
  gcry_sexp_t l1;
  gcry_mpi_t *factors = NULL;
  gcry_sexp_t misc_info = NULL;

  memset (&sk, 0, sizeof sk);

  rc = _gcry_pk_util_get_nbits (genparms, &nbits);
  if (rc)
    return rc;

  /* Parse the optional xvalue element. */
  l1 = sexp_find_token (genparms, "xvalue", 0);
  if (l1)
    {
      xvalue = sexp_nth_mpi (l1, 1, 0);
      sexp_release (l1);
      if (!xvalue)
        return GPG_ERR_BAD_MPI;
    }

  if (xvalue)
    {
      rc = generate_using_x (&sk, nbits, xvalue, &factors);
      mpi_free (xvalue);
    }
  else
    {
      rc = generate (&sk, nbits, &factors);
    }
  if (rc)
    goto leave;

  if (factors && factors[0])
    {
      int nfac;
      void **arg_list;
      char *buffer, *p;

      for (nfac = 0; factors[nfac]; nfac++)
        ;
      arg_list = xtrycalloc (nfac+1, sizeof *arg_list);
      if (!arg_list)
        {
          rc = gpg_err_code_from_syserror ();
          goto leave;
        }
      buffer = xtrymalloc (30 + nfac*2 + 2 + 1);
      if (!buffer)
        {
          rc = gpg_err_code_from_syserror ();
          xfree (arg_list);
          goto leave;
        }
      p = stpcpy (buffer, "(misc-key-info(pm1-factors");
      for(nfac = 0; factors[nfac]; nfac++)
        {
          p = stpcpy (p, "%m");
          arg_list[nfac] = factors + nfac;
        }
      p = stpcpy (p, "))");
      rc = sexp_build_array (&misc_info, NULL, buffer, arg_list);
      xfree (arg_list);
      xfree (buffer);
      if (rc)
        goto leave;
    }

  rc = sexp_build (r_skey, NULL,
                   "(key-data"
                   " (public-key"
                   "  (elg(p%m)(g%m)(y%m)))"
                   " (private-key"
                   "  (elg(p%m)(g%m)(y%m)(x%m)))"
                   " %S)",
                   sk.p, sk.g, sk.y,
                   sk.p, sk.g, sk.y, sk.x,
                   misc_info);

 leave:
  mpi_free (sk.p);
  mpi_free (sk.g);
  mpi_free (sk.y);
  mpi_free (sk.x);
  sexp_release (misc_info);
  if (factors)
    {
      gcry_mpi_t *mp;
      for (mp = factors; *mp; mp++)
        mpi_free (*mp);
      xfree (factors);
    }

  return rc;
}


static gcry_err_code_t
elg_check_secret_key (gcry_sexp_t keyparms)
{
  gcry_err_code_t rc;
  ELG_secret_key sk = {NULL, NULL, NULL, NULL};

  rc = sexp_extract_param (keyparms, NULL, "pgyx",
                           &sk.p, &sk.g, &sk.y, &sk.x,
                           NULL);
  if (rc)
    goto leave;

  if (!check_secret_key (&sk))
    rc = GPG_ERR_BAD_SECKEY;

 leave:
  _gcry_mpi_release (sk.p);
  _gcry_mpi_release (sk.g);
  _gcry_mpi_release (sk.y);
  _gcry_mpi_release (sk.x);
  if (DBG_CIPHER)
    log_debug ("elg_testkey    => %s\n", gpg_strerror (rc));
  return rc;
}


static gcry_err_code_t
elg_encrypt (gcry_sexp_t *r_ciph, gcry_sexp_t s_data, gcry_sexp_t keyparms)
{
  gcry_err_code_t rc;
  struct pk_encoding_ctx ctx;
  gcry_mpi_t mpi_a = NULL;
  gcry_mpi_t mpi_b = NULL;
  gcry_mpi_t data = NULL;
  ELG_public_key pk = { NULL, NULL, NULL };

  _gcry_pk_util_init_encoding_ctx (&ctx, PUBKEY_OP_ENCRYPT,
                                   elg_get_nbits (keyparms));

  /* Extract the data.  */
  rc = _gcry_pk_util_data_to_mpi (s_data, &data, &ctx);
  if (rc)
    goto leave;
  if (DBG_CIPHER)
    log_mpidump ("elg_encrypt data", data);
  if (mpi_is_opaque (data))
    {
      rc = GPG_ERR_INV_DATA;
      goto leave;
    }

  /* Extract the key.  */
  rc = sexp_extract_param (keyparms, NULL, "pgy",
                           &pk.p, &pk.g, &pk.y, NULL);
  if (rc)
    goto leave;
  if (DBG_CIPHER)
    {
      log_mpidump ("elg_encrypt  p", pk.p);
      log_mpidump ("elg_encrypt  g", pk.g);
      log_mpidump ("elg_encrypt  y", pk.y);
    }

  /* Do Elgamal computation and build result.  */
  mpi_a = mpi_new (0);
  mpi_b = mpi_new (0);
  do_encrypt (mpi_a, mpi_b, data, &pk);
  rc = sexp_build (r_ciph, NULL, "(enc-val(elg(a%m)(b%m)))", mpi_a, mpi_b);

 leave:
  _gcry_mpi_release (mpi_a);
  _gcry_mpi_release (mpi_b);
  _gcry_mpi_release (pk.p);
  _gcry_mpi_release (pk.g);
  _gcry_mpi_release (pk.y);
  _gcry_mpi_release (data);
  _gcry_pk_util_free_encoding_ctx (&ctx);
  if (DBG_CIPHER)
    log_debug ("elg_encrypt   => %s\n", gpg_strerror (rc));
  return rc;
}


static gcry_err_code_t
elg_decrypt (gcry_sexp_t *r_plain, gcry_sexp_t s_data, gcry_sexp_t keyparms)
{
  gpg_err_code_t rc;
  struct pk_encoding_ctx ctx;
  gcry_sexp_t l1 = NULL;
  gcry_mpi_t data_a = NULL;
  gcry_mpi_t data_b = NULL;
  ELG_secret_key sk = {NULL, NULL, NULL, NULL};
  gcry_mpi_t plain = NULL;
  unsigned char *unpad = NULL;
  size_t unpadlen = 0;

  _gcry_pk_util_init_encoding_ctx (&ctx, PUBKEY_OP_DECRYPT,
                                   elg_get_nbits (keyparms));

  /* Extract the data.  */
  rc = _gcry_pk_util_preparse_encval (s_data, elg_names, &l1, &ctx);
  if (rc)
    goto leave;
  rc = sexp_extract_param (l1, NULL, "ab", &data_a, &data_b, NULL);
  if (rc)
    goto leave;
  if (DBG_CIPHER)
    {
      log_printmpi ("elg_decrypt  d_a", data_a);
      log_printmpi ("elg_decrypt  d_b", data_b);
    }
  if (mpi_is_opaque (data_a) || mpi_is_opaque (data_b))
    {
      rc = GPG_ERR_INV_DATA;
      goto leave;
    }

  /* Extract the key.  */
  rc = sexp_extract_param (keyparms, NULL, "pgyx",
                           &sk.p, &sk.g, &sk.y, &sk.x,
                           NULL);
  if (rc)
    goto leave;
  if (DBG_CIPHER)
    {
      log_printmpi ("elg_decrypt    p", sk.p);
      log_printmpi ("elg_decrypt    g", sk.g);
      log_printmpi ("elg_decrypt    y", sk.y);
      if (!fips_mode ())
        log_printmpi ("elg_decrypt    x", sk.x);
    }

  plain = mpi_snew (ctx.nbits);
  decrypt (plain, data_a, data_b, &sk);
  if (DBG_CIPHER)
    log_printmpi ("elg_decrypt  res", plain);

  /* Reverse the encoding and build the s-expression.  */
  switch (ctx.encoding)
    {
    case PUBKEY_ENC_PKCS1:
      rc = _gcry_rsa_pkcs1_decode_for_enc (&unpad, &unpadlen, ctx.nbits, plain);
      mpi_free (plain); plain = NULL;
      if (!rc)
        rc = sexp_build (r_plain, NULL, "(value %b)", (int)unpadlen, unpad);
      break;

    case PUBKEY_ENC_OAEP:
      rc = _gcry_rsa_oaep_decode (&unpad, &unpadlen,
                                  ctx.nbits, ctx.hash_algo, plain,
                                  ctx.label, ctx.labellen);
      mpi_free (plain); plain = NULL;
      if (!rc)
        rc = sexp_build (r_plain, NULL, "(value %b)", (int)unpadlen, unpad);
      break;

    default:
      /* Raw format.  For backward compatibility we need to assume a
         signed mpi by using the sexp format string "%m".  */
      rc = sexp_build (r_plain, NULL,
                       (ctx.flags & PUBKEY_FLAG_LEGACYRESULT)
                       ? "%m" : "(value %m)",
                       plain);
      break;
    }


 leave:
  xfree (unpad);
  _gcry_mpi_release (plain);
  _gcry_mpi_release (sk.p);
  _gcry_mpi_release (sk.g);
  _gcry_mpi_release (sk.y);
  _gcry_mpi_release (sk.x);
  _gcry_mpi_release (data_a);
  _gcry_mpi_release (data_b);
  sexp_release (l1);
  _gcry_pk_util_free_encoding_ctx (&ctx);
  if (DBG_CIPHER)
    log_debug ("elg_decrypt    => %s\n", gpg_strerror (rc));
  return rc;
}


static gcry_err_code_t
elg_sign (gcry_sexp_t *r_sig, gcry_sexp_t s_data, gcry_sexp_t keyparms)
{
  gcry_err_code_t rc;
  struct pk_encoding_ctx ctx;
  gcry_mpi_t data = NULL;
  ELG_secret_key sk = {NULL, NULL, NULL, NULL};
  gcry_mpi_t sig_r = NULL;
  gcry_mpi_t sig_s = NULL;

  _gcry_pk_util_init_encoding_ctx (&ctx, PUBKEY_OP_SIGN,
                                   elg_get_nbits (keyparms));

  /* Extract the data.  */
  rc = _gcry_pk_util_data_to_mpi (s_data, &data, &ctx);
  if (rc)
    goto leave;
  if (DBG_CIPHER)
    log_mpidump ("elg_sign   data", data);
  if (mpi_is_opaque (data))
    {
      rc = GPG_ERR_INV_DATA;
      goto leave;
    }

  /* Extract the key.  */
  rc = sexp_extract_param (keyparms, NULL, "pgyx",
                           &sk.p, &sk.g, &sk.y, &sk.x, NULL);
  if (rc)
    goto leave;
  if (DBG_CIPHER)
    {
      log_mpidump ("elg_sign      p", sk.p);
      log_mpidump ("elg_sign      g", sk.g);
      log_mpidump ("elg_sign      y", sk.y);
      if (!fips_mode ())
        log_mpidump ("elg_sign      x", sk.x);
    }

  sig_r = mpi_new (0);
  sig_s = mpi_new (0);
  sign (sig_r, sig_s, data, &sk);
  if (DBG_CIPHER)
    {
      log_mpidump ("elg_sign  sig_r", sig_r);
      log_mpidump ("elg_sign  sig_s", sig_s);
    }
  rc = sexp_build (r_sig, NULL, "(sig-val(elg(r%M)(s%M)))", sig_r, sig_s);

 leave:
  _gcry_mpi_release (sig_r);
  _gcry_mpi_release (sig_s);
  _gcry_mpi_release (sk.p);
  _gcry_mpi_release (sk.g);
  _gcry_mpi_release (sk.y);
  _gcry_mpi_release (sk.x);
  _gcry_mpi_release (data);
  _gcry_pk_util_free_encoding_ctx (&ctx);
  if (DBG_CIPHER)
    log_debug ("elg_sign      => %s\n", gpg_strerror (rc));
  return rc;
}


static gcry_err_code_t
elg_verify (gcry_sexp_t s_sig, gcry_sexp_t s_data, gcry_sexp_t s_keyparms)
{
  gcry_err_code_t rc;
  struct pk_encoding_ctx ctx;
  gcry_sexp_t l1 = NULL;
  gcry_mpi_t sig_r = NULL;
  gcry_mpi_t sig_s = NULL;
  gcry_mpi_t data = NULL;
  ELG_public_key pk = { NULL, NULL, NULL };

  _gcry_pk_util_init_encoding_ctx (&ctx, PUBKEY_OP_VERIFY,
                                   elg_get_nbits (s_keyparms));

  /* Extract the data.  */
  rc = _gcry_pk_util_data_to_mpi (s_data, &data, &ctx);
  if (rc)
    goto leave;
  if (DBG_CIPHER)
    log_mpidump ("elg_verify data", data);
  if (mpi_is_opaque (data))
    {
      rc = GPG_ERR_INV_DATA;
      goto leave;
    }

  /* Extract the signature value.  */
  rc = _gcry_pk_util_preparse_sigval (s_sig, elg_names, &l1, NULL);
  if (rc)
    goto leave;
  rc = sexp_extract_param (l1, NULL, "rs", &sig_r, &sig_s, NULL);
  if (rc)
    goto leave;
  if (DBG_CIPHER)
    {
      log_mpidump ("elg_verify  s_r", sig_r);
      log_mpidump ("elg_verify  s_s", sig_s);
    }

  /* Extract the key.  */
  rc = sexp_extract_param (s_keyparms, NULL, "pgy",
                                 &pk.p, &pk.g, &pk.y, NULL);
  if (rc)
    goto leave;
  if (DBG_CIPHER)
    {
      log_mpidump ("elg_verify    p", pk.p);
      log_mpidump ("elg_verify    g", pk.g);
      log_mpidump ("elg_verify    y", pk.y);
    }

  /* Verify the signature.  */
  if (!verify (sig_r, sig_s, data, &pk))
    rc = GPG_ERR_BAD_SIGNATURE;

 leave:
  _gcry_mpi_release (pk.p);
  _gcry_mpi_release (pk.g);
  _gcry_mpi_release (pk.y);
  _gcry_mpi_release (data);
  _gcry_mpi_release (sig_r);
  _gcry_mpi_release (sig_s);
  sexp_release (l1);
  _gcry_pk_util_free_encoding_ctx (&ctx);
  if (DBG_CIPHER)
    log_debug ("elg_verify    => %s\n", rc?gpg_strerror (rc):"Good");
  return rc;
}


/* Return the number of bits for the key described by PARMS.  On error
 * 0 is returned.  The format of PARMS starts with the algorithm name;
 * for example:
 *
 *   (dsa
 *     (p <mpi>)
 *     (g <mpi>)
 *     (y <mpi>))
 *
 * More parameters may be given but we only need P here.
 */
static unsigned int
elg_get_nbits (gcry_sexp_t parms)
{
  gcry_sexp_t l1;
  gcry_mpi_t p;
  unsigned int nbits;

  l1 = sexp_find_token (parms, "p", 1);
  if (!l1)
    return 0; /* Parameter P not found.  */

  p= sexp_nth_mpi (l1, 1, GCRYMPI_FMT_USG);
  sexp_release (l1);
  nbits = p? mpi_get_nbits (p) : 0;
  _gcry_mpi_release (p);
  return nbits;
}



gcry_pk_spec_t _gcry_pubkey_spec_elg =
  {
    GCRY_PK_ELG, { 0, 0 },
    (GCRY_PK_USAGE_SIGN | GCRY_PK_USAGE_ENCR),
    "ELG", elg_names,
    "pgy", "pgyx", "ab", "rs", "pgy",
    elg_generate,
    elg_check_secret_key,
    elg_encrypt,
    elg_decrypt,
    elg_sign,
    elg_verify,
    elg_get_nbits,
  };
