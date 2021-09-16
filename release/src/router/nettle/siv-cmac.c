/* siv-cmac.c

   SIV-CMAC, RFC5297

   Copyright (C) 2017 Nikos Mavrogiannopoulos

   This file is part of GNU Nettle.

   GNU Nettle is free software: you can redistribute it and/or
   modify it under the terms of either:

     * the GNU Lesser General Public License as published by the Free
       Software Foundation; either version 3 of the License, or (at your
       option) any later version.

   or

     * the GNU General Public License as published by the Free
       Software Foundation; either version 2 of the License, or (at your
       option) any later version.

   or both in parallel, as here.

   GNU Nettle is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received copies of the GNU General Public License and
   the GNU Lesser General Public License along with this program.  If
   not, see http://www.gnu.org/licenses/.
*/

#if HAVE_CONFIG_H
#include "config.h"
#endif

#include <assert.h>
#include <string.h>

#include "aes.h"
#include "siv-cmac.h"
#include "cmac.h"
#include "ctr.h"
#include "memxor.h"
#include "memops.h"
#include "nettle-internal.h"
#include "block-internal.h"

/* This is an implementation of S2V for the AEAD case where
 * vectors if zero, are considered as S empty components */
static void
_siv_s2v (const struct nettle_cipher *nc,
	  const struct cmac128_key *cmac_key,
	  const void *cmac_cipher,
	  size_t alength, const uint8_t * adata,
	  size_t nlength, const uint8_t * nonce,
	  size_t plength, const uint8_t * pdata, uint8_t * v)
{
  union nettle_block16 D, S, T;
  static const union nettle_block16 const_zero = {.b = 0 };
  struct cmac128_ctx cmac_ctx;
  assert (nlength >= SIV_MIN_NONCE_SIZE);

  cmac128_init(&cmac_ctx);
  cmac128_update (&cmac_ctx, cmac_cipher, nc->encrypt, 16, const_zero.b);
  cmac128_digest (&cmac_ctx, cmac_key, cmac_cipher, nc->encrypt, 16, D.b);

  block16_mulx_be (&D, &D);
  cmac128_update (&cmac_ctx, cmac_cipher, nc->encrypt, alength, adata);
  cmac128_digest (&cmac_ctx, cmac_key, cmac_cipher, nc->encrypt, 16, S.b);
  block16_xor (&D, &S);

  block16_mulx_be (&D, &D);
  cmac128_update (&cmac_ctx, cmac_cipher, nc->encrypt, nlength, nonce);
  cmac128_digest (&cmac_ctx, cmac_key, cmac_cipher, nc->encrypt, 16, S.b);
  block16_xor (&D, &S);

  /* Sn */
  if (plength >= 16)
    {
      cmac128_update (&cmac_ctx, cmac_cipher, nc->encrypt, plength - 16, pdata);

      pdata += plength - 16;

      block16_xor_bytes (&T, &D, pdata);
    }
  else
    {
      union nettle_block16 pad;

      block16_mulx_be (&T, &D);
      memcpy (pad.b, pdata, plength);
      pad.b[plength] = 0x80;
      if (plength + 1 < 16)
	memset (&pad.b[plength + 1], 0, 16 - plength - 1);

      block16_xor (&T, &pad);
    }

  cmac128_update (&cmac_ctx, cmac_cipher, nc->encrypt, 16, T.b);
  cmac128_digest (&cmac_ctx, cmac_key, cmac_cipher, nc->encrypt, 16, v);
}

void
siv_cmac_set_key (struct cmac128_key *cmac_key, void *cmac_cipher, void *siv_cipher,
		  const struct nettle_cipher *nc, const uint8_t * key)
{
  nc->set_encrypt_key (cmac_cipher, key);
  cmac128_set_key (cmac_key, cmac_cipher, nc->encrypt);
  nc->set_encrypt_key (siv_cipher, key + nc->key_size);
}

void
siv_cmac_encrypt_message (const struct cmac128_key *cmac_key,
			  const void *cmac_cipher,
			  const struct nettle_cipher *nc,
			  const void *ctr_cipher,
			  size_t nlength, const uint8_t * nonce,
			  size_t alength, const uint8_t * adata,
			  size_t clength, uint8_t * dst, const uint8_t * src)
{
  union nettle_block16 siv;
  size_t slength;

  assert (clength >= SIV_DIGEST_SIZE);
  slength = clength - SIV_DIGEST_SIZE;

  /* create CTR nonce */
  _siv_s2v (nc, cmac_key, cmac_cipher, alength, adata, nlength, nonce, slength, src, siv.b);

  memcpy (dst, siv.b, SIV_DIGEST_SIZE);
  siv.b[8] &= ~0x80;
  siv.b[12] &= ~0x80;

  ctr_crypt (ctr_cipher, nc->encrypt, AES_BLOCK_SIZE, siv.b, slength,
	     dst + SIV_DIGEST_SIZE, src);
}

int
siv_cmac_decrypt_message (const struct cmac128_key *cmac_key,
			  const void *cmac_cipher,
			  const struct nettle_cipher *nc,
			  const void *ctr_cipher,
			  size_t nlength, const uint8_t * nonce,
			  size_t alength, const uint8_t * adata,
			  size_t mlength, uint8_t * dst, const uint8_t * src)
{
  union nettle_block16 siv;
  union nettle_block16 ctr;

  memcpy (ctr.b, src, SIV_DIGEST_SIZE);
  ctr.b[8] &= ~0x80;
  ctr.b[12] &= ~0x80;

  ctr_crypt (ctr_cipher, nc->encrypt, AES_BLOCK_SIZE, ctr.b,
	     mlength, dst, src + SIV_DIGEST_SIZE);

  /* create CTR nonce */
  _siv_s2v (nc,
	    cmac_key, cmac_cipher, alength, adata,
	    nlength, nonce, mlength, dst, siv.b);

  return memeql_sec (siv.b, src, SIV_DIGEST_SIZE);
}
