/* aes-decrypt.c

   Decryption function for aes/rijndael block cipher.

   Copyright (C) 2002, 2013 Niels Möller

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

#include <stdlib.h>

#include "aes-internal.h"

void
aes_decrypt(const struct aes_ctx *ctx,
	    size_t length, uint8_t *dst,
	    const uint8_t *src)
{
  switch (ctx->key_size)
    {
    default: abort();
    case AES128_KEY_SIZE:
      aes128_decrypt(&ctx->u.ctx128, length, dst, src);
      break;
    case AES192_KEY_SIZE:
      aes192_decrypt(&ctx->u.ctx192, length, dst, src);
      break;
    case AES256_KEY_SIZE:
      aes256_decrypt(&ctx->u.ctx256, length, dst, src);
      break;
    }
}
