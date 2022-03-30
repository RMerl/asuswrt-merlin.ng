/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2015 Rockchip Electronics Co., Ltd
 */
#ifndef _ASM_ARCH_CRU_RK3036_H
#define _ASM_ARCH_CRU_RK3036_H

#include <common.h>

#define OSC_HZ		(24 * 1000 * 1000)

#define APLL_HZ		(600 * 1000000)
#define GPLL_HZ		(594 * 1000000)

#define CORE_PERI_HZ	150000000
#define CORE_ACLK_HZ	300000000

#define BUS_ACLK_HZ	148500000
#define BUS_HCLK_HZ	148500000
#define BUS_PCLK_HZ	74250000

#define PERI_ACLK_HZ	148500000
#define PERI_HCLK_HZ	148500000
#define PERI_PCLK_HZ	74250000

/* Private data for the clock driver - used by rockchip_get_cru() */
struct rk3036_clk_priv {
	struct rk3036_cru *cru;
	ulong rate;
};

struct rk3036_cru {
	struct rk3036_pll {
		unsigned int con0;
		unsigned int con1;
		unsigned int con2;
		unsigned int con3;
	} pll[4];
	unsigned int cru_mode_con;
	unsigned int cru_clksel_con[35];
	unsigned int cru_clkgate_con[11];
	unsigned int reserved;
	unsigned int cru_glb_srst_fst_value;
	unsigned int cru_glb_srst_snd_value;
	unsigned int reserved1[2];
	unsigned int cru_softrst_con[9];
	unsigned int cru_misc_con;
	unsigned int reserved2[2];
	unsigned int cru_glb_cnt_th;
	unsigned int cru_sdmmc_con[2];
	unsigned int cru_sdio_con[2];
	unsigned int cru_emmc_con[2];
	unsigned int reserved3;
	unsigned int cru_rst_st;
	unsigned int reserved4[0x23];
	unsigned int cru_pll_mask_con;
};
check_member(rk3036_cru, cru_pll_mask_con, 0x01f0);

struct pll_div {
	u32 refdiv;
	u32 fbdiv;
	u32 postdiv1;
	u32 postdiv2;
	u32 frac;
};

enum {
	/* PLLCON0*/
	PLL_POSTDIV1_SHIFT	= 12,
	PLL_POSTDIV1_MASK	= 7 << PLL_POSTDIV1_SHIFT,
	PLL_FBDIV_SHIFT		= 0,
	PLL_FBDIV_MASK		= 0xfff,

	/* PLLCON1 */
	PLL_RST_SHIFT		= 14,
	PLL_DSMPD_SHIFT		= 12,
	PLL_DSMPD_MASK		= 1 << PLL_DSMPD_SHIFT,
	PLL_LOCK_STATUS_SHIFT	= 10,
	PLL_LOCK_STATUS_MASK	= 1 << PLL_LOCK_STATUS_SHIFT,
	PLL_POSTDIV2_SHIFT	= 6,
	PLL_POSTDIV2_MASK	= 7 << PLL_POSTDIV2_SHIFT,
	PLL_REFDIV_SHIFT	= 0,
	PLL_REFDIV_MASK		= 0x3f,

	/* CRU_MODE */
	GPLL_MODE_SHIFT		= 12,
	GPLL_MODE_MASK		= 3 << GPLL_MODE_SHIFT,
	GPLL_MODE_SLOW		= 0,
	GPLL_MODE_NORM,
	GPLL_MODE_DEEP,
	DPLL_MODE_SHIFT		= 4,
	DPLL_MODE_MASK		= 1 << DPLL_MODE_SHIFT,
	DPLL_MODE_SLOW		= 0,
	DPLL_MODE_NORM,
	APLL_MODE_SHIFT		= 0,
	APLL_MODE_MASK		= 1 << APLL_MODE_SHIFT,
	APLL_MODE_SLOW		= 0,
	APLL_MODE_NORM,

	/* CRU_CLK_SEL0_CON */
	BUS_ACLK_PLL_SEL_SHIFT	= 14,
	BUS_ACLK_PLL_SEL_MASK	= 3 << BUS_ACLK_PLL_SEL_SHIFT,
	BUS_ACLK_PLL_SEL_APLL	= 0,
	BUS_ACLK_PLL_SEL_DPLL,
	BUS_ACLK_PLL_SEL_GPLL,
	BUS_ACLK_DIV_SHIFT	= 8,
	BUS_ACLK_DIV_MASK	= 0x1f << BUS_ACLK_DIV_SHIFT,
	CORE_CLK_PLL_SEL_SHIFT	= 7,
	CORE_CLK_PLL_SEL_MASK	= 1 << CORE_CLK_PLL_SEL_SHIFT,
	CORE_CLK_PLL_SEL_APLL	= 0,
	CORE_CLK_PLL_SEL_GPLL,
	CORE_DIV_CON_SHIFT	= 0,
	CORE_DIV_CON_MASK	= 0x1f << CORE_DIV_CON_SHIFT,

	/* CRU_CLK_SEL1_CON */
	BUS_PCLK_DIV_SHIFT	= 12,
	BUS_PCLK_DIV_MASK	= 7 << BUS_PCLK_DIV_SHIFT,
	BUS_HCLK_DIV_SHIFT	= 8,
	BUS_HCLK_DIV_MASK	= 3 << BUS_HCLK_DIV_SHIFT,
	CORE_ACLK_DIV_SHIFT	= 4,
	CORE_ACLK_DIV_MASK	= 7 << CORE_ACLK_DIV_SHIFT,
	CORE_PERI_DIV_SHIFT	= 0,
	CORE_PERI_DIV_MASK	= 0xf << CORE_PERI_DIV_SHIFT,

	/* CRU_CLKSEL10_CON */
	PERI_PLL_SEL_SHIFT	= 14,
	PERI_PLL_SEL_MASK	= 3 << PERI_PLL_SEL_SHIFT,
	PERI_PLL_APLL		= 0,
	PERI_PLL_DPLL,
	PERI_PLL_GPLL,
	PERI_PCLK_DIV_SHIFT	= 12,
	PERI_PCLK_DIV_MASK	= 3 << PERI_PCLK_DIV_SHIFT,
	PERI_HCLK_DIV_SHIFT	= 8,
	PERI_HCLK_DIV_MASK	= 3 << PERI_HCLK_DIV_SHIFT,
	PERI_ACLK_DIV_SHIFT	= 0,
	PERI_ACLK_DIV_MASK	= 0x1f << PERI_ACLK_DIV_SHIFT,

	/* CRU_CLKSEL11_CON */
	SDIO_DIV_SHIFT		= 8,
	SDIO_DIV_MASK		= 0x7f << SDIO_DIV_SHIFT,
	MMC0_DIV_SHIFT		= 0,
	MMC0_DIV_MASK		= 0x7f << MMC0_DIV_SHIFT,

	/* CRU_CLKSEL12_CON */
	EMMC_PLL_SHIFT		= 12,
	EMMC_PLL_MASK		= 3 << EMMC_PLL_SHIFT,
	EMMC_SEL_APLL		= 0,
	EMMC_SEL_DPLL,
	EMMC_SEL_GPLL,
	EMMC_SEL_24M,
	SDIO_PLL_SHIFT		= 10,
	SDIO_PLL_MASK		= 3 << SDIO_PLL_SHIFT,
	SDIO_SEL_APLL		= 0,
	SDIO_SEL_DPLL,
	SDIO_SEL_GPLL,
	SDIO_SEL_24M,
	MMC0_PLL_SHIFT		= 8,
	MMC0_PLL_MASK		= 3 << MMC0_PLL_SHIFT,
	MMC0_SEL_APLL		= 0,
	MMC0_SEL_DPLL,
	MMC0_SEL_GPLL,
	MMC0_SEL_24M,
	EMMC_DIV_SHIFT		= 0,
	EMMC_DIV_MASK		= 0x7f << EMMC_DIV_SHIFT,

	/* CRU_SOFTRST5_CON */
	DDRCTRL_PSRST_SHIFT	= 11,
	DDRCTRL_SRST_SHIFT	= 10,
	DDRPHY_PSRST_SHIFT	= 9,
	DDRPHY_SRST_SHIFT	= 8,
};
#endif
