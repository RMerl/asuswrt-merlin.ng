/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Utility functions for FEL mode.
 *
 * Copyright (c) 2015 Google, Inc
 */

#include <asm-offsets.h>
#include <config.h>
#include <asm/system.h>
#include <linux/linkage.h>

ENTRY(save_boot_params)
	ldr	r0, =fel_stash
	str	sp, [r0, #0]
	str	lr, [r0, #4]
	mrs	lr, cpsr		@ Read CPSR
	str	lr, [r0, #8]
	mrc	p15, 0, lr, c1, c0, 0	@ Read CP15 SCTLR Register
	str	lr, [r0, #12]
	mrc	p15, 0, lr, c12, c0, 0	@ Read VBAR
	str	lr, [r0, #16]
	mrc	p15, 0, lr, c1, c0, 0	@ Read CP15 Control Register
	str	lr, [r0, #20]
	b	save_boot_params_ret
ENDPROC(save_boot_params)

ENTRY(return_to_fel)
	mov	sp, r0
	mov	lr, r1
	ldr	r0, =fel_stash
	ldr	r1, [r0, #20]
	mcr	p15, 0, r1, c1, c0, 0	@ Write CP15 Control Register
	ldr	r1, [r0, #16]
	mcr	p15, 0, r1, c12, c0, 0	@ Write VBAR
	ldr	r1, [r0, #12]
	mcr	p15, 0, r1, c1, c0, 0	@ Write CP15 SCTLR Register
	ldr	r1, [r0, #8]
	msr	cpsr, r1		@ Write CPSR
	bx	lr
ENDPROC(return_to_fel)
