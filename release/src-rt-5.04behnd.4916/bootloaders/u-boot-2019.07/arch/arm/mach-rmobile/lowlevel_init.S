/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2012 Nobuhiro Iwamatsu <nobuhiro.Iwamatsu.yj@renesas.com>
 * Copyright (C) 2012 Renesas Solutions Corp.
 */

#include <config.h>
#include <linux/linkage.h>

ENTRY(lowlevel_init)
	ldr		r0, =MERAM_BASE
	mov		r1, #0x0
	str		r1, [r0]

	mrc		p15, 0, r0, c0, c0, 5
	ands	r0, r0, #0xF
	beq		lowlevel_init__
	b		wait_interrupt

	.pool
	.align 4

wait_interrupt:
#ifdef ICCICR
	ldr     r1, =ICCICR
	mov     r2, #0x0
	str     r2, [r1]
	mov     r2, #0xF0
	adds    r1, r1, #4 /* ICCPMR */
	str     r2, [r1]
	ldr     r1, =ICCICR
	mov     r2, #0x1
	str     r2, [r1]
#endif

wait_loop:
	.long	0xE320F003 /* wfi */

	ldr		r2, [r1, #0xC]
	str		r2, [r1, #0x10]

	ldr		r0, =MERAM_BASE
	ldr		r2, [r0]
	cmp		r2, #0
	movne	pc, r2

	b		wait_loop

wait_loop_end:
	.pool
	.align 4

lowlevel_init__:

	mov r0, #0x200000

loop0:
	subs r0, r0, #1
	bne  loop0

	ldr sp, MERAM_STACK
	b s_init

	.pool
	.align 4

ENDPROC(lowlevel_init)
	.ltorg

MERAM_STACK:
	.word LOW_LEVEL_MERAM_STACK
