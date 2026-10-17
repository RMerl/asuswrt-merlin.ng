/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2022 Ventana Micro Systems Inc.
 */

#include <asm/asm.h>
#include <linux/linkage.h>

.pushsection .text.smh_trap, "ax"
ENTRY(smh_trap)
	.align	2
	.option	push
	.option	norvc		/* semihosting sequence must be 32-bit wide */

	slli zero, zero, 0x1f	/* Entry NOP to identify semihosting */
	ebreak
	srai zero, zero, 7	/* NOP encoding of semihosting call number */
	.option pop

	ret
ENDPROC(smh_trap)
.popsection
