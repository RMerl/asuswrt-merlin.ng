/* ecc-eddsa.c  -  Elliptic Curve EdDSA signatures
 * Copyright (C) 2013, 2014 g10 Code GmbH
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



void
reverse_buffer (unsigned char *buffer, unsigned int length)
{
  unsigned int tmp, i;

  for (i=0; i < length/2; i++)
    {
      tmp = buffer[i];
      buffer[i] = buffer[length-1-i];
      buffer[length-1-i] = tmp;
    }
}


/* Helper to scan a hex string. */
static gcry_mpi_t
scanval (const char *string)
{
  gpg_err_code_t rc;
  gcry_mpi_t val;

  rc = _gcry_mpi_scan (&val, GCRYMPI_FMT_HEX, string, 0, NULL);
  if (rc)
    log_fatal ("scanning ECC parameter failed: %s\n", gpg_strerror (rc));
  return val;
}



/* Encode MPI using the EdDSA scheme.  MINLEN specifies the required
   length of the buffer in bytes.  On success 0 is returned an a
   malloced buffer with the encoded point is stored at R_BUFFER; the
   length of this buffer is stored at R_BUFLEN.  */
static gpg_err_code_t
eddsa_encodempi (gcry_mpi_t mpi, unsigned int nbits,
                 unsigned char **r_buffer, unsigned int *r_buflen)
{
  unsigned char *rawmpi;
  unsigned int rawmpilen;
  unsigned int minlen = (nbits%8) == 0 ? (nbits/8 + 1): (nbits+7)/8;

  rawmpi = _gcry_mpi_get_buffer (mpi, minlen, &rawmpilen, NULL);
  if (!rawmpi)
    return gpg_err_code_from_syserror ();

  *r_buffer = rawmpi;
  *r_buflen = rawmpilen;
  return 0;
}


/* Encode (X,Y) using the EdDSA scheme.  NBITS is the number of bits
   of the field of the curve.  If WITH_PREFIX is set the returned
   buffer is prefixed with a 0x40 byte.  On success 0 is returned and
   a malloced buffer with the encoded point is stored at R_BUFFER; the
   length of this buffer is stored at R_BUFLEN.  */
static gpg_err_code_t
eddsa_encode_x_y (gcry_mpi_t x, gcry_mpi_t y, unsigned int nbits,
                  int with_prefix,
                  unsigned char **r_buffer, unsigned int *r_buflen)
{
  unsigned char *rawmpi;
  unsigned int rawmpilen;
  int off = with_prefix? 1:0;
  unsigned int minlen = (nbits%8) == 0 ? (nbits/8 + 1): (nbits+7)/8;

  rawmpi = _gcry_mpi_get_buffer_extra (y, minlen, off?-1:0, &rawmpilen, NULL);
  if (!rawmpi)
    return gpg_err_code_from_syserror ();
  if (mpi_test_bit (x, 0) && rawmpilen)
    rawmpi[off + rawmpilen - 1] |= 0x80;  /* Set sign bit.  */
  if (off)
    rawmpi[0] = 0x40;

  *r_buffer = rawmpi;
  *r_buflen = rawmpilen + off;
  return 0;
}

/* Encode POINT using the EdDSA scheme.  X and Y are either scratch
   variables supplied by the caller or NULL.  CTX is the usual
   context.  If WITH_PREFIX is set the returned buffer is prefixed
   with a 0x40 byte.  On success 0 is returned and a malloced buffer
   with the encoded point is stored at R_BUFFER; the length of this
   buffer is stored at R_BUFLEN.  */
gpg_err_code_t
_gcry_ecc_eddsa_encodepoint (mpi_point_t point, mpi_ec_t ec,
                             gcry_mpi_t x_in, gcry_mpi_t y_in,
                             int with_prefix,
                             unsigned char **r_buffer, unsigned int *r_buflen)
{
  gpg_err_code_t rc;
  gcry_mpi_t x, y;

  x = x_in? x_in : mpi_new (0);
  y = y_in? y_in : mpi_new (0);

  if (_gcry_mpi_ec_get_affine (x, y, point, ec))
    {
      log_error ("eddsa_encodepoint: Failed to get affine coordinates\n");
      rc = GPG_ERR_INTERNAL;
    }
  else
    rc = eddsa_encode_x_y (x, y, ec->nbits, with_prefix, r_buffer, r_buflen);

  if (!x_in)
    mpi_free (x);
  if (!y_in)
    mpi_free (y);
  return rc;
}


/* Make sure that the opaque MPI VALUE is in compact EdDSA format.
   This function updates MPI if needed.  */
gpg_err_code_t
_gcry_ecc_eddsa_ensure_compact (gcry_mpi_t value, unsigned int nbits)
{
  gpg_err_code_t rc;
  const unsigned char *buf;
  unsigned int rawmpilen;
  gcry_mpi_t x, y;
  unsigned char *enc;
  unsigned int enclen;

  if (!mpi_is_opaque (value))
    return GPG_ERR_INV_OBJ;
  buf = mpi_get_opaque (value, &rawmpilen);
  if (!buf)
    return GPG_ERR_INV_OBJ;
  rawmpilen = (rawmpilen + 7)/8;

  if (rawmpilen > 1 && (rawmpilen%2))
    {
      if (buf[0] == 0x04)
        {
          /* Buffer is in SEC1 uncompressed format.  Extract y and
             compress.  */
          rc = _gcry_mpi_scan (&x, GCRYMPI_FMT_USG,
                               buf+1, (rawmpilen-1)/2, NULL);
          if (rc)
            return rc;
          rc = _gcry_mpi_scan (&y, GCRYMPI_FMT_USG,
                               buf+1+(rawmpilen-1)/2, (rawmpilen-1)/2, NULL);
          if (rc)
            {
              mpi_free (x);
              return rc;
            }

          rc = eddsa_encode_x_y (x, y, nbits, 0, &enc, &enclen);
          mpi_free (x);
          mpi_free (y);
          if (rc)
            return rc;

          mpi_set_opaque (value, enc, 8*enclen);
        }
      else if (buf[0] == 0x40)
        {
          /* Buffer is compressed but with our SEC1 alike compression
             indicator.  Remove that byte.  FIXME: We should write and
             use a function to manipulate an opaque MPI in place. */
          if (!_gcry_mpi_set_opaque_copy (value, buf + 1, (rawmpilen - 1)*8))
            return gpg_err_code_from_syserror ();
        }
    }

  return 0;
}


static gpg_err_code_t
ecc_ed448_recover_x (gcry_mpi_t x, gcry_mpi_t y, int x_0, mpi_ec_t ec)
{
  gpg_err_code_t rc = 0;
  gcry_mpi_t u, v, u3, v3, t;
  static gcry_mpi_t p34; /* Hard coded (P-3)/4 */

  if (mpi_cmp (y, ec->p) >= 0)
    rc = GPG_ERR_INV_OBJ;

  if (!p34)
    p34 = scanval ("3FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF"
                   "BFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF");

  u   = mpi_new (0);
  v   = mpi_new (0);
  u3  = mpi_new (0);
  v3  = mpi_new (0);
  t   = mpi_new (0);

  /* Compute u and v */
  /* u = y^2    */
  mpi_mulm (u, y, y, ec->p);
  /* v = b*y^2   */
  mpi_mulm (v, ec->b, u, ec->p);
  /* u = y^2-1  */
  mpi_sub_ui (u, u, 1);
  /* v = b*y^2-1 */
  mpi_sub_ui (v, v, 1);

  /* Compute sqrt(u/v) */
  /* u3 = u^3 */
  mpi_powm (u3, u, mpi_const (MPI_C_THREE), ec->p);
  mpi_powm (v3, v, mpi_const (MPI_C_THREE), ec->p);
  /* t = u^4 * u * v3 = u^5 * v^3 */
  mpi_powm (t, u, mpi_const (MPI_C_FOUR), ec->p);
  mpi_mulm (t, t, u, ec->p);
  mpi_mulm (t, t, v3, ec->p);
  /* t = t^((p-3)/4) = (u^5 * v^3)^((p-3)/4)  */
  mpi_powm (t, t, p34, ec->p);
  /* x = t * u^3 * v = (u^3 * v) * (u^5 * v^3)^((p-3)/4) */
  mpi_mulm (t, t, u3, ec->p);
  mpi_mulm (x, t, v, ec->p);

  /* t = v * x^2  */
  mpi_mulm (t, x, x, ec->p);
  mpi_mulm (t, t, v, ec->p);

  if (mpi_cmp (t, u) != 0)
    rc = GPG_ERR_INV_OBJ;
  else
    {
      if (!mpi_cmp_ui (x, 0) && x_0)
        rc = GPG_ERR_INV_OBJ;

      /* Choose the desired square root according to parity */
      if (mpi_test_bit (x, 0) != !!x_0)
        mpi_sub (x, ec->p, x);
    }

  mpi_free (t);
  mpi_free (u3);
  mpi_free (v3);
  mpi_free (v);
  mpi_free (u);

  return rc;
}


/* Recover X from Y and SIGN (which actually is a parity bit).  */
gpg_err_code_t
_gcry_ecc_eddsa_recover_x (gcry_mpi_t x, gcry_mpi_t y, int sign, mpi_ec_t ec)
{
  gpg_err_code_t rc = 0;
  gcry_mpi_t u, v, v3, t;
  static gcry_mpi_t p58, seven;

  /*
   * This routine is actually curve specific.  Now, only supports
   * Ed25519 and Ed448.
   */

  if (ec->dialect != ECC_DIALECT_ED25519)
    /* For now, it's only Ed448.  */
    return ecc_ed448_recover_x (x, y, sign, ec);

  /* It's Ed25519.  */

  if (!p58)
    p58 = scanval ("0FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF"
                   "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFD");
  if (!seven)
    seven = mpi_set_ui (NULL, 7);

  u   = mpi_new (0);
  v   = mpi_new (0);
  v3  = mpi_new (0);
  t   = mpi_new (0);

  /* Compute u and v */
  /* u = y^2    */
  mpi_mulm (u, y, y, ec->p);
  /* v = b*y^2   */
  mpi_mulm (v, ec->b, u, ec->p);
  /* u = y^2-1  */
  mpi_sub_ui (u, u, 1);
  /* v = b*y^2+1 */
  mpi_add_ui (v, v, 1);

  /* Compute sqrt(u/v) */
  /* v3 = v^3 */
  mpi_powm (v3, v, mpi_const (MPI_C_THREE), ec->p);
  /* t = v3 * v3 * u * v = u * v^7 */
  mpi_powm (t, v, seven, ec->p);
  mpi_mulm (t, t, u, ec->p);
  /* t = t^((p-5)/8) = (u * v^7)^((p-5)/8)  */
  mpi_powm (t, t, p58, ec->p);
  /* x = t * u * v^3 = (u * v^3) * (u * v^7)^((p-5)/8) */
  mpi_mulm (t, t, u, ec->p);
  mpi_mulm (x, t, v3, ec->p);

  /* Adjust if needed.  */
  /* t = v * x^2  */
  mpi_mulm (t, x, x, ec->p);
  mpi_mulm (t, t, v, ec->p);
  /* -t == u ? x = x * sqrt(-1) */
  mpi_sub (t, ec->p, t);
  if (!mpi_cmp (t, u))
    {
      static gcry_mpi_t m1;  /* Fixme: this is not thread-safe.  */
      if (!m1)
        m1 = scanval ("2B8324804FC1DF0B2B4D00993DFBD7A7"
                      "2F431806AD2FE478C4EE1B274A0EA0B0");
      mpi_mulm (x, x, m1, ec->p);
      /* t = v * x^2  */
      mpi_mulm (t, x, x, ec->p);
      mpi_mulm (t, t, v, ec->p);
      /* -t == u ? x = x * sqrt(-1) */
      mpi_sub (t, ec->p, t);
      if (!mpi_cmp (t, u))
        rc = GPG_ERR_INV_OBJ;
    }

  /* Choose the desired square root according to parity */
  if (mpi_test_bit (x, 0) != !!sign)
    mpi_sub (x, ec->p, x);

  mpi_free (t);
  mpi_free (v3);
  mpi_free (v);
  mpi_free (u);

  return rc;
}


/* Decode the EdDSA style encoded PK and set it into RESULT.  CTX is
   the usual curve context.  If R_ENCPK is not NULL, the encoded PK is
   stored at that address; this is a new copy to be released by the
   caller.  In contrast to the supplied PK, this is not an MPI and
   thus guaranteed to be properly padded.  R_ENCPKLEN receives the
   length of that encoded key.  */
gpg_err_code_t
_gcry_ecc_eddsa_decodepoint (gcry_mpi_t pk, mpi_ec_t ctx, mpi_point_t result,
                             unsigned char **r_encpk, unsigned int *r_encpklen)
{
  gpg_err_code_t rc;
  unsigned char *rawmpi;
  unsigned int rawmpilen;
  int sign;

  if (mpi_is_opaque (pk))
    {
      const unsigned char *buf;
      unsigned int len;

      len = (ctx->nbits%8) == 0 ? (ctx->nbits/8 + 1): (ctx->nbits+7)/8;

      buf = mpi_get_opaque (pk, &rawmpilen);
      if (!buf)
        return GPG_ERR_INV_OBJ;
      rawmpilen = (rawmpilen + 7)/8;

      if (!(rawmpilen == len
            || rawmpilen == len + 1
            || rawmpilen == len * 2 + 1))
        return GPG_ERR_INV_OBJ;

      /* Handle compression prefixes.  The size of the buffer will be
         odd in this case.  */
      if (rawmpilen > 1 && (rawmpilen == len + 1 || rawmpilen == len * 2 + 1))
        {
          /* First check whether the public key has been given in
             standard uncompressed format (SEC1).  No need to recover
             x in this case.  */
          if (buf[0] == 0x04)
            {
              gcry_mpi_t x, y;

              rc = _gcry_mpi_scan (&x, GCRYMPI_FMT_USG,
                                   buf+1, (rawmpilen-1)/2, NULL);
              if (rc)
                return rc;
              rc = _gcry_mpi_scan (&y, GCRYMPI_FMT_USG,
                                   buf+1+(rawmpilen-1)/2, (rawmpilen-1)/2,NULL);
              if (rc)
                {
                  mpi_free (x);
                  return rc;
                }

              if (r_encpk)
                {
                  rc = eddsa_encode_x_y (x, y, ctx->nbits, 0,
                                         r_encpk, r_encpklen);
                  if (rc)
                    {
                      mpi_free (x);
                      mpi_free (y);
                      return rc;
                    }
                }
              mpi_snatch (result->x, x);
              mpi_snatch (result->y, y);
              mpi_set_ui (result->z, 1);
              return 0;
            }

          /* Check whether the public key has been prefixed with a 0x40
             byte to explicitly indicate compressed format using a SEC1
             alike prefix byte.  This is a Libgcrypt extension.  */
          if (buf[0] == 0x40)
            {
              rawmpilen--;
              buf++;
            }
        }

      /* EdDSA compressed point.  */
      rawmpi = xtrymalloc (rawmpilen);
      if (!rawmpi)
        return gpg_err_code_from_syserror ();
      memcpy (rawmpi, buf, rawmpilen);
      reverse_buffer (rawmpi, rawmpilen);
    }
  else
    {
      /* Note: Without using an opaque MPI it is not reliable possible
         to find out whether the public key has been given in
         uncompressed format.  Thus we expect native EdDSA format.  */
      rawmpi = _gcry_mpi_get_buffer (pk, (ctx->nbits+7)/8, &rawmpilen, NULL);
      if (!rawmpi)
        return gpg_err_code_from_syserror ();
    }

  if (rawmpilen)
    {
      sign = !!(rawmpi[0] & 0x80);
      rawmpi[0] &= 0x7f;
    }
  else
    sign = 0;
  _gcry_mpi_set_buffer (result->y, rawmpi, rawmpilen, 0);
  if (r_encpk)
    {
      /* Revert to little endian.  */
      if (sign && rawmpilen)
        rawmpi[0] |= 0x80;
      reverse_buffer (rawmpi, rawmpilen);
      *r_encpk = rawmpi;
      if (r_encpklen)
        *r_encpklen = rawmpilen;
    }
  else
    xfree (rawmpi);

  rc = _gcry_ecc_eddsa_recover_x (result->x, result->y, sign, ctx);
  mpi_set_ui (result->z, 1);

  return rc;
}


/* Compute the A value as used by EdDSA.  The caller needs to provide
   the context EC and the actual secret D as an MPI.  The function
   returns a newly allocated 64 byte buffer at r_digest; the first 32
   bytes represent the A value.  NULL is returned on error and NULL
   stored at R_DIGEST.  */
gpg_err_code_t
_gcry_ecc_eddsa_compute_h_d (unsigned char **r_digest, mpi_ec_t ec)
{
  gpg_err_code_t rc;
  unsigned char *rawmpi = NULL;
  unsigned int rawmpilen;
  unsigned char *digest;
  int hashalgo, b, digestlen;
  gcry_buffer_t hvec[2];

  *r_digest = NULL;

  b = (ec->nbits+7)/8;

  /*
   * Choice of hashalgo is curve specific.
   * For now, it's determine by the bit size of the field.
   */
  if (ec->nbits == 255)
    {
      hashalgo = GCRY_MD_SHA512;
      digestlen = 64;
    }
  else if (ec->nbits == 448)
    {
      b++;
      hashalgo = GCRY_MD_SHAKE256;
      digestlen = 2 * b;
    }
  else
    return GPG_ERR_NOT_IMPLEMENTED;

  /* Note that we clear DIGEST so we can use it as input to left pad
     the key with zeroes for hashing.  */
  digest = xtrycalloc_secure (2, b);
  if (!digest)
    return gpg_err_code_from_syserror ();

  rawmpi = _gcry_mpi_get_buffer (ec->d, 0, &rawmpilen, NULL);
  if (!rawmpi)
    {
      xfree (digest);
      return gpg_err_code_from_syserror ();
    }

  memset (hvec, 0, sizeof hvec);

  hvec[0].data = digest;
  hvec[0].len = (hashalgo == GCRY_MD_SHA512 && b > rawmpilen)
		  ? b - rawmpilen : 0;
  hvec[1].data = rawmpi;
  hvec[1].len = rawmpilen;
  rc = _gcry_md_hash_buffers_extract (hashalgo, 0, digest, digestlen, hvec, 2);

  xfree (rawmpi);
  if (rc)
    {
      xfree (digest);
      return rc;
    }

  /* Compute the A value.  */
  reverse_buffer (digest, b);  /* Only the first half of the hash.  */

  /* Field specific handling of clearing/setting bits. */
  if (ec->nbits == 255)
    {
      digest[0]   = (digest[0] & 0x7f) | 0x40;
      digest[31] &= 0xf8;
    }
  else
    {
      digest[0]   = 0;
      digest[1]  |= 0x80;
      digest[56] &= 0xfc;
    }

  *r_digest = digest;
  return 0;
}


/**
 * _gcry_ecc_eddsa_genkey - EdDSA version of the key generation.
 *
 * @ec: Elliptic curve computation context.
 * @flags: Flags controlling aspects of the creation.
 *
 * Return: An error code.
 *
 * The only @flags bit used by this function is %PUBKEY_FLAG_TRANSIENT
 * to use a faster RNG.
 */
gpg_err_code_t
_gcry_ecc_eddsa_genkey (mpi_ec_t ec, int flags)
{
  gpg_err_code_t rc;
  int b;
  gcry_mpi_t a, x, y;
  mpi_point_struct Q;
  gcry_random_level_t random_level;
  char *dbuf;
  size_t dlen;
  unsigned char *hash_d = NULL;

  if ((flags & PUBKEY_FLAG_TRANSIENT_KEY))
    random_level = GCRY_STRONG_RANDOM;
  else
    random_level = GCRY_VERY_STRONG_RANDOM;

  b = (ec->nbits+7)/8;

  if (ec->nbits == 255)
    ;
  else if (ec->nbits == 448)
    b++;
  else
    return GPG_ERR_NOT_IMPLEMENTED;

  dlen = b;

  a = mpi_snew (0);
  x = mpi_new (0);
  y = mpi_new (0);

  /* Generate a secret.  */
  dbuf = _gcry_random_bytes_secure (dlen, random_level);
  ec->d = _gcry_mpi_set_opaque (NULL, dbuf, dlen*8);
  rc = _gcry_ecc_eddsa_compute_h_d (&hash_d, ec);
  if (rc)
    goto leave;

  _gcry_mpi_set_buffer (a, hash_d, b, 0);
  xfree (hash_d);
  /* log_printmpi ("ecgen         a", a); */

  /* Compute Q.  */
  point_init (&Q);
  _gcry_mpi_ec_mul_point (&Q, a, ec->G, ec);
  if (DBG_CIPHER)
    log_printpnt ("ecgen      pk", &Q, ec);

  ec->Q = mpi_point_snatch_set (NULL, Q.x, Q.y, Q.z);
  Q.x = NULL;
  Q.y = NULL;
  Q.x = NULL;

 leave:
  _gcry_mpi_release (a);
  _gcry_mpi_release (x);
  _gcry_mpi_release (y);
  return rc;
}


/* Compute an EdDSA signature. See:
 *   [ed25519] 23pp. (PDF) Daniel J. Bernstein, Niels Duif, Tanja
 *   Lange, Peter Schwabe, Bo-Yin Yang. High-speed high-security
 *   signatures.  Journal of Cryptographic Engineering 2 (2012), 77-89.
 *   Document ID: a1a62a2f76d23f65d622484ddd09caf8.
 *   URL: http://cr.yp.to/papers.html#ed25519. Date: 2011.09.26.
 *
 * Despite that this function requires the specification of a hash
 * algorithm, we only support what has been specified by the paper.
 * This may change in the future.
 *
 * Return the signature struct (r,s) from the message hash.  The caller
 * must have allocated R_R and S.
 */

/* String to be used with Ed448 */
#define DOM25519     "SigEd25519 no Ed25519 collisions"
#define DOM25519_LEN 32
#define DOM448       "SigEd448"
#define DOM448_LEN   8

gpg_err_code_t
_gcry_ecc_eddsa_sign (gcry_mpi_t input, mpi_ec_t ec,
                      gcry_mpi_t r_r, gcry_mpi_t s,
                      struct pk_encoding_ctx *ctx)
{
  int rc;
  unsigned int tmp;
  unsigned char *digest = NULL;
  const void *mbuf;
  size_t mlen;
  unsigned char *rawmpi = NULL;
  unsigned int rawmpilen = 0;
  unsigned char *encpk = NULL; /* Encoded public key.  */
  unsigned int encpklen = 0;
  mpi_point_struct I;          /* Intermediate value.  */
  gcry_mpi_t a, x, y, r;
  const char *dom;
  int domlen, digestlen;
  int b, i;
  unsigned char x_olen[2];
  unsigned char prehashed_msg[64];
  gcry_buffer_t hvec[6];
  gcry_buffer_t hvec2[1];

  b = (ec->nbits+7)/8;

  if (ec->nbits == 255)
    {
      dom = DOM25519;
      domlen = DOM25519_LEN;
      digestlen = 64;
    }
  else if (ec->nbits == 448)
    {
      b++;
      dom = DOM448;
      domlen = DOM448_LEN;
      digestlen = 2 * b;
    }
  else
    return GPG_ERR_NOT_IMPLEMENTED;

  if (!mpi_is_opaque (input))
    return GPG_ERR_INV_DATA;

  /* Initialize some helpers.  */
  point_init (&I);
  a = mpi_snew (0);
  x = mpi_new (0);
  y = mpi_new (0);
  r = mpi_snew (0);

  rc = _gcry_ecc_eddsa_compute_h_d (&digest, ec);
  if (rc)
    goto leave;
  _gcry_mpi_set_buffer (a, digest, b, 0);

  /* Compute the public key if it's not available (only secret part).  */
  if (ec->Q == NULL)
    {
      mpi_point_struct Q;

      point_init (&Q);
      _gcry_mpi_ec_mul_point (&Q, a, ec->G, ec);
      ec->Q = mpi_point_snatch_set (NULL, Q.x, Q.y, Q.z);
    }
  rc = _gcry_ecc_eddsa_encodepoint (ec->Q, ec, x, y, 0, &encpk, &encpklen);
  if (rc)
    goto leave;
  if (DBG_CIPHER)
    log_printhex ("  e_pk", encpk, encpklen);

  /* Compute R.  */
  mbuf = mpi_get_opaque (input, &tmp);
  mlen = (tmp +7)/8;
  if (DBG_CIPHER)
    log_printhex ("     m", mbuf, mlen);

  memset (hvec, 0, sizeof hvec);
  i = 0;

  if ((ctx->flags & PUBKEY_FLAG_PREHASH) || ctx->labellen || ec->nbits == 448)
    {
      hvec[i].data = (void *)dom;
      hvec[i].len  = domlen;
      i++;
      x_olen[0] = !!(ctx->flags & PUBKEY_FLAG_PREHASH);
      x_olen[1] = ctx->labellen;
      hvec[i].data = x_olen;
      hvec[i].len  = 2;
      i++;
      if (ctx->labellen)
	{
	  hvec[i].data = ctx->label;
	  hvec[i].len  = ctx->labellen;
	  i++;
	}
    }

  hvec[i].data = digest;
  hvec[i].off  = b;
  hvec[i].len  = b;
  i++;
  if ((ctx->flags & PUBKEY_FLAG_PREHASH))
    {
      memset (hvec2, 0, sizeof hvec2);

      hvec2[0].data = (char*)mbuf;
      hvec2[0].len  = mlen;

      _gcry_md_hash_buffers_extract (ctx->hash_algo, 0, prehashed_msg, 64,
				     hvec2, 1);
      hvec[i].data = (char*)prehashed_msg;
      hvec[i].len  = 64;
    }
  else
    {
      hvec[i].data = (char*)mbuf;
      hvec[i].len  = mlen;
    }
  i++;

  rc = _gcry_md_hash_buffers_extract (ctx->hash_algo, 0, digest, digestlen,
				      hvec, i);
  if (rc)
    goto leave;
  reverse_buffer (digest, digestlen);
  if (DBG_CIPHER)
    log_printhex ("     r", digest, digestlen);
  _gcry_mpi_set_buffer (r, digest, digestlen, 0);
  mpi_mod (r, r, ec->n);
  _gcry_mpi_ec_mul_point (&I, r, ec->G, ec);
  if (DBG_CIPHER)
    log_printpnt ("   r", &I, ec);

  /* Convert R into affine coordinates and apply encoding.  */
  rc = _gcry_ecc_eddsa_encodepoint (&I, ec, x, y, 0, &rawmpi, &rawmpilen);
  if (rc)
    goto leave;
  if (DBG_CIPHER)
    log_printhex ("   e_r", rawmpi, rawmpilen);

  memset (hvec, 0, sizeof hvec);
  i = 0;

  if ((ctx->flags & PUBKEY_FLAG_PREHASH) || ctx->labellen || ec->nbits == 448)
    {
      hvec[i].data = (void *)dom;
      hvec[i].len  = domlen;
      i++;
      x_olen[0] = !!(ctx->flags & PUBKEY_FLAG_PREHASH);
      x_olen[1] = ctx->labellen;
      hvec[i].data = x_olen;
      hvec[i].len  = 2;
      i++;
      if (ctx->labellen)
	{
	  hvec[i].data = ctx->label;
	  hvec[i].len  = ctx->labellen;
	  i++;
	}
    }

  /* S = r + a * H(dom2(F,C)+encodepoint(R)+encodepoint(pk)+m) mod n  */
  hvec[i].data = rawmpi;  /* (this is R) */
  hvec[i].len  = rawmpilen;
  i++;
  hvec[i].data = encpk;
  hvec[i].len  = encpklen;
  i++;
  if ((ctx->flags & PUBKEY_FLAG_PREHASH))
    {
      hvec[i].data = (char*)prehashed_msg;
      hvec[i].len  = 64;
    }
  else
    {
      hvec[i].data = (char*)mbuf;
      hvec[i].len  = mlen;
    }
  i++;

  rc = _gcry_md_hash_buffers_extract (ctx->hash_algo, 0, digest, digestlen,
				      hvec, i);
  if (rc)
    goto leave;

  /* No more need for RAWMPI thus we now transfer it to R_R.  */
  mpi_set_opaque (r_r, rawmpi, rawmpilen*8);
  rawmpi = NULL;

  reverse_buffer (digest, digestlen);
  if (DBG_CIPHER)
    log_printhex (" H(R+)", digest, digestlen);
  _gcry_mpi_set_buffer (s, digest, digestlen, 0);
  mpi_mulm (s, s, a, ec->n);
  mpi_addm (s, s, r, ec->n);
  rc = eddsa_encodempi (s, ec->nbits, &rawmpi, &rawmpilen);
  if (rc)
    goto leave;
  if (DBG_CIPHER)
    log_printhex ("   e_s", rawmpi, rawmpilen);
  mpi_set_opaque (s, rawmpi, rawmpilen*8);
  rawmpi = NULL;

  rc = 0;

 leave:
  _gcry_mpi_release (a);
  _gcry_mpi_release (x);
  _gcry_mpi_release (y);
  _gcry_mpi_release (r);
  xfree (digest);
  point_free (&I);
  xfree (encpk);
  xfree (rawmpi);
  return rc;
}


/* Verify an EdDSA signature.  See sign_eddsa for the reference.
 * Check if R_IN and S_IN verifies INPUT.
 */
gpg_err_code_t
_gcry_ecc_eddsa_verify (gcry_mpi_t input, mpi_ec_t ec,
                        gcry_mpi_t r_in, gcry_mpi_t s_in,
                        struct pk_encoding_ctx *ctx)
{
  int rc;
  int b;
  unsigned int tmp;
  unsigned char *encpk = NULL; /* Encoded public key.  */
  unsigned int encpklen = 0;
  const void *mbuf, *rbuf;
  unsigned char *tbuf = NULL;
  size_t mlen, rlen;
  unsigned int tlen;
  unsigned char digest[114];
  gcry_mpi_t h, s;
  mpi_point_struct Ia, Ib;
  const char *dom;
  int domlen, digestlen;
  int i;
  unsigned char x_olen[2];
  unsigned char prehashed_msg[64];
  gcry_buffer_t hvec[6];
  gcry_buffer_t hvec2[1];

  if (!mpi_is_opaque (input) || !mpi_is_opaque (r_in) || !mpi_is_opaque (s_in))
    return GPG_ERR_INV_DATA;

  b = (ec->nbits+7)/8;

  if (ec->nbits == 255)
    {
      dom = DOM25519;
      domlen = DOM25519_LEN;
      digestlen = 64;
    }
  else if (ec->nbits == 448)
    {
      b++;
      dom = DOM448;
      domlen = DOM448_LEN;
      digestlen = 2 * b;
    }
  else
    return GPG_ERR_NOT_IMPLEMENTED;

  point_init (&Ia);
  point_init (&Ib);
  h = mpi_new (0);
  s = mpi_new (0);

  /* Encode and check the public key.  */
  rc = _gcry_ecc_eddsa_encodepoint (ec->Q, ec, NULL, NULL, 0,
                                    &encpk, &encpklen);
  if (rc)
    goto leave;
  if (!_gcry_mpi_ec_curve_point (ec->Q, ec))
    {
      rc = GPG_ERR_BROKEN_PUBKEY;
      goto leave;
    }
  if (DBG_CIPHER)
    log_printhex ("  e_pk", encpk, encpklen);
  if (encpklen != b)
    {
      rc = GPG_ERR_INV_LENGTH;
      goto leave;
    }

  /* Convert the other input parameters.  */
  mbuf = mpi_get_opaque (input, &tmp);
  mlen = (tmp +7)/8;
  if (DBG_CIPHER)
    log_printhex ("     m", mbuf, mlen);
  rbuf = mpi_get_opaque (r_in, &tmp);
  rlen = (tmp +7)/8;
  if (DBG_CIPHER)
    log_printhex ("     r", rbuf, rlen);
  if (rlen != b)
    {
      rc = GPG_ERR_INV_LENGTH;
      goto leave;
    }

  memset (hvec, 0, sizeof hvec);
  i = 0;

  /* h = H(dom2(F,C)+encodepoint(R)+encodepoint(pk)+m)  */
  if ((ctx->flags & PUBKEY_FLAG_PREHASH) || ctx->labellen || ec->nbits == 448)
    {
      hvec[i].data = (void *)dom;
      hvec[i].len  = domlen;
      i++;
      x_olen[0] = !!(ctx->flags & PUBKEY_FLAG_PREHASH);
      x_olen[1] = ctx->labellen;
      hvec[i].data = x_olen;
      hvec[i].len  = 2;
      i++;
      if (ctx->labellen)
	{
	  hvec[i].data = ctx->label;
	  hvec[i].len  = ctx->labellen;
	  i++;
	}
    }

  hvec[i].data = (char*)rbuf;
  hvec[i].len  = rlen;
  i++;
  hvec[i].data = encpk;
  hvec[i].len  = encpklen;
  i++;
  if ((ctx->flags & PUBKEY_FLAG_PREHASH))
    {
      memset (hvec2, 0, sizeof hvec2);

      hvec2[0].data = (char*)mbuf;
      hvec2[0].len  = mlen;

      _gcry_md_hash_buffers_extract (ctx->hash_algo, 0, prehashed_msg, 64,
				      hvec2, 1);
      hvec[i].data = (char*)prehashed_msg;
      hvec[i].len  = 64;
    }
  else
    {
      hvec[i].data = (char*)mbuf;
      hvec[i].len  = mlen;
    }
  i++;

  rc = _gcry_md_hash_buffers_extract (ctx->hash_algo, 0, digest, digestlen,
				      hvec, i);
  if (rc)
    goto leave;
  reverse_buffer (digest, digestlen);
  if (DBG_CIPHER)
    log_printhex (" H(R+)", digest, digestlen);
  _gcry_mpi_set_buffer (h, digest, digestlen, 0);

  /* According to the paper the best way for verification is:
         encodepoint(sG - h·Q) = encodepoint(r)
     because we don't need to decode R. */
  {
    void *sbuf;
    unsigned int slen;

    sbuf = _gcry_mpi_get_opaque_copy (s_in, &tmp);
    slen = (tmp +7)/8;
    reverse_buffer (sbuf, slen);
    if (DBG_CIPHER)
      log_printhex ("     s", sbuf, slen);
    _gcry_mpi_set_buffer (s, sbuf, slen, 0);
    xfree (sbuf);
    if (slen != b)
      {
        rc = GPG_ERR_INV_LENGTH;
        goto leave;
      }
  }

  _gcry_mpi_ec_mul_point (&Ia, s, ec->G, ec);
  _gcry_mpi_ec_mul_point (&Ib, h, ec->Q, ec);
  _gcry_mpi_sub (Ib.x, ec->p, Ib.x);
  _gcry_mpi_ec_add_points (&Ia, &Ia, &Ib, ec);
  rc = _gcry_ecc_eddsa_encodepoint (&Ia, ec, s, h, 0, &tbuf, &tlen);
  if (rc)
    goto leave;
  if (tlen != rlen || memcmp (tbuf, rbuf, tlen))
    {
      rc = GPG_ERR_BAD_SIGNATURE;
      goto leave;
    }

  rc = 0;

 leave:
  xfree (encpk);
  xfree (tbuf);
  _gcry_mpi_release (s);
  _gcry_mpi_release (h);
  point_free (&Ia);
  point_free (&Ib);
  return rc;
}
