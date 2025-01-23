/* crc-armv8-ce.c - ARMv8-CE PMULL accelerated CRC implementation
 * Copyright (C) 2019 Jussi Kivilinna <jussi.kivilinna@iki.fi>
 *
 * This file is part of Libgcrypt.
 *
 * Libgcrypt is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * Libgcrypt is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 *
 */

#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "g10lib.h"

#include "bithelp.h"
#include "bufhelp.h"


#if defined(ENABLE_ARM_CRYPTO_SUPPORT) && defined(__AARCH64EL__) && \
    defined(HAVE_COMPATIBLE_GCC_AARCH64_PLATFORM_AS) && \
    defined(HAVE_GCC_INLINE_ASM_AARCH64_CRYPTO)


#define ALIGNED_16 __attribute__ ((aligned (16)))


struct u16_unaligned_s
{
  u16 a;
} __attribute__((packed, aligned (1), may_alias));

struct u32_unaligned_s
{
  u32 a;
} __attribute__((packed, aligned (1), may_alias));


/* Constants structure for generic reflected/non-reflected CRC32 PMULL
 * functions. */
struct crc32_consts_s
{
  /* k: { x^(32*17), x^(32*15), x^(32*5), x^(32*3), x^(32*2), 0 } mod P(x) */
  u64 k[6];
  /* my_p: { floor(x^64 / P(x)), P(x) } */
  u64 my_p[2];
};

/* PMULL constants for CRC32 and CRC32RFC1510. */
static const struct crc32_consts_s crc32_consts ALIGNED_16 =
{
  { /* k[6] = reverse_33bits( x^(32*y) mod P(x) ) */
    U64_C(0x154442bd4), U64_C(0x1c6e41596), /* y = { 17, 15 } */
    U64_C(0x1751997d0), U64_C(0x0ccaa009e), /* y = { 5, 3 } */
    U64_C(0x163cd6124), 0                   /* y = 2 */
  },
  { /* my_p[2] = reverse_33bits ( { floor(x^64 / P(x)), P(x) } ) */
    U64_C(0x1f7011641), U64_C(0x1db710641)
  }
};

/* PMULL constants for CRC24RFC2440 (polynomial multiplied with x⁸). */
static const struct crc32_consts_s crc24rfc2440_consts ALIGNED_16 =
{
  { /* k[6] = x^(32*y) mod P(x) << 32*/
    U64_C(0x08289a00) << 32, U64_C(0x74b44a00) << 32, /* y = { 17, 15 } */
    U64_C(0xc4b14d00) << 32, U64_C(0xfd7e0c00) << 32, /* y = { 5, 3 } */
    U64_C(0xd9fe8c00) << 32, 0                        /* y = 2 */
  },
  { /* my_p[2] = { floor(x^64 / P(x)), P(x) } */
    U64_C(0x1f845fe24), U64_C(0x1864cfb00)
  }
};


u32 _gcry_crc32r_armv8_ce_reduction_4 (u32 data, u32 crc,
				       const struct crc32_consts_s *consts);
void _gcry_crc32r_armv8_ce_bulk (u32 *pcrc, const byte *inbuf, size_t inlen,
                                 const struct crc32_consts_s *consts);

u32 _gcry_crc32_armv8_ce_reduction_4 (u32 data, u32 crc,
				      const struct crc32_consts_s *consts);
void _gcry_crc32_armv8_ce_bulk (u32 *pcrc, const byte *inbuf, size_t inlen,
                                const struct crc32_consts_s *consts);


static inline void
crc32r_less_than_16 (u32 *pcrc, const byte *inbuf, size_t inlen,
		     const struct crc32_consts_s *consts)
{
  u32 crc = *pcrc;
  u32 data;

  while (inlen >= 4)
    {
      data = ((const struct u32_unaligned_s *)inbuf)->a;
      data ^= crc;

      inlen -= 4;
      inbuf += 4;

      crc = _gcry_crc32r_armv8_ce_reduction_4 (data, 0, consts);
    }

  switch (inlen)
    {
    case 0:
      break;
    case 1:
      data = inbuf[0];
      data ^= crc;
      data <<= 24;
      crc >>= 8;
      crc = _gcry_crc32r_armv8_ce_reduction_4 (data, crc, consts);
      break;
    case 2:
      data = ((const struct u16_unaligned_s *)inbuf)->a;
      data ^= crc;
      data <<= 16;
      crc >>= 16;
      crc = _gcry_crc32r_armv8_ce_reduction_4 (data, crc, consts);
      break;
    case 3:
      data = ((const struct u16_unaligned_s *)inbuf)->a;
      data |= inbuf[2] << 16;
      data ^= crc;
      data <<= 8;
      crc >>= 24;
      crc = _gcry_crc32r_armv8_ce_reduction_4 (data, crc, consts);
      break;
    }

  *pcrc = crc;
}

static inline void
crc32_less_than_16 (u32 *pcrc, const byte *inbuf, size_t inlen,
		    const struct crc32_consts_s *consts)
{
  u32 crc = *pcrc;
  u32 data;

  while (inlen >= 4)
    {
      data = ((const struct u32_unaligned_s *)inbuf)->a;
      data ^= crc;
      data = _gcry_bswap32(data);

      inlen -= 4;
      inbuf += 4;

      crc = _gcry_crc32_armv8_ce_reduction_4 (data, 0, consts);
    }

  switch (inlen)
    {
    case 0:
      break;
    case 1:
      data = inbuf[0];
      data ^= crc;
      data = data & 0xffU;
      crc = _gcry_bswap32(crc >> 8);
      crc = _gcry_crc32_armv8_ce_reduction_4 (data, crc, consts);
      break;
    case 2:
      data = ((const struct u16_unaligned_s *)inbuf)->a;
      data ^= crc;
      data = _gcry_bswap32(data << 16);
      crc = _gcry_bswap32(crc >> 16);
      crc = _gcry_crc32_armv8_ce_reduction_4 (data, crc, consts);
      break;
    case 3:
      data = ((const struct u16_unaligned_s *)inbuf)->a;
      data |= inbuf[2] << 16;
      data ^= crc;
      data = _gcry_bswap32(data << 8);
      crc = crc & 0xff000000U;
      crc = _gcry_crc32_armv8_ce_reduction_4 (data, crc, consts);
      break;
    }

  *pcrc = crc;
}

void
_gcry_crc32_armv8_ce_pmull (u32 *pcrc, const byte *inbuf, size_t inlen)
{
  const struct crc32_consts_s *consts = &crc32_consts;

  if (!inlen)
    return;

  if (inlen >= 16)
    _gcry_crc32r_armv8_ce_bulk (pcrc, inbuf, inlen, consts);
  else
    crc32r_less_than_16 (pcrc, inbuf, inlen, consts);
}

void
_gcry_crc24rfc2440_armv8_ce_pmull (u32 *pcrc, const byte *inbuf, size_t inlen)
{
  const struct crc32_consts_s *consts = &crc24rfc2440_consts;

  if (!inlen)
    return;

  /* Note: *pcrc in input endian. */

  if (inlen >= 16)
    _gcry_crc32_armv8_ce_bulk (pcrc, inbuf, inlen, consts);
  else
    crc32_less_than_16 (pcrc, inbuf, inlen, consts);
}

#endif
