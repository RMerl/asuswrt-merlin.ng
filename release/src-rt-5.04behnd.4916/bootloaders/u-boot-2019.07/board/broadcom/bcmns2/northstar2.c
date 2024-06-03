// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2016 Broadcom Ltd.
 */
#include <common.h>
#include <asm/system.h>
#include <asm/armv8/mmu.h>

static struct mm_region ns2_mem_map[] = {
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
		.size = 0xff80000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
			 PTE_BLOCK_INNER_SHARE
	}, {
		/* List terminator */
		0,
	}
};

struct mm_region *mem_map = ns2_mem_map;

DECLARE_GLOBAL_DATA_PTR;

int board_init(void)
{
	return 0;
}

int dram_init(void)
{
	gd->ram_size = get_ram_size((long *)CONFIG_SYS_SDRAM_BASE,
				    PHYS_SDRAM_1_SIZE + PHYS_SDRAM_2_SIZE);
	return 0;
}

int dram_init_banksize(void)
{
	gd->bd->bi_dram[0].start = CONFIG_SYS_SDRAM_BASE;
	gd->bd->bi_dram[0].size = PHYS_SDRAM_1_SIZE;

	gd->bd->bi_dram[1].start = CONFIG_SYS_SDRAM_BASE + PHYS_SDRAM_1_SIZE;
	gd->bd->bi_dram[1].size = PHYS_SDRAM_2_SIZE;

	return 0;
}

void reset_cpu(ulong addr)
{
	psci_system_reset();
}
