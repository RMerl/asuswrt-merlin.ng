// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016 Marek Vasut <marex@denx.de>
 */

#include <common.h>
#include <asm/io.h>
#include <asm/addrspace.h>
#include <asm/types.h>
#include <mach/ar71xx_regs.h>
#include <mach/ath79.h>
#include <wait_bit.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * The math for calculating PLL:
 *                                       NFRAC * 2^8
 *                               NINT + -------------
 *                XTAL [MHz]              2^(18 - 1)
 *   PLL [MHz] = ------------ * ----------------------
 *                  REFDIV              2^OUTDIV
 *
 * Unfortunatelly, there is no way to reliably compute the variables.
 * The vendor U-Boot port contains macros for various combinations of
 * CPU PLL / DDR PLL / AHB bus speed and there is no obvious pattern
 * in those numbers.
 */
struct ar934x_pll_config {
	u8	range;
	u8	refdiv;
	u8	outdiv;
	/* Index 0 is for XTAL=25MHz , Index 1 is for XTAL=40MHz */
	u8	nint[2];
};

struct ar934x_clock_config {
	u16				cpu_freq;
	u16				ddr_freq;
	u16				ahb_freq;

	struct ar934x_pll_config	cpu_pll;
	struct ar934x_pll_config	ddr_pll;
};

static const struct ar934x_clock_config ar934x_clock_config[] = {
	{ 300, 300, 150, { 1, 1, 1, { 24, 15 } }, { 1, 1, 1, { 24, 15 } } },
	{ 400, 200, 200, { 1, 1, 1, { 32, 20 } }, { 1, 1, 2, { 32, 20 } } },
	{ 400, 400, 200, { 0, 1, 1, { 32, 20 } }, { 0, 1, 1, { 32, 20 } } },
	{ 500, 400, 200, { 1, 1, 0, { 20, 12 } }, { 0, 1, 1, { 32, 20 } } },
	{ 533, 400, 200, { 1, 1, 0, { 21, 13 } }, { 0, 1, 1, { 32, 20 } } },
	{ 533, 500, 250, { 1, 1, 0, { 21, 13 } }, { 0, 1, 0, { 20, 12 } } },
	{ 560, 480, 240, { 1, 1, 0, { 22, 14 } }, { 1, 1, 0, { 19, 12 } } },
	{ 566, 400, 200, { 1, 1, 0, { 22, 14 } }, { 1, 1, 0, { 16, 10 } } },
	{ 566, 450, 225, { 1, 1, 0, { 22, 14 } }, { 0, 1, 1, { 36, 22 } } },
	{ 566, 475, 237, { 1, 1, 0, { 22, 14 } }, { 1, 1, 0, { 19, 11 } } },
	{ 566, 500, 250, { 1, 1, 0, { 22, 14 } }, { 1, 1, 0, { 20, 12 } } },
	{ 566, 525, 262, { 1, 1, 0, { 22, 14 } }, { 1, 1, 0, { 21, 13 } } },
	{ 566, 550, 275, { 1, 1, 0, { 22, 14 } }, { 1, 1, 0, { 22, 13 } } },
	{ 600, 266, 133, { 0, 1, 0, { 24, 15 } }, { 1, 1, 1, { 21, 16 } } },
	{ 600, 266, 200, { 0, 1, 0, { 24, 15 } }, { 1, 1, 1, { 21, 16 } } },
	{ 600, 300, 150, { 0, 1, 0, { 24, 15 } }, { 0, 1, 1, { 24, 15 } } },
	{ 600, 332, 166, { 0, 1, 0, { 24, 15 } }, { 1, 1, 1, { 26, 16 } } },
	{ 600, 332, 200, { 0, 1, 0, { 24, 15 } }, { 1, 1, 1, { 26, 16 } } },
	{ 600, 400, 200, { 0, 1, 0, { 24, 15 } }, { 0, 1, 1, { 32, 20 } } },
	{ 600, 450, 200, { 0, 1, 0, { 24, 15 } }, { 0, 1, 0, { 18, 20 } } },
	{ 600, 500, 250, { 0, 1, 0, { 24, 15 } }, { 1, 1, 0, { 20, 12 } } },
	{ 600, 525, 262, { 0, 1, 0, { 24, 15 } }, { 0, 1, 0, { 21, 20 } } },
	{ 600, 550, 275, { 0, 1, 0, { 24, 15 } }, { 0, 1, 0, { 22, 20 } } },
	{ 600, 575, 287, { 0, 1, 0, { 24, 15 } }, { 0, 1, 0, { 23, 14 } } },
	{ 600, 600, 300, { 0, 1, 0, { 24, 15 } }, { 0, 1, 0, { 24, 20 } } },
	{ 600, 650, 325, { 0, 1, 0, { 24, 15 } }, { 0, 1, 0, { 26, 20 } } },
	{ 650, 600, 300, { 0, 1, 0, { 26, 15 } }, { 0, 1, 0, { 24, 20 } } },
	{ 700, 400, 200, { 3, 1, 0, { 28, 17 } }, { 0, 1, 1, { 32, 20 } } },
};

static void ar934x_srif_pll_cfg(void __iomem *pll_reg_base, const u32 srif_val)
{
	u32 reg;
	do {
		writel(0x10810f00, pll_reg_base + 0x4);
		writel(srif_val, pll_reg_base + 0x0);
		writel(0xd0810f00, pll_reg_base + 0x4);
		writel(0x03000000, pll_reg_base + 0x8);
		writel(0xd0800f00, pll_reg_base + 0x4);

		clrbits_be32(pll_reg_base + 0x8, BIT(30));
		udelay(5);
		setbits_be32(pll_reg_base + 0x8, BIT(30));
		udelay(5);

		wait_for_bit_le32(pll_reg_base + 0xc, BIT(3), 1, 10, 0);

		clrbits_be32(pll_reg_base + 0x8, BIT(30));
		udelay(5);

		/* Check if CPU SRIF PLL locked. */
		reg = readl(pll_reg_base + 0x8);
		reg = (reg & 0x7ffff8) >> 3;
	} while (reg >= 0x40000);
}

void ar934x_pll_init(const u16 cpu_mhz, const u16 ddr_mhz, const u16 ahb_mhz)
{
	void __iomem *srif_regs = map_physmem(AR934X_SRIF_BASE,
					      AR934X_SRIF_SIZE, MAP_NOCACHE);
	void __iomem *pll_regs = map_physmem(AR71XX_PLL_BASE,
					     AR71XX_PLL_SIZE, MAP_NOCACHE);
	const struct ar934x_pll_config *pll_cfg;
	int i, pll_nint, pll_refdiv, xtal_40 = 0;
	u32 reg, cpu_pll, cpu_srif, ddr_pll, ddr_srif;

	/* Configure SRIF PLL with initial values. */
	writel(0x13210f00, srif_regs + AR934X_SRIF_CPU_DPLL2_REG);
	writel(0x03000000, srif_regs + AR934X_SRIF_CPU_DPLL3_REG);
	writel(0x13210f00, srif_regs + AR934X_SRIF_DDR_DPLL2_REG);
	writel(0x03000000, srif_regs + AR934X_SRIF_DDR_DPLL3_REG);
	writel(0x03000000, srif_regs + 0x188); /* Undocumented reg :-) */

	/* Test for 40MHz XTAL */
	reg = ath79_get_bootstrap();
	if (reg & AR934X_BOOTSTRAP_REF_CLK_40) {
		xtal_40 = 1;
		cpu_srif = 0x41c00000;
		ddr_srif = 0x41680000;
	} else {
		xtal_40 = 0;
		cpu_srif = 0x29c00000;
		ddr_srif = 0x29680000;
	}

	/* Locate CPU/DDR PLL configuration */
	for (i = 0; i < ARRAY_SIZE(ar934x_clock_config); i++) {
		if (cpu_mhz != ar934x_clock_config[i].cpu_freq)
			continue;
		if (ddr_mhz != ar934x_clock_config[i].ddr_freq)
			continue;
		if (ahb_mhz != ar934x_clock_config[i].ahb_freq)
			continue;

		/* Entry found */
		pll_cfg = &ar934x_clock_config[i].cpu_pll;
		pll_nint = pll_cfg->nint[xtal_40];
		pll_refdiv = pll_cfg->refdiv;
		cpu_pll =
			(pll_nint << AR934X_PLL_CPU_CONFIG_NINT_SHIFT) |
			(pll_refdiv << AR934X_PLL_CPU_CONFIG_REFDIV_SHIFT) |
			(pll_cfg->range << AR934X_PLL_CPU_CONFIG_RANGE_SHIFT) |
			(pll_cfg->outdiv << AR934X_PLL_CPU_CONFIG_OUTDIV_SHIFT);

		pll_cfg = &ar934x_clock_config[i].ddr_pll;
		pll_nint = pll_cfg->nint[xtal_40];
		pll_refdiv = pll_cfg->refdiv;
		ddr_pll =
			(pll_nint << AR934X_PLL_DDR_CONFIG_NINT_SHIFT) |
			(pll_refdiv << AR934X_PLL_DDR_CONFIG_REFDIV_SHIFT) |
			(pll_cfg->range << AR934X_PLL_DDR_CONFIG_RANGE_SHIFT) |
			(pll_cfg->outdiv << AR934X_PLL_DDR_CONFIG_OUTDIV_SHIFT);
		break;
	}

	/* PLL configuration not found, hang. */
	if (i == ARRAY_SIZE(ar934x_clock_config))
		hang();

	/* Set PLL Bypass */
	setbits_be32(pll_regs + AR934X_PLL_CPU_DDR_CLK_CTRL_REG,
		     AR934X_PLL_CLK_CTRL_CPU_PLL_BYPASS);
	setbits_be32(pll_regs + AR934X_PLL_CPU_DDR_CLK_CTRL_REG,
		     AR934X_PLL_CLK_CTRL_DDR_PLL_BYPASS);
	setbits_be32(pll_regs + AR934X_PLL_CPU_DDR_CLK_CTRL_REG,
		     AR934X_PLL_CLK_CTRL_AHB_PLL_BYPASS);

	/* Configure CPU PLL */
	writel(cpu_pll | AR934X_PLL_CPU_CONFIG_PLLPWD,
	       pll_regs + AR934X_PLL_CPU_CONFIG_REG);
	/* Configure DDR PLL */
	writel(ddr_pll | AR934X_PLL_DDR_CONFIG_PLLPWD,
	       pll_regs + AR934X_PLL_DDR_CONFIG_REG);
	/* Configure PLL routing */
	writel(AR934X_PLL_CLK_CTRL_CPU_PLL_BYPASS |
	       AR934X_PLL_CLK_CTRL_DDR_PLL_BYPASS |
	       AR934X_PLL_CLK_CTRL_AHB_PLL_BYPASS |
	       (0 << AR934X_PLL_CLK_CTRL_CPU_POST_DIV_SHIFT) |
	       (0 << AR934X_PLL_CLK_CTRL_DDR_POST_DIV_SHIFT) |
	       (1 << AR934X_PLL_CLK_CTRL_AHB_POST_DIV_SHIFT) |
	       AR934X_PLL_CLK_CTRL_CPUCLK_FROM_CPUPLL |
	       AR934X_PLL_CLK_CTRL_DDRCLK_FROM_DDRPLL |
	       AR934X_PLL_CLK_CTRL_AHBCLK_FROM_DDRPLL,
	       pll_regs + AR934X_PLL_CPU_DDR_CLK_CTRL_REG);

	/* Configure SRIF PLLs, which is completely undocumented :-) */
	ar934x_srif_pll_cfg(srif_regs + AR934X_SRIF_CPU_DPLL1_REG, cpu_srif);
	ar934x_srif_pll_cfg(srif_regs + AR934X_SRIF_DDR_DPLL1_REG, ddr_srif);

	/* Unset PLL Bypass */
	clrbits_be32(pll_regs + AR934X_PLL_CPU_DDR_CLK_CTRL_REG,
		     AR934X_PLL_CLK_CTRL_CPU_PLL_BYPASS);
	clrbits_be32(pll_regs + AR934X_PLL_CPU_DDR_CLK_CTRL_REG,
		     AR934X_PLL_CLK_CTRL_DDR_PLL_BYPASS);
	clrbits_be32(pll_regs + AR934X_PLL_CPU_DDR_CLK_CTRL_REG,
		     AR934X_PLL_CLK_CTRL_AHB_PLL_BYPASS);

	/* Enable PLL dithering */
	writel((1 << AR934X_PLL_DDR_DIT_FRAC_STEP_SHIFT) |
	       (0xf << AR934X_PLL_DDR_DIT_UPD_CNT_SHIFT),
	       pll_regs + AR934X_PLL_DDR_DIT_FRAC_REG);
	writel(48 << AR934X_PLL_CPU_DIT_UPD_CNT_SHIFT,
	       pll_regs + AR934X_PLL_CPU_DIT_FRAC_REG);
}

static u32 ar934x_get_xtal(void)
{
	u32 val;

	val = ath79_get_bootstrap();
	if (val & AR934X_BOOTSTRAP_REF_CLK_40)
		return 40000000;
	else
		return 25000000;
}

int get_serial_clock(void)
{
	return ar934x_get_xtal();
}

static u32 ar934x_cpupll_to_hz(const u32 regval)
{
	const u32 outdiv = (regval >> AR934X_PLL_CPU_CONFIG_OUTDIV_SHIFT) &
			   AR934X_PLL_CPU_CONFIG_OUTDIV_MASK;
	const u32 refdiv = (regval >> AR934X_PLL_CPU_CONFIG_REFDIV_SHIFT) &
			   AR934X_PLL_CPU_CONFIG_REFDIV_MASK;
	const u32 nint = (regval >> AR934X_PLL_CPU_CONFIG_NINT_SHIFT) &
			   AR934X_PLL_CPU_CONFIG_NINT_MASK;
	const u32 nfrac = (regval >> AR934X_PLL_CPU_CONFIG_NFRAC_SHIFT) &
			   AR934X_PLL_CPU_CONFIG_NFRAC_MASK;
	const u32 xtal = ar934x_get_xtal();

	return (xtal * (nint + (nfrac >> 9))) / (refdiv * (1 << outdiv));
}

static u32 ar934x_ddrpll_to_hz(const u32 regval)
{
	const u32 outdiv = (regval >> AR934X_PLL_DDR_CONFIG_OUTDIV_SHIFT) &
			   AR934X_PLL_DDR_CONFIG_OUTDIV_MASK;
	const u32 refdiv = (regval >> AR934X_PLL_DDR_CONFIG_REFDIV_SHIFT) &
			   AR934X_PLL_DDR_CONFIG_REFDIV_MASK;
	const u32 nint = (regval >> AR934X_PLL_DDR_CONFIG_NINT_SHIFT) &
			   AR934X_PLL_DDR_CONFIG_NINT_MASK;
	const u32 nfrac = (regval >> AR934X_PLL_DDR_CONFIG_NFRAC_SHIFT) &
			   AR934X_PLL_DDR_CONFIG_NFRAC_MASK;
	const u32 xtal = ar934x_get_xtal();

	return (xtal * (nint + (nfrac >> 9))) / (refdiv * (1 << outdiv));
}

static void ar934x_update_clock(void)
{
	void __iomem *regs;
	u32 ctrl, cpu, cpupll, ddr, ddrpll;
	u32 cpudiv, ddrdiv, busdiv;
	u32 cpuclk, ddrclk, busclk;

	regs = map_physmem(AR71XX_PLL_BASE, AR71XX_PLL_SIZE,
			   MAP_NOCACHE);

	cpu = readl(regs + AR934X_PLL_CPU_CONFIG_REG);
	ddr = readl(regs + AR934X_PLL_DDR_CONFIG_REG);
	ctrl = readl(regs + AR934X_PLL_CPU_DDR_CLK_CTRL_REG);

	cpupll = ar934x_cpupll_to_hz(cpu);
	ddrpll = ar934x_ddrpll_to_hz(ddr);

	if (ctrl & AR934X_PLL_CLK_CTRL_CPU_PLL_BYPASS)
		cpuclk = ar934x_get_xtal();
	else if (ctrl & AR934X_PLL_CLK_CTRL_CPUCLK_FROM_CPUPLL)
		cpuclk = cpupll;
	else
		cpuclk = ddrpll;

	if (ctrl & AR934X_PLL_CLK_CTRL_DDR_PLL_BYPASS)
		ddrclk = ar934x_get_xtal();
	else if (ctrl & AR934X_PLL_CLK_CTRL_DDRCLK_FROM_DDRPLL)
		ddrclk = ddrpll;
	else
		ddrclk = cpupll;

	if (ctrl & AR934X_PLL_CLK_CTRL_AHB_PLL_BYPASS)
		busclk = ar934x_get_xtal();
	else if (ctrl & AR934X_PLL_CLK_CTRL_AHBCLK_FROM_DDRPLL)
		busclk = ddrpll;
	else
		busclk = cpupll;

	cpudiv = (ctrl >> AR934X_PLL_CLK_CTRL_CPU_POST_DIV_SHIFT) &
		 AR934X_PLL_CLK_CTRL_CPU_POST_DIV_MASK;
	ddrdiv = (ctrl >> AR934X_PLL_CLK_CTRL_DDR_POST_DIV_SHIFT) &
		 AR934X_PLL_CLK_CTRL_DDR_POST_DIV_MASK;
	busdiv = (ctrl >> AR934X_PLL_CLK_CTRL_AHB_POST_DIV_SHIFT) &
		 AR934X_PLL_CLK_CTRL_AHB_POST_DIV_MASK;

	gd->cpu_clk = cpuclk / (cpudiv + 1);
	gd->mem_clk = ddrclk / (ddrdiv + 1);
	gd->bus_clk = busclk / (busdiv + 1);
}

ulong get_bus_freq(ulong dummy)
{
	ar934x_update_clock();
	return gd->bus_clk;
}

ulong get_ddr_freq(ulong dummy)
{
	ar934x_update_clock();
	return gd->mem_clk;
}

int do_ar934x_showclk(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	ar934x_update_clock();
	printf("CPU:       %8ld MHz\n", gd->cpu_clk / 1000000);
	printf("Memory:    %8ld MHz\n", gd->mem_clk / 1000000);
	printf("AHB:       %8ld MHz\n", gd->bus_clk / 1000000);
	return 0;
}

U_BOOT_CMD(
	clocks,	CONFIG_SYS_MAXARGS, 1, do_ar934x_showclk,
	"display clocks",
	""
);
