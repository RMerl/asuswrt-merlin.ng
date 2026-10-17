// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2018 MediaTek Inc.
 * Author: Ryder Lee <ryder.lee@mediatek.com>
 */

#include <clk.h>
#include <common.h>
#include <dm.h>
#include <fdtdec.h>
#include <init.h>
#include <log.h>
#include <ram.h>
#include <asm/arch/misc.h>
#include <asm/global_data.h>
#include <asm/sections.h>
#include <dm/uclass.h>
#include <linux/bitops.h>
#include <linux/io.h>

#include <dt-bindings/clock/mt7629-clk.h>

#define L2_CFG_BASE		0x10200000
#define L2_CFG_SIZE		0x1000
#define L2_SHARE_CFG_MP0	0x7f0
#define L2_SHARE_MODE_OFF	BIT(8)

DECLARE_GLOBAL_DATA_PTR;

int mtk_pll_early_init(void)
{
	unsigned long pll_rates[] = {
		[CLK_APMIXED_ARMPLL] = 1250000000,
		[CLK_APMIXED_MAINPLL] = 1120000000,
		[CLK_APMIXED_UNIV2PLL] = 1200000000,
		[CLK_APMIXED_ETH1PLL] = 500000000,
		[CLK_APMIXED_ETH2PLL] = 700000000,
		[CLK_APMIXED_SGMIPLL] = 650000000,
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

	/* setup mcu bus */
	ret = uclass_get_device_by_driver(UCLASS_SYSCON,
			DM_DRIVER_GET(mtk_mcucfg), &dev);
	if (ret)
		return ret;

	return 0;
}

int mtk_soc_early_init(void)
{
	struct udevice *dev;
	int ret;

	/* initialize early clocks */
	ret = mtk_pll_early_init();
	if (ret)
		return ret;

	ret = uclass_first_device_err(UCLASS_RAM, &dev);
	if (ret)
		return ret;

	return 0;
}

int mach_cpu_init(void)
{
	void __iomem *base;

	base = ioremap(L2_CFG_BASE, L2_CFG_SIZE);

	/* disable L2C shared mode */
	writel(L2_SHARE_MODE_OFF, base + L2_SHARE_CFG_MP0);

	return 0;
}

int dram_init(void)
{
	struct ram_info ram;
	struct udevice *dev;
	int ret;

	ret = uclass_first_device_err(UCLASS_RAM, &dev);
	if (ret)
		return ret;

	ret = ram_get_info(dev, &ram);
	if (ret)
		return ret;

	debug("RAM init base=%lx, size=%x\n", ram.base, ram.size);

	gd->ram_size = ram.size;

	return 0;
}

int print_cpuinfo(void)
{
	void __iomem *chipid;
	u32 hwcode, swver;

	chipid = ioremap(VER_BASE, VER_SIZE);
	hwcode = readl(chipid + APHW_CODE);
	swver = readl(chipid + APSW_VER);

	printf("CPU:   MediaTek MT%04x E%d\n", hwcode, (swver & 0xf) + 1);

	return 0;
}
