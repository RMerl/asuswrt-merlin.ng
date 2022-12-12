/* ghash-update.c

   Galois counter mode, specified by NIST,
   http://csrc.nist.gov/publications/nistpubs/800-38D/SP-800-38D.pdf

   See also the gcm paper at
   http://www.cryptobarn.com/papers/gcm-spec.pdf.

   Copyright (C) 2011 Katholieke Universiteit Leuven
   Copyright (C) 2011, 2013, 2018, 2022 Niels Möller
   Copyright (C) 2018 Red Hat, Inc.

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

#include "ghash-internal.h"
#include "block-internal.h"

#if GCM_TABLE_BITS != 8
# error Unsupported table size.
#endif

/* For fat builds */
#if HAVE_NATIVE_ghash_update
const uint8_t *
_nettle_ghash_update_c (const struct gcm_key *ctx, union nettle_block16 *state,
			size_t blocks, const uint8_t *data);
#define _nettle_ghash_update _nettle_ghash_update_c

#endif
#if WORDS_BIGENDIAN
# define W(left,right) (0x##left##right)
#else
# define W(left,right) (0x##right##left)
#endif

static const uint16_t
shift_table[0x100] = {
  W(00,00),W(01,c2),W(03,84),W(02,46),W(07,08),W(06,ca),W(04,8c),W(05,4e),
  W(0e,10),W(0f,d2),W(0d,94),W(0c,56),W(09,18),W(08,da),W(0a,9c),W(0b,5e),
  W(1c,20),W(1d,e2),W(1f,a4),W(1e,66),W(1b,28),W(1a,ea),W(18,ac),W(19,6e),
  W(12,30),W(13,f2),W(11,b4),W(10,76),W(15,38),W(14,fa),W(16,bc),W(17,7e),
  W(38,40),W(39,82),W(3b,c4),W(3a,06),W(3f,48),W(3e,8a),W(3c,cc),W(3d,0e),
  W(36,50),W(37,92),W(35,d4),W(34,16),W(31,58),W(30,9a),W(32,dc),W(33,1e),
  W(24,60),W(25,a2),W(27,e4),W(26,26),W(23,68),W(22,aa),W(20,ec),W(21,2e),
  W(2a,70),W(2b,b2),W(29,f4),W(28,36),W(2d,78),W(2c,ba),W(2e,fc),W(2f,3e),
  W(70,80),W(71,42),W(73,04),W(72,c6),W(77,88),W(76,4a),W(74,0c),W(75,ce),
  W(7e,90),W(7f,52),W(7d,14),W(7c,d6),W(79,98),W(78,5a),W(7a,1c),W(7b,de),
  W(6c,a0),W(6d,62),W(6f,24),W(6e,e6),W(6b,a8),W(6a,6a),W(68,2c),W(69,ee),
  W(62,b0),W(63,72),W(61,34),W(60,f6),W(65,b8),W(64,7a),W(66,3c),W(67,fe),
  W(48,c0),W(49,02),W(4b,44),W(4a,86),W(4f,c8),W(4e,0a),W(4c,4c),W(4d,8e),
  W(46,d0),W(47,12),W(45,54),W(44,96),W(41,d8),W(40,1a),W(42,5c),W(43,9e),
  W(54,e0),W(55,22),W(57,64),W(56,a6),W(53,e8),W(52,2a),W(50,6c),W(51,ae),
  W(5a,f0),W(5b,32),W(59,74),W(58,b6),W(5d,f8),W(5c,3a),W(5e,7c),W(5f,be),
  W(e1,00),W(e0,c2),W(e2,84),W(e3,46),W(e6,08),W(e7,ca),W(e5,8c),W(e4,4e),
  W(ef,10),W(ee,d2),W(ec,94),W(ed,56),W(e8,18),W(e9,da),W(eb,9c),W(ea,5e),
  W(fd,20),W(fc,e2),W(fe,a4),W(ff,66),W(fa,28),W(fb,ea),W(f9,ac),W(f8,6e),
  W(f3,30),W(f2,f2),W(f0,b4),W(f1,76),W(f4,38),W(f5,fa),W(f7,bc),W(f6,7e),
  W(d9,40),W(d8,82),W(da,c4),W(db,06),W(de,48),W(df,8a),W(dd,cc),W(dc,0e),
  W(d7,50),W(d6,92),W(d4,d4),W(d5,16),W(d0,58),W(d1,9a),W(d3,dc),W(d2,1e),
  W(c5,60),W(c4,a2),W(c6,e4),W(c7,26),W(c2,68),W(c3,aa),W(c1,ec),W(c0,2e),
  W(cb,70),W(ca,b2),W(c8,f4),W(c9,36),W(cc,78),W(cd,ba),W(cf,fc),W(ce,3e),
  W(91,80),W(90,42),W(92,04),W(93,c6),W(96,88),W(97,4a),W(95,0c),W(94,ce),
  W(9f,90),W(9e,52),W(9c,14),W(9d,d6),W(98,98),W(99,5a),W(9b,1c),W(9a,de),
  W(8d,a0),W(8c,62),W(8e,24),W(8f,e6),W(8a,a8),W(8b,6a),W(89,2c),W(88,ee),
  W(83,b0),W(82,72),W(80,34),W(81,f6),W(84,b8),W(85,7a),W(87,3c),W(86,fe),
  W(a9,c0),W(a8,02),W(aa,44),W(ab,86),W(ae,c8),W(af,0a),W(ad,4c),W(ac,8e),
  W(a7,d0),W(a6,12),W(a4,54),W(a5,96),W(a0,d8),W(a1,1a),W(a3,5c),W(a2,9e),
  W(b5,e0),W(b4,22),W(b6,64),W(b7,a6),W(b2,e8),W(b3,2a),W(b1,6c),W(b0,ae),
  W(bb,f0),W(ba,32),W(b8,74),W(b9,b6),W(bc,f8),W(bd,3a),W(bf,7c),W(be,be),
};
#undef W

static void
gcm_gf_shift_8(union nettle_block16 *x)
{
  uint64_t reduce;

  /* Shift uses big-endian representation. */
#if WORDS_BIGENDIAN
  reduce = shift_table[x->u64[1] & 0xff];
  x->u64[1] = (x->u64[1] >> 8) | ((x->u64[0] & 0xff) << 56);
  x->u64[0] = (x->u64[0] >> 8) ^ (reduce << 48);
#else /* ! WORDS_BIGENDIAN */
  reduce = shift_table[(x->u64[1] >> 56) & 0xff];
  x->u64[1] = (x->u64[1] << 8) | (x->u64[0] >> 56);
  x->u64[0] = (x->u64[0] << 8) ^ reduce;
#endif /* ! WORDS_BIGENDIAN */
}

static void
gcm_gf_mul (union nettle_block16 *x, const union nettle_block16 *table)
{
  union nettle_block16 Z;
  unsigned i;

  Z = table[x->b[GCM_BLOCK_SIZE-1]];

  for (i = GCM_BLOCK_SIZE-2; i > 0; i--)
    {
      gcm_gf_shift_8(&Z);
      block16_xor(&Z, &table[x->b[i]]);
    }
  gcm_gf_shift_8(&Z);
  block16_xor3(x, &Z, &table[x->b[0]]);
}

const uint8_t *
_ghash_update (const struct gcm_key *ctx, union nettle_block16 *state,
	       size_t blocks, const uint8_t *data)
{
  for (; blocks-- > 0; data += GCM_BLOCK_SIZE)
    {
      memxor (state->b, data, GCM_BLOCK_SIZE);
      gcm_gf_mul (state, ctx->h);
    }
  return data;
}
