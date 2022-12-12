/* nettle-meta-macs.c

   Copyright (C) 2020 Daiki Ueno

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

#include <stddef.h>

#include "nettle-meta.h"

const struct nettle_mac * const _nettle_macs[] = {
  &nettle_cmac_aes128,
  &nettle_cmac_aes256,
  &nettle_cmac_des3,
  &nettle_hmac_md5,
  &nettle_hmac_ripemd160,
  &nettle_hmac_sha1,
  &nettle_hmac_sha224,
  &nettle_hmac_sha256,
  &nettle_hmac_sha384,
  &nettle_hmac_sha512,
  &nettle_hmac_streebog256,
  &nettle_hmac_streebog512,
  &nettle_hmac_sm3,
  NULL
};

const struct nettle_mac * const *
nettle_get_macs (void)
{
  return _nettle_macs;
}
