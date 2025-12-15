/* dsa-hash.c

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

#include "dsa.h"
#include "dsa-internal.h"

#include "gmp-glue.h"

/* Convert hash value to an integer. The general description of DSA in
   FIPS186-3 allows both larger and smaller q; in the the former case
   the hash is zero-padded at the left, in the latter case, the hash
   is truncated at the right.

   NOTE: We don't considered the hash value to be secret, so it's ok
   if the running time of this conversion depends on h.

   Output size is ceil(bit_size / GMP_NUMB_BITS).
*/

void
_nettle_dsa_hash (mp_limb_t *hp, unsigned bit_size,
		  size_t length, const uint8_t *digest)
{
  unsigned octet_size = (bit_size + 7) / 8;
  unsigned limb_size = NETTLE_BIT_SIZE_TO_LIMB_SIZE (bit_size);

  if (length > octet_size)
    length = octet_size;

  mpn_set_base256(hp, limb_size, digest, length);

  if (8 * length > bit_size)
    /* We got a few extra bits, at the low end. Discard them. */
    mpn_rshift (hp, hp, limb_size, 8*length - bit_size);
}

/* Uses little-endian order, and no trimming of left-over bits in the
   last byte (bits will instead be reduced mod q later). */
void
_nettle_gostdsa_hash (mp_limb_t *hp, unsigned bit_size,
		      size_t length, const uint8_t *digest)
{
  unsigned octet_size = (bit_size + 7) / 8;
  unsigned limb_size = NETTLE_BIT_SIZE_TO_LIMB_SIZE (bit_size);

  if (length > octet_size)
    length = octet_size;

  mpn_set_base256_le(hp, limb_size, digest, length);
}
