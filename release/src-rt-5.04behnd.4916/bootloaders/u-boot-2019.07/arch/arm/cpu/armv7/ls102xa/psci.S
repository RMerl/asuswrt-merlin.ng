/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2015 Freescale Semiconductor, Inc.
 * Author: Wang Dongsheng <dongsheng.wang@freescale.com>
 */

#include <config.h>
#include <linux/linkage.h>

#include <asm/armv7.h>
#include <asm/arch-armv7/generictimer.h>
#include <asm/psci.h>

#define RCPM_TWAITSR		0x04C

#define SCFG_CORE0_SFT_RST      0x130
#define SCFG_CORESRENCR         0x204

#define DCFG_CCSR_RSTCR			0x0B0
#define DCFG_CCSR_RSTCR_RESET_REQ	0x2
#define DCFG_CCSR_BRR			0x0E4
#define DCFG_CCSR_SCRATCHRW1		0x200

#define PSCI_FN_PSCI_VERSION_FEATURE_MASK	0x0
#define PSCI_FN_CPU_SUSPEND_FEATURE_MASK	0x0
#define PSCI_FN_CPU_OFF_FEATURE_MASK		0x0
#define PSCI_FN_CPU_ON_FEATURE_MASK		0x0
#define PSCI_FN_AFFINITY_INFO_FEATURE_MASK	0x0
#define PSCI_FN_SYSTEM_OFF_FEATURE_MASK		0x0
#define PSCI_FN_SYSTEM_RESET_FEATURE_MASK	0x0
#define PSCI_FN_SYSTEM_SUSPEND_FEATURE_MASK	0x0

	.pushsection ._secure.text, "ax"

	.arch_extension sec

	.align	5

#define	ONE_MS		(COUNTER_FREQUENCY / 1000)
#define	RESET_WAIT	(30 * ONE_MS)

.globl	psci_version
psci_version:
	movw	r0, #0
	movt	r0, #1

	bx	lr

_ls102x_psci_supported_table:
	.word	ARM_PSCI_0_2_FN_PSCI_VERSION
	.word	PSCI_FN_PSCI_VERSION_FEATURE_MASK
	.word	ARM_PSCI_0_2_FN_CPU_SUSPEND
	.word	PSCI_FN_CPU_SUSPEND_FEATURE_MASK
	.word	ARM_PSCI_0_2_FN_CPU_OFF
	.word	PSCI_FN_CPU_OFF_FEATURE_MASK
	.word	ARM_PSCI_0_2_FN_CPU_ON
	.word	PSCI_FN_CPU_ON_FEATURE_MASK
	.word	ARM_PSCI_0_2_FN_AFFINITY_INFO
	.word	PSCI_FN_AFFINITY_INFO_FEATURE_MASK
	.word	ARM_PSCI_0_2_FN_SYSTEM_OFF
	.word	PSCI_FN_SYSTEM_OFF_FEATURE_MASK
	.word	ARM_PSCI_0_2_FN_SYSTEM_RESET
	.word	PSCI_FN_SYSTEM_RESET_FEATURE_MASK
	.word	ARM_PSCI_1_0_FN_SYSTEM_SUSPEND
	.word	PSCI_FN_SYSTEM_SUSPEND_FEATURE_MASK
	.word	0
	.word	ARM_PSCI_RET_NI

.globl	psci_features
psci_features:
	adr	r2, _ls102x_psci_supported_table
1:	ldr	r3, [r2]
	cmp	r3, #0
	beq	out_psci_features
	cmp	r1, r3
	addne	r2, r2, #8
	bne	1b

out_psci_features:
	ldr	r0, [r2, #4]
	bx	lr

@ r0: return value ARM_PSCI_RET_SUCCESS or ARM_PSCI_RET_INVAL
@ r1: input target CPU ID in MPIDR format, original value in r1 may be dropped
@ r4: output validated CPU ID if ARM_PSCI_RET_SUCCESS returns, meaningless for
@ ARM_PSCI_RET_INVAL,suppose caller saves r4 before calling
LENTRY(psci_check_target_cpu_id)
	@ Get the real CPU number
	and	r4, r1, #0xff
	mov	r0, #ARM_PSCI_RET_INVAL

	@ Bit[31:24], bits must be zero.
	tst	r1, #0xff000000
	bxne	lr

	@ Affinity level 2 - Cluster: only one cluster in LS1021xa.
	tst	r1, #0xff0000
	bxne	lr

	@ Affinity level 1 - Processors: should be in 0xf00 format.
	lsr	r1, r1, #8
	teq	r1, #0xf
	bxne	lr

	@ Affinity level 0 - CPU: only 0, 1 are valid in LS1021xa.
	cmp	r4, #2
	bxge	lr

	mov	r0, #ARM_PSCI_RET_SUCCESS
	bx	lr
ENDPROC(psci_check_target_cpu_id)

	@ r1 = target CPU
	@ r2 = target PC
.globl	psci_cpu_on
psci_cpu_on:
	push	{r4, r5, r6, lr}

	@ Clear and Get the correct CPU number
	@ r1 = 0xf01
	bl	psci_check_target_cpu_id
	cmp	r0, #ARM_PSCI_RET_INVAL
	beq	out_psci_cpu_on

	mov	r0, r4
	mov	r1, r2
	mov	r2, r3
	bl	psci_save
	mov	r1, r4

	@ Get DCFG base address
	movw	r4, #(CONFIG_SYS_FSL_GUTS_ADDR & 0xffff)
	movt	r4, #(CONFIG_SYS_FSL_GUTS_ADDR >> 16)

	@ Detect target CPU state
	ldr	r2, [r4, #DCFG_CCSR_BRR]
	rev	r2, r2
	lsr	r2, r2, r1
	ands	r2, r2, #1
	beq	holdoff_release

	@ Reset target CPU
	@ Get SCFG base address
	movw	r0, #(CONFIG_SYS_FSL_SCFG_ADDR & 0xffff)
	movt	r0, #(CONFIG_SYS_FSL_SCFG_ADDR >> 16)

	@ Enable CORE Soft Reset
	movw	r5, #0
	movt	r5, #(1 << 15)
	rev	r5, r5
	str	r5, [r0, #SCFG_CORESRENCR]

	@ Get CPUx offset register
	mov	r6, #0x4
	mul	r6, r6, r1
	add	r2, r0, r6

	@ Do reset on target CPU
	movw	r5, #0
	movt	r5, #(1 << 15)
	rev	r5, r5
	str	r5, [r2, #SCFG_CORE0_SFT_RST]

	@ Wait target CPU up
	timer_wait	r2, RESET_WAIT

	@ Disable CORE soft reset
	mov	r5, #0
	str	r5, [r0, #SCFG_CORESRENCR]

holdoff_release:
	@ Release on target CPU
	ldr	r2, [r4, #DCFG_CCSR_BRR]
	mov	r6, #1
	lsl	r6, r6, r1	@ 32 bytes per CPU

	rev	r6, r6
	orr	r2, r2, r6
	str	r2, [r4, #DCFG_CCSR_BRR]

	@ Set secondary boot entry
	ldr	r6, =psci_cpu_entry
	rev	r6, r6
	str	r6, [r4, #DCFG_CCSR_SCRATCHRW1]

	isb
	dsb

	@ Return
	mov	r0, #ARM_PSCI_RET_SUCCESS

out_psci_cpu_on:
	pop	{r4, r5, r6, lr}
	bx	lr

.globl	psci_cpu_off
psci_cpu_off:
	bl	psci_cpu_off_common

1:	wfi
	b	1b

.globl	psci_affinity_info
psci_affinity_info:
	push	{lr}

	mov	r0, #ARM_PSCI_RET_INVAL

	@ Verify Affinity level
	cmp	r2, #0
	bne	out_affinity_info

	bl	psci_check_target_cpu_id
	cmp	r0, #ARM_PSCI_RET_INVAL
	beq	out_affinity_info
	mov	r1, r4

	@ Get RCPM base address
	movw	r4, #(CONFIG_SYS_FSL_RCPM_ADDR & 0xffff)
	movt	r4, #(CONFIG_SYS_FSL_RCPM_ADDR >> 16)

	mov	r0, #PSCI_AFFINITY_LEVEL_ON

	@ Detect target CPU state
	ldr	r2, [r4, #RCPM_TWAITSR]
	rev	r2, r2
	lsr	r2, r2, r1
	ands	r2, r2, #1
	beq	out_affinity_info

	mov	r0, #PSCI_AFFINITY_LEVEL_OFF

out_affinity_info:
	pop	{pc}

.globl	psci_system_reset
psci_system_reset:
	@ Get DCFG base address
	movw	r1, #(CONFIG_SYS_FSL_GUTS_ADDR & 0xffff)
	movt	r1, #(CONFIG_SYS_FSL_GUTS_ADDR >> 16)

	mov	r2, #DCFG_CCSR_RSTCR_RESET_REQ
	rev	r2, r2
	str	r2, [r1, #DCFG_CCSR_RSTCR]

1:	wfi
	b	1b

.globl	psci_system_suspend
psci_system_suspend:
	push	{lr}

	bl	ls1_system_suspend

	pop	{pc}

	.popsection
