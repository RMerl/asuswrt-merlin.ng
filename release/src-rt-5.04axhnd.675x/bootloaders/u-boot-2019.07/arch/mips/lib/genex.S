/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 1994 - 2000, 2001, 2003 Ralf Baechle
 * Copyright (C) 1999, 2000 Silicon Graphics, Inc.
 * Copyright (C) 2002, 2007  Maciej W. Rozycki
 * Copyright (C) 2001, 2012 MIPS Technologies, Inc.  All rights reserved.
 */

#include <asm/asm.h>
#include <asm/regdef.h>
#include <asm/mipsregs.h>
#include <asm/asm-offsets.h>

#define STATMASK 0x1f

	.set	noreorder

	/*
	 * Macros copied and adapted from Linux MIPS
	 */
	.macro	SAVE_AT
	.set	push
	.set	noat
	LONG_S	$1, PT_R1(sp)
	.set	pop
	.endm

	.macro	SAVE_TEMP
#if __mips_isa_rev < 6
	mfhi	v1
#endif
#ifdef CONFIG_32BIT
	LONG_S	$8, PT_R8(sp)
	LONG_S	$9, PT_R9(sp)
#endif
	LONG_S	$10, PT_R10(sp)
	LONG_S	$11, PT_R11(sp)
	LONG_S	$12, PT_R12(sp)
#if __mips_isa_rev < 6
	LONG_S	v1, PT_HI(sp)
	mflo	v1
#endif
	LONG_S	$13, PT_R13(sp)
	LONG_S	$14, PT_R14(sp)
	LONG_S	$15, PT_R15(sp)
	LONG_S	$24, PT_R24(sp)
#if __mips_isa_rev < 6
	LONG_S	v1, PT_LO(sp)
#endif
	.endm

	.macro	SAVE_STATIC
	LONG_S	$16, PT_R16(sp)
	LONG_S	$17, PT_R17(sp)
	LONG_S	$18, PT_R18(sp)
	LONG_S	$19, PT_R19(sp)
	LONG_S	$20, PT_R20(sp)
	LONG_S	$21, PT_R21(sp)
	LONG_S	$22, PT_R22(sp)
	LONG_S	$23, PT_R23(sp)
	LONG_S	$30, PT_R30(sp)
	.endm

	.macro	SAVE_SOME
	.set	push
	.set	noat
	PTR_SUBU k1, sp, PT_SIZE
	LONG_S	sp, PT_R29(k1)
	move	sp, k1

	LONG_S	$3, PT_R3(sp)
	LONG_S	$0, PT_R0(sp)
	mfc0	v1, CP0_STATUS
	LONG_S	$2, PT_R2(sp)
	LONG_S	v1, PT_STATUS(sp)
	LONG_S	$4, PT_R4(sp)
	mfc0	v1, CP0_CAUSE
	LONG_S	$5, PT_R5(sp)
	LONG_S	v1, PT_CAUSE(sp)
	LONG_S	$6, PT_R6(sp)
	MFC0	v1, CP0_EPC
	LONG_S	$7, PT_R7(sp)
#ifdef CONFIG_64BIT
	LONG_S	$8, PT_R8(sp)
	LONG_S	$9, PT_R9(sp)
#endif
	LONG_S	v1, PT_EPC(sp)
	LONG_S	$25, PT_R25(sp)
	LONG_S	$28, PT_R28(sp)
	LONG_S	$31, PT_R31(sp)
	.set	pop
	.endm

	.macro	RESTORE_AT
	.set	push
	.set	noat
	LONG_L	$1,  PT_R1(sp)
	.set	pop
	.endm

	.macro	RESTORE_TEMP
#if __mips_isa_rev < 6
	LONG_L	$24, PT_LO(sp)
	mtlo	$24
	LONG_L	$24, PT_HI(sp)
	mthi	$24
#endif
#ifdef CONFIG_32BIT
	LONG_L	$8, PT_R8(sp)
	LONG_L	$9, PT_R9(sp)
#endif
	LONG_L	$10, PT_R10(sp)
	LONG_L	$11, PT_R11(sp)
	LONG_L	$12, PT_R12(sp)
	LONG_L	$13, PT_R13(sp)
	LONG_L	$14, PT_R14(sp)
	LONG_L	$15, PT_R15(sp)
	LONG_L	$24, PT_R24(sp)
	.endm

	.macro	RESTORE_STATIC
	LONG_L	$16, PT_R16(sp)
	LONG_L	$17, PT_R17(sp)
	LONG_L	$18, PT_R18(sp)
	LONG_L	$19, PT_R19(sp)
	LONG_L	$20, PT_R20(sp)
	LONG_L	$21, PT_R21(sp)
	LONG_L	$22, PT_R22(sp)
	LONG_L	$23, PT_R23(sp)
	LONG_L	$30, PT_R30(sp)
	.endm

	.macro	RESTORE_SOME
	.set	push
	.set	reorder
	.set	noat
	mfc0	a0, CP0_STATUS
	ori	a0, STATMASK
	xori	a0, STATMASK
	mtc0	a0, CP0_STATUS
	li	v1, ST0_CU1 | ST0_FR | ST0_IM
	and	a0, v1
	LONG_L	v0, PT_STATUS(sp)
	nor	v1, $0, v1
	and	v0, v1
	or	v0, a0
	mtc0	v0, CP0_STATUS
	LONG_L	v1, PT_EPC(sp)
	MTC0	v1, CP0_EPC
	LONG_L	$31, PT_R31(sp)
	LONG_L	$28, PT_R28(sp)
	LONG_L	$25, PT_R25(sp)
#ifdef CONFIG_64BIT
	LONG_L	$8, PT_R8(sp)
	LONG_L	$9, PT_R9(sp)
#endif
	LONG_L	$7,  PT_R7(sp)
	LONG_L	$6,  PT_R6(sp)
	LONG_L	$5,  PT_R5(sp)
	LONG_L	$4,  PT_R4(sp)
	LONG_L	$3,  PT_R3(sp)
	LONG_L	$2,  PT_R2(sp)
	.set	pop
	.endm

	.macro	RESTORE_SP
	LONG_L	sp, PT_R29(sp)
	.endm

NESTED(except_vec3_generic, 0, sp)
	PTR_LA	k1, handle_reserved
	jr	k1
	 nop
	END(except_vec3_generic)

NESTED(except_vec_ejtag_debug, 0, sp)
	PTR_LA	k1, handle_ejtag_debug
	jr	k1
	 nop
	END(except_vec_ejtag_debug)

NESTED(handle_reserved, PT_SIZE, sp)
	SAVE_SOME
	SAVE_AT
	SAVE_TEMP
	SAVE_STATIC

	PTR_LA	t9, do_reserved
	jr	t9
	 move	a0, sp
	END(handle_reserved)

NESTED(handle_ejtag_debug, PT_SIZE, sp)
	.set	push
	.set	noat
	MTC0	k1, CP0_DESAVE

	/* Check for SDBBP */
	MFC0	k1, CP0_DEBUG
	sll	k1, k1, 30
	bgez	k1, ejtag_return
	 nop

	SAVE_SOME
	SAVE_AT
	SAVE_TEMP
	SAVE_STATIC

	PTR_LA	t9, do_ejtag_debug
	jalr	t9
	 move	a0, sp

	RESTORE_TEMP
	RESTORE_STATIC
	RESTORE_AT
	RESTORE_SOME
	RESTORE_SP

ejtag_return:
	MFC0	k1, CP0_DESAVE
	deret
	.set pop
	END(handle_ejtag_debug)
