// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2011 by Vladimir Zapolskiy <vz@mleia.com>
 */

#include <common.h>
#include <div64.h>
#include <asm/arch/cpu.h>
#include <asm/arch/clk.h>
#include <asm/io.h>

static struct clk_pm_regs *clk = (struct clk_pm_regs *)CLK_PM_BASE;

unsigned int get_sys_clk_rate(void)
{
	if (readl(&clk->sysclk_ctrl) & CLK_SYSCLK_PLL397)
		return RTC_CLK_FREQUENCY * 397;
	else
		return OSC_CLK_FREQUENCY;
}

unsigned int get_hclk_pll_rate(void)
{
	unsigned long long fin, fref, fcco, fout;
	u32 val, m_div, n_div, p_div;

	/*
	 * Valid frequency ranges:
	 *     1 * 10^6 <=  Fin <=  20 * 10^6
	 *     1 * 10^6 <= Fref <=  27 * 10^6
	 *   156 * 10^6 <= Fcco <= 320 * 10^6
	 */

	fref = fin = get_sys_clk_rate();
	if (fin > 20000000ULL || fin < 1000000ULL)
		return 0;

	val = readl(&clk->hclkpll_ctrl);
	m_div = ((val & CLK_HCLK_PLL_FEEDBACK_DIV_MASK) >> 1) + 1;
	n_div = ((val & CLK_HCLK_PLL_PREDIV_MASK) >> 9) + 1;
	if (val & CLK_HCLK_PLL_DIRECT)
		p_div = 0;
	else
		p_div = ((val & CLK_HCLK_PLL_POSTDIV_MASK) >> 11) + 1;
	p_div = 1 << p_div;

	if (val & CLK_HCLK_PLL_BYPASS) {
		do_div(fin, p_div);
		return fin;
	}

	do_div(fref, n_div);
	if (fref > 27000000ULL || fref < 1000000ULL)
		return 0;

	fcco = fref * m_div;
	fout = fcco;
	if (val & CLK_HCLK_PLL_FEEDBACK)
		fcco *= p_div;
	else
		do_div(fout, p_div);

	if (fcco > 320000000ULL || fcco < 156000000ULL)
		return 0;

	return fout;
}

unsigned int get_hclk_clk_div(void)
{
	u32 val;

	val = readl(&clk->hclkdiv_ctrl) & CLK_HCLK_ARM_PLL_DIV_MASK;

	return 1 << val;
}

unsigned int get_hclk_clk_rate(void)
{
	return get_hclk_pll_rate() / get_hclk_clk_div();
}

unsigned int get_periph_clk_div(void)
{
	u32 val;

	val = readl(&clk->hclkdiv_ctrl) & CLK_HCLK_PERIPH_DIV_MASK;

	return (val >> 2) + 1;
}

unsigned int get_periph_clk_rate(void)
{
	if (!(readl(&clk->pwr_ctrl) & CLK_PWR_NORMAL_RUN))
		return get_sys_clk_rate();

	return get_hclk_pll_rate() / get_periph_clk_div();
}

unsigned int get_sdram_clk_rate(void)
{
	unsigned int src_clk;

	if (!(readl(&clk->pwr_ctrl) & CLK_PWR_NORMAL_RUN))
		return get_sys_clk_rate();

	src_clk = get_hclk_pll_rate();

	if (readl(&clk->sdramclk_ctrl) & CLK_SDRAM_DDR_SEL) {
		/* using DDR */
		switch (readl(&clk->hclkdiv_ctrl) & CLK_HCLK_DDRAM_MASK) {
		case CLK_HCLK_DDRAM_HALF:
			return src_clk/2;
		case CLK_HCLK_DDRAM_NOMINAL:
			return src_clk;
		default:
			return 0;
		}
	} else {
		/* using SDR */
		switch (readl(&clk->hclkdiv_ctrl) & CLK_HCLK_ARM_PLL_DIV_MASK) {
		case CLK_HCLK_ARM_PLL_DIV_4:
			return src_clk/4;
		case CLK_HCLK_ARM_PLL_DIV_2:
			return src_clk/2;
		case CLK_HCLK_ARM_PLL_DIV_1:
			return src_clk;
		default:
			return 0;
		}
	}
}

int get_serial_clock(void)
{
	return get_periph_clk_rate();
}
