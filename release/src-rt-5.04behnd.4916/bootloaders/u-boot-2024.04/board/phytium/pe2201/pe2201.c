// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2023, Phytium Technology Co., Ltd.
 * lixinde          <lixinde@phytium.com.cn>
 * weichangzheng    <weichangzheng@phytium.com.cn>
 */

#include <stdio.h>
#include <command.h>
#include <init.h>
#include <asm/armv8/mmu.h>
#include <asm/io.h>
#include <linux/arm-smccc.h>
#include <scsi.h>
#include <asm/u-boot.h>
#include "cpu.h"

DECLARE_GLOBAL_DATA_PTR;

int mach_cpu_init(void)
{
	check_reset();
	return 0;
}

int board_early_init_f(void)
{
	pcie_init();
	return 0;
}

int dram_init(void)
{
	debug("Phytium ddr init\n");
	ddr_init();

	gd->mem_clk = 0;
	gd->ram_size = PHYS_SDRAM_1_SIZE;

	sec_init();
	debug("PBF relocate done\n");

	return 0;
}

int dram_init_banksize(void)
{
	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[0].size = PHYS_SDRAM_1_SIZE;

	return 0;
}

int board_init(void)
{
	return 0;
}

void reset_cpu(void)
{
	struct arm_smccc_res res;

	debug("run in reset cpu\n");
	arm_smccc_smc(0x84000009, 0, 0, 0, 0, 0, 0, 0, &res);
	if (res.a0 != 0)
		panic("reset cpu error, %lx\n", res.a0);
}

static struct mm_region pe2201_mem_map[] = {
	{
		.virt = 0x0UL,
		.phys = 0x0UL,
		.size = 0x80000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) | PTE_BLOCK_NON_SHARE | PTE_BLOCK_PXN
	},
	{
		.virt = 0x80000000UL,
		.phys = 0x80000000UL,
		.size = 0x7b000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) | PTE_BLOCK_NS | PTE_BLOCK_INNER_SHARE
	},
	{
		0,
	}
};

struct mm_region *mem_map = pe2201_mem_map;

int last_stage_init(void)
{
	return 0;
}
