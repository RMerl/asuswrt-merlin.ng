/* cipher-siv.c  -  SIV implementation (RFC 5297)
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


static inline void
s2v_double (unsigned char *d)
{
  u64 hi, lo, mask;

  hi = buf_get_be64(d + 0);
  lo = buf_get_be64(d + 8);

  mask = -(hi >> 63);
  hi = (hi << 1) ^ (lo >> 63);
  lo = (lo << 1) ^ (mask & 0x87);

  buf_put_be64(d + 0, hi);
  buf_put_be64(d + 8, lo);
}


static void
s2v_pad (unsigned char *out, const byte *in, size_t inlen)
{
  static const unsigned char padding[GCRY_SIV_BLOCK_LEN] = { 0x80 };

  gcry_assert(inlen < GCRY_SIV_BLOCK_LEN);

  buf_cpy (out, in, inlen);
  buf_cpy (out + inlen, padding, GCRY_SIV_BLOCK_LEN - inlen);
}


gcry_err_code_t
_gcry_cipher_siv_setkey (gcry_cipher_hd_t c,
	                 const unsigned char *ctrkey, size_t ctrkeylen)
{
  static const unsigned char zero[GCRY_SIV_BLOCK_LEN] = { 0 };
  gcry_err_code_t err;

  if (c->spec->blocksize != GCRY_SIV_BLOCK_LEN)
    return GPG_ERR_CIPHER_ALGO;

  c->u_mode.siv.aad_count = 0;
  c->u_mode.siv.dec_tag_set = 0;
  c->marks.tag = 0;
  c->marks.iv = 0;

  /* Set CTR mode key. */
  err = c->spec->setkey (c->u_mode.siv.ctr_context, ctrkey, ctrkeylen,
			 &c->bulk);
  if (err != 0)
    return err;

  /* Initialize S2V. */
  memset (&c->u_mode.siv.s2v_cmac, 0, sizeof(c->u_mode.siv.s2v_cmac));
  err = _gcry_cmac_generate_subkeys (c, &c->u_mode.siv.s2v_cmac);
  if (err != 0)
    return err;

  err = _gcry_cmac_write (c, &c->u_mode.siv.s2v_cmac, zero, GCRY_SIV_BLOCK_LEN);
  if (err != 0)
    return err;

  err = _gcry_cmac_final (c, &c->u_mode.siv.s2v_cmac);
  if (err != 0)
    return err;

  memcpy (c->u_mode.siv.s2v_zero_block, c->u_mode.siv.s2v_cmac.u_iv.iv,
	  GCRY_SIV_BLOCK_LEN);
  memcpy (c->u_mode.siv.s2v_d, c->u_mode.siv.s2v_zero_block,
	  GCRY_SIV_BLOCK_LEN);

  return 0;
}


gcry_err_code_t
_gcry_cipher_siv_authenticate (gcry_cipher_hd_t c,
                               const byte *aadbuf, size_t aadbuflen)
{
  gcry_err_code_t err;

  if (c->spec->blocksize != GCRY_SIV_BLOCK_LEN)
    return GPG_ERR_CIPHER_ALGO;
  if (c->marks.tag)
    return GPG_ERR_INV_STATE;
  if (c->marks.iv)
    return GPG_ERR_INV_STATE;

  if (c->u_mode.siv.aad_count >= 126)
    return GPG_ERR_INV_STATE; /* Too many AD vector components. */

  c->u_mode.siv.aad_count++;

  _gcry_cmac_reset (&c->u_mode.siv.s2v_cmac);

  err = _gcry_cmac_write (c, &c->u_mode.siv.s2v_cmac, aadbuf, aadbuflen);
  if (err != 0)
    return err;

  err = _gcry_cmac_final (c, &c->u_mode.siv.s2v_cmac);
  if (err != 0)
    return err;

  s2v_double (c->u_mode.siv.s2v_d);
  cipher_block_xor_1 (c->u_mode.siv.s2v_d, c->u_mode.siv.s2v_cmac.u_iv.iv,
		      GCRY_SIV_BLOCK_LEN);

  return 0;
}


gcry_err_code_t
_gcry_cipher_siv_set_nonce (gcry_cipher_hd_t c, const byte *nonce,
			    size_t noncelen)
{
  gcry_err_code_t err;

  err = _gcry_cipher_siv_authenticate (c, nonce, noncelen);
  if (err)
    return err;

  /* Nonce is the last AD before plaintext. */
  c->marks.iv = 1;

  return 0;
}


static gcry_err_code_t
s2v_plaintext (gcry_cipher_hd_t c, const byte *plain, size_t plainlen)
{
  gcry_err_code_t err;

  if (c->u_mode.siv.aad_count >= 127)
    return GPG_ERR_INV_STATE; /* Too many AD vector components. */

  _gcry_cmac_reset (&c->u_mode.siv.s2v_cmac);

  if (plainlen >= GCRY_SIV_BLOCK_LEN)
    {
      err = _gcry_cmac_write (c, &c->u_mode.siv.s2v_cmac, plain,
			      plainlen - GCRY_SIV_BLOCK_LEN);
      if (err)
        return err;

      cipher_block_xor_1 (c->u_mode.siv.s2v_d,
			  plain + plainlen - GCRY_SIV_BLOCK_LEN,
			  GCRY_SIV_BLOCK_LEN);

      err = _gcry_cmac_write (c, &c->u_mode.siv.s2v_cmac, c->u_mode.siv.s2v_d,
			      GCRY_SIV_BLOCK_LEN);
      if (err)
        return err;
    }
  else
    {
      unsigned char pad_sn[GCRY_SIV_BLOCK_LEN];

      s2v_double (c->u_mode.siv.s2v_d);
      s2v_pad (pad_sn, plain, plainlen);
      cipher_block_xor_1 (pad_sn, c->u_mode.siv.s2v_d, GCRY_SIV_BLOCK_LEN);

      err = _gcry_cmac_write (c, &c->u_mode.siv.s2v_cmac, pad_sn,
			      GCRY_SIV_BLOCK_LEN);
      wipememory (pad_sn, sizeof(pad_sn));
      if (err)
        return err;
    }

  c->u_mode.siv.aad_count++;

  return _gcry_cmac_final (c, &c->u_mode.siv.s2v_cmac);
}


gcry_err_code_t
_gcry_cipher_siv_encrypt (gcry_cipher_hd_t c,
                          byte *outbuf, size_t outbuflen,
                          const byte *inbuf, size_t inbuflen)
{
  gcry_err_code_t err;
  u64 q_lo;

  if (c->spec->blocksize != GCRY_SIV_BLOCK_LEN)
    return GPG_ERR_CIPHER_ALGO;
  if (outbuflen < inbuflen)
    return GPG_ERR_BUFFER_TOO_SHORT;
  if (c->marks.tag)
    return GPG_ERR_INV_STATE;
  if (c->u_mode.siv.dec_tag_set)
    return GPG_ERR_INV_STATE;

  /* Pass plaintext to S2V. */
  err = s2v_plaintext (c, inbuf, inbuflen);
  if (err != 0)
    return err;

  /* Clear 31th and 63th bits. */
  memcpy (c->u_ctr.ctr, c->u_mode.siv.s2v_cmac.u_iv.iv, GCRY_SIV_BLOCK_LEN);
  q_lo = buf_get_be64(c->u_ctr.ctr + 8);
  q_lo &= ~((u64)1 << 31);
  q_lo &= ~((u64)1 << 63);
  buf_put_be64(c->u_ctr.ctr + 8, q_lo);

  /* Encrypt plaintext. */
  err = _gcry_cipher_ctr_encrypt_ctx(c, outbuf, outbuflen, inbuf, inbuflen,
				     c->u_mode.siv.ctr_context);
  if (err != 0)
    return err;

  c->marks.tag = 1;

  return 0;
}


gcry_err_code_t
_gcry_cipher_siv_set_decryption_tag (gcry_cipher_hd_t c,
				     const byte *tag, size_t taglen)
{
  if (taglen != GCRY_SIV_BLOCK_LEN)
    return GPG_ERR_INV_ARG;
  if (c->spec->blocksize != GCRY_SIV_BLOCK_LEN)
    return GPG_ERR_CIPHER_ALGO;
  if (c->marks.tag)
    return GPG_ERR_INV_STATE;

  memcpy (&c->u_mode.siv.dec_tag, tag, GCRY_SIV_BLOCK_LEN);
  c->u_mode.siv.dec_tag_set = 1;

  return 0;
}


gcry_err_code_t
_gcry_cipher_siv_decrypt (gcry_cipher_hd_t c,
                          byte *outbuf, size_t outbuflen,
                          const byte *inbuf, size_t inbuflen)
{
  gcry_err_code_t err;
  u64 q_lo;

  if (c->spec->blocksize != GCRY_SIV_BLOCK_LEN)
    return GPG_ERR_CIPHER_ALGO;
  if (outbuflen < inbuflen)
    return GPG_ERR_BUFFER_TOO_SHORT;
  if (c->marks.tag)
    return GPG_ERR_INV_STATE;
  if (!c->u_mode.siv.dec_tag_set)
    return GPG_ERR_INV_STATE;

  /* Clear 31th and 63th bits. */
  memcpy (c->u_ctr.ctr, c->u_mode.siv.dec_tag, GCRY_SIV_BLOCK_LEN);
  q_lo = buf_get_be64(c->u_ctr.ctr + 8);
  q_lo &= ~((u64)1 << 31);
  q_lo &= ~((u64)1 << 63);
  buf_put_be64(c->u_ctr.ctr + 8, q_lo);

  /* Decrypt ciphertext. */
  err = _gcry_cipher_ctr_encrypt_ctx(c, outbuf, outbuflen, inbuf, inbuflen,
				     c->u_mode.siv.ctr_context);
  if (err != 0)
    return err;

  /* Pass plaintext to S2V. */
  err = s2v_plaintext (c, outbuf, inbuflen);
  if (err != 0)
    return err;

  c->marks.tag = 1;

  if (!buf_eq_const(c->u_mode.siv.s2v_cmac.u_iv.iv, c->u_mode.siv.dec_tag,
		    GCRY_SIV_BLOCK_LEN))
    {
      wipememory (outbuf, inbuflen);
      return GPG_ERR_CHECKSUM;
    }

  return 0;
}


gcry_err_code_t
_gcry_cipher_siv_get_tag (gcry_cipher_hd_t c, unsigned char *outbuf,
                          size_t outbuflen)
{
  gcry_err_code_t err;

  if (c->spec->blocksize != GCRY_SIV_BLOCK_LEN)
    return GPG_ERR_CIPHER_ALGO;
  if (c->u_mode.siv.dec_tag_set)
    return GPG_ERR_INV_STATE;

  if (!c->marks.tag)
    {
      /* Finalize SIV with zero-length plaintext. */
      err = s2v_plaintext (c, NULL, 0);
      if (err != 0)
        return err;

      c->marks.tag = 1;
    }

  if (outbuflen > GCRY_SIV_BLOCK_LEN)
    outbuflen = GCRY_SIV_BLOCK_LEN;

  /* We already checked that OUTBUF is large enough to hold
   * the result or has valid truncated length.  */
  memcpy (outbuf, c->u_mode.siv.s2v_cmac.u_iv.iv, outbuflen);

  return 0;
}


gcry_err_code_t
_gcry_cipher_siv_check_tag (gcry_cipher_hd_t c, const unsigned char *intag,
                            size_t taglen)
{
  gcry_err_code_t err;
  size_t n;

  if (c->spec->blocksize != GCRY_SIV_BLOCK_LEN)
    return GPG_ERR_CIPHER_ALGO;

  if (!c->marks.tag)
    {
      /* Finalize SIV with zero-length plaintext. */
      err = s2v_plaintext (c, NULL, 0);
      if (err != 0)
        return err;

      c->marks.tag = 1;
    }

  n = GCRY_SIV_BLOCK_LEN;
  if (taglen < n)
    n = taglen;

  if (!buf_eq_const(c->u_mode.siv.s2v_cmac.u_iv.iv, intag, n)
      || GCRY_SIV_BLOCK_LEN != taglen)
    {
      return GPG_ERR_CHECKSUM;
    }

  return 0;
}
