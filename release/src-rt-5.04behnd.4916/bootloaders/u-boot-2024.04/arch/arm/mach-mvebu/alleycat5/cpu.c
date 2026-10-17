// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018 Marvell International Ltd.
 */

#include <common.h>
#include <dm.h>
#include <fdtdec.h>
#include <linux/libfdt.h>
#include <asm/io.h>
#include <asm/system.h>
#include <asm/arch/cpu.h>
#include <linux/sizes.h>
#include <asm/armv8/mmu.h>
#include "soc.h"

DECLARE_GLOBAL_DATA_PTR;

#define AC5_PTE_BLOCK_DEVICE \
	(PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) | \
			 PTE_BLOCK_NON_SHARE | \
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN)

static struct mm_region ac5_mem_map[] = {
	{
		/* RAM */
		.phys = CFG_SYS_SDRAM_BASE,
		.virt = CFG_SYS_SDRAM_BASE,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
			 PTE_BLOCK_INNER_SHARE
	},
	{
		/* MMIO regions */
		.phys = 0x00000000,
		.virt = 0xa0000000,
		.size = 0x100000,
		.attrs = AC5_PTE_BLOCK_DEVICE,
	},
	{
		/* MMIO regions */
		.phys = 0x100000,
		.virt = 0x100000,
		.size = 0x3ff00000,
		.attrs = AC5_PTE_BLOCK_DEVICE,
	},
	{
		.phys = 0x7F000000,
		.virt = 0x7F000000,
		.size = SZ_8M,
		.attrs = AC5_PTE_BLOCK_DEVICE,
	},
	{
		.phys = 0x7F800000,
		.virt = 0x7F800000,
		.size = SZ_4M,
		.attrs = AC5_PTE_BLOCK_DEVICE,
	},
	{
		.phys = 0x7FC00000,
		.virt = 0x7FC00000,
		.size = SZ_512K,
		.attrs = AC5_PTE_BLOCK_DEVICE,
	},
	{
		.phys = 0x7FC80000,
		.virt = 0x7FC80000,
		.size = SZ_512K,
		.attrs = AC5_PTE_BLOCK_DEVICE,
	},
	{
		.phys = 0x7FD00000,
		.virt = 0x7FD00000,
		.size = SZ_512K,
		.attrs = AC5_PTE_BLOCK_DEVICE,
	},
	/* ATF region 0x7FE00000-0x7FE20000 not mapped */
	{
		.phys = 0x7FE80000,
		.virt = 0x7FE80000,
		.size = SZ_512K,
		.attrs = AC5_PTE_BLOCK_DEVICE,
	},
	{
		.phys = 0x7FFF0000,
		.virt = 0x7FFF0000,
		.size = SZ_1M,
		.attrs = AC5_PTE_BLOCK_DEVICE,
	},
	{
		.phys = 0x80000000,
		.virt = 0x80000000,
		.size = SZ_2G,
		.attrs = AC5_PTE_BLOCK_DEVICE,
	},
	{
		0,
	}
};

struct mm_region *mem_map = ac5_mem_map;

void reset_cpu(void)
{
}

int print_cpuinfo(void)
{
	soc_print_device_info();
	soc_print_clock_info();

	return 0;
}

int alleycat5_dram_init(void)
{
#define SCRATCH_PAD_REG		0x80010018
	int ret;

	/* override DDR_FW size if DTS is set with size */
	ret = fdtdec_setup_mem_size_base();
	if (ret == -EINVAL)
		gd->ram_size = readl(SCRATCH_PAD_REG) * 4ULL;

	/* if DRAM size == 0, print error message */
	if (gd->ram_size == 0) {
		pr_err("DRAM size not initialized - check DRAM configuration\n");
		printf("\n Using temporary DRAM size of 512MB.\n\n");
		gd->ram_size = SZ_512M;
	}

	ac5_mem_map[0].size = gd->ram_size;

	return 0;
}

int alleycat5_dram_init_banksize(void)
{
	/*
	 * Config single DRAM bank
	 */
	gd->bd->bi_dram[0].start = CFG_SYS_SDRAM_BASE;
	gd->bd->bi_dram[0].size = gd->ram_size;

	return 0;
}

int timer_init(void)
{
	return 0;
}

/*
 * get_ref_clk
 *
 * return: reference clock in MHz
 */
u32 get_ref_clk(void)
{
	return 25;
}
