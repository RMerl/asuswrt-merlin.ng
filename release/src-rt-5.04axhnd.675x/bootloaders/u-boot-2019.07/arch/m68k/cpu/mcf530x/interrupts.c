// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2014  Angelo Dureghello <angelo@sysam.it>
 *
 */

#include <common.h>
#include <asm/immap.h>
#include <asm/io.h>

#ifdef CONFIG_M5307
int interrupt_init(void)
{
	enable_interrupts();

	return 0;
}

void dtimer_intr_setup(void)
{
	intctrl_t *icr = (intctrl_t *)(MMAP_INTC);

	/* clearing TIMER2 mask, so enabling the related interrupt */
	out_be32(&icr->imr, in_be32(&icr->imr) & ~0x00000400);
	/* set TIMER2 interrupt priority */
	out_8(&icr->icr2, CONFIG_SYS_TMRINTR_PRI);
}
#endif
