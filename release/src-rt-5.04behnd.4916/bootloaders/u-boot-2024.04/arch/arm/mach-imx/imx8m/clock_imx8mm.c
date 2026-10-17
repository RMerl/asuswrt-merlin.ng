// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2018-2019 NXP
 *
 * Peng Fan <peng.fan@nxp.com>
 */

#include <common.h>
#include <asm/arch/clock.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/sys_proto.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <div64.h>
#include <errno.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <phy.h>

DECLARE_GLOBAL_DATA_PTR;

static struct anamix_pll *ana_pll = (struct anamix_pll *)ANATOP_BASE_ADDR;

static u32 get_root_clk(enum clk_root_index clock_id);

#ifdef CONFIG_IMX_HAB
void hab_caam_clock_enable(unsigned char enable)
{
	/* The CAAM clock is always on for iMX8M */
}
#endif

void enable_ocotp_clk(unsigned char enable)
{
	clock_enable(CCGR_OCOTP, !!enable);
}

int enable_i2c_clk(unsigned char enable, unsigned i2c_num)
{
	u8 i2c_ccgr[] = {
			CCGR_I2C1, CCGR_I2C2, CCGR_I2C3, CCGR_I2C4,
#if (IS_ENABLED(CONFIG_IMX8MP))
			CCGR_I2C5_8MP, CCGR_I2C6_8MP
#endif
	};

	if (i2c_num >= ARRAY_SIZE(i2c_ccgr))
		return -EINVAL;

	clock_enable(i2c_ccgr[i2c_num], !!enable);

	return 0;
}

#ifdef CONFIG_SPL_BUILD
static struct imx_int_pll_rate_table imx8mm_fracpll_tbl[] = {
	PLL_1443X_RATE(1000000000U, 250, 3, 1, 0),
	PLL_1443X_RATE(933000000U, 311, 4, 1, 0),
	PLL_1443X_RATE(900000000U, 300, 8, 0, 0),
	PLL_1443X_RATE(800000000U, 300, 9, 0, 0),
	PLL_1443X_RATE(750000000U, 250, 8, 0, 0),
	PLL_1443X_RATE(650000000U, 325, 3, 2, 0),
	PLL_1443X_RATE(600000000U, 300, 3, 2, 0),
	PLL_1443X_RATE(594000000U, 99, 1, 2, 0),
	PLL_1443X_RATE(400000000U, 300, 9, 1, 0),
	PLL_1443X_RATE(266000000U, 400, 9, 2, 0),
	PLL_1443X_RATE(167000000U, 334, 3, 4, 0),
	PLL_1443X_RATE(100000000U, 300, 9, 3, 0),
};

static int fracpll_configure(enum pll_clocks pll, u32 freq)
{
	int i;
	u32 tmp, div_val;
	void *pll_base;
	struct imx_int_pll_rate_table *rate;

	for (i = 0; i < ARRAY_SIZE(imx8mm_fracpll_tbl); i++) {
		if (freq == imx8mm_fracpll_tbl[i].rate)
			break;
	}

	if (i == ARRAY_SIZE(imx8mm_fracpll_tbl)) {
		printf("%s: No matched freq table %u\n", __func__, freq);
		return -EINVAL;
	}

	rate = &imx8mm_fracpll_tbl[i];

	switch (pll) {
	case ANATOP_DRAM_PLL:
		setbits_le32(GPC_BASE_ADDR + 0xEC, 1 << 7);
		setbits_le32(GPC_BASE_ADDR + 0xF8, 1 << 5);

		pll_base = &ana_pll->dram_pll_gnrl_ctl;
		break;
	case ANATOP_VIDEO_PLL:
		pll_base = &ana_pll->video_pll1_gnrl_ctl;
		break;
	default:
		return 0;
	}
	/* Bypass clock and set lock to pll output lock */
	tmp = readl(pll_base);
	tmp |= BYPASS_MASK;
	writel(tmp, pll_base);

	/* Enable RST */
	tmp &= ~RST_MASK;
	writel(tmp, pll_base);

	div_val = (rate->mdiv << MDIV_SHIFT) | (rate->pdiv << PDIV_SHIFT) |
		(rate->sdiv << SDIV_SHIFT);
	writel(div_val, pll_base + 4);
	writel(rate->kdiv << KDIV_SHIFT, pll_base + 8);

	__udelay(100);

	/* Disable RST */
	tmp |= RST_MASK;
	writel(tmp, pll_base);

	/* Wait Lock*/
	while (!(readl(pll_base) & LOCK_STATUS))
		;

	/* Bypass */
	tmp &= ~BYPASS_MASK;
	writel(tmp, pll_base);

	return 0;
}

void dram_pll_init(ulong pll_val)
{
	fracpll_configure(ANATOP_DRAM_PLL, pll_val);
}

static struct dram_bypass_clk_setting imx8mm_dram_bypass_tbl[] = {
	DRAM_BYPASS_ROOT_CONFIG(MHZ(100), 2, CLK_ROOT_PRE_DIV1, 2,
				CLK_ROOT_PRE_DIV2),
	DRAM_BYPASS_ROOT_CONFIG(MHZ(250), 3, CLK_ROOT_PRE_DIV2, 2,
				CLK_ROOT_PRE_DIV2),
	DRAM_BYPASS_ROOT_CONFIG(MHZ(400), 1, CLK_ROOT_PRE_DIV2, 3,
				CLK_ROOT_PRE_DIV2),
};

void dram_enable_bypass(ulong clk_val)
{
	int i;
	struct dram_bypass_clk_setting *config;

	for (i = 0; i < ARRAY_SIZE(imx8mm_dram_bypass_tbl); i++) {
		if (clk_val == imx8mm_dram_bypass_tbl[i].clk)
			break;
	}

	if (i == ARRAY_SIZE(imx8mm_dram_bypass_tbl)) {
		printf("%s: No matched freq table %lu\n", __func__, clk_val);
		return;
	}

	config = &imx8mm_dram_bypass_tbl[i];

	clock_set_target_val(DRAM_ALT_CLK_ROOT, CLK_ROOT_ON |
			     CLK_ROOT_SOURCE_SEL(config->alt_root_sel) |
			     CLK_ROOT_PRE_DIV(config->alt_pre_div));
	clock_set_target_val(DRAM_APB_CLK_ROOT, CLK_ROOT_ON |
			     CLK_ROOT_SOURCE_SEL(config->apb_root_sel) |
			     CLK_ROOT_PRE_DIV(config->apb_pre_div));
	clock_set_target_val(DRAM_SEL_CFG, CLK_ROOT_ON |
			     CLK_ROOT_SOURCE_SEL(1));
}

void dram_disable_bypass(void)
{
	clock_set_target_val(DRAM_SEL_CFG, CLK_ROOT_ON |
			     CLK_ROOT_SOURCE_SEL(0));
	clock_set_target_val(DRAM_APB_CLK_ROOT, CLK_ROOT_ON |
			     CLK_ROOT_SOURCE_SEL(4) |
			     CLK_ROOT_PRE_DIV(CLK_ROOT_PRE_DIV5));
}
#endif

int intpll_configure(enum pll_clocks pll, ulong freq)
{
	void __iomem *pll_gnrl_ctl, __iomem *pll_div_ctl;
	u32 pll_div_ctl_val, pll_clke_masks;

	switch (pll) {
	case ANATOP_SYSTEM_PLL1:
		pll_gnrl_ctl = &ana_pll->sys_pll1_gnrl_ctl;
		pll_div_ctl = &ana_pll->sys_pll1_div_ctl;
		pll_clke_masks = INTPLL_DIV20_CLKE_MASK |
			INTPLL_DIV10_CLKE_MASK | INTPLL_DIV8_CLKE_MASK |
			INTPLL_DIV6_CLKE_MASK | INTPLL_DIV5_CLKE_MASK |
			INTPLL_DIV4_CLKE_MASK | INTPLL_DIV3_CLKE_MASK |
			INTPLL_DIV2_CLKE_MASK | INTPLL_CLKE_MASK;
		break;
	case ANATOP_SYSTEM_PLL2:
		pll_gnrl_ctl = &ana_pll->sys_pll2_gnrl_ctl;
		pll_div_ctl = &ana_pll->sys_pll2_div_ctl;
		pll_clke_masks = INTPLL_DIV20_CLKE_MASK |
			INTPLL_DIV10_CLKE_MASK | INTPLL_DIV8_CLKE_MASK |
			INTPLL_DIV6_CLKE_MASK | INTPLL_DIV5_CLKE_MASK |
			INTPLL_DIV4_CLKE_MASK | INTPLL_DIV3_CLKE_MASK |
			INTPLL_DIV2_CLKE_MASK | INTPLL_CLKE_MASK;
		break;
	case ANATOP_SYSTEM_PLL3:
		pll_gnrl_ctl = &ana_pll->sys_pll3_gnrl_ctl;
		pll_div_ctl = &ana_pll->sys_pll3_div_ctl;
		pll_clke_masks = INTPLL_CLKE_MASK;
		break;
	case ANATOP_ARM_PLL:
		pll_gnrl_ctl = &ana_pll->arm_pll_gnrl_ctl;
		pll_div_ctl = &ana_pll->arm_pll_div_ctl;
		pll_clke_masks = INTPLL_CLKE_MASK;
		break;
	case ANATOP_GPU_PLL:
		pll_gnrl_ctl = &ana_pll->gpu_pll_gnrl_ctl;
		pll_div_ctl = &ana_pll->gpu_pll_div_ctl;
		pll_clke_masks = INTPLL_CLKE_MASK;
		break;
	case ANATOP_VPU_PLL:
		pll_gnrl_ctl = &ana_pll->vpu_pll_gnrl_ctl;
		pll_div_ctl = &ana_pll->vpu_pll_div_ctl;
		pll_clke_masks = INTPLL_CLKE_MASK;
		break;
	default:
		return -EINVAL;
	};

	switch (freq) {
	case MHZ(600):
		/* 24 * 0x12c / 3 / 2 ^ 2 */
		pll_div_ctl_val = INTPLL_MAIN_DIV_VAL(0x12c) |
			INTPLL_PRE_DIV_VAL(3) | INTPLL_POST_DIV_VAL(2);
		break;
	case MHZ(750):
		/* 24 * 0xfa / 2 / 2 ^ 2 */
		pll_div_ctl_val = INTPLL_MAIN_DIV_VAL(0xfa) |
			INTPLL_PRE_DIV_VAL(2) | INTPLL_POST_DIV_VAL(2);
		break;
	case MHZ(800):
		/* 24 * 0x190 / 3 / 2 ^ 2 */
		pll_div_ctl_val = INTPLL_MAIN_DIV_VAL(0x190) |
			INTPLL_PRE_DIV_VAL(3) | INTPLL_POST_DIV_VAL(2);
		break;
	case MHZ(1000):
		/* 24 * 0xfa / 3 / 2 ^ 1 */
		pll_div_ctl_val = INTPLL_MAIN_DIV_VAL(0xfa) |
			INTPLL_PRE_DIV_VAL(3) | INTPLL_POST_DIV_VAL(1);
		break;
	case MHZ(1200):
		/* 24 * 0x12c / 3 / 2 ^ 1 */
		pll_div_ctl_val = INTPLL_MAIN_DIV_VAL(0x12c) |
			INTPLL_PRE_DIV_VAL(3) | INTPLL_POST_DIV_VAL(1);
		break;
	case MHZ(1400):
		/* 24 * 0x15e / 3 / 2 ^ 1 */
		pll_div_ctl_val = INTPLL_MAIN_DIV_VAL(0x15e) |
			INTPLL_PRE_DIV_VAL(3) | INTPLL_POST_DIV_VAL(1);
		break;
	case MHZ(1500):
		/* 24 * 0x177 / 3 / 2 ^ 1 */
		pll_div_ctl_val = INTPLL_MAIN_DIV_VAL(0x177) |
			INTPLL_PRE_DIV_VAL(3) | INTPLL_POST_DIV_VAL(1);
		break;
	case MHZ(1600):
		/* 24 * 0xc8 / 3 / 2 ^ 0 */
		pll_div_ctl_val = INTPLL_MAIN_DIV_VAL(0xc8) |
			INTPLL_PRE_DIV_VAL(3) | INTPLL_POST_DIV_VAL(0);
		break;
	case MHZ(1800):
		/* 24 * 0xe1 / 3 / 2 ^ 0 */
		pll_div_ctl_val = INTPLL_MAIN_DIV_VAL(0xe1) |
			INTPLL_PRE_DIV_VAL(3) | INTPLL_POST_DIV_VAL(0);
		break;
	case MHZ(2000):
		/* 24 * 0xfa / 3 / 2 ^ 0 */
		pll_div_ctl_val = INTPLL_MAIN_DIV_VAL(0xfa) |
			INTPLL_PRE_DIV_VAL(3) | INTPLL_POST_DIV_VAL(0);
		break;
	default:
		return -EINVAL;
	};
	/* Bypass clock and set lock to pll output lock */
	setbits_le32(pll_gnrl_ctl, INTPLL_BYPASS_MASK | INTPLL_LOCK_SEL_MASK);
	/* Enable reset */
	clrbits_le32(pll_gnrl_ctl, INTPLL_RST_MASK);
	/* Configure */
	writel(pll_div_ctl_val, pll_div_ctl);

	__udelay(100);

	/* Disable reset */
	setbits_le32(pll_gnrl_ctl, INTPLL_RST_MASK);
	/* Wait Lock */
	while (!(readl(pll_gnrl_ctl) & INTPLL_LOCK_MASK))
		;
	/* Clear bypass */
	clrbits_le32(pll_gnrl_ctl, INTPLL_BYPASS_MASK);
	setbits_le32(pll_gnrl_ctl, pll_clke_masks);

	return 0;
}

void init_uart_clk(u32 index)
{
	/*
	 * set uart clock root
	 * 24M OSC
	 */
	switch (index) {
	case 0:
		clock_enable(CCGR_UART1, 0);
		clock_set_target_val(UART1_CLK_ROOT, CLK_ROOT_ON |
				     CLK_ROOT_SOURCE_SEL(0));
		clock_enable(CCGR_UART1, 1);
		return;
	case 1:
		clock_enable(CCGR_UART2, 0);
		clock_set_target_val(UART2_CLK_ROOT, CLK_ROOT_ON |
				     CLK_ROOT_SOURCE_SEL(0));
		clock_enable(CCGR_UART2, 1);
		return;
	case 2:
		clock_enable(CCGR_UART3, 0);
		clock_set_target_val(UART3_CLK_ROOT, CLK_ROOT_ON |
				     CLK_ROOT_SOURCE_SEL(0));
		clock_enable(CCGR_UART3, 1);
		return;
	case 3:
		clock_enable(CCGR_UART4, 0);
		clock_set_target_val(UART4_CLK_ROOT, CLK_ROOT_ON |
				     CLK_ROOT_SOURCE_SEL(0));
		clock_enable(CCGR_UART4, 1);
		return;
	default:
		printf("Invalid uart index\n");
		return;
	}
}

void init_wdog_clk(void)
{
	clock_enable(CCGR_WDOG1, 0);
	clock_enable(CCGR_WDOG2, 0);
	clock_enable(CCGR_WDOG3, 0);
	clock_set_target_val(WDOG_CLK_ROOT, CLK_ROOT_ON |
			     CLK_ROOT_SOURCE_SEL(0));
	clock_enable(CCGR_WDOG1, 1);
	clock_enable(CCGR_WDOG2, 1);
	clock_enable(CCGR_WDOG3, 1);
}

void init_clk_usdhc(u32 index)
{
	/*
	 * set usdhc clock root
	 * sys pll1 400M
	 */
	switch (index) {
	case 0:
		clock_enable(CCGR_USDHC1, 0);
		clock_set_target_val(USDHC1_CLK_ROOT, CLK_ROOT_ON |
				     CLK_ROOT_SOURCE_SEL(1));
		clock_enable(CCGR_USDHC1, 1);
		return;
	case 1:
		clock_enable(CCGR_USDHC2, 0);
		clock_set_target_val(USDHC2_CLK_ROOT, CLK_ROOT_ON |
				     CLK_ROOT_SOURCE_SEL(1));
		clock_enable(CCGR_USDHC2, 1);
		return;
	case 2:
		clock_enable(CCGR_USDHC3, 0);
		clock_set_target_val(USDHC3_CLK_ROOT, CLK_ROOT_ON |
				     CLK_ROOT_SOURCE_SEL(1));
		clock_enable(CCGR_USDHC3, 1);
		return;
	default:
		printf("Invalid usdhc index\n");
		return;
	}
}

void init_clk_ecspi(u32 index)
{
	switch (index) {
	case 0:
		clock_enable(CCGR_ECSPI1, 0);
		clock_set_target_val(ECSPI1_CLK_ROOT, CLK_ROOT_ON | CLK_ROOT_SOURCE_SEL(0));
		clock_enable(CCGR_ECSPI1, 1);
		return;
	case 1:
		clock_enable(CCGR_ECSPI2, 0);
		clock_set_target_val(ECSPI2_CLK_ROOT, CLK_ROOT_ON | CLK_ROOT_SOURCE_SEL(0));
		clock_enable(CCGR_ECSPI2, 1);
		return;
	case 2:
		clock_enable(CCGR_ECSPI3, 0);
		clock_set_target_val(ECSPI3_CLK_ROOT, CLK_ROOT_ON | CLK_ROOT_SOURCE_SEL(0));
		clock_enable(CCGR_ECSPI3, 1);
		return;
	default:
		printf("Invalid ecspi index\n");
		return;
	}
}

void init_nand_clk(void)
{
	/*
	 * set rawnand root
	 * sys pll1 400M
	 */
	clock_enable(CCGR_RAWNAND, 0);
	clock_set_target_val(NAND_CLK_ROOT, CLK_ROOT_ON |
		CLK_ROOT_SOURCE_SEL(3) | CLK_ROOT_POST_DIV(CLK_ROOT_POST_DIV4)); /* 100M */
	clock_enable(CCGR_RAWNAND, 1);
}

int clock_init(void)
{
	u32 val_cfg0;

	/*
	 * The gate is not exported to clk tree, so configure them here.
	 * According to ANAMIX SPEC
	 * sys pll1 fixed at 800MHz
	 * sys pll2 fixed at 1GHz
	 * Here we only enable the outputs.
	 */
	val_cfg0 = readl(&ana_pll->sys_pll1_gnrl_ctl);
	val_cfg0 |= INTPLL_CLKE_MASK | INTPLL_DIV2_CLKE_MASK |
		INTPLL_DIV3_CLKE_MASK | INTPLL_DIV4_CLKE_MASK |
		INTPLL_DIV5_CLKE_MASK | INTPLL_DIV6_CLKE_MASK |
		INTPLL_DIV8_CLKE_MASK | INTPLL_DIV10_CLKE_MASK |
		INTPLL_DIV20_CLKE_MASK;
	writel(val_cfg0, &ana_pll->sys_pll1_gnrl_ctl);

	val_cfg0 = readl(&ana_pll->sys_pll2_gnrl_ctl);
	val_cfg0 |= INTPLL_CLKE_MASK | INTPLL_DIV2_CLKE_MASK |
		INTPLL_DIV3_CLKE_MASK | INTPLL_DIV4_CLKE_MASK |
		INTPLL_DIV5_CLKE_MASK | INTPLL_DIV6_CLKE_MASK |
		INTPLL_DIV8_CLKE_MASK | INTPLL_DIV10_CLKE_MASK |
		INTPLL_DIV20_CLKE_MASK;
	writel(val_cfg0, &ana_pll->sys_pll2_gnrl_ctl);

	/* Configure ARM at 1.2GHz */
	clock_set_target_val(ARM_A53_CLK_ROOT, CLK_ROOT_ON |
			     CLK_ROOT_SOURCE_SEL(2));

	intpll_configure(ANATOP_ARM_PLL, MHZ(1200));

	/* Bypass CCM A53 ROOT, Switch to ARM PLL -> MUX-> CPU */
	clock_set_target_val(CORE_SEL_CFG, CLK_ROOT_SOURCE_SEL(1));

	if (is_imx8mn() || is_imx8mp())
		intpll_configure(ANATOP_SYSTEM_PLL3, MHZ(600));
	else
		intpll_configure(ANATOP_SYSTEM_PLL3, MHZ(750));

#ifdef CONFIG_IMX8MP
	/* 8MP ROM already set NOC to 800Mhz, only need to configure NOC_IO clk to 600Mhz */
	/* 8MP ROM already set GIC to 400Mhz, system_pll1_800m with div = 2 */
	clock_set_target_val(NOC_IO_CLK_ROOT, CLK_ROOT_ON | CLK_ROOT_SOURCE_SEL(2));
#else
	clock_set_target_val(NOC_CLK_ROOT, CLK_ROOT_ON | CLK_ROOT_SOURCE_SEL(2));

	/* config GIC to sys_pll2_100m */
	clock_enable(CCGR_GIC, 0);
	clock_set_target_val(GIC_CLK_ROOT, CLK_ROOT_ON |
			     CLK_ROOT_SOURCE_SEL(3));
	clock_enable(CCGR_GIC, 1);
#endif

	clock_set_target_val(NAND_USDHC_BUS_CLK_ROOT, CLK_ROOT_ON |
			     CLK_ROOT_SOURCE_SEL(1));

	clock_enable(CCGR_DDR1, 0);
	clock_set_target_val(DRAM_ALT_CLK_ROOT, CLK_ROOT_ON |
			     CLK_ROOT_SOURCE_SEL(1));
	clock_set_target_val(DRAM_APB_CLK_ROOT, CLK_ROOT_ON |
			     CLK_ROOT_SOURCE_SEL(1));
	clock_enable(CCGR_DDR1, 1);

	init_wdog_clk();

	clock_enable(CCGR_TEMP_SENSOR, 1);

	clock_enable(CCGR_SEC_DEBUG, 1);

	return 0;
};

u32 imx_get_uartclk(void)
{
	return 24000000U;
}

static u32 decode_intpll(enum clk_root_src intpll)
{
	u32 pll_gnrl_ctl, pll_div_ctl, pll_clke_mask;
	u32 main_div, pre_div, post_div, div;
	u64 freq;

	switch (intpll) {
	case ARM_PLL_CLK:
		pll_gnrl_ctl = readl(&ana_pll->arm_pll_gnrl_ctl);
		pll_div_ctl = readl(&ana_pll->arm_pll_div_ctl);
		break;
	case GPU_PLL_CLK:
		pll_gnrl_ctl = readl(&ana_pll->gpu_pll_gnrl_ctl);
		pll_div_ctl = readl(&ana_pll->gpu_pll_div_ctl);
		break;
	case VPU_PLL_CLK:
		pll_gnrl_ctl = readl(&ana_pll->vpu_pll_gnrl_ctl);
		pll_div_ctl = readl(&ana_pll->vpu_pll_div_ctl);
		break;
	case SYSTEM_PLL1_800M_CLK:
	case SYSTEM_PLL1_400M_CLK:
	case SYSTEM_PLL1_266M_CLK:
	case SYSTEM_PLL1_200M_CLK:
	case SYSTEM_PLL1_160M_CLK:
	case SYSTEM_PLL1_133M_CLK:
	case SYSTEM_PLL1_100M_CLK:
	case SYSTEM_PLL1_80M_CLK:
	case SYSTEM_PLL1_40M_CLK:
		pll_gnrl_ctl = readl(&ana_pll->sys_pll1_gnrl_ctl);
		pll_div_ctl = readl(&ana_pll->sys_pll1_div_ctl);
		break;
	case SYSTEM_PLL2_1000M_CLK:
	case SYSTEM_PLL2_500M_CLK:
	case SYSTEM_PLL2_333M_CLK:
	case SYSTEM_PLL2_250M_CLK:
	case SYSTEM_PLL2_200M_CLK:
	case SYSTEM_PLL2_166M_CLK:
	case SYSTEM_PLL2_125M_CLK:
	case SYSTEM_PLL2_100M_CLK:
	case SYSTEM_PLL2_50M_CLK:
		pll_gnrl_ctl = readl(&ana_pll->sys_pll2_gnrl_ctl);
		pll_div_ctl = readl(&ana_pll->sys_pll2_div_ctl);
		break;
	case SYSTEM_PLL3_CLK:
		pll_gnrl_ctl = readl(&ana_pll->sys_pll3_gnrl_ctl);
		pll_div_ctl = readl(&ana_pll->sys_pll3_div_ctl);
		break;
	default:
		return -EINVAL;
	}

	/* Only support SYS_XTAL 24M, PAD_CLK not take into consideration */
	if ((pll_gnrl_ctl & INTPLL_REF_CLK_SEL_MASK) != 0)
		return 0;

	if ((pll_gnrl_ctl & INTPLL_RST_MASK) == 0)
		return 0;

	/*
	 * When BYPASS is equal to 1, PLL enters the bypass mode
	 * regardless of the values of RESETB
	 */
	if (pll_gnrl_ctl & INTPLL_BYPASS_MASK)
		return 24000000u;

	if (!(pll_gnrl_ctl & INTPLL_LOCK_MASK)) {
		puts("pll not locked\n");
		return 0;
	}

	switch (intpll) {
	case ARM_PLL_CLK:
	case GPU_PLL_CLK:
	case VPU_PLL_CLK:
	case SYSTEM_PLL3_CLK:
	case SYSTEM_PLL1_800M_CLK:
	case SYSTEM_PLL2_1000M_CLK:
		pll_clke_mask = INTPLL_CLKE_MASK;
		div = 1;
		break;

	case SYSTEM_PLL1_400M_CLK:
	case SYSTEM_PLL2_500M_CLK:
		pll_clke_mask = INTPLL_DIV2_CLKE_MASK;
		div = 2;
		break;

	case SYSTEM_PLL1_266M_CLK:
	case SYSTEM_PLL2_333M_CLK:
		pll_clke_mask = INTPLL_DIV3_CLKE_MASK;
		div = 3;
		break;

	case SYSTEM_PLL1_200M_CLK:
	case SYSTEM_PLL2_250M_CLK:
		pll_clke_mask = INTPLL_DIV4_CLKE_MASK;
		div = 4;
		break;

	case SYSTEM_PLL1_160M_CLK:
	case SYSTEM_PLL2_200M_CLK:
		pll_clke_mask = INTPLL_DIV5_CLKE_MASK;
		div = 5;
		break;

	case SYSTEM_PLL1_133M_CLK:
	case SYSTEM_PLL2_166M_CLK:
		pll_clke_mask = INTPLL_DIV6_CLKE_MASK;
		div = 6;
		break;

	case SYSTEM_PLL1_100M_CLK:
	case SYSTEM_PLL2_125M_CLK:
		pll_clke_mask = INTPLL_DIV8_CLKE_MASK;
		div = 8;
		break;

	case SYSTEM_PLL1_80M_CLK:
	case SYSTEM_PLL2_100M_CLK:
		pll_clke_mask = INTPLL_DIV10_CLKE_MASK;
		div = 10;
		break;

	case SYSTEM_PLL1_40M_CLK:
	case SYSTEM_PLL2_50M_CLK:
		pll_clke_mask = INTPLL_DIV20_CLKE_MASK;
		div = 20;
		break;
	default:
		return -EINVAL;
	}

	if ((pll_gnrl_ctl & pll_clke_mask) == 0)
		return 0;

	main_div = (pll_div_ctl & INTPLL_MAIN_DIV_MASK) >>
		INTPLL_MAIN_DIV_SHIFT;
	pre_div = (pll_div_ctl & INTPLL_PRE_DIV_MASK) >>
		INTPLL_PRE_DIV_SHIFT;
	post_div = (pll_div_ctl & INTPLL_POST_DIV_MASK) >>
		INTPLL_POST_DIV_SHIFT;

	/* FFVCO = (m * FFIN) / p, FFOUT = (m * FFIN) / (p * 2^s) */
	freq = 24000000ULL * main_div;
	return lldiv(freq, pre_div * (1 << post_div) * div);
}

static u32 decode_fracpll(enum clk_root_src frac_pll)
{
	u32 pll_gnrl_ctl, pll_fdiv_ctl0, pll_fdiv_ctl1;
	u32 main_div, pre_div, post_div, k;

	switch (frac_pll) {
	case DRAM_PLL1_CLK:
		pll_gnrl_ctl = readl(&ana_pll->dram_pll_gnrl_ctl);
		pll_fdiv_ctl0 = readl(&ana_pll->dram_pll_fdiv_ctl0);
		pll_fdiv_ctl1 = readl(&ana_pll->dram_pll_fdiv_ctl1);
		break;
	case AUDIO_PLL1_CLK:
		pll_gnrl_ctl = readl(&ana_pll->audio_pll1_gnrl_ctl);
		pll_fdiv_ctl0 = readl(&ana_pll->audio_pll1_fdiv_ctl0);
		pll_fdiv_ctl1 = readl(&ana_pll->audio_pll1_fdiv_ctl1);
		break;
	case AUDIO_PLL2_CLK:
		pll_gnrl_ctl = readl(&ana_pll->audio_pll2_gnrl_ctl);
		pll_fdiv_ctl0 = readl(&ana_pll->audio_pll2_fdiv_ctl0);
		pll_fdiv_ctl1 = readl(&ana_pll->audio_pll2_fdiv_ctl1);
		break;
	case VIDEO_PLL_CLK:
		pll_gnrl_ctl = readl(&ana_pll->video_pll1_gnrl_ctl);
		pll_fdiv_ctl0 = readl(&ana_pll->video_pll1_fdiv_ctl0);
		pll_fdiv_ctl1 = readl(&ana_pll->video_pll1_fdiv_ctl1);
		break;
	default:
		printf("Unsupported clk_root_src %d\n", frac_pll);
		return 0;
	}

	/* Only support SYS_XTAL 24M, PAD_CLK not take into consideration */
	if ((pll_gnrl_ctl & GENMASK(1, 0)) != 0)
		return 0;

	if ((pll_gnrl_ctl & RST_MASK) == 0)
		return 0;
	/*
	 * When BYPASS is equal to 1, PLL enters the bypass mode
	 * regardless of the values of RESETB
	 */
	if (pll_gnrl_ctl & BYPASS_MASK)
		return 24000000u;

	if (!(pll_gnrl_ctl & LOCK_STATUS)) {
		puts("pll not locked\n");
		return 0;
	}

	if (!(pll_gnrl_ctl & CLKE_MASK))
		return 0;

	main_div = (pll_fdiv_ctl0 & MDIV_MASK) >>
		MDIV_SHIFT;
	pre_div = (pll_fdiv_ctl0 & PDIV_MASK) >>
		PDIV_SHIFT;
	post_div = (pll_fdiv_ctl0 & SDIV_MASK) >>
		SDIV_SHIFT;

	k = pll_fdiv_ctl1 & KDIV_MASK;

	return lldiv((main_div * 65536 + k) * 24000000ULL,
		     65536 * pre_div * (1 << post_div));
}

static u32 get_root_src_clk(enum clk_root_src root_src)
{
	switch (root_src) {
	case OSC_24M_CLK:
		return 24000000u;
	case OSC_HDMI_CLK:
		return 26000000u;
	case OSC_32K_CLK:
		return 32000u;
	case ARM_PLL_CLK:
	case GPU_PLL_CLK:
	case VPU_PLL_CLK:
	case SYSTEM_PLL1_800M_CLK:
	case SYSTEM_PLL1_400M_CLK:
	case SYSTEM_PLL1_266M_CLK:
	case SYSTEM_PLL1_200M_CLK:
	case SYSTEM_PLL1_160M_CLK:
	case SYSTEM_PLL1_133M_CLK:
	case SYSTEM_PLL1_100M_CLK:
	case SYSTEM_PLL1_80M_CLK:
	case SYSTEM_PLL1_40M_CLK:
	case SYSTEM_PLL2_1000M_CLK:
	case SYSTEM_PLL2_500M_CLK:
	case SYSTEM_PLL2_333M_CLK:
	case SYSTEM_PLL2_250M_CLK:
	case SYSTEM_PLL2_200M_CLK:
	case SYSTEM_PLL2_166M_CLK:
	case SYSTEM_PLL2_125M_CLK:
	case SYSTEM_PLL2_100M_CLK:
	case SYSTEM_PLL2_50M_CLK:
	case SYSTEM_PLL3_CLK:
		return decode_intpll(root_src);
	case DRAM_PLL1_CLK:
	case AUDIO_PLL1_CLK:
	case AUDIO_PLL2_CLK:
	case VIDEO_PLL_CLK:
		return decode_fracpll(root_src);
	case ARM_A53_ALT_CLK:
		return get_root_clk(ARM_A53_CLK_ROOT);
	default:
		return 0;
	}

	return 0;
}

static u32 get_root_clk(enum clk_root_index clock_id)
{
	enum clk_root_src root_src;
	u32 post_podf, pre_podf, root_src_clk;

	if (clock_root_enabled(clock_id) <= 0)
		return 0;

	if (clock_get_prediv(clock_id, &pre_podf) < 0)
		return 0;

	if (clock_get_postdiv(clock_id, &post_podf) < 0)
		return 0;

	if (clock_get_src(clock_id, &root_src) < 0)
		return 0;

	root_src_clk = get_root_src_clk(root_src);

	return root_src_clk / (post_podf + 1) / (pre_podf + 1);
}

u32 get_arm_core_clk(void)
{
	enum clk_root_src root_src;
	u32 root_src_clk;

	if (clock_get_src(CORE_SEL_CFG, &root_src) < 0)
		return 0;

	root_src_clk = get_root_src_clk(root_src);

	return root_src_clk;
}

u32 mxc_get_clock(enum mxc_clock clk)
{
	u32 val;

	switch (clk) {
	case MXC_ARM_CLK:
		return get_arm_core_clk();
	case MXC_IPG_CLK:
		clock_get_target_val(IPG_CLK_ROOT, &val);
		val = val & 0x3;
		return get_root_clk(AHB_CLK_ROOT) / 2 / (val + 1);
	case MXC_CSPI_CLK:
		return get_root_clk(ECSPI1_CLK_ROOT);
	case MXC_ESDHC_CLK:
		return get_root_clk(USDHC1_CLK_ROOT);
	case MXC_ESDHC2_CLK:
		return get_root_clk(USDHC2_CLK_ROOT);
	case MXC_ESDHC3_CLK:
		return get_root_clk(USDHC3_CLK_ROOT);
	case MXC_I2C_CLK:
		return get_root_clk(I2C1_CLK_ROOT);
	case MXC_UART_CLK:
		return get_root_clk(UART1_CLK_ROOT);
	case MXC_QSPI_CLK:
		return get_root_clk(QSPI_CLK_ROOT);
	default:
		printf("Unsupported mxc_clock %d\n", clk);
		break;
	}

	return 0;
}

#if defined(CONFIG_IMX8MP) && defined(CONFIG_DWC_ETH_QOS)
static int imx8mp_eqos_interface_init(struct udevice *dev,
				      phy_interface_t interface_type)
{
	struct iomuxc_gpr_base_regs *gpr =
		(struct iomuxc_gpr_base_regs *)IOMUXC_GPR_BASE_ADDR;

	clrbits_le32(&gpr->gpr[1],
		     IOMUXC_GPR_GPR1_GPR_ENET_QOS_INTF_SEL_MASK |
		     IOMUXC_GPR_GPR1_GPR_ENET_QOS_RGMII_EN |
		     IOMUXC_GPR_GPR1_GPR_ENET_QOS_CLK_TX_CLK_SEL |
		     IOMUXC_GPR_GPR1_GPR_ENET_QOS_CLK_GEN_EN);

	switch (interface_type) {
	case PHY_INTERFACE_MODE_MII:
		setbits_le32(&gpr->gpr[1],
			     IOMUXC_GPR_GPR1_GPR_ENET_QOS_CLK_GEN_EN |
			     IOMUXC_GPR_GPR1_GPR_ENET_QOS_INTF_SEL_MII);
		break;
	case PHY_INTERFACE_MODE_RMII:
		setbits_le32(&gpr->gpr[1],
			     IOMUXC_GPR_GPR1_GPR_ENET_QOS_CLK_TX_CLK_SEL |
			     IOMUXC_GPR_GPR1_GPR_ENET_QOS_CLK_GEN_EN |
			     IOMUXC_GPR_GPR1_GPR_ENET_QOS_INTF_SEL_RMII);
		break;
	case PHY_INTERFACE_MODE_RGMII:
	case PHY_INTERFACE_MODE_RGMII_ID:
	case PHY_INTERFACE_MODE_RGMII_RXID:
	case PHY_INTERFACE_MODE_RGMII_TXID:
		setbits_le32(&gpr->gpr[1],
			     IOMUXC_GPR_GPR1_GPR_ENET_QOS_RGMII_EN |
			     IOMUXC_GPR_GPR1_GPR_ENET_QOS_CLK_GEN_EN |
			     IOMUXC_GPR_GPR1_GPR_ENET_QOS_INTF_SEL_RGMII);
		break;
	default:
		return -EINVAL;
	}

	return 0;
}
#else
static int imx8mp_eqos_interface_init(struct udevice *dev,
				      phy_interface_t interface_type)
{
	return 0;
}
#endif

#ifdef CONFIG_FEC_MXC
static int imx8mp_fec_interface_init(struct udevice *dev,
				     phy_interface_t interface_type,
				     bool mx8mp)
{
	/* i.MX8MP has extra RGMII_EN bit in IOMUXC GPR1 register */
	const u32 rgmii_en = mx8mp ? IOMUXC_GPR_GPR1_GPR_ENET1_RGMII_EN : 0;
	struct iomuxc_gpr_base_regs *gpr =
		(struct iomuxc_gpr_base_regs *)IOMUXC_GPR_BASE_ADDR;

	clrbits_le32(&gpr->gpr[1],
		     rgmii_en |
		     IOMUXC_GPR_GPR1_GPR_ENET1_TX_CLK_SEL);

	switch (interface_type) {
	case PHY_INTERFACE_MODE_MII:
	case PHY_INTERFACE_MODE_RMII:
		setbits_le32(&gpr->gpr[1], IOMUXC_GPR_GPR1_GPR_ENET1_TX_CLK_SEL);
		break;
	case PHY_INTERFACE_MODE_RGMII:
	case PHY_INTERFACE_MODE_RGMII_ID:
	case PHY_INTERFACE_MODE_RGMII_RXID:
	case PHY_INTERFACE_MODE_RGMII_TXID:
		setbits_le32(&gpr->gpr[1], rgmii_en);
		break;
	default:
		return -EINVAL;
	}

	return 0;
}
#else
static int imx8mp_fec_interface_init(struct udevice *dev,
				     phy_interface_t interface_type,
				     bool mx8mp)
{
	return 0;
}
#endif

int board_interface_eth_init(struct udevice *dev, phy_interface_t interface_type)
{
	if (IS_ENABLED(CONFIG_IMX8MM) &&
	    IS_ENABLED(CONFIG_FEC_MXC) &&
	    device_is_compatible(dev, "fsl,imx8mm-fec"))
		return imx8mp_fec_interface_init(dev, interface_type, false);

	if (IS_ENABLED(CONFIG_IMX8MN) &&
	    IS_ENABLED(CONFIG_FEC_MXC) &&
	    device_is_compatible(dev, "fsl,imx8mn-fec"))
		return imx8mp_fec_interface_init(dev, interface_type, false);

	if (IS_ENABLED(CONFIG_IMX8MP) &&
	    IS_ENABLED(CONFIG_FEC_MXC) &&
	    device_is_compatible(dev, "fsl,imx8mp-fec"))
		return imx8mp_fec_interface_init(dev, interface_type, true);

	if (IS_ENABLED(CONFIG_IMX8MP) &&
	    IS_ENABLED(CONFIG_DWC_ETH_QOS) &&
	    device_is_compatible(dev, "nxp,imx8mp-dwmac-eqos"))
		return imx8mp_eqos_interface_init(dev, interface_type);

	return -EINVAL;
}
