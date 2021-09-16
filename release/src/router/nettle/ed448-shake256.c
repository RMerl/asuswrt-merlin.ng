/* ed448-shake256.c

   Copyright (C) 2019 Niels Möller

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

#include "eddsa-internal.h"

#include "nettle-types.h"
#include "sha3.h"

#define DOM_SIZE 10

static nettle_eddsa_dom_func ed448_dom;

static void
ed448_dom(void *ctx)
{
  static const uint8_t dom[DOM_SIZE] =
    { 'S', 'i', 'g', 'E', 'd', '4', '4', '8', 0, 0};
  sha3_256_update (ctx, DOM_SIZE, dom);
}

const struct ecc_eddsa _nettle_ed448_shake256 =
  {
    (nettle_hash_update_func *) sha3_256_update,
    (nettle_hash_digest_func *) sha3_256_shake,
    ed448_dom,
    ~(mp_limb_t) 3,
    (mp_limb_t) 1 << (447 % GMP_NUMB_BITS),
  };
