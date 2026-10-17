/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * sun8i a83t clock register definitions
 *
 * (C) Copyright 2007-2011
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * Tom Cubie <tangliang@allwinnertech.com>
 *
 * (C) Copyright 2015 Vishnu Patekar <vishnupatekar0510@gmail.com>
 * from sun6i.h
 */

#ifndef _SUNXI_CLOCK_SUN8I_A83T_H
#define _SUNXI_CLOCK_SUN8I_A83T_H

struct sunxi_ccm_reg {
	u32 pll1_c0_cfg;	/* 0x00 c1cpu# pll control */
	u32 pll1_c1_cfg;	/* 0x04 c1cpu# pll control */
	u32 pll2_cfg;		/* 0x08 pll2 audio control */
	u32 reserved1;
	u32 pll3_cfg;		/* 0x10 pll3 video0 control */
	u32 reserved2;
	u32 pll4_cfg;		/* 0x18 pll4 ve control */
	u32 reserved3;
	u32 pll5_cfg;		/* 0x20 pll5 ddr control */
	u32 reserved4;
	u32 pll6_cfg;		/* 0x28 pll6 peripheral control */
	u32 reserved5[3];	/* 0x2c */
	u32 pll7_cfg;		/* 0x38 pll7 gpu control */
	u32 reserved6[2];	/* 0x3c */
	u32 pll8_cfg;		/* 0x44 pll8 hsic control */
	u32 pll9_cfg;		/* 0x48 pll9 de control */
	u32 pll10_cfg;		/* 0x4c pll10 video1 control */
	u32 cpu_axi_cfg;	/* 0x50 CPU/AXI divide ratio */
	u32 ahb1_apb1_div;	/* 0x54 AHB1/APB1 divide ratio */
	u32 apb2_div;		/* 0x58 APB2 divide ratio */
	u32 ahb2_div;		/* 0x5c AHB2 divide ratio */
	u32 ahb_gate0;		/* 0x60 ahb module clock gating 0 */
	u32 ahb_gate1;		/* 0x64 ahb module clock gating 1 */
	u32 apb1_gate;		/* 0x68 apb1 module clock gating 3 */
	u32 apb2_gate;		/* 0x6c apb2 module clock gating 4 */
	u32 reserved7[2];	/* 0x70 */
	u32 cci400_cfg;		/* 0x78 cci400 clock configuration A83T only */
	u32 reserved8;		/* 0x7c */
	u32 nand0_clk_cfg;	/* 0x80 nand clock control */
	u32 reserved9;		/* 0x84 */
	u32 sd0_clk_cfg;	/* 0x88 sd0 clock control */
	u32 sd1_clk_cfg;	/* 0x8c sd1 clock control */
	u32 sd2_clk_cfg;	/* 0x90 sd2 clock control */
	u32 sd3_clk_cfg;	/* 0x94 sd3 clock control */
	u32 reserved10;		/* 0x98 */
	u32 ss_clk_cfg;		/* 0x9c security system clock control */
	u32 spi0_clk_cfg;	/* 0xa0 spi0 clock control */
	u32 spi1_clk_cfg;	/* 0xa4 spi1 clock control */
	u32 reserved11[2];	/* 0xa8 */
	u32 i2s0_clk_cfg;	/* 0xb0 I2S0 clock control */
	u32 i2s1_clk_cfg;	/* 0xb4 I2S1 clock control */
	u32 i2s2_clk_cfg;	/* 0xb8 I2S2 clock control */
	u32 tdm_clk_cfg;	/* 0xbc TDM clock control */
	u32 spdif_clk_cfg;      /* 0xc0 SPDIF clock control */
	u32 reserved12[2];	/* 0xc4 */
	u32 usb_clk_cfg;	/* 0xcc USB clock control */
	u32 reserved13[9];	/* 0xd0 */
	u32 dram_clk_cfg;	/* 0xf4 DRAM configuration clock control */
	u32 dram_pll_cfg;	/* 0xf8 PLL_DDR cfg register */
	u32 mbus_reset;		/* 0xfc MBUS reset control */
	u32 dram_clk_gate;	/* 0x100 DRAM module gating */
	u32 reserved14[5];	/* 0x104 BE0 */
	u32 lcd0_clk_cfg;	/* 0x118 LCD0 module clock */
	u32 lcd1_clk_cfg;	/* 0x11c LCD1 module clock */
	u32 reserved15[4];	/* 0x120 */
	u32 mipi_csi_clk_cfg;	/* 0x130 MIPI CSI module clock */
	u32 csi_clk_cfg;	/* 0x134 CSI module clock */
	u32 reserved16;		/* 0x138 */
	u32 ve_clk_cfg;		/* 0x13c VE module clock */
	u32 reserved17;		/* 0x140 */
	u32 avs_clk_cfg;	/* 0x144 AVS module clock */
	u32 reserved18[2];	/* 0x148 */
	u32 hdmi_clk_cfg;	/* 0x150 HDMI module clock */
	u32 hdmi_slow_clk_cfg;	/* 0x154 HDMI slow module clock */
	u32 reserved19;		/* 0x158 */
	u32 mbus_clk_cfg;	/* 0x15c MBUS module clock */
	u32 reserved20[2];	/* 0x160 */
	u32 mipi_dsi_clk_cfg;	/* 0x168 MIPI DSI clock control */
	u32 reserved21[13];	/* 0x16c */
	u32 gpu_core_clk_cfg;	/* 0x1a0 GPU core clock config */
	u32 gpu_mem_clk_cfg;	/* 0x1a4 GPU memory clock config */
	u32 gpu_hyd_clk_cfg;	/* 0x1a8 GPU HYD clock config */
	u32 reserved22[21];	/* 0x1ac */
	u32 pll_stable0;	/* 0x200 PLL stable time 0 */
	u32 pll_stable1;	/* 0x204 PLL stable time 1 */
	u32 reserved23;		/* 0x208 */
	u32 pll_stable_status;	/* 0x20c PLL stable status register */
	u32 reserved24[4];	/* 0x210 */
	u32 pll1_c0_bias_cfg;	/* 0x220 PLL1 c0cpu# Bias config */
	u32 pll2_bias_cfg;	/* 0x224 PLL2 audio Bias config */
	u32 pll3_bias_cfg;	/* 0x228 PLL3 video Bias config */
	u32 pll4_bias_cfg;	/* 0x22c PLL4 ve Bias config */
	u32 pll5_bias_cfg;	/* 0x230 PLL5 ddr Bias config */
	u32 pll6_bias_cfg;	/* 0x234 PLL6 periph Bias config */
	u32 pll1_c1_bias_cfg;	/* 0x238 PLL1 c1cpu# Bias config */
	u32 pll8_bias_cfg;	/* 0x23c PLL7 Bias config */
	u32 reserved25;		/* 0x240 */
	u32 pll9_bias_cfg;	/* 0x244 PLL9 hsic Bias config */
	u32 de_bias_cfg;	/* 0x248 display engine Bias config */
	u32 video1_bias_cfg;	/* 0x24c pll video1 bias register */
	u32 c0_tuning_cfg;	/* 0x250 pll c0cpu# tuning register */
	u32 c1_tuning_cfg;	/* 0x254 pll c1cpu# tuning register */
	u32 reserved26[11];	/* 0x258 */
	u32 pll2_pattern_cfg0;	/* 0x284 PLL2 Pattern register 0 */
	u32 pll3_pattern_cfg0;	/* 0x288 PLL3 Pattern register 0 */
	u32 reserved27;		/* 0x28c */
	u32 pll5_pattern_cfg0;	/* 0x290 PLL5 Pattern register 0*/
	u32 reserved28[4];	/* 0x294 */
	u32 pll2_pattern_cfg1;	/* 0x2a4 PLL2 Pattern register 1 */
	u32 pll3_pattern_cfg1;	/* 0x2a8 PLL3 Pattern register 1 */
	u32 reserved29;		/* 0x2ac */
	u32 pll5_pattern_cfg1;	/* 0x2b0 PLL5 Pattern register 1 */
	u32 reserved30[3];	/* 0x2b4 */
	u32 ahb_reset0_cfg;	/* 0x2c0 AHB1 Reset 0 config */
	u32 ahb_reset1_cfg;	/* 0x2c4 AHB1 Reset 1 config */
	u32 ahb_reset2_cfg;	/* 0x2c8 AHB1 Reset 2 config */
	u32 reserved31;
	u32 ahb_reset3_cfg;	/* 0x2d0 AHB1 Reset 3 config */
	u32 reserved32;		/* 0x2d4 */
	u32 apb2_reset_cfg;	/* 0x2d8 BUS Reset 4 config */
};

/* apb2 bit field */
#define APB2_CLK_SRC_LOSC		(0x0 << 24)
#define APB2_CLK_SRC_OSC24M		(0x1 << 24)
#define APB2_CLK_SRC_PLL6		(0x2 << 24)
#define APB2_CLK_SRC_MASK		(0x3 << 24)
#define APB2_CLK_RATE_N_1		(0x0 << 16)
#define APB2_CLK_RATE_N_2		(0x1 << 16)
#define APB2_CLK_RATE_N_4		(0x2 << 16)
#define APB2_CLK_RATE_N_8		(0x3 << 16)
#define APB2_CLK_RATE_N_MASK		(3 << 16)
#define APB2_CLK_RATE_M(m)		(((m)-1) << 0)
#define APB2_CLK_RATE_M_MASK            (0x1f << 0)

/* apb2 gate field */
#define APB2_GATE_UART_SHIFT	(16)
#define APB2_GATE_UART_MASK		(0xff << APB2_GATE_UART_SHIFT)
#define APB2_GATE_TWI_SHIFT	(0)
#define APB2_GATE_TWI_MASK		(0xf << APB2_GATE_TWI_SHIFT)

/* cpu_axi_cfg bits */
#define AXI0_DIV_SHIFT			0
#define AXI1_DIV_SHIFT			16
#define C0_CPUX_CLK_SRC_SHIFT		12
#define C1_CPUX_CLK_SRC_SHIFT		28

#define AXI_DIV_1			0
#define AXI_DIV_2			1
#define AXI_DIV_3			2
#define AXI_DIV_4			3
#define CPU_CLK_SRC_OSC24M		0
#define CPU_CLK_SRC_PLL1		1

#define CCM_PLL1_CTRL_N(n)		(((n) & 0xff) << 8)
#define CCM_PLL1_CTRL_P(n)		(((n) & 0x1) << 16)
#define CCM_PLL1_CTRL_EN		(0x1 << 31)
#define CMM_PLL1_CLOCK_TIME_2		(0x2 << 24)

#define PLL8_CFG_DEFAULT		0x42800
#define CCM_CCI400_CLK_SEL_HSIC		(0x2<<24)

#define CCM_PLL5_DIV1_SHIFT		16
#define CCM_PLL5_DIV2_SHIFT		18
#define CCM_PLL5_CTRL_N(n)		(((n) - 1) << 8)
#define CCM_PLL5_CTRL_UPD		(0x1 << 30)
#define CCM_PLL5_CTRL_EN		(0x1 << 31)

#define PLL6_CFG_DEFAULT		0x80001900 /* 600 MHz */
#define CCM_PLL6_CTRL_N_SHIFT		8
#define CCM_PLL6_CTRL_N_MASK		(0xff << CCM_PLL6_CTRL_N_SHIFT)
#define CCM_PLL6_CTRL_DIV1_SHIFT	16
#define CCM_PLL6_CTRL_DIV1_MASK		(0x1 << CCM_PLL6_CTRL_DIV1_SHIFT)
#define CCM_PLL6_CTRL_DIV2_SHIFT	18
#define CCM_PLL6_CTRL_DIV2_MASK		(0x1 << CCM_PLL6_CTRL_DIV2_SHIFT)

#define AHB1_ABP1_DIV_DEFAULT		0x00002190
#define AHB1_CLK_SRC_MASK		(0x3<<12)
#define AHB1_CLK_SRC_INTOSC		(0x0<<12)
#define AHB1_CLK_SRC_OSC24M		(0x1<<12)
#define AHB1_CLK_SRC_PLL6		(0x2<<12)

#define AXI_GATE_OFFSET_DRAM		0

/* ahb_gate0 offsets */
#define AHB_GATE_OFFSET_USB_OHCI1	30
#define AHB_GATE_OFFSET_USB_OHCI0	29
#define AHB_GATE_OFFSET_USB_EHCI1	27
#define AHB_GATE_OFFSET_USB_EHCI0	26
#define AHB_GATE_OFFSET_USB0		24
#define AHB_GATE_OFFSET_SPI1		21
#define AHB_GATE_OFFSET_SPI0		20
#define AHB_GATE_OFFSET_HSTIMER		19
#define AHB_GATE_OFFSET_EMAC		17
#define AHB_GATE_OFFSET_MCTL		14
#define AHB_GATE_OFFSET_GMAC		17
#define AHB_GATE_OFFSET_NAND0		13
#define AHB_GATE_OFFSET_MMC0		8
#define AHB_GATE_OFFSET_MMC(n)		(AHB_GATE_OFFSET_MMC0 + (n))
#define AHB_GATE_OFFSET_DMA		6
#define AHB_GATE_OFFSET_SS		5

/* ahb_gate1 offsets */
#define AHB_GATE_OFFSET_DRC0		25
#define AHB_GATE_OFFSET_DE_FE0		14
#define AHB_GATE_OFFSET_DE_BE0		12
#define AHB_GATE_OFFSET_HDMI		11
#define AHB_GATE_OFFSET_LCD1		5
#define AHB_GATE_OFFSET_LCD0		4

#define CCM_MMC_CTRL_M(x)		((x) - 1)
#define CCM_MMC_CTRL_OCLK_DLY(x)	((x) << 8)
#define CCM_MMC_CTRL_N(x)		((x) << 16)
#define CCM_MMC_CTRL_SCLK_DLY(x)	((x) << 20)
#define CCM_MMC_CTRL_OSCM24		(0x0 << 24)
#define CCM_MMC_CTRL_PLL6		(0x1 << 24)
#define CCM_MMC_CTRL_MODE_SEL_NEW	(0x1 << 30)
#define CCM_MMC_CTRL_ENABLE		(0x1 << 31)

#define CCM_USB_CTRL_PHY0_RST (0x1 << 0)
#define CCM_USB_CTRL_PHY1_RST (0x1 << 1)
#define CCM_USB_CTRL_HSIC_RST (0x1 << 2)
/* There is no global phy clk gate on sun6i, define as 0 */
#define CCM_USB_CTRL_PHYGATE 0
#define CCM_USB_CTRL_PHY0_CLK (0x1 << 8)
#define CCM_USB_CTRL_PHY1_CLK (0x1 << 9)
#define CCM_USB_CTRL_HSIC_CLK (0x1 << 10)
#define CCM_USB_CTRL_12M_CLK (0x1 << 11)
#define CCM_USB_CTRL_OHCI0_CLK (0x1 << 16)

#define CCM_GMAC_CTRL_TX_CLK_SRC_MII	0x0
#define CCM_GMAC_CTRL_TX_CLK_SRC_EXT_RGMII 0x1
#define CCM_GMAC_CTRL_TX_CLK_SRC_INT_RGMII 0x2
#define CCM_GMAC_CTRL_GPIT_MII		(0x0 << 2)
#define CCM_GMAC_CTRL_GPIT_RGMII	(0x1 << 2)
#define CCM_GMAC_CTRL_RX_CLK_DELAY(x)	((x) << 5)
#define CCM_GMAC_CTRL_TX_CLK_DELAY(x)	((x) << 10)

#define MDFS_CLK_DEFAULT		0x81000002 /* PLL6 / 3 */

#define CCM_DRAMCLK_CFG_DIV(x)		((x - 1) << 0)
#define CCM_DRAMCLK_CFG_DIV_MASK	(0xf << 0)
#define CCM_DRAMCLK_CFG_DIV0(x)		((x - 1) << 8)
#define CCM_DRAMCLK_CFG_DIV0_MASK	(0xf << 8)
#define CCM_DRAMCLK_CFG_UPD		(0x1 << 16)
#define CCM_DRAMCLK_CFG_RST		(0x1 << 31)

#define CCM_DRAMPLL_CFG_SRC_PLL5	(0x0 << 16) /* Select PLL5 (DDR0) */
#define CCM_DRAMPLL_CFG_SRC_PLL11	(0x1 << 16) /* Select PLL11 (DDR1) */
#define CCM_DRAMPLL_CFG_SRC_MASK	(0x1 << 16)

#define CCM_MBUS_RESET_RESET		(0x1 << 31)

#define CCM_DRAM_GATE_OFFSET_DE_FE0	24
#define CCM_DRAM_GATE_OFFSET_DE_FE1	25
#define CCM_DRAM_GATE_OFFSET_DE_BE0	26
#define CCM_DRAM_GATE_OFFSET_DE_BE1	27


#define MBUS_CLK_DEFAULT		0x81000002 /* PLL6 / 2 */

#define MBUS_CLK_GATE			(0x1 << 31)

/* ahb_reset0 offsets */
#define AHB_RESET_OFFSET_GMAC		17
#define AHB_RESET_OFFSET_MCTL		14
#define AHB_RESET_OFFSET_MMC3		11
#define AHB_RESET_OFFSET_MMC2		10
#define AHB_RESET_OFFSET_MMC1		9
#define AHB_RESET_OFFSET_MMC0		8
#define AHB_RESET_OFFSET_MMC(n)		(AHB_RESET_OFFSET_MMC0 + (n))
#define AHB_RESET_OFFSET_SS		5

/* ahb_reset1 offsets */
#define AHB_RESET_OFFSET_SAT		26
#define AHB_RESET_OFFSET_DRC0		25
#define AHB_RESET_OFFSET_DE_FE0		14
#define AHB_RESET_OFFSET_DE_BE0		12
#define AHB_RESET_OFFSET_HDMI		11
#define AHB_RESET_OFFSET_LCD1		5
#define AHB_RESET_OFFSET_LCD0		4

/* ahb_reset2 offsets */
#define AHB_RESET_OFFSET_LVDS		0

/* apb2 reset */
#define APB2_RESET_UART_SHIFT		(16)
#define APB2_RESET_UART_MASK		(0xff << APB2_RESET_UART_SHIFT)
#define APB2_RESET_TWI_SHIFT		(0)
#define APB2_RESET_TWI_MASK		(0xf << APB2_RESET_TWI_SHIFT)


#ifndef __ASSEMBLY__
void clock_set_pll1(unsigned int hz);
void clock_set_pll5(unsigned int clk);
unsigned int clock_get_pll6(void);
#endif

#endif /* _SUNXI_CLOCK_SUN8I_A83T_H */
