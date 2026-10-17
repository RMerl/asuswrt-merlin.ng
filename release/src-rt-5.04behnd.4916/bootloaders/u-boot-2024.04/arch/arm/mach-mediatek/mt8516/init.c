// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2018 MediaTek Inc.
 * Copyright (C) 2019 BayLibre, SAS
 * Author: Fabien Parent <fparent@baylibre.com>
 */

#include <clk.h>
#include <common.h>
#include <cpu_func.h>
#include <dm.h>
#include <fdtdec.h>
#include <init.h>
#include <ram.h>
#include <asm/arch/misc.h>
#include <asm/armv8/mmu.h>
#include <asm/cache.h>
#include <asm/global_data.h>
#include <asm/sections.h>
#include <dm/uclass.h>
#include <dt-bindings/clock/mt8516-clk.h>

DECLARE_GLOBAL_DATA_PTR;

int dram_init(void)
{
	int ret;

	ret = fdtdec_setup_memory_banksize();
	if (ret)
		return ret;

	return fdtdec_setup_mem_size_base();
}

int dram_init_banksize(void)
{
	gd->bd->bi_dram[0].start = gd->ram_base;
	gd->bd->bi_dram[0].size = gd->ram_size;

	return 0;
}

int mtk_pll_early_init(void)
{
	unsigned long pll_rates[] = {
		[CLK_APMIXED_ARMPLL] =   1300000000,
		[CLK_APMIXED_MAINPLL] =  1501000000,
		[CLK_APMIXED_UNIVPLL] =  1248000000,
		[CLK_APMIXED_MMPLL] =     380000000,
	};
	struct udevice *dev;
	int ret, i;

	ret = uclass_get_device_by_driver(UCLASS_CLK,
			DM_DRIVER_GET(mtk_clk_apmixedsys), &dev);
	if (ret)
		return ret;

	/* configure default rate then enable apmixedsys */
	for (i = 0; i < ARRAY_SIZE(pll_rates); i++) {
		struct clk clk = { .id = i, .dev = dev };

		ret = clk_set_rate(&clk, pll_rates[i]);
		if (ret)
			return ret;

		ret = clk_enable(&clk);
		if (ret)
			return ret;
	}

	return 0;
}

int mtk_soc_early_init(void)
{
	int ret;

	/* initialize early clocks */
	ret = mtk_pll_early_init();
	if (ret)
		return ret;

	return 0;
}

void reset_cpu(void)
{
	psci_system_reset();
}

int print_cpuinfo(void)
{
	printf("CPU:   MediaTek MT8516\n");
	return 0;
}

static struct mm_region mt8516_mem_map[] = {
	{
		/* DDR */
		.virt = 0x40000000UL,
		.phys = 0x40000000UL,
		.size = 0x20000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) | PTE_BLOCK_OUTER_SHARE,
	}, {
		.virt = 0x00000000UL,
		.phys = 0x00000000UL,
		.size = 0x20000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	}, {
		0,
	}
};
struct mm_region *mem_map = mt8516_mem_map;
