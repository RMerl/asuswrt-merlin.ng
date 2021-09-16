/* pss-mgf1.c

   PKCS#1 mask generation function 1, used in RSA-PSS (RFC-3447).

   Copyright (C) 2017 Daiki Ueno

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

#include "pss-mgf1.h"

#include <assert.h>
#include <string.h>

#include "nettle-internal.h"
#include "macros.h"

void
pss_mgf1(const void *seed, const struct nettle_hash *hash,
	 size_t length, uint8_t *mask)
{
  TMP_DECL(h, uint8_t, NETTLE_MAX_HASH_DIGEST_SIZE);
  TMP_DECL_ALIGN(state, NETTLE_MAX_HASH_CONTEXT_SIZE);
  size_t i;
  uint8_t c[4];

  TMP_ALLOC(h, hash->digest_size);
  TMP_ALLOC_ALIGN(state, hash->context_size);

  for (i = 0;;
       i++, mask += hash->digest_size, length -= hash->digest_size)
    {
      WRITE_UINT32(c, i);

      memcpy(state, seed, hash->context_size);
      hash->update(state, 4, c);

      if (length <= hash->digest_size)
	{
	  hash->digest(state, length, mask);
	  return;
	}
      hash->digest(state, hash->digest_size, mask);
    }
}
