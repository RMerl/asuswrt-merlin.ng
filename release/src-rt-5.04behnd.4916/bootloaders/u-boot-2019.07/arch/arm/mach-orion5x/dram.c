// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2010 Albert ARIBAUD <albert.u.boot@aribaud.net>
 *
 * Based on original Kirkwood support which is
 * (C) Copyright 2009
 * Marvell Semiconductor <www.marvell.com>
 * Written-by: Prafulla Wadaskar <prafulla@marvell.com>
 */

#include <common.h>
#include <config.h>
#include <asm/arch/cpu.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * orion5x_sdram_bar - reads SDRAM Base Address Register
 */
u32 orion5x_sdram_bar(enum memory_bank bank)
{
	struct orion5x_ddr_addr_decode_registers *winregs =
		(struct orion5x_ddr_addr_decode_registers *)
		ORION5X_DRAM_BASE;

	u32 result = 0;
	u32 enable = 0x01 & winregs[bank].size;

	if ((!enable) || (bank > BANK3))
		return 0;

	result = winregs[bank].base;
	return result;
}
int dram_init (void)
{
	/* dram_init must store complete ramsize in gd->ram_size */
	gd->ram_size = get_ram_size(
			(long *) orion5x_sdram_bar(0),
			CONFIG_MAX_RAM_BANK_SIZE);
	return 0;
}

int dram_init_banksize(void)
{
	int i;

	for (i = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
		gd->bd->bi_dram[i].start = orion5x_sdram_bar(i);
		gd->bd->bi_dram[i].size = get_ram_size(
			(long *) (gd->bd->bi_dram[i].start),
			CONFIG_MAX_RAM_BANK_SIZE);
	}

	return 0;
}
