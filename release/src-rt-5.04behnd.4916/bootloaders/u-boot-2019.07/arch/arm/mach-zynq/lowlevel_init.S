/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2013 - 2015 Xilinx, Inc. All rights reserved.
 */

#include <asm-offsets.h>
#include <config.h>
#include <linux/linkage.h>

ENTRY(lowlevel_init)

	/* Enable the the VFP */
	mrc	p15, 0, r1, c1, c0, 2
	orr	r1, r1, #(0x3 << 20)
	orr	r1, r1, #(0x3 << 20)
	mcr	p15, 0, r1, c1, c0, 2
	isb
	fmrx	r1, FPEXC
	orr	r1,r1, #(1<<30)
	fmxr	FPEXC, r1

	/* Move back to caller */
	mov	pc, lr

ENDPROC(lowlevel_init)
