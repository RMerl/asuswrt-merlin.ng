/* balloon-sha1.c

   Balloon password-hashing algorithm.

   Copyright (C) 2022 Zoltan Fridrich
   Copyright (C) 2022 Red Hat, Inc.

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

#include "balloon.h"
#include "sha1.h"

void
balloon_sha1(size_t s_cost, size_t t_cost,
             size_t passwd_length, const uint8_t *passwd,
             size_t salt_length, const uint8_t *salt,
             uint8_t *scratch, uint8_t *dst)
{
  struct sha1_ctx ctx;
  sha1_init(&ctx);
  balloon(&ctx,
          (nettle_hash_update_func*)sha1_update,
          (nettle_hash_digest_func*)sha1_digest,
          SHA1_DIGEST_SIZE, s_cost, t_cost,
          passwd_length, passwd, salt_length, salt, scratch, dst);
}
