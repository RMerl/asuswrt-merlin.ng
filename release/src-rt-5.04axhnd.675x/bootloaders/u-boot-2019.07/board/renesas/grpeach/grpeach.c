// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017 Renesas Electronics
 * Copyright (C) Chris Brandt
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/sys_proto.h>

#define RZA1_WDT_BASE	0xfcfe0000
#define WTCSR		0x00
#define WTCNT		0x02
#define WRCSR		0x04

DECLARE_GLOBAL_DATA_PTR;

int board_init(void)
{
	gd->bd->bi_boot_params = (CONFIG_SYS_SDRAM_BASE + 0x100);

	return 0;
}

int dram_init(void)
{
	if (fdtdec_setup_mem_size_base() != 0)
		return -EINVAL;

	return 0;
}

int dram_init_banksize(void)
{
	fdtdec_setup_memory_banksize();

	return 0;
}

void reset_cpu(ulong addr)
{
	/* Dummy read (must read WRCSR:WOVF at least once before clearing) */
	readb(RZA1_WDT_BASE + WRCSR);

	writew(0xa500, RZA1_WDT_BASE + WRCSR);
	writew(0x5a5f, RZA1_WDT_BASE + WRCSR);
	writew(0x5a00, RZA1_WDT_BASE + WTCNT);
	writew(0xa578, RZA1_WDT_BASE + WTCSR);

	for (;;)
		asm volatile("wfi");
}
