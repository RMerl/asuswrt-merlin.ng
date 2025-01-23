/* asm-common-amd64.h  -  Poly1305 macros for zSeries assembly
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

#ifndef GCRY_ASM_POLY1305_S390X_H
#define GCRY_ASM_POLY1305_S390X_H

#include "asm-common-s390x.h"

/**********************************************************************
  poly1305 for stitched chacha20-poly1305
 **********************************************************************/

#define POLY_RSTATE       %r1
#define POLY_RSRC         %r14

#define POLY_R_H0_TMP_HI  %r6  // even-
#define POLY_R_H0         %r7  //      odd pair
#define POLY_R_H1_TMP_HI  %r8  // even-
#define POLY_R_H1         %r9  //      odd pair
#define POLY_R_H2         %r10
#define POLY_R_R0         %r11
#define POLY_R_R1         %r12
#define POLY_R_R1_MUL5    %r13
#define POLY_R_X0_HI      %r2  // even-
#define POLY_R_X0_LO      %r3  //      odd pair
#define POLY_R_X1_HI      %r4  // even-
#define POLY_R_X1_LO      %r5  //      odd pair

#define POLY_S_R0      (4 * 4 + 0 * 8)(POLY_RSTATE)
#define POLY_S_R1      (4 * 4 + 1 * 8)(POLY_RSTATE)
#define POLY_S_H0      (4 * 4 + 2 * 8 + 0 * 8)(POLY_RSTATE)
#define POLY_S_H1      (4 * 4 + 2 * 8 + 1 * 8)(POLY_RSTATE)
#define POLY_S_H2d     (4 * 4 + 2 * 8 + 2 * 8)(POLY_RSTATE)

#define INC_POLY1305_SRC(a) \
	aghi POLY_RSRC, (a);

#define POLY1305_LOAD_STATE() \
	lg POLY_R_H0, POLY_S_H0; \
	lg POLY_R_H1, POLY_S_H1; \
	llgf POLY_R_H2, POLY_S_H2d; \
	rllg POLY_R_H0, POLY_R_H0, 32; \
	rllg POLY_R_H1, POLY_R_H1, 32; \
	lg POLY_R_R0, POLY_S_R0; \
	lg POLY_R_R1, POLY_S_R1; \
	rllg POLY_R_R0, POLY_R_R0, 32; \
	rllg POLY_R_R1, POLY_R_R1, 32; \
	srlg POLY_R_R1_MUL5, POLY_R_R1, 2; \
	algr POLY_R_R1_MUL5, POLY_R_R1;

#define POLY1305_STORE_STATE() \
	rllg POLY_R_H0, POLY_R_H0, 32; \
	rllg POLY_R_H1, POLY_R_H1, 32; \
	stg POLY_R_H0, POLY_S_H0; \
	stg POLY_R_H1, POLY_S_H1; \
	st POLY_R_H2, POLY_S_H2d;

/* a = h + m */
#define POLY1305_BLOCK_PART1_HB(src_offset, high_pad) \
	lrvg POLY_R_X0_HI, ((src_offset) + 1 * 8)(POLY_RSRC); \
	lrvg POLY_R_X0_LO, ((src_offset) + 0 * 8)(POLY_RSRC); \
	lghi POLY_R_H1_TMP_HI, (high_pad);

#define POLY1305_BLOCK_PART1(src_offset) \
	POLY1305_BLOCK_PART1_HB(src_offset, 1);

#define POLY1305_BLOCK_PART2() \
	algr POLY_R_H0, POLY_R_X0_LO; \
	alcgr POLY_R_H1, POLY_R_X0_HI; \
	alcgr POLY_R_H2, POLY_R_H1_TMP_HI; \
	lgr POLY_R_X1_LO, POLY_R_H0; \
	lgr POLY_R_X0_LO, POLY_R_H0;

#define POLY1305_BLOCK_PART3() \
	/* h = a * r (partial mod 2^130-5): */ \
	\
	/* h0 * r1 */ \
	mlgr POLY_R_X1_HI, POLY_R_R1; \
	\
	/* h1 * r0 */ \
	lgr POLY_R_H0, POLY_R_H1; \
	mlgr POLY_R_H0_TMP_HI, POLY_R_R0; \
	\
	/* h1 * r1 mod 2^130-5 */ \
	mlgr POLY_R_H1_TMP_HI, POLY_R_R1_MUL5;

#define POLY1305_BLOCK_PART4() \
	\
	/* h0 * r0 */ \
	mlgr POLY_R_X0_HI, POLY_R_R0; \
	\
	algr POLY_R_X1_LO, POLY_R_H0; \
	alcgr POLY_R_X1_HI, POLY_R_H0_TMP_HI; \
	\
	lgr POLY_R_H0_TMP_HI, POLY_R_H2; \
	msgr POLY_R_H0_TMP_HI, POLY_R_R1_MUL5; /* h2 * r1 mod 2^130-5 */ \
	msgr POLY_R_H2, POLY_R_R0;             /* h2 * r0 */

#define POLY1305_BLOCK_PART5() \
	\
	algr POLY_R_X0_LO, POLY_R_H1; \
	alcgr POLY_R_X0_HI, POLY_R_H1_TMP_HI;

#define POLY1305_BLOCK_PART6() \
	\
	algrk POLY_R_H1, POLY_R_H0_TMP_HI, POLY_R_X1_LO; \
	alcgr POLY_R_H2, POLY_R_X1_HI;

#define POLY1305_BLOCK_PART7() \
	\
	/* carry propagation */ \
	srlg POLY_R_H0, POLY_R_H2, 2; \
	risbgn POLY_R_X1_LO, POLY_R_H2, 0, 0x80 | 61, 0; \
	lghi POLY_R_H1_TMP_HI, 0; \
	agr POLY_R_H0, POLY_R_X1_LO; \
	risbgn POLY_R_H2, POLY_R_H2, 62, 0x80 | 63, 0;

#define POLY1305_BLOCK_PART8() \
	algr POLY_R_H0, POLY_R_X0_LO; \
	alcgr POLY_R_H1, POLY_R_X0_HI; \
	alcgr POLY_R_H2, POLY_R_H1_TMP_HI;

#endif /* GCRY_ASM_POLY1305_AMD64_H */
