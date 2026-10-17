// SPDX-License-Identifier: GPL-2.0
/*
 * Configuration for MediaTek MT8512 SoC
 *
 * Copyright (C) 2019 MediaTek Inc.
 * Author: Mingming Lee <mingming.lee@mediatek.com>
 */

#include <clk.h>
#include <common.h>
#include <dm.h>
#include <fdtdec.h>
#include <init.h>
#include <log.h>
#include <ram.h>
#include <wdt.h>
#include <asm/arch/misc.h>
#include <asm/armv8/mmu.h>
#include <asm/cache.h>
#include <asm/global_data.h>
#include <asm/sections.h>
#include <dm/uclass.h>
#include <dt-bindings/clock/mt8512-clk.h>
#include <linux/sizes.h>

DECLARE_GLOBAL_DATA_PTR;

int dram_init(void)
{
	return fdtdec_setup_mem_size_base();
}

phys_size_t  get_effective_memsize(void)
{
	/* limit stack below tee reserve memory */
	return gd->ram_size - 6 * SZ_1M;
}

int dram_init_banksize(void)
{
	gd->bd->bi_dram[0].start = gd->ram_base;
	gd->bd->bi_dram[0].size = get_effective_memsize();

	return 0;
}

void reset_cpu(void)
{
	struct udevice *watchdog_dev = NULL;

	if (uclass_get_device_by_seq(UCLASS_WDT, 0, &watchdog_dev))
		if (uclass_get_device(UCLASS_WDT, 0, &watchdog_dev))
			psci_system_reset();

	wdt_expire_now(watchdog_dev, 0);
}

int print_cpuinfo(void)
{
	debug("CPU:   MediaTek MT8512\n");
	return 0;
}

static struct mm_region mt8512_mem_map[] = {
	{
		/* DDR */
		.virt = 0x40000000UL,
		.phys = 0x40000000UL,
		.size = 0x40000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) | PTE_BLOCK_OUTER_SHARE,
	}, {
		.virt = 0x00000000UL,
		.phys = 0x00000000UL,
		.size = 0x40000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	}, {
		0,
	}
};

struct mm_region *mem_map = mt8512_mem_map;
