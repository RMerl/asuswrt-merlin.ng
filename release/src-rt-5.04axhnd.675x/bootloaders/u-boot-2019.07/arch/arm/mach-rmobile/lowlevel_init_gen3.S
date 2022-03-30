/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * arch/arm/cpu/armv8/rcar_gen3/lowlevel_init.S
 *	This file is lowlevel initialize routine.
 *
 * (C) Copyright 2015 Renesas Electronics Corporation
 *
 * This file is based on the arch/arm/cpu/armv8/start.S
 *
 * (C) Copyright 2013
 * David Feng <fenghua@phytium.com.cn>
 */

#include <asm-offsets.h>
#include <config.h>
#include <linux/linkage.h>
#include <asm/macro.h>

.align 8
.globl	rcar_atf_boot_args
rcar_atf_boot_args:
	.dword 0
	.dword 0
	.dword 0
	.dword 0

ENTRY(save_boot_params)
	adr	x8, rcar_atf_boot_args
	stp	x0, x1, [x8], #16
	stp	x2, x3, [x8], #16
	b	save_boot_params_ret
ENDPROC(save_boot_params)

ENTRY(lowlevel_init)
	mov	x29, lr			/* Save LR */

#ifndef CONFIG_ARMV8_MULTIENTRY
	/*
	 * For single-entry systems the lowlevel init is very simple.
	 */
	ldr	x0, =GICD_BASE
	bl	gic_init_secure

#else /* CONFIG_ARMV8_MULTIENTRY is set */

#if defined(CONFIG_GICV2) || defined(CONFIG_GICV3)
	branch_if_slave x0, 1f
	ldr	x0, =GICD_BASE
	bl	gic_init_secure
1:
#if defined(CONFIG_GICV3)
	ldr	x0, =GICR_BASE
	bl	gic_init_secure_percpu
#elif defined(CONFIG_GICV2)
	ldr	x0, =GICD_BASE
	ldr	x1, =GICC_BASE
	bl	gic_init_secure_percpu
#endif
#endif

	branch_if_master x0, x1, 2f

	/*
	 * Slave should wait for master clearing spin table.
	 * This sync prevent salves observing incorrect
	 * value of spin table and jumping to wrong place.
	 */
#if defined(CONFIG_GICV2) || defined(CONFIG_GICV3)
#ifdef CONFIG_GICV2
	ldr	x0, =GICC_BASE
#endif
	bl	gic_wait_for_interrupt
#endif

	/*
	 * All slaves will enter EL2 and optionally EL1.
	 */
	adr	x4, lowlevel_in_el2
	ldr	x5, =ES_TO_AARCH64
	bl	armv8_switch_to_el2

lowlevel_in_el2:
#ifdef CONFIG_ARMV8_SWITCH_TO_EL1
	adr	x4, lowlevel_in_el1
	ldr	x5, =ES_TO_AARCH64
	bl	armv8_switch_to_el1

lowlevel_in_el1:
#endif
#endif /* CONFIG_ARMV8_MULTIENTRY */

	bl      s_init

2:
	mov	lr, x29			/* Restore LR */
	ret
ENDPROC(lowlevel_init)
