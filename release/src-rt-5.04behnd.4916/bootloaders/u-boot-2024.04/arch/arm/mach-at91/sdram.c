// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2014
 * Heiko Schocher, DENX Software Engineering, hs@denx.de.
 *
 * Based on:
 * (C) Copyright 2007-2008
 * Stelian Pop <stelian@popies.net>
 * Lead Tech Design <www.leadtechdesign.com>
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/at91_common.h>
#include <asm/arch/at91sam9_sdramc.h>
#include <asm/arch/gpio.h>

int sdramc_initialize(unsigned int sdram_address, const struct sdramc_reg *p)
{
	struct sdramc_reg *reg = (struct sdramc_reg *)ATMEL_BASE_SDRAMC;
	unsigned int i;

	/* SDRAM feature must be in the configuration register */
	writel(p->cr, &reg->cr);

	/* The SDRAM memory type must be set in the Memory Device Register */
	writel(p->mdr, &reg->mdr);

	/*
	 * The minimum pause of 200 us is provided to precede any single
	 * toggle
	 */
	for (i = 0; i < 1000; i++)
		;

	/* A NOP command is issued to the SDRAM devices */
	writel(AT91_SDRAMC_MODE_NOP, &reg->mr);
	writel(0x00000000, sdram_address);

	/* An All Banks Precharge command is issued to the SDRAM devices */
	writel(AT91_SDRAMC_MODE_PRECHARGE, &reg->mr);
	writel(0x00000000, sdram_address);

	for (i = 0; i < 10000; i++)
		;

	/* Eight auto-refresh cycles are provided */
	for (i = 0; i < 8; i++) {
		writel(AT91_SDRAMC_MODE_REFRESH, &reg->mr);
		writel(0x00000001 + i, sdram_address + 4 + 4 * i);
	}

	/*
	 * A Mode Register set (MRS) cyscle is issued to program the
	 * SDRAM parameters(TCSR, PASR, DS)
	 */
	writel(AT91_SDRAMC_MODE_LMR, &reg->mr);
	writel(0xcafedede, sdram_address + 0x24);

	/*
	 * The application must go into Normal Mode, setting Mode
	 * to 0 in the Mode Register and perform a write access at
	 * any location in the SDRAM.
	 */
	writel(AT91_SDRAMC_MODE_NORMAL, &reg->mr);
	writel(0x00000000, sdram_address);	/* Perform Normal mode */

	/*
	 * Write the refresh rate into the count field in the SDRAMC
	 * Refresh Timer Rgister.
	 */
	writel(p->tr, &reg->tr);

	return 0;
}
