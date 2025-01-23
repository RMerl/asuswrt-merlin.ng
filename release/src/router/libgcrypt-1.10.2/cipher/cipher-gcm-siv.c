/* cipher-gcm-siv.c  - GCM-SIV implementation (RFC 8452)
 * Copyright (C) 2021 Jussi Kivilinna <jussi.kivilinna@iki.fi>
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


#define GCM_SIV_NONCE_LENGTH (96 / 8)


static inline void
mulx_ghash (byte *a)
{
  u64 t[2], mask;

  t[0] = buf_get_be64(a + 0);
  t[1] = buf_get_be64(a + 8);
  mask = -(t[1] & 1) & 0xe1;
  mask <<= 56;

  buf_put_be64(a + 8, (t[1] >> 1) ^ (t[0] << 63));
  buf_put_be64(a + 0, (t[0] >> 1) ^ mask);
}


static inline void
gcm_siv_bytecounter_add (u32 ctr[2], size_t add)
{
  if (sizeof(add) > sizeof(u32))
    {
      u32 high_add = ((add >> 31) >> 1) & 0xffffffff;
      ctr[1] += high_add;
    }

  ctr[0] += add;
  if (ctr[0] >= add)
    return;
  ++ctr[1];
}


static inline int
gcm_siv_check_len (u32 ctr[2])
{
  /* len(plaintext/aadlen) <= 2^39-256 bits == 2^36-32 bytes == 2^32-2 blocks */
  if (ctr[1] > 0xfU)
    return 0;
  if (ctr[1] < 0xfU)
    return 1;

  if (ctr[0] <= 0xffffffe0U)
    return 1;

  return 0;
}


static void
polyval_set_key (gcry_cipher_hd_t c, const byte *auth_key)
{
  cipher_block_bswap (c->u_mode.gcm.u_ghash_key.key, auth_key,
		      GCRY_SIV_BLOCK_LEN);
  mulx_ghash (c->u_mode.gcm.u_ghash_key.key);
  _gcry_cipher_gcm_setupM (c);
}


static void
do_polyval_buf(gcry_cipher_hd_t c, byte *hash, const byte *buf,
	       size_t buflen, int do_padding)
{
  unsigned int blocksize = GCRY_SIV_BLOCK_LEN;
  unsigned int unused = c->u_mode.gcm.mac_unused;
  ghash_fn_t ghash_fn = c->u_mode.gcm.ghash_fn;
  ghash_fn_t polyval_fn = c->u_mode.gcm.polyval_fn;
  byte tmp_blocks[16][GCRY_SIV_BLOCK_LEN];
  size_t nblocks, n;
  unsigned int burn = 0, nburn;
  unsigned int num_blks_used = 0;

  if (buflen == 0 && (unused == 0 || !do_padding))
    return;

  do
    {
      if (buflen > 0 && (buflen + unused < blocksize || unused > 0))
        {
          n = blocksize - unused;
          n = n < buflen ? n : buflen;

          buf_cpy (&c->u_mode.gcm.macbuf[unused], buf, n);

          unused += n;
          buf += n;
          buflen -= n;
        }
      if (!buflen)
        {
          if (!do_padding && unused < blocksize)
	    {
	      break;
	    }

	  n = blocksize - unused;
	  if (n > 0)
	    {
	      memset (&c->u_mode.gcm.macbuf[unused], 0, n);
	      unused = blocksize;
	    }
        }

      if (unused > 0)
        {
          gcry_assert (unused == blocksize);

          /* Process one block from macbuf.  */
          if (polyval_fn)
            {
              nburn = polyval_fn (c, hash, c->u_mode.gcm.macbuf, 1);
            }
          else
            {
              cipher_block_bswap (c->u_mode.gcm.macbuf, c->u_mode.gcm.macbuf,
                                  blocksize);
              nburn = ghash_fn (c, hash, c->u_mode.gcm.macbuf, 1);
            }

          burn = nburn > burn ? nburn : burn;
          unused = 0;
        }

      nblocks = buflen / blocksize;

      while (nblocks)
        {
          if (polyval_fn)
            {
              n = nblocks;
              nburn = polyval_fn (c, hash, buf, n);
            }
          else
            {
              for (n = 0; n < (nblocks > 16 ? 16 : nblocks); n++)
                cipher_block_bswap (tmp_blocks[n], buf + n * blocksize,
                                    blocksize);

              num_blks_used = n > num_blks_used ? n : num_blks_used;

              nburn = ghash_fn (c, hash, tmp_blocks[0], n);
            }

          burn = nburn > burn ? nburn : burn;
          buf += n * blocksize;
          buflen -= n * blocksize;
          nblocks -= n;
        }
    }
  while (buflen > 0);

  c->u_mode.gcm.mac_unused = unused;

  if (num_blks_used)
    wipememory (tmp_blocks, num_blks_used * blocksize);
  if (burn)
    _gcry_burn_stack (burn);
}


static void
do_ctr_le32 (gcry_cipher_hd_t c, byte *outbuf, const byte *inbuf,
	     size_t inbuflen)
{
  gcry_cipher_encrypt_t enc_fn = c->spec->encrypt;
  unsigned char tmp[GCRY_SIV_BLOCK_LEN];
  unsigned int burn = 0, nburn;
  size_t nblocks;

  if (inbuflen == 0)
    return;

  /* Use a bulk method if available.  */
  nblocks = inbuflen / GCRY_SIV_BLOCK_LEN;
  if (nblocks && c->bulk.ctr32le_enc)
    {
      c->bulk.ctr32le_enc (c->context.c, c->u_ctr.ctr, outbuf, inbuf, nblocks);
      inbuf  += nblocks * GCRY_SIV_BLOCK_LEN;
      outbuf += nblocks * GCRY_SIV_BLOCK_LEN;
      inbuflen -= nblocks * GCRY_SIV_BLOCK_LEN;
    }

  do
    {
      nburn = enc_fn (c->context.c, tmp, c->u_ctr.ctr);
      burn = nburn > burn ? nburn : burn;

      buf_put_le32(c->u_ctr.ctr, buf_get_le32(c->u_ctr.ctr) + 1);

      if (inbuflen < GCRY_SIV_BLOCK_LEN)
	break;
      cipher_block_xor(outbuf, inbuf, tmp, GCRY_SIV_BLOCK_LEN);

      inbuflen -= GCRY_SIV_BLOCK_LEN;
      outbuf += GCRY_SIV_BLOCK_LEN;
      inbuf += GCRY_SIV_BLOCK_LEN;
    }
  while (inbuflen);

  if (inbuflen)
    {
      buf_xor(outbuf, inbuf, tmp, inbuflen);

      outbuf += inbuflen;
      inbuf += inbuflen;
      inbuflen -= inbuflen;
    }

  wipememory (tmp, sizeof(tmp));

  if (burn > 0)
    _gcry_burn_stack (burn + 4 * sizeof(void *));
}


static int
gcm_siv_selftest (gcry_cipher_hd_t c)
{
  static const byte in1[GCRY_SIV_BLOCK_LEN] =
      "\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00";
  static const byte out1[GCRY_SIV_BLOCK_LEN] =
      "\x00\x80\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00";
  static const byte in2[GCRY_SIV_BLOCK_LEN] =
      "\x9c\x98\xc0\x4d\xf9\x38\x7d\xed\x82\x81\x75\xa9\x2b\xa6\x52\xd8";
  static const byte out2[GCRY_SIV_BLOCK_LEN] =
      "\x4e\x4c\x60\x26\xfc\x9c\x3e\xf6\xc1\x40\xba\xd4\x95\xd3\x29\x6c";
  static const byte polyval_key[GCRY_SIV_BLOCK_LEN] =
      "\x25\x62\x93\x47\x58\x92\x42\x76\x1d\x31\xf8\x26\xba\x4b\x75\x7b";
  static const byte ghash_key[GCRY_SIV_BLOCK_LEN] =
      "\xdc\xba\xa5\xdd\x13\x7c\x18\x8e\xbb\x21\x49\x2c\x23\xc9\xb1\x12";
  static const byte polyval_data[GCRY_SIV_BLOCK_LEN * 2] =
      "\x4f\x4f\x95\x66\x8c\x83\xdf\xb6\x40\x17\x62\xbb\x2d\x01\xa2\x62"
      "\xd1\xa2\x4d\xdd\x27\x21\xd0\x06\xbb\xe4\x5f\x20\xd3\xc9\xf3\x62";
  static const byte polyval_tag[GCRY_SIV_BLOCK_LEN] =
      "\xf7\xa3\xb4\x7b\x84\x61\x19\xfa\xe5\xb7\x86\x6c\xf5\xe5\xb7\x7e";
  byte tmp[GCRY_SIV_BLOCK_LEN];

  /* Test mulx_ghash */
  memcpy (tmp, in1, GCRY_SIV_BLOCK_LEN);
  mulx_ghash (tmp);
  if (memcmp (tmp, out1, GCRY_SIV_BLOCK_LEN) != 0)
    return -1;

  memcpy (tmp, in2, GCRY_SIV_BLOCK_LEN);
  mulx_ghash (tmp);
  if (memcmp (tmp, out2, GCRY_SIV_BLOCK_LEN) != 0)
    return -1;

  /* Test GHASH key generation */
  memcpy (tmp, polyval_key, GCRY_SIV_BLOCK_LEN);
  cipher_block_bswap (tmp, tmp, GCRY_SIV_BLOCK_LEN);
  mulx_ghash (tmp);
  if (memcmp (tmp, ghash_key, GCRY_SIV_BLOCK_LEN) != 0)
    return -1;

  /* Test POLYVAL */
  memset (&c->u_mode.gcm, 0, sizeof(c->u_mode.gcm));
  polyval_set_key (c, polyval_key);
  memset (&tmp, 0, sizeof(tmp));
  do_polyval_buf (c, tmp, polyval_data, GCRY_SIV_BLOCK_LEN * 2, 1);
  cipher_block_bswap (tmp, tmp, GCRY_SIV_BLOCK_LEN);
  if (memcmp (tmp, polyval_tag, GCRY_SIV_BLOCK_LEN) != 0)
    return -1;

  return 0;
}


gcry_err_code_t
_gcry_cipher_gcm_siv_setkey (gcry_cipher_hd_t c, unsigned int keylen)
{
  static int done;

  if (keylen != 16 && keylen != 32)
    return GPG_ERR_INV_KEYLEN;

  if (!done)
    {
      if (gcm_siv_selftest (c))
	return GPG_ERR_SELFTEST_FAILED;

      done = 1;
    }

  c->marks.iv = 0;
  c->marks.tag = 0;
  memset (&c->u_mode.gcm, 0, sizeof(c->u_mode.gcm));
  c->u_mode.gcm.siv_keylen = keylen;
  return 0;
}


gcry_err_code_t
_gcry_cipher_gcm_siv_set_nonce (gcry_cipher_hd_t c, const byte *iv,
				size_t ivlen)
{
  byte auth_key[GCRY_SIV_BLOCK_LEN];
  byte tmp_in[GCRY_SIV_BLOCK_LEN];
  byte tmp[GCRY_SIV_BLOCK_LEN];
  byte enc_key[32];
  gcry_err_code_t err;

  if (c->spec->blocksize != GCRY_SIV_BLOCK_LEN)
    return GPG_ERR_CIPHER_ALGO;
  if (ivlen != GCM_SIV_NONCE_LENGTH)
    return GPG_ERR_INV_ARG;
  if (c->u_mode.gcm.siv_keylen == 0)
    return GPG_ERR_INV_STATE;
  if (c->marks.iv)
    {
      /* If nonce is already set, use cipher_reset or setkey first to reset
       * cipher state. */
      return GPG_ERR_INV_STATE;
    }

  memset (c->u_mode.gcm.aadlen, 0, sizeof(c->u_mode.gcm.aadlen));
  memset (c->u_mode.gcm.datalen, 0, sizeof(c->u_mode.gcm.datalen));
  memset (c->u_mode.gcm.u_tag.tag, 0, sizeof(c->u_mode.gcm.u_tag.tag));
  c->u_mode.gcm.datalen_over_limits = 0;
  c->u_mode.gcm.ghash_data_finalized = 0;
  c->u_mode.gcm.ghash_aad_finalized = 0;

  memset (c->u_iv.iv, 0, GCRY_SIV_BLOCK_LEN);
  memcpy (c->u_iv.iv, iv, ivlen);
  memcpy (tmp_in + 4, iv, ivlen);

  /* Derive message authentication key */
  buf_put_le32(tmp_in, 0);
  c->spec->encrypt (&c->context.c, tmp, tmp_in);
  memcpy (auth_key + 0, tmp, 8);

  buf_put_le32(tmp_in, 1);
  c->spec->encrypt (&c->context.c, tmp, tmp_in);
  memcpy (auth_key + 8, tmp, 8);

  polyval_set_key (c, auth_key);
  wipememory (auth_key, sizeof(auth_key));

  /* Derive message encryption key */
  buf_put_le32(tmp_in, 2);
  c->spec->encrypt (&c->context.c, tmp, tmp_in);
  memcpy (enc_key + 0, tmp, 8);

  buf_put_le32(tmp_in, 3);
  c->spec->encrypt (&c->context.c, tmp, tmp_in);
  memcpy (enc_key + 8, tmp, 8);

  if (c->u_mode.gcm.siv_keylen >= 24)
    {
      buf_put_le32(tmp_in, 4);
      c->spec->encrypt (&c->context.c, tmp, tmp_in);
      memcpy (enc_key + 16, tmp, 8);
    }

  if (c->u_mode.gcm.siv_keylen >= 32)
    {
      buf_put_le32(tmp_in, 5);
      c->spec->encrypt (&c->context.c, tmp, tmp_in);
      memcpy (enc_key + 24, tmp, 8);
    }

  wipememory (tmp, sizeof(tmp));
  wipememory (tmp_in, sizeof(tmp_in));

  err = c->spec->setkey (&c->context.c, enc_key, c->u_mode.gcm.siv_keylen,
			 &c->bulk);
  wipememory (enc_key, sizeof(enc_key));
  if (err)
    return err;

  c->marks.iv = 1;
  return 0;
}


gcry_err_code_t
_gcry_cipher_gcm_siv_authenticate (gcry_cipher_hd_t c,
				   const byte *aadbuf, size_t aadbuflen)
{
  if (c->spec->blocksize != GCRY_SIV_BLOCK_LEN)
    return GPG_ERR_CIPHER_ALGO;
  if (c->u_mode.gcm.datalen_over_limits)
    return GPG_ERR_INV_LENGTH;
  if (c->marks.tag
      || !c->marks.iv
      || c->u_mode.gcm.ghash_aad_finalized
      || c->u_mode.gcm.ghash_data_finalized
      || !c->u_mode.gcm.ghash_fn)
    return GPG_ERR_INV_STATE;

  gcm_siv_bytecounter_add (c->u_mode.gcm.aadlen, aadbuflen);
  if (!gcm_siv_check_len (c->u_mode.gcm.aadlen))
    {
      c->u_mode.gcm.datalen_over_limits = 1;
      return GPG_ERR_INV_LENGTH;
    }

  do_polyval_buf (c, c->u_mode.gcm.u_tag.tag, aadbuf, aadbuflen, 0);

  return 0;
}


gcry_err_code_t
_gcry_cipher_gcm_siv_encrypt (gcry_cipher_hd_t c,
			      byte *outbuf, size_t outbuflen,
			      const byte *inbuf, size_t inbuflen)
{
  u32 bitlengths[2][2];

  if (c->spec->blocksize != GCRY_SIV_BLOCK_LEN)
    return GPG_ERR_CIPHER_ALGO;
  if (outbuflen < inbuflen)
    return GPG_ERR_BUFFER_TOO_SHORT;
  if (c->u_mode.gcm.datalen_over_limits)
    return GPG_ERR_INV_LENGTH;
  if (c->marks.tag
      || !c->marks.iv
      || c->u_mode.gcm.ghash_data_finalized
      || !c->u_mode.gcm.ghash_fn)
    return GPG_ERR_INV_STATE;

  if (!c->u_mode.gcm.ghash_aad_finalized)
    {
      /* Start of encryption marks end of AAD stream. */
      do_polyval_buf(c, c->u_mode.gcm.u_tag.tag, NULL, 0, 1);
      c->u_mode.gcm.ghash_aad_finalized = 1;
    }

  gcm_siv_bytecounter_add (c->u_mode.gcm.datalen, inbuflen);
  if (!gcm_siv_check_len (c->u_mode.gcm.datalen))
    {
      c->u_mode.gcm.datalen_over_limits = 1;
      return GPG_ERR_INV_LENGTH;
    }

  /* Plaintext and padding to POLYVAL. */
  do_polyval_buf (c, c->u_mode.gcm.u_tag.tag, inbuf, inbuflen, 1);
  c->u_mode.gcm.ghash_data_finalized = 1;

  /* aad length */
  bitlengths[0][0] = le_bswap32(c->u_mode.gcm.aadlen[0] << 3);
  bitlengths[0][1] = le_bswap32((c->u_mode.gcm.aadlen[0] >> 29) |
                                (c->u_mode.gcm.aadlen[1] << 3));
  /* data length */
  bitlengths[1][0] = le_bswap32(c->u_mode.gcm.datalen[0] << 3);
  bitlengths[1][1] = le_bswap32((c->u_mode.gcm.datalen[0] >> 29) |
                                (c->u_mode.gcm.datalen[1] << 3));

  /* Length block to POLYVAL. */
  do_polyval_buf(c, c->u_mode.gcm.u_tag.tag, (byte *)bitlengths,
		 GCRY_SIV_BLOCK_LEN, 1);
  wipememory (bitlengths, sizeof(bitlengths));

  /* Prepare tag and counter. */
  cipher_block_bswap (c->u_mode.gcm.u_tag.tag, c->u_mode.gcm.u_tag.tag,
		      GCRY_SIV_BLOCK_LEN);
  cipher_block_xor (c->u_mode.gcm.tagiv, c->u_iv.iv, c->u_mode.gcm.u_tag.tag,
		    GCRY_SIV_BLOCK_LEN);
  c->u_mode.gcm.tagiv[GCRY_SIV_BLOCK_LEN - 1] &= 0x7f;
  c->spec->encrypt (&c->context.c, c->u_mode.gcm.tagiv, c->u_mode.gcm.tagiv);
  c->marks.tag = 1;
  memcpy (c->u_ctr.ctr, c->u_mode.gcm.tagiv, GCRY_SIV_BLOCK_LEN);
  c->u_ctr.ctr[GCRY_SIV_BLOCK_LEN - 1] |= 0x80;

  /* Encrypt data */
  do_ctr_le32 (c, outbuf, inbuf, inbuflen);
  return 0;
}


gcry_err_code_t
_gcry_cipher_gcm_siv_set_decryption_tag (gcry_cipher_hd_t c,
					 const byte *tag, size_t taglen)
{
  if (taglen != GCRY_SIV_BLOCK_LEN)
    return GPG_ERR_INV_ARG;
  if (c->spec->blocksize != GCRY_SIV_BLOCK_LEN)
    return GPG_ERR_CIPHER_ALGO;
  if (c->marks.tag)
    return GPG_ERR_INV_STATE;

  memcpy (c->u_mode.gcm.tagiv, tag, GCRY_SIV_BLOCK_LEN);
  c->marks.tag = 1;

  return 0;
}


gcry_err_code_t
_gcry_cipher_gcm_siv_decrypt (gcry_cipher_hd_t c,
			      byte *outbuf, size_t outbuflen,
			      const byte *inbuf, size_t inbuflen)
{
  byte expected_tag[GCRY_SIV_BLOCK_LEN];
  u32 bitlengths[2][2];
  gcry_err_code_t rc = 0;

  if (c->spec->blocksize != GCRY_SIV_BLOCK_LEN)
    return GPG_ERR_CIPHER_ALGO;
  if (outbuflen < inbuflen)
    return GPG_ERR_BUFFER_TOO_SHORT;
  if (c->u_mode.gcm.datalen_over_limits)
    return GPG_ERR_INV_LENGTH;
  if (!c->marks.tag
      || !c->marks.iv
      || c->u_mode.gcm.ghash_data_finalized
      || !c->u_mode.gcm.ghash_fn)
    return GPG_ERR_INV_STATE;

  if (!c->u_mode.gcm.ghash_aad_finalized)
    {
      /* Start of encryption marks end of AAD stream. */
      do_polyval_buf(c, c->u_mode.gcm.u_tag.tag, NULL, 0, 1);
      c->u_mode.gcm.ghash_aad_finalized = 1;
    }

  gcm_siv_bytecounter_add (c->u_mode.gcm.datalen, inbuflen);
  if (!gcm_siv_check_len (c->u_mode.gcm.datalen))
    {
      c->u_mode.gcm.datalen_over_limits = 1;
      return GPG_ERR_INV_LENGTH;
    }

  /* Prepare counter. */
  memcpy (c->u_ctr.ctr, c->u_mode.gcm.tagiv, GCRY_SIV_BLOCK_LEN);
  c->u_ctr.ctr[GCRY_SIV_BLOCK_LEN - 1] |= 0x80;

  /* Decrypt data. */
  do_ctr_le32 (c, outbuf, inbuf, inbuflen);

  /* Plaintext and padding to POLYVAL. */
  do_polyval_buf (c, c->u_mode.gcm.u_tag.tag, outbuf, inbuflen, 1);
  c->u_mode.gcm.ghash_data_finalized = 1;

  /* aad length */
  bitlengths[0][0] = le_bswap32(c->u_mode.gcm.aadlen[0] << 3);
  bitlengths[0][1] = le_bswap32((c->u_mode.gcm.aadlen[0] >> 29) |
                                (c->u_mode.gcm.aadlen[1] << 3));
  /* data length */
  bitlengths[1][0] = le_bswap32(c->u_mode.gcm.datalen[0] << 3);
  bitlengths[1][1] = le_bswap32((c->u_mode.gcm.datalen[0] >> 29) |
                                (c->u_mode.gcm.datalen[1] << 3));

  /* Length block to POLYVAL. */
  do_polyval_buf(c, c->u_mode.gcm.u_tag.tag, (byte *)bitlengths,
		 GCRY_SIV_BLOCK_LEN, 1);
  wipememory (bitlengths, sizeof(bitlengths));

  /* Prepare tag. */
  cipher_block_bswap (c->u_mode.gcm.u_tag.tag, c->u_mode.gcm.u_tag.tag,
		      GCRY_SIV_BLOCK_LEN);
  cipher_block_xor (expected_tag, c->u_iv.iv, c->u_mode.gcm.u_tag.tag,
		    GCRY_SIV_BLOCK_LEN);
  expected_tag[GCRY_SIV_BLOCK_LEN - 1] &= 0x7f;
  c->spec->encrypt (&c->context.c, expected_tag, expected_tag);

  if (!buf_eq_const(c->u_mode.gcm.tagiv, expected_tag, GCRY_SIV_BLOCK_LEN))
    {
      wipememory (outbuf, inbuflen);
      rc = GPG_ERR_CHECKSUM;
    }

  wipememory (expected_tag, sizeof(expected_tag));
  return rc;
}


static gcry_err_code_t
_gcry_cipher_gcm_siv_tag (gcry_cipher_hd_t c,
			  byte * outbuf, size_t outbuflen, int check)
{
  gcry_err_code_t err;

  if (!c->marks.tag)
    {
      if (!c->u_mode.gcm.ghash_fn)
        return GPG_ERR_INV_STATE;

      if (!c->marks.tag)
        {
          /* Finalize GCM-SIV with zero-length plaintext. */
          err = _gcry_cipher_gcm_siv_encrypt (c, NULL, 0, NULL, 0);
          if (err != 0)
            return err;
        }
    }

  if (c->u_mode.gcm.datalen_over_limits)
    return GPG_ERR_INV_LENGTH;
  if (!c->u_mode.gcm.ghash_data_finalized)
    return GPG_ERR_INV_STATE;
  if (!c->marks.tag)
    return GPG_ERR_INV_STATE;

  if (!check)
    {
      if (outbuflen > GCRY_SIV_BLOCK_LEN)
        outbuflen = GCRY_SIV_BLOCK_LEN;

      /* NB: We already checked that OUTBUF is large enough to hold
       * the result or has valid truncated length.  */
      memcpy (outbuf, c->u_mode.gcm.tagiv, outbuflen);
    }
  else
    {
      /* OUTBUFLEN gives the length of the user supplied tag in OUTBUF
       * and thus we need to compare its length first.  */
      if (outbuflen != GCRY_SIV_BLOCK_LEN
          || !buf_eq_const (outbuf, c->u_mode.gcm.tagiv, outbuflen))
        return GPG_ERR_CHECKSUM;
    }

  return 0;
}


gcry_err_code_t
_gcry_cipher_gcm_siv_get_tag (gcry_cipher_hd_t c, unsigned char *outtag,
			      size_t taglen)
{
  return _gcry_cipher_gcm_siv_tag (c, outtag, taglen, 0);
}


gcry_err_code_t
_gcry_cipher_gcm_siv_check_tag (gcry_cipher_hd_t c,
				   const unsigned char *intag,
				   size_t taglen)
{
  return _gcry_cipher_gcm_siv_tag (c, (unsigned char *)intag, taglen, 1);
}
