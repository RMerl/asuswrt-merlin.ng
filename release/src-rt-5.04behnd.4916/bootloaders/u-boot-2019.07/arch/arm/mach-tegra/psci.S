/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2014, NVIDIA
 * Copyright (C) 2015, Siemens AG
 *
 * Authors:
 *  Thierry Reding <treding@nvidia.com>
 *  Jan Kiszka <jan.kiszka@siemens.com>
 */

#include <linux/linkage.h>
#include <asm/macro.h>
#include <asm/psci.h>

	.pushsection ._secure.text, "ax"
	.arch_extension sec

#define TEGRA_SB_CSR_0			0x6000c200
#define NS_RST_VEC_WR_DIS		(1 << 1)

#define TEGRA_RESET_EXCEPTION_VECTOR	0x6000f100

#define TEGRA_FLOW_CTRL_BASE		0x60007000
#define FLOW_CTRL_CPU_CSR		0x08
#define CSR_ENABLE			(1 << 0)
#define CSR_IMMEDIATE_WAKE		(1 << 3)
#define CSR_WAIT_WFI_SHIFT		8
#define FLOW_CTRL_CPU1_CSR		0x18

@ converts CPU ID into FLOW_CTRL_CPUn_CSR offset
.macro get_csr_reg cpu, ofs, tmp
	cmp	\cpu, #0		@ CPU0?
	lsl	\tmp, \cpu, #3	@ multiple by 8 (register offset CPU1-3)
	moveq	\ofs, #FLOW_CTRL_CPU_CSR
	addne	\ofs, \tmp, #FLOW_CTRL_CPU1_CSR - 8
.endm

ENTRY(psci_arch_init)
	mov	r6, lr

	mrc	p15, 0, r5, c1, c1, 0	@ Read SCR
	bic	r5, r5, #1		@ Secure mode
	mcr	p15, 0, r5, c1, c1, 0	@ Write SCR
	isb

	@ lock reset vector for non-secure
	ldr	r4, =TEGRA_SB_CSR_0
	ldr	r5, [r4]
	orr	r5, r5, #NS_RST_VEC_WR_DIS
	str	r5, [r4]

	bl	psci_get_cpu_id		@ CPU ID => r0

	adr	r5, _sys_clock_freq
	cmp	r0, #0

	mrceq	p15, 0, r7, c14, c0, 0	@ read CNTFRQ from CPU0
	streq	r7, [r5]

	ldrne	r7, [r5]
	mcrne	p15, 0, r7, c14, c0, 0	@ write CNTFRQ to CPU1..3

	bx	r6
ENDPROC(psci_arch_init)

_sys_clock_freq:
	.word	0

ENTRY(psci_cpu_off)
	bl	psci_cpu_off_common

	bl	psci_get_cpu_id		@ CPU ID => r0

	get_csr_reg r0, r2, r3

	ldr	r6, =TEGRA_FLOW_CTRL_BASE
	mov	r5, #(CSR_ENABLE)
	mov	r4, #(1 << CSR_WAIT_WFI_SHIFT)
	add	r5, r4, lsl r0
	str	r5, [r6, r2]

_loop:	wfi
	b	_loop
ENDPROC(psci_cpu_off)

ENTRY(psci_cpu_on)
	push	{r4, r5, r6, lr}

	mov	r4, r1
	mov	r0, r1
	mov	r1, r2
	mov	r2, r3
	bl	psci_save		@ store target PC and context id
	mov	r1, r4

	ldr	r6, =TEGRA_RESET_EXCEPTION_VECTOR
	ldr	r5, =psci_cpu_entry
	str	r5, [r6]

	get_csr_reg r1, r2, r3

	ldr	r6, =TEGRA_FLOW_CTRL_BASE
	mov	r5, #(CSR_IMMEDIATE_WAKE | CSR_ENABLE)
	str	r5, [r6, r2]

	mov	r0, #ARM_PSCI_RET_SUCCESS	@ Return PSCI_RET_SUCCESS
	pop	{r4, r5, r6, pc}
ENDPROC(psci_cpu_on)

	.popsection
