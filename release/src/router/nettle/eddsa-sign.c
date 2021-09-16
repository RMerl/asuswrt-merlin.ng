/* eddsa-sign.c

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

#include <assert.h>

#include "eddsa.h"
#include "eddsa-internal.h"

#include "ecc.h"
#include "ecc-internal.h"
#include "nettle-meta.h"

mp_size_t
_eddsa_sign_itch (const struct ecc_curve *ecc)
{
  assert (ecc->mul_g_itch <= _eddsa_compress_itch (ecc));
  return 5*ecc->p.size + _eddsa_compress_itch (ecc);
}

void
_eddsa_sign (const struct ecc_curve *ecc,
	     const struct ecc_eddsa *eddsa,
	     void *ctx,
	     const uint8_t *pub,
	     const uint8_t *k1,
	     const mp_limb_t *k2,
	     size_t length,
	     const uint8_t *msg,
	     uint8_t *signature,
	     mp_limb_t *scratch)
{
  mp_size_t size;
  size_t nbytes;
  mp_limb_t q, cy;

#define rp scratch
#define hp (scratch + size)
#define P (scratch + 2*size)
#define sp (scratch + 2*size)
#define hash ((uint8_t *) (scratch + 3*size))
#define scratch_out (scratch + 5*size)

  size = ecc->p.size;
  nbytes = 1 + ecc->p.bit_size / 8;

  eddsa->dom (ctx);
  eddsa->update (ctx, nbytes, k1);
  eddsa->update (ctx, length, msg);
  eddsa->digest (ctx, 2*nbytes, hash);
  _eddsa_hash (&ecc->q, rp, 2*nbytes, hash);

  ecc->mul_g (ecc, P, rp, scratch_out);
  _eddsa_compress (ecc, signature, P, scratch_out);

  eddsa->dom (ctx);
  eddsa->update (ctx, nbytes, signature);
  eddsa->update (ctx, nbytes, pub);
  eddsa->update (ctx, length, msg);
  eddsa->digest (ctx, 2*nbytes, hash);
  _eddsa_hash (&ecc->q, hp, 2*nbytes, hash);

  ecc_mod_mul (&ecc->q, sp, hp, k2, sp);
  ecc_mod_add (&ecc->q, sp, sp, rp); /* FIXME: Can be plain add */
  if (ecc->p.bit_size == 255)
    {
      /* FIXME: Special code duplicated in ecc_curve25519_modq
	 Define a suitable method for canonical reduction? */

      /* q is slightly larger than 2^252, underflow from below
	 mpn_submul_1 is unlikely. */
      unsigned shift = 252 - GMP_NUMB_BITS * (ecc->p.size - 1);
      q = sp[ecc->p.size-1] >> shift;
    }
  else
    {
      unsigned shift;

      assert (ecc->p.bit_size == 448);
      /* q is slightly smaller than 2^446 */
      shift = 446 - GMP_NUMB_BITS * (ecc->p.size - 1);
      /* Add one, then it's possible but unlikely that below
	 mpn_submul_1 does *not* underflow. */
      q = (sp[ecc->p.size-1] >> shift) + 1;
    }

  cy = mpn_submul_1 (sp, ecc->q.m, ecc->p.size, q);
  assert (cy < 2);
  cy -= mpn_cnd_add_n (cy, sp, sp, ecc->q.m, ecc->p.size);
  assert (cy == 0);

  mpn_get_base256_le (signature + nbytes, nbytes, sp, ecc->q.size);
#undef rp
#undef hp
#undef P
#undef sp
#undef hash
}
