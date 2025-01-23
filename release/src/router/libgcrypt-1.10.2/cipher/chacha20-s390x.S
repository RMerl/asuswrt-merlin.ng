/* chacha20-s390x.S  -  zSeries implementation of ChaCha20 cipher
 *
 * Copyright (C) 2020 Jussi Kivilinna <jussi.kivilinna@iki.fi>
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

#if defined (__s390x__) && __GNUC__ >= 4 && __ARCH__ >= 9
#include <config.h>
#if defined(HAVE_GCC_INLINE_ASM_S390X_VX)

#include "asm-common-s390x.h"
#include "asm-poly1305-s390x.h"

.machine "z13+vx"
.text

.balign 16
.Lconsts:
.Lwordswap:
	.byte 12, 13, 14, 15, 8, 9, 10, 11, 4, 5, 6, 7, 0, 1, 2, 3
.Lbswap128:
	.byte 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
.Lbswap32:
	.byte 3, 2, 1, 0, 7, 6, 5, 4, 11, 10, 9, 8, 15, 14, 13, 12
.Lone:
	.long 0, 0, 0, 1
.Ladd_counter_0123:
	.long 0, 1, 2, 3
.Ladd_counter_4567:
	.long 4, 5, 6, 7

/* register macros */
#define INPUT %r2
#define DST   %r3
#define SRC   %r4
#define NBLKS %r0
#define ROUND %r1

/* stack structure */

#define STACK_FRAME_STD    (8 * 16 + 8 * 4)
#define STACK_FRAME_F8_F15 (8 * 8)
#define STACK_FRAME_Y0_Y15 (16 * 16)
#define STACK_FRAME_CTR    (4 * 16)
#define STACK_FRAME_PARAMS (6 * 8)

#define STACK_MAX   (STACK_FRAME_STD + STACK_FRAME_F8_F15 + \
		     STACK_FRAME_Y0_Y15 + STACK_FRAME_CTR + \
		     STACK_FRAME_PARAMS)

#define STACK_F8     (STACK_MAX - STACK_FRAME_F8_F15)
#define STACK_F9     (STACK_F8 + 8)
#define STACK_F10    (STACK_F9 + 8)
#define STACK_F11    (STACK_F10 + 8)
#define STACK_F12    (STACK_F11 + 8)
#define STACK_F13    (STACK_F12 + 8)
#define STACK_F14    (STACK_F13 + 8)
#define STACK_F15    (STACK_F14 + 8)
#define STACK_Y0_Y15 (STACK_F8 - STACK_FRAME_Y0_Y15)
#define STACK_CTR    (STACK_Y0_Y15 - STACK_FRAME_CTR)
#define STACK_INPUT  (STACK_CTR - STACK_FRAME_PARAMS)
#define STACK_DST    (STACK_INPUT + 8)
#define STACK_SRC    (STACK_DST + 8)
#define STACK_NBLKS  (STACK_SRC + 8)
#define STACK_POCTX  (STACK_NBLKS + 8)
#define STACK_POSRC  (STACK_POCTX + 8)

#define STACK_G0_H3  STACK_Y0_Y15

/* vector registers */
#define A0 %v0
#define A1 %v1
#define A2 %v2
#define A3 %v3

#define B0 %v4
#define B1 %v5
#define B2 %v6
#define B3 %v7

#define C0 %v8
#define C1 %v9
#define C2 %v10
#define C3 %v11

#define D0 %v12
#define D1 %v13
#define D2 %v14
#define D3 %v15

#define E0 %v16
#define E1 %v17
#define E2 %v18
#define E3 %v19

#define F0 %v20
#define F1 %v21
#define F2 %v22
#define F3 %v23

#define G0 %v24
#define G1 %v25
#define G2 %v26
#define G3 %v27

#define H0 %v28
#define H1 %v29
#define H2 %v30
#define H3 %v31

#define IO0 E0
#define IO1 E1
#define IO2 E2
#define IO3 E3
#define IO4 F0
#define IO5 F1
#define IO6 F2
#define IO7 F3

#define S0 G0
#define S1 G1
#define S2 G2
#define S3 G3

#define TMP0 H0
#define TMP1 H1
#define TMP2 H2
#define TMP3 H3

#define X0 A0
#define X1 A1
#define X2 A2
#define X3 A3
#define X4 B0
#define X5 B1
#define X6 B2
#define X7 B3
#define X8 C0
#define X9 C1
#define X10 C2
#define X11 C3
#define X12 D0
#define X13 D1
#define X14 D2
#define X15 D3

#define Y0 E0
#define Y1 E1
#define Y2 E2
#define Y3 E3
#define Y4 F0
#define Y5 F1
#define Y6 F2
#define Y7 F3
#define Y8 G0
#define Y9 G1
#define Y10 G2
#define Y11 G3
#define Y12 H0
#define Y13 H1
#define Y14 H2
#define Y15 H3

/**********************************************************************
  helper macros
 **********************************************************************/

#define _ /*_*/

#define CLEAR(x,...) vzero x;

#define START_STACK(last_r) \
	lgr %r0, %r15; \
	lghi %r1, ~15; \
	stmg %r6, last_r, 6 * 8(%r15); \
	aghi %r0, -STACK_MAX; \
	ngr %r0, %r1; \
	lgr %r1, %r15; \
	CFI_DEF_CFA_REGISTER(1); \
	lgr %r15, %r0; \
	stg %r1, 0(%r15); \
	CFI_CFA_ON_STACK(0, 0); \
	std %f8, STACK_F8(%r15); \
	std %f9, STACK_F9(%r15); \
	std %f10, STACK_F10(%r15); \
	std %f11, STACK_F11(%r15); \
	std %f12, STACK_F12(%r15); \
	std %f13, STACK_F13(%r15); \
	std %f14, STACK_F14(%r15); \
	std %f15, STACK_F15(%r15);

#define END_STACK(last_r) \
	lg %r1, 0(%r15); \
	ld %f8, STACK_F8(%r15); \
	ld %f9, STACK_F9(%r15); \
	ld %f10, STACK_F10(%r15); \
	ld %f11, STACK_F11(%r15); \
	ld %f12, STACK_F12(%r15); \
	ld %f13, STACK_F13(%r15); \
	ld %f14, STACK_F14(%r15); \
	ld %f15, STACK_F15(%r15); \
	lmg %r6, last_r, 6 * 8(%r1); \
	lgr %r15, %r1; \
	CFI_DEF_CFA_REGISTER(DW_REGNO_SP);

#define PLUS(dst,src) \
	vaf dst, dst, src;

#define XOR(dst,src) \
	vx dst, dst, src;

#define ROTATE(v1,c) \
	verllf v1, v1, (c)(0);

#define WORD_ROTATE(v1,s) \
	vsldb v1, v1, v1, ((s) * 4);

#define DST_1(OPER, I, J) \
	OPER(A##I, J);

#define DST_2(OPER, I, J) \
	OPER(A##I, J); OPER(B##I, J);

#define DST_4(OPER, I, J) \
	OPER(A##I, J); OPER(B##I, J); OPER(C##I, J); OPER(D##I, J);

#define DST_8(OPER, I, J) \
	OPER(A##I, J); OPER(B##I, J); OPER(C##I, J); OPER(D##I, J); \
	OPER(E##I, J); OPER(F##I, J); OPER(G##I, J); OPER(H##I, J);

#define DST_SRC_1(OPER, I, J) \
	OPER(A##I, A##J);

#define DST_SRC_2(OPER, I, J) \
	OPER(A##I, A##J); OPER(B##I, B##J);

#define DST_SRC_4(OPER, I, J) \
	OPER(A##I, A##J); OPER(B##I, B##J); OPER(C##I, C##J); \
	OPER(D##I, D##J);

#define DST_SRC_8(OPER, I, J) \
	OPER(A##I, A##J); OPER(B##I, B##J); OPER(C##I, C##J); \
	OPER(D##I, D##J); OPER(E##I, E##J); OPER(F##I, F##J); \
	OPER(G##I, G##J); OPER(H##I, H##J);

/**********************************************************************
  round macros
 **********************************************************************/

#define QUARTERROUND4_POLY(wrot_1,wrot_2,wrot_3,op1,op2) \
	op1; DST_SRC_1(PLUS, 0, 1); DST_SRC_1(XOR, 3, 0); DST_1(ROTATE, 3, 16); \
	DST_SRC_1(PLUS, 2, 3); DST_SRC_1(XOR, 1, 2); DST_1(ROTATE, 1, 12); \
	DST_SRC_1(PLUS, 0, 1); DST_SRC_1(XOR, 3, 0); DST_1(ROTATE, 3, 8); \
	op2; DST_SRC_1(PLUS, 2, 3); DST_SRC_1(XOR, 1, 2); DST_1(ROTATE, 1, 7); \
	DST_1(WORD_ROTATE, 3, wrot_3); \
	DST_1(WORD_ROTATE, 2, wrot_2); \
	DST_1(WORD_ROTATE, 1, wrot_1);

#define QUARTERROUND4(wrot_1,wrot_2,wrot_3) \
	QUARTERROUND4_POLY(wrot_1,wrot_2,wrot_3,,)

#define QUARTERROUND4_2_POLY(wrot_1,wrot_2,wrot_3,op1,op2,op3,op4) \
	op1; DST_SRC_2(PLUS, 0, 1); DST_SRC_2(XOR, 3, 0); DST_2(ROTATE, 3, 16); \
	DST_SRC_2(PLUS, 2, 3); op2; DST_SRC_2(XOR, 1, 2); DST_2(ROTATE, 1, 12); \
	DST_SRC_2(PLUS, 0, 1); DST_SRC_2(XOR, 3, 0); op3; DST_2(ROTATE, 3, 8); \
	DST_SRC_2(PLUS, 2, 3); DST_SRC_2(XOR, 1, 2); DST_2(ROTATE, 1, 7); op4; \
	DST_2(WORD_ROTATE, 3, wrot_3); \
	DST_2(WORD_ROTATE, 2, wrot_2); \
	DST_2(WORD_ROTATE, 1, wrot_1);

#define QUARTERROUND4_2(wrot_1,wrot_2,wrot_3) \
	QUARTERROUND4_2_POLY(wrot_1,wrot_2,wrot_3,,,,)

#define QUARTERROUND4_4_POLY(wrot_1,wrot_2,wrot_3,op1,op2,op3,op4,op5,op6) \
	DST_SRC_4(PLUS, 0, 1); DST_SRC_4(XOR, 3, 0); op1; DST_4(ROTATE, 3, 16); \
	DST_SRC_4(PLUS, 2, 3); op2; DST_SRC_4(XOR, 1, 2); DST_4(ROTATE, 1, 12); \
	op3; DST_SRC_4(PLUS, 0, 1); DST_SRC_4(XOR, 3, 0); op4; DST_4(ROTATE, 3, 8); \
	DST_SRC_4(PLUS, 2, 3); op5; DST_SRC_4(XOR, 1, 2); DST_4(ROTATE, 1, 7); \
	op6; \
	DST_4(WORD_ROTATE, 3, wrot_3); \
	DST_4(WORD_ROTATE, 2, wrot_2); \
	DST_4(WORD_ROTATE, 1, wrot_1);

#define QUARTERROUND4_4(wrot_1,wrot_2,wrot_3) \
	QUARTERROUND4_4_POLY(wrot_1,wrot_2,wrot_3,,,,,,)

/**********************************************************************
  4-way && 2-way && 1-way chacha20 ("horizontal")
 **********************************************************************/

.balign 8
.globl _gcry_chacha20_s390x_vx_blocks4_2_1
ELF(.type _gcry_chacha20_s390x_vx_blocks4_2_1,@function;)

_gcry_chacha20_s390x_vx_blocks4_2_1:
	/* input:
	 *	%r2: input
	 *	%r3: dst
	 *	%r4: src
	 *	%r5: nblks
	 */
	CFI_STARTPROC();

	START_STACK(%r7);
	lgr NBLKS, %r5;

	/* Load constants. */
	larl %r7, .Lconsts;
	vl TMP0, (.Lwordswap - .Lconsts)(%r7);
	vl TMP1, (.Lone - .Lconsts)(%r7);
	vl TMP2, (.Lbswap128 - .Lconsts)(%r7);

	/* Load state. */
	vlm S0, S3, 0(INPUT);
	vperm S0, S0, S0, TMP0;
	vperm S1, S1, S1, TMP0;
	vperm S2, S2, S2, TMP0;
	vperm S3, S3, S3, TMP0;

	clgijl NBLKS, 4, .Lloop2;

.balign 4
.Lloop4:
	/* Process four chacha20 blocks. */
	vlr TMP3, S3;
	lghi ROUND, (20 / 2);
	vlr A0, S0;
	vlr A1, S1;
	vlr A2, S2;
	vlr A3, TMP3;
	vag TMP3, TMP3, TMP1;
	vlr B0, S0;
	vlr B1, S1;
	vlr B2, S2;
	vlr B3, TMP3;
	vag TMP3, TMP3, TMP1;
	vlr C0, S0;
	vlr C1, S1;
	vlr C2, S2;
	vlr C3, TMP3;
	vlr D0, S0;
	vlr D1, S1;
	vlr D2, S2;
	vag D3, TMP3, TMP1;

	slgfi NBLKS, 4;

.balign 4
.Lround2_4:
	QUARTERROUND4_4(3, 2, 1);
	QUARTERROUND4_4(1, 2, 3);
	brctg ROUND, .Lround2_4;

	vlm IO0, IO7, 0(SRC);

	PLUS(A0, S0);
	PLUS(A1, S1);
	PLUS(A2, S2);
	PLUS(A3, S3);
	vag S3, S3, TMP1; /* Update counter. */
	PLUS(B0, S0);
	PLUS(B1, S1);
	PLUS(B2, S2);
	PLUS(B3, S3);
	vag S3, S3, TMP1; /* Update counter. */
	vperm A0, A0, A0, TMP2;
	vperm A1, A1, A1, TMP2;
	vperm A2, A2, A2, TMP2;
	vperm A3, A3, A3, TMP2;
	vperm B0, B0, B0, TMP2;
	vperm B1, B1, B1, TMP2;
	vperm B2, B2, B2, TMP2;
	vperm B3, B3, B3, TMP2;
	PLUS(C0, S0);
	PLUS(C1, S1);
	PLUS(C2, S2);
	PLUS(C3, S3);
	vag S3, S3, TMP1; /* Update counter. */
	PLUS(D0, S0);
	PLUS(D1, S1);
	PLUS(D2, S2);
	PLUS(D3, S3);
	vag S3, S3, TMP1; /* Update counter. */
	vperm C0, C0, C0, TMP2;
	vperm C1, C1, C1, TMP2;
	vperm C2, C2, C2, TMP2;
	vperm C3, C3, C3, TMP2;
	vperm D0, D0, D0, TMP2;
	vperm D1, D1, D1, TMP2;
	vperm D2, D2, D2, TMP2;
	vperm D3, D3, D3, TMP2;

	XOR(IO0, A0);
	XOR(IO1, A1);
	XOR(IO2, A2);
	XOR(IO3, A3);
	XOR(IO4, B0);
	XOR(IO5, B1);
	XOR(IO6, B2);
	XOR(IO7, B3);
	vlm A0, B3, 128(SRC);
	vstm IO0, IO7, 0(DST);
	XOR(A0, C0);
	XOR(A1, C1);
	XOR(A2, C2);
	XOR(A3, C3);
	XOR(B0, D0);
	XOR(B1, D1);
	XOR(B2, D2);
	XOR(B3, D3);
	vstm A0, B3, 128(DST);

	aghi SRC, 256;
	aghi DST, 256;

	clgijhe NBLKS, 4, .Lloop4;

	CLEAR(C0);
	CLEAR(C1);
	CLEAR(C2);
	CLEAR(C3);
	CLEAR(D0);
	CLEAR(D1);
	CLEAR(D2);
	CLEAR(D3);

.balign 4
.Lloop2:
	clgijl NBLKS, 2, .Lloop1;

	/* Process two chacha20 blocks. */
	lghi ROUND, (20 / 2);
	vlr A0, S0;
	vlr A1, S1;
	vlr A2, S2;
	vlr A3, S3;
	vlr B0, S0;
	vlr B1, S1;
	vlr B2, S2;
	vag B3, S3, TMP1;

	slgfi NBLKS, 2;

.balign 4
.Lround2_2:
	QUARTERROUND4_2(3, 2, 1);
	QUARTERROUND4_2(1, 2, 3);
	brctg ROUND, .Lround2_2;

	vlm IO0, IO7, 0(SRC);

	PLUS(A0, S0);
	PLUS(A1, S1);
	PLUS(A2, S2);
	PLUS(A3, S3);
	vag S3, S3, TMP1; /* Update counter. */
	PLUS(B0, S0);
	PLUS(B1, S1);
	PLUS(B2, S2);
	PLUS(B3, S3);
	vag S3, S3, TMP1; /* Update counter. */
	vperm A0, A0, A0, TMP2;
	vperm A1, A1, A1, TMP2;
	vperm A2, A2, A2, TMP2;
	vperm A3, A3, A3, TMP2;
	vperm B0, B0, B0, TMP2;
	vperm B1, B1, B1, TMP2;
	vperm B2, B2, B2, TMP2;
	vperm B3, B3, B3, TMP2;

	XOR(IO0, A0);
	XOR(IO1, A1);
	XOR(IO2, A2);
	XOR(IO3, A3);
	XOR(IO4, B0);
	XOR(IO5, B1);
	XOR(IO6, B2);
	XOR(IO7, B3);
	vstm IO0, IO7, 0(DST);

	aghi SRC, 128;
	aghi DST, 128;

	clgijhe NBLKS, 2, .Lloop2;

	CLEAR(B0);
	CLEAR(B1);
	CLEAR(B2);
	CLEAR(B3);

.balign 4
.Lloop1:
	clgijl NBLKS, 1, .Ldone;

	/* Process one chacha20 block.*/
	lghi ROUND, (20 / 2);
	vlr A0, S0;
	vlr A1, S1;
	vlr A2, S2;
	vlr A3, S3;

	slgfi NBLKS, 1;

.balign 4
.Lround2_1:
	QUARTERROUND4(3, 2, 1);
	QUARTERROUND4(1, 2, 3);
	brct ROUND, .Lround2_1;

	vlm IO0, IO3, 0(SRC);

	PLUS(A0, S0);
	PLUS(A1, S1);
	PLUS(A2, S2);
	PLUS(A3, S3);
	vag S3, S3, TMP1; /* Update counter. */

	vperm A0, A0, A0, TMP2;
	vperm A1, A1, A1, TMP2;
	vperm A2, A2, A2, TMP2;
	vperm A3, A3, A3, TMP2;
	XOR(IO0, A0);
	XOR(IO1, A1);
	XOR(IO2, A2);
	XOR(IO3, A3);
	vstm IO0, IO3, 0(DST);

	aghi SRC, 64;
	aghi DST, 64;

	clgijhe NBLKS, 1, .Lloop1;

.balign 4
.Ldone:
	/* Store counter. */
	vperm S3, S3, S3, TMP0;
	vst S3, (48)(INPUT);

	/* Clear the used vector registers. */
	CLEAR(A0);
	CLEAR(A1);
	CLEAR(A2);
	CLEAR(A3);
	CLEAR(IO0);
	CLEAR(IO1);
	CLEAR(IO2);
	CLEAR(IO3);
	CLEAR(IO4);
	CLEAR(IO5);
	CLEAR(IO6);
	CLEAR(IO7);
	CLEAR(TMP0);
	CLEAR(TMP1);
	CLEAR(TMP2);

	END_STACK(%r7);
	xgr %r2, %r2;
	br %r14;
	CFI_ENDPROC();
ELF(.size _gcry_chacha20_s390x_vx_blocks4_2_1,
    .-_gcry_chacha20_s390x_vx_blocks4_2_1;)

/**********************************************************************
  4-way && 2-way && 1-way stitched chacha20-poly1305 ("horizontal")
 **********************************************************************/

.balign 8
.globl _gcry_chacha20_poly1305_s390x_vx_blocks4_2_1
ELF(.type _gcry_chacha20_poly1305_s390x_vx_blocks4_2_1,@function;)

_gcry_chacha20_poly1305_s390x_vx_blocks4_2_1:
	/* input:
	 *       %r2: input
	 *       %r3: dst
	 *       %r4: src
	 *       %r5: nblks
	 *       %r6: poly1305 state
	 * 160(%r15): poly1305 src
	 */
	CFI_STARTPROC();

	START_STACK(%r14);
	lgr NBLKS, %r5;

	/* Load constants. */
	larl %r8, .Lconsts;
	vl TMP0, (.Lwordswap - .Lconsts)(%r8);
	vl TMP1, (.Lone - .Lconsts)(%r8);
	vl TMP2, (.Lbswap128 - .Lconsts)(%r8);

	/* Load state. */
	vlm S0, S3, 0(INPUT);
	vperm S0, S0, S0, TMP0;
	vperm S1, S1, S1, TMP0;
	vperm S2, S2, S2, TMP0;
	vperm S3, S3, S3, TMP0;

	/* Store parameters to stack. */
	stmg %r2, %r6, STACK_INPUT(%r15);

	lgr POLY_RSTATE, %r6;
	lgr NBLKS, %r5;

	lg POLY_RSRC, 0(%r15);
	lg POLY_RSRC, 160(POLY_RSRC);
	stg POLY_RSRC, STACK_POSRC(%r15);

	/* Load poly1305 state */
	POLY1305_LOAD_STATE();

	clgijl NBLKS, 4, .Lloop2_poly;

.balign 4
.Lloop4_poly:
	/* Process four chacha20 blocks and 16 poly1305 blocks. */
	vlr TMP3, S3;
	lghi ROUND, (20 / 4);
	vlr A0, S0;
	vlr A1, S1;
	vlr A2, S2;
	vlr A3, TMP3;
	vag TMP3, TMP3, TMP1;
	vlr B0, S0;
	vlr B1, S1;
	vlr B2, S2;
	vlr B3, TMP3;
	vag TMP3, TMP3, TMP1;
	vlr C0, S0;
	vlr C1, S1;
	vlr C2, S2;
	vlr C3, TMP3;
	vlr D0, S0;
	vlr D1, S1;
	vlr D2, S2;
	vag D3, TMP3, TMP1;

	slgfi NBLKS, 4;

.balign 4
.Lround4_4_poly:
	/* Total 15 poly1305 blocks processed by this loop. */
	QUARTERROUND4_4_POLY(3, 2, 1,
			     POLY1305_BLOCK_PART1(0 * 16),
			     POLY1305_BLOCK_PART2(),
			     POLY1305_BLOCK_PART3(),
			     POLY1305_BLOCK_PART4(),
			     POLY1305_BLOCK_PART5(),
			     POLY1305_BLOCK_PART6());
	QUARTERROUND4_4_POLY(1, 2, 3,
			     POLY1305_BLOCK_PART7(),
			     POLY1305_BLOCK_PART8(),
			     POLY1305_BLOCK_PART1(1 * 16),
			     POLY1305_BLOCK_PART2(),
			     POLY1305_BLOCK_PART3(),
			     POLY1305_BLOCK_PART4());
	QUARTERROUND4_4_POLY(3, 2, 1,
			     POLY1305_BLOCK_PART5(),
			     POLY1305_BLOCK_PART6(),
			     POLY1305_BLOCK_PART7(),
			     POLY1305_BLOCK_PART8(),
			     POLY1305_BLOCK_PART1(2 * 16);
			      INC_POLY1305_SRC(3 * 16),
			     POLY1305_BLOCK_PART2());
	QUARTERROUND4_4_POLY(1, 2, 3,
			     POLY1305_BLOCK_PART3(),
			     POLY1305_BLOCK_PART4(),
			     POLY1305_BLOCK_PART5(),
			     POLY1305_BLOCK_PART6(),
			     POLY1305_BLOCK_PART7(),
			     POLY1305_BLOCK_PART8());
	brctg ROUND, .Lround4_4_poly;

	POLY1305_BLOCK_PART1(0 * 16);
	INC_POLY1305_SRC(1 * 16);
	stg POLY_RSRC, STACK_POSRC(%r15);

	lg %r14, STACK_SRC(%r15);
	vlm IO0, IO7, 0(%r14);

	PLUS(A0, S0);
	PLUS(A1, S1);
	PLUS(A2, S2);
	PLUS(A3, S3);
	vag S3, S3, TMP1; /* Update counter. */
	POLY1305_BLOCK_PART2();
	PLUS(B0, S0);
	PLUS(B1, S1);
	PLUS(B2, S2);
	PLUS(B3, S3);
	vag S3, S3, TMP1; /* Update counter. */
	POLY1305_BLOCK_PART3();
	vperm A0, A0, A0, TMP2;
	vperm A1, A1, A1, TMP2;
	vperm A2, A2, A2, TMP2;
	vperm A3, A3, A3, TMP2;
	vperm B0, B0, B0, TMP2;
	vperm B1, B1, B1, TMP2;
	vperm B2, B2, B2, TMP2;
	vperm B3, B3, B3, TMP2;
	POLY1305_BLOCK_PART4();
	PLUS(C0, S0);
	PLUS(C1, S1);
	PLUS(C2, S2);
	PLUS(C3, S3);
	vag S3, S3, TMP1; /* Update counter. */
	PLUS(D0, S0);
	PLUS(D1, S1);
	PLUS(D2, S2);
	PLUS(D3, S3);
	vag S3, S3, TMP1; /* Update counter. */
	POLY1305_BLOCK_PART5();
	vperm C0, C0, C0, TMP2;
	vperm C1, C1, C1, TMP2;
	vperm C2, C2, C2, TMP2;
	vperm C3, C3, C3, TMP2;
	vperm D0, D0, D0, TMP2;
	vperm D1, D1, D1, TMP2;
	vperm D2, D2, D2, TMP2;
	vperm D3, D3, D3, TMP2;

	POLY1305_BLOCK_PART6();
	XOR(IO0, A0);
	XOR(IO1, A1);
	XOR(IO2, A2);
	XOR(IO3, A3);
	XOR(IO4, B0);
	XOR(IO5, B1);
	XOR(IO6, B2);
	XOR(IO7, B3);
	vlm A0, B3, 128(%r14);
	aghi %r14, 256;
	stg %r14, STACK_SRC(%r15);

	lg %r14, STACK_DST(%r15);
	POLY1305_BLOCK_PART7();
	vstm IO0, IO7, 0(%r14);
	XOR(A0, C0);
	XOR(A1, C1);
	XOR(A2, C2);
	XOR(A3, C3);
	XOR(B0, D0);
	XOR(B1, D1);
	XOR(B2, D2);
	XOR(B3, D3);
	POLY1305_BLOCK_PART8();
	vstm A0, B3, 128(%r14);
	aghi %r14, 256;
	stg %r14, STACK_DST(%r15);

	lg POLY_RSRC, STACK_POSRC(%r15);

	clgijhe NBLKS, 4, .Lloop4_poly;

	CLEAR(C0);
	CLEAR(C1);
	CLEAR(C2);
	CLEAR(C3);
	CLEAR(D0);
	CLEAR(D1);
	CLEAR(D2);
	CLEAR(D3);

.balign 4
.Lloop2_poly:
	clgijl NBLKS, 2, .Lloop1_poly;

	/* Process two chacha20 and eight poly1305 blocks. */
	lghi ROUND, ((20 - 4) / 2);
	vlr A0, S0;
	vlr A1, S1;
	vlr A2, S2;
	vlr A3, S3;
	vlr B0, S0;
	vlr B1, S1;
	vlr B2, S2;
	vag B3, S3, TMP1;

	slgfi NBLKS, 2;

.balign 4
.Lround4_2_poly:
	/* Total eight poly1305 blocks processed by this loop. */
	QUARTERROUND4_2_POLY(3, 2, 1,
			     POLY1305_BLOCK_PART1(0 * 16),
			     POLY1305_BLOCK_PART2(),
			     POLY1305_BLOCK_PART3(),
			     POLY1305_BLOCK_PART4());
			     INC_POLY1305_SRC(1 * 16);
	QUARTERROUND4_2_POLY(1, 2, 3,
			     POLY1305_BLOCK_PART5(),
			     POLY1305_BLOCK_PART6(),
			     POLY1305_BLOCK_PART7(),
			     POLY1305_BLOCK_PART8());
	brctg ROUND, .Lround4_2_poly;

	stg POLY_RSRC, STACK_POSRC(%r15);
	lg %r14, STACK_SRC(%r15);

	QUARTERROUND4_2(3, 2, 1);
	QUARTERROUND4_2(1, 2, 3);
	QUARTERROUND4_2(3, 2, 1);
	QUARTERROUND4_2(1, 2, 3);

	vlm IO0, IO7, 0(%r14);
	aghi %r14, 128;
	stg %r14, STACK_SRC(%r15);

	PLUS(A0, S0);
	PLUS(A1, S1);
	PLUS(A2, S2);
	PLUS(A3, S3);
	vag S3, S3, TMP1; /* Update counter. */
	PLUS(B0, S0);
	PLUS(B1, S1);
	PLUS(B2, S2);
	PLUS(B3, S3);
	vag S3, S3, TMP1; /* Update counter. */
	vperm A0, A0, A0, TMP2;
	vperm A1, A1, A1, TMP2;
	vperm A2, A2, A2, TMP2;
	vperm A3, A3, A3, TMP2;
	vperm B0, B0, B0, TMP2;
	vperm B1, B1, B1, TMP2;
	vperm B2, B2, B2, TMP2;
	vperm B3, B3, B3, TMP2;

	lg %r14, STACK_DST(%r15);
	XOR(IO0, A0);
	XOR(IO1, A1);
	XOR(IO2, A2);
	XOR(IO3, A3);
	XOR(IO4, B0);
	XOR(IO5, B1);
	XOR(IO6, B2);
	XOR(IO7, B3);
	vstm IO0, IO7, 0(%r14);
	aghi %r14, 128;
	stg %r14, STACK_DST(%r15);

	lg POLY_RSRC, STACK_POSRC(%r15);

	clgijhe NBLKS, 2, .Lloop2_poly;

	CLEAR(B0);
	CLEAR(B1);
	CLEAR(B2);
	CLEAR(B3);

.balign 4
.Lloop1_poly:
	clgijl NBLKS, 1, .Ldone_poly;

	/* Process one chacha20 block and four poly1305 blocks.*/
	lghi ROUND, ((20 - 4) / 4);
	vlr A0, S0;
	vlr A1, S1;
	vlr A2, S2;
	vlr A3, S3;

	slgfi NBLKS, 1;

.balign 4
.Lround4_1_poly:
	/* Total four poly1305 blocks processed by this loop. */
	QUARTERROUND4_POLY(3, 2, 1,
			   POLY1305_BLOCK_PART1(0 * 16),
			   POLY1305_BLOCK_PART2());
			   INC_POLY1305_SRC(1 * 16);
	QUARTERROUND4_POLY(1, 2, 3,
			   POLY1305_BLOCK_PART3(),
			   POLY1305_BLOCK_PART4());
	QUARTERROUND4_POLY(3, 2, 1,
			   POLY1305_BLOCK_PART5(),
			   POLY1305_BLOCK_PART6());
	QUARTERROUND4_POLY(1, 2, 3,
			   POLY1305_BLOCK_PART7(),
			   POLY1305_BLOCK_PART8());
	brct ROUND, .Lround4_1_poly;

	stg POLY_RSRC, STACK_POSRC(%r15);
	lg %r14, STACK_SRC(%r15);

	QUARTERROUND4(3, 2, 1);
	QUARTERROUND4(1, 2, 3);
	QUARTERROUND4(3, 2, 1);
	QUARTERROUND4(1, 2, 3);

	vlm IO0, IO3, 0(%r14);
	aghi %r14, 64;
	stg %r14, STACK_SRC(%r15);

	PLUS(A0, S0);
	PLUS(A1, S1);
	PLUS(A2, S2);
	PLUS(A3, S3);
	vag S3, S3, TMP1; /* Update counter. */

	lg %r14, STACK_DST(%r15);
	vperm A0, A0, A0, TMP2;
	vperm A1, A1, A1, TMP2;
	vperm A2, A2, A2, TMP2;
	vperm A3, A3, A3, TMP2;
	XOR(IO0, A0);
	XOR(IO1, A1);
	XOR(IO2, A2);
	XOR(IO3, A3);
	vstm IO0, IO3, 0(%r14);
	aghi %r14, 64;
	stg %r14, STACK_DST(%r15);

	lg POLY_RSRC, STACK_POSRC(%r15);

	clgijhe NBLKS, 1, .Lloop1_poly;

.balign 4
.Ldone_poly:
	/* Store poly1305 state */
	lg POLY_RSTATE, STACK_POCTX(%r15);
	POLY1305_STORE_STATE();

	/* Store counter. */
	lg INPUT, STACK_INPUT(%r15);
	vperm S3, S3, S3, TMP0;
	vst S3, (48)(INPUT);

	/* Clear the used vector registers. */
	CLEAR(A0);
	CLEAR(A1);
	CLEAR(A2);
	CLEAR(A3);
	CLEAR(IO0);
	CLEAR(IO1);
	CLEAR(IO2);
	CLEAR(IO3);
	CLEAR(IO4);
	CLEAR(IO5);
	CLEAR(IO6);
	CLEAR(IO7);
	CLEAR(TMP0);
	CLEAR(TMP1);
	CLEAR(TMP2);

	END_STACK(%r14);
	xgr %r2, %r2;
	br %r14;
	CFI_ENDPROC();
ELF(.size _gcry_chacha20_poly1305_s390x_vx_blocks4_2_1,
    .-_gcry_chacha20_poly1305_s390x_vx_blocks4_2_1;)

/**********************************************************************
  8-way chacha20 ("vertical")
 **********************************************************************/

#define QUARTERROUND4_V8_POLY(x0,x1,x2,x3,x4,x5,x6,x7,\
			      x8,x9,x10,x11,x12,x13,x14,x15,\
			      y0,y1,y2,y3,y4,y5,y6,y7,\
			      y8,y9,y10,y11,y12,y13,y14,y15,\
			      op1,op2,op3,op4,op5,op6,op7,op8,\
			      op9,op10,op11,op12) \
	op1;							\
	PLUS(x0, x1); PLUS(x4, x5);				\
	PLUS(x8, x9); PLUS(x12, x13);				\
	PLUS(y0, y1); PLUS(y4, y5);				\
	PLUS(y8, y9); PLUS(y12, y13);				\
	    op2;						\
	    XOR(x3, x0);  XOR(x7, x4);				\
	    XOR(x11, x8); XOR(x15, x12);			\
	    XOR(y3, y0);  XOR(y7, y4);				\
	    XOR(y11, y8); XOR(y15, y12);			\
		op3;						\
		ROTATE(x3, 16); ROTATE(x7, 16);			\
		ROTATE(x11, 16); ROTATE(x15, 16);		\
		ROTATE(y3, 16); ROTATE(y7, 16);			\
		ROTATE(y11, 16); ROTATE(y15, 16);		\
	op4;							\
	PLUS(x2, x3); PLUS(x6, x7);				\
	PLUS(x10, x11); PLUS(x14, x15);				\
	PLUS(y2, y3); PLUS(y6, y7);				\
	PLUS(y10, y11); PLUS(y14, y15);				\
	    op5;						\
	    XOR(x1, x2); XOR(x5, x6);				\
	    XOR(x9, x10); XOR(x13, x14);			\
	    XOR(y1, y2); XOR(y5, y6);				\
	    XOR(y9, y10); XOR(y13, y14);			\
		op6;						\
		ROTATE(x1,12); ROTATE(x5,12);			\
		ROTATE(x9,12); ROTATE(x13,12);			\
		ROTATE(y1,12); ROTATE(y5,12);			\
		ROTATE(y9,12); ROTATE(y13,12);			\
	op7;							\
	PLUS(x0, x1); PLUS(x4, x5);				\
	PLUS(x8, x9); PLUS(x12, x13);				\
	PLUS(y0, y1); PLUS(y4, y5);				\
	PLUS(y8, y9); PLUS(y12, y13);				\
	    op8;						\
	    XOR(x3, x0); XOR(x7, x4);				\
	    XOR(x11, x8); XOR(x15, x12);			\
	    XOR(y3, y0); XOR(y7, y4);				\
	    XOR(y11, y8); XOR(y15, y12);			\
		op9;						\
		ROTATE(x3,8); ROTATE(x7,8);			\
		ROTATE(x11,8); ROTATE(x15,8);			\
		ROTATE(y3,8); ROTATE(y7,8);			\
		ROTATE(y11,8); ROTATE(y15,8);			\
	op10;							\
	PLUS(x2, x3); PLUS(x6, x7);				\
	PLUS(x10, x11); PLUS(x14, x15);				\
	PLUS(y2, y3); PLUS(y6, y7);				\
	PLUS(y10, y11); PLUS(y14, y15);				\
	    op11;						\
	    XOR(x1, x2); XOR(x5, x6);				\
	    XOR(x9, x10); XOR(x13, x14);			\
	    XOR(y1, y2); XOR(y5, y6);				\
	    XOR(y9, y10); XOR(y13, y14);			\
		op12;						\
		ROTATE(x1,7); ROTATE(x5,7);			\
		ROTATE(x9,7); ROTATE(x13,7);			\
		ROTATE(y1,7); ROTATE(y5,7);			\
		ROTATE(y9,7); ROTATE(y13,7);

#define QUARTERROUND4_V8(x0,x1,x2,x3,x4,x5,x6,x7,x8,x9,x10,x11,x12,x13,x14,x15,\
			 y0,y1,y2,y3,y4,y5,y6,y7,y8,y9,y10,y11,y12,y13,y14,y15) \
	QUARTERROUND4_V8_POLY(x0,x1,x2,x3,x4,x5,x6,x7,\
			      x8,x9,x10,x11,x12,x13,x14,x15,\
			      y0,y1,y2,y3,y4,y5,y6,y7,\
			      y8,y9,y10,y11,y12,y13,y14,y15,\
			      ,,,,,,,,,,,)

#define TRANSPOSE_4X4_2(v0,v1,v2,v3,va,vb,vc,vd,tmp0,tmp1,tmp2,tmpa,tmpb,tmpc) \
	  vmrhf tmp0, v0, v1;					\
	  vmrhf tmp1, v2, v3;					\
	  vmrlf tmp2, v0, v1;					\
	  vmrlf   v3, v2, v3;					\
	  vmrhf tmpa, va, vb;					\
	  vmrhf tmpb, vc, vd;					\
	  vmrlf tmpc, va, vb;					\
	  vmrlf   vd, vc, vd;					\
	  vpdi v0, tmp0, tmp1, 0;				\
	  vpdi v1, tmp0, tmp1, 5;				\
	  vpdi v2, tmp2,   v3, 0;				\
	  vpdi v3, tmp2,   v3, 5;				\
	  vpdi va, tmpa, tmpb, 0;				\
	  vpdi vb, tmpa, tmpb, 5;				\
	  vpdi vc, tmpc,   vd, 0;				\
	  vpdi vd, tmpc,   vd, 5;

.balign 8
.globl _gcry_chacha20_s390x_vx_blocks8
ELF(.type _gcry_chacha20_s390x_vx_blocks8,@function;)

_gcry_chacha20_s390x_vx_blocks8:
	/* input:
	 *	%r2: input
	 *	%r3: dst
	 *	%r4: src
	 *	%r5: nblks (multiple of 8)
	 */
	CFI_STARTPROC();

	START_STACK(%r8);
	lgr NBLKS, %r5;

	larl %r7, .Lconsts;

	/* Load counter. */
	lg %r8, (12 * 4)(INPUT);
	rllg %r8, %r8, 32;

.balign 4
	/* Process eight chacha20 blocks per loop. */
.Lloop8:
	vlm Y0, Y3, 0(INPUT);

	slgfi NBLKS, 8;
	lghi ROUND, (20 / 2);

	/* Construct counter vectors X12/X13 & Y12/Y13. */
	vl X4, (.Ladd_counter_0123 - .Lconsts)(%r7);
	vl Y4, (.Ladd_counter_4567 - .Lconsts)(%r7);
	vrepf Y12, Y3, 0;
	vrepf Y13, Y3, 1;
	vaccf X5, Y12, X4;
	vaccf Y5, Y12, Y4;
	vaf X12, Y12, X4;
	vaf Y12, Y12, Y4;
	vaf X13, Y13, X5;
	vaf Y13, Y13, Y5;

	vrepf X0, Y0, 0;
	vrepf X1, Y0, 1;
	vrepf X2, Y0, 2;
	vrepf X3, Y0, 3;
	vrepf X4, Y1, 0;
	vrepf X5, Y1, 1;
	vrepf X6, Y1, 2;
	vrepf X7, Y1, 3;
	vrepf X8, Y2, 0;
	vrepf X9, Y2, 1;
	vrepf X10, Y2, 2;
	vrepf X11, Y2, 3;
	vrepf X14, Y3, 2;
	vrepf X15, Y3, 3;

	/* Store counters for blocks 0-7. */
	vstm X12, X13, (STACK_CTR + 0 * 16)(%r15);
	vstm Y12, Y13, (STACK_CTR + 2 * 16)(%r15);

	vlr Y0, X0;
	vlr Y1, X1;
	vlr Y2, X2;
	vlr Y3, X3;
	vlr Y4, X4;
	vlr Y5, X5;
	vlr Y6, X6;
	vlr Y7, X7;
	vlr Y8, X8;
	vlr Y9, X9;
	vlr Y10, X10;
	vlr Y11, X11;
	vlr Y14, X14;
	vlr Y15, X15;

	/* Update and store counter. */
	agfi %r8, 8;
	rllg %r5, %r8, 32;
	stg %r5, (12 * 4)(INPUT);

.balign 4
.Lround2_8:
	QUARTERROUND4_V8(X0, X4,  X8, X12,   X1, X5,  X9, X13,
			 X2, X6, X10, X14,   X3, X7, X11, X15,
			 Y0, Y4,  Y8, Y12,   Y1, Y5,  Y9, Y13,
			 Y2, Y6, Y10, Y14,   Y3, Y7, Y11, Y15);
	QUARTERROUND4_V8(X0, X5, X10, X15,   X1, X6, X11, X12,
			 X2, X7,  X8, X13,   X3, X4,  X9, X14,
			 Y0, Y5, Y10, Y15,   Y1, Y6, Y11, Y12,
			 Y2, Y7,  Y8, Y13,   Y3, Y4,  Y9, Y14);
	brctg ROUND, .Lround2_8;

	/* Store blocks 4-7. */
	vstm Y0, Y15, STACK_Y0_Y15(%r15);

	/* Load counters for blocks 0-3. */
	vlm Y0, Y1, (STACK_CTR + 0 * 16)(%r15);

	lghi ROUND, 1;
	j .Lfirst_output_4blks_8;

.balign 4
.Lsecond_output_4blks_8:
	/* Load blocks 4-7. */
	vlm X0, X15, STACK_Y0_Y15(%r15);

	/* Load counters for blocks 4-7. */
	vlm Y0, Y1, (STACK_CTR + 2 * 16)(%r15);

	lghi ROUND, 0;

.balign 4
	/* Output four chacha20 blocks per loop. */
.Lfirst_output_4blks_8:
	vlm Y12, Y15, 0(INPUT);
	PLUS(X12, Y0);
	PLUS(X13, Y1);
	vrepf Y0, Y12, 0;
	vrepf Y1, Y12, 1;
	vrepf Y2, Y12, 2;
	vrepf Y3, Y12, 3;
	vrepf Y4, Y13, 0;
	vrepf Y5, Y13, 1;
	vrepf Y6, Y13, 2;
	vrepf Y7, Y13, 3;
	vrepf Y8, Y14, 0;
	vrepf Y9, Y14, 1;
	vrepf Y10, Y14, 2;
	vrepf Y11, Y14, 3;
	vrepf Y14, Y15, 2;
	vrepf Y15, Y15, 3;
	PLUS(X0, Y0);
	PLUS(X1, Y1);
	PLUS(X2, Y2);
	PLUS(X3, Y3);
	PLUS(X4, Y4);
	PLUS(X5, Y5);
	PLUS(X6, Y6);
	PLUS(X7, Y7);
	PLUS(X8, Y8);
	PLUS(X9, Y9);
	PLUS(X10, Y10);
	PLUS(X11, Y11);
	PLUS(X14, Y14);
	PLUS(X15, Y15);

	vl Y15, (.Lbswap32 - .Lconsts)(%r7);
	TRANSPOSE_4X4_2(X0, X1, X2, X3, X4, X5, X6, X7,
			Y9, Y10, Y11, Y12, Y13, Y14);
	TRANSPOSE_4X4_2(X8, X9, X10, X11, X12, X13, X14, X15,
			Y9, Y10, Y11, Y12, Y13, Y14);

	vlm Y0, Y14, 0(SRC);
	vperm X0, X0, X0, Y15;
	vperm X1, X1, X1, Y15;
	vperm X2, X2, X2, Y15;
	vperm X3, X3, X3, Y15;
	vperm X4, X4, X4, Y15;
	vperm X5, X5, X5, Y15;
	vperm X6, X6, X6, Y15;
	vperm X7, X7, X7, Y15;
	vperm X8, X8, X8, Y15;
	vperm X9, X9, X9, Y15;
	vperm X10, X10, X10, Y15;
	vperm X11, X11, X11, Y15;
	vperm X12, X12, X12, Y15;
	vperm X13, X13, X13, Y15;
	vperm X14, X14, X14, Y15;
	vperm X15, X15, X15, Y15;
	vl Y15, (15 * 16)(SRC);

	XOR(Y0, X0);
	XOR(Y1, X4);
	XOR(Y2, X8);
	XOR(Y3, X12);
	XOR(Y4, X1);
	XOR(Y5, X5);
	XOR(Y6, X9);
	XOR(Y7, X13);
	XOR(Y8, X2);
	XOR(Y9, X6);
	XOR(Y10, X10);
	XOR(Y11, X14);
	XOR(Y12, X3);
	XOR(Y13, X7);
	XOR(Y14, X11);
	XOR(Y15, X15);
	vstm Y0, Y15, 0(DST);

	aghi SRC, 256;
	aghi DST, 256;

	clgije ROUND, 1, .Lsecond_output_4blks_8;

	clgijhe NBLKS, 8, .Lloop8;

	/* Clear the used vector registers. */
	DST_8(CLEAR, 0, _);
	DST_8(CLEAR, 1, _);
	DST_8(CLEAR, 2, _);
	DST_8(CLEAR, 3, _);

	/* Clear sensitive data in stack. */
	vlm Y0, Y15, STACK_Y0_Y15(%r15);
	vlm Y0, Y3, STACK_CTR(%r15);

	END_STACK(%r8);
	xgr %r2, %r2;
	br %r14;
	CFI_ENDPROC();
ELF(.size _gcry_chacha20_s390x_vx_blocks8,
    .-_gcry_chacha20_s390x_vx_blocks8;)

/**********************************************************************
  8-way stitched chacha20-poly1305 ("vertical")
 **********************************************************************/

.balign 8
.globl _gcry_chacha20_poly1305_s390x_vx_blocks8
ELF(.type _gcry_chacha20_poly1305_s390x_vx_blocks8,@function;)

_gcry_chacha20_poly1305_s390x_vx_blocks8:
	/* input:
	 *       %r2: input
	 *       %r3: dst
	 *       %r4: src
	 *       %r5: nblks (multiple of 8)
	 *       %r6: poly1305 state
	 * 160(%r15): poly1305 src
	 */
	CFI_STARTPROC();

	START_STACK(%r14);

	/* Store parameters to stack. */
	stmg %r2, %r6, STACK_INPUT(%r15);

	lgr POLY_RSTATE, %r6;
	lgr NBLKS, %r5;

	lg POLY_RSRC, 0(%r15);
	lg POLY_RSRC, 160(POLY_RSRC);
	stg POLY_RSRC, STACK_POSRC(%r15);

	/* Load poly1305 state */
	POLY1305_LOAD_STATE();

.balign 4
	/* Process eight chacha20 blocks and 32 poly1305 blocks per loop. */
.Lloop8_poly:
	lg INPUT, STACK_INPUT(%r15);
	larl %r8, .Lconsts;

	vlm Y0, Y3, 0(INPUT);

	slgfi NBLKS, 8;
	lghi ROUND, (20 / 2);

	/* Construct counter vectors X12/X13 & Y12/Y13. */
	vl X4, (.Ladd_counter_0123 - .Lconsts)(%r8);
	vl Y4, (.Ladd_counter_4567 - .Lconsts)(%r8);
	lg %r8, (12 * 4)(INPUT); /* Update counter. */
	vrepf Y12, Y3, 0;
	vrepf Y13, Y3, 1;
	vaccf X5, Y12, X4;
	vaccf Y5, Y12, Y4;
	vaf X12, Y12, X4;
	vaf Y12, Y12, Y4;
	vaf X13, Y13, X5;
	vaf Y13, Y13, Y5;
	rllg %r8, %r8, 32;

	vrepf X0, Y0, 0;
	vrepf X1, Y0, 1;
	vrepf X2, Y0, 2;
	vrepf X3, Y0, 3;
	vrepf X4, Y1, 0;
	vrepf X5, Y1, 1;
	vrepf X6, Y1, 2;
	vrepf X7, Y1, 3;
	vrepf X8, Y2, 0;
	vrepf X9, Y2, 1;
	vrepf X10, Y2, 2;
	vrepf X11, Y2, 3;
	vrepf X14, Y3, 2;
	vrepf X15, Y3, 3;
	agfi %r8, 8;

	/* Store counters for blocks 0-7. */
	vstm X12, X13, (STACK_CTR + 0 * 16)(%r15);
	vstm Y12, Y13, (STACK_CTR + 2 * 16)(%r15);
	rllg %r8, %r8, 32;

	vlr Y0, X0;
	vlr Y1, X1;
	vlr Y2, X2;
	vlr Y3, X3;
	vlr Y4, X4;
	vlr Y5, X5;
	vlr Y6, X6;
	vlr Y7, X7;
	vlr Y8, X8;
	vlr Y9, X9;
	vlr Y10, X10;
	vlr Y11, X11;
	vlr Y14, X14;
	vlr Y15, X15;
	stg %r8, (12 * 4)(INPUT);

.balign 4
.Lround2_8_poly:
	/* Total 30 poly1305 blocks processed by this loop. */
	QUARTERROUND4_V8_POLY(X0, X4,  X8, X12,   X1, X5,  X9, X13,
			      X2, X6, X10, X14,   X3, X7, X11, X15,
			      Y0, Y4,  Y8, Y12,   Y1, Y5,  Y9, Y13,
			      Y2, Y6, Y10, Y14,   Y3, Y7, Y11, Y15,
			      POLY1305_BLOCK_PART1(0 * 16),
			      POLY1305_BLOCK_PART2(),
			      POLY1305_BLOCK_PART3(),
			      POLY1305_BLOCK_PART4(),
			      POLY1305_BLOCK_PART5(),
			      POLY1305_BLOCK_PART6(),
			      POLY1305_BLOCK_PART7(),
			      POLY1305_BLOCK_PART8(),
			      POLY1305_BLOCK_PART1(1 * 16),
			      POLY1305_BLOCK_PART2(),
			      POLY1305_BLOCK_PART3(),
			      POLY1305_BLOCK_PART4());
	QUARTERROUND4_V8_POLY(X0, X5, X10, X15,   X1, X6, X11, X12,
			      X2, X7,  X8, X13,   X3, X4,  X9, X14,
			      Y0, Y5, Y10, Y15,   Y1, Y6, Y11, Y12,
			      Y2, Y7,  Y8, Y13,   Y3, Y4,  Y9, Y14,
			      POLY1305_BLOCK_PART5(),
			      POLY1305_BLOCK_PART6(),
			      POLY1305_BLOCK_PART7(),
			      POLY1305_BLOCK_PART8(),
			      POLY1305_BLOCK_PART1(2 * 16);
				INC_POLY1305_SRC(3 * 16),
			      POLY1305_BLOCK_PART2(),
			      POLY1305_BLOCK_PART3(),
			      POLY1305_BLOCK_PART4(),
			      POLY1305_BLOCK_PART5(),
			      POLY1305_BLOCK_PART6(),
			      POLY1305_BLOCK_PART7(),
			      POLY1305_BLOCK_PART8());
	brctg ROUND, .Lround2_8_poly;

	POLY1305_BLOCK_PART1(0 * 16);

	/* Store blocks 4-7. */
	vstm Y0, Y15, STACK_Y0_Y15(%r15);

	/* Load counters for blocks 0-3. */
	vlm Y0, Y1, (STACK_CTR + 0 * 16)(%r15);

	stg POLY_RSRC, STACK_POSRC(%r15); /* %r14 used for INPUT/SRC/DST pointer. */

	lghi ROUND, 1;
	j .Lfirst_output_4blks_8_poly;

.balign 4
.Lsecond_output_4blks_8_poly:

	POLY1305_BLOCK_PART1(1 * 16);

	/* Load blocks 4-7. */
	vlm X0, X15, STACK_Y0_Y15(%r15);

	/* Load counters for blocks 4-7. */
	vlm Y0, Y1, (STACK_CTR + 2 * 16)(%r15);

	INC_POLY1305_SRC(2 * 16);
	stg POLY_RSRC, STACK_POSRC(%r15); /* %r14 used for INPUT/SRC/DST pointer. */

	lghi ROUND, 0;

.balign 4
	/* Output four chacha20 blocks and one poly1305 block per loop. */
.Lfirst_output_4blks_8_poly:
	lg %r14, STACK_INPUT(%r15);
	vlm Y12, Y15, 0(%r14);
	POLY1305_BLOCK_PART2();
	PLUS(X12, Y0);
	PLUS(X13, Y1);
	vrepf Y0, Y12, 0;
	vrepf Y1, Y12, 1;
	vrepf Y2, Y12, 2;
	vrepf Y3, Y12, 3;
	vrepf Y4, Y13, 0;
	vrepf Y5, Y13, 1;
	vrepf Y6, Y13, 2;
	vrepf Y7, Y13, 3;
	vrepf Y8, Y14, 0;
	vrepf Y9, Y14, 1;
	vrepf Y10, Y14, 2;
	vrepf Y11, Y14, 3;
	vrepf Y14, Y15, 2;
	vrepf Y15, Y15, 3;
	POLY1305_BLOCK_PART3();
	PLUS(X0, Y0);
	PLUS(X1, Y1);
	PLUS(X2, Y2);
	PLUS(X3, Y3);
	PLUS(X4, Y4);
	PLUS(X5, Y5);
	PLUS(X6, Y6);
	PLUS(X7, Y7);
	PLUS(X8, Y8);
	PLUS(X9, Y9);
	PLUS(X10, Y10);
	PLUS(X11, Y11);
	PLUS(X14, Y14);
	PLUS(X15, Y15);
	POLY1305_BLOCK_PART4();

	larl %r14, .Lconsts;
	vl Y15, (.Lbswap32 - .Lconsts)(%r14);
	TRANSPOSE_4X4_2(X0, X1, X2, X3, X4, X5, X6, X7,
			Y9, Y10, Y11, Y12, Y13, Y14);
	lg %r14, STACK_SRC(%r15);
	POLY1305_BLOCK_PART5();
	TRANSPOSE_4X4_2(X8, X9, X10, X11, X12, X13, X14, X15,
			Y9, Y10, Y11, Y12, Y13, Y14);

	vlm Y0, Y14, 0(%r14);
	POLY1305_BLOCK_PART6();
	vperm X0, X0, X0, Y15;
	vperm X1, X1, X1, Y15;
	vperm X2, X2, X2, Y15;
	vperm X3, X3, X3, Y15;
	vperm X4, X4, X4, Y15;
	vperm X5, X5, X5, Y15;
	vperm X6, X6, X6, Y15;
	vperm X7, X7, X7, Y15;
	vperm X8, X8, X8, Y15;
	vperm X9, X9, X9, Y15;
	vperm X10, X10, X10, Y15;
	vperm X11, X11, X11, Y15;
	vperm X12, X12, X12, Y15;
	vperm X13, X13, X13, Y15;
	vperm X14, X14, X14, Y15;
	vperm X15, X15, X15, Y15;
	vl Y15, (15 * 16)(%r14);
	POLY1305_BLOCK_PART7();

	aghi %r14, 256;
	stg %r14, STACK_SRC(%r15);
	lg %r14, STACK_DST(%r15);

	XOR(Y0, X0);
	XOR(Y1, X4);
	XOR(Y2, X8);
	XOR(Y3, X12);
	XOR(Y4, X1);
	XOR(Y5, X5);
	XOR(Y6, X9);
	XOR(Y7, X13);
	XOR(Y8, X2);
	XOR(Y9, X6);
	XOR(Y10, X10);
	XOR(Y11, X14);
	XOR(Y12, X3);
	XOR(Y13, X7);
	XOR(Y14, X11);
	XOR(Y15, X15);
	POLY1305_BLOCK_PART8();
	vstm Y0, Y15, 0(%r14);

	aghi %r14, 256;
	stg %r14, STACK_DST(%r15);

	lg POLY_RSRC, STACK_POSRC(%r15);

	clgije ROUND, 1, .Lsecond_output_4blks_8_poly;

	clgijhe NBLKS, 8, .Lloop8_poly;

	/* Store poly1305 state */
	lg POLY_RSTATE, STACK_POCTX(%r15);
	POLY1305_STORE_STATE();

	/* Clear the used vector registers */
	DST_8(CLEAR, 0, _);
	DST_8(CLEAR, 1, _);
	DST_8(CLEAR, 2, _);
	DST_8(CLEAR, 3, _);

	/* Clear sensitive data in stack. */
	vlm Y0, Y15, STACK_Y0_Y15(%r15);
	vlm Y0, Y3, STACK_CTR(%r15);

	END_STACK(%r14);
	xgr %r2, %r2;
	br %r14;
	CFI_ENDPROC();
ELF(.size _gcry_chacha20_poly1305_s390x_vx_blocks8,
    .-_gcry_chacha20_poly1305_s390x_vx_blocks8;)

#endif /*HAVE_GCC_INLINE_ASM_S390X_VX*/
#endif /*__s390x__*/
