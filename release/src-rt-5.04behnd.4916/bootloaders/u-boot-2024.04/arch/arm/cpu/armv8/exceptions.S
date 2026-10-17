/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2013
 * David Feng <fenghua@phytium.com.cn>
 */

#include <asm-offsets.h>
#include <config.h>
#include <asm/ptrace.h>
#include <asm/macro.h>
#include <linux/linkage.h>

/*
 * AArch64 exception vectors:
 * We have four types of exceptions:
 * - synchronous: traps, data aborts, undefined instructions, ...
 * - IRQ: group 1 (normal) interrupts
 * - FIQ: group 0 or secure interrupts
 * - SError: fatal system errors
 * There are entries for all four of those for different contexts:
 * - from same exception level, when using the SP_EL0 stack pointer
 * - from same exception level, when using the SP_ELx stack pointer
 * - from lower exception level, when this is AArch64
 * - from lower exception level, when this is AArch32
 * Each of those 16 entries have space for 32 instructions, each entry must
 * be 128 byte aligned, the whole table must be 2K aligned.
 * The 32 instructions are not enough to save and restore all registers and
 * to branch to the actual handler, so we split this up:
 * Each entry saves the LR, branches to the save routine, then to the actual
 * handler, then to the restore routine. The save and restore routines are
 * each split in half and stuffed in the unused gap between the entries.
 * Also as we do not run anything in a lower exception level, we just provide
 * the first 8 entries for exceptions from the same EL.
 */
	.align	11
	.globl	vectors
vectors:
	.align	7		/* Current EL Synchronous Thread */
	stp	x29, x30, [sp, #-16]!
	bl	_exception_entry
	bl	do_bad_sync
	b	exception_exit

/*
 * Save (most of) the GP registers to the stack frame.
 * This is the first part of the shared routine called into from all entries.
 */
_exception_entry:
	stp	x27, x28, [sp, #-16]!
	stp	x25, x26, [sp, #-16]!
	stp	x23, x24, [sp, #-16]!
	stp	x21, x22, [sp, #-16]!
	stp	x19, x20, [sp, #-16]!
	stp	x17, x18, [sp, #-16]!
	stp	x15, x16, [sp, #-16]!
	stp	x13, x14, [sp, #-16]!
	stp	x11, x12, [sp, #-16]!
	stp	x9, x10, [sp, #-16]!
	stp	x7, x8, [sp, #-16]!
	stp	x5, x6, [sp, #-16]!
	stp	x3, x4, [sp, #-16]!
	stp	x1, x2, [sp, #-16]!
	b	_save_el_regs			/* jump to the second part */

	.align	7		/* Current EL IRQ Thread */
	stp	x29, x30, [sp, #-16]!
	bl	_exception_entry
	bl	do_bad_irq
	b	exception_exit

/*
 * Save exception specific context: ESR and ELR, for all exception levels.
 * This is the second part of the shared routine called into from all entries.
 */
_save_el_regs:
	/* Could be running at EL3/EL2/EL1 */
	switch_el x11, 3f, 2f, 1f
3:	mrs	x1, esr_el3
	mrs	x2, elr_el3
	mrs	x3, spsr_el3
	b	0f
2:	mrs	x1, esr_el2
	mrs	x2, elr_el2
	mrs	x3, spsr_el2
	b	0f
1:	mrs	x1, esr_el1
	mrs	x2, elr_el1
	mrs	x3, spsr_el1
0:
	stp	x1, x0, [sp, #-16]!
	stp	x3, x2, [sp, #-16]!
	mov	x0, sp
	ret

	.align	7		/* Current EL FIQ Thread */
	stp	x29, x30, [sp, #-16]!
	bl	_exception_entry
	bl	do_bad_fiq
				/* falling through to _exception_exit */
/*
 * Restore the exception return address, for all exception levels.
 * This is the first part of the shared routine called into from all entries.
 */
exception_exit:
	ldp	xzr, x2, [sp],#16
	switch_el x11, 3f, 2f, 1f
3:	msr	elr_el3, x2
	b	_restore_regs
2:	msr	elr_el2, x2
	b	_restore_regs
1:	msr	elr_el1, x2
	b	_restore_regs		/* jump to the second part */

	.align	7		/* Current EL Error Thread */
	stp	x29, x30, [sp, #-16]!
	bl	_exception_entry
	bl	do_bad_error
	b	exception_exit

/*
 * Restore the general purpose registers from the exception stack, then return.
 * This is the second part of the shared routine called into from all entries.
 */
_restore_regs:
	ldp	xzr, x0, [sp],#16
	ldp	x1, x2, [sp],#16
	ldp	x3, x4, [sp],#16
	ldp	x5, x6, [sp],#16
	ldp	x7, x8, [sp],#16
	ldp	x9, x10, [sp],#16
	ldp	x11, x12, [sp],#16
	ldp	x13, x14, [sp],#16
	ldp	x15, x16, [sp],#16
	ldp	x17, x18, [sp],#16
	ldp	x19, x20, [sp],#16
	ldp	x21, x22, [sp],#16
	ldp	x23, x24, [sp],#16
	ldp	x25, x26, [sp],#16
	ldp	x27, x28, [sp],#16
	ldp	x29, x30, [sp],#16
	eret

	.align	7		 /* Current EL (SP_ELx) Synchronous Handler */
	stp	x29, x30, [sp, #-16]!
	bl	_exception_entry
	bl	do_sync
	b	exception_exit

	.align	7		 /* Current EL (SP_ELx) IRQ Handler */
	stp	x29, x30, [sp, #-16]!
	bl	_exception_entry
	bl	do_irq
	b	exception_exit

	.align	7		 /* Current EL (SP_ELx) FIQ Handler */
	stp	x29, x30, [sp, #-16]!
	bl	_exception_entry
	bl	do_fiq
	b	exception_exit

	.align	7		 /* Current EL (SP_ELx) Error Handler */
	stp	x29, x30, [sp, #-16]!
	bl	_exception_entry
	bl	do_error
	b	exception_exit
