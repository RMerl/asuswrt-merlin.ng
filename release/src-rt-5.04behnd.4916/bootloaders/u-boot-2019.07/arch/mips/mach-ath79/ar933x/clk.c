// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015-2016 Wills Wang <wills.wang@live.com>
 */

#include <common.h>
#include <asm/io.h>
#include <asm/addrspace.h>
#include <asm/types.h>
#include <mach/ar71xx_regs.h>
#include <mach/ath79.h>

DECLARE_GLOBAL_DATA_PTR;

static u32 ar933x_get_xtal(void)
{
	u32 val;

	val = ath79_get_bootstrap();
	if (val & AR933X_BOOTSTRAP_REF_CLK_40)
		return 40000000;
	else
		return 25000000;
}

int get_serial_clock(void)
{
	return ar933x_get_xtal();
}

int get_clocks(void)
{
	void __iomem *regs;
	u32 val, xtal, pll, div;

	regs = map_physmem(AR71XX_PLL_BASE, AR71XX_PLL_SIZE,
			   MAP_NOCACHE);
	xtal = ar933x_get_xtal();
	val = readl(regs + AR933X_PLL_CPU_CONFIG_REG);

	/* VCOOUT = XTAL * DIV_INT */
	div = (val >> AR933X_PLL_CPU_CONFIG_REFDIV_SHIFT)
			& AR933X_PLL_CPU_CONFIG_REFDIV_MASK;
	pll = xtal / div;

	/* PLLOUT = VCOOUT * (1/2^OUTDIV) */
	div = (val >> AR933X_PLL_CPU_CONFIG_NINT_SHIFT)
			& AR933X_PLL_CPU_CONFIG_NINT_MASK;
	pll *= div;
	div = (val >> AR933X_PLL_CPU_CONFIG_OUTDIV_SHIFT)
			& AR933X_PLL_CPU_CONFIG_OUTDIV_MASK;
	if (!div)
		div = 1;
	pll >>= div;

	val = readl(regs + AR933X_PLL_CLK_CTRL_REG);

	/* CPU_CLK = PLLOUT / CPU_POST_DIV */
	div = ((val >> AR933X_PLL_CLK_CTRL_CPU_POST_DIV_SHIFT)
			& AR933X_PLL_CLK_CTRL_CPU_POST_DIV_MASK) + 1;
	gd->cpu_clk = pll / div;

	/* DDR_CLK = PLLOUT / DDR_POST_DIV */
	div = ((val >> AR933X_PLL_CLK_CTRL_DDR_POST_DIV_SHIFT)
			& AR933X_PLL_CLK_CTRL_DDR_POST_DIV_MASK) + 1;
	gd->mem_clk = pll / div;

	/* AHB_CLK = PLLOUT / AHB_POST_DIV */
	div = ((val >> AR933X_PLL_CLK_CTRL_AHB_POST_DIV_SHIFT)
			& AR933X_PLL_CLK_CTRL_AHB_POST_DIV_MASK) + 1;
	gd->bus_clk = pll / div;

	return 0;
}

ulong get_bus_freq(ulong dummy)
{
	if (!gd->bus_clk)
		get_clocks();
	return gd->bus_clk;
}

ulong get_ddr_freq(ulong dummy)
{
	if (!gd->mem_clk)
		get_clocks();
	return gd->mem_clk;
}
