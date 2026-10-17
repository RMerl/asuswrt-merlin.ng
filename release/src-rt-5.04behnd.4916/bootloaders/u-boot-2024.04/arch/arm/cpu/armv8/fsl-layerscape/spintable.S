/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2014-2015 Freescale Semiconductor
 * Copyright 2019 NXP
 */

#include <config.h>
#include <linux/linkage.h>
#include <asm/macro.h>
#include <asm/system.h>
#include <asm/arch/mp.h>

.align 3
.global secondary_boot_addr
secondary_boot_addr:
	.quad __secondary_boot_func

.global secondary_boot_code_start
secondary_boot_code_start:
	.quad __secondary_boot_code_start

.global secondary_boot_code_size
secondary_boot_code_size:
	.quad __secondary_boot_code_end - __secondary_boot_code_start

	/* Using 64 bit alignment since the spin table is accessed as data */
	.align 3
	/* Secondary Boot Code starts here */
__secondary_boot_code_start:
__spin_table:
	.space CONFIG_MAX_CPUS*SPIN_TABLE_ELEM_SIZE

	.align 2
__secondary_boot_func:
	/*
	 * MPIDR_EL1 Fields:
	 * MPIDR[1:0] = AFF0_CPUID <- Core ID (0,1)
	 * MPIDR[7:2] = AFF0_RES
	 * MPIDR[15:8] = AFF1_CLUSTERID <- Cluster ID (0,1,2,3)
	 * MPIDR[23:16] = AFF2_CLUSTERID
	 * MPIDR[24] = MT
	 * MPIDR[29:25] = RES0
	 * MPIDR[30] = U
	 * MPIDR[31] = ME
	 * MPIDR[39:32] = AFF3
	 *
	 * Linear Processor ID (LPID) calculation from MPIDR_EL1:
	 * (We only use AFF0_CPUID and AFF1_CLUSTERID for now
	 * until AFF2_CLUSTERID and AFF3 have non-zero values)
	 *
	 * LPID = MPIDR[15:8] | MPIDR[1:0]
	 */
	mrs	x0, mpidr_el1
	ubfm	x1, x0, #8, #15
	ubfm	x2, x0, #0, #1
	orr	x10, x2, x1, lsl #2	/* x10 has LPID */
	ubfm    x9, x0, #0, #15         /* x9 contains MPIDR[15:0] */
	/*
	 * offset of the spin table element for this core from start of spin
	 * table (each elem is padded to 64 bytes)
	 */
	lsl	x1, x10, #6
	adr	x0, __spin_table
	/* physical address of this cpus spin table element */
	add	x11, x1, x0

	adr	x0, __real_cntfrq
	ldr	x0, [x0]
	msr	cntfrq_el0, x0	/* set with real frequency */
	str	x9, [x11, #16]	/* LPID */
	mov	x4, #1
	str	x4, [x11, #8]	/* STATUS */
	dsb	sy

1:
	wfe
	ldr	x4, [x11]
	cbz	x4, 1b
	mrs     x1, sctlr_el2
	tbz     x1, #25, 2f
	rev     x4, x4                  /* BE to LE conversion */
2:
	ldr	x6, =ES_TO_AARCH64
#ifdef CONFIG_ARMV8_SWITCH_TO_EL1
	adr	x5, 3f
	switch_el x7, 0f, _dead_loop, _dead_loop
0:	armv8_switch_to_el2_m x5, x6, x7
#endif
3:
	ldr	x7, [x11, #24]	/* ARCH_COMP */
	cbz	x7, 4f
	ldr	x6, =ES_TO_AARCH32
4:
#ifdef CONFIG_ARMV8_SWITCH_TO_EL1
	switch_el x7, _dead_loop, 0f, _dead_loop
0:	armv8_switch_to_el1_m x4, x6, x7, x9
#else
	switch_el x7, 0f, _dead_loop, _dead_loop
0:	armv8_switch_to_el2_m x4, x6, x7
#endif

_dead_loop:
	wfe
	b _dead_loop

	/* Ensure that the literals used by the secondary boot code are
	 * assembled within it (this is required so that we can protect
	 * this area with a single memreserve region
	 */
	.ltorg

	/* 64 bit alignment for elements accessed as data */
	.align 3
	.global __real_cntfrq
__real_cntfrq:
	.quad CONFIG_COUNTER_FREQUENCY
	/* Secondary Boot Code ends here */
__secondary_boot_code_end:
