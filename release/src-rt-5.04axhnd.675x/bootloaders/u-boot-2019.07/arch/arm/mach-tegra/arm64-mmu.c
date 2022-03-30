// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2014 - 2015 Xilinx, Inc.
 * Michal Simek <michal.simek@xilinx.com>
 * (This file derived from arch/arm/mach-zynqmp/cpu.c)
 *
 * Copyright (c) 2015, NVIDIA CORPORATION. All rights reserved.
 */

#include <common.h>
#include <asm/system.h>
#include <asm/armv8/mmu.h>

/* size: IO + NR_DRAM_BANKS + terminator */
struct mm_region tegra_mem_map[1 + CONFIG_NR_DRAM_BANKS + 1] = {
	{
		.virt = 0x0UL,
		.phys = 0x0UL,
		.size = 0x80000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	}, {
		.virt = 0x80000000UL,
		.phys = 0x80000000UL,
		.size = 0x80000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
			 PTE_BLOCK_INNER_SHARE
	}, {
		/* List terminator */
		0,
	}
};

struct mm_region *mem_map = tegra_mem_map;
