// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2003
 * Josef Baumgartner <josef.baumgartner@telex.de>
 *
 * Copyright (C) 2004-2007, 2012 Freescale Semiconductor, Inc.
 * Hayden Fraser (Hayden.Fraser@freescale.com)
 */

#include <common.h>
#include <asm/processor.h>
#include <asm/immap.h>
#include <asm/io.h>

DECLARE_GLOBAL_DATA_PTR;

/* get_clocks() fills in gd->cpu_clock and gd->bus_clk */
int get_clocks (void)
{
#if defined(CONFIG_M5208)
	pll_t *pll = (pll_t *) MMAP_PLL;

	out_8(&pll->odr, CONFIG_SYS_PLL_ODR);
	out_8(&pll->fdr, CONFIG_SYS_PLL_FDR);
#endif

#if defined(CONFIG_M5249) || defined(CONFIG_M5253)
	volatile unsigned long cpll = mbar2_readLong(MCFSIM_PLLCR);
	unsigned long pllcr;

#ifndef CONFIG_SYS_PLL_BYPASS

#ifdef CONFIG_M5249
	/* Setup the PLL to run at the specified speed */
#ifdef CONFIG_SYS_FAST_CLK
	pllcr = 0x925a3100;	/* ~140MHz clock (PLL bypass = 0) */
#else
	pllcr = 0x135a4140;	/* ~72MHz clock (PLL bypass = 0) */
#endif
#endif				/* CONFIG_M5249 */

#ifdef CONFIG_M5253
	pllcr = CONFIG_SYS_PLLCR;
#endif				/* CONFIG_M5253 */

	cpll = cpll & 0xfffffffe;	/* Set PLL bypass mode = 0 (PSTCLK = crystal) */
	mbar2_writeLong(MCFSIM_PLLCR, cpll);	/* Set the PLL to bypass mode (PSTCLK = crystal) */
	mbar2_writeLong(MCFSIM_PLLCR, pllcr);	/* set the clock speed */
	pllcr ^= 0x00000001;	/* Set pll bypass to 1 */
	mbar2_writeLong(MCFSIM_PLLCR, pllcr);	/* Start locking (pll bypass = 1) */
	udelay(0x20);		/* Wait for a lock ... */
#endif				/* #ifndef CONFIG_SYS_PLL_BYPASS */

#endif				/* CONFIG_M5249 || CONFIG_M5253 */

#if defined(CONFIG_M5275)
	pll_t *pll = (pll_t *)(MMAP_PLL);

	/* Setup PLL */
	out_be32(&pll->syncr, 0x01080000);
	while (!(in_be32(&pll->synsr) & FMPLL_SYNSR_LOCK))
		;
	out_be32(&pll->syncr, 0x01000000);
	while (!(in_be32(&pll->synsr) & FMPLL_SYNSR_LOCK))
		;
#endif

	gd->cpu_clk = CONFIG_SYS_CLK;
#if defined(CONFIG_M5208) || defined(CONFIG_M5249) || defined(CONFIG_M5253) || \
    defined(CONFIG_M5271) || defined(CONFIG_M5275)
	gd->bus_clk = gd->cpu_clk / 2;
#else
	gd->bus_clk = gd->cpu_clk;
#endif

#ifdef CONFIG_SYS_I2C_FSL
	gd->arch.i2c1_clk = gd->bus_clk;
#ifdef CONFIG_SYS_I2C2_FSL_OFFSET
	gd->arch.i2c2_clk = gd->bus_clk;
#endif
#endif

	return (0);
}
