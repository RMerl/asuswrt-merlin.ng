// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015, Bin Meng <bmeng.cn@gmail.com>
 */

#include <common.h>
#include <init.h>
#include <asm/global_data.h>
#include <asm/post.h>
#include <asm/arch/qemu.h>
#include <linux/sizes.h>

DECLARE_GLOBAL_DATA_PTR;

u32 qemu_get_low_memory_size(void)
{
	u32 ram;

	outb(HIGH_RAM_ADDR, CMOS_ADDR_PORT);
	ram = ((u32)inb(CMOS_DATA_PORT)) << 14;
	outb(LOW_RAM_ADDR, CMOS_ADDR_PORT);
	ram |= ((u32)inb(CMOS_DATA_PORT)) << 6;
	ram += 16 * 1024;

	return ram * 1024;
}

u64 qemu_get_high_memory_size(void)
{
	u64 ram;

	outb(HIGH_HIGHRAM_ADDR, CMOS_ADDR_PORT);
	ram = ((u64)inb(CMOS_DATA_PORT)) << 22;
	outb(MID_HIGHRAM_ADDR, CMOS_ADDR_PORT);
	ram |= ((u64)inb(CMOS_DATA_PORT)) << 14;
	outb(LOW_HIGHRAM_ADDR, CMOS_ADDR_PORT);
	ram |= ((u64)inb(CMOS_DATA_PORT)) << 6;

	return ram * 1024;
}

int dram_init(void)
{
	gd->ram_size = qemu_get_low_memory_size();
	gd->ram_size += qemu_get_high_memory_size();
	post_code(POST_DRAM);

	return 0;
}

int dram_init_banksize(void)
{
	u64 high_mem_size;

	gd->bd->bi_dram[0].start = 0;
	gd->bd->bi_dram[0].size = qemu_get_low_memory_size();

	high_mem_size = qemu_get_high_memory_size();
	if (high_mem_size) {
		gd->bd->bi_dram[1].start = SZ_4G;
		gd->bd->bi_dram[1].size = high_mem_size;
	}

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
phys_addr_t board_get_usable_ram_top(phys_size_t total_size)
{
	return qemu_get_low_memory_size();
}
