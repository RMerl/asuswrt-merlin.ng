// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2019
 * shuyiqi <shuyiqi@phytium.com.cn>
 * liuhao  <liuhao@phytium.com.cn>
 */

#include <common.h>
#include <command.h>
#include <cpu_func.h>
#include <event.h>
#include <init.h>
#include <log.h>
#include <asm/armv8/mmu.h>
#include <asm/cache.h>
#include <asm/global_data.h>
#include <asm/system.h>
#include <asm/io.h>
#include <linux/arm-smccc.h>
#include <linux/kernel.h>
#include <scsi.h>
#include "cpu.h"

DECLARE_GLOBAL_DATA_PTR;

int dram_init(void)
{
	gd->mem_clk = 0;
	gd->ram_size = PHYS_SDRAM_1_SIZE;
	return 0;
}

int dram_init_banksize(void)
{
	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[0].size =  PHYS_SDRAM_1_SIZE;

	return 0;
}

int board_init(void)
{
	return 0;
}

void reset_cpu(void)
{
	struct arm_smccc_res res;

	arm_smccc_smc(0x84000009, 0, 0, 0, 0, 0, 0, 0, &res);
	debug("reset cpu error, %lx\n", res.a0);
}

static struct mm_region durian_mem_map[] = {
	{
		.virt = 0x0UL,
		.phys = 0x0UL,
		.size = 0x80000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
				 PTE_BLOCK_NON_SHARE |
				 PTE_BLOCK_PXN |
				 PTE_BLOCK_UXN
	},
	{
		.virt = (u64)PHYS_SDRAM_1,
		.phys = (u64)PHYS_SDRAM_1,
		.size = (u64)PHYS_SDRAM_1_SIZE,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
				 PTE_BLOCK_NS |
				 PTE_BLOCK_INNER_SHARE
	},
	{
		0,
	}
};

struct mm_region *mem_map = durian_mem_map;

int print_cpuinfo(void)
{
	printf("CPU: Phytium ft2004 %ld MHz\n", gd->cpu_clk);
	return 0;
}

int __asm_flush_l3_dcache(void)
{
	int i, pstate;

	for (i = 0; i < HNF_COUNT; i++)
		writeq(HNF_PSTATE_SFONLY, HNF_PSTATE_REQ + i * HNF_STRIDE);
	for (i = 0; i < HNF_COUNT; i++) {
		do {
			pstate = readq(HNF_PSTATE_STAT + i * HNF_STRIDE);
		} while ((pstate & 0xf) != (HNF_PSTATE_SFONLY << 2));
	}

	for (i = 0; i < HNF_COUNT; i++)
		writeq(HNF_PSTATE_FULL, HNF_PSTATE_REQ + i * HNF_STRIDE);

	return 0;
}

static int last_stage_init(void)
{
	int ret;

	/* pci e */
	pci_init();
	/* scsi scan */
	ret = scsi_scan(true);
	if (ret) {
		printf("scsi scan failed\n");
		return CMD_RET_FAILURE;
	}
	return ret;
}
EVENT_SPY_SIMPLE(EVT_LAST_STAGE_INIT, last_stage_init);
