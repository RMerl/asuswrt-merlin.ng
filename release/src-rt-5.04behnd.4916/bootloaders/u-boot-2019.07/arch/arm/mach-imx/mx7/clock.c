// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015 Freescale Semiconductor, Inc.
 *
 * Author:
 *	Peng Fan <Peng.Fan@freescale.com>
 */

#include <common.h>
#include <div64.h>
#include <asm/io.h>
#include <linux/errno.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/crm_regs.h>
#include <asm/arch/clock.h>
#include <asm/arch/sys_proto.h>

struct mxc_ccm_anatop_reg *ccm_anatop = (struct mxc_ccm_anatop_reg *)
					 ANATOP_BASE_ADDR;
struct mxc_ccm_reg *ccm_reg = (struct mxc_ccm_reg *)CCM_BASE_ADDR;

#ifdef CONFIG_FSL_ESDHC
DECLARE_GLOBAL_DATA_PTR;
#endif

int get_clocks(void)
{
#ifdef CONFIG_FSL_ESDHC
#if CONFIG_SYS_FSL_ESDHC_ADDR == USDHC2_BASE_ADDR
	gd->arch.sdhc_clk = mxc_get_clock(MXC_ESDHC2_CLK);
#elif CONFIG_SYS_FSL_ESDHC_ADDR == USDHC3_BASE_ADDR
	gd->arch.sdhc_clk = mxc_get_clock(MXC_ESDHC3_CLK);
#else
	gd->arch.sdhc_clk = mxc_get_clock(MXC_ESDHC_CLK);
#endif
#endif
	return 0;
}

u32 get_ahb_clk(void)
{
	return get_root_clk(AHB_CLK_ROOT);
}

static u32 get_ipg_clk(void)
{
	/*
	 * The AHB and IPG are fixed at 2:1 ratio, and synchronized to
	 * each other.
	 */
	return get_ahb_clk() / 2;
}

u32 imx_get_uartclk(void)
{
	return get_root_clk(UART_CLK_ROOT);
}

u32 imx_get_fecclk(void)
{
	return get_root_clk(ENET_AXI_CLK_ROOT);
}

#ifdef CONFIG_MXC_OCOTP
void enable_ocotp_clk(unsigned char enable)
{
	clock_enable(CCGR_OCOTP, enable);
}

void enable_thermal_clk(void)
{
	enable_ocotp_clk(1);
}
#endif

void enable_usboh3_clk(unsigned char enable)
{
	u32 target;

	if (enable) {
		/* disable the clock gate first */
		clock_enable(CCGR_USB_HSIC, 0);

		/* 120Mhz */
		target = CLK_ROOT_ON |
			 USB_HSIC_CLK_ROOT_FROM_PLL_SYS_MAIN_480M_CLK |
			 CLK_ROOT_PRE_DIV(CLK_ROOT_PRE_DIV1) |
			 CLK_ROOT_POST_DIV(CLK_ROOT_POST_DIV1);
		clock_set_target_val(USB_HSIC_CLK_ROOT, target);

		/* enable the clock gate */
		clock_enable(CCGR_USB_CTRL, 1);
		clock_enable(CCGR_USB_HSIC, 1);
		clock_enable(CCGR_USB_PHY1, 1);
		clock_enable(CCGR_USB_PHY2, 1);
	} else {
		clock_enable(CCGR_USB_CTRL, 0);
		clock_enable(CCGR_USB_HSIC, 0);
		clock_enable(CCGR_USB_PHY1, 0);
		clock_enable(CCGR_USB_PHY2, 0);
	}
}

static u32 decode_pll(enum pll_clocks pll, u32 infreq)
{
	u32 reg, div_sel;
	u32 num, denom;

	/*
	 * Alought there are four choices for the bypass src,
	 * we choose OSC_24M which is the default set in ROM.
	 */
	switch (pll) {
	case PLL_CORE:
		reg = readl(&ccm_anatop->pll_arm);

		if (reg & CCM_ANALOG_PLL_ARM_POWERDOWN_MASK)
			return 0;

		if (reg & CCM_ANALOG_PLL_ARM_BYPASS_MASK)
			return MXC_HCLK;

		div_sel = (reg & CCM_ANALOG_PLL_ARM_DIV_SELECT_MASK) >>
			   CCM_ANALOG_PLL_ARM_DIV_SELECT_SHIFT;

		return (infreq * div_sel) / 2;

	case PLL_SYS:
		reg = readl(&ccm_anatop->pll_480);

		if (reg & CCM_ANALOG_PLL_480_POWERDOWN_MASK)
			return 0;

		if (reg & CCM_ANALOG_PLL_480_BYPASS_MASK)
			return MXC_HCLK;

		if (((reg & CCM_ANALOG_PLL_480_DIV_SELECT_MASK) >>
			CCM_ANALOG_PLL_480_DIV_SELECT_SHIFT) == 0)
			return 480000000u;
		else
			return 528000000u;

	case PLL_ENET:
		reg = readl(&ccm_anatop->pll_enet);

		if (reg & CCM_ANALOG_PLL_ENET_POWERDOWN_MASK)
			return 0;

		if (reg & CCM_ANALOG_PLL_ENET_BYPASS_MASK)
			return MXC_HCLK;

		return 1000000000u;

	case PLL_DDR:
		reg = readl(&ccm_anatop->pll_ddr);

		if (reg & CCM_ANALOG_PLL_DDR_POWERDOWN_MASK)
			return 0;

		num = ccm_anatop->pll_ddr_num;
		denom = ccm_anatop->pll_ddr_denom;

		if (reg & CCM_ANALOG_PLL_DDR_BYPASS_MASK)
			return MXC_HCLK;

		div_sel = (reg & CCM_ANALOG_PLL_DDR_DIV_SELECT_MASK) >>
			   CCM_ANALOG_PLL_DDR_DIV_SELECT_SHIFT;

		return infreq * (div_sel + num / denom);

	case PLL_USB:
		return 480000000u;

	default:
		printf("Unsupported pll clocks %d\n", pll);
		break;
	}

	return 0;
}

static u32 mxc_get_pll_sys_derive(int derive)
{
	u32 freq, div, frac;
	u32 reg;

	div = 1;
	reg = readl(&ccm_anatop->pll_480);
	freq = decode_pll(PLL_SYS, MXC_HCLK);

	switch (derive) {
	case PLL_SYS_MAIN_480M_CLK:
		if (reg & CCM_ANALOG_PLL_480_MAIN_DIV1_CLKGATE_MASK)
			return 0;
		else
			return freq;
	case PLL_SYS_MAIN_240M_CLK:
		if (reg & CCM_ANALOG_PLL_480_MAIN_DIV2_CLKGATE_MASK)
			return 0;
		else
			return freq / 2;
	case PLL_SYS_MAIN_120M_CLK:
		if (reg & CCM_ANALOG_PLL_480_MAIN_DIV4_CLKGATE_MASK)
			return 0;
		else
			return freq / 4;
	case PLL_SYS_PFD0_392M_CLK:
		reg = readl(&ccm_anatop->pfd_480a);
		if (reg & CCM_ANALOG_PFD_480A_PFD0_DIV1_CLKGATE_MASK)
			return 0;
		frac = (reg & CCM_ANALOG_PFD_480A_PFD0_FRAC_MASK) >>
			CCM_ANALOG_PFD_480A_PFD0_FRAC_SHIFT;
		break;
	case PLL_SYS_PFD0_196M_CLK:
		if (reg & CCM_ANALOG_PLL_480_PFD0_DIV2_CLKGATE_MASK)
			return 0;
		reg = readl(&ccm_anatop->pfd_480a);
		frac = (reg & CCM_ANALOG_PFD_480A_PFD0_FRAC_MASK) >>
			CCM_ANALOG_PFD_480A_PFD0_FRAC_SHIFT;
		div = 2;
		break;
	case PLL_SYS_PFD1_332M_CLK:
		reg = readl(&ccm_anatop->pfd_480a);
		if (reg & CCM_ANALOG_PFD_480A_PFD1_DIV1_CLKGATE_MASK)
			return 0;
		frac = (reg & CCM_ANALOG_PFD_480A_PFD1_FRAC_MASK) >>
			CCM_ANALOG_PFD_480A_PFD1_FRAC_SHIFT;
		break;
	case PLL_SYS_PFD1_166M_CLK:
		if (reg & CCM_ANALOG_PLL_480_PFD1_DIV2_CLKGATE_MASK)
			return 0;
		reg = readl(&ccm_anatop->pfd_480a);
		frac = (reg & CCM_ANALOG_PFD_480A_PFD1_FRAC_MASK) >>
			CCM_ANALOG_PFD_480A_PFD1_FRAC_SHIFT;
		div = 2;
		break;
	case PLL_SYS_PFD2_270M_CLK:
		reg = readl(&ccm_anatop->pfd_480a);
		if (reg & CCM_ANALOG_PFD_480A_PFD2_DIV1_CLKGATE_MASK)
			return 0;
		frac = (reg & CCM_ANALOG_PFD_480A_PFD2_FRAC_MASK) >>
			CCM_ANALOG_PFD_480A_PFD2_FRAC_SHIFT;
		break;
	case PLL_SYS_PFD2_135M_CLK:
		if (reg & CCM_ANALOG_PLL_480_PFD2_DIV2_CLKGATE_MASK)
			return 0;
		reg = readl(&ccm_anatop->pfd_480a);
		frac = (reg & CCM_ANALOG_PFD_480A_PFD2_FRAC_MASK) >>
			CCM_ANALOG_PFD_480A_PFD2_FRAC_SHIFT;
		div = 2;
		break;
	case PLL_SYS_PFD3_CLK:
		reg = readl(&ccm_anatop->pfd_480a);
		if (reg & CCM_ANALOG_PFD_480A_PFD3_DIV1_CLKGATE_MASK)
			return 0;
		frac = (reg & CCM_ANALOG_PFD_480A_PFD3_FRAC_MASK) >>
			CCM_ANALOG_PFD_480A_PFD3_FRAC_SHIFT;
		break;
	case PLL_SYS_PFD4_CLK:
		reg = readl(&ccm_anatop->pfd_480b);
		if (reg & CCM_ANALOG_PFD_480B_PFD4_DIV1_CLKGATE_MASK)
			return 0;
		frac = (reg & CCM_ANALOG_PFD_480B_PFD4_FRAC_MASK) >>
			CCM_ANALOG_PFD_480B_PFD4_FRAC_SHIFT;
		break;
	case PLL_SYS_PFD5_CLK:
		reg = readl(&ccm_anatop->pfd_480b);
		if (reg & CCM_ANALOG_PFD_480B_PFD5_DIV1_CLKGATE_MASK)
			return 0;
		frac = (reg & CCM_ANALOG_PFD_480B_PFD5_FRAC_MASK) >>
			CCM_ANALOG_PFD_480B_PFD5_FRAC_SHIFT;
		break;
	case PLL_SYS_PFD6_CLK:
		reg = readl(&ccm_anatop->pfd_480b);
		if (reg & CCM_ANALOG_PFD_480B_PFD6_DIV1_CLKGATE_MASK)
			return 0;
		frac = (reg & CCM_ANALOG_PFD_480B_PFD6_FRAC_MASK) >>
			CCM_ANALOG_PFD_480B_PFD6_FRAC_SHIFT;
		break;
	case PLL_SYS_PFD7_CLK:
		reg = readl(&ccm_anatop->pfd_480b);
		if (reg & CCM_ANALOG_PFD_480B_PFD7_DIV1_CLKGATE_MASK)
			return 0;
		frac = (reg & CCM_ANALOG_PFD_480B_PFD7_FRAC_MASK) >>
			CCM_ANALOG_PFD_480B_PFD7_FRAC_SHIFT;
		break;
	default:
		printf("Error derived pll_sys clock %d\n", derive);
		return 0;
	}

	return ((freq / frac) * 18) / div;
}

static u32 mxc_get_pll_enet_derive(int derive)
{
	u32 freq, reg;

	freq = decode_pll(PLL_ENET, MXC_HCLK);
	reg = readl(&ccm_anatop->pll_enet);

	switch (derive) {
	case PLL_ENET_MAIN_500M_CLK:
		if (reg & CCM_ANALOG_PLL_ENET_ENABLE_CLK_500MHZ_MASK)
			return freq / 2;
		break;
	case PLL_ENET_MAIN_250M_CLK:
		if (reg & CCM_ANALOG_PLL_ENET_ENABLE_CLK_250MHZ_MASK)
			return freq / 4;
		break;
	case PLL_ENET_MAIN_125M_CLK:
		if (reg & CCM_ANALOG_PLL_ENET_ENABLE_CLK_125MHZ_MASK)
			return freq / 8;
		break;
	case PLL_ENET_MAIN_100M_CLK:
		if (reg & CCM_ANALOG_PLL_ENET_ENABLE_CLK_100MHZ_MASK)
			return freq / 10;
		break;
	case PLL_ENET_MAIN_50M_CLK:
		if (reg & CCM_ANALOG_PLL_ENET_ENABLE_CLK_50MHZ_MASK)
			return freq / 20;
		break;
	case PLL_ENET_MAIN_40M_CLK:
		if (reg & CCM_ANALOG_PLL_ENET_ENABLE_CLK_40MHZ_MASK)
			return freq / 25;
		break;
	case PLL_ENET_MAIN_25M_CLK:
		if (reg & CCM_ANALOG_PLL_ENET_ENABLE_CLK_25MHZ_MASK)
			return freq / 40;
		break;
	default:
		printf("Error derived pll_enet clock %d\n", derive);
		break;
	}

	return 0;
}

static u32 mxc_get_pll_ddr_derive(int derive)
{
	u32 freq, reg;

	freq = decode_pll(PLL_DDR, MXC_HCLK);
	reg = readl(&ccm_anatop->pll_ddr);

	switch (derive) {
	case PLL_DRAM_MAIN_1066M_CLK:
		return freq;
	case PLL_DRAM_MAIN_533M_CLK:
		if (reg & CCM_ANALOG_PLL_DDR_DIV2_ENABLE_CLK_MASK)
			return freq / 2;
		break;
	default:
		printf("Error derived pll_ddr clock %d\n", derive);
		break;
	}

	return 0;
}

static u32 mxc_get_pll_derive(enum pll_clocks pll, int derive)
{
	switch (pll) {
	case PLL_SYS:
		return mxc_get_pll_sys_derive(derive);
	case PLL_ENET:
		return mxc_get_pll_enet_derive(derive);
	case PLL_DDR:
		return mxc_get_pll_ddr_derive(derive);
	default:
		printf("Error pll.\n");
		return 0;
	}
}

static u32 get_root_src_clk(enum clk_root_src root_src)
{
	switch (root_src) {
	case OSC_24M_CLK:
		return 24000000u;
	case PLL_ARM_MAIN_800M_CLK:
		return decode_pll(PLL_CORE, MXC_HCLK);

	case PLL_SYS_MAIN_480M_CLK:
	case PLL_SYS_MAIN_240M_CLK:
	case PLL_SYS_MAIN_120M_CLK:
	case PLL_SYS_PFD0_392M_CLK:
	case PLL_SYS_PFD0_196M_CLK:
	case PLL_SYS_PFD1_332M_CLK:
	case PLL_SYS_PFD1_166M_CLK:
	case PLL_SYS_PFD2_270M_CLK:
	case PLL_SYS_PFD2_135M_CLK:
	case PLL_SYS_PFD3_CLK:
	case PLL_SYS_PFD4_CLK:
	case PLL_SYS_PFD5_CLK:
	case PLL_SYS_PFD6_CLK:
	case PLL_SYS_PFD7_CLK:
		return mxc_get_pll_derive(PLL_SYS, root_src);

	case PLL_ENET_MAIN_500M_CLK:
	case PLL_ENET_MAIN_250M_CLK:
	case PLL_ENET_MAIN_125M_CLK:
	case PLL_ENET_MAIN_100M_CLK:
	case PLL_ENET_MAIN_50M_CLK:
	case PLL_ENET_MAIN_40M_CLK:
	case PLL_ENET_MAIN_25M_CLK:
		return mxc_get_pll_derive(PLL_ENET, root_src);

	case PLL_DRAM_MAIN_1066M_CLK:
	case PLL_DRAM_MAIN_533M_CLK:
		return mxc_get_pll_derive(PLL_DDR, root_src);

	case PLL_AUDIO_MAIN_CLK:
		return decode_pll(PLL_AUDIO, MXC_HCLK);
	case PLL_VIDEO_MAIN_CLK:
		return decode_pll(PLL_VIDEO, MXC_HCLK);

	case PLL_USB_MAIN_480M_CLK:
		return decode_pll(PLL_USB, MXC_HCLK);

	case REF_1M_CLK:
		return 1000000;
	case OSC_32K_CLK:
		return MXC_CLK32;

	case EXT_CLK_1:
	case EXT_CLK_2:
	case EXT_CLK_3:
	case EXT_CLK_4:
		printf("No EXT CLK supported??\n");
		break;
	};

	return 0;
}

u32 get_root_clk(enum clk_root_index clock_id)
{
	enum clk_root_src root_src;
	u32 post_podf, pre_podf, auto_podf, root_src_clk;
	int auto_en;

	if (clock_root_enabled(clock_id) <= 0)
		return 0;

	if (clock_get_prediv(clock_id, &pre_podf) < 0)
		return 0;

	if (clock_get_postdiv(clock_id, &post_podf) < 0)
		return 0;

	if (clock_get_autopostdiv(clock_id, &auto_podf, &auto_en) < 0)
		return 0;

	if (auto_en == 0)
		auto_podf = 0;

	if (clock_get_src(clock_id, &root_src) < 0)
		return 0;

	root_src_clk = get_root_src_clk(root_src);

	/*
	 * bypass clk is ignored.
	 */

	return root_src_clk / (post_podf + 1) / (pre_podf + 1) /
		(auto_podf + 1);
}

static u32 get_ddrc_clk(void)
{
	u32 reg, freq;
	enum root_post_div post_div;

	reg = readl(&ccm_reg->root[DRAM_CLK_ROOT].target_root);
	if (reg & CLK_ROOT_MUX_MASK)
		/* DRAM_ALT_CLK_ROOT */
		freq = get_root_clk(DRAM_ALT_CLK_ROOT);
	else
		/* PLL_DRAM_MAIN_1066M_CLK */
		freq = mxc_get_pll_derive(PLL_DDR, PLL_DRAM_MAIN_1066M_CLK);

	post_div = reg & DRAM_CLK_ROOT_POST_DIV_MASK;

	return freq / (post_div + 1) / 2;
}

unsigned int mxc_get_clock(enum mxc_clock clk)
{
	switch (clk) {
	case MXC_ARM_CLK:
		return get_root_clk(ARM_A7_CLK_ROOT);
	case MXC_AXI_CLK:
		return get_root_clk(MAIN_AXI_CLK_ROOT);
	case MXC_AHB_CLK:
		return get_root_clk(AHB_CLK_ROOT);
	case MXC_IPG_CLK:
		return get_ipg_clk();
	case MXC_I2C_CLK:
		return get_root_clk(I2C1_CLK_ROOT);
	case MXC_UART_CLK:
		return get_root_clk(UART1_CLK_ROOT);
	case MXC_CSPI_CLK:
		return get_root_clk(ECSPI1_CLK_ROOT);
	case MXC_DDR_CLK:
		return get_ddrc_clk();
	case MXC_ESDHC_CLK:
		return get_root_clk(USDHC1_CLK_ROOT);
	case MXC_ESDHC2_CLK:
		return get_root_clk(USDHC2_CLK_ROOT);
	case MXC_ESDHC3_CLK:
		return get_root_clk(USDHC3_CLK_ROOT);
	default:
		printf("Unsupported mxc_clock %d\n", clk);
		break;
	}

	return 0;
}

#ifdef CONFIG_SYS_I2C_MXC
/* i2c_num can be 0 - 3 */
int enable_i2c_clk(unsigned char enable, unsigned i2c_num)
{
	u32 target;

	if (i2c_num >= 4)
		return -EINVAL;

	if (enable) {
		clock_enable(CCGR_I2C1 + i2c_num, 0);

		/* Set i2c root clock to PLL_SYS_MAIN_120M_CLK */

		target = CLK_ROOT_ON |
			 I2C1_CLK_ROOT_FROM_PLL_SYS_MAIN_120M_CLK |
			 CLK_ROOT_PRE_DIV(CLK_ROOT_PRE_DIV1) |
			 CLK_ROOT_POST_DIV(CLK_ROOT_POST_DIV2);
		clock_set_target_val(I2C1_CLK_ROOT + i2c_num, target);

		clock_enable(CCGR_I2C1 + i2c_num, 1);
	} else {
		clock_enable(CCGR_I2C1 + i2c_num, 0);
	}

	return 0;
}
#endif

static void init_clk_esdhc(void)
{
	u32 target;

	/* disable the clock gate first */
	clock_enable(CCGR_USDHC1, 0);
	clock_enable(CCGR_USDHC2, 0);
	clock_enable(CCGR_USDHC3, 0);

	/* 196: 392/2 */
	target = CLK_ROOT_ON | USDHC1_CLK_ROOT_FROM_PLL_SYS_PFD0_392M_CLK |
		 CLK_ROOT_PRE_DIV(CLK_ROOT_PRE_DIV1) |
		 CLK_ROOT_POST_DIV(CLK_ROOT_POST_DIV2);
	clock_set_target_val(USDHC1_CLK_ROOT, target);

	target = CLK_ROOT_ON | USDHC1_CLK_ROOT_FROM_PLL_SYS_PFD0_392M_CLK |
		 CLK_ROOT_PRE_DIV(CLK_ROOT_PRE_DIV1) |
		 CLK_ROOT_POST_DIV(CLK_ROOT_POST_DIV2);
	clock_set_target_val(USDHC2_CLK_ROOT, target);

	target = CLK_ROOT_ON | USDHC1_CLK_ROOT_FROM_PLL_SYS_PFD0_392M_CLK |
		 CLK_ROOT_PRE_DIV(CLK_ROOT_PRE_DIV1) |
		 CLK_ROOT_POST_DIV(CLK_ROOT_POST_DIV2);
	clock_set_target_val(USDHC3_CLK_ROOT, target);

	/* enable the clock gate */
	clock_enable(CCGR_USDHC1, 1);
	clock_enable(CCGR_USDHC2, 1);
	clock_enable(CCGR_USDHC3, 1);
}

static void init_clk_uart(void)
{
	u32 target;

	/* disable the clock gate first */
	clock_enable(CCGR_UART1, 0);
	clock_enable(CCGR_UART2, 0);
	clock_enable(CCGR_UART3, 0);
	clock_enable(CCGR_UART4, 0);
	clock_enable(CCGR_UART5, 0);
	clock_enable(CCGR_UART6, 0);
	clock_enable(CCGR_UART7, 0);

	/* 24Mhz */
	target = CLK_ROOT_ON | UART1_CLK_ROOT_FROM_OSC_24M_CLK |
		 CLK_ROOT_PRE_DIV(CLK_ROOT_PRE_DIV1) |
		 CLK_ROOT_POST_DIV(CLK_ROOT_POST_DIV1);
	clock_set_target_val(UART1_CLK_ROOT, target);

	target = CLK_ROOT_ON | UART2_CLK_ROOT_FROM_OSC_24M_CLK |
		 CLK_ROOT_PRE_DIV(CLK_ROOT_PRE_DIV1) |
		 CLK_ROOT_POST_DIV(CLK_ROOT_POST_DIV1);
	clock_set_target_val(UART2_CLK_ROOT, target);

	target = CLK_ROOT_ON | UART3_CLK_ROOT_FROM_OSC_24M_CLK |
		 CLK_ROOT_PRE_DIV(CLK_ROOT_PRE_DIV1) |
		 CLK_ROOT_POST_DIV(CLK_ROOT_POST_DIV1);
	clock_set_target_val(UART3_CLK_ROOT, target);

	target = CLK_ROOT_ON | UART4_CLK_ROOT_FROM_OSC_24M_CLK |
		 CLK_ROOT_PRE_DIV(CLK_ROOT_PRE_DIV1) |
		 CLK_ROOT_POST_DIV(CLK_ROOT_POST_DIV1);
	clock_set_target_val(UART4_CLK_ROOT, target);

	target = CLK_ROOT_ON | UART5_CLK_ROOT_FROM_OSC_24M_CLK |
		 CLK_ROOT_PRE_DIV(CLK_ROOT_PRE_DIV1) |
		 CLK_ROOT_POST_DIV(CLK_ROOT_POST_DIV1);
	clock_set_target_val(UART5_CLK_ROOT, target);

	target = CLK_ROOT_ON | UART6_CLK_ROOT_FROM_OSC_24M_CLK |
		 CLK_ROOT_PRE_DIV(CLK_ROOT_PRE_DIV1) |
		 CLK_ROOT_POST_DIV(CLK_ROOT_POST_DIV1);
	clock_set_target_val(UART6_CLK_ROOT, target);

	target = CLK_ROOT_ON | UART7_CLK_ROOT_FROM_OSC_24M_CLK |
		 CLK_ROOT_PRE_DIV(CLK_ROOT_PRE_DIV1) |
		 CLK_ROOT_POST_DIV(CLK_ROOT_POST_DIV1);
	clock_set_target_val(UART7_CLK_ROOT, target);

	/* enable the clock gate */
	clock_enable(CCGR_UART1, 1);
	clock_enable(CCGR_UART2, 1);
	clock_enable(CCGR_UART3, 1);
	clock_enable(CCGR_UART4, 1);
	clock_enable(CCGR_UART5, 1);
	clock_enable(CCGR_UART6, 1);
	clock_enable(CCGR_UART7, 1);
}

static void init_clk_weim(void)
{
	u32 target;

	/* disable the clock gate first */
	clock_enable(CCGR_WEIM, 0);

	/* 120Mhz */
	target = CLK_ROOT_ON | EIM_CLK_ROOT_FROM_PLL_SYS_MAIN_120M_CLK |
		 CLK_ROOT_PRE_DIV(CLK_ROOT_PRE_DIV1) |
		 CLK_ROOT_POST_DIV(CLK_ROOT_POST_DIV1);
	clock_set_target_val(EIM_CLK_ROOT, target);

	/* enable the clock gate */
	clock_enable(CCGR_WEIM, 1);
}

static void init_clk_ecspi(void)
{
	u32 target;

	/* disable the clock gate first */
	clock_enable(CCGR_ECSPI1, 0);
	clock_enable(CCGR_ECSPI2, 0);
	clock_enable(CCGR_ECSPI3, 0);
	clock_enable(CCGR_ECSPI4, 0);

	/* 60Mhz: 240/4 */
	target = CLK_ROOT_ON | ECSPI1_CLK_ROOT_FROM_PLL_SYS_MAIN_240M_CLK |
		 CLK_ROOT_PRE_DIV(CLK_ROOT_PRE_DIV1) |
		 CLK_ROOT_POST_DIV(CLK_ROOT_POST_DIV4);
	clock_set_target_val(ECSPI1_CLK_ROOT, target);

	target = CLK_ROOT_ON | ECSPI2_CLK_ROOT_FROM_PLL_SYS_MAIN_240M_CLK |
		 CLK_ROOT_PRE_DIV(CLK_ROOT_PRE_DIV1) |
		 CLK_ROOT_POST_DIV(CLK_ROOT_POST_DIV4);
	clock_set_target_val(ECSPI2_CLK_ROOT, target);

	target = CLK_ROOT_ON | ECSPI3_CLK_ROOT_FROM_PLL_SYS_MAIN_240M_CLK |
		 CLK_ROOT_PRE_DIV(CLK_ROOT_PRE_DIV1) |
		 CLK_ROOT_POST_DIV(CLK_ROOT_POST_DIV4);
	clock_set_target_val(ECSPI3_CLK_ROOT, target);

	target = CLK_ROOT_ON | ECSPI4_CLK_ROOT_FROM_PLL_SYS_MAIN_240M_CLK |
		 CLK_ROOT_PRE_DIV(CLK_ROOT_PRE_DIV1) |
		 CLK_ROOT_POST_DIV(CLK_ROOT_POST_DIV4);
	clock_set_target_val(ECSPI4_CLK_ROOT, target);

	/* enable the clock gate */
	clock_enable(CCGR_ECSPI1, 1);
	clock_enable(CCGR_ECSPI2, 1);
	clock_enable(CCGR_ECSPI3, 1);
	clock_enable(CCGR_ECSPI4, 1);
}

static void init_clk_wdog(void)
{
	u32 target;

	/* disable the clock gate first */
	clock_enable(CCGR_WDOG1, 0);
	clock_enable(CCGR_WDOG2, 0);
	clock_enable(CCGR_WDOG3, 0);
	clock_enable(CCGR_WDOG4, 0);

	/* 24Mhz */
	target = CLK_ROOT_ON | WDOG_CLK_ROOT_FROM_OSC_24M_CLK |
		 CLK_ROOT_PRE_DIV(CLK_ROOT_PRE_DIV1) |
		 CLK_ROOT_POST_DIV(CLK_ROOT_POST_DIV1);
	clock_set_target_val(WDOG_CLK_ROOT, target);

	/* enable the clock gate */
	clock_enable(CCGR_WDOG1, 1);
	clock_enable(CCGR_WDOG2, 1);
	clock_enable(CCGR_WDOG3, 1);
	clock_enable(CCGR_WDOG4, 1);
}

#ifdef CONFIG_MXC_EPDC
static void init_clk_epdc(void)
{
	u32 target;

	/* disable the clock gate first */
	clock_enable(CCGR_EPDC, 0);

	/* 24Mhz */
	target = CLK_ROOT_ON | EPDC_PIXEL_CLK_ROOT_FROM_PLL_SYS_MAIN_480M_CLK |
		 CLK_ROOT_PRE_DIV(CLK_ROOT_PRE_DIV1) |
		 CLK_ROOT_POST_DIV(CLK_ROOT_POST_DIV12);
	clock_set_target_val(EPDC_PIXEL_CLK_ROOT, target);

	/* enable the clock gate */
	clock_enable(CCGR_EPDC, 1);
}
#endif

static int enable_pll_enet(void)
{
	u32 reg;
	s32 timeout = 100000;

	reg = readl(&ccm_anatop->pll_enet);
	/* If pll_enet powered up, no need to set it again */
	if (reg & ANADIG_PLL_ENET_PWDN_MASK) {
		reg &= ~ANADIG_PLL_ENET_PWDN_MASK;
		writel(reg, &ccm_anatop->pll_enet);

		while (timeout--) {
			if (readl(&ccm_anatop->pll_enet) & ANADIG_PLL_LOCK)
				break;
		}

		if (timeout <= 0) {
			/* If timeout, we set pwdn for pll_enet. */
			reg |= ANADIG_PLL_ENET_PWDN_MASK;
			return -ETIME;
		}
	}

	/* Clear bypass */
	writel(CCM_ANALOG_PLL_ENET_BYPASS_MASK, &ccm_anatop->pll_enet_clr);

	writel((CCM_ANALOG_PLL_ENET_ENABLE_CLK_500MHZ_MASK
		| CCM_ANALOG_PLL_ENET_ENABLE_CLK_250MHZ_MASK
		| CCM_ANALOG_PLL_ENET_ENABLE_CLK_125MHZ_MASK
		| CCM_ANALOG_PLL_ENET_ENABLE_CLK_100MHZ_MASK
		| CCM_ANALOG_PLL_ENET_ENABLE_CLK_50MHZ_MASK
		| CCM_ANALOG_PLL_ENET_ENABLE_CLK_40MHZ_MASK
		| CCM_ANALOG_PLL_ENET_ENABLE_CLK_25MHZ_MASK),
	       &ccm_anatop->pll_enet_set);

	return 0;
}
static int enable_pll_video(u32 pll_div, u32 pll_num, u32 pll_denom,
	u32 post_div)
{
	u32 reg = 0;
	ulong start;

	debug("pll5 div = %d, num = %d, denom = %d\n",
		pll_div, pll_num, pll_denom);

	/* Power up PLL5 video and disable its output */
	writel(CCM_ANALOG_PLL_VIDEO_CLR_ENABLE_CLK_MASK |
		CCM_ANALOG_PLL_VIDEO_CLR_POWERDOWN_MASK |
		CCM_ANALOG_PLL_VIDEO_CLR_BYPASS_MASK |
		CCM_ANALOG_PLL_VIDEO_CLR_DIV_SELECT_MASK |
		CCM_ANALOG_PLL_VIDEO_CLR_POST_DIV_SEL_MASK |
		CCM_ANALOG_PLL_VIDEO_CLR_TEST_DIV_SELECT_MASK,
		&ccm_anatop->pll_video_clr);

	/* Set div, num and denom */
	switch (post_div) {
	case 1:
		writel(CCM_ANALOG_PLL_VIDEO_SET_DIV_SELECT(pll_div) |
			CCM_ANALOG_PLL_VIDEO_SET_TEST_DIV_SELECT(0x1) |
			CCM_ANALOG_PLL_VIDEO_SET_POST_DIV_SEL(0x0),
			&ccm_anatop->pll_video_set);
		break;
	case 2:
		writel(CCM_ANALOG_PLL_VIDEO_SET_DIV_SELECT(pll_div) |
			CCM_ANALOG_PLL_VIDEO_SET_TEST_DIV_SELECT(0x0) |
			CCM_ANALOG_PLL_VIDEO_SET_POST_DIV_SEL(0x0),
			&ccm_anatop->pll_video_set);
		break;
	case 3:
		writel(CCM_ANALOG_PLL_VIDEO_SET_DIV_SELECT(pll_div) |
			CCM_ANALOG_PLL_VIDEO_SET_TEST_DIV_SELECT(0x0) |
			CCM_ANALOG_PLL_VIDEO_SET_POST_DIV_SEL(0x1),
			&ccm_anatop->pll_video_set);
		break;
	case 4:
		writel(CCM_ANALOG_PLL_VIDEO_SET_DIV_SELECT(pll_div) |
			CCM_ANALOG_PLL_VIDEO_SET_TEST_DIV_SELECT(0x0) |
			CCM_ANALOG_PLL_VIDEO_SET_POST_DIV_SEL(0x3),
			&ccm_anatop->pll_video_set);
		break;
	case 0:
	default:
		writel(CCM_ANALOG_PLL_VIDEO_SET_DIV_SELECT(pll_div) |
			CCM_ANALOG_PLL_VIDEO_SET_TEST_DIV_SELECT(0x2) |
			CCM_ANALOG_PLL_VIDEO_SET_POST_DIV_SEL(0x0),
			&ccm_anatop->pll_video_set);
		break;
	}

	writel(CCM_ANALOG_PLL_VIDEO_NUM_A(pll_num),
		&ccm_anatop->pll_video_num);

	writel(CCM_ANALOG_PLL_VIDEO_DENOM_B(pll_denom),
		&ccm_anatop->pll_video_denom);

	/* Wait PLL5 lock */
	start = get_timer(0);	/* Get current timestamp */

	do {
		reg = readl(&ccm_anatop->pll_video);
		if (reg & CCM_ANALOG_PLL_VIDEO_LOCK_MASK) {
			/* Enable PLL out */
			writel(CCM_ANALOG_PLL_VIDEO_CLR_ENABLE_CLK_MASK,
					&ccm_anatop->pll_video_set);
			return 0;
		}
	} while (get_timer(0) < (start + 10)); /* Wait 10ms */

	printf("Lock PLL5 timeout\n");

	return 1;
}

int set_clk_qspi(void)
{
	u32 target;

	/* disable the clock gate first */
	clock_enable(CCGR_QSPI, 0);

	/* 49M: 392/2/4 */
	target = CLK_ROOT_ON | QSPI_CLK_ROOT_FROM_PLL_SYS_PFD4_CLK |
		 CLK_ROOT_PRE_DIV(CLK_ROOT_PRE_DIV1) |
		 CLK_ROOT_POST_DIV(CLK_ROOT_POST_DIV2);
	clock_set_target_val(QSPI_CLK_ROOT, target);

	/* enable the clock gate */
	clock_enable(CCGR_QSPI, 1);

	return 0;
}

int set_clk_nand(void)
{
	u32 target;

	/* disable the clock gate first */
	clock_enable(CCGR_RAWNAND, 0);

	enable_pll_enet();
	/* 100: 500/5 */
	target = CLK_ROOT_ON | NAND_CLK_ROOT_FROM_PLL_ENET_MAIN_500M_CLK |
		 CLK_ROOT_PRE_DIV(CLK_ROOT_PRE_DIV1) |
		 CLK_ROOT_POST_DIV(CLK_ROOT_POST_DIV5);
	clock_set_target_val(NAND_CLK_ROOT, target);

	/* enable the clock gate */
	clock_enable(CCGR_RAWNAND, 1);

	return 0;
}

void mxs_set_lcdclk(uint32_t base_addr, uint32_t freq)
{
	u32 hck = MXC_HCLK/1000;
	u32 min = hck * 27;
	u32 max = hck * 54;
	u32 temp, best = 0;
	u32 i, j, pred = 1, postd = 1;
	u32 pll_div, pll_num, pll_denom, post_div = 0;
	u32 target;

	debug("mxs_set_lcdclk, freq = %d\n", freq);

	clock_enable(CCGR_LCDIF, 0);

	temp = (freq * 8 * 8);
	if (temp < min) {
		for (i = 1; i <= 4; i++) {
			if ((temp * (1 << i)) > min) {
				post_div = i;
				freq = (freq * (1 << i));
				break;
			}
		}

		if (5 == i) {
			printf("Fail to set rate to %dkhz", freq);
			return;
		}
	}

	for (i = 1; i <= 8; i++) {
		for (j = 1; j <= 8; j++) {
			temp = freq * i * j;
			if (temp > max || temp < min)
				continue;

			if (best == 0 || temp < best) {
				best = temp;
				pred = i;
				postd = j;
			}
		}
	}

	if (best == 0) {
		printf("Fail to set rate to %dkhz", freq);
		return;
	}

	debug("best %d, pred = %d, postd = %d\n", best, pred, postd);

	pll_div = best / hck;
	pll_denom = 1000000;
	pll_num = (best - hck * pll_div) * pll_denom / hck;

	if (enable_pll_video(pll_div, pll_num, pll_denom, post_div))
		return;

	target = CLK_ROOT_ON | LCDIF_PIXEL_CLK_ROOT_FROM_PLL_VIDEO_MAIN_CLK |
		 CLK_ROOT_PRE_DIV((pred - 1)) | CLK_ROOT_POST_DIV((postd - 1));
	clock_set_target_val(LCDIF_PIXEL_CLK_ROOT, target);

	clock_enable(CCGR_LCDIF, 1);
}

#ifdef CONFIG_FEC_MXC
int set_clk_enet(enum enet_freq type)
{
	u32 target;
	int ret;
	u32 enet1_ref, enet2_ref;

	/* disable the clock first */
	clock_enable(CCGR_ENET1, 0);
	clock_enable(CCGR_ENET2, 0);

	switch (type) {
	case ENET_125MHZ:
		enet1_ref = ENET1_REF_CLK_ROOT_FROM_PLL_ENET_MAIN_125M_CLK;
		enet2_ref = ENET2_REF_CLK_ROOT_FROM_PLL_ENET_MAIN_125M_CLK;
		break;
	case ENET_50MHZ:
		enet1_ref = ENET1_REF_CLK_ROOT_FROM_PLL_ENET_MAIN_50M_CLK;
		enet2_ref = ENET2_REF_CLK_ROOT_FROM_PLL_ENET_MAIN_50M_CLK;
		break;
	case ENET_25MHZ:
		enet1_ref = ENET1_REF_CLK_ROOT_FROM_PLL_ENET_MAIN_25M_CLK;
		enet2_ref = ENET2_REF_CLK_ROOT_FROM_PLL_ENET_MAIN_25M_CLK;
		break;
	default:
		return -EINVAL;
	}

	ret = enable_pll_enet();
	if (ret != 0)
		return ret;

	/* set enet axi clock 196M: 392/2 */
	target = CLK_ROOT_ON | ENET_AXI_CLK_ROOT_FROM_PLL_SYS_PFD4_CLK |
		 CLK_ROOT_PRE_DIV(CLK_ROOT_PRE_DIV1) |
		 CLK_ROOT_POST_DIV(CLK_ROOT_POST_DIV2);
	clock_set_target_val(ENET_AXI_CLK_ROOT, target);

	target = CLK_ROOT_ON | enet1_ref |
		 CLK_ROOT_PRE_DIV(CLK_ROOT_PRE_DIV1) |
		 CLK_ROOT_POST_DIV(CLK_ROOT_POST_DIV1);
	clock_set_target_val(ENET1_REF_CLK_ROOT, target);

	target = CLK_ROOT_ON | ENET1_TIME_CLK_ROOT_FROM_PLL_ENET_MAIN_100M_CLK |
		 CLK_ROOT_PRE_DIV(CLK_ROOT_PRE_DIV1) |
		 CLK_ROOT_POST_DIV(CLK_ROOT_POST_DIV4);
	clock_set_target_val(ENET1_TIME_CLK_ROOT, target);

	target = CLK_ROOT_ON | enet2_ref |
		 CLK_ROOT_PRE_DIV(CLK_ROOT_PRE_DIV1) |
		 CLK_ROOT_POST_DIV(CLK_ROOT_POST_DIV1);
	clock_set_target_val(ENET2_REF_CLK_ROOT, target);

	target = CLK_ROOT_ON | ENET2_TIME_CLK_ROOT_FROM_PLL_ENET_MAIN_100M_CLK |
		 CLK_ROOT_PRE_DIV(CLK_ROOT_PRE_DIV1) |
		 CLK_ROOT_POST_DIV(CLK_ROOT_POST_DIV4);
	clock_set_target_val(ENET2_TIME_CLK_ROOT, target);

#ifdef CONFIG_FEC_MXC_25M_REF_CLK
	target = CLK_ROOT_ON |
		 ENET_PHY_REF_CLK_ROOT_FROM_PLL_ENET_MAIN_25M_CLK |
		 CLK_ROOT_PRE_DIV(CLK_ROOT_PRE_DIV1) |
		 CLK_ROOT_POST_DIV(CLK_ROOT_POST_DIV1);
	clock_set_target_val(ENET_PHY_REF_CLK_ROOT, target);
#endif
	/* enable clock */
	clock_enable(CCGR_ENET1, 1);
	clock_enable(CCGR_ENET2, 1);

	return 0;
}
#endif

/* Configure PLL/PFD freq */
void clock_init(void)
{
/* Rom has enabled PLL_ARM, PLL_DDR, PLL_SYS, PLL_ENET
 *   In u-boot, we have to:
 *   1. Configure PFD3- PFD7 for freq we needed in u-boot
 *   2. Set clock root for peripherals (ip channel) used in u-boot but without set rate
 *       interface.  The clocks for these peripherals are enabled after this intialization.
 *   3. Other peripherals with set clock rate interface does not be set in this function.
 */
	u32 reg;

	/*
	 * Configure PFD4 to 392M
	 * 480M * 18 / 0x16 = 392M
	 */
	reg = readl(&ccm_anatop->pfd_480b);

	reg &= ~(ANATOP_PFD480B_PFD4_FRAC_MASK |
		 CCM_ANALOG_PFD_480B_PFD4_DIV1_CLKGATE_MASK);
	reg |= ANATOP_PFD480B_PFD4_FRAC_392M_VAL;

	writel(reg, &ccm_anatop->pfd_480b);

	init_clk_esdhc();
	init_clk_uart();
	init_clk_weim();
	init_clk_ecspi();
	init_clk_wdog();
#ifdef CONFIG_MXC_EPDC
	init_clk_epdc();
#endif

	enable_usboh3_clk(1);

	clock_enable(CCGR_SNVS, 1);

#ifdef CONFIG_NAND_MXS
	clock_enable(CCGR_RAWNAND, 1);
#endif

	if (IS_ENABLED(CONFIG_IMX_RDC)) {
		clock_enable(CCGR_RDC, 1);
		clock_enable(CCGR_SEMA1, 1);
		clock_enable(CCGR_SEMA2, 1);
	}
}

#ifdef CONFIG_SECURE_BOOT
void hab_caam_clock_enable(unsigned char enable)
{
	if (enable)
		clock_enable(CCGR_CAAM, 1);
	else
		clock_enable(CCGR_CAAM, 0);
}
#endif

#ifdef CONFIG_MXC_EPDC
void epdc_clock_enable(void)
{
	clock_enable(CCGR_EPDC, 1);
}
void epdc_clock_disable(void)
{
	clock_enable(CCGR_EPDC, 0);
}
#endif

#ifndef CONFIG_SPL_BUILD
/*
 * Dump some core clockes.
 */
int do_mx7_showclocks(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	u32 freq;
	freq = decode_pll(PLL_CORE, MXC_HCLK);
	printf("PLL_CORE    %8d MHz\n", freq / 1000000);
	freq = decode_pll(PLL_SYS, MXC_HCLK);
	printf("PLL_SYS    %8d MHz\n", freq / 1000000);
	freq = decode_pll(PLL_ENET, MXC_HCLK);
	printf("PLL_NET    %8d MHz\n", freq / 1000000);

	printf("\n");

	printf("IPG        %8d kHz\n", mxc_get_clock(MXC_IPG_CLK) / 1000);
	printf("UART       %8d kHz\n", mxc_get_clock(MXC_UART_CLK) / 1000);
#ifdef CONFIG_MXC_SPI
	printf("CSPI       %8d kHz\n", mxc_get_clock(MXC_CSPI_CLK) / 1000);
#endif
	printf("AHB        %8d kHz\n", mxc_get_clock(MXC_AHB_CLK) / 1000);
	printf("AXI        %8d kHz\n", mxc_get_clock(MXC_AXI_CLK) / 1000);
	printf("DDR        %8d kHz\n", mxc_get_clock(MXC_DDR_CLK) / 1000);
	printf("USDHC1     %8d kHz\n", mxc_get_clock(MXC_ESDHC_CLK) / 1000);
	printf("USDHC2     %8d kHz\n", mxc_get_clock(MXC_ESDHC2_CLK) / 1000);
	printf("USDHC3     %8d kHz\n", mxc_get_clock(MXC_ESDHC3_CLK) / 1000);

	return 0;
}

U_BOOT_CMD(
	clocks,	CONFIG_SYS_MAXARGS, 1, do_mx7_showclocks,
	"display clocks",
	""
);
#endif
