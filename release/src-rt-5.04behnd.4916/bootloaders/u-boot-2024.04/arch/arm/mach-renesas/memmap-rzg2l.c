// SPDX-License-Identifier: GPL-2.0+
/*
 * Renesas RZ/G2L family memory map tables
 *
 * Copyright (C) 2017 Marek Vasut <marek.vasut@gmail.com>
 * Copyright (C) 2023 Renesas Electronics Corp.
 */

#include <asm/armv8/mmu.h>
#include <asm/global_data.h>
#include <asm/u-boot.h>
#include <cpu_func.h>

#define RZG2L_NR_REGIONS 16

/*
 * RZ/G2L supports up to 4 GiB RAM starting at 0x40000000, of
 * which the first 128 MiB is reserved by TF-A.
 */
static struct mm_region rzg2l_mem_map[RZG2L_NR_REGIONS] = {
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
		.size = 0xF8200000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
			 PTE_BLOCK_INNER_SHARE
	}, {
		/* List terminator */
		0,
	}
};

struct mm_region *mem_map = rzg2l_mem_map;

DECLARE_GLOBAL_DATA_PTR;

#define debug_memmap(i, map) \
	debug("memmap %d: virt 0x%llx -> phys 0x%llx, size=0x%llx, attrs=0x%llx\n", \
	      i, map[i].virt, map[i].phys, map[i].size, map[i].attrs)

void enable_caches(void)
{
	unsigned int bank, i = 0;
	u64 start, size;

	/* Create map for register access */
	rzg2l_mem_map[i].virt = 0x0ULL;
	rzg2l_mem_map[i].phys = 0x0ULL;
	rzg2l_mem_map[i].size = 0x40000000ULL;
	rzg2l_mem_map[i].attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
				 PTE_BLOCK_NON_SHARE |
				 PTE_BLOCK_PXN | PTE_BLOCK_UXN;
	debug_memmap(i, rzg2l_mem_map);
	i++;

	/* Generate entries for DRAM in 32bit address space */
	for (bank = 0; bank < CONFIG_NR_DRAM_BANKS; bank++) {
		start = gd->bd->bi_dram[bank].start;
		size = gd->bd->bi_dram[bank].size;

		/* Skip empty DRAM banks */
		if (!size)
			continue;

		/* Mark memory reserved by ATF as cacheable too. */
		if (start == 0x48000000) {
			/* Unmark protection area (0x43F00000 to 0x47DFFFFF) */
			rzg2l_mem_map[i].virt = 0x40000000ULL;
			rzg2l_mem_map[i].phys = 0x40000000ULL;
			rzg2l_mem_map[i].size = 0x03F00000ULL;
			rzg2l_mem_map[i].attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
						 PTE_BLOCK_INNER_SHARE;
			debug_memmap(i, rzg2l_mem_map);
			i++;

			start = 0x47E00000ULL;
			size += 0x00200000ULL;
		}

		rzg2l_mem_map[i].virt = start;
		rzg2l_mem_map[i].phys = start;
		rzg2l_mem_map[i].size = size;
		rzg2l_mem_map[i].attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
					 PTE_BLOCK_INNER_SHARE;
		debug_memmap(i, rzg2l_mem_map);
		i++;
	}

	/* Zero out the remaining regions. */
	for (; i < RZG2L_NR_REGIONS; i++) {
		rzg2l_mem_map[i].virt = 0;
		rzg2l_mem_map[i].phys = 0;
		rzg2l_mem_map[i].size = 0;
		rzg2l_mem_map[i].attrs = 0;
		debug_memmap(i, rzg2l_mem_map);
	}

	if (!icache_status())
		icache_enable();

	dcache_enable();
}

int dram_init(void)
{
	return fdtdec_setup_mem_size_base();
}

int dram_init_banksize(void)
{
	fdtdec_setup_memory_banksize();

	return 0;
}
