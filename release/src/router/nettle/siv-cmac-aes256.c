/* siv-cmac-aes256.c

   AES-SIV, RFC5297

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
# include "config.h"
#endif

#include <assert.h>
#include <string.h>

#include "aes.h"
#include "siv-cmac.h"
#include "cmac.h"
#include "ctr.h"
#include "memxor.h"
#include "memops.h"

void
siv_cmac_aes256_set_key(struct siv_cmac_aes256_ctx *ctx, const uint8_t *key)
{
  siv_cmac_set_key(&ctx->cmac_key, &ctx->cmac_cipher, &ctx->ctr_cipher, &nettle_aes256, key);
}

void
siv_cmac_aes256_encrypt_message(const struct siv_cmac_aes256_ctx *ctx,
				size_t nlength, const uint8_t *nonce,
				size_t alength, const uint8_t *adata,
				size_t clength, uint8_t *dst, const uint8_t *src)
{
  siv_cmac_encrypt_message(&ctx->cmac_key, &ctx->cmac_cipher,
			   &nettle_aes256, &ctx->ctr_cipher,
			   nlength, nonce, alength, adata,
			   clength, dst, src);
}

int
siv_cmac_aes256_decrypt_message(const struct siv_cmac_aes256_ctx *ctx,
				size_t nlength, const uint8_t *nonce,
				size_t alength, const uint8_t *adata,
				size_t mlength, uint8_t *dst, const uint8_t *src)
{
  return siv_cmac_decrypt_message(&ctx->cmac_key, &ctx->cmac_cipher,
				  &nettle_aes256, &ctx->ctr_cipher,
				  nlength, nonce, alength, adata,
				  mlength, dst, src);
}
