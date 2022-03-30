// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2015 Andreas Bießmann <andreas@biessmann.org>
 *
 * Copyright (c) 2011 The Chromium OS Authors.
 * (C) Copyright 2002-2006
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 */
#include <common.h>

DECLARE_GLOBAL_DATA_PTR;

int arch_reserve_stacks(void)
{
	ulong *s;

	/* setup stack pointer for exceptions */
	gd->irq_sp = gd->start_addr_sp;

	/* Clear initial stack frame */
	s = (ulong *)gd->start_addr_sp;
	*s = 0; /* Terminate back chain */
	*++s = 0; /* NULL return address */

	return 0;
}
