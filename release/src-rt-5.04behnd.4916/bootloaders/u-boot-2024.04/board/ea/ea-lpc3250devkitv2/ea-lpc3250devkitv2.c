// SPDX-License-Identifier: GPL-2.0+
/*
 * Board init file for Embedded Artists LPC3250 DevKit v2
 * Copyright (C) 2021  Trevor Woerner <twoerner@gmail.com>
 */

#include <init.h>
#include <common.h>
#include <asm/io.h>
#include <asm/global_data.h>

#include <asm/arch/clk.h>
#include <asm/arch/wdt.h>
#include <asm/arch/sys_proto.h>

DECLARE_GLOBAL_DATA_PTR;

int
board_early_init_f(void)
{
	lpc32xx_uart_init(CONFIG_CONS_INDEX);
	if (IS_ENABLED(CONFIG_SYS_I2C_LPC32XX)) {
		lpc32xx_i2c_init(1);
		lpc32xx_i2c_init(2);
	}
	return 0;
}

int
board_init(void)
{
	gd->bd->bi_boot_params = CFG_SYS_SDRAM_BASE + 0x2000;
	return 0;
}

int
dram_init(void)
{
	gd->ram_size = get_ram_size((void *)CFG_SYS_SDRAM_BASE, SZ_64M);
	return 0;
}
