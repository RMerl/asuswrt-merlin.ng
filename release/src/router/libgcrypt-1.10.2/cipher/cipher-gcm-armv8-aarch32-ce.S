/* cipher-gcm-armv8-aarch32-ce.S - ARM/CE accelerated GHASH
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
    defined(HAVE_GCC_INLINE_ASM_AARCH32_CRYPTO)

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

#define rbuf2 q4
#define rbuf2_l d8
#define rbuf2_h d9

#define rbuf3 q5
#define rbuf3_l d10
#define rbuf3_h d11

#define rh2 q6
#define rh2_l d12
#define rh2_h d13

#define rh3 q7
#define rh3_l d14
#define rh3_h d15

#define rh4 q8
#define rh4_l d16
#define rh4_h d17

#define rr2 q9
#define rr2_l d18
#define rr2_h d19

#define rr3 q10
#define rr3_l d20
#define rr3_h d21

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

/* GHASH macros */

/* See "Gouvêa, C. P. L. & López, J. Implementing GCM on ARMv8. Topics in
 * Cryptology — CT-RSA 2015" for details.
 */

/* Input: 'a' and 'b', Output: 'r0:r1' (low 128-bits in r0, high in r1)
 *  Note: 'r1' may be 'a' or 'b', 'r0' must not be either 'a' or 'b'.
 */
#define PMUL_128x128(r0, r1, a, b, t, interleave_op) \
        veor t##_h, b##_l, b##_h; \
        veor t##_l, a##_l, a##_h; \
        vmull.p64 r0, a##_l, b##_l; \
        vmull.p64 r1, a##_h, b##_h; \
        vmull.p64 t, t##_h, t##_l; \
        interleave_op; \
        veor t, r0; \
        veor t, r1; \
        veor r0##_h, t##_l; \
        veor r1##_l, t##_h;

/* Input: 'aA' and 'bA', Output: 'r0A:r1A' (low 128-bits in r0A, high in r1A)
 *  Note: 'r1A' may be 'aA' or 'bA', 'r0A' must not be either 'aA' or 'bA'.
 * Input: 'aB' and 'bB', Output: 'r0B:r1B' (low 128-bits in r0B, high in r1B)
 *  Note: 'r1B' may be 'aB' or 'bB', 'r0B' must not be either 'aB' or 'bB'.
 */
#define PMUL_128x128_2(r0A, r1A, aA, bA, r0B, r1B, aB, bB, tA, tB, interleave_op) \
        veor tA##_h, bA##_l, bA##_h; \
        veor tA##_l, aA##_l, aA##_h; \
          veor tB##_h, bB##_l, bB##_h; \
          veor tB##_l, aB##_l, aB##_h; \
        vmull.p64 r0A, aA##_l, bA##_l; \
        vmull.p64 r1A, aA##_h, bA##_h; \
        vmull.p64 tA, tA##_h, tA##_l; \
          vmull.p64 r0B, aB##_l, bB##_l; \
          vmull.p64 r1B, aB##_h, bB##_h; \
          vmull.p64 tB, tB##_h, tB##_l; \
        interleave_op; \
        veor tA, r0A; \
        veor tA, r1A; \
          veor tB, r0B; \
          veor tB, r1B; \
        veor r0A##_h, tA##_l; \
        veor r1A##_l, tA##_h; \
          veor r0B##_h, tB##_l; \
          veor r1B##_l, tB##_h; \

/* Input: 'r0:r1', Output: 'a' */
#define REDUCTION(a, r0, r1, rconst, t, interleave_op) \
        vmull.p64 t, r0##_l, rconst; \
        veor r0##_h, t##_l; \
        veor r1##_l, t##_h; \
        interleave_op; \
        vmull.p64 t, r0##_h, rconst; \
        veor r1, t; \
        veor a, r0, r1;

#define _(...) __VA_ARGS__
#define __ _()

/* Other functional macros */

#define CLEAR_REG(reg) vmov.i8 reg, #0;


/*
 * unsigned int _gcry_ghash_armv8_ce_pmull (void *gcm_key, byte *result,
 *                                          const byte *buf, size_t nblocks,
 *                                          void *gcm_table);
 */
.align 3
.globl _gcry_ghash_armv8_ce_pmull
.type  _gcry_ghash_armv8_ce_pmull,%function;
_gcry_ghash_armv8_ce_pmull:
  /* input:
   *    r0: gcm_key
   *    r1: result/hash
   *    r2: buf
   *    r3: nblocks
   *    %st+0: gcm_table
   */
  push {r4-r6, lr}

  cmp r3, #0
  beq .Ldo_nothing

  GET_DATA_POINTER(r4, .Lrconst64, lr)

  vld1.64 {rhash}, [r1]
  vld1.64 {rh1}, [r0]

  vrev64.8 rhash, rhash /* byte-swap */
  vld1.64 {rrconst_h}, [r4]
  vext.8 rhash, rhash, rhash, #8

  cmp r3, #4
  blo .Less_than_4

  /* Bulk processing of 4 blocks per loop iteration. */

  ldr r5, [sp, #(4*4)];
  add r6, r5, #32

  vpush {q4-q7}

  vld1.64 {rh2-rh3}, [r5]
  vld1.64 {rh4}, [r6]

  vld1.64 {rbuf-rbuf1}, [r2]!
  sub r3, r3, #4
  vld1.64 {rbuf2-rbuf3}, [r2]!

  cmp r3, #4
  vrev64.8 rbuf, rbuf /* byte-swap */
  vrev64.8 rbuf1, rbuf1 /* byte-swap */
  vrev64.8 rbuf2, rbuf2 /* byte-swap */
  vrev64.8 rbuf3, rbuf3 /* byte-swap */

  vext.8 rbuf, rbuf, rbuf, #8
  vext.8 rbuf1, rbuf1, rbuf1, #8
  vext.8 rbuf2, rbuf2, rbuf2, #8
  vext.8 rbuf3, rbuf3, rbuf3, #8
  veor rhash, rhash, rbuf /* in0 ^ hash */

  blo .Lend_4

.Loop_4:
  /* (in0 ^ hash) * H⁴ => rr2:rr3 */
  /* (in1) * H³ => rr0:rr1 */
  PMUL_128x128_2(rr0, rr1, rbuf1, rh3, rr2, rr3, rhash, rh4, rt1, rt0, __)

  vld1.64 {rbuf-rbuf1}, [r2]!
  sub r3, r3, #4
  veor rr0, rr0, rr2
  veor rr1, rr1, rr3

  /* (in2) * H² => rr2:rr3 */
  /* (in3) * H¹ => rhash:rbuf3 */
  PMUL_128x128_2(rr2, rr3, rbuf2, rh2, rhash, rbuf3, rbuf3, rh1, rt0, rt1,
                 _(vrev64.8 rbuf, rbuf))

  vld1.64 {rbuf2}, [r2]!

  vrev64.8 rbuf1, rbuf1
  veor rr0, rr0, rr2
  veor rr1, rr1, rr3

  cmp r3, #4
  vext.8 rbuf, rbuf, rbuf, #8
  vext.8 rbuf1, rbuf1, rbuf1, #8

  veor rr0, rr0, rhash
  veor rr1, rr1, rbuf3

  vld1.64 {rbuf3}, [r2]!

  REDUCTION(rhash, rr0, rr1, rrconst_h, rt1,
            _(vrev64.8 rbuf2, rbuf2;
              vrev64.8 rbuf3, rbuf3))

  vext.8 rbuf2, rbuf2, rbuf2, #8
  vext.8 rbuf3, rbuf3, rbuf3, #8
  veor rhash, rhash, rbuf /* in0 ^ hash */

  bhs .Loop_4

.Lend_4:
  /* (in0 ^ hash) * H⁴ => rr2:rr3 */
  /* (in1) * H³ => rr0:rr1 */
  PMUL_128x128_2(rr0, rr1, rbuf1, rh3, rr2, rr3, rhash, rh4, rt1, rt0, __)

  /* (in2) * H² => rhash:rbuf */
  /* (in3) * H¹ => rbuf1:rbuf2 */
  PMUL_128x128_2(rhash, rbuf, rbuf2, rh2, rbuf1, rbuf2, rbuf3, rh1, rt0, rt1,
                 _(veor rr0, rr0, rr2;
                   veor rr1, rr1, rr3))

  veor rr0, rr0, rhash
  veor rr1, rr1, rbuf

  veor rr0, rr0, rbuf1
  veor rr1, rr1, rbuf2

  REDUCTION(rhash, rr0, rr1, rrconst_h, rt1,
            _(CLEAR_REG(rr2);
              CLEAR_REG(rr3);
              CLEAR_REG(rbuf1);
              CLEAR_REG(rbuf2);
              CLEAR_REG(rbuf3);
              CLEAR_REG(rh2);
              CLEAR_REG(rh3);
              CLEAR_REG(rh4)))

  vpop {q4-q7}

  cmp r3, #0
  beq .Ldone

.Less_than_4:
  /* Handle remaining blocks. */

  vld1.64 {rbuf}, [r2]!
  subs r3, r3, #1

  vrev64.8 rbuf, rbuf /* byte-swap */
  vext.8 rbuf, rbuf, rbuf, #8

  veor rhash, rhash, rbuf

  beq .Lend

.Loop:
  vld1.64 {rbuf}, [r2]!
  subs r3, r3, #1
  PMUL_128x128(rr0, rr1, rhash, rh1, rt0, _(vrev64.8 rbuf, rbuf))
  REDUCTION(rhash, rr0, rr1, rrconst_h, rt0, _(vext.8 rbuf, rbuf, rbuf, #8))
  veor rhash, rhash, rbuf

  bne .Loop

.Lend:
  PMUL_128x128(rr0, rr1, rhash, rh1, rt0, _(CLEAR_REG(rbuf)))
  REDUCTION(rhash, rr0, rr1, rrconst_h, rt0, _(CLEAR_REG(rh1)))

.Ldone:
  CLEAR_REG(rr1)
  vrev64.8 rhash, rhash /* byte-swap */
  CLEAR_REG(rt0)
  CLEAR_REG(rr0)
  vext.8 rhash, rhash, rhash, #8
  CLEAR_REG(rt1)
  vst1.64 {rhash}, [r1]
  CLEAR_REG(rhash)

.Ldo_nothing:
  mov r0, #0
  pop {r4-r6, pc}
.size _gcry_ghash_armv8_ce_pmull,.-_gcry_ghash_armv8_ce_pmull;


/*
 * unsigned int _gcry_polyval_armv8_ce_pmull (void *gcm_key, byte *result,
 *                                            const byte *buf, size_t nblocks,
 *                                            void *gcm_table);
 */
.align 3
.globl _gcry_polyval_armv8_ce_pmull
.type  _gcry_polyval_armv8_ce_pmull,%function;
_gcry_polyval_armv8_ce_pmull:
  /* input:
   *    r0: gcm_key
   *    r1: result/hash
   *    r2: buf
   *    r3: nblocks
   *    %st+0: gcm_table
   */
  push {r4-r6, lr}

  cmp r3, #0
  beq .Lpolyval_do_nothing

  GET_DATA_POINTER(r4, .Lrconst64, lr)

  vld1.64 {rhash}, [r1]
  vld1.64 {rh1}, [r0]

  vrev64.8 rhash, rhash /* byte-swap */
  vld1.64 {rrconst_h}, [r4]
  vext.8 rhash, rhash, rhash, #8

  cmp r3, #4
  blo .Lpolyval_less_than_4

  /* Bulk processing of 4 blocks per loop iteration. */

  ldr r5, [sp, #(4*4)];
  add r6, r5, #32

  vpush {q4-q7}

  vld1.64 {rh2-rh3}, [r5]
  vld1.64 {rh4}, [r6]

  vld1.64 {rbuf-rbuf1}, [r2]!
  sub r3, r3, #4
  vld1.64 {rbuf2-rbuf3}, [r2]!

  cmp r3, #4
  veor rhash, rhash, rbuf /* in0 ^ hash */

  blo .Lpolyval_end_4

.Lpolyval_loop_4:
  /* (in0 ^ hash) * H⁴ => rr2:rr3 */
  /* (in1) * H³ => rr0:rr1 */
  PMUL_128x128_2(rr0, rr1, rbuf1, rh3, rr2, rr3, rhash, rh4, rt1, rt0, __)

  vld1.64 {rbuf-rbuf1}, [r2]!
  sub r3, r3, #4
  veor rr0, rr0, rr2
  veor rr1, rr1, rr3

  /* (in2) * H² => rr2:rr3 */
  /* (in3) * H¹ => rhash:rbuf3 */
  PMUL_128x128_2(rr2, rr3, rbuf2, rh2, rhash, rbuf3, rbuf3, rh1, rt0, rt1, __)

  vld1.64 {rbuf2}, [r2]!

  veor rr0, rr0, rr2
  veor rr1, rr1, rr3

  cmp r3, #4

  veor rr0, rr0, rhash
  veor rr1, rr1, rbuf3

  vld1.64 {rbuf3}, [r2]!

  REDUCTION(rhash, rr0, rr1, rrconst_h, rt1, __)

  veor rhash, rhash, rbuf /* in0 ^ hash */

  bhs .Lpolyval_loop_4

.Lpolyval_end_4:
  /* (in0 ^ hash) * H⁴ => rr2:rr3 */
  /* (in1) * H³ => rr0:rr1 */
  PMUL_128x128_2(rr0, rr1, rbuf1, rh3, rr2, rr3, rhash, rh4, rt1, rt0, __)

  /* (in2) * H² => rhash:rbuf */
  /* (in3) * H¹ => rbuf1:rbuf2 */
  PMUL_128x128_2(rhash, rbuf, rbuf2, rh2, rbuf1, rbuf2, rbuf3, rh1, rt0, rt1,
                 _(veor rr0, rr0, rr2;
                   veor rr1, rr1, rr3))

  veor rr0, rr0, rhash
  veor rr1, rr1, rbuf

  veor rr0, rr0, rbuf1
  veor rr1, rr1, rbuf2

  REDUCTION(rhash, rr0, rr1, rrconst_h, rt1,
            _(CLEAR_REG(rr2);
              CLEAR_REG(rr3);
              CLEAR_REG(rbuf1);
              CLEAR_REG(rbuf2);
              CLEAR_REG(rbuf3);
              CLEAR_REG(rh2);
              CLEAR_REG(rh3);
              CLEAR_REG(rh4)))

  vpop {q4-q7}

  cmp r3, #0
  beq .Lpolyval_done

.Lpolyval_less_than_4:
  /* Handle remaining blocks. */

  vld1.64 {rbuf}, [r2]!
  subs r3, r3, #1

  veor rhash, rhash, rbuf

  beq .Lpolyval_end

.Lpolyval_loop:
  vld1.64 {rbuf}, [r2]!
  subs r3, r3, #1
  PMUL_128x128(rr0, rr1, rhash, rh1, rt0, __)
  REDUCTION(rhash, rr0, rr1, rrconst_h, rt0, __)
  veor rhash, rhash, rbuf

  bne .Lpolyval_loop

.Lpolyval_end:
  PMUL_128x128(rr0, rr1, rhash, rh1, rt0, _(CLEAR_REG(rbuf)))
  REDUCTION(rhash, rr0, rr1, rrconst_h, rt0, _(CLEAR_REG(rh1)))

.Lpolyval_done:
  CLEAR_REG(rr1)
  vrev64.8 rhash, rhash /* byte-swap */
  CLEAR_REG(rt0)
  CLEAR_REG(rr0)
  vext.8 rhash, rhash, rhash, #8
  CLEAR_REG(rt1)
  vst1.64 {rhash}, [r1]
  CLEAR_REG(rhash)

.Lpolyval_do_nothing:
  mov r0, #0
  pop {r4-r6, pc}
.size _gcry_polyval_armv8_ce_pmull,.-_gcry_polyval_armv8_ce_pmull;


/*
 * void _gcry_ghash_setup_armv8_ce_pmull (void *gcm_key, void *gcm_table);
 */
.align 3
.globl _gcry_ghash_setup_armv8_ce_pmull
.type  _gcry_ghash_setup_armv8_ce_pmull,%function;
_gcry_ghash_setup_armv8_ce_pmull:
  /* input:
   *	r0: gcm_key
   *	r1: gcm_table
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

  /* H² */
  PMUL_128x128(rr0, rr1, rbuf1, rh1, rt0, __)
  REDUCTION(rh2, rr0, rr1, rrconst_h, rt0, __)
  vmov rhash, rh2
  GCM_LSH_1(r1, rh2_l, rh2_h, rrconst_h, rbuf1_l, rbuf1_h, rt1_l) /* H²<<<1 */
  add r1, r1, #16

  /* H³ */
  PMUL_128x128(rr0, rr1, rhash, rh1, rt1, __)
  REDUCTION(rh3, rr0, rr1, rrconst_h, rt1, __)

  /* H⁴ */
  PMUL_128x128(rr0, rr1, rhash, rbuf1, rt0, __)
  REDUCTION(rh4, rr0, rr1, rrconst_h, rt0, __)

  GCM_LSH_1(r1, rh3_l, rh3_h, rrconst_h, rt0_l, rt0_h, rt1_l) /* H³<<<1 */
  add r1, r1, #16
  GCM_LSH_1(r1, rh4_l, rh4_h, rrconst_h, rt0_l, rt0_h, rt1_l) /* H⁴<<<1 */

  CLEAR_REG(rt0)
  CLEAR_REG(rt1)
  CLEAR_REG(rr1)
  CLEAR_REG(rr0)
  CLEAR_REG(rh1)
  CLEAR_REG(rh2)
  CLEAR_REG(rh3)
  CLEAR_REG(rh4)
  CLEAR_REG(rhash)
  CLEAR_REG(rbuf1)
  CLEAR_REG(rrconst)
  vpop {q4-q7}
  bx lr
.size _gcry_ghash_setup_armv8_ce_pmull,.-_gcry_ghash_setup_armv8_ce_pmull;

#endif
