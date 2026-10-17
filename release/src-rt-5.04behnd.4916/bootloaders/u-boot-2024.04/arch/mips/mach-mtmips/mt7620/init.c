// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2020 MediaTek Inc. All Rights Reserved.
 *
 * Author:  Weijie Gao <weijie.gao@mediatek.com>
 */

#include <config.h>
#include <asm/global_data.h>
#include <linux/io.h>
#include "mt7620.h"

DECLARE_GLOBAL_DATA_PTR;

static const char * const dram_type[] = {
	"SDRAM", "DDR", "DDR2", "SDRAM"
};

static const char * const boot_mode[(CHIP_MODE_M >> CHIP_MODE_S) + 1] = {
	[1] = "NAND 4-cycles 2KB-page",
	[2] = "SPI-NOR 3-Byte Addr",
	[3] = "SPI-NOR 4-Byte Addr",
	[10] = "NAND 4-cycles 512B-page",
	[11] = "NAND 5-cycles 2KB-page",
	[12] = "NAND 3-cycles 512B-page",
};

static void cpu_pll_init(void)
{
	void __iomem *sysc = ioremap_nocache(SYSCTL_BASE, SYSCTL_SIZE);
	u32 pllmul = CONFIG_CPU_FREQ_MULTI;

	/* Make sure the pll multiplier is valid */
	if (pllmul > 7)
		pllmul = 7;

	/* Set init CPU clock to 480MHz */
	clrsetbits_32(sysc + SYSCTL_CPLL_CFG1_REG, CPU_CLK_AUX1, CPU_CLK_AUX0);

	/* Enable software control of CPU PLL */
	setbits_32(sysc + SYSCTL_CPLL_CFG0_REG, CPLL_SW_CFG);

	/* CPU PLL power down */
	setbits_32(sysc + SYSCTL_CPLL_CFG1_REG, CPLL_PD);

	/* PLL configuration */
	clrsetbits_32(sysc + SYSCTL_CPLL_CFG0_REG, PLL_MULT_RATIO_M |
		      PLL_DIV_RATIO_M | SSC_UP_BOUND_M | SSC_EN,
		      (pllmul << PLL_MULT_RATIO_S) | SSC_SWING_M);

	/* CPU PLL power up */
	clrbits_32(sysc + SYSCTL_CPLL_CFG1_REG, CPLL_PD);

	/* Wait for CPU PLL locked */
	while (!(readl(sysc + SYSCTL_CPLL_CFG1_REG) & CPLL_LD))
		;

	/* Set final CPU clock source */
	clrbits_32(sysc + SYSCTL_CPLL_CFG1_REG, CPU_CLK_AUX1 | CPU_CLK_AUX0);

	/* Adjust CPU clock */
	clrsetbits_32(sysc + SYSCTL_CPU_SYS_CLKCFG_REG,
		      CPU_FDIV_M | CPU_FFRAC_M,
		      (1 << CPU_FDIV_S) | (1 << CPU_FFRAC_S));
}

void mt7620_init(void)
{
	u32 cpu_clk;

	cpu_pll_init();

	/*
	 * Set timer freq, which will be used during DRAM initialization
	 * Note that this function is using a temporary gd which will be
	 * destroyed after leaving this function.
	 */
	mt7620_get_clks(&cpu_clk, NULL, NULL);
	gd->arch.timer_freq = cpu_clk / 2;

	mt7620_dram_init();
}

void mt7620_get_clks(u32 *cpu_clk, u32 *sys_clk, u32 *xtal_clk)
{
	void __iomem *sysc = ioremap_nocache(SYSCTL_BASE, SYSCTL_SIZE);
	u32 val, multi, div, fdiv, ffrac, dram_type, sys_div;
	u32 cpu_freq, xtal_freq;

	static const u32 div_ratio_table[] = {2, 3, 4, 8};

	val = readl(sysc + SYSCTL_SYSCFG0_REG);

	dram_type = (val & DRAM_TYPE_M) >> DRAM_TYPE_S;

	if (val & XTAL_FREQ_SEL)
		xtal_freq = 40000000;
	else
		xtal_freq = 20000000;

	val = readl(sysc + SYSCTL_CPLL_CFG1_REG);
	if (val & CPU_CLK_AUX1) {
		cpu_freq = xtal_freq;
	} else if (val & CPU_CLK_AUX0) {
		cpu_freq = 480000000;
	} else {
		val = readl(sysc + SYSCTL_CPLL_CFG0_REG);
		if (val & CPLL_SW_CFG) {
			multi = (val & PLL_MULT_RATIO_M) >> PLL_MULT_RATIO_S;
			div = (val & PLL_DIV_RATIO_M) >> PLL_DIV_RATIO_S;
			cpu_freq = (multi + 24) * 40000000 /
					div_ratio_table[div];
		} else {
			cpu_freq = 600000000;
		}
	}

	val = readl(sysc + SYSCTL_CUR_CLK_STS_REG);
	ffrac = (val & CUR_CPU_FFRAC_M) >> CUR_CPU_FFRAC_S;
	fdiv = (val & CUR_CPU_FDIV_M) >> CUR_CPU_FDIV_S;
	cpu_freq = (cpu_freq * ffrac) / fdiv;

	switch (dram_type) {
	case DRAM_SDRAM_E1:
		sys_div = 4;
		break;
	case DRAM_DDR1:
	case DRAM_DDR2:
		sys_div = 3;
		break;
	case DRAM_SDRAM:
		sys_div = 5;
		break;
	}

	if (cpu_clk)
		*cpu_clk = cpu_freq;

	if (sys_clk)
		*sys_clk = cpu_freq / sys_div;

	if (xtal_clk)
		*xtal_clk = xtal_freq;
}

int print_cpuinfo(void)
{
	void __iomem *sysc = ioremap_nocache(SYSCTL_BASE, SYSCTL_SIZE);
	u32 cpu_clk, bus_clk, xtal_clk;
	u32 val, ver, eco, pkg, dram, chipmode;
	const char *bootdev;

	val = readl(sysc + SYSCTL_CHIP_REV_ID_REG);
	ver = (val & VER_M) >> VER_S;
	eco = (val & ECO_M) >> ECO_S;
	pkg = !!(val & PKG_ID);

	val = readl(sysc + SYSCTL_SYSCFG0_REG);
	dram = (val & DRAM_TYPE_M) >> DRAM_TYPE_S;
	chipmode = (val & CHIP_MODE_M) >> CHIP_MODE_S;

	bootdev = boot_mode[chipmode];
	if (!bootdev)
		bootdev = "Unsupported boot mode";

	printf("CPU:   MediaTek MT7620%c ver:%u eco:%u\n",
	       pkg ? 'A' : 'N', ver, eco);

	printf("Boot:  %s, %s\n", dram_type[dram], bootdev);

	mt7620_get_clks(&cpu_clk, &bus_clk, &xtal_clk);

	/* Set final timer frequency */
	gd->arch.timer_freq = cpu_clk / 2;

	printf("Clock: CPU: %uMHz, Bus: %uMHz, XTAL: %uMHz\n",
	       cpu_clk / 1000000, bus_clk / 1000000, xtal_clk / 1000000);

	return 0;
}

ulong notrace get_tbclk(void)
{
	return gd->arch.timer_freq;
}

void _machine_restart(void)
{
	void __iomem *sysc = ioremap_nocache(SYSCTL_BASE, SYSCTL_SIZE);

	while (1)
		writel(SYS_RST, sysc + SYSCTL_RSTCTL_REG);
}
