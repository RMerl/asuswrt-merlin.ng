// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000-2003
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Copyright (C) 2004-2007, 2012 Freescale Semiconductor, Inc.
 * TsiChung Liew (Tsi-Chung.Liew@freescale.com)
 */

#include <common.h>
#include <asm/immap.h>
#include <asm/io.h>

DECLARE_GLOBAL_DATA_PTR;

int checkboard(void)
{
	puts("Board: ");
	puts("Freescale M52277 EVB\n");
	return 0;
};

int dram_init(void)
{
	u32 dramsize;

#ifdef CONFIG_CF_SBF
	/*
	 * Serial Boot: The dram is already initialized in start.S
	 * only require to return DRAM size
	 */
	dramsize = CONFIG_SYS_SDRAM_SIZE * 0x100000;
#else
	sdramc_t *sdram = (sdramc_t *)(MMAP_SDRAM);
	gpio_t *gpio = (gpio_t *)(MMAP_GPIO);
	u32 i;

	dramsize = CONFIG_SYS_SDRAM_SIZE * 0x100000;

	for (i = 0x13; i < 0x20; i++) {
		if (dramsize == (1 << i))
			break;
	}
	i--;

	out_8(&gpio->mscr_sdram, CONFIG_SYS_SDRAM_DRV_STRENGTH);

	out_be32(&sdram->sdcs0, CONFIG_SYS_SDRAM_BASE | i);

	out_be32(&sdram->sdcfg1, CONFIG_SYS_SDRAM_CFG1);
	out_be32(&sdram->sdcfg2, CONFIG_SYS_SDRAM_CFG2);

	/* Issue PALL */
	out_be32(&sdram->sdcr, CONFIG_SYS_SDRAM_CTRL | 2);
	__asm__("nop");

	/* Issue LEMR */
	out_be32(&sdram->sdmr, CONFIG_SYS_SDRAM_MODE);
	__asm__("nop");
	out_be32(&sdram->sdmr, CONFIG_SYS_SDRAM_EMOD);
	__asm__("nop");

	udelay(1000);

	/* Issue PALL */
	out_be32(&sdram->sdcr, CONFIG_SYS_SDRAM_CTRL | 2);
	__asm__("nop");

	/* Perform two refresh cycles */
	out_be32(&sdram->sdcr, CONFIG_SYS_SDRAM_CTRL | 4);
	__asm__("nop");
	out_be32(&sdram->sdcr, CONFIG_SYS_SDRAM_CTRL | 4);
	__asm__("nop");

	out_be32(&sdram->sdcr,
		(CONFIG_SYS_SDRAM_CTRL & ~0x80000000) | 0x10000c00);

	udelay(100);
#endif
	gd->ram_size = dramsize;

	return 0;
};

int testdram(void)
{
	/* TODO: XXX XXX XXX */
	printf("DRAM test not implemented!\n");

	return (0);
}
