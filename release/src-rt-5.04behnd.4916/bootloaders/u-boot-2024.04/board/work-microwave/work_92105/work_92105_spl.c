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
#include <asm/arch/emc.h>
#include <asm/gpio.h>
#include <spl.h>
#include "work_92105_display.h"

struct emc_dram_settings dram_64mb = {
	.cmddelay = 0x0001C000,
	.config0 = 0x00005682,
	.rascas0 = 0x00000302,
	.rdconfig = 0x00000011,
	.trp = 52631578,
	.tras = 20833333,
	.tsrex = 12500000,
	.twr = 66666666,
	.trc = 13888888,
	.trfc = 10256410,
	.txsr = 12500000,
	.trrd = 1,
	.tmrd = 1,
	.tcdlr = 0,
	.refresh = 128000,
	.mode = 0x00018000,
	.emode = 0x02000000
};

const struct emc_dram_settings dram_128mb = {
	.cmddelay = 0x0001C000,
	.config0 = 0x00005882,
	.rascas0 = 0x00000302,
	.rdconfig = 0x00000011,
	.trp = 52631578,
	.tras = 22222222,
	.tsrex = 8333333,
	.twr = 66666666,
	.trc = 14814814,
	.trfc = 10256410,
	.txsr = 8333333,
	.trrd = 1,
	.tmrd = 1,
	.tcdlr = 0,
	.refresh = 128000,
	.mode = 0x00030000,
	.emode = 0x02000000
};

void spl_board_init(void)
{
	/* initialize serial port for console */
	lpc32xx_uart_init(CONFIG_CONS_INDEX);
	/* initialize console */
	preloader_console_init();
	/* init DDR and NAND to chainload U-Boot */
	ddr_init(&dram_128mb);
	/*
	 * If this is actually a 64MB module, then the highest column
	 * bit in any address will be ignored, and thus address 0x80000000
	 * should be mirrored at address 0x80000800. Test this.
	 */
	writel(0x31415926, 0x80000000); /* write Pi at 0x80000000 */
	writel(0x16180339, 0x80000800); /* write Phi at 0x80000800 */
	if (readl(0x80000000) == 0x16180339) /* check 0x80000000 */ {
		/* actually 64MB mirrored: reconfigure controller */
		ddr_init(&dram_64mb);
	}
	/* initialize NAND controller to load U-Boot from NAND */
	lpc32xx_mlc_nand_init();
}

u32 spl_boot_device(void)
{
	return BOOT_DEVICE_NAND;
}
