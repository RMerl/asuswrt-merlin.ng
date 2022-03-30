/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2018 Texas Instruments Incorporated - http://www.ti.com/
 *	Lokesh Vutla <lokeshvutla@ti.com>
 */

#include <linux/linkage.h>

ENTRY(lowlevel_init)

	mrc	p15, 0, r0, c0, c0, 5		@ Read MPIDR
	and	r0, #0xff
	cmp	r0, #0x0
	bne	park_cpu
	bx	lr
park_cpu:
	wfi
	b	park_cpu

ENDPROC(lowlevel_init)
