/* dsa-common.c - Common code for DSA
 * Copyright (C) 1998, 1999 Free Software Foundation, Inc.
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


/*
 * Modify K, so that computation time difference can be small,
 * by making K large enough.
 *
 * Originally, (EC)DSA computation requires k where 0 < k < q.  Here,
 * we add q (the order), to keep k in a range: q < k < 2*q (or,
 * addming more q, to keep k in a range: 2*q < k < 3*q), so that
 * timing difference of the EC multiply (or exponentiation) operation
 * can be small.  The result of (EC)DSA computation is same.
 */
void
_gcry_dsa_modify_k (gcry_mpi_t k, gcry_mpi_t q, int qbits)
{
  gcry_mpi_t k1 = mpi_new (qbits+2);

  mpi_resize (k, (qbits+2+BITS_PER_MPI_LIMB-1) / BITS_PER_MPI_LIMB);
  k->nlimbs = k->alloced;
  mpi_add (k, k, q);
  mpi_add (k1, k, q);
  mpi_set_cond (k, k1, !mpi_test_bit (k, qbits));

  mpi_free (k1);
}

/*
 * Generate a random secret exponent K less than Q.
 * Note that ECDSA uses this code also to generate D.
 */
gcry_mpi_t
_gcry_dsa_gen_k (gcry_mpi_t q, int security_level)
{
  gcry_mpi_t k        = mpi_alloc_secure (mpi_get_nlimbs (q));
  unsigned int nbits  = mpi_get_nbits (q);
  unsigned int nbytes = (nbits+7)/8;
  char *rndbuf = NULL;

  /* To learn why we don't use mpi_mod to get the requested bit size,
     read the paper: "The Insecurity of the Digital Signature
     Algorithm with Partially Known Nonces" by Nguyen and Shparlinski.
     Journal of Cryptology, New York. Vol 15, nr 3 (2003)  */

  if (DBG_CIPHER)
    log_debug ("choosing a random k of %u bits at seclevel %d\n",
               nbits, security_level);
  for (;;)
    {
      if ( !rndbuf || nbits < 32 )
        {
          xfree (rndbuf);
          rndbuf = _gcry_random_bytes_secure (nbytes, security_level);
	}
      else
        { /* Change only some of the higher bits.  We could improve
	     this by directly requesting more memory at the first call
	     to get_random_bytes() and use these extra bytes here.
	     However the required management code is more complex and
	     thus we better use this simple method.  */
          char *pp = _gcry_random_bytes_secure (4, security_level);
          memcpy (rndbuf, pp, 4);
          xfree (pp);
	}
      _gcry_mpi_set_buffer (k, rndbuf, nbytes, 0);

      /* Make sure we have the requested number of bits.  This code
         looks a bit funny but it is easy to understand if you
         consider that mpi_set_highbit clears all higher bits.  We
         don't have a clear_highbit, thus we first set the high bit
         and then clear it again.  */
      if (mpi_test_bit (k, nbits-1))
        mpi_set_highbit (k, nbits-1);
      else
        {
          mpi_set_highbit (k, nbits-1);
          mpi_clear_bit (k, nbits-1);
	}

      if (!(mpi_cmp (k, q) < 0))    /* check: k < q */
        {
          if (DBG_CIPHER)
            log_debug ("\tk too large - again\n");
          continue; /* no  */
        }
      if (!(mpi_cmp_ui (k, 0) > 0)) /* check: k > 0 */
        {
          if (DBG_CIPHER)
            log_debug ("\tk is zero - again\n");
          continue; /* no */
        }
      break;	/* okay */
    }
  xfree (rndbuf);

  return k;
}


/* Turn VALUE into an octet string and store it in an allocated buffer
   at R_FRAME.  If the resulting octet string is shorter than NBYTES
   the result will be left padded with zeroes.  If VALUE does not fit
   into NBYTES an error code is returned.  */
static gpg_err_code_t
int2octets (unsigned char **r_frame, gcry_mpi_t value, size_t nbytes)
{
  gpg_err_code_t rc;
  size_t nframe, noff, n;
  unsigned char *frame;

  rc = _gcry_mpi_print (GCRYMPI_FMT_USG, NULL, 0, &nframe, value);
  if (rc)
    return rc;
  if (nframe > nbytes)
    return GPG_ERR_TOO_LARGE; /* Value too long to fit into NBYTES.  */

  noff = (nframe < nbytes)? nbytes - nframe : 0;
  n = nframe + noff;
  frame = mpi_is_secure (value)? xtrymalloc_secure (n) : xtrymalloc (n);
  if (!frame)
    return gpg_err_code_from_syserror ();
  if (noff)
    memset (frame, 0, noff);
  nframe += noff;
  rc = _gcry_mpi_print (GCRYMPI_FMT_USG, frame+noff, nframe-noff, NULL, value);
  if (rc)
    {
      xfree (frame);
      return rc;
    }

  *r_frame = frame;
  return 0;
}


/* Connert the bit string BITS of length NBITS into an octet string
   with a length of (QBITS+7)/8 bytes.  On success store the result at
   R_FRAME.  */
static gpg_err_code_t
bits2octets (unsigned char **r_frame,
             const void *bits, unsigned int nbits,
             gcry_mpi_t q, unsigned int qbits)
{
  gpg_err_code_t rc;
  gcry_mpi_t z1;

  /* z1 = bits2int (b) */
  rc = _gcry_mpi_scan (&z1, GCRYMPI_FMT_USG, bits, (nbits+7)/8, NULL);
  if (rc)
    return rc;
  if (nbits > qbits)
    mpi_rshift (z1, z1, nbits - qbits);

  /* z2 - z1 mod q */
  if (mpi_cmp (z1, q) >= 0)
    mpi_sub (z1, z1, q);

  /* Convert to an octet string.  */
  rc = int2octets (r_frame, z1, (qbits+7)/8);

  mpi_free (z1);
  return rc;
}


/*
 * Generate a deterministic secret exponent K less than DSA_Q.  H1 is
 * the to be signed digest with a length of HLEN bytes.  HALGO is the
 * algorithm used to create the hash.  On success the value for K is
 * stored at R_K.
 */
gpg_err_code_t
_gcry_dsa_gen_rfc6979_k (gcry_mpi_t *r_k,
                         gcry_mpi_t dsa_q, gcry_mpi_t dsa_x,
                         const unsigned char *h1, unsigned int hlen,
                         int halgo, unsigned int extraloops)
{
  gpg_err_code_t rc;
  unsigned char *V = NULL;
  unsigned char *K = NULL;
  unsigned char *x_buf = NULL;
  unsigned char *h1_buf = NULL;
  gcry_md_hd_t hd = NULL;
  unsigned char *t = NULL;
  gcry_mpi_t k = NULL;
  unsigned int tbits, qbits;
  int i;

  qbits = mpi_get_nbits (dsa_q);

  if (!qbits || !h1 || !hlen)
    return GPG_ERR_EINVAL;

  if (_gcry_md_get_algo_dlen (halgo) != hlen)
    return GPG_ERR_DIGEST_ALGO;

  /* Step b:  V = 0x01 0x01 0x01 ... 0x01 */
  V = xtrymalloc (hlen);
  if (!V)
    {
      rc = gpg_err_code_from_syserror ();
      goto leave;
    }
  for (i=0; i < hlen; i++)
    V[i] = 1;

  /* Step c:  K = 0x00 0x00 0x00 ... 0x00 */
  K = xtrycalloc (1, hlen);
  if (!K)
    {
      rc = gpg_err_code_from_syserror ();
      goto leave;
    }

  rc = int2octets (&x_buf, dsa_x, (qbits+7)/8);
  if (rc)
    goto leave;

  rc = bits2octets (&h1_buf, h1, hlen*8, dsa_q, qbits);
  if (rc)
    goto leave;

  /* Create a handle to compute the HMACs.  */
  rc = _gcry_md_open (&hd, halgo, (GCRY_MD_FLAG_SECURE | GCRY_MD_FLAG_HMAC));
  if (rc)
    goto leave;

  /* Step d:  K = HMAC_K(V || 0x00 || int2octets(x) || bits2octets(h1) */
  rc = _gcry_md_setkey (hd, K, hlen);
  if (rc)
    goto leave;
  _gcry_md_write (hd, V, hlen);
  _gcry_md_write (hd, "", 1);
  _gcry_md_write (hd, x_buf, (qbits+7)/8);
  _gcry_md_write (hd, h1_buf, (qbits+7)/8);
  memcpy (K, _gcry_md_read (hd, 0), hlen);

  /* Step e:  V = HMAC_K(V) */
  rc = _gcry_md_setkey (hd, K, hlen);
  if (rc)
    goto leave;
  _gcry_md_write (hd, V, hlen);
  memcpy (V, _gcry_md_read (hd, 0), hlen);

  /* Step f:  K = HMAC_K(V || 0x01 || int2octets(x) || bits2octets(h1) */
  rc = _gcry_md_setkey (hd, K, hlen);
  if (rc)
    goto leave;
  _gcry_md_write (hd, V, hlen);
  _gcry_md_write (hd, "\x01", 1);
  _gcry_md_write (hd, x_buf, (qbits+7)/8);
  _gcry_md_write (hd, h1_buf, (qbits+7)/8);
  memcpy (K, _gcry_md_read (hd, 0), hlen);

  /* Step g:  V = HMAC_K(V) */
  rc = _gcry_md_setkey (hd, K, hlen);
  if (rc)
    goto leave;
  _gcry_md_write (hd, V, hlen);
  memcpy (V, _gcry_md_read (hd, 0), hlen);

  /* Step h. */
  t = xtrymalloc_secure ((qbits+7)/8+hlen);
  if (!t)
    {
      rc = gpg_err_code_from_syserror ();
      goto leave;
    }

 again:
  for (tbits = 0; tbits < qbits;)
    {
      /* V = HMAC_K(V) */
      rc = _gcry_md_setkey (hd, K, hlen);
      if (rc)
        goto leave;
      _gcry_md_write (hd, V, hlen);
      memcpy (V, _gcry_md_read (hd, 0), hlen);

      /* T = T || V */
      memcpy (t+(tbits+7)/8, V, hlen);
      tbits += 8*hlen;
    }

  /* k = bits2int (T) */
  mpi_free (k);
  k = NULL;
  rc = _gcry_mpi_scan (&k, GCRYMPI_FMT_USG, t, (tbits+7)/8, NULL);
  if (rc)
    goto leave;
  if (tbits > qbits)
    mpi_rshift (k, k, tbits - qbits);

  /* Check: k < q and k > 1 */
  if (!(mpi_cmp (k, dsa_q) < 0 && mpi_cmp_ui (k, 0) > 0))
    {
      /* K = HMAC_K(V || 0x00) */
      rc = _gcry_md_setkey (hd, K, hlen);
      if (rc)
        goto leave;
      _gcry_md_write (hd, V, hlen);
      _gcry_md_write (hd, "", 1);
      memcpy (K, _gcry_md_read (hd, 0), hlen);

      /* V = HMAC_K(V) */
      rc = _gcry_md_setkey (hd, K, hlen);
      if (rc)
        goto leave;
      _gcry_md_write (hd, V, hlen);
      memcpy (V, _gcry_md_read (hd, 0), hlen);

      goto again;
    }

  /* The caller may have requested that we introduce some extra loops.
     This is for example useful if the caller wants another value for
     K because the last returned one yielded an R of 0.  Because this
     is very unlikely we implement it in a straightforward way.  */
  if (extraloops)
    {
      extraloops--;

      /* K = HMAC_K(V || 0x00) */
      rc = _gcry_md_setkey (hd, K, hlen);
      if (rc)
        goto leave;
      _gcry_md_write (hd, V, hlen);
      _gcry_md_write (hd, "", 1);
      memcpy (K, _gcry_md_read (hd, 0), hlen);

      /* V = HMAC_K(V) */
      rc = _gcry_md_setkey (hd, K, hlen);
      if (rc)
        goto leave;
      _gcry_md_write (hd, V, hlen);
      memcpy (V, _gcry_md_read (hd, 0), hlen);

      goto again;
    }

  /* log_mpidump ("  k", k); */

 leave:
  xfree (t);
  _gcry_md_close (hd);
  xfree (h1_buf);
  xfree (x_buf);
  xfree (K);
  xfree (V);

  if (rc)
    mpi_free (k);
  else
    *r_k = k;
  return rc;
}



/*
 * For DSA/ECDSA, as prehash function, compute hash with HASHALGO for
 * INPUT.  Result hash value is returned in R_HASH as an opaque MPI.
 * Returns error code.
 */
gpg_err_code_t
_gcry_dsa_compute_hash (gcry_mpi_t *r_hash, gcry_mpi_t input, int hashalgo)
{
  gpg_err_code_t rc = 0;
  size_t hlen;
  void *hashbuf;
  void *abuf;
  unsigned int abits;
  unsigned int n;

  hlen = _gcry_md_get_algo_dlen (hashalgo);
  hashbuf = xtrymalloc (hlen);
  if (!hashbuf)
    {
      rc = gpg_err_code_from_syserror ();
      return rc;
    }

  if (mpi_is_opaque (input))
    {
      abuf = mpi_get_opaque (input, &abits);
      n = (abits+7)/8;
      _gcry_md_hash_buffer (hashalgo, hashbuf, abuf, n);
    }
  else
    {
      abits = mpi_get_nbits (input);
      n = (abits+7)/8;
      abuf = xtrymalloc (n);
      if (!abuf)
        {
          rc = gpg_err_code_from_syserror ();
          xfree (hashbuf);
          return rc;
        }
      _gcry_mpi_to_octet_string (NULL, abuf, input, n);
      _gcry_md_hash_buffer (hashalgo, hashbuf, abuf, n);
      xfree (abuf);
    }

  *r_hash = mpi_set_opaque (NULL, hashbuf, hlen*8);
  if (!*r_hash)
    rc = GPG_ERR_INV_OBJ;

  return rc;
}


/*
 * Truncate opaque hash value to qbits for DSA.
 * Non-opaque input is not truncated, in hope that user
 * knows what is passed. It is not possible to correctly
 * trucate non-opaque inputs.
 */
gpg_err_code_t
_gcry_dsa_normalize_hash (gcry_mpi_t input,
                          gcry_mpi_t *out,
                          unsigned int qbits)
{
  gpg_err_code_t rc = 0;
  const void *abuf;
  unsigned int abits;
  gcry_mpi_t hash;

  if (mpi_is_opaque (input))
    {
      abuf = mpi_get_opaque (input, &abits);
      rc = _gcry_mpi_scan (&hash, GCRYMPI_FMT_USG, abuf, (abits+7)/8, NULL);
      if (rc)
        return rc;
      if (abits > qbits)
        mpi_rshift (hash, hash, abits - qbits);
    }
  else
    hash = input;

  *out = hash;

  return rc;
}
