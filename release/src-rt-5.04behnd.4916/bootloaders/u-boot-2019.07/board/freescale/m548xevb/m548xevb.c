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
#include <pci.h>
#include <asm/immap.h>
#include <asm/io.h>

DECLARE_GLOBAL_DATA_PTR;

int checkboard(void)
{
	puts("Board: ");
	puts("Freescale FireEngine 5485 EVB\n");
	return 0;
};

int dram_init(void)
{
	siu_t *siu = (siu_t *) (MMAP_SIU);
	sdram_t *sdram = (sdram_t *)(MMAP_SDRAM);
	u32 dramsize, i;
#ifdef CONFIG_SYS_DRAMSZ1
	u32 temp;
#endif

	out_be32(&siu->drv, CONFIG_SYS_SDRAM_DRVSTRENGTH);

	dramsize = CONFIG_SYS_DRAMSZ * 0x100000;
	for (i = 0x13; i < 0x20; i++) {
		if (dramsize == (1 << i))
			break;
	}
	i--;
	out_be32(&siu->cs0cfg, CONFIG_SYS_SDRAM_BASE | i);

#ifdef CONFIG_SYS_DRAMSZ1
	temp = CONFIG_SYS_DRAMSZ1 * 0x100000;
	for (i = 0x13; i < 0x20; i++) {
		if (temp == (1 << i))
			break;
	}
	i--;
	dramsize += temp;
	out_be32(&siu->cs1cfg, (CONFIG_SYS_SDRAM_BASE + temp) | i);
#endif

	out_be32(&sdram->cfg1, CONFIG_SYS_SDRAM_CFG1);
	out_be32(&sdram->cfg2, CONFIG_SYS_SDRAM_CFG2);

	/* Issue PALL */
	out_be32(&sdram->ctrl, CONFIG_SYS_SDRAM_CTRL | 2);

	/* Issue LEMR */
	out_be32(&sdram->mode, CONFIG_SYS_SDRAM_EMOD);
	out_be32(&sdram->mode, CONFIG_SYS_SDRAM_MODE | 0x04000000);

	udelay(500);

	/* Issue PALL */
	out_be32(&sdram->ctrl, CONFIG_SYS_SDRAM_CTRL | 2);

	/* Perform two refresh cycles */
	out_be32(&sdram->ctrl, CONFIG_SYS_SDRAM_CTRL | 4);
	out_be32(&sdram->ctrl, CONFIG_SYS_SDRAM_CTRL | 4);

	out_be32(&sdram->mode, CONFIG_SYS_SDRAM_MODE);

	out_be32(&sdram->ctrl,
		(CONFIG_SYS_SDRAM_CTRL & ~0x80000000) | 0x10000F00);

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

#if defined(CONFIG_PCI)
/*
 * Initialize PCI devices, report devices found.
 */
static struct pci_controller hose;
extern void pci_mcf547x_8x_init(struct pci_controller *hose);

void pci_init_board(void)
{
	pci_mcf547x_8x_init(&hose);
}
#endif				/* CONFIG_PCI */
