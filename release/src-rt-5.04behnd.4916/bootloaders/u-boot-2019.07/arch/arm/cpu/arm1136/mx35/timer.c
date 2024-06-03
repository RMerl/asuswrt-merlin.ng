// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2007
 * Sascha Hauer, Pengutronix
 *
 * (C) Copyright 2008-2009 Freescale Semiconductor, Inc.
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/crm_regs.h>

/* General purpose timers bitfields */
#define GPTCR_SWR       (1<<15)	/* Software reset */
#define GPTCR_FRR       (1<<9)	/* Freerun / restart */
#define GPTCR_CLKSOURCE_32   (4<<6)	/* Clock source */
#define GPTCR_TEN       (1)	/* Timer enable */

/*
 * nothing really to do with interrupts, just starts up a counter.
 * The 32KHz 32-bit timer overruns in 134217 seconds
 */
int timer_init(void)
{
	int i;
	struct gpt_regs *gpt = (struct gpt_regs *)GPT1_BASE_ADDR;
	struct ccm_regs *ccm = (struct ccm_regs *)CCM_BASE_ADDR;

	/* setup GP Timer 1 */
	writel(GPTCR_SWR, &gpt->ctrl);

	writel(readl(&ccm->cgr1) | 3 << MXC_CCM_CGR1_GPT_OFFSET, &ccm->cgr1);

	for (i = 0; i < 100; i++)
		writel(0, &gpt->ctrl); /* We have no udelay by now */
	writel(0, &gpt->pre); /* prescaler = 1 */
	/* Freerun Mode, 32KHz input */
	writel(readl(&gpt->ctrl) | GPTCR_CLKSOURCE_32 | GPTCR_FRR,
			&gpt->ctrl);
	writel(readl(&gpt->ctrl) | GPTCR_TEN, &gpt->ctrl);

	return 0;
}
