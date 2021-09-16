/* curve448-mul-g.c

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

#include <string.h>

#include "curve448.h"

#include "ecc.h"
#include "ecc-internal.h"

/* Intended to be compatible with NaCl's crypto_scalarmult_base. */
void
curve448_mul_g (uint8_t *r, const uint8_t *n)
{
  const struct ecc_curve *ecc = &_nettle_curve448;
  uint8_t t[CURVE448_SIZE];
  mp_limb_t *scratch;
  mp_size_t itch;

#define ng scratch
#define x (scratch + 3*ecc->p.size)
#define scratch_out (scratch + 4*ecc->p.size)

  memcpy (t, n, sizeof(t));
  t[0] &= ~3;
  t[CURVE448_SIZE-1] = (t[CURVE448_SIZE-1] & 0x7f) | 0x80;

  itch = 5*ecc->p.size + ecc->mul_g_itch;
  scratch = gmp_alloc_limbs (itch);

  mpn_set_base256_le (x, ecc->p.size, t, CURVE448_SIZE);

  ecc_mul_g_eh (ecc, ng, x, scratch_out);
  curve448_eh_to_x (x, ng, scratch_out);

  mpn_get_base256_le (r, CURVE448_SIZE, x, ecc->p.size);
  gmp_free_limbs (scratch, itch);
#undef ng
#undef x
#undef scratch_out
}
