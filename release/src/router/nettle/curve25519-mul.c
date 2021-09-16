/* curve25519-mul.c

   Copyright (C) 2014 Niels Möller

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

#include <string.h>

#include "curve25519.h"

#include "ecc.h"
#include "ecc-internal.h"

/* Intended to be compatible with NaCl's crypto_scalarmult. */
void
curve25519_mul (uint8_t *q, const uint8_t *n, const uint8_t *p)
{
  const struct ecc_modulo *m = &_nettle_curve25519.p;
  mp_size_t itch;
  mp_limb_t *x;

  itch = m->size + ECC_MUL_M_ITCH(m->size);
  x = gmp_alloc_limbs (itch);

  /* Note that 255 % GMP_NUMB_BITS == 0 isn't supported, so x always
     holds at least 256 bits. */
  mpn_set_base256_le (x, m->size, p, CURVE25519_SIZE);
  /* Clear bit 255, as required by RFC 7748. */
  x[255/GMP_NUMB_BITS] &= ~((mp_limb_t) 1 << (255 % GMP_NUMB_BITS));

  ecc_mul_m (m, 121665, 3, 253, x, n, x, x + m->size);
  mpn_get_base256_le (q, CURVE25519_SIZE, x, m->size);

  gmp_free_limbs (x, itch);
}
