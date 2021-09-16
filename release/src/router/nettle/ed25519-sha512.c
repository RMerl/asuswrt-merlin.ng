/* ed25519-sha512.c

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
#include "sha2.h"

static nettle_eddsa_dom_func ed25519_dom;

static void ed25519_dom(void *ctx UNUSED) {}

const struct ecc_eddsa _nettle_ed25519_sha512 =
  {
    (nettle_hash_update_func *) sha512_update,
    (nettle_hash_digest_func *) sha512_digest,
    ed25519_dom,
    ~(mp_limb_t) 7,
    (mp_limb_t) 1 << (254 % GMP_NUMB_BITS),
  };
