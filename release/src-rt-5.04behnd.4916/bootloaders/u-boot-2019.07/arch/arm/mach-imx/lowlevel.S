/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2019 NXP
 */

#include <linux/linkage.h>

ENTRY(lowlevel_init)
	mrs	x0, CurrentEL
	cmp	x0, #8
	b.eq	1f
	ret
1:
	msr daifclr, #4

	/* set HCR_EL2.AMO to catch SERROR */
	mrs	x0, hcr_el2
	orr	x0, x0, #0x20
	msr	hcr_el2, x0
	isb
	ret
ENDPROC(lowlevel_init)
