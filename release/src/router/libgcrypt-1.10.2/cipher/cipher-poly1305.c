/* cipher-poly1305.c  -  Poly1305 based AEAD cipher mode, RFC-8439
 * Copyright (C) 2014 Jussi Kivilinna <jussi.kivilinna@iki.fi>
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
#include "./poly1305-internal.h"


static inline int
poly1305_bytecounter_add (u32 ctr[2], size_t add)
{
  int overflow = 0;

  if (sizeof(add) > sizeof(u32))
    {
      u32 high_add = ((add >> 31) >> 1) & 0xffffffff;
      ctr[1] += high_add;
      if (ctr[1] < high_add)
        overflow = 1;
    }

  ctr[0] += add;
  if (ctr[0] >= add)
    return overflow;

  ctr[1] += 1;
  return (ctr[1] < 1) || overflow;
}


static void
poly1305_fill_bytecounts (gcry_cipher_hd_t c)
{
  u32 lenbuf[4];

  lenbuf[0] = le_bswap32(c->u_mode.poly1305.aadcount[0]);
  lenbuf[1] = le_bswap32(c->u_mode.poly1305.aadcount[1]);
  lenbuf[2] = le_bswap32(c->u_mode.poly1305.datacount[0]);
  lenbuf[3] = le_bswap32(c->u_mode.poly1305.datacount[1]);
  _gcry_poly1305_update (&c->u_mode.poly1305.ctx, (byte*)lenbuf,
			 sizeof(lenbuf));

  wipememory(lenbuf, sizeof(lenbuf));
}


static void
poly1305_do_padding (gcry_cipher_hd_t c, u32 ctr[2])
{
  static const byte zero_padding_buf[15] = {};
  u32 padding_count;

  /* Padding to 16 byte boundary. */
  if (ctr[0] % 16 > 0)
    {
      padding_count = 16 - ctr[0] % 16;

      _gcry_poly1305_update (&c->u_mode.poly1305.ctx, zero_padding_buf,
			     padding_count);
    }
}


static void
poly1305_aad_finish (gcry_cipher_hd_t c)
{
  /* After AAD, feed padding bytes so we get 16 byte alignment. */
  poly1305_do_padding (c, c->u_mode.poly1305.aadcount);

  /* Start of encryption marks end of AAD stream. */
  c->u_mode.poly1305.aad_finalized = 1;

  c->u_mode.poly1305.datacount[0] = 0;
  c->u_mode.poly1305.datacount[1] = 0;
}


static gcry_err_code_t
poly1305_set_zeroiv (gcry_cipher_hd_t c)
{
  byte zero[8] = { 0, };

  return _gcry_cipher_poly1305_setiv (c, zero, sizeof(zero));
}


gcry_err_code_t
_gcry_cipher_poly1305_authenticate (gcry_cipher_hd_t c,
				    const byte * aadbuf, size_t aadbuflen)
{
  if (c->u_mode.poly1305.bytecount_over_limits)
    return GPG_ERR_INV_LENGTH;
  if (c->u_mode.poly1305.aad_finalized)
    return GPG_ERR_INV_STATE;
  if (c->marks.tag)
    return GPG_ERR_INV_STATE;

  if (!c->marks.iv)
    poly1305_set_zeroiv(c);

  if (poly1305_bytecounter_add(c->u_mode.poly1305.aadcount, aadbuflen))
    {
      c->u_mode.poly1305.bytecount_over_limits = 1;
      return GPG_ERR_INV_LENGTH;
    }

  _gcry_poly1305_update (&c->u_mode.poly1305.ctx, aadbuf, aadbuflen);

  return 0;
}


gcry_err_code_t
_gcry_cipher_poly1305_encrypt (gcry_cipher_hd_t c,
			       byte *outbuf, size_t outbuflen,
			       const byte *inbuf, size_t inbuflen)
{
  gcry_err_code_t err;

  if (outbuflen < inbuflen)
    return GPG_ERR_BUFFER_TOO_SHORT;
  if (c->marks.tag)
    return GPG_ERR_INV_STATE;
  if (c->u_mode.poly1305.bytecount_over_limits)
    return GPG_ERR_INV_LENGTH;

  if (!c->marks.iv)
    {
      err = poly1305_set_zeroiv(c);
      if (err)
        return err;
    }

  if (!c->u_mode.poly1305.aad_finalized)
    poly1305_aad_finish(c);

  if (poly1305_bytecounter_add(c->u_mode.poly1305.datacount, inbuflen))
    {
      c->u_mode.poly1305.bytecount_over_limits = 1;
      return GPG_ERR_INV_LENGTH;
    }

#ifdef USE_CHACHA20
  if (LIKELY(inbuflen > 0) && LIKELY(c->spec->algo == GCRY_CIPHER_CHACHA20))
    {
      return _gcry_chacha20_poly1305_encrypt (c, outbuf, inbuf, inbuflen);
    }
#endif

  while (inbuflen)
    {
      size_t currlen = inbuflen;

      /* Since checksumming is done after encryption, process input in 24KiB
       * chunks to keep data loaded in L1 cache for checksumming. */
      if (currlen > 24 * 1024)
	currlen = 24 * 1024;

      c->spec->stencrypt(&c->context.c, outbuf, (byte*)inbuf, currlen);

      _gcry_poly1305_update (&c->u_mode.poly1305.ctx, outbuf, currlen);

      outbuf += currlen;
      inbuf += currlen;
      outbuflen -= currlen;
      inbuflen -= currlen;
    }

  return 0;
}


gcry_err_code_t
_gcry_cipher_poly1305_decrypt (gcry_cipher_hd_t c,
			       byte *outbuf, size_t outbuflen,
			       const byte *inbuf, size_t inbuflen)
{
  gcry_err_code_t err;

  if (outbuflen < inbuflen)
    return GPG_ERR_BUFFER_TOO_SHORT;
  if (c->marks.tag)
    return GPG_ERR_INV_STATE;
  if (c->u_mode.poly1305.bytecount_over_limits)
    return GPG_ERR_INV_LENGTH;

  if (!c->marks.iv)
    {
      err = poly1305_set_zeroiv(c);
      if (err)
        return err;
    }

  if (!c->u_mode.poly1305.aad_finalized)
    poly1305_aad_finish(c);

  if (poly1305_bytecounter_add(c->u_mode.poly1305.datacount, inbuflen))
    {
      c->u_mode.poly1305.bytecount_over_limits = 1;
      return GPG_ERR_INV_LENGTH;
    }

#ifdef USE_CHACHA20
  if (LIKELY(inbuflen > 0) && LIKELY(c->spec->algo == GCRY_CIPHER_CHACHA20))
    {
      return _gcry_chacha20_poly1305_decrypt (c, outbuf, inbuf, inbuflen);
    }
#endif

  while (inbuflen)
    {
      size_t currlen = inbuflen;

      /* Since checksumming is done before decryption, process input in 24KiB
       * chunks to keep data loaded in L1 cache for decryption. */
      if (currlen > 24 * 1024)
	currlen = 24 * 1024;

      _gcry_poly1305_update (&c->u_mode.poly1305.ctx, inbuf, currlen);

      c->spec->stdecrypt(&c->context.c, outbuf, (byte*)inbuf, currlen);

      outbuf += currlen;
      inbuf += currlen;
      outbuflen -= currlen;
      inbuflen -= currlen;
    }

  return 0;
}


static gcry_err_code_t
_gcry_cipher_poly1305_tag (gcry_cipher_hd_t c,
			   byte * outbuf, size_t outbuflen, int check)
{
  gcry_err_code_t err;

  if (outbuflen < POLY1305_TAGLEN)
    return GPG_ERR_BUFFER_TOO_SHORT;
  if (c->u_mode.poly1305.bytecount_over_limits)
    return GPG_ERR_INV_LENGTH;

  if (!c->marks.iv)
    {
      err = poly1305_set_zeroiv(c);
      if (err)
        return err;
    }

  if (!c->u_mode.poly1305.aad_finalized)
    poly1305_aad_finish(c);

  if (!c->marks.tag)
    {
      /* After data, feed padding bytes so we get 16 byte alignment. */
      poly1305_do_padding (c, c->u_mode.poly1305.datacount);

      /* Write byte counts to poly1305. */
      poly1305_fill_bytecounts(c);

      _gcry_poly1305_finish(&c->u_mode.poly1305.ctx, c->u_iv.iv);

      c->marks.tag = 1;
    }

  if (!check)
    {
      memcpy (outbuf, c->u_iv.iv, POLY1305_TAGLEN);
    }
  else
    {
      /* OUTBUFLEN gives the length of the user supplied tag in OUTBUF
       * and thus we need to compare its length first.  */
      if (outbuflen != POLY1305_TAGLEN
          || !buf_eq_const (outbuf, c->u_iv.iv, POLY1305_TAGLEN))
        return GPG_ERR_CHECKSUM;
    }

  return 0;
}


gcry_err_code_t
_gcry_cipher_poly1305_get_tag (gcry_cipher_hd_t c, unsigned char *outtag,
                          size_t taglen)
{
  return _gcry_cipher_poly1305_tag (c, outtag, taglen, 0);
}

gcry_err_code_t
_gcry_cipher_poly1305_check_tag (gcry_cipher_hd_t c, const unsigned char *intag,
                            size_t taglen)
{
  return _gcry_cipher_poly1305_tag (c, (unsigned char *) intag, taglen, 1);
}


void
_gcry_cipher_poly1305_setkey (gcry_cipher_hd_t c)
{
  c->u_mode.poly1305.aadcount[0] = 0;
  c->u_mode.poly1305.aadcount[1] = 0;

  c->u_mode.poly1305.datacount[0] = 0;
  c->u_mode.poly1305.datacount[1] = 0;

  c->u_mode.poly1305.bytecount_over_limits = 0;
  c->u_mode.poly1305.aad_finalized = 0;
  c->marks.tag = 0;
  c->marks.iv = 0;
}


gcry_err_code_t
_gcry_cipher_poly1305_setiv (gcry_cipher_hd_t c, const byte *iv, size_t ivlen)
{
  byte tmpbuf[64]; /* size of ChaCha20 block */
  gcry_err_code_t err;

  /* IV must be 96-bits */
  if (!iv && ivlen != (96 / 8))
    return GPG_ERR_INV_ARG;

  memset(&c->u_mode.poly1305.ctx, 0, sizeof(c->u_mode.poly1305.ctx));

  c->u_mode.poly1305.aadcount[0] = 0;
  c->u_mode.poly1305.aadcount[1] = 0;

  c->u_mode.poly1305.datacount[0] = 0;
  c->u_mode.poly1305.datacount[1] = 0;

  c->u_mode.poly1305.bytecount_over_limits = 0;
  c->u_mode.poly1305.aad_finalized = 0;
  c->marks.tag = 0;
  c->marks.iv = 0;

  /* Set up IV for stream cipher. */
  c->spec->setiv (&c->context.c, iv, ivlen);

  /* Get the first block from ChaCha20. */
  memset(tmpbuf, 0, sizeof(tmpbuf));
  c->spec->stencrypt(&c->context.c, tmpbuf, tmpbuf, sizeof(tmpbuf));

  /* Use the first 32-bytes as Poly1305 key. */
  err = _gcry_poly1305_init (&c->u_mode.poly1305.ctx, tmpbuf, POLY1305_KEYLEN);

  wipememory(tmpbuf, sizeof(tmpbuf));

  if (err)
    return err;

  c->marks.iv = 1;
  return 0;
}
