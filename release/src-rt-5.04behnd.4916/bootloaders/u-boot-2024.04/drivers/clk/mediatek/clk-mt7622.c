// SPDX-License-Identifier: GPL-2.0
/*
 * MediaTek clock driver for MT7622 SoC
 *
 * Copyright (C) 2019 MediaTek Inc.
 * Author: Ryder Lee <ryder.lee@mediatek.com>
 */

#include <common.h>
#include <dm.h>
#include <log.h>
#include <asm/arch-mediatek/reset.h>
#include <asm/io.h>
#include <dt-bindings/clock/mt7622-clk.h>
#include <linux/bitops.h>

#include "clk-mtk.h"

#define MT7622_CLKSQ_STB_CON0		0x20
#define MT7622_PLL_ISO_CON0		0x2c
#define MT7622_PLL_FMAX			(2500UL * MHZ)
#define MT7622_CON0_RST_BAR		BIT(24)

#define MCU_AXI_DIV			0x640
#define AXI_DIV_MSK			GENMASK(4, 0)
#define AXI_DIV_SEL(x)			(x)

#define MCU_BUS_MUX			0x7c0
#define MCU_BUS_MSK			GENMASK(10, 9)
#define MCU_BUS_SEL(x)			((x) << 9)

/* apmixedsys */
#define PLL(_id, _reg, _pwr_reg, _en_mask, _flags, _pcwbits, _pd_reg,	\
	    _pd_shift, _pcw_reg, _pcw_shift) {				\
		.id = _id,						\
		.reg = _reg,						\
		.pwr_reg = _pwr_reg,					\
		.en_mask = _en_mask,					\
		.rst_bar_mask = MT7622_CON0_RST_BAR,			\
		.fmax = MT7622_PLL_FMAX,				\
		.flags = _flags,					\
		.pcwbits = _pcwbits,					\
		.pd_reg = _pd_reg,					\
		.pd_shift = _pd_shift,					\
		.pcw_reg = _pcw_reg,					\
		.pcw_shift = _pcw_shift,				\
	}

static const struct mtk_pll_data apmixed_plls[] = {
	PLL(CLK_APMIXED_ARMPLL, 0x200, 0x20c, 0x1, 0,
	    21, 0x204, 24, 0x204, 0),
	PLL(CLK_APMIXED_MAINPLL, 0x210, 0x21c, 0x1, HAVE_RST_BAR,
	    21, 0x214, 24, 0x214, 0),
	PLL(CLK_APMIXED_UNIV2PLL, 0x220, 0x22c, 0x1, HAVE_RST_BAR,
	    7, 0x224, 24, 0x224, 14),
	PLL(CLK_APMIXED_ETH1PLL, 0x300, 0x310, 0x1, 0,
	    21, 0x300, 1, 0x304, 0),
	PLL(CLK_APMIXED_ETH2PLL, 0x314, 0x320, 0x1, 0,
	    21, 0x314, 1, 0x318, 0),
	PLL(CLK_APMIXED_AUD1PLL, 0x324, 0x330, 0x1, 0,
	    31, 0x324, 1, 0x328, 0),
	PLL(CLK_APMIXED_AUD2PLL, 0x334, 0x340, 0x1, 0,
	    31, 0x334, 1, 0x338, 0),
	PLL(CLK_APMIXED_TRGPLL, 0x344, 0x354, 0x1, 0,
	    21, 0x344, 1, 0x348, 0),
	PLL(CLK_APMIXED_SGMIPLL, 0x358, 0x368, 0x1, 0,
	    21, 0x358, 1, 0x35c, 0),
};

/* topckgen */
#define FACTOR0(_id, _parent, _mult, _div)			\
	FACTOR(_id, _parent, _mult, _div, CLK_PARENT_APMIXED)

#define FACTOR1(_id, _parent, _mult, _div)			\
	FACTOR(_id, _parent, _mult, _div, CLK_PARENT_TOPCKGEN)

#define FACTOR2(_id, _parent, _mult, _div)			\
	FACTOR(_id, _parent, _mult, _div, 0)

static const struct mtk_fixed_clk top_fixed_clks[] = {
	FIXED_CLK(CLK_TOP_TO_U2_PHY, CLK_XTAL, 31250000),
	FIXED_CLK(CLK_TOP_TO_U2_PHY_1P, CLK_XTAL, 31250000),
	FIXED_CLK(CLK_TOP_PCIE0_PIPE_EN, CLK_XTAL, 125000000),
	FIXED_CLK(CLK_TOP_PCIE1_PIPE_EN, CLK_XTAL, 125000000),
	FIXED_CLK(CLK_TOP_SSUSB_TX250M, CLK_XTAL, 250000000),
	FIXED_CLK(CLK_TOP_SSUSB_EQ_RX250M, CLK_XTAL, 250000000),
	FIXED_CLK(CLK_TOP_SSUSB_CDR_REF, CLK_XTAL, 33333333),
	FIXED_CLK(CLK_TOP_SSUSB_CDR_FB, CLK_XTAL, 50000000),
	FIXED_CLK(CLK_TOP_SATA_ASIC, CLK_XTAL, 50000000),
	FIXED_CLK(CLK_TOP_SATA_RBC, CLK_XTAL, 50000000),
};

static const struct mtk_fixed_factor top_fixed_divs[] = {
	FACTOR0(CLK_TOP_TO_USB3_SYS, CLK_APMIXED_ETH1PLL, 1, 4),
	FACTOR0(CLK_TOP_P1_1MHZ, CLK_APMIXED_ETH1PLL, 1, 500),
	FACTOR0(CLK_TOP_4MHZ, CLK_APMIXED_ETH1PLL, 1, 125),
	FACTOR0(CLK_TOP_P0_1MHZ, CLK_APMIXED_ETH1PLL, 1, 500),
	FACTOR1(CLK_TOP_TXCLK_SRC_PRE, CLK_TOP_SGMIIPLL_D2, 1, 1),
	FACTOR2(CLK_TOP_RTC, CLK_XTAL, 1, 1024),
	FACTOR2(CLK_TOP_MEMPLL, CLK_XTAL, 32, 1),
	FACTOR1(CLK_TOP_DMPLL, CLK_TOP_MEMPLL, 1, 1),
	FACTOR0(CLK_TOP_SYSPLL_D2, CLK_APMIXED_MAINPLL, 1, 2),
	FACTOR0(CLK_TOP_SYSPLL1_D2, CLK_APMIXED_MAINPLL, 1, 4),
	FACTOR0(CLK_TOP_SYSPLL1_D4, CLK_APMIXED_MAINPLL, 1, 8),
	FACTOR0(CLK_TOP_SYSPLL1_D8, CLK_APMIXED_MAINPLL, 1, 16),
	FACTOR0(CLK_TOP_SYSPLL2_D4, CLK_APMIXED_MAINPLL, 1, 12),
	FACTOR0(CLK_TOP_SYSPLL2_D8, CLK_APMIXED_MAINPLL, 1, 24),
	FACTOR0(CLK_TOP_SYSPLL_D5, CLK_APMIXED_MAINPLL, 1, 5),
	FACTOR0(CLK_TOP_SYSPLL3_D2, CLK_APMIXED_MAINPLL, 1, 10),
	FACTOR0(CLK_TOP_SYSPLL3_D4, CLK_APMIXED_MAINPLL, 1, 20),
	FACTOR0(CLK_TOP_SYSPLL4_D2, CLK_APMIXED_MAINPLL, 1, 14),
	FACTOR0(CLK_TOP_SYSPLL4_D4, CLK_APMIXED_MAINPLL, 1, 28),
	FACTOR0(CLK_TOP_SYSPLL4_D16, CLK_APMIXED_MAINPLL, 1, 112),
	FACTOR0(CLK_TOP_UNIVPLL, CLK_APMIXED_UNIV2PLL, 1, 2),
	FACTOR0(CLK_TOP_UNIVPLL_D2, CLK_TOP_UNIVPLL, 1, 2),
	FACTOR1(CLK_TOP_UNIVPLL1_D2, CLK_TOP_UNIVPLL, 1, 4),
	FACTOR1(CLK_TOP_UNIVPLL1_D4, CLK_TOP_UNIVPLL, 1, 8),
	FACTOR1(CLK_TOP_UNIVPLL1_D8, CLK_TOP_UNIVPLL, 1, 16),
	FACTOR1(CLK_TOP_UNIVPLL1_D16, CLK_TOP_UNIVPLL, 1, 32),
	FACTOR1(CLK_TOP_UNIVPLL2_D2, CLK_TOP_UNIVPLL, 1, 6),
	FACTOR1(CLK_TOP_UNIVPLL2_D4, CLK_TOP_UNIVPLL, 1, 12),
	FACTOR1(CLK_TOP_UNIVPLL2_D8, CLK_TOP_UNIVPLL, 1, 24),
	FACTOR1(CLK_TOP_UNIVPLL2_D16, CLK_TOP_UNIVPLL, 1, 48),
	FACTOR1(CLK_TOP_UNIVPLL_D5, CLK_TOP_UNIVPLL, 1, 5),
	FACTOR1(CLK_TOP_UNIVPLL3_D2, CLK_TOP_UNIVPLL, 1, 10),
	FACTOR1(CLK_TOP_UNIVPLL3_D4, CLK_TOP_UNIVPLL, 1, 20),
	FACTOR1(CLK_TOP_UNIVPLL3_D16, CLK_TOP_UNIVPLL, 1, 80),
	FACTOR1(CLK_TOP_UNIVPLL_D7, CLK_TOP_UNIVPLL, 1, 7),
	FACTOR1(CLK_TOP_UNIVPLL_D80_D4, CLK_TOP_UNIVPLL, 1, 320),
	FACTOR1(CLK_TOP_UNIV48M, CLK_TOP_UNIVPLL, 1, 25),
	FACTOR0(CLK_TOP_SGMIIPLL, CLK_APMIXED_SGMIPLL, 1, 1),
	FACTOR0(CLK_TOP_SGMIIPLL_D2, CLK_APMIXED_SGMIPLL, 1, 2),
	FACTOR0(CLK_TOP_AUD1PLL, CLK_APMIXED_AUD1PLL, 1, 1),
	FACTOR0(CLK_TOP_AUD2PLL, CLK_APMIXED_AUD2PLL, 1, 1),
	FACTOR1(CLK_TOP_AUD_I2S2_MCK, CLK_TOP_I2S2_MCK_SEL, 1, 2),
	FACTOR1(CLK_TOP_TO_USB3_REF, CLK_TOP_UNIVPLL2_D4, 1, 4),
	FACTOR1(CLK_TOP_PCIE1_MAC_EN, CLK_TOP_UNIVPLL1_D4, 1, 1),
	FACTOR1(CLK_TOP_PCIE0_MAC_EN, CLK_TOP_UNIVPLL1_D4, 1, 1),
	FACTOR0(CLK_TOP_ETH_500M, CLK_APMIXED_ETH1PLL, 1, 1),
};

static const int axi_parents[] = {
	CLK_XTAL,
	CLK_TOP_SYSPLL1_D2,
	CLK_TOP_SYSPLL_D5,
	CLK_TOP_SYSPLL1_D4,
	CLK_TOP_UNIVPLL_D5,
	CLK_TOP_UNIVPLL2_D2,
	CLK_TOP_UNIVPLL_D7
};

static const int mem_parents[] = {
	CLK_XTAL,
	CLK_TOP_DMPLL
};

static const int ddrphycfg_parents[] = {
	CLK_XTAL,
	CLK_TOP_SYSPLL1_D8
};

static const int eth_parents[] = {
	CLK_XTAL,
	CLK_TOP_SYSPLL1_D2,
	CLK_TOP_UNIVPLL1_D2,
	CLK_TOP_SYSPLL1_D4,
	CLK_TOP_UNIVPLL_D5,
	-1,
	CLK_TOP_UNIVPLL_D7
};

static const int pwm_parents[] = {
	CLK_XTAL,
	CLK_TOP_UNIVPLL2_D4
};

static const int f10m_ref_parents[] = {
	CLK_XTAL,
	CLK_TOP_SYSPLL4_D16
};

static const int nfi_infra_parents[] = {
	CLK_XTAL,
	CLK_XTAL,
	CLK_XTAL,
	CLK_XTAL,
	CLK_XTAL,
	CLK_XTAL,
	CLK_XTAL,
	CLK_XTAL,
	CLK_TOP_UNIVPLL2_D8,
	CLK_TOP_SYSPLL1_D8,
	CLK_TOP_UNIVPLL1_D8,
	CLK_TOP_SYSPLL4_D2,
	CLK_TOP_UNIVPLL2_D4,
	CLK_TOP_UNIVPLL3_D2,
	CLK_TOP_SYSPLL1_D4
};

static const int flash_parents[] = {
	CLK_XTAL,
	CLK_TOP_UNIVPLL_D80_D4,
	CLK_TOP_SYSPLL2_D8,
	CLK_TOP_SYSPLL3_D4,
	CLK_TOP_UNIVPLL3_D4,
	CLK_TOP_UNIVPLL1_D8,
	CLK_TOP_SYSPLL2_D4,
	CLK_TOP_UNIVPLL2_D4
};

static const int uart_parents[] = {
	CLK_XTAL,
	CLK_TOP_UNIVPLL2_D8
};

static const int spi0_parents[] = {
	CLK_XTAL,
	CLK_TOP_SYSPLL3_D2,
	CLK_XTAL,
	CLK_TOP_SYSPLL2_D4,
	CLK_TOP_SYSPLL4_D2,
	CLK_TOP_UNIVPLL2_D4,
	CLK_TOP_UNIVPLL1_D8,
	CLK_XTAL
};

static const int spi1_parents[] = {
	CLK_XTAL,
	CLK_TOP_SYSPLL3_D2,
	CLK_XTAL,
	CLK_TOP_SYSPLL4_D4,
	CLK_TOP_SYSPLL4_D2,
	CLK_TOP_UNIVPLL2_D4,
	CLK_TOP_UNIVPLL1_D8,
	CLK_XTAL
};

static const int msdc30_0_parents[] = {
	CLK_XTAL,
	CLK_TOP_UNIVPLL2_D16,
	CLK_TOP_UNIV48M
};

static const int a1sys_hp_parents[] = {
	CLK_XTAL,
	CLK_TOP_AUD1PLL,
	CLK_TOP_AUD2PLL,
	CLK_XTAL
};

static const int intdir_parents[] = {
	CLK_XTAL,
	CLK_TOP_SYSPLL1_D2,
	CLK_TOP_UNIVPLL_D2,
	CLK_TOP_SGMIIPLL
};

static const int aud_intbus_parents[] = {
	CLK_XTAL,
	CLK_TOP_SYSPLL1_D4,
	CLK_TOP_SYSPLL4_D2,
	CLK_TOP_SYSPLL3_D2
};

static const int pmicspi_parents[] = {
	CLK_XTAL,
	-1,
	-1,
	-1,
	-1,
	CLK_TOP_UNIVPLL2_D16
};

static const int atb_parents[] = {
	CLK_XTAL,
	CLK_TOP_SYSPLL1_D2,
	CLK_TOP_SYSPLL_D5
};

static const int audio_parents[] = {
	CLK_XTAL,
	CLK_TOP_SYSPLL3_D4,
	CLK_TOP_SYSPLL4_D4,
	CLK_TOP_UNIVPLL1_D16
};

static const int usb20_parents[] = {
	CLK_XTAL,
	CLK_TOP_UNIVPLL3_D4,
	CLK_TOP_SYSPLL1_D8,
	CLK_XTAL
};

static const int aud1_parents[] = {
	CLK_XTAL,
	CLK_TOP_AUD1PLL
};

static const int asm_l_parents[] = {
	CLK_XTAL,
	CLK_TOP_SYSPLL_D5,
	CLK_TOP_UNIVPLL2_D2,
	CLK_TOP_UNIVPLL2_D4
};

static const int apll1_ck_parents[] = {
	CLK_TOP_AUD1_SEL,
	CLK_TOP_AUD2_SEL
};

static const struct mtk_composite top_muxes[] = {
	/* CLK_CFG_0 */
	MUX_GATE(CLK_TOP_AXI_SEL, axi_parents, 0x40, 0, 3, 7),
	MUX_GATE(CLK_TOP_MEM_SEL, mem_parents, 0x40, 8, 1, 15),
	MUX_GATE(CLK_TOP_DDRPHYCFG_SEL, ddrphycfg_parents, 0x40, 16, 1, 23),
	MUX_GATE(CLK_TOP_ETH_SEL, eth_parents, 0x40, 24, 3, 31),

	/* CLK_CFG_1 */
	MUX_GATE(CLK_TOP_PWM_SEL, pwm_parents, 0x50, 0, 2, 7),
	MUX_GATE(CLK_TOP_F10M_REF_SEL, f10m_ref_parents, 0x50, 8, 1, 15),
	MUX_GATE(CLK_TOP_NFI_INFRA_SEL, nfi_infra_parents, 0x50, 16, 4, 23),
	MUX_GATE(CLK_TOP_FLASH_SEL, flash_parents, 0x50, 24, 3, 31),

	/* CLK_CFG_2 */
	MUX_GATE(CLK_TOP_UART_SEL, uart_parents, 0x60, 0, 1, 7),
	MUX_GATE(CLK_TOP_SPI0_SEL, spi0_parents, 0x60, 8, 3, 15),
	MUX_GATE(CLK_TOP_SPI1_SEL, spi1_parents, 0x60, 16, 3, 23),
	MUX_GATE(CLK_TOP_MSDC50_0_SEL, uart_parents, 0x60, 24, 3, 31),

	/* CLK_CFG_3 */
	MUX_GATE(CLK_TOP_MSDC30_0_SEL, msdc30_0_parents, 0x70, 0, 3, 7),
	MUX_GATE(CLK_TOP_MSDC30_1_SEL, msdc30_0_parents, 0x70, 8, 3, 15),
	MUX_GATE(CLK_TOP_A1SYS_HP_SEL, a1sys_hp_parents, 0x70, 16, 3, 23),
	MUX_GATE(CLK_TOP_A2SYS_HP_SEL, a1sys_hp_parents, 0x70, 24, 3, 31),

	/* CLK_CFG_4 */
	MUX_GATE(CLK_TOP_INTDIR_SEL, intdir_parents, 0x80, 0, 2, 7),
	MUX_GATE(CLK_TOP_AUD_INTBUS_SEL, aud_intbus_parents, 0x80, 8, 2, 15),
	MUX_GATE(CLK_TOP_PMICSPI_SEL, pmicspi_parents, 0x80, 16, 3, 23),
	MUX_GATE(CLK_TOP_SCP_SEL, ddrphycfg_parents, 0x80, 24, 2, 31),

	/* CLK_CFG_5 */
	MUX_GATE(CLK_TOP_ATB_SEL, atb_parents, 0x90, 0, 2, 7),
	MUX_GATE_FLAGS(CLK_TOP_HIF_SEL, eth_parents, 0x90, 8, 3, 15,
		       CLK_DOMAIN_SCPSYS),
	MUX_GATE(CLK_TOP_AUDIO_SEL, audio_parents, 0x90, 16, 2, 23),
	MUX_GATE(CLK_TOP_U2_SEL, usb20_parents, 0x90, 24, 2, 31),

	/* CLK_CFG_6 */
	MUX_GATE(CLK_TOP_AUD1_SEL, aud1_parents, 0xA0, 0, 1, 7),
	MUX_GATE(CLK_TOP_AUD2_SEL, aud1_parents, 0xA0, 8, 1, 15),
	MUX_GATE(CLK_TOP_IRRX_SEL, f10m_ref_parents, 0xA0, 16, 1, 23),
	MUX_GATE(CLK_TOP_IRTX_SEL, f10m_ref_parents, 0xA0, 24, 1, 31),

	/* CLK_CFG_7 */
	MUX_GATE(CLK_TOP_ASM_L_SEL, asm_l_parents, 0xB0, 0, 2, 7),
	MUX_GATE(CLK_TOP_ASM_M_SEL, asm_l_parents, 0xB0, 8, 2, 15),
	MUX_GATE(CLK_TOP_ASM_H_SEL, asm_l_parents, 0xB0, 16, 2, 23),

	/* CLK_AUDDIV_0 */
	MUX(CLK_TOP_APLL1_SEL, apll1_ck_parents, 0x120, 6, 1),
	MUX(CLK_TOP_APLL2_SEL, apll1_ck_parents, 0x120, 7, 1),
	MUX(CLK_TOP_I2S0_MCK_SEL, apll1_ck_parents, 0x120, 8, 1),
	MUX(CLK_TOP_I2S1_MCK_SEL, apll1_ck_parents, 0x120, 9, 1),
	MUX(CLK_TOP_I2S2_MCK_SEL, apll1_ck_parents, 0x120, 10, 1),
	MUX(CLK_TOP_I2S3_MCK_SEL, apll1_ck_parents, 0x120, 161, 1),
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
	GATE_INFRA(CLK_INFRA_DBGCLK_PD, CLK_TOP_AXI_SEL, 0),
	GATE_INFRA(CLK_INFRA_TRNG, CLK_TOP_AXI_SEL, 2),
	GATE_INFRA(CLK_INFRA_AUDIO_PD, CLK_TOP_AUD_INTBUS_SEL, 5),
	GATE_INFRA(CLK_INFRA_IRRX_PD, CLK_TOP_IRRX_SEL, 16),
	GATE_INFRA(CLK_INFRA_APXGPT_PD, CLK_TOP_F10M_REF_SEL, 18),
	GATE_INFRA(CLK_INFRA_PMIC_PD, CLK_TOP_PMICSPI_SEL, 22),
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
	/* PERI0 */
	GATE_PERI0(CLK_PERI_THERM_PD, CLK_TOP_AXI_SEL, 1),
	GATE_PERI0(CLK_PERI_PWM1_PD, CLK_XTAL, 2),
	GATE_PERI0(CLK_PERI_PWM2_PD, CLK_XTAL, 3),
	GATE_PERI0(CLK_PERI_PWM3_PD, CLK_XTAL, 4),
	GATE_PERI0(CLK_PERI_PWM4_PD, CLK_XTAL, 5),
	GATE_PERI0(CLK_PERI_PWM5_PD, CLK_XTAL, 6),
	GATE_PERI0(CLK_PERI_PWM6_PD, CLK_XTAL, 7),
	GATE_PERI0(CLK_PERI_PWM7_PD, CLK_XTAL, 8),
	GATE_PERI0(CLK_PERI_PWM_PD, CLK_XTAL, 9),
	GATE_PERI0(CLK_PERI_AP_DMA_PD, CLK_TOP_AXI_SEL, 12),
	GATE_PERI0(CLK_PERI_MSDC30_0_PD, CLK_TOP_MSDC30_0_SEL, 13),
	GATE_PERI0(CLK_PERI_MSDC30_1_PD, CLK_TOP_MSDC30_1_SEL, 14),
	GATE_PERI0(CLK_PERI_UART0_PD, CLK_TOP_AXI_SEL, 17),
	GATE_PERI0(CLK_PERI_UART1_PD, CLK_TOP_AXI_SEL, 18),
	GATE_PERI0(CLK_PERI_UART2_PD, CLK_TOP_AXI_SEL, 19),
	GATE_PERI0(CLK_PERI_UART3_PD, CLK_TOP_AXI_SEL, 20),
	GATE_PERI0(CLK_PERI_BTIF_PD, CLK_TOP_AXI_SEL, 22),
	GATE_PERI0(CLK_PERI_I2C0_PD, CLK_TOP_AXI_SEL, 23),
	GATE_PERI0(CLK_PERI_I2C1_PD, CLK_TOP_AXI_SEL, 24),
	GATE_PERI0(CLK_PERI_I2C2_PD, CLK_TOP_AXI_SEL, 25),
	GATE_PERI0(CLK_PERI_SPI1_PD, CLK_TOP_SPI1_SEL, 26),
	GATE_PERI0(CLK_PERI_AUXADC_PD, CLK_XTAL, 27),
	GATE_PERI0(CLK_PERI_SPI0_PD, CLK_TOP_SPI0_SEL, 28),
	GATE_PERI0(CLK_PERI_SNFI_PD, CLK_TOP_NFI_INFRA_SEL, 29),
	GATE_PERI0(CLK_PERI_NFI_PD, CLK_TOP_AXI_SEL, 30),
	GATE_PERI1(CLK_PERI_NFIECC_PD, CLK_TOP_AXI_SEL, 31),

	/* PERI1 */
	GATE_PERI1(CLK_PERI_FLASH_PD, CLK_TOP_FLASH_SEL, 1),
	GATE_PERI1(CLK_PERI_IRTX_PD, CLK_TOP_IRTX_SEL, 2),
};

/* pciesys */
static const struct mtk_gate_regs pcie_cg_regs = {
	.set_ofs = 0x30,
	.clr_ofs = 0x30,
	.sta_ofs = 0x30,
};

#define GATE_PCIE(_id, _parent, _shift) {		\
		.id = _id,				\
		.parent = _parent,			\
		.regs = &pcie_cg_regs,			\
		.shift = _shift,			\
		.flags = CLK_GATE_NO_SETCLR_INV | CLK_PARENT_TOPCKGEN,	\
	}

static const struct mtk_gate pcie_cgs[] = {
	GATE_PCIE(CLK_PCIE_P1_AUX_EN, CLK_TOP_P1_1MHZ, 12),
	GATE_PCIE(CLK_PCIE_P1_OBFF_EN, CLK_TOP_4MHZ, 13),
	GATE_PCIE(CLK_PCIE_P1_AHB_EN, CLK_TOP_AXI_SEL, 14),
	GATE_PCIE(CLK_PCIE_P1_AXI_EN, CLK_TOP_HIF_SEL, 15),
	GATE_PCIE(CLK_PCIE_P1_MAC_EN, CLK_TOP_PCIE1_MAC_EN, 16),
	GATE_PCIE(CLK_PCIE_P1_PIPE_EN, CLK_TOP_PCIE1_PIPE_EN, 17),
	GATE_PCIE(CLK_PCIE_P0_AUX_EN, CLK_TOP_P0_1MHZ, 18),
	GATE_PCIE(CLK_PCIE_P0_OBFF_EN, CLK_TOP_4MHZ, 19),
	GATE_PCIE(CLK_PCIE_P0_AHB_EN, CLK_TOP_AXI_SEL, 20),
	GATE_PCIE(CLK_PCIE_P0_AXI_EN, CLK_TOP_HIF_SEL, 21),
	GATE_PCIE(CLK_PCIE_P0_MAC_EN, CLK_TOP_PCIE0_MAC_EN, 22),
	GATE_PCIE(CLK_PCIE_P0_PIPE_EN, CLK_TOP_PCIE0_PIPE_EN, 23),
	GATE_PCIE(CLK_SATA_AHB_EN, CLK_TOP_AXI_SEL, 26),
	GATE_PCIE(CLK_SATA_AXI_EN, CLK_TOP_HIF_SEL, 27),
	GATE_PCIE(CLK_SATA_ASIC_EN, CLK_TOP_SATA_ASIC, 28),
	GATE_PCIE(CLK_SATA_RBC_EN, CLK_TOP_SATA_RBC, 29),
	GATE_PCIE(CLK_SATA_PM_EN, CLK_TOP_UNIVPLL2_D4, 30),
};

/* ethsys */
static const struct mtk_gate_regs eth_cg_regs = {
	.sta_ofs = 0x30,
};

#define GATE_ETH(_id, _parent, _shift) {			\
		.id = _id,					\
		.parent = _parent,				\
		.regs = &eth_cg_regs,				\
		.shift = _shift,				\
		.flags = CLK_GATE_NO_SETCLR_INV | CLK_PARENT_TOPCKGEN,	\
	}

static const struct mtk_gate eth_cgs[] = {
	GATE_ETH(CLK_ETH_HSDMA_EN, CLK_TOP_ETH_SEL, 5),
	GATE_ETH(CLK_ETH_ESW_EN, CLK_TOP_ETH_500M, 6),
	GATE_ETH(CLK_ETH_GP2_EN, CLK_TOP_TXCLK_SRC_PRE, 7),
	GATE_ETH(CLK_ETH_GP1_EN, CLK_TOP_TXCLK_SRC_PRE, 8),
	GATE_ETH(CLK_ETH_GP0_EN, CLK_TOP_TXCLK_SRC_PRE, 9),
};

static const struct mtk_gate_regs sgmii_cg_regs = {
	.sta_ofs = 0xE4,
};

#define GATE_SGMII(_id, _parent, _shift) {			\
	.id = _id,						\
	.parent = _parent,					\
	.regs = &sgmii_cg_regs,					\
	.shift = _shift,					\
	.flags = CLK_GATE_NO_SETCLR_INV | CLK_PARENT_TOPCKGEN,	\
}

static const struct mtk_gate_regs ssusb_cg_regs = {
	.set_ofs = 0x30,
	.clr_ofs = 0x30,
	.sta_ofs = 0x30,
};

#define GATE_SSUSB(_id, _parent, _shift) {                      \
	.id = _id,                                              \
	.parent = _parent,                                      \
	.regs = &ssusb_cg_regs,                                 \
	.shift = _shift,                                        \
	.flags = CLK_GATE_NO_SETCLR_INV | CLK_PARENT_TOPCKGEN,  \
}

static const struct mtk_gate sgmii_cgs[] = {
	GATE_SGMII(CLK_SGMII_TX250M_EN, CLK_TOP_SSUSB_TX250M, 2),
	GATE_SGMII(CLK_SGMII_RX250M_EN, CLK_TOP_SSUSB_EQ_RX250M, 3),
	GATE_SGMII(CLK_SGMII_CDR_REF, CLK_TOP_SSUSB_CDR_REF, 4),
	GATE_SGMII(CLK_SGMII_CDR_FB, CLK_TOP_SSUSB_CDR_FB, 5),
};

static const struct mtk_gate ssusb_cgs[] = {
	GATE_SSUSB(CLK_SSUSB_U2_PHY_1P_EN, CLK_TOP_TO_U2_PHY_1P, 0),
	GATE_SSUSB(CLK_SSUSB_U2_PHY_EN, CLK_TOP_TO_U2_PHY, 1),
	GATE_SSUSB(CLK_SSUSB_REF_EN, CLK_TOP_TO_USB3_REF, 5),
	GATE_SSUSB(CLK_SSUSB_SYS_EN, CLK_TOP_TO_USB3_SYS, 6),
	GATE_SSUSB(CLK_SSUSB_MCU_EN, CLK_TOP_AXI_SEL, 7),
	GATE_SSUSB(CLK_SSUSB_DMA_EN, CLK_TOP_HIF_SEL, 8),
};

static const struct mtk_clk_tree mt7622_clk_tree = {
	.xtal_rate = 25 * MHZ,
	.xtal2_rate = 25 * MHZ,
	.fdivs_offs = CLK_TOP_TO_USB3_SYS,
	.muxes_offs = CLK_TOP_AXI_SEL,
	.plls = apmixed_plls,
	.fclks = top_fixed_clks,
	.fdivs = top_fixed_divs,
	.muxes = top_muxes,
};

static int mt7622_mcucfg_probe(struct udevice *dev)
{
	void __iomem *base;

	base = dev_read_addr_ptr(dev);
	if (!base)
		return -ENOENT;

	clrsetbits_le32(base + MCU_AXI_DIV, AXI_DIV_MSK,
			AXI_DIV_SEL(0x12));
	clrsetbits_le32(base + MCU_BUS_MUX, MCU_BUS_MSK,
			MCU_BUS_SEL(0x1));

	return 0;
}

static int mt7622_apmixedsys_probe(struct udevice *dev)
{
	struct mtk_clk_priv *priv = dev_get_priv(dev);
	int ret;

	ret = mtk_common_clk_init(dev, &mt7622_clk_tree);
	if (ret)
		return ret;

	/* reduce clock square disable time */
	// writel(0x501, priv->base + MT7622_CLKSQ_STB_CON0);
	writel(0x98940501, priv->base + MT7622_CLKSQ_STB_CON0);

	/* extend pwr/iso control timing to 1us */
	writel(0x80008, priv->base + MT7622_PLL_ISO_CON0);

	return 0;
}

static int mt7622_topckgen_probe(struct udevice *dev)
{
	return mtk_common_clk_init(dev, &mt7622_clk_tree);
}

static int mt7622_infracfg_probe(struct udevice *dev)
{
	return mtk_common_clk_gate_init(dev, &mt7622_clk_tree, infra_cgs);
}

static int mt7622_pericfg_probe(struct udevice *dev)
{
	return mtk_common_clk_gate_init(dev, &mt7622_clk_tree, peri_cgs);
}

static int mt7622_pciesys_probe(struct udevice *dev)
{
	return mtk_common_clk_gate_init(dev, &mt7622_clk_tree, pcie_cgs);
}

static int mt7622_pciesys_bind(struct udevice *dev)
{
	int ret = 0;

	if (IS_ENABLED(CONFIG_RESET_MEDIATEK)) {
	ret = mediatek_reset_bind(dev, ETHSYS_HIFSYS_RST_CTRL_OFS, 1);
	if (ret)
		debug("Warning: failed to bind reset controller\n");
	}

	return ret;
}

static int mt7622_ethsys_probe(struct udevice *dev)
{
	return mtk_common_clk_gate_init(dev, &mt7622_clk_tree, eth_cgs);
}

static int mt7622_ethsys_bind(struct udevice *dev)
{
	int ret = 0;

#if CONFIG_IS_ENABLED(RESET_MEDIATEK)
	ret = mediatek_reset_bind(dev, ETHSYS_HIFSYS_RST_CTRL_OFS, 1);
	if (ret)
		debug("Warning: failed to bind reset controller\n");
#endif

	return ret;
}

static int mt7622_sgmiisys_probe(struct udevice *dev)
{
	return mtk_common_clk_gate_init(dev, &mt7622_clk_tree, sgmii_cgs);
}

static int mt7622_ssusbsys_probe(struct udevice *dev)
{
	return mtk_common_clk_gate_init(dev, &mt7622_clk_tree, ssusb_cgs);
}

static const struct udevice_id mt7622_apmixed_compat[] = {
	{ .compatible = "mediatek,mt7622-apmixedsys" },
	{ }
};

static const struct udevice_id mt7622_topckgen_compat[] = {
	{ .compatible = "mediatek,mt7622-topckgen" },
	{ }
};

static const struct udevice_id mt7622_infracfg_compat[] = {
	{ .compatible = "mediatek,mt7622-infracfg", },
	{ }
};

static const struct udevice_id mt7622_pericfg_compat[] = {
	{ .compatible = "mediatek,mt7622-pericfg", },
	{ }
};

static const struct udevice_id mt7622_pciesys_compat[] = {
	{ .compatible = "mediatek,mt7622-pciesys", },
	{ }
};

static const struct udevice_id mt7622_ethsys_compat[] = {
	{ .compatible = "mediatek,mt7622-ethsys", },
	{ }
};

static const struct udevice_id mt7622_sgmiisys_compat[] = {
	{ .compatible = "mediatek,mt7622-sgmiisys", },
	{ }
};

static const struct udevice_id mt7622_mcucfg_compat[] = {
	{ .compatible = "mediatek,mt7622-mcucfg" },
	{ }
};

static const struct udevice_id mt7622_ssusbsys_compat[] = {
	{ .compatible = "mediatek,mt7622-ssusbsys" },
	{ }
};

U_BOOT_DRIVER(mtk_mcucfg) = {
	.name = "mt7622-mcucfg",
	.id = UCLASS_SYSCON,
	.of_match = mt7622_mcucfg_compat,
	.probe = mt7622_mcucfg_probe,
	.flags = DM_FLAG_PRE_RELOC,
};

U_BOOT_DRIVER(mtk_clk_apmixedsys) = {
	.name = "mt7622-clock-apmixedsys",
	.id = UCLASS_CLK,
	.of_match = mt7622_apmixed_compat,
	.probe = mt7622_apmixedsys_probe,
	.priv_auto	= sizeof(struct mtk_clk_priv),
	.ops = &mtk_clk_apmixedsys_ops,
	.flags = DM_FLAG_PRE_RELOC,
};

U_BOOT_DRIVER(mtk_clk_topckgen) = {
	.name = "mt7622-clock-topckgen",
	.id = UCLASS_CLK,
	.of_match = mt7622_topckgen_compat,
	.probe = mt7622_topckgen_probe,
	.priv_auto	= sizeof(struct mtk_clk_priv),
	.ops = &mtk_clk_topckgen_ops,
	.flags = DM_FLAG_PRE_RELOC,
};

U_BOOT_DRIVER(mtk_clk_infracfg) = {
	.name = "mt7622-clock-infracfg",
	.id = UCLASS_CLK,
	.of_match = mt7622_infracfg_compat,
	.probe = mt7622_infracfg_probe,
	.priv_auto	= sizeof(struct mtk_cg_priv),
	.ops = &mtk_clk_gate_ops,
	.flags = DM_FLAG_PRE_RELOC,
};

U_BOOT_DRIVER(mtk_clk_pericfg) = {
	.name = "mt7622-clock-pericfg",
	.id = UCLASS_CLK,
	.of_match = mt7622_pericfg_compat,
	.probe = mt7622_pericfg_probe,
	.priv_auto	= sizeof(struct mtk_cg_priv),
	.ops = &mtk_clk_gate_ops,
	.flags = DM_FLAG_PRE_RELOC,
};

U_BOOT_DRIVER(mtk_clk_pciesys) = {
	.name = "mt7622-clock-pciesys",
	.id = UCLASS_CLK,
	.of_match = mt7622_pciesys_compat,
	.probe = mt7622_pciesys_probe,
	.bind = mt7622_pciesys_bind,
	.priv_auto	= sizeof(struct mtk_cg_priv),
	.ops = &mtk_clk_gate_ops,
};

U_BOOT_DRIVER(mtk_clk_ethsys) = {
	.name = "mt7622-clock-ethsys",
	.id = UCLASS_CLK,
	.of_match = mt7622_ethsys_compat,
	.probe = mt7622_ethsys_probe,
	.bind = mt7622_ethsys_bind,
	.priv_auto	= sizeof(struct mtk_cg_priv),
	.ops = &mtk_clk_gate_ops,
};

U_BOOT_DRIVER(mtk_clk_sgmiisys) = {
	.name = "mt7622-clock-sgmiisys",
	.id = UCLASS_CLK,
	.of_match = mt7622_sgmiisys_compat,
	.probe = mt7622_sgmiisys_probe,
	.priv_auto	= sizeof(struct mtk_cg_priv),
	.ops = &mtk_clk_gate_ops,
};

U_BOOT_DRIVER(mtk_clk_ssusbsys) = {
	.name = "mt7622-clock-ssusbsys",
	.id = UCLASS_CLK,
	.of_match = mt7622_ssusbsys_compat,
	.probe = mt7622_ssusbsys_probe,
	.priv_auto	= sizeof(struct mtk_cg_priv),
	.ops = &mtk_clk_gate_ops,
};
