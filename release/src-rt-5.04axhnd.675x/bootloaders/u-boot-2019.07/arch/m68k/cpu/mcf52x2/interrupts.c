// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000-2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Copyright (C) 2004-2007, 2012 Freescale Semiconductor, Inc.
 * TsiChung Liew (Tsi-Chung.Liew@freescale.com)
 */

#include <common.h>
#include <watchdog.h>
#include <asm/processor.h>
#include <asm/immap.h>
#include <asm/io.h>

#ifdef	CONFIG_M5272
int interrupt_init(void)
{
	intctrl_t *intp = (intctrl_t *) (MMAP_INTC);

	/* disable all external interrupts */
	out_be32(&intp->int_icr1, 0x88888888);
	out_be32(&intp->int_icr2, 0x88888888);
	out_be32(&intp->int_icr3, 0x88888888);
	out_be32(&intp->int_icr4, 0x88888888);
	out_be32(&intp->int_pitr, 0x00000000);

	/* initialize vector register */
	out_8(&intp->int_pivr, 0x40);

	enable_interrupts();

	return 0;
}

#if defined(CONFIG_MCFTMR)
void dtimer_intr_setup(void)
{
	intctrl_t *intp = (intctrl_t *) (CONFIG_SYS_INTR_BASE);

	clrbits_be32(&intp->int_icr1, INT_ICR1_TMR3MASK);
	setbits_be32(&intp->int_icr1, CONFIG_SYS_TMRINTR_PRI);
}
#endif				/* CONFIG_MCFTMR */
#endif				/* CONFIG_M5272 */

#if defined(CONFIG_M5208) || defined(CONFIG_M5282) || \
    defined(CONFIG_M5271) || defined(CONFIG_M5275)
int interrupt_init(void)
{
	int0_t *intp = (int0_t *) (CONFIG_SYS_INTR_BASE);

	/* Make sure all interrupts are disabled */
#if defined(CONFIG_M5208)
	out_be32(&intp->imrl0, 0xffffffff);
	out_be32(&intp->imrh0, 0xffffffff);
#else
	setbits_be32(&intp->imrl0, 0x1);
#endif

	enable_interrupts();
	return 0;
}

#if defined(CONFIG_MCFTMR)
void dtimer_intr_setup(void)
{
	int0_t *intp = (int0_t *) (CONFIG_SYS_INTR_BASE);

	out_8(&intp->icr0[CONFIG_SYS_TMRINTR_NO], CONFIG_SYS_TMRINTR_PRI);
	clrbits_be32(&intp->imrl0, 0x00000001);
	clrbits_be32(&intp->imrl0, CONFIG_SYS_TMRINTR_MASK);
}
#endif				/* CONFIG_MCFTMR */
#endif				/* CONFIG_M5282 | CONFIG_M5271 | CONFIG_M5275 */

#if defined(CONFIG_M5249) || defined(CONFIG_M5253)
int interrupt_init(void)
{
	enable_interrupts();

	return 0;
}

#if defined(CONFIG_MCFTMR)
void dtimer_intr_setup(void)
{
	mbar_writeLong(MCFSIM_IMR, mbar_readLong(MCFSIM_IMR) & ~0x00000400);
	mbar_writeByte(MCFSIM_TIMER2ICR, CONFIG_SYS_TMRINTR_PRI);
}
#endif				/* CONFIG_MCFTMR */
#endif				/* CONFIG_M5249 || CONFIG_M5253 */
