/* chacha20-aarch64.S - ARMv8/AArch64 accelerated chacha20 blocks function
 *
 * Copyright (C) 2017-2019 Jussi Kivilinna <jussi.kivilinna@iki.fi>
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

#include "asm-common-aarch64.h"

#if defined(__AARCH64EL__) && \
    defined(HAVE_COMPATIBLE_GCC_AARCH64_PLATFORM_AS) && \
    defined(HAVE_GCC_INLINE_ASM_AARCH64_NEON) && \
    defined(USE_CHACHA20)

.cpu generic+simd

.text

#include "asm-poly1305-aarch64.h"

/* register macros */
#define INPUT     x0
#define DST       x1
#define SRC       x2
#define NBLKS     x3
#define ROUND     x4
#define INPUT_CTR x5
#define INPUT_POS x6
#define CTR       x7

/* vector registers */
#define X0 v16
#define X1 v17
#define X2 v18
#define X3 v19
#define X4 v20
#define X5 v21
#define X6 v22
#define X7 v23
#define X8 v24
#define X9 v25
#define X10 v26
#define X11 v27
#define X12 v28
#define X13 v29
#define X14 v30
#define X15 v31

#define VCTR    v0
#define VTMP0   v1
#define VTMP1   v2
#define VTMP2   v3
#define VTMP3   v4
#define X12_TMP v5
#define X13_TMP v6
#define ROT8    v7

/**********************************************************************
  helper macros
 **********************************************************************/

#define _(...) __VA_ARGS__

#define vpunpckldq(s1, s2, dst) \
	zip1 dst.4s, s2.4s, s1.4s;

#define vpunpckhdq(s1, s2, dst) \
	zip2 dst.4s, s2.4s, s1.4s;

#define vpunpcklqdq(s1, s2, dst) \
	zip1 dst.2d, s2.2d, s1.2d;

#define vpunpckhqdq(s1, s2, dst) \
	zip2 dst.2d, s2.2d, s1.2d;

/* 4x4 32-bit integer matrix transpose */
#define transpose_4x4(x0, x1, x2, x3, t1, t2, t3) \
	vpunpckhdq(x1, x0, t2); \
	vpunpckldq(x1, x0, x0); \
	\
	vpunpckldq(x3, x2, t1); \
	vpunpckhdq(x3, x2, x2); \
	\
	vpunpckhqdq(t1, x0, x1); \
	vpunpcklqdq(t1, x0, x0); \
	\
	vpunpckhqdq(x2, t2, x3); \
	vpunpcklqdq(x2, t2, x2);

#define clear(x) \
	movi x.16b, #0;

/**********************************************************************
  4-way chacha20
 **********************************************************************/

#define XOR(d,s1,s2) \
	eor d.16b, s2.16b, s1.16b;

#define PLUS(ds,s) \
	add ds.4s, ds.4s, s.4s;

#define ROTATE4(dst1,dst2,dst3,dst4,c,src1,src2,src3,src4,iop1,iop2,iop3) \
	shl dst1.4s, src1.4s, #(c);		\
	shl dst2.4s, src2.4s, #(c);		\
	iop1;					\
	shl dst3.4s, src3.4s, #(c);		\
	shl dst4.4s, src4.4s, #(c);		\
	iop2;					\
	sri dst1.4s, src1.4s, #(32 - (c));	\
	sri dst2.4s, src2.4s, #(32 - (c));	\
	iop3;					\
	sri dst3.4s, src3.4s, #(32 - (c));	\
	sri dst4.4s, src4.4s, #(32 - (c));

#define ROTATE4_8(dst1,dst2,dst3,dst4,src1,src2,src3,src4,iop1,iop2,iop3) \
	tbl dst1.16b, {src1.16b}, ROT8.16b;     \
	iop1;					\
	tbl dst2.16b, {src2.16b}, ROT8.16b;	\
	iop2;					\
	tbl dst3.16b, {src3.16b}, ROT8.16b;	\
	iop3;					\
	tbl dst4.16b, {src4.16b}, ROT8.16b;

#define ROTATE4_16(dst1,dst2,dst3,dst4,src1,src2,src3,src4,iop1) \
	rev32 dst1.8h, src1.8h;			\
	rev32 dst2.8h, src2.8h;			\
	iop1;					\
	rev32 dst3.8h, src3.8h;			\
	rev32 dst4.8h, src4.8h;

#define QUARTERROUND4(a1,b1,c1,d1,a2,b2,c2,d2,a3,b3,c3,d3,a4,b4,c4,d4,ign,tmp1,tmp2,tmp3,tmp4,\
		      iop1,iop2,iop3,iop4,iop5,iop6,iop7,iop8,iop9,iop10,iop11,iop12,iop13,iop14,\
		      iop15,iop16,iop17,iop18,iop19,iop20,iop21,iop22,iop23,iop24,iop25,iop26,\
		      iop27,iop28,iop29) \
	PLUS(a1,b1); PLUS(a2,b2); iop1;						\
	PLUS(a3,b3); PLUS(a4,b4); iop2;						\
	    XOR(tmp1,d1,a1); XOR(tmp2,d2,a2); iop3;				\
	    XOR(tmp3,d3,a3); XOR(tmp4,d4,a4); iop4;				\
		ROTATE4_16(d1, d2, d3, d4, tmp1, tmp2, tmp3, tmp4, _(iop5));	\
		iop6;								\
	PLUS(c1,d1); PLUS(c2,d2); iop7;						\
	PLUS(c3,d3); PLUS(c4,d4); iop8;						\
	    XOR(tmp1,b1,c1); XOR(tmp2,b2,c2); iop9;				\
	    XOR(tmp3,b3,c3); XOR(tmp4,b4,c4); iop10;				\
		ROTATE4(b1, b2, b3, b4, 12, tmp1, tmp2, tmp3, tmp4,		\
			_(iop11), _(iop12), _(iop13)); iop14;			\
	PLUS(a1,b1); PLUS(a2,b2); iop15;					\
	PLUS(a3,b3); PLUS(a4,b4); iop16;					\
	    XOR(tmp1,d1,a1); XOR(tmp2,d2,a2); iop17;				\
	    XOR(tmp3,d3,a3); XOR(tmp4,d4,a4); iop18;				\
		ROTATE4_8(d1, d2, d3, d4, tmp1, tmp2, tmp3, tmp4,		\
			  _(iop19), _(iop20), _(iop21)); iop22;			\
	PLUS(c1,d1); PLUS(c2,d2); iop23;					\
	PLUS(c3,d3); PLUS(c4,d4); iop24;					\
	    XOR(tmp1,b1,c1); XOR(tmp2,b2,c2); iop25;				\
	    XOR(tmp3,b3,c3); XOR(tmp4,b4,c4); iop26;				\
		ROTATE4(b1, b2, b3, b4, 7, tmp1, tmp2, tmp3, tmp4,		\
			_(iop27), _(iop28), _(iop29));

.align 4
.globl _gcry_chacha20_aarch64_blocks4_data_inc_counter
_gcry_chacha20_aarch64_blocks4_data_inc_counter:
	.long 0,1,2,3

.align 4
.globl _gcry_chacha20_aarch64_blocks4_data_rot8
_gcry_chacha20_aarch64_blocks4_data_rot8:
	.byte 3,0,1,2
	.byte 7,4,5,6
	.byte 11,8,9,10
	.byte 15,12,13,14

.align 3
.globl _gcry_chacha20_aarch64_blocks4
ELF(.type _gcry_chacha20_aarch64_blocks4,%function;)

_gcry_chacha20_aarch64_blocks4:
	/* input:
	 *	x0: input
	 *	x1: dst
	 *	x2: src
	 *	x3: nblks (multiple of 4)
	 */
	CFI_STARTPROC()

	GET_DATA_POINTER(CTR, _gcry_chacha20_aarch64_blocks4_data_rot8);
	add INPUT_CTR, INPUT, #(12*4);
	ld1 {ROT8.16b}, [CTR];
	GET_DATA_POINTER(CTR, _gcry_chacha20_aarch64_blocks4_data_inc_counter);
	mov INPUT_POS, INPUT;
	ld1 {VCTR.16b}, [CTR];

.Loop4:
	/* Construct counter vectors X12 and X13 */

	ld1 {X15.16b}, [INPUT_CTR];
	mov ROUND, #20;
	ld1 {VTMP1.16b-VTMP3.16b}, [INPUT_POS];

	dup X12.4s, X15.s[0];
	dup X13.4s, X15.s[1];
	ldr CTR, [INPUT_CTR];
	add X12.4s, X12.4s, VCTR.4s;
	dup X0.4s, VTMP1.s[0];
	dup X1.4s, VTMP1.s[1];
	dup X2.4s, VTMP1.s[2];
	dup X3.4s, VTMP1.s[3];
	dup X14.4s, X15.s[2];
	cmhi VTMP0.4s, VCTR.4s, X12.4s;
	dup X15.4s, X15.s[3];
	add CTR, CTR, #4; /* Update counter */
	dup X4.4s, VTMP2.s[0];
	dup X5.4s, VTMP2.s[1];
	dup X6.4s, VTMP2.s[2];
	dup X7.4s, VTMP2.s[3];
	sub X13.4s, X13.4s, VTMP0.4s;
	dup X8.4s, VTMP3.s[0];
	dup X9.4s, VTMP3.s[1];
	dup X10.4s, VTMP3.s[2];
	dup X11.4s, VTMP3.s[3];
	mov X12_TMP.16b, X12.16b;
	mov X13_TMP.16b, X13.16b;
	str CTR, [INPUT_CTR];

.Lround2:
	subs ROUND, ROUND, #2
	QUARTERROUND4(X0, X4,  X8, X12,   X1, X5,  X9, X13,
		      X2, X6, X10, X14,   X3, X7, X11, X15,
		      tmp:=,VTMP0,VTMP1,VTMP2,VTMP3,
		      ,,,,,,,,,,,,,,,,,,,,,,,,,,,,)
	QUARTERROUND4(X0, X5, X10, X15,   X1, X6, X11, X12,
		      X2, X7,  X8, X13,   X3, X4,  X9, X14,
		      tmp:=,VTMP0,VTMP1,VTMP2,VTMP3,
		      ,,,,,,,,,,,,,,,,,,,,,,,,,,,,)
	b.ne .Lround2;

	ld1 {VTMP0.16b, VTMP1.16b}, [INPUT_POS], #32;

	PLUS(X12, X12_TMP);        /* INPUT + 12 * 4 + counter */
	PLUS(X13, X13_TMP);        /* INPUT + 13 * 4 + counter */

	dup VTMP2.4s, VTMP0.s[0]; /* INPUT + 0 * 4 */
	dup VTMP3.4s, VTMP0.s[1]; /* INPUT + 1 * 4 */
	dup X12_TMP.4s, VTMP0.s[2]; /* INPUT + 2 * 4 */
	dup X13_TMP.4s, VTMP0.s[3]; /* INPUT + 3 * 4 */
	PLUS(X0, VTMP2);
	PLUS(X1, VTMP3);
	PLUS(X2, X12_TMP);
	PLUS(X3, X13_TMP);

	dup VTMP2.4s, VTMP1.s[0]; /* INPUT + 4 * 4 */
	dup VTMP3.4s, VTMP1.s[1]; /* INPUT + 5 * 4 */
	dup X12_TMP.4s, VTMP1.s[2]; /* INPUT + 6 * 4 */
	dup X13_TMP.4s, VTMP1.s[3]; /* INPUT + 7 * 4 */
	ld1 {VTMP0.16b, VTMP1.16b}, [INPUT_POS];
	mov INPUT_POS, INPUT;
	PLUS(X4, VTMP2);
	PLUS(X5, VTMP3);
	PLUS(X6, X12_TMP);
	PLUS(X7, X13_TMP);

	dup VTMP2.4s, VTMP0.s[0]; /* INPUT + 8 * 4 */
	dup VTMP3.4s, VTMP0.s[1]; /* INPUT + 9 * 4 */
	dup X12_TMP.4s, VTMP0.s[2]; /* INPUT + 10 * 4 */
	dup X13_TMP.4s, VTMP0.s[3]; /* INPUT + 11 * 4 */
	dup VTMP0.4s, VTMP1.s[2]; /* INPUT + 14 * 4 */
	dup VTMP1.4s, VTMP1.s[3]; /* INPUT + 15 * 4 */
	PLUS(X8, VTMP2);
	PLUS(X9, VTMP3);
	PLUS(X10, X12_TMP);
	PLUS(X11, X13_TMP);
	PLUS(X14, VTMP0);
	PLUS(X15, VTMP1);

	transpose_4x4(X0, X1, X2, X3, VTMP0, VTMP1, VTMP2);
	transpose_4x4(X4, X5, X6, X7, VTMP0, VTMP1, VTMP2);
	transpose_4x4(X8, X9, X10, X11, VTMP0, VTMP1, VTMP2);
	transpose_4x4(X12, X13, X14, X15, VTMP0, VTMP1, VTMP2);

	subs NBLKS, NBLKS, #4;

	ld1 {VTMP0.16b-VTMP3.16b}, [SRC], #64;
	ld1 {X12_TMP.16b-X13_TMP.16b}, [SRC], #32;
	eor VTMP0.16b, X0.16b, VTMP0.16b;
	eor VTMP1.16b, X4.16b, VTMP1.16b;
	eor VTMP2.16b, X8.16b, VTMP2.16b;
	eor VTMP3.16b, X12.16b, VTMP3.16b;
	eor X12_TMP.16b, X1.16b, X12_TMP.16b;
	eor X13_TMP.16b, X5.16b, X13_TMP.16b;
	st1 {VTMP0.16b-VTMP3.16b}, [DST], #64;
	ld1 {VTMP0.16b-VTMP3.16b}, [SRC], #64;
	st1 {X12_TMP.16b-X13_TMP.16b}, [DST], #32;
	ld1 {X12_TMP.16b-X13_TMP.16b}, [SRC], #32;
	eor VTMP0.16b, X9.16b, VTMP0.16b;
	eor VTMP1.16b, X13.16b, VTMP1.16b;
	eor VTMP2.16b, X2.16b, VTMP2.16b;
	eor VTMP3.16b, X6.16b, VTMP3.16b;
	eor X12_TMP.16b, X10.16b, X12_TMP.16b;
	eor X13_TMP.16b, X14.16b, X13_TMP.16b;
	st1 {VTMP0.16b-VTMP3.16b}, [DST], #64;
	ld1 {VTMP0.16b-VTMP3.16b}, [SRC], #64;
	st1 {X12_TMP.16b-X13_TMP.16b}, [DST], #32;
	eor VTMP0.16b, X3.16b, VTMP0.16b;
	eor VTMP1.16b, X7.16b, VTMP1.16b;
	eor VTMP2.16b, X11.16b, VTMP2.16b;
	eor VTMP3.16b, X15.16b, VTMP3.16b;
	st1 {VTMP0.16b-VTMP3.16b}, [DST], #64;

	b.ne .Loop4;

	/* clear the used vector registers and stack */
	clear(VTMP0);
	clear(VTMP1);
	clear(VTMP2);
	clear(VTMP3);
	clear(X12_TMP);
	clear(X13_TMP);
	clear(X0);
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

	eor x0, x0, x0
	ret_spec_stop
	CFI_ENDPROC()
ELF(.size _gcry_chacha20_aarch64_blocks4, .-_gcry_chacha20_aarch64_blocks4;)

/**********************************************************************
  4-way stitched chacha20-poly1305
 **********************************************************************/

.align 3
.globl _gcry_chacha20_poly1305_aarch64_blocks4
ELF(.type _gcry_chacha20_poly1305_aarch64_blocks4,%function;)

_gcry_chacha20_poly1305_aarch64_blocks4:
	/* input:
	 *	x0: input
	 *	x1: dst
	 *	x2: src
	 *	x3: nblks (multiple of 4)
	 *	x4: poly1305-state
	 *	x5: poly1305-src
	 */
	CFI_STARTPROC()
	POLY1305_PUSH_REGS()

	mov POLY_RSTATE, x4;
	mov POLY_RSRC, x5;

	GET_DATA_POINTER(CTR, _gcry_chacha20_aarch64_blocks4_data_rot8);
	add INPUT_CTR, INPUT, #(12*4);
	ld1 {ROT8.16b}, [CTR];
	GET_DATA_POINTER(CTR, _gcry_chacha20_aarch64_blocks4_data_inc_counter);
	mov INPUT_POS, INPUT;
	ld1 {VCTR.16b}, [CTR];

	POLY1305_LOAD_STATE()

.Loop_poly4:
	/* Construct counter vectors X12 and X13 */

	ld1 {X15.16b}, [INPUT_CTR];
	ld1 {VTMP1.16b-VTMP3.16b}, [INPUT_POS];

	dup X12.4s, X15.s[0];
	dup X13.4s, X15.s[1];
	ldr CTR, [INPUT_CTR];
	add X12.4s, X12.4s, VCTR.4s;
	dup X0.4s, VTMP1.s[0];
	dup X1.4s, VTMP1.s[1];
	dup X2.4s, VTMP1.s[2];
	dup X3.4s, VTMP1.s[3];
	dup X14.4s, X15.s[2];
	cmhi VTMP0.4s, VCTR.4s, X12.4s;
	dup X15.4s, X15.s[3];
	add CTR, CTR, #4; /* Update counter */
	dup X4.4s, VTMP2.s[0];
	dup X5.4s, VTMP2.s[1];
	dup X6.4s, VTMP2.s[2];
	dup X7.4s, VTMP2.s[3];
	sub X13.4s, X13.4s, VTMP0.4s;
	dup X8.4s, VTMP3.s[0];
	dup X9.4s, VTMP3.s[1];
	dup X10.4s, VTMP3.s[2];
	dup X11.4s, VTMP3.s[3];
	mov X12_TMP.16b, X12.16b;
	mov X13_TMP.16b, X13.16b;
	str CTR, [INPUT_CTR];

	mov ROUND, #20
.Lround4_with_poly1305_outer:
	mov POLY_CHACHA_ROUND, #6;
.Lround4_with_poly1305_inner1:
		      POLY1305_BLOCK_PART1(0 * 16)
	QUARTERROUND4(X0, X4,  X8, X12,   X1, X5,  X9, X13,
		      X2, X6, X10, X14,   X3, X7, X11, X15,
		      tmp:=,VTMP0,VTMP1,VTMP2,VTMP3,
		      POLY1305_BLOCK_PART2(0 * 16),
		      POLY1305_BLOCK_PART3(),
		      POLY1305_BLOCK_PART4(),
		      POLY1305_BLOCK_PART5(),
		      POLY1305_BLOCK_PART6(),
		      POLY1305_BLOCK_PART7(),
		      POLY1305_BLOCK_PART8(),
		      POLY1305_BLOCK_PART9(),
		      POLY1305_BLOCK_PART10(),
		      POLY1305_BLOCK_PART11(),
		      POLY1305_BLOCK_PART12(),
		      POLY1305_BLOCK_PART13(),
		      POLY1305_BLOCK_PART14(),
		      POLY1305_BLOCK_PART15(),
		      POLY1305_BLOCK_PART16(),
		      POLY1305_BLOCK_PART17(),
		      POLY1305_BLOCK_PART18(),
		      POLY1305_BLOCK_PART19(),
		      POLY1305_BLOCK_PART20(),
		      POLY1305_BLOCK_PART21(),
		      POLY1305_BLOCK_PART22(),
		      POLY1305_BLOCK_PART23(),
		      POLY1305_BLOCK_PART24(),
		      POLY1305_BLOCK_PART25(),
		      POLY1305_BLOCK_PART26(),
		      POLY1305_BLOCK_PART27(),
		      POLY1305_BLOCK_PART28(),
		      POLY1305_BLOCK_PART29(),
		      POLY1305_BLOCK_PART1(1 * 16))
		      POLY1305_BLOCK_PART2(1 * 16)
	QUARTERROUND4(X0, X5, X10, X15,   X1, X6, X11, X12,
		      X2, X7,  X8, X13,   X3, X4,  X9, X14,
		      tmp:=,VTMP0,VTMP1,VTMP2,VTMP3,
		      _(add POLY_RSRC, POLY_RSRC, #(2*16)),
		      POLY1305_BLOCK_PART3(),
		      POLY1305_BLOCK_PART4(),
		      POLY1305_BLOCK_PART5(),
		      POLY1305_BLOCK_PART6(),
		      POLY1305_BLOCK_PART7(),
		      POLY1305_BLOCK_PART8(),
		      POLY1305_BLOCK_PART9(),
		      POLY1305_BLOCK_PART10(),
		      POLY1305_BLOCK_PART11(),
		      POLY1305_BLOCK_PART12(),
		      POLY1305_BLOCK_PART13(),
		      POLY1305_BLOCK_PART14(),
		      POLY1305_BLOCK_PART15(),
		      POLY1305_BLOCK_PART16(),
		      POLY1305_BLOCK_PART17(),
		      POLY1305_BLOCK_PART18(),
		      POLY1305_BLOCK_PART19(),
		      POLY1305_BLOCK_PART20(),
		      POLY1305_BLOCK_PART21(),
		      POLY1305_BLOCK_PART22(),
		      POLY1305_BLOCK_PART23(),
		      POLY1305_BLOCK_PART24(),
		      POLY1305_BLOCK_PART25(),
		      POLY1305_BLOCK_PART26(),
		      POLY1305_BLOCK_PART27(),
		      POLY1305_BLOCK_PART28(),
		      POLY1305_BLOCK_PART29(),
		      _(subs POLY_CHACHA_ROUND, POLY_CHACHA_ROUND, #2));
	b.ne .Lround4_with_poly1305_inner1;

	mov POLY_CHACHA_ROUND, #4;
.Lround4_with_poly1305_inner2:
		      POLY1305_BLOCK_PART1(0 * 16)
	QUARTERROUND4(X0, X4,  X8, X12,   X1, X5,  X9, X13,
		      X2, X6, X10, X14,   X3, X7, X11, X15,
		      tmp:=,VTMP0,VTMP1,VTMP2,VTMP3,,
		      POLY1305_BLOCK_PART2(0 * 16),,
		      _(add POLY_RSRC, POLY_RSRC, #(1*16)),,
		      POLY1305_BLOCK_PART3(),,
		      POLY1305_BLOCK_PART4(),,
		      POLY1305_BLOCK_PART5(),,
		      POLY1305_BLOCK_PART6(),,
		      POLY1305_BLOCK_PART7(),,
		      POLY1305_BLOCK_PART8(),,
		      POLY1305_BLOCK_PART9(),,
		      POLY1305_BLOCK_PART10(),,
		      POLY1305_BLOCK_PART11(),,
		      POLY1305_BLOCK_PART12(),,
		      POLY1305_BLOCK_PART13(),,
		      POLY1305_BLOCK_PART14(),)
		      POLY1305_BLOCK_PART15()
	QUARTERROUND4(X0, X5, X10, X15,   X1, X6, X11, X12,
		      X2, X7,  X8, X13,   X3, X4,  X9, X14,
		      tmp:=,VTMP0,VTMP1,VTMP2,VTMP3,
		      POLY1305_BLOCK_PART16(),,
		      POLY1305_BLOCK_PART17(),,
		      POLY1305_BLOCK_PART18(),,
		      POLY1305_BLOCK_PART19(),,
		      POLY1305_BLOCK_PART20(),,
		      POLY1305_BLOCK_PART21(),,
		      POLY1305_BLOCK_PART22(),,
		      POLY1305_BLOCK_PART23(),,
		      POLY1305_BLOCK_PART24(),,
		      POLY1305_BLOCK_PART25(),,
		      POLY1305_BLOCK_PART26(),,
		      POLY1305_BLOCK_PART27(),,
		      POLY1305_BLOCK_PART28(),,
		      POLY1305_BLOCK_PART29(),
		      _(subs POLY_CHACHA_ROUND, POLY_CHACHA_ROUND, #2),)
	b.ne .Lround4_with_poly1305_inner2;

	subs ROUND, ROUND, #10
	b.ne .Lround4_with_poly1305_outer;

	ld1 {VTMP0.16b, VTMP1.16b}, [INPUT_POS], #32;

	PLUS(X12, X12_TMP);        /* INPUT + 12 * 4 + counter */
	PLUS(X13, X13_TMP);        /* INPUT + 13 * 4 + counter */

	dup VTMP2.4s, VTMP0.s[0]; /* INPUT + 0 * 4 */
	dup VTMP3.4s, VTMP0.s[1]; /* INPUT + 1 * 4 */
	dup X12_TMP.4s, VTMP0.s[2]; /* INPUT + 2 * 4 */
	dup X13_TMP.4s, VTMP0.s[3]; /* INPUT + 3 * 4 */
	PLUS(X0, VTMP2);
	PLUS(X1, VTMP3);
	PLUS(X2, X12_TMP);
	PLUS(X3, X13_TMP);

	dup VTMP2.4s, VTMP1.s[0]; /* INPUT + 4 * 4 */
	dup VTMP3.4s, VTMP1.s[1]; /* INPUT + 5 * 4 */
	dup X12_TMP.4s, VTMP1.s[2]; /* INPUT + 6 * 4 */
	dup X13_TMP.4s, VTMP1.s[3]; /* INPUT + 7 * 4 */
	ld1 {VTMP0.16b, VTMP1.16b}, [INPUT_POS];
	mov INPUT_POS, INPUT;
	PLUS(X4, VTMP2);
	PLUS(X5, VTMP3);
	PLUS(X6, X12_TMP);
	PLUS(X7, X13_TMP);

	dup VTMP2.4s, VTMP0.s[0]; /* INPUT + 8 * 4 */
	dup VTMP3.4s, VTMP0.s[1]; /* INPUT + 9 * 4 */
	dup X12_TMP.4s, VTMP0.s[2]; /* INPUT + 10 * 4 */
	dup X13_TMP.4s, VTMP0.s[3]; /* INPUT + 11 * 4 */
	dup VTMP0.4s, VTMP1.s[2]; /* INPUT + 14 * 4 */
	dup VTMP1.4s, VTMP1.s[3]; /* INPUT + 15 * 4 */
	PLUS(X8, VTMP2);
	PLUS(X9, VTMP3);
	PLUS(X10, X12_TMP);
	PLUS(X11, X13_TMP);
	PLUS(X14, VTMP0);
	PLUS(X15, VTMP1);

	transpose_4x4(X0, X1, X2, X3, VTMP0, VTMP1, VTMP2);
	transpose_4x4(X4, X5, X6, X7, VTMP0, VTMP1, VTMP2);
	transpose_4x4(X8, X9, X10, X11, VTMP0, VTMP1, VTMP2);
	transpose_4x4(X12, X13, X14, X15, VTMP0, VTMP1, VTMP2);

	subs NBLKS, NBLKS, #4;

	ld1 {VTMP0.16b-VTMP3.16b}, [SRC], #64;
	ld1 {X12_TMP.16b-X13_TMP.16b}, [SRC], #32;
	eor VTMP0.16b, X0.16b, VTMP0.16b;
	eor VTMP1.16b, X4.16b, VTMP1.16b;
	eor VTMP2.16b, X8.16b, VTMP2.16b;
	eor VTMP3.16b, X12.16b, VTMP3.16b;
	eor X12_TMP.16b, X1.16b, X12_TMP.16b;
	eor X13_TMP.16b, X5.16b, X13_TMP.16b;
	st1 {VTMP0.16b-VTMP3.16b}, [DST], #64;
	ld1 {VTMP0.16b-VTMP3.16b}, [SRC], #64;
	st1 {X12_TMP.16b-X13_TMP.16b}, [DST], #32;
	ld1 {X12_TMP.16b-X13_TMP.16b}, [SRC], #32;
	eor VTMP0.16b, X9.16b, VTMP0.16b;
	eor VTMP1.16b, X13.16b, VTMP1.16b;
	eor VTMP2.16b, X2.16b, VTMP2.16b;
	eor VTMP3.16b, X6.16b, VTMP3.16b;
	eor X12_TMP.16b, X10.16b, X12_TMP.16b;
	eor X13_TMP.16b, X14.16b, X13_TMP.16b;
	st1 {VTMP0.16b-VTMP3.16b}, [DST], #64;
	ld1 {VTMP0.16b-VTMP3.16b}, [SRC], #64;
	st1 {X12_TMP.16b-X13_TMP.16b}, [DST], #32;
	eor VTMP0.16b, X3.16b, VTMP0.16b;
	eor VTMP1.16b, X7.16b, VTMP1.16b;
	eor VTMP2.16b, X11.16b, VTMP2.16b;
	eor VTMP3.16b, X15.16b, VTMP3.16b;
	st1 {VTMP0.16b-VTMP3.16b}, [DST], #64;

	b.ne .Loop_poly4;

	POLY1305_STORE_STATE()

	/* clear the used vector registers and stack */
	clear(VTMP0);
	clear(VTMP1);
	clear(VTMP2);
	clear(VTMP3);
	clear(X12_TMP);
	clear(X13_TMP);
	clear(X0);
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

	eor x0, x0, x0
	POLY1305_POP_REGS()
	ret_spec_stop
	CFI_ENDPROC()
ELF(.size _gcry_chacha20_poly1305_aarch64_blocks4, .-_gcry_chacha20_poly1305_aarch64_blocks4;)

#endif
