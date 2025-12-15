/* siv-gcm-aes256.c

   AES-GCM-SIV, RFC8452

   Copyright (C) 2022 Red Hat, Inc.

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

#include "nettle-meta.h"
#include "siv-gcm.h"

void
siv_gcm_aes256_encrypt_message (const struct aes256_ctx *ctx,
				size_t nlength, const uint8_t *nonce,
				size_t alength, const uint8_t *adata,
				size_t clength, uint8_t *dst, const uint8_t *src)
{
  struct aes256_ctx ctr_ctx;
  siv_gcm_encrypt_message (&nettle_aes256, ctx, &ctr_ctx,
			   nlength, nonce,
			   alength, adata,
			   clength, dst, src);
}

int
siv_gcm_aes256_decrypt_message (const struct aes256_ctx *ctx,
				size_t nlength, const uint8_t *nonce,
				size_t alength, const uint8_t *adata,
				size_t mlength, uint8_t *dst, const uint8_t *src)
{
  struct aes256_ctx ctr_ctx;
  return siv_gcm_decrypt_message (&nettle_aes256, ctx, &ctr_ctx,
				  nlength, nonce,
				  alength, adata,
				  mlength, dst, src);
}
