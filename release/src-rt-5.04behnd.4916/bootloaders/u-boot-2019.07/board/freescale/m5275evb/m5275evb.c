// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000-2003
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Copyright (C) 2005-2008 Arthur Shipkowski (art@videon-central.com)
 *
 * Copyright (C) 2012 Freescale Semiconductor, Inc. All Rights Reserved.
 */

#include <common.h>
#include <asm/immap.h>
#include <asm/io.h>

DECLARE_GLOBAL_DATA_PTR;

#define PERIOD		13	/* system bus period in ns */
#define SDRAM_TREFI	7800	/* in ns */

int checkboard(void)
{
	puts("Board: ");
	puts("Freescale MCF5275 EVB\n");
	return 0;
};

int dram_init(void)
{
	sdramctrl_t *sdp = (sdramctrl_t *)(MMAP_SDRAM);
	gpio_t *gpio_reg = (gpio_t *)(MMAP_GPIO);

	/* Enable SDRAM */
	out_be16(&gpio_reg->par_sdram, 0x3FF);

	/* Set up chip select */
	out_be32(&sdp->sdbar0, CONFIG_SYS_SDRAM_BASE);
	out_be32(&sdp->sdbmr0, MCF_SDRAMC_SDMRn_BAM_32M | MCF_SDRAMC_SDMRn_V);

	/* Set up timing */
	out_be32(&sdp->sdcfg1, 0x83711630);
	out_be32(&sdp->sdcfg2, 0x46770000);

	/* Enable clock */
	out_be32(&sdp->sdcr, MCF_SDRAMC_SDCR_MODE_EN | MCF_SDRAMC_SDCR_CKE);

	/* Set precharge */
	setbits_be32(&sdp->sdcr, MCF_SDRAMC_SDCR_IPALL);

	/* Dummy write to start SDRAM */
	*((volatile unsigned long *)CONFIG_SYS_SDRAM_BASE) = 0xa5a59696;

	/* Send LEMR */
	setbits_be32(&sdp->sdmr,
		MCF_SDRAMC_SDMR_BNKAD_LEMR | MCF_SDRAMC_SDMR_AD(0x0) |
		MCF_SDRAMC_SDMR_CMD);
	*((volatile unsigned long *)CONFIG_SYS_SDRAM_BASE) = 0xa5a59696;

	/* Send LMR */
	out_be32(&sdp->sdmr, 0x058d0000);
	*((volatile unsigned long *)CONFIG_SYS_SDRAM_BASE) = 0xa5a59696;

	/* Stop sending commands */
	clrbits_be32(&sdp->sdmr, MCF_SDRAMC_SDMR_CMD);

	/* Set precharge */
	setbits_be32(&sdp->sdcr, MCF_SDRAMC_SDCR_IPALL);
	*((volatile unsigned long *)CONFIG_SYS_SDRAM_BASE) = 0xa5a59696;

	/* Stop manual precharge, send 2 IREF */
	clrbits_be32(&sdp->sdcr, MCF_SDRAMC_SDCR_IPALL);
	setbits_be32(&sdp->sdcr, MCF_SDRAMC_SDCR_IREF);
	*((volatile unsigned long *)CONFIG_SYS_SDRAM_BASE) = 0xa5a59696;
	*((volatile unsigned long *)CONFIG_SYS_SDRAM_BASE) = 0xa5a59696;


	out_be32(&sdp->sdmr, 0x018d0000);
	*((volatile unsigned long *)CONFIG_SYS_SDRAM_BASE) = 0xa5a59696;

	/* Stop sending commands */
	clrbits_be32(&sdp->sdmr, MCF_SDRAMC_SDMR_CMD);
	clrbits_be32(&sdp->sdcr, MCF_SDRAMC_SDCR_MODE_EN);

	/* Turn on auto refresh, lock SDMR */
	out_be32(&sdp->sdcr,
		MCF_SDRAMC_SDCR_CKE
		| MCF_SDRAMC_SDCR_REF
		| MCF_SDRAMC_SDCR_MUX(1)
		/* 1 added to round up */
		| MCF_SDRAMC_SDCR_RCNT((SDRAM_TREFI/(PERIOD*64)) - 1 + 1)
		| MCF_SDRAMC_SDCR_DQS_OE(0x3));

	gd->ram_size = CONFIG_SYS_SDRAM_SIZE * 1024 * 1024;

	return 0;
};

int testdram(void)
{
	/* TODO: XXX XXX XXX */
	printf("DRAM test not implemented!\n");

	return (0);
}
