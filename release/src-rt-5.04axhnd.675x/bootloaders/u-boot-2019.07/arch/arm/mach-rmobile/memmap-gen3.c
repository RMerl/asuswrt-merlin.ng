// SPDX-License-Identifier: GPL-2.0+
/*
 * Renesas RCar Gen3 memory map tables
 *
 * Copyright (C) 2017 Marek Vasut <marek.vasut@gmail.com>
 */

#include <common.h>
#include <asm/armv8/mmu.h>

#define GEN3_NR_REGIONS 16

static struct mm_region gen3_mem_map[GEN3_NR_REGIONS] = {
	{
		.virt = 0x0UL,
		.phys = 0x0UL,
		.size = 0x40000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	}, {
		.virt = 0x40000000UL,
		.phys = 0x40000000UL,
		.size = 0x03F00000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
			 PTE_BLOCK_INNER_SHARE
	}, {
		.virt = 0x47E00000UL,
		.phys = 0x47E00000UL,
		.size = 0x78200000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
			 PTE_BLOCK_INNER_SHARE
	}, {
		.virt = 0xc0000000UL,
		.phys = 0xc0000000UL,
		.size = 0x40000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	}, {
		.virt = 0x100000000UL,
		.phys = 0x100000000UL,
		.size = 0xf00000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
			 PTE_BLOCK_INNER_SHARE
	}, {
		/* List terminator */
		0,
	}
};

struct mm_region *mem_map = gen3_mem_map;

DECLARE_GLOBAL_DATA_PTR;

void enable_caches(void)
{
	u64 start, size;
	int bank, i = 0;

	/* Create map for RPC access */
	gen3_mem_map[i].virt = 0x0ULL;
	gen3_mem_map[i].phys = 0x0ULL;
	gen3_mem_map[i].size = 0x40000000ULL;
	gen3_mem_map[i].attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
				PTE_BLOCK_NON_SHARE |
				PTE_BLOCK_PXN | PTE_BLOCK_UXN;
	i++;

	/* Generate entires for DRAM in 32bit address space */
	for (bank = 0; bank < CONFIG_NR_DRAM_BANKS; bank++) {
		start = gd->bd->bi_dram[bank].start;
		size = gd->bd->bi_dram[bank].size;

		/* Skip empty DRAM banks */
		if (!size)
			continue;

		/* Skip DRAM above 4 GiB */
		if (start >> 32ULL)
			continue;

		/* Mark memory reserved by ATF as cacheable too. */
		if (start == 0x48000000) {
			/* Unmark protection area (0x43F00000 to 0x47DFFFFF) */
			gen3_mem_map[i].virt = 0x40000000ULL;
			gen3_mem_map[i].phys = 0x40000000ULL;
			gen3_mem_map[i].size = 0x03F00000ULL;
			gen3_mem_map[i].attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
						PTE_BLOCK_INNER_SHARE;
			i++;

			start = 0x47E00000ULL;
			size += 0x00200000ULL;
		}

		gen3_mem_map[i].virt = start;
		gen3_mem_map[i].phys = start;
		gen3_mem_map[i].size = size;
		gen3_mem_map[i].attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
					PTE_BLOCK_INNER_SHARE;
		i++;
	}

	/* Create map for register access */
	gen3_mem_map[i].virt = 0xc0000000ULL;
	gen3_mem_map[i].phys = 0xc0000000ULL;
	gen3_mem_map[i].size = 0x40000000ULL;
	gen3_mem_map[i].attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
				PTE_BLOCK_NON_SHARE |
				PTE_BLOCK_PXN | PTE_BLOCK_UXN;
	i++;

	/* Generate entires for DRAM in 64bit address space */
	for (bank = 0; bank < CONFIG_NR_DRAM_BANKS; bank++) {
		start = gd->bd->bi_dram[bank].start;
		size = gd->bd->bi_dram[bank].size;

		/* Skip empty DRAM banks */
		if (!size)
			continue;

		/* Skip DRAM below 4 GiB */
		if (!(start >> 32ULL))
			continue;

		gen3_mem_map[i].virt = start;
		gen3_mem_map[i].phys = start;
		gen3_mem_map[i].size = size;
		gen3_mem_map[i].attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
					PTE_BLOCK_INNER_SHARE;
		i++;
	}

	/* Zero out the remaining regions. */
	for (; i < GEN3_NR_REGIONS; i++) {
		gen3_mem_map[i].virt = 0;
		gen3_mem_map[i].phys = 0;
		gen3_mem_map[i].size = 0;
		gen3_mem_map[i].attrs = 0;
	}

	if (!icache_status())
		icache_enable();

	dcache_enable();
}
