/* aes-set-encrypt-key.c

   Key setup for the aes/rijndael block cipher.

   Copyright (C) 2000, 2001, 2002 Rafael R. Sevilla, Niels Möller
   Copyright (C) 2013 Niels Möller

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

#include "aes.h"

void
aes_set_encrypt_key(struct aes_ctx *ctx,
		    size_t key_size, const uint8_t *key)
{
  switch (key_size)
    {
    default: abort();
    case AES128_KEY_SIZE:
      aes128_set_encrypt_key(&ctx->u.ctx128, key);
      break;
    case AES192_KEY_SIZE:
      aes192_set_encrypt_key(&ctx->u.ctx192, key);
      break;
    case AES256_KEY_SIZE:
      aes256_set_encrypt_key(&ctx->u.ctx256, key);
      break;
    }
  
  ctx->key_size = key_size;
}
