/* SPDX-License-Identifier: GPL-2.0 */
/*
 *  linux/arch/arm/lib/memcpy.S
 *
 *  Author:	Nicolas Pitre
 *  Created:	Sep 28, 2005
 *  Copyright:	MontaVista Software, Inc.
 */

#include <linux/linkage.h>
#include <asm/assembler.h>

#define LDR1W_SHIFT	0
#define STR1W_SHIFT	0

	.macro ldr1w ptr reg abort
	W(ldr) \reg, [\ptr], #4
	.endm

	.macro ldr4w ptr reg1 reg2 reg3 reg4 abort
	ldmia \ptr!, {\reg1, \reg2, \reg3, \reg4}
	.endm

	.macro ldr8w ptr reg1 reg2 reg3 reg4 reg5 reg6 reg7 reg8 abort
	ldmia \ptr!, {\reg1, \reg2, \reg3, \reg4, \reg5, \reg6, \reg7, \reg8}
	.endm

	.macro ldr1b ptr reg cond=al abort
	ldrb\cond\() \reg, [\ptr], #1
	.endm

	.macro str1w ptr reg abort
	W(str) \reg, [\ptr], #4
	.endm

	.macro str8w ptr reg1 reg2 reg3 reg4 reg5 reg6 reg7 reg8 abort
	stmia \ptr!, {\reg1, \reg2, \reg3, \reg4, \reg5, \reg6, \reg7, \reg8}
	.endm

	.macro str1b ptr reg cond=al abort
	strb\cond\() \reg, [\ptr], #1
	.endm

	.macro enter reg1 reg2
	stmdb sp!, {r0, \reg1, \reg2}
	.endm

	.macro exit reg1 reg2
	ldmfd sp!, {r0, \reg1, \reg2}
	.endm

	.text

/* Prototype: void *memcpy(void *dest, const void *src, size_t n); */
	.syntax unified
#if CONFIG_IS_ENABLED(SYS_THUMB_BUILD) && !defined(MEMCPY_NO_THUMB_BUILD)
	.thumb
	.thumb_func
#endif
ENTRY(memcpy)
		cmp	r0, r1
		bxeq	lr

		enter	r4, lr

		subs	r2, r2, #4
		blt	8f
		ands	ip, r0, #3
	PLD(	pld	[r1, #0]		)
		bne	9f
		ands	ip, r1, #3
		bne	10f

1:		subs	r2, r2, #(28)
		stmfd	sp!, {r5 - r8}
		blt	5f

	CALGN(	ands	ip, r0, #31		)
	CALGN(	rsb	r3, ip, #32		)
	CALGN(	sbcsne	r4, r3, r2		)  @ C is always set here
	CALGN(	bcs	2f			)
	CALGN(	adr	r4, 6f			)
	CALGN(	subs	r2, r2, r3		)  @ C gets set
	CALGN(	add	pc, r4, ip		)

	PLD(	pld	[r1, #0]		)
2:	PLD(	subs	r2, r2, #96		)
	PLD(	pld	[r1, #28]		)
	PLD(	blt	4f			)
	PLD(	pld	[r1, #60]		)
	PLD(	pld	[r1, #92]		)

3:	PLD(	pld	[r1, #124]		)
4:		ldr8w	r1, r3, r4, r5, r6, r7, r8, ip, lr, abort=20f
		subs	r2, r2, #32
		str8w	r0, r3, r4, r5, r6, r7, r8, ip, lr, abort=20f
		bge	3b
	PLD(	cmn	r2, #96			)
	PLD(	bge	4b			)

5:		ands	ip, r2, #28
		rsb	ip, ip, #32
#if LDR1W_SHIFT > 0
		lsl	ip, ip, #LDR1W_SHIFT
#endif
		addne	pc, pc, ip		@ C is always clear here
		b	7f
6:
		.rept	(1 << LDR1W_SHIFT)
		W(nop)
		.endr
		ldr1w	r1, r3, abort=20f
		ldr1w	r1, r4, abort=20f
		ldr1w	r1, r5, abort=20f
		ldr1w	r1, r6, abort=20f
		ldr1w	r1, r7, abort=20f
		ldr1w	r1, r8, abort=20f
		ldr1w	r1, lr, abort=20f

#if LDR1W_SHIFT < STR1W_SHIFT
		lsl	ip, ip, #STR1W_SHIFT - LDR1W_SHIFT
#elif LDR1W_SHIFT > STR1W_SHIFT
		lsr	ip, ip, #LDR1W_SHIFT - STR1W_SHIFT
#endif
		add	pc, pc, ip
		nop
		.rept	(1 << STR1W_SHIFT)
		W(nop)
		.endr
		str1w	r0, r3, abort=20f
		str1w	r0, r4, abort=20f
		str1w	r0, r5, abort=20f
		str1w	r0, r6, abort=20f
		str1w	r0, r7, abort=20f
		str1w	r0, r8, abort=20f
		str1w	r0, lr, abort=20f

	CALGN(	bcs	2b			)

7:		ldmfd	sp!, {r5 - r8}

8:		movs	r2, r2, lsl #31
		ldr1b	r1, r3, ne, abort=21f
		ldr1b	r1, r4, cs, abort=21f
		ldr1b	r1, ip, cs, abort=21f
		str1b	r0, r3, ne, abort=21f
		str1b	r0, r4, cs, abort=21f
		str1b	r0, ip, cs, abort=21f

		exit	r4, lr
		bx	lr

9:		rsb	ip, ip, #4
		cmp	ip, #2
		ldr1b	r1, r3, gt, abort=21f
		ldr1b	r1, r4, ge, abort=21f
		ldr1b	r1, lr, abort=21f
		str1b	r0, r3, gt, abort=21f
		str1b	r0, r4, ge, abort=21f
		subs	r2, r2, ip
		str1b	r0, lr, abort=21f
		blt	8b
		ands	ip, r1, #3
		beq	1b

10:		bic	r1, r1, #3
		cmp	ip, #2
		ldr1w	r1, lr, abort=21f
		beq	17f
		bgt	18f


		.macro	forward_copy_shift pull push

		subs	r2, r2, #28
		blt	14f

	CALGN(	ands	ip, r0, #31		)
	CALGN(	rsb	ip, ip, #32		)
	CALGN(	sbcsne	r4, ip, r2		)  @ C is always set here
	CALGN(	subcc	r2, r2, ip		)
	CALGN(	bcc	15f			)

11:		stmfd	sp!, {r5 - r9}

	PLD(	pld	[r1, #0]		)
	PLD(	subs	r2, r2, #96		)
	PLD(	pld	[r1, #28]		)
	PLD(	blt	13f			)
	PLD(	pld	[r1, #60]		)
	PLD(	pld	[r1, #92]		)

12:	PLD(	pld	[r1, #124]		)
13:		ldr4w	r1, r4, r5, r6, r7, abort=19f
		mov	r3, lr, lspull #\pull
		subs	r2, r2, #32
		ldr4w	r1, r8, r9, ip, lr, abort=19f
		orr	r3, r3, r4, lspush #\push
		mov	r4, r4, lspull #\pull
		orr	r4, r4, r5, lspush #\push
		mov	r5, r5, lspull #\pull
		orr	r5, r5, r6, lspush #\push
		mov	r6, r6, lspull #\pull
		orr	r6, r6, r7, lspush #\push
		mov	r7, r7, lspull #\pull
		orr	r7, r7, r8, lspush #\push
		mov	r8, r8, lspull #\pull
		orr	r8, r8, r9, lspush #\push
		mov	r9, r9, lspull #\pull
		orr	r9, r9, ip, lspush #\push
		mov	ip, ip, lspull #\pull
		orr	ip, ip, lr, lspush #\push
		str8w	r0, r3, r4, r5, r6, r7, r8, r9, ip, , abort=19f
		bge	12b
	PLD(	cmn	r2, #96			)
	PLD(	bge	13b			)

		ldmfd	sp!, {r5 - r9}

14:		ands	ip, r2, #28
		beq	16f

15:		mov	r3, lr, lspull #\pull
		ldr1w	r1, lr, abort=21f
		subs	ip, ip, #4
		orr	r3, r3, lr, lspush #\push
		str1w	r0, r3, abort=21f
		bgt	15b
	CALGN(	cmp	r2, #0			)
	CALGN(	bge	11b			)

16:		sub	r1, r1, #(\push / 8)
		b	8b

		.endm


		forward_copy_shift	pull=8	push=24

17:		forward_copy_shift	pull=16	push=16

18:		forward_copy_shift	pull=24	push=8


/*
 * Abort preamble and completion macros.
 * If a fixup handler is required then those macros must surround it.
 * It is assumed that the fixup code will handle the private part of
 * the exit macro.
 */

	.macro	copy_abort_preamble
19:	ldmfd	sp!, {r5 - r9}
	b	21f
20:	ldmfd	sp!, {r5 - r8}
21:
	.endm

	.macro	copy_abort_end
	ldmfd	sp!, {r4, lr}
	bx	lr
	.endm

ENDPROC(memcpy)
