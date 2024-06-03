// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Alex Zuepke <azu@sysgo.de>
 *
 * (C) Copyright 2002
 * Gary Jennejohn, DENX Software Engineering, <gj@denx.de>
 *
 * (C) Copyright 2009
 * Ilya Yanok, Emcraft Systems Ltd, <yanok@emcraft.com>
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/imx-regs.h>

/*
 * Reset the cpu by setting up the watchdog timer and let it time out
 */
void reset_cpu(ulong ignored)
{
	struct wdog_regs *regs = (struct wdog_regs *)IMX_WDT_BASE;
	/* Disable watchdog and set Time-Out field to 0 */
	writew(0, &regs->wcr);

	/* Write Service Sequence */
	writew(WSR_UNLOCK1, &regs->wsr);
	writew(WSR_UNLOCK2, &regs->wsr);

	/* Enable watchdog */
	writew(WCR_WDE, &regs->wcr);

	while (1) ;
}
