// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015, Bin Meng <bmeng.cn@gmail.com>
 */

#include <common.h>
#include <asm/post.h>
#include <asm/arch/qemu.h>

DECLARE_GLOBAL_DATA_PTR;

int dram_init(void)
{
	u32 ram;

	outb(HIGH_RAM_ADDR, CMOS_ADDR_PORT);
	ram = ((u32)inb(CMOS_DATA_PORT)) << 14;
	outb(LOW_RAM_ADDR, CMOS_ADDR_PORT);
	ram |= ((u32)inb(CMOS_DATA_PORT)) << 6;
	ram += 16 * 1024;

	gd->ram_size = ram * 1024;
	post_code(POST_DRAM);

	return 0;
}

int dram_init_banksize(void)
{
	gd->bd->bi_dram[0].start = 0;
	gd->bd->bi_dram[0].size = gd->ram_size;

	return 0;
}

/*
 * This function looks for the highest region of memory lower than 4GB which
 * has enough space for U-Boot where U-Boot is aligned on a page boundary.
 * It overrides the default implementation found elsewhere which simply
 * picks the end of ram, wherever that may be. The location of the stack,
 * the relocation address, and how far U-Boot is moved by relocation are
 * set in the global data structure.
 */
ulong board_get_usable_ram_top(ulong total_size)
{
	return gd->ram_size;
}
