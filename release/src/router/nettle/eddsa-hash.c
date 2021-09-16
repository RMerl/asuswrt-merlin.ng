/* eddsa-hash.c

   Copyright (C) 2014, 2019 Niels Möller
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

#include "eddsa.h"
#include "eddsa-internal.h"

#include "ecc.h"
#include "ecc-internal.h"
#include "nettle-internal.h"

/* Convert hash digest to integer, and reduce canonically modulo q.
   Needs space for 2*m->size + 1 at rp. */
void
_eddsa_hash (const struct ecc_modulo *m,
	     mp_limb_t *rp, size_t digest_size, const uint8_t *digest)
{
  mp_size_t nlimbs = (8*digest_size + GMP_NUMB_BITS - 1) / GMP_NUMB_BITS;
  mp_limb_t cy;

  mpn_set_base256_le (rp, nlimbs, digest, digest_size);

  if (nlimbs > 2*m->size)
    {
      /* Special case for Ed448: reduce rp to 2*m->size limbs.
	 After decoding rp from a hash of size 2*rn:

	 rp = r2 || r1 || r0

	 where r0 and r1 have m->size limbs.  Reduce this to:

	 rp = r1' || r0

	 where r1' has m->size limbs.  */
      mp_limb_t hi = rp[2*m->size];
      assert (nlimbs == 2*m->size + 1);

      hi = mpn_addmul_1 (rp + m->size, m->B, m->size, hi);
      assert (hi <= 1);
      hi = mpn_cnd_add_n (hi, rp + m->size, rp + m->size, m->B, m->size);
      assert (hi == 0);
    }
  m->mod (m, rp + m->size , rp);
  /* Ensure canonical reduction. */
  cy = mpn_sub_n (rp, rp + m->size, m->m, m->size);
  cnd_copy (cy, rp, rp + m->size, m->size);
}
