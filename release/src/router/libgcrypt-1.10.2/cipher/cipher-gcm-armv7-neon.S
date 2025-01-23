/* cipher-gcm-armv7-neon.S - ARM/NEON accelerated GHASH
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

#if defined(HAVE_ARM_ARCH_V6) && defined(__ARMEL__) && \
    defined(HAVE_COMPATIBLE_GCC_ARM_PLATFORM_AS) && \
    defined(HAVE_GCC_INLINE_ASM_NEON)

.syntax unified
.fpu neon
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
gcry_gcm_reduction_constant:
.Lrconst64:
  .quad 0xc200000000000000

/* Register macros */

#define rhash q0
#define rhash_l d0
#define rhash_h d1

#define rh1 q1
#define rh1_l d2
#define rh1_h d3

#define rbuf q2
#define rbuf_l d4
#define rbuf_h d5

#define rbuf1 q3
#define rbuf1_l d6
#define rbuf1_h d7

#define t0q q4
#define t0l d8
#define t0h d9

#define t1q q5
#define t1l d10
#define t1h d11

#define t2q q6
#define t2l d12
#define t2h d13

#define t3q q7
#define t3l d14
#define t3h d15

/* q8 */
#define k16 d16
#define k32 d17

/* q9 */
#define k48 d18

#define k0 q10

#define rr0 q11
#define rr0_l d22
#define rr0_h d23

#define rr1 q12
#define rr1_l d24
#define rr1_h d25

#define rt0 q13
#define rt0_l d26
#define rt0_h d27

#define rt1 q14
#define rt1_l d28
#define rt1_h d29

#define rrconst q15
#define rrconst_l d30
#define rrconst_h d31

/* Macro for 64x64=>128 carry-less multiplication using vmull.p8 instruction.
 *
 * From "Câmara, D.; Gouvêa, C. P. L.; López, J. & Dahab, R. Fast Software
 * Polynomial Multiplication on ARM Processors using the NEON Engine. The
 * Second International Workshop on Modern Cryptography and Security
 * Engineering — MoCrySEn, 2013". */

#define vmull_p64(rq, rl, rh, ad, bd) \
	vext.8 t0l, ad, ad, $1; \
	vmull.p8 t0q, t0l, bd; \
	vext.8 rl, bd, bd, $1; \
	vmull.p8 rq, ad, rl; \
	vext.8 t1l, ad, ad, $2; \
	vmull.p8 t1q, t1l, bd; \
	vext.8 t3l, bd, bd, $2; \
	vmull.p8 t3q, ad, t3l; \
	vext.8 t2l, ad, ad, $3; \
	vmull.p8 t2q, t2l, bd; \
	veor t0q, t0q, rq; \
	vext.8 rl, bd, bd, $3; \
	vmull.p8 rq, ad, rl; \
	veor t1q, t1q, t3q; \
	vext.8 t3l, bd, bd, $4; \
	vmull.p8 t3q, ad, t3l; \
	veor t0l, t0l, t0h; \
	vand t0h, t0h, k48; \
	veor t1l, t1l, t1h; \
	vand t1h, t1h, k32; \
	veor t2q, t2q, rq; \
	veor t0l, t0l, t0h; \
	veor t1l, t1l, t1h; \
	veor t2l, t2l, t2h; \
	vand t2h, t2h, k16; \
	veor t3l, t3l, t3h; \
	vmov.i64 t3h, $0; \
	vext.8 t0q, t0q, t0q, $15; \
	veor t2l, t2l, t2h; \
	vext.8 t1q, t1q, t1q, $14; \
	vmull.p8 rq, ad, bd; \
	vext.8 t2q, t2q, t2q, $13; \
	vext.8 t3q, t3q, t3q, $12; \
	veor t0q, t0q, t1q; \
	veor t2q, t2q, t3q; \
	veor rq, rq, t0q; \
	veor rq, rq, t2q;

/* GHASH macros.
 *
 * See "Gouvêa, C. P. L. & López, J. Implementing GCM on ARMv8. Topics in
 * Cryptology — CT-RSA 2015" for details.
 */

/* Input: 'a' and 'b', Output: 'r0:r1' (low 128-bits in r0, high in r1)
 *  Note: 'r1' may be 'a' or 'b', 'r0' must not be either 'a' or 'b'.
 */
#define PMUL_128x128(r0, r1, a, b, t1, t2, interleave_op) \
        veor t1##_h, b##_l, b##_h; \
        veor t1##_l, a##_l, a##_h; \
        vmull_p64( r0, r0##_l, r0##_h, a##_l, b##_l ); \
        vmull_p64( r1, r1##_l, r1##_h, a##_h, b##_h ); \
        vmull_p64( t2, t2##_h, t2##_l, t1##_h, t1##_l ); \
        interleave_op; \
        veor t2, r0; \
        veor t2, r1; \
        veor r0##_h, t2##_l; \
        veor r1##_l, t2##_h;

/* Reduction using Xor and Shift.
 * Input: 'r0:r1', Output: 'a'
 *
 * See "Shay Gueron, Michael E. Kounavis. Intel Carry-Less Multiplication
 * Instruction and its Usage for Computing the GCM Mode" for details.
 */
#define REDUCTION(a, r0, r1, t, interleave_op) \
        vshl.u32 t0q, r0, #31; \
        vshl.u32 t1q, r0, #30; \
        vshl.u32 t2q, r0, #25; \
        veor t0q, t0q, t1q; \
        veor t0q, t0q, t2q; \
        vext.8 t, t0q, k0, #4; \
        vext.8 t0q, k0, t0q, #(16-12); \
        veor r0, r0, t0q; \
        interleave_op; \
        vshr.u32 t0q, r0, #1; \
        vshr.u32 t1q, r0, #2; \
        vshr.u32 t2q, r0, #7; \
        veor t0q, t0q, t1q; \
        veor t0q, t0q, t2q; \
        veor t0q, t0q, t; \
        veor r0, r0, t0q; \
        veor a, r0, r1;

#define _(...) __VA_ARGS__
#define __ _()

/* Other functional macros */

#define CLEAR_REG(reg) vmov.i8 reg, #0;


/*
 * unsigned int _gcry_ghash_armv7_neon (void *gcm_key, byte *result,
 *                                      const byte *buf, size_t nblocks);
 */
.align 3
.globl _gcry_ghash_armv7_neon
.type  _gcry_ghash_armv7_neon,%function;
_gcry_ghash_armv7_neon:
  /* input:
   *    r0: gcm_key
   *    r1: result/hash
   *    r2: buf
   *    r3: nblocks
   */
  push {r4-r6, lr}

  cmp r3, #0
  beq .Ldo_nothing

  vpush {q4-q7}

  vld1.64 {rhash}, [r1]
  vld1.64 {rh1}, [r0]

  vrev64.8 rhash, rhash /* byte-swap */

  vmov.i64 k0, #0x0
  vmov.i64 k16, #0xffff
  vmov.i64 k32, #0xffffffff
  vmov.i64 k48, #0xffffffffffff

  vext.8 rhash, rhash, rhash, #8

  /* Handle remaining blocks. */

  vld1.64 {rbuf}, [r2]!
  subs r3, r3, #1

  vrev64.8 rbuf, rbuf /* byte-swap */
  vext.8 rbuf, rbuf, rbuf, #8

  veor rhash, rhash, rbuf

  beq .Lend

.Loop:
  vld1.64 {rbuf}, [r2]!
  PMUL_128x128(rr0, rr1, rhash, rh1, rt0, rt1, _(vrev64.8 rbuf, rbuf))
  REDUCTION(rhash, rr0, rr1, rt0, _(vext.8 rbuf, rbuf, rbuf, #8))
  subs r3, r3, #1
  veor rhash, rhash, rbuf

  bne .Loop

.Lend:
  PMUL_128x128(rr0, rr1, rhash, rh1, rt0, rt1, _(CLEAR_REG(rbuf)))
  REDUCTION(rhash, rr0, rr1, rt0, _(CLEAR_REG(rh1)))

.Ldone:
  CLEAR_REG(rr1)
  vrev64.8 rhash, rhash /* byte-swap */
  CLEAR_REG(rt0)
  CLEAR_REG(rr0)
  vext.8 rhash, rhash, rhash, #8
  CLEAR_REG(rt1)
  CLEAR_REG(t0q)
  CLEAR_REG(t1q)
  CLEAR_REG(t2q)
  CLEAR_REG(t3q)
  vst1.64 {rhash}, [r1]
  CLEAR_REG(rhash)

  vpop {q4-q7}

.Ldo_nothing:
  mov r0, #0
  pop {r4-r6, pc}
.size _gcry_ghash_armv7_neon,.-_gcry_ghash_armv7_neon;


/*
 * void _gcry_ghash_armv7_neon (void *gcm_key);
 */
.align 3
.globl _gcry_ghash_setup_armv7_neon
.type  _gcry_ghash_setup_armv7_neon,%function;
_gcry_ghash_setup_armv7_neon:
  /* input:
   *	r0: gcm_key
   */

  vpush {q4-q7}

  GET_DATA_POINTER(r2, .Lrconst64, r3)

  vld1.64 {rrconst_h}, [r2]

#define GCM_LSH_1(r_out, ia, ib, const_d, oa, ob, ma) \
        /* H <<< 1 */ \
        vshr.s64 ma, ib, #63; \
        vshr.u64 oa, ib, #63; \
        vshr.u64 ob, ia, #63; \
        vand ma, const_d; \
        vshl.u64 ib, ib, #1; \
        vshl.u64 ia, ia, #1; \
        vorr ob, ib; \
        vorr oa, ia; \
        veor ob, ma; \
        vst1.64 {oa, ob}, [r_out]

  vld1.64 {rhash}, [r0]
  vrev64.8 rhash, rhash /* byte-swap */
  vext.8 rhash, rhash, rhash, #8

  vmov rbuf1, rhash
  GCM_LSH_1(r0, rhash_l, rhash_h, rrconst_h, rh1_l, rh1_h, rt1_l) /* H<<<1 */

  CLEAR_REG(rh1)
  CLEAR_REG(rhash)
  CLEAR_REG(rbuf1)
  CLEAR_REG(rrconst)
  vpop {q4-q7}
  bx lr
.size _gcry_ghash_setup_armv7_neon,.-_gcry_ghash_setup_armv7_neon;

#endif
