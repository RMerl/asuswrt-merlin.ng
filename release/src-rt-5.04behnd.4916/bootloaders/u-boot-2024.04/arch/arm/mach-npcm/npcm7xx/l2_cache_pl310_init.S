/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2021 Nuvoton Technology Corp.
 */

.align 5

#include <linux/linkage.h>

#ifndef CONFIG_SYS_L2CACHE_OFF

ENTRY(l2_pl310_init)

@------------------------------------------------------------------
@ L2CC (PL310) Initialization
@------------------------------------------------------------------
	@ In this example PL310 PA = VA. The memory was marked as Device memory
	@ in previous stages when defining CORE0 private address space
	LDR     r0, =0xF03FC000        @ A9_BASE_ADDR

	@ Disable L2 Cache controller just in case it is already on
	LDR     r1, =0x0
	STR     r1, [r0,#0x100]

	@ Set aux cntrl
	@ Way size = 32KB
	@ Way = 16
	LDR     r1, =0x02050000
	ORR	r1, r1, #(1 << 29)	@ Instruction prefetch enable
	ORR	r1, r1, #(1 << 28)	@ Data prefetch enable
	ORR	r1, r1, #(1 << 22)	@ cache replacement policy
	STR     r1, [r0,#0x104]		@ auxilary control reg at offset 0x104

	@ Set tag RAM latency
	@ 1 cycle RAM write access latency
	@ 1 cycle RAM read access latency
	@ 1 cycle RAM setup latency
	LDR     r1, =0x00000000
	STR     r1, [r0,#0x108]		@ tag ram control reg at offset 0x108

	@ Set Data RAM latency
	@ 1 cycle RAM write access latency
	@ 2 cycles RAM read access latency
	@ 1 cycle RAM setup latency
	LDR     r1, =0x00000000
	STR     r1, [r0,#0x10C]		@ data ram control reg at offset 0x108

	@Cache maintenance - invalidate 16 ways (0xffff) - base offset 0x77C
	LDR     r1, =0xFFFF
	STR     r1, [r0,#0x77C]		@ invalidate by way register at offset 0x77C
poll_invalidate:
	LDR     r1, [r0,#0x77C]		@ invalidate by way register at offset 0x77C
	TST     r1, #1
	BNE     poll_invalidate

	@ Ensure L2 remains disabled for the time being
	LDR     r1, =0x0
	STR     r1, [r0,#0x100]

	MRC     p15, 4, r0, c15, c0, 0     @ Read periph base address
	@ SCU offset from base of private peripheral space = 0x000

	LDR     r1, [r0, #0x0]             @ Read the SCU Control Register
	ORR     r1, r1, #0x1               @ Set bit 0 (The Enable bit)
	STR     r1, [r0, #0x0]             @ Write back modifed value

	BX	lr

ENDPROC(l2_pl310_init)

#endif
