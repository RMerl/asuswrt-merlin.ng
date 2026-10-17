// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2020 MediaTek Inc.
 *
 * Author:  Weijie Gao <weijie.gao@mediatek.com>
 */

#include <clk.h>
#include <dm.h>
#include <asm/global_data.h>
#include <dm/uclass.h>
#include <dt-bindings/clock/mt7628-clk.h>
#include <linux/io.h>
#include "mt7628.h"

DECLARE_GLOBAL_DATA_PTR;

static void set_init_timer_freq(void)
{
	void __iomem *sysc;
	u32 bs, val, timer_freq_post;

	sysc = ioremap_nocache(SYSCTL_BASE, SYSCTL_SIZE);

	/* We can't use the clk driver as the DM has not been initialized yet */
	bs = readl(sysc + SYSCTL_SYSCFG0_REG);
	if ((bs & XTAL_FREQ_SEL) == XTAL_25MHZ) {
		gd->arch.timer_freq = 25000000;
		timer_freq_post = 575000000;
	} else {
		gd->arch.timer_freq = 40000000;
		timer_freq_post = 580000000;
	}

	val = readl(sysc + SYSCTL_CLKCFG0_REG);
	if (!(val & (CPU_PLL_FROM_BBP | CPU_PLL_FROM_XTAL)))
		gd->arch.timer_freq = timer_freq_post;
}

void mt7628_init(void)
{
	set_init_timer_freq();

	mt7628_ddr_init();
}

int print_cpuinfo(void)
{
	void __iomem *sysc;
	struct udevice *clkdev;
	u32 val, ver, eco, pkg, ddr, chipmode, ee;
	ulong cpu_clk, bus_clk, xtal_clk, timer_freq;
	struct clk clk;
	int ret;

	sysc = ioremap_nocache(SYSCTL_BASE, SYSCTL_SIZE);

	val = readl(sysc + SYSCTL_CHIP_REV_ID_REG);
	ver = (val & VER_M) >> VER_S;
	eco = (val & ECO_M) >> ECO_S;
	pkg = !!(val & PKG_ID);

	val = readl(sysc + SYSCTL_SYSCFG0_REG);
	ddr = val & DRAM_TYPE;
	chipmode = (val & CHIP_MODE_M) >> CHIP_MODE_S;

	val = readl(sysc + SYSCTL_EFUSE_CFG_REG);
	ee = val & EFUSE_MT7688;

	if (pkg == PKG_ID_KN)
		ddr = DRAM_DDR1;

	printf("CPU:   MediaTek MT%u%c ver:%u eco:%u\n",
	       ee ? 7688 : 7628, pkg ? 'A' : 'K', ver, eco);

	printf("Boot:  DDR%s, SPI-NOR %u-Byte Addr, CPU clock from %s\n",
	       ddr ? "" : "2", chipmode & 0x01 ? 4 : 3,
	       chipmode & 0x02 ? "XTAL" : "CPLL");

	ret = uclass_get_device_by_driver(UCLASS_CLK, DM_DRIVER_GET(mt7628_clk),
					  &clkdev);
	if (ret)
		return ret;

	clk.dev = clkdev;

	clk.id = CLK_CPU;
	cpu_clk = clk_get_rate(&clk);

	clk.id = CLK_SYS;
	bus_clk = clk_get_rate(&clk);

	clk.id = CLK_XTAL;
	xtal_clk = clk_get_rate(&clk);

	clk.id = CLK_MIPS_CNT;
	timer_freq = clk_get_rate(&clk);

	/* Set final timer frequency */
	if (timer_freq)
		gd->arch.timer_freq = timer_freq;

	printf("Clock: CPU: %luMHz, Bus: %luMHz, XTAL: %luMHz\n",
	       cpu_clk / 1000000, bus_clk / 1000000, xtal_clk / 1000000);

	return 0;
}

ulong notrace get_tbclk(void)
{
	return gd->arch.timer_freq;
}
