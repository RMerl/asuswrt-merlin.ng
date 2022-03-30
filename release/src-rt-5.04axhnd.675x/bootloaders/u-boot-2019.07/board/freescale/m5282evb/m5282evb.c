// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000-2003
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#include <common.h>
#include <asm/immap.h>

DECLARE_GLOBAL_DATA_PTR;

int checkboard (void)
{
	puts ("Board: Freescale M5282EVB Evaluation Board\n");
	return 0;
}

int dram_init(void)
{
	u32 dramsize, i, dramclk;

	dramsize = CONFIG_SYS_SDRAM_SIZE * 0x100000;
	for (i = 0x13; i < 0x20; i++) {
		if (dramsize == (1 << i))
			break;
	}
	i--;

	if (!(MCFSDRAMC_DACR0 & MCFSDRAMC_DACR_RE))
	{
		dramclk = gd->bus_clk / (CONFIG_SYS_HZ * CONFIG_SYS_HZ);

		/* Initialize DRAM Control Register: DCR */
		MCFSDRAMC_DCR = (0
			| MCFSDRAMC_DCR_RTIM_6
			| MCFSDRAMC_DCR_RC((15 * dramclk)>>4));
		asm("nop");

		/* Initialize DACR0 */
		MCFSDRAMC_DACR0 = (0
			| MCFSDRAMC_DACR_BASE(CONFIG_SYS_SDRAM_BASE)
			| MCFSDRAMC_DACR_CASL(1)
			| MCFSDRAMC_DACR_CBM(3)
			| MCFSDRAMC_DACR_PS_32);
		asm("nop");

		/* Initialize DMR0 */
		MCFSDRAMC_DMR0 = (0
			| ((dramsize - 1) & 0xFFFC0000)
			| MCFSDRAMC_DMR_V);
		asm("nop");

		/* Set IP (bit 3) in DACR */
		MCFSDRAMC_DACR0 |= MCFSDRAMC_DACR_IP;
		asm("nop");

		/* Wait 30ns to allow banks to precharge */
		for (i = 0; i < 5; i++) {
			asm ("nop");
		}

		/* Write to this block to initiate precharge */
		*(u32 *)(CONFIG_SYS_SDRAM_BASE) = 0xA5A59696;
		asm("nop");

		/* Set RE (bit 15) in DACR */
		MCFSDRAMC_DACR0 |= MCFSDRAMC_DACR_RE;
		asm("nop");

		/* Wait for at least 8 auto refresh cycles to occur */
		for (i = 0; i < 2000; i++) {
			asm(" nop");
		}

		/* Finish the configuration by issuing the IMRS. */
		MCFSDRAMC_DACR0 |= MCFSDRAMC_DACR_IMRS;
		asm("nop");

		/* Write to the SDRAM Mode Register */
		*(u32 *)(CONFIG_SYS_SDRAM_BASE + 0x400) = 0xA5A59696;
	}
	gd->ram_size = dramsize;

	return 0;
}
