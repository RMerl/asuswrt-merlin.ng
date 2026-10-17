// SPDX-License-Identifier: GPL-2.0
/*
 * MediaTek clock driver for MT7623 SoC
 *
 * Copyright (C) 2018 MediaTek Inc.
 * Author: Ryder Lee <ryder.lee@mediatek.com>
 */

#include <common.h>
#include <dm.h>
#include <log.h>
#include <asm/arch-mediatek/reset.h>
#include <asm/io.h>
#include <dt-bindings/clock/mt7623-clk.h>
#include <linux/bitops.h>

#include "clk-mtk.h"

#define MT7623_CLKSQ_STB_CON0		0x18
#define MT7623_PLL_ISO_CON0		0x24
#define MT7623_PLL_FMAX			(2000UL * MHZ)
#define MT7623_CON0_RST_BAR		BIT(27)

#define MCU_AXI_DIV			0x60
#define AXI_DIV_MSK			GENMASK(4, 0)
#define AXI_DIV_SEL(x)			(x)

/* apmixedsys */
#define PLL(_id, _reg, _pwr_reg, _en_mask, _flags, _pcwbits, _pd_reg,	\
	    _pd_shift, _pcw_reg, _pcw_shift) {				\
		.id = _id,						\
		.reg = _reg,						\
		.pwr_reg = _pwr_reg,					\
		.en_mask = _en_mask,					\
		.rst_bar_mask = MT7623_CON0_RST_BAR,			\
		.fmax = MT7623_PLL_FMAX,				\
		.flags = _flags,					\
		.pcwbits = _pcwbits,					\
		.pd_reg = _pd_reg,					\
		.pd_shift = _pd_shift,					\
		.pcw_reg = _pcw_reg,					\
		.pcw_shift = _pcw_shift,				\
	}

static const struct mtk_pll_data apmixed_plls[] = {
	PLL(CLK_APMIXED_ARMPLL, 0x200, 0x20c, 0x80000001, 0,
	    21, 0x204, 24, 0x204, 0),
	PLL(CLK_APMIXED_MAINPLL, 0x210, 0x21c, 0xf0000001, HAVE_RST_BAR,
	    21, 0x210, 4, 0x214, 0),
	PLL(CLK_APMIXED_UNIVPLL, 0x220, 0x22c, 0xf3000001, HAVE_RST_BAR,
	    7, 0x220, 4, 0x224, 14),
	PLL(CLK_APMIXED_MMPLL, 0x230, 0x23c, 0x00000001, 0,
	    21, 0x230, 4, 0x234, 0),
	PLL(CLK_APMIXED_MSDCPLL, 0x240, 0x24c, 0x00000001, 0,
	    21, 0x240, 4, 0x244, 0),
	PLL(CLK_APMIXED_TVDPLL, 0x250, 0x25c, 0x00000001, 0,
	    21, 0x250, 4, 0x254, 0),
	PLL(CLK_APMIXED_AUD1PLL, 0x270, 0x27c, 0x00000001, 0,
	    31, 0x270, 4, 0x274, 0),
	PLL(CLK_APMIXED_TRGPLL, 0x280, 0x28c, 0x00000001, 0,
	    31, 0x280, 4, 0x284, 0),
	PLL(CLK_APMIXED_ETHPLL, 0x290, 0x29c, 0x00000001, 0,
	    31, 0x290, 4, 0x294, 0),
	PLL(CLK_APMIXED_VDECPLL, 0x2a0, 0x2ac, 0x00000001, 0,
	    31, 0x2a0, 4, 0x2a4, 0),
	PLL(CLK_APMIXED_HADDS2PLL, 0x2b0, 0x2bc, 0x00000001, 0,
	    31, 0x2b0, 4, 0x2b4, 0),
	PLL(CLK_APMIXED_AUD2PLL, 0x2c0, 0x2cc, 0x00000001, 0,
	    31, 0x2c0, 4, 0x2c4, 0),
	PLL(CLK_APMIXED_TVD2PLL, 0x2d0, 0x2dc, 0x00000001, 0,
	    21, 0x2d0, 4, 0x2d4, 0),
};

/* topckgen */
#define FACTOR0(_id, _parent, _mult, _div)			\
	FACTOR(_id, _parent, _mult, _div, CLK_PARENT_APMIXED)

#define FACTOR1(_id, _parent, _mult, _div)			\
	FACTOR(_id, _parent, _mult, _div, CLK_PARENT_TOPCKGEN)

#define FACTOR2(_id, _parent, _mult, _div)			\
	FACTOR(_id, _parent, _mult, _div, 0)

static const struct mtk_fixed_clk top_fixed_clks[] = {
	FIXED_CLK(CLK_TOP_DPI, CLK_XTAL, 108 * MHZ),
	FIXED_CLK(CLK_TOP_DMPLL, CLK_XTAL, 400 * MHZ),
	FIXED_CLK(CLK_TOP_VENCPLL, CLK_XTAL, 295.75 * MHZ),
	FIXED_CLK(CLK_TOP_HDMI_0_PIX340M, CLK_XTAL, 340 * MHZ),
	FIXED_CLK(CLK_TOP_HDMI_0_DEEP340M, CLK_XTAL, 340 * MHZ),
	FIXED_CLK(CLK_TOP_HDMI_0_PLL340M, CLK_XTAL, 340 * MHZ),
	FIXED_CLK(CLK_TOP_HADDS2_FB, CLK_XTAL, 27 * MHZ),
	FIXED_CLK(CLK_TOP_WBG_DIG_416M, CLK_XTAL, 416 * MHZ),
	FIXED_CLK(CLK_TOP_DSI0_LNTC_DSI, CLK_XTAL, 143 * MHZ),
	FIXED_CLK(CLK_TOP_HDMI_SCL_RX, CLK_XTAL, 27 * MHZ),
	FIXED_CLK(CLK_TOP_32K_EXTERNAL, CLK_XTAL, 32000),
	FIXED_CLK(CLK_TOP_HDMITX_CLKDIG_CTS, CLK_XTAL, 300 * MHZ),
	FIXED_CLK(CLK_TOP_AUD_EXT1, CLK_XTAL, 0),
	FIXED_CLK(CLK_TOP_AUD_EXT2, CLK_XTAL, 0),
	FIXED_CLK(CLK_TOP_NFI1X_PAD, CLK_XTAL, 0),
};

static const struct mtk_fixed_factor top_fixed_divs[] = {
	FACTOR0(CLK_TOP_SYSPLL, CLK_APMIXED_MAINPLL, 1, 1),
	FACTOR0(CLK_TOP_SYSPLL_D2, CLK_APMIXED_MAINPLL, 1, 2),
	FACTOR0(CLK_TOP_SYSPLL_D3, CLK_APMIXED_MAINPLL, 1, 3),
	FACTOR0(CLK_TOP_SYSPLL_D5, CLK_APMIXED_MAINPLL, 1, 5),
	FACTOR0(CLK_TOP_SYSPLL_D7, CLK_APMIXED_MAINPLL, 1, 7),
	FACTOR1(CLK_TOP_SYSPLL1_D2, CLK_TOP_SYSPLL_D2, 1, 2),
	FACTOR1(CLK_TOP_SYSPLL1_D4, CLK_TOP_SYSPLL_D2, 1, 4),
	FACTOR1(CLK_TOP_SYSPLL1_D8, CLK_TOP_SYSPLL_D2, 1, 8),
	FACTOR1(CLK_TOP_SYSPLL1_D16, CLK_TOP_SYSPLL_D2, 1, 16),
	FACTOR1(CLK_TOP_SYSPLL2_D2, CLK_TOP_SYSPLL_D3, 1, 2),
	FACTOR1(CLK_TOP_SYSPLL2_D4, CLK_TOP_SYSPLL_D3, 1, 4),
	FACTOR1(CLK_TOP_SYSPLL2_D8, CLK_TOP_SYSPLL_D3, 1, 8),
	FACTOR1(CLK_TOP_SYSPLL3_D2, CLK_TOP_SYSPLL_D5, 1, 2),
	FACTOR1(CLK_TOP_SYSPLL3_D4, CLK_TOP_SYSPLL_D5, 1, 4),
	FACTOR1(CLK_TOP_SYSPLL4_D2, CLK_TOP_SYSPLL_D7, 1, 2),
	FACTOR1(CLK_TOP_SYSPLL4_D4, CLK_TOP_SYSPLL_D7, 1, 4),

	FACTOR0(CLK_TOP_UNIVPLL, CLK_APMIXED_UNIVPLL, 1, 1),
	FACTOR0(CLK_TOP_UNIVPLL_D2, CLK_APMIXED_UNIVPLL, 1, 2),
	FACTOR0(CLK_TOP_UNIVPLL_D3, CLK_APMIXED_UNIVPLL, 1, 3),
	FACTOR0(CLK_TOP_UNIVPLL_D5, CLK_APMIXED_UNIVPLL, 1, 5),
	FACTOR0(CLK_TOP_UNIVPLL_D7, CLK_APMIXED_UNIVPLL, 1, 7),
	FACTOR0(CLK_TOP_UNIVPLL_D26, CLK_APMIXED_UNIVPLL, 1, 26),
	FACTOR0(CLK_TOP_UNIVPLL_D52, CLK_APMIXED_UNIVPLL, 1, 52),
	FACTOR0(CLK_TOP_UNIVPLL_D108, CLK_APMIXED_UNIVPLL, 1, 108),
	FACTOR0(CLK_TOP_USB_PHY48M, CLK_APMIXED_UNIVPLL, 1, 26),
	FACTOR1(CLK_TOP_UNIVPLL1_D2, CLK_TOP_UNIVPLL_D2, 1, 2),
	FACTOR1(CLK_TOP_UNIVPLL1_D4, CLK_TOP_UNIVPLL_D2, 1, 4),
	FACTOR1(CLK_TOP_UNIVPLL1_D8, CLK_TOP_UNIVPLL_D2, 1, 8),
	FACTOR1(CLK_TOP_UNIVPLL2_D2, CLK_TOP_UNIVPLL_D3, 1, 2),
	FACTOR1(CLK_TOP_UNIVPLL2_D4, CLK_TOP_UNIVPLL_D3, 1, 4),
	FACTOR1(CLK_TOP_UNIVPLL2_D8, CLK_TOP_UNIVPLL_D3, 1, 8),
	FACTOR1(CLK_TOP_UNIVPLL2_D16, CLK_TOP_UNIVPLL_D3, 1, 16),
	FACTOR1(CLK_TOP_UNIVPLL2_D32, CLK_TOP_UNIVPLL_D3, 1, 32),
	FACTOR1(CLK_TOP_UNIVPLL3_D2, CLK_TOP_UNIVPLL_D5, 1, 2),
	FACTOR1(CLK_TOP_UNIVPLL3_D4, CLK_TOP_UNIVPLL_D5, 1, 4),
	FACTOR1(CLK_TOP_UNIVPLL3_D8, CLK_TOP_UNIVPLL_D5, 1, 8),

	FACTOR0(CLK_TOP_MSDCPLL, CLK_APMIXED_MSDCPLL, 1, 1),
	FACTOR0(CLK_TOP_MSDCPLL_D2, CLK_APMIXED_MSDCPLL, 1, 2),
	FACTOR0(CLK_TOP_MSDCPLL_D4, CLK_APMIXED_MSDCPLL, 1, 4),
	FACTOR0(CLK_TOP_MSDCPLL_D8, CLK_APMIXED_MSDCPLL, 1, 8),

	FACTOR0(CLK_TOP_MMPLL, CLK_APMIXED_MMPLL, 1, 1),
	FACTOR0(CLK_TOP_MMPLL_D2, CLK_APMIXED_MMPLL, 1, 2),

	FACTOR1(CLK_TOP_DMPLL_D2, CLK_TOP_DMPLL, 1, 2),
	FACTOR1(CLK_TOP_DMPLL_D4, CLK_TOP_DMPLL, 1, 4),
	FACTOR1(CLK_TOP_DMPLL_X2, CLK_TOP_DMPLL, 1, 1),

	FACTOR0(CLK_TOP_TVDPLL, CLK_APMIXED_TVDPLL, 1, 1),
	FACTOR0(CLK_TOP_TVDPLL_D2, CLK_APMIXED_TVDPLL, 1, 2),
	FACTOR0(CLK_TOP_TVDPLL_D4, CLK_APMIXED_TVDPLL, 1, 4),

	FACTOR0(CLK_TOP_VDECPLL, CLK_APMIXED_VDECPLL, 1, 1),
	FACTOR0(CLK_TOP_TVD2PLL, CLK_APMIXED_TVD2PLL, 1, 1),
	FACTOR0(CLK_TOP_TVD2PLL_D2, CLK_APMIXED_TVD2PLL, 1, 2),

	FACTOR1(CLK_TOP_MIPIPLL, CLK_TOP_DPI, 1, 1),
	FACTOR1(CLK_TOP_MIPIPLL_D2, CLK_TOP_DPI, 1, 2),
	FACTOR1(CLK_TOP_MIPIPLL_D4, CLK_TOP_DPI, 1, 4),

	FACTOR1(CLK_TOP_HDMIPLL, CLK_TOP_HDMITX_CLKDIG_CTS, 1, 1),
	FACTOR1(CLK_TOP_HDMIPLL_D2, CLK_TOP_HDMITX_CLKDIG_CTS, 1, 2),
	FACTOR1(CLK_TOP_HDMIPLL_D3, CLK_TOP_HDMITX_CLKDIG_CTS, 1, 3),

	FACTOR0(CLK_TOP_ARMPLL_1P3G, CLK_APMIXED_ARMPLL, 1, 1),

	FACTOR1(CLK_TOP_AUDPLL, CLK_TOP_AUDPLL_MUX_SEL, 1, 1),
	FACTOR1(CLK_TOP_AUDPLL_D4, CLK_TOP_AUDPLL_MUX_SEL, 1, 4),
	FACTOR1(CLK_TOP_AUDPLL_D8, CLK_TOP_AUDPLL_MUX_SEL, 1, 8),
	FACTOR1(CLK_TOP_AUDPLL_D16, CLK_TOP_AUDPLL_MUX_SEL, 1, 16),
	FACTOR1(CLK_TOP_AUDPLL_D24, CLK_TOP_AUDPLL_MUX_SEL, 1, 24),

	FACTOR0(CLK_TOP_AUD1PLL_98M, CLK_APMIXED_AUD1PLL, 1, 3),
	FACTOR0(CLK_TOP_AUD2PLL_90M, CLK_APMIXED_AUD2PLL, 1, 3),
	FACTOR0(CLK_TOP_HADDS2PLL_98M, CLK_APMIXED_HADDS2PLL, 1, 3),
	FACTOR0(CLK_TOP_HADDS2PLL_294M, CLK_APMIXED_HADDS2PLL, 1, 1),
	FACTOR0(CLK_TOP_ETHPLL_500M, CLK_APMIXED_ETHPLL, 1, 1),
	FACTOR2(CLK_TOP_CLK26M_D8, CLK_XTAL, 1, 8),
	FACTOR2(CLK_TOP_32K_INTERNAL, CLK_XTAL, 1, 793),
	FACTOR1(CLK_TOP_AXISEL_D4, CLK_TOP_AXI_SEL, 1, 4),
	FACTOR1(CLK_TOP_8BDAC, CLK_TOP_UNIVPLL_D2, 1, 1),
};

static const int axi_parents[] = {
	CLK_XTAL,
	CLK_TOP_SYSPLL1_D2,
	CLK_TOP_SYSPLL_D5,
	CLK_TOP_SYSPLL1_D4,
	CLK_TOP_UNIVPLL_D5,
	CLK_TOP_UNIVPLL2_D2,
	CLK_TOP_MMPLL_D2,
	CLK_TOP_DMPLL_D2
};

static const int mem_parents[] = {
	CLK_XTAL,
	CLK_TOP_DMPLL
};

static const int ddrphycfg_parents[] = {
	CLK_XTAL,
	CLK_TOP_SYSPLL1_D8
};

static const int mm_parents[] = {
	CLK_XTAL,
	CLK_TOP_VENCPLL,
	CLK_TOP_SYSPLL1_D2,
	CLK_TOP_SYSPLL1_D4,
	CLK_TOP_UNIVPLL_D5,
	CLK_TOP_UNIVPLL1_D2,
	CLK_TOP_UNIVPLL2_D2,
	CLK_TOP_DMPLL
};

static const int pwm_parents[] = {
	CLK_XTAL,
	CLK_TOP_UNIVPLL2_D4,
	CLK_TOP_UNIVPLL3_D2,
	CLK_TOP_UNIVPLL1_D4
};

static const int vdec_parents[] = {
	CLK_XTAL,
	CLK_TOP_VDECPLL,
	CLK_TOP_SYSPLL_D5,
	CLK_TOP_SYSPLL1_D4,
	CLK_TOP_UNIVPLL_D5,
	CLK_TOP_UNIVPLL2_D2,
	CLK_TOP_VENCPLL,
	CLK_TOP_MSDCPLL_D2,
	CLK_TOP_MMPLL_D2
};

static const int mfg_parents[] = {
	CLK_XTAL,
	CLK_TOP_MMPLL,
	CLK_TOP_DMPLL_X2,
	CLK_TOP_MSDCPLL,
	CLK_XTAL,
	CLK_TOP_SYSPLL_D3,
	CLK_TOP_UNIVPLL_D3,
	CLK_TOP_UNIVPLL1_D2
};

static const int camtg_parents[] = {
	CLK_XTAL,
	CLK_TOP_UNIVPLL_D26,
	CLK_TOP_UNIVPLL2_D2,
	CLK_TOP_SYSPLL3_D2,
	CLK_TOP_SYSPLL3_D4,
	CLK_TOP_MSDCPLL_D2,
	CLK_TOP_MMPLL_D2
};

static const int uart_parents[] = {
	CLK_XTAL,
	CLK_TOP_UNIVPLL2_D8
};

static const int spi_parents[] = {
	CLK_XTAL,
	CLK_TOP_SYSPLL3_D2,
	CLK_TOP_SYSPLL4_D2,
	CLK_TOP_UNIVPLL2_D4,
	CLK_TOP_UNIVPLL1_D8
};

static const int usb20_parents[] = {
	CLK_XTAL,
	CLK_TOP_UNIVPLL1_D8,
	CLK_TOP_UNIVPLL3_D4
};

static const int msdc30_parents[] = {
	CLK_XTAL,
	CLK_TOP_MSDCPLL_D2,
	CLK_TOP_SYSPLL2_D2,
	CLK_TOP_SYSPLL1_D4,
	CLK_TOP_UNIVPLL1_D4,
	CLK_TOP_UNIVPLL2_D4,
};

static const int aud_intbus_parents[] = {
	CLK_XTAL,
	CLK_TOP_SYSPLL1_D4,
	CLK_TOP_SYSPLL3_D2,
	CLK_TOP_SYSPLL4_D2,
	CLK_TOP_UNIVPLL3_D2,
	CLK_TOP_UNIVPLL2_D4
};

static const int pmicspi_parents[] = {
	CLK_XTAL,
	CLK_TOP_SYSPLL1_D8,
	CLK_TOP_SYSPLL2_D4,
	CLK_TOP_SYSPLL4_D2,
	CLK_TOP_SYSPLL3_D4,
	CLK_TOP_SYSPLL2_D8,
	CLK_TOP_SYSPLL1_D16,
	CLK_TOP_UNIVPLL3_D4,
	CLK_TOP_UNIVPLL_D26,
	CLK_TOP_DMPLL_D2,
	CLK_TOP_DMPLL_D4
};

static const int scp_parents[] = {
	CLK_XTAL,
	CLK_TOP_SYSPLL1_D8,
	CLK_TOP_DMPLL_D2,
	CLK_TOP_DMPLL_D4
};

static const int dpi0_tve_parents[] = {
	CLK_XTAL,
	CLK_TOP_MIPIPLL,
	CLK_TOP_MIPIPLL_D2,
	CLK_TOP_MIPIPLL_D4,
	CLK_XTAL,
	CLK_TOP_TVDPLL,
	CLK_TOP_TVDPLL_D2,
	CLK_TOP_TVDPLL_D4
};

static const int dpi1_parents[] = {
	CLK_XTAL,
	CLK_TOP_TVDPLL,
	CLK_TOP_TVDPLL_D2,
	CLK_TOP_TVDPLL_D4
};

static const int hdmi_parents[] = {
	CLK_XTAL,
	CLK_TOP_HDMIPLL,
	CLK_TOP_HDMIPLL_D2,
	CLK_TOP_HDMIPLL_D3
};

static const int apll_parents[] = {
	CLK_XTAL,
	CLK_TOP_AUDPLL,
	CLK_TOP_AUDPLL_D4,
	CLK_TOP_AUDPLL_D8,
	CLK_TOP_AUDPLL_D16,
	CLK_TOP_AUDPLL_D24,
	CLK_XTAL,
	CLK_XTAL
};

static const int rtc_parents[] = {
	CLK_TOP_32K_INTERNAL,
	CLK_TOP_32K_EXTERNAL,
	CLK_XTAL,
	CLK_TOP_UNIVPLL3_D8
};

static const int nfi2x_parents[] = {
	CLK_XTAL,
	CLK_TOP_SYSPLL2_D2,
	CLK_TOP_SYSPLL_D7,
	CLK_TOP_UNIVPLL3_D2,
	CLK_TOP_SYSPLL2_D4,
	CLK_TOP_UNIVPLL3_D4,
	CLK_TOP_SYSPLL4_D4,
	CLK_XTAL
};

static const int emmc_hclk_parents[] = {
	CLK_XTAL,
	CLK_TOP_SYSPLL1_D2,
	CLK_TOP_SYSPLL1_D4,
	CLK_TOP_SYSPLL2_D2
};

static const int flash_parents[] = {
	CLK_TOP_CLK26M_D8,
	CLK_XTAL,
	CLK_TOP_SYSPLL2_D8,
	CLK_TOP_SYSPLL3_D4,
	CLK_TOP_UNIVPLL3_D4,
	CLK_TOP_SYSPLL4_D2,
	CLK_TOP_SYSPLL2_D4,
	CLK_TOP_UNIVPLL2_D4
};

static const int di_parents[] = {
	CLK_XTAL,
	CLK_TOP_TVD2PLL,
	CLK_TOP_TVD2PLL_D2,
	CLK_XTAL
};

static const int nr_osd_parents[] = {
	CLK_XTAL,
	CLK_TOP_VENCPLL,
	CLK_TOP_SYSPLL1_D2,
	CLK_TOP_SYSPLL1_D4,
	CLK_TOP_UNIVPLL_D5,
	CLK_TOP_UNIVPLL1_D2,
	CLK_TOP_UNIVPLL2_D2,
	CLK_TOP_DMPLL
};

static const int hdmirx_bist_parents[] = {
	CLK_XTAL,
	CLK_TOP_SYSPLL_D3,
	CLK_XTAL,
	CLK_TOP_SYSPLL1_D16,
	CLK_TOP_SYSPLL4_D2,
	CLK_TOP_SYSPLL1_D4,
	CLK_TOP_VENCPLL,
	CLK_XTAL
};

static const int intdir_parents[] = {
	CLK_XTAL,
	CLK_TOP_MMPLL,
	CLK_TOP_SYSPLL_D2,
	CLK_TOP_UNIVPLL_D2
};

static const int asm_parents[] = {
	CLK_XTAL,
	CLK_TOP_UNIVPLL2_D4,
	CLK_TOP_UNIVPLL2_D2,
	CLK_TOP_SYSPLL_D5
};

static const int ms_card_parents[] = {
	CLK_XTAL,
	CLK_TOP_UNIVPLL3_D8,
	CLK_TOP_SYSPLL4_D4
};

static const int ethif_parents[] = {
	CLK_XTAL,
	CLK_TOP_SYSPLL1_D2,
	CLK_TOP_SYSPLL_D5,
	CLK_TOP_SYSPLL1_D4,
	CLK_TOP_UNIVPLL_D5,
	CLK_TOP_UNIVPLL1_D2,
	CLK_TOP_DMPLL,
	CLK_TOP_DMPLL_D2
};

static const int hdmirx_parents[] = {
	CLK_XTAL,
	CLK_TOP_UNIVPLL_D52
};

static const int cmsys_parents[] = {
	CLK_XTAL,
	CLK_TOP_SYSPLL1_D2,
	CLK_TOP_UNIVPLL1_D2,
	CLK_TOP_UNIVPLL_D5,
	CLK_TOP_SYSPLL_D5,
	CLK_TOP_SYSPLL2_D2,
	CLK_TOP_SYSPLL1_D4,
	CLK_TOP_SYSPLL3_D2,
	CLK_TOP_SYSPLL2_D4,
	CLK_TOP_SYSPLL1_D8,
	CLK_XTAL,
	CLK_XTAL,
	CLK_XTAL,
	CLK_XTAL,
	CLK_XTAL
};

static const int clk_8bdac_parents[] = {
	CLK_TOP_32K_INTERNAL,
	CLK_TOP_8BDAC,
	CLK_XTAL,
	CLK_XTAL
};

static const int aud2dvd_parents[] = {
	CLK_TOP_AUD_48K_TIMING,
	CLK_TOP_AUD_44K_TIMING
};

static const int padmclk_parents[] = {
	CLK_XTAL,
	CLK_TOP_UNIVPLL_D26,
	CLK_TOP_UNIVPLL_D52,
	CLK_TOP_UNIVPLL_D108,
	CLK_TOP_UNIVPLL2_D8,
	CLK_TOP_UNIVPLL2_D16,
	CLK_TOP_UNIVPLL2_D32
};

static const int aud_mux_parents[] = {
	CLK_XTAL,
	CLK_TOP_AUD1PLL_98M,
	CLK_TOP_AUD2PLL_90M,
	CLK_TOP_HADDS2PLL_98M,
	CLK_TOP_AUD_EXTCK1_DIV,
	CLK_TOP_AUD_EXTCK2_DIV
};

static const int aud_src_parents[] = {
	CLK_TOP_AUD_MUX1_SEL,
	CLK_TOP_AUD_MUX2_SEL
};

static const struct mtk_composite top_muxes[] = {
	MUX_GATE(CLK_TOP_AXI_SEL, axi_parents, 0x40, 0, 3, 7),
	MUX_GATE(CLK_TOP_MEM_SEL, mem_parents, 0x40, 8, 1, 15),
	MUX_GATE(CLK_TOP_DDRPHYCFG_SEL, ddrphycfg_parents, 0x40, 16, 1, 23),
	MUX_GATE_FLAGS(CLK_TOP_MM_SEL, mm_parents, 0x40, 24, 3, 31,
		       CLK_DOMAIN_SCPSYS),

	MUX_GATE(CLK_TOP_PWM_SEL, pwm_parents, 0x50, 0, 2, 7),
	MUX_GATE(CLK_TOP_VDEC_SEL, vdec_parents, 0x50, 8, 4, 15),
	MUX_GATE_FLAGS(CLK_TOP_MFG_SEL, mfg_parents, 0x50, 16, 3, 23,
		       CLK_DOMAIN_SCPSYS),
	MUX_GATE(CLK_TOP_CAMTG_SEL, camtg_parents, 0x50, 24, 3, 31),

	MUX_GATE(CLK_TOP_UART_SEL, uart_parents, 0x60, 0, 1, 7),
	MUX_GATE(CLK_TOP_SPI0_SEL, spi_parents, 0x60, 8, 3, 15),
	MUX_GATE(CLK_TOP_USB20_SEL, usb20_parents, 0x60, 16, 2, 23),
	MUX_GATE(CLK_TOP_MSDC30_0_SEL, msdc30_parents, 0x60, 24, 3, 31),

	MUX_GATE(CLK_TOP_MSDC30_1_SEL, msdc30_parents, 0x70, 0, 3, 7),
	MUX_GATE(CLK_TOP_MSDC30_2_SEL, msdc30_parents, 0x70, 8, 3, 15),
	MUX_GATE(CLK_TOP_AUDIO_SEL, msdc30_parents, 0x70, 16, 1, 23),
	MUX_GATE(CLK_TOP_AUDINTBUS_SEL, aud_intbus_parents, 0x70, 24, 3, 31),

	MUX_GATE(CLK_TOP_PMICSPI_SEL, pmicspi_parents, 0x80, 0, 4, 7),
	MUX_GATE(CLK_TOP_SCP_SEL, scp_parents, 0x80, 8, 2, 15),
	MUX_GATE(CLK_TOP_DPI0_SEL, dpi0_tve_parents, 0x80, 16, 3, 23),
	MUX_GATE(CLK_TOP_DPI1_SEL, dpi1_parents, 0x80, 24, 2, 31),

	MUX_GATE(CLK_TOP_TVE_SEL, dpi0_tve_parents, 0x90, 0, 3, 7),
	MUX_GATE(CLK_TOP_HDMI_SEL, hdmi_parents, 0x90, 8, 2, 15),
	MUX_GATE(CLK_TOP_APLL_SEL, apll_parents, 0x90, 16, 3, 23),

	MUX_GATE(CLK_TOP_RTC_SEL, rtc_parents, 0xA0, 0, 2, 7),
	MUX_GATE(CLK_TOP_NFI2X_SEL, nfi2x_parents, 0xA0, 8, 3, 15),
	MUX_GATE(CLK_TOP_EMMC_HCLK_SEL, emmc_hclk_parents, 0xA0, 24, 2, 31),

	MUX_GATE(CLK_TOP_FLASH_SEL, flash_parents, 0xB0, 0, 3, 7),
	MUX_GATE(CLK_TOP_DI_SEL, di_parents, 0xB0, 8, 2, 15),
	MUX_GATE(CLK_TOP_NR_SEL, nr_osd_parents, 0xB0, 16, 3, 23),
	MUX_GATE(CLK_TOP_OSD_SEL, nr_osd_parents, 0xB0, 24, 3, 31),

	MUX_GATE(CLK_TOP_HDMIRX_BIST_SEL, hdmirx_bist_parents, 0xC0, 0, 3, 7),
	MUX_GATE(CLK_TOP_INTDIR_SEL, intdir_parents, 0xC0, 8, 2, 15),
	MUX_GATE(CLK_TOP_ASM_I_SEL, asm_parents, 0xC0, 16, 2, 23),
	MUX_GATE(CLK_TOP_ASM_M_SEL, asm_parents, 0xC0, 24, 3, 31),

	MUX_GATE(CLK_TOP_ASM_H_SEL, asm_parents, 0xD0, 0, 2, 7),
	MUX_GATE(CLK_TOP_MS_CARD_SEL, ms_card_parents, 0xD0, 16, 2, 23),
	MUX_GATE_FLAGS(CLK_TOP_ETHIF_SEL, ethif_parents, 0xD0, 24, 3, 31,
		       CLK_DOMAIN_SCPSYS),

	MUX_GATE(CLK_TOP_HDMIRX26_24_SEL, hdmirx_parents, 0xE0, 0, 1, 7),
	MUX_GATE(CLK_TOP_MSDC30_3_SEL, msdc30_parents, 0xE0, 8, 3, 15),
	MUX_GATE(CLK_TOP_CMSYS_SEL, cmsys_parents, 0xE0, 16, 4, 23),

	MUX_GATE(CLK_TOP_SPI1_SEL, spi_parents, 0xE0, 24, 3, 31),
	MUX_GATE(CLK_TOP_SPI2_SEL, spi_parents, 0xF0, 0, 3, 7),
	MUX_GATE(CLK_TOP_8BDAC_SEL, clk_8bdac_parents, 0xF0, 8, 2, 15),
	MUX_GATE(CLK_TOP_AUD2DVD_SEL, aud2dvd_parents, 0xF0, 16, 1, 23),

	MUX(CLK_TOP_PADMCLK_SEL, padmclk_parents, 0x100, 0, 3),

	MUX(CLK_TOP_AUD_MUX1_SEL, aud_mux_parents, 0x12c, 0, 3),
	MUX(CLK_TOP_AUD_MUX2_SEL, aud_mux_parents, 0x12c, 3, 3),
	MUX(CLK_TOP_AUDPLL_MUX_SEL, aud_mux_parents, 0x12c, 6, 3),

	MUX_GATE(CLK_TOP_AUD_K1_SRC_SEL, aud_src_parents, 0x12c, 15, 1, 23),
	MUX_GATE(CLK_TOP_AUD_K2_SRC_SEL, aud_src_parents, 0x12c, 16, 1, 24),
	MUX_GATE(CLK_TOP_AUD_K3_SRC_SEL, aud_src_parents, 0x12c, 17, 1, 25),
	MUX_GATE(CLK_TOP_AUD_K4_SRC_SEL, aud_src_parents, 0x12c, 18, 1, 26),
	MUX_GATE(CLK_TOP_AUD_K5_SRC_SEL, aud_src_parents, 0x12c, 19, 1, 27),
	MUX_GATE(CLK_TOP_AUD_K6_SRC_SEL, aud_src_parents, 0x12c, 20, 1, 28),
};

/* infracfg */
static const struct mtk_gate_regs infra_cg_regs = {
	.set_ofs = 0x40,
	.clr_ofs = 0x44,
	.sta_ofs = 0x48,
};

#define GATE_INFRA(_id, _parent, _shift) {			\
		.id = _id,					\
		.parent = _parent,				\
		.regs = &infra_cg_regs,				\
		.shift = _shift,				\
		.flags = CLK_GATE_SETCLR | CLK_PARENT_TOPCKGEN,	\
	}

static const struct mtk_gate infra_cgs[] = {
	GATE_INFRA(CLK_INFRA_DBG, CLK_TOP_AXI_SEL, 0),
	GATE_INFRA(CLK_INFRA_SMI, CLK_TOP_MM_SEL, 1),
	GATE_INFRA(CLK_INFRA_QAXI_CM4, CLK_TOP_AXI_SEL, 2),
	GATE_INFRA(CLK_INFRA_AUD_SPLIN_B, CLK_TOP_HADDS2PLL_294M, 4),
	GATE_INFRA(CLK_INFRA_AUDIO, CLK_XTAL, 5),
	GATE_INFRA(CLK_INFRA_EFUSE, CLK_XTAL, 6),
	GATE_INFRA(CLK_INFRA_L2C_SRAM, CLK_TOP_MM_SEL, 7),
	GATE_INFRA(CLK_INFRA_M4U, CLK_TOP_MEM_SEL, 8),
	GATE_INFRA(CLK_INFRA_CONNMCU, CLK_TOP_WBG_DIG_416M, 12),
	GATE_INFRA(CLK_INFRA_TRNG, CLK_TOP_AXI_SEL, 13),
	GATE_INFRA(CLK_INFRA_RAMBUFIF, CLK_TOP_MEM_SEL, 14),
	GATE_INFRA(CLK_INFRA_CPUM, CLK_TOP_MEM_SEL, 15),
	GATE_INFRA(CLK_INFRA_KP, CLK_TOP_AXI_SEL, 16),
	GATE_INFRA(CLK_INFRA_CEC, CLK_TOP_RTC_SEL, 18),
	GATE_INFRA(CLK_INFRA_IRRX, CLK_TOP_AXI_SEL, 19),
	GATE_INFRA(CLK_INFRA_PMICSPI, CLK_TOP_PMICSPI_SEL, 22),
	GATE_INFRA(CLK_INFRA_PMICWRAP, CLK_TOP_AXI_SEL, 23),
	GATE_INFRA(CLK_INFRA_DDCCI, CLK_TOP_AXI_SEL, 24),
};

/* pericfg */
static const struct mtk_gate_regs peri0_cg_regs = {
	.set_ofs = 0x8,
	.clr_ofs = 0x10,
	.sta_ofs = 0x18,
};

static const struct mtk_gate_regs peri1_cg_regs = {
	.set_ofs = 0xC,
	.clr_ofs = 0x14,
	.sta_ofs = 0x1C,
};

#define GATE_PERI0(_id, _parent, _shift) {			\
		.id = _id,					\
		.parent = _parent,				\
		.regs = &peri0_cg_regs,				\
		.shift = _shift,				\
		.flags = CLK_GATE_SETCLR | CLK_PARENT_TOPCKGEN,	\
	}

#define GATE_PERI1(_id, _parent, _shift) {			\
		.id = _id,					\
		.parent = _parent,				\
		.regs = &peri1_cg_regs,				\
		.shift = _shift,				\
		.flags = CLK_GATE_SETCLR | CLK_PARENT_TOPCKGEN,	\
	}

static const struct mtk_gate peri_cgs[] = {
	GATE_PERI0(CLK_PERI_NFI, CLK_TOP_NFI2X_SEL, 0),
	GATE_PERI0(CLK_PERI_THERM, CLK_TOP_AXI_SEL, 1),
	GATE_PERI0(CLK_PERI_PWM1, CLK_TOP_AXISEL_D4, 2),
	GATE_PERI0(CLK_PERI_PWM2, CLK_TOP_AXISEL_D4, 3),
	GATE_PERI0(CLK_PERI_PWM3, CLK_TOP_AXISEL_D4, 4),
	GATE_PERI0(CLK_PERI_PWM4, CLK_TOP_AXISEL_D4, 5),
	GATE_PERI0(CLK_PERI_PWM5, CLK_TOP_AXISEL_D4, 6),
	GATE_PERI0(CLK_PERI_PWM6, CLK_TOP_AXISEL_D4, 7),
	GATE_PERI0(CLK_PERI_PWM7, CLK_TOP_AXISEL_D4, 8),
	GATE_PERI0(CLK_PERI_PWM, CLK_TOP_AXI_SEL, 9),
	GATE_PERI0(CLK_PERI_USB0, CLK_TOP_USB20_SEL, 10),
	GATE_PERI0(CLK_PERI_USB1, CLK_TOP_USB20_SEL, 11),
	GATE_PERI0(CLK_PERI_AP_DMA, CLK_TOP_AXI_SEL, 12),
	GATE_PERI0(CLK_PERI_MSDC30_0, CLK_TOP_MSDC30_0_SEL, 13),
	GATE_PERI0(CLK_PERI_MSDC30_1, CLK_TOP_MSDC30_1_SEL, 14),
	GATE_PERI0(CLK_PERI_MSDC30_2, CLK_TOP_MSDC30_2_SEL, 15),
	GATE_PERI0(CLK_PERI_MSDC30_3, CLK_TOP_MSDC30_3_SEL, 16),
	GATE_PERI0(CLK_PERI_MSDC50_3, CLK_TOP_EMMC_HCLK_SEL, 17),
	GATE_PERI0(CLK_PERI_NLI, CLK_TOP_AXI_SEL, 18),
	GATE_PERI0(CLK_PERI_UART0, CLK_TOP_AXI_SEL, 19),
	GATE_PERI0(CLK_PERI_UART1, CLK_TOP_AXI_SEL, 20),
	GATE_PERI0(CLK_PERI_UART2, CLK_TOP_AXI_SEL, 21),
	GATE_PERI0(CLK_PERI_UART3, CLK_TOP_AXI_SEL, 22),
	GATE_PERI0(CLK_PERI_BTIF, CLK_TOP_AXI_SEL, 23),
	GATE_PERI0(CLK_PERI_I2C0, CLK_TOP_AXI_SEL, 24),
	GATE_PERI0(CLK_PERI_I2C1, CLK_TOP_AXI_SEL, 25),
	GATE_PERI0(CLK_PERI_I2C2, CLK_TOP_AXI_SEL, 26),
	GATE_PERI0(CLK_PERI_I2C3, CLK_XTAL, 27),
	GATE_PERI0(CLK_PERI_AUXADC, CLK_XTAL, 28),
	GATE_PERI0(CLK_PERI_SPI0, CLK_TOP_SPI0_SEL, 29),
	GATE_PERI0(CLK_PERI_ETH, CLK_XTAL, 30),
	GATE_PERI0(CLK_PERI_USB0_MCU, CLK_TOP_AXI_SEL, 31),

	GATE_PERI1(CLK_PERI_USB1_MCU, CLK_TOP_AXI_SEL, 0),
	GATE_PERI1(CLK_PERI_USB_SLV, CLK_TOP_AXI_SEL, 1),
	GATE_PERI1(CLK_PERI_GCPU, CLK_TOP_AXI_SEL, 2),
	GATE_PERI1(CLK_PERI_NFI_ECC, CLK_TOP_NFI1X_PAD, 3),
	GATE_PERI1(CLK_PERI_NFI_PAD, CLK_TOP_NFI1X_PAD, 4),
	GATE_PERI1(CLK_PERI_FLASH, CLK_TOP_NFI2X_SEL, 5),
	GATE_PERI1(CLK_PERI_HOST89_INT, CLK_TOP_AXI_SEL, 6),
	GATE_PERI1(CLK_PERI_HOST89_SPI, CLK_TOP_SPI0_SEL, 7),
	GATE_PERI1(CLK_PERI_HOST89_DVD, CLK_TOP_AUD2DVD_SEL, 8),
	GATE_PERI1(CLK_PERI_SPI1, CLK_TOP_SPI1_SEL, 9),
	GATE_PERI1(CLK_PERI_SPI2, CLK_TOP_SPI2_SEL, 10),
	GATE_PERI1(CLK_PERI_FCI, CLK_TOP_MS_CARD_SEL, 11),
};

/* ethsys and hifsys */
static const struct mtk_gate_regs eth_hif_cg_regs = {
	.sta_ofs = 0x30,
};

#define GATE_ETH_HIF(_id, _parent, _shift, _flag) {		\
		.id = _id,					\
		.parent = _parent,				\
		.regs = &eth_hif_cg_regs,			\
		.shift = _shift,				\
		.flags = CLK_GATE_NO_SETCLR_INV | (_flag),	\
	}

#define GATE_ETH_HIF0(_id, _parent, _shift)				\
	GATE_ETH_HIF(_id, _parent, _shift, CLK_PARENT_APMIXED)

#define GATE_ETH_HIF1(_id, _parent, _shift)				\
	GATE_ETH_HIF(_id, _parent, _shift, CLK_PARENT_TOPCKGEN)

static const struct mtk_gate eth_cgs[] = {
	GATE_ETH_HIF1(CLK_ETHSYS_HSDMA, CLK_TOP_ETHIF_SEL, 5),
	GATE_ETH_HIF1(CLK_ETHSYS_ESW, CLK_TOP_ETHPLL_500M, 6),
	GATE_ETH_HIF0(CLK_ETHSYS_GP2, CLK_APMIXED_TRGPLL, 7),
	GATE_ETH_HIF1(CLK_ETHSYS_GP1, CLK_TOP_ETHPLL_500M, 8),
	GATE_ETH_HIF1(CLK_ETHSYS_PCM, CLK_TOP_ETHIF_SEL, 11),
	GATE_ETH_HIF1(CLK_ETHSYS_GDMA, CLK_TOP_ETHIF_SEL, 14),
	GATE_ETH_HIF1(CLK_ETHSYS_I2S, CLK_TOP_ETHIF_SEL, 17),
	GATE_ETH_HIF1(CLK_ETHSYS_CRYPTO, CLK_TOP_ETHIF_SEL, 29),
};

static const struct mtk_gate hif_cgs[] = {
	GATE_ETH_HIF1(CLK_HIFSYS_USB0PHY, CLK_TOP_ETHPLL_500M, 21),
	GATE_ETH_HIF1(CLK_HIFSYS_USB1PHY, CLK_TOP_ETHPLL_500M, 22),
	GATE_ETH_HIF1(CLK_HIFSYS_PCIE0, CLK_TOP_ETHPLL_500M, 24),
	GATE_ETH_HIF1(CLK_HIFSYS_PCIE1, CLK_TOP_ETHPLL_500M, 25),
	GATE_ETH_HIF1(CLK_HIFSYS_PCIE2, CLK_TOP_ETHPLL_500M, 26),
};

static const struct mtk_clk_tree mt7623_clk_tree = {
	.xtal_rate = 26 * MHZ,
	.xtal2_rate = 26 * MHZ,
	.fdivs_offs = CLK_TOP_SYSPLL,
	.muxes_offs = CLK_TOP_AXI_SEL,
	.plls = apmixed_plls,
	.fclks = top_fixed_clks,
	.fdivs = top_fixed_divs,
	.muxes = top_muxes,
};

static int mt7623_mcucfg_probe(struct udevice *dev)
{
	void __iomem *base;

	base = dev_read_addr_ptr(dev);
	if (!base)
		return -ENOENT;

	clrsetbits_le32(base + MCU_AXI_DIV, AXI_DIV_MSK,
			AXI_DIV_SEL(0x12));

	return 0;
}

static int mt7623_apmixedsys_probe(struct udevice *dev)
{
	struct mtk_clk_priv *priv = dev_get_priv(dev);
	int ret;

	ret = mtk_common_clk_init(dev, &mt7623_clk_tree);
	if (ret)
		return ret;

	/* reduce clock square disable time */
	writel(0x50001, priv->base + MT7623_CLKSQ_STB_CON0);
	/* extend control timing to 1us */
	writel(0x888, priv->base + MT7623_PLL_ISO_CON0);

	return 0;
}

static int mt7623_topckgen_probe(struct udevice *dev)
{
	return mtk_common_clk_init(dev, &mt7623_clk_tree);
}

static int mt7623_infracfg_probe(struct udevice *dev)
{
	return mtk_common_clk_gate_init(dev, &mt7623_clk_tree, infra_cgs);
}

static int mt7623_pericfg_probe(struct udevice *dev)
{
	return mtk_common_clk_gate_init(dev, &mt7623_clk_tree, peri_cgs);
}

static int mt7623_hifsys_probe(struct udevice *dev)
{
	return mtk_common_clk_gate_init(dev, &mt7623_clk_tree, hif_cgs);
}

static int mt7623_ethsys_probe(struct udevice *dev)
{
	return mtk_common_clk_gate_init(dev, &mt7623_clk_tree, eth_cgs);
}

static int mt7623_ethsys_hifsys_bind(struct udevice *dev)
{
	int ret = 0;

#if CONFIG_IS_ENABLED(RESET_MEDIATEK)
	ret = mediatek_reset_bind(dev, ETHSYS_HIFSYS_RST_CTRL_OFS, 1);
	if (ret)
		debug("Warning: failed to bind reset controller\n");
#endif

	return ret;
}

static const struct udevice_id mt7623_apmixed_compat[] = {
	{ .compatible = "mediatek,mt7623-apmixedsys" },
	{ }
};

static const struct udevice_id mt7623_topckgen_compat[] = {
	{ .compatible = "mediatek,mt7623-topckgen" },
	{ }
};

static const struct udevice_id mt7623_infracfg_compat[] = {
	{ .compatible = "mediatek,mt7623-infracfg", },
	{ }
};

static const struct udevice_id mt7623_pericfg_compat[] = {
	{ .compatible = "mediatek,mt7623-pericfg", },
	{ }
};

static const struct udevice_id mt7623_ethsys_compat[] = {
	{ .compatible = "mediatek,mt7623-ethsys" },
	{ }
};

static const struct udevice_id mt7623_hifsys_compat[] = {
	{ .compatible = "mediatek,mt7623-hifsys" },
	{ }
};

static const struct udevice_id mt7623_mcucfg_compat[] = {
	{ .compatible = "mediatek,mt7623-mcucfg" },
	{ }
};

U_BOOT_DRIVER(mtk_mcucfg) = {
	.name = "mt7623-mcucfg",
	.id = UCLASS_SYSCON,
	.of_match = mt7623_mcucfg_compat,
	.probe = mt7623_mcucfg_probe,
	.flags = DM_FLAG_PRE_RELOC,
};

U_BOOT_DRIVER(mtk_clk_apmixedsys) = {
	.name = "mt7623-clock-apmixedsys",
	.id = UCLASS_CLK,
	.of_match = mt7623_apmixed_compat,
	.probe = mt7623_apmixedsys_probe,
	.priv_auto	= sizeof(struct mtk_clk_priv),
	.ops = &mtk_clk_apmixedsys_ops,
	.flags = DM_FLAG_PRE_RELOC,
};

U_BOOT_DRIVER(mtk_clk_topckgen) = {
	.name = "mt7623-clock-topckgen",
	.id = UCLASS_CLK,
	.of_match = mt7623_topckgen_compat,
	.probe = mt7623_topckgen_probe,
	.priv_auto	= sizeof(struct mtk_clk_priv),
	.ops = &mtk_clk_topckgen_ops,
	.flags = DM_FLAG_PRE_RELOC,
};

U_BOOT_DRIVER(mtk_clk_infracfg) = {
	.name = "mt7623-infracfg",
	.id = UCLASS_CLK,
	.of_match = mt7623_infracfg_compat,
	.probe = mt7623_infracfg_probe,
	.priv_auto	= sizeof(struct mtk_cg_priv),
	.ops = &mtk_clk_gate_ops,
	.flags = DM_FLAG_PRE_RELOC,
};

U_BOOT_DRIVER(mtk_clk_pericfg) = {
	.name = "mt7623-pericfg",
	.id = UCLASS_CLK,
	.of_match = mt7623_pericfg_compat,
	.probe = mt7623_pericfg_probe,
	.priv_auto	= sizeof(struct mtk_cg_priv),
	.ops = &mtk_clk_gate_ops,
	.flags = DM_FLAG_PRE_RELOC,
};

U_BOOT_DRIVER(mtk_clk_hifsys) = {
	.name = "mt7623-clock-hifsys",
	.id = UCLASS_CLK,
	.of_match = mt7623_hifsys_compat,
	.probe = mt7623_hifsys_probe,
	.bind = mt7623_ethsys_hifsys_bind,
	.priv_auto	= sizeof(struct mtk_cg_priv),
	.ops = &mtk_clk_gate_ops,
};

U_BOOT_DRIVER(mtk_clk_ethsys) = {
	.name = "mt7623-clock-ethsys",
	.id = UCLASS_CLK,
	.of_match = mt7623_ethsys_compat,
	.probe = mt7623_ethsys_probe,
	.bind = mt7623_ethsys_hifsys_bind,
	.priv_auto	= sizeof(struct mtk_cg_priv),
	.ops = &mtk_clk_gate_ops,
};
