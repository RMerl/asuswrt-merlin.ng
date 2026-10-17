// SPDX-License-Identifier: GPL-2.0+
/*
 * Samsung Exynos850 clock driver.
 * Copyright (c) 2023 Linaro Ltd.
 * Author: Sam Protsenko <semen.protsenko@linaro.org>
 */

#include <dm.h>
#include <asm/io.h>
#include <dt-bindings/clock/exynos850.h>
#include "clk.h"

/* ---- CMU_TOP ------------------------------------------------------------- */

/* Register Offset definitions for CMU_TOP (0x120e0000) */
#define PLL_CON0_PLL_MMC			0x0100
#define PLL_CON3_PLL_MMC			0x010c
#define PLL_CON0_PLL_SHARED0			0x0140
#define PLL_CON3_PLL_SHARED0			0x014c
#define PLL_CON0_PLL_SHARED1			0x0180
#define PLL_CON3_PLL_SHARED1			0x018c
#define CLK_CON_MUX_MUX_CLKCMU_PERI_BUS		0x1070
#define CLK_CON_MUX_MUX_CLKCMU_PERI_IP		0x1074
#define CLK_CON_MUX_MUX_CLKCMU_PERI_UART	0x1078
#define CLK_CON_DIV_CLKCMU_PERI_BUS		0x187c
#define CLK_CON_DIV_CLKCMU_PERI_IP		0x1880
#define CLK_CON_DIV_CLKCMU_PERI_UART		0x1884
#define CLK_CON_DIV_PLL_SHARED0_DIV2		0x188c
#define CLK_CON_DIV_PLL_SHARED0_DIV3		0x1890
#define CLK_CON_DIV_PLL_SHARED0_DIV4		0x1894
#define CLK_CON_DIV_PLL_SHARED1_DIV2		0x1898
#define CLK_CON_DIV_PLL_SHARED1_DIV3		0x189c
#define CLK_CON_DIV_PLL_SHARED1_DIV4		0x18a0
#define CLK_CON_GAT_GATE_CLKCMU_PERI_BUS	0x2080
#define CLK_CON_GAT_GATE_CLKCMU_PERI_IP		0x2084
#define CLK_CON_GAT_GATE_CLKCMU_PERI_UART	0x2088

static const struct samsung_pll_clock top_pure_pll_clks[] = {
	PLL(pll_0822x, CLK_FOUT_SHARED0_PLL, "fout_shared0_pll", "clock-oscclk",
	    PLL_CON3_PLL_SHARED0),
	PLL(pll_0822x, CLK_FOUT_SHARED1_PLL, "fout_shared1_pll", "clock-oscclk",
	    PLL_CON3_PLL_SHARED1),
	PLL(pll_0831x, CLK_FOUT_MMC_PLL, "fout_mmc_pll", "clock-oscclk",
	    PLL_CON3_PLL_MMC),
};

/* List of parent clocks for Muxes in CMU_TOP */
PNAME(mout_shared0_pll_p)	= { "clock-oscclk", "fout_shared0_pll" };
PNAME(mout_shared1_pll_p)	= { "clock-oscclk", "fout_shared1_pll" };
PNAME(mout_mmc_pll_p)		= { "clock-oscclk", "fout_mmc_pll" };
/* List of parent clocks for Muxes in CMU_TOP: for CMU_PERI */
PNAME(mout_peri_bus_p)		= { "dout_shared0_div4", "dout_shared1_div4" };
PNAME(mout_peri_uart_p)		= { "clock-oscclk", "dout_shared0_div4",
				    "dout_shared1_div4", "clock-oscclk" };
PNAME(mout_peri_ip_p)		= { "clock-oscclk", "dout_shared0_div4",
				    "dout_shared1_div4", "clock-oscclk" };

static const struct samsung_mux_clock top_pure_mux_clks[] = {
	MUX(CLK_MOUT_SHARED0_PLL, "mout_shared0_pll", mout_shared0_pll_p,
	    PLL_CON0_PLL_SHARED0, 4, 1),
	MUX(CLK_MOUT_SHARED1_PLL, "mout_shared1_pll", mout_shared1_pll_p,
	    PLL_CON0_PLL_SHARED1, 4, 1),
	MUX(CLK_MOUT_MMC_PLL, "mout_mmc_pll", mout_mmc_pll_p,
	    PLL_CON0_PLL_MMC, 4, 1),
};

static const struct samsung_mux_clock top_peri_mux_clks[] = {
	MUX(CLK_MOUT_PERI_BUS, "mout_peri_bus", mout_peri_bus_p,
	    CLK_CON_MUX_MUX_CLKCMU_PERI_BUS, 0, 1),
	MUX(CLK_MOUT_PERI_UART, "mout_peri_uart", mout_peri_uart_p,
	    CLK_CON_MUX_MUX_CLKCMU_PERI_UART, 0, 2),
	MUX(CLK_MOUT_PERI_IP, "mout_peri_ip", mout_peri_ip_p,
	    CLK_CON_MUX_MUX_CLKCMU_PERI_IP, 0, 2),
};

static const struct samsung_div_clock top_pure_div_clks[] = {
	DIV(CLK_DOUT_SHARED0_DIV3, "dout_shared0_div3", "mout_shared0_pll",
	    CLK_CON_DIV_PLL_SHARED0_DIV3, 0, 2),
	DIV(CLK_DOUT_SHARED0_DIV2, "dout_shared0_div2", "mout_shared0_pll",
	    CLK_CON_DIV_PLL_SHARED0_DIV2, 0, 1),
	DIV(CLK_DOUT_SHARED1_DIV3, "dout_shared1_div3", "mout_shared1_pll",
	    CLK_CON_DIV_PLL_SHARED1_DIV3, 0, 2),
	DIV(CLK_DOUT_SHARED1_DIV2, "dout_shared1_div2", "mout_shared1_pll",
	    CLK_CON_DIV_PLL_SHARED1_DIV2, 0, 1),
	DIV(CLK_DOUT_SHARED0_DIV4, "dout_shared0_div4", "dout_shared0_div2",
	    CLK_CON_DIV_PLL_SHARED0_DIV4, 0, 1),
	DIV(CLK_DOUT_SHARED1_DIV4, "dout_shared1_div4", "dout_shared1_div2",
	    CLK_CON_DIV_PLL_SHARED1_DIV4, 0, 1),
};

static const struct samsung_div_clock top_peri_div_clks[] = {
	DIV(CLK_DOUT_PERI_BUS, "dout_peri_bus", "gout_peri_bus",
	    CLK_CON_DIV_CLKCMU_PERI_BUS, 0, 4),
	DIV(CLK_DOUT_PERI_UART, "dout_peri_uart", "gout_peri_uart",
	    CLK_CON_DIV_CLKCMU_PERI_UART, 0, 4),
	DIV(CLK_DOUT_PERI_IP, "dout_peri_ip", "gout_peri_ip",
	    CLK_CON_DIV_CLKCMU_PERI_IP, 0, 4),
};

static const struct samsung_gate_clock top_peri_gate_clks[] = {
	GATE(CLK_GOUT_PERI_BUS, "gout_peri_bus", "mout_peri_bus",
	     CLK_CON_GAT_GATE_CLKCMU_PERI_BUS, 21, 0, 0),
	GATE(CLK_GOUT_PERI_UART, "gout_peri_uart", "mout_peri_uart",
	     CLK_CON_GAT_GATE_CLKCMU_PERI_UART, 21, 0, 0),
	GATE(CLK_GOUT_PERI_IP, "gout_peri_ip", "mout_peri_ip",
	     CLK_CON_GAT_GATE_CLKCMU_PERI_IP, 21, 0, 0),
};

static const struct samsung_clk_group top_cmu_clks[] = {
	/* CMU_TOP_PURECLKCOMP */
	{ S_CLK_PLL, top_pure_pll_clks, ARRAY_SIZE(top_pure_pll_clks) },
	{ S_CLK_MUX, top_pure_mux_clks, ARRAY_SIZE(top_pure_mux_clks) },
	{ S_CLK_DIV, top_pure_div_clks, ARRAY_SIZE(top_pure_div_clks) },

	/* CMU_TOP clocks for CMU_PERI */
	{ S_CLK_MUX, top_peri_mux_clks, ARRAY_SIZE(top_peri_mux_clks) },
	{ S_CLK_GATE, top_peri_gate_clks, ARRAY_SIZE(top_peri_gate_clks) },
	{ S_CLK_DIV, top_peri_div_clks, ARRAY_SIZE(top_peri_div_clks) },
};

static int exynos850_cmu_top_probe(struct udevice *dev)
{
	return samsung_cmu_register_one(dev, top_cmu_clks,
					ARRAY_SIZE(top_cmu_clks));
}

static const struct udevice_id exynos850_cmu_top_ids[] = {
	{ .compatible = "samsung,exynos850-cmu-top" },
	{ }
};

U_BOOT_DRIVER(exynos850_cmu_top) = {
	.name		= "exynos850-cmu-top",
	.id		= UCLASS_CLK,
	.of_match	= exynos850_cmu_top_ids,
	.ops		= &ccf_clk_ops,
	.probe		= exynos850_cmu_top_probe,
	.flags		= DM_FLAG_PRE_RELOC,
};

/* ---- CMU_PERI ------------------------------------------------------------ */

/* Register Offset definitions for CMU_PERI (0x10030000) */
#define PLL_CON0_MUX_CLKCMU_PERI_BUS_USER	0x0600
#define PLL_CON0_MUX_CLKCMU_PERI_UART_USER	0x0630
#define CLK_CON_GAT_GOUT_PERI_UART_IPCLK	0x20a8
#define CLK_CON_GAT_GOUT_PERI_UART_PCLK		0x20ac

/* List of parent clocks for Muxes in CMU_PERI */
PNAME(mout_peri_bus_user_p)	= { "clock-oscclk", "dout_peri_bus" };
PNAME(mout_peri_uart_user_p)	= { "clock-oscclk", "dout_peri_uart" };

static const struct samsung_mux_clock peri_mux_clks[] = {
	MUX(CLK_MOUT_PERI_BUS_USER, "mout_peri_bus_user", mout_peri_bus_user_p,
	    PLL_CON0_MUX_CLKCMU_PERI_BUS_USER, 4, 1),
	MUX(CLK_MOUT_PERI_UART_USER, "mout_peri_uart_user",
	    mout_peri_uart_user_p, PLL_CON0_MUX_CLKCMU_PERI_UART_USER, 4, 1),
};

static const struct samsung_gate_clock peri_gate_clks[] = {
	GATE(CLK_GOUT_UART_IPCLK, "gout_uart_ipclk", "mout_peri_uart_user",
	     CLK_CON_GAT_GOUT_PERI_UART_IPCLK, 21, 0, 0),
	GATE(CLK_GOUT_UART_PCLK, "gout_uart_pclk", "mout_peri_bus_user",
	     CLK_CON_GAT_GOUT_PERI_UART_PCLK, 21, 0, 0),
};

static const struct samsung_clk_group peri_cmu_clks[] = {
	{ S_CLK_MUX, peri_mux_clks, ARRAY_SIZE(peri_mux_clks) },
	{ S_CLK_GATE, peri_gate_clks, ARRAY_SIZE(peri_gate_clks) },
};

static int exynos850_cmu_peri_probe(struct udevice *dev)
{
	return samsung_register_cmu(dev, peri_cmu_clks, exynos850_cmu_top);
}

static const struct udevice_id exynos850_cmu_peri_ids[] = {
	{ .compatible = "samsung,exynos850-cmu-peri" },
	{ }
};

U_BOOT_DRIVER(exynos850_cmu_peri) = {
	.name		= "exynos850-cmu-peri",
	.id		= UCLASS_CLK,
	.of_match	= exynos850_cmu_peri_ids,
	.ops		= &ccf_clk_ops,
	.probe		= exynos850_cmu_peri_probe,
	.flags		= DM_FLAG_PRE_RELOC,
};
