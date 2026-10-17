/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) ASPEED Technology Inc.
 */
#include <asm/arch/scu_ast2500.h>

/* registers for low level init */
#define SCU_PROT_KEY		0x1e6e2000
#define SCU_VGA_HANDSHAKE	0x1e6e2040
#define SCU_HW_STRAP		0x1e6e2070
#define SCU_HW_STRAP_CLR	0x1e6e207c
#define WDT3_CTRL		0x1e78504c

.global lowlevel_init
lowlevel_init:

	/* unlock SCU */
	ldr	r0, =SCU_PROT_KEY
	ldr	r1, =SCU_UNLOCK_VALUE
	str	r1, [r0]

	/* set BMC FW as DRAM initializer */
	ldr	r0, =SCU_VGA_HANDSHAKE
	ldr	r1, [r0]
	orr	r1, #0x80
	str	r1, [r0]

	/* set PERST# as LPC reset source if eSPI mode is enabled*/
	ldr	r0, =SCU_HW_STRAP
	ldr	r1, [r0]
	tst	r1, #(0x1 << 25)
	ldrne	r0, =SCU_HW_STRAP_CLR
	movne	r1, #(0x1 << 14)
	strne	r1, [r0]

	/* disable WDT3 for SPI 3/4 bytes auto-detection */
	ldr	r0, =WDT3_CTRL
	mov	r1, #0x0
	str	r1, [r0]

	mov	pc, lr
