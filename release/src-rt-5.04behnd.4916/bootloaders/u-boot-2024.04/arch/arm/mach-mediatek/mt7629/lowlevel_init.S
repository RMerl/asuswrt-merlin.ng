/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2018 MediaTek Inc.
 */

#include <linux/linkage.h>
#include <asm/proc-armv/ptrace.h>

#define WAIT_CODE_SRAM_BASE	0x0010ff00

#define SLAVE_JUMP_REG		0x10202034
#define SLAVE1_MAGIC_REG	0x10202038
#define SLAVE1_MAGIC_NUM	0x534c4131

#define GIC_CPU_BASE		0x10320000

ENTRY(lowlevel_init)

#ifndef CONFIG_SPL_BUILD
	/* Return to U-Boot via saved link register */
	mov	pc, lr
#else
	/*
	 * Arch timer :
	 * set CNTFRQ = 20Mhz, set CNTVOFF = 0
	 */
	movw	r0, #0x2d00
	movt	r0, #0x131
	mcr	p15, 0, r0, c14, c0, 0

	cps	#MON_MODE
	mrc	p15, 0, r1, c1, c1, 0	@ Get Secure Config
	orr	r0, r1, #1
	mcr	p15, 0, r0, c1, c1, 0	@ Set Non Secure bit
	isb
	mov	r0, #0
	mcrr	p15, 4, r0, r0, c14	@ CNTVOFF = 0
	isb
	mcr	p15, 0, r1, c1, c1, 0	@ Set Secure bit
	isb
	cps	#SVC_MODE

	/* enable SMP bit */
	mrc	p15, 0, r0, c1, c0, 1
	orr	r0, r0, #0x40
	mcr	p15, 0, r0, c1, c0, 1

	/* if MP core, handle secondary cores */
	mrc	p15, 0, r0, c0, c0, 5
	ands	r1, r0, #0x40000000
	bne	go			@ Go if UP
	/* read slave CPU number */
	ands	r0, r0, #0x0f
	beq	go			@ Go if core0 on primary core tile
	b	secondary

go:
	/* master CPU */
	mov	pc, lr

secondary:
	/* enable GIC as cores will be waken up by IPI */
	ldr	r2, =GIC_CPU_BASE
	mov	r1, #0xf0
	str	r1, [r2, #4]
	mov	r1, #1
	str	r1, [r2, #0]

	ldr	r1, [r2]
	orr	r1, #1
	str	r1, [r2]

	/* copy wait code into SRAM */
	ldr	r0, =slave_cpu_wait
	ldm	r0, {r1 - r8}		@ slave_cpu_wait has eight insns
	ldr	r0, =WAIT_CODE_SRAM_BASE
	stm	r0, {r1 - r8}

	/* pass args to slave_cpu_wait */
	ldr	r0, =SLAVE1_MAGIC_REG
	ldr	r1, =SLAVE1_MAGIC_NUM

	/* jump to wait code in SRAM */
	ldr	pc, =WAIT_CODE_SRAM_BASE

#endif
ENDPROC(lowlevel_init)

/* This function will be copied into SRAM */
ENTRY(slave_cpu_wait)
	wfi
	ldr	r2, [r0]
	cmp	r2, r1
	bne	slave_cpu_wait
	movw	r0, #:lower16:SLAVE_JUMP_REG
	movt	r0, #:upper16:SLAVE_JUMP_REG
	ldr	r1, [r0]
	mov	pc, r1
ENDPROC(slave_cpu_wait)
