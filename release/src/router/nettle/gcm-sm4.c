/* gcm-sm4.c

   Galois counter mode using SM4 as the underlying cipher.

   Copyright (C) 2022 Tianjia Zhang <tianjia.zhang@linux.alibaba.com>

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

#include "gcm.h"

void
gcm_sm4_set_key(struct gcm_sm4_ctx *ctx, const uint8_t *key)
{
  GCM_SET_KEY(ctx, sm4_set_encrypt_key, sm4_crypt, key);
}

void
gcm_sm4_set_iv(struct gcm_sm4_ctx *ctx,
	       size_t length, const uint8_t *iv)
{
  GCM_SET_IV (ctx, length, iv);
}

void
gcm_sm4_update(struct gcm_sm4_ctx *ctx,
	       size_t length, const uint8_t *data)
{
  GCM_UPDATE (ctx, length, data);
}

void
gcm_sm4_encrypt(struct gcm_sm4_ctx *ctx,
		size_t length, uint8_t *dst, const uint8_t *src)
{
  GCM_ENCRYPT(ctx, sm4_crypt, length, dst, src);
}

void
gcm_sm4_decrypt(struct gcm_sm4_ctx *ctx,
		size_t length, uint8_t *dst, const uint8_t *src)
{
  GCM_DECRYPT(ctx, sm4_crypt, length, dst, src);
}

void
gcm_sm4_digest(struct gcm_sm4_ctx *ctx,
	       size_t length, uint8_t *digest)
{
  GCM_DIGEST(ctx, sm4_crypt, length, digest);
}
