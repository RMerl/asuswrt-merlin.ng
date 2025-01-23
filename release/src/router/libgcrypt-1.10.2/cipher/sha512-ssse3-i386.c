/* sha512-ssse3-i386.c - i386/SSSE3 implementation of SHA-512 transform
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

/*
 * SHA512 Message Expansion (I2 and W2 macros) based on implementation
 * from file "sha512-ssse3-amd64.s":
 ************************************************************************
 * Copyright (c) 2012, Intel Corporation
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 * * Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the
 *   distribution.
 *
 * * Neither the name of the Intel Corporation nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 *
 *
 * THIS SOFTWARE IS PROVIDED BY INTEL CORPORATION "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL INTEL CORPORATION OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 ************************************************************************
 */

#include <config.h>

#if defined(__i386__) && SIZEOF_UNSIGNED_LONG == 4 && __GNUC__ >= 4 && \
    defined(HAVE_GCC_INLINE_ASM_SSSE3) && defined(USE_SHA512)

#include "bufhelp.h"


#if _GCRY_GCC_VERSION >= 40400 /* 4.4 */
/* Prevent compiler from issuing SSE/MMX instructions between asm blocks. */
#  pragma GCC target("no-sse")
#  pragma GCC target("no-mmx")
#endif
#if __clang__
#  pragma clang attribute push (__attribute__((target("no-sse"))), apply_to = function)
#  pragma clang attribute push (__attribute__((target("no-mmx"))), apply_to = function)
#endif


#define ALWAYS_INLINE inline __attribute__((always_inline))
#define NO_INLINE __attribute__((noinline))
#define NO_INSTRUMENT_FUNCTION __attribute__((no_instrument_function))

#define ASM_FUNC_ATTR          NO_INSTRUMENT_FUNCTION
#define ASM_FUNC_ATTR_INLINE   ASM_FUNC_ATTR ALWAYS_INLINE
#define ASM_FUNC_ATTR_NOINLINE ASM_FUNC_ATTR NO_INLINE


static const u64 K[80] __attribute__ ((aligned (16))) =
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

static const unsigned char bshuf_mask[16] __attribute__ ((aligned (16))) =
  { 7, 6, 5, 4, 3, 2, 1, 0, 15, 14, 13, 12, 11, 10, 9, 8 };


/* SHA2 round */
#define RA "%%mm0"
#define RB "%%mm1"
#define RC "%%mm2"
#define RD "%%mm3"
#define RE "%%mm4"
#define RF "%%mm5"
#define RG "%%mm6"
#define RH "%%mm7"

#define Rx(a,b,c,d,e,f,g,h,wk) \
	asm volatile (/* Cho + Sum1 */					\
		      "movq2dq "a", %%xmm2;\n\t"			\
		      "movq "e", "a";\n\t"				\
		      "movq2dq "c", %%xmm3;\n\t"			\
		      "movq "e", "c";\n\t"				\
		      "movq2dq "b", %%xmm4;\n\t"			\
		      "movq "e", "b";\n\t"				\
		      "psrlq $(41-18), "c";\n\t"			\
		      "pandn "g", "a";\n\t"				\
		      "pxor "e", "c";\n\t"				\
		      "pand "f", "b";\n\t"				\
		      "psrlq $(18-14), "c";\n\t"			\
		      "paddq "a", "h";\n\t"				\
		      wk(a)						\
		      "pxor "e", "c";\n\t"				\
		      "paddq "b", "h";\n\t"				\
		      "psrlq $(14), "c";\n\t"				\
		      "movq "e", "b";\n\t"				\
		      "psllq $(50-46), "b";\n\t"			\
		      "paddq "a", "h";\n\t"				\
		      "movdq2q %%xmm2, "a";\n\t"			\
		      "pxor "e", "b";\n\t"				\
		      "psllq $(46-23), "b";\n\t"			\
		      "pxor "e", "b";\n\t"				\
		      "psllq $(23), "b";\n\t"				\
		      "pxor "b", "c";\n\t"				\
		      "movdq2q %%xmm4, "b";\n\t"			\
		      "paddq "c", "h";\n\t"				\
		      "movdq2q %%xmm3, "c";\n\t"			\
		      \
		      /* Maj + Sum0 */ \
		      "movq2dq "e", %%xmm2;\n\t"			\
		      "movq "a", "e";\n\t"				\
		      "movq2dq "g", %%xmm3;\n\t"			\
		      "movq "a", "g";\n\t"				\
		      "movq2dq "f", %%xmm4;\n\t"			\
		      "movq "a", "f";\n\t"				\
		      "psrlq $(39-34), "g";\n\t"			\
		      "pxor "b", "e";\n\t"				\
		      "pxor "a", "g";\n\t"				\
		      "pand "b", "f";\n\t"				\
		      "psrlq $(34-28), "g";\n\t"			\
		      "pand "c", "e";\n\t"				\
		      "pxor "a", "g";\n\t"				\
		      "paddq "h", "d";\n\t"				\
		      "paddq "f", "h";\n\t"				\
		      "movdq2q %%xmm4, "f";\n\t"			\
		      "psrlq $28, "g";\n\t"				\
		      "paddq "e", "h";\n\t"				\
		      "movq "a", "e";\n\t"				\
		      "psllq $(36-30), "e";\n\t"			\
		      "pxor "a", "e";\n\t"				\
		      "psllq $(30-25), "e";\n\t"			\
		      "pxor "a", "e";\n\t"				\
		      "psllq $(25), "e";\n\t"				\
		      "pxor "e", "g";\n\t"				\
		      "movdq2q %%xmm2, "e";\n\t"			\
		      "paddq "g", "h";\n\t"				\
		      "movdq2q %%xmm3, "g";\n\t"			\
		      \
		      :							\
		      :							\
		      : "memory" )

#define WK0(tmp)      "movdq2q %%xmm0, "tmp";\n\t"			\
		      "pshufd $0xee, %%xmm0, %%xmm0;\n\t"

#define WK1(tmp)      "movdq2q %%xmm0, "tmp";\n\t"

/* Message expansion */
#define I2(i) \
	asm volatile ("movdqu %[inbuf], %%xmm0;\n\t"			\
		      "pshufb %%xmm6, %%xmm0;\n\t"			\
		      "movdqu %%xmm0, %[w];\n\t"			\
		      "paddq %[k], %%xmm0;\n\t"				\
		      :							\
		      : [k] "m" (K[i]),					\
		        [w] "m" (w[i]),					\
		        [inbuf] "m" (data[(i)*8])			\
		      : "memory" )

#define W2(i) \
	asm volatile ("movdqu %[w_t_m_2], %%xmm2;\n\t"			\
		      "movdqa %%xmm2, %%xmm0;\n\t"			\
		      "movdqu %[w_t_m_15], %%xmm5;\n\t"			\
		      :							\
		      : [w_t_m_2] "m" (w[(i)-2]),			\
		        [w_t_m_15] "m" (w[(i)-15])			\
		      : "memory" );					\
	asm volatile ("movdqa %%xmm5, %%xmm3;\n\t"			\
		      "psrlq $(61-19), %%xmm0;\n\t"			\
		      "psrlq $(8-7), %%xmm3;\n\t"			\
		      "pxor %%xmm2, %%xmm0;\n\t"			\
		      "pxor %%xmm5, %%xmm3;\n\t"			\
		      "psrlq $(19-6), %%xmm0;\n\t"			\
		      "psrlq $(7-1), %%xmm3;\n\t"			\
		      "pxor %%xmm2, %%xmm0;\n\t"			\
		      "pxor %%xmm5, %%xmm3;\n\t"			\
		      "psrlq $6, %%xmm0;\n\t"				\
		      "psrlq $1, %%xmm3;\n\t"				\
		      "movdqa %%xmm2, %%xmm1;\n\t"			\
		      "movdqa %%xmm5, %%xmm4;\n\t"			\
		      "psllq $(61-19), %%xmm1;\n\t"			\
		      "psllq $(8-1), %%xmm4;\n\t"			\
		      "pxor %%xmm2, %%xmm1;\n\t"			\
		      "pxor %%xmm5, %%xmm4;\n\t"			\
		      "psllq $(64-61), %%xmm1;\n\t"			\
		      "psllq $(64-8), %%xmm4;\n\t"			\
		      "pxor %%xmm1, %%xmm0;\n\t"			\
		      "movdqu %[w_t_m_16], %%xmm2;\n\t"			\
		      "pxor %%xmm4, %%xmm3;\n\t"			\
		      "movdqu %[w_t_m_7], %%xmm1;\n\t"			\
		      :							\
		      : [w_t_m_7] "m" (w[(i)-7]),			\
		        [w_t_m_16] "m" (w[(i)-16])			\
		      : "memory" );					\
	asm volatile ("paddq %%xmm3, %%xmm0;\n\t"			\
		      "paddq %%xmm2, %%xmm0;\n\t"			\
		      "paddq %%xmm1, %%xmm0;\n\t"			\
		      "movdqu %%xmm0, %[w_t_m_0];\n\t"			\
		      "paddq %[k], %%xmm0;\n\t"				\
		      :	[w_t_m_0] "=m" (w[(i)-0])			\
		      : [k] "m" (K[i])					\
		      : "memory" )

unsigned int ASM_FUNC_ATTR
_gcry_sha512_transform_i386_ssse3(u64 state[8], const unsigned char *data,
				  size_t nblks)
{
  unsigned int t;
  u64 w[80];

  /* Load state to MMX registers. */
  asm volatile ("movq 8*0(%[state]), "RA";\n\t"
		"movq 8*1(%[state]), "RB";\n\t"
		"movq 8*2(%[state]), "RC";\n\t"
		"movq 8*3(%[state]), "RD";\n\t"
		"movq 8*4(%[state]), "RE";\n\t"
		"movq 8*5(%[state]), "RF";\n\t"
		"movq 8*6(%[state]), "RG";\n\t"
		"movq 8*7(%[state]), "RH";\n\t"
		:
		: [state] "r" (state)
		: "memory" );

  asm volatile ("movdqa %[bshuf_mask], %%xmm6;\n\t"
		:
		: [bshuf_mask] "m" (*bshuf_mask)
		: "memory" );

  while (nblks)
    {
      I2(0);
      Rx(RA, RB, RC, RD, RE, RF, RG, RH, WK0);
      Rx(RH, RA, RB, RC, RD, RE, RF, RG, WK1);
      I2(2);
      Rx(RG, RH, RA, RB, RC, RD, RE, RF, WK0);
      Rx(RF, RG, RH, RA, RB, RC, RD, RE, WK1);
      I2(4);
      Rx(RE, RF, RG, RH, RA, RB, RC, RD, WK0);
      Rx(RD, RE, RF, RG, RH, RA, RB, RC, WK1);
      I2(6);
      Rx(RC, RD, RE, RF, RG, RH, RA, RB, WK0);
      Rx(RB, RC, RD, RE, RF, RG, RH, RA, WK1);
      I2(8);
      Rx(RA, RB, RC, RD, RE, RF, RG, RH, WK0);
      Rx(RH, RA, RB, RC, RD, RE, RF, RG, WK1);
      I2(10);
      Rx(RG, RH, RA, RB, RC, RD, RE, RF, WK0);
      Rx(RF, RG, RH, RA, RB, RC, RD, RE, WK1);
      I2(12);
      Rx(RE, RF, RG, RH, RA, RB, RC, RD, WK0);
      Rx(RD, RE, RF, RG, RH, RA, RB, RC, WK1);
      I2(14);
      Rx(RC, RD, RE, RF, RG, RH, RA, RB, WK0);
      Rx(RB, RC, RD, RE, RF, RG, RH, RA, WK1);
      data += 128;

      for (t = 16; t < 80; t += 16)
	{
	  W2(t + 0);
	  Rx(RA, RB, RC, RD, RE, RF, RG, RH, WK0);
	  Rx(RH, RA, RB, RC, RD, RE, RF, RG, WK1);
	  W2(t + 2);
	  Rx(RG, RH, RA, RB, RC, RD, RE, RF, WK0);
	  Rx(RF, RG, RH, RA, RB, RC, RD, RE, WK1);
	  W2(t + 4);
	  Rx(RE, RF, RG, RH, RA, RB, RC, RD, WK0);
	  Rx(RD, RE, RF, RG, RH, RA, RB, RC, WK1);
	  W2(t + 6);
	  Rx(RC, RD, RE, RF, RG, RH, RA, RB, WK0);
	  Rx(RB, RC, RD, RE, RF, RG, RH, RA, WK1);
	  W2(t + 8);
	  Rx(RA, RB, RC, RD, RE, RF, RG, RH, WK0);
	  Rx(RH, RA, RB, RC, RD, RE, RF, RG, WK1);
	  W2(t + 10);
	  Rx(RG, RH, RA, RB, RC, RD, RE, RF, WK0);
	  Rx(RF, RG, RH, RA, RB, RC, RD, RE, WK1);
	  W2(t + 12);
	  Rx(RE, RF, RG, RH, RA, RB, RC, RD, WK0);
	  Rx(RD, RE, RF, RG, RH, RA, RB, RC, WK1);
	  W2(t + 14);
	  Rx(RC, RD, RE, RF, RG, RH, RA, RB, WK0);
	  Rx(RB, RC, RD, RE, RF, RG, RH, RA, WK1);
	}

      asm volatile ("paddq 8*0(%[state]), "RA";\n\t"
		    "paddq 8*1(%[state]), "RB";\n\t"
		    "paddq 8*2(%[state]), "RC";\n\t"
		    "paddq 8*3(%[state]), "RD";\n\t"
		    "paddq 8*4(%[state]), "RE";\n\t"
		    "paddq 8*5(%[state]), "RF";\n\t"
		    "paddq 8*6(%[state]), "RG";\n\t"
		    "paddq 8*7(%[state]), "RH";\n\t"
		    "movq "RA", 8*0(%[state]);\n\t"
		    "movq "RB", 8*1(%[state]);\n\t"
		    "movq "RC", 8*2(%[state]);\n\t"
		    "movq "RD", 8*3(%[state]);\n\t"
		    "movq "RE", 8*4(%[state]);\n\t"
		    "movq "RF", 8*5(%[state]);\n\t"
		    "movq "RG", 8*6(%[state]);\n\t"
		    "movq "RH", 8*7(%[state]);\n\t"
		    :
		    : [state] "r" (state)
		    : "memory" );

      nblks--;
    }

  /* Clear registers */
  asm volatile ("pxor %%xmm0, %%xmm0;\n\t"
		"pxor %%xmm1, %%xmm1;\n\t"
		"pxor %%xmm2, %%xmm2;\n\t"
		"pxor %%xmm3, %%xmm3;\n\t"
		"pxor %%xmm4, %%xmm4;\n\t"
		"pxor %%xmm5, %%xmm5;\n\t"
		"pxor %%xmm6, %%xmm6;\n\t"
		"pxor %%mm0, %%mm0;\n\t"
		"pxor %%mm1, %%mm1;\n\t"
		"pxor %%mm2, %%mm2;\n\t"
		"pxor %%mm3, %%mm3;\n\t"
		"pxor %%mm4, %%mm4;\n\t"
		"pxor %%mm5, %%mm5;\n\t"
		"pxor %%mm6, %%mm6;\n\t"
		"pxor %%mm7, %%mm7;\n\t"
		"emms;\n\t"
	       :
	       :
	       : "memory" );

  return sizeof(w);
}

#if __clang__
#  pragma clang attribute pop
#  pragma clang attribute pop
#endif

#endif
