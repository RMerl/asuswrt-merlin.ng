// SPDX-License-Identifier: GPL-2.0+
/*
 * Broadcom Northstar generic board set-up code
 * Copyright (C) 2023 Linus Walleij <linus.walleij@linaro.org>
 */

#include <common.h>
#include <dm.h>
#include <init.h>
#include <log.h>
#include <ram.h>
#include <serial.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/armv7m.h>

DECLARE_GLOBAL_DATA_PTR;

int dram_init(void)
{
	return fdtdec_setup_mem_size_base();
}

int dram_init_banksize(void)
{
	return fdtdec_setup_memory_banksize();
}

int board_late_init(void)
{
	/* LEDs etc can be initialized here */
	return 0;
}

int board_init(void)
{
	return 0;
}

void reset_cpu(void)
{
}

int print_cpuinfo(void)
{
	printf("BCMNS Northstar SoC\n");
	return 0;
}

int misc_init_r(void)
{
	return 0;
}

int ft_board_setup(void *fdt, struct bd_info *bd)
{
	printf("Northstar board setup: DTB at 0x%08lx\n", (ulong)fdt);
	return 0;
}

