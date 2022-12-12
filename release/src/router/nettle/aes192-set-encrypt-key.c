/* aes192-set-encrypt-key.c

   Copyright (C) 2013, Niels Möller

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

#include "aes-internal.h"

/* For fat builds */
#if HAVE_NATIVE_aes192_set_encrypt_key
void
_nettle_aes192_set_encrypt_key_c(struct aes192_ctx *ctx, const uint8_t *key);
# define nettle_aes192_set_encrypt_key _nettle_aes192_set_encrypt_key_c
#endif

void
nettle_aes192_set_encrypt_key(struct aes192_ctx *ctx, const uint8_t *key)
{
  _nettle_aes_set_key (_AES192_ROUNDS, AES192_KEY_SIZE / 4, ctx->keys, key);
}
