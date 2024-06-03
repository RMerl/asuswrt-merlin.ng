// SPDX-License-Identifier: GPL-2.0+
/*
 * Embest/Timll DevKit3250 board support
 *
 * Copyright (C) 2011-2015 Vladimir Zapolskiy <vz@mleia.com>
 */

#include <common.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/clk.h>
#include <asm/arch/cpu.h>
#include <asm/arch/emc.h>
#include <asm/arch/wdt.h>
#include <asm/io.h>

DECLARE_GLOBAL_DATA_PTR;

static struct emc_regs *emc = (struct emc_regs *)EMC_BASE;
static struct clk_pm_regs *clk = (struct clk_pm_regs *)CLK_PM_BASE;
static struct wdt_regs *wdt = (struct wdt_regs *)WDT_BASE;

void reset_periph(void)
{
	/* This function resets peripherals by triggering RESOUT_N */
	setbits_le32(&clk->timclk_ctrl, CLK_TIMCLK_WATCHDOG);
	writel(WDTIM_MCTRL_RESFRC1, &wdt->mctrl);
	udelay(300);

	writel(0, &wdt->mctrl);
	clrbits_le32(&clk->timclk_ctrl, CLK_TIMCLK_WATCHDOG);

	/* Such a long delay is needed to initialize SMSC phy */
	udelay(10000);
}

int board_early_init_f(void)
{
	lpc32xx_uart_init(CONFIG_SYS_LPC32XX_UART);
	lpc32xx_i2c_init(1);
	lpc32xx_i2c_init(2);
	lpc32xx_ssp_init();
	lpc32xx_mac_init();

	/*
	 * nWP may be controlled by GPO19, but unpopulated by default R23
	 * makes no sense to configure this GPIO level, nWP is always high
	 */
	lpc32xx_slc_nand_init();

	return 0;
}

int board_init(void)
{
	/* adress of boot parameters */
	gd->bd->bi_boot_params  = CONFIG_SYS_SDRAM_BASE + 0x100;

#ifdef CONFIG_SYS_FLASH_CFI
	/* Use 16-bit memory interface for NOR Flash */
	emc->stat[0].config	= EMC_STAT_CONFIG_PB | EMC_STAT_CONFIG_16BIT;

	/* Change the NOR timings to optimum value to get maximum bandwidth */
	emc->stat[0].waitwen	= EMC_STAT_WAITWEN(1);
	emc->stat[0].waitoen	= EMC_STAT_WAITOEN(0);
	emc->stat[0].waitrd	= EMC_STAT_WAITRD(12);
	emc->stat[0].waitpage	= EMC_STAT_WAITPAGE(12);
	emc->stat[0].waitwr	= EMC_STAT_WAITWR(5);
	emc->stat[0].waitturn	= EMC_STAT_WAITTURN(2);
#endif

	return 0;
}

int dram_init(void)
{
	gd->ram_size = get_ram_size((void *)CONFIG_SYS_SDRAM_BASE,
				    CONFIG_SYS_SDRAM_SIZE);

	return 0;
}
