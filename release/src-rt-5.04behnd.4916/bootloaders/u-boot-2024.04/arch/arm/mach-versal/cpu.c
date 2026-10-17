// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2016 - 2018 Xilinx, Inc.
 * Michal Simek <michal.simek@amd.com>
 */

#include <common.h>
#include <init.h>
#include <asm/armv8/mmu.h>
#include <asm/cache.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/arch/hardware.h>
#include <asm/arch/sys_proto.h>
#include <asm/cache.h>
#include <dm/platdata.h>

DECLARE_GLOBAL_DATA_PTR;

#define VERSAL_MEM_MAP_USED	5

#define DRAM_BANKS CONFIG_NR_DRAM_BANKS

#if defined(CONFIG_DEFINE_TCM_OCM_MMAP)
#define TCM_MAP 1
#else
#define TCM_MAP 0
#endif

/* +1 is end of list which needs to be empty */
#define VERSAL_MEM_MAP_MAX (VERSAL_MEM_MAP_USED + DRAM_BANKS + TCM_MAP + 1)

static struct mm_region versal_mem_map[VERSAL_MEM_MAP_MAX] = {
	{
		.virt = 0x80000000UL,
		.phys = 0x80000000UL,
		.size = 0x70000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	}, {
		.virt = 0xf0000000UL,
		.phys = 0xf0000000UL,
		.size = 0x0fe00000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	}, {
		.virt = 0x400000000UL,
		.phys = 0x400000000UL,
		.size = 0x200000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	}, {
		.virt = 0x600000000UL,
		.phys = 0x600000000UL,
		.size = 0x800000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
			 PTE_BLOCK_INNER_SHARE
	}, {
		.virt = 0xe00000000UL,
		.phys = 0xe00000000UL,
		.size = 0xf200000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	}
};

void mem_map_fill(void)
{
	int banks = VERSAL_MEM_MAP_USED;

#if defined(CONFIG_DEFINE_TCM_OCM_MMAP)
	versal_mem_map[banks].virt = 0xffe00000UL;
	versal_mem_map[banks].phys = 0xffe00000UL;
	versal_mem_map[banks].size = 0x00200000UL;
	versal_mem_map[banks].attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
				      PTE_BLOCK_INNER_SHARE;
	banks = banks + 1;
#endif

	for (int i = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
		/* Zero size means no more DDR that's this is end */
		if (!gd->bd->bi_dram[i].size)
			break;

#if defined(CONFIG_VERSAL_NO_DDR)
		if (gd->bd->bi_dram[i].start < 0x80000000UL ||
		    gd->bd->bi_dram[i].start > 0x100000000UL) {
			printf("Ignore caches over %llx/%llx\n",
			       gd->bd->bi_dram[i].start,
			       gd->bd->bi_dram[i].size);
			continue;
		}
#endif
		versal_mem_map[banks].virt = gd->bd->bi_dram[i].start;
		versal_mem_map[banks].phys = gd->bd->bi_dram[i].start;
		versal_mem_map[banks].size = gd->bd->bi_dram[i].size;
		versal_mem_map[banks].attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
					      PTE_BLOCK_INNER_SHARE;
		banks = banks + 1;
	}
}

struct mm_region *mem_map = versal_mem_map;

u64 get_page_table_size(void)
{
	return 0x14000;
}

#if defined(CONFIG_SYS_MEM_RSVD_FOR_MMU)
int arm_reserve_mmu(void)
{
	tcm_init(TCM_LOCK);
	gd->arch.tlb_size = PGTABLE_SIZE;
	gd->arch.tlb_addr = VERSAL_TCM_BASE_ADDR;

	return 0;
}
#endif

U_BOOT_DRVINFO(soc_xilinx_versal) = {
	.name = "soc_xilinx_versal",
};
