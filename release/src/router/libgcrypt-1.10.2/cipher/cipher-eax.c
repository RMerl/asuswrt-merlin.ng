/* cipher-eax.c  -  EAX implementation
 * Copyright (C) 2018 Jussi Kivilinna <jussi.kivilinna@iki.fi>
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


gcry_err_code_t
_gcry_cipher_eax_encrypt (gcry_cipher_hd_t c,
                          byte *outbuf, size_t outbuflen,
                          const byte *inbuf, size_t inbuflen)
{
  gcry_err_code_t err;

  if (outbuflen < inbuflen)
    return GPG_ERR_BUFFER_TOO_SHORT;
  if (c->marks.tag)
    return GPG_ERR_INV_STATE;

  if (!c->marks.iv)
    {
      err = _gcry_cipher_eax_set_nonce (c, NULL, 0);
      if (err != 0)
	return err;
    }

  while (inbuflen)
    {
      size_t currlen = inbuflen;

      /* Since checksumming is done after encryption, process input in 24KiB
       * chunks to keep data loaded in L1 cache for checksumming. */
      if (currlen > 24 * 1024)
	currlen = 24 * 1024;

      err = _gcry_cipher_ctr_encrypt (c, outbuf, outbuflen, inbuf, currlen);
      if (err != 0)
	return err;

      err = _gcry_cmac_write (c, &c->u_mode.eax.cmac_ciphertext, outbuf,
			      currlen);
      if (err != 0)
	return err;

      outbuf += currlen;
      inbuf += currlen;
      outbuflen -= currlen;
      inbuflen -= currlen;
    }

  return 0;
}


gcry_err_code_t
_gcry_cipher_eax_decrypt (gcry_cipher_hd_t c,
                          byte *outbuf, size_t outbuflen,
                          const byte *inbuf, size_t inbuflen)
{
  gcry_err_code_t err;

  if (outbuflen < inbuflen)
    return GPG_ERR_BUFFER_TOO_SHORT;
  if (c->marks.tag)
    return GPG_ERR_INV_STATE;

  if (!c->marks.iv)
    {
      err = _gcry_cipher_eax_set_nonce (c, NULL, 0);
      if (err != 0)
	return err;
    }

  while (inbuflen)
    {
      size_t currlen = inbuflen;

      /* Since checksumming is done before decryption, process input in 24KiB
       * chunks to keep data loaded in L1 cache for decryption. */
      if (currlen > 24 * 1024)
	currlen = 24 * 1024;

      err = _gcry_cmac_write (c, &c->u_mode.eax.cmac_ciphertext, inbuf,
			      currlen);
      if (err != 0)
	return err;

      err = _gcry_cipher_ctr_encrypt (c, outbuf, outbuflen, inbuf, currlen);
      if (err != 0)
	return err;

      outbuf += currlen;
      inbuf += currlen;
      outbuflen -= currlen;
      inbuflen -= currlen;
    }

  return 0;
}


gcry_err_code_t
_gcry_cipher_eax_authenticate (gcry_cipher_hd_t c,
                               const byte * aadbuf, size_t aadbuflen)
{
  gcry_err_code_t err;

  if (c->marks.tag)
    return GPG_ERR_INV_STATE;

  if (!c->marks.iv)
    {
      err = _gcry_cipher_eax_set_nonce (c, NULL, 0);
      if (err != 0)
	return err;
    }

  return _gcry_cmac_write (c, &c->u_mode.eax.cmac_header, aadbuf, aadbuflen);
}


gcry_err_code_t
_gcry_cipher_eax_setkey (gcry_cipher_hd_t c)
{
  gcry_err_code_t err;

  err = _gcry_cmac_generate_subkeys (c, &c->u_mode.eax.cmac_header);
  if (err != 0)
    return err;

  buf_cpy (c->u_mode.eax.cmac_ciphertext.subkeys,
	   c->u_mode.eax.cmac_header.subkeys,
	   sizeof(c->u_mode.eax.cmac_header.subkeys));

  return 0;
}


gcry_err_code_t
_gcry_cipher_eax_set_nonce (gcry_cipher_hd_t c, const byte *nonce,
			    size_t noncelen)
{
  gcry_cmac_context_t nonce_cmac;
  unsigned char initbuf[MAX_BLOCKSIZE];
  gcry_err_code_t err;

  c->marks.iv = 0;
  c->marks.tag = 0;

  _gcry_cmac_reset (&c->u_mode.eax.cmac_header);
  _gcry_cmac_reset (&c->u_mode.eax.cmac_ciphertext);

  /* Calculate nonce CMAC */

  memset(&nonce_cmac, 0, sizeof(nonce_cmac));
  memset(&initbuf, 0, sizeof(initbuf));

  buf_cpy (&nonce_cmac.subkeys, c->u_mode.eax.cmac_header.subkeys,
	   sizeof(c->u_mode.eax.cmac_header.subkeys));

  err = _gcry_cmac_write (c, &nonce_cmac, initbuf, c->spec->blocksize);
  if (err != 0)
    return err;

  if (noncelen != 0)
    {
      err = _gcry_cmac_write (c, &nonce_cmac, nonce, noncelen);
      if (err != 0)
        return err;
    }

  err = _gcry_cmac_final (c, &nonce_cmac);
  if (err != 0)
    return err;

  cipher_block_cpy (c->u_iv.iv, nonce_cmac.u_iv.iv, MAX_BLOCKSIZE);
  cipher_block_cpy (c->u_ctr.ctr, nonce_cmac.u_iv.iv, MAX_BLOCKSIZE);

  wipememory (&nonce_cmac, sizeof(nonce_cmac));

  /* Prepare header CMAC */

  initbuf[c->spec->blocksize - 1] = 1;
  err = _gcry_cmac_write (c, &c->u_mode.eax.cmac_header, initbuf,
			  c->spec->blocksize);
  if (err != 0)
    return err;

  /* Prepare ciphertext CMAC */

  initbuf[c->spec->blocksize - 1] = 2;
  err = _gcry_cmac_write (c, &c->u_mode.eax.cmac_ciphertext, initbuf,
			  c->spec->blocksize);
  if (err != 0)
    return err;

  c->marks.iv = 1;
  c->marks.tag = 0;

  return 0;
}


static gcry_err_code_t
_gcry_cipher_eax_tag (gcry_cipher_hd_t c,
                      byte *outbuf, size_t outbuflen, int check)
{
  gcry_err_code_t err;

  if (!c->marks.tag)
    {
      err = _gcry_cmac_final (c, &c->u_mode.eax.cmac_header);
      if (err != 0)
	return err;

      err = _gcry_cmac_final (c, &c->u_mode.eax.cmac_ciphertext);
      if (err != 0)
	return err;

      cipher_block_xor_1 (c->u_iv.iv, c->u_mode.eax.cmac_header.u_iv.iv,
                          MAX_BLOCKSIZE);
      cipher_block_xor_1 (c->u_iv.iv, c->u_mode.eax.cmac_ciphertext.u_iv.iv,
                          MAX_BLOCKSIZE);

      _gcry_cmac_reset (&c->u_mode.eax.cmac_header);
      _gcry_cmac_reset (&c->u_mode.eax.cmac_ciphertext);

      c->marks.tag = 1;
    }

  if (!check)
    {
      if (outbuflen > c->spec->blocksize)
        outbuflen = c->spec->blocksize;

      /* NB: We already checked that OUTBUF is large enough to hold
       * the result or has valid truncated length.  */
      memcpy (outbuf, c->u_iv.iv, outbuflen);
    }
  else
    {
      /* OUTBUFLEN gives the length of the user supplied tag in OUTBUF
       * and thus we need to compare its length first.  */
      if (!(outbuflen <= c->spec->blocksize)
          || !buf_eq_const (outbuf, c->u_iv.iv, outbuflen))
        return GPG_ERR_CHECKSUM;
    }

  return 0;
}


gcry_err_code_t
_gcry_cipher_eax_get_tag (gcry_cipher_hd_t c, unsigned char *outtag,
                          size_t taglen)
{
  return _gcry_cipher_eax_tag (c, outtag, taglen, 0);
}

gcry_err_code_t
_gcry_cipher_eax_check_tag (gcry_cipher_hd_t c, const unsigned char *intag,
                            size_t taglen)
{
  return _gcry_cipher_eax_tag (c, (unsigned char *) intag, taglen, 1);
}
