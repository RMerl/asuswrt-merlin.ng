// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000-2003
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Copyright (C) 2004-2007, 2012 Freescale Semiconductor, Inc.
 * TsiChung Liew (Tsi-Chung.Liew@freescale.com)
 */

#include <config.h>
#include <common.h>
#include <init.h>
#include <asm/global_data.h>
#include <asm/immap.h>
#include <asm/io.h>
#include <linux/delay.h>

DECLARE_GLOBAL_DATA_PTR;

int checkboard(void)
{
	puts("Board: ");
	puts("Freescale FireEngine 5329 EVB\n");
	return 0;
};

int dram_init(void)
{
	sdram_t *sdram = (sdram_t *)(MMAP_SDRAM);
	u32 dramsize, i;

	dramsize = CFG_SYS_SDRAM_SIZE * 0x100000;

	for (i = 0x13; i < 0x20; i++) {
		if (dramsize == (1 << i))
			break;
	}
	i--;

	out_be32(&sdram->cs0, CFG_SYS_SDRAM_BASE | i);
	out_be32(&sdram->cfg1, CFG_SYS_SDRAM_CFG1);
	out_be32(&sdram->cfg2, CFG_SYS_SDRAM_CFG2);

	/* Issue PALL */
	out_be32(&sdram->ctrl, CFG_SYS_SDRAM_CTRL | 2);

	/* Issue LEMR */
	out_be32(&sdram->mode, CFG_SYS_SDRAM_EMOD);
	out_be32(&sdram->mode, CFG_SYS_SDRAM_MODE | 0x04000000);

	udelay(500);

	/* Issue PALL */
	out_be32(&sdram->ctrl, CFG_SYS_SDRAM_CTRL | 2);

	/* Perform two refresh cycles */
	out_be32(&sdram->ctrl, CFG_SYS_SDRAM_CTRL | 4);
	out_be32(&sdram->ctrl, CFG_SYS_SDRAM_CTRL | 4);

	out_be32(&sdram->mode, CFG_SYS_SDRAM_MODE);

	out_be32(&sdram->ctrl,
		(CFG_SYS_SDRAM_CTRL & ~0x80000000) | 0x10000c00);

	udelay(100);

	gd->ram_size = dramsize;

	return 0;
};

int testdram(void)
{
	/* TODO: XXX XXX XXX */
	printf("DRAM test not implemented!\n");

	return (0);
}
