/* nettle-meta-hashes.c

   Copyright (C) 2011 Daniel Kahn Gillmor

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

const struct nettle_hash * const _nettle_hashes[] = {
  &nettle_gosthash94,
  &nettle_gosthash94cp,
  &nettle_md2,
  &nettle_md4,
  &nettle_md5,
  &nettle_ripemd160,
  &nettle_sha1,
  &nettle_sha224,
  &nettle_sha256,
  &nettle_sha384,
  &nettle_sha512,
  &nettle_sha3_224,
  &nettle_sha3_256,
  &nettle_sha3_384,
  &nettle_sha3_512,
  &nettle_streebog256,
  &nettle_streebog512,
  &nettle_sm3,
  NULL
};

const struct nettle_hash * const *
nettle_get_hashes (void)
{
  return _nettle_hashes;
}
