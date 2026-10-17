// SPDX-License-Identifier: GPL-2.0+
/*
 * Timll DevKit3250 board support, SPL board configuration
 *
 * (C) Copyright 2015 Vladimir Zapolskiy <vz@mleia.com>
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/cpu.h>
#include <asm/arch/emc.h>
#include <asm/arch-lpc32xx/gpio.h>
#include <spl.h>

static struct gpio_regs *gpio = (struct gpio_regs *)GPIO_BASE;

/*
 * SDRAM K4S561632N-LC60 settings are selected in assumption that
 * SDRAM clock may be set up to 166 MHz, however at the moment
 * it is 104 MHz. Most delay values are converted to be a multiple of
 * base clock, and precise pinned values are not needed here.
 */
struct emc_dram_settings dram_64mb = {
	.cmddelay	= 0x0001C000,
	.config0	= 0x00005682,
	.rascas0	= 0x00000302,
	.rdconfig	= 0x00000011,	/* undocumented but crucial value */

	.trp	= 83333333,
	.tras	= 23809524,
	.tsrex	= 12500000,
	.twr	= 83000000,		/* tWR = tRDL = 2 CLK */
	.trc	= 15384616,
	.trfc	= 15384616,
	.txsr	= 12500000,
	.trrd	= 1,
	.tmrd	= 1,
	.tcdlr	= 0,

	.refresh	= 130000,	/* 800 clock cycles */

	.mode	= 0x00018000,
	.emode	= 0x02000000,
};

void spl_board_init(void)
{
	/* First of all silence buzzer controlled by GPO_20 */
	writel((1 << 20), &gpio->p3_outp_clr);

	lpc32xx_uart_init(CONFIG_CONS_INDEX);
	preloader_console_init();

	ddr_init(&dram_64mb);

	/*
	 * NAND initialization is done by nand_init(),
	 * here just enable NAND SLC clocks
	 */
	lpc32xx_slc_nand_init();
}

u32 spl_boot_device(void)
{
	return BOOT_DEVICE_NAND;
}
