// SPDX-License-Identifier: GPL-2.0
/*
 * MediaTek clock driver for MT8512 SoC
 *
 * Copyright (C) 2019 BayLibre, SAS
 * Author: Chen Zhong <chen.zhong@mediatek.com>
 */

#include <common.h>
#include <dm.h>
#include <asm/io.h>
#include <dt-bindings/clock/mt8512-clk.h>
#include <linux/bitops.h>

#include "clk-mtk.h"

#define MT8512_PLL_FMAX		(3800UL * MHZ)
#define MT8512_PLL_FMIN		(1500UL * MHZ)
#define MT8512_CON0_RST_BAR	BIT(23)

/* apmixedsys */
#define PLL(_id, _reg, _pwr_reg, _en_mask, _flags, _pcwbits, _pd_reg,	\
	    _pd_shift, _pcw_reg, _pcw_shift, _pcw_chg_reg) {		\
		.id = _id,						\
		.reg = _reg,						\
		.pwr_reg = _pwr_reg,					\
		.en_mask = _en_mask,					\
		.rst_bar_mask = MT8512_CON0_RST_BAR,			\
		.fmax = MT8512_PLL_FMAX,				\
		.fmin = MT8512_PLL_FMIN,				\
		.flags = _flags,					\
		.pcwbits = _pcwbits,					\
		.pcwibits = 8,					\
		.pd_reg = _pd_reg,					\
		.pd_shift = _pd_shift,					\
		.pcw_reg = _pcw_reg,					\
		.pcw_shift = _pcw_shift,				\
		.pcw_chg_reg = _pcw_chg_reg,			\
	}

static const struct mtk_pll_data apmixed_plls[] = {
	PLL(CLK_APMIXED_ARMPLL, 0x030C, 0x0318, 0x00000001,
	    0, 22, 0x0310, 24, 0x0310, 0, 0),
	PLL(CLK_APMIXED_MAINPLL, 0x0228, 0x0234, 0x00000001,
	    HAVE_RST_BAR, 22, 0x022C, 24, 0x022C, 0, 0),
	PLL(CLK_APMIXED_UNIVPLL2, 0x0208, 0x0214, 0x00000001,
	    HAVE_RST_BAR, 22, 0x020C, 24, 0x020C, 0, 0),
	PLL(CLK_APMIXED_MSDCPLL, 0x0350, 0x035C, 0x00000001,
	    0, 22, 0x0354, 24, 0x0354, 0, 0),
	PLL(CLK_APMIXED_APLL1, 0x031C, 0x032C, 0x00000001,
	    0, 32, 0x0320, 24, 0x0324, 0, 0x0320),
	PLL(CLK_APMIXED_APLL2, 0x0360, 0x0370, 0x00000001,
	    0, 32, 0x0364, 24, 0x0368, 0, 0x0364),
	PLL(CLK_APMIXED_IPPLL, 0x0374, 0x0380, 0x00000001,
	    0, 22, 0x0378, 24, 0x0378, 0, 0),
	PLL(CLK_APMIXED_DSPPLL, 0x0390, 0x039C, 0x00000001,
	    0, 22, 0x0394, 24, 0x0394, 0, 0),
	PLL(CLK_APMIXED_TCONPLL, 0x03A0, 0x03AC, 0x00000001,
	    0, 22, 0x03A4, 24, 0x03A4, 0, 0),
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
	FIXED_CLK(CLK_TOP_CLK32K, CLK_XTAL, 32000),
};

static const struct mtk_fixed_factor top_fixed_divs[] = {
	FACTOR0(CLK_TOP_SYSPLL1_D2, CLK_APMIXED_MAINPLL, 1, 4),
	FACTOR0(CLK_TOP_SYSPLL1_D4, CLK_APMIXED_MAINPLL, 1, 8),
	FACTOR0(CLK_TOP_SYSPLL1_D8, CLK_APMIXED_MAINPLL, 1, 16),
	FACTOR0(CLK_TOP_SYSPLL1_D16, CLK_APMIXED_MAINPLL, 1, 32),
	FACTOR0(CLK_TOP_SYSPLL_D3, CLK_APMIXED_MAINPLL, 1, 3),
	FACTOR0(CLK_TOP_SYSPLL2_D2, CLK_APMIXED_MAINPLL, 1, 6),
	FACTOR0(CLK_TOP_SYSPLL2_D4, CLK_APMIXED_MAINPLL, 1, 12),
	FACTOR0(CLK_TOP_SYSPLL2_D8, CLK_APMIXED_MAINPLL, 1, 24),
	FACTOR0(CLK_TOP_SYSPLL_D5, CLK_APMIXED_MAINPLL, 1, 5),
	FACTOR0(CLK_TOP_SYSPLL3_D4, CLK_APMIXED_MAINPLL, 1, 20),
	FACTOR0(CLK_TOP_SYSPLL_D7, CLK_APMIXED_MAINPLL, 1, 7),
	FACTOR0(CLK_TOP_SYSPLL4_D2, CLK_APMIXED_MAINPLL, 1, 14),
	FACTOR0(CLK_TOP_UNIVPLL, CLK_APMIXED_UNIVPLL2, 1, 2),
	FACTOR1(CLK_TOP_UNIVPLL_D2, CLK_TOP_UNIVPLL, 1, 2),
	FACTOR1(CLK_TOP_UNIVPLL1_D2, CLK_TOP_UNIVPLL, 1, 4),
	FACTOR1(CLK_TOP_UNIVPLL1_D4, CLK_TOP_UNIVPLL, 1, 8),
	FACTOR1(CLK_TOP_UNIVPLL1_D8, CLK_TOP_UNIVPLL, 1, 16),
	FACTOR1(CLK_TOP_UNIVPLL_D3, CLK_TOP_UNIVPLL, 1, 3),
	FACTOR1(CLK_TOP_UNIVPLL2_D2, CLK_TOP_UNIVPLL, 1, 6),
	FACTOR1(CLK_TOP_UNIVPLL2_D4, CLK_TOP_UNIVPLL, 1, 12),
	FACTOR1(CLK_TOP_UNIVPLL2_D8, CLK_TOP_UNIVPLL, 1, 24),
	FACTOR1(CLK_TOP_UNIVPLL_D5, CLK_TOP_UNIVPLL, 1, 5),
	FACTOR1(CLK_TOP_UNIVPLL3_D2, CLK_TOP_UNIVPLL, 1, 10),
	FACTOR1(CLK_TOP_UNIVPLL3_D4, CLK_TOP_UNIVPLL, 1, 20),
	FACTOR0(CLK_TOP_TCONPLL_D2, CLK_APMIXED_TCONPLL, 1, 2),
	FACTOR0(CLK_TOP_TCONPLL_D4, CLK_APMIXED_TCONPLL, 1, 4),
	FACTOR0(CLK_TOP_TCONPLL_D8, CLK_APMIXED_TCONPLL, 1, 8),
	FACTOR0(CLK_TOP_TCONPLL_D16, CLK_APMIXED_TCONPLL, 1, 16),
	FACTOR0(CLK_TOP_TCONPLL_D32, CLK_APMIXED_TCONPLL, 1, 32),
	FACTOR0(CLK_TOP_TCONPLL_D64, CLK_APMIXED_TCONPLL, 1, 64),
	FACTOR1(CLK_TOP_USB20_192M, CLK_TOP_UNIVPLL, 2, 13),
	FACTOR1(CLK_TOP_USB20_192M_D2, CLK_TOP_USB20_192M, 1, 2),
	FACTOR1(CLK_TOP_USB20_192M_D4_T, CLK_TOP_USB20_192M, 1, 4),
	FACTOR0(CLK_TOP_APLL1, CLK_APMIXED_APLL1, 1, 1),
	FACTOR0(CLK_TOP_APLL1_D2, CLK_APMIXED_APLL1, 1, 2),
	FACTOR0(CLK_TOP_APLL1_D3, CLK_APMIXED_APLL1, 1, 3),
	FACTOR0(CLK_TOP_APLL1_D4, CLK_APMIXED_APLL1, 1, 4),
	FACTOR0(CLK_TOP_APLL1_D8, CLK_APMIXED_APLL1, 1, 8),
	FACTOR0(CLK_TOP_APLL1_D16, CLK_APMIXED_APLL1, 1, 16),
	FACTOR0(CLK_TOP_APLL2, CLK_APMIXED_APLL2, 1, 1),
	FACTOR0(CLK_TOP_APLL2_D2, CLK_APMIXED_APLL2, 1, 2),
	FACTOR0(CLK_TOP_APLL2_D3, CLK_APMIXED_APLL2, 1, 3),
	FACTOR0(CLK_TOP_APLL2_D4, CLK_APMIXED_APLL2, 1, 4),
	FACTOR0(CLK_TOP_APLL2_D8, CLK_APMIXED_APLL2, 1, 8),
	FACTOR0(CLK_TOP_APLL2_D16, CLK_APMIXED_APLL2, 1, 16),
	FACTOR2(CLK_TOP_CLK26M, CLK_XTAL, 1, 1),
	FACTOR2(CLK_TOP_SYS_26M_D2, CLK_XTAL, 1, 2),
	FACTOR0(CLK_TOP_MSDCPLL, CLK_APMIXED_MSDCPLL, 1, 1),
	FACTOR0(CLK_TOP_MSDCPLL_D2, CLK_APMIXED_MSDCPLL, 1, 2),
	FACTOR0(CLK_TOP_DSPPLL, CLK_APMIXED_DSPPLL, 1, 1),
	FACTOR0(CLK_TOP_DSPPLL_D2, CLK_APMIXED_DSPPLL, 1, 2),
	FACTOR0(CLK_TOP_DSPPLL_D4, CLK_APMIXED_DSPPLL, 1, 4),
	FACTOR0(CLK_TOP_DSPPLL_D8, CLK_APMIXED_DSPPLL, 1, 8),
	FACTOR0(CLK_TOP_IPPLL, CLK_APMIXED_IPPLL, 1, 1),
	FACTOR0(CLK_TOP_IPPLL_D2, CLK_APMIXED_IPPLL, 1, 2),
	FACTOR1(CLK_TOP_NFI2X_CK_D2, CLK_TOP_NFI2X_SEL, 1, 2),
};

static const int axi_parents[] = {
	CLK_TOP_CLK26M,
	CLK_TOP_SYSPLL1_D4,
	CLK_TOP_UNIVPLL3_D2,
	CLK_TOP_SYSPLL1_D8,
	CLK_TOP_SYS_26M_D2,
	CLK_TOP_CLK32K
};

static const int mem_parents[] = {
	CLK_TOP_DSPPLL,
	CLK_TOP_IPPLL,
	CLK_TOP_CLK26M,
	CLK_TOP_UNIVPLL_D3
};

static const int uart_parents[] = {
	CLK_TOP_CLK26M,
	CLK_TOP_UNIVPLL2_D8
};

static const int spi_parents[] = {
	CLK_TOP_CLK26M,
	CLK_TOP_UNIVPLL2_D2,
	CLK_TOP_SYSPLL2_D2,
	CLK_TOP_UNIVPLL1_D4,
	CLK_TOP_SYSPLL1_D4,
	CLK_TOP_UNIVPLL3_D2,
	CLK_TOP_UNIVPLL2_D4,
	CLK_TOP_SYSPLL4_D2
};

static const int spis_parents[] = {
	CLK_TOP_CLK26M,
	CLK_TOP_UNIVPLL_D3,
	CLK_TOP_SYSPLL_D3,
	CLK_TOP_UNIVPLL1_D2,
	CLK_TOP_UNIVPLL2_D2,
	CLK_TOP_UNIVPLL1_D4,
	CLK_TOP_UNIVPLL2_D4,
	CLK_TOP_SYSPLL4_D2
};

static const int msdc50_0_hc_parents[] = {
	CLK_TOP_CLK26M,
	CLK_TOP_SYSPLL1_D2,
	CLK_TOP_UNIVPLL1_D4,
	CLK_TOP_SYSPLL2_D2
};

static const int msdc50_0_parents[] = {
	CLK_TOP_CLK26M,
	CLK_TOP_MSDCPLL_D2,
	CLK_TOP_UNIVPLL2_D2,
	CLK_TOP_SYSPLL2_D2,
	CLK_TOP_UNIVPLL1_D4,
	CLK_TOP_SYSPLL1_D4,
	CLK_TOP_SYSPLL2_D4,
	CLK_TOP_UNIVPLL2_D8
};

static const int msdc50_2_parents[] = {
	CLK_TOP_CLK26M,
	CLK_TOP_MSDCPLL,
	CLK_TOP_UNIVPLL_D3,
	CLK_TOP_UNIVPLL1_D2,
	CLK_TOP_SYSPLL1_D2,
	CLK_TOP_UNIVPLL2_D2,
	CLK_TOP_SYSPLL2_D2,
	CLK_TOP_UNIVPLL1_D4
};

static const int audio_parents[] = {
	CLK_TOP_CLK26M,
	CLK_TOP_UNIVPLL2_D8,
	CLK_TOP_APLL1_D4,
	CLK_TOP_APLL2_D4
};

static const int aud_intbus_parents[] = {
	CLK_TOP_CLK26M,
	CLK_TOP_SYSPLL1_D4,
	CLK_TOP_UNIVPLL3_D2,
	CLK_TOP_APLL2_D8,
	CLK_TOP_SYS_26M_D2,
	CLK_TOP_APLL1_D8,
	CLK_TOP_UNIVPLL3_D4
};

static const int hapll1_parents[] = {
	CLK_TOP_CLK26M,
	CLK_TOP_APLL1,
	CLK_TOP_APLL1_D2,
	CLK_TOP_APLL1_D3,
	CLK_TOP_APLL1_D4,
	CLK_TOP_APLL1_D8,
	CLK_TOP_APLL1_D16,
	CLK_TOP_SYS_26M_D2
};

static const int hapll2_parents[] = {
	CLK_TOP_CLK26M,
	CLK_TOP_APLL2,
	CLK_TOP_APLL2_D2,
	CLK_TOP_APLL2_D3,
	CLK_TOP_APLL2_D4,
	CLK_TOP_APLL2_D8,
	CLK_TOP_APLL2_D16,
	CLK_TOP_SYS_26M_D2
};

static const int asm_l_parents[] = {
	CLK_TOP_CLK26M,
	CLK_TOP_UNIVPLL2_D4,
	CLK_TOP_UNIVPLL2_D2,
	CLK_TOP_SYSPLL_D5
};

static const int aud_spdif_parents[] = {
	CLK_TOP_CLK26M,
	CLK_TOP_UNIVPLL_D2,
	CLK_TOP_DSPPLL
};

static const int aud_1_parents[] = {
	CLK_TOP_CLK26M,
	CLK_TOP_APLL1
};

static const int aud_2_parents[] = {
	CLK_TOP_CLK26M,
	CLK_TOP_APLL2
};

static const int ssusb_sys_parents[] = {
	CLK_TOP_CLK26M,
	CLK_TOP_UNIVPLL3_D4,
	CLK_TOP_UNIVPLL2_D4,
	CLK_TOP_UNIVPLL3_D2
};

static const int spm_parents[] = {
	CLK_TOP_CLK26M,
	CLK_TOP_SYSPLL1_D8
};

static const int i2c_parents[] = {
	CLK_TOP_CLK26M,
	CLK_TOP_SYS_26M_D2,
	CLK_TOP_UNIVPLL3_D4,
	CLK_TOP_UNIVPLL3_D2,
	CLK_TOP_SYSPLL1_D8,
	CLK_TOP_SYSPLL2_D8,
	CLK_TOP_CLK32K
};

static const int pwm_parents[] = {
	CLK_TOP_CLK26M,
	CLK_TOP_UNIVPLL3_D4,
	CLK_TOP_SYSPLL1_D8,
	CLK_TOP_UNIVPLL2_D4,
	CLK_TOP_SYS_26M_D2,
	CLK_TOP_CLK32K
};

static const int dsp_parents[] = {
	CLK_TOP_CLK26M,
	CLK_TOP_DSPPLL,
	CLK_TOP_DSPPLL_D2,
	CLK_TOP_DSPPLL_D4,
	CLK_TOP_DSPPLL_D8,
	CLK_TOP_APLL2_D4,
	CLK_TOP_SYS_26M_D2,
	CLK_TOP_CLK32K
};

static const int nfi2x_parents[] = {
	CLK_TOP_CLK26M,
	CLK_TOP_SYSPLL2_D2,
	CLK_TOP_SYSPLL_D7,
	CLK_TOP_SYSPLL_D3,
	CLK_TOP_SYSPLL2_D4,
	CLK_TOP_MSDCPLL_D2,
	CLK_TOP_UNIVPLL1_D2,
	CLK_TOP_UNIVPLL_D5
};

static const int spinfi_parents[] = {
	CLK_TOP_CLK26M,
	CLK_TOP_UNIVPLL2_D8,
	CLK_TOP_UNIVPLL3_D4,
	CLK_TOP_SYSPLL1_D8,
	CLK_TOP_SYSPLL4_D2,
	CLK_TOP_SYSPLL2_D4,
	CLK_TOP_UNIVPLL2_D4,
	CLK_TOP_UNIVPLL3_D2
};

static const int ecc_parents[] = {
	CLK_TOP_CLK26M,
	CLK_TOP_SYSPLL_D5,
	CLK_TOP_SYSPLL_D3,
	CLK_TOP_UNIVPLL_D3
};

static const int gcpu_parents[] = {
	CLK_TOP_CLK26M,
	CLK_TOP_UNIVPLL_D3,
	CLK_TOP_SYSPLL_D3,
	CLK_TOP_UNIVPLL1_D2,
	CLK_TOP_SYSPLL1_D2,
	CLK_TOP_UNIVPLL2_D2
};

static const int gcpu_cpm_parents[] = {
	CLK_TOP_CLK26M,
	CLK_TOP_UNIVPLL2_D2,
	CLK_TOP_SYSPLL2_D2,
	CLK_TOP_UNIVPLL1_D4
};

static const int mbist_diag_parents[] = {
	CLK_TOP_CLK26M,
	CLK_TOP_SYS_26M_D2
};

static const int ip0_nna_parents[] = {
	CLK_TOP_CLK26M,
	CLK_TOP_DSPPLL,
	CLK_TOP_DSPPLL_D2,
	CLK_TOP_DSPPLL_D4,
	CLK_TOP_IPPLL,
	CLK_TOP_SYS_26M_D2,
	CLK_TOP_IPPLL_D2,
	CLK_TOP_MSDCPLL_D2
};

static const int ip2_wfst_parents[] = {
	CLK_TOP_CLK26M,
	CLK_TOP_UNIVPLL_D3,
	CLK_TOP_UNIVPLL1_D2,
	CLK_TOP_UNIVPLL2_D2,
	CLK_TOP_IPPLL,
	CLK_TOP_IPPLL_D2,
	CLK_TOP_SYS_26M_D2,
	CLK_TOP_MSDCPLL
};

static const int sflash_parents[] = {
	CLK_TOP_CLK26M,
	CLK_TOP_SYSPLL1_D16,
	CLK_TOP_SYSPLL2_D8,
	CLK_TOP_SYSPLL3_D4,
	CLK_TOP_UNIVPLL3_D4,
	CLK_TOP_UNIVPLL1_D8,
	CLK_TOP_USB20_192M_D2,
	CLK_TOP_UNIVPLL2_D4
};

static const int sram_parents[] = {
	CLK_TOP_CLK26M,
	CLK_TOP_DSPPLL,
	CLK_TOP_UNIVPLL_D3,
	CLK_TOP_SYSPLL1_D2,
	CLK_TOP_APLL1,
	CLK_TOP_APLL2,
	CLK_TOP_SYSPLL1_D4,
	CLK_TOP_SYS_26M_D2
};

static const int mm_parents[] = {
	CLK_TOP_CLK26M,
	CLK_TOP_SYSPLL_D3,
	CLK_TOP_SYSPLL1_D2,
	CLK_TOP_SYSPLL_D5,
	CLK_TOP_SYSPLL1_D4,
	CLK_TOP_UNIVPLL_D5,
	CLK_TOP_UNIVPLL1_D2,
	CLK_TOP_UNIVPLL_D3
};

static const int dpi0_parents[] = {
	CLK_TOP_CLK26M,
	CLK_TOP_TCONPLL_D2,
	CLK_TOP_TCONPLL_D4,
	CLK_TOP_TCONPLL_D8,
	CLK_TOP_TCONPLL_D16,
	CLK_TOP_TCONPLL_D32,
	CLK_TOP_TCONPLL_D64
};

static const int dbg_atclk_parents[] = {
	CLK_TOP_CLK26M,
	CLK_TOP_UNIVPLL1_D2,
	CLK_TOP_UNIVPLL_D5
};

static const int occ_104m_parents[] = {
	CLK_TOP_UNIVPLL2_D4,
	CLK_TOP_UNIVPLL2_D8
};

static const int occ_68m_parents[] = {
	CLK_TOP_SYSPLL1_D8,
	CLK_TOP_UNIVPLL2_D8
};

static const int occ_182m_parents[] = {
	CLK_TOP_SYSPLL2_D2,
	CLK_TOP_UNIVPLL1_D4,
	CLK_TOP_UNIVPLL2_D8
};

static const struct mtk_composite top_muxes[] = {
	/* CLK_CFG_0 */
	MUX_CLR_SET_UPD_FLAGS(CLK_TOP_AXI_SEL, axi_parents,
			      0x040, 0x044, 0x048, 0, 3, 7,
			      0x4, 0, CLK_MUX_SETCLR_UPD),
	MUX_CLR_SET_UPD_FLAGS(CLK_TOP_MEM_SEL, mem_parents,
			      0x040, 0x044, 0x048, 8, 2, 15,
			      0x4, 1, CLK_MUX_SETCLR_UPD),
	MUX_CLR_SET_UPD_FLAGS(CLK_TOP_UART_SEL, uart_parents,
			      0x040, 0x044, 0x048, 16, 1, 23,
			      0x4, 2, CLK_MUX_SETCLR_UPD),
	MUX_CLR_SET_UPD_FLAGS(CLK_TOP_SPI_SEL, spi_parents,
			      0x040, 0x044, 0x048, 24, 3, 31,
			      0x4, 3, CLK_MUX_SETCLR_UPD),
	/* CLK_CFG_1 */
	MUX_CLR_SET_UPD_FLAGS(CLK_TOP_SPIS_SEL, spis_parents,
			      0x050, 0x054, 0x058, 0, 3, 7,
			      0x4, 4, CLK_MUX_SETCLR_UPD),
	MUX_CLR_SET_UPD_FLAGS(CLK_TOP_MSDC50_0_HC_SEL, msdc50_0_hc_parents,
			      0x050, 0x054, 0x058, 8, 2, 15,
			      0x4, 5, CLK_MUX_SETCLR_UPD),
	MUX_CLR_SET_UPD_FLAGS(CLK_TOP_MSDC2_2_HC_SEL, msdc50_0_hc_parents,
			      0x050, 0x054, 0x058, 16, 2, 23,
			      0x4, 6, CLK_MUX_SETCLR_UPD),
	MUX_CLR_SET_UPD_FLAGS(CLK_TOP_MSDC50_0_SEL, msdc50_0_parents,
			      0x050, 0x054, 0x058, 24, 3, 31,
			      0x4, 7, CLK_MUX_SETCLR_UPD),
	/* CLK_CFG_2 */
	MUX_CLR_SET_UPD_FLAGS(CLK_TOP_MSDC50_2_SEL, msdc50_2_parents,
			      0x060, 0x064, 0x068, 0, 3, 7,
			      0x4, 8, CLK_MUX_SETCLR_UPD),
	MUX_CLR_SET_UPD_FLAGS(CLK_TOP_MSDC30_1_SEL, msdc50_0_parents,
			      0x060, 0x064, 0x068, 8, 3, 15,
			      0x4, 9, CLK_MUX_SETCLR_UPD),
	MUX_CLR_SET_UPD_FLAGS(CLK_TOP_AUDIO_SEL, audio_parents,
			      0x060, 0x064, 0x068, 16, 2, 23,
			      0x4, 10, CLK_MUX_SETCLR_UPD),
	MUX_CLR_SET_UPD_FLAGS(CLK_TOP_AUD_INTBUS_SEL, aud_intbus_parents,
			      0x060, 0x064, 0x068, 24, 3, 31,
			      0x4, 11, CLK_MUX_SETCLR_UPD),
	/* CLK_CFG_3 */
	MUX_CLR_SET_UPD_FLAGS(CLK_TOP_HAPLL1_SEL, hapll1_parents,
			      0x070, 0x074, 0x078, 0, 3, 7,
			      0x4, 12, CLK_MUX_SETCLR_UPD),
	MUX_CLR_SET_UPD_FLAGS(CLK_TOP_HAPLL2_SEL, hapll2_parents,
			      0x070, 0x074, 0x078, 8, 3, 15,
			      0x4, 13, CLK_MUX_SETCLR_UPD),
	MUX_CLR_SET_UPD_FLAGS(CLK_TOP_A2SYS_SEL, hapll1_parents,
			      0x070, 0x074, 0x078, 16, 3, 23,
			      0x4, 14, CLK_MUX_SETCLR_UPD),
	MUX_CLR_SET_UPD_FLAGS(CLK_TOP_A1SYS_SEL, hapll2_parents,
			      0x070, 0x074, 0x078, 24, 3, 31,
			      0x4, 15, CLK_MUX_SETCLR_UPD),
	/* CLK_CFG_4 */
	MUX_CLR_SET_UPD_FLAGS(CLK_TOP_ASM_L_SEL, asm_l_parents,
			      0x080, 0x084, 0x088, 0, 2, 7,
			      0x4, 16, CLK_MUX_SETCLR_UPD),
	MUX_CLR_SET_UPD_FLAGS(CLK_TOP_ASM_M_SEL, asm_l_parents,
			      0x080, 0x084, 0x088, 8, 2, 15,
			      0x4, 17, CLK_MUX_SETCLR_UPD),
	MUX_CLR_SET_UPD_FLAGS(CLK_TOP_ASM_H_SEL, asm_l_parents,
			      0x080, 0x084, 0x088, 16, 2, 23,
			      0x4, 18, CLK_MUX_SETCLR_UPD),
	MUX_CLR_SET_UPD_FLAGS(CLK_TOP_AUD_SPDIF_SEL, aud_spdif_parents,
			      0x080, 0x084, 0x088, 24, 2, 31,
			      0x4, 19, CLK_MUX_SETCLR_UPD),
	/* CLK_CFG_5 */
	MUX_CLR_SET_UPD_FLAGS(CLK_TOP_AUD_1_SEL, aud_1_parents,
			      0x090, 0x094, 0x098, 0, 1, 7,
			      0x4, 20, CLK_MUX_SETCLR_UPD),
	MUX_CLR_SET_UPD_FLAGS(CLK_TOP_AUD_2_SEL, aud_2_parents,
			      0x090, 0x094, 0x098, 8, 1, 15,
			      0x4, 21, CLK_MUX_SETCLR_UPD),
	MUX_CLR_SET_UPD_FLAGS(CLK_TOP_SSUSB_SYS_SEL, ssusb_sys_parents,
			      0x090, 0x094, 0x098, 16, 2, 23,
			      0x4, 22, CLK_MUX_SETCLR_UPD),
	MUX_CLR_SET_UPD_FLAGS(CLK_TOP_SSUSB_XHCI_SEL, ssusb_sys_parents,
			      0x090, 0x094, 0x098, 24, 2, 31,
			      0x4, 23, CLK_MUX_SETCLR_UPD),
	/* CLK_CFG_6 */
	MUX_CLR_SET_UPD_FLAGS(CLK_TOP_SPM_SEL, spm_parents,
			      0x0a0, 0x0a4, 0x0a8, 0, 1, 7,
			      0x4, 24, CLK_MUX_SETCLR_UPD),
	MUX_CLR_SET_UPD_FLAGS(CLK_TOP_I2C_SEL, i2c_parents,
			      0x0a0, 0x0a4, 0x0a8, 8, 3, 15,
			      0x4, 25, CLK_MUX_SETCLR_UPD),
	MUX_CLR_SET_UPD_FLAGS(CLK_TOP_PWM_SEL, pwm_parents,
			      0x0a0, 0x0a4, 0x0a8, 16, 3, 23,
			      0x4, 26, CLK_MUX_SETCLR_UPD),
	MUX_CLR_SET_UPD_FLAGS(CLK_TOP_DSP_SEL, dsp_parents,
			      0x0a0, 0x0a4, 0x0a8, 24, 3, 31,
			      0x4, 27, CLK_MUX_SETCLR_UPD),
	/* CLK_CFG_7 */
	MUX_CLR_SET_UPD_FLAGS(CLK_TOP_NFI2X_SEL, nfi2x_parents,
			      0x0b0, 0x0b4, 0x0b8, 0, 3, 7,
			      0x4, 28, CLK_MUX_SETCLR_UPD),
	MUX_CLR_SET_UPD_FLAGS(CLK_TOP_SPINFI_SEL, spinfi_parents,
			      0x0b0, 0x0b4, 0x0b8, 8, 3, 15,
			      0x4, 29, CLK_MUX_SETCLR_UPD),
	MUX_CLR_SET_UPD_FLAGS(CLK_TOP_ECC_SEL, ecc_parents,
			      0x0b0, 0x0b4, 0x0b8, 16, 2, 23,
			      0x4, 30, CLK_MUX_SETCLR_UPD),
	MUX_CLR_SET_UPD_FLAGS(CLK_TOP_GCPU_SEL, gcpu_parents,
			      0x0b0, 0x0b4, 0x0b8, 24, 3, 31,
			      0x4, 31, CLK_MUX_SETCLR_UPD),
	/* CLK_CFG_8 */
	MUX_CLR_SET_UPD_FLAGS(CLK_TOP_GCPU_CPM_SEL, gcpu_cpm_parents,
			      0x0c0, 0x0c4, 0x0c8, 0, 2, 7,
			      0x8, 0, CLK_MUX_SETCLR_UPD),
	MUX_CLR_SET_UPD_FLAGS(CLK_TOP_MBIST_DIAG_SEL, mbist_diag_parents,
			      0x0c0, 0x0c4, 0x0c8, 8, 1, 15,
			      0x8, 1, CLK_MUX_SETCLR_UPD),
	MUX_CLR_SET_UPD_FLAGS(CLK_TOP_IP0_NNA_SEL, ip0_nna_parents,
			      0x0c0, 0x0c4, 0x0c8, 16, 3, 23,
			      0x8, 2, CLK_MUX_SETCLR_UPD),
	MUX_CLR_SET_UPD_FLAGS(CLK_TOP_IP1_NNA_SEL, ip0_nna_parents,
			      0x0c0, 0x0c4, 0x0c8, 24, 3, 31,
			      0x8, 3, CLK_MUX_SETCLR_UPD),
	/* CLK_CFG_9 */
	MUX_CLR_SET_UPD_FLAGS(CLK_TOP_IP2_WFST_SEL, ip2_wfst_parents,
			      0x0d0, 0x0d4, 0x0d8, 0, 3, 7,
			      0x8, 4, CLK_MUX_SETCLR_UPD),
	MUX_CLR_SET_UPD_FLAGS(CLK_TOP_SFLASH_SEL, sflash_parents,
			      0x0d0, 0x0d4, 0x0d8, 8, 3, 15,
			      0x8, 5, CLK_MUX_SETCLR_UPD),
	MUX_CLR_SET_UPD_FLAGS(CLK_TOP_SRAM_SEL, sram_parents,
			      0x0d0, 0x0d4, 0x0d8, 16, 3, 23,
			      0x8, 6, CLK_MUX_SETCLR_UPD),
	MUX_CLR_SET_UPD_FLAGS(CLK_TOP_MM_SEL, mm_parents,
			      0x0d0, 0x0d4, 0x0d8, 24, 3, 31,
			      0x8, 7, CLK_MUX_SETCLR_UPD),
	/* CLK_CFG_10 */
	MUX_CLR_SET_UPD_FLAGS(CLK_TOP_DPI0_SEL, dpi0_parents,
			      0x0e0, 0x0e4, 0x0e8, 0, 3, 7,
			      0x8, 8, CLK_MUX_SETCLR_UPD),
	MUX_CLR_SET_UPD_FLAGS(CLK_TOP_DBG_ATCLK_SEL, dbg_atclk_parents,
			      0x0e0, 0x0e4, 0x0e8, 8, 2, 15,
			      0x8, 9, CLK_MUX_SETCLR_UPD),
	MUX_CLR_SET_UPD_FLAGS(CLK_TOP_OCC_104M_SEL, occ_104m_parents,
			      0x0e0, 0x0e4, 0x0e8, 16, 1, 23,
			      0x8, 10, CLK_MUX_SETCLR_UPD),
	MUX_CLR_SET_UPD_FLAGS(CLK_TOP_OCC_68M_SEL, occ_68m_parents,
			      0x0e0, 0x0e4, 0x0e8, 24, 1, 31,
			      0x8, 11, CLK_MUX_SETCLR_UPD),
	/* CLK_CFG_11 */
	MUX_CLR_SET_UPD_FLAGS(CLK_TOP_OCC_182M_SEL, occ_182m_parents,
			      0x0ec, 0x0f0, 0x0f4, 0, 2, 7,
			      0x8, 12, CLK_MUX_SETCLR_UPD),
};

static const struct mtk_gate_regs top0_cg_regs = {
	.set_ofs = 0x0,
	.clr_ofs = 0x0,
	.sta_ofs = 0x0,
};

static const struct mtk_gate_regs top1_cg_regs = {
	.set_ofs = 0x104,
	.clr_ofs = 0x104,
	.sta_ofs = 0x104,
};

#define GATE_TOP0(_id, _parent, _shift) {			\
		.id = _id,					\
		.parent = _parent,				\
		.regs = &top0_cg_regs,				\
		.shift = _shift,				\
		.flags = CLK_GATE_NO_SETCLR | CLK_PARENT_TOPCKGEN,	\
	}

#define GATE_TOP1(_id, _parent, _shift) {			\
		.id = _id,					\
		.parent = _parent,				\
		.regs = &top1_cg_regs,				\
		.shift = _shift,				\
		.flags = CLK_GATE_NO_SETCLR_INV | CLK_PARENT_TOPCKGEN,	\
	}

static const struct mtk_gate top_clks[] = {
	/* TOP0 */
	GATE_TOP0(CLK_TOP_CONN_32K, CLK_TOP_CLK32K, 10),
	GATE_TOP0(CLK_TOP_CONN_26M, CLK_TOP_CLK26M, 11),
	GATE_TOP0(CLK_TOP_DSP_32K, CLK_TOP_CLK32K, 16),
	GATE_TOP0(CLK_TOP_DSP_26M, CLK_TOP_CLK26M, 17),
	/* TOP1 */
	GATE_TOP1(CLK_TOP_USB20_48M_EN, CLK_TOP_USB20_192M_D4_T, 8),
	GATE_TOP1(CLK_TOP_UNIVPLL_48M_EN, CLK_TOP_USB20_192M_D4_T, 9),
	GATE_TOP1(CLK_TOP_SSUSB_TOP_CK_EN, CLK_TOP_CLK_NULL, 22),
	GATE_TOP1(CLK_TOP_SSUSB_PHY_CK_EN, CLK_TOP_CLK_NULL, 23),
};

static const struct mtk_gate_regs infra0_cg_regs = {
	.set_ofs = 0x294,
	.clr_ofs = 0x294,
	.sta_ofs = 0x294,
};

static const struct mtk_gate_regs infra1_cg_regs = {
	.set_ofs = 0x80,
	.clr_ofs = 0x84,
	.sta_ofs = 0x90,
};

static const struct mtk_gate_regs infra2_cg_regs = {
	.set_ofs = 0x88,
	.clr_ofs = 0x8c,
	.sta_ofs = 0x94,
};

static const struct mtk_gate_regs infra3_cg_regs = {
	.set_ofs = 0xa4,
	.clr_ofs = 0xa8,
	.sta_ofs = 0xac,
};

static const struct mtk_gate_regs infra4_cg_regs = {
	.set_ofs = 0xc0,
	.clr_ofs = 0xc4,
	.sta_ofs = 0xc8,
};

static const struct mtk_gate_regs infra5_cg_regs = {
	.set_ofs = 0xd0,
	.clr_ofs = 0xd4,
	.sta_ofs = 0xd8,
};

#define GATE_INFRA0(_id, _parent, _shift) {			\
		.id = _id,					\
		.parent = _parent,				\
		.regs = &infra0_cg_regs,				\
		.shift = _shift,				\
		.flags = CLK_GATE_NO_SETCLR_INV | CLK_PARENT_TOPCKGEN,	\
	}

#define GATE_INFRA1(_id, _parent, _shift) {			\
		.id = _id,					\
		.parent = _parent,				\
		.regs = &infra1_cg_regs,				\
		.shift = _shift,				\
		.flags = CLK_GATE_SETCLR | CLK_PARENT_TOPCKGEN,	\
	}

#define GATE_INFRA2(_id, _parent, _shift) {			\
		.id = _id,					\
		.parent = _parent,				\
		.regs = &infra2_cg_regs,				\
		.shift = _shift,				\
		.flags = CLK_GATE_SETCLR | CLK_PARENT_TOPCKGEN,	\
	}

#define GATE_INFRA3(_id, _parent, _shift) {			\
		.id = _id,					\
		.parent = _parent,				\
		.regs = &infra3_cg_regs,				\
		.shift = _shift,				\
		.flags = CLK_GATE_SETCLR | CLK_PARENT_TOPCKGEN,	\
	}

#define GATE_INFRA4(_id, _parent, _shift) {			\
		.id = _id,					\
		.parent = _parent,				\
		.regs = &infra4_cg_regs,				\
		.shift = _shift,				\
		.flags = CLK_GATE_SETCLR | CLK_PARENT_TOPCKGEN,	\
	}

#define GATE_INFRA5(_id, _parent, _shift) {			\
		.id = _id,					\
		.parent = _parent,				\
		.regs = &infra5_cg_regs,				\
		.shift = _shift,				\
		.flags = CLK_GATE_SETCLR | CLK_PARENT_TOPCKGEN,	\
	}

static const struct mtk_gate infra_clks[] = {
	/* INFRA0 */
	GATE_INFRA0(CLK_INFRA_DSP_AXI, CLK_TOP_AXI_SEL, 8),
	/* INFRA1 */
	GATE_INFRA1(CLK_INFRA_APXGPT, CLK_TOP_AXI_SEL, 6),
	GATE_INFRA1(CLK_INFRA_ICUSB, CLK_TOP_AXI_SEL, 8),
	GATE_INFRA1(CLK_INFRA_GCE, CLK_TOP_AXI_SEL, 9),
	GATE_INFRA1(CLK_INFRA_THERM, CLK_TOP_AXI_SEL, 10),
	GATE_INFRA1(CLK_INFRA_PWM_HCLK, CLK_TOP_AXI_SEL, 15),
	GATE_INFRA1(CLK_INFRA_PWM1, CLK_TOP_PWM_SEL, 16),
	GATE_INFRA1(CLK_INFRA_PWM2, CLK_TOP_PWM_SEL, 17),
	GATE_INFRA1(CLK_INFRA_PWM3, CLK_TOP_PWM_SEL, 18),
	GATE_INFRA1(CLK_INFRA_PWM4, CLK_TOP_PWM_SEL, 19),
	GATE_INFRA1(CLK_INFRA_PWM5, CLK_TOP_PWM_SEL, 20),
	GATE_INFRA1(CLK_INFRA_PWM, CLK_TOP_PWM_SEL, 21),
	GATE_INFRA1(CLK_INFRA_UART0, CLK_TOP_UART_SEL, 22),
	GATE_INFRA1(CLK_INFRA_UART1, CLK_TOP_UART_SEL, 23),
	GATE_INFRA1(CLK_INFRA_UART2, CLK_TOP_UART_SEL, 24),
	GATE_INFRA1(CLK_INFRA_DSP_UART, CLK_TOP_UART_SEL, 26),
	GATE_INFRA1(CLK_INFRA_GCE_26M, CLK_TOP_CLK26M, 27),
	GATE_INFRA1(CLK_INFRA_CQDMA_FPC, CLK_TOP_AXI_SEL, 28),
	GATE_INFRA1(CLK_INFRA_BTIF, CLK_TOP_AXI_SEL, 31),
	/* INFRA2 */
	GATE_INFRA2(CLK_INFRA_SPI, CLK_TOP_SPI_SEL, 1),
	GATE_INFRA2(CLK_INFRA_MSDC0, CLK_TOP_MSDC50_0_HC_SEL, 2),
	GATE_INFRA2(CLK_INFRA_MSDC1, CLK_TOP_AXI_SEL, 4),
	GATE_INFRA2(CLK_INFRA_DVFSRC, CLK_TOP_CLK26M, 7),
	GATE_INFRA2(CLK_INFRA_GCPU, CLK_TOP_AXI_SEL, 8),
	GATE_INFRA2(CLK_INFRA_TRNG, CLK_TOP_AXI_SEL, 9),
	GATE_INFRA2(CLK_INFRA_AUXADC, CLK_TOP_CLK26M, 10),
	GATE_INFRA2(CLK_INFRA_AUXADC_MD, CLK_TOP_CLK26M, 14),
	GATE_INFRA2(CLK_INFRA_AP_DMA, CLK_TOP_AXI_SEL, 18),
	GATE_INFRA2(CLK_INFRA_DEBUGSYS, CLK_TOP_AXI_SEL, 24),
	GATE_INFRA2(CLK_INFRA_AUDIO, CLK_TOP_AXI_SEL, 25),
	GATE_INFRA2(CLK_INFRA_FLASHIF, CLK_TOP_SFLASH_SEL, 29),
	/* INFRA3 */
	GATE_INFRA3(CLK_INFRA_PWM_FB6, CLK_TOP_PWM_SEL, 0),
	GATE_INFRA3(CLK_INFRA_PWM_FB7, CLK_TOP_PWM_SEL, 1),
	GATE_INFRA3(CLK_INFRA_AUD_ASRC, CLK_TOP_AXI_SEL, 3),
	GATE_INFRA3(CLK_INFRA_AUD_26M, CLK_TOP_CLK26M, 4),
	GATE_INFRA3(CLK_INFRA_SPIS, CLK_TOP_AXI_SEL, 6),
	GATE_INFRA3(CLK_INFRA_CQ_DMA, CLK_TOP_AXI_SEL, 27),
	/* INFRA4 */
	GATE_INFRA4(CLK_INFRA_AP_MSDC0, CLK_TOP_MSDC50_0_SEL, 7),
	GATE_INFRA4(CLK_INFRA_MD_MSDC0, CLK_TOP_MSDC50_0_SEL, 8),
	GATE_INFRA4(CLK_INFRA_MSDC0_SRC, CLK_TOP_MSDC50_0_SEL, 9),
	GATE_INFRA4(CLK_INFRA_MSDC1_SRC, CLK_TOP_MSDC30_1_SEL, 10),
	GATE_INFRA4(CLK_INFRA_IRRX_26M, CLK_TOP_AXI_SEL, 22),
	GATE_INFRA4(CLK_INFRA_IRRX_32K, CLK_TOP_CLK32K, 23),
	GATE_INFRA4(CLK_INFRA_I2C0_AXI, CLK_TOP_I2C_SEL, 24),
	GATE_INFRA4(CLK_INFRA_I2C1_AXI, CLK_TOP_I2C_SEL, 25),
	GATE_INFRA4(CLK_INFRA_I2C2_AXI, CLK_TOP_I2C_SEL, 26),
	/* INFRA5 */
	GATE_INFRA5(CLK_INFRA_NFI, CLK_TOP_NFI2X_CK_D2, 1),
	GATE_INFRA5(CLK_INFRA_NFIECC, CLK_TOP_NFI2X_CK_D2, 2),
	GATE_INFRA5(CLK_INFRA_NFI_HCLK, CLK_TOP_AXI_SEL, 3),
	GATE_INFRA5(CLK_INFRA_SUSB_133, CLK_TOP_AXI_SEL, 7),
	GATE_INFRA5(CLK_INFRA_USB_SYS, CLK_TOP_SSUSB_SYS_SEL, 9),
	GATE_INFRA5(CLK_INFRA_USB_XHCI, CLK_TOP_SSUSB_XHCI_SEL, 11),
};

static const struct mtk_clk_tree mt8512_clk_tree = {
	.xtal_rate = 26 * MHZ,
	.xtal2_rate = 26 * MHZ,
	.fdivs_offs = CLK_TOP_SYSPLL1_D2,
	.muxes_offs = CLK_TOP_AXI_SEL,
	.plls = apmixed_plls,
	.fclks = top_fixed_clks,
	.fdivs = top_fixed_divs,
	.muxes = top_muxes,
};

static int mt8512_apmixedsys_probe(struct udevice *dev)
{
	return mtk_common_clk_init(dev, &mt8512_clk_tree);
}

static int mt8512_topckgen_probe(struct udevice *dev)
{
	return mtk_common_clk_init(dev, &mt8512_clk_tree);
}

static int mt8512_topckgen_cg_probe(struct udevice *dev)
{
	return mtk_common_clk_gate_init(dev, &mt8512_clk_tree, top_clks);
}

static int mt8512_infracfg_probe(struct udevice *dev)
{
	return mtk_common_clk_gate_init(dev, &mt8512_clk_tree, infra_clks);
}

static const struct udevice_id mt8512_apmixed_compat[] = {
	{ .compatible = "mediatek,mt8512-apmixedsys", },
	{ }
};

static const struct udevice_id mt8512_topckgen_compat[] = {
	{ .compatible = "mediatek,mt8512-topckgen", },
	{ }
};

static const struct udevice_id mt8512_topckgen_cg_compat[] = {
	{ .compatible = "mediatek,mt8512-topckgen-cg", },
	{ }
};

static const struct udevice_id mt8512_infracfg_compat[] = {
	{ .compatible = "mediatek,mt8512-infracfg", },
	{ }
};

U_BOOT_DRIVER(mtk_clk_apmixedsys) = {
	.name = "mt8512-apmixedsys",
	.id = UCLASS_CLK,
	.of_match = mt8512_apmixed_compat,
	.probe = mt8512_apmixedsys_probe,
	.priv_auto	= sizeof(struct mtk_clk_priv),
	.ops = &mtk_clk_apmixedsys_ops,
	.flags = DM_FLAG_PRE_RELOC,
};

U_BOOT_DRIVER(mtk_clk_topckgen) = {
	.name = "mt8512-topckgen",
	.id = UCLASS_CLK,
	.of_match = mt8512_topckgen_compat,
	.probe = mt8512_topckgen_probe,
	.priv_auto	= sizeof(struct mtk_clk_priv),
	.ops = &mtk_clk_topckgen_ops,
	.flags = DM_FLAG_PRE_RELOC,
};

U_BOOT_DRIVER(mtk_clk_topckgen_cg) = {
	.name = "mt8512-topckgen-cg",
	.id = UCLASS_CLK,
	.of_match = mt8512_topckgen_cg_compat,
	.probe = mt8512_topckgen_cg_probe,
	.priv_auto	= sizeof(struct mtk_cg_priv),
	.ops = &mtk_clk_gate_ops,
	.flags = DM_FLAG_PRE_RELOC,
};

U_BOOT_DRIVER(mtk_clk_infracfg) = {
	.name = "mt8512-infracfg",
	.id = UCLASS_CLK,
	.of_match = mt8512_infracfg_compat,
	.probe = mt8512_infracfg_probe,
	.priv_auto	= sizeof(struct mtk_cg_priv),
	.ops = &mtk_clk_gate_ops,
	.flags = DM_FLAG_PRE_RELOC,
};
