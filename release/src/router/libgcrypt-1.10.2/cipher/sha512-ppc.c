/* sha512-ppc.c - PowerPC vcrypto implementation of SHA-512 transform
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
    defined(USE_SHA512) && \
    __GNUC__ >= 4

#include <altivec.h>
#include "bufhelp.h"


typedef vector unsigned char vector16x_u8;
typedef vector unsigned long long vector2x_u64;


#define ALWAYS_INLINE inline __attribute__((always_inline))
#define NO_INLINE __attribute__((noinline))
#define NO_INSTRUMENT_FUNCTION __attribute__((no_instrument_function))

#define ASM_FUNC_ATTR          NO_INSTRUMENT_FUNCTION
#define ASM_FUNC_ATTR_INLINE   ASM_FUNC_ATTR ALWAYS_INLINE
#define ASM_FUNC_ATTR_NOINLINE ASM_FUNC_ATTR NO_INLINE


static const u64 K[80] =
  {
    U64_C(0x428a2f98d728ae22), U64_C(0x7137449123ef65cd),
    U64_C(0xb5c0fbcfec4d3b2f), U64_C(0xe9b5dba58189dbbc),
    U64_C(0x3956c25bf348b538), U64_C(0x59f111f1b605d019),
    U64_C(0x923f82a4af194f9b), U64_C(0xab1c5ed5da6d8118),
    U64_C(0xd807aa98a3030242), U64_C(0x12835b0145706fbe),
    U64_C(0x243185be4ee4b28c), U64_C(0x550c7dc3d5ffb4e2),
    U64_C(0x72be5d74f27b896f), U64_C(0x80deb1fe3b1696b1),
    U64_C(0x9bdc06a725c71235), U64_C(0xc19bf174cf692694),
    U64_C(0xe49b69c19ef14ad2), U64_C(0xefbe4786384f25e3),
    U64_C(0x0fc19dc68b8cd5b5), U64_C(0x240ca1cc77ac9c65),
    U64_C(0x2de92c6f592b0275), U64_C(0x4a7484aa6ea6e483),
    U64_C(0x5cb0a9dcbd41fbd4), U64_C(0x76f988da831153b5),
    U64_C(0x983e5152ee66dfab), U64_C(0xa831c66d2db43210),
    U64_C(0xb00327c898fb213f), U64_C(0xbf597fc7beef0ee4),
    U64_C(0xc6e00bf33da88fc2), U64_C(0xd5a79147930aa725),
    U64_C(0x06ca6351e003826f), U64_C(0x142929670a0e6e70),
    U64_C(0x27b70a8546d22ffc), U64_C(0x2e1b21385c26c926),
    U64_C(0x4d2c6dfc5ac42aed), U64_C(0x53380d139d95b3df),
    U64_C(0x650a73548baf63de), U64_C(0x766a0abb3c77b2a8),
    U64_C(0x81c2c92e47edaee6), U64_C(0x92722c851482353b),
    U64_C(0xa2bfe8a14cf10364), U64_C(0xa81a664bbc423001),
    U64_C(0xc24b8b70d0f89791), U64_C(0xc76c51a30654be30),
    U64_C(0xd192e819d6ef5218), U64_C(0xd69906245565a910),
    U64_C(0xf40e35855771202a), U64_C(0x106aa07032bbd1b8),
    U64_C(0x19a4c116b8d2d0c8), U64_C(0x1e376c085141ab53),
    U64_C(0x2748774cdf8eeb99), U64_C(0x34b0bcb5e19b48a8),
    U64_C(0x391c0cb3c5c95a63), U64_C(0x4ed8aa4ae3418acb),
    U64_C(0x5b9cca4f7763e373), U64_C(0x682e6ff3d6b2b8a3),
    U64_C(0x748f82ee5defb2fc), U64_C(0x78a5636f43172f60),
    U64_C(0x84c87814a1f0ab72), U64_C(0x8cc702081a6439ec),
    U64_C(0x90befffa23631e28), U64_C(0xa4506cebde82bde9),
    U64_C(0xbef9a3f7b2c67915), U64_C(0xc67178f2e372532b),
    U64_C(0xca273eceea26619c), U64_C(0xd186b8c721c0c207),
    U64_C(0xeada7dd6cde0eb1e), U64_C(0xf57d4f7fee6ed178),
    U64_C(0x06f067aa72176fba), U64_C(0x0a637dc5a2c898a6),
    U64_C(0x113f9804bef90dae), U64_C(0x1b710b35131c471b),
    U64_C(0x28db77f523047d84), U64_C(0x32caab7b40c72493),
    U64_C(0x3c9ebe0a15c9bebc), U64_C(0x431d67c49c100d4c),
    U64_C(0x4cc5d4becb3e42b6), U64_C(0x597f299cfc657e2a),
    U64_C(0x5fcb6fab3ad6faec), U64_C(0x6c44198c4a475817)
  };


static ASM_FUNC_ATTR_INLINE u64
ror64 (u64 v, u64 shift)
{
  return (v >> (shift & 63)) ^ (v << ((64 - shift) & 63));
}


static ASM_FUNC_ATTR_INLINE vector2x_u64
vec_rol_elems(vector2x_u64 v, unsigned int idx)
{
#ifndef WORDS_BIGENDIAN
  return vec_sld (v, v, (16 - (8 * idx)) & 15);
#else
  return vec_sld (v, v, (8 * idx) & 15);
#endif
}


static ASM_FUNC_ATTR_INLINE vector2x_u64
vec_merge_idx0_elems(vector2x_u64 v0, vector2x_u64 v1)
{
  return vec_mergeh (v0, v1);
}


static ASM_FUNC_ATTR_INLINE vector2x_u64
vec_vshasigma_u64(vector2x_u64 v, unsigned int a, unsigned int b)
{
  __asm__ ("vshasigmad %0,%1,%2,%3"
	   : "=v" (v)
	   : "v" (v), "g" (a), "g" (b)
	   : "memory");
  return v;
}


static ASM_FUNC_ATTR_INLINE vector2x_u64
vec_u64_load(unsigned long offset, const void *ptr)
{
  vector2x_u64 vecu64;
#if __GNUC__ >= 4
  if (__builtin_constant_p (offset) && offset == 0)
    __asm__ ("lxvd2x %x0,0,%1\n\t"
	     : "=wa" (vecu64)
	     : "r" ((uintptr_t)ptr)
	     : "memory");
  else
#endif
    __asm__ ("lxvd2x %x0,%1,%2\n\t"
	     : "=wa" (vecu64)
	     : "r" (offset), "r" ((uintptr_t)ptr)
	     : "memory", "r0");
#ifndef WORDS_BIGENDIAN
  __asm__ ("xxswapd %x0, %x1"
	   : "=wa" (vecu64)
	   : "wa" (vecu64));
#endif
  return vecu64;
}


static ASM_FUNC_ATTR_INLINE void
vec_u64_store(vector2x_u64 vecu64, unsigned long offset, void *ptr)
{
#ifndef WORDS_BIGENDIAN
  __asm__ ("xxswapd %x0, %x1"
	   : "=wa" (vecu64)
	   : "wa" (vecu64));
#endif
#if __GNUC__ >= 4
  if (__builtin_constant_p (offset) && offset == 0)
    __asm__ ("stxvd2x %x0,0,%1\n\t"
	     :
	     : "wa" (vecu64), "r" ((uintptr_t)ptr)
	     : "memory");
  else
#endif
    __asm__ ("stxvd2x %x0,%1,%2\n\t"
	     :
	     : "wa" (vecu64), "r" (offset), "r" ((uintptr_t)ptr)
	     : "memory", "r0");
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

#define Sum0(x)       (vec_vshasigma_u64(x, 1, 0))

#define Sum1(x)       (vec_vshasigma_u64(x, 1, 15))


/* Message expansion on general purpose registers */
#define S0(x) (ror64 ((x), 1) ^ ror64 ((x), 8) ^ ((x) >> 7))
#define S1(x) (ror64 ((x), 19) ^ ror64 ((x), 61) ^ ((x) >> 6))

#define I(i) ( w[i] = buf_get_be64(data + i * 8) )
#define WN(i) ({ w[i&0x0f] +=    w[(i-7) &0x0f];  \
		 w[i&0x0f] += S0(w[(i-15)&0x0f]); \
		 w[i&0x0f] += S1(w[(i-2) &0x0f]); \
		 w[i&0x0f]; })
#define W(i) ({ u64 r = w[i&0x0f]; WN(i); r; })
#define L(i) w[i&0x0f]


unsigned int ASM_FUNC_ATTR
_gcry_sha512_transform_ppc8(u64 state[8],
			    const unsigned char *data, size_t nblks)
{
  /* GPRs used for message expansion as vector intrinsics based generates
   * slower code. */
  vector2x_u64 h0, h1, h2, h3, h4, h5, h6, h7;
  vector2x_u64 a, b, c, d, e, f, g, h, t1, t2;
  u64 w[16];

  h0 = vec_u64_load (8 * 0, (unsigned long long *)state);
  h1 = vec_rol_elems (h0, 1);
  h2 = vec_u64_load (8 * 2, (unsigned long long *)state);
  h3 = vec_rol_elems (h2, 1);
  h4 = vec_u64_load (8 * 4, (unsigned long long *)state);
  h5 = vec_rol_elems (h4, 1);
  h6 = vec_u64_load (8 * 6, (unsigned long long *)state);
  h7 = vec_rol_elems (h6, 1);

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

      I(0); I(1); I(2); I(3);
      I(4); I(5); I(6); I(7);
      I(8); I(9); I(10); I(11);
      I(12); I(13); I(14); I(15);
      data += 128;
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

      R(a, b, c, d, e, f, g, h, K[64], L(64));
      R(h, a, b, c, d, e, f, g, K[65], L(65));
      R(g, h, a, b, c, d, e, f, K[66], L(66));
      R(f, g, h, a, b, c, d, e, K[67], L(67));
      I(0); I(1); I(2); I(3);
      R(e, f, g, h, a, b, c, d, K[68], L(68));
      R(d, e, f, g, h, a, b, c, K[69], L(69));
      R(c, d, e, f, g, h, a, b, K[70], L(70));
      R(b, c, d, e, f, g, h, a, K[71], L(71));
      I(4); I(5); I(6); I(7);
      R(a, b, c, d, e, f, g, h, K[72], L(72));
      R(h, a, b, c, d, e, f, g, K[73], L(73));
      R(g, h, a, b, c, d, e, f, K[74], L(74));
      R(f, g, h, a, b, c, d, e, K[75], L(75));
      I(8); I(9); I(10); I(11);
      R(e, f, g, h, a, b, c, d, K[76], L(76));
      R(d, e, f, g, h, a, b, c, K[77], L(77));
      R(c, d, e, f, g, h, a, b, K[78], L(78));
      R(b, c, d, e, f, g, h, a, K[79], L(79));
      I(12); I(13); I(14); I(15);
      data += 128;

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

      R(a, b, c, d, e, f, g, h, K[64], L(64));
      R(h, a, b, c, d, e, f, g, K[65], L(65));
      R(g, h, a, b, c, d, e, f, K[66], L(66));
      R(f, g, h, a, b, c, d, e, K[67], L(67));
      R(e, f, g, h, a, b, c, d, K[68], L(68));
      R(d, e, f, g, h, a, b, c, K[69], L(69));
      R(c, d, e, f, g, h, a, b, K[70], L(70));
      R(b, c, d, e, f, g, h, a, K[71], L(71));
      R(a, b, c, d, e, f, g, h, K[72], L(72));
      R(h, a, b, c, d, e, f, g, K[73], L(73));
      R(g, h, a, b, c, d, e, f, K[74], L(74));
      R(f, g, h, a, b, c, d, e, K[75], L(75));
      R(e, f, g, h, a, b, c, d, K[76], L(76));
      R(d, e, f, g, h, a, b, c, K[77], L(77));
      R(c, d, e, f, g, h, a, b, K[78], L(78));
      R(b, c, d, e, f, g, h, a, K[79], L(79));

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

      I(0); I(1); I(2); I(3);
      I(4); I(5); I(6); I(7);
      I(8); I(9); I(10); I(11);
      I(12); I(13); I(14); I(15);
      data += 128;
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

      R(a, b, c, d, e, f, g, h, K[64], L(64));
      R(h, a, b, c, d, e, f, g, K[65], L(65));
      R(g, h, a, b, c, d, e, f, K[66], L(66));
      R(f, g, h, a, b, c, d, e, K[67], L(67));
      R(e, f, g, h, a, b, c, d, K[68], L(68));
      R(d, e, f, g, h, a, b, c, K[69], L(69));
      R(c, d, e, f, g, h, a, b, K[70], L(70));
      R(b, c, d, e, f, g, h, a, K[71], L(71));
      R(a, b, c, d, e, f, g, h, K[72], L(72));
      R(h, a, b, c, d, e, f, g, K[73], L(73));
      R(g, h, a, b, c, d, e, f, K[74], L(74));
      R(f, g, h, a, b, c, d, e, K[75], L(75));
      R(e, f, g, h, a, b, c, d, K[76], L(76));
      R(d, e, f, g, h, a, b, c, K[77], L(77));
      R(c, d, e, f, g, h, a, b, K[78], L(78));
      R(b, c, d, e, f, g, h, a, K[79], L(79));

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

  h0 = vec_merge_idx0_elems (h0, h1);
  h2 = vec_merge_idx0_elems (h2, h3);
  h4 = vec_merge_idx0_elems (h4, h5);
  h6 = vec_merge_idx0_elems (h6, h7);
  vec_u64_store (h0, 8 * 0, (unsigned long long *)state);
  vec_u64_store (h2, 8 * 2, (unsigned long long *)state);
  vec_u64_store (h4, 8 * 4, (unsigned long long *)state);
  vec_u64_store (h6, 8 * 6, (unsigned long long *)state);

  return sizeof(w);
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

#define Sum0(x)       (ror64(x, 28) ^ ror64(x ^ ror64(x, 39-34), 34))

#define Sum1(x)       (ror64(x, 14) ^ ror64(x, 18) ^ ror64(x, 41))


/* Message expansion on general purpose registers */
#define S0(x) (ror64 ((x), 1) ^ ror64 ((x), 8) ^ ((x) >> 7))
#define S1(x) (ror64 ((x), 19) ^ ror64 ((x), 61) ^ ((x) >> 6))

#define I(i) ( w[i] = buf_get_be64(data + i * 8) )
#define WN(i) ({ w[i&0x0f] +=    w[(i-7) &0x0f];  \
		 w[i&0x0f] += S0(w[(i-15)&0x0f]); \
		 w[i&0x0f] += S1(w[(i-2) &0x0f]); \
		 w[i&0x0f]; })
#define W(i) ({ u64 r = w[i&0x0f]; WN(i); r; })
#define L(i) w[i&0x0f]


unsigned int ASM_FUNC_ATTR
_gcry_sha512_transform_ppc9(u64 state[8], const unsigned char *data,
			    size_t nblks)
{
  /* GPRs used for round function and message expansion as vector intrinsics
   * based generates slower code for POWER9. */
  u64 a, b, c, d, e, f, g, h, t1, t2;
  u64 w[16];

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
      data += 128;
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

      R(a, b, c, d, e, f, g, h, K[64], L(64));
      R(h, a, b, c, d, e, f, g, K[65], L(65));
      R(g, h, a, b, c, d, e, f, K[66], L(66));
      R(f, g, h, a, b, c, d, e, K[67], L(67));
      I(0); I(1); I(2); I(3);
      R(e, f, g, h, a, b, c, d, K[68], L(68));
      R(d, e, f, g, h, a, b, c, K[69], L(69));
      R(c, d, e, f, g, h, a, b, K[70], L(70));
      R(b, c, d, e, f, g, h, a, K[71], L(71));
      I(4); I(5); I(6); I(7);
      R(a, b, c, d, e, f, g, h, K[72], L(72));
      R(h, a, b, c, d, e, f, g, K[73], L(73));
      R(g, h, a, b, c, d, e, f, K[74], L(74));
      R(f, g, h, a, b, c, d, e, K[75], L(75));
      I(8); I(9); I(10); I(11);
      R(e, f, g, h, a, b, c, d, K[76], L(76));
      R(d, e, f, g, h, a, b, c, K[77], L(77));
      R(c, d, e, f, g, h, a, b, K[78], L(78));
      R(b, c, d, e, f, g, h, a, K[79], L(79));
      I(12); I(13); I(14); I(15);
      data += 128;

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

      R(a, b, c, d, e, f, g, h, K[64], L(64));
      R(h, a, b, c, d, e, f, g, K[65], L(65));
      R(g, h, a, b, c, d, e, f, K[66], L(66));
      R(f, g, h, a, b, c, d, e, K[67], L(67));
      R(e, f, g, h, a, b, c, d, K[68], L(68));
      R(d, e, f, g, h, a, b, c, K[69], L(69));
      R(c, d, e, f, g, h, a, b, K[70], L(70));
      R(b, c, d, e, f, g, h, a, K[71], L(71));
      R(a, b, c, d, e, f, g, h, K[72], L(72));
      R(h, a, b, c, d, e, f, g, K[73], L(73));
      R(g, h, a, b, c, d, e, f, K[74], L(74));
      R(f, g, h, a, b, c, d, e, K[75], L(75));
      R(e, f, g, h, a, b, c, d, K[76], L(76));
      R(d, e, f, g, h, a, b, c, K[77], L(77));
      R(c, d, e, f, g, h, a, b, K[78], L(78));
      R(b, c, d, e, f, g, h, a, K[79], L(79));

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
      data += 128;
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

      R(a, b, c, d, e, f, g, h, K[64], L(64));
      R(h, a, b, c, d, e, f, g, K[65], L(65));
      R(g, h, a, b, c, d, e, f, K[66], L(66));
      R(f, g, h, a, b, c, d, e, K[67], L(67));
      R(e, f, g, h, a, b, c, d, K[68], L(68));
      R(d, e, f, g, h, a, b, c, K[69], L(69));
      R(c, d, e, f, g, h, a, b, K[70], L(70));
      R(b, c, d, e, f, g, h, a, K[71], L(71));
      R(a, b, c, d, e, f, g, h, K[72], L(72));
      R(h, a, b, c, d, e, f, g, K[73], L(73));
      R(g, h, a, b, c, d, e, f, K[74], L(74));
      R(f, g, h, a, b, c, d, e, K[75], L(75));
      R(e, f, g, h, a, b, c, d, K[76], L(76));
      R(d, e, f, g, h, a, b, c, K[77], L(77));
      R(c, d, e, f, g, h, a, b, K[78], L(78));
      R(b, c, d, e, f, g, h, a, K[79], L(79));

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
