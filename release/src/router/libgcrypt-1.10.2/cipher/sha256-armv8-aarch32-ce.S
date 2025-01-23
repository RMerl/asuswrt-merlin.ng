/* sha256-armv8-aarch32-ce.S - ARM/CE accelerated SHA-256 transform function
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
    defined(HAVE_GCC_INLINE_ASM_AARCH32_CRYPTO) && defined(USE_SHA256)

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

.align 4
gcry_sha256_aarch32_ce_K:
.LK:
  .long 0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5
  .long 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5
  .long 0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3
  .long 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174
  .long 0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc
  .long 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da
  .long 0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7
  .long 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967
  .long 0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13
  .long 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85
  .long 0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3
  .long 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070
  .long 0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5
  .long 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3
  .long 0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208
  .long 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2


/* Register macros */

#define qH0123 q0
#define qH4567 q1

#define qABCD0 q2
#define qABCD1 q3
#define qEFGH  q4

#define qT0 q5
#define qT1 q6

#define qW0 q8
#define qW1 q9
#define qW2 q10
#define qW3 q11

#define qK0 q12
#define qK1 q13
#define qK2 q14
#define qK3 q15


/* Round macros */

#define _(...) /*_*/

#define do_loadk(nk0, nk1) vld1.32 {nk0-nk1},[lr]!;
#define do_add(a, b) vadd.u32 a, a, b;
#define do_sha256su0(w0, w1) sha256su0.32 w0, w1;
#define do_sha256su1(w0, w2, w3) sha256su1.32 w0, w2, w3;

#define do_rounds(k, nk0, nk1, w0, w1, w2, w3, loadk_fn, add_fn, su0_fn, su1_fn) \
        loadk_fn(   nk0, nk1     ); \
        su0_fn(     w0, w1       ); \
        vmov        qABCD1, qABCD0; \
        sha256h.32  qABCD0, qEFGH, k; \
        sha256h2.32 qEFGH, qABCD1, k; \
        add_fn(     nk0, w2      ); \
        su1_fn(     w0, w2, w3   );


/* Other functional macros */

#define CLEAR_REG(reg) vmov.i8 reg, #0;


/*
 * unsigned int
 * _gcry_sha256_transform_armv8_ce (u32 state[8], const void *input_data,
 *                                  size_t num_blks)
 */
.align 3
.globl _gcry_sha256_transform_armv8_ce
.type  _gcry_sha256_transform_armv8_ce,%function;
_gcry_sha256_transform_armv8_ce:
  /* input:
   *	r0: ctx, CTX
   *	r1: data (64*nblks bytes)
   *	r2: nblks
   */

  cmp r2, #0;
  push {r4,lr};
  beq .Ldo_nothing;

  vpush {q4-q7};

  GET_DATA_POINTER(r4, .LK, lr);
  mov lr, r4

  vld1.32 {qH0123-qH4567}, [r0]  /* load state */

  vld1.8 {qW0-qW1}, [r1]!
  do_loadk(qK0, qK1)
  vld1.8 {qW2-qW3}, [r1]!
  vmov qABCD0, qH0123
  vmov qEFGH, qH4567

  vrev32.8 qW0, qW0
  vrev32.8 qW1, qW1
  vrev32.8 qW2, qW2
  do_add(qK0, qW0)
  vrev32.8 qW3, qW3
  do_add(qK1, qW1)

.Loop:
  do_rounds(qK0, qK2, qK3, qW0, qW1, qW2, qW3, do_loadk, do_add, do_sha256su0, do_sha256su1)
  subs r2,r2,#1
  do_rounds(qK1, qK3, _  , qW1, qW2, qW3, qW0, _       , do_add, do_sha256su0, do_sha256su1)
  do_rounds(qK2, qK0, qK1, qW2, qW3, qW0, qW1, do_loadk, do_add, do_sha256su0, do_sha256su1)
  do_rounds(qK3, qK1, _  , qW3, qW0, qW1, qW2, _       , do_add, do_sha256su0, do_sha256su1)

  do_rounds(qK0, qK2, qK3, qW0, qW1, qW2, qW3, do_loadk, do_add, do_sha256su0, do_sha256su1)
  do_rounds(qK1, qK3, _  , qW1, qW2, qW3, qW0, _       , do_add, do_sha256su0, do_sha256su1)
  do_rounds(qK2, qK0, qK1, qW2, qW3, qW0, qW1, do_loadk, do_add, do_sha256su0, do_sha256su1)
  do_rounds(qK3, qK1, _  , qW3, qW0, qW1, qW2, _       , do_add, do_sha256su0, do_sha256su1)

  do_rounds(qK0, qK2, qK3, qW0, qW1, qW2, qW3, do_loadk, do_add, do_sha256su0, do_sha256su1)
  do_rounds(qK1, qK3, _  , qW1, qW2, qW3, qW0, _       , do_add, do_sha256su0, do_sha256su1)
  do_rounds(qK2, qK0, qK1, qW2, qW3, qW0, qW1, do_loadk, do_add, do_sha256su0, do_sha256su1)
  do_rounds(qK3, qK1, _  , qW3, qW0, qW1, qW2, _       , do_add, do_sha256su0, do_sha256su1)

  beq .Lend

  do_rounds(qK0, qK2, qK3, qW0, _  , qW2, qW3, do_loadk, do_add, _, _)
  vld1.8 {qW0}, [r1]!
  mov lr, r4
  do_rounds(qK1, qK3, _  , qW1, _  , qW3, _  , _       , do_add, _, _)
  vld1.8 {qW1}, [r1]!
  vrev32.8 qW0, qW0
  do_rounds(qK2, qK0, qK1, qW2, _  , qW0, _  , do_loadk, do_add, _, _)
  vrev32.8 qW1, qW1
  vld1.8 {qW2}, [r1]!
  do_rounds(qK3, qK1, _  , qW3, _  , qW1, _  , _       , do_add, _, _)
  vld1.8 {qW3}, [r1]!

  vadd.u32 qH0123, qABCD0
  vadd.u32 qH4567, qEFGH

  vrev32.8 qW2, qW2
  vmov qABCD0, qH0123
  vrev32.8 qW3, qW3
  vmov qEFGH, qH4567

  b .Loop

.Lend:

  do_rounds(qK0, qK2, qK3, qW0, _  , qW2, qW3, do_loadk, do_add, _, _)
  do_rounds(qK1, qK3, _  , qW1, _  , qW3, _  , _       , do_add, _, _)
  do_rounds(qK2, _  , _  , qW2, _  , _  , _  , _       , _, _, _)
  do_rounds(qK3, _  , _  , qW3, _  , _  , _  , _       , _, _, _)

  CLEAR_REG(qW0)
  CLEAR_REG(qW1)
  CLEAR_REG(qW2)
  CLEAR_REG(qW3)
  CLEAR_REG(qK0)
  CLEAR_REG(qK1)
  CLEAR_REG(qK2)
  CLEAR_REG(qK3)

  vadd.u32 qH0123, qABCD0
  vadd.u32 qH4567, qEFGH

  CLEAR_REG(qABCD0)
  CLEAR_REG(qABCD1)
  CLEAR_REG(qEFGH)

  vst1.32 {qH0123-qH4567}, [r0] /* store state */

  CLEAR_REG(qH0123)
  CLEAR_REG(qH4567)
  vpop {q4-q7}

.Ldo_nothing:
  mov r0, #0
  pop {r4,pc}
.size _gcry_sha256_transform_armv8_ce,.-_gcry_sha256_transform_armv8_ce;

#endif
