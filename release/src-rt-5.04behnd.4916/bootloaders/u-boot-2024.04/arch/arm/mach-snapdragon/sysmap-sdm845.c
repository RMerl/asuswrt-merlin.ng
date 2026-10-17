// SPDX-License-Identifier: GPL-2.0+
/*
 * Qualcomm SDM845 memory map
 *
 * (C) Copyright 2021 Dzmitry Sankouski <dsankousk@gmail.com>
 */

#include <common.h>
#include <asm/armv8/mmu.h>

static struct mm_region sdm845_mem_map[] = {
	{
		.virt = 0x0UL, /* Peripheral block */
		.phys = 0x0UL, /* Peripheral block */
		.size = 0x10000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	}, {
		.virt = 0x80000000UL, /* DDR */
		.phys = 0x80000000UL, /* DDR */
		.size = 0x200000000UL, /* 8GiB - maximum allowed memory */
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
			 PTE_BLOCK_INNER_SHARE
	}, {
		/* List terminator */
		0,
	}
};

struct mm_region *mem_map = sdm845_mem_map;
