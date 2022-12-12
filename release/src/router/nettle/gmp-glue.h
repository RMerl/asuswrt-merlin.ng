/* gmp-glue.h

   Copyright (C) 2013 Niels Möller
   Copyright (C) 2013 Red Hat

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

#ifndef NETTLE_GMP_GLUE_H_INCLUDED
#define NETTLE_GMP_GLUE_H_INCLUDED

#include "bignum.h"

#define mpz_limbs_copy _nettle_mpz_limbs_copy
#define mpz_set_n _nettle_mpz_set_n
#define sec_zero_p _nettle_sec_zero_p
#define mpn_set_base256 _nettle_mpn_set_base256
#define mpn_set_base256_le _nettle_mpn_set_base256_le
#define mpn_get_base256 _nettle_mpn_get_base256
#define mpn_get_base256_le _nettle_mpn_get_base256_le
#define gmp_alloc_limbs _nettle_gmp_alloc_limbs
#define gmp_free_limbs _nettle_gmp_free_limbs
#define gmp_free _nettle_gmp_free
#define gmp_alloc _nettle_gmp_alloc

#define TMP_GMP_DECL(name, type) type *name;	\
  size_t tmp_##name##_size
#define TMP_GMP_ALLOC(name, size) do {					\
    tmp_##name##_size = (size);						\
    (name) = gmp_alloc(sizeof (*name) * (size));	\
  } while (0)
#define TMP_GMP_FREE(name) (gmp_free(name, tmp_##name##_size))

#if NETTLE_USE_MINI_GMP
mp_limb_t
mpn_cnd_add_n (mp_limb_t cnd, mp_limb_t *rp,
	       const mp_limb_t *ap, const mp_limb_t *bp, mp_size_t n);

mp_limb_t
mpn_cnd_sub_n (mp_limb_t cnd, mp_limb_t *rp,
	       const mp_limb_t *ap, const mp_limb_t *bp, mp_size_t n);

void
mpn_cnd_swap (mp_limb_t cnd, volatile mp_limb_t *ap, volatile mp_limb_t *bp, mp_size_t n);
#endif

/* Side-channel silent variant of mpn_zero_p. */
int
sec_zero_p (const mp_limb_t *ap, mp_size_t n);

#define NETTLE_OCTET_SIZE_TO_LIMB_SIZE(n) \
  (((n) * 8 + GMP_NUMB_BITS - 1) / GMP_NUMB_BITS)

/* Convenience functions */

/* Copy limbs, with zero-padding. */
/* FIXME: Reorder arguments, on the theory that the first argument of
   an _mpz_* function should be an mpz_t? Or rename to _mpz_get_limbs,
   with argument order consistent with mpz_get_*. */
void
mpz_limbs_copy (mp_limb_t *xp, mpz_srcptr x, mp_size_t n);

void
mpz_set_n (mpz_t r, const mp_limb_t *xp, mp_size_t xn);

/* Like mpn_set_str, but always writes rn limbs. If input is larger,
   higher bits are ignored. */
void
mpn_set_base256 (mp_limb_t *rp, mp_size_t rn,
		 const uint8_t *xp, size_t xn);

void
mpn_set_base256_le (mp_limb_t *rp, mp_size_t rn,
		    const uint8_t *xp, size_t xn);

void
mpn_get_base256 (uint8_t *rp, size_t rn,
	         const mp_limb_t *xp, mp_size_t xn);

void
mpn_get_base256_le (uint8_t *rp, size_t rn,
		    const mp_limb_t *xp, mp_size_t xn);


mp_limb_t *
gmp_alloc_limbs (mp_size_t n);

void
gmp_free_limbs (mp_limb_t *p, mp_size_t n);

void *gmp_alloc(size_t n);
void gmp_free(void *p, size_t n);

#endif /* NETTLE_GMP_GLUE_H_INCLUDED */
