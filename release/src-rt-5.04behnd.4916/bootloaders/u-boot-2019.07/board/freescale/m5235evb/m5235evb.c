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
#include <asm/immap.h>
#include <asm/io.h>

DECLARE_GLOBAL_DATA_PTR;

int checkboard(void)
{
	puts("Board: ");
	puts("Freescale M5235 EVB\n");
	return 0;
};

int dram_init(void)
{
	sdram_t *sdram = (sdram_t *)(MMAP_SDRAM);
	gpio_t *gpio = (gpio_t *)(MMAP_GPIO);
	u32 dramsize, i, dramclk;

	/*
	 * When booting from external Flash, the port-size is less than
	 * the port-size of SDRAM.  In this case it is necessary to enable
	 * Data[15:0] on Port Address/Data.
	 */
	out_8(&gpio->par_ad,
		GPIO_PAR_AD_ADDR23 | GPIO_PAR_AD_ADDR22 | GPIO_PAR_AD_ADDR21 |
		GPIO_PAR_AD_DATAL);

	/* Initialize PAR to enable SDRAM signals */
	out_8(&gpio->par_sdram,
		GPIO_PAR_SDRAM_SDWE | GPIO_PAR_SDRAM_SCAS |
		GPIO_PAR_SDRAM_SRAS | GPIO_PAR_SDRAM_SCKE |
		GPIO_PAR_SDRAM_SDCS(3));

	dramsize = CONFIG_SYS_SDRAM_SIZE * 0x100000;
	for (i = 0x13; i < 0x20; i++) {
		if (dramsize == (1 << i))
			break;
	}
	i--;

	if (!(in_be32(&sdram->dacr0) & SDRAMC_DARCn_RE)) {
		dramclk = gd->bus_clk / (CONFIG_SYS_HZ * CONFIG_SYS_HZ);

		/* Initialize DRAM Control Register: DCR */
		out_be16(&sdram->dcr, SDRAMC_DCR_RTIM_9CLKS |
			SDRAMC_DCR_RTIM_6CLKS |
			SDRAMC_DCR_RC((15 * dramclk) >> 4));

		/* Initialize DACR0 */
		out_be32(&sdram->dacr0,
			SDRAMC_DARCn_BA(CONFIG_SYS_SDRAM_BASE) |
			SDRAMC_DARCn_CASL_C1 | SDRAMC_DARCn_CBM_CMD20 |
			SDRAMC_DARCn_PS_32);
		asm("nop");

		/* Initialize DMR0 */
		out_be32(&sdram->dmr0,
			((dramsize - 1) & 0xFFFC0000) | SDRAMC_DMRn_V);
		asm("nop");

		/* Set IP (bit 3) in DACR */
		setbits_be32(&sdram->dacr0, SDRAMC_DARCn_IP);

		/* Wait 30ns to allow banks to precharge */
		for (i = 0; i < 5; i++) {
			asm("nop");
		}

		/* Write to this block to initiate precharge */
		*(u32 *) (CONFIG_SYS_SDRAM_BASE) = 0xA5A59696;

		/*  Set RE (bit 15) in DACR */
		setbits_be32(&sdram->dacr0, SDRAMC_DARCn_RE);

		/* Wait for at least 8 auto refresh cycles to occur */
		for (i = 0; i < 0x2000; i++) {
			asm("nop");
		}

		/* Finish the configuration by issuing the MRS. */
		setbits_be32(&sdram->dacr0, SDRAMC_DARCn_IMRS);
		asm("nop");

		/* Write to the SDRAM Mode Register */
		*(u32 *) (CONFIG_SYS_SDRAM_BASE + 0x400) = 0xA5A59696;
	}

	gd->ram_size = dramsize;

	return 0;
};

int testdram(void)
{
	/* TODO: XXX XXX XXX */
	printf("DRAM test not implemented!\n");

	return (0);
}
