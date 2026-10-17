// SPDX-License-Identifier: GPL-2.0+
/*
 * Memory map for Actions Semi Owl series SoCs.
 *
 * Copyright (C) 2015 Actions Semi Co., Ltd.
 * Copyright (C) 2018 Manivannan Sadhasivam <manivannan.sadhasivam@linaro.org>
 */

#include <common.h>
#include <asm/armv8/mmu.h>

static struct mm_region owl_mem_map[] = {
	{
		.virt = 0x0UL, /* DDR */
		.phys = 0x0UL, /* DDR */
		.size = 0x80000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
			 PTE_BLOCK_INNER_SHARE
	}, {
		.virt = 0xE0000000UL, /* Peripheral block */
		.phys = 0xE0000000UL, /* Peripheral block */
		.size = 0x08000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	}, {
		/* List terminator */
		0,
	}
};

struct mm_region *mem_map = owl_mem_map;
