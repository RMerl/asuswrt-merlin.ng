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
#include <init.h>
#include <asm/global_data.h>
#include <asm/mp.h>

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

int arch_setup_dest_addr(void)
{
#if defined(CONFIG_MP) && (defined(CONFIG_MPC86xx) || defined(CONFIG_E500))
	/*
	 * We need to make sure the location we intend to put secondary core
	 * boot code is reserved and not used by any part of u-boot
	 */
	if (gd->relocaddr > determine_mp_bootpg(NULL)) {
		gd->relocaddr = determine_mp_bootpg(NULL);
		debug("Reserving MP boot page to %08lx\n", gd->relocaddr);
	}
#endif

	return 0;
}
