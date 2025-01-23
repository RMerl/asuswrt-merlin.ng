/* rsa-common.c - Supporting functions for RSA
 * Copyright (C) 2011 Free Software Foundation, Inc.
 * Copyright (C) 2013  g10 Code GmbH
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


/* Turn VALUE into an octet string and store it in an allocated buffer
   at R_FRAME or - if R_RAME is NULL - copy it into the caller
   provided buffer SPACE; either SPACE or R_FRAME may be used.  If
   SPACE if not NULL, the caller must provide a buffer of at least
   NBYTES.  If the resulting octet string is shorter than NBYTES pad
   it to the left with zeroes.  If VALUE does not fit into NBYTES
   return an error code.  */
static gpg_err_code_t
octet_string_from_mpi (unsigned char **r_frame, void *space,
                       gcry_mpi_t value, size_t nbytes)
{
  return _gcry_mpi_to_octet_string (r_frame, space, value, nbytes);
}



/* Encode {VALUE,VALUELEN} for an NBITS keys using the pkcs#1 block
   type 2 padding.  On success the result is stored as a new MPI at
   R_RESULT.  On error the value at R_RESULT is undefined.

   If {RANDOM_OVERRIDE, RANDOM_OVERRIDE_LEN} is given it is used as
   the seed instead of using a random string for it.  This feature is
   only useful for regression tests.  Note that this value may not
   contain zero bytes.

   We encode the value in this way:

     0  2  RND(n bytes)  0  VALUE

   0   is a marker we unfortunately can't encode because we return an
       MPI which strips all leading zeroes.
   2   is the block type.
   RND are non-zero random bytes.

   (Note that OpenPGP includes the cipher algorithm and a checksum in
   VALUE; the caller needs to prepare the value accordingly.)
  */
gpg_err_code_t
_gcry_rsa_pkcs1_encode_for_enc (gcry_mpi_t *r_result, unsigned int nbits,
                                const unsigned char *value, size_t valuelen,
                                const unsigned char *random_override,
                                size_t random_override_len)
{
  gcry_err_code_t rc = 0;
  unsigned char *frame = NULL;
  size_t nframe = (nbits+7) / 8;
  int i;
  size_t n;
  unsigned char *p;

  if (valuelen + 7 > nframe || !nframe)
    {
      /* Can't encode a VALUELEN value in a NFRAME bytes frame.  */
      return GPG_ERR_TOO_SHORT; /* The key is too short.  */
    }

  if ( !(frame = xtrymalloc_secure (nframe)))
    return gpg_err_code_from_syserror ();

  n = 0;
  frame[n++] = 0;
  frame[n++] = 2; /* block type */
  i = nframe - 3 - valuelen;
  gcry_assert (i > 0);

  if (random_override)
    {
      int j;

      if (random_override_len != i)
        {
          xfree (frame);
          return GPG_ERR_INV_ARG;
        }
      /* Check that random does not include a zero byte.  */
      for (j=0; j < random_override_len; j++)
        if (!random_override[j])
          {
            xfree (frame);
            return GPG_ERR_INV_ARG;
          }
      memcpy (frame + n, random_override, random_override_len);
      n += random_override_len;
    }
  else
    {
      p = _gcry_random_bytes_secure (i, GCRY_STRONG_RANDOM);
      /* Replace zero bytes by new values. */
      for (;;)
        {
          int j, k;
          unsigned char *pp;

          /* Count the zero bytes. */
          for (j=k=0; j < i; j++)
            {
              if (!p[j])
                k++;
            }
          if (!k)
            break; /* Okay: no (more) zero bytes. */

          k += k/128 + 3; /* Better get some more. */
          pp = _gcry_random_bytes_secure (k, GCRY_STRONG_RANDOM);
          for (j=0; j < i && k; )
            {
              if (!p[j])
                p[j] = pp[--k];
              if (p[j])
                j++;
            }
          xfree (pp);
        }
      memcpy (frame+n, p, i);
      n += i;
      xfree (p);
    }

  frame[n++] = 0;
  memcpy (frame+n, value, valuelen);
  n += valuelen;
  gcry_assert (n == nframe);

  rc = _gcry_mpi_scan (r_result, GCRYMPI_FMT_USG, frame, n, &nframe);
  if (!rc &&DBG_CIPHER)
    log_mpidump ("PKCS#1 block type 2 encoded data", *r_result);
  xfree (frame);

  return rc;
}


/* Decode a plaintext in VALUE assuming pkcs#1 block type 2 padding.
   NBITS is the size of the secret key.  On success the result is
   stored as a newly allocated buffer at R_RESULT and its valid length at
   R_RESULTLEN.  On error NULL is stored at R_RESULT.  */
gpg_err_code_t
_gcry_rsa_pkcs1_decode_for_enc (unsigned char **r_result, size_t *r_resultlen,
                                unsigned int nbits, gcry_mpi_t value)
{
  gcry_error_t err;
  unsigned char *frame = NULL;
  size_t nframe = (nbits+7) / 8;
  size_t n;

  *r_result = NULL;

  if ( !(frame = xtrymalloc_secure (nframe)))
    return gpg_err_code_from_syserror ();

  err = _gcry_mpi_print (GCRYMPI_FMT_USG, frame, nframe, &n, value);
  if (err)
    {
      xfree (frame);
      return gcry_err_code (err);
    }

  nframe = n; /* Set NFRAME to the actual length.  */

  /* FRAME = 0x00 || 0x02 || PS || 0x00 || M

     pkcs#1 requires that the first byte is zero.  Our MPIs usually
     strip leading zero bytes; thus we are not able to detect them.
     However due to the way gcry_mpi_print is implemented we may see
     leading zero bytes nevertheless.  We handle this by making the
     first zero byte optional.  */
  if (nframe < 4)
    {
      xfree (frame);
      return GPG_ERR_ENCODING_PROBLEM;  /* Too short.  */
    }
  n = 0;
  if (!frame[0])
    n++;
  if (frame[n++] != 0x02)
    {
      xfree (frame);
      return GPG_ERR_ENCODING_PROBLEM;  /* Wrong block type.  */
    }

  /* Skip the non-zero random bytes and the terminating zero byte.  */
  for (; n < nframe && frame[n] != 0x00; n++)
    ;
  if (n+1 >= nframe)
    {
      xfree (frame);
      return GPG_ERR_ENCODING_PROBLEM; /* No zero byte.  */
    }
  n++; /* Skip the zero byte.  */

  /* To avoid an extra allocation we reuse the frame buffer.  The only
     caller of this function will anyway free the result soon.  */
  memmove (frame, frame + n, nframe - n);
  *r_result = frame;
  *r_resultlen = nframe - n;

  if (DBG_CIPHER)
    log_printhex ("value extracted from PKCS#1 block type 2 encoded data",
                  *r_result, *r_resultlen);

  return 0;
}


/* Encode {VALUE,VALUELEN} for an NBITS keys and hash algorithm ALGO
   using the pkcs#1 block type 1 padding.  On success the result is
   stored as a new MPI at R_RESULT.  On error the value at R_RESULT is
   undefined.

   We encode the value in this way:

     0  1  PAD(n bytes)  0  ASN(asnlen bytes) VALUE(valuelen bytes)

   0   is a marker we unfortunately can't encode because we return an
       MPI which strips all leading zeroes.
   1   is the block type.
   PAD consists of 0xff bytes.
   0   marks the end of the padding.
   ASN is the DER encoding of the hash algorithm; along with the VALUE
       it yields a valid DER encoding.

   (Note that PGP prior to version 2.3 encoded the message digest as:
      0   1   MD(16 bytes)   0   PAD(n bytes)   1
    The MD is always 16 bytes here because it's always MD5.  GnuPG
    does not not support pre-v2.3 signatures, but I'm including this
    comment so the information is easily found if needed.)
*/
gpg_err_code_t
_gcry_rsa_pkcs1_encode_for_sig (gcry_mpi_t *r_result, unsigned int nbits,
                                const unsigned char *value, size_t valuelen,
                                int algo)
{
  gcry_err_code_t rc = 0;
  byte asn[100];
  byte *frame = NULL;
  size_t nframe = (nbits+7) / 8;
  int i;
  size_t n;
  size_t asnlen, dlen;

  asnlen = DIM(asn);
  dlen = _gcry_md_get_algo_dlen (algo);

  if (_gcry_md_algo_info (algo, GCRYCTL_GET_ASNOID, asn, &asnlen))
    {
      /* We don't have yet all of the above algorithms.  */
      return GPG_ERR_NOT_IMPLEMENTED;
    }

  if ( valuelen != dlen )
    {
      /* Hash value does not match the length of digest for
         the given algorithm.  */
      return GPG_ERR_CONFLICT;
    }

  if ( !dlen || dlen + asnlen + 4 > nframe)
    {
      /* Can't encode an DLEN byte digest MD into an NFRAME byte
         frame.  */
      return GPG_ERR_TOO_SHORT;
    }

  if ( !(frame = xtrymalloc (nframe)) )
    return gpg_err_code_from_syserror ();

  /* Assemble the pkcs#1 block type 1. */
  n = 0;
  frame[n++] = 0;
  frame[n++] = 1; /* block type */
  i = nframe - valuelen - asnlen - 3 ;
  gcry_assert (i > 1);
  memset (frame+n, 0xff, i );
  n += i;
  frame[n++] = 0;
  memcpy (frame+n, asn, asnlen);
  n += asnlen;
  memcpy (frame+n, value, valuelen );
  n += valuelen;
  gcry_assert (n == nframe);

  /* Convert it into an MPI. */
  rc = _gcry_mpi_scan (r_result, GCRYMPI_FMT_USG, frame, n, &nframe);
  if (!rc && DBG_CIPHER)
    log_mpidump ("PKCS#1 block type 1 encoded data", *r_result);
  xfree (frame);

  return rc;
}

/* Encode {VALUE,VALUELEN} for an NBITS keys using the pkcs#1 block
   type 1 padding.  On success the result is stored as a new MPI at
   R_RESULT.  On error the value at R_RESULT is undefined.

   We encode the value in this way:

     0  1  PAD(n bytes)  0  VALUE(valuelen bytes)

   0   is a marker we unfortunately can't encode because we return an
       MPI which strips all leading zeroes.
   1   is the block type.
   PAD consists of 0xff bytes.
   0   marks the end of the padding.

   (Note that PGP prior to version 2.3 encoded the message digest as:
      0   1   MD(16 bytes)   0   PAD(n bytes)   1
    The MD is always 16 bytes here because it's always MD5.  GnuPG
    does not not support pre-v2.3 signatures, but I'm including this
    comment so the information is easily found if needed.)
*/
gpg_err_code_t
_gcry_rsa_pkcs1_encode_raw_for_sig (gcry_mpi_t *r_result, unsigned int nbits,
                                const unsigned char *value, size_t valuelen)
{
  gcry_err_code_t rc = 0;
  gcry_error_t err;
  byte *frame = NULL;
  size_t nframe = (nbits+7) / 8;
  int i;
  size_t n;

  if ( !valuelen || valuelen + 4 > nframe)
    {
      /* Can't encode an DLEN byte digest MD into an NFRAME byte
         frame.  */
      return GPG_ERR_TOO_SHORT;
    }

  if ( !(frame = xtrymalloc (nframe)) )
    return gpg_err_code_from_syserror ();

  /* Assemble the pkcs#1 block type 1. */
  n = 0;
  frame[n++] = 0;
  frame[n++] = 1; /* block type */
  i = nframe - valuelen - 3 ;
  gcry_assert (i > 1);
  memset (frame+n, 0xff, i );
  n += i;
  frame[n++] = 0;
  memcpy (frame+n, value, valuelen );
  n += valuelen;
  gcry_assert (n == nframe);

  /* Convert it into an MPI. */
  err = _gcry_mpi_scan (r_result, GCRYMPI_FMT_USG, frame, n, &nframe);
  if (err)
    rc = gcry_err_code (err);
  else if (DBG_CIPHER)
    log_mpidump ("PKCS#1 block type 1 encoded data", *r_result);
  xfree (frame);

  return rc;
}


/* Mask generation function for OAEP.  See RFC-3447 B.2.1.  */
static gcry_err_code_t
mgf1 (unsigned char *output, size_t outlen, unsigned char *seed, size_t seedlen,
      int algo)
{
  size_t dlen, nbytes, n;
  int idx;
  gcry_md_hd_t hd;
  gcry_err_code_t err;

  err = _gcry_md_open (&hd, algo, 0);
  if (err)
    return err;

  dlen = _gcry_md_get_algo_dlen (algo);

  /* We skip step 1 which would be assert(OUTLEN <= 2^32).  The loop
     in step 3 is merged with step 4 by concatenating no more octets
     than what would fit into OUTPUT.  The ceiling for the counter IDX
     is implemented indirectly.  */
  nbytes = 0;  /* Step 2.  */
  idx = 0;
  while ( nbytes < outlen )
    {
      unsigned char c[4], *digest;

      if (idx)
        _gcry_md_reset (hd);

      c[0] = (idx >> 24) & 0xFF;
      c[1] = (idx >> 16) & 0xFF;
      c[2] = (idx >> 8) & 0xFF;
      c[3] = idx & 0xFF;
      idx++;

      _gcry_md_write (hd, seed, seedlen);
      _gcry_md_write (hd, c, 4);
      digest = _gcry_md_read (hd, 0);

      n = (outlen - nbytes < dlen)? (outlen - nbytes) : dlen;
      memcpy (output+nbytes, digest, n);
      nbytes += n;
    }

  _gcry_md_close (hd);
  return GPG_ERR_NO_ERROR;
}


/* RFC-3447 (pkcs#1 v2.1) OAEP encoding.  NBITS is the length of the
   key measured in bits.  ALGO is the hash function; it must be a
   valid and usable algorithm.  {VALUE,VALUELEN} is the message to
   encrypt.  {LABEL,LABELLEN} is the optional label to be associated
   with the message, if LABEL is NULL the default is to use the empty
   string as label.  On success the encoded ciphertext is returned at
   R_RESULT.

   If {RANDOM_OVERRIDE, RANDOM_OVERRIDE_LEN} is given it is used as
   the seed instead of using a random string for it.  This feature is
   only useful for regression tests.

   Here is figure 1 from the RFC depicting the process:

                             +----------+---------+-------+
                        DB = |  lHash   |    PS   |   M   |
                             +----------+---------+-------+
                                            |
                  +----------+              V
                  |   seed   |--> MGF ---> xor
                  +----------+              |
                        |                   |
               +--+     V                   |
               |00|    xor <----- MGF <-----|
               +--+     |                   |
                 |      |                   |
                 V      V                   V
               +--+----------+----------------------------+
         EM =  |00|maskedSeed|          maskedDB          |
               +--+----------+----------------------------+
  */
gpg_err_code_t
_gcry_rsa_oaep_encode (gcry_mpi_t *r_result, unsigned int nbits, int algo,
                       const unsigned char *value, size_t valuelen,
                       const unsigned char *label, size_t labellen,
                       const void *random_override, size_t random_override_len)
{
  gcry_err_code_t rc = 0;
  unsigned char *frame = NULL;
  size_t nframe = (nbits+7) / 8;
  unsigned char *p;
  size_t hlen;
  size_t n;

  *r_result = NULL;

  /* Set defaults for LABEL.  */
  if (!label || !labellen)
    {
      label = (const unsigned char*)"";
      labellen = 0;
    }

  hlen = _gcry_md_get_algo_dlen (algo);

  /* We skip step 1a which would be to check that LABELLEN is not
     greater than 2^61-1.  See rfc-3447 7.1.1. */

  /* Step 1b.  Note that the obsolete rfc-2437 uses the check:
     valuelen > nframe - 2 * hlen - 1 .  */
  if (valuelen > nframe - 2 * hlen - 2 || !nframe)
    {
      /* Can't encode a VALUELEN value in a NFRAME bytes frame. */
      return GPG_ERR_TOO_SHORT; /* The key is too short.  */
    }

  /* Allocate the frame.  */
  frame = xtrycalloc_secure (1, nframe);
  if (!frame)
    return gpg_err_code_from_syserror ();

  /* Step 2a: Compute the hash of the label.  We store it in the frame
     where later the maskedDB will commence.  */
  _gcry_md_hash_buffer (algo, frame + 1 + hlen, label, labellen);

  /* Step 2b: Set octet string to zero.  */
  /* This has already been done while allocating FRAME.  */

  /* Step 2c: Create DB by concatenating lHash, PS, 0x01 and M.  */
  n = nframe - valuelen - 1;
  frame[n] = 0x01;
  memcpy (frame + n + 1, value, valuelen);

  /* Step 3d: Generate seed.  We store it where the maskedSeed will go
     later. */
  if (random_override)
    {
      if (random_override_len != hlen)
        {
          xfree (frame);
          return GPG_ERR_INV_ARG;
        }
      memcpy (frame + 1, random_override, hlen);
    }
  else
    _gcry_randomize (frame + 1, hlen, GCRY_STRONG_RANDOM);

  /* Step 2e and 2f: Create maskedDB.  */
  {
    unsigned char *dmask;

    dmask = xtrymalloc_secure (nframe - hlen - 1);
    if (!dmask)
      {
        rc = gpg_err_code_from_syserror ();
        xfree (frame);
        return rc;
      }
    rc = mgf1 (dmask, nframe - hlen - 1, frame+1, hlen, algo);
    if (rc)
      {
        xfree (dmask);
        xfree (frame);
        return rc;
      }
    for (n = 1 + hlen, p = dmask; n < nframe; n++)
      frame[n] ^= *p++;
    xfree (dmask);
  }

  /* Step 2g and 2h: Create maskedSeed.  */
  {
    unsigned char *smask;

    smask = xtrymalloc_secure (hlen);
    if (!smask)
      {
        rc = gpg_err_code_from_syserror ();
        xfree (frame);
        return rc;
      }
    rc = mgf1 (smask, hlen, frame + 1 + hlen, nframe - hlen - 1, algo);
    if (rc)
      {
        xfree (smask);
        xfree (frame);
        return rc;
      }
    for (n = 1, p = smask; n < 1 + hlen; n++)
      frame[n] ^= *p++;
    xfree (smask);
  }

  /* Step 2i: Concatenate 0x00, maskedSeed and maskedDB.  */
  /* This has already been done by using in-place operations.  */

  /* Convert the stuff into an MPI as expected by the caller.  */
  rc = _gcry_mpi_scan (r_result, GCRYMPI_FMT_USG, frame, nframe, NULL);
  if (!rc && DBG_CIPHER)
    log_mpidump ("OAEP encoded data", *r_result);
  xfree (frame);

  return rc;
}


/* RFC-3447 (pkcs#1 v2.1) OAEP decoding.  NBITS is the length of the
   key measured in bits.  ALGO is the hash function; it must be a
   valid and usable algorithm.  VALUE is the raw decrypted message
   {LABEL,LABELLEN} is the optional label to be associated with the
   message, if LABEL is NULL the default is to use the empty string as
   label.  On success the plaintext is returned as a newly allocated
   buffer at R_RESULT; its valid length is stored at R_RESULTLEN.  On
   error NULL is stored at R_RESULT.  */
gpg_err_code_t
_gcry_rsa_oaep_decode (unsigned char **r_result, size_t *r_resultlen,
                       unsigned int nbits, int algo,
                       gcry_mpi_t value,
                       const unsigned char *label, size_t labellen)
{
  gcry_err_code_t rc;
  unsigned char *frame = NULL; /* Encoded messages (EM).  */
  unsigned char *masked_seed;  /* Points into FRAME.  */
  unsigned char *masked_db;    /* Points into FRAME.  */
  unsigned char *seed = NULL;  /* Allocated space for the seed and DB.  */
  unsigned char *db;           /* Points into SEED.  */
  unsigned char *lhash = NULL; /* Hash of the label.  */
  size_t nframe;               /* Length of the ciphertext (EM).  */
  size_t hlen;                 /* Length of the hash digest.  */
  size_t db_len;               /* Length of DB and masked_db.  */
  size_t nkey = (nbits+7)/8;   /* Length of the key in bytes.  */
  int failed = 0;              /* Error indicator.  */
  size_t n;

  *r_result = NULL;

  /* This code is implemented as described by rfc-3447 7.1.2.  */

  /* Set defaults for LABEL.  */
  if (!label || !labellen)
    {
      label = (const unsigned char*)"";
      labellen = 0;
    }

  /* Get the length of the digest.  */
  hlen = _gcry_md_get_algo_dlen (algo);

  /* Hash the label right away.  */
  lhash = xtrymalloc (hlen);
  if (!lhash)
    return gpg_err_code_from_syserror ();
  _gcry_md_hash_buffer (algo, lhash, label, labellen);

  /* Turn the MPI into an octet string.  If the octet string is
     shorter than the key we pad it to the left with zeroes.  This may
     happen due to the leading zero in OAEP frames and due to the
     following random octets (seed^mask) which may have leading zero
     bytes.  This all is needed to cope with our leading zeroes
     suppressing MPI implementation.  The code implictly implements
     Step 1b (bail out if NFRAME != N).  */
  rc = octet_string_from_mpi (&frame, NULL, value, nkey);
  if (rc)
    {
      xfree (lhash);
      return GPG_ERR_ENCODING_PROBLEM;
    }
  nframe = nkey;

  /* Step 1c: Check that the key is long enough.  */
  if ( nframe < 2 * hlen + 2 )
    {
      xfree (frame);
      xfree (lhash);
      return GPG_ERR_ENCODING_PROBLEM;
    }

  /* Step 2 has already been done by the caller and the
     gcry_mpi_aprint above.  */

  /* Allocate space for SEED and DB.  */
  seed = xtrymalloc_secure (nframe - 1);
  if (!seed)
    {
      rc = gpg_err_code_from_syserror ();
      xfree (frame);
      xfree (lhash);
      return rc;
    }
  db = seed + hlen;

  /* To avoid chosen ciphertext attacks from now on we make sure to
     run all code even in the error case; this avoids possible timing
     attacks as described by Manger.  */

  /* Step 3a: Hash the label.  */
  /* This has already been done.  */

  /* Step 3b: Separate the encoded message.  */
  masked_seed = frame + 1;
  masked_db   = frame + 1 + hlen;
  db_len      = nframe - 1 - hlen;

  /* Step 3c and 3d: seed = maskedSeed ^ mgf(maskedDB, hlen).  */
  if (mgf1 (seed, hlen, masked_db, db_len, algo))
    failed = 1;
  for (n = 0; n < hlen; n++)
    seed[n] ^= masked_seed[n];

  /* Step 3e and 3f: db = maskedDB ^ mgf(seed, db_len).  */
  if (mgf1 (db, db_len, seed, hlen, algo))
    failed = 1;
  for (n = 0; n < db_len; n++)
    db[n] ^= masked_db[n];

  /* Step 3g: Check lhash, an possible empty padding string terminated
     by 0x01 and the first byte of EM being 0.  */
  if (memcmp (lhash, db, hlen))
    failed = 1;
  for (n = hlen; n < db_len; n++)
    if (db[n] == 0x01)
      break;
  if (n == db_len)
    failed = 1;
  if (frame[0])
    failed = 1;

  xfree (lhash);
  xfree (frame);
  if (failed)
    {
      xfree (seed);
      return GPG_ERR_ENCODING_PROBLEM;
    }

  /* Step 4: Output M.  */
  /* To avoid an extra allocation we reuse the seed buffer.  The only
     caller of this function will anyway free the result soon.  */
  n++;
  memmove (seed, db + n, db_len - n);
  *r_result = seed;
  *r_resultlen = db_len - n;
  seed = NULL;

  if (DBG_CIPHER)
    log_printhex ("value extracted from OAEP encoded data",
                  *r_result, *r_resultlen);

  return 0;
}


/* RFC-3447 (pkcs#1 v2.1) PSS encoding.  Encode {VALUE,VALUELEN} for
   an NBITS key.  ALGO is a valid hash algorithm and SALTLEN is the
   length of salt to be used.  When HASHED_ALREADY is set, VALUE is
   already the mHash from the picture below.  Otherwise, VALUE is M.

   On success the result is stored as a new MPI at R_RESULT.  On error
   the value at R_RESULT is undefined.

   If RANDOM_OVERRIDE is given it is used as the salt instead of using
   a random string for the salt.  This feature is only useful for
   regression tests.

   Here is figure 2 from the RFC (errata 595 applied) depicting the
   process:

                                  +-----------+
                                  |     M     |
                                  +-----------+
                                        |
                                        V
                                      Hash
                                        |
                                        V
                          +--------+----------+----------+
                     M' = |Padding1|  mHash   |   salt   |
                          +--------+----------+----------+
                                         |
               +--------+----------+     V
         DB =  |Padding2| salt     |   Hash
               +--------+----------+     |
                         |               |
                         V               |    +----+
                        xor <--- MGF <---|    |0xbc|
                         |               |    +----+
                         |               |      |
                         V               V      V
               +-------------------+----------+----+
         EM =  |    maskedDB       |     H    |0xbc|
               +-------------------+----------+----+

  */
gpg_err_code_t
_gcry_rsa_pss_encode (gcry_mpi_t *r_result, unsigned int nbits, int algo,
                      int saltlen, int hashed_already,
                      const unsigned char *value, size_t valuelen,
                      const void *random_override)
{
  gcry_err_code_t rc = 0;
  gcry_md_hd_t hd = NULL;
  unsigned char *digest;
  size_t hlen;                 /* Length of the hash digest.  */
  unsigned char *em = NULL;    /* Encoded message.  */
  size_t emlen = (nbits+7)/8;  /* Length in bytes of EM.  */
  unsigned char *h;            /* Points into EM.  */
  unsigned char *buf = NULL;   /* Help buffer.  */
  size_t buflen;               /* Length of BUF.  */
  unsigned char *mhash;        /* Points into BUF.  */
  unsigned char *salt;         /* Points into BUF.  */
  unsigned char *dbmask;       /* Points into BUF.  */
  unsigned char *p;
  size_t n;


  /* This code is implemented as described by rfc-3447 9.1.1.  */

  rc = _gcry_md_open (&hd, algo, 0);
  if (rc)
    return rc;

  /* Get the length of the digest.  */
  hlen = _gcry_md_get_algo_dlen (algo);
  gcry_assert (hlen);  /* We expect a valid ALGO here.  */

  /* The FIPS 186-4 Section 5.5 allows only 0 <= sLen <= hLen */
  if (fips_mode () && saltlen > hlen)
    {
      rc = GPG_ERR_INV_ARG;
      goto leave;
    }

  /* Allocate a help buffer and setup some pointers.  */
  buflen = 8 + hlen + saltlen + (emlen - hlen - 1);
  buf = xtrymalloc (buflen);
  if (!buf)
    {
      rc = gpg_err_code_from_syserror ();
      goto leave;
    }
  mhash = buf + 8;
  salt  = mhash + hlen;
  dbmask= salt + saltlen;

  /* Step 2: mHash = Hash(M) (or copy input to mHash, if already hashed).   */
  if (!hashed_already)
    {
      _gcry_md_write (hd, value, valuelen);
      digest = _gcry_md_read (hd, 0);
      memcpy (mhash, digest, hlen);
      _gcry_md_reset (hd);
    }
  else
    {
      if (valuelen != hlen)
        {
          rc = GPG_ERR_INV_LENGTH;
          goto leave;
        }
      memcpy (mhash, value, hlen);
    }

  /* Step 3: Check length constraints.  */
  if (emlen < hlen + saltlen + 2)
    {
      rc = GPG_ERR_TOO_SHORT;
      goto leave;
    }

  /* Allocate space for EM.  */
  em = xtrymalloc (emlen);
  if (!em)
    {
      rc = gpg_err_code_from_syserror ();
      goto leave;
    }
  h = em + emlen - 1 - hlen;

  /* Step 4: Create a salt.  */
  if (saltlen)
    {
      if (random_override)
        memcpy (salt, random_override, saltlen);
      else
        _gcry_randomize (salt, saltlen, GCRY_STRONG_RANDOM);
    }

  /* Step 5 and 6: M' = Hash(Padding1 || mHash || salt).  */
  memset (buf, 0, 8);  /* Padding.  */

  _gcry_md_write (hd, buf, 8 + hlen + saltlen);
  digest = _gcry_md_read (hd, 0);
  memcpy (h, digest, hlen);

  /* Step 7 and 8: DB = PS || 0x01 || salt.  */
  /* Note that we use EM to store DB and later Xor in-place.  */
  p = em + emlen - 1 - hlen - saltlen - 1;
  memset (em, 0, p - em);
  *p++ = 0x01;
  memcpy (p, salt, saltlen);

  /* Step 9: dbmask = MGF(H, emlen - hlen - 1).  */
  mgf1 (dbmask, emlen - hlen - 1, h, hlen, algo);

  /* Step 10: maskedDB = DB ^ dbMask */
  for (n = 0, p = dbmask; n < emlen - hlen - 1; n++, p++)
    em[n] ^= *p;

  /* Step 11: Set the leftmost bits to zero.  */
  em[0] &= 0xFF >> (8 * emlen - nbits);

  /* Step 12: EM = maskedDB || H || 0xbc.  */
  em[emlen-1] = 0xbc;

  /* Convert EM into an MPI.  */
  rc = _gcry_mpi_scan (r_result, GCRYMPI_FMT_USG, em, emlen, NULL);
  if (!rc && DBG_CIPHER)
    log_mpidump ("PSS encoded data", *r_result);

 leave:
  _gcry_md_close (hd);
  if (em)
    {
      wipememory (em, emlen);
      xfree (em);
    }
  if (buf)
    {
      wipememory (buf, buflen);
      xfree (buf);
    }
  return rc;
}


/* Verify a signature assuming PSS padding.  When HASHED_ALREADY is
   set, VALUE is the hash of the message (mHash); its length must
   match the digest length of ALGO.  Otherwise, its M (before mHash).
   VALUE is an opaque MPI.  ENCODED is the output of the RSA public
   key function (EM).  NBITS is the size of the public key.  ALGO is
   the hash algorithm and SALTLEN is the length of the used salt.  The
   function returns 0 on success or on error code.  */
gpg_err_code_t
_gcry_rsa_pss_verify (gcry_mpi_t value, int hashed_already,
                      gcry_mpi_t encoded,
                      unsigned int nbits, int algo, size_t saltlen)
{
  gcry_err_code_t rc = 0;
  gcry_md_hd_t hd = NULL;
  unsigned char *digest;
  size_t hlen;                 /* Length of the hash digest.  */
  unsigned char *em = NULL;    /* Encoded message.  */
  size_t emlen = (nbits+7)/8;  /* Length in bytes of EM.  */
  unsigned char *salt;         /* Points into EM.  */
  unsigned char *h;            /* Points into EM.  */
  unsigned char *buf = NULL;   /* Help buffer.  */
  size_t buflen;               /* Length of BUF.  */
  unsigned char *dbmask;       /* Points into BUF.  */
  unsigned char *mhash;        /* Points into BUF.  */
  unsigned char *p;
  size_t n;
  unsigned int input_nbits;

  /* This code is implemented as described by rfc-3447 9.1.2.  */

  rc = _gcry_md_open (&hd, algo, 0);
  if (rc)
    return rc;

  /* Get the length of the digest.  */
  hlen = _gcry_md_get_algo_dlen (algo);
  gcry_assert (hlen);  /* We expect a valid ALGO here.  */

  /* The FIPS 186-4 Section 5.5 allows only 0 <= sLen <= hLen */
  if (fips_mode () && saltlen > hlen)
    {
      rc = GPG_ERR_INV_ARG;
      goto leave;
    }

  /* Allocate a help buffer and setup some pointers.
     This buffer is used for two purposes:
        +------------------------------+-------+
     1. | dbmask                       | mHash |
        +------------------------------+-------+
           emlen - hlen - 1              hlen

        +----------+-------+---------+-+-------+
     2. | padding1 | mHash | salt    | | mHash |
        +----------+-------+---------+-+-------+
             8       hlen    saltlen     hlen
  */
  buflen = 8 + hlen + saltlen;
  if (buflen < emlen - hlen - 1)
    buflen = emlen - hlen - 1;
  buflen += hlen;
  buf = xtrymalloc (buflen);
  if (!buf)
    {
      rc = gpg_err_code_from_syserror ();
      goto leave;
    }
  dbmask = buf;
  mhash = buf + buflen - hlen;

  /* Step 2: mHash = Hash(M) (or copy input to mHash, if already hashed).   */
  p = mpi_get_opaque (value, &input_nbits);
  if (!p)
    {
      rc = GPG_ERR_INV_ARG;
      goto leave;
    }

  if (!hashed_already)
    {
      _gcry_md_write (hd, p, (input_nbits+7)/8);
      digest = _gcry_md_read (hd, 0);
      memcpy (mhash, digest, hlen);
      _gcry_md_reset (hd);
    }
  else
    memcpy (mhash, p, hlen);

  /* Convert the signature into an octet string.  */
  rc = octet_string_from_mpi (&em, NULL, encoded, emlen);
  if (rc)
    goto leave;

  /* Step 3: Check length of EM.  Because we internally use MPI
     functions we can't do this properly; EMLEN is always the length
     of the key because octet_string_from_mpi needs to left pad the
     result with zero to cope with the fact that our MPIs suppress all
     leading zeroes.  Thus what we test here are merely the digest and
     salt lengths to the key.  */
  if (emlen < hlen + saltlen + 2)
    {
      rc = GPG_ERR_TOO_SHORT; /* For the hash and saltlen.  */
      goto leave;
    }

  /* Step 4: Check last octet.  */
  if (em[emlen - 1] != 0xbc)
    {
      rc = GPG_ERR_BAD_SIGNATURE;
      goto leave;
    }

  /* Step 5: Split EM.  */
  h = em + emlen - 1 - hlen;

  /* Step 6: Check the leftmost bits.  */
  if ((em[0] & ~(0xFF >> (8 * emlen - nbits))))
    {
      rc = GPG_ERR_BAD_SIGNATURE;
      goto leave;
    }

  /* Step 7: dbmask = MGF(H, emlen - hlen - 1).  */
  mgf1 (dbmask, emlen - hlen - 1, h, hlen, algo);

  /* Step 8: maskedDB = DB ^ dbMask.  */
  for (n = 0, p = dbmask; n < emlen - hlen - 1; n++, p++)
    em[n] ^= *p;

  /* Step 9: Set leftmost bits in DB to zero.  */
  em[0] &= 0xFF >> (8 * emlen - nbits);

  /* Step 10: Check the padding of DB.  */
  for (n = 0; n < emlen - hlen - saltlen - 2 && !em[n]; n++)
    ;
  if (n != emlen - hlen - saltlen - 2 || em[n++] != 1)
    {
      rc = GPG_ERR_BAD_SIGNATURE;
      goto leave;
    }

  /* Step 11: Extract salt from DB.  */
  salt = em + n;

  /* Step 12:  M' = (0x)00 00 00 00 00 00 00 00 || mHash || salt */
  memset (buf, 0, 8);
  memcpy (buf+8, mhash, hlen);
  memcpy (buf+8+hlen, salt, saltlen);

  /* Step 13:  H' = Hash(M').  */
  _gcry_md_write (hd, buf, 8 + hlen + saltlen);
  digest = _gcry_md_read (hd, 0);
  memcpy (buf, digest, hlen);

  /* Step 14:  Check H == H'.   */
  rc = memcmp (h, buf, hlen) ? GPG_ERR_BAD_SIGNATURE : GPG_ERR_NO_ERROR;

 leave:
  _gcry_md_close (hd);
  if (em)
    {
      wipememory (em, emlen);
      xfree (em);
    }
  if (buf)
    {
      wipememory (buf, buflen);
      xfree (buf);
    }
  return rc;
}
