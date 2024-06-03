// SPDX-License-Identifier: GPL-2.0+
/*
 * Board-specific init.
 *
 * (C) Copyright 2017 Angelo Dureghello <angelo@sysam.it>
 */

#include <common.h>
#include <spi.h>
#include <asm/io.h>
#include <asm/immap.h>
#include <mmc.h>
#include <fsl_esdhc.h>

DECLARE_GLOBAL_DATA_PTR;

int checkboard(void)
{
	/*
	 * need to to:
	 * Check serial flash size. if 2mb evb, else 8mb demo
	 */
	puts("Board: ");
	puts("Sysam stmark2\n");
	return 0;
}

int dram_init(void)
{
	u32 dramsize;

	/*
	 * Serial Boot: The dram is already initialized in start.S
	 * only require to return DRAM size
	 */
	dramsize = CONFIG_SYS_SDRAM_SIZE * 0x100000;

	gd->ram_size = dramsize;

	return 0;
}

int testdram(void)
{
	return 0;
}
