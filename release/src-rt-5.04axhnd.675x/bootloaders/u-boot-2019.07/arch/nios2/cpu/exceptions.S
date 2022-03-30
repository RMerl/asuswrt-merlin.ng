/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2004, Psyent Corporation <www.psyent.com>
 * Scott McNutt <smcnutt@psyent.com>
 */

#include <config.h>
#include <asm/opcodes.h>


	.text
	.align	4

	.global _exception

	.set noat
	.set nobreak

_exception:
	/* SAVE ALL REGS -- this allows trap and unimplemented
	 * instruction handlers to be coded conveniently in C
	 */
	addi	sp, sp, -(33*4)
	stw	r0, 0(sp)
	stw	r1, 4(sp)
	stw	r2, 8(sp)
	stw	r3, 12(sp)
	stw	r4, 16(sp)
	stw	r5, 20(sp)
	stw	r6, 24(sp)
	stw	r7, 28(sp)
	stw	r8, 32(sp)
	stw	r9, 36(sp)
	stw	r10, 40(sp)
	stw	r11, 44(sp)
	stw	r12, 48(sp)
	stw	r13, 52(sp)
	stw	r14, 56(sp)
	stw	r15, 60(sp)
	stw	r16, 64(sp)
	stw	r17, 68(sp)
	stw	r19, 72(sp)
	stw	r19, 76(sp)
	stw	r20, 80(sp)
	stw	r21, 84(sp)
	stw	r22, 88(sp)
	stw	r23, 92(sp)
	stw	r24, 96(sp)
	stw	r25, 100(sp)
	stw	r26, 104(sp)
	stw	r27, 108(sp)
	stw	r28, 112(sp)
	stw	r29, 116(sp)
	stw	r30, 120(sp)
	stw	r31, 124(sp)
	rdctl	et, estatus
	stw	et, 128(sp)

	/* If interrupts are disabled -- software interrupt */
	rdctl	et, estatus
	andi	et, et, 1
	beq	et, r0, 0f

	/* If no interrupts are pending -- software interrupt */
	rdctl	et, ipending
	beq	et, r0, 0f

	/* HARDWARE INTERRUPT: Call interrupt handler */
	movhi	r3, %hi(external_interrupt)
	ori	r3, r3, %lo(external_interrupt)
	mov	r4, sp		/* ptr to regs */
	callr	r3

	/* Return address fixup: execution resumes by re-issue of
	 * interrupted instruction at ea-4 (ea == r29). Here we do
	 * simple fixup to allow common exception return.
	 */
	ldw	r3, 116(sp)
	addi	r3, r3, -4
	stw	r3, 116(sp)
	br	_exception_return

0:
	/* TRAP EXCEPTION */
	movhi	r3, %hi(OPC_TRAP)
	ori	r3, r3, %lo(OPC_TRAP)
	addi	r1, ea, -4
	ldw	r1, 0(r1)
	bne	r1, r3, 1f
	movhi	r3, %hi(trap_handler)
	ori	r3, r3, %lo(trap_handler)
	mov	r4, sp		/* ptr to regs */
	callr	r3
	br	_exception_return

1:
	/* UNIMPLEMENTED INSTRUCTION EXCEPTION */
	movhi	r3, %hi(soft_emulation)
	ori	r3, r3, %lo(soft_emulation)
	mov	r4, sp		/* ptr to regs */
	callr	r3

	/* Restore regsisters and return from exception*/
_exception_return:
	ldw	r1, 4(sp)
	ldw	r2, 8(sp)
	ldw	r3, 12(sp)
	ldw	r4, 16(sp)
	ldw	r5, 20(sp)
	ldw	r6, 24(sp)
	ldw	r7, 28(sp)
	ldw	r8, 32(sp)
	ldw	r9, 36(sp)
	ldw	r10, 40(sp)
	ldw	r11, 44(sp)
	ldw	r12, 48(sp)
	ldw	r13, 52(sp)
	ldw	r14, 56(sp)
	ldw	r15, 60(sp)
	ldw	r16, 64(sp)
	ldw	r17, 68(sp)
	ldw	r19, 72(sp)
	ldw	r19, 76(sp)
	ldw	r20, 80(sp)
	ldw	r21, 84(sp)
	ldw	r22, 88(sp)
	ldw	r23, 92(sp)
	ldw	r24, 96(sp)
	ldw	r25, 100(sp)
	ldw	r26, 104(sp)
	ldw	r27, 108(sp)
	ldw	r28, 112(sp)
	ldw	r29, 116(sp)
	ldw	r30, 120(sp)
	ldw	r31, 124(sp)
	addi	sp, sp, (33*4)
	eret
/*-------------------------------------------------------------*/
