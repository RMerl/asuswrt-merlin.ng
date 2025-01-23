/* chacha20-armv7-neon.S  -  ARMv7 NEON implementation of ChaCha20 cipher
 *
 * Copyright (C) 2017,2018 Jussi Kivilinna <jussi.kivilinna@iki.fi>
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
 * Based on D. J. Bernstein reference implementation at
 * http://cr.yp.to/chacha.html:
 *
 * chacha-regs.c version 20080118
 * D. J. Bernstein
 * Public domain.
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

/* register macros */
#define INPUT r0
#define DST   r1
#define SRC   r2
#define NBLKS r3
#define ROUND r4

/* stack structure */
#define STACK_VEC_X12 (16)
#define STACK_VEC_X13 (STACK_VEC_X12 + 16)
#define STACK_TMP     (STACK_VEC_X13 + 16)
#define STACK_TMP1    (16 + STACK_TMP)
#define STACK_TMP2    (16 + STACK_TMP1)

#define STACK_MAX     (16 + STACK_TMP2)

/* vector registers */
#define X0 q0
#define X1 q1
#define X2 q2
#define X3 q3
#define X4 q4
#define X5 q5
#define X6 q6
#define X7 q7
#define X8 q8
#define X9 q9
#define X10 q10
#define X11 q11
#define X12 q12
#define X13 q13
#define X14 q14
#define X15 q15

#define X0l d0
#define X1l d2
#define X2l d4
#define X3l d6
#define X4l d8
#define X5l d10
#define X6l d12
#define X7l d14
#define X8l d16
#define X9l d18
#define X10l d20
#define X11l d22
#define X12l d24
#define X13l d26
#define X14l d28
#define X15l d30

#define X0h d1
#define X1h d3
#define X2h d5
#define X3h d7
#define X4h d9
#define X5h d11
#define X6h d13
#define X7h d15
#define X8h d17
#define X9h d19
#define X10h d21
#define X11h d23
#define X12h d25
#define X13h d27
#define X14h d29
#define X15h d31

/**********************************************************************
  helper macros
 **********************************************************************/

/* 4x4 32-bit integer matrix transpose */
#define transpose_4x4_part1(_q0, _q1, _q2, _q3)	\
	vtrn.32 _q0, _q1;			\
	vtrn.32 _q2, _q3;
#define transpose_4x4_part2(_q0, _q1, _q2, _q3)	\
	vswp _q0##h, _q2##l;			\
	vswp _q1##h, _q3##l;

#define clear(x) vmov.i8 x, #0;

/**********************************************************************
  4-way chacha20
 **********************************************************************/

#define ROTATE2(dst1,dst2,c,src1,src2)		\
	vshl.u32 dst1, src1, #(c);		\
	vshl.u32 dst2, src2, #(c);		\
	vsri.u32 dst1, src1, #(32 - (c));	\
	vsri.u32 dst2, src2, #(32 - (c));

#define ROTATE2_16(dst1,dst2,src1,src2)		\
	vrev32.16 dst1, src1;			\
	vrev32.16 dst2, src2;

#define XOR(d,s1,s2) \
	veor d, s2, s1;

#define PLUS(ds,s) \
	vadd.u32 ds, ds, s;

#define QUARTERROUND2(a1,b1,c1,d1,a2,b2,c2,d2,ign,tmp1,tmp2)		\
	PLUS(a1,b1); PLUS(a2,b2); XOR(tmp1,d1,a1); XOR(tmp2,d2,a2);	\
	    ROTATE2_16(d1, d2, tmp1, tmp2);				\
	PLUS(c1,d1); PLUS(c2,d2); XOR(tmp1,b1,c1); XOR(tmp2,b2,c2);	\
	    ROTATE2(b1, b2, 12, tmp1, tmp2);				\
	PLUS(a1,b1); PLUS(a2,b2); XOR(tmp1,d1,a1); XOR(tmp2,d2,a2);	\
	    ROTATE2(d1, d2,  8, tmp1, tmp2);				\
	PLUS(c1,d1); PLUS(c2,d2); XOR(tmp1,b1,c1); XOR(tmp2,b2,c2);	\
	    ROTATE2(b1, b2,  7, tmp1, tmp2);

chacha20_data:
.align 4
.Linc_counter:
	.long 0,1,2,3

.align 3
.globl _gcry_chacha20_armv7_neon_blocks4
.type _gcry_chacha20_armv7_neon_blocks4,%function;

_gcry_chacha20_armv7_neon_blocks4:
	/* input:
	 *	r0: input
	 *	r1: dst
	 *	r2: src
	 *	r3: nblks (multiple of 4)
	 */

	vpush {q4-q7};
	push {r4-r12,lr};

	mov r12, sp

	mov r6, sp;
	sub r6, r6, #(STACK_MAX);
	and r6, r6, #(~15);
	mov sp, r6;
	GET_DATA_POINTER(r9, .Linc_counter, lr);
	add lr, INPUT, #(12*4);
	add r8, sp, #STACK_VEC_X12;

.Loop4:
	mov ROUND, #20;

	/* Construct counter vectors X12 and X13 */

	vld1.8 {X15}, [lr];
	mov lr, INPUT;
	vld1.8 {X8}, [r9];
	vdup.32 X12, X15l[0];
	vdup.32 X13, X15l[1];
	vld1.8 {X3}, [lr]!;
	vadd.u32 X12, X12, X8;
	vdup.32 X0, X3l[0];
	vdup.32 X1, X3l[1];
	vdup.32 X2, X3h[0];
	vcgt.u32 X8, X8, X12;
	vdup.32 X3, X3h[1];
	vdup.32 X14, X15h[0];
	vdup.32 X15, X15h[1];
	vsub.u32 X13, X13, X8;
	vld1.8 {X7}, [lr]!;
	vld1.8 {X11}, [lr];
	vst1.8 {X12, X13}, [r8];
	vdup.32 X4, X7l[0];
	vdup.32 X5, X7l[1];
	vdup.32 X6, X7h[0];
	vdup.32 X7, X7h[1];
	vdup.32 X8, X11l[0];
	vdup.32 X9, X11l[1];
	vdup.32 X10, X11h[0];
	vdup.32 X11, X11h[1];

	add r7, sp, #STACK_TMP2;
	add r6, sp, #STACK_TMP1;
	add r5, sp, #STACK_TMP;
	vst1.8 {X15}, [r6];
	vst1.8 {X11}, [r5];

	mov lr, INPUT;
.Lround2:
	subs ROUND, ROUND, #2
	QUARTERROUND2(X0, X4,  X8, X12,   X1, X5,  X9, X13, tmp:=,X11,X15)
	vld1.8 {X11}, [r5];
	vld1.8 {X15}, [r6];
	vst1.8 {X8}, [r5];
	vst1.8 {X9}, [r6];
	QUARTERROUND2(X2, X6, X10, X14,   X3, X7, X11, X15, tmp:=,X8,X9)
	QUARTERROUND2(X0, X5, X10, X15,   X1, X6, X11, X12, tmp:=,X8,X9)
	vld1.8 {X8}, [r5];
	vld1.8 {X9}, [r6];
	vst1.8 {X11}, [r5];
	vst1.8 {X15}, [r6];
	QUARTERROUND2(X2, X7,  X8, X13,   X3, X4,  X9, X14, tmp:=,X11,X15)
	bne .Lround2;

	vld1.8 {X11}, [lr]!;
	vst1.8 {X14}, [r7];

	vdup.32 X14, X11l[0]; /* INPUT + 0 * 4 */
	vdup.32 X15, X11l[1]; /* INPUT + 1 * 4 */
	PLUS(X0, X14);
	PLUS(X1, X15);
	vdup.32 X14, X11h[0]; /* INPUT + 2 * 4 */
	vdup.32 X15, X11h[1]; /* INPUT + 3 * 4 */
	PLUS(X2, X14);
	PLUS(X3, X15);

	vld1.8 {X11}, [r5];
	vld1.8 {X15}, [r6];
	vst1.8 {X0}, [r5];
	vld1.8 {X0}, [lr]!;
	vst1.8 {X1}, [r6];

	vdup.32 X14, X0l[0]; /* INPUT + 4 * 4 */
	vdup.32  X1, X0l[1]; /* INPUT + 5 * 4 */
	PLUS(X4, X14);
	PLUS(X5, X1);
	vdup.32 X14, X0h[0]; /* INPUT + 6 * 4 */
	vdup.32  X1, X0h[1]; /* INPUT + 7 * 4 */
	PLUS(X6, X14);
	PLUS(X7, X1);

	vld1.8 {X0}, [lr]!;

	vdup.32 X14, X0l[0]; /* INPUT + 8 * 4 */
	vdup.32  X1, X0l[1]; /* INPUT + 9 * 4 */
	PLUS(X8, X14);
	PLUS(X9, X1);
	vdup.32 X14, X0h[0]; /* INPUT + 10 * 4 */
	vdup.32  X1, X0h[1]; /* INPUT + 11 * 4 */
	PLUS(X10, X14);
	PLUS(X11, X1);

	vld1.8 {X0}, [lr];
	add lr, INPUT, #(12*4)
	vld1.8 {X14}, [r7];

	vdup.32 X1, X0h[0]; /* INPUT + 10 * 4 */
	ldm lr, {r10, r11}; /* Update counter */
	vdup.32 X0, X0h[1]; /* INPUT + 11 * 4 */
	PLUS(X14, X1);
	PLUS(X15, X0);
	adds r10, r10, #4;  /* Update counter */
	vld1.8 {X0, X1}, [r8];

	PLUS(X12, X0);
	vld1.8 {X0}, [r5];
	PLUS(X13, X1);
	adc r11, r11, #0;   /* Update counter */

	vld1.8 {X1}, [r6];
	stm lr, {r10, r11}; /* Update counter */
	transpose_4x4_part1(X0, X1, X2, X3);
	transpose_4x4_part1(X4, X5, X6, X7);
	transpose_4x4_part1(X8, X9, X10, X11);
	transpose_4x4_part1(X12, X13, X14, X15);
	transpose_4x4_part2(X0, X1, X2, X3);
	transpose_4x4_part2(X4, X5, X6, X7);
	transpose_4x4_part2(X8, X9, X10, X11);
	transpose_4x4_part2(X12, X13, X14, X15);

	subs NBLKS, NBLKS, #4;

	vst1.8 {X10}, [r5];
	add lr, INPUT, #(12*4)
	vst1.8 {X11}, [r6];
	vld1.8 {X10, X11}, [SRC]!;
	veor X10, X0, X10;
	vld1.8 {X0}, [SRC]!;
	veor X11, X4, X11;
	vld1.8 {X4}, [SRC]!;
	vst1.8 {X10, X11}, [DST]!;
	vld1.8 {X10, X11}, [SRC]!;
	veor X0, X8, X0;
	veor X4, X12, X4;
	veor X10, X1, X10;
	veor X11, X5, X11;
	vst1.8 {X0}, [DST]!;
	vld1.8 {X0, X1}, [SRC]!;
	vst1.8 {X4}, [DST]!;
	vld1.8 {X4, X5}, [SRC]!;
	vst1.8 {X10, X11}, [DST]!;
	vld1.8 {X10}, [r5];
	vld1.8 {X11}, [r6];
	veor X0, X9, X0;
	vld1.8 {X8, X9}, [SRC]!;
	veor X1, X13, X1;
	vld1.8 {X12, X13}, [SRC]!;
	veor X4, X2, X4;
	veor X5, X6, X5;
	vst1.8 {X0, X1}, [DST]!;
	vld1.8 {X0, X1}, [SRC]!;
	vst1.8 {X4, X5}, [DST]!;
	veor X8, X10, X8;
	veor X9, X14, X9;
	veor X12, X3, X12;
	veor X13, X7, X13;
	veor X0, X11, X0;
	veor X1, X15, X1;
	vst1.8 {X8, X9}, [DST]!;
	vst1.8 {X12, X13}, [DST]!;
	vst1.8 {X0, X1}, [DST]!;

	bne .Loop4;

	/* clear the used vector registers and stack */
	clear(X0);
	vst1.8 {X0}, [r5];
	vst1.8 {X0}, [r6];
	vst1.8 {X0}, [r7];
	vst1.8 {X0}, [r8]!;
	vst1.8 {X0}, [r8];

	mov sp, r12
	clear(X1);
	clear(X2);
	clear(X3);
	clear(X4);
	clear(X5);
	clear(X6);
	clear(X7);
	clear(X8);
	clear(X9);
	clear(X10);
	clear(X11);
	clear(X12);
	clear(X13);
	clear(X14);
	clear(X15);

	pop {r4-r12,lr}
	vpop {q4-q7}
	eor r0, r0, r0
	bx lr
.size _gcry_chacha20_armv7_neon_blocks4, .-_gcry_chacha20_armv7_neon_blocks4;

#endif
