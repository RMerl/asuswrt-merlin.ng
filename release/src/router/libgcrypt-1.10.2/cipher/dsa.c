/* dsa.c - DSA signature algorithm
 * Copyright (C) 1998, 2000, 2001, 2002, 2003,
 *               2006, 2008  Free Software Foundation, Inc.
 * Copyright (C) 2013 g10 Code GmbH.
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

#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "g10lib.h"
#include "mpi.h"
#include "cipher.h"
#include "pubkey-internal.h"


typedef struct
{
  gcry_mpi_t p;	    /* prime */
  gcry_mpi_t q;	    /* group order */
  gcry_mpi_t g;	    /* group generator */
  gcry_mpi_t y;	    /* g^x mod p */
} DSA_public_key;


typedef struct
{
  gcry_mpi_t p;	    /* prime */
  gcry_mpi_t q;	    /* group order */
  gcry_mpi_t g;	    /* group generator */
  gcry_mpi_t y;	    /* g^x mod p */
  gcry_mpi_t x;	    /* secret exponent */
} DSA_secret_key;


/* A structure used to hold domain parameters.  */
typedef struct
{
  gcry_mpi_t p;	    /* prime */
  gcry_mpi_t q;	    /* group order */
  gcry_mpi_t g;	    /* group generator */
} dsa_domain_t;


static const char *dsa_names[] =
  {
    "dsa",
    "openpgp-dsa",
    NULL,
  };


/* A sample 1024 bit DSA key used for the selftests.  Not anymore
 * used, kept only for reference.  */
#if 0
static const char sample_secret_key_1024[] =
"(private-key"
" (dsa"
"  (p #00AD7C0025BA1A15F775F3F2D673718391D00456978D347B33D7B49E7F32EDAB"
"      96273899DD8B2BB46CD6ECA263FAF04A28903503D59062A8865D2AE8ADFB5191"
"      CF36FFB562D0E2F5809801A1F675DAE59698A9E01EFE8D7DCFCA084F4C6F5A44"
"      44D499A06FFAEA5E8EF5E01F2FD20A7B7EF3F6968AFBA1FB8D91F1559D52D8777B#)"
"  (q #00EB7B5751D25EBBB7BD59D920315FD840E19AEBF9#)"
"  (g #1574363387FDFD1DDF38F4FBE135BB20C7EE4772FB94C337AF86EA8E49666503"
"      AE04B6BE81A2F8DD095311E0217ACA698A11E6C5D33CCDAE71498ED35D13991E"
"      B02F09AB40BD8F4C5ED8C75DA779D0AE104BC34C960B002377068AB4B5A1F984"
"      3FBA91F537F1B7CAC4D8DD6D89B0D863AF7025D549F9C765D2FC07EE208F8D15#)"
"  (y #64B11EF8871BE4AB572AA810D5D3CA11A6CDBC637A8014602C72960DB135BF46"
"      A1816A724C34F87330FC9E187C5D66897A04535CC2AC9164A7150ABFA8179827"
"      6E45831AB811EEE848EBB24D9F5F2883B6E5DDC4C659DEF944DCFD80BF4D0A20"
"      42CAA7DC289F0C5A9D155F02D3D551DB741A81695B74D4C8F477F9C7838EB0FB#)"
"  (x #11D54E4ADBD3034160F2CED4B7CD292A4EBF3EC0#)))";
/* A sample 1024 bit DSA key used for the selftests (public only).  */
static const char sample_public_key_1024[] =
"(public-key"
" (dsa"
"  (p #00AD7C0025BA1A15F775F3F2D673718391D00456978D347B33D7B49E7F32EDAB"
"      96273899DD8B2BB46CD6ECA263FAF04A28903503D59062A8865D2AE8ADFB5191"
"      CF36FFB562D0E2F5809801A1F675DAE59698A9E01EFE8D7DCFCA084F4C6F5A44"
"      44D499A06FFAEA5E8EF5E01F2FD20A7B7EF3F6968AFBA1FB8D91F1559D52D8777B#)"
"  (q #00EB7B5751D25EBBB7BD59D920315FD840E19AEBF9#)"
"  (g #1574363387FDFD1DDF38F4FBE135BB20C7EE4772FB94C337AF86EA8E49666503"
"      AE04B6BE81A2F8DD095311E0217ACA698A11E6C5D33CCDAE71498ED35D13991E"
"      B02F09AB40BD8F4C5ED8C75DA779D0AE104BC34C960B002377068AB4B5A1F984"
"      3FBA91F537F1B7CAC4D8DD6D89B0D863AF7025D549F9C765D2FC07EE208F8D15#)"
"  (y #64B11EF8871BE4AB572AA810D5D3CA11A6CDBC637A8014602C72960DB135BF46"
"      A1816A724C34F87330FC9E187C5D66897A04535CC2AC9164A7150ABFA8179827"
"      6E45831AB811EEE848EBB24D9F5F2883B6E5DDC4C659DEF944DCFD80BF4D0A20"
"      42CAA7DC289F0C5A9D155F02D3D551DB741A81695B74D4C8F477F9C7838EB0FB#)))";
#endif /*0*/

/* 2048 DSA key from RFC 6979 A.2.2 */
static const char sample_public_key_2048[] =
"(public-key"
" (dsa"
"  (p #9DB6FB5951B66BB6FE1E140F1D2CE5502374161FD6538DF1648218642F0B5C48C8F7A41AADFA187324B87674FA1822B00F1ECF8136943D7C55757264E5A1A44FFE012E9936E00C1D3E9310B01C7D179805D3058B2A9F4BB6F9716BFE6117C6B5B3CC4D9BE341104AD4A80AD6C94E005F4B993E14F091EB51743BF33050C38DE235567E1B34C3D6A5C0CEAA1A0F368213C3D19843D0B4B09DCB9FC72D39C8DE41F1BF14D4BB4563CA28371621CAD3324B6A2D392145BEBFAC748805236F5CA2FE92B871CD8F9C36D3292B5509CA8CAA77A2ADFC7BFD77DDA6F71125A7456FEA153E433256A2261C6A06ED3693797E7995FAD5AABBCFBE3EDA2741E375404AE25B#)"
"  (q #F2C3119374CE76C9356990B465374A17F23F9ED35089BD969F61C6DDE9998C1F#)"
"  (g #5C7FF6B06F8F143FE8288433493E4769C4D988ACE5BE25A0E24809670716C613D7B0CEE6932F8FAA7C44D2CB24523DA53FBE4F6EC3595892D1AA58C4328A06C46A15662E7EAA703A1DECF8BBB2D05DBE2EB956C142A338661D10461C0D135472085057F3494309FFA73C611F78B32ADBB5740C361C9F35BE90997DB2014E2EF5AA61782F52ABEB8BD6432C4DD097BC5423B285DAFB60DC364E8161F4A2A35ACA3A10B1C4D203CC76A470A33AFDCBDD92959859ABD8B56E1725252D78EAC66E71BA9AE3F1DD2487199874393CD4D832186800654760E1E34C09E4D155179F9EC0DC4473F996BDCE6EED1CABED8B6F116F7AD9CF505DF0F998E34AB27514B0FFE7#)"
"  (y #667098C654426C78D7F8201EAC6C203EF030D43605032C2F1FA937E5237DBD949F34A0A2564FE126DC8B715C5141802CE0979C8246463C40E6B6BDAA2513FA611728716C2E4FD53BC95B89E69949D96512E873B9C8F8DFD499CC312882561ADECB31F658E934C0C197F2C4D96B05CBAD67381E7B768891E4DA3843D24D94CDFB5126E9B8BF21E8358EE0E0A30EF13FD6A664C0DCE3731F7FB49A4845A4FD8254687972A2D382599C9BAC4E0ED7998193078913032558134976410B89D2C171D123AC35FD977219597AA7D15C1A9A428E59194F75C721EBCBCFAE44696A499AFA74E04299F132026601638CB87AB79190D4A0986315DA8EEC6561C938996BEADF#)))";

static const char sample_secret_key_2048[] =
"(private-key"
" (dsa"
"  (p #9DB6FB5951B66BB6FE1E140F1D2CE5502374161FD6538DF1648218642F0B5C48C8F7A41AADFA187324B87674FA1822B00F1ECF8136943D7C55757264E5A1A44FFE012E9936E00C1D3E9310B01C7D179805D3058B2A9F4BB6F9716BFE6117C6B5B3CC4D9BE341104AD4A80AD6C94E005F4B993E14F091EB51743BF33050C38DE235567E1B34C3D6A5C0CEAA1A0F368213C3D19843D0B4B09DCB9FC72D39C8DE41F1BF14D4BB4563CA28371621CAD3324B6A2D392145BEBFAC748805236F5CA2FE92B871CD8F9C36D3292B5509CA8CAA77A2ADFC7BFD77DDA6F71125A7456FEA153E433256A2261C6A06ED3693797E7995FAD5AABBCFBE3EDA2741E375404AE25B#)"
"  (q #F2C3119374CE76C9356990B465374A17F23F9ED35089BD969F61C6DDE9998C1F#)"
"  (g #5C7FF6B06F8F143FE8288433493E4769C4D988ACE5BE25A0E24809670716C613D7B0CEE6932F8FAA7C44D2CB24523DA53FBE4F6EC3595892D1AA58C4328A06C46A15662E7EAA703A1DECF8BBB2D05DBE2EB956C142A338661D10461C0D135472085057F3494309FFA73C611F78B32ADBB5740C361C9F35BE90997DB2014E2EF5AA61782F52ABEB8BD6432C4DD097BC5423B285DAFB60DC364E8161F4A2A35ACA3A10B1C4D203CC76A470A33AFDCBDD92959859ABD8B56E1725252D78EAC66E71BA9AE3F1DD2487199874393CD4D832186800654760E1E34C09E4D155179F9EC0DC4473F996BDCE6EED1CABED8B6F116F7AD9CF505DF0F998E34AB27514B0FFE7#)"
"  (y #667098C654426C78D7F8201EAC6C203EF030D43605032C2F1FA937E5237DBD949F34A0A2564FE126DC8B715C5141802CE0979C8246463C40E6B6BDAA2513FA611728716C2E4FD53BC95B89E69949D96512E873B9C8F8DFD499CC312882561ADECB31F658E934C0C197F2C4D96B05CBAD67381E7B768891E4DA3843D24D94CDFB5126E9B8BF21E8358EE0E0A30EF13FD6A664C0DCE3731F7FB49A4845A4FD8254687972A2D382599C9BAC4E0ED7998193078913032558134976410B89D2C171D123AC35FD977219597AA7D15C1A9A428E59194F75C721EBCBCFAE44696A499AFA74E04299F132026601638CB87AB79190D4A0986315DA8EEC6561C938996BEADF#)"
"  (x #69C7548C21D0DFEA6B9A51C9EAD4E27C33D3B3F180316E5BCAB92C933F0E4DBC#)))";



static int test_keys (DSA_secret_key *sk, unsigned int qbits);
static int check_secret_key (DSA_secret_key *sk);
static gpg_err_code_t generate (DSA_secret_key *sk,
                                unsigned int nbits,
                                unsigned int qbits,
                                int transient_key,
                                dsa_domain_t *domain,
                                gcry_mpi_t **ret_factors);
static gpg_err_code_t sign (gcry_mpi_t r, gcry_mpi_t s,
                            gcry_mpi_t input, gcry_mpi_t k,
                            DSA_secret_key *skey, int flags, int hashalgo);
static gpg_err_code_t verify (gcry_mpi_t r, gcry_mpi_t s, gcry_mpi_t input,
                              DSA_public_key *pkey, int flags, int hashalgo);
static unsigned int dsa_get_nbits (gcry_sexp_t parms);


static void (*progress_cb) (void *,const char *, int, int, int );
static void *progress_cb_data;


/* Check the DSA key length is acceptable for key generation or usage */
static gpg_err_code_t
dsa_check_keysize (unsigned int nbits)
{
  if (fips_mode () && nbits < 2048)
    return GPG_ERR_INV_VALUE;

  return 0;
}


void
_gcry_register_pk_dsa_progress (void (*cb) (void *, const char *,
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
    progress_cb (progress_cb_data, "pk_dsa", c, 0, 0);
}


/* Check that a freshly generated key actually works.  Returns 0 on success. */
static int
test_keys (DSA_secret_key *sk, unsigned int qbits)
{
  int result = -1;  /* Default to failure.  */
  DSA_public_key pk;
  gcry_mpi_t data  = mpi_new (qbits);
  gcry_mpi_t sig_a = mpi_new (qbits);
  gcry_mpi_t sig_b = mpi_new (qbits);

  /* Put the relevant parameters into a public key structure.  */
  pk.p = sk->p;
  pk.q = sk->q;
  pk.g = sk->g;
  pk.y = sk->y;

  /* Create a random plaintext.  */
  _gcry_mpi_randomize (data, qbits, GCRY_WEAK_RANDOM);

  /* Sign DATA using the secret key.  */
  sign (sig_a, sig_b, data, NULL, sk, 0, 0);

  /* Verify the signature using the public key.  */
  if ( verify (sig_a, sig_b, data, &pk, 0, 0) )
    goto leave; /* Signature does not match.  */

  /* Modify the data and check that the signing fails.  */
  mpi_add_ui (data, data, 1);
  if ( !verify (sig_a, sig_b, data, &pk, 0, 0) )
    goto leave; /* Signature matches but should not.  */

  result = 0; /* The test succeeded.  */

 leave:
  _gcry_mpi_release (sig_b);
  _gcry_mpi_release (sig_a);
  _gcry_mpi_release (data);
  return result;
}



/*
   Generate a DSA key pair with a key of size NBITS.  If transient_key
   is true the key is generated using the standard RNG and not the
   very secure one.

   Returns: 2 structures filled with all needed values
 	    and an array with the n-1 factors of (p-1)
 */
static gpg_err_code_t
generate (DSA_secret_key *sk, unsigned int nbits, unsigned int qbits,
          int transient_key, dsa_domain_t *domain, gcry_mpi_t **ret_factors )
{
  gpg_err_code_t rc;
  gcry_mpi_t p;    /* the prime */
  gcry_mpi_t q;    /* the 160 bit prime factor */
  gcry_mpi_t g;    /* the generator */
  gcry_mpi_t y;    /* g^x mod p */
  gcry_mpi_t x;    /* the secret exponent */
  gcry_mpi_t h, e;  /* helper */
  unsigned char *rndbuf;
  gcry_random_level_t random_level;

  if (qbits)
    ; /* Caller supplied qbits.  Use this value.  */
  else if ( nbits >= 512 && nbits <= 1024 )
    qbits = 160;
  else if ( nbits == 2048 )
    qbits = 224;
  else if ( nbits == 3072 )
    qbits = 256;
  else if ( nbits == 7680 )
    qbits = 384;
  else if ( nbits == 15360 )
    qbits = 512;
  else
    return GPG_ERR_INV_VALUE;

  if (qbits < 160 || qbits > 512 || (qbits%8) )
    return GPG_ERR_INV_VALUE;
  if (nbits < 2*qbits || nbits > 15360)
    return GPG_ERR_INV_VALUE;

  if (domain->p && domain->q && domain->g)
    {
      /* Domain parameters are given; use them.  */
      p = mpi_copy (domain->p);
      q = mpi_copy (domain->q);
      g = mpi_copy (domain->g);
      gcry_assert (mpi_get_nbits (p) == nbits);
      gcry_assert (mpi_get_nbits (q) == qbits);
      h = mpi_alloc (0);
      e = NULL;
    }
  else
    {
      /* Generate new domain parameters.  */
      rc = _gcry_generate_elg_prime (1, nbits, qbits, NULL, &p, ret_factors);
      if (rc)
        return rc;

      /* Get q out of factors.  */
      q = mpi_copy ((*ret_factors)[0]);
      gcry_assert (mpi_get_nbits (q) == qbits);

      /* Find a generator g (h and e are helpers).
         e = (p-1)/q */
      e = mpi_alloc (mpi_get_nlimbs (p));
      mpi_sub_ui (e, p, 1);
      mpi_fdiv_q (e, e, q);
      g = mpi_alloc (mpi_get_nlimbs (p));
      h = mpi_alloc_set_ui (1); /* (We start with 2.) */
      do
        {
          mpi_add_ui (h, h, 1);
          /* g = h^e mod p */
          mpi_powm (g, h, e, p);
        }
      while (!mpi_cmp_ui (g, 1));  /* Continue until g != 1. */
    }

  /* Select a random number X with the property:
   *	 0 < x < q-1
   *
   * FIXME: Why do we use the requirement x < q-1 ? It should be
   * sufficient to test for x < q.  FIPS-186-3 check x < q-1 but it
   * does not check for 0 < x because it makes sure that Q is unsigned
   * and finally adds one to the result so that 0 will never be
   * returned.  We should replace the code below with _gcry_dsa_gen_k.
   *
   * This must be a very good random number because this is the secret
   * part.  The random quality depends on the transient_key flag.  */
  random_level = transient_key ? GCRY_STRONG_RANDOM : GCRY_VERY_STRONG_RANDOM;
  if (DBG_CIPHER)
    log_debug("choosing a random x%s\n", transient_key? " (transient-key)":"");
  gcry_assert( qbits >= 160 );
  x = mpi_alloc_secure( mpi_get_nlimbs(q) );
  mpi_sub_ui( h, q, 1 );  /* put q-1 into h */
  rndbuf = NULL;
  do
    {
      if( DBG_CIPHER )
        progress('.');
      if( !rndbuf )
        rndbuf = _gcry_random_bytes_secure ((qbits+7)/8, random_level);
      else
        { /* Change only some of the higher bits (= 2 bytes)*/
          char *r = _gcry_random_bytes_secure (2, random_level);
          memcpy(rndbuf, r, 2 );
          xfree(r);
        }

      _gcry_mpi_set_buffer( x, rndbuf, (qbits+7)/8, 0 );
      mpi_clear_highbit( x, qbits+1 );
    }
  while ( !( mpi_cmp_ui( x, 0 )>0 && mpi_cmp( x, h )<0 ) );
  xfree(rndbuf);
  mpi_free( e );
  mpi_free( h );

  /* y = g^x mod p */
  y = mpi_alloc( mpi_get_nlimbs(p) );
  mpi_powm (y, g, x, p);

  if( DBG_CIPHER )
    {
      progress('\n');
      log_mpidump("dsa  p", p );
      log_mpidump("dsa  q", q );
      log_mpidump("dsa  g", g );
      log_mpidump("dsa  y", y );
      log_mpidump("dsa  x", x );
    }

  /* Copy the stuff to the key structures. */
  sk->p = p;
  sk->q = q;
  sk->g = g;
  sk->y = y;
  sk->x = x;

  /* Now we can test our keys (this should never fail!). */
  if ( test_keys (sk, qbits) )
    {
      _gcry_mpi_release (sk->p); sk->p = NULL;
      _gcry_mpi_release (sk->q); sk->q = NULL;
      _gcry_mpi_release (sk->g); sk->g = NULL;
      _gcry_mpi_release (sk->y); sk->y = NULL;
      _gcry_mpi_release (sk->x); sk->x = NULL;
      fips_signal_error ("self-test after key generation failed");
      return GPG_ERR_SELFTEST_FAILED;
    }
  return 0;
}


/* Generate a DSA key pair with a key of size NBITS using the
   algorithm given in FIPS-186-3.  If USE_FIPS186_2 is true,
   FIPS-186-2 is used and thus the length is restricted to 1024/160.
   If DERIVEPARMS is not NULL it may contain a seed value.  If domain
   parameters are specified in DOMAIN, DERIVEPARMS may not be given
   and NBITS and QBITS must match the specified domain parameters.  */
static gpg_err_code_t
generate_fips186 (DSA_secret_key *sk, unsigned int nbits, unsigned int qbits,
                  gcry_sexp_t deriveparms, int use_fips186_2,
                  dsa_domain_t *domain,
                  int *r_counter, void **r_seed, size_t *r_seedlen,
                  gcry_mpi_t *r_h)
{
  gpg_err_code_t ec;
  struct {
    gcry_sexp_t sexp;
    const void *seed;
    size_t seedlen;
  } initial_seed = { NULL, NULL, 0 };
  gcry_mpi_t prime_q = NULL;
  gcry_mpi_t prime_p = NULL;
  gcry_mpi_t value_g = NULL; /* The generator. */
  gcry_mpi_t value_y = NULL; /* g^x mod p */
  gcry_mpi_t value_x = NULL; /* The secret exponent. */
  gcry_mpi_t value_h = NULL; /* Helper.  */
  gcry_mpi_t value_e = NULL; /* Helper.  */
  gcry_mpi_t value_c = NULL; /* helper for x */
  gcry_mpi_t value_qm2 = NULL; /* q - 2 */

  /* Preset return values.  */
  *r_counter = 0;
  *r_seed = NULL;
  *r_seedlen = 0;
  *r_h = NULL;

  /* Derive QBITS from NBITS if requested  */
  if (!qbits)
    {
      if (nbits == 1024)
        qbits = 160;
      else if (nbits == 2048)
        qbits = 224;
      else if (nbits == 3072)
        qbits = 256;
    }

  /* Check that QBITS and NBITS match the standard.  Note that FIPS
     186-3 uses N for QBITS and L for NBITS.  */
  if (nbits == 1024 && qbits == 160 && use_fips186_2)
    ; /* Allowed in FIPS 186-2 mode.  */
  else if (nbits == 2048 && qbits == 224)
    ;
  else if (nbits == 2048 && qbits == 256)
    ;
  else if (nbits == 3072 && qbits == 256)
    ;
  else
    return GPG_ERR_INV_VALUE;

  ec = dsa_check_keysize (nbits);
  if (ec)
    return ec;

  if (domain->p && domain->q && domain->g)
    {
      /* Domain parameters are given; use them.  */
      prime_p = mpi_copy (domain->p);
      prime_q = mpi_copy (domain->q);
      value_g = mpi_copy (domain->g);
      gcry_assert (mpi_get_nbits (prime_p) == nbits);
      gcry_assert (mpi_get_nbits (prime_q) == qbits);
      gcry_assert (!deriveparms);
      ec = 0;
    }
  else
    {
      /* Generate new domain parameters.  */

      /* Get an initial seed value.  */
      if (deriveparms)
        {
          initial_seed.sexp = sexp_find_token (deriveparms, "seed", 0);
          if (initial_seed.sexp)
            initial_seed.seed = sexp_nth_data (initial_seed.sexp, 1,
                                               &initial_seed.seedlen);
        }

      if (use_fips186_2)
        ec = _gcry_generate_fips186_2_prime (nbits, qbits,
                                             initial_seed.seed,
                                             initial_seed.seedlen,
                                             &prime_q, &prime_p,
                                             r_counter,
                                             r_seed, r_seedlen);
      else
        ec = _gcry_generate_fips186_3_prime (nbits, qbits,
                                             initial_seed.seed,
                                             initial_seed.seedlen,
                                             &prime_q, &prime_p,
                                             r_counter,
                                             r_seed, r_seedlen, NULL);
      sexp_release (initial_seed.sexp);
      if (ec)
        goto leave;

      /* Find a generator g (h and e are helpers).
       *    e = (p-1)/q
       */
      value_e = mpi_alloc_like (prime_p);
      mpi_sub_ui (value_e, prime_p, 1);
      mpi_fdiv_q (value_e, value_e, prime_q );
      value_g = mpi_alloc_like (prime_p);
      value_h = mpi_alloc_set_ui (1);
      do
        {
          mpi_add_ui (value_h, value_h, 1);
          /* g = h^e mod p */
          mpi_powm (value_g, value_h, value_e, prime_p);
        }
      while (!mpi_cmp_ui (value_g, 1));  /* Continue until g != 1.  */
    }

  value_c = mpi_snew (qbits);
  value_x = mpi_snew (qbits);
  value_qm2 = mpi_snew (qbits);
  mpi_sub_ui (value_qm2, prime_q, 2);

  /* FIPS 186-4 B.1.2 steps 4-6 */
  do
    {
      if( DBG_CIPHER )
        progress('.');
      _gcry_mpi_randomize (value_c, qbits, GCRY_VERY_STRONG_RANDOM);
      mpi_clear_highbit (value_c, qbits+1);
    }
  while (!(mpi_cmp_ui (value_c, 0) > 0 && mpi_cmp (value_c, value_qm2) < 0));
  /* while (mpi_cmp (value_c, value_qm2) > 0); */

  /* x = c + 1 */
  mpi_add_ui(value_x, value_c, 1);

  /* y = g^x mod p */
  value_y = mpi_alloc_like (prime_p);
  mpi_powm (value_y, value_g, value_x, prime_p);

  if (DBG_CIPHER)
    {
      progress('\n');
      log_mpidump("dsa  p", prime_p );
      log_mpidump("dsa  q", prime_q );
      log_mpidump("dsa  g", value_g );
      log_mpidump("dsa  y", value_y );
      log_mpidump("dsa  x", value_x );
      log_mpidump("dsa  h", value_h );
    }

  /* Copy the stuff to the key structures. */
  sk->p = prime_p; prime_p = NULL;
  sk->q = prime_q; prime_q = NULL;
  sk->g = value_g; value_g = NULL;
  sk->y = value_y; value_y = NULL;
  sk->x = value_x; value_x = NULL;
  *r_h = value_h; value_h = NULL;

 leave:
  _gcry_mpi_release (prime_p);
  _gcry_mpi_release (prime_q);
  _gcry_mpi_release (value_g);
  _gcry_mpi_release (value_y);
  _gcry_mpi_release (value_x);
  _gcry_mpi_release (value_h);
  _gcry_mpi_release (value_e);
  _gcry_mpi_release (value_c);
  _gcry_mpi_release (value_qm2);

  /* As a last step test this keys (this should never fail of course). */
  if (!ec && test_keys (sk, qbits) )
    {
      _gcry_mpi_release (sk->p); sk->p = NULL;
      _gcry_mpi_release (sk->q); sk->q = NULL;
      _gcry_mpi_release (sk->g); sk->g = NULL;
      _gcry_mpi_release (sk->y); sk->y = NULL;
      _gcry_mpi_release (sk->x); sk->x = NULL;
      fips_signal_error ("self-test after key generation failed");
      ec = GPG_ERR_SELFTEST_FAILED;
    }

  if (ec)
    {
      *r_counter = 0;
      xfree (*r_seed); *r_seed = NULL;
      *r_seedlen = 0;
      _gcry_mpi_release (*r_h); *r_h = NULL;
    }

  return ec;
}



/*
   Test whether the secret key is valid.
   Returns: if this is a valid key.
 */
static int
check_secret_key( DSA_secret_key *sk )
{
  int rc;
  gcry_mpi_t y = mpi_alloc( mpi_get_nlimbs(sk->y) );

  mpi_powm( y, sk->g, sk->x, sk->p );
  rc = !mpi_cmp( y, sk->y );
  mpi_free( y );
  return rc;
}



/*
   Make a DSA signature from INPUT and put it into r and s.

   INPUT may either be a plain MPI or an opaque MPI which is then
   internally converted to a plain MPI.  FLAGS and HASHALGO may both
   be 0 for standard operation mode.

   The random value, K_SUPPLIED, may be supplied externally.  If not,
   it is generated internally.

   The return value is 0 on success or an error code.  Note that for
   backward compatibility the function will not return any error if
   FLAGS and HASHALGO are both 0 and INPUT is a plain MPI.
 */
static gpg_err_code_t
sign (gcry_mpi_t r, gcry_mpi_t s, gcry_mpi_t input, gcry_mpi_t k_supplied,
      DSA_secret_key *skey, int flags, int hashalgo)
{
  gpg_err_code_t rc;
  gcry_mpi_t hash;
  gcry_mpi_t k;
  gcry_mpi_t kinv;
  gcry_mpi_t tmp;
  const void *abuf;
  unsigned int abits, qbits;
  int extraloops = 0;
  gcry_mpi_t hash_computed_internally = NULL;

  qbits = mpi_get_nbits (skey->q);

  if ((flags & PUBKEY_FLAG_PREHASH))
    {
      rc = _gcry_dsa_compute_hash (&hash_computed_internally, input, hashalgo);
      if (rc)
        return rc;
      input = hash_computed_internally;
    }

  /* Convert the INPUT into an MPI.  */
  rc = _gcry_dsa_normalize_hash (input, &hash, qbits);
  if (rc)
    {
      mpi_free (hash_computed_internally);
      return rc;
    }

 again:
  if (k_supplied)
    k = k_supplied;
  /* Create the K value.  */
  else if ((flags & PUBKEY_FLAG_RFC6979) && hashalgo)
    {
      /* Use Pornin's method for deterministic DSA.  If this flag is
         set, it is expected that HASH is an opaque MPI with the to be
         signed hash.  That hash is also used as h1 from 3.2.a.  */
      if (!mpi_is_opaque (input))
        {
          rc = GPG_ERR_CONFLICT;
          goto leave;
        }

      abuf = mpi_get_opaque (input, &abits);
      rc = _gcry_dsa_gen_rfc6979_k (&k, skey->q, skey->x,
                                    abuf, (abits+7)/8, hashalgo, extraloops);
      if (rc)
        goto leave;
    }
  else
    {
      /* Select a random k with 0 < k < q */
      k = _gcry_dsa_gen_k (skey->q, GCRY_STRONG_RANDOM);
    }

  /* kinv = k^(-1) mod q */
  kinv = mpi_alloc( mpi_get_nlimbs(k) );
  mpi_invm(kinv, k, skey->q );

  _gcry_dsa_modify_k (k, skey->q, qbits);

  /* r = (a^k mod p) mod q */
  mpi_powm( r, skey->g, k, skey->p );
  mpi_fdiv_r( r, r, skey->q );

  /* s = (kinv * ( hash + x * r)) mod q */
  tmp = mpi_alloc( mpi_get_nlimbs(skey->p) );
  mpi_mul( tmp, skey->x, r );
  mpi_add( tmp, tmp, hash );
  mpi_mulm( s , kinv, tmp, skey->q );

  if (!k_supplied)
    mpi_free(k);
  mpi_free(kinv);
  mpi_free(tmp);

  if (!mpi_cmp_ui (r, 0))
    {
      if (k_supplied)
        {
          rc = GPG_ERR_INV_VALUE;
          goto leave;
        }

      /* This is a highly unlikely code path.  */
      extraloops++;
      goto again;
    }

  rc = 0;

 leave:
  if (hash != input)
    mpi_free (hash);
  mpi_free (hash_computed_internally);

  return rc;
}


/*
   Returns true if the signature composed from R and S is valid.
 */
static gpg_err_code_t
verify (gcry_mpi_t r, gcry_mpi_t s, gcry_mpi_t input, DSA_public_key *pkey,
        int flags, int hashalgo)
{
  gpg_err_code_t rc = 0;
  gcry_mpi_t w, u1, u2, v;
  gcry_mpi_t base[3];
  gcry_mpi_t ex[3];
  gcry_mpi_t hash;
  unsigned int nbits;
  gcry_mpi_t hash_computed_internally = NULL;

  if( !(mpi_cmp_ui( r, 0 ) > 0 && mpi_cmp( r, pkey->q ) < 0) )
    return GPG_ERR_BAD_SIGNATURE; /* Assertion	0 < r < n  failed.  */
  if( !(mpi_cmp_ui( s, 0 ) > 0 && mpi_cmp( s, pkey->q ) < 0) )
    return GPG_ERR_BAD_SIGNATURE; /* Assertion	0 < s < n  failed.  */

  nbits = mpi_get_nbits (pkey->q);
  if ((flags & PUBKEY_FLAG_PREHASH))
    {
      rc = _gcry_dsa_compute_hash (&hash_computed_internally, input, hashalgo);
      if (rc)
        return rc;
      input = hash_computed_internally;
    }
  rc = _gcry_dsa_normalize_hash (input, &hash, nbits);
  if (rc)
    {
      mpi_free (hash_computed_internally);
      return rc;
    }

  w  = mpi_alloc( mpi_get_nlimbs(pkey->q) );
  u1 = mpi_alloc( mpi_get_nlimbs(pkey->q) );
  u2 = mpi_alloc( mpi_get_nlimbs(pkey->q) );
  v  = mpi_alloc( mpi_get_nlimbs(pkey->p) );

  /* w = s^(-1) mod q */
  mpi_invm( w, s, pkey->q );

  /* u1 = (hash * w) mod q */
  mpi_mulm( u1, hash, w, pkey->q );

  /* u2 = r * w mod q  */
  mpi_mulm( u2, r, w, pkey->q );

  /* v =  g^u1 * y^u2 mod p mod q */
  base[0] = pkey->g; ex[0] = u1;
  base[1] = pkey->y; ex[1] = u2;
  base[2] = NULL;    ex[2] = NULL;
  mpi_mulpowm( v, base, ex, pkey->p );
  mpi_fdiv_r( v, v, pkey->q );

  if (mpi_cmp( v, r ))
    {
      if (DBG_CIPHER)
        {
          log_mpidump ("     i", input);
          log_mpidump ("     h", hash);
          log_mpidump ("     v", v);
          log_mpidump ("     r", r);
          log_mpidump ("     s", s);
        }
      rc = GPG_ERR_BAD_SIGNATURE;
    }

  mpi_free(w);
  mpi_free(u1);
  mpi_free(u2);
  mpi_free(v);
  if (hash != input)
    mpi_free (hash);
  mpi_free (hash_computed_internally);

  return rc;
}


/*********************************************
 **************  interface  ******************
 *********************************************/

static gcry_err_code_t
dsa_generate (const gcry_sexp_t genparms, gcry_sexp_t *r_skey)
{
  gpg_err_code_t rc;
  unsigned int nbits;
  gcry_sexp_t domainsexp;
  DSA_secret_key sk;
  gcry_sexp_t l1;
  unsigned int qbits = 0;
  gcry_sexp_t deriveparms = NULL;
  gcry_sexp_t seedinfo = NULL;
  gcry_sexp_t misc_info = NULL;
  int flags = 0;
  dsa_domain_t domain;
  gcry_mpi_t *factors = NULL;

  memset (&sk, 0, sizeof sk);
  memset (&domain, 0, sizeof domain);

  rc = _gcry_pk_util_get_nbits (genparms, &nbits);
  if (rc)
    return rc;

  /* Parse the optional flags list.  */
  l1 = sexp_find_token (genparms, "flags", 0);
  if (l1)
    {
      rc = _gcry_pk_util_parse_flaglist (l1, &flags, NULL);
      sexp_release (l1);
      if (rc)
        return rc;\
    }

  /* Parse the optional qbits element.  */
  l1 = sexp_find_token (genparms, "qbits", 0);
  if (l1)
    {
      char buf[50];
      const char *s;
      size_t n;

      s = sexp_nth_data (l1, 1, &n);
      if (!s || n >= DIM (buf) - 1 )
        {
          sexp_release (l1);
          return GPG_ERR_INV_OBJ; /* No value or value too large.  */
        }
      memcpy (buf, s, n);
      buf[n] = 0;
      qbits = (unsigned int)strtoul (buf, NULL, 0);
      sexp_release (l1);
    }

  /* Parse the optional transient-key flag.  */
  if (!(flags & PUBKEY_FLAG_TRANSIENT_KEY))
    {
      l1 = sexp_find_token (genparms, "transient-key", 0);
      if (l1)
        {
          flags |= PUBKEY_FLAG_TRANSIENT_KEY;
          sexp_release (l1);
        }
    }

  /* Get the optional derive parameters.  */
  deriveparms = sexp_find_token (genparms, "derive-parms", 0);

  /* Parse the optional "use-fips186" flags.  */
  if (!(flags & PUBKEY_FLAG_USE_FIPS186))
    {
      l1 = sexp_find_token (genparms, "use-fips186", 0);
      if (l1)
        {
          flags |= PUBKEY_FLAG_USE_FIPS186;
          sexp_release (l1);
        }
    }
  if (!(flags & PUBKEY_FLAG_USE_FIPS186_2))
    {
      l1 = sexp_find_token (genparms, "use-fips186-2", 0);
      if (l1)
        {
          flags |= PUBKEY_FLAG_USE_FIPS186_2;
          sexp_release (l1);
        }
    }

  /* Check whether domain parameters are given.  */
  domainsexp = sexp_find_token (genparms, "domain", 0);
  if (domainsexp)
    {
      /* DERIVEPARMS can't be used together with domain parameters.
         NBITS abnd QBITS may not be specified because there values
         are derived from the domain parameters.  */
      if (deriveparms || qbits || nbits)
        {
          sexp_release (domainsexp);
          sexp_release (deriveparms);
          return GPG_ERR_INV_VALUE;
        }

      /* Put all domain parameters into the domain object.  */
      l1 = sexp_find_token (domainsexp, "p", 0);
      domain.p = sexp_nth_mpi (l1, 1, GCRYMPI_FMT_USG);
      sexp_release (l1);
      l1 = sexp_find_token (domainsexp, "q", 0);
      domain.q = sexp_nth_mpi (l1, 1, GCRYMPI_FMT_USG);
      sexp_release (l1);
      l1 = sexp_find_token (domainsexp, "g", 0);
      domain.g = sexp_nth_mpi (l1, 1, GCRYMPI_FMT_USG);
      sexp_release (l1);
      sexp_release (domainsexp);

      /* Check that all domain parameters are available.  */
      if (!domain.p || !domain.q || !domain.g)
        {
          _gcry_mpi_release (domain.p);
          _gcry_mpi_release (domain.q);
          _gcry_mpi_release (domain.g);
          sexp_release (deriveparms);
          return GPG_ERR_MISSING_VALUE;
        }

      /* Get NBITS and QBITS from the domain parameters.  */
      nbits = mpi_get_nbits (domain.p);
      qbits = mpi_get_nbits (domain.q);
    }

  if (deriveparms
      || (flags & PUBKEY_FLAG_USE_FIPS186)
      || (flags & PUBKEY_FLAG_USE_FIPS186_2)
      || fips_mode ())
    {
      int counter;
      void *seed;
      size_t seedlen;
      gcry_mpi_t h_value;

      rc = generate_fips186 (&sk, nbits, qbits, deriveparms,
                             !!(flags & PUBKEY_FLAG_USE_FIPS186_2),
                             &domain,
                             &counter, &seed, &seedlen, &h_value);
      if (!rc && h_value)
        {
          /* Format the seed-values unless domain parameters are used
             for which a H_VALUE of NULL is an indication.  */
          rc = sexp_build (&seedinfo, NULL,
                           "(seed-values(counter %d)(seed %b)(h %m))",
                           counter, (int)seedlen, seed, h_value);
          xfree (seed);
          _gcry_mpi_release (h_value);
        }
    }
  else
    {
      rc = generate (&sk, nbits, qbits,
                     !!(flags & PUBKEY_FLAG_TRANSIENT_KEY),
                     &domain, &factors);
    }

  if (!rc)
    {
      /* Put the factors into MISC_INFO.  Note that the factors are
         not confidential thus we can store them in standard memory.  */
      int nfactors, i, j;
      char *p;
      char *format = NULL;
      void **arg_list = NULL;

      for (nfactors=0; factors && factors[nfactors]; nfactors++)
        ;
      /* Allocate space for the format string:
         "(misc-key-info%S(pm1-factors%m))"
         with one "%m" for each factor and construct it.  */
      format = xtrymalloc (50 + 2*nfactors);
      if (!format)
        rc = gpg_err_code_from_syserror ();
      else
        {
          p = stpcpy (format, "(misc-key-info");
          if (seedinfo)
            p = stpcpy (p, "%S");
          if (nfactors)
            {
              p = stpcpy (p, "(pm1-factors");
              for (i=0; i < nfactors; i++)
                p = stpcpy (p, "%m");
              p = stpcpy (p, ")");
            }
          p = stpcpy (p, ")");

          /* Allocate space for the list of factors plus one for the
             seedinfo s-exp plus an extra NULL entry for safety and
             fill it with the factors.  */
          arg_list = xtrycalloc (nfactors+1+1, sizeof *arg_list);
          if (!arg_list)
            rc = gpg_err_code_from_syserror ();
          else
            {
              i = 0;
              if (seedinfo)
                arg_list[i++] = &seedinfo;
              for (j=0; j < nfactors; j++)
                arg_list[i++] = factors + j;
              arg_list[i] = NULL;

              rc = sexp_build_array (&misc_info, NULL, format, arg_list);
            }
        }

      xfree (arg_list);
      xfree (format);
    }

  if (!rc)
    rc = sexp_build (r_skey, NULL,
                     "(key-data"
                     " (public-key"
                     "  (dsa(p%m)(q%m)(g%m)(y%m)))"
                     " (private-key"
                     "  (dsa(p%m)(q%m)(g%m)(y%m)(x%m)))"
                     " %S)",
                     sk.p, sk.q, sk.g, sk.y,
                     sk.p, sk.q, sk.g, sk.y, sk.x,
                     misc_info);


  _gcry_mpi_release (sk.p);
  _gcry_mpi_release (sk.q);
  _gcry_mpi_release (sk.g);
  _gcry_mpi_release (sk.y);
  _gcry_mpi_release (sk.x);

  _gcry_mpi_release (domain.p);
  _gcry_mpi_release (domain.q);
  _gcry_mpi_release (domain.g);

  sexp_release (seedinfo);
  sexp_release (misc_info);
  sexp_release (deriveparms);
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
dsa_check_secret_key (gcry_sexp_t keyparms)
{
  gcry_err_code_t rc;
  DSA_secret_key sk = {NULL, NULL, NULL, NULL, NULL};

  rc = _gcry_sexp_extract_param (keyparms, NULL, "pqgyx",
                                  &sk.p, &sk.q, &sk.g, &sk.y, &sk.x,
                                  NULL);
  if (rc)
    goto leave;

  if (!check_secret_key (&sk))
    rc = GPG_ERR_BAD_SECKEY;

 leave:
  _gcry_mpi_release (sk.p);
  _gcry_mpi_release (sk.q);
  _gcry_mpi_release (sk.g);
  _gcry_mpi_release (sk.y);
  _gcry_mpi_release (sk.x);
  if (DBG_CIPHER)
    log_debug ("dsa_testkey    => %s\n", gpg_strerror (rc));
  return rc;
}


static gcry_err_code_t
dsa_sign (gcry_sexp_t *r_sig, gcry_sexp_t s_data, gcry_sexp_t keyparms)
{
  gcry_err_code_t rc;
  struct pk_encoding_ctx ctx;
  gcry_mpi_t data = NULL;
  gcry_mpi_t k = NULL;
  DSA_secret_key sk = {NULL, NULL, NULL, NULL, NULL};
  gcry_mpi_t sig_r = NULL;
  gcry_mpi_t sig_s = NULL;
  unsigned int nbits = dsa_get_nbits (keyparms);

  rc = dsa_check_keysize (nbits);
  if (rc)
    return rc;

  _gcry_pk_util_init_encoding_ctx (&ctx, PUBKEY_OP_SIGN, nbits);

  /* Extract the data.  */
  rc = _gcry_pk_util_data_to_mpi (s_data, &data, &ctx);
  if (rc)
    goto leave;
  if (DBG_CIPHER)
    log_mpidump ("dsa_sign   data", data);

  if (ctx.label)
    rc = _gcry_mpi_scan (&k, GCRYMPI_FMT_USG, ctx.label, ctx.labellen, NULL);
  if (rc)
    goto leave;

  /* Extract the key.  */
  rc = _gcry_sexp_extract_param (keyparms, NULL, "pqgyx",
                                 &sk.p, &sk.q, &sk.g, &sk.y, &sk.x, NULL);
  if (rc)
    goto leave;
  if (DBG_CIPHER)
    {
      log_mpidump ("dsa_sign      p", sk.p);
      log_mpidump ("dsa_sign      q", sk.q);
      log_mpidump ("dsa_sign      g", sk.g);
      log_mpidump ("dsa_sign      y", sk.y);
      if (!fips_mode ())
        log_mpidump ("dsa_sign      x", sk.x);
    }

  sig_r = mpi_new (0);
  sig_s = mpi_new (0);
  rc = sign (sig_r, sig_s, data, k, &sk, ctx.flags, ctx.hash_algo);
  if (rc)
    goto leave;
  if (DBG_CIPHER)
    {
      log_mpidump ("dsa_sign  sig_r", sig_r);
      log_mpidump ("dsa_sign  sig_s", sig_s);
    }
  rc = sexp_build (r_sig, NULL, "(sig-val(dsa(r%M)(s%M)))", sig_r, sig_s);

 leave:
  _gcry_mpi_release (sig_r);
  _gcry_mpi_release (sig_s);
  _gcry_mpi_release (sk.p);
  _gcry_mpi_release (sk.q);
  _gcry_mpi_release (sk.g);
  _gcry_mpi_release (sk.y);
  _gcry_mpi_release (sk.x);
  _gcry_mpi_release (data);
  _gcry_mpi_release (k);
  _gcry_pk_util_free_encoding_ctx (&ctx);
  if (DBG_CIPHER)
    log_debug ("dsa_sign      => %s\n", gpg_strerror (rc));
  return rc;
}


static gcry_err_code_t
dsa_verify (gcry_sexp_t s_sig, gcry_sexp_t s_data, gcry_sexp_t s_keyparms)
{
  gcry_err_code_t rc;
  struct pk_encoding_ctx ctx;
  gcry_sexp_t l1 = NULL;
  gcry_mpi_t sig_r = NULL;
  gcry_mpi_t sig_s = NULL;
  gcry_mpi_t data = NULL;
  DSA_public_key pk = { NULL, NULL, NULL, NULL };
  unsigned int nbits = dsa_get_nbits (s_keyparms);

  rc = dsa_check_keysize (nbits);
  if (rc)
    return rc;

  _gcry_pk_util_init_encoding_ctx (&ctx, PUBKEY_OP_VERIFY, nbits);

  /* Extract the data.  */
  rc = _gcry_pk_util_data_to_mpi (s_data, &data, &ctx);
  if (rc)
    goto leave;
  if (DBG_CIPHER)
    log_mpidump ("dsa_verify data", data);

  /* Extract the signature value.  */
  rc = _gcry_pk_util_preparse_sigval (s_sig, dsa_names, &l1, NULL);
  if (rc)
    goto leave;
  rc = _gcry_sexp_extract_param (l1, NULL, "rs", &sig_r, &sig_s, NULL);
  if (rc)
    goto leave;
  if (DBG_CIPHER)
    {
      log_mpidump ("dsa_verify  s_r", sig_r);
      log_mpidump ("dsa_verify  s_s", sig_s);
    }

  /* Extract the key.  */
  rc = _gcry_sexp_extract_param (s_keyparms, NULL, "pqgy",
                                 &pk.p, &pk.q, &pk.g, &pk.y, NULL);
  if (rc)
    goto leave;
  if (DBG_CIPHER)
    {
      log_mpidump ("dsa_verify    p", pk.p);
      log_mpidump ("dsa_verify    q", pk.q);
      log_mpidump ("dsa_verify    g", pk.g);
      log_mpidump ("dsa_verify    y", pk.y);
    }

  /* Verify the signature.  */
  rc = verify (sig_r, sig_s, data, &pk, ctx.flags, ctx.hash_algo);

 leave:
  _gcry_mpi_release (pk.p);
  _gcry_mpi_release (pk.q);
  _gcry_mpi_release (pk.g);
  _gcry_mpi_release (pk.y);
  _gcry_mpi_release (data);
  _gcry_mpi_release (sig_r);
  _gcry_mpi_release (sig_s);
  sexp_release (l1);
  _gcry_pk_util_free_encoding_ctx (&ctx);
  if (DBG_CIPHER)
    log_debug ("dsa_verify    => %s\n", rc?gpg_strerror (rc):"Good");
  return rc;
}


/* Return the number of bits for the key described by PARMS.  On error
 * 0 is returned.  The format of PARMS starts with the algorithm name;
 * for example:
 *
 *   (dsa
 *     (p <mpi>)
 *     (q <mpi>)
 *     (g <mpi>)
 *     (y <mpi>))
 *
 * More parameters may be given but we only need P here.
 */
static unsigned int
dsa_get_nbits (gcry_sexp_t parms)
{
  gcry_sexp_t l1;
  gcry_mpi_t p;
  unsigned int nbits;

  l1 = sexp_find_token (parms, "p", 1);
  if (!l1)
    return 0; /* Parameter P not found.  */

  p = sexp_nth_mpi (l1, 1, GCRYMPI_FMT_USG);
  sexp_release (l1);
  nbits = p? mpi_get_nbits (p) : 0;
  _gcry_mpi_release (p);
  return nbits;
}



/*
     Self-test section.
 */

static const char *
selftest_sign (gcry_sexp_t pkey, gcry_sexp_t skey)
{
  /* Sample data from RFC 6979 section A.2.2, hash is of message "sample" */
  static const char sample_data[] =
    "(data (flags rfc6979 prehash)"
    " (hash-algo sha256)"
    " (value 6:sample))";
  static const char sample_data_bad[] =
    "(data (flags rfc6979)"
    " (hash sha256 #bf2bdbe1aa9b6ec1e2ade1d694f41fc71a831d0268e9891562113d8a62add1bf#))";
  static const char signature_r[] =
    "eace8bdbbe353c432a795d9ec556c6d021f7a03f42c36e9bc87e4ac7932cc809";
  static const char signature_s[] =
    "7081e175455f9247b812b74583e9e94f9ea79bd640dc962533b0680793a38d53";

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
  l2 = _gcry_sexp_find_token (l1, "dsa", 0);
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
  _gcry_mpi_release (calculated_s);
  _gcry_mpi_release (calculated_r);
  _gcry_mpi_release (s);
  _gcry_mpi_release (r);
  sexp_release (l2);
  sexp_release (l1);
  sexp_release (sig);
  sexp_release (data_bad);
  sexp_release (data);
  return errtxt;
}


static gpg_err_code_t
selftests_dsa_2048 (selftest_report_func_t report)
{
  const char *what;
  const char *errtxt;
  gcry_error_t err;
  gcry_sexp_t skey = NULL;
  gcry_sexp_t pkey = NULL;

  /* Convert the S-expressions into the internal representation.  */
  what = "convert";
  err = sexp_sscan (&skey, NULL, sample_secret_key_2048, strlen (sample_secret_key_2048));
  if (!err)
    err = sexp_sscan (&pkey, NULL,
                      sample_public_key_2048, strlen (sample_public_key_2048));
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

  what = "sign";
  errtxt = selftest_sign (pkey, skey);
  if (errtxt)
    goto failed;

  sexp_release (pkey);
  sexp_release (skey);
  return 0; /* Succeeded. */

 failed:
  sexp_release (pkey);
  sexp_release (skey);
  if (report)
    report ("pubkey", GCRY_PK_DSA, what, errtxt);
  return GPG_ERR_SELFTEST_FAILED;
}


/* Run a full self-test for ALGO and return 0 on success.  */
static gpg_err_code_t
run_selftests (int algo, int extended, selftest_report_func_t report)
{
  gpg_err_code_t ec;

  (void)extended;

  switch (algo)
    {
    case GCRY_PK_DSA:
      ec = selftests_dsa_2048 (report);
      break;
    default:
      ec = GPG_ERR_PUBKEY_ALGO;
      break;

    }
  return ec;
}



gcry_pk_spec_t _gcry_pubkey_spec_dsa =
  {
    GCRY_PK_DSA, { 0, 0 },
    GCRY_PK_USAGE_SIGN,
    "DSA", dsa_names,
    "pqgy", "pqgyx", "", "rs", "pqgy",
    dsa_generate,
    dsa_check_secret_key,
    NULL,
    NULL,
    dsa_sign,
    dsa_verify,
    dsa_get_nbits,
    run_selftests
  };
