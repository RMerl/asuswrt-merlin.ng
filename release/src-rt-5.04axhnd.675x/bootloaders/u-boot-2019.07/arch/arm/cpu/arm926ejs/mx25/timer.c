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
 *
 * (C) Copyright 2009 DENX Software Engineering
 * Author: John Rigby <jrigby@gmail.com>
 *	Add support for MX25
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/imx-regs.h>

/* nothing really to do with interrupts, just starts up a counter. */
/* The 32KHz 32-bit timer overruns in 134217 seconds */
int timer_init(void)
{
	int i;
	struct gpt_regs *gpt = (struct gpt_regs *)IMX_GPT1_BASE;
	struct ccm_regs *ccm = (struct ccm_regs *)IMX_CCM_BASE;

	/* setup GP Timer 1 */
	writel(GPT_CTRL_SWR, &gpt->ctrl);

	writel(readl(&ccm->cgr1) | CCM_CGR1_GPT1, &ccm->cgr1);

	for (i = 0; i < 100; i++)
		writel(0, &gpt->ctrl); /* We have no udelay by now */
	writel(0, &gpt->pre); /* prescaler = 1 */
	/* Freerun Mode, 32KHz input */
	writel(readl(&gpt->ctrl) | GPT_CTRL_CLKSOURCE_32 | GPT_CTRL_FRR,
			&gpt->ctrl);
	writel(readl(&gpt->ctrl) | GPT_CTRL_TEN, &gpt->ctrl);

	return 0;
}
