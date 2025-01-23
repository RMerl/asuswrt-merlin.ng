/* ecc-misc.c  -  Elliptic Curve miscellaneous functions
 * Copyright (C) 2007, 2008, 2010, 2011 Free Software Foundation, Inc.
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
#include "ecc-common.h"


/*
 * Release a curve object.
 */
void
_gcry_ecc_curve_free (elliptic_curve_t *E)
{
  mpi_free (E->p); E->p = NULL;
  mpi_free (E->a); E->a = NULL;
  mpi_free (E->b);  E->b = NULL;
  _gcry_mpi_point_free_parts (&E->G);
  mpi_free (E->n);  E->n = NULL;
}


/*
 * Return a copy of a curve object.
 */
elliptic_curve_t
_gcry_ecc_curve_copy (elliptic_curve_t E)
{
  elliptic_curve_t R;

  R.model = E.model;
  R.dialect = E.dialect;
  R.name = E.name;
  R.p = mpi_copy (E.p);
  R.a = mpi_copy (E.a);
  R.b = mpi_copy (E.b);
  _gcry_mpi_point_init (&R.G);
  point_set (&R.G, &E.G);
  R.n = mpi_copy (E.n);
  R.h = E.h;

  return R;
}


/*
 * Return a description of the curve model.
 */
const char *
_gcry_ecc_model2str (enum gcry_mpi_ec_models model)
{
  const char *str = "?";
  switch (model)
    {
    case MPI_EC_WEIERSTRASS:    str = "Weierstrass"; break;
    case MPI_EC_MONTGOMERY:     str = "Montgomery";  break;
    case MPI_EC_EDWARDS:        str = "Edwards"; break;
    }
  return str;
}


/*
 * Return a description of the curve dialect.
 */
const char *
_gcry_ecc_dialect2str (enum ecc_dialects dialect)
{
  const char *str = "?";
  switch (dialect)
    {
    case ECC_DIALECT_STANDARD:  str = "Standard"; break;
    case ECC_DIALECT_ED25519:   str = "Ed25519"; break;
    case ECC_DIALECT_SAFECURVE: str = "SafeCurve"; break;
    }
  return str;
}


/* Return an uncompressed point (X,Y) in P as a malloced buffer with
 * its byte length stored at R_LENGTH.  May not be used for sensitive
 * data. */
unsigned char *
_gcry_ecc_ec2os_buf (gcry_mpi_t x, gcry_mpi_t y, gcry_mpi_t p,
                     unsigned int *r_length)
{
  gpg_err_code_t rc;
  int pbytes = (mpi_get_nbits (p)+7)/8;
  size_t n;
  unsigned char *buf, *ptr;

  buf = xmalloc ( 1 + 2*pbytes );
  *buf = 04; /* Uncompressed point.  */
  ptr = buf+1;
  rc = _gcry_mpi_print (GCRYMPI_FMT_USG, ptr, pbytes, &n, x);
  if (rc)
    log_fatal ("mpi_print failed: %s\n", gpg_strerror (rc));
  if (n < pbytes)
    {
      memmove (ptr+(pbytes-n), ptr, n);
      memset (ptr, 0, (pbytes-n));
    }
  ptr += pbytes;
  rc = _gcry_mpi_print (GCRYMPI_FMT_USG, ptr, pbytes, &n, y);
  if (rc)
    log_fatal ("mpi_print failed: %s\n", gpg_strerror (rc));
  if (n < pbytes)
    {
      memmove (ptr+(pbytes-n), ptr, n);
      memset (ptr, 0, (pbytes-n));
    }

  *r_length = 1 + 2*pbytes;
  return buf;
}


gcry_mpi_t
_gcry_ecc_ec2os (gcry_mpi_t x, gcry_mpi_t y, gcry_mpi_t p)
{
  unsigned char *buf;
  unsigned int buflen;

  buf = _gcry_ecc_ec2os_buf (x, y, p, &buflen);
  return mpi_set_opaque (NULL, buf, 8*buflen);
}

/* Convert POINT into affine coordinates using the context CTX and
   return a newly allocated MPI.  If the conversion is not possible
   NULL is returned.  This function won't print an error message.  */
gcry_mpi_t
_gcry_mpi_ec_ec2os (gcry_mpi_point_t point, mpi_ec_t ec)
{
  gcry_mpi_t g_x, g_y, result;

  g_x = mpi_new (0);
  g_y = mpi_new (0);
  if (_gcry_mpi_ec_get_affine (g_x, g_y, point, ec))
    result = NULL;
  else
    result = _gcry_ecc_ec2os (g_x, g_y, ec->p);
  mpi_free (g_x);
  mpi_free (g_y);

  return result;
}


/* Decode octet string in VALUE into RESULT, in the format defined by SEC 1.
   RESULT must have been initialized and is set on success to the
   point given by VALUE.  */
gpg_err_code_t
_gcry_ecc_sec_decodepoint  (gcry_mpi_t value, mpi_ec_t ec, mpi_point_t result)
{
  gpg_err_code_t rc;
  size_t n;
  const unsigned char *buf;
  unsigned char *buf_memory;
  gcry_mpi_t x, y;

  if (mpi_is_opaque (value))
    {
      unsigned int nbits;

      buf = mpi_get_opaque (value, &nbits);
      if (!buf)
        return GPG_ERR_INV_OBJ;
      n = (nbits + 7)/8;
      buf_memory = NULL;
    }
  else
    {
      n = (mpi_get_nbits (value)+7)/8;
      buf_memory = xmalloc (n);
      rc = _gcry_mpi_print (GCRYMPI_FMT_USG, buf_memory, n, &n, value);
      if (rc)
        {
          xfree (buf_memory);
          return rc;
        }
      buf = buf_memory;
    }

  if (n < 1)
    {
      xfree (buf_memory);
      return GPG_ERR_INV_OBJ;
    }

  if (*buf == 2 || *buf == 3)
    {
      gcry_mpi_t x3;
      gcry_mpi_t t;
      gcry_mpi_t p1_4;
      int y_bit = (*buf == 3);

      if (!mpi_test_bit (ec->p, 1))
        {
          xfree (buf_memory);
          return GPG_ERR_NOT_IMPLEMENTED; /* No support for point compression.  */
        }

      n = n - 1;
      rc = _gcry_mpi_scan (&x, GCRYMPI_FMT_USG, buf+1, n, NULL);
      xfree (buf_memory);
      if (rc)
        return rc;

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
    }
  else if (*buf == 4)
    {
      if ( ((n-1)%2) )
        {
          xfree (buf_memory);
          return GPG_ERR_INV_OBJ;
        }
      n = (n-1)/2;
      rc = _gcry_mpi_scan (&x, GCRYMPI_FMT_USG, buf+1, n, NULL);
      if (rc)
        {
          xfree (buf_memory);
          return rc;
        }
      rc = _gcry_mpi_scan (&y, GCRYMPI_FMT_USG, buf+1+n, n, NULL);
      xfree (buf_memory);
      if (rc)
        {
          mpi_free (x);
          return rc;
        }
    }
  else
    {
      xfree (buf_memory);
      return GPG_ERR_INV_OBJ;
    }

  mpi_set (result->x, x);
  mpi_set (result->y, y);
  mpi_set_ui (result->z, 1);

  mpi_free (x);
  mpi_free (y);

  return 0;
}


/* Compute the public key from the the context EC.  Obviously a
   requirement is that the secret key is available in EC.  On success
   Q is returned; on error NULL.  If Q is NULL a newly allocated point
   is returned.  If G or D are given they override the values taken
   from EC. */
mpi_point_t
_gcry_ecc_compute_public (mpi_point_t Q, mpi_ec_t ec)
{
  if (!ec->d || !ec->G || !ec->p || !ec->a)
    return NULL;
  if (ec->model == MPI_EC_EDWARDS && !ec->b)
    return NULL;

  if ((ec->dialect == ECC_DIALECT_ED25519 && (ec->flags & PUBKEY_FLAG_EDDSA))
      || (ec->model == MPI_EC_EDWARDS && ec->dialect == ECC_DIALECT_SAFECURVE))
    {
      gcry_mpi_t a;
      unsigned char *digest;

      if (_gcry_ecc_eddsa_compute_h_d (&digest, ec))
        return NULL;

      a = mpi_snew (0);
      _gcry_mpi_set_buffer (a, digest, 32, 0);
      xfree (digest);

      /* And finally the public key.  */
      if (!Q)
        Q = mpi_point_new (0);
      if (Q)
        _gcry_mpi_ec_mul_point (Q, a, ec->G, ec);
      mpi_free (a);
    }
  else
    {
      if (!Q)
        Q = mpi_point_new (0);
      if (Q)
        _gcry_mpi_ec_mul_point (Q, ec->d, ec->G, ec);
    }

  return Q;
}


gpg_err_code_t
_gcry_ecc_mont_encodepoint (gcry_mpi_t x, unsigned int nbits,
                            int with_prefix,
                            unsigned char **r_buffer, unsigned int *r_buflen)
{
  unsigned char *rawmpi;
  unsigned int rawmpilen;

  rawmpi = _gcry_mpi_get_buffer_extra (x, (nbits+7)/8,
                                       with_prefix? -1 : 0, &rawmpilen, NULL);
  if (rawmpi == NULL)
    return gpg_err_code_from_syserror ();

  if (with_prefix)
    {
      rawmpi[0] = 0x40;
      rawmpilen++;
    }

  *r_buffer = rawmpi;
  *r_buflen = rawmpilen;
  return 0;
}


gpg_err_code_t
_gcry_ecc_mont_decodepoint (gcry_mpi_t pk, mpi_ec_t ec, mpi_point_t result)
{
  unsigned char *rawmpi;
  unsigned int rawmpilen;
  unsigned int nbytes = (ec->nbits+7)/8;

  /*
   * It is not reliable to assume that the first byte of 0x40
   * means the prefix.
   *
   * For newer implementation, it is reliable since we always put
   * 0x40 for x-only coordinate.
   *
   * For data by older implementation (non-released development
   * version in 2015), there is no 0x40 prefix added.
   *
   * So, it is possible to have shorter length of data when it was
   * handled as MPI, removing preceding zeros.
   *
   * Besides, when data was parsed as MPI, we might have 0x00
   * prefix (when the MSB in the first byte is set).
   */

  if (mpi_is_opaque (pk))
    {
      const unsigned char *buf;
      unsigned char *p;

      buf = mpi_get_opaque (pk, &rawmpilen);
      if (!buf)
        return GPG_ERR_INV_OBJ;
      rawmpilen = (rawmpilen + 7)/8;

      if (rawmpilen == nbytes + 1
          && (buf[0] == 0x00 || buf[0] == 0x40))
        {
          rawmpilen--;
          buf++;
        }
      else if (rawmpilen > nbytes)
        return GPG_ERR_INV_OBJ;

      rawmpi = xtrymalloc (nbytes);
      if (!rawmpi)
        return gpg_err_code_from_syserror ();

      p = rawmpi + rawmpilen;
      while (p > rawmpi)
        *--p = *buf++;

      if (rawmpilen < nbytes)
        memset (rawmpi + nbytes - rawmpilen, 0, nbytes - rawmpilen);
    }
  else
    {
      rawmpi = _gcry_mpi_get_buffer (pk, nbytes, &rawmpilen, NULL);
      if (!rawmpi)
        return gpg_err_code_from_syserror ();
      if (rawmpilen > nbytes + BYTES_PER_MPI_LIMB)
        {
          xfree (rawmpi);
          return GPG_ERR_INV_OBJ;
        }
      /*
       * When we have the prefix (0x40 or 0x00), it comes at the end,
       * since it is taken by _gcry_mpi_get_buffer with little endian.
       * Just setting RAWMPILEN to NBYTES is enough in this case.
       * Othewise, RAWMPILEN is NBYTES already.
       */
      rawmpilen = nbytes;
    }

  if ((ec->nbits % 8))
    rawmpi[0] &= (1 << (ec->nbits % 8)) - 1;
  _gcry_mpi_set_buffer (result->x, rawmpi, rawmpilen, 0);
  xfree (rawmpi);
  mpi_set_ui (result->z, 1);

  return 0;
}
