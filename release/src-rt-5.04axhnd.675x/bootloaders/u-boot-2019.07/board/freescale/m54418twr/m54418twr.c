// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2010-2012 Freescale Semiconductor, Inc.
 * TsiChung Liew (Tsi-Chung.Liew@freescale.com)
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
	puts("Freescale MCF54418 Tower System\n");
	return 0;
};

int dram_init(void)
{
	u32 dramsize;

#if defined(CONFIG_SERIAL_BOOT)
	/*
	 * Serial Boot: The dram is already initialized in start.S
	 * only require to return DRAM size
	 */
	dramsize = CONFIG_SYS_SDRAM_SIZE * 0x100000;
#else
	sdramc_t *sdram = (sdramc_t *)(MMAP_SDRAM);
	ccm_t *ccm = (ccm_t *)MMAP_CCM;
	gpio_t *gpio = (gpio_t *) MMAP_GPIO;
	pm_t *pm = (pm_t *) MMAP_PM;
	u32 i;

	dramsize = CONFIG_SYS_SDRAM_SIZE * 0x100000;

	for (i = 0x13; i < 0x20; i++) {
		if (dramsize == (1 << i))
			break;
	}

	out_8(&pm->pmcr0, 0x2E);
	out_8(&gpio->mscr_sdram, 1);

	clrbits_be16(&ccm->misccr2, CCM_MISCCR2_FBHALF);
	setbits_be16(&ccm->misccr2, CCM_MISCCR2_DDR2CLK);

	out_be32(&sdram->rcrcr, 0x40000000);
	out_be32(&sdram->padcr, 0x01030203);

	out_be32(&sdram->cr00, 0x01010101);
	out_be32(&sdram->cr01, 0x00000101);
	out_be32(&sdram->cr02, 0x01010100);
	out_be32(&sdram->cr03, 0x01010000);
	out_be32(&sdram->cr04, 0x00010101);
	out_be32(&sdram->cr06, 0x00010100);
	out_be32(&sdram->cr07, 0x00000001);
	out_be32(&sdram->cr08, 0x01000001);
	out_be32(&sdram->cr09, 0x00000100);
	out_be32(&sdram->cr10, 0x00010001);
	out_be32(&sdram->cr11, 0x00000200);
	out_be32(&sdram->cr12, 0x01000002);
	out_be32(&sdram->cr13, 0x00000000);
	out_be32(&sdram->cr14, 0x00000100);
	out_be32(&sdram->cr15, 0x02000100);
	out_be32(&sdram->cr16, 0x02000407);
	out_be32(&sdram->cr17, 0x02030007);
	out_be32(&sdram->cr18, 0x02000100);
	out_be32(&sdram->cr19, 0x0A030203);
	out_be32(&sdram->cr20, 0x00020708);
	out_be32(&sdram->cr21, 0x00050008);
	out_be32(&sdram->cr22, 0x04030002);
	out_be32(&sdram->cr23, 0x00000004);
	out_be32(&sdram->cr24, 0x020A0000);
	out_be32(&sdram->cr25, 0x0C00000E);
	out_be32(&sdram->cr26, 0x00002004);
	out_be32(&sdram->cr28, 0x00100010);
	out_be32(&sdram->cr29, 0x00100010);
	out_be32(&sdram->cr31, 0x07990000);
	out_be32(&sdram->cr40, 0x00000000);
	out_be32(&sdram->cr41, 0x00C80064);
	out_be32(&sdram->cr42, 0x44520002);
	out_be32(&sdram->cr43, 0x00C80023);
	out_be32(&sdram->cr45, 0x0000C350);
	out_be32(&sdram->cr56, 0x04000000);
	out_be32(&sdram->cr57, 0x03000304);
	out_be32(&sdram->cr58, 0x40040000);
	out_be32(&sdram->cr59, 0xC0004004);
	out_be32(&sdram->cr60, 0x0642C000);
	out_be32(&sdram->cr61, 0x00000642);
	asm("tpf");

	out_be32(&sdram->cr09, 0x01000100);

	udelay(100);
#endif
	gd->ram_size = dramsize;

	return 0;
};

int testdram(void)
{
	return 0;
}
