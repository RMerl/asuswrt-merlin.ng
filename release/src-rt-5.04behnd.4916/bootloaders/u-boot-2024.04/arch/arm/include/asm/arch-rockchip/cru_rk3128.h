/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2017 Rockchip Electronics Co., Ltd
 */

#ifndef _ASM_ARCH_CRU_RK3128_H
#define _ASM_ARCH_CRU_RK3128_H

#ifndef __ASSEMBLY__
#include <linux/bitops.h>
#endif

#define MHz		1000000
#define OSC_HZ		(24 * MHz)

#define APLL_HZ		(600 * MHz)
#define GPLL_HZ		(594 * MHz)

#define CORE_PERI_HZ	150000000
#define CORE_ACLK_HZ	300000000

#define BUS_ACLK_HZ	148500000
#define BUS_HCLK_HZ	148500000
#define BUS_PCLK_HZ	74250000

#define PERI_ACLK_HZ	148500000
#define PERI_HCLK_HZ	148500000
#define PERI_PCLK_HZ	74250000

/* Private data for the clock driver - used by rockchip_get_cru() */
struct rk3128_clk_priv {
	struct rk3128_cru *cru;
};

struct rk3128_cru {
	struct rk3128_pll {
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
	unsigned int reserved3[3];
	unsigned int cru_glb_rst_st;
	unsigned int reserved4[(0x1c0 - 0x150) / 4 - 1];
	unsigned int cru_sdmmc_con[2];
	unsigned int cru_sdio_con[2];
	unsigned int reserved5[2];
	unsigned int cru_emmc_con[2];
	unsigned int reserved6[4];
	unsigned int cru_pll_prg_en;
};
check_member(rk3128_cru, cru_pll_prg_en, 0x01f0);

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
	PLL_PD_SHIFT		= 13,
	PLL_PD_MASK		= 1 << PLL_PD_SHIFT,
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
	CPLL_MODE_SHIFT		= 8,
	CPLL_MODE_MASK		= 1 << CPLL_MODE_SHIFT,
	CPLL_MODE_SLOW		= 0,
	CPLL_MODE_NORM,
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
	BUS_ACLK_PLL_SEL_CPLL	= 0,
	BUS_ACLK_PLL_SEL_GPLL,
	BUS_ACLK_PLL_SEL_GPLL_DIV2,
	BUS_ACLK_PLL_SEL_GPLL_DIV3,
	BUS_ACLK_DIV_SHIFT	= 8,
	BUS_ACLK_DIV_MASK	= 0x1f << BUS_ACLK_DIV_SHIFT,
	CORE_CLK_PLL_SEL_SHIFT	= 7,
	CORE_CLK_PLL_SEL_MASK	= 1 << CORE_CLK_PLL_SEL_SHIFT,
	CORE_CLK_PLL_SEL_APLL	= 0,
	CORE_CLK_PLL_SEL_GPLL_DIV2,
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

	/* CRU_CLK_SEL2_CON */
	NANDC_PLL_SEL_SHIFT	= 14,
	NANDC_PLL_SEL_MASK	= 3 << NANDC_PLL_SEL_SHIFT,
	NANDC_PLL_SEL_CPLL	= 0,
	NANDC_PLL_SEL_GPLL,
	NANDC_CLK_DIV_SHIFT	= 8,
	NANDC_CLK_DIV_MASK	= 0x1f << NANDC_CLK_DIV_SHIFT,
	PVTM_CLK_DIV_SHIFT	= 0,
	PVTM_CLK_DIV_MASK	= 0x3f << PVTM_CLK_DIV_SHIFT,

	/* CRU_CLKSEL10_CON */
	PERI_PLL_SEL_SHIFT	= 14,
	PERI_PLL_SEL_MASK	= 1 << PERI_PLL_SEL_SHIFT,
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
	MMC0_PLL_SHIFT		= 6,
	MMC0_PLL_MASK		= 3 << MMC0_PLL_SHIFT,
	MMC0_SEL_APLL		= 0,
	MMC0_SEL_GPLL,
	MMC0_SEL_GPLL_DIV2,
	MMC0_SEL_24M,
	MMC0_DIV_SHIFT		= 0,
	MMC0_DIV_MASK		= 0x3f << MMC0_DIV_SHIFT,

	/* CRU_CLKSEL12_CON */
	EMMC_PLL_SHIFT		= 14,
	EMMC_PLL_MASK		= 3 << EMMC_PLL_SHIFT,
	EMMC_SEL_APLL		= 0,
	EMMC_SEL_GPLL,
	EMMC_SEL_GPLL_DIV2,
	EMMC_SEL_24M,
	EMMC_DIV_SHIFT		= 8,
	EMMC_DIV_MASK		= 0x3f << EMMC_DIV_SHIFT,

	/* CLKSEL_CON24 */
	SARADC_DIV_CON_SHIFT	= 8,
	SARADC_DIV_CON_MASK	= GENMASK(15, 8),
	SARADC_DIV_CON_WIDTH	= 8,

	/* CRU_CLKSEL27_CON*/
	DCLK_VOP_SEL_SHIFT         = 0,
	DCLK_VOP_SEL_MASK          = 1 << DCLK_VOP_SEL_SHIFT,
	DCLK_VOP_PLL_SEL_CPLL           = 0,
	DCLK_VOP_DIV_CON_SHIFT          = 8,
	DCLK_VOP_DIV_CON_MASK           = 0xff << DCLK_VOP_DIV_CON_SHIFT,

	/* CRU_CLKSEL31_CON */
	VIO0_PLL_SHIFT		= 5,
	VIO0_PLL_MASK		= 7 << VIO0_PLL_SHIFT,
	VI00_SEL_CPLL		= 0,
	VIO0_SEL_GPLL,
	VIO0_DIV_SHIFT		= 0,
	VIO0_DIV_MASK		= 0x1f << VIO0_DIV_SHIFT,
	VIO1_PLL_SHIFT		= 13,
	VIO1_PLL_MASK		= 7 << VIO1_PLL_SHIFT,
	VI01_SEL_CPLL		= 0,
	VIO1_SEL_GPLL,
	VIO1_DIV_SHIFT		= 8,
	VIO1_DIV_MASK		= 0x1f << VIO1_DIV_SHIFT,

	/* CRU_SOFTRST5_CON */
	DDRCTRL_PSRST_SHIFT	= 11,
	DDRCTRL_SRST_SHIFT	= 10,
	DDRPHY_PSRST_SHIFT	= 9,
	DDRPHY_SRST_SHIFT	= 8,
};
#endif
