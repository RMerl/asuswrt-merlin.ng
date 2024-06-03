// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2016 Rockchip Electronics Co., Ltd
 */

#include <common.h>
#include <asm/arch-rockchip/hardware.h>
#include <asm/armv8/mmu.h>
#include <asm/io.h>

DECLARE_GLOBAL_DATA_PTR;

static struct mm_region rk3328_mem_map[] = {
	{
		.virt = 0x0UL,
		.phys = 0x0UL,
		.size = 0xff000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
			 PTE_BLOCK_INNER_SHARE
	}, {
		.virt = 0xff000000UL,
		.phys = 0xff000000UL,
		.size = 0x1000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	}, {
		/* List terminator */
		0,
	}
};

struct mm_region *mem_map = rk3328_mem_map;

int dram_init_banksize(void)
{
	size_t max_size = min((unsigned long)gd->ram_size, gd->ram_top);

	/* Reserve 0x200000 for ATF bl31 */
	gd->bd->bi_dram[0].start = 0x200000;
	gd->bd->bi_dram[0].size = max_size - gd->bd->bi_dram[0].start;

	return 0;
}

int arch_cpu_init(void)
{
	/* We do some SoC one time setting here. */

	return 0;
}
