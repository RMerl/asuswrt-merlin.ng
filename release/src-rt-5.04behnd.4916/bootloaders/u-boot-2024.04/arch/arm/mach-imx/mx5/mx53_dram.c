// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017  Beckhoff Automation GmbH & Co. KG
 * Patrick Bruenn <p.bruenn@beckhoff.com>
 */

#include <common.h>
#include <init.h>
#include <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

phys_size_t get_effective_memsize(void)
{
	/*
	 * WARNING: We must override get_effective_memsize() function here
	 * to report only the size of the first DRAM bank. This is to make
	 * U-Boot relocator place U-Boot into valid memory, that is, at the
	 * end of the first DRAM bank. If we did not override this function
	 * like so, U-Boot would be placed at the address of the first DRAM
	 * bank + total DRAM size - sizeof(uboot), which in the setup where
	 * each DRAM bank contains 512MiB of DRAM would result in placing
	 * U-Boot into invalid memory area close to the end of the first
	 * DRAM bank.
	 */
	return get_ram_size((void *)PHYS_SDRAM_1, 1 << 30);
}

int dram_init(void)
{
	gd->ram_size = get_ram_size((void *)PHYS_SDRAM_1, 1 << 30);
	gd->ram_size += get_ram_size((void *)PHYS_SDRAM_2, 1 << 30);

	return 0;
}

int dram_init_banksize(void)
{
	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[0].size = get_ram_size((void *)PHYS_SDRAM_1, 1 << 30);

	gd->bd->bi_dram[1].start = PHYS_SDRAM_2;
	gd->bd->bi_dram[1].size = get_ram_size((void *)PHYS_SDRAM_2, 1 << 30);

	return 0;
}
