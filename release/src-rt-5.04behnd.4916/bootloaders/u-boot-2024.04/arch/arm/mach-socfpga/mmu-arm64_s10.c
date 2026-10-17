// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2016-2018 Intel Corporation <www.intel.com>
 *
 */

#include <common.h>
#include <asm/armv8/mmu.h>
#include <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

static struct mm_region socfpga_stratix10_mem_map[] = {
	{
		/* MEM 2GB*/
		.virt	= 0x0UL,
		.phys	= 0x0UL,
		.size	= 0x80000000UL,
		.attrs	= PTE_BLOCK_MEMTYPE(MT_NORMAL) |
				PTE_BLOCK_INNER_SHARE,
	}, {
		/* FPGA 1.5GB */
		.virt	= 0x80000000UL,
		.phys	= 0x80000000UL,
		.size	= 0x60000000UL,
		.attrs	= PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
				PTE_BLOCK_NON_SHARE |
				PTE_BLOCK_PXN | PTE_BLOCK_UXN,
	}, {
		/* DEVICE 142MB */
		.virt	= 0xF7000000UL,
		.phys	= 0xF7000000UL,
		.size	= 0x08E00000UL,
		.attrs	= PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
				PTE_BLOCK_NON_SHARE |
				PTE_BLOCK_PXN | PTE_BLOCK_UXN,
	}, {
		/* OCRAM 1MB but available 256KB */
		.virt	= 0xFFE00000UL,
		.phys	= 0xFFE00000UL,
		.size	= 0x00100000UL,
		.attrs	= PTE_BLOCK_MEMTYPE(MT_NORMAL) |
				PTE_BLOCK_INNER_SHARE,
	}, {
		/* DEVICE 32KB */
		.virt	= 0xFFFC0000UL,
		.phys	= 0xFFFC0000UL,
		.size	= 0x00008000UL,
		.attrs	= PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
				PTE_BLOCK_NON_SHARE |
				PTE_BLOCK_PXN | PTE_BLOCK_UXN,
	}, {
		/* MEM 124GB */
		.virt	= 0x0100000000UL,
		.phys	= 0x0100000000UL,
		.size	= 0x1F00000000UL,
		.attrs	= PTE_BLOCK_MEMTYPE(MT_NORMAL) |
				PTE_BLOCK_INNER_SHARE,
	}, {
		/* DEVICE 4GB */
		.virt	= 0x2000000000UL,
		.phys	= 0x2000000000UL,
		.size	= 0x0100000000UL,
		.attrs	= PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
				PTE_BLOCK_NON_SHARE |
				PTE_BLOCK_PXN | PTE_BLOCK_UXN,
	}, {
		/* List terminator */
	},
};

struct mm_region *mem_map = socfpga_stratix10_mem_map;
