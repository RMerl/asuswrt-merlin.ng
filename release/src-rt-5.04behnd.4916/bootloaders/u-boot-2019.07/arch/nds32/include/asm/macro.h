/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * include/asm-nds32/macro.h
 *
 * Copyright (C) 2009 Jean-Christophe PLAGNIOL-VILLARD <plagnioj@jcrosoft.com>
 * Copyright (C) 2011 Andes Technology Corporation
 * Macpaul Lin, Andes Technology Corporation <macpaul@andestech.com>
 */

#ifndef __ASM_NDS_MACRO_H
#define __ASM_NDS_MACRO_H
#ifdef __ASSEMBLY__

/*
 * These macros provide a convenient way to write 8, 16 and 32 bit data
 * to an "immediate address (address used by periphal)" only.
 * Registers r4 and r5 are used, any data in these registers are
 * overwritten by the macros.
 * The macros are valid for any NDS32 architecture, they do not implement
 * any memory barriers so caution is recommended when using these when the
 * caches are enabled or on a multi-core system.
 */

.macro	write32, addr, data
	li	$r4, \addr
	li	$r5, \data
	swi	$r5, [$r4]
.endm

.macro	write16, addr, data
	li	$r4, \addr
	li	$r5, \data
	shi	$r5, [$r4]
.endm

.macro	write8, addr, data
	li	$r4, \addr
	li	$r5, \data
	sbi	$r5, [$r4]
.endm

/*
 * This macro read a value from a register, then do OR operation
 * (set bit fields) to the value, and then store it back to the register.
 * Note: Instruction 'ori' supports immediate value up to 15 bits.
 */
.macro	setbf32, addr, data
	li	$r4, \addr
	lwi	$r5, [$r4]
	li	$r6, \data
	or	$r5, $r5, $r6
	swi	$r5, [$r4]
.endm

.macro	setbf15, addr, data
	li	$r4, \addr
	lwi	$r5, [$r4]
	ori	$r5, $r5, \data
	swi	$r5, [$r4]
.endm

/*
 * This macro generates a loop that can be used for delays in the code.
 * Register r4 is used, any data in this register is overwritten by the
 * macro.
 * The macro is valid for any NDS32 architeture. The actual time spent in the
 * loop will vary from CPU to CPU though.
 */

.macro	wait_timer, time
	li	$r4, \time
1:
	nop
	addi	$r4, $r4, -1
	bnez    $r4, 1b
.endm

#endif /* __ASSEMBLY__ */
#endif /* __ASM_ARM_MACRO_H */
