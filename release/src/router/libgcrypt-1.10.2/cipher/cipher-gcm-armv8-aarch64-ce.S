/* cipher-gcm-armv8-aarch64-ce.S - ARM/CE accelerated GHASH
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

#include "asm-common-aarch64.h"

#if defined(__AARCH64EL__) && \
    defined(HAVE_COMPATIBLE_GCC_AARCH64_PLATFORM_AS) && \
    defined(HAVE_GCC_INLINE_ASM_AARCH64_CRYPTO)

.cpu generic+simd+crypto

.text


/* Constants */

.align 4
gcry_gcm_reduction_constant:
.Lrconst:
  .quad 0x87


/* Register macros */

#define rhash   v0
#define rr0     v1
#define rr1     v2
#define rbuf    v3
#define rbuf1   v4
#define rbuf2   v5
#define rbuf3   v6
#define rbuf4   v7
#define rbuf5   v8
#define rr2     v9
#define rr3     v10
#define rr4     v11
#define rr5     v12
#define rr6     v13
#define rr7     v14
#define rr8     v15
#define rr9     v16

#define rrconst v18
#define rh1     v19
#define rh2     v20
#define rh3     v21
#define rh4     v22
#define rh5     v23
#define rh6     v24
#define t0      v25
#define t1      v26
#define t2      v27
#define t3      v28
#define t4      v29
#define t5      v30
#define vZZ     v31

/* GHASH macros */

/* See "Gouvêa, C. P. L. & López, J. Implementing GCM on ARMv8. Topics in
 * Cryptology — CT-RSA 2015" for details.
 */

/* Input: 'a' and 'b', Output: 'r0:r1' (low 128-bits in r0, high in r1) */
#define PMUL_128x128(r0, r1, a, b, T0, T1, interleave_op) \
	ext T0.16b, b.16b, b.16b, #8; \
	pmull r0.1q, a.1d, b.1d; \
	pmull2 r1.1q, a.2d, b.2d; \
	pmull T1.1q, a.1d, T0.1d; \
	pmull2 T0.1q, a.2d, T0.2d; \
	interleave_op; \
	eor T0.16b, T0.16b, T1.16b; \
	ext T1.16b, vZZ.16b, T0.16b, #8; \
	ext T0.16b, T0.16b, vZZ.16b, #8; \
	eor r0.16b, r0.16b, T1.16b; \
	eor r1.16b, r1.16b, T0.16b;

/* Input: 'aA' and 'bA', Output: 'r0A:r1A' (low 128-bits in r0A, high in r1A)
 * Input: 'aB' and 'bB', Output: 'r0B:r1B' (low 128-bits in r0B, high in r1B)
 * Input: 'aC' and 'bC', Output: 'r0C:r1C' (low 128-bits in r0C, high in r1C)
 */
#define PMUL_128x128_3(r0A, r1A, aA, bA, t0A, t1A, \
                       r0B, r1B, aB, bB, t0B, t1B, \
                       r0C, r1C, aC, bC, t0C, t1C,  interleave_op) \
        ext t0A.16b, bA.16b, bA.16b, #8; \
        pmull r0A.1q, aA.1d, bA.1d; \
        pmull2 r1A.1q, aA.2d, bA.2d; \
          ext t0B.16b, bB.16b, bB.16b, #8; \
          pmull r0B.1q, aB.1d, bB.1d; \
          pmull2 r1B.1q, aB.2d, bB.2d; \
            ext t0C.16b, bC.16b, bC.16b, #8; \
            pmull r0C.1q, aC.1d, bC.1d; \
            pmull2 r1C.1q, aC.2d, bC.2d; \
        pmull t1A.1q, aA.1d, t0A.1d; \
        pmull2 t0A.1q, aA.2d, t0A.2d; \
          pmull t1B.1q, aB.1d, t0B.1d; \
          pmull2 t0B.1q, aB.2d, t0B.2d; \
            pmull t1C.1q, aC.1d, t0C.1d; \
            pmull2 t0C.1q, aC.2d, t0C.2d; \
        eor t0A.16b, t0A.16b, t1A.16b; \
          eor t0B.16b, t0B.16b, t1B.16b; \
            eor t0C.16b, t0C.16b, t1C.16b; \
              interleave_op; \
        ext t1A.16b, vZZ.16b, t0A.16b, #8; \
        ext t0A.16b, t0A.16b, vZZ.16b, #8; \
          ext t1B.16b, vZZ.16b, t0B.16b, #8; \
          ext t0B.16b, t0B.16b, vZZ.16b, #8; \
            ext t1C.16b, vZZ.16b, t0C.16b, #8; \
            ext t0C.16b, t0C.16b, vZZ.16b, #8; \
        eor r0A.16b, r0A.16b, t1A.16b; \
        eor r1A.16b, r1A.16b, t0A.16b; \
          eor r0B.16b, r0B.16b, t1B.16b; \
          eor r1B.16b, r1B.16b, t0B.16b; \
            eor r0C.16b, r0C.16b, t1C.16b; \
            eor r1C.16b, r1C.16b, t0C.16b; \

/* Input: 'r0:r1', Output: 'a' */
#define REDUCTION(a, r0, r1, rconst, T0, T1, interleave_op1, interleave_op2, \
                  interleave_op3) \
        pmull2 T0.1q, r1.2d, rconst.2d; \
        interleave_op1; \
        ext T1.16b, T0.16b, vZZ.16b, #8; \
        ext T0.16b, vZZ.16b, T0.16b, #8; \
        interleave_op2; \
        eor r1.16b, r1.16b, T1.16b; \
        eor r0.16b, r0.16b, T0.16b; \
        pmull T0.1q, r1.1d, rconst.1d; \
        interleave_op3; \
        eor a.16b, r0.16b, T0.16b;

/* Other functional macros */

#define _(...) __VA_ARGS__
#define __ _()

#define CLEAR_REG(reg) movi reg.16b, #0;

#define VPUSH_ABI \
        stp d8, d9, [sp, #-16]!; \
        CFI_ADJUST_CFA_OFFSET(16); \
        stp d10, d11, [sp, #-16]!; \
        CFI_ADJUST_CFA_OFFSET(16); \
        stp d12, d13, [sp, #-16]!; \
        CFI_ADJUST_CFA_OFFSET(16); \
        stp d14, d15, [sp, #-16]!; \
        CFI_ADJUST_CFA_OFFSET(16);

#define VPOP_ABI \
        ldp d14, d15, [sp], #16; \
        CFI_ADJUST_CFA_OFFSET(-16); \
        ldp d12, d13, [sp], #16; \
        CFI_ADJUST_CFA_OFFSET(-16); \
        ldp d10, d11, [sp], #16; \
        CFI_ADJUST_CFA_OFFSET(-16); \
        ldp d8, d9, [sp], #16; \
        CFI_ADJUST_CFA_OFFSET(-16);

/*
 * unsigned int _gcry_ghash_armv8_ce_pmull (void *gcm_key, byte *result,
 *                                          const byte *buf, size_t nblocks,
 *                                          void *gcm_table);
 */
.align 3
.globl _gcry_ghash_armv8_ce_pmull
ELF(.type  _gcry_ghash_armv8_ce_pmull,%function;)
_gcry_ghash_armv8_ce_pmull:
  /* input:
   *    x0: gcm_key
   *    x1: result/hash
   *    x2: buf
   *    x3: nblocks
   *    x4: gcm_table
   */
  CFI_STARTPROC();

  cbz x3, .Ldo_nothing;

  GET_DATA_POINTER(x5, .Lrconst)

  eor vZZ.16b, vZZ.16b, vZZ.16b
  ld1 {rhash.16b}, [x1]
  ld1 {rh1.16b}, [x0]

  rbit rhash.16b, rhash.16b /* bit-swap */
  ld1r {rrconst.2d}, [x5]

  cmp x3, #6
  b.lo .Less_than_6

  add x6, x4, #64
  VPUSH_ABI

  ld1 {rh2.16b-rh5.16b}, [x4]
  ld1 {rh6.16b}, [x6]

  sub x3, x3, #6

  ld1 {rbuf.16b-rbuf2.16b}, [x2], #(3*16)
  ld1 {rbuf3.16b-rbuf5.16b}, [x2], #(3*16)

  rbit rbuf.16b, rbuf.16b /* bit-swap */
  rbit rbuf1.16b, rbuf1.16b /* bit-swap */
  rbit rbuf2.16b, rbuf2.16b /* bit-swap */
  rbit rbuf3.16b, rbuf3.16b /* bit-swap */
  rbit rbuf4.16b, rbuf4.16b /* bit-swap */
  rbit rbuf5.16b, rbuf5.16b /* bit-swap */
  eor rhash.16b, rhash.16b, rbuf.16b

  cmp x3, #6
  b.lo .Lend_6

.Loop_6:

  /* (in1) * H⁵ => rr0:rr1 */
  /* (in2) * H⁴ => rr2:rr3 */
  /* (in0 ^ hash) * H⁶ => rr4:rr5 */
  PMUL_128x128_3(rr0, rr1, rbuf1, rh5, t0, t1,
                 rr2, rr3, rbuf2, rh4, t2, t3,
                 rr4, rr5, rhash, rh6, t4, t5,
                 _(sub x3, x3, #6))

  ld1 {rbuf.16b-rbuf2.16b}, [x2], #(3*16)
  cmp x3, #6

  eor rr0.16b, rr0.16b, rr2.16b
  eor rr1.16b, rr1.16b, rr3.16b

  /* (in3) * H³ => rr2:rr3 */
  /* (in4) * H² => rr6:rr7 */
  /* (in5) * H¹ => rr8:rr9 */
  PMUL_128x128_3(rr2, rr3, rbuf3, rh3, t0, t1,
                 rr6, rr7, rbuf4, rh2, t2, t3,
                 rr8, rr9, rbuf5, rh1, t4, t5,
                 _(eor rr0.16b, rr0.16b, rr4.16b;
                   eor rr1.16b, rr1.16b, rr5.16b))

  eor rr0.16b, rr0.16b, rr2.16b
  eor rr1.16b, rr1.16b, rr3.16b
  rbit rbuf.16b, rbuf.16b
  eor rr0.16b, rr0.16b, rr6.16b
  eor rr1.16b, rr1.16b, rr7.16b
  rbit rbuf1.16b, rbuf1.16b
  eor rr0.16b, rr0.16b, rr8.16b
  eor rr1.16b, rr1.16b, rr9.16b
  ld1 {rbuf3.16b-rbuf5.16b}, [x2], #(3*16)

  REDUCTION(rhash, rr0, rr1, rrconst, t0, t1,
            _(rbit rbuf2.16b, rbuf2.16b),
            _(rbit rbuf3.16b, rbuf3.16b),
            _(rbit rbuf4.16b, rbuf4.16b))

  rbit rbuf5.16b, rbuf5.16b
  eor rhash.16b, rhash.16b, rbuf.16b

  b.hs .Loop_6

.Lend_6:

  /* (in1) * H⁵ => rr0:rr1 */
  /* (in0 ^ hash) * H⁶ => rr2:rr3 */
  /* (in2) * H⁴ => rr4:rr5 */
  PMUL_128x128_3(rr0, rr1, rbuf1, rh5, t0, t1,
                 rr2, rr3, rhash, rh6, t2, t3,
                 rr4, rr5, rbuf2, rh4, t4, t5,
                 __)
  eor rr0.16b, rr0.16b, rr2.16b
  eor rr1.16b, rr1.16b, rr3.16b
  eor rr0.16b, rr0.16b, rr4.16b
  eor rr1.16b, rr1.16b, rr5.16b

  /* (in3) * H³ => rhash:rbuf */
  /* (in4) * H² => rr6:rr7 */
  /* (in5) * H¹ => rr8:rr9 */
  PMUL_128x128_3(rhash, rbuf, rbuf3, rh3, t0, t1,
                 rr6, rr7, rbuf4, rh2, t2, t3,
                 rr8, rr9, rbuf5, rh1, t4, t5,
                 _(CLEAR_REG(rh4);
                   CLEAR_REG(rh5);
                   CLEAR_REG(rh6)))
  eor rr0.16b, rr0.16b, rhash.16b
  eor rr1.16b, rr1.16b, rbuf.16b
  eor rr0.16b, rr0.16b, rr6.16b
  eor rr1.16b, rr1.16b, rr7.16b
  eor rr0.16b, rr0.16b, rr8.16b
  eor rr1.16b, rr1.16b, rr9.16b

  REDUCTION(rhash, rr0, rr1, rrconst, t0, t1,
            _(CLEAR_REG(rh2);
              CLEAR_REG(rh3);
              CLEAR_REG(rr2);
              CLEAR_REG(rbuf2);
              CLEAR_REG(rbuf3)),
            _(CLEAR_REG(rr3);
              CLEAR_REG(rr4);
              CLEAR_REG(rr5);
              CLEAR_REG(rr6);
              CLEAR_REG(rr7)),
            _(CLEAR_REG(rr8);
              CLEAR_REG(rr9);
              CLEAR_REG(rbuf1);
              CLEAR_REG(rbuf2)))

  CLEAR_REG(rbuf4)
  CLEAR_REG(rbuf5)
  CLEAR_REG(t2)
  CLEAR_REG(t3)
  CLEAR_REG(t4)
  CLEAR_REG(t5)

  VPOP_ABI

  cbz x3, .Ldone

.Less_than_6:
  /* Handle remaining blocks. */

  ld1 {rbuf.16b}, [x2], #16
  sub x3, x3, #1

  rbit rbuf.16b, rbuf.16b /* bit-swap */

  eor rhash.16b, rhash.16b, rbuf.16b

  cbz x3, .Lend

.Loop:
  PMUL_128x128(rr0, rr1, rh1, rhash, t0, t1, _(ld1 {rbuf.16b}, [x2], #16))
  REDUCTION(rhash, rr0, rr1, rrconst, t0, t1,
            _(sub x3, x3, #1),
            _(rbit rbuf.16b, rbuf.16b),
            __)
  eor rhash.16b, rhash.16b, rbuf.16b

  cbnz x3, .Loop

.Lend:
  PMUL_128x128(rr0, rr1, rh1, rhash, t0, t1, _(CLEAR_REG(rbuf)))
  REDUCTION(rhash, rr0, rr1, rrconst, t0, t1, __, _(CLEAR_REG(rh1)), __)

.Ldone:
  CLEAR_REG(rr1)
  CLEAR_REG(rr0)
  rbit rhash.16b, rhash.16b /* bit-swap */
  CLEAR_REG(t0)
  CLEAR_REG(t1)

  st1 {rhash.2d}, [x1]
  CLEAR_REG(rhash)

.Ldo_nothing:
  mov x0, #0
  ret_spec_stop
  CFI_ENDPROC()
ELF(.size _gcry_ghash_armv8_ce_pmull,.-_gcry_ghash_armv8_ce_pmull;)


/*
 * unsigned int _gcry_polyval_armv8_ce_pmull (void *gcm_key, byte *result,
 *                                            const byte *buf, size_t nblocks,
 *                                            void *gcm_table);
 */
.align 3
.globl _gcry_polyval_armv8_ce_pmull
ELF(.type  _gcry_polyval_armv8_ce_pmull,%function;)
_gcry_polyval_armv8_ce_pmull:
  /* input:
   *    x0: gcm_key
   *    x1: result/hash
   *    x2: buf
   *    x3: nblocks
   *    x4: gcm_table
   */
  CFI_STARTPROC();

  cbz x3, .Lpolyval_do_nothing;

  GET_DATA_POINTER(x5, .Lrconst)

  eor vZZ.16b, vZZ.16b, vZZ.16b
  ld1 {rhash.16b}, [x1]
  ld1 {rh1.16b}, [x0]

  rbit rhash.16b, rhash.16b /* bit-swap */
  ld1r {rrconst.2d}, [x5]

  cmp x3, #6
  b.lo .Lpolyval_less_than_6

  add x6, x4, #64
  VPUSH_ABI

  ld1 {rh2.16b-rh5.16b}, [x4]
  ld1 {rh6.16b}, [x6]

  sub x3, x3, #6

  ld1 {rbuf.16b-rbuf2.16b}, [x2], #(3*16)
  ld1 {rbuf3.16b-rbuf5.16b}, [x2], #(3*16)
  rev64 rbuf.16b, rbuf.16b /* byte-swap */
  rev64 rbuf1.16b, rbuf1.16b /* byte-swap */
  rev64 rbuf2.16b, rbuf2.16b /* byte-swap */
  rev64 rbuf3.16b, rbuf3.16b /* byte-swap */
  rev64 rbuf4.16b, rbuf4.16b /* byte-swap */
  rev64 rbuf5.16b, rbuf5.16b /* byte-swap */
  ext rbuf.16b, rbuf.16b, rbuf.16b, #8 /* byte-swap */
  ext rbuf1.16b, rbuf1.16b, rbuf1.16b, #8 /* byte-swap */
  ext rbuf2.16b, rbuf2.16b, rbuf2.16b, #8 /* byte-swap */
  ext rbuf3.16b, rbuf3.16b, rbuf3.16b, #8 /* byte-swap */
  ext rbuf4.16b, rbuf4.16b, rbuf4.16b, #8 /* byte-swap */
  ext rbuf5.16b, rbuf5.16b, rbuf5.16b, #8 /* byte-swap */
  rbit rbuf.16b, rbuf.16b /* bit-swap */
  rbit rbuf1.16b, rbuf1.16b /* bit-swap */
  rbit rbuf2.16b, rbuf2.16b /* bit-swap */
  rbit rbuf3.16b, rbuf3.16b /* bit-swap */
  rbit rbuf4.16b, rbuf4.16b /* bit-swap */
  rbit rbuf5.16b, rbuf5.16b /* bit-swap */
  eor rhash.16b, rhash.16b, rbuf.16b

  cmp x3, #6
  b.lo .Lpolyval_end_6

.Lpolyval_loop_6:

  /* (in1) * H⁵ => rr0:rr1 */
  /* (in2) * H⁴ => rr2:rr3 */
  /* (in0 ^ hash) * H⁶ => rr4:rr5 */
  PMUL_128x128_3(rr0, rr1, rbuf1, rh5, t0, t1,
                 rr2, rr3, rbuf2, rh4, t2, t3,
                 rr4, rr5, rhash, rh6, t4, t5,
                 _(sub x3, x3, #6))

  ld1 {rbuf.16b-rbuf2.16b}, [x2], #(3*16)
  cmp x3, #6

  eor rr0.16b, rr0.16b, rr2.16b
  eor rr1.16b, rr1.16b, rr3.16b

  /* (in3) * H³ => rr2:rr3 */
  /* (in4) * H² => rr6:rr7 */
  /* (in5) * H¹ => rr8:rr9 */
  PMUL_128x128_3(rr2, rr3, rbuf3, rh3, t0, t1,
                 rr6, rr7, rbuf4, rh2, t2, t3,
                 rr8, rr9, rbuf5, rh1, t4, t5,
                 _(eor rr0.16b, rr0.16b, rr4.16b;
                   eor rr1.16b, rr1.16b, rr5.16b))

  rev64 rbuf.16b, rbuf.16b /* byte-swap */
  rev64 rbuf1.16b, rbuf1.16b /* byte-swap */
  rev64 rbuf2.16b, rbuf2.16b /* byte-swap */
  ext rbuf.16b, rbuf.16b, rbuf.16b, #8 /* byte-swap */
  ext rbuf1.16b, rbuf1.16b, rbuf1.16b, #8 /* byte-swap */
  ext rbuf2.16b, rbuf2.16b, rbuf2.16b, #8 /* byte-swap */

  eor rr0.16b, rr0.16b, rr2.16b
  eor rr1.16b, rr1.16b, rr3.16b
  rbit rbuf.16b, rbuf.16b /* bit-swap */
  eor rr0.16b, rr0.16b, rr6.16b
  eor rr1.16b, rr1.16b, rr7.16b
  rbit rbuf1.16b, rbuf1.16b /* bit-swap */
  eor rr0.16b, rr0.16b, rr8.16b
  eor rr1.16b, rr1.16b, rr9.16b
  ld1 {rbuf3.16b-rbuf5.16b}, [x2], #(3*16)

  REDUCTION(rhash, rr0, rr1, rrconst, t0, t1,
            _(rbit rbuf2.16b, rbuf2.16b), /* bit-swap */
            _(rev64 rbuf3.16b, rbuf3.16b), /* byte-swap */
            _(rev64 rbuf4.16b, rbuf4.16b)) /* byte-swap */

  rev64 rbuf5.16b, rbuf5.16b /* byte-swap */
  ext rbuf3.16b, rbuf3.16b, rbuf3.16b, #8 /* byte-swap */

  eor rhash.16b, rhash.16b, rbuf.16b

  ext rbuf4.16b, rbuf4.16b, rbuf4.16b, #8 /* byte-swap */
  ext rbuf5.16b, rbuf5.16b, rbuf5.16b, #8 /* byte-swap */
  rbit rbuf3.16b, rbuf3.16b /* bit-swap */
  rbit rbuf4.16b, rbuf4.16b /* bit-swap */
  rbit rbuf5.16b, rbuf5.16b /* bit-swap */

  b.hs .Lpolyval_loop_6

.Lpolyval_end_6:

  /* (in1) * H⁵ => rr0:rr1 */
  /* (in0 ^ hash) * H⁶ => rr2:rr3 */
  /* (in2) * H⁴ => rr4:rr5 */
  PMUL_128x128_3(rr0, rr1, rbuf1, rh5, t0, t1,
                 rr2, rr3, rhash, rh6, t2, t3,
                 rr4, rr5, rbuf2, rh4, t4, t5,
                 __)
  eor rr0.16b, rr0.16b, rr2.16b
  eor rr1.16b, rr1.16b, rr3.16b
  eor rr0.16b, rr0.16b, rr4.16b
  eor rr1.16b, rr1.16b, rr5.16b

  /* (in3) * H³ => rhash:rbuf */
  /* (in4) * H² => rr6:rr7 */
  /* (in5) * H¹ => rr8:rr9 */
  PMUL_128x128_3(rhash, rbuf, rbuf3, rh3, t0, t1,
                 rr6, rr7, rbuf4, rh2, t2, t3,
                 rr8, rr9, rbuf5, rh1, t4, t5,
                 _(CLEAR_REG(rh4);
                   CLEAR_REG(rh5);
                   CLEAR_REG(rh6)))
  eor rr0.16b, rr0.16b, rhash.16b
  eor rr1.16b, rr1.16b, rbuf.16b
  eor rr0.16b, rr0.16b, rr6.16b
  eor rr1.16b, rr1.16b, rr7.16b
  eor rr0.16b, rr0.16b, rr8.16b
  eor rr1.16b, rr1.16b, rr9.16b

  REDUCTION(rhash, rr0, rr1, rrconst, t0, t1,
            _(CLEAR_REG(rh2);
              CLEAR_REG(rh3);
              CLEAR_REG(rr2);
              CLEAR_REG(rbuf2);
              CLEAR_REG(rbuf3)),
            _(CLEAR_REG(rr3);
              CLEAR_REG(rr4);
              CLEAR_REG(rr5);
              CLEAR_REG(rr6);
              CLEAR_REG(rr7)),
            _(CLEAR_REG(rr8);
              CLEAR_REG(rr9);
              CLEAR_REG(rbuf1);
              CLEAR_REG(rbuf2)))

  CLEAR_REG(rbuf4)
  CLEAR_REG(rbuf5)
  CLEAR_REG(t2)
  CLEAR_REG(t3)
  CLEAR_REG(t4)
  CLEAR_REG(t5)

  VPOP_ABI

  cbz x3, .Lpolyval_done

.Lpolyval_less_than_6:
  /* Handle remaining blocks. */

  ld1 {rbuf.16b}, [x2], #16
  sub x3, x3, #1

  rev64 rbuf.16b, rbuf.16b /* byte-swap */
  ext rbuf.16b, rbuf.16b, rbuf.16b, #8 /* byte-swap */
  rbit rbuf.16b, rbuf.16b /* bit-swap */

  eor rhash.16b, rhash.16b, rbuf.16b

  cbz x3, .Lpolyval_end

.Lpolyval_loop:
  PMUL_128x128(rr0, rr1, rh1, rhash, t0, t1, _(ld1 {rbuf.16b}, [x2], #16))
  REDUCTION(rhash, rr0, rr1, rrconst, t0, t1,
            _(sub x3, x3, #1;
              rev64 rbuf.16b, rbuf.16b), /* byte-swap */
            _(ext rbuf.16b, rbuf.16b, rbuf.16b, #8), /* byte-swap */
            _(rbit rbuf.16b, rbuf.16b)) /* bit-swap */
  eor rhash.16b, rhash.16b, rbuf.16b

  cbnz x3, .Lpolyval_loop

.Lpolyval_end:
  PMUL_128x128(rr0, rr1, rh1, rhash, t0, t1, _(CLEAR_REG(rbuf)))
  REDUCTION(rhash, rr0, rr1, rrconst, t0, t1, __, _(CLEAR_REG(rh1)), __)

.Lpolyval_done:
  CLEAR_REG(rr1)
  CLEAR_REG(rr0)
  rbit rhash.16b, rhash.16b /* bit-swap */
  CLEAR_REG(t0)
  CLEAR_REG(t1)

  st1 {rhash.2d}, [x1]
  CLEAR_REG(rhash)

.Lpolyval_do_nothing:
  mov x0, #0
  ret_spec_stop
  CFI_ENDPROC()
ELF(.size _gcry_polyval_armv8_ce_pmull,.-_gcry_polyval_armv8_ce_pmull;)


/*
 * void _gcry_ghash_setup_armv8_ce_pmull (void *gcm_key, void *gcm_table);
 */
.align 3
.globl _gcry_ghash_setup_armv8_ce_pmull
ELF(.type  _gcry_ghash_setup_armv8_ce_pmull,%function;)
_gcry_ghash_setup_armv8_ce_pmull:
  /* input:
   *	x0: gcm_key
   *	x1: gcm_table
   */
  CFI_STARTPROC()

  GET_DATA_POINTER(x2, .Lrconst)

  eor vZZ.16b, vZZ.16b, vZZ.16b

  /* H¹ */
  ld1 {rh1.16b}, [x0]
  rbit rh1.16b, rh1.16b
  st1 {rh1.16b}, [x0]

  ld1r {rrconst.2d}, [x2]

  /* H² */
  PMUL_128x128(rr0, rr1, rh1, rh1, t0, t1, __)
  REDUCTION(rh2, rr0, rr1, rrconst, t0, t1, __, __, __)

  /* H³ */
  PMUL_128x128(rr0, rr1, rh2, rh1, t0, t1, __)
  REDUCTION(rh3, rr0, rr1, rrconst, t0, t1, __, __, __)

  /* H⁴ */
  PMUL_128x128(rr0, rr1, rh2, rh2, t0, t1, __)
  REDUCTION(rh4, rr0, rr1, rrconst, t0, t1, __, __, __)

  /* H⁵ */
  PMUL_128x128(rr0, rr1, rh2, rh3, t0, t1, __)
  REDUCTION(rh5, rr0, rr1, rrconst, t0, t1, __, __, __)

  /* H⁶ */
  PMUL_128x128(rr0, rr1, rh3, rh3, t0, t1, __)
  REDUCTION(rh6, rr0, rr1, rrconst, t0, t1, __, __, __)

  st1 {rh2.16b-rh4.16b}, [x1], #(3*16)
  st1 {rh5.16b-rh6.16b}, [x1]

  ret_spec_stop
  CFI_ENDPROC()
ELF(.size _gcry_ghash_setup_armv8_ce_pmull,.-_gcry_ghash_setup_armv8_ce_pmull;)

#endif
