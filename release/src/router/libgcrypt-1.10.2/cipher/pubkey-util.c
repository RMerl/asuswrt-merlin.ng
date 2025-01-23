/* pubkey-util.c - Supporting functions for all pubkey modules.
 * Copyright (C) 1998, 1999, 2000, 2002, 2003, 2005,
 *               2007, 2008, 2011 Free Software Foundation, Inc.
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

#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "g10lib.h"
#include "mpi.h"
#include "cipher.h"
#include "pubkey-internal.h"


/* Callback for the pubkey algorithm code to verify PSS signatures.
   OPAQUE is the data provided by the actual caller.  The meaning of
   TMP depends on the actual algorithm (but there is only RSA); now
   for RSA it is the output of running the public key function on the
   input.  */
static int
pss_verify_cmp (void *opaque, gcry_mpi_t tmp)
{
  struct pk_encoding_ctx *ctx = opaque;
  gcry_mpi_t value = ctx->verify_arg;

  return _gcry_rsa_pss_verify (value, !(ctx->flags & PUBKEY_FLAG_PREHASH),
                               tmp, ctx->nbits - 1,
                               ctx->hash_algo, ctx->saltlen);
}


/* Parser for a flag list.  On return the encoding is stored at
   R_ENCODING and the flags are stored at R_FLAGS.  If any of them is
   not needed, NULL may be passed.  The function returns 0 on success
   or an error code. */
gpg_err_code_t
_gcry_pk_util_parse_flaglist (gcry_sexp_t list,
                              int *r_flags, enum pk_encoding *r_encoding)
{
  gpg_err_code_t rc = 0;
  const char *s;
  size_t n;
  int i;
  int encoding = PUBKEY_ENC_UNKNOWN;
  int flags = 0;
  int igninvflag = 0;

  for (i = list ? sexp_length (list)-1 : 0; i > 0; i--)
    {
      s = sexp_nth_data (list, i, &n);
      if (!s)
        continue; /* Not a data element. */

      switch (n)
        {
        case 3:
          if (!memcmp (s, "pss", 3) && encoding == PUBKEY_ENC_UNKNOWN)
            {
              encoding = PUBKEY_ENC_PSS;
              flags |= PUBKEY_FLAG_FIXEDLEN;
            }
          else if (!memcmp (s, "raw", 3) && encoding == PUBKEY_ENC_UNKNOWN)
            {
              encoding = PUBKEY_ENC_RAW;
              flags |= PUBKEY_FLAG_RAW_FLAG; /* Explicitly given.  */
            }
          else if (!memcmp (s, "sm2", 3))
            {
                encoding = PUBKEY_ENC_RAW;
                flags |= PUBKEY_FLAG_SM2 | PUBKEY_FLAG_RAW_FLAG;
            }
          else if (!igninvflag)
            rc = GPG_ERR_INV_FLAG;
          break;

        case 4:
          if (!memcmp (s, "comp", 4))
            flags |= PUBKEY_FLAG_COMP;
          else if (!memcmp (s, "oaep", 4) && encoding == PUBKEY_ENC_UNKNOWN)
            {
              encoding = PUBKEY_ENC_OAEP;
              flags |= PUBKEY_FLAG_FIXEDLEN;
            }
          else if (!memcmp (s, "gost", 4))
            {
              encoding = PUBKEY_ENC_RAW;
              flags |= PUBKEY_FLAG_GOST;
            }
          else if (!igninvflag)
            rc = GPG_ERR_INV_FLAG;
          break;

        case 5:
          if (!memcmp (s, "eddsa", 5))
            {
              encoding = PUBKEY_ENC_RAW;
              flags |= PUBKEY_FLAG_EDDSA;
              flags |= PUBKEY_FLAG_DJB_TWEAK;
            }
          else if (!memcmp (s, "pkcs1", 5) && encoding == PUBKEY_ENC_UNKNOWN)
            {
              encoding = PUBKEY_ENC_PKCS1;
              flags |= PUBKEY_FLAG_FIXEDLEN;
            }
          else if (!memcmp (s, "param", 5))
            flags |= PUBKEY_FLAG_PARAM;
          else if (!igninvflag)
            rc = GPG_ERR_INV_FLAG;
          break;

        case 6:
          if (!memcmp (s, "nocomp", 6))
            flags |= PUBKEY_FLAG_NOCOMP;
          else if (!igninvflag)
            rc = GPG_ERR_INV_FLAG;
          break;

        case 7:
          if (!memcmp (s, "rfc6979", 7))
            flags |= PUBKEY_FLAG_RFC6979;
          else if (!memcmp (s, "noparam", 7))
            ; /* Ignore - it is the default.  */
          else if (!memcmp (s, "prehash", 7))
            flags |= PUBKEY_FLAG_PREHASH;
          else if (!igninvflag)
            rc = GPG_ERR_INV_FLAG;
          break;

        case 8:
          if (!memcmp (s, "use-x931", 8))
            flags |= PUBKEY_FLAG_USE_X931;
          else if (!igninvflag)
            rc = GPG_ERR_INV_FLAG;
          break;

        case 9:
          if (!memcmp (s, "pkcs1-raw", 9) && encoding == PUBKEY_ENC_UNKNOWN)
            {
              encoding = PUBKEY_ENC_PKCS1_RAW;
              flags |= PUBKEY_FLAG_FIXEDLEN;
            }
          else if (!memcmp (s, "djb-tweak", 9))
            {
              encoding = PUBKEY_ENC_RAW;
              flags |= PUBKEY_FLAG_DJB_TWEAK;
            }
          else if (!igninvflag)
            rc = GPG_ERR_INV_FLAG;
          break;

        case 10:
          if (!memcmp (s, "igninvflag", 10))
            igninvflag = 1;
          else if (!memcmp (s, "no-keytest", 10))
            flags |= PUBKEY_FLAG_NO_KEYTEST;
          else if (!igninvflag)
            rc = GPG_ERR_INV_FLAG;
          break;

        case 11:
          if (!memcmp (s, "no-blinding", 11))
            flags |= PUBKEY_FLAG_NO_BLINDING;
          else if (!memcmp (s, "use-fips186", 11))
            flags |= PUBKEY_FLAG_USE_FIPS186;
          else if (!igninvflag)
            rc = GPG_ERR_INV_FLAG;
          break;

        case 13:
          if (!memcmp (s, "use-fips186-2", 13))
            flags |= PUBKEY_FLAG_USE_FIPS186_2;
          else if (!memcmp (s, "transient-key", 13))
            flags |= PUBKEY_FLAG_TRANSIENT_KEY;
          else if (!igninvflag)
            rc = GPG_ERR_INV_FLAG;
          break;

        default:
          if (!igninvflag)
            rc = GPG_ERR_INV_FLAG;
          break;
        }
    }

  if (r_flags)
    *r_flags = flags;
  if (r_encoding)
    *r_encoding = encoding;

  return rc;
}


static int
get_hash_algo (const char *s, size_t n)
{
  static const struct { const char *name; int algo; } hashnames[] = {
    { "sha1",   GCRY_MD_SHA1 },
    { "md5",    GCRY_MD_MD5 },
    { "sha256", GCRY_MD_SHA256 },
    { "ripemd160", GCRY_MD_RMD160 },
    { "rmd160", GCRY_MD_RMD160 },
    { "sha384", GCRY_MD_SHA384 },
    { "sha512", GCRY_MD_SHA512 },
    { "sha224", GCRY_MD_SHA224 },
    { "md2",    GCRY_MD_MD2 },
    { "md4",    GCRY_MD_MD4 },
    { "tiger",  GCRY_MD_TIGER },
    { "haval",  GCRY_MD_HAVAL },
    { "sha3-224", GCRY_MD_SHA3_224 },
    { "sha3-256", GCRY_MD_SHA3_256 },
    { "sha3-384", GCRY_MD_SHA3_384 },
    { "sha3-512", GCRY_MD_SHA3_512 },
    { "sm3", GCRY_MD_SM3 },
    { "shake128", GCRY_MD_SHAKE128 },
    { "shake256", GCRY_MD_SHAKE256 },
    { "sha512-224", GCRY_MD_SHA512_224 },
    { "sha512-256", GCRY_MD_SHA512_256 },
    { NULL, 0 }
  };
  int algo;
  int i;

  for (i=0; hashnames[i].name; i++)
    {
      if ( strlen (hashnames[i].name) == n
	   && !memcmp (hashnames[i].name, s, n))
	break;
    }
  if (hashnames[i].name)
    algo = hashnames[i].algo;
  else
    {
      /* In case of not listed or dynamically allocated hash
	 algorithm we fall back to this somewhat slower
	 method.  Further, it also allows to use OIDs as
	 algorithm names. */
      char *tmpname;

      tmpname = xtrymalloc (n+1);
      if (!tmpname)
	algo = 0;  /* Out of core - silently give up.  */
      else
	{
	  memcpy (tmpname, s, n);
	  tmpname[n] = 0;
	  algo = _gcry_md_map_name (tmpname);
	  xfree (tmpname);
	}
    }
  return algo;
}


/* Get the "nbits" parameter from an s-expression of the format:
 *
 *   (algo
 *     (parameter_name_1 ....)
 *      ....
 *     (parameter_name_n ....))
 *
 * Example:
 *
 *   (rsa
 *     (nbits 4:2048))
 *
 * On success the value for nbits is stored at R_NBITS.  If no nbits
 * parameter is found, the function returns success and stores 0 at
 * R_NBITS.  For parsing errors the function returns an error code and
 * stores 0 at R_NBITS.
 */
gpg_err_code_t
_gcry_pk_util_get_nbits (gcry_sexp_t list, unsigned int *r_nbits)
{
  char buf[50];
  const char *s;
  size_t n;

  *r_nbits = 0;

  list = sexp_find_token (list, "nbits", 0);
  if (!list)
    return 0; /* No NBITS found.  */

  s = sexp_nth_data (list, 1, &n);
  if (!s || n >= DIM (buf) - 1 )
    {
      /* NBITS given without a cdr.  */
      sexp_release (list);
      return GPG_ERR_INV_OBJ;
    }
  memcpy (buf, s, n);
  buf[n] = 0;
  *r_nbits = (unsigned int)strtoul (buf, NULL, 0);
  sexp_release (list);
  return 0;
}


/* Get the optional "rsa-use-e" parameter from an s-expression of the
 * format:
 *
 *   (algo
 *     (parameter_name_1 ....)
 *      ....
 *     (parameter_name_n ....))
 *
 * Example:
 *
 *   (rsa
 *     (nbits 4:2048)
 *     (rsa-use-e 2:41))
 *
 * On success the value for nbits is stored at R_E.  If no rsa-use-e
 * parameter is found, the function returns success and stores 65537 at
 * R_E.  For parsing errors the function returns an error code and
 * stores 0 at R_E.
 */
gpg_err_code_t
_gcry_pk_util_get_rsa_use_e (gcry_sexp_t list, unsigned long *r_e)
{
  char buf[50];
  const char *s;
  size_t n;

  *r_e = 0;

  list = sexp_find_token (list, "rsa-use-e", 0);
  if (!list)
    {
      *r_e = 65537; /* Not given, use the value generated by old versions. */
      return 0;
    }

  s = sexp_nth_data (list, 1, &n);
  if (!s || n >= DIM (buf) - 1 )
    {
      /* No value or value too large.  */
      sexp_release (list);
      return GPG_ERR_INV_OBJ;
    }
  memcpy (buf, s, n);
  buf[n] = 0;
  *r_e = strtoul (buf, NULL, 0);
  sexp_release (list);
  return 0;
}


/* Parse a "sig-val" s-expression and store the inner parameter list at
   R_PARMS.  ALGO_NAMES is used to verify that the algorithm in
   "sig-val" is valid.  Returns 0 on success and stores a new list at
   R_PARMS which must be freed by the caller.  On error R_PARMS is set
   to NULL and an error code returned.  If R_ECCFLAGS is not NULL flag
   values are set into it; as of now they are only used with ecc
   algorithms.  */
gpg_err_code_t
_gcry_pk_util_preparse_sigval (gcry_sexp_t s_sig, const char **algo_names,
                               gcry_sexp_t *r_parms, int *r_eccflags)
{
  gpg_err_code_t rc;
  gcry_sexp_t l1 = NULL;
  gcry_sexp_t l2 = NULL;
  char *name = NULL;
  int i;

  *r_parms = NULL;
  if (r_eccflags)
    *r_eccflags = 0;

  /* Extract the signature value.  */
  l1 = sexp_find_token (s_sig, "sig-val", 0);
  if (!l1)
    {
      rc = GPG_ERR_INV_OBJ; /* Does not contain a signature value object.  */
      goto leave;
    }

  l2 = sexp_nth (l1, 1);
  if (!l2)
    {
      rc = GPG_ERR_NO_OBJ;   /* No cadr for the sig object.  */
      goto leave;
    }
  name = sexp_nth_string (l2, 0);
  if (!name)
    {
      rc = GPG_ERR_INV_OBJ;  /* Invalid structure of object.  */
      goto leave;
    }
  else if (!strcmp (name, "flags"))
    {
      /* Skip a "flags" parameter and look again for the algorithm
	 name.  This is not used but here just for the sake of
	 consistent S-expressions we need to handle it. */
      sexp_release (l2);
      l2 = sexp_nth (l1, 2);
      if (!l2)
	{
	  rc = GPG_ERR_INV_OBJ;
          goto leave;
	}
      xfree (name);
      name = sexp_nth_string (l2, 0);
      if (!name)
        {
          rc = GPG_ERR_INV_OBJ;  /* Invalid structure of object.  */
          goto leave;
        }
    }

  for (i=0; algo_names[i]; i++)
    if (!stricmp (name, algo_names[i]))
      break;
  if (!algo_names[i])
    {
      rc = GPG_ERR_CONFLICT; /* "sig-val" uses an unexpected algo. */
      goto leave;
    }
  if (r_eccflags)
    {
      if (!strcmp (name, "eddsa"))
        *r_eccflags = PUBKEY_FLAG_EDDSA;
      if (!strcmp (name, "gost"))
        *r_eccflags = PUBKEY_FLAG_GOST;
      if (!strcmp (name, "sm2"))
        *r_eccflags = PUBKEY_FLAG_SM2;
    }

  *r_parms = l2;
  l2 = NULL;
  rc = 0;

 leave:
  xfree (name);
  sexp_release (l2);
  sexp_release (l1);
  return rc;
}


/* Parse a "enc-val" s-expression and store the inner parameter list
   at R_PARMS.  ALGO_NAMES is used to verify that the algorithm in
   "enc-val" is valid.  Returns 0 on success and stores a new list at
   R_PARMS which must be freed by the caller.  On error R_PARMS is set
   to NULL and an error code returned.  If R_ECCFLAGS is not NULL flag
   values are set into it; as of now they are only used with ecc
   algorithms.

     (enc-val
       [(flags [raw, pkcs1, oaep, no-blinding])]
       [(hash-algo <algo>)]
       [(label <label>)]
        (<algo>
          (<param_name1> <mpi>)
          ...
          (<param_namen> <mpi>)))

   HASH-ALGO and LABEL are specific to OAEP.  CTX will be updated with
   encoding information.  */
gpg_err_code_t
_gcry_pk_util_preparse_encval (gcry_sexp_t sexp, const char **algo_names,
                               gcry_sexp_t *r_parms,
                               struct pk_encoding_ctx *ctx)
{
  gcry_err_code_t rc = 0;
  gcry_sexp_t l1 = NULL;
  gcry_sexp_t l2 = NULL;
  char *name = NULL;
  size_t n;
  int parsed_flags = 0;
  int i;

  *r_parms = NULL;

  /* Check that the first element is valid.  */
  l1 = sexp_find_token (sexp, "enc-val" , 0);
  if (!l1)
    {
      rc = GPG_ERR_INV_OBJ; /* Does not contain an encrypted value object.  */
      goto leave;
    }

  l2 = sexp_nth (l1, 1);
  if (!l2)
    {
      rc = GPG_ERR_NO_OBJ;  /* No cadr for the data object.  */
      goto leave;
    }

  /* Extract identifier of sublist.  */
  name = sexp_nth_string (l2, 0);
  if (!name)
    {
      rc = GPG_ERR_INV_OBJ; /* Invalid structure of object.  */
      goto leave;
    }

  if (!strcmp (name, "flags"))
    {
      const char *s;

      /* There is a flags element - process it.  */
      rc = _gcry_pk_util_parse_flaglist (l2, &parsed_flags, &ctx->encoding);
      if (rc)
        goto leave;
      if (ctx->encoding == PUBKEY_ENC_PSS)
        {
          rc = GPG_ERR_CONFLICT;
          goto leave;
        }

      /* Get the OAEP parameters HASH-ALGO and LABEL, if any. */
      if (ctx->encoding == PUBKEY_ENC_OAEP)
	{
	  /* Get HASH-ALGO. */
          sexp_release (l2);
	  l2 = sexp_find_token (l1, "hash-algo", 0);
	  if (l2)
	    {
	      s = sexp_nth_data (l2, 1, &n);
	      if (!s)
		rc = GPG_ERR_NO_OBJ;
	      else
		{
		  ctx->hash_algo = get_hash_algo (s, n);
		  if (!ctx->hash_algo)
		    rc = GPG_ERR_DIGEST_ALGO;
		}
	      if (rc)
		goto leave;
	    }

	  /* Get LABEL. */
          sexp_release (l2);
	  l2 = sexp_find_token (l1, "label", 0);
	  if (l2)
	    {
	      s = sexp_nth_data (l2, 1, &n);
	      if (!s)
		rc = GPG_ERR_NO_OBJ;
	      else if (n > 0)
		{
		  ctx->label = xtrymalloc (n);
		  if (!ctx->label)
		    rc = gpg_err_code_from_syserror ();
		  else
		    {
		      memcpy (ctx->label, s, n);
		      ctx->labellen = n;
		    }
		}
	      if (rc)
		goto leave;
	    }
	}

      /* Get the next which has the actual data - skip HASH-ALGO and LABEL. */
      for (i = 2; (sexp_release (l2), l2 = sexp_nth (l1, i)); i++)
	{
	  s = sexp_nth_data (l2, 0, &n);
	  if (!(n == 9 && !memcmp (s, "hash-algo", 9))
	      && !(n == 5 && !memcmp (s, "label", 5))
	      && !(n == 15 && !memcmp (s, "random-override", 15)))
	    break;
	}
      if (!l2)
        {
          rc = GPG_ERR_NO_OBJ; /* No cadr for the data object. */
          goto leave;
        }

      /* Extract sublist identifier.  */
      xfree (name);
      name = sexp_nth_string (l2, 0);
      if (!name)
        {
          rc = GPG_ERR_INV_OBJ; /* Invalid structure of object. */
          goto leave;
        }
    }
  else /* No flags - flag as legacy structure.  */
    parsed_flags |= PUBKEY_FLAG_LEGACYRESULT;

  for (i=0; algo_names[i]; i++)
    if (!stricmp (name, algo_names[i]))
      break;
  if (!algo_names[i])
    {
      rc = GPG_ERR_CONFLICT; /* "enc-val" uses an unexpected algo. */
      goto leave;
    }

  *r_parms = l2;
  l2 = NULL;
  ctx->flags |= parsed_flags;
  rc = 0;

 leave:
  xfree (name);
  sexp_release (l2);
  sexp_release (l1);
  return rc;
}


/* Initialize an encoding context.  */
void
_gcry_pk_util_init_encoding_ctx (struct pk_encoding_ctx *ctx,
                                 enum pk_operation op,
                                 unsigned int nbits)
{
  ctx->op = op;
  ctx->nbits = nbits;
  ctx->encoding = PUBKEY_ENC_UNKNOWN;
  ctx->flags = 0;
  if (fips_mode ())
    {
      ctx->hash_algo = GCRY_MD_SHA256;
    }
  else
    {
      ctx->hash_algo = GCRY_MD_SHA1;
    }
  ctx->label = NULL;
  ctx->labellen = 0;
  ctx->saltlen = 20;
  ctx->verify_cmp = NULL;
  ctx->verify_arg = NULL;
}

/* Free a context initialzied by _gcry_pk_util_init_encoding_ctx.  */
void
_gcry_pk_util_free_encoding_ctx (struct pk_encoding_ctx *ctx)
{
  xfree (ctx->label);
}


/* Take the hash value and convert into an MPI, suitable for
   passing to the low level functions.  We currently support the
   old style way of passing just a MPI and the modern interface which
   allows to pass flags so that we can choose between raw and pkcs1
   padding - may be more padding options later.

   (<mpi>)
   or
   (data
    [(flags [raw, direct, pkcs1, oaep, pss,
             no-blinding, rfc6979, eddsa, prehash])]
    [(hash <algo> <value>)]
    [(value <text>)]
    [(hash-algo <algo>)]
    [(label <label>)]
    [(salt-length <length>)]
    [(random-override <data>)]
   )

   Either the VALUE or the HASH element must be present for use
   with signatures.  VALUE is used for encryption.

   HASH-ALGO is specific to OAEP, PSS and EDDSA.

   LABEL is specific to OAEP.

   SALT-LENGTH is for PSS, it is limited to 16384 bytes.

   RANDOM-OVERRIDE is used to replace random nonces for regression
   testing.  */
gcry_err_code_t
_gcry_pk_util_data_to_mpi (gcry_sexp_t input, gcry_mpi_t *ret_mpi,
                           struct pk_encoding_ctx *ctx)
{
  gcry_err_code_t rc = 0;
  gcry_sexp_t ldata, lhash, lvalue;
  size_t n;
  const char *s;
  int unknown_flag = 0;
  int parsed_flags = 0;

  *ret_mpi = NULL;
  ldata = sexp_find_token (input, "data", 0);
  if (!ldata)
    { /* assume old style */
      int mpifmt = (ctx->flags & PUBKEY_FLAG_RAW_FLAG) ?
        GCRYMPI_FMT_OPAQUE : GCRYMPI_FMT_STD;

      *ret_mpi = sexp_nth_mpi (input, 0, mpifmt);
      return *ret_mpi ? GPG_ERR_NO_ERROR : GPG_ERR_INV_OBJ;
    }

  /* See whether there is a flags list.  */
  {
    gcry_sexp_t lflags = sexp_find_token (ldata, "flags", 0);
    if (lflags)
      {
        if (_gcry_pk_util_parse_flaglist (lflags,
                                          &parsed_flags, &ctx->encoding))
          unknown_flag = 1;
        sexp_release (lflags);
      }
  }

  if (ctx->encoding == PUBKEY_ENC_UNKNOWN)
    ctx->encoding = PUBKEY_ENC_RAW; /* default to raw */

  /* Get HASH or MPI */
  lhash = sexp_find_token (ldata, "hash", 0);
  lvalue = lhash? NULL : sexp_find_token (ldata, "value", 0);

  if (!(!lhash ^ !lvalue))
    rc = GPG_ERR_INV_OBJ; /* none or both given */
  else if (unknown_flag)
    rc = GPG_ERR_INV_FLAG;
  else if (ctx->encoding == PUBKEY_ENC_RAW
           && ((parsed_flags & PUBKEY_FLAG_EDDSA)
               || (ctx->flags & PUBKEY_FLAG_EDDSA)))
    {
      /* Prepare for EdDSA.  */
      gcry_sexp_t list;
      void *value;
      size_t valuelen;

      if (!lvalue)
        {
          rc = GPG_ERR_INV_OBJ;
          goto leave;
        }
      /* Hash algo is determined by curve.  No hash-algo is OK.  */
      /* Get HASH-ALGO. */
      list = sexp_find_token (ldata, "hash-algo", 0);
      if (list)
        {
          s = sexp_nth_data (list, 1, &n);
          if (!s)
            rc = GPG_ERR_NO_OBJ;
          else
            {
              ctx->hash_algo = get_hash_algo (s, n);
              if (!ctx->hash_algo)
                rc = GPG_ERR_DIGEST_ALGO;
            }
          sexp_release (list);
        }
      if (rc)
        goto leave;

      /* Get LABEL. */
      list = sexp_find_token (ldata, "label", 0);
      if (list)
        {
          s = sexp_nth_data (list, 1, &n);
          if (!s)
            rc = GPG_ERR_NO_OBJ;
          else if (n > 0)
            {
              ctx->label = xtrymalloc (n);
              if (!ctx->label)
                rc = gpg_err_code_from_syserror ();
              else
                {
                  memcpy (ctx->label, s, n);
                  ctx->labellen = n;
                }
            }
          sexp_release (list);
          if (rc)
            goto leave;
        }

      /* Get VALUE.  */
      value = sexp_nth_buffer (lvalue, 1, &valuelen);
      if (!value)
        {
          /* We assume that a zero length message is meant by
             "(value)".  This is commonly used by test vectors.  Note
             that S-expression do not allow zero length items. */
          valuelen = 0;
          value = xtrymalloc (1);
          if (!value)
            rc = gpg_err_code_from_syserror ();
        }
      else if ((valuelen * 8) < valuelen)
        {
          xfree (value);
          rc = GPG_ERR_TOO_LARGE;
        }
      if (rc)
        goto leave;

      /* Note that mpi_set_opaque takes ownership of VALUE.  */
      *ret_mpi = mpi_set_opaque (NULL, value, valuelen*8);
    }
  else if (ctx->encoding == PUBKEY_ENC_RAW
           && (lhash || (lvalue && (parsed_flags & PUBKEY_FLAG_PREHASH)))
           && ((parsed_flags & PUBKEY_FLAG_RAW_FLAG)
               || (parsed_flags & PUBKEY_FLAG_RFC6979)))
    {
      void * value;
      size_t valuelen;
      gcry_sexp_t list;

      /* Raw encoding along with a hash element.  This is commonly
         used for DSA.  For better backward error compatibility we
         allow this only if either the rfc6979 flag has been given or
         the raw flags was explicitly given.  */

      if (lvalue && (parsed_flags & PUBKEY_FLAG_PREHASH))
        {
          /* Get HASH-ALGO. */
          list = sexp_find_token (ldata, "hash-algo", 0);
          if (list)
            {
              s = sexp_nth_data (list, 1, &n);
              if (!s)
                rc = GPG_ERR_NO_OBJ;
              else
                {
                  ctx->hash_algo = get_hash_algo (s, n);
                  if (!ctx->hash_algo)
                    rc = GPG_ERR_DIGEST_ALGO;
                }
              sexp_release (list);
              if (rc)
                goto leave;
            }

          /* Get optional LABEL.  */
          list = sexp_find_token (ldata, "label", 0);
          if (list)
            {
              s = sexp_nth_data (list, 1, &n);
              if (!s)
                rc = GPG_ERR_NO_OBJ;
              else if (n > 0)
                {
                  ctx->label = xtrymalloc (n);
                  if (!ctx->label)
                    rc = gpg_err_code_from_syserror ();
                  else
                    {
                      memcpy (ctx->label, s, n);
                      ctx->labellen = n;
                    }
                }
              else
                rc = GPG_ERR_INV_ARG;
              sexp_release (list);
              if (rc)
                goto leave;
            }

          if ( !(value=sexp_nth_buffer (lvalue, 1, &valuelen)) || !valuelen )
            rc = GPG_ERR_INV_OBJ;
          else if ((valuelen * 8) < valuelen)
            {
              xfree (value);
              rc = GPG_ERR_TOO_LARGE;
            }
          else
            *ret_mpi = mpi_set_opaque (NULL, value, valuelen*8);
        }
      else if (lhash)
        {
          /* Get optional LABEL.  */
          list = sexp_find_token (ldata, "label", 0);
          if (list)
            {
              s = sexp_nth_data (list, 1, &n);
              if (!s)
                rc = GPG_ERR_NO_OBJ;
              else if (n > 0)
                {
                  ctx->label = xtrymalloc (n);
                  if (!ctx->label)
                    rc = gpg_err_code_from_syserror ();
                  else
                    {
                      memcpy (ctx->label, s, n);
                      ctx->labellen = n;
                    }
                }
              else
                rc = GPG_ERR_INV_ARG;
              sexp_release (list);
              if (rc)
                goto leave;
            }

          if (sexp_length (lhash) != 3)
            rc = GPG_ERR_INV_OBJ;
          else if ( !(s=sexp_nth_data (lhash, 1, &n)) || !n )
            rc = GPG_ERR_INV_OBJ;
          else
            {
              ctx->hash_algo = get_hash_algo (s, n);

              if (!ctx->hash_algo)
                rc = GPG_ERR_DIGEST_ALGO;
              else if ( !(value=sexp_nth_buffer (lhash, 2, &valuelen))
                        || !valuelen )
                rc = GPG_ERR_INV_OBJ;
              else if ((valuelen * 8) < valuelen)
                {
                  xfree (value);
                  rc = GPG_ERR_TOO_LARGE;
                }
              else
                *ret_mpi = mpi_set_opaque (NULL, value, valuelen*8);
            }
        }
      else
        rc = GPG_ERR_CONFLICT;

      if (rc)
        goto leave;
    }
  else if (ctx->encoding == PUBKEY_ENC_RAW && lvalue)
    {
      /* RFC6979 may only be used with the a hash value and not the
         MPI based value.  */
      if (parsed_flags & PUBKEY_FLAG_RFC6979)
        {
          rc = GPG_ERR_CONFLICT;
          goto leave;
        }

      /* Get the value */
      *ret_mpi = sexp_nth_mpi (lvalue, 1, GCRYMPI_FMT_USG);
      if (!*ret_mpi)
        rc = GPG_ERR_INV_OBJ;
    }
  else if (ctx->encoding == PUBKEY_ENC_PKCS1 && lvalue
	   && ctx->op == PUBKEY_OP_ENCRYPT)
    {
      const void * value;
      size_t valuelen;
      gcry_sexp_t list;
      void *random_override = NULL;
      size_t random_override_len = 0;

      /* The RSA PKCS#1.5 encryption is no longer supported by FIPS */
      if (fips_mode ())
        rc = GPG_ERR_INV_FLAG;
      else if ( !(value=sexp_nth_data (lvalue, 1, &valuelen)) || !valuelen )
        rc = GPG_ERR_INV_OBJ;
      else
        {
          /* Get optional RANDOM-OVERRIDE.  */
          list = sexp_find_token (ldata, "random-override", 0);
          if (list)
            {
              s = sexp_nth_data (list, 1, &n);
              if (!s)
                rc = GPG_ERR_NO_OBJ;
              else if (n > 0)
                {
                  random_override = xtrymalloc (n);
                  if (!random_override)
                    rc = gpg_err_code_from_syserror ();
                  else
                    {
                      memcpy (random_override, s, n);
                      random_override_len = n;
                    }
                }
              sexp_release (list);
              if (rc)
                goto leave;
            }

          rc = _gcry_rsa_pkcs1_encode_for_enc (ret_mpi, ctx->nbits,
                                               value, valuelen,
                                               random_override,
                                               random_override_len);
          xfree (random_override);
        }
    }
  else if (ctx->encoding == PUBKEY_ENC_PKCS1
           && (lhash || (lvalue && (parsed_flags & PUBKEY_FLAG_PREHASH)))
	   && (ctx->op == PUBKEY_OP_SIGN || ctx->op == PUBKEY_OP_VERIFY))
    {
      if (lvalue && (parsed_flags & PUBKEY_FLAG_PREHASH))
        {
          void * value;
          size_t valuelen;
          gcry_sexp_t list;

          /* Get HASH-ALGO. */
          list = sexp_find_token (ldata, "hash-algo", 0);
          if (list)
            {
              s = sexp_nth_data (list, 1, &n);
              if (!s)
                rc = GPG_ERR_NO_OBJ;
              else
                {
                  ctx->hash_algo = get_hash_algo (s, n);
                  if (!ctx->hash_algo)
                    rc = GPG_ERR_DIGEST_ALGO;
                }
              sexp_release (list);
              if (rc)
                goto leave;
            }

          if ( !(value=sexp_nth_buffer (lvalue, 1, &valuelen)) || !valuelen )
            rc = GPG_ERR_INV_OBJ;
          else if ((valuelen * 8) < valuelen)
            {
              xfree (value);
              rc = GPG_ERR_TOO_LARGE;
            }
          else
            {
              void *hash;

              n = _gcry_md_get_algo_dlen (ctx->hash_algo);
              hash = xtrymalloc (n);
              if (!hash)
                rc = gpg_err_code_from_syserror ();
              else
                {
                  _gcry_md_hash_buffer (ctx->hash_algo, hash, value, valuelen);
                  rc = _gcry_rsa_pkcs1_encode_for_sig (ret_mpi, ctx->nbits,
                                                       hash, n, ctx->hash_algo);
                  xfree (hash);
                }
            }
        }
      else if (lhash)
        {
          if (sexp_length (lhash) != 3)
            rc = GPG_ERR_INV_OBJ;
          else if ( !(s=sexp_nth_data (lhash, 1, &n)) || !n )
            rc = GPG_ERR_INV_OBJ;
          else
            {
              const void * value;
              size_t valuelen;

              ctx->hash_algo = get_hash_algo (s, n);

              if (!ctx->hash_algo)
                rc = GPG_ERR_DIGEST_ALGO;
              else if ( !(value=sexp_nth_data (lhash, 2, &valuelen))
                        || !valuelen )
                rc = GPG_ERR_INV_OBJ;
              else
                rc = _gcry_rsa_pkcs1_encode_for_sig (ret_mpi, ctx->nbits,
                                                     value, valuelen,
                                                     ctx->hash_algo);
            }
        }
    }
  else if (ctx->encoding == PUBKEY_ENC_PKCS1_RAW && lvalue
	   && (ctx->op == PUBKEY_OP_SIGN || ctx->op == PUBKEY_OP_VERIFY))
    {
      const void * value;
      size_t valuelen;

      if (sexp_length (lvalue) != 2)
        rc = GPG_ERR_INV_OBJ;
      else if ( !(value=sexp_nth_data (lvalue, 1, &valuelen))
                || !valuelen )
        rc = GPG_ERR_INV_OBJ;
      else
        rc = _gcry_rsa_pkcs1_encode_raw_for_sig (ret_mpi, ctx->nbits,
                                                 value, valuelen);
    }
  else if (ctx->encoding == PUBKEY_ENC_OAEP && lvalue
	   && ctx->op == PUBKEY_OP_ENCRYPT)
    {
      const void * value;
      size_t valuelen;

      /* The RSA OAEP encryption requires some more assurances in FIPS */
      if (fips_mode ())
        rc = GPG_ERR_INV_FLAG;
      else if ( !(value=sexp_nth_data (lvalue, 1, &valuelen)) || !valuelen )
	rc = GPG_ERR_INV_OBJ;
      else
	{
	  gcry_sexp_t list;
          void *random_override = NULL;
          size_t random_override_len = 0;

	  /* Get HASH-ALGO. */
	  list = sexp_find_token (ldata, "hash-algo", 0);
	  if (list)
	    {
	      s = sexp_nth_data (list, 1, &n);
	      if (!s)
		rc = GPG_ERR_NO_OBJ;
	      else
		{
		  ctx->hash_algo = get_hash_algo (s, n);
		  if (!ctx->hash_algo)
		    rc = GPG_ERR_DIGEST_ALGO;
		}
	      sexp_release (list);
	      if (rc)
		goto leave;
	    }

	  /* Get LABEL. */
	  list = sexp_find_token (ldata, "label", 0);
	  if (list)
	    {
	      s = sexp_nth_data (list, 1, &n);
	      if (!s)
		rc = GPG_ERR_NO_OBJ;
	      else if (n > 0)
		{
		  ctx->label = xtrymalloc (n);
		  if (!ctx->label)
		    rc = gpg_err_code_from_syserror ();
		  else
		    {
		      memcpy (ctx->label, s, n);
		      ctx->labellen = n;
		    }
		}
	      sexp_release (list);
	      if (rc)
		goto leave;
	    }
          /* Get optional RANDOM-OVERRIDE.  */
          list = sexp_find_token (ldata, "random-override", 0);
          if (list)
            {
              s = sexp_nth_data (list, 1, &n);
              if (!s)
                rc = GPG_ERR_NO_OBJ;
              else if (n > 0)
                {
                  random_override = xtrymalloc (n);
                  if (!random_override)
                    rc = gpg_err_code_from_syserror ();
                  else
                    {
                      memcpy (random_override, s, n);
                      random_override_len = n;
                    }
                }
              sexp_release (list);
              if (rc)
                goto leave;
            }

	  rc = _gcry_rsa_oaep_encode (ret_mpi, ctx->nbits, ctx->hash_algo,
                                      value, valuelen,
                                      ctx->label, ctx->labellen,
                                      random_override, random_override_len);

          xfree (random_override);
	}
    }
  else if (ctx->encoding == PUBKEY_ENC_PSS && ctx->op == PUBKEY_OP_SIGN)
    {
      const void * value;
      size_t valuelen;
      gcry_sexp_t list;
      void *random_override = NULL;

      if (lvalue)
        {
          /* Get HASH-ALGO. */
          list = sexp_find_token (ldata, "hash-algo", 0);
          if (list)
            {
              s = sexp_nth_data (list, 1, &n);
              if (!s)
                rc = GPG_ERR_NO_OBJ;
              else
                {
                  ctx->hash_algo = get_hash_algo (s, n);
                  if (!ctx->hash_algo)
                    rc = GPG_ERR_DIGEST_ALGO;
                }
              sexp_release (list);
              if (rc)
                goto leave;
            }

          if ( !(value=sexp_nth_data (lvalue, 1, &valuelen)) || !valuelen )
            rc = GPG_ERR_INV_OBJ;
          ctx->flags |= PUBKEY_FLAG_PREHASH;
        }
      else if (lhash)
        {
          if (sexp_length (lhash) != 3)
            rc = GPG_ERR_INV_OBJ;
          else if ( !(s=sexp_nth_data (lhash, 1, &n)) || !n )
            rc = GPG_ERR_INV_OBJ;
          else
            {
              ctx->hash_algo = get_hash_algo (s, n);

              if (!ctx->hash_algo)
                rc = GPG_ERR_DIGEST_ALGO;
              else if ( !(value=sexp_nth_data (lhash, 2, &valuelen))
                        || !valuelen )
                rc = GPG_ERR_INV_OBJ;
            }
        }
      else
        rc = GPG_ERR_CONFLICT;

      if (rc)
        goto leave;

      /* Get SALT-LENGTH. */
      list = sexp_find_token (ldata, "salt-length", 0);
      if (list)
        {
          s = sexp_nth_data (list, 1, &n);
          if (!s)
            {
              rc = GPG_ERR_NO_OBJ;
              goto leave;
            }
          ctx->saltlen = (unsigned int)strtoul (s, NULL, 10);
          sexp_release (list);
        }

      /* Get optional RANDOM-OVERRIDE.  */
      list = sexp_find_token (ldata, "random-override", 0);
      if (list)
        {
          s = sexp_nth_data (list, 1, &n);
          if (!s)
            rc = GPG_ERR_NO_OBJ;
          else if (n == ctx->saltlen)
            {
              random_override = xtrymalloc (n);
              if (!random_override)
                rc = gpg_err_code_from_syserror ();
              else
                memcpy (random_override, s, n);
            }
          else
            rc = GPG_ERR_INV_ARG;
          sexp_release (list);
          if (rc)
            goto leave;
        }

      /* Encode the data.  (NBITS-1 is due to 8.1.1, step 1.) */
      rc = _gcry_rsa_pss_encode (ret_mpi, ctx->nbits - 1,
                                 ctx->hash_algo, ctx->saltlen,
                                 !(ctx->flags & PUBKEY_FLAG_PREHASH),
                                 value, valuelen,
                                 random_override);
      xfree (random_override);
    }
  else if (ctx->encoding == PUBKEY_ENC_PSS && ctx->op == PUBKEY_OP_VERIFY)
    {
      gcry_sexp_t list;

      if (lvalue)
        {
          /* Get HASH-ALGO. */
          list = sexp_find_token (ldata, "hash-algo", 0);
          if (list)
            {
              s = sexp_nth_data (list, 1, &n);
              if (!s)
                rc = GPG_ERR_NO_OBJ;
              else
                {
                  ctx->hash_algo = get_hash_algo (s, n);
                  if (!ctx->hash_algo)
                    rc = GPG_ERR_DIGEST_ALGO;
                }
              sexp_release (list);
              if (rc)
                goto leave;
            }

          *ret_mpi = sexp_nth_mpi (lvalue, 1, GCRYMPI_FMT_OPAQUE);
          if (!*ret_mpi)
            rc = GPG_ERR_INV_OBJ;

          ctx->flags |= PUBKEY_FLAG_PREHASH;
        }
      else if (lhash)
        {
          if (sexp_length (lhash) != 3)
            rc = GPG_ERR_INV_OBJ;
          else if ( !(s=sexp_nth_data (lhash, 1, &n)) || !n )
            rc = GPG_ERR_INV_OBJ;
          else
            {
              ctx->hash_algo = get_hash_algo (s, n);

              if (!ctx->hash_algo)
                rc = GPG_ERR_DIGEST_ALGO;
              else
                {
                  *ret_mpi = sexp_nth_mpi (lhash, 2, GCRYMPI_FMT_OPAQUE);
                  if (!*ret_mpi)
                    rc = GPG_ERR_INV_OBJ;
                }
            }
        }
      else
        rc = GPG_ERR_CONFLICT;

      if (rc)
        goto leave;

      /* Get SALT-LENGTH. */
      list = sexp_find_token (ldata, "salt-length", 0);
      if (list)
        {
          s = sexp_nth_data (list, 1, &n);
          if (!s)
            {
              rc = GPG_ERR_NO_OBJ;
              goto leave;
            }
          ctx->saltlen = (unsigned int)strtoul (s, NULL, 10);
          if (ctx->saltlen > 16384)
            rc = GPG_ERR_TOO_LARGE;
          sexp_release (list);
          if (rc)
            goto leave;
        }

      ctx->verify_cmp = pss_verify_cmp;
      ctx->verify_arg = *ret_mpi;
    }
  else
    rc = GPG_ERR_CONFLICT;

 leave:
  sexp_release (ldata);
  sexp_release (lhash);
  sexp_release (lvalue);

  if (!rc)
    ctx->flags |= parsed_flags;
  else
    {
      xfree (ctx->label);
      ctx->label = NULL;
    }

  return rc;
}
