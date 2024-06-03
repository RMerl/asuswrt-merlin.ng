// SPDX-License-Identifier: GPL-2.0+
/*
 * Qualcomm APQ8016 memory map
 *
 * (C) Copyright 2016 Mateusz Kulikowski <mateusz.kulikowski@gmail.com>
 */

#include <common.h>
#include <asm/armv8/mmu.h>

static struct mm_region apq8016_mem_map[] = {
	{
		.virt = 0x0UL, /* Peripheral block */
		.phys = 0x0UL, /* Peripheral block */
		.size = 0x8000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	}, {
		.virt = 0x80000000UL, /* DDR */
		.phys = 0x80000000UL, /* DDR */
		.size = 0x80000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
			 PTE_BLOCK_INNER_SHARE
	}, {
		/* List terminator */
		0,
	}
};

struct mm_region *mem_map = apq8016_mem_map;
