/* cmac-aes256.c

   CMAC using AES256 as the underlying cipher.

   Copyright (C) 2017 Red Hat, Inc.

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

#include "cmac.h"

void
cmac_aes256_set_key(struct cmac_aes256_ctx *ctx, const uint8_t *key)
{
  CMAC128_SET_KEY(ctx, aes256_set_encrypt_key, aes256_encrypt, key);
}

void
cmac_aes256_update (struct cmac_aes256_ctx *ctx,
		   size_t length, const uint8_t *data)
{
  CMAC128_UPDATE (ctx, aes256_encrypt, length, data);
}

void
cmac_aes256_digest(struct cmac_aes256_ctx *ctx,
		  size_t length, uint8_t *digest)
{
  CMAC128_DIGEST(ctx, aes256_encrypt, length, digest);
}
