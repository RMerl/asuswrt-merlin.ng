// SPDX-License-Identifier: GPL-2.0
/*
 * MediaTek clock driver for MT8516 SoC
 *
 * Copyright (C) 2018 BayLibre, SAS
 * Author: Fabien Parent <fparent@baylibre.com>
 */

#include <common.h>
#include <dm.h>
#include <asm/io.h>
#include <dt-bindings/clock/mt8516-clk.h>
#include <linux/bitops.h>

#include "clk-mtk.h"

#define MT8516_PLL_FMAX		(1502UL * MHZ)
#define MT8516_CON0_RST_BAR	BIT(27)

/* apmixedsys */
#define PLL(_id, _reg, _pwr_reg, _en_mask, _flags, _pcwbits, _pd_reg,	\
	    _pd_shift, _pcw_reg, _pcw_shift) {				\
		.id = _id,						\
		.reg = _reg,						\
		.pwr_reg = _pwr_reg,					\
		.en_mask = _en_mask,					\
		.rst_bar_mask = MT8516_CON0_RST_BAR,			\
		.fmax = MT8516_PLL_FMAX,				\
		.flags = _flags,					\
		.pcwbits = _pcwbits,					\
		.pd_reg = _pd_reg,					\
		.pd_shift = _pd_shift,					\
		.pcw_reg = _pcw_reg,					\
		.pcw_shift = _pcw_shift,				\
	}

static const struct mtk_pll_data apmixed_plls[] = {
	PLL(CLK_APMIXED_ARMPLL, 0x0100, 0x0110, 0x00000001, 0,
		21, 0x0104, 24, 0x0104, 0),
	PLL(CLK_APMIXED_MAINPLL, 0x0120, 0x0130, 0x00000001,
		HAVE_RST_BAR, 21, 0x0124, 24, 0x0124, 0),
	PLL(CLK_APMIXED_UNIVPLL, 0x0140, 0x0150, 0x30000001,
		HAVE_RST_BAR, 7, 0x0144, 24, 0x0144, 0),
	PLL(CLK_APMIXED_MMPLL, 0x0160, 0x0170, 0x00000001, 0,
		21, 0x0164, 24, 0x0164, 0),
	PLL(CLK_APMIXED_APLL1, 0x0180, 0x0190, 0x00000001, 0,
		31, 0x0180, 1, 0x0184, 0),
	PLL(CLK_APMIXED_APLL2, 0x01A0, 0x01B0, 0x00000001, 0,
		31, 0x01A0, 1, 0x01A4, 0),
};

/* topckgen */
#define FACTOR0(_id, _parent, _mult, _div)	\
	FACTOR(_id, _parent, _mult, _div, CLK_PARENT_APMIXED)

#define FACTOR1(_id, _parent, _mult, _div)	\
	FACTOR(_id, _parent, _mult, _div, CLK_PARENT_TOPCKGEN)

#define FACTOR2(_id, _parent, _mult, _div)	\
	FACTOR(_id, _parent, _mult, _div, 0)

static const struct mtk_fixed_clk top_fixed_clks[] = {
	FIXED_CLK(CLK_TOP_CLK_NULL, CLK_XTAL, 26000000),
	FIXED_CLK(CLK_TOP_I2S_INFRA_BCK, CLK_TOP_CLK_NULL, 26000000),
	FIXED_CLK(CLK_TOP_MEMPLL, CLK_TOP_CLK26M, 800000000),
};

static const struct mtk_fixed_factor top_fixed_divs[] = {
	FACTOR1(CLK_TOP_DMPLL, CLK_TOP_MEMPLL, 1, 1),
	FACTOR0(CLK_TOP_MAINPLL_D2, CLK_APMIXED_MAINPLL, 1, 2),
	FACTOR0(CLK_TOP_MAINPLL_D4, CLK_APMIXED_MAINPLL, 1, 4),
	FACTOR0(CLK_TOP_MAINPLL_D8, CLK_APMIXED_MAINPLL, 1, 8),
	FACTOR0(CLK_TOP_MAINPLL_D16, CLK_APMIXED_MAINPLL, 1, 16),
	FACTOR0(CLK_TOP_MAINPLL_D11, CLK_APMIXED_MAINPLL, 1, 11),
	FACTOR0(CLK_TOP_MAINPLL_D22, CLK_APMIXED_MAINPLL, 1, 22),
	FACTOR0(CLK_TOP_MAINPLL_D3, CLK_APMIXED_MAINPLL, 1, 3),
	FACTOR0(CLK_TOP_MAINPLL_D6, CLK_APMIXED_MAINPLL, 1, 6),
	FACTOR0(CLK_TOP_MAINPLL_D12, CLK_APMIXED_MAINPLL, 1, 12),
	FACTOR0(CLK_TOP_MAINPLL_D5, CLK_APMIXED_MAINPLL, 1, 5),
	FACTOR0(CLK_TOP_MAINPLL_D10, CLK_APMIXED_MAINPLL, 1, 10),
	FACTOR0(CLK_TOP_MAINPLL_D20, CLK_APMIXED_MAINPLL, 1, 20),
	FACTOR0(CLK_TOP_MAINPLL_D40, CLK_APMIXED_MAINPLL, 1, 40),
	FACTOR0(CLK_TOP_MAINPLL_D7, CLK_APMIXED_MAINPLL, 1, 7),
	FACTOR0(CLK_TOP_MAINPLL_D14, CLK_APMIXED_MAINPLL, 1, 14),
	FACTOR0(CLK_TOP_UNIVPLL_D2, CLK_APMIXED_UNIVPLL, 1, 2),
	FACTOR0(CLK_TOP_UNIVPLL_D4, CLK_APMIXED_UNIVPLL, 1, 4),
	FACTOR0(CLK_TOP_UNIVPLL_D8, CLK_APMIXED_UNIVPLL, 1, 8),
	FACTOR0(CLK_TOP_UNIVPLL_D16, CLK_APMIXED_UNIVPLL, 1, 16),
	FACTOR0(CLK_TOP_UNIVPLL_D3, CLK_APMIXED_UNIVPLL, 1, 3),
	FACTOR0(CLK_TOP_UNIVPLL_D6, CLK_APMIXED_UNIVPLL, 1, 6),
	FACTOR0(CLK_TOP_UNIVPLL_D12, CLK_APMIXED_UNIVPLL, 1, 12),
	FACTOR0(CLK_TOP_UNIVPLL_D24, CLK_APMIXED_UNIVPLL, 1, 24),
	FACTOR0(CLK_TOP_UNIVPLL_D5, CLK_APMIXED_UNIVPLL, 1, 5),
	FACTOR0(CLK_TOP_UNIVPLL_D20, CLK_APMIXED_UNIVPLL, 1, 20),
	FACTOR0(CLK_TOP_MMPLL380M, CLK_APMIXED_MMPLL, 1, 1),
	FACTOR0(CLK_TOP_MMPLL_D2, CLK_APMIXED_MMPLL, 1, 2),
	FACTOR0(CLK_TOP_MMPLL_200M, CLK_APMIXED_MMPLL, 1, 3),
	FACTOR0(CLK_TOP_USB_PHY48M, CLK_APMIXED_UNIVPLL, 1, 26),
	FACTOR0(CLK_TOP_APLL1, CLK_APMIXED_APLL1, 1, 1),
	FACTOR1(CLK_TOP_APLL1_D2, CLK_TOP_APLL1, 1, 2),
	FACTOR1(CLK_TOP_APLL1_D4, CLK_TOP_RG_APLL1_D2_EN, 1, 2),
	FACTOR1(CLK_TOP_APLL1_D8, CLK_TOP_RG_APLL1_D4_EN, 1, 2),
	FACTOR0(CLK_TOP_APLL2, CLK_APMIXED_APLL2, 1, 1),
	FACTOR1(CLK_TOP_APLL2_D2, CLK_TOP_APLL2, 1, 2),
	FACTOR1(CLK_TOP_APLL2_D4, CLK_TOP_RG_APLL2_D2_EN, 1, 2),
	FACTOR1(CLK_TOP_APLL2_D8, CLK_TOP_RG_APLL2_D4_EN, 1, 2),
	FACTOR2(CLK_TOP_CLK26M, CLK_XTAL, 1, 1),
	FACTOR2(CLK_TOP_CLK26M_D2, CLK_XTAL, 1, 2),
	FACTOR1(CLK_TOP_AHB_INFRA_D2, CLK_TOP_AHB_INFRA_SEL, 1, 2),
	FACTOR1(CLK_TOP_NFI1X, CLK_TOP_NFI2X_PAD_SEL, 1, 2),
	FACTOR1(CLK_TOP_ETH_D2, CLK_TOP_ETH_SEL, 1, 2),
};

static const int uart0_parents[] = {
	CLK_TOP_CLK26M,
	CLK_TOP_UNIVPLL_D24,
};

static const int gfmux_emi1x_parents[] = {
	CLK_TOP_CLK26M,
	CLK_TOP_DMPLL,
};

static const int emi_ddrphy_parents[] = {
	CLK_TOP_GFMUX_EMI1X_SEL,
	CLK_TOP_GFMUX_EMI1X_SEL,
};

static const int ahb_infra_parents[] = {
	CLK_TOP_CLK_NULL,
	CLK_TOP_CLK26M,
	CLK_TOP_MAINPLL_D11,
	CLK_TOP_CLK_NULL,
	CLK_TOP_MAINPLL_D12,
	CLK_TOP_CLK_NULL,
	CLK_TOP_CLK_NULL,
	CLK_TOP_CLK_NULL,
	CLK_TOP_CLK_NULL,
	CLK_TOP_CLK_NULL,
	CLK_TOP_CLK_NULL,
	CLK_TOP_CLK_NULL,
	CLK_TOP_MAINPLL_D10,
};

static const int csw_mux_mfg_parents[] = {
	CLK_TOP_CLK_NULL,
	CLK_TOP_CLK_NULL,
	CLK_TOP_UNIVPLL_D3,
	CLK_TOP_UNIVPLL_D2,
	CLK_TOP_CLK26M,
	CLK_TOP_MAINPLL_D4,
	CLK_TOP_UNIVPLL_D24,
	CLK_TOP_MMPLL380M,
};

static const int msdc0_parents[] = {
	CLK_TOP_CLK26M,
	CLK_TOP_UNIVPLL_D6,
	CLK_TOP_MAINPLL_D8,
	CLK_TOP_UNIVPLL_D8,
	CLK_TOP_MAINPLL_D16,
	CLK_TOP_MMPLL_200M,
	CLK_TOP_MAINPLL_D12,
	CLK_TOP_MMPLL_D2,
};

static const int pwm_mm_parents[] = {
	CLK_TOP_CLK26M,
	CLK_TOP_UNIVPLL_D12,
};

static const int uart1_parents[] = {
	CLK_TOP_CLK26M,
	CLK_TOP_UNIVPLL_D24,
};

static const int msdc1_parents[] = {
	CLK_TOP_CLK26M,
	CLK_TOP_UNIVPLL_D6,
	CLK_TOP_MAINPLL_D8,
	CLK_TOP_UNIVPLL_D8,
	CLK_TOP_MAINPLL_D16,
	CLK_TOP_MMPLL_200M,
	CLK_TOP_MAINPLL_D12,
	CLK_TOP_MMPLL_D2,
};

static const int spm_52m_parents[] = {
	CLK_TOP_CLK26M,
	CLK_TOP_UNIVPLL_D24,
};

static const int pmicspi_parents[] = {
	CLK_TOP_UNIVPLL_D20,
	CLK_TOP_USB_PHY48M,
	CLK_TOP_UNIVPLL_D16,
	CLK_TOP_CLK26M,
};

static const int qaxi_aud26m_parents[] = {
	CLK_TOP_CLK26M,
	CLK_TOP_AHB_INFRA_SEL,
};

static const int aud_intbus_parents[] = {
	CLK_TOP_CLK_NULL,
	CLK_TOP_CLK26M,
	CLK_TOP_MAINPLL_D22,
	CLK_TOP_CLK_NULL,
	CLK_TOP_MAINPLL_D11,
};

static const int nfi2x_pad_parents[] = {
	CLK_TOP_CLK_NULL,
	CLK_TOP_CLK_NULL,
	CLK_TOP_CLK_NULL,
	CLK_TOP_CLK_NULL,
	CLK_TOP_CLK_NULL,
	CLK_TOP_CLK_NULL,
	CLK_TOP_CLK_NULL,
	CLK_TOP_CLK_NULL,
	CLK_TOP_CLK26M,
	CLK_TOP_CLK_NULL,
	CLK_TOP_CLK_NULL,
	CLK_TOP_CLK_NULL,
	CLK_TOP_CLK_NULL,
	CLK_TOP_CLK_NULL,
	CLK_TOP_CLK_NULL,
	CLK_TOP_CLK_NULL,
	CLK_TOP_CLK_NULL,
	CLK_TOP_MAINPLL_D12,
	CLK_TOP_MAINPLL_D8,
	CLK_TOP_CLK_NULL,
	CLK_TOP_MAINPLL_D6,
	CLK_TOP_CLK_NULL,
	CLK_TOP_CLK_NULL,
	CLK_TOP_CLK_NULL,
	CLK_TOP_CLK_NULL,
	CLK_TOP_CLK_NULL,
	CLK_TOP_CLK_NULL,
	CLK_TOP_CLK_NULL,
	CLK_TOP_CLK_NULL,
	CLK_TOP_CLK_NULL,
	CLK_TOP_CLK_NULL,
	CLK_TOP_CLK_NULL,
	CLK_TOP_MAINPLL_D4,
	CLK_TOP_CLK_NULL,
	CLK_TOP_CLK_NULL,
	CLK_TOP_CLK_NULL,
	CLK_TOP_CLK_NULL,
	CLK_TOP_CLK_NULL,
	CLK_TOP_CLK_NULL,
	CLK_TOP_CLK_NULL,
	CLK_TOP_CLK_NULL,
	CLK_TOP_CLK_NULL,
	CLK_TOP_CLK_NULL,
	CLK_TOP_CLK_NULL,
	CLK_TOP_CLK_NULL,
	CLK_TOP_CLK_NULL,
	CLK_TOP_CLK_NULL,
	CLK_TOP_CLK_NULL,
	CLK_TOP_CLK_NULL,
	CLK_TOP_CLK_NULL,
	CLK_TOP_CLK_NULL,
	CLK_TOP_CLK_NULL,
	CLK_TOP_CLK_NULL,
	CLK_TOP_CLK_NULL,
	CLK_TOP_CLK_NULL,
	CLK_TOP_CLK_NULL,
	CLK_TOP_CLK_NULL,
	CLK_TOP_CLK_NULL,
	CLK_TOP_CLK_NULL,
	CLK_TOP_CLK_NULL,
	CLK_TOP_CLK_NULL,
	CLK_TOP_CLK_NULL,
	CLK_TOP_CLK_NULL,
	CLK_TOP_CLK_NULL,
	CLK_TOP_CLK_NULL,
	CLK_TOP_CLK_NULL,
	CLK_TOP_CLK_NULL,
	CLK_TOP_CLK_NULL,
	CLK_TOP_CLK_NULL,
	CLK_TOP_CLK_NULL,
	CLK_TOP_CLK_NULL,
	CLK_TOP_CLK_NULL,
	CLK_TOP_CLK_NULL,
	CLK_TOP_CLK_NULL,
	CLK_TOP_CLK_NULL,
	CLK_TOP_CLK_NULL,
	CLK_TOP_CLK_NULL,
	CLK_TOP_CLK_NULL,
	CLK_TOP_CLK_NULL,
	CLK_TOP_CLK_NULL,
	CLK_TOP_CLK_NULL,
	CLK_TOP_MAINPLL_D10,
	CLK_TOP_MAINPLL_D7,
	CLK_TOP_CLK_NULL,
	CLK_TOP_MAINPLL_D5
};

static const int nfi1x_pad_parents[] = {
	CLK_TOP_AHB_INFRA_SEL,
	CLK_TOP_NFI1X,
};

static const int mfg_mm_parents[] = {
	CLK_TOP_CLK_NULL,
	CLK_TOP_CLK_NULL,
	CLK_TOP_CLK_NULL,
	CLK_TOP_CLK_NULL,
	CLK_TOP_CLK_NULL,
	CLK_TOP_CLK_NULL,
	CLK_TOP_CLK_NULL,
	CLK_TOP_CLK_NULL,
	CLK_TOP_CSW_MUX_MFG_SEL,
	CLK_TOP_CLK_NULL,
	CLK_TOP_CLK_NULL,
	CLK_TOP_CLK_NULL,
	CLK_TOP_CLK_NULL,
	CLK_TOP_CLK_NULL,
	CLK_TOP_CLK_NULL,
	CLK_TOP_CLK_NULL,
	CLK_TOP_MAINPLL_D3,
	CLK_TOP_CLK_NULL,
	CLK_TOP_CLK_NULL,
	CLK_TOP_CLK_NULL,
	CLK_TOP_CLK_NULL,
	CLK_TOP_CLK_NULL,
	CLK_TOP_CLK_NULL,
	CLK_TOP_CLK_NULL,
	CLK_TOP_CLK_NULL,
	CLK_TOP_CLK_NULL,
	CLK_TOP_CLK_NULL,
	CLK_TOP_CLK_NULL,
	CLK_TOP_CLK_NULL,
	CLK_TOP_CLK_NULL,
	CLK_TOP_CLK_NULL,
	CLK_TOP_CLK_NULL,
	CLK_TOP_CLK_NULL,
	CLK_TOP_MAINPLL_D5,
	CLK_TOP_MAINPLL_D7,
	CLK_TOP_CLK_NULL,
	CLK_TOP_MAINPLL_D14
};

static const int ddrphycfg_parents[] = {
	CLK_TOP_CLK26M,
	CLK_TOP_MAINPLL_D16
};

static const int usb_78m_parents[] = {
	CLK_TOP_CLK_NULL,
	CLK_TOP_CLK26M,
	CLK_TOP_UNIVPLL_D16,
	CLK_TOP_CLK_NULL,
	CLK_TOP_MAINPLL_D20,
};

static const int spinor_parents[] = {
	CLK_TOP_CLK26M_D2,
	CLK_TOP_CLK26M,
	CLK_TOP_MAINPLL_D40,
	CLK_TOP_UNIVPLL_D24,
	CLK_TOP_UNIVPLL_D20,
	CLK_TOP_MAINPLL_D20,
	CLK_TOP_MAINPLL_D16,
	CLK_TOP_UNIVPLL_D12
};

static const int msdc2_parents[] = {
	CLK_TOP_CLK26M,
	CLK_TOP_UNIVPLL_D6,
	CLK_TOP_MAINPLL_D8,
	CLK_TOP_UNIVPLL_D8,
	CLK_TOP_MAINPLL_D16,
	CLK_TOP_MMPLL_200M,
	CLK_TOP_MAINPLL_D12,
	CLK_TOP_MMPLL_D2
};

static const int eth_parents[] = {
	CLK_TOP_CLK26M,
	CLK_TOP_MAINPLL_D40,
	CLK_TOP_UNIVPLL_D24,
	CLK_TOP_UNIVPLL_D20,
	CLK_TOP_MAINPLL_D20
};

static const int axi_mfg_in_parents[] = {
	CLK_TOP_CLK26M,
	CLK_TOP_MAINPLL_D11,
	CLK_TOP_UNIVPLL_D24,
	CLK_TOP_MMPLL380M,
};

static const int slow_mfg_parents[] = {
	CLK_TOP_CLK26M,
	CLK_TOP_UNIVPLL_D12,
	CLK_TOP_UNIVPLL_D24
};

static const int aud1_parents[] = {
	CLK_TOP_CLK26M,
	CLK_TOP_APLL1
};

static const int aud2_parents[] = {
	CLK_TOP_CLK26M,
	CLK_TOP_APLL2
};

static const int aud_engen1_parents[] = {
	CLK_TOP_CLK26M,
	CLK_TOP_RG_APLL1_D2_EN,
	CLK_TOP_RG_APLL1_D4_EN,
	CLK_TOP_RG_APLL1_D8_EN
};

static const int aud_engen2_parents[] = {
	CLK_TOP_CLK26M,
	CLK_TOP_RG_APLL2_D2_EN,
	CLK_TOP_RG_APLL2_D4_EN,
	CLK_TOP_RG_APLL2_D8_EN
};

static const int i2c_parents[] = {
	CLK_TOP_CLK26M,
	CLK_TOP_UNIVPLL_D20,
	CLK_TOP_UNIVPLL_D16,
	CLK_TOP_UNIVPLL_D12
};

static const int aud_i2s0_m_parents[] = {
	CLK_TOP_RG_AUD1,
	CLK_TOP_RG_AUD2
};

static const int pwm_parents[] = {
	CLK_TOP_CLK26M,
	CLK_TOP_UNIVPLL_D12
};

static const int spi_parents[] = {
	CLK_TOP_CLK26M,
	CLK_TOP_UNIVPLL_D12,
	CLK_TOP_UNIVPLL_D8,
	CLK_TOP_UNIVPLL_D6
};

static const int aud_spdifin_parents[] = {
	CLK_TOP_CLK26M,
	CLK_TOP_UNIVPLL_D2
};

static const int uart2_parents[] = {
	CLK_TOP_CLK26M,
	CLK_TOP_UNIVPLL_D24
};

static const int bsi_parents[] = {
	CLK_TOP_CLK26M,
	CLK_TOP_MAINPLL_D10,
	CLK_TOP_MAINPLL_D12,
	CLK_TOP_MAINPLL_D20
};

static const int dbg_atclk_parents[] = {
	CLK_TOP_CLK_NULL,
	CLK_TOP_CLK26M,
	CLK_TOP_MAINPLL_D5,
	CLK_TOP_CLK_NULL,
	CLK_TOP_UNIVPLL_D5
};

static const int csw_nfiecc_parents[] = {
	CLK_TOP_CLK_NULL,
	CLK_TOP_MAINPLL_D7,
	CLK_TOP_MAINPLL_D6,
	CLK_TOP_CLK_NULL,
	CLK_TOP_MAINPLL_D5
};

static const int nfiecc_parents[] = {
	CLK_TOP_CLK_NULL,
	CLK_TOP_NFI2X_PAD_SEL,
	CLK_TOP_MAINPLL_D4,
	CLK_TOP_CLK_NULL,
	CLK_TOP_CSW_NFIECC_SEL,
};

static const struct mtk_composite top_muxes[] = {
	/* CLK_MUX_SEL0 */
	MUX(CLK_TOP_UART0_SEL, uart0_parents, 0x000, 0, 1),
	MUX(CLK_TOP_GFMUX_EMI1X_SEL, gfmux_emi1x_parents, 0x000, 1, 1),
	MUX(CLK_TOP_EMI_DDRPHY_SEL, emi_ddrphy_parents, 0x000, 2, 1),
	MUX(CLK_TOP_AHB_INFRA_SEL, ahb_infra_parents, 0x000, 4, 4),
	MUX(CLK_TOP_CSW_MUX_MFG_SEL, csw_mux_mfg_parents, 0x000, 8, 3),
	MUX(CLK_TOP_MSDC0_SEL, msdc0_parents, 0x000, 11, 3),
	MUX(CLK_TOP_PWM_MM_SEL, pwm_mm_parents, 0x000, 18, 1),
	MUX(CLK_TOP_UART1_SEL, uart1_parents, 0x000, 19, 1),
	MUX(CLK_TOP_MSDC1_SEL, msdc1_parents, 0x000, 20, 3),
	MUX(CLK_TOP_SPM_52M_SEL, spm_52m_parents, 0x000, 23, 1),
	MUX(CLK_TOP_PMICSPI_SEL, pmicspi_parents, 0x000, 24, 2),
	MUX(CLK_TOP_QAXI_AUD26M_SEL, qaxi_aud26m_parents, 0x000, 26, 1),
	MUX(CLK_TOP_AUD_INTBUS_SEL, aud_intbus_parents, 0x000, 27, 3),
	/* CLK_MUX_SEL1 */
	MUX(CLK_TOP_NFI2X_PAD_SEL, nfi2x_pad_parents, 0x004, 0, 7),
	MUX(CLK_TOP_NFI1X_PAD_SEL, nfi1x_pad_parents, 0x004, 7, 1),
	MUX(CLK_TOP_MFG_MM_SEL, mfg_mm_parents, 0x004, 8, 6),
	MUX(CLK_TOP_DDRPHYCFG_SEL, ddrphycfg_parents, 0x004, 15, 1),
	MUX(CLK_TOP_USB_78M_SEL, usb_78m_parents, 0x004, 20, 3),
	/* CLK_MUX_SEL8 */
	MUX(CLK_TOP_SPINOR_SEL, spinor_parents, 0x040, 0, 3),
	MUX(CLK_TOP_MSDC2_SEL, msdc2_parents, 0x040, 3, 3),
	MUX(CLK_TOP_ETH_SEL, eth_parents, 0x040, 6, 3),
	MUX(CLK_TOP_AXI_MFG_IN_SEL, axi_mfg_in_parents, 0x040, 18, 2),
	MUX(CLK_TOP_SLOW_MFG_SEL, slow_mfg_parents, 0x040, 20, 2),
	MUX(CLK_TOP_AUD1_SEL, aud1_parents, 0x040, 22, 1),
	MUX(CLK_TOP_AUD2_SEL, aud2_parents, 0x040, 23, 1),
	MUX(CLK_TOP_AUD_ENGEN1_SEL, aud_engen1_parents, 0x040, 24, 2),
	MUX(CLK_TOP_AUD_ENGEN2_SEL, aud_engen2_parents, 0x040, 26, 2),
	MUX(CLK_TOP_I2C_SEL, i2c_parents, 0x040, 28, 2),
	/* CLK_MUX_SEL9 */
	MUX(CLK_TOP_AUD_I2S0_M_SEL, aud_i2s0_m_parents, 0x044, 12, 1),
	MUX(CLK_TOP_AUD_I2S1_M_SEL, aud_i2s0_m_parents, 0x044, 13, 1),
	MUX(CLK_TOP_AUD_I2S2_M_SEL, aud_i2s0_m_parents, 0x044, 14, 1),
	MUX(CLK_TOP_AUD_I2S3_M_SEL, aud_i2s0_m_parents, 0x044, 15, 1),
	MUX(CLK_TOP_AUD_I2S4_M_SEL, aud_i2s0_m_parents, 0x044, 16, 1),
	MUX(CLK_TOP_AUD_I2S5_M_SEL, aud_i2s0_m_parents, 0x044, 17, 1),
	MUX(CLK_TOP_AUD_SPDIF_B_SEL, aud_i2s0_m_parents, 0x044, 18, 1),
	/* CLK_MUX_SEL13 */
	MUX(CLK_TOP_PWM_SEL, pwm_parents, 0x07c, 0, 1),
	MUX(CLK_TOP_SPI_SEL, spi_parents, 0x07c, 1, 2),
	MUX(CLK_TOP_AUD_SPDIFIN_SEL, aud_spdifin_parents, 0x07c, 3, 1),
	MUX(CLK_TOP_UART2_SEL, uart2_parents, 0x07c, 4, 1),
	MUX(CLK_TOP_BSI_SEL, bsi_parents, 0x07c, 5, 2),
	MUX(CLK_TOP_DBG_ATCLK_SEL, dbg_atclk_parents, 0x07c, 7, 3),
	MUX(CLK_TOP_CSW_NFIECC_SEL, csw_nfiecc_parents, 0x07c, 10, 3),
	MUX(CLK_TOP_NFIECC_SEL, nfiecc_parents, 0x07c, 13, 3),
};

static const struct mtk_gate_regs top0_cg_regs = {
	.set_ofs = 0x50,
	.clr_ofs = 0x80,
	.sta_ofs = 0x20,
};

static const struct mtk_gate_regs top1_cg_regs = {
	.set_ofs = 0x54,
	.clr_ofs = 0x84,
	.sta_ofs = 0x24,
};

static const struct mtk_gate_regs top2_cg_regs = {
	.set_ofs = 0x6c,
	.clr_ofs = 0x9c,
	.sta_ofs = 0x3c,
};

static const struct mtk_gate_regs top3_cg_regs = {
	.set_ofs = 0xa0,
	.clr_ofs = 0xb0,
	.sta_ofs = 0x70,
};

static const struct mtk_gate_regs top4_cg_regs = {
	.set_ofs = 0xa4,
	.clr_ofs = 0xb4,
	.sta_ofs = 0x74,
};

static const struct mtk_gate_regs top5_cg_regs = {
	.set_ofs = 0x44,
	.clr_ofs = 0x44,
	.sta_ofs = 0x44,
};

#define GATE_TOP0(_id, _parent, _shift) {			\
		.id = _id,					\
		.parent = _parent,				\
		.regs = &top0_cg_regs,				\
		.shift = _shift,				\
		.flags = CLK_GATE_SETCLR | CLK_PARENT_TOPCKGEN,	\
	}

#define GATE_TOP1(_id, _parent, _shift) {			\
		.id = _id,					\
		.parent = _parent,				\
		.regs = &top1_cg_regs,				\
		.shift = _shift,				\
		.flags = CLK_GATE_SETCLR | CLK_PARENT_TOPCKGEN,	\
	}

#define GATE_TOP2(_id, _parent, _shift) {			\
		.id = _id,					\
		.parent = _parent,				\
		.regs = &top2_cg_regs,				\
		.shift = _shift,				\
		.flags = CLK_GATE_SETCLR | CLK_PARENT_TOPCKGEN,	\
	}

#define GATE_TOP2_I(_id, _parent, _shift) {				\
		.id = _id,						\
		.parent = _parent,					\
		.regs = &top2_cg_regs,					\
		.shift = _shift,					\
		.flags = CLK_GATE_SETCLR_INV | CLK_PARENT_TOPCKGEN,	\
	}

#define GATE_TOP3(_id, _parent, _shift) {			\
		.id = _id,					\
		.parent = _parent,				\
		.regs = &top3_cg_regs,				\
		.shift = _shift,				\
		.flags = CLK_GATE_SETCLR | CLK_PARENT_TOPCKGEN,	\
	}

#define GATE_TOP4_I(_id, _parent, _shift) {				\
		.id = _id,						\
		.parent = _parent,					\
		.regs = &top4_cg_regs,					\
		.shift = _shift,					\
		.flags = CLK_GATE_SETCLR_INV | CLK_PARENT_TOPCKGEN,	\
	}

#define GATE_TOP5(_id, _parent, _shift) {				\
		.id = _id,						\
		.parent = _parent,					\
		.regs = &top5_cg_regs,					\
		.shift = _shift,					\
		.flags = CLK_GATE_NO_SETCLR | CLK_PARENT_TOPCKGEN,	\
	}

static const struct mtk_gate top_clks[] = {
	/* TOP0 */
	GATE_TOP0(CLK_TOP_PWM_MM, CLK_TOP_PWM_MM_SEL, 0),
	GATE_TOP0(CLK_TOP_MFG_MM, CLK_TOP_MFG_MM_SEL, 2),
	GATE_TOP0(CLK_TOP_SPM_52M, CLK_TOP_SPM_52M_SEL, 3),
	/* TOP1 */
	GATE_TOP1(CLK_TOP_THEM, CLK_TOP_AHB_INFRA_SEL, 1),
	GATE_TOP1(CLK_TOP_APDMA, CLK_TOP_AHB_INFRA_SEL, 2),
	GATE_TOP1(CLK_TOP_I2C0, CLK_IFR_I2C0_SEL, 3),
	GATE_TOP1(CLK_TOP_I2C1, CLK_IFR_I2C1_SEL, 4),
	GATE_TOP1(CLK_TOP_AUXADC1, CLK_TOP_AHB_INFRA_SEL, 5),
	GATE_TOP1(CLK_TOP_NFI, CLK_TOP_NFI1X_PAD_SEL, 6),
	GATE_TOP1(CLK_TOP_NFIECC, CLK_TOP_RG_NFIECC, 7),
	GATE_TOP1(CLK_TOP_DEBUGSYS, CLK_TOP_RG_DBG_ATCLK, 8),
	GATE_TOP1(CLK_TOP_PWM, CLK_TOP_AHB_INFRA_SEL, 9),
	GATE_TOP1(CLK_TOP_UART0, CLK_TOP_UART0_SEL, 10),
	GATE_TOP1(CLK_TOP_UART1, CLK_TOP_UART1_SEL, 11),
	GATE_TOP1(CLK_TOP_BTIF, CLK_TOP_AHB_INFRA_SEL, 12),
	GATE_TOP1(CLK_TOP_USB, CLK_TOP_USB_78M, 13),
	GATE_TOP1(CLK_TOP_FLASHIF_26M, CLK_TOP_CLK26M, 14),
	GATE_TOP1(CLK_TOP_AUXADC2, CLK_TOP_AHB_INFRA_SEL, 15),
	GATE_TOP1(CLK_TOP_I2C2, CLK_IFR_I2C2_SEL, 16),
	GATE_TOP1(CLK_TOP_MSDC0, CLK_TOP_MSDC0_SEL, 17),
	GATE_TOP1(CLK_TOP_MSDC1, CLK_TOP_MSDC1_SEL, 18),
	GATE_TOP1(CLK_TOP_NFI2X, CLK_TOP_NFI2X_PAD_SEL, 19),
	GATE_TOP1(CLK_TOP_PMICWRAP_AP, CLK_TOP_CLK26M, 20),
	GATE_TOP1(CLK_TOP_SEJ, CLK_TOP_AHB_INFRA_SEL, 21),
	GATE_TOP1(CLK_TOP_MEMSLP_DLYER, CLK_TOP_CLK26M, 22),
	GATE_TOP1(CLK_TOP_SPI, CLK_TOP_SPI_SEL, 23),
	GATE_TOP1(CLK_TOP_APXGPT, CLK_TOP_CLK26M, 24),
	GATE_TOP1(CLK_TOP_AUDIO, CLK_TOP_CLK26M, 25),
	GATE_TOP1(CLK_TOP_PMICWRAP_MD, CLK_TOP_CLK26M, 27),
	GATE_TOP1(CLK_TOP_PMICWRAP_CONN, CLK_TOP_CLK26M, 28),
	GATE_TOP1(CLK_TOP_PMICWRAP_26M, CLK_TOP_CLK26M, 29),
	GATE_TOP1(CLK_TOP_AUX_ADC, CLK_TOP_CLK26M, 30),
	GATE_TOP1(CLK_TOP_AUX_TP, CLK_TOP_CLK26M, 31),
	/* TOP2 */
	GATE_TOP2(CLK_TOP_MSDC2, CLK_TOP_AHB_INFRA_SEL, 0),
	GATE_TOP2(CLK_TOP_RBIST, CLK_TOP_UNIVPLL_D12, 1),
	GATE_TOP2(CLK_TOP_NFI_BUS, CLK_TOP_AHB_INFRA_SEL, 2),
	GATE_TOP2(CLK_TOP_GCE, CLK_TOP_AHB_INFRA_SEL, 4),
	GATE_TOP2(CLK_TOP_TRNG, CLK_TOP_AHB_INFRA_SEL, 5),
	GATE_TOP2(CLK_TOP_SEJ_13M, CLK_TOP_CLK26M, 6),
	GATE_TOP2(CLK_TOP_AES, CLK_TOP_AHB_INFRA_SEL, 7),
	GATE_TOP2(CLK_TOP_PWM_B, CLK_TOP_RG_PWM_INFRA, 8),
	GATE_TOP2(CLK_TOP_PWM1_FB, CLK_TOP_RG_PWM_INFRA, 9),
	GATE_TOP2(CLK_TOP_PWM2_FB, CLK_TOP_RG_PWM_INFRA, 10),
	GATE_TOP2(CLK_TOP_PWM3_FB, CLK_TOP_RG_PWM_INFRA, 11),
	GATE_TOP2(CLK_TOP_PWM4_FB, CLK_TOP_RG_PWM_INFRA, 12),
	GATE_TOP2(CLK_TOP_PWM5_FB, CLK_TOP_RG_PWM_INFRA, 13),
	GATE_TOP2(CLK_TOP_USB_1P, CLK_TOP_USB_78M, 14),
	GATE_TOP2(CLK_TOP_FLASHIF_FREERUN, CLK_TOP_AHB_INFRA_SEL, 15),
	GATE_TOP2(CLK_TOP_66M_ETH, CLK_TOP_AHB_INFRA_D2, 19),
	GATE_TOP2(CLK_TOP_133M_ETH, CLK_TOP_AHB_INFRA_SEL, 20),
	GATE_TOP2(CLK_TOP_FETH_25M, CLK_IFR_ETH_25M_SEL, 21),
	GATE_TOP2(CLK_TOP_FETH_50M, CLK_TOP_RG_ETH, 22),
	GATE_TOP2(CLK_TOP_FLASHIF_AXI, CLK_TOP_AHB_INFRA_SEL, 23),
	GATE_TOP2(CLK_TOP_USBIF, CLK_TOP_AHB_INFRA_SEL, 24),
	GATE_TOP2(CLK_TOP_UART2, CLK_TOP_RG_UART2, 25),
	GATE_TOP2(CLK_TOP_BSI, CLK_TOP_AHB_INFRA_SEL, 26),
	GATE_TOP2_I(CLK_TOP_MSDC0_INFRA, CLK_TOP_MSDC0, 28),
	GATE_TOP2_I(CLK_TOP_MSDC1_INFRA, CLK_TOP_MSDC1, 29),
	GATE_TOP2_I(CLK_TOP_MSDC2_INFRA, CLK_TOP_RG_MSDC2, 30),
	GATE_TOP2(CLK_TOP_USB_78M, CLK_TOP_USB_78M_SEL, 31),
	/* TOP3 */
	GATE_TOP3(CLK_TOP_RG_SPINOR, CLK_TOP_SPINOR_SEL, 0),
	GATE_TOP3(CLK_TOP_RG_MSDC2, CLK_TOP_MSDC2_SEL, 1),
	GATE_TOP3(CLK_TOP_RG_ETH, CLK_TOP_ETH_SEL, 2),
	GATE_TOP3(CLK_TOP_RG_AXI_MFG, CLK_TOP_AXI_MFG_IN_SEL, 6),
	GATE_TOP3(CLK_TOP_RG_SLOW_MFG, CLK_TOP_SLOW_MFG_SEL, 7),
	GATE_TOP3(CLK_TOP_RG_AUD1, CLK_TOP_AUD1_SEL, 8),
	GATE_TOP3(CLK_TOP_RG_AUD2, CLK_TOP_AUD2_SEL, 9),
	GATE_TOP3(CLK_TOP_RG_AUD_ENGEN1, CLK_TOP_AUD_ENGEN1_SEL, 10),
	GATE_TOP3(CLK_TOP_RG_AUD_ENGEN2, CLK_TOP_AUD_ENGEN2_SEL, 11),
	GATE_TOP3(CLK_TOP_RG_I2C, CLK_TOP_I2C_SEL, 12),
	GATE_TOP3(CLK_TOP_RG_PWM_INFRA, CLK_TOP_PWM_SEL, 13),
	GATE_TOP3(CLK_TOP_RG_AUD_SPDIF_IN, CLK_TOP_AUD_SPDIFIN_SEL, 14),
	GATE_TOP3(CLK_TOP_RG_UART2, CLK_TOP_UART2_SEL, 15),
	GATE_TOP3(CLK_TOP_RG_BSI, CLK_TOP_BSI_SEL, 16),
	GATE_TOP3(CLK_TOP_RG_DBG_ATCLK, CLK_TOP_DBG_ATCLK_SEL, 17),
	GATE_TOP3(CLK_TOP_RG_NFIECC, CLK_TOP_NFIECC_SEL, 18),
	/* TOP4 */
	GATE_TOP4_I(CLK_TOP_RG_APLL1_D2_EN, CLK_TOP_APLL1_D2, 8),
	GATE_TOP4_I(CLK_TOP_RG_APLL1_D4_EN, CLK_TOP_APLL1_D4, 9),
	GATE_TOP4_I(CLK_TOP_RG_APLL1_D8_EN, CLK_TOP_APLL1_D8, 10),
	GATE_TOP4_I(CLK_TOP_RG_APLL2_D2_EN, CLK_TOP_APLL2_D2, 11),
	GATE_TOP4_I(CLK_TOP_RG_APLL2_D4_EN, CLK_TOP_APLL2_D4, 12),
	GATE_TOP4_I(CLK_TOP_RG_APLL2_D8_EN, CLK_TOP_APLL2_D8, 13),
	/* TOP5 */
	GATE_TOP5(CLK_TOP_APLL12_DIV0, CLK_TOP_APLL12_CK_DIV0, 0),
	GATE_TOP5(CLK_TOP_APLL12_DIV1, CLK_TOP_APLL12_CK_DIV1, 1),
	GATE_TOP5(CLK_TOP_APLL12_DIV2, CLK_TOP_APLL12_CK_DIV2, 2),
	GATE_TOP5(CLK_TOP_APLL12_DIV3, CLK_TOP_APLL12_CK_DIV3, 3),
	GATE_TOP5(CLK_TOP_APLL12_DIV4, CLK_TOP_APLL12_CK_DIV4, 4),
	GATE_TOP5(CLK_TOP_APLL12_DIV4B, CLK_TOP_APLL12_CK_DIV4B, 5),
	GATE_TOP5(CLK_TOP_APLL12_DIV5, CLK_TOP_APLL12_CK_DIV5, 6),
	GATE_TOP5(CLK_TOP_APLL12_DIV5B, CLK_TOP_APLL12_CK_DIV5B, 7),
	GATE_TOP5(CLK_TOP_APLL12_DIV6, CLK_TOP_APLL12_CK_DIV6, 8),
};

static const struct mtk_clk_tree mt8516_clk_tree = {
	.xtal_rate = 26 * MHZ,
	.xtal2_rate = 26 * MHZ,
	.fdivs_offs = CLK_TOP_DMPLL,
	.muxes_offs = CLK_TOP_UART0_SEL,
	.plls = apmixed_plls,
	.fclks = top_fixed_clks,
	.fdivs = top_fixed_divs,
	.muxes = top_muxes,
};

static int mt8516_apmixedsys_probe(struct udevice *dev)
{
	return mtk_common_clk_init(dev, &mt8516_clk_tree);
}

static int mt8516_topckgen_probe(struct udevice *dev)
{
	return mtk_common_clk_init(dev, &mt8516_clk_tree);
}

static int mt8516_topckgen_cg_probe(struct udevice *dev)
{
	return mtk_common_clk_gate_init(dev, &mt8516_clk_tree, top_clks);
}

static const struct udevice_id mt8516_apmixed_compat[] = {
	{ .compatible = "mediatek,mt8516-apmixedsys", },
	{ }
};

static const struct udevice_id mt8516_topckgen_compat[] = {
	{ .compatible = "mediatek,mt8516-topckgen", },
	{ }
};

static const struct udevice_id mt8516_topckgen_cg_compat[] = {
	{ .compatible = "mediatek,mt8516-topckgen-cg", },
	{ }
};

U_BOOT_DRIVER(mtk_clk_apmixedsys) = {
	.name = "mt8516-apmixedsys",
	.id = UCLASS_CLK,
	.of_match = mt8516_apmixed_compat,
	.probe = mt8516_apmixedsys_probe,
	.priv_auto	= sizeof(struct mtk_clk_priv),
	.ops = &mtk_clk_apmixedsys_ops,
	.flags = DM_FLAG_PRE_RELOC,
};

U_BOOT_DRIVER(mtk_clk_topckgen) = {
	.name = "mt8516-topckgen",
	.id = UCLASS_CLK,
	.of_match = mt8516_topckgen_compat,
	.probe = mt8516_topckgen_probe,
	.priv_auto	= sizeof(struct mtk_clk_priv),
	.ops = &mtk_clk_topckgen_ops,
	.flags = DM_FLAG_PRE_RELOC,
};

U_BOOT_DRIVER(mtk_clk_topckgen_cg) = {
	.name = "mt8516-topckgen-cg",
	.id = UCLASS_CLK,
	.of_match = mt8516_topckgen_cg_compat,
	.probe = mt8516_topckgen_cg_probe,
	.priv_auto	= sizeof(struct mtk_cg_priv),
	.ops = &mtk_clk_gate_ops,
	.flags = DM_FLAG_PRE_RELOC,
};
