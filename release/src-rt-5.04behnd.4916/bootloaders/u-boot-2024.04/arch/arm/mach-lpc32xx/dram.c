// SPDX-License-Identifier: GPL-2.0+
/*
 * LPC32xx dram init
 *
 * (C) Copyright 2014  DENX Software Engineering GmbH
 * Written-by: Albert ARIBAUD <albert.aribaud@3adev.fr>
 *
 * This is called by SPL to gain access to the SDR DRAM.
 *
 * This code runs from SRAM.
 */

#include <common.h>
#include <netdev.h>
#include <asm/arch/cpu.h>
#include <asm/arch/clk.h>
#include <asm/arch/wdt.h>
#include <asm/arch/emc.h>
#include <asm/io.h>
#include <linux/delay.h>

static struct clk_pm_regs *clk = (struct clk_pm_regs *)CLK_PM_BASE;
static struct emc_regs *emc = (struct emc_regs *)EMC_BASE;

void ddr_init(struct emc_dram_settings *dram)
{
	uint32_t ck;

	/* Enable EMC interface and choose little endian mode */
	writel(1, &emc->ctrl);
	writel(0, &emc->config);
	/* Select maximum EMC Dynamic Memory Refresh Time */
	writel(0x7FF, &emc->refresh);
	/* Determine CLK */
	ck = get_sdram_clk_rate();
	/* Configure SDRAM */
	writel(dram->cmddelay, &clk->sdramclk_ctrl);
	writel(dram->config0, &emc->config0);
	writel(dram->rascas0, &emc->rascas0);
	writel(dram->rdconfig, &emc->read_config);
	/* Set timings */
	writel((ck / dram->trp) & 0x0000000F, &emc->t_rp);
	writel((ck / dram->tras) & 0x0000000F, &emc->t_ras);
	writel((ck / dram->tsrex) & 0x0000007F, &emc->t_srex);
	writel((ck / dram->twr) & 0x0000000F, &emc->t_wr);
	writel((ck / dram->trc) & 0x0000001F, &emc->t_rc);
	writel((ck / dram->trfc) & 0x0000001F, &emc->t_rfc);
	writel((ck / dram->txsr) & 0x000000FF, &emc->t_xsr);
	writel(dram->trrd, &emc->t_rrd);
	writel(dram->tmrd, &emc->t_mrd);
	writel(dram->tcdlr, &emc->t_cdlr);
	/* Dynamic refresh */
	writel((((ck / dram->refresh) >> 4) & 0x7FF), &emc->refresh);
	udelay(10);
	/* Force all clocks, enable inverted ck, issue NOP command */
	writel(0x00000193, &emc->control);
	udelay(100);
	/* Keep all clocks enabled, issue a PRECHARGE ALL command */
	writel(0x00000113, &emc->control);
	/* Fast dynamic refresh for at least a few SDRAM ck cycles */
	writel((((128) >> 4) & 0x7FF), &emc->refresh);
	udelay(10);
	/* set correct dynamic refresh timing */
	writel((((ck / dram->refresh) >> 4) & 0x7FF), &emc->refresh);
	udelay(10);
	/* set normal mode to CAS=3 */
	writel(0x00000093, &emc->control);
	readl(EMC_DYCS0_BASE | dram->mode);
	/* set extended mode to all zeroes */
	writel(0x00000093, &emc->control);
	readl(EMC_DYCS0_BASE | dram->emode);
	/* stop forcing clocks, keep inverted clock, issue normal mode */
	writel(0x00000010, &emc->control);
}
