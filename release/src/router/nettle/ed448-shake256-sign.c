/* ed448-shake256-sign.c

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

#include "eddsa.h"

#include "ecc-internal.h"
#include "eddsa-internal.h"
#include "sha3.h"

void
ed448_shake256_sign (const uint8_t *pub,
		     const uint8_t *priv,
		     size_t length, const uint8_t *msg,
		     uint8_t *signature)
{
  const struct ecc_curve *ecc = &_nettle_curve448;
  const struct ecc_eddsa *eddsa = &_nettle_ed448_shake256;
  mp_size_t itch = ecc->q.size + _eddsa_sign_itch (ecc);
  mp_limb_t *scratch = gmp_alloc_limbs (itch);
#define k2 scratch
#define scratch_out (scratch + ecc->q.size)
  struct sha3_256_ctx ctx;
  uint8_t digest[ED448_SIGNATURE_SIZE];

  sha3_256_init (&ctx);
  _eddsa_expand_key (ecc, eddsa, &ctx, priv, digest, k2);

  _eddsa_sign (ecc, eddsa, &ctx,
	       pub, digest + ED448_KEY_SIZE, k2,
	       length, msg, signature, scratch_out);

  gmp_free_limbs (scratch, itch);
#undef k1
#undef k2
#undef scratch_out
}
