// SPDX-License-Identifier: GPL-2.0+
/*
 * Hisilicon HiSTB memory map
 *
 * (C) Copyright 2023 Yang Xiwen <forbidden405@outlook.com>
 */

#include <common.h>
#include <asm/armv8/mmu.h>

static struct mm_region histb_mem_map[] = {
	{
		.virt = 0x0UL, /* DRAM */
		.phys = 0x0UL,
		.size = 0x80000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
			 PTE_BLOCK_INNER_SHARE
	}, {
		.virt = 0x80000000UL, /* Peripheral block */
		.phys = 0x80000000UL,
		.size = 0x80000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	}, {
		/* Terminator */
		0,
	}
};

struct mm_region *mem_map = histb_mem_map;
