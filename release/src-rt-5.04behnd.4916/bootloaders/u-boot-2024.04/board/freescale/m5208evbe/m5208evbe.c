// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000-2003
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Copyright (C) 2004-2008, 2012 Freescale Semiconductor, Inc.
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
	puts("Freescale M5208EVBe\n");
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
#ifdef CFG_SYS_SDRAM_BASE1
	out_be32(&sdram->cs1, CFG_SYS_SDRAM_BASE | i);
#endif
	out_be32(&sdram->cfg1, CFG_SYS_SDRAM_CFG1);
	out_be32(&sdram->cfg2, CFG_SYS_SDRAM_CFG2);

	udelay(500);

	/* Issue PALL */
	out_be32(&sdram->ctrl, CFG_SYS_SDRAM_CTRL | 2);
	asm("nop");

	/* Perform two refresh cycles */
	out_be32(&sdram->ctrl, CFG_SYS_SDRAM_CTRL | 4);
	out_be32(&sdram->ctrl, CFG_SYS_SDRAM_CTRL | 4);
	asm("nop");

	/* Issue LEMR */
	out_be32(&sdram->mode, CFG_SYS_SDRAM_MODE);
	asm("nop");
	out_be32(&sdram->mode, CFG_SYS_SDRAM_EMOD);
	asm("nop");

	out_be32(&sdram->ctrl, CFG_SYS_SDRAM_CTRL | 2);
	asm("nop");

	out_be32(&sdram->ctrl,
		(CFG_SYS_SDRAM_CTRL & ~0x80000000) | 0x10000F00);
	asm("nop");

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
