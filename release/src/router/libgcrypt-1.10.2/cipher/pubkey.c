/* pubkey.c  -	pubkey dispatcher
 * Copyright (C) 1998, 1999, 2000, 2002, 2003, 2005,
 *               2007, 2008, 2011 Free Software Foundation, Inc.
 * Copyright (C) 2013 g10 Code GmbH
 *
 * This file is part of Libgcrypt.
 *
 * Libgcrypt is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser general Public License as
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
#include <errno.h>

#include "g10lib.h"
#include "mpi.h"
#include "cipher.h"
#include "context.h"
#include "pubkey-internal.h"


/* This is the list of the public-key algorithms included in
   Libgcrypt.  */
static gcry_pk_spec_t * const pubkey_list[] =
  {
#if USE_ECC
    &_gcry_pubkey_spec_ecc,
#endif
#if USE_RSA
    &_gcry_pubkey_spec_rsa,
#endif
#if USE_DSA
    &_gcry_pubkey_spec_dsa,
#endif
#if USE_ELGAMAL
    &_gcry_pubkey_spec_elg,
#endif
    NULL
  };


static int
map_algo (int algo)
{
 switch (algo)
   {
   case GCRY_PK_RSA_E: return GCRY_PK_RSA;
   case GCRY_PK_RSA_S: return GCRY_PK_RSA;
   case GCRY_PK_ELG_E: return GCRY_PK_ELG;
   case GCRY_PK_ECDSA: return GCRY_PK_ECC;
   case GCRY_PK_EDDSA: return GCRY_PK_ECC;
   case GCRY_PK_ECDH:  return GCRY_PK_ECC;
   default:            return algo;
   }
}


/* Return the spec structure for the public key algorithm ALGO.  For
   an unknown algorithm NULL is returned.  */
static gcry_pk_spec_t *
spec_from_algo (int algo)
{
  int idx;
  gcry_pk_spec_t *spec;

  algo = map_algo (algo);

  for (idx = 0; (spec = pubkey_list[idx]); idx++)
    if (algo == spec->algo)
      return spec;
  return NULL;
}


/* Return the spec structure for the public key algorithm with NAME.
   For an unknown name NULL is returned.  */
static gcry_pk_spec_t *
spec_from_name (const char *name)
{
  gcry_pk_spec_t *spec;
  int idx;
  const char **aliases;

  for (idx=0; (spec = pubkey_list[idx]); idx++)
    {
      if (!stricmp (name, spec->name))
        return spec;
      for (aliases = spec->aliases; *aliases; aliases++)
        if (!stricmp (name, *aliases))
          return spec;
    }

  return NULL;
}



/* Given the s-expression SEXP with the first element be either
 * "private-key" or "public-key" return the spec structure for it.  We
 * look through the list to find a list beginning with "private-key"
 * or "public-key" - the first one found is used.  If WANT_PRIVATE is
 * set the function will only succeed if a private key has been given.
 * On success the spec is stored at R_SPEC.  On error NULL is stored
 * at R_SPEC and an error code returned.  If R_PARMS is not NULL and
 * the function returns success, the parameter list below
 * "private-key" or "public-key" is stored there and the caller must
 * call gcry_sexp_release on it.
 */
static gcry_err_code_t
spec_from_sexp (gcry_sexp_t sexp, int want_private,
                gcry_pk_spec_t **r_spec, gcry_sexp_t *r_parms)
{
  gcry_sexp_t list, l2;
  char *name;
  gcry_pk_spec_t *spec;

  *r_spec = NULL;
  if (r_parms)
    *r_parms = NULL;

  /* Check that the first element is valid.  If we are looking for a
     public key but a private key was supplied, we allow the use of
     the private key anyway.  The rationale for this is that the
     private key is a superset of the public key.  */
  list = sexp_find_token (sexp, want_private? "private-key":"public-key", 0);
  if (!list && !want_private)
    list = sexp_find_token (sexp, "private-key", 0);
  if (!list)
    return GPG_ERR_INV_OBJ; /* Does not contain a key object.  */

  l2 = sexp_cadr (list);
  sexp_release (list);
  list = l2;
  name = sexp_nth_string (list, 0);
  if (!name)
    {
      sexp_release ( list );
      return GPG_ERR_INV_OBJ;      /* Invalid structure of object. */
    }
  spec = spec_from_name (name);
  xfree (name);
  if (!spec)
    {
      sexp_release (list);
      return GPG_ERR_PUBKEY_ALGO; /* Unknown algorithm. */
    }
  *r_spec = spec;
  if (r_parms)
    *r_parms = list;
  else
    sexp_release (list);
  return 0;
}



/* Disable the use of the algorithm ALGO.  This is not thread safe and
   should thus be called early.  */
static void
disable_pubkey_algo (int algo)
{
  gcry_pk_spec_t *spec = spec_from_algo (algo);

  if (spec)
    spec->flags.disabled = 1;
}



/*
 * Map a string to the pubkey algo
 */
int
_gcry_pk_map_name (const char *string)
{
  gcry_pk_spec_t *spec;

  if (!string)
    return 0;
  spec = spec_from_name (string);
  if (!spec)
    return 0;
  if (spec->flags.disabled)
    return 0;
  if (!spec->flags.fips && fips_mode ())
    return 0;
  return spec->algo;
}


/* Map the public key algorithm whose ID is contained in ALGORITHM to
   a string representation of the algorithm name.  For unknown
   algorithm IDs this functions returns "?". */
const char *
_gcry_pk_algo_name (int algo)
{
  gcry_pk_spec_t *spec;

  spec = spec_from_algo (algo);
  if (spec)
    return spec->name;
  return "?";
}


/****************
 * A USE of 0 means: don't care.
 */
static gcry_err_code_t
check_pubkey_algo (int algo, unsigned use)
{
  gcry_err_code_t err = 0;
  gcry_pk_spec_t *spec;

  spec = spec_from_algo (algo);
  if (spec && !spec->flags.disabled && (spec->flags.fips || !fips_mode ()))
    {
      if (((use & GCRY_PK_USAGE_SIGN)
	   && (! (spec->use & GCRY_PK_USAGE_SIGN)))
	  || ((use & GCRY_PK_USAGE_ENCR)
	      && (! (spec->use & GCRY_PK_USAGE_ENCR))))
	err = GPG_ERR_WRONG_PUBKEY_ALGO;
    }
  else
    err = GPG_ERR_PUBKEY_ALGO;

  return err;
}


/****************
 * Return the number of public key material numbers
 */
static int
pubkey_get_npkey (int algo)
{
  gcry_pk_spec_t *spec = spec_from_algo (algo);

  return spec? strlen (spec->elements_pkey) : 0;
}


/****************
 * Return the number of secret key material numbers
 */
static int
pubkey_get_nskey (int algo)
{
  gcry_pk_spec_t *spec = spec_from_algo (algo);

  return spec? strlen (spec->elements_skey) : 0;
}


/****************
 * Return the number of signature material numbers
 */
static int
pubkey_get_nsig (int algo)
{
  gcry_pk_spec_t *spec = spec_from_algo (algo);

  return spec? strlen (spec->elements_sig) : 0;
}

/****************
 * Return the number of encryption material numbers
 */
static int
pubkey_get_nenc (int algo)
{
  gcry_pk_spec_t *spec = spec_from_algo (algo);

  return spec? strlen (spec->elements_enc) : 0;
}



/*
   Do a PK encrypt operation

   Caller has to provide a public key as the SEXP pkey and data as a
   SEXP with just one MPI in it. Alternatively S_DATA might be a
   complex S-Expression, similar to the one used for signature
   verification.  This provides a flag which allows to handle PKCS#1
   block type 2 padding.  The function returns a sexp which may be
   passed to to pk_decrypt.

   Returns: 0 or an errorcode.

   s_data = See comment for _gcry_pk_util_data_to_mpi
   s_pkey = <key-as-defined-in-sexp_to_key>
   r_ciph = (enc-val
               (<algo>
                 (<param_name1> <mpi>)
                 ...
                 (<param_namen> <mpi>)
               ))

*/
gcry_err_code_t
_gcry_pk_encrypt (gcry_sexp_t *r_ciph, gcry_sexp_t s_data, gcry_sexp_t s_pkey)
{
  gcry_err_code_t rc;
  gcry_pk_spec_t *spec;
  gcry_sexp_t keyparms;

  *r_ciph = NULL;

  rc = spec_from_sexp (s_pkey, 0, &spec, &keyparms);
  if (rc)
    goto leave;

  if (spec->flags.disabled)
    rc = GPG_ERR_PUBKEY_ALGO;
  else if (!spec->flags.fips && fips_mode ())
    rc = GPG_ERR_PUBKEY_ALGO;
  else if (spec->encrypt)
    rc = spec->encrypt (r_ciph, s_data, keyparms);
  else
    rc = GPG_ERR_NOT_IMPLEMENTED;

 leave:
  sexp_release (keyparms);
  return rc;
}


/*
   Do a PK decrypt operation

   Caller has to provide a secret key as the SEXP skey and data in a
   format as created by gcry_pk_encrypt.  For historic reasons the
   function returns simply an MPI as an S-expression part; this is
   deprecated and the new method should be used which returns a real
   S-expressionl this is selected by adding at least an empty flags
   list to S_DATA.

   Returns: 0 or an errorcode.

   s_data = (enc-val
              [(flags [raw, pkcs1, oaep])]
              (<algo>
                (<param_name1> <mpi>)
                ...
                (<param_namen> <mpi>)
              ))
   s_skey = <key-as-defined-in-sexp_to_key>
   r_plain= Either an incomplete S-expression without the parentheses
            or if the flags list is used (even if empty) a real S-expression:
            (value PLAIN).  In raw mode (or no flags given) the returned value
            is to be interpreted as a signed MPI, thus it may have an extra
            leading zero octet even if not included in the original data.
            With pkcs1 or oaep decoding enabled the returned value is a
            verbatim octet string.
 */
gcry_err_code_t
_gcry_pk_decrypt (gcry_sexp_t *r_plain, gcry_sexp_t s_data, gcry_sexp_t s_skey)
{
  gcry_err_code_t rc;
  gcry_pk_spec_t *spec;
  gcry_sexp_t keyparms;

  *r_plain = NULL;

  rc = spec_from_sexp (s_skey, 1, &spec, &keyparms);
  if (rc)
    goto leave;

  if (spec->flags.disabled)
    rc = GPG_ERR_PUBKEY_ALGO;
  else if (!spec->flags.fips && fips_mode ())
    rc = GPG_ERR_PUBKEY_ALGO;
  else if (spec->decrypt)
    rc = spec->decrypt (r_plain, s_data, keyparms);
  else
    rc = GPG_ERR_NOT_IMPLEMENTED;

 leave:
  sexp_release (keyparms);
  return rc;
}



/*
   Create a signature.

   Caller has to provide a secret key as the SEXP skey and data
   expressed as a SEXP list hash with only one element which should
   instantly be available as a MPI. Alternatively the structure given
   below may be used for S_HASH, it provides the abiliy to pass flags
   to the operation; the flags defined by now are "pkcs1" which does
   PKCS#1 block type 1 style padding and "pss" for PSS encoding.

   Returns: 0 or an errorcode.
            In case of 0 the function returns a new SEXP with the
            signature value; the structure of this signature depends on the
            other arguments but is always suitable to be passed to
            gcry_pk_verify

   s_hash = See comment for _gcry-pk_util_data_to_mpi

   s_skey = <key-as-defined-in-sexp_to_key>
   r_sig  = (sig-val
              (<algo>
                (<param_name1> <mpi>)
                ...
                (<param_namen> <mpi>))
             [(hash algo)])

  Note that (hash algo) in R_SIG is not used.
*/
gcry_err_code_t
_gcry_pk_sign (gcry_sexp_t *r_sig, gcry_sexp_t s_hash, gcry_sexp_t s_skey)
{
  gcry_err_code_t rc;
  gcry_pk_spec_t *spec;
  gcry_sexp_t keyparms;

  *r_sig = NULL;

  rc = spec_from_sexp (s_skey, 1, &spec, &keyparms);
  if (rc)
    goto leave;

  if (spec->flags.disabled)
    rc = GPG_ERR_PUBKEY_ALGO;
  else if (!spec->flags.fips && fips_mode ())
    rc = GPG_ERR_PUBKEY_ALGO;
  else if (spec->sign)
    rc = spec->sign (r_sig, s_hash, keyparms);
  else
    rc = GPG_ERR_NOT_IMPLEMENTED;

 leave:
  sexp_release (keyparms);
  return rc;
}


gcry_err_code_t
_gcry_pk_sign_md (gcry_sexp_t *r_sig, const char *tmpl, gcry_md_hd_t hd_orig,
                  gcry_sexp_t s_skey, gcry_ctx_t ctx)
{
  gcry_err_code_t rc;
  gcry_pk_spec_t *spec;
  gcry_sexp_t keyparms = NULL;
  gcry_sexp_t s_hash = NULL;
  int algo;
  const unsigned char *digest;
  gcry_error_t err;
  gcry_md_hd_t hd;
  const char *s;
  char *hash_name;

  *r_sig = NULL;

  /* Check if it has fixed hash name or %s */
  s = strstr (tmpl, "(hash ");
  if (s == NULL)
    return GPG_ERR_DIGEST_ALGO;

  s += 6;
  if (!strncmp (s, "%s", 2))
    hash_name = NULL;
  else
    {
      const char *p;

      for (p = s; *p && *p != ' '; p++)
	;

      hash_name = xtrymalloc (p - s + 1);
      if (!hash_name)
	return gpg_error_from_syserror ();
      memcpy (hash_name, s, p - s);
      hash_name[p - s] = 0;
    }

  err = _gcry_md_copy (&hd, hd_orig);
  if (err)
    {
      xfree (hash_name);
      return gpg_err_code (err);
    }

  if (hash_name)
    {
      algo = _gcry_md_map_name (hash_name);
      if (algo == 0
          || (fips_mode () && algo == GCRY_MD_SHA1))
	{
	  xfree (hash_name);
	  _gcry_md_close (hd);
	  return GPG_ERR_DIGEST_ALGO;
	}

      digest = _gcry_md_read (hd, algo);
    }
  else
    {
      algo = _gcry_md_get_algo (hd);

      if (fips_mode () && algo == GCRY_MD_SHA1)
        {
          _gcry_md_close (hd);
          return GPG_ERR_DIGEST_ALGO;
        }

      digest = _gcry_md_read (hd, 0);
    }

  if (!digest)
    {
      xfree (hash_name);
      _gcry_md_close (hd);
      return GPG_ERR_NOT_IMPLEMENTED;
    }

  if (!ctx)
    {
      if (hash_name)
	rc = _gcry_sexp_build (&s_hash, NULL, tmpl,
			       (int) _gcry_md_get_algo_dlen (algo),
			       digest);
      else
	rc = _gcry_sexp_build (&s_hash, NULL, tmpl,
			       _gcry_md_algo_name (algo),
			       (int) _gcry_md_get_algo_dlen (algo),
			       digest);
    }
  else
    {
      const unsigned char *p;
      size_t len;

      rc = _gcry_pk_get_random_override (ctx, &p, &len);
      if (rc)
        {
          _gcry_md_close (hd);
          return rc;
        }

      if (hash_name)
	rc = _gcry_sexp_build (&s_hash, NULL, tmpl,
			       (int) _gcry_md_get_algo_dlen (algo),
			       digest,
			       (int) len, p);
      else
	rc = _gcry_sexp_build (&s_hash, NULL, tmpl,
			       _gcry_md_algo_name (algo),
			       (int) _gcry_md_get_algo_dlen (algo),
			       digest,
			       (int) len, p);
    }

  xfree (hash_name);
  _gcry_md_close (hd);
  if (rc)
    return rc;

  rc = spec_from_sexp (s_skey, 1, &spec, &keyparms);
  if (rc)
    goto leave;

  if (spec->flags.disabled)
    rc = GPG_ERR_PUBKEY_ALGO;
  else if (!spec->flags.fips && fips_mode ())
    rc = GPG_ERR_PUBKEY_ALGO;
  else if (spec->sign)
    rc = spec->sign (r_sig, s_hash, keyparms);
  else
    rc = GPG_ERR_NOT_IMPLEMENTED;

 leave:
  sexp_release (s_hash);
  sexp_release (keyparms);
  return rc;
}


/*
   Verify a signature.

   Caller has to supply the public key pkey, the signature sig and his
   hashvalue data.  Public key has to be a standard public key given
   as an S-Exp, sig is a S-Exp as returned from gcry_pk_sign and data
   must be an S-Exp like the one in sign too.  */
gcry_err_code_t
_gcry_pk_verify (gcry_sexp_t s_sig, gcry_sexp_t s_hash, gcry_sexp_t s_pkey)
{
  gcry_err_code_t rc;
  gcry_pk_spec_t *spec;
  gcry_sexp_t keyparms;

  rc = spec_from_sexp (s_pkey, 0, &spec, &keyparms);
  if (rc)
    goto leave;

  if (spec->flags.disabled)
    rc = GPG_ERR_PUBKEY_ALGO;
  else if (!spec->flags.fips && fips_mode ())
    rc = GPG_ERR_PUBKEY_ALGO;
  else if (spec->verify)
    rc = spec->verify (s_sig, s_hash, keyparms);
  else
    rc = GPG_ERR_NOT_IMPLEMENTED;

 leave:
  sexp_release (keyparms);
  return rc;
}


gcry_err_code_t
_gcry_pk_verify_md (gcry_sexp_t s_sig, const char *tmpl, gcry_md_hd_t hd_orig,
                    gcry_sexp_t s_pkey, gcry_ctx_t ctx)
{
  gcry_err_code_t rc;
  gcry_pk_spec_t *spec;
  gcry_sexp_t keyparms = NULL;
  gcry_sexp_t s_hash = NULL;
  int algo;
  const unsigned char *digest;
  gcry_error_t err;
  gcry_md_hd_t hd;
  const char *s;
  char *hash_name;

  /* Check if it has fixed hash name or %s */
  s = strstr (tmpl, "(hash ");
  if (s == NULL)
    return GPG_ERR_DIGEST_ALGO;

  s += 6;
  if (!strncmp (s, "%s", 2))
    hash_name = NULL;
  else
    {
      const char *p;

      for (p = s; *p && *p != ' '; p++)
        ;

      hash_name = xtrymalloc (p - s + 1);
      if (!hash_name)
        return gpg_error_from_syserror ();
      memcpy (hash_name, s, p - s);
      hash_name[p - s] = 0;
    }

  err = _gcry_md_copy (&hd, hd_orig);
  if (err)
    {
      xfree (hash_name);
      return gpg_err_code (err);
    }

  if (hash_name)
    {
      algo = _gcry_md_map_name (hash_name);
      if (algo == 0
          || (fips_mode () && algo == GCRY_MD_SHA1))
        {
          xfree (hash_name);
          _gcry_md_close (hd);
          return GPG_ERR_DIGEST_ALGO;
        }

      digest = _gcry_md_read (hd, algo);
    }
  else
    {
      algo = _gcry_md_get_algo (hd);

      if (fips_mode () && algo == GCRY_MD_SHA1)
        {
          _gcry_md_close (hd);
          return GPG_ERR_DIGEST_ALGO;
        }

      digest = _gcry_md_read (hd, 0);
    }

  if (!digest)
    {
      xfree (hash_name);
      _gcry_md_close (hd);
      return GPG_ERR_DIGEST_ALGO;
    }

  if (!ctx)
    {
      if (hash_name)
        rc = _gcry_sexp_build (&s_hash, NULL, tmpl,
                               (int) _gcry_md_get_algo_dlen (algo),
                               digest);
      else
        rc = _gcry_sexp_build (&s_hash, NULL, tmpl,
                               _gcry_md_algo_name (algo),
                               (int) _gcry_md_get_algo_dlen (algo),
                               digest);
    }
  else
    {
      const unsigned char *p;
      size_t len;

      rc = _gcry_pk_get_random_override (ctx, &p, &len);
      if (rc)
        {
          _gcry_md_close (hd);
          return rc;
        }

      if (hash_name)
        rc = _gcry_sexp_build (&s_hash, NULL, tmpl,
                               (int) _gcry_md_get_algo_dlen (algo),
                               digest,
                               (int) len, p);
      else
        rc = _gcry_sexp_build (&s_hash, NULL, tmpl,
                               _gcry_md_algo_name (algo),
                               (int) _gcry_md_get_algo_dlen (algo),
                               digest,
                               (int) len, p);
    }

  xfree (hash_name);
  _gcry_md_close (hd);
  if (rc)
    return rc;

  rc = spec_from_sexp (s_pkey, 0, &spec, &keyparms);
  if (rc)
    goto leave;

  if (spec->flags.disabled)
    rc = GPG_ERR_PUBKEY_ALGO;
  else if (!spec->flags.fips && fips_mode ())
    rc = GPG_ERR_PUBKEY_ALGO;
  else if (spec->verify)
    rc = spec->verify (s_sig, s_hash, keyparms);
  else
    rc = GPG_ERR_NOT_IMPLEMENTED;

 leave:
  sexp_release (s_hash);
  sexp_release (keyparms);
  return rc;
}


/*
   Test a key.

   This may be used either for a public or a secret key to see whether
   the internal structure is okay.

   Returns: 0 or an errorcode.

   NOTE: We currently support only secret key checking. */
gcry_err_code_t
_gcry_pk_testkey (gcry_sexp_t s_key)
{
  gcry_err_code_t rc;
  gcry_pk_spec_t *spec;
  gcry_sexp_t keyparms;

  rc = spec_from_sexp (s_key, 1, &spec, &keyparms);
  if (rc)
    goto leave;

  if (spec->flags.disabled)
    rc = GPG_ERR_PUBKEY_ALGO;
  else if (!spec->flags.fips && fips_mode ())
    rc = GPG_ERR_PUBKEY_ALGO;
  else if (spec->check_secret_key)
    rc = spec->check_secret_key (keyparms);
  else
    rc = GPG_ERR_NOT_IMPLEMENTED;

 leave:
  sexp_release (keyparms);
  return rc;
}


/*
  Create a public key pair and return it in r_key.
  How the key is created depends on s_parms:
  (genkey
   (algo
     (parameter_name_1 ....)
      ....
     (parameter_name_n ....)
  ))
  The key is returned in a format depending on the
  algorithm. Both, private and secret keys are returned
  and optionally some additional informatin.
  For elgamal we return this structure:
  (key-data
   (public-key
     (elg
 	(p <mpi>)
 	(g <mpi>)
 	(y <mpi>)
     )
   )
   (private-key
     (elg
 	(p <mpi>)
 	(g <mpi>)
 	(y <mpi>)
 	(x <mpi>)
     )
   )
   (misc-key-info
      (pm1-factors n1 n2 ... nn)
   ))
 */
gcry_err_code_t
_gcry_pk_genkey (gcry_sexp_t *r_key, gcry_sexp_t s_parms)
{
  gcry_pk_spec_t *spec = NULL;
  gcry_sexp_t list = NULL;
  gcry_sexp_t l2 = NULL;
  char *name = NULL;
  gcry_err_code_t rc;

  *r_key = NULL;

  list = sexp_find_token (s_parms, "genkey", 0);
  if (!list)
    {
      rc = GPG_ERR_INV_OBJ; /* Does not contain genkey data. */
      goto leave;
    }

  l2 = sexp_cadr (list);
  sexp_release (list);
  list = l2;
  l2 = NULL;
  if (! list)
    {
      rc = GPG_ERR_NO_OBJ; /* No cdr for the genkey. */
      goto leave;
    }

  name = _gcry_sexp_nth_string (list, 0);
  if (!name)
    {
      rc = GPG_ERR_INV_OBJ; /* Algo string missing.  */
      goto leave;
    }

  spec = spec_from_name (name);
  xfree (name);
  name = NULL;
  if (!spec || spec->flags.disabled || (!spec->flags.fips && fips_mode ()))
    {
      rc = GPG_ERR_PUBKEY_ALGO; /* Unknown algorithm.  */
      goto leave;
    }

  if (spec->generate)
    rc = spec->generate (list, r_key);
  else
    rc = GPG_ERR_NOT_IMPLEMENTED;

 leave:
  sexp_release (list);
  xfree (name);
  sexp_release (l2);

  return rc;
}


/*
   Get the number of nbits from the public key.

   Hmmm: Should we have really this function or is it better to have a
   more general function to retrieve different properties of the key?  */
unsigned int
_gcry_pk_get_nbits (gcry_sexp_t key)
{
  gcry_pk_spec_t *spec;
  gcry_sexp_t parms;
  unsigned int nbits;

  /* Parsing KEY might be considered too much overhead.  For example
     for RSA we would only need to look at P and stop parsing right
     away.  However, with ECC things are more complicate in that only
     a curve name might be specified.  Thus we need to tear the sexp
     apart. */

  if (spec_from_sexp (key, 0, &spec, &parms))
    return 0; /* Error - 0 is a suitable indication for that.  */
  if (spec->flags.disabled)
    return 0;
  if (!spec->flags.fips && fips_mode ())
    return 0;

  nbits = spec->get_nbits (parms);
  sexp_release (parms);
  return nbits;
}


/* Return the so called KEYGRIP which is the SHA-1 hash of the public
   key parameters expressed in a way depending on the algorithm.

   ARRAY must either be 20 bytes long or NULL; in the latter case a
   newly allocated array of that size is returned, otherwise ARRAY or
   NULL is returned to indicate an error which is most likely an
   unknown algorithm.  The function accepts public or secret keys. */
unsigned char *
_gcry_pk_get_keygrip (gcry_sexp_t key, unsigned char *array)
{
  gcry_sexp_t list = NULL;
  gcry_sexp_t l2 = NULL;
  gcry_pk_spec_t *spec = NULL;
  const char *s;
  char *name = NULL;
  int idx;
  const char *elems;
  gcry_md_hd_t md = NULL;
  int okay = 0;

  /* Check that the first element is valid. */
  list = sexp_find_token (key, "public-key", 0);
  if (! list)
    list = sexp_find_token (key, "private-key", 0);
  if (! list)
    list = sexp_find_token (key, "protected-private-key", 0);
  if (! list)
    list = sexp_find_token (key, "shadowed-private-key", 0);
  if (! list)
    return NULL; /* No public- or private-key object. */

  l2 = sexp_cadr (list);
  sexp_release (list);
  list = l2;
  l2 = NULL;

  name = _gcry_sexp_nth_string (list, 0);
  if (!name)
    goto fail; /* Invalid structure of object. */

  spec = spec_from_name (name);
  if (!spec)
    goto fail; /* Unknown algorithm.  */

  elems = spec->elements_grip;
  if (!elems)
    goto fail; /* No grip parameter.  */

  if (_gcry_md_open (&md, GCRY_MD_SHA1, 0))
    goto fail;

  if (spec->comp_keygrip)
    {
      /* Module specific method to compute a keygrip.  */
      if (spec->comp_keygrip (md, list))
        goto fail;
    }
  else
    {
      /* Generic method to compute a keygrip.  */
      for (idx = 0, s = elems; *s; s++, idx++)
        {
          const char *data;
          size_t datalen;
          char buf[30];

          l2 = sexp_find_token (list, s, 1);
          if (! l2)
            goto fail;
          data = sexp_nth_data (l2, 1, &datalen);
          if (! data)
            goto fail;

          snprintf (buf, sizeof buf, "(1:%c%u:", *s, (unsigned int)datalen);
          _gcry_md_write (md, buf, strlen (buf));
          _gcry_md_write (md, data, datalen);
          sexp_release (l2);
          l2 = NULL;
          _gcry_md_write (md, ")", 1);
        }
    }

  if (!array)
    {
      array = xtrymalloc (20);
      if (! array)
        goto fail;
    }

  memcpy (array, _gcry_md_read (md, GCRY_MD_SHA1), 20);
  okay = 1;

 fail:
  xfree (name);
  sexp_release (l2);
  _gcry_md_close (md);
  sexp_release (list);
  return okay? array : NULL;
}



const char *
_gcry_pk_get_curve (gcry_sexp_t key, int iterator, unsigned int *r_nbits)
{
  const char *result = NULL;
  gcry_pk_spec_t *spec;
  gcry_sexp_t keyparms = NULL;

  if (r_nbits)
    *r_nbits = 0;

  if (key)
    {
      iterator = 0;

      if (spec_from_sexp (key, 0, &spec, &keyparms))
        return NULL;
    }
  else
    {
      spec = spec_from_name ("ecc");
      if (!spec)
        return NULL;
    }

  if (spec->flags.disabled)
    return NULL;
  if (!spec->flags.fips && fips_mode ())
    return NULL;
  if (spec->get_curve)
    result = spec->get_curve (keyparms, iterator, r_nbits);

  sexp_release (keyparms);
  return result;
}



gcry_sexp_t
_gcry_pk_get_param (int algo, const char *name)
{
  gcry_sexp_t result = NULL;
  gcry_pk_spec_t *spec = NULL;

  algo = map_algo (algo);

  if (algo != GCRY_PK_ECC)
    return NULL;

  spec = spec_from_name ("ecc");
  if (spec)
    {
      if (spec && spec->get_curve_param)
        result = spec->get_curve_param (name);
    }
  return result;
}



gcry_err_code_t
_gcry_pk_ctl (int cmd, void *buffer, size_t buflen)
{
  gcry_err_code_t rc = 0;

  switch (cmd)
    {
    case GCRYCTL_DISABLE_ALGO:
      /* This one expects a buffer pointing to an integer with the
         algo number.  */
      if ((! buffer) || (buflen != sizeof (int)))
	rc = GPG_ERR_INV_ARG;
      else
	disable_pubkey_algo (*((int *) buffer));
      break;

    default:
      rc = GPG_ERR_INV_OP;
    }

  return rc;
}


/* Return information about the given algorithm

   WHAT selects the kind of information returned:

    GCRYCTL_TEST_ALGO:
        Returns 0 when the specified algorithm is available for use.
        Buffer must be NULL, nbytes  may have the address of a variable
        with the required usage of the algorithm. It may be 0 for don't
        care or a combination of the GCRY_PK_USAGE_xxx flags;

    GCRYCTL_GET_ALGO_USAGE:
        Return the usage flags for the given algo.  An invalid algo
        returns 0.  Disabled algos are ignored here because we
        only want to know whether the algo is at all capable of
        the usage.

   Note: Because this function is in most cases used to return an
   integer value, we can make it easier for the caller to just look at
   the return value.  The caller will in all cases consult the value
   and thereby detecting whether a error occurred or not (i.e. while
   checking the block size) */
gcry_err_code_t
_gcry_pk_algo_info (int algorithm, int what, void *buffer, size_t *nbytes)
{
  gcry_err_code_t rc = 0;

  switch (what)
    {
    case GCRYCTL_TEST_ALGO:
      {
	int use = nbytes ? *nbytes : 0;
	if (buffer)
	  rc = GPG_ERR_INV_ARG;
	else if (check_pubkey_algo (algorithm, use))
	  rc = GPG_ERR_PUBKEY_ALGO;
	break;
      }

    case GCRYCTL_GET_ALGO_USAGE:
      {
	gcry_pk_spec_t *spec;

	spec = spec_from_algo (algorithm);
        *nbytes = spec? spec->use : 0;
	break;
      }

    case GCRYCTL_GET_ALGO_NPKEY:
      {
	/* FIXME?  */
	int npkey = pubkey_get_npkey (algorithm);
	*nbytes = npkey;
	break;
      }
    case GCRYCTL_GET_ALGO_NSKEY:
      {
	/* FIXME?  */
	int nskey = pubkey_get_nskey (algorithm);
	*nbytes = nskey;
	break;
      }
    case GCRYCTL_GET_ALGO_NSIGN:
      {
	/* FIXME?  */
	int nsign = pubkey_get_nsig (algorithm);
	*nbytes = nsign;
	break;
      }
    case GCRYCTL_GET_ALGO_NENCR:
      {
	/* FIXME?  */
	int nencr = pubkey_get_nenc (algorithm);
	*nbytes = nencr;
	break;
      }

    default:
      rc = GPG_ERR_INV_OP;
    }

  return rc;
}


/* Return an S-expression representing the context CTX.  Depending on
   the state of that context, the S-expression may either be a public
   key, a private key or any other object used with public key
   operations.  On success a new S-expression is stored at R_SEXP and
   0 is returned, on error NULL is store there and an error code is
   returned.  MODE is either 0 or one of the GCRY_PK_GET_xxx values.

   As of now it only support certain ECC operations because a context
   object is right now only defined for ECC.  Over time this function
   will be extended to cover more algorithms.  Note also that the name
   of the function is gcry_pubkey_xxx and not gcry_pk_xxx.  The idea
   is that we will eventually provide variants of the existing
   gcry_pk_xxx functions which will take a context parameter.   */
gcry_err_code_t
_gcry_pubkey_get_sexp (gcry_sexp_t *r_sexp, int mode, gcry_ctx_t ctx)
{
  mpi_ec_t ec;

  if (!r_sexp)
    return GPG_ERR_INV_VALUE;
  *r_sexp = NULL;
  switch (mode)
    {
    case 0:
    case GCRY_PK_GET_PUBKEY:
    case GCRY_PK_GET_SECKEY:
      break;
    default:
      return GPG_ERR_INV_VALUE;
    }
  if (!ctx)
    return GPG_ERR_NO_CRYPT_CTX;

  ec = _gcry_ctx_find_pointer (ctx, CONTEXT_TYPE_EC);
  if (ec)
    return _gcry_pk_ecc_get_sexp (r_sexp, mode, ec);

  return GPG_ERR_WRONG_CRYPT_CTX;
}



/* Explicitly initialize this module.  */
gcry_err_code_t
_gcry_pk_init (void)
{
  return 0;
}


/* Run the selftests for pubkey algorithm ALGO with optional reporting
   function REPORT.  */
gpg_error_t
_gcry_pk_selftest (int algo, int extended, selftest_report_func_t report)
{
  gcry_err_code_t ec;
  gcry_pk_spec_t *spec;

  algo = map_algo (algo);
  spec = spec_from_algo (algo);
  if (spec && !spec->flags.disabled
      && (spec->flags.fips || !fips_mode ())
      && spec->selftest)
    ec = spec->selftest (algo, extended, report);
  else
    {
      ec = GPG_ERR_PUBKEY_ALGO;
      /* Fixme: We need to change the report function to allow passing
         of an encryption mode (e.g. pkcs1, ecdsa, or ecdh).  */
      if (report)
        report ("pubkey", algo, "module",
                spec && !spec->flags.disabled
                && (spec->flags.fips || !fips_mode ())?
                "no selftest available" :
                spec? "algorithm disabled" :
                "algorithm not found");
    }

  return gpg_error (ec);
}


struct pk_random_override {
  size_t len;
  unsigned char area[];
};

gpg_err_code_t
_gcry_pk_random_override_new (gcry_ctx_t *r_ctx,
                              const unsigned char *p, size_t len)
{
  gcry_ctx_t ctx;
  struct pk_random_override *pro;

  *r_ctx = NULL;
  if (!p)
    return GPG_ERR_EINVAL;

  ctx = _gcry_ctx_alloc (CONTEXT_TYPE_RANDOM_OVERRIDE,
                         sizeof (size_t) + len, NULL);
  if (!ctx)
    return gpg_err_code_from_syserror ();
  pro = _gcry_ctx_get_pointer (ctx, CONTEXT_TYPE_RANDOM_OVERRIDE);
  pro->len = len;
  memcpy (pro->area, p, len);

  *r_ctx = ctx;
  return 0;
}

gpg_err_code_t
_gcry_pk_get_random_override (gcry_ctx_t ctx,
                              const unsigned char **r_p, size_t *r_len)
{
  struct pk_random_override *pro;

  pro = _gcry_ctx_find_pointer (ctx, CONTEXT_TYPE_RANDOM_OVERRIDE);
  if (!pro)
    return GPG_ERR_EINVAL;

  *r_p = pro->area;
  *r_len = pro->len;

  return 0;
}
