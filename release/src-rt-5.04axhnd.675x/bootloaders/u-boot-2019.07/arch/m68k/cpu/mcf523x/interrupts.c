// SPDX-License-Identifier: GPL-2.0+
/*
 *
 * Copyright (C) 2004-2007, 2012 Freescale Semiconductor, Inc.
 * TsiChung Liew (Tsi-Chung.Liew@freescale.com)
 */

/* CPU specific interrupt routine */
#include <common.h>
#include <asm/immap.h>
#include <asm/io.h>

int interrupt_init(void)
{
	int0_t *intp = (int0_t *) (CONFIG_SYS_INTR_BASE);

	/* Make sure all interrupts are disabled */
	setbits_be32(&intp->imrl0, 0x1);

	enable_interrupts();
	return 0;
}

#if defined(CONFIG_MCFTMR)
void dtimer_intr_setup(void)
{
	int0_t *intp = (int0_t *) (CONFIG_SYS_INTR_BASE);

	out_8(&intp->icr0[CONFIG_SYS_TMRINTR_NO], CONFIG_SYS_TMRINTR_PRI);
	clrbits_be32(&intp->imrl0, INTC_IPRL_INT0);
	clrbits_be32(&intp->imrl0, CONFIG_SYS_TMRINTR_MASK);
}
#endif
