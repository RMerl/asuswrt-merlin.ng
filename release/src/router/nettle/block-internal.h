/* block-internal.h

   Internal implementations of nettle_blockZ-related functions.

   Copyright (C) 2011 Katholieke Universiteit Leuven
   Copyright (C) 2011, 2013, 2018 Niels Möller
   Copyright (C) 2018 Red Hat, Inc.
   Copyright (C) 2019 Dmitry Eremin-Solenikov

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

#ifndef NETTLE_BLOCK_INTERNAL_H_INCLUDED
#define NETTLE_BLOCK_INTERNAL_H_INCLUDED

#include <assert.h>

#include "nettle-types.h"
#include "memxor.h"

static inline void
block16_xor (union nettle_block16 *r,
	     const union nettle_block16 *x)
{
  r->u64[0] ^= x->u64[0];
  r->u64[1] ^= x->u64[1];
}

static inline void
block16_xor3 (union nettle_block16 *r,
	      const union nettle_block16 *x,
	      const union nettle_block16 *y)
{
  r->u64[0] = x->u64[0] ^ y->u64[0];
  r->u64[1] = x->u64[1] ^ y->u64[1];
}

static inline void
block16_xor_bytes (union nettle_block16 *r,
		   const union nettle_block16 *x,
		   const uint8_t *bytes)
{
  memxor3 (r->b, x->b, bytes, 16);
}

static inline void
block8_xor (union nettle_block8 *r,
	    const union nettle_block8 *x)
{
  r->u64 ^= x->u64;
}

static inline void
block8_xor3 (union nettle_block8 *r,
	     const union nettle_block8 *x,
	     const union nettle_block8 *y)
{
  r->u64 = x->u64 ^ y->u64;
}

static inline void
block8_xor_bytes (union nettle_block8 *r,
		  const union nettle_block8 *x,
		  const uint8_t *bytes)
{
  memxor3 (r->b, x->b, bytes, 8);
}

/* Do a foreign-endianness shift of data */

#define LSHIFT_ALIEN_UINT64(x) \
	((((x) & UINT64_C(0x7f7f7f7f7f7f7f7f)) << 1) | \
	 (((x) & UINT64_C(0x8080808080808080)) >> 15))
#define RSHIFT_ALIEN_UINT64(x) \
	((((x) & UINT64_C(0xfefefefefefefefe)) >> 1) | \
	 (((x) & UINT64_C(0x0001010101010101)) << 15))

/* Two typical defining polynoms */

#define BLOCK16_POLY (UINT64_C(0x87))
#define BLOCK8_POLY (UINT64_C(0x1b))
#define GHASH_POLY (UINT64_C(0xE1))

/* Galois multiplications by 2:
 * functions differ in shifting right or left, big- or little- endianness
 * and by defining polynom.
 * r == x is allowed. */

#if WORDS_BIGENDIAN
static inline void
block16_mulx_be (union nettle_block16 *dst,
		 const union nettle_block16 *src)
{
  uint64_t carry = src->u64[0] >> 63;
  dst->u64[0] = (src->u64[0] << 1) | (src->u64[1] >> 63);
  dst->u64[1] = (src->u64[1] << 1) ^ (BLOCK16_POLY & -carry);
}

static inline void
block16_mulx_le (union nettle_block16 *dst,
		 const union nettle_block16 *src)
{
  uint64_t carry = (src->u64[1] & 0x80) >> 7;
  dst->u64[1] = LSHIFT_ALIEN_UINT64(src->u64[1]) | ((src->u64[0] & 0x80) << 49);
  dst->u64[0] = LSHIFT_ALIEN_UINT64(src->u64[0]) ^ ((BLOCK16_POLY << 56) & -carry);
}

static inline void
block8_mulx_be (union nettle_block8 *dst,
		const union nettle_block8 *src)
{
  uint64_t carry = src->u64 >> 63;

  dst->u64 = (src->u64 << 1) ^ (BLOCK8_POLY & -carry);
}

static inline void
block16_mulx_ghash (union nettle_block16 *r,
		    const union nettle_block16 *x)
{
  uint64_t mask;

  /* Shift uses big-endian representation. */
  mask = - (x->u64[1] & 1);
  r->u64[1] = (x->u64[1] >> 1) | ((x->u64[0] & 1) << 63);
  r->u64[0] = (x->u64[0] >> 1) ^ (mask & (GHASH_POLY << 56));
}
#else /* !WORDS_BIGENDIAN */
static inline void
block16_mulx_be (union nettle_block16 *dst,
		 const union nettle_block16 *src)
{
  uint64_t carry = (src->u64[0] & 0x80) >> 7;
  dst->u64[0] = LSHIFT_ALIEN_UINT64(src->u64[0]) | ((src->u64[1] & 0x80) << 49);
  dst->u64[1] = LSHIFT_ALIEN_UINT64(src->u64[1]) ^ ((BLOCK16_POLY << 56) & -carry);
}

static inline void
block16_mulx_le (union nettle_block16 *dst,
		 const union nettle_block16 *src)
{
  uint64_t carry = src->u64[1] >> 63;
  dst->u64[1] = (src->u64[1] << 1) | (src->u64[0] >> 63);
  dst->u64[0] = (src->u64[0] << 1) ^ (BLOCK16_POLY & -carry);
}

static inline void
block8_mulx_be (union nettle_block8 *dst,
		const union nettle_block8 *src)
{
  uint64_t carry = (src->u64 & 0x80) >> 7;

  dst->u64 = LSHIFT_ALIEN_UINT64(src->u64) ^ ((BLOCK8_POLY << 56) & -carry);
}

static inline void
block16_mulx_ghash (union nettle_block16 *r,
		    const union nettle_block16 *x)
{
  uint64_t mask;

  /* Shift uses big-endian representation. */
  mask = - ((x->u64[1] >> 56) & 1);
  r->u64[1] = RSHIFT_ALIEN_UINT64(x->u64[1]) | ((x->u64[0] >> 49) & 0x80);
  r->u64[0] = RSHIFT_ALIEN_UINT64(x->u64[0]) ^ (mask & GHASH_POLY);
}
#endif /* ! WORDS_BIGENDIAN */

#endif /* NETTLE_BLOCK_INTERNAL_H_INCLUDED */
