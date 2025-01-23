/* sha256-ppc.c - PowerPC vcrypto implementation of SHA-256 transform
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
 * License along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#include <config.h>

#if defined(ENABLE_PPC_CRYPTO_SUPPORT) && \
    defined(HAVE_COMPATIBLE_CC_PPC_ALTIVEC) && \
    defined(HAVE_GCC_INLINE_ASM_PPC_ALTIVEC) && \
    defined(USE_SHA256) && \
    __GNUC__ >= 4

#include <altivec.h>
#include "bufhelp.h"


typedef vector unsigned char vector16x_u8;
typedef vector unsigned int vector4x_u32;
typedef vector unsigned long long vector2x_u64;


#define ALWAYS_INLINE inline __attribute__((always_inline))
#define NO_INLINE __attribute__((noinline))
#define NO_INSTRUMENT_FUNCTION __attribute__((no_instrument_function))

#define ASM_FUNC_ATTR          NO_INSTRUMENT_FUNCTION
#define ASM_FUNC_ATTR_INLINE   ASM_FUNC_ATTR ALWAYS_INLINE
#define ASM_FUNC_ATTR_NOINLINE ASM_FUNC_ATTR NO_INLINE


static const u32 K[64] =
  {
#define TBL(v) v
    TBL(0x428a2f98), TBL(0x71374491), TBL(0xb5c0fbcf), TBL(0xe9b5dba5),
    TBL(0x3956c25b), TBL(0x59f111f1), TBL(0x923f82a4), TBL(0xab1c5ed5),
    TBL(0xd807aa98), TBL(0x12835b01), TBL(0x243185be), TBL(0x550c7dc3),
    TBL(0x72be5d74), TBL(0x80deb1fe), TBL(0x9bdc06a7), TBL(0xc19bf174),
    TBL(0xe49b69c1), TBL(0xefbe4786), TBL(0x0fc19dc6), TBL(0x240ca1cc),
    TBL(0x2de92c6f), TBL(0x4a7484aa), TBL(0x5cb0a9dc), TBL(0x76f988da),
    TBL(0x983e5152), TBL(0xa831c66d), TBL(0xb00327c8), TBL(0xbf597fc7),
    TBL(0xc6e00bf3), TBL(0xd5a79147), TBL(0x06ca6351), TBL(0x14292967),
    TBL(0x27b70a85), TBL(0x2e1b2138), TBL(0x4d2c6dfc), TBL(0x53380d13),
    TBL(0x650a7354), TBL(0x766a0abb), TBL(0x81c2c92e), TBL(0x92722c85),
    TBL(0xa2bfe8a1), TBL(0xa81a664b), TBL(0xc24b8b70), TBL(0xc76c51a3),
    TBL(0xd192e819), TBL(0xd6990624), TBL(0xf40e3585), TBL(0x106aa070),
    TBL(0x19a4c116), TBL(0x1e376c08), TBL(0x2748774c), TBL(0x34b0bcb5),
    TBL(0x391c0cb3), TBL(0x4ed8aa4a), TBL(0x5b9cca4f), TBL(0x682e6ff3),
    TBL(0x748f82ee), TBL(0x78a5636f), TBL(0x84c87814), TBL(0x8cc70208),
    TBL(0x90befffa), TBL(0xa4506ceb), TBL(0xbef9a3f7), TBL(0xc67178f2)
#undef TBL
  };


static ASM_FUNC_ATTR_INLINE vector4x_u32
vec_rol_elems(vector4x_u32 v, unsigned int idx)
{
#ifndef WORDS_BIGENDIAN
  return vec_sld (v, v, (16 - (4 * idx)) & 15);
#else
  return vec_sld (v, v, (4 * idx) & 15);
#endif
}


static ASM_FUNC_ATTR_INLINE vector4x_u32
vec_merge_idx0_elems(vector4x_u32 v0, vector4x_u32 v1,
		     vector4x_u32 v2, vector4x_u32 v3)
{
  return (vector4x_u32)vec_mergeh ((vector2x_u64) vec_mergeh(v0, v1),
				   (vector2x_u64) vec_mergeh(v2, v3));
}


static ASM_FUNC_ATTR_INLINE vector4x_u32
vec_ror_u32(vector4x_u32 v, unsigned int shift)
{
  return (v >> (shift & 31)) ^ (v << ((32 - shift) & 31));
}


static ASM_FUNC_ATTR_INLINE vector4x_u32
vec_vshasigma_u32(vector4x_u32 v, unsigned int a, unsigned int b)
{
  asm ("vshasigmaw %0,%1,%2,%3"
       : "=v" (v)
       : "v" (v), "g" (a), "g" (b)
       : "memory");
  return v;
}


/* SHA2 round in vector registers */
#define R(a,b,c,d,e,f,g,h,k,w) do                             \
    {                                                         \
      t1  = (h);                                              \
      t1 += ((k) + (w));                                      \
      t1 += Cho((e),(f),(g));                                 \
      t1 += Sum1((e));                                        \
      t2  = Sum0((a));                                        \
      t2 += Maj((a),(b),(c));                                 \
      d  += t1;                                               \
      h   = t1 + t2;                                          \
    } while (0)

#define Cho(b, c, d)  (vec_sel(d, c, b))

#define Maj(c, d, b)  (vec_sel(c, b, c ^ d))

#define Sum0(x)       (vec_vshasigma_u32(x, 1, 0))

#define Sum1(x)       (vec_vshasigma_u32(x, 1, 15))


/* Message expansion on general purpose registers */
#define S0(x) (ror ((x), 7) ^ ror ((x), 18) ^ ((x) >> 3))
#define S1(x) (ror ((x), 17) ^ ror ((x), 19) ^ ((x) >> 10))

#define I(i) ( w[i] = buf_get_be32(data + i * 4) )
#define W(i) ({ w[i&0x0f] +=    w[(i-7) &0x0f];  \
		w[i&0x0f] += S0(w[(i-15)&0x0f]); \
		w[i&0x0f] += S1(w[(i-2) &0x0f]); \
		w[i&0x0f]; })

#define I2(i) ( w2[i] = buf_get_be32(64 + data + i * 4), I(i) )
#define W2(i) ({ w2[i]  = w2[i-7];       \
		 w2[i] += S1(w2[i-2]);   \
		 w2[i] += S0(w2[i-15]);  \
		 w2[i] += w2[i-16];      \
		 W(i); })
#define R2(i) ( w2[i] )


unsigned int ASM_FUNC_ATTR
_gcry_sha256_transform_ppc8(u32 state[8], const unsigned char *data,
			    size_t nblks)
{
  /* GPRs used for message expansion as vector intrinsics based generates
   * slower code. */
  vector4x_u32 h0, h1, h2, h3, h4, h5, h6, h7;
  vector4x_u32 h0_h3, h4_h7;
  vector4x_u32 a, b, c, d, e, f, g, h, t1, t2;
  u32 w[16];
  u32 w2[64];

  h0_h3 = vec_vsx_ld (4 * 0, state);
  h4_h7 = vec_vsx_ld (4 * 4, state);

  h0 = h0_h3;
  h1 = vec_rol_elems (h0_h3, 1);
  h2 = vec_rol_elems (h0_h3, 2);
  h3 = vec_rol_elems (h0_h3, 3);
  h4 = h4_h7;
  h5 = vec_rol_elems (h4_h7, 1);
  h6 = vec_rol_elems (h4_h7, 2);
  h7 = vec_rol_elems (h4_h7, 3);

  while (nblks >= 2)
    {
      a = h0;
      b = h1;
      c = h2;
      d = h3;
      e = h4;
      f = h5;
      g = h6;
      h = h7;

      R(a, b, c, d, e, f, g, h, K[0], I2(0));
      R(h, a, b, c, d, e, f, g, K[1], I2(1));
      R(g, h, a, b, c, d, e, f, K[2], I2(2));
      R(f, g, h, a, b, c, d, e, K[3], I2(3));
      R(e, f, g, h, a, b, c, d, K[4], I2(4));
      R(d, e, f, g, h, a, b, c, K[5], I2(5));
      R(c, d, e, f, g, h, a, b, K[6], I2(6));
      R(b, c, d, e, f, g, h, a, K[7], I2(7));
      R(a, b, c, d, e, f, g, h, K[8], I2(8));
      R(h, a, b, c, d, e, f, g, K[9], I2(9));
      R(g, h, a, b, c, d, e, f, K[10], I2(10));
      R(f, g, h, a, b, c, d, e, K[11], I2(11));
      R(e, f, g, h, a, b, c, d, K[12], I2(12));
      R(d, e, f, g, h, a, b, c, K[13], I2(13));
      R(c, d, e, f, g, h, a, b, K[14], I2(14));
      R(b, c, d, e, f, g, h, a, K[15], I2(15));
      data += 64 * 2;

      R(a, b, c, d, e, f, g, h, K[16], W2(16));
      R(h, a, b, c, d, e, f, g, K[17], W2(17));
      R(g, h, a, b, c, d, e, f, K[18], W2(18));
      R(f, g, h, a, b, c, d, e, K[19], W2(19));
      R(e, f, g, h, a, b, c, d, K[20], W2(20));
      R(d, e, f, g, h, a, b, c, K[21], W2(21));
      R(c, d, e, f, g, h, a, b, K[22], W2(22));
      R(b, c, d, e, f, g, h, a, K[23], W2(23));
      R(a, b, c, d, e, f, g, h, K[24], W2(24));
      R(h, a, b, c, d, e, f, g, K[25], W2(25));
      R(g, h, a, b, c, d, e, f, K[26], W2(26));
      R(f, g, h, a, b, c, d, e, K[27], W2(27));
      R(e, f, g, h, a, b, c, d, K[28], W2(28));
      R(d, e, f, g, h, a, b, c, K[29], W2(29));
      R(c, d, e, f, g, h, a, b, K[30], W2(30));
      R(b, c, d, e, f, g, h, a, K[31], W2(31));

      R(a, b, c, d, e, f, g, h, K[32], W2(32));
      R(h, a, b, c, d, e, f, g, K[33], W2(33));
      R(g, h, a, b, c, d, e, f, K[34], W2(34));
      R(f, g, h, a, b, c, d, e, K[35], W2(35));
      R(e, f, g, h, a, b, c, d, K[36], W2(36));
      R(d, e, f, g, h, a, b, c, K[37], W2(37));
      R(c, d, e, f, g, h, a, b, K[38], W2(38));
      R(b, c, d, e, f, g, h, a, K[39], W2(39));
      R(a, b, c, d, e, f, g, h, K[40], W2(40));
      R(h, a, b, c, d, e, f, g, K[41], W2(41));
      R(g, h, a, b, c, d, e, f, K[42], W2(42));
      R(f, g, h, a, b, c, d, e, K[43], W2(43));
      R(e, f, g, h, a, b, c, d, K[44], W2(44));
      R(d, e, f, g, h, a, b, c, K[45], W2(45));
      R(c, d, e, f, g, h, a, b, K[46], W2(46));
      R(b, c, d, e, f, g, h, a, K[47], W2(47));

      R(a, b, c, d, e, f, g, h, K[48], W2(48));
      R(h, a, b, c, d, e, f, g, K[49], W2(49));
      R(g, h, a, b, c, d, e, f, K[50], W2(50));
      R(f, g, h, a, b, c, d, e, K[51], W2(51));
      R(e, f, g, h, a, b, c, d, K[52], W2(52));
      R(d, e, f, g, h, a, b, c, K[53], W2(53));
      R(c, d, e, f, g, h, a, b, K[54], W2(54));
      R(b, c, d, e, f, g, h, a, K[55], W2(55));
      R(a, b, c, d, e, f, g, h, K[56], W2(56));
      R(h, a, b, c, d, e, f, g, K[57], W2(57));
      R(g, h, a, b, c, d, e, f, K[58], W2(58));
      R(f, g, h, a, b, c, d, e, K[59], W2(59));
      R(e, f, g, h, a, b, c, d, K[60], W2(60));
      R(d, e, f, g, h, a, b, c, K[61], W2(61));
      R(c, d, e, f, g, h, a, b, K[62], W2(62));
      R(b, c, d, e, f, g, h, a, K[63], W2(63));

      h0 += a;
      h1 += b;
      h2 += c;
      h3 += d;
      h4 += e;
      h5 += f;
      h6 += g;
      h7 += h;

      a = h0;
      b = h1;
      c = h2;
      d = h3;
      e = h4;
      f = h5;
      g = h6;
      h = h7;

      R(a, b, c, d, e, f, g, h, K[0], R2(0));
      R(h, a, b, c, d, e, f, g, K[1], R2(1));
      R(g, h, a, b, c, d, e, f, K[2], R2(2));
      R(f, g, h, a, b, c, d, e, K[3], R2(3));
      R(e, f, g, h, a, b, c, d, K[4], R2(4));
      R(d, e, f, g, h, a, b, c, K[5], R2(5));
      R(c, d, e, f, g, h, a, b, K[6], R2(6));
      R(b, c, d, e, f, g, h, a, K[7], R2(7));
      R(a, b, c, d, e, f, g, h, K[8], R2(8));
      R(h, a, b, c, d, e, f, g, K[9], R2(9));
      R(g, h, a, b, c, d, e, f, K[10], R2(10));
      R(f, g, h, a, b, c, d, e, K[11], R2(11));
      R(e, f, g, h, a, b, c, d, K[12], R2(12));
      R(d, e, f, g, h, a, b, c, K[13], R2(13));
      R(c, d, e, f, g, h, a, b, K[14], R2(14));
      R(b, c, d, e, f, g, h, a, K[15], R2(15));

      R(a, b, c, d, e, f, g, h, K[16], R2(16));
      R(h, a, b, c, d, e, f, g, K[17], R2(17));
      R(g, h, a, b, c, d, e, f, K[18], R2(18));
      R(f, g, h, a, b, c, d, e, K[19], R2(19));
      R(e, f, g, h, a, b, c, d, K[20], R2(20));
      R(d, e, f, g, h, a, b, c, K[21], R2(21));
      R(c, d, e, f, g, h, a, b, K[22], R2(22));
      R(b, c, d, e, f, g, h, a, K[23], R2(23));
      R(a, b, c, d, e, f, g, h, K[24], R2(24));
      R(h, a, b, c, d, e, f, g, K[25], R2(25));
      R(g, h, a, b, c, d, e, f, K[26], R2(26));
      R(f, g, h, a, b, c, d, e, K[27], R2(27));
      R(e, f, g, h, a, b, c, d, K[28], R2(28));
      R(d, e, f, g, h, a, b, c, K[29], R2(29));
      R(c, d, e, f, g, h, a, b, K[30], R2(30));
      R(b, c, d, e, f, g, h, a, K[31], R2(31));

      R(a, b, c, d, e, f, g, h, K[32], R2(32));
      R(h, a, b, c, d, e, f, g, K[33], R2(33));
      R(g, h, a, b, c, d, e, f, K[34], R2(34));
      R(f, g, h, a, b, c, d, e, K[35], R2(35));
      R(e, f, g, h, a, b, c, d, K[36], R2(36));
      R(d, e, f, g, h, a, b, c, K[37], R2(37));
      R(c, d, e, f, g, h, a, b, K[38], R2(38));
      R(b, c, d, e, f, g, h, a, K[39], R2(39));
      R(a, b, c, d, e, f, g, h, K[40], R2(40));
      R(h, a, b, c, d, e, f, g, K[41], R2(41));
      R(g, h, a, b, c, d, e, f, K[42], R2(42));
      R(f, g, h, a, b, c, d, e, K[43], R2(43));
      R(e, f, g, h, a, b, c, d, K[44], R2(44));
      R(d, e, f, g, h, a, b, c, K[45], R2(45));
      R(c, d, e, f, g, h, a, b, K[46], R2(46));
      R(b, c, d, e, f, g, h, a, K[47], R2(47));

      R(a, b, c, d, e, f, g, h, K[48], R2(48));
      R(h, a, b, c, d, e, f, g, K[49], R2(49));
      R(g, h, a, b, c, d, e, f, K[50], R2(50));
      R(f, g, h, a, b, c, d, e, K[51], R2(51));
      R(e, f, g, h, a, b, c, d, K[52], R2(52));
      R(d, e, f, g, h, a, b, c, K[53], R2(53));
      R(c, d, e, f, g, h, a, b, K[54], R2(54));
      R(b, c, d, e, f, g, h, a, K[55], R2(55));
      R(a, b, c, d, e, f, g, h, K[56], R2(56));
      R(h, a, b, c, d, e, f, g, K[57], R2(57));
      R(g, h, a, b, c, d, e, f, K[58], R2(58));
      R(f, g, h, a, b, c, d, e, K[59], R2(59));
      R(e, f, g, h, a, b, c, d, K[60], R2(60));
      R(d, e, f, g, h, a, b, c, K[61], R2(61));
      R(c, d, e, f, g, h, a, b, K[62], R2(62));
      R(b, c, d, e, f, g, h, a, K[63], R2(63));

      h0 += a;
      h1 += b;
      h2 += c;
      h3 += d;
      h4 += e;
      h5 += f;
      h6 += g;
      h7 += h;

      nblks -= 2;
    }

  while (nblks)
    {
      a = h0;
      b = h1;
      c = h2;
      d = h3;
      e = h4;
      f = h5;
      g = h6;
      h = h7;

      R(a, b, c, d, e, f, g, h, K[0], I(0));
      R(h, a, b, c, d, e, f, g, K[1], I(1));
      R(g, h, a, b, c, d, e, f, K[2], I(2));
      R(f, g, h, a, b, c, d, e, K[3], I(3));
      R(e, f, g, h, a, b, c, d, K[4], I(4));
      R(d, e, f, g, h, a, b, c, K[5], I(5));
      R(c, d, e, f, g, h, a, b, K[6], I(6));
      R(b, c, d, e, f, g, h, a, K[7], I(7));
      R(a, b, c, d, e, f, g, h, K[8], I(8));
      R(h, a, b, c, d, e, f, g, K[9], I(9));
      R(g, h, a, b, c, d, e, f, K[10], I(10));
      R(f, g, h, a, b, c, d, e, K[11], I(11));
      R(e, f, g, h, a, b, c, d, K[12], I(12));
      R(d, e, f, g, h, a, b, c, K[13], I(13));
      R(c, d, e, f, g, h, a, b, K[14], I(14));
      R(b, c, d, e, f, g, h, a, K[15], I(15));
      data += 64;

      R(a, b, c, d, e, f, g, h, K[16], W(16));
      R(h, a, b, c, d, e, f, g, K[17], W(17));
      R(g, h, a, b, c, d, e, f, K[18], W(18));
      R(f, g, h, a, b, c, d, e, K[19], W(19));
      R(e, f, g, h, a, b, c, d, K[20], W(20));
      R(d, e, f, g, h, a, b, c, K[21], W(21));
      R(c, d, e, f, g, h, a, b, K[22], W(22));
      R(b, c, d, e, f, g, h, a, K[23], W(23));
      R(a, b, c, d, e, f, g, h, K[24], W(24));
      R(h, a, b, c, d, e, f, g, K[25], W(25));
      R(g, h, a, b, c, d, e, f, K[26], W(26));
      R(f, g, h, a, b, c, d, e, K[27], W(27));
      R(e, f, g, h, a, b, c, d, K[28], W(28));
      R(d, e, f, g, h, a, b, c, K[29], W(29));
      R(c, d, e, f, g, h, a, b, K[30], W(30));
      R(b, c, d, e, f, g, h, a, K[31], W(31));

      R(a, b, c, d, e, f, g, h, K[32], W(32));
      R(h, a, b, c, d, e, f, g, K[33], W(33));
      R(g, h, a, b, c, d, e, f, K[34], W(34));
      R(f, g, h, a, b, c, d, e, K[35], W(35));
      R(e, f, g, h, a, b, c, d, K[36], W(36));
      R(d, e, f, g, h, a, b, c, K[37], W(37));
      R(c, d, e, f, g, h, a, b, K[38], W(38));
      R(b, c, d, e, f, g, h, a, K[39], W(39));
      R(a, b, c, d, e, f, g, h, K[40], W(40));
      R(h, a, b, c, d, e, f, g, K[41], W(41));
      R(g, h, a, b, c, d, e, f, K[42], W(42));
      R(f, g, h, a, b, c, d, e, K[43], W(43));
      R(e, f, g, h, a, b, c, d, K[44], W(44));
      R(d, e, f, g, h, a, b, c, K[45], W(45));
      R(c, d, e, f, g, h, a, b, K[46], W(46));
      R(b, c, d, e, f, g, h, a, K[47], W(47));

      R(a, b, c, d, e, f, g, h, K[48], W(48));
      R(h, a, b, c, d, e, f, g, K[49], W(49));
      R(g, h, a, b, c, d, e, f, K[50], W(50));
      R(f, g, h, a, b, c, d, e, K[51], W(51));
      R(e, f, g, h, a, b, c, d, K[52], W(52));
      R(d, e, f, g, h, a, b, c, K[53], W(53));
      R(c, d, e, f, g, h, a, b, K[54], W(54));
      R(b, c, d, e, f, g, h, a, K[55], W(55));
      R(a, b, c, d, e, f, g, h, K[56], W(56));
      R(h, a, b, c, d, e, f, g, K[57], W(57));
      R(g, h, a, b, c, d, e, f, K[58], W(58));
      R(f, g, h, a, b, c, d, e, K[59], W(59));
      R(e, f, g, h, a, b, c, d, K[60], W(60));
      R(d, e, f, g, h, a, b, c, K[61], W(61));
      R(c, d, e, f, g, h, a, b, K[62], W(62));
      R(b, c, d, e, f, g, h, a, K[63], W(63));

      h0 += a;
      h1 += b;
      h2 += c;
      h3 += d;
      h4 += e;
      h5 += f;
      h6 += g;
      h7 += h;

      nblks--;
    }

  h0_h3 = vec_merge_idx0_elems (h0, h1, h2, h3);
  h4_h7 = vec_merge_idx0_elems (h4, h5, h6, h7);
  vec_vsx_st (h0_h3, 4 * 0, state);
  vec_vsx_st (h4_h7, 4 * 4, state);

  return sizeof(w2) + sizeof(w);
}
#undef R
#undef Cho
#undef Maj
#undef Sum0
#undef Sum1
#undef S0
#undef S1
#undef I
#undef W
#undef I2
#undef W2
#undef R2


/* SHA2 round in general purpose registers */
#define R(a,b,c,d,e,f,g,h,k,w) do                                 \
          {                                                       \
            t1 = (h) + Sum1((e)) + Cho((e),(f),(g)) + ((k) + (w));\
            t2 = Sum0((a)) + Maj((a),(b),(c));                    \
            d += t1;                                              \
            h  = t1 + t2;                                         \
          } while (0)

#define Cho(x, y, z)  ((x & y) + (~x & z))

#define Maj(z, x, y)  ((x & y) + (z & (x ^ y)))

#define Sum0(x)       (ror (x, 2) ^ ror (x ^ ror (x, 22-13), 13))

#define Sum1(x)       (ror (x, 6) ^ ror (x, 11) ^ ror (x, 25))


/* Message expansion on general purpose registers */
#define S0(x) (ror ((x), 7) ^ ror ((x), 18) ^ ((x) >> 3))
#define S1(x) (ror ((x), 17) ^ ror ((x), 19) ^ ((x) >> 10))

#define I(i) ( w[i] = buf_get_be32(data + i * 4) )
#define WN(i) ({ w[i&0x0f] +=    w[(i-7) &0x0f];  \
		 w[i&0x0f] += S0(w[(i-15)&0x0f]); \
		 w[i&0x0f] += S1(w[(i-2) &0x0f]); \
		 w[i&0x0f]; })
#define W(i) ({ u32 r = w[i&0x0f]; WN(i); r; })
#define L(i) w[i&0x0f]


unsigned int ASM_FUNC_ATTR
_gcry_sha256_transform_ppc9(u32 state[8], const unsigned char *data,
			    size_t nblks)
{
  /* GPRs used for round function and message expansion as vector intrinsics
   * based generates slower code for POWER9. */
  u32 a, b, c, d, e, f, g, h, t1, t2;
  u32 w[16];

  a = state[0];
  b = state[1];
  c = state[2];
  d = state[3];
  e = state[4];
  f = state[5];
  g = state[6];
  h = state[7];

  while (nblks >= 2)
    {
      I(0); I(1); I(2); I(3);
      I(4); I(5); I(6); I(7);
      I(8); I(9); I(10); I(11);
      I(12); I(13); I(14); I(15);
      data += 64;
      R(a, b, c, d, e, f, g, h, K[0], W(0));
      R(h, a, b, c, d, e, f, g, K[1], W(1));
      R(g, h, a, b, c, d, e, f, K[2], W(2));
      R(f, g, h, a, b, c, d, e, K[3], W(3));
      R(e, f, g, h, a, b, c, d, K[4], W(4));
      R(d, e, f, g, h, a, b, c, K[5], W(5));
      R(c, d, e, f, g, h, a, b, K[6], W(6));
      R(b, c, d, e, f, g, h, a, K[7], W(7));
      R(a, b, c, d, e, f, g, h, K[8], W(8));
      R(h, a, b, c, d, e, f, g, K[9], W(9));
      R(g, h, a, b, c, d, e, f, K[10], W(10));
      R(f, g, h, a, b, c, d, e, K[11], W(11));
      R(e, f, g, h, a, b, c, d, K[12], W(12));
      R(d, e, f, g, h, a, b, c, K[13], W(13));
      R(c, d, e, f, g, h, a, b, K[14], W(14));
      R(b, c, d, e, f, g, h, a, K[15], W(15));

      R(a, b, c, d, e, f, g, h, K[16], W(16));
      R(h, a, b, c, d, e, f, g, K[17], W(17));
      R(g, h, a, b, c, d, e, f, K[18], W(18));
      R(f, g, h, a, b, c, d, e, K[19], W(19));
      R(e, f, g, h, a, b, c, d, K[20], W(20));
      R(d, e, f, g, h, a, b, c, K[21], W(21));
      R(c, d, e, f, g, h, a, b, K[22], W(22));
      R(b, c, d, e, f, g, h, a, K[23], W(23));
      R(a, b, c, d, e, f, g, h, K[24], W(24));
      R(h, a, b, c, d, e, f, g, K[25], W(25));
      R(g, h, a, b, c, d, e, f, K[26], W(26));
      R(f, g, h, a, b, c, d, e, K[27], W(27));
      R(e, f, g, h, a, b, c, d, K[28], W(28));
      R(d, e, f, g, h, a, b, c, K[29], W(29));
      R(c, d, e, f, g, h, a, b, K[30], W(30));
      R(b, c, d, e, f, g, h, a, K[31], W(31));

      R(a, b, c, d, e, f, g, h, K[32], W(32));
      R(h, a, b, c, d, e, f, g, K[33], W(33));
      R(g, h, a, b, c, d, e, f, K[34], W(34));
      R(f, g, h, a, b, c, d, e, K[35], W(35));
      R(e, f, g, h, a, b, c, d, K[36], W(36));
      R(d, e, f, g, h, a, b, c, K[37], W(37));
      R(c, d, e, f, g, h, a, b, K[38], W(38));
      R(b, c, d, e, f, g, h, a, K[39], W(39));
      R(a, b, c, d, e, f, g, h, K[40], W(40));
      R(h, a, b, c, d, e, f, g, K[41], W(41));
      R(g, h, a, b, c, d, e, f, K[42], W(42));
      R(f, g, h, a, b, c, d, e, K[43], W(43));
      R(e, f, g, h, a, b, c, d, K[44], W(44));
      R(d, e, f, g, h, a, b, c, K[45], W(45));
      R(c, d, e, f, g, h, a, b, K[46], W(46));
      R(b, c, d, e, f, g, h, a, K[47], W(47));

      R(a, b, c, d, e, f, g, h, K[48], L(48));
      R(h, a, b, c, d, e, f, g, K[49], L(49));
      R(g, h, a, b, c, d, e, f, K[50], L(50));
      R(f, g, h, a, b, c, d, e, K[51], L(51));
      I(0); I(1); I(2); I(3);
      R(e, f, g, h, a, b, c, d, K[52], L(52));
      R(d, e, f, g, h, a, b, c, K[53], L(53));
      R(c, d, e, f, g, h, a, b, K[54], L(54));
      R(b, c, d, e, f, g, h, a, K[55], L(55));
      I(4); I(5); I(6); I(7);
      R(a, b, c, d, e, f, g, h, K[56], L(56));
      R(h, a, b, c, d, e, f, g, K[57], L(57));
      R(g, h, a, b, c, d, e, f, K[58], L(58));
      R(f, g, h, a, b, c, d, e, K[59], L(59));
      I(8); I(9); I(10); I(11);
      R(e, f, g, h, a, b, c, d, K[60], L(60));
      R(d, e, f, g, h, a, b, c, K[61], L(61));
      R(c, d, e, f, g, h, a, b, K[62], L(62));
      R(b, c, d, e, f, g, h, a, K[63], L(63));
      I(12); I(13); I(14); I(15);
      data += 64;

      a += state[0];
      b += state[1];
      c += state[2];
      d += state[3];
      e += state[4];
      f += state[5];
      g += state[6];
      h += state[7];
      state[0] = a;
      state[1] = b;
      state[2] = c;
      state[3] = d;
      state[4] = e;
      state[5] = f;
      state[6] = g;
      state[7] = h;

      R(a, b, c, d, e, f, g, h, K[0], W(0));
      R(h, a, b, c, d, e, f, g, K[1], W(1));
      R(g, h, a, b, c, d, e, f, K[2], W(2));
      R(f, g, h, a, b, c, d, e, K[3], W(3));
      R(e, f, g, h, a, b, c, d, K[4], W(4));
      R(d, e, f, g, h, a, b, c, K[5], W(5));
      R(c, d, e, f, g, h, a, b, K[6], W(6));
      R(b, c, d, e, f, g, h, a, K[7], W(7));
      R(a, b, c, d, e, f, g, h, K[8], W(8));
      R(h, a, b, c, d, e, f, g, K[9], W(9));
      R(g, h, a, b, c, d, e, f, K[10], W(10));
      R(f, g, h, a, b, c, d, e, K[11], W(11));
      R(e, f, g, h, a, b, c, d, K[12], W(12));
      R(d, e, f, g, h, a, b, c, K[13], W(13));
      R(c, d, e, f, g, h, a, b, K[14], W(14));
      R(b, c, d, e, f, g, h, a, K[15], W(15));

      R(a, b, c, d, e, f, g, h, K[16], W(16));
      R(h, a, b, c, d, e, f, g, K[17], W(17));
      R(g, h, a, b, c, d, e, f, K[18], W(18));
      R(f, g, h, a, b, c, d, e, K[19], W(19));
      R(e, f, g, h, a, b, c, d, K[20], W(20));
      R(d, e, f, g, h, a, b, c, K[21], W(21));
      R(c, d, e, f, g, h, a, b, K[22], W(22));
      R(b, c, d, e, f, g, h, a, K[23], W(23));
      R(a, b, c, d, e, f, g, h, K[24], W(24));
      R(h, a, b, c, d, e, f, g, K[25], W(25));
      R(g, h, a, b, c, d, e, f, K[26], W(26));
      R(f, g, h, a, b, c, d, e, K[27], W(27));
      R(e, f, g, h, a, b, c, d, K[28], W(28));
      R(d, e, f, g, h, a, b, c, K[29], W(29));
      R(c, d, e, f, g, h, a, b, K[30], W(30));
      R(b, c, d, e, f, g, h, a, K[31], W(31));

      R(a, b, c, d, e, f, g, h, K[32], W(32));
      R(h, a, b, c, d, e, f, g, K[33], W(33));
      R(g, h, a, b, c, d, e, f, K[34], W(34));
      R(f, g, h, a, b, c, d, e, K[35], W(35));
      R(e, f, g, h, a, b, c, d, K[36], W(36));
      R(d, e, f, g, h, a, b, c, K[37], W(37));
      R(c, d, e, f, g, h, a, b, K[38], W(38));
      R(b, c, d, e, f, g, h, a, K[39], W(39));
      R(a, b, c, d, e, f, g, h, K[40], W(40));
      R(h, a, b, c, d, e, f, g, K[41], W(41));
      R(g, h, a, b, c, d, e, f, K[42], W(42));
      R(f, g, h, a, b, c, d, e, K[43], W(43));
      R(e, f, g, h, a, b, c, d, K[44], W(44));
      R(d, e, f, g, h, a, b, c, K[45], W(45));
      R(c, d, e, f, g, h, a, b, K[46], W(46));
      R(b, c, d, e, f, g, h, a, K[47], W(47));

      R(a, b, c, d, e, f, g, h, K[48], L(48));
      R(h, a, b, c, d, e, f, g, K[49], L(49));
      R(g, h, a, b, c, d, e, f, K[50], L(50));
      R(f, g, h, a, b, c, d, e, K[51], L(51));
      R(e, f, g, h, a, b, c, d, K[52], L(52));
      R(d, e, f, g, h, a, b, c, K[53], L(53));
      R(c, d, e, f, g, h, a, b, K[54], L(54));
      R(b, c, d, e, f, g, h, a, K[55], L(55));
      R(a, b, c, d, e, f, g, h, K[56], L(56));
      R(h, a, b, c, d, e, f, g, K[57], L(57));
      R(g, h, a, b, c, d, e, f, K[58], L(58));
      R(f, g, h, a, b, c, d, e, K[59], L(59));
      R(e, f, g, h, a, b, c, d, K[60], L(60));
      R(d, e, f, g, h, a, b, c, K[61], L(61));
      R(c, d, e, f, g, h, a, b, K[62], L(62));
      R(b, c, d, e, f, g, h, a, K[63], L(63));

      a += state[0];
      b += state[1];
      c += state[2];
      d += state[3];
      e += state[4];
      f += state[5];
      g += state[6];
      h += state[7];
      state[0] = a;
      state[1] = b;
      state[2] = c;
      state[3] = d;
      state[4] = e;
      state[5] = f;
      state[6] = g;
      state[7] = h;

      nblks -= 2;
    }

  while (nblks)
    {
      I(0); I(1); I(2); I(3);
      I(4); I(5); I(6); I(7);
      I(8); I(9); I(10); I(11);
      I(12); I(13); I(14); I(15);
      data += 64;
      R(a, b, c, d, e, f, g, h, K[0], W(0));
      R(h, a, b, c, d, e, f, g, K[1], W(1));
      R(g, h, a, b, c, d, e, f, K[2], W(2));
      R(f, g, h, a, b, c, d, e, K[3], W(3));
      R(e, f, g, h, a, b, c, d, K[4], W(4));
      R(d, e, f, g, h, a, b, c, K[5], W(5));
      R(c, d, e, f, g, h, a, b, K[6], W(6));
      R(b, c, d, e, f, g, h, a, K[7], W(7));
      R(a, b, c, d, e, f, g, h, K[8], W(8));
      R(h, a, b, c, d, e, f, g, K[9], W(9));
      R(g, h, a, b, c, d, e, f, K[10], W(10));
      R(f, g, h, a, b, c, d, e, K[11], W(11));
      R(e, f, g, h, a, b, c, d, K[12], W(12));
      R(d, e, f, g, h, a, b, c, K[13], W(13));
      R(c, d, e, f, g, h, a, b, K[14], W(14));
      R(b, c, d, e, f, g, h, a, K[15], W(15));

      R(a, b, c, d, e, f, g, h, K[16], W(16));
      R(h, a, b, c, d, e, f, g, K[17], W(17));
      R(g, h, a, b, c, d, e, f, K[18], W(18));
      R(f, g, h, a, b, c, d, e, K[19], W(19));
      R(e, f, g, h, a, b, c, d, K[20], W(20));
      R(d, e, f, g, h, a, b, c, K[21], W(21));
      R(c, d, e, f, g, h, a, b, K[22], W(22));
      R(b, c, d, e, f, g, h, a, K[23], W(23));
      R(a, b, c, d, e, f, g, h, K[24], W(24));
      R(h, a, b, c, d, e, f, g, K[25], W(25));
      R(g, h, a, b, c, d, e, f, K[26], W(26));
      R(f, g, h, a, b, c, d, e, K[27], W(27));
      R(e, f, g, h, a, b, c, d, K[28], W(28));
      R(d, e, f, g, h, a, b, c, K[29], W(29));
      R(c, d, e, f, g, h, a, b, K[30], W(30));
      R(b, c, d, e, f, g, h, a, K[31], W(31));

      R(a, b, c, d, e, f, g, h, K[32], W(32));
      R(h, a, b, c, d, e, f, g, K[33], W(33));
      R(g, h, a, b, c, d, e, f, K[34], W(34));
      R(f, g, h, a, b, c, d, e, K[35], W(35));
      R(e, f, g, h, a, b, c, d, K[36], W(36));
      R(d, e, f, g, h, a, b, c, K[37], W(37));
      R(c, d, e, f, g, h, a, b, K[38], W(38));
      R(b, c, d, e, f, g, h, a, K[39], W(39));
      R(a, b, c, d, e, f, g, h, K[40], W(40));
      R(h, a, b, c, d, e, f, g, K[41], W(41));
      R(g, h, a, b, c, d, e, f, K[42], W(42));
      R(f, g, h, a, b, c, d, e, K[43], W(43));
      R(e, f, g, h, a, b, c, d, K[44], W(44));
      R(d, e, f, g, h, a, b, c, K[45], W(45));
      R(c, d, e, f, g, h, a, b, K[46], W(46));
      R(b, c, d, e, f, g, h, a, K[47], W(47));

      R(a, b, c, d, e, f, g, h, K[48], L(48));
      R(h, a, b, c, d, e, f, g, K[49], L(49));
      R(g, h, a, b, c, d, e, f, K[50], L(50));
      R(f, g, h, a, b, c, d, e, K[51], L(51));
      R(e, f, g, h, a, b, c, d, K[52], L(52));
      R(d, e, f, g, h, a, b, c, K[53], L(53));
      R(c, d, e, f, g, h, a, b, K[54], L(54));
      R(b, c, d, e, f, g, h, a, K[55], L(55));
      R(a, b, c, d, e, f, g, h, K[56], L(56));
      R(h, a, b, c, d, e, f, g, K[57], L(57));
      R(g, h, a, b, c, d, e, f, K[58], L(58));
      R(f, g, h, a, b, c, d, e, K[59], L(59));
      R(e, f, g, h, a, b, c, d, K[60], L(60));
      R(d, e, f, g, h, a, b, c, K[61], L(61));
      R(c, d, e, f, g, h, a, b, K[62], L(62));
      R(b, c, d, e, f, g, h, a, K[63], L(63));

      a += state[0];
      b += state[1];
      c += state[2];
      d += state[3];
      e += state[4];
      f += state[5];
      g += state[6];
      h += state[7];
      state[0] = a;
      state[1] = b;
      state[2] = c;
      state[3] = d;
      state[4] = e;
      state[5] = f;
      state[6] = g;
      state[7] = h;

      nblks--;
    }

  return sizeof(w);
}

#endif /* ENABLE_PPC_CRYPTO_SUPPORT */
