// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018 Philippe Reynes <philippe.reynes@softathome.com>
 */

#include <common.h>
#include <fdtdec.h>
#include <linux/io.h>

#ifdef CONFIG_ARM64
#include <asm/armv8/mmu.h>

static struct mm_region broadcom_bcm968580xref_mem_map[] = {
	{
		/* RAM */
		.virt = 0x00000000UL,
		.phys = 0x00000000UL,
		.size = 8UL * SZ_1G,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
			 PTE_BLOCK_INNER_SHARE
	}, {
		/* SoC */
		.virt = 0x80000000UL,
		.phys = 0x80000000UL,
		.size = 0xff80000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	}, {
		/* List terminator */
		0,
	}
};

struct mm_region *mem_map = broadcom_bcm968580xref_mem_map;
#endif

int board_init(void)
{
	return 0;
}

int dram_init(void)
{
	if (fdtdec_setup_mem_size_base() != 0)
		printf("fdtdec_setup_mem_size_base() has failed\n");

	return 0;
}

int dram_init_banksize(void)
{
	fdtdec_setup_memory_banksize();

	return 0;
}

int print_cpuinfo(void)
{
	return 0;
}
