/* cipher-ocb.c -  OCB cipher mode
 * Copyright (C) 2015, 2016 g10 Code GmbH
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
 *
 *
 * OCB is covered by several patents but may be used freely by most
 * software.  See http://web.cs.ucdavis.edu/~rogaway/ocb/license.htm .
 * In particular license 1 is suitable for Libgcrypt: See
 * http://web.cs.ucdavis.edu/~rogaway/ocb/license1.pdf for the full
 * license document; it basically says:
 *
 *   License 1 — License for Open-Source Software Implementations of OCB
 *               (Jan 9, 2013)
 *
 *   Under this license, you are authorized to make, use, and
 *   distribute open-source software implementations of OCB. This
 *   license terminates for you if you sue someone over their
 *   open-source software implementation of OCB claiming that you have
 *   a patent covering their implementation.
 */


#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "g10lib.h"
#include "cipher.h"
#include "bufhelp.h"
#include "./cipher-internal.h"


/* Double the OCB_BLOCK_LEN sized block B in-place.  */
static inline void
double_block (u64 b[2])
{
  u64 l_0, l, r;

  l = b[1];
  r = b[0];

  l_0 = -(l >> 63);
  l = (l + l) ^ (r >> 63);
  r = (r + r) ^ (l_0 & 135);

  b[1] = l;
  b[0] = r;
}


/* Copy OCB_BLOCK_LEN from buffer S starting at bit offset BITOFF to
 * buffer D.  */
static void
bit_copy (unsigned char d[16], const unsigned char s[24], unsigned int bitoff)
{
  u64 s0l, s1l, s1r, s2r;
  unsigned int shift;
  unsigned int byteoff;

  byteoff = bitoff / 8;
  shift = bitoff % 8;

  s0l = buf_get_be64 (s + byteoff + 0);
  s1l = buf_get_be64 (s + byteoff + 8);
  s1r = shift ? s1l : 0;
  s2r = shift ? buf_get_be64 (s + 16) << (8 * byteoff) : 0;

  buf_put_be64 (d + 0, (s0l << shift) | (s1r >> ((64 - shift) & 63)));
  buf_put_be64 (d + 8, (s1l << shift) | (s2r >> ((64 - shift) & 63)));
}


/* Get L_big value for block N, where N is multiple of 65536. */
static void
ocb_get_L_big (gcry_cipher_hd_t c, u64 n, unsigned char *l_buf)
{
  int ntz = _gcry_ctz64 (n);
  u64 L[2];

  gcry_assert(ntz >= OCB_L_TABLE_SIZE);

  L[1] = buf_get_be64 (c->u_mode.ocb.L[OCB_L_TABLE_SIZE - 1]);
  L[0] = buf_get_be64 (c->u_mode.ocb.L[OCB_L_TABLE_SIZE - 1] + 8);

  for (ntz -= OCB_L_TABLE_SIZE - 1; ntz; ntz--)
    double_block (L);

  buf_put_be64 (l_buf + 0, L[1]);
  buf_put_be64 (l_buf + 8, L[0]);
}


/* Called after key has been set. Sets up L table. */
void
_gcry_cipher_ocb_setkey (gcry_cipher_hd_t c)
{
  unsigned char ktop[OCB_BLOCK_LEN];
  unsigned int burn = 0;
  unsigned int nburn;
  u64 L[2];
  int i;

  /* L_star = E(zero_128) */
  memset (ktop, 0, OCB_BLOCK_LEN);
  nburn = c->spec->encrypt (&c->context.c, c->u_mode.ocb.L_star, ktop);
  burn = nburn > burn ? nburn : burn;
  /* L_dollar = double(L_star)  */
  L[1] = buf_get_be64 (c->u_mode.ocb.L_star);
  L[0] = buf_get_be64 (c->u_mode.ocb.L_star + 8);
  double_block (L);
  buf_put_be64 (c->u_mode.ocb.L_dollar + 0, L[1]);
  buf_put_be64 (c->u_mode.ocb.L_dollar + 8, L[0]);
  /* L_0 = double(L_dollar), ...  */
  double_block (L);
  buf_put_be64 (c->u_mode.ocb.L[0] + 0, L[1]);
  buf_put_be64 (c->u_mode.ocb.L[0] + 8, L[0]);
  for (i = 1; i < OCB_L_TABLE_SIZE; i++)
    {
      double_block (L);
      buf_put_be64 (c->u_mode.ocb.L[i] + 0, L[1]);
      buf_put_be64 (c->u_mode.ocb.L[i] + 8, L[0]);
    }
  /* Precalculated offset L0+L1 */
  cipher_block_xor (c->u_mode.ocb.L0L1,
		    c->u_mode.ocb.L[0], c->u_mode.ocb.L[1], OCB_BLOCK_LEN);

  /* Cleanup */
  wipememory (ktop, sizeof ktop);
  if (burn > 0)
    _gcry_burn_stack (burn + 4*sizeof(void*));
}


/* Set the nonce for OCB.  This requires that the key has been set.
   Using it again resets start a new encryption cycle using the same
   key.  */
gcry_err_code_t
_gcry_cipher_ocb_set_nonce (gcry_cipher_hd_t c, const unsigned char *nonce,
                            size_t noncelen)
{
  unsigned char ktop[OCB_BLOCK_LEN];
  unsigned char stretch[OCB_BLOCK_LEN + 8];
  unsigned int bottom;
  unsigned int burn = 0;
  unsigned int nburn;

  /* Check args.  */
  if (!c->marks.key)
    return GPG_ERR_INV_STATE;  /* Key must have been set first.  */
  switch (c->u_mode.ocb.taglen)
    {
    case 8:
    case 12:
    case 16:
      break;
    default:
      return GPG_ERR_BUG; /* Invalid tag length. */
    }

  if (c->spec->blocksize != OCB_BLOCK_LEN)
    return GPG_ERR_CIPHER_ALGO;
  if (!nonce)
    return GPG_ERR_INV_ARG;
  /* 120 bit is the allowed maximum.  In addition we impose a minimum
     of 64 bit.  */
  if (noncelen > (120/8) || noncelen < (64/8) || noncelen >= OCB_BLOCK_LEN)
    return GPG_ERR_INV_LENGTH;

  /* Prepare the nonce.  */
  memset (ktop, 0, OCB_BLOCK_LEN);
  buf_cpy (ktop + (OCB_BLOCK_LEN - noncelen), nonce, noncelen);
  ktop[0] = ((c->u_mode.ocb.taglen * 8) % 128) << 1;
  ktop[OCB_BLOCK_LEN - noncelen - 1] |= 1;
  bottom = ktop[OCB_BLOCK_LEN - 1] & 0x3f;
  ktop[OCB_BLOCK_LEN - 1] &= 0xc0; /* Zero the bottom bits.  */
  nburn = c->spec->encrypt (&c->context.c, ktop, ktop);
  burn = nburn > burn ? nburn : burn;
  /* Stretch = Ktop || (Ktop[1..64] xor Ktop[9..72]) */
  cipher_block_cpy (stretch, ktop, OCB_BLOCK_LEN);
  cipher_block_xor (stretch + OCB_BLOCK_LEN, ktop, ktop + 1, 8);
  /* Offset_0 = Stretch[1+bottom..128+bottom]
     (We use the IV field to store the offset) */
  bit_copy (c->u_iv.iv, stretch, bottom);
  c->marks.iv = 1;

  /* Checksum_0 = zeros(128)
     (We use the CTR field to store the checksum) */
  memset (c->u_ctr.ctr, 0, OCB_BLOCK_LEN);

  /* Clear AAD buffer.  */
  memset (c->u_mode.ocb.aad_offset, 0, OCB_BLOCK_LEN);
  memset (c->u_mode.ocb.aad_sum, 0, OCB_BLOCK_LEN);

  /* Setup other values.  */
  memset (c->lastiv, 0, sizeof(c->lastiv));
  c->unused = 0;
  c->marks.tag = 0;
  c->marks.finalize = 0;
  c->u_mode.ocb.data_nblocks = 0;
  c->u_mode.ocb.aad_nblocks = 0;
  c->u_mode.ocb.aad_nleftover = 0;
  c->u_mode.ocb.data_finalized = 0;
  c->u_mode.ocb.aad_finalized = 0;

  /* log_printhex ("L_*       ", c->u_mode.ocb.L_star, OCB_BLOCK_LEN); */
  /* log_printhex ("L_$       ", c->u_mode.ocb.L_dollar, OCB_BLOCK_LEN); */
  /* log_printhex ("L_0       ", c->u_mode.ocb.L[0], OCB_BLOCK_LEN); */
  /* log_printhex ("L_1       ", c->u_mode.ocb.L[1], OCB_BLOCK_LEN); */
  /* log_debug (   "bottom    : %u (decimal)\n", bottom); */
  /* log_printhex ("Ktop      ", ktop, OCB_BLOCK_LEN); */
  /* log_printhex ("Stretch   ", stretch, sizeof stretch); */
  /* log_printhex ("Offset_0  ", c->u_iv.iv, OCB_BLOCK_LEN); */

  /* Cleanup */
  wipememory (ktop, sizeof ktop);
  wipememory (stretch, sizeof stretch);
  if (burn > 0)
    _gcry_burn_stack (burn + 4*sizeof(void*));

  return 0;
}


/* Process additional authentication data.  This implementation allows
   to add additional authentication data at any time before the final
   gcry_cipher_gettag.  */
gcry_err_code_t
_gcry_cipher_ocb_authenticate (gcry_cipher_hd_t c, const unsigned char *abuf,
                               size_t abuflen)
{
  const size_t table_maxblks = 1 << OCB_L_TABLE_SIZE;
  const u32 table_size_mask = ((1 << OCB_L_TABLE_SIZE) - 1);
  unsigned char l_tmp[OCB_BLOCK_LEN];
  unsigned int burn = 0;
  unsigned int nburn;
  size_t n;

  /* Check that a nonce and thus a key has been set and that we have
     not yet computed the tag.  We also return an error if the aad has
     been finalized (i.e. a short block has been processed).  */
  if (!c->marks.iv || c->marks.tag || c->u_mode.ocb.aad_finalized)
    return GPG_ERR_INV_STATE;

  /* Check correct usage and arguments.  */
  if (c->spec->blocksize != OCB_BLOCK_LEN)
    return GPG_ERR_CIPHER_ALGO;

  /* Process remaining data from the last call first.  */
  if (c->u_mode.ocb.aad_nleftover)
    {
      n = abuflen;
      if (n > OCB_BLOCK_LEN - c->u_mode.ocb.aad_nleftover)
	n = OCB_BLOCK_LEN - c->u_mode.ocb.aad_nleftover;

      buf_cpy (&c->u_mode.ocb.aad_leftover[c->u_mode.ocb.aad_nleftover],
	       abuf, n);
      c->u_mode.ocb.aad_nleftover += n;
      abuf += n;
      abuflen -= n;

      if (c->u_mode.ocb.aad_nleftover == OCB_BLOCK_LEN)
        {
          c->u_mode.ocb.aad_nblocks++;

          if ((c->u_mode.ocb.aad_nblocks % table_maxblks) == 0)
            {
              /* Table overflow, L needs to be generated. */
              ocb_get_L_big(c, c->u_mode.ocb.aad_nblocks + 1, l_tmp);
            }
          else
            {
              cipher_block_cpy (l_tmp, ocb_get_l (c, c->u_mode.ocb.aad_nblocks),
                                OCB_BLOCK_LEN);
            }

          /* Offset_i = Offset_{i-1} xor L_{ntz(i)} */
          cipher_block_xor_1 (c->u_mode.ocb.aad_offset, l_tmp, OCB_BLOCK_LEN);
          /* Sum_i = Sum_{i-1} xor ENCIPHER(K, A_i xor Offset_i)  */
          cipher_block_xor (l_tmp, c->u_mode.ocb.aad_offset,
                            c->u_mode.ocb.aad_leftover, OCB_BLOCK_LEN);
          nburn = c->spec->encrypt (&c->context.c, l_tmp, l_tmp);
          burn = nburn > burn ? nburn : burn;
          cipher_block_xor_1 (c->u_mode.ocb.aad_sum, l_tmp, OCB_BLOCK_LEN);

          c->u_mode.ocb.aad_nleftover = 0;
        }
    }

  if (!abuflen)
    {
      if (burn > 0)
        _gcry_burn_stack (burn + 4*sizeof(void*));

      return 0;
    }

  /* Full blocks handling. */
  while (abuflen >= OCB_BLOCK_LEN)
    {
      size_t nblks = abuflen / OCB_BLOCK_LEN;
      size_t nmaxblks;

      /* Check how many blocks to process till table overflow. */
      nmaxblks = (c->u_mode.ocb.aad_nblocks + 1) % table_maxblks;
      nmaxblks = (table_maxblks - nmaxblks) % table_maxblks;

      if (nmaxblks == 0)
        {
          /* Table overflow, generate L and process one block. */
          c->u_mode.ocb.aad_nblocks++;
          ocb_get_L_big(c, c->u_mode.ocb.aad_nblocks, l_tmp);

          /* Offset_i = Offset_{i-1} xor L_{ntz(i)} */
          cipher_block_xor_1 (c->u_mode.ocb.aad_offset, l_tmp, OCB_BLOCK_LEN);
          /* Sum_i = Sum_{i-1} xor ENCIPHER(K, A_i xor Offset_i)  */
          cipher_block_xor (l_tmp, c->u_mode.ocb.aad_offset, abuf,
                            OCB_BLOCK_LEN);
          nburn = c->spec->encrypt (&c->context.c, l_tmp, l_tmp);
          burn = nburn > burn ? nburn : burn;
          cipher_block_xor_1 (c->u_mode.ocb.aad_sum, l_tmp, OCB_BLOCK_LEN);

          abuf += OCB_BLOCK_LEN;
          abuflen -= OCB_BLOCK_LEN;
          nblks--;

          /* With overflow handled, retry loop again. Next overflow will
           * happen after 65535 blocks. */
          continue;
        }

      nblks = nblks < nmaxblks ? nblks : nmaxblks;

      /* Use a bulk method if available.  */
      if (nblks && c->bulk.ocb_auth)
        {
          size_t nleft;
          size_t ndone;

          nleft = c->bulk.ocb_auth (c, abuf, nblks);
          ndone = nblks - nleft;

          abuf += ndone * OCB_BLOCK_LEN;
          abuflen -= ndone * OCB_BLOCK_LEN;
          nblks = nleft;
        }

      /* Hash all full blocks.  */
      while (nblks)
        {
          c->u_mode.ocb.aad_nblocks++;

          gcry_assert(c->u_mode.ocb.aad_nblocks & table_size_mask);

          /* Offset_i = Offset_{i-1} xor L_{ntz(i)} */
          cipher_block_xor_1 (c->u_mode.ocb.aad_offset,
                              ocb_get_l (c, c->u_mode.ocb.aad_nblocks),
                              OCB_BLOCK_LEN);
          /* Sum_i = Sum_{i-1} xor ENCIPHER(K, A_i xor Offset_i)  */
          cipher_block_xor (l_tmp, c->u_mode.ocb.aad_offset, abuf,
                            OCB_BLOCK_LEN);
          nburn = c->spec->encrypt (&c->context.c, l_tmp, l_tmp);
          burn = nburn > burn ? nburn : burn;
          cipher_block_xor_1 (c->u_mode.ocb.aad_sum, l_tmp, OCB_BLOCK_LEN);

          abuf += OCB_BLOCK_LEN;
          abuflen -= OCB_BLOCK_LEN;
          nblks--;
        }
    }

  /* Store away the remaining data.  */
  if (abuflen)
    {
      n = abuflen;
      if (n > OCB_BLOCK_LEN - c->u_mode.ocb.aad_nleftover)
	n = OCB_BLOCK_LEN - c->u_mode.ocb.aad_nleftover;

      buf_cpy (&c->u_mode.ocb.aad_leftover[c->u_mode.ocb.aad_nleftover],
	       abuf, n);
      c->u_mode.ocb.aad_nleftover += n;
      abuf += n;
      abuflen -= n;
    }

  gcry_assert (!abuflen);

  if (burn > 0)
    _gcry_burn_stack (burn + 4*sizeof(void*));

  return 0;
}


/* Hash final partial AAD block.  */
static void
ocb_aad_finalize (gcry_cipher_hd_t c)
{
  unsigned char l_tmp[OCB_BLOCK_LEN];
  unsigned int burn = 0;
  unsigned int nburn;

  /* Check that a nonce and thus a key has been set and that we have
     not yet computed the tag.  We also skip this if the aad has been
     finalized.  */
  if (!c->marks.iv || c->marks.tag || c->u_mode.ocb.aad_finalized)
    return;
  if (c->spec->blocksize != OCB_BLOCK_LEN)
    return;  /* Ooops.  */

  /* Hash final partial block if any.  */
  if (c->u_mode.ocb.aad_nleftover)
    {
      /* Offset_* = Offset_m xor L_*  */
      cipher_block_xor_1 (c->u_mode.ocb.aad_offset,
                          c->u_mode.ocb.L_star, OCB_BLOCK_LEN);
      /* CipherInput = (A_* || 1 || zeros(127-bitlen(A_*))) xor Offset_*  */
      buf_cpy (l_tmp, c->u_mode.ocb.aad_leftover, c->u_mode.ocb.aad_nleftover);
      memset (l_tmp + c->u_mode.ocb.aad_nleftover, 0,
              OCB_BLOCK_LEN - c->u_mode.ocb.aad_nleftover);
      l_tmp[c->u_mode.ocb.aad_nleftover] = 0x80;
      cipher_block_xor_1 (l_tmp, c->u_mode.ocb.aad_offset, OCB_BLOCK_LEN);
      /* Sum = Sum_m xor ENCIPHER(K, CipherInput)  */
      nburn = c->spec->encrypt (&c->context.c, l_tmp, l_tmp);
      burn = nburn > burn ? nburn : burn;
      cipher_block_xor_1 (c->u_mode.ocb.aad_sum, l_tmp, OCB_BLOCK_LEN);

      c->u_mode.ocb.aad_nleftover = 0;
    }

  /* Mark AAD as finalized so that gcry_cipher_ocb_authenticate can
   * return an erro when called again.  */
  c->u_mode.ocb.aad_finalized = 1;

  if (burn > 0)
    _gcry_burn_stack (burn + 4*sizeof(void*));
}



/* Checksumming for encrypt and decrypt.  */
static void
ocb_checksum (unsigned char *chksum, const unsigned char *plainbuf,
              size_t nblks)
{
  while (nblks > 0)
    {
      /* Checksum_i = Checksum_{i-1} xor P_i  */
      cipher_block_xor_1(chksum, plainbuf, OCB_BLOCK_LEN);

      plainbuf += OCB_BLOCK_LEN;
      nblks--;
    }
}


/* Common code for encrypt and decrypt.  */
static gcry_err_code_t
ocb_crypt (gcry_cipher_hd_t c, int encrypt,
           unsigned char *outbuf, size_t outbuflen,
           const unsigned char *inbuf, size_t inbuflen)
{
  const size_t table_maxblks = 1 << OCB_L_TABLE_SIZE;
  const u32 table_size_mask = ((1 << OCB_L_TABLE_SIZE) - 1);
  unsigned char l_tmp[OCB_BLOCK_LEN];
  unsigned int burn = 0;
  unsigned int nburn;
  gcry_cipher_encrypt_t crypt_fn =
      encrypt ? c->spec->encrypt : c->spec->decrypt;

  /* Check that a nonce and thus a key has been set and that we are
     not yet in end of data state. */
  if (!c->marks.iv || c->u_mode.ocb.data_finalized)
    return GPG_ERR_INV_STATE;

  /* Check correct usage and arguments.  */
  if (c->spec->blocksize != OCB_BLOCK_LEN)
    return GPG_ERR_CIPHER_ALGO;
  if (outbuflen < inbuflen)
    return GPG_ERR_BUFFER_TOO_SHORT;
  if (c->marks.finalize)
    ; /* Allow arbitarty length. */
  else if ((inbuflen % OCB_BLOCK_LEN))
    return GPG_ERR_INV_LENGTH;  /* We support only full blocks for now.  */

  /* Full blocks handling. */
  while (inbuflen >= OCB_BLOCK_LEN)
    {
      size_t nblks = inbuflen / OCB_BLOCK_LEN;
      size_t nmaxblks;

      /* Check how many blocks to process till table overflow. */
      nmaxblks = (c->u_mode.ocb.data_nblocks + 1) % table_maxblks;
      nmaxblks = (table_maxblks - nmaxblks) % table_maxblks;

      if (nmaxblks == 0)
        {
          /* Table overflow, generate L and process one block. */
          c->u_mode.ocb.data_nblocks++;
          ocb_get_L_big(c, c->u_mode.ocb.data_nblocks, l_tmp);

          if (encrypt)
            {
              /* Checksum_i = Checksum_{i-1} xor P_i  */
              ocb_checksum (c->u_ctr.ctr, inbuf, 1);
            }

          /* Offset_i = Offset_{i-1} xor L_{ntz(i)} */
          cipher_block_xor_1 (c->u_iv.iv, l_tmp, OCB_BLOCK_LEN);
          /* C_i = Offset_i xor ENCIPHER(K, P_i xor Offset_i)  */
          cipher_block_xor (outbuf, c->u_iv.iv, inbuf, OCB_BLOCK_LEN);
          nburn = crypt_fn (&c->context.c, outbuf, outbuf);
          burn = nburn > burn ? nburn : burn;
          cipher_block_xor_1 (outbuf, c->u_iv.iv, OCB_BLOCK_LEN);

          if (!encrypt)
            {
              /* Checksum_i = Checksum_{i-1} xor P_i  */
              ocb_checksum (c->u_ctr.ctr, outbuf, 1);
            }

          inbuf += OCB_BLOCK_LEN;
          inbuflen -= OCB_BLOCK_LEN;
          outbuf += OCB_BLOCK_LEN;
          outbuflen =- OCB_BLOCK_LEN;
          nblks--;

          /* With overflow handled, retry loop again. Next overflow will
           * happen after 65535 blocks. */
          continue;
        }

      nblks = nblks < nmaxblks ? nblks : nmaxblks;

      /* Since checksum xoring is done before/after encryption/decryption,
	process input in 24KiB chunks to keep data loaded in L1 cache for
	checksumming. */
      if (nblks > 24 * 1024 / OCB_BLOCK_LEN)
	nblks = 24 * 1024 / OCB_BLOCK_LEN;

      /* Use a bulk method if available.  */
      if (nblks && c->bulk.ocb_crypt)
        {
          size_t nleft;
          size_t ndone;

          nleft = c->bulk.ocb_crypt (c, outbuf, inbuf, nblks, encrypt);
          ndone = nblks - nleft;

          inbuf += ndone * OCB_BLOCK_LEN;
          outbuf += ndone * OCB_BLOCK_LEN;
          inbuflen -= ndone * OCB_BLOCK_LEN;
          outbuflen -= ndone * OCB_BLOCK_LEN;
          nblks = nleft;
        }

      if (nblks)
        {
          size_t nblks_chksum = nblks;

          if (encrypt)
            {
              /* Checksum_i = Checksum_{i-1} xor P_i  */
              ocb_checksum (c->u_ctr.ctr, inbuf, nblks_chksum);
            }

          /* Encrypt all full blocks.  */
          while (nblks)
            {
              c->u_mode.ocb.data_nblocks++;

              gcry_assert(c->u_mode.ocb.data_nblocks & table_size_mask);

              /* Offset_i = Offset_{i-1} xor L_{ntz(i)} */
              cipher_block_xor_1 (c->u_iv.iv,
                                  ocb_get_l (c, c->u_mode.ocb.data_nblocks),
                                  OCB_BLOCK_LEN);
              /* C_i = Offset_i xor ENCIPHER(K, P_i xor Offset_i)  */
              cipher_block_xor (outbuf, c->u_iv.iv, inbuf, OCB_BLOCK_LEN);
              nburn = crypt_fn (&c->context.c, outbuf, outbuf);
              burn = nburn > burn ? nburn : burn;
              cipher_block_xor_1 (outbuf, c->u_iv.iv, OCB_BLOCK_LEN);

              inbuf += OCB_BLOCK_LEN;
              inbuflen -= OCB_BLOCK_LEN;
              outbuf += OCB_BLOCK_LEN;
              outbuflen =- OCB_BLOCK_LEN;
              nblks--;
            }

          if (!encrypt)
            {
              /* Checksum_i = Checksum_{i-1} xor P_i  */
              ocb_checksum (c->u_ctr.ctr,
                            outbuf - nblks_chksum * OCB_BLOCK_LEN,
                            nblks_chksum);
            }
        }
    }

  /* Encrypt final partial block.  Note that we expect INBUFLEN to be
     shorter than OCB_BLOCK_LEN (see above).  */
  if (inbuflen)
    {
      unsigned char pad[OCB_BLOCK_LEN];

      /* Offset_* = Offset_m xor L_*  */
      cipher_block_xor_1 (c->u_iv.iv, c->u_mode.ocb.L_star, OCB_BLOCK_LEN);
      /* Pad = ENCIPHER(K, Offset_*) */
      nburn = c->spec->encrypt (&c->context.c, pad, c->u_iv.iv);
      burn = nburn > burn ? nburn : burn;

      if (encrypt)
        {
          /* Checksum_* = Checksum_m xor (P_* || 1 || zeros(127-bitlen(P_*))) */
          /* Note that INBUFLEN is less than OCB_BLOCK_LEN.  */
          buf_cpy (l_tmp, inbuf, inbuflen);
          memset (l_tmp + inbuflen, 0, OCB_BLOCK_LEN - inbuflen);
          l_tmp[inbuflen] = 0x80;
          cipher_block_xor_1 (c->u_ctr.ctr, l_tmp, OCB_BLOCK_LEN);
          /* C_* = P_* xor Pad[1..bitlen(P_*)] */
          buf_xor (outbuf, inbuf, pad, inbuflen);
        }
      else
        {
          /* P_* = C_* xor Pad[1..bitlen(C_*)] */
          /* Checksum_* = Checksum_m xor (P_* || 1 || zeros(127-bitlen(P_*))) */
          cipher_block_cpy (l_tmp, pad, OCB_BLOCK_LEN);
          buf_cpy (l_tmp, inbuf, inbuflen);
          cipher_block_xor_1 (l_tmp, pad, OCB_BLOCK_LEN);
          l_tmp[inbuflen] = 0x80;
          buf_cpy (outbuf, l_tmp, inbuflen);

          cipher_block_xor_1 (c->u_ctr.ctr, l_tmp, OCB_BLOCK_LEN);
        }
    }

  /* Compute the tag if the finalize flag has been set.  */
  if (c->marks.finalize)
    {
      /* Tag = ENCIPHER(K, Checksum xor Offset xor L_$) xor HASH(K,A) */
      cipher_block_xor (c->u_mode.ocb.tag, c->u_ctr.ctr, c->u_iv.iv,
                        OCB_BLOCK_LEN);
      cipher_block_xor_1 (c->u_mode.ocb.tag, c->u_mode.ocb.L_dollar,
                          OCB_BLOCK_LEN);
      nburn = c->spec->encrypt (&c->context.c,
                                c->u_mode.ocb.tag, c->u_mode.ocb.tag);
      burn = nburn > burn ? nburn : burn;

      c->u_mode.ocb.data_finalized = 1;
      /* Note that the the final part of the tag computation is done
         by _gcry_cipher_ocb_get_tag.  */
    }

  if (burn > 0)
    _gcry_burn_stack (burn + 4*sizeof(void*));

  return 0;
}


/* Encrypt (INBUF,INBUFLEN) in OCB mode to OUTBUF.  OUTBUFLEN gives
   the allocated size of OUTBUF.  This function accepts only multiples
   of a full block unless gcry_cipher_final has been called in which
   case the next block may have any length.  */
gcry_err_code_t
_gcry_cipher_ocb_encrypt (gcry_cipher_hd_t c,
                          unsigned char *outbuf, size_t outbuflen,
                          const unsigned char *inbuf, size_t inbuflen)

{
  return ocb_crypt (c, 1, outbuf, outbuflen, inbuf, inbuflen);
}


/* Decrypt (INBUF,INBUFLEN) in OCB mode to OUTBUF.  OUTBUFLEN gives
   the allocated size of OUTBUF.  This function accepts only multiples
   of a full block unless gcry_cipher_final has been called in which
   case the next block may have any length.  */
gcry_err_code_t
_gcry_cipher_ocb_decrypt (gcry_cipher_hd_t c,
                          unsigned char *outbuf, size_t outbuflen,
                          const unsigned char *inbuf, size_t inbuflen)
{
  return ocb_crypt (c, 0, outbuf, outbuflen, inbuf, inbuflen);
}


/* Compute the tag.  The last data operation has already done some
   part of it.  To allow adding AAD even after having done all data,
   we finish the tag computation only here.  */
static void
compute_tag_if_needed (gcry_cipher_hd_t c)
{
  if (!c->marks.tag)
    {
      ocb_aad_finalize (c);
      cipher_block_xor_1 (c->u_mode.ocb.tag, c->u_mode.ocb.aad_sum,
                          OCB_BLOCK_LEN);
      c->marks.tag = 1;
    }
}


/* Copy the already computed tag to OUTTAG.  OUTTAGSIZE is the
   allocated size of OUTTAG; the function returns an error if that is
   too short to hold the tag.  */
gcry_err_code_t
_gcry_cipher_ocb_get_tag (gcry_cipher_hd_t c,
                          unsigned char *outtag, size_t outtagsize)
{
  if (c->u_mode.ocb.taglen > outtagsize)
    return GPG_ERR_BUFFER_TOO_SHORT;
  if (!c->u_mode.ocb.data_finalized)
    return GPG_ERR_INV_STATE; /* Data has not yet been finalized.  */

  compute_tag_if_needed (c);

  memcpy (outtag, c->u_mode.ocb.tag, c->u_mode.ocb.taglen);

  return 0;
}


/* Check that the tag (INTAG,TAGLEN) matches the computed tag for the
   handle C.  */
gcry_err_code_t
_gcry_cipher_ocb_check_tag (gcry_cipher_hd_t c, const unsigned char *intag,
			    size_t taglen)
{
  size_t n;

  if (!c->u_mode.ocb.data_finalized)
    return GPG_ERR_INV_STATE; /* Data has not yet been finalized.  */

  compute_tag_if_needed (c);

  n = c->u_mode.ocb.taglen;
  if (taglen < n)
    n = taglen;

  if (!buf_eq_const (intag, c->u_mode.ocb.tag, n)
      || c->u_mode.ocb.taglen != taglen)
    return GPG_ERR_CHECKSUM;

  return 0;
}
