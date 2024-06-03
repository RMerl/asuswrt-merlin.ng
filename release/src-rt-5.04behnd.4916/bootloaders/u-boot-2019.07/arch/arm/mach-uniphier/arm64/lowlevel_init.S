/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2017 Socionext Inc.
 */

#include <linux/linkage.h>

ENTRY(lowlevel_init)
	/* LD20 needs the following code to boot.  I do not know why. */
	mrs	x0, sctlr_el1
	msr	sctlr_el1, x0
	ret
ENDPROC(lowlevel_init)
