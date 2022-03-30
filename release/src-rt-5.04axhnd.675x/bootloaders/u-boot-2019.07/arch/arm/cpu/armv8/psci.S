/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2016 Freescale Semiconductor, Inc.
 * Author: Hongbo Zhang <hongbo.zhang@nxp.com>
 * This file implements LS102X platform PSCI SYSTEM-SUSPEND function
 */

#include <config.h>
#include <linux/linkage.h>
#include <asm/psci.h>
#include <asm/secure.h>

/* Default PSCI function, return -1, Not Implemented */
#define PSCI_DEFAULT(__fn) \
	ENTRY(__fn); \
	mov	w0, #ARM_PSCI_RET_NI; \
	ret; \
	ENDPROC(__fn); \
	.weak __fn

/* PSCI function and ID table definition*/
#define PSCI_TABLE(__id, __fn) \
	.quad __id; \
	.quad __fn

.pushsection ._secure.text, "ax"

/* 32 bits PSCI default functions */
PSCI_DEFAULT(psci_version)
PSCI_DEFAULT(psci_cpu_suspend)
PSCI_DEFAULT(psci_cpu_off)
PSCI_DEFAULT(psci_cpu_on)
PSCI_DEFAULT(psci_affinity_info)
PSCI_DEFAULT(psci_migrate)
PSCI_DEFAULT(psci_migrate_info_type)
PSCI_DEFAULT(psci_migrate_info_up_cpu)
PSCI_DEFAULT(psci_system_off)
PSCI_DEFAULT(psci_system_reset)
PSCI_DEFAULT(psci_features)
PSCI_DEFAULT(psci_cpu_freeze)
PSCI_DEFAULT(psci_cpu_default_suspend)
PSCI_DEFAULT(psci_node_hw_state)
PSCI_DEFAULT(psci_system_suspend)
PSCI_DEFAULT(psci_set_suspend_mode)
PSCI_DEFAULT(psi_stat_residency)
PSCI_DEFAULT(psci_stat_count)

.align 3
_psci_32_table:
PSCI_TABLE(ARM_PSCI_FN_CPU_SUSPEND, psci_cpu_suspend)
PSCI_TABLE(ARM_PSCI_FN_CPU_OFF, psci_cpu_off)
PSCI_TABLE(ARM_PSCI_FN_CPU_ON, psci_cpu_on)
PSCI_TABLE(ARM_PSCI_FN_MIGRATE, psci_migrate)
PSCI_TABLE(ARM_PSCI_0_2_FN_PSCI_VERSION, psci_version)
PSCI_TABLE(ARM_PSCI_0_2_FN_CPU_SUSPEND, psci_cpu_suspend)
PSCI_TABLE(ARM_PSCI_0_2_FN_CPU_OFF, psci_cpu_off)
PSCI_TABLE(ARM_PSCI_0_2_FN_CPU_ON, psci_cpu_on)
PSCI_TABLE(ARM_PSCI_0_2_FN_AFFINITY_INFO, psci_affinity_info)
PSCI_TABLE(ARM_PSCI_0_2_FN_MIGRATE, psci_migrate)
PSCI_TABLE(ARM_PSCI_0_2_FN_MIGRATE_INFO_TYPE, psci_migrate_info_type)
PSCI_TABLE(ARM_PSCI_0_2_FN_MIGRATE_INFO_UP_CPU, psci_migrate_info_up_cpu)
PSCI_TABLE(ARM_PSCI_0_2_FN_SYSTEM_OFF, psci_system_off)
PSCI_TABLE(ARM_PSCI_0_2_FN_SYSTEM_RESET, psci_system_reset)
PSCI_TABLE(ARM_PSCI_1_0_FN_PSCI_FEATURES, psci_features)
PSCI_TABLE(ARM_PSCI_1_0_FN_CPU_FREEZE, psci_cpu_freeze)
PSCI_TABLE(ARM_PSCI_1_0_FN_CPU_DEFAULT_SUSPEND, psci_cpu_default_suspend)
PSCI_TABLE(ARM_PSCI_1_0_FN_NODE_HW_STATE, psci_node_hw_state)
PSCI_TABLE(ARM_PSCI_1_0_FN_SYSTEM_SUSPEND, psci_system_suspend)
PSCI_TABLE(ARM_PSCI_1_0_FN_SET_SUSPEND_MODE, psci_set_suspend_mode)
PSCI_TABLE(ARM_PSCI_1_0_FN_STAT_RESIDENCY, psi_stat_residency)
PSCI_TABLE(ARM_PSCI_1_0_FN_STAT_COUNT, psci_stat_count)
PSCI_TABLE(0, 0)

/* 64 bits PSCI default functions */
PSCI_DEFAULT(psci_cpu_suspend_64)
PSCI_DEFAULT(psci_cpu_on_64)
PSCI_DEFAULT(psci_affinity_info_64)
PSCI_DEFAULT(psci_migrate_64)
PSCI_DEFAULT(psci_migrate_info_up_cpu_64)
PSCI_DEFAULT(psci_cpu_default_suspend_64)
PSCI_DEFAULT(psci_node_hw_state_64)
PSCI_DEFAULT(psci_system_suspend_64)
PSCI_DEFAULT(psci_stat_residency_64)
PSCI_DEFAULT(psci_stat_count_64)

.align 3
_psci_64_table:
PSCI_TABLE(ARM_PSCI_0_2_FN64_CPU_SUSPEND, psci_cpu_suspend_64)
PSCI_TABLE(ARM_PSCI_0_2_FN64_CPU_ON, psci_cpu_on_64)
PSCI_TABLE(ARM_PSCI_0_2_FN64_AFFINITY_INFO, psci_affinity_info_64)
PSCI_TABLE(ARM_PSCI_0_2_FN64_MIGRATE, psci_migrate_64)
PSCI_TABLE(ARM_PSCI_0_2_FN64_MIGRATE_INFO_UP_CPU, psci_migrate_info_up_cpu_64)
PSCI_TABLE(ARM_PSCI_1_0_FN64_CPU_DEFAULT_SUSPEND, psci_cpu_default_suspend_64)
PSCI_TABLE(ARM_PSCI_1_0_FN64_NODE_HW_STATE, psci_node_hw_state_64)
PSCI_TABLE(ARM_PSCI_1_0_FN64_SYSTEM_SUSPEND, psci_system_suspend_64)
PSCI_TABLE(ARM_PSCI_1_0_FN64_STAT_RESIDENCY, psci_stat_residency_64)
PSCI_TABLE(ARM_PSCI_1_0_FN64_STAT_COUNT, psci_stat_count_64)
PSCI_TABLE(0, 0)

.macro	psci_enter
	/* PSCI call is Fast Call(atomic), so mask DAIF */
	mrs	x15, DAIF
	stp	x15, xzr, [sp, #-16]!
	ldr	x15, =0x3C0
	msr	DAIF, x15
	/* SMC convention, x18 ~ x30 should be saved by callee */
	stp	x29, x30, [sp, #-16]!
	stp	x27, x28, [sp, #-16]!
	stp	x25, x26, [sp, #-16]!
	stp	x23, x24, [sp, #-16]!
	stp	x21, x22, [sp, #-16]!
	stp	x19, x20, [sp, #-16]!
	mrs	x15, elr_el3
	stp	x18, x15, [sp, #-16]!
.endm

.macro	psci_return
	/* restore registers */
	ldp	x18, x15, [sp], #16
	msr	elr_el3, x15
	ldp	x19, x20, [sp], #16
	ldp	x21, x22, [sp], #16
	ldp	x23, x24, [sp], #16
	ldp	x25, x26, [sp], #16
	ldp	x27, x28, [sp], #16
	ldp	x29, x30, [sp], #16
	/* restore DAIF */
	ldp	x15, xzr, [sp], #16
	msr	DAIF, x15
	eret
.endm

/* Caller must put PSCI function-ID table base in x9 */
handle_psci:
	psci_enter
1:	ldr	x10, [x9]		/* Load PSCI function table */
	cbz	x10, 3f			/* If reach the end, bail out */
	cmp	x10, x0
	b.eq	2f			/* PSCI function found */
	add x9, x9, #16			/* If not match, try next entry */
	b	1b

2:	ldr	x11, [x9, #8]		/* Load PSCI function */
	blr	x11			/* Call PSCI function */
	psci_return

3:	mov	x0, #ARM_PSCI_RET_NI
	psci_return

/*
 * Handle SiP service functions defined in SiP service function table.
 * Use DECLARE_SECURE_SVC(_name, _id, _fn) to add platform specific SiP
 * service function into the SiP service function table.
 * SiP service function table is located in '._secure_svc_tbl_entries' section,
 * which is next to '._secure.text' section.
 */
handle_svc:
	adr	x9, __secure_svc_tbl_start
	adr	x10, __secure_svc_tbl_end
	subs	x12, x10, x9	/* Get number of entries in table */
	b.eq	2f		/* Make sure SiP function table is not empty */
	psci_enter
1:	ldr x10, [x9]		/* Load SiP function table */
	ldr x11, [x9, #8]
	cmp	w10, w0
	b.eq	2b		/* SiP service function found */
	add x9, x9, #SECURE_SVC_TBL_OFFSET	/* Move to next entry */
	subs	x12, x12, #SECURE_SVC_TBL_OFFSET
	b.eq	3b		/* If reach the end, bail out */
	b	1b
2:	ldr	x0, =0xFFFFFFFF
	eret

handle_smc32:
	/* SMC function ID  0x84000000-0x8400001F: 32 bits PSCI */
	ldr	w9, =0x8400001F
	cmp	w0, w9
	b.gt	handle_svc
	ldr	w9, =0x84000000
	cmp	w0, w9
	b.lt	handle_svc

	adr	x9, _psci_32_table
	b	handle_psci

handle_smc64:
	/* check SMC32 or SMC64 calls */
	ubfx	x9, x0, #30, #1
	cbz	x9, handle_smc32

	/* SMC function ID 0xC4000000-0xC400001F: 64 bits PSCI */
	ldr	x9, =0xC400001F
	cmp	x0, x9
	b.gt	handle_svc
	ldr	x9, =0xC4000000
	cmp	x0, x9
	b.lt	handle_svc

	adr	x9, _psci_64_table
	b	handle_psci

/*
 * Get CPU ID from MPIDR, suppose every cluster has same number of CPU cores,
 * Platform with asymmetric clusters should implement their own interface.
 * In case this function being called by other platform's C code, the ARM
 * Architecture Procedure Call Standard is considered, e.g. register X0 is
 * used for the return value, while in this PSCI environment, X0 usually holds
 * the SMC function identifier, so X0 should be saved by caller function.
 */
ENTRY(psci_get_cpu_id)
#ifdef CONFIG_ARMV8_PSCI_CPUS_PER_CLUSTER
	mrs	x9, MPIDR_EL1
	ubfx	x9, x9, #8, #8
	ldr	x10, =CONFIG_ARMV8_PSCI_CPUS_PER_CLUSTER
	mul	x9, x10, x9
#else
	mov	x9, xzr
#endif
	mrs	x10, MPIDR_EL1
	ubfx	x10, x10, #0, #8
	add	x0, x10, x9
	ret
ENDPROC(psci_get_cpu_id)
.weak psci_get_cpu_id

/* CPU ID input in x0, stack top output in x0*/
LENTRY(psci_get_cpu_stack_top)
	adr	x9, __secure_stack_end
	lsl	x0, x0, #ARM_PSCI_STACK_SHIFT
	sub	x0, x9, x0
	ret
ENDPROC(psci_get_cpu_stack_top)

unhandled_exception:
	b	unhandled_exception	/* simply dead loop */

handle_sync:
	mov	x15, x30
	mov	x14, x0

	bl	psci_get_cpu_id
	bl	psci_get_cpu_stack_top
	mov	x9, #1
	msr	spsel, x9
	mov	sp, x0

	mov	x0, x14
	mov	x30, x15

	mrs	x9, esr_el3
	ubfx	x9, x9, #26, #6
	cmp	x9, #0x13
	b.eq	handle_smc32
	cmp	x9, #0x17
	b.eq	handle_smc64

	b	unhandled_exception

#ifdef CONFIG_ARMV8_EA_EL3_FIRST
/*
 * Override this function if custom error handling is
 * needed for asynchronous aborts
 */
ENTRY(plat_error_handler)
	ret
ENDPROC(plat_error_handler)
.weak plat_error_handler

handle_error:
	bl	psci_get_cpu_id
	bl	psci_get_cpu_stack_top
	mov	x9, #1
	msr	spsel, x9
	mov	sp, x0

	bl	plat_error_handler	/* Platform specific error handling */
deadloop:
	b	deadloop		/* Never return */
#endif

	.align	11
	.globl	el3_exception_vectors
el3_exception_vectors:
	b	unhandled_exception	/* Sync, Current EL using SP0 */
	.align	7
	b	unhandled_exception	/* IRQ, Current EL using SP0 */
	.align	7
	b	unhandled_exception	/* FIQ, Current EL using SP0 */
	.align	7
	b	unhandled_exception	/* SError, Current EL using SP0 */
	.align	7
	b	unhandled_exception	/* Sync, Current EL using SPx */
	.align	7
	b	unhandled_exception	/* IRQ, Current EL using SPx */
	.align	7
	b	unhandled_exception	/* FIQ, Current EL using SPx */
	.align	7
	b	unhandled_exception	/* SError, Current EL using SPx */
	.align	7
	b	handle_sync		/* Sync, Lower EL using AArch64 */
	.align	7
	b	unhandled_exception	/* IRQ, Lower EL using AArch64 */
	.align	7
	b	unhandled_exception	/* FIQ, Lower EL using AArch64 */
	.align	7
#ifdef CONFIG_ARMV8_EA_EL3_FIRST
	b	handle_error		/* SError, Lower EL using AArch64 */
#else
	b	unhandled_exception	/* SError, Lower EL using AArch64 */
#endif
	.align	7
	b	unhandled_exception	/* Sync, Lower EL using AArch32 */
	.align	7
	b	unhandled_exception	/* IRQ, Lower EL using AArch32 */
	.align	7
	b	unhandled_exception	/* FIQ, Lower EL using AArch32 */
	.align	7
	b	unhandled_exception	/* SError, Lower EL using AArch32 */

ENTRY(psci_setup_vectors)
	adr	x0, el3_exception_vectors
	msr	vbar_el3, x0
	ret
ENDPROC(psci_setup_vectors)

ENTRY(psci_arch_init)
	ret
ENDPROC(psci_arch_init)
.weak psci_arch_init

.popsection
