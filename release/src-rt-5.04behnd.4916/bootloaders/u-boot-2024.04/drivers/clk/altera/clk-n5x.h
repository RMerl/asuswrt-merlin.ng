/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 * Copyright (C) 2020-2022 Intel Corporation <www.intel.com>
 */

#ifndef	_CLK_N5X_
#define	_CLK_N5X_

#ifndef __ASSEMBLY__
#include <linux/bitops.h>
#endif

#define CM_REG_READL(plat, reg)				\
	readl((plat)->regs + (reg))

#define CM_REG_WRITEL(plat, data, reg)			\
	writel(data, (plat)->regs + (reg))

#define CM_REG_CLRBITS(plat, reg, clear)		\
	clrbits_le32((plat)->regs + (reg), (clear))

#define CM_REG_SETBITS(plat, reg, set)			\
	setbits_le32((plat)->regs + (reg), (set))

struct cm_config {
	/* main group */
	u32 main_pll_mpuclk;
	u32 main_pll_nocclk;
	u32 main_pll_nocdiv;
	u32 main_pll_pllglob;
	u32 main_pll_plldiv;
	u32 main_pll_plloutdiv;
	u32 spare_1[4];

	/* peripheral group */
	u32 per_pll_emacctl;
	u32 per_pll_gpiodiv;
	u32 per_pll_pllglob;
	u32 per_pll_plldiv;
	u32 per_pll_plloutdiv;
	u32 spare_2[4];

	/* altera group */
	u32 alt_emacactr;
	u32 alt_emacbctr;
	u32 alt_emacptpctr;
	u32 alt_gpiodbctr;
	u32 alt_sdmmcctr;
	u32 alt_s2fuser0ctr;
	u32 alt_s2fuser1ctr;
	u32 alt_psirefctr;

	/* incoming clock */
	u32 hps_osc_clk_hz;
	u32 fpga_clk_hz;
	u32 spare_3[3];

	/* memory clock group */
	u32 mem_memdiv;
	u32 mem_pllglob;
	u32 mem_plldiv;
	u32 mem_plloutdiv;
	u32 spare_4[4];
};

/* Clock Manager registers */
#define CLKMGR_CTRL					0
#define CLKMGR_STAT					4
#define CLKMGR_TESTIOCTRL				8
#define CLKMGR_INTRGEN					0x0c
#define CLKMGR_INTRMSK					0x10
#define CLKMGR_INTRCLR					0x14
#define CLKMGR_INTRSTS					0x18
#define CLKMGR_INTRSTK					0x1c
#define CLKMGR_INTRRAW					0x20

/* Clock Manager Main PPL group registers */
#define CLKMGR_MAINPLL_EN				0x24
#define CLKMGR_MAINPLL_ENS				0x28
#define CLKMGR_MAINPLL_ENR				0x2c
#define CLKMGR_MAINPLL_BYPASS				0x30
#define CLKMGR_MAINPLL_BYPASSS				0x34
#define CLKMGR_MAINPLL_BYPASSR				0x38
#define CLKMGR_MAINPLL_MPUCLK				0x3c
#define CLKMGR_MAINPLL_NOCCLK				0x40
#define CLKMGR_MAINPLL_NOCDIV				0x44
#define CLKMGR_MAINPLL_PLLGLOB				0x48
#define CLKMGR_MAINPLL_PLLCTRL				0x4c
#define CLKMGR_MAINPLL_PLLDIV				0x50
#define CLKMGR_MAINPLL_PLLOUTDIV			0x54
#define CLKMGR_MAINPLL_LOSTLOCK				0x58

/* Clock Manager Peripheral PPL group registers */
#define CLKMGR_PERPLL_EN				0x7c
#define CLKMGR_PERPLL_ENS				0x80
#define CLKMGR_PERPLL_ENR				0x84
#define CLKMGR_PERPLL_BYPASS				0x88
#define CLKMGR_PERPLL_BYPASSS				0x8c
#define CLKMGR_PERPLL_BYPASSR				0x90
#define CLKMGR_PERPLL_EMACCTL				0x94
#define CLKMGR_PERPLL_GPIODIV				0x98
#define CLKMGR_PERPLL_PLLGLOB				0x9c
#define CLKMGR_PERPLL_PLLCTRL				0xa0
#define CLKMGR_PERPLL_PLLDIV				0xa4
#define CLKMGR_PERPLL_PLLOUTDIV				0xa8
#define CLKMGR_PERPLL_LOSTLOCK				0xac

/* Clock Manager Altera group registers */
#define CLKMGR_ALTR_EMACACTR				0xd4
#define CLKMGR_ALTR_EMACBCTR				0xd8
#define CLKMGR_ALTR_EMACPTPCTR				0xdc
#define CLKMGR_ALTR_GPIODBCTR				0xe0
#define CLKMGR_ALTR_SDMMCCTR				0xe4
#define CLKMGR_ALTR_S2FUSER0CTR				0xe8
#define CLKMGR_ALTR_S2FUSER1CTR				0xec
#define CLKMGR_ALTR_PSIREFCTR				0xf0
#define CLKMGR_ALTR_EXTCNTRST				0xf4

#define CLKMGR_CTRL_BOOTMODE				BIT(0)

#define CLKMGR_STAT_BUSY				BIT(0)
#define CLKMGR_STAT_MAINPLL_LOCKED			BIT(8)
#define CLKMGR_STAT_MAIN_TRANS				BIT(9)
#define CLKMGR_STAT_PERPLL_LOCKED			BIT(16)
#define CLKMGR_STAT_PERF_TRANS				BIT(17)
#define CLKMGR_STAT_BOOTMODE				BIT(24)
#define CLKMGR_STAT_BOOTCLKSRC				BIT(25)

#define CLKMGR_STAT_ALLPLL_LOCKED_MASK			\
	(CLKMGR_STAT_MAINPLL_LOCKED | CLKMGR_STAT_PERPLL_LOCKED)

#define CLKMGR_INTER_MAINPLLLOCKED_MASK			BIT(0)
#define CLKMGR_INTER_PERPLLLOCKED_MASK			BIT(1)
#define CLKMGR_INTER_MAINPLLLOST_MASK			BIT(2)
#define CLKMGR_INTER_PERPLLLOST_MASK			BIT(3)

#define CLKMGR_CLKSRC_MASK				GENMASK(18, 16)
#define CLKMGR_CLKSRC_OFFSET				16
#define CLKMGR_CLKSRC_MAIN				0
#define CLKMGR_CLKSRC_PER				1
#define CLKMGR_CLKSRC_OSC1				2
#define CLKMGR_CLKSRC_INTOSC				3
#define CLKMGR_CLKSRC_FPGA				4
#define CLKMGR_CLKCNT_MSK				GENMASK(10, 0)

#define CLKMGR_BYPASS_MAINPLL_ALL			0x7
#define CLKMGR_BYPASS_PERPLL_ALL			0x7f

#define CLKMGR_NOCDIV_L4MAIN_OFFSET			0
#define CLKMGR_NOCDIV_L4MPCLK_OFFSET			8
#define CLKMGR_NOCDIV_L4SPCLK_OFFSET			16
#define CLKMGR_NOCDIV_CSATCLK_OFFSET			24
#define CLKMGR_NOCDIV_CSTRACECLK_OFFSET			26
#define CLKMGR_NOCDIV_CSPDBGCLK_OFFSET			28
#define CLKMGR_NOCDIV_DIVIDER_MASK			0x3

#define CLKMGR_PLLGLOB_VCO_PSRC_MASK			GENMASK(17, 16)
#define CLKMGR_PLLGLOB_VCO_PSRC_OFFSET			16
#define CLKMGR_PLLGLOB_LOSTLOCK_BYPASS_EN_MASK		BIT(28)
#define CLKMGR_PLLGLOB_CLR_LOSTLOCK_BYPASS_MASK		BIT(29)

#define CLKMGR_VCO_PSRC_EOSC1				0
#define CLKMGR_VCO_PSRC_INTOSC				1
#define CLKMGR_VCO_PSRC_F2S				2

#define CLKMGR_PLLCTRL_BYPASS_MASK			BIT(0)
#define CLKMGR_PLLCTRL_RST_N_MASK			BIT(1)

#define CLKMGR_PLLDIV_REFCLKDIV_MASK			GENMASK(5, 0)
#define CLKMGR_PLLDIV_FDIV_MASK				GENMASK(16, 8)
#define CLKMGR_PLLDIV_OUTDIV_QDIV_MASK			GENMASK(26, 24)
#define CLKMGR_PLLDIV_RANGE_MASK			GENMASK(30, 28)

#define CLKMGR_PLLDIV_REFCLKDIV_OFFSET			0
#define CLKMGR_PLLDIV_FDIV_OFFSET			8
#define CLKMGR_PLLDIV_OUTDIV_QDIV_OFFSET		24
#define CLKMGR_PLLDIV_RANGE_OFFSET			28

#define CLKMGR_PLLOUTDIV_C0CNT_MASK			GENMASK(4, 0)
#define CLKMGR_PLLOUTDIV_C1CNT_MASK			GENMASK(12, 8)
#define CLKMGR_PLLOUTDIV_C2CNT_MASK			GENMASK(20, 16)
#define CLKMGR_PLLOUTDIV_C3CNT_MASK			GENMASK(28, 24)

#define CLKMGR_PLLOUTDIV_C0CNT_OFFSET			0
#define CLKMGR_PLLOUTDIV_C1CNT_OFFSET			8
#define CLKMGR_PLLOUTDIV_C2CNT_OFFSET			16
#define CLKMGR_PLLOUTDIV_C3CNT_OFFSET			24

#define CLKMGR_PLLCX_EN_SET_MSK				BIT(27)
#define CLKMGR_PLLCX_MUTE_SET_MSK			BIT(28)

#define CLKMGR_VCOCALIB_MSCNT_MASK			GENMASK(23, 16)
#define CLKMGR_VCOCALIB_MSCNT_OFFSET			16
#define CLKMGR_VCOCALIB_HSCNT_MASK			GENMASK(9, 0)
#define CLKMGR_VCOCALIB_MSCNT_CONST			100
#define CLKMGR_VCOCALIB_HSCNT_CONST			4

#define CLKMGR_PLLM_MDIV_MASK				GENMASK(9, 0)

#define CLKMGR_LOSTLOCK_SET_MASK			BIT(0)

#define CLKMGR_PERPLLGRP_EN_SDMMCCLK_MASK		BIT(5)
#define CLKMGR_PERPLLGRP_EMACCTL_EMAC0SELB_OFFSET	26
#define CLKMGR_PERPLLGRP_EMACCTL_EMAC0SELB_MASK		BIT(26)
#define CLKMGR_PERPLLGRP_EMACCTL_EMAC1SELB_OFFSET	27
#define CLKMGR_PERPLLGRP_EMACCTL_EMAC1SELB_MASK		BIT(27)
#define CLKMGR_PERPLLGRP_EMACCTL_EMAC2SELB_OFFSET	28
#define CLKMGR_PERPLLGRP_EMACCTL_EMAC2SELB_MASK		BIT(28)

#define CLKMGR_ALT_EMACCTR_SRC_OFFSET			16
#define CLKMGR_ALT_EMACCTR_SRC_MASK			GENMASK(18, 16)
#define CLKMGR_ALT_EMACCTR_CNT_OFFSET			0
#define CLKMGR_ALT_EMACCTR_CNT_MASK			GENMASK(10, 0)

#define CLKMGR_ALT_EXTCNTRST_ALLCNTRST_MASK		GENMASK(15, 0)

#endif /* _CLK_N5X_ */
