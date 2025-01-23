/* sha1-armv8-aarch32-ce.S - ARM/CE accelerated SHA-1 transform function
 * Copyright (C) 2016 Jussi Kivilinna <jussi.kivilinna@iki.fi>
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

#if defined(HAVE_ARM_ARCH_V6) && defined(__ARMEL__) && \
    defined(HAVE_COMPATIBLE_GCC_ARM_PLATFORM_AS) && \
    defined(HAVE_GCC_INLINE_ASM_AARCH32_CRYPTO) && defined(USE_SHA1)

.syntax unified
.arch armv8-a
.fpu crypto-neon-fp-armv8
.arm

.text

#ifdef __PIC__
#  define GET_DATA_POINTER(reg, name, rtmp) \
		ldr reg, 1f; \
		ldr rtmp, 2f; \
		b 3f; \
	1:	.word _GLOBAL_OFFSET_TABLE_-(3f+8); \
	2:	.word name(GOT); \
	3:	add reg, pc, reg; \
		ldr reg, [reg, rtmp];
#else
#  define GET_DATA_POINTER(reg, name, rtmp) ldr reg, =name
#endif


/* Constants */

#define K1  0x5A827999
#define K2  0x6ED9EBA1
#define K3  0x8F1BBCDC
#define K4  0xCA62C1D6
.align 4
gcry_sha1_aarch32_ce_K_VEC:
.LK_VEC:
.LK1:	.long K1, K1, K1, K1
.LK2:	.long K2, K2, K2, K2
.LK3:	.long K3, K3, K3, K3
.LK4:	.long K4, K4, K4, K4


/* Register macros */

#define qH4    q0
#define sH4    s0
#define qH0123 q1

#define qABCD q2
#define qE0   q3
#define qE1   q4

#define qT0   q5
#define qT1   q6

#define qW0 q8
#define qW1 q9
#define qW2 q10
#define qW3 q11

#define qK1 q12
#define qK2 q13
#define qK3 q14
#define qK4 q15


/* Round macros */

#define _(...) /*_*/
#define do_add(dst, src0, src1) vadd.u32 dst, src0, src1;
#define do_sha1su0(w0,w1,w2) sha1su0.32 w0,w1,w2;
#define do_sha1su1(w0,w3) sha1su1.32 w0,w3;

#define do_rounds(f, e0, e1, t, k, w0, w1, w2, w3, add_fn, sha1su0_fn, sha1su1_fn) \
        sha1su1_fn( w3, w2     ); \
        sha1h.32    e0, qABCD; \
        sha1##f.32  qABCD, e1, t; \
        add_fn(     t, w2, k   ); \
        sha1su0_fn( w0, w1, w2 );


/* Other functional macros */

#define CLEAR_REG(reg) vmov.i8 reg, #0;


/*
 * unsigned int
 * _gcry_sha1_transform_armv8_ce (void *ctx, const unsigned char *data,
 *                                size_t nblks)
 */
.align 3
.globl _gcry_sha1_transform_armv8_ce
.type  _gcry_sha1_transform_armv8_ce,%function;
_gcry_sha1_transform_armv8_ce:
  /* input:
   *	r0: ctx, CTX
   *	r1: data (64*nblks bytes)
   *	r2: nblks
   */

  cmp r2, #0;
  push {r4,lr};
  beq .Ldo_nothing;

  vpush {q4-q7};

  GET_DATA_POINTER(r4, .LK_VEC, lr);

  veor qH4, qH4
  vld1.32 {qH0123}, [r0]    /* load h0,h1,h2,h3 */

  vld1.32 {qK1-qK2}, [r4]!  /* load K1,K2 */
  vldr sH4, [r0, #16]       /* load h4 */
  vld1.32 {qK3-qK4}, [r4]   /* load K3,K4 */

  vld1.8 {qW0-qW1}, [r1]!
  vmov qABCD, qH0123
  vld1.8 {qW2-qW3}, [r1]!

  vrev32.8 qW0, qW0
  vrev32.8 qW1, qW1
  vrev32.8 qW2, qW2
  do_add(qT0, qW0, qK1)
  vrev32.8 qW3, qW3
  do_add(qT1, qW1, qK1)

.Loop:
  do_rounds(c, qE1, qH4, qT0, qK1, qW0, qW1, qW2, qW3, do_add, do_sha1su0, _)
  subs r2, r2, #1
  do_rounds(c, qE0, qE1, qT1, qK1, qW1, qW2, qW3, qW0, do_add, do_sha1su0, do_sha1su1)
  do_rounds(c, qE1, qE0, qT0, qK1, qW2, qW3, qW0, qW1, do_add, do_sha1su0, do_sha1su1)
  do_rounds(c, qE0, qE1, qT1, qK2, qW3, qW0, qW1, qW2, do_add, do_sha1su0, do_sha1su1)
  do_rounds(c, qE1, qE0, qT0, qK2, qW0, qW1, qW2, qW3, do_add, do_sha1su0, do_sha1su1)

  do_rounds(p, qE0, qE1, qT1, qK2, qW1, qW2, qW3, qW0, do_add, do_sha1su0, do_sha1su1)
  do_rounds(p, qE1, qE0, qT0, qK2, qW2, qW3, qW0, qW1, do_add, do_sha1su0, do_sha1su1)
  do_rounds(p, qE0, qE1, qT1, qK2, qW3, qW0, qW1, qW2, do_add, do_sha1su0, do_sha1su1)
  do_rounds(p, qE1, qE0, qT0, qK3, qW0, qW1, qW2, qW3, do_add, do_sha1su0, do_sha1su1)
  do_rounds(p, qE0, qE1, qT1, qK3, qW1, qW2, qW3, qW0, do_add, do_sha1su0, do_sha1su1)

  do_rounds(m, qE1, qE0, qT0, qK3, qW2, qW3, qW0, qW1, do_add, do_sha1su0, do_sha1su1)
  do_rounds(m, qE0, qE1, qT1, qK3, qW3, qW0, qW1, qW2, do_add, do_sha1su0, do_sha1su1)
  do_rounds(m, qE1, qE0, qT0, qK3, qW0, qW1, qW2, qW3, do_add, do_sha1su0, do_sha1su1)
  do_rounds(m, qE0, qE1, qT1, qK4, qW1, qW2, qW3, qW0, do_add, do_sha1su0, do_sha1su1)
  do_rounds(m, qE1, qE0, qT0, qK4, qW2, qW3, qW0, qW1, do_add, do_sha1su0, do_sha1su1)

  do_rounds(p, qE0, qE1, qT1, qK4, qW3, qW0, qW1, qW2, do_add, do_sha1su0, do_sha1su1)
  beq .Lend

  vld1.8 {qW0-qW1}, [r1]! /* preload */
  do_rounds(p, qE1, qE0, qT0, qK4, _  , _  , qW2, qW3, do_add, _, do_sha1su1)
  vrev32.8 qW0, qW0
  vld1.8 {qW2}, [r1]!
  vrev32.8 qW1, qW1
  do_rounds(p, qE0, qE1, qT1, qK4, _  , _  , qW3, _  , do_add, _, _)
  vld1.8 {qW3}, [r1]!
  vrev32.8 qW2, qW2
  do_rounds(p, qE1, qE0, qT0, _, _, _, _, _, _, _, _)
  vrev32.8 qW3, qW3
  do_rounds(p, qE0, qE1, qT1, _, _, _, _, _, _, _, _)

  do_add(qT0, qW0, qK1)
  vadd.u32 qH4, qE0
  vadd.u32 qABCD, qH0123
  do_add(qT1, qW1, qK1)

  vmov qH0123, qABCD

  b .Loop

.Lend:
  do_rounds(p, qE1, qE0, qT0, qK4, _  , _  , qW2, qW3, do_add, _, do_sha1su1)
  do_rounds(p, qE0, qE1, qT1, qK4, _  , _  , qW3, _  , do_add, _, _)
  do_rounds(p, qE1, qE0, qT0, _, _, _, _, _, _, _, _)
  do_rounds(p, qE0, qE1, qT1, _, _, _, _, _, _, _, _)

  vadd.u32 qH4, qE0
  vadd.u32 qH0123, qABCD

  CLEAR_REG(qW0)
  CLEAR_REG(qW1)
  CLEAR_REG(qW2)
  CLEAR_REG(qW3)
  CLEAR_REG(qABCD)
  CLEAR_REG(qE1)
  CLEAR_REG(qE0)

  vstr sH4, [r0, #16]    /* store h4 */
  vst1.32 {qH0123}, [r0] /* store h0,h1,h2,h3 */

  CLEAR_REG(qH0123)
  CLEAR_REG(qH4)
  vpop {q4-q7}

.Ldo_nothing:
  mov r0, #0
  pop {r4,pc}
.size _gcry_sha1_transform_armv8_ce,.-_gcry_sha1_transform_armv8_ce;

#endif
