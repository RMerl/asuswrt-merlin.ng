// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016 Samsung Electronics
 * Thomas Abraham <thomas.ab@samsung.com>
 */

#include <common.h>
#include <asm/armv8/mmu.h>

#ifdef CONFIG_EXYNOS7420
static struct mm_region exynos7420_mem_map[] = {
	{
		.virt	= 0x10000000UL,
		.phys	= 0x10000000UL,
		.size	= 0x10000000UL,
		.attrs	= PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
				PTE_BLOCK_NON_SHARE |
				PTE_BLOCK_PXN | PTE_BLOCK_UXN,
	}, {
		.virt	= 0x40000000UL,
		.phys	= 0x40000000UL,
		.size	= 0x80000000UL,
		.attrs	= PTE_BLOCK_MEMTYPE(MT_NORMAL) |
				PTE_BLOCK_INNER_SHARE,
	}, {
		/* List terminator */
	},
};

struct mm_region *mem_map = exynos7420_mem_map;
#endif
