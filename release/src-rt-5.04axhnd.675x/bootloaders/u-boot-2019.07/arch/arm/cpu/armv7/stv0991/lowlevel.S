/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2014 stmicroelectronics
 */

#include <config.h>
#include <linux/linkage.h>

ENTRY(lowlevel_init)
	mov	pc, lr
ENDPROC(lowlevel_init)
