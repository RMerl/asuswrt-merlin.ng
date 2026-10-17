// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016 Samsung Electronics
 * Thomas Abraham <thomas.ab@samsung.com>
 */

#include <common.h>
#include <asm/armv8/mmu.h>
#include <linux/sizes.h>

#if IS_ENABLED(CONFIG_EXYNOS7420)

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

#elif CONFIG_IS_ENABLED(EXYNOS7870)

static struct mm_region exynos7870_mem_map[] = {
	{
		.virt	= 0x10000000UL,
		.phys	= 0x10000000UL,
		.size	= 0x10000000UL,
		.attrs	= PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
				PTE_BLOCK_NON_SHARE |
				PTE_BLOCK_PXN | PTE_BLOCK_UXN,
	},
	{
		.virt	= 0x40000000UL,
		.phys	= 0x40000000UL,
		.size	= 0x3E400000UL,
		.attrs	= PTE_BLOCK_MEMTYPE(MT_NORMAL) |
				PTE_BLOCK_INNER_SHARE,
	},
	{
		.virt	= 0x80000000UL,
		.phys	= 0x80000000UL,
		.size	= 0x40000000UL,
		.attrs	= PTE_BLOCK_MEMTYPE(MT_NORMAL) |
				PTE_BLOCK_INNER_SHARE,
	},

	{
		/* List terminator */
	},
};

struct mm_region *mem_map = exynos7870_mem_map;

#elif CONFIG_IS_ENABLED(EXYNOS7880)

static struct mm_region exynos7880_mem_map[] = {
	{
		.virt	= 0x10000000UL,
		.phys	= 0x10000000UL,
		.size	= 0x10000000UL,
		.attrs	= PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
				PTE_BLOCK_NON_SHARE |
				PTE_BLOCK_PXN | PTE_BLOCK_UXN,
	},
	{
		.virt	= 0x40000000UL,
		.phys	= 0x40000000UL,
		.size	= 0x3E400000UL,
		.attrs	= PTE_BLOCK_MEMTYPE(MT_NORMAL) |
				PTE_BLOCK_INNER_SHARE,
	},
	{
		.virt	= 0x80000000UL,
		.phys	= 0x80000000UL,
		.size	= 0x80000000UL,
		.attrs	= PTE_BLOCK_MEMTYPE(MT_NORMAL) |
				PTE_BLOCK_INNER_SHARE,
	},

	{
		/* List terminator */
	},
};

struct mm_region *mem_map = exynos7880_mem_map;

#elif IS_ENABLED(CONFIG_EXYNOS850)

static struct mm_region exynos850_mem_map[] = {
	{
		/* Peripheral block */
		.virt = 0x10000000UL,
		.phys = 0x10000000UL,
		.size = SZ_256M,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	}, {
		/* DDR, 32-bit area */
		.virt = 0x80000000UL,
		.phys = 0x80000000UL,
		.size = SZ_2G,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
			 PTE_BLOCK_INNER_SHARE
	}, {
		/* DDR, 64-bit area */
		.virt = 0x880000000UL,
		.phys = 0x880000000UL,
		.size = SZ_2G,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
			 PTE_BLOCK_INNER_SHARE
	}, {
		/* List terminator */
	}
};

struct mm_region *mem_map = exynos850_mem_map;

#endif
