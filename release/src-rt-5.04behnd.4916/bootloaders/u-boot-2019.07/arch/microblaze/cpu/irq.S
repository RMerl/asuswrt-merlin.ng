/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2007 Michal Simek
 *
 * Michal  SIMEK <monstr@monstr.eu>
 */

#include <config.h>
#include <asm/asm.h>
	.text
	.global _interrupt_handler
_interrupt_handler:
	addik	r1, r1, -124
	swi	r2, r1, 4
	swi	r3, r1, 8
	swi	r4, r1, 12
	swi	r5, r1, 16
	swi	r6, r1, 20
	swi	r7, r1, 24
	swi	r8, r1, 28
	swi	r9, r1, 32
	swi	r10, r1, 36
	swi	r11, r1, 40
	swi	r12, r1, 44
	swi	r13, r1, 48
	swi	r14, r1, 52
	swi	r15, r1, 56
	swi	r16, r1, 60
	swi	r17, r1, 64
	swi	r18, r1, 68
	swi	r19, r1, 72
	swi	r20, r1, 76
	swi	r21, r1, 80
	swi	r22, r1, 84
	swi	r23, r1, 88
	swi	r24, r1, 92
	swi	r25, r1, 96
	swi	r26, r1, 100
	swi	r27, r1, 104
	swi	r28, r1, 108
	swi	r29, r1, 112
	swi	r30, r1, 116
	swi	r31, r1, 120
	brlid	r15, interrupt_handler
	nop
	lwi	r31, r1, 120
	lwi	r30, r1, 116
	lwi	r29, r1, 112
	lwi	r28, r1, 108
	lwi	r27, r1, 104
	lwi	r26, r1, 100
	lwi	r25, r1, 96
	lwi	r24, r1, 92
	lwi	r23, r1, 88
	lwi	r22, r1, 84
	lwi	r21, r1, 80
	lwi	r20, r1, 76
	lwi	r19, r1, 72
	lwi	r18, r1, 68
	lwi	r17, r1, 64
	lwi	r16, r1, 60
	lwi	r15, r1, 56
	lwi	r14, r1, 52
	lwi	r13, r1, 48
	lwi	r12, r1, 44
	lwi	r11, r1, 40
	lwi	r10, r1, 36
	lwi	r9, r1, 32
	lwi	r8, r1, 28
	lwi	r7, r1, 24
	lwi	r6, r1, 20
	lwi	r5, r1, 16
	lwi	r4, r1, 12
	lwi	r3, r1, 8
	lwi	r2, r1, 4
	addik	r1, r1, 124
	rtid	r14, 0
	nop
	.size _interrupt_handler,.-_interrupt_handler
