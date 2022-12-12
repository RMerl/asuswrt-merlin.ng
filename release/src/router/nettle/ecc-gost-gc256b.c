/* ecc-gost-gc256b.c

   Copyright (C) 2016-2020 Dmitry Eremin-Solenikov

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

#include "ecc.h"
#include "ecc-internal.h"

#define USE_REDC 0

#include "ecc-gost-gc256b.h"

static void
ecc_gost_gc256b_modp (const struct ecc_modulo *m, mp_limb_t *rp, mp_limb_t *xp)
{
  mp_size_t mn = m->size;
  mp_limb_t hi;

  hi = mpn_addmul_1(xp, xp + mn, mn, 0x269);
  hi = sec_add_1 (xp, xp, mn, hi * 0x269);
  hi = sec_add_1 (rp, xp, mn, hi * 0x269);
  assert(hi == 0);
}

#define ecc_gost_gc256b_modp ecc_gost_gc256b_modp
#define ecc_gost_gc256b_modq ecc_mod

const struct ecc_curve _nettle_gost_gc256b =
{
  {
    256,
    ECC_LIMB_SIZE,
    ECC_BMODP_SIZE,
    ECC_REDC_SIZE,
    ECC_MOD_INV_ITCH (ECC_LIMB_SIZE),
    0,
    0,

    ecc_p,
    ecc_Bmodp,
    ecc_Bmodp_shifted,
    ecc_redc_ppm1,

    ecc_pp1h,
    ecc_gost_gc256b_modp,
    ecc_gost_gc256b_modp,
    ecc_mod_inv,
    NULL,
    NULL,
  },
  {
    256,
    ECC_LIMB_SIZE,
    ECC_BMODQ_SIZE,
    0,
    ECC_MOD_INV_ITCH (ECC_LIMB_SIZE),
    0,
    0,

    ecc_q,
    ecc_Bmodq,
    ecc_Bmodq_shifted,
    NULL,
    ecc_qp1h,

    ecc_gost_gc256b_modq,
    ecc_gost_gc256b_modq,
    ecc_mod_inv,
    NULL,
    NULL,
  },

  USE_REDC,
  ECC_PIPPENGER_K,
  ECC_PIPPENGER_C,

  ECC_ADD_JJA_ITCH (ECC_LIMB_SIZE),
  ECC_ADD_JJJ_ITCH (ECC_LIMB_SIZE),
  ECC_DUP_JJ_ITCH (ECC_LIMB_SIZE),
  ECC_MUL_A_ITCH (ECC_LIMB_SIZE),
  ECC_MUL_G_ITCH (ECC_LIMB_SIZE),
  ECC_J_TO_A_ITCH (ECC_LIMB_SIZE, ECC_MOD_INV_ITCH(ECC_LIMB_SIZE)),

  ecc_add_jja,
  ecc_add_jjj,
  ecc_dup_jj,
  ecc_mul_a,
  ecc_mul_g,
  ecc_j_to_a,

  ecc_b,
  ecc_unit,
  ecc_table
};

const struct ecc_curve *nettle_get_gost_gc256b(void)
{
  return &_nettle_gost_gc256b;
}
