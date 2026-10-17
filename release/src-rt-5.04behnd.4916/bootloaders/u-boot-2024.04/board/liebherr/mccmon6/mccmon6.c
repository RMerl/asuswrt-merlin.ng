// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016-2017
 * Lukasz Majewski, DENX Software Engineering, lukma@denx.de
 */

#include <common.h>
#include <env.h>
#include <init.h>
#include <serial.h>
#include <asm/arch/clock.h>
#include <asm/arch/iomux.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/sys_proto.h>
#include <asm/global_data.h>
#include <asm/gpio.h>

DECLARE_GLOBAL_DATA_PTR;

int dram_init(void)
{
	gd->ram_size = imx_ddr_size();

	return 0;
}

int board_init(void)
{
	/* address of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM + 0x100;

	return 0;
}

int board_late_init(void)
{
	env_set("board_name", "mccmon6");

	return 0;
}

int checkboard(void)
{
	puts("Board: MCCMON6\n");

	return 0;
}
