/* hkdf.c

   Copyright (C) 2017 Red Hat, Inc.

   Author: Nikos Mavrogiannopoulos

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

/* Functions for the HKDF handling.
 */

#if HAVE_CONFIG_H
# include "config.h"
#endif

#include "hkdf.h"

/* hkdf_extract: Outputs a PRK of digest_size
 */
void
hkdf_extract(void *mac_ctx,
	     nettle_hash_update_func *update,
	     nettle_hash_digest_func *digest,
	     size_t digest_size,
	     size_t secret_size, const uint8_t *secret,
	     uint8_t *dst)
{
  update(mac_ctx, secret_size, secret);
  digest(mac_ctx, digest_size, dst);
}

/* hkdf_expand: Outputs an arbitrary key of size specified by length
 */
void
hkdf_expand(void *mac_ctx,
	    nettle_hash_update_func *update,
	    nettle_hash_digest_func *digest,
	    size_t digest_size,
	    size_t info_size, const uint8_t *info,
	    size_t length, uint8_t *dst)
{
  uint8_t i = 1;

  if (!length)
    return;

  for (;; dst += digest_size, length -= digest_size, i++)
    {
      update(mac_ctx, info_size, info);
      update(mac_ctx, 1, &i);
      if (length <= digest_size)
	break;

      digest(mac_ctx, digest_size, dst);
      update(mac_ctx, digest_size, dst);
    }

  digest(mac_ctx, length, dst);
}
