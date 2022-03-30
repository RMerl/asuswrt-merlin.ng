// SPDX-License-Identifier: GPL-2.0+
/*
 *
 * (C) Copyright 2000-2003
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Copyright (C) 2004-2007, 2012 Freescale Semiconductor, Inc.
 * TsiChung Liew (Tsi-Chung.Liew@freescale.com)
 */

#include <common.h>
#include <asm/processor.h>

#include <asm/immap.h>
#include <asm/io.h>

DECLARE_GLOBAL_DATA_PTR;
/*
 * get_clocks() fills in gd->cpu_clock and gd->bus_clk
 */
int get_clocks(void)
{
	pll_t *pll = (pll_t *)(MMAP_PLL);

	out_be32(&pll->syncr, PLL_SYNCR_MFD(1));

	while (!(in_be32(&pll->synsr) & PLL_SYNSR_LOCK))
		;

	gd->bus_clk = CONFIG_SYS_CLK;
	gd->cpu_clk = (gd->bus_clk * 2);

#ifdef CONFIG_SYS_I2C_FSL
	gd->arch.i2c1_clk = gd->bus_clk;
#endif

	return (0);
}
