/* curve448-mul.c

   Copyright (C) 2017 Daiki Ueno
   Copyright (C) 2017 Red Hat, Inc.

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

#include <assert.h>
#include <string.h>

#include "curve448.h"

#include "ecc.h"
#include "ecc-internal.h"

/* Intended to be compatible with NaCl's crypto_scalarmult. */
void
curve448_mul (uint8_t *q, const uint8_t *n, const uint8_t *p)
{
  const struct ecc_modulo *m = &_nettle_curve448.p;
  mp_size_t itch;
  mp_limb_t *x;

  itch = m->size + ECC_MUL_M_ITCH(m->size);
  x = gmp_alloc_limbs (itch);

  mpn_set_base256_le (x, m->size, p, CURVE448_SIZE);
  ecc_mul_m (m, 39081, 2, 446, x, n, x, x + m->size);
  mpn_get_base256_le (q, CURVE448_SIZE, x, m->size);

  gmp_free_limbs (x, itch);
}
