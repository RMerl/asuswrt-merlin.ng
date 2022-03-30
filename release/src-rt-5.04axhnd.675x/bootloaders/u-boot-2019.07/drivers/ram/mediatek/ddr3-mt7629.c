// SPDX-License-Identifier: GPL-2.0
/*
 * MediaTek DDR3 driver for MT7629 SoC
 *
 * Copyright (C) 2018 MediaTek Inc.
 * Author: Wu Zou <wu.zou@mediatek.com>
 *	   Ryder Lee <ryder.lee@mediatek.com>
 */

#include <clk.h>
#include <common.h>
#include <dm.h>
#include <ram.h>
#include <asm/io.h>

/* EMI */
#define EMI_CONA			0x000
#define EMI_CONF			0x028
#define EMI_CONM			0x060

/* DDR PHY */
#define DDRPHY_PLL1			0x0000
#define DDRPHY_PLL2			0x0004
#define DDRPHY_PLL3			0x0008
#define DDRPHY_PLL4			0x000c
#define DDRPHY_PLL5			0x0010
#define DDRPHY_PLL7			0x0018
#define DDRPHY_B0_DLL_ARPI0		0x0080
#define DDRPHY_B0_DLL_ARPI1		0x0084
#define DDRPHY_B0_DLL_ARPI2		0x0088
#define DDRPHY_B0_DLL_ARPI3		0x008c
#define DDRPHY_B0_DLL_ARPI4		0x0090
#define DDRPHY_B0_DLL_ARPI5		0x0094
#define DDRPHY_B0_DQ2			0x00a0
#define DDRPHY_B0_DQ3			0x00a4
#define DDRPHY_B0_DQ4			0x00a8
#define DDRPHY_B0_DQ5			0x00ac
#define DDRPHY_B0_DQ6			0x00b0
#define DDRPHY_B0_DQ7			0x00b4
#define DDRPHY_B0_DQ8			0x00b8
#define DDRPHY_B1_DLL_ARPI0		0x0100
#define DDRPHY_B1_DLL_ARPI1		0x0104
#define DDRPHY_B1_DLL_ARPI2		0x0108
#define DDRPHY_B1_DLL_ARPI3		0x010c
#define DDRPHY_B1_DLL_ARPI4		0x0110
#define DDRPHY_B1_DLL_ARPI5		0x0114
#define DDRPHY_B1_DQ2			0x0120
#define DDRPHY_B1_DQ3			0x0124
#define DDRPHY_B1_DQ4			0x0128
#define DDRPHY_B1_DQ5			0x012c
#define DDRPHY_B1_DQ6			0x0130
#define DDRPHY_B1_DQ7			0x0134
#define DDRPHY_B1_DQ8			0x0138
#define DDRPHY_CA_DLL_ARPI0		0x0180
#define DDRPHY_CA_DLL_ARPI1		0x0184
#define DDRPHY_CA_DLL_ARPI2		0x0188
#define DDRPHY_CA_DLL_ARPI3		0x018c
#define DDRPHY_CA_DLL_ARPI4		0x0190
#define DDRPHY_CA_DLL_ARPI5		0x0194
#define DDRPHY_CA_CMD2			0x01a0
#define DDRPHY_CA_CMD3			0x01a4
#define DDRPHY_CA_CMD5			0x01ac
#define DDRPHY_CA_CMD6			0x01b0
#define DDRPHY_CA_CMD7			0x01b4
#define DDRPHY_CA_CMD8			0x01b8
#define DDRPHY_MISC_VREF_CTRL		0x0264
#define DDRPHY_MISC_IMP_CTRL0		0x0268
#define DDRPHY_MISC_IMP_CTRL1		0x026c
#define DDRPHY_MISC_SHU_OPT		0x0270
#define DDRPHY_MISC_SPM_CTRL0		0x0274
#define DDRPHY_MISC_SPM_CTRL1		0x0278
#define DDRPHY_MISC_SPM_CTRL2		0x027c
#define DDRPHY_MISC_CG_CTRL0		0x0284
#define DDRPHY_MISC_CG_CTRL1		0x0288
#define DDRPHY_MISC_CG_CTRL2		0x028c
#define DDRPHY_MISC_CG_CTRL4		0x0294
#define DDRPHY_MISC_CTRL0		0x029c
#define DDRPHY_MISC_CTRL1		0x02a0
#define DDRPHY_MISC_CTRL3		0x02a8
#define DDRPHY_MISC_RXDVS1		0x05e4
#define DDRPHY_SHU1_B0_DQ4		0x0c10
#define DDRPHY_SHU1_B0_DQ5		0x0c14
#define DDRPHY_SHU1_B0_DQ6		0x0c18
#define DDRPHY_SHU1_B0_DQ7		0x0c1c
#define DDRPHY_SHU1_B1_DQ4		0x0c90
#define DDRPHY_SHU1_B1_DQ5		0x0c94
#define DDRPHY_SHU1_B1_DQ6		0x0c98
#define DDRPHY_SHU1_B1_DQ7		0x0c9c
#define DDRPHY_SHU1_CA_CMD2		0x0d08
#define DDRPHY_SHU1_CA_CMD4		0x0d10
#define DDRPHY_SHU1_CA_CMD5		0x0d14
#define DDRPHY_SHU1_CA_CMD6		0x0d18
#define DDRPHY_SHU1_CA_CMD7		0x0d1c
#define DDRPHY_SHU1_PLL0		0x0d80
#define DDRPHY_SHU1_PLL1		0x0d84
#define DDRPHY_SHU1_PLL4		0x0d90
#define DDRPHY_SHU1_PLL5		0x0d94
#define DDRPHY_SHU1_PLL6		0x0d98
#define DDRPHY_SHU1_PLL7		0x0d9C
#define DDRPHY_SHU1_PLL8		0x0da0
#define DDRPHY_SHU1_PLL9		0x0da4
#define DDRPHY_SHU1_PLL10		0x0da8
#define DDRPHY_SHU1_PLL11		0x0dac
#define DDRPHY_SHU1_R0_B0_DQ2		0x0e08
#define DDRPHY_SHU1_R0_B0_DQ3		0x0e0c
#define DDRPHY_SHU1_R0_B0_DQ4		0x0e10
#define DDRPHY_SHU1_R0_B0_DQ5		0x0e14
#define DDRPHY_SHU1_R0_B0_DQ6		0x0e18
#define DDRPHY_SHU1_R0_B0_DQ7		0x0e1c
#define DDRPHY_SHU1_R0_B1_DQ2		0x0e58
#define DDRPHY_SHU1_R0_B1_DQ3		0x0e5c
#define DDRPHY_SHU1_R0_B1_DQ4		0x0e60
#define DDRPHY_SHU1_R0_B1_DQ5		0x0e64
#define DDRPHY_SHU1_R0_B1_DQ6		0x0e68
#define DDRPHY_SHU1_R0_B1_DQ7		0x0e6c
#define DDRPHY_SHU1_R0_CA_CMD9		0x0ec4
#define DDRPHY_SHU1_R1_B0_DQ2		0x0f08
#define DDRPHY_SHU1_R1_B0_DQ3		0x0f0c
#define DDRPHY_SHU1_R1_B0_DQ4		0x0f10
#define DDRPHY_SHU1_R1_B0_DQ5		0x0f14
#define DDRPHY_SHU1_R1_B0_DQ6		0x0f18
#define DDRPHY_SHU1_R1_B0_DQ7		0x0f1c
#define DDRPHY_SHU1_R1_B1_DQ2		0x0f58
#define DDRPHY_SHU1_R1_B1_DQ3		0x0f5c
#define DDRPHY_SHU1_R1_B1_DQ4		0x0f60
#define DDRPHY_SHU1_R1_B1_DQ5		0x0f64
#define DDRPHY_SHU1_R1_B1_DQ6		0x0f68
#define DDRPHY_SHU1_R1_B1_DQ7		0x0f6c
#define DDRPHY_SHU1_R1_CA_CMD9		0x0fc4

/* DRAMC */
#define DRAMC_DDRCONF0			0x0000
#define DRAMC_DRAMCTRL			0x0004
#define DRAMC_MISCTL0			0x0008
#define DRAMC_PERFCTL0			0x000c
#define DRAMC_ARBCTL			0x0010
#define DRAMC_RSTMASK			0x001c
#define DRAMC_PADCTRL			0x0020
#define DRAMC_CKECTRL			0x0024
#define DRAMC_RKCFG			0x0034
#define DRAMC_DRAMC_PD_CTRL		0x0038
#define DRAMC_CLKAR			0x003c
#define DRAMC_CLKCTRL			0x0040
#define DRAMC_SREFCTRL			0x0048
#define DRAMC_REFCTRL0			0x004c
#define DRAMC_REFCTRL1			0x0050
#define DRAMC_REFRATRE_FILTER		0x0054
#define DRAMC_ZQCS			0x0058
#define DRAMC_MRS			0x005c
#define DRAMC_SPCMD			0x0060
#define DRAMC_SPCMDCTRL			0x0064
#define DRAMC_HW_MRR_FUN		0x0074
#define DRAMC_TEST2_1			0x0094
#define DRAMC_TEST2_2			0x0098
#define DRAMC_TEST2_3			0x009c
#define DRAMC_TEST2_4			0x00a0
#define DRAMC_CATRAINING1		0x00b0
#define DRAMC_DUMMY_RD			0x00d0
#define DRAMC_SHUCTRL			0x00d4
#define DRAMC_SHUCTRL2			0x00dc
#define DRAMC_STBCAL			0x0200
#define DRAMC_STBCAL1			0x0204
#define DRAMC_EYESCAN			0x020c
#define DRAMC_DVFSDLL			0x0210
#define DRAMC_SHU_ACTIM0		0x0800
#define DRAMC_SHU_ACTIM1		0x0804
#define DRAMC_SHU_ACTIM2		0x0808
#define DRAMC_SHU_ACTIM3		0x080c
#define DRAMC_SHU_ACTIM4		0x0810
#define DRAMC_SHU_ACTIM5		0x0814
#define DRAMC_SHU_ACTIM_XRT		0x081c
#define DRAMC_SHU_AC_TIME_05T		0x0820
#define DRAMC_SHU_CONF0			0x0840
#define DRAMC_SHU_CONF1			0x0844
#define DRAMC_SHU_CONF2			0x0848
#define DRAMC_SHU_CONF3			0x084c
#define DRAMC_SHU_RANKCTL		0x0858
#define DRAMC_SHU_CKECTRL		0x085c
#define DRAMC_SHU_ODTCTRL		0x0860
#define DRAMC_SHU_PIPE			0x0878
#define DRAMC_SHU_SELPH_CA1		0x0880
#define DRAMC_SHU_SELPH_CA2		0x0884
#define DRAMC_SHU_SELPH_CA3		0x0888
#define DRAMC_SHU_SELPH_CA4		0x088c
#define DRAMC_SHU_SELPH_CA5		0x0890
#define DRAMC_SHU_SELPH_CA6		0x0894
#define DRAMC_SHU_SELPH_CA7		0x0898
#define DRAMC_SHU_SELPH_CA8		0x089c
#define DRAMC_SHU_SELPH_DQS0		0x08a0
#define DRAMC_SHU_SELPH_DQS1		0x08a4
#define DRAMC_SHU1_DRVING1		0x08a8
#define DRAMC_SHU1_DRVING2		0x08ac
#define DRAMC_SHU1_WODT			0x08c0
#define DRAMC_SHU_SCINTV		0x08c8
#define DRAMC_SHURK0_DQSCTL		0x0a00
#define DRAMC_SHURK0_DQSIEN		0x0a04
#define DRAMC_SHURK0_SELPH_ODTEN0	0x0a1c
#define DRAMC_SHURK0_SELPH_ODTEN1	0x0a20
#define DRAMC_SHURK0_SELPH_DQSG0	0x0a24
#define DRAMC_SHURK0_SELPH_DQSG1	0x0a28
#define DRAMC_SHURK0_SELPH_DQ0		0x0a2c
#define DRAMC_SHURK0_SELPH_DQ1		0x0a30
#define DRAMC_SHURK0_SELPH_DQ2		0x0a34
#define DRAMC_SHURK0_SELPH_DQ3		0x0a38
#define DRAMC_SHURK1_DQSCTL		0x0b00
#define DRAMC_SHURK1_SELPH_ODTEN0	0x0b1c
#define DRAMC_SHURK1_SELPH_ODTEN1	0x0b20
#define DRAMC_SHURK1_SELPH_DQSG0	0x0b24
#define DRAMC_SHURK1_SELPH_DQSG1	0x0b28
#define DRAMC_SHURK1_SELPH_DQ0		0x0b2c
#define DRAMC_SHURK1_SELPH_DQ1		0x0b30
#define DRAMC_SHURK1_SELPH_DQ2		0x0b34
#define DRAMC_SHURK1_SELPH_DQ3		0x0b38
#define DRAMC_SHURK2_SELPH_ODTEN0	0x0c1c
#define DRAMC_SHURK2_SELPH_ODTEN1	0x0c20
#define DRAMC_SHU_DQSG_RETRY		0x0c54

#define EMI_COL_ADDR_MASK		GENMASK(13, 12)
#define EMI_COL_ADDR_SHIFT		12
#define WALKING_PATTERN			0x12345678
#define WALKING_STEP			0x4000000

struct mtk_ddr3_priv {
	fdt_addr_t emi;
	fdt_addr_t ddrphy;
	fdt_addr_t dramc_ao;
	struct clk phy;
	struct clk phy_mux;
	struct clk mem;
	struct clk mem_mux;
};

#ifdef CONFIG_SPL_BUILD
static int mtk_ddr3_rank_size_detect(struct udevice *dev)
{
	struct mtk_ddr3_priv *priv = dev_get_priv(dev);
	int step;
	u32 start, test;

	/* To detect size, we have to make sure it's single rank
	 * and it has maximum addressing region
	 */

	writel(WALKING_PATTERN, CONFIG_SYS_SDRAM_BASE);

	if (readl(CONFIG_SYS_SDRAM_BASE) != WALKING_PATTERN)
		return -EINVAL;

	for (step = 0; step < 5; step++) {
		writel(~WALKING_PATTERN, CONFIG_SYS_SDRAM_BASE +
		       (WALKING_STEP << step));

		start = readl(CONFIG_SYS_SDRAM_BASE);
		test = readl(CONFIG_SYS_SDRAM_BASE + (WALKING_STEP << step));
		if ((test != ~WALKING_PATTERN) || test == start)
			break;
	}

	step = step ? step - 1 : 3;
	clrsetbits_le32(priv->emi + EMI_CONA, EMI_COL_ADDR_MASK,
			step << EMI_COL_ADDR_SHIFT);

	return 0;
}

static int mtk_ddr3_init(struct udevice *dev)
{
	struct mtk_ddr3_priv *priv = dev_get_priv(dev);
	int ret;

	ret = clk_set_parent(&priv->phy, &priv->phy_mux);
	if (ret)
		return ret;

	/* EMI Setting */
	writel(0x00003010, priv->emi + EMI_CONA);
	writel(0x00000000, priv->emi + EMI_CONF);
	writel(0x000006b8, priv->emi + EMI_CONM);
	/* DQS */
	writel(0x20c00, priv->dramc_ao + DRAMC_SHU1_DRVING1);
	/* Clock */
	writel(0x8320c83, priv->dramc_ao + DRAMC_SHU1_DRVING2);

	/* DDRPHY setting */
	writel(0x2201, priv->dramc_ao + DRAMC_DRAMCTRL);
	writel(0x3000000c, priv->dramc_ao + DRAMC_CLKCTRL);
	writel(0xe08, priv->ddrphy + DDRPHY_CA_CMD5);
	writel(0x60e, priv->ddrphy + DDRPHY_SHU1_CA_CMD5);
	writel(0x0, priv->ddrphy + DDRPHY_MISC_SPM_CTRL1);
	writel(0xffffffff, priv->ddrphy + DDRPHY_MISC_SPM_CTRL0);
	writel(0xffffffff, priv->ddrphy + DDRPHY_MISC_SPM_CTRL2);
	writel(0x6003bf, priv->ddrphy + DDRPHY_MISC_CG_CTRL2);
	writel(0x13300000, priv->ddrphy + DDRPHY_MISC_CG_CTRL4);

	writel(0x1, priv->ddrphy + DDRPHY_SHU1_CA_CMD7);
	writel(0x21, priv->ddrphy + DDRPHY_SHU1_B0_DQ7);
	writel(0x1, priv->ddrphy + DDRPHY_SHU1_B1_DQ7);
	writel(0xfff0, priv->ddrphy + DDRPHY_CA_CMD2);
	writel(0x0, priv->ddrphy + DDRPHY_B0_DQ2);
	writel(0x0, priv->ddrphy + DDRPHY_B1_DQ2);
	writel(0x7, priv->ddrphy + DDRPHY_MISC_RXDVS1);
	writel(0x10, priv->ddrphy + DDRPHY_PLL3);
	writel(0x8e8e0000, priv->ddrphy + DDRPHY_MISC_VREF_CTRL);
	writel(0x2e0040, priv->ddrphy + DDRPHY_MISC_IMP_CTRL0);
	writel(0x50060e, priv->ddrphy + DDRPHY_SHU1_B0_DQ5);
	writel(0x50060e, priv->ddrphy + DDRPHY_SHU1_B1_DQ5);
	udelay(1);

	writel(0x10, priv->ddrphy + DDRPHY_B0_DQ3);
	writel(0x10, priv->ddrphy + DDRPHY_B1_DQ3);
	writel(0x3f600, priv->ddrphy + DDRPHY_MISC_CG_CTRL1);
	writel(0x1010, priv->ddrphy + DDRPHY_B0_DQ4);
	writel(0x1110e0e, priv->ddrphy + DDRPHY_B0_DQ5);
	writel(0x10c10d0, priv->ddrphy + DDRPHY_B0_DQ6);
	writel(0x3110e0e, priv->ddrphy + DDRPHY_B0_DQ5);
	writel(0x1010, priv->ddrphy + DDRPHY_B1_DQ4);
	writel(0x1110e0e, priv->ddrphy + DDRPHY_B1_DQ5);
	writel(0x10c10d0, priv->ddrphy + DDRPHY_B1_DQ6);
	writel(0x3110e0e, priv->ddrphy + DDRPHY_B1_DQ5);
	writel(0x7fffffc, priv->ddrphy + DDRPHY_CA_CMD3);
	writel(0xc0010, priv->ddrphy + DDRPHY_CA_CMD6);
	writel(0x101, priv->ddrphy + DDRPHY_SHU1_CA_CMD2);
	writel(0x41e, priv->ddrphy + DDRPHY_B0_DQ3);
	writel(0x41e, priv->ddrphy + DDRPHY_B1_DQ3);
	writel(0x180101, priv->ddrphy + DDRPHY_CA_CMD8);
	writel(0x0, priv->ddrphy + DDRPHY_MISC_IMP_CTRL1);
	writel(0x11400000, priv->ddrphy + DDRPHY_MISC_CG_CTRL4);
	writel(0xfff0f0f0, priv->ddrphy + DDRPHY_MISC_SHU_OPT);
	writel(0x1f, priv->ddrphy + DDRPHY_MISC_CG_CTRL0);

	writel(0x0, priv->ddrphy + DDRPHY_SHU1_CA_CMD6);
	writel(0x0, priv->ddrphy + DDRPHY_SHU1_B0_DQ6);
	writel(0x0, priv->ddrphy + DDRPHY_SHU1_B1_DQ6);
	writel(0x40000, priv->ddrphy + DDRPHY_PLL4);
	writel(0x0, priv->ddrphy + DDRPHY_PLL1);
	writel(0x0, priv->ddrphy + DDRPHY_PLL2);
	writel(0x666008, priv->ddrphy + DDRPHY_CA_DLL_ARPI5);
	writel(0x80666008, priv->ddrphy + DDRPHY_B0_DLL_ARPI5);
	writel(0x80666008, priv->ddrphy + DDRPHY_B1_DLL_ARPI5);
	writel(0x0, priv->ddrphy + DDRPHY_CA_DLL_ARPI0);
	writel(0x0, priv->ddrphy + DDRPHY_B0_DLL_ARPI0);
	writel(0x0, priv->ddrphy + DDRPHY_B1_DLL_ARPI0);
	writel(0x400, priv->ddrphy + DDRPHY_CA_DLL_ARPI2);
	writel(0x20400, priv->ddrphy + DDRPHY_B0_DLL_ARPI2);
	writel(0x20400, priv->ddrphy + DDRPHY_B1_DLL_ARPI2);
	writel(0x0, priv->ddrphy + DDRPHY_SHU1_PLL9);
	writel(0x0, priv->ddrphy + DDRPHY_SHU1_PLL11);
	writel(0xf7f, priv->ddrphy + DDRPHY_SHU1_PLL0);
	writel(0x40000, priv->ddrphy + DDRPHY_SHU1_PLL8);
	writel(0x40000, priv->ddrphy + DDRPHY_SHU1_PLL10);
	writel(0xe57800fe, priv->ddrphy + DDRPHY_SHU1_PLL4);
	writel(0xe57800fe, priv->ddrphy + DDRPHY_SHU1_PLL6);

	writel(0xB5000000, priv->ddrphy + DDRPHY_SHU1_PLL5);
	writel(0xB5000000, priv->ddrphy + DDRPHY_SHU1_PLL7);

	writel(0x14d0002, priv->ddrphy + DDRPHY_PLL5);
	writel(0x14d0002, priv->ddrphy + DDRPHY_PLL7);
	writel(0x80040000, priv->ddrphy + DDRPHY_SHU1_PLL8);
	writel(0x80040000, priv->ddrphy + DDRPHY_SHU1_PLL10);
	writel(0xf, priv->ddrphy + DDRPHY_SHU1_PLL1);
	writel(0x4, priv->ddrphy + DDRPHY_CA_DLL_ARPI0);
	writel(0x1, priv->ddrphy + DDRPHY_B0_DLL_ARPI0);
	writel(0x1, priv->ddrphy + DDRPHY_B1_DLL_ARPI0);
	writel(0x698600, priv->ddrphy + DDRPHY_CA_DLL_ARPI5);
	writel(0xc0778600, priv->ddrphy + DDRPHY_B0_DLL_ARPI5);
	writel(0xc0778600, priv->ddrphy + DDRPHY_B1_DLL_ARPI5);
	writel(0x0, priv->ddrphy + DDRPHY_CA_DLL_ARPI4);
	writel(0x0, priv->ddrphy + DDRPHY_B0_DLL_ARPI4);
	writel(0x0, priv->ddrphy + DDRPHY_B1_DLL_ARPI4);
	writel(0x2ba800, priv->ddrphy + DDRPHY_CA_DLL_ARPI1);
	writel(0x2ae806, priv->ddrphy + DDRPHY_B0_DLL_ARPI1);
	writel(0xae806, priv->ddrphy + DDRPHY_B1_DLL_ARPI1);
	writel(0xba000, priv->ddrphy + DDRPHY_CA_DLL_ARPI3);
	writel(0x2e800, priv->ddrphy + DDRPHY_B0_DLL_ARPI3);
	writel(0x2e800, priv->ddrphy + DDRPHY_B1_DLL_ARPI3);
	writel(0x0, priv->ddrphy + DDRPHY_SHU1_CA_CMD4);
	writel(0x0, priv->ddrphy + DDRPHY_SHU1_B0_DQ4);
	writel(0x0, priv->ddrphy + DDRPHY_SHU1_B1_DQ4);
	writel(0x4, priv->ddrphy + DDRPHY_CA_DLL_ARPI0);
	writel(0x1, priv->ddrphy + DDRPHY_B0_DLL_ARPI0);
	writel(0x1, priv->ddrphy + DDRPHY_B1_DLL_ARPI0);
	writel(0x32cf0000, priv->ddrphy + DDRPHY_SHU1_CA_CMD6);
	writel(0x32cd0000, priv->ddrphy + DDRPHY_SHU1_B0_DQ6);
	writel(0x32cd0000, priv->ddrphy + DDRPHY_SHU1_B1_DQ6);
	writel(0x80010000, priv->ddrphy + DDRPHY_PLL1);
	writel(0x80000000, priv->ddrphy + DDRPHY_PLL2);
	udelay(100);

	writel(0xc, priv->ddrphy + DDRPHY_CA_DLL_ARPI0);
	writel(0x9, priv->ddrphy + DDRPHY_B0_DLL_ARPI0);
	writel(0x9, priv->ddrphy + DDRPHY_B1_DLL_ARPI0);
	writel(0xd0000, priv->ddrphy + DDRPHY_PLL4);
	udelay(1);

	writel(0x82, priv->ddrphy + DDRPHY_MISC_CTRL1);
	writel(0x2, priv->dramc_ao + DRAMC_DDRCONF0);
	writel(0x3acf0000, priv->ddrphy + DDRPHY_SHU1_CA_CMD6);
	writel(0x3acd0000, priv->ddrphy + DDRPHY_SHU1_B0_DQ6);
	writel(0x3acd0000, priv->ddrphy + DDRPHY_SHU1_B1_DQ6);
	udelay(1);

	writel(0x0, priv->ddrphy + DDRPHY_CA_DLL_ARPI2);
	writel(0x0, priv->ddrphy + DDRPHY_B0_DLL_ARPI2);
	writel(0x0, priv->ddrphy + DDRPHY_B1_DLL_ARPI2);
	writel(0x80, priv->ddrphy + DDRPHY_MISC_CTRL1);
	writel(0x0, priv->dramc_ao + DRAMC_DDRCONF0);
	writel(0x80000000, priv->ddrphy + DDRPHY_PLL1);
	udelay(1);

	writel(0x698e00, priv->ddrphy + DDRPHY_CA_DLL_ARPI5);
	udelay(1);

	writel(0xc0778e00, priv->ddrphy + DDRPHY_B0_DLL_ARPI5);
	udelay(1);

	writel(0xc0778e00, priv->ddrphy + DDRPHY_B1_DLL_ARPI5);
	udelay(1);

	ret = clk_set_parent(&priv->mem, &priv->mem_mux);
	if (ret)
		return ret;

	/* DDR PHY PLL setting */
	writel(0x51e, priv->ddrphy + DDRPHY_B0_DQ3);
	writel(0x51e, priv->ddrphy + DDRPHY_B1_DQ3);
	writel(0x8100008c, priv->ddrphy + DDRPHY_MISC_CTRL1);
	writel(0x80101, priv->ddrphy + DDRPHY_CA_CMD8);
	writel(0x100, priv->ddrphy + DDRPHY_CA_CMD7);
	writel(0x0, priv->ddrphy + DDRPHY_CA_CMD7);
	writel(0x0, priv->ddrphy + DDRPHY_B0_DQ7);
	writel(0x0, priv->ddrphy + DDRPHY_B1_DQ7);
	writel(0x51e, priv->ddrphy + DDRPHY_B0_DQ3);
	writel(0xff051e, priv->ddrphy + DDRPHY_B1_DQ3);
	writel(0x0, priv->ddrphy + DDRPHY_B0_DQ2);
	writel(0x1ff, priv->ddrphy + DDRPHY_B1_DQ2);

	/* Update initial setting */
	writel(0x5fc, priv->ddrphy + DDRPHY_B0_DQ3);
	writel(0xff05fc, priv->ddrphy + DDRPHY_B1_DQ3);
	writel(0x10c12d9, priv->ddrphy + DDRPHY_B0_DQ6);
	writel(0x10c12d9, priv->ddrphy + DDRPHY_B1_DQ6);
	writel(0xc0259, priv->ddrphy + DDRPHY_CA_CMD6);
	writel(0x4000, priv->ddrphy + DDRPHY_B0_DQ2);
	writel(0x41ff, priv->ddrphy + DDRPHY_B1_DQ2);
	writel(0x0, priv->ddrphy + DDRPHY_B0_DQ8);
	writel(0x100, priv->ddrphy + DDRPHY_B1_DQ8);
	writel(0x3110e0e, priv->ddrphy + DDRPHY_B0_DQ5);
	writel(0x3110e0e, priv->ddrphy + DDRPHY_B1_DQ5);
	writel(0x51060e, priv->ddrphy + DDRPHY_SHU1_B0_DQ5);
	writel(0x51060e, priv->ddrphy + DDRPHY_SHU1_B1_DQ5);
	writel(0x39eff6, priv->dramc_ao + DRAMC_SHU_SCINTV);
	writel(0x204ffff, priv->dramc_ao + DRAMC_CLKAR);
	writel(0x31b1f1cf, priv->dramc_ao + DRAMC_SPCMDCTRL);
	writel(0x0, priv->dramc_ao + DRAMC_PERFCTL0);
	writel(0x80000, priv->dramc_ao + DRAMC_PERFCTL0);

	/* Dramc setting PC3 */
	writel(0x65714001, priv->dramc_ao + DRAMC_REFCTRL0);

	writel(0x11351131, priv->ddrphy + DDRPHY_MISC_CTRL3);
	writel(0x200600, priv->dramc_ao + DRAMC_SHU_DQSG_RETRY);
	writel(0x101d007, priv->dramc_ao + DRAMC_SHUCTRL2);
	writel(0xe090601, priv->dramc_ao + DRAMC_DVFSDLL);
	writel(0x20003000, priv->dramc_ao + DRAMC_DDRCONF0);
	writel(0x3900020f, priv->ddrphy + DDRPHY_MISC_CTRL0);
	writel(0xa20810bf, priv->dramc_ao + DRAMC_SHU_CONF0);
	writel(0x30050, priv->dramc_ao + DRAMC_SHU_ODTCTRL);
	writel(0x25712000, priv->dramc_ao + DRAMC_REFCTRL0);
	writel(0xb0100000, priv->dramc_ao + DRAMC_STBCAL);
	writel(0x8000000, priv->dramc_ao + DRAMC_SREFCTRL);
	writel(0xc0000000, priv->dramc_ao + DRAMC_SHU_PIPE);
	writel(0x731004, priv->dramc_ao + DRAMC_RKCFG);
	writel(0x8007320f, priv->dramc_ao + DRAMC_SHU_CONF2);
	writel(0x2a7c0, priv->dramc_ao + DRAMC_SHU_SCINTV);
	writel(0xc110, priv->dramc_ao + DRAMC_SHUCTRL);
	writel(0x30000700, priv->dramc_ao + DRAMC_REFCTRL1);
	writel(0x6543b321, priv->dramc_ao + DRAMC_REFRATRE_FILTER);

	/* Update PCDDR3 default setting */
	writel(0x0, priv->dramc_ao + DRAMC_SHU_SELPH_CA1);
	writel(0x0, priv->dramc_ao + DRAMC_SHU_SELPH_CA2);
	writel(0x0, priv->dramc_ao + DRAMC_SHU_SELPH_CA3);
	writel(0x0, priv->dramc_ao + DRAMC_SHU_SELPH_CA4);
	writel(0x10000111, priv->dramc_ao + DRAMC_SHU_SELPH_CA5);
	writel(0x1000000, priv->dramc_ao + DRAMC_SHU_SELPH_CA6);
	writel(0x0, priv->dramc_ao + DRAMC_SHU_SELPH_CA7);
	writel(0x0, priv->dramc_ao + DRAMC_SHU_SELPH_CA8);
	writel(0x0, priv->ddrphy + DDRPHY_SHU1_R0_CA_CMD9);
	writel(0x0, priv->ddrphy + DDRPHY_SHU1_R1_CA_CMD9);
	writel(0x11112222, priv->dramc_ao + DRAMC_SHU_SELPH_DQS0);
	writel(0x33331111, priv->dramc_ao + DRAMC_SHU_SELPH_DQS1);
	writel(0x11112222, priv->dramc_ao + DRAMC_SHURK0_SELPH_DQ0);
	writel(0x11112222, priv->dramc_ao + DRAMC_SHURK0_SELPH_DQ1);
	writel(0x33331111, priv->dramc_ao + DRAMC_SHURK0_SELPH_DQ2);
	writel(0x33331111, priv->dramc_ao + DRAMC_SHURK0_SELPH_DQ3);
	writel(0x11112222, priv->dramc_ao + DRAMC_SHURK1_SELPH_DQ0);
	writel(0x11112222, priv->dramc_ao + DRAMC_SHURK1_SELPH_DQ1);
	writel(0x33331111, priv->dramc_ao + DRAMC_SHURK1_SELPH_DQ2);
	writel(0x33331111, priv->dramc_ao + DRAMC_SHURK1_SELPH_DQ3);
	writel(0xf0f00, priv->ddrphy + DDRPHY_SHU1_R0_B0_DQ7);
	writel(0xf0f00, priv->ddrphy + DDRPHY_SHU1_R1_B0_DQ7);
	writel(0xf0f00, priv->ddrphy + DDRPHY_SHU1_R0_B1_DQ7);
	writel(0xf0f00, priv->ddrphy + DDRPHY_SHU1_R1_B1_DQ7);
	writel(0x0, priv->dramc_ao + DRAMC_SHURK0_SELPH_ODTEN0);
	writel(0x0, priv->dramc_ao + DRAMC_SHURK0_SELPH_ODTEN1);
	writel(0x0, priv->dramc_ao + DRAMC_SHURK1_SELPH_ODTEN0);
	writel(0x0, priv->dramc_ao + DRAMC_SHURK1_SELPH_ODTEN1);
	writel(0x0, priv->dramc_ao + DRAMC_SHURK2_SELPH_ODTEN0);
	writel(0x66666666, priv->dramc_ao + DRAMC_SHURK2_SELPH_ODTEN1);
	writel(0x2c000b0f, priv->dramc_ao + DRAMC_SHU_CONF1);
	writel(0x11111111, priv->dramc_ao + DRAMC_SHURK0_SELPH_DQSG0);
	writel(0x64646464, priv->dramc_ao + DRAMC_SHURK0_SELPH_DQSG1);
	writel(0x11111111, priv->dramc_ao + DRAMC_SHURK1_SELPH_DQSG0);
	writel(0x64646464, priv->dramc_ao + DRAMC_SHURK1_SELPH_DQSG1);
	writel(0xc0c0c0c, priv->ddrphy + DDRPHY_SHU1_R0_B0_DQ2);
	writel(0xc0c0c0c, priv->ddrphy + DDRPHY_SHU1_R0_B0_DQ3);
	writel(0xc0c0c0c, priv->ddrphy + DDRPHY_SHU1_R0_B0_DQ4);
	writel(0xc0c0c0c, priv->ddrphy + DDRPHY_SHU1_R0_B0_DQ5);
	writel(0x0, priv->ddrphy + DDRPHY_SHU1_R0_B0_DQ6);
	writel(0xc0c0c0c, priv->ddrphy + DDRPHY_SHU1_R1_B0_DQ2);
	writel(0xc0c0c0c, priv->ddrphy + DDRPHY_SHU1_R1_B0_DQ3);
	writel(0xc0c0c0c, priv->ddrphy + DDRPHY_SHU1_R1_B0_DQ4);
	writel(0xc0c0c0c, priv->ddrphy + DDRPHY_SHU1_R1_B0_DQ5);
	writel(0x0, priv->ddrphy + DDRPHY_SHU1_R1_B0_DQ6);
	writel(0xc0c0c0c, priv->ddrphy + DDRPHY_SHU1_R0_B1_DQ2);
	writel(0xc0c0c0c, priv->ddrphy + DDRPHY_SHU1_R0_B1_DQ3);
	writel(0xc0c0c0c, priv->ddrphy + DDRPHY_SHU1_R0_B1_DQ4);
	writel(0xc0c0c0c, priv->ddrphy + DDRPHY_SHU1_R0_B1_DQ5);
	writel(0x0, priv->ddrphy + DDRPHY_SHU1_R0_B1_DQ6);
	writel(0xc0c0c0c, priv->ddrphy + DDRPHY_SHU1_R1_B1_DQ2);
	writel(0xc0c0c0c, priv->ddrphy + DDRPHY_SHU1_R1_B1_DQ3);
	writel(0xc0c0c0c, priv->ddrphy + DDRPHY_SHU1_R1_B1_DQ4);
	writel(0xc0c0c0c, priv->ddrphy + DDRPHY_SHU1_R1_B1_DQ5);
	writel(0x0, priv->ddrphy + DDRPHY_SHU1_R1_B1_DQ6);
	writel(0x20000001, priv->dramc_ao + DRAMC_SHU_RANKCTL);
	writel(0x2, priv->dramc_ao + DRAMC_SHURK0_DQSCTL);
	writel(0x2, priv->dramc_ao + DRAMC_SHURK1_DQSCTL);
	writel(0x4020b07, priv->dramc_ao + DRAMC_SHU_ACTIM0);
	writel(0xb060400, priv->dramc_ao + DRAMC_SHU_ACTIM1);
	writel(0x8090200, priv->dramc_ao + DRAMC_SHU_ACTIM2);
	writel(0x810018, priv->dramc_ao + DRAMC_SHU_ACTIM3);
	writel(0x1e9700ff, priv->dramc_ao + DRAMC_SHU_ACTIM4);
	writel(0x1000908, priv->dramc_ao + DRAMC_SHU_ACTIM5);
	writel(0x801040b, priv->dramc_ao + DRAMC_SHU_ACTIM_XRT);
	writel(0x20000D1, priv->dramc_ao + DRAMC_SHU_AC_TIME_05T);
	writel(0x80010000, priv->ddrphy + DDRPHY_PLL2);
	udelay(500);

	writel(0x81080000, priv->dramc_ao + DRAMC_MISCTL0);
	writel(0xacf13, priv->dramc_ao + DRAMC_PERFCTL0);
	writel(0xacf12, priv->dramc_ao + DRAMC_PERFCTL0);
	writel(0x80, priv->dramc_ao + DRAMC_ARBCTL);
	writel(0x9, priv->dramc_ao + DRAMC_PADCTRL);
	writel(0x80000107, priv->dramc_ao + DRAMC_DRAMC_PD_CTRL);
	writel(0x3000000c, priv->dramc_ao + DRAMC_CLKCTRL);
	writel(0x25714001, priv->dramc_ao + DRAMC_REFCTRL0);
	writel(0x35b1f1cf, priv->dramc_ao + DRAMC_SPCMDCTRL);
	writel(0x4300000, priv->dramc_ao + DRAMC_CATRAINING1);
	writel(0xb0300000, priv->dramc_ao + DRAMC_STBCAL);
	writel(0x731414, priv->dramc_ao + DRAMC_RKCFG);
	writel(0x733414, priv->dramc_ao + DRAMC_RKCFG);
	udelay(20);

	writel(0x80002050, priv->dramc_ao + DRAMC_CKECTRL);
	udelay(100);

	writel(0x400000, priv->dramc_ao + DRAMC_MRS);
	writel(0x401800, priv->dramc_ao + DRAMC_MRS);
	writel(0x1, priv->dramc_ao + DRAMC_SPCMD);
	writel(0x0, priv->dramc_ao + DRAMC_SPCMD);
	udelay(100);

	writel(0x601800, priv->dramc_ao + DRAMC_MRS);
	writel(0x600000, priv->dramc_ao + DRAMC_MRS);
	writel(0x1, priv->dramc_ao + DRAMC_SPCMD);
	writel(0x0, priv->dramc_ao + DRAMC_SPCMD);
	udelay(100);

	writel(0x200000, priv->dramc_ao + DRAMC_MRS);
	writel(0x200400, priv->dramc_ao + DRAMC_MRS);
	writel(0x1, priv->dramc_ao + DRAMC_SPCMD);
	writel(0x0, priv->dramc_ao + DRAMC_SPCMD);
	udelay(100);

	writel(0x400, priv->dramc_ao + DRAMC_MRS);
	writel(0x1d7000, priv->dramc_ao + DRAMC_MRS);
	writel(0x1, priv->dramc_ao + DRAMC_SPCMD);
	writel(0x0, priv->dramc_ao + DRAMC_SPCMD);
	udelay(100);

	writel(0x702201, priv->dramc_ao + DRAMC_DRAMCTRL);
	writel(0x10, priv->dramc_ao + DRAMC_SPCMD);
	writel(0x0, priv->dramc_ao + DRAMC_SPCMD);
	writel(0x20, priv->dramc_ao + DRAMC_SPCMD);
	writel(0x0, priv->dramc_ao + DRAMC_SPCMD);
	writel(0x1, priv->dramc_ao + DRAMC_HW_MRR_FUN);
	writel(0x702301, priv->dramc_ao + DRAMC_DRAMCTRL);
	writel(0x702301, priv->dramc_ao + DRAMC_DRAMCTRL);
	writel(0xa56, priv->dramc_ao + DRAMC_ZQCS);
	writel(0xff0000, priv->dramc_ao + DRAMC_SHU_CONF3);
	writel(0x15b1f1cf, priv->dramc_ao + DRAMC_SPCMDCTRL);
	writel(0x2cb00b0f, priv->dramc_ao + DRAMC_SHU_CONF1);
	writel(0x65714001, priv->dramc_ao + DRAMC_REFCTRL0);
	writel(0x48000000, priv->dramc_ao + DRAMC_SREFCTRL);
	writel(0xc0000107, priv->dramc_ao + DRAMC_DRAMC_PD_CTRL);
	writel(0x10002, priv->dramc_ao + DRAMC_EYESCAN);
	writel(0x15e00, priv->dramc_ao + DRAMC_STBCAL1);
	writel(0x100000, priv->dramc_ao + DRAMC_TEST2_1);
	writel(0x4000, priv->dramc_ao + DRAMC_TEST2_2);
	writel(0x12000480, priv->dramc_ao + DRAMC_TEST2_3);
	writel(0x301d007, priv->dramc_ao + DRAMC_SHUCTRL2);
	writel(0x4782321, priv->dramc_ao + DRAMC_DRAMCTRL);
	writel(0x30210000, priv->dramc_ao + DRAMC_SHU_CKECTRL);
	writel(0x20000, priv->dramc_ao + DRAMC_DUMMY_RD);
	writel(0x4080110d, priv->dramc_ao + DRAMC_TEST2_4);
	writel(0x30000721, priv->dramc_ao + DRAMC_REFCTRL1);
	writel(0x0, priv->dramc_ao + DRAMC_RSTMASK);
	writel(0x4782320, priv->dramc_ao + DRAMC_DRAMCTRL);
	writel(0x80002000, priv->dramc_ao + DRAMC_CKECTRL);
	writel(0x45714001, priv->dramc_ao + DRAMC_REFCTRL0);

	/* Apply config before calibration */
	writel(0x120, priv->dramc_ao + DRAMC_DRAMC_PD_CTRL);
	writel(0x11351131, priv->ddrphy + DDRPHY_MISC_CTRL3);
	writel(0xffffffff, priv->ddrphy + DDRPHY_MISC_CG_CTRL0);
	writel(0x2a7fe, priv->dramc_ao + DRAMC_SHU_SCINTV);
	writel(0xff01ff, priv->dramc_ao + DRAMC_SHU_CONF3);
	writel(0x4782320, priv->dramc_ao + DRAMC_DRAMCTRL);
	writel(0xa56, priv->dramc_ao + DRAMC_ZQCS);
	writel(0x80000000, priv->dramc_ao + DRAMC_SHU1_WODT);
	writel(0x21, priv->ddrphy + DDRPHY_SHU1_B0_DQ7);
	writel(0x1, priv->ddrphy + DDRPHY_SHU1_B1_DQ7);
	writel(0x35b1f1cf, priv->dramc_ao + DRAMC_SPCMDCTRL);
	writel(0x35b1f1cf, priv->dramc_ao + DRAMC_SPCMDCTRL);
	writel(0x35b1f1cf, priv->dramc_ao + DRAMC_SPCMDCTRL);
	writel(0xb0300000, priv->dramc_ao + DRAMC_STBCAL);
	writel(0xb0300000, priv->dramc_ao + DRAMC_STBCAL);
	writel(0x10002, priv->dramc_ao + DRAMC_EYESCAN);
	writel(0x8100008c, priv->ddrphy + DDRPHY_MISC_CTRL1);
	writel(0x45714001, priv->dramc_ao + DRAMC_REFCTRL0);
	writel(0xb0300000, priv->dramc_ao + DRAMC_STBCAL);
	writel(0xb0300000, priv->dramc_ao + DRAMC_STBCAL);

	/* Write leveling */
	writel(0x1f2e2e00, priv->ddrphy + DDRPHY_SHU1_R0_B0_DQ7);
	writel(0x202f2f00, priv->ddrphy + DDRPHY_SHU1_R0_B1_DQ7);
	writel(0x33221100, priv->dramc_ao + DRAMC_SHU_SELPH_DQS1);
	writel(0x11112222, priv->dramc_ao + DRAMC_SHU_SELPH_DQS0);

	/* RX dqs gating cal */
	writel(0x11111010, priv->dramc_ao + DRAMC_SHURK0_SELPH_DQSG0);
	writel(0x20201717, priv->dramc_ao + DRAMC_SHURK0_SELPH_DQSG1);
	writel(0x1d1f, priv->dramc_ao + DRAMC_SHURK0_DQSIEN);

	/* RX window per-bit cal */
	writel(0x03030404, priv->ddrphy + DDRPHY_SHU1_R0_B0_DQ2);
	writel(0x01010303, priv->ddrphy + DDRPHY_SHU1_R0_B0_DQ3);
	writel(0x01010303, priv->ddrphy + DDRPHY_SHU1_R0_B0_DQ4);
	writel(0x01010000, priv->ddrphy + DDRPHY_SHU1_R0_B0_DQ5);
	writel(0x03030606, priv->ddrphy + DDRPHY_SHU1_R0_B1_DQ2);
	writel(0x02020202, priv->ddrphy + DDRPHY_SHU1_R0_B1_DQ3);
	writel(0x04040303, priv->ddrphy + DDRPHY_SHU1_R0_B1_DQ4);
	writel(0x06060101, priv->ddrphy + DDRPHY_SHU1_R0_B1_DQ5);

	/* RX datlat cal */
	writel(0x28b00a0e, priv->dramc_ao + DRAMC_SHU_CONF1);

	/* TX window per-byte with 2UI cal */
	writel(0x11112222, priv->dramc_ao + DRAMC_SHURK0_SELPH_DQ0);
	writel(0x22220000, priv->dramc_ao + DRAMC_SHURK0_SELPH_DQ2);
	writel(0x11112222, priv->dramc_ao + DRAMC_SHURK0_SELPH_DQ1);
	writel(0x22220000, priv->dramc_ao + DRAMC_SHURK0_SELPH_DQ3);
	writel(0x1f2e2e00, priv->ddrphy + DDRPHY_SHU1_R0_B0_DQ7);
	writel(0x202f2f00, priv->ddrphy + DDRPHY_SHU1_R0_B1_DQ7);

	return mtk_ddr3_rank_size_detect(dev);
}
#endif

static int mtk_ddr3_probe(struct udevice *dev)
{
	struct mtk_ddr3_priv *priv = dev_get_priv(dev);

	priv->emi = dev_read_addr_index(dev, 0);
	if (priv->emi == FDT_ADDR_T_NONE)
		return -EINVAL;

	priv->ddrphy = dev_read_addr_index(dev, 1);
	if (priv->ddrphy == FDT_ADDR_T_NONE)
		return -EINVAL;

	priv->dramc_ao = dev_read_addr_index(dev, 2);
	if (priv->dramc_ao == FDT_ADDR_T_NONE)
		return -EINVAL;

#ifdef CONFIG_SPL_BUILD
	int ret;

	ret = clk_get_by_index(dev, 0, &priv->phy);
	if (ret)
		return ret;

	ret = clk_get_by_index(dev, 1, &priv->phy_mux);
	if (ret)
		return ret;

	ret = clk_get_by_index(dev, 2, &priv->mem);
	if (ret)
		return ret;

	ret = clk_get_by_index(dev, 3, &priv->mem_mux);
	if (ret)
		return ret;

	ret = mtk_ddr3_init(dev);
	if (ret)
		return ret;
#endif
	return 0;
}

static int mtk_ddr3_get_info(struct udevice *dev, struct ram_info *info)
{
	struct mtk_ddr3_priv *priv = dev_get_priv(dev);
	u32 val = readl(priv->emi + EMI_CONA);

	info->base = CONFIG_SYS_SDRAM_BASE;

	switch ((val & EMI_COL_ADDR_MASK) >> EMI_COL_ADDR_SHIFT) {
	case 0:
		info->size = SZ_128M;
		break;
	case 1:
		info->size = SZ_256M;
		break;
	case 2:
		info->size = SZ_512M;
		break;
	case 3:
		info->size = SZ_1G;
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

static struct ram_ops mtk_ddr3_ops = {
	.get_info = mtk_ddr3_get_info,
};

static const struct udevice_id mtk_ddr3_ids[] = {
	{ .compatible = "mediatek,mt7629-dramc" },
	{ }
};

U_BOOT_DRIVER(mediatek_ddr3) = {
	.name     = "mediatek_ddr3",
	.id       = UCLASS_RAM,
	.of_match = mtk_ddr3_ids,
	.ops      = &mtk_ddr3_ops,
	.probe    = mtk_ddr3_probe,
	.priv_auto_alloc_size = sizeof(struct mtk_ddr3_priv),
};
