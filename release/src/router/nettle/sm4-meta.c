/* sm4-meta.c

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

#include "nettle-meta.h"

#include "sm4.h"

const struct nettle_cipher nettle_sm4 = {
  "sm4",
  sizeof(struct sm4_ctx),
  SM4_BLOCK_SIZE,
  SM4_KEY_SIZE,
  (nettle_set_key_func *) sm4_set_encrypt_key,
  (nettle_set_key_func *) sm4_set_decrypt_key,
  (nettle_cipher_func *) sm4_crypt,
  (nettle_cipher_func *) sm4_crypt
};
