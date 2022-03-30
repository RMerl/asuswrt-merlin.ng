// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016 Masahiro Yamada <yamada.masahiro@socionext.com>
 */

#include <common.h>
#include <linux/types.h>
#include <asm/armv8/mmu.h>

static struct mm_region uniphier_mem_map[] = {
	{
		.virt = 0x00000000,
		.phys = 0x00000000,
		.size = 0x80000000,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	},
	{
		.virt = 0x80000000,
		.phys = 0x80000000,
		.size = 0xc0000000,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
			 PTE_BLOCK_INNER_SHARE
	},
	{ /* sentinel */ }
};

struct mm_region *mem_map = uniphier_mem_map;
