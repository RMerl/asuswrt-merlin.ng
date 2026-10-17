/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2020 Cortina-Access
 *
 */


#include <asm-offsets.h>
#include <config.h>
#include <linux/linkage.h>
#include <asm/macro.h>
#include <asm/armv8/mmu.h>

	.globl lowlevel_init
lowlevel_init:
	mov	x29, lr			/* Save LR */

#if defined(CONFIG_SOC_CA7774)
	/* Enable SMPEN in CPUECTLR */
	mrs     x0, s3_1_c15_c2_1
	tst     x0, #0x40
        b.ne    skip_smp_setup
	orr     x0, x0, #0x40
	msr     s3_1_c15_c2_1, x0
skip_smp_setup:
#endif

#if defined(CONFIG_SOC_CA8277B)
	/* Enable CPU Timer */
	ldr x0, =CFG_SYS_TIMER_BASE
	mov x1, #1
	str w1, [x0]
#endif

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

#ifdef CONFIG_ARMV8_MULTIENTRY
	branch_if_master x0, 2f

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

2:
	mov	lr, x29			/* Restore LR */
	ret
