/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2016 Socionext Inc.
 *   Author: Masahiro Yamada <yamada.masahiro@socionext.com>
 */

#include <linux/linkage.h>
#include <asm/system.h>

	.section ._secure.text, "ax"

ENTRY(uniphier_smp_trampoline)
	ldr	r0, 0f
	mrc	p15, 0, r1, c1, c0, 0	@ SCTLR (System Control Register)
	orr	r1, r1, #CR_I		@ Enable ICache
	bic	r1, r1, #(CR_C | CR_M)	@ Disable MMU and Dcache
	mcr	p15, 0, r1, c1, c0, 0

	bx	r0
0:	.word	uniphier_secondary_startup
	.globl	uniphier_smp_trampoline_end
uniphier_smp_trampoline_end:
ENDPROC(uniphier_smp_trampoline)

LENTRY(uniphier_secondary_startup)
	mrc	p15, 0, r1, c0, c0, 5	@ MPIDR (Multiprocessor Affinity Reg)
	and	r1, r1, #0xff

	ldr	r2, =uniphier_smp_booted
	mov	r0, #1
	str	r0, [r2, r1, lsl #2]

	ldr	r2, =uniphier_psci_holding_pen_release
pen:	ldr	r0, [r2]
	cmp	r0, r1
	beq	psci_cpu_entry
	wfe
	b	pen
ENDPROC(uniphier_secondary_startup)
