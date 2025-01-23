/* asm-common-aarch64.h  -  Poly1305 macros for ARMv8/AArch64 assembly
 *
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

#ifndef GCRY_ASM_POLY1305_AARCH64_H
#define GCRY_ASM_POLY1305_AARCH64_H

#include "asm-common-aarch64.h"

#ifdef __AARCH64EL__
  #define le_to_host(reg) /*_*/
#else
  #define le_to_host(reg) rev reg, reg;
#endif

/**********************************************************************
  poly1305 for stitched chacha20-poly1305 Aarch64 implementations
 **********************************************************************/

#define POLY_RSTATE    x8
#define POLY_RSRC      x9

#define POLY_R_H0      x10
#define POLY_R_H1      x11
#define POLY_R_H2      x12
#define POLY_R_H2d     w12
#define POLY_R_R0      x13
#define POLY_R_R1      x14
#define POLY_R_R1_MUL5 x15
#define POLY_R_X0_HI   x16
#define POLY_R_X0_LO   x17
#define POLY_R_X1_HI   x19
#define POLY_R_X1_LO   x20
#define POLY_R_ONE     x21
#define POLY_R_ONEd    w21

#define POLY_TMP0      x22
#define POLY_TMP1      x23
#define POLY_TMP2      x24
#define POLY_TMP3      x25

#define POLY_CHACHA_ROUND x26

#define POLY_S_R0      (4 * 4 + 0 * 8)
#define POLY_S_R1      (4 * 4 + 1 * 8)
#define POLY_S_H0      (4 * 4 + 2 * 8 + 0 * 8)
#define POLY_S_H1      (4 * 4 + 2 * 8 + 1 * 8)
#define POLY_S_H2d     (4 * 4 + 2 * 8 + 2 * 8)

#define POLY1305_PUSH_REGS() \
	stp x19, x20, [sp, #-16]!; \
	CFI_ADJUST_CFA_OFFSET(16); \
	CFI_REG_ON_STACK(19, 0); \
	CFI_REG_ON_STACK(20, 8); \
	stp x21, x22, [sp, #-16]!; \
	CFI_ADJUST_CFA_OFFSET(16); \
	CFI_REG_ON_STACK(21, 0); \
	CFI_REG_ON_STACK(22, 8); \
	stp x23, x24, [sp, #-16]!; \
	CFI_ADJUST_CFA_OFFSET(16); \
	CFI_REG_ON_STACK(23, 0); \
	CFI_REG_ON_STACK(24, 8); \
	stp x25, x26, [sp, #-16]!; \
	CFI_ADJUST_CFA_OFFSET(16); \
	CFI_REG_ON_STACK(25, 0); \
	CFI_REG_ON_STACK(26, 8);

#define POLY1305_POP_REGS() \
	ldp x25, x26, [sp], #16; \
	CFI_ADJUST_CFA_OFFSET(-16); \
	CFI_RESTORE(x25); \
	CFI_RESTORE(x26); \
	ldp x23, x24, [sp], #16; \
	CFI_ADJUST_CFA_OFFSET(-16); \
	CFI_RESTORE(x23); \
	CFI_RESTORE(x24); \
	ldp x21, x22, [sp], #16; \
	CFI_ADJUST_CFA_OFFSET(-16); \
	CFI_RESTORE(x21); \
	CFI_RESTORE(x22); \
	ldp x19, x20, [sp], #16; \
	CFI_ADJUST_CFA_OFFSET(-16); \
	CFI_RESTORE(x19); \
	CFI_RESTORE(x20);

#define POLY1305_LOAD_STATE() \
	ldr POLY_R_R1, [POLY_RSTATE, #(POLY_S_R1)]; \
	ldr POLY_R_H0, [POLY_RSTATE, #(POLY_S_H0)];  \
	ldr POLY_R_H1, [POLY_RSTATE, #(POLY_S_H1)]; \
	ldr POLY_R_H2d, [POLY_RSTATE, #(POLY_S_H2d)]; \
	ldr POLY_R_R0, [POLY_RSTATE, #(POLY_S_R0)]; \
	add POLY_R_R1_MUL5, POLY_R_R1, POLY_R_R1, lsr #2; \
	mov POLY_R_ONE, #1;

#define POLY1305_STORE_STATE() \
	str POLY_R_H0, [POLY_RSTATE, #(POLY_S_H0)]; \
	str POLY_R_H1, [POLY_RSTATE, #(POLY_S_H1)]; \
	str POLY_R_H2d, [POLY_RSTATE, #(POLY_S_H2d)];

#define POLY1305_BLOCK_PART1(src_offset) \
	/* a = h + m */ \
	ldr POLY_TMP0, [POLY_RSRC, #((src_offset) + 0 * 8)];
#define POLY1305_BLOCK_PART2(src_offset) \
	ldr POLY_TMP1, [POLY_RSRC, #((src_offset) + 1 * 8)];
#define POLY1305_BLOCK_PART3() \
	le_to_host(POLY_TMP0);
#define POLY1305_BLOCK_PART4() \
	le_to_host(POLY_TMP1);
#define POLY1305_BLOCK_PART5() \
	adds POLY_R_H0, POLY_R_H0, POLY_TMP0;
#define POLY1305_BLOCK_PART6() \
	adcs POLY_R_H1, POLY_R_H1, POLY_TMP1;
#define POLY1305_BLOCK_PART7() \
	adc POLY_R_H2d, POLY_R_H2d, POLY_R_ONEd;

#define POLY1305_BLOCK_PART8() \
	/* h = a * r (partial mod 2^130-5): */ \
	mul POLY_R_X1_LO, POLY_R_H0, POLY_R_R1;   /* lo: h0 * r1 */
#define POLY1305_BLOCK_PART9() \
	mul POLY_TMP0, POLY_R_H1, POLY_R_R0;      /* lo: h1 * r0 */
#define POLY1305_BLOCK_PART10() \
	mul POLY_R_X0_LO, POLY_R_H0, POLY_R_R0;   /* lo: h0 * r0 */
#define POLY1305_BLOCK_PART11() \
	umulh POLY_R_X1_HI, POLY_R_H0, POLY_R_R1; /* hi: h0 * r1 */
#define POLY1305_BLOCK_PART12() \
	adds POLY_R_X1_LO, POLY_R_X1_LO, POLY_TMP0;
#define POLY1305_BLOCK_PART13() \
	umulh POLY_TMP1, POLY_R_H1, POLY_R_R0;    /* hi: h1 * r0 */
#define POLY1305_BLOCK_PART14() \
	mul POLY_TMP2, POLY_R_H1, POLY_R_R1_MUL5;   /* lo: h1 * r1 mod 2^130-5 */
#define POLY1305_BLOCK_PART15() \
	umulh POLY_R_X0_HI, POLY_R_H0, POLY_R_R0; /* hi: h0 * r0 */
#define POLY1305_BLOCK_PART16() \
	adc POLY_R_X1_HI, POLY_R_X1_HI, POLY_TMP1;
#define POLY1305_BLOCK_PART17() \
	umulh POLY_TMP3, POLY_R_H1, POLY_R_R1_MUL5; /* hi: h1 * r1 mod 2^130-5 */
#define POLY1305_BLOCK_PART18() \
	adds POLY_R_X0_LO, POLY_R_X0_LO, POLY_TMP2;
#define POLY1305_BLOCK_PART19() \
	mul POLY_R_H1, POLY_R_H2, POLY_R_R1_MUL5; /* h2 * r1 mod 2^130-5 */
#define POLY1305_BLOCK_PART20() \
	adc POLY_R_X0_HI, POLY_R_X0_HI, POLY_TMP3;
#define POLY1305_BLOCK_PART21() \
	mul POLY_R_H2, POLY_R_H2, POLY_R_R0;      /* h2 * r0 */
#define POLY1305_BLOCK_PART22() \
	adds POLY_R_H1, POLY_R_H1, POLY_R_X1_LO;
#define POLY1305_BLOCK_PART23() \
	adc POLY_R_H0, POLY_R_H2, POLY_R_X1_HI;

#define POLY1305_BLOCK_PART24() \
	/* carry propagation */ \
	and POLY_R_H2, POLY_R_H0, #3;
#define POLY1305_BLOCK_PART25() \
	lsr POLY_R_H0, POLY_R_H0, #2;
#define POLY1305_BLOCK_PART26() \
	add POLY_R_H0, POLY_R_H0, POLY_R_H0, lsl #2;
#define POLY1305_BLOCK_PART27() \
	adds POLY_R_H0, POLY_R_H0, POLY_R_X0_LO;
#define POLY1305_BLOCK_PART28() \
	adcs POLY_R_H1, POLY_R_H1, POLY_R_X0_HI;
#define POLY1305_BLOCK_PART29() \
	adc POLY_R_H2d, POLY_R_H2d, wzr;

//#define TESTING_POLY1305_ASM
#ifdef TESTING_POLY1305_ASM
/* for testing only. */
.align 3
.globl _gcry_poly1305_aarch64_blocks1
ELF(.type _gcry_poly1305_aarch64_blocks1,%function;)
_gcry_poly1305_aarch64_blocks1:
	/* input:
	 *	x0: poly1305-state
	 *	x1: src
	 *	x2: nblks
	 */
	CFI_STARTPROC()
	POLY1305_PUSH_REGS();

	mov POLY_RSTATE, x0;
	mov POLY_RSRC, x1;

	POLY1305_LOAD_STATE();

.L_gcry_poly1305_aarch64_loop1:
	POLY1305_BLOCK_PART1(0 * 16);
	POLY1305_BLOCK_PART2(0 * 16);
	add POLY_RSRC, POLY_RSRC, #16;
	POLY1305_BLOCK_PART3();
	POLY1305_BLOCK_PART4();
	POLY1305_BLOCK_PART5();
	POLY1305_BLOCK_PART6();
	POLY1305_BLOCK_PART7();
	POLY1305_BLOCK_PART8();
	POLY1305_BLOCK_PART9();
	POLY1305_BLOCK_PART10();
	POLY1305_BLOCK_PART11();
	POLY1305_BLOCK_PART12();
	POLY1305_BLOCK_PART13();
	POLY1305_BLOCK_PART14();
	POLY1305_BLOCK_PART15();
	POLY1305_BLOCK_PART16();
	POLY1305_BLOCK_PART17();
	POLY1305_BLOCK_PART18();
	POLY1305_BLOCK_PART19();
	POLY1305_BLOCK_PART20();
	POLY1305_BLOCK_PART21();
	POLY1305_BLOCK_PART22();
	POLY1305_BLOCK_PART23();
	POLY1305_BLOCK_PART24();
	POLY1305_BLOCK_PART25();
	POLY1305_BLOCK_PART26();
	POLY1305_BLOCK_PART27();
	POLY1305_BLOCK_PART28();
	POLY1305_BLOCK_PART29();

	subs x2, x2, #1;
	b.ne .L_gcry_poly1305_aarch64_loop1;

	POLY1305_STORE_STATE();

	mov x0, #0;

	POLY1305_POP_REGS();
	ret_spec_stop;
	CFI_ENDPROC()
ELF(.size _gcry_poly1305_aarch64_blocks1, .-_gcry_poly1305_aarch64_blocks1;)
#endif

#endif /* GCRY_ASM_POLY1305_AARCH64_H */
