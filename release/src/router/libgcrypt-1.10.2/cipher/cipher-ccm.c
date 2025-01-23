/* cipher-ccm.c - CTR mode with CBC-MAC mode implementation
 * Copyright (C) 2013 Jussi Kivilinna <jussi.kivilinna@iki.fi>
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
#include "cipher.h"
#include "bufhelp.h"
#include "./cipher-internal.h"


#define set_burn(burn, nburn) do { \
  unsigned int __nburn = (nburn); \
  (burn) = (burn) > __nburn ? (burn) : __nburn; } while (0)


static unsigned int
do_cbc_mac (gcry_cipher_hd_t c, const unsigned char *inbuf, size_t inlen,
            int do_padding)
{
  const unsigned int blocksize = 16;
  gcry_cipher_encrypt_t enc_fn = c->spec->encrypt;
  unsigned char tmp[blocksize];
  unsigned int burn = 0;
  unsigned int unused = c->u_mode.ccm.mac_unused;
  size_t nblocks;
  size_t n;

  if (inlen == 0 && (unused == 0 || !do_padding))
    return 0;

  do
    {
      if (inlen + unused < blocksize || unused > 0)
        {
	  n = (inlen > blocksize - unused) ? blocksize - unused : inlen;

	  buf_cpy (&c->u_mode.ccm.macbuf[unused], inbuf, n);
	  unused += n;
	  inlen -= n;
	  inbuf += n;
        }
      if (!inlen)
        {
          if (!do_padding)
            break;

	  n = blocksize - unused;
	  if (n > 0)
	    {
	      memset (&c->u_mode.ccm.macbuf[unused], 0, n);
	      unused = blocksize;
	    }
        }

      if (unused > 0)
        {
          /* Process one block from macbuf.  */
          cipher_block_xor(c->u_iv.iv, c->u_iv.iv, c->u_mode.ccm.macbuf,
                           blocksize);
          set_burn (burn, enc_fn ( &c->context.c, c->u_iv.iv, c->u_iv.iv ));

          unused = 0;
        }

      if (c->bulk.cbc_enc)
        {
          nblocks = inlen / blocksize;
          c->bulk.cbc_enc (&c->context.c, c->u_iv.iv, tmp, inbuf, nblocks, 1);
          inbuf += nblocks * blocksize;
          inlen -= nblocks * blocksize;

          wipememory (tmp, sizeof(tmp));
        }
      else
        {
          while (inlen >= blocksize)
            {
              cipher_block_xor(c->u_iv.iv, c->u_iv.iv, inbuf, blocksize);

              set_burn (burn, enc_fn ( &c->context.c, c->u_iv.iv, c->u_iv.iv ));

              inlen -= blocksize;
              inbuf += blocksize;
            }
        }
    }
  while (inlen > 0);

  c->u_mode.ccm.mac_unused = unused;

  if (burn)
    burn += 4 * sizeof(void *);

  return burn;
}


gcry_err_code_t
_gcry_cipher_ccm_set_nonce (gcry_cipher_hd_t c, const unsigned char *nonce,
                            size_t noncelen)
{
  unsigned int marks_key;
  size_t L = 15 - noncelen;
  size_t L_;

  L_ = L - 1;

  if (!nonce)
    return GPG_ERR_INV_ARG;
  /* Length field must be 2, 3, ..., or 8. */
  if (L < 2 || L > 8)
    return GPG_ERR_INV_LENGTH;

  /* Reset state */
  marks_key = c->marks.key;
  memset (&c->u_mode, 0, sizeof(c->u_mode));
  memset (&c->marks, 0, sizeof(c->marks));
  memset (&c->u_iv, 0, sizeof(c->u_iv));
  memset (&c->u_ctr, 0, sizeof(c->u_ctr));
  memset (c->lastiv, 0, sizeof(c->lastiv));
  c->unused = 0;
  c->marks.key = marks_key;

  /* Setup CTR */
  c->u_ctr.ctr[0] = L_;
  memcpy (&c->u_ctr.ctr[1], nonce, noncelen);
  memset (&c->u_ctr.ctr[1 + noncelen], 0, L);

  /* Setup IV */
  c->u_iv.iv[0] = L_;
  memcpy (&c->u_iv.iv[1], nonce, noncelen);
  /* Add (8 * M_ + 64 * flags) to iv[0] and set iv[noncelen + 1 ... 15] later
     in set_aad.  */
  memset (&c->u_iv.iv[1 + noncelen], 0, L);

  c->u_mode.ccm.nonce = 1;

  return GPG_ERR_NO_ERROR;
}


gcry_err_code_t
_gcry_cipher_ccm_set_lengths (gcry_cipher_hd_t c, u64 encryptlen, u64 aadlen,
                              u64 taglen)
{
  unsigned int burn = 0;
  unsigned char b0[16];
  size_t noncelen = 15 - (c->u_iv.iv[0] + 1);
  u64 M = taglen;
  u64 M_;
  int i;

  M_ = (M - 2) / 2;

  /* Authentication field must be 4, 6, 8, 10, 12, 14 or 16. */
  if ((M_ * 2 + 2) != M || M < 4 || M > 16)
    return GPG_ERR_INV_LENGTH;
  if (!c->u_mode.ccm.nonce || c->marks.tag)
    return GPG_ERR_INV_STATE;
  if (c->u_mode.ccm.lengths)
    return GPG_ERR_INV_STATE;

  c->u_mode.ccm.authlen = taglen;
  c->u_mode.ccm.encryptlen = encryptlen;
  c->u_mode.ccm.aadlen = aadlen;

  /* Complete IV setup.  */
  c->u_iv.iv[0] += (aadlen > 0) * 64 + M_ * 8;
  for (i = 16 - 1; i >= 1 + noncelen; i--)
    {
      c->u_iv.iv[i] = encryptlen & 0xff;
      encryptlen >>= 8;
    }

  memcpy (b0, c->u_iv.iv, 16);
  memset (c->u_iv.iv, 0, 16);

  set_burn (burn, do_cbc_mac (c, b0, 16, 0));

  if (aadlen == 0)
    {
      /* Do nothing.  */
    }
  else if (aadlen > 0 && aadlen <= (unsigned int)0xfeff)
    {
      b0[0] = (aadlen >> 8) & 0xff;
      b0[1] = aadlen & 0xff;
      set_burn (burn, do_cbc_mac (c, b0, 2, 0));
    }
  else if (aadlen > 0xfeff && aadlen <= (unsigned int)0xffffffff)
    {
      b0[0] = 0xff;
      b0[1] = 0xfe;
      buf_put_be32(&b0[2], aadlen);
      set_burn (burn, do_cbc_mac (c, b0, 6, 0));
    }
  else if (aadlen > (unsigned int)0xffffffff)
    {
      b0[0] = 0xff;
      b0[1] = 0xff;
      buf_put_be64(&b0[2], aadlen);
      set_burn (burn, do_cbc_mac (c, b0, 10, 0));
    }

  /* Generate S_0 and increase counter.  */
  set_burn (burn, c->spec->encrypt ( &c->context.c, c->u_mode.ccm.s0,
                                     c->u_ctr.ctr ));
  c->u_ctr.ctr[15]++;

  if (burn)
    _gcry_burn_stack (burn + sizeof(void *) * 5);

  c->u_mode.ccm.lengths = 1;

  return GPG_ERR_NO_ERROR;
}


gcry_err_code_t
_gcry_cipher_ccm_authenticate (gcry_cipher_hd_t c, const unsigned char *abuf,
                               size_t abuflen)
{
  unsigned int burn;

  if (abuflen > 0 && !abuf)
    return GPG_ERR_INV_ARG;
  if (!c->u_mode.ccm.nonce || !c->u_mode.ccm.lengths || c->marks.tag)
    return GPG_ERR_INV_STATE;
  if (abuflen > c->u_mode.ccm.aadlen)
    return GPG_ERR_INV_LENGTH;

  c->u_mode.ccm.aadlen -= abuflen;
  burn = do_cbc_mac (c, abuf, abuflen, c->u_mode.ccm.aadlen == 0);

  if (burn)
    _gcry_burn_stack (burn + sizeof(void *) * 5);

  return GPG_ERR_NO_ERROR;
}


gcry_err_code_t
_gcry_cipher_ccm_tag (gcry_cipher_hd_t c, unsigned char *outbuf,
		      size_t outbuflen, int check)
{
  unsigned int burn;

  if (!outbuf || outbuflen == 0)
    return GPG_ERR_INV_ARG;
  /* Tag length must be same as initial authlen.  */
  if (c->u_mode.ccm.authlen != outbuflen)
    return GPG_ERR_INV_LENGTH;
  if (!c->u_mode.ccm.nonce || !c->u_mode.ccm.lengths || c->u_mode.ccm.aadlen > 0)
    return GPG_ERR_INV_STATE;
  /* Initial encrypt length must match with length of actual data processed.  */
  if (c->u_mode.ccm.encryptlen > 0)
    return GPG_ERR_UNFINISHED;

  if (!c->marks.tag)
    {
      burn = do_cbc_mac (c, NULL, 0, 1); /* Perform final padding.  */

      /* Add S_0 */
      cipher_block_xor (c->u_iv.iv, c->u_iv.iv, c->u_mode.ccm.s0, 16);

      wipememory (c->u_ctr.ctr, 16);
      wipememory (c->u_mode.ccm.s0, 16);
      wipememory (c->u_mode.ccm.macbuf, 16);

      if (burn)
        _gcry_burn_stack (burn + sizeof(void *) * 5);

      c->marks.tag = 1;
    }

  if (!check)
    {
      memcpy (outbuf, c->u_iv.iv, outbuflen);
      return GPG_ERR_NO_ERROR;
    }
  else
    {
      return buf_eq_const(outbuf, c->u_iv.iv, outbuflen) ?
             GPG_ERR_NO_ERROR : GPG_ERR_CHECKSUM;
    }
}


gcry_err_code_t
_gcry_cipher_ccm_get_tag (gcry_cipher_hd_t c, unsigned char *outtag,
			  size_t taglen)
{
  return _gcry_cipher_ccm_tag (c, outtag, taglen, 0);
}


gcry_err_code_t
_gcry_cipher_ccm_check_tag (gcry_cipher_hd_t c, const unsigned char *intag,
			    size_t taglen)
{
  return _gcry_cipher_ccm_tag (c, (unsigned char *)intag, taglen, 1);
}


gcry_err_code_t
_gcry_cipher_ccm_encrypt (gcry_cipher_hd_t c, unsigned char *outbuf,
                          size_t outbuflen, const unsigned char *inbuf,
                          size_t inbuflen)
{
  gcry_err_code_t err = 0;
  unsigned int burn = 0;
  unsigned int nburn;

  if (outbuflen < inbuflen)
    return GPG_ERR_BUFFER_TOO_SHORT;
  if (!c->u_mode.ccm.nonce || c->marks.tag || !c->u_mode.ccm.lengths ||
      c->u_mode.ccm.aadlen > 0)
    return GPG_ERR_INV_STATE;
  if (inbuflen > c->u_mode.ccm.encryptlen)
    return GPG_ERR_INV_LENGTH;

  while (inbuflen)
    {
      size_t currlen = inbuflen;

      /* Since checksumming is done before encryption, process input in 24KiB
       * chunks to keep data loaded in L1 cache for encryption. */
      if (currlen > 24 * 1024)
	currlen = 24 * 1024;

      c->u_mode.ccm.encryptlen -= currlen;
      nburn = do_cbc_mac (c, inbuf, currlen, 0);
      burn = nburn > burn ? nburn : burn;

      err = _gcry_cipher_ctr_encrypt (c, outbuf, outbuflen, inbuf, currlen);
      if (err)
	break;

      outbuf += currlen;
      inbuf += currlen;
      outbuflen -= currlen;
      inbuflen -= currlen;
    }

  if (burn)
    _gcry_burn_stack (burn + sizeof(void *) * 5);
  return err;
}


gcry_err_code_t
_gcry_cipher_ccm_decrypt (gcry_cipher_hd_t c, unsigned char *outbuf,
                          size_t outbuflen, const unsigned char *inbuf,
                          size_t inbuflen)
{
  gcry_err_code_t err = 0;
  unsigned int burn = 0;
  unsigned int nburn;

  if (outbuflen < inbuflen)
    return GPG_ERR_BUFFER_TOO_SHORT;
  if (!c->u_mode.ccm.nonce || c->marks.tag || !c->u_mode.ccm.lengths ||
      c->u_mode.ccm.aadlen > 0)
    return GPG_ERR_INV_STATE;
  if (inbuflen > c->u_mode.ccm.encryptlen)
    return GPG_ERR_INV_LENGTH;

  while (inbuflen)
    {
      size_t currlen = inbuflen;

      /* Since checksumming is done after decryption, process input in 24KiB
       * chunks to keep data loaded in L1 cache for checksumming. */
      if (currlen > 24 * 1024)
	currlen = 24 * 1024;

      err = _gcry_cipher_ctr_encrypt (c, outbuf, outbuflen, inbuf, currlen);
      if (err)
	break;

      c->u_mode.ccm.encryptlen -= currlen;
      nburn = do_cbc_mac (c, outbuf, currlen, 0);
      burn = nburn > burn ? nburn : burn;

      outbuf += currlen;
      inbuf += currlen;
      outbuflen -= currlen;
      inbuflen -= currlen;
    }

  if (burn)
    _gcry_burn_stack (burn + sizeof(void *) * 5);
  return err;
}
