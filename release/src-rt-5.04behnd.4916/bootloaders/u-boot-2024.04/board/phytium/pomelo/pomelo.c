// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2021
 * lixinde         <lixinde@phytium.com.cn>
 * weichangzheng   <weichangzheng@phytium.com.cn>
 */

#include <stdio.h>
#include <command.h>
#include <event.h>
#include <init.h>
#include <asm/armv8/mmu.h>
#include <asm/io.h>
#include <linux/arm-smccc.h>
#include <scsi.h>
#include <init.h>
#include <asm/u-boot.h>
#include "cpu.h"

DECLARE_GLOBAL_DATA_PTR;

int dram_init(void)
{
	debug("Phytium ddr init\n");
	ddr_init();

	gd->mem_clk = 0;
	gd->ram_size = get_ram_size((void *)CFG_SYS_SDRAM_BASE, 0x7b000000);

	sec_init();
	debug("PBF relocate done\n");

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

static struct mm_region pomelo_mem_map[] = {
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
		.virt = 0x80000000UL,
		.phys = 0x80000000UL,
		.size = 0x7b000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
				 PTE_BLOCK_NS |
				 PTE_BLOCK_INNER_SHARE
	},
	{
		0,
	}
};

struct mm_region *mem_map = pomelo_mem_map;

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
