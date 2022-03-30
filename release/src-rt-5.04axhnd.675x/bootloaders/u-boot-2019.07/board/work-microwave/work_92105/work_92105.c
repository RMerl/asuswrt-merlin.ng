// SPDX-License-Identifier: GPL-2.0+
/*
 * WORK Microwave work_92105 board support
 *
 * (C) Copyright 2014  DENX Software Engineering GmbH
 * Written-by: Albert ARIBAUD <albert.aribaud@3adev.fr>
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/cpu.h>
#include <asm/arch/clk.h>
#include <asm/arch/emc.h>
#include <asm/arch/wdt.h>
#include <asm/gpio.h>
#include <spl.h>
#include "work_92105_display.h"

DECLARE_GLOBAL_DATA_PTR;

static struct clk_pm_regs *clk = (struct clk_pm_regs *)CLK_PM_BASE;
static struct wdt_regs  *wdt = (struct wdt_regs *)WDT_BASE;

void reset_periph(void)
{
	setbits_le32(&clk->timclk_ctrl, CLK_TIMCLK_WATCHDOG);
	writel(WDTIM_MCTRL_RESFRC1, &wdt->mctrl);
	udelay(150);
	writel(0, &wdt->mctrl);
	clrbits_le32(&clk->timclk_ctrl, CLK_TIMCLK_WATCHDOG);
}

int board_early_init_f(void)
{
	/* initialize serial port for console */
	lpc32xx_uart_init(CONFIG_SYS_LPC32XX_UART);
	/* enable I2C, SSP, MAC, NAND */
	lpc32xx_i2c_init(1); /* only I2C1 has devices, I2C2 has none */
	lpc32xx_ssp_init();
	lpc32xx_mac_init();
	lpc32xx_mlc_nand_init();
	/* Display must wait until after relocation and devices init */
	return 0;
}

#define GPO_19 115

int board_early_init_r(void)
{
	/* Set NAND !WP to 1 through GPO_19 */
	gpio_request(GPO_19, "NAND_nWP");
	gpio_direction_output(GPO_19, 1);

#ifdef CONFIG_DEPRECATED
	/* initialize display */
	work_92105_display_init();
#endif

	return 0;
}

int board_init(void)
{
	reset_periph();
	/* adress of boot parameters */
	gd->bd->bi_boot_params  = CONFIG_SYS_SDRAM_BASE + 0x100;

	return 0;
}

int dram_init(void)
{
	gd->ram_size = get_ram_size((void *)CONFIG_SYS_SDRAM_BASE,
				    CONFIG_SYS_SDRAM_SIZE);

	return 0;
}
