/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * sun4i, sun5i and sun7i clock register definitions
 *
 * (C) Copyright 2007-2011
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * Tom Cubie <tangliang@allwinnertech.com>
 */

#ifndef _SUNXI_CLOCK_SUN4I_H
#define _SUNXI_CLOCK_SUN4I_H

struct sunxi_ccm_reg {
	u32 pll1_cfg;		/* 0x00 pll1 control */
	u32 pll1_tun;		/* 0x04 pll1 tuning */
	u32 pll2_cfg;		/* 0x08 pll2 control */
	u32 pll2_tun;		/* 0x0c pll2 tuning */
	u32 pll3_cfg;		/* 0x10 pll3 control */
	u8 res0[0x4];
	u32 pll4_cfg;		/* 0x18 pll4 control */
	u8 res1[0x4];
	u32 pll5_cfg;		/* 0x20 pll5 control */
	u32 pll5_tun;		/* 0x24 pll5 tuning */
	u32 pll6_cfg;		/* 0x28 pll6 control */
	u32 pll6_tun;		/* 0x2c pll6 tuning */
	u32 pll7_cfg;		/* 0x30 pll7 control */
	u32 pll1_tun2;		/* 0x34 pll5 tuning2 */
	u8 res2[0x4];
	u32 pll5_tun2;		/* 0x3c pll5 tuning2 */
	u8 res3[0xc];
	u32 pll_lock_dbg;	/* 0x4c pll lock time debug */
	u32 osc24m_cfg;		/* 0x50 osc24m control */
	u32 cpu_ahb_apb0_cfg;	/* 0x54 cpu,ahb and apb0 divide ratio */
	u32 apb1_clk_div_cfg;	/* 0x58 apb1 clock dividor */
	u32 axi_gate;		/* 0x5c axi module clock gating */
	u32 ahb_gate0;		/* 0x60 ahb module clock gating 0 */
	u32 ahb_gate1;		/* 0x64 ahb module clock gating 1 */
	u32 apb0_gate;		/* 0x68 apb0 module clock gating */
	u32 apb1_gate;		/* 0x6c apb1 module clock gating */
	u8 res4[0x10];
	u32 nand0_clk_cfg;	/* 0x80 nand sub clock control */
	u32 ms_sclk_cfg;	/* 0x84 memory stick sub clock control */
	u32 sd0_clk_cfg;	/* 0x88 sd0 clock control */
	u32 sd1_clk_cfg;	/* 0x8c sd1 clock control */
	u32 sd2_clk_cfg;	/* 0x90 sd2 clock control */
	u32 sd3_clk_cfg;	/* 0x94 sd3 clock control */
	u32 ts_clk_cfg;		/* 0x98 transport stream clock control */
	u32 ss_clk_cfg;		/* 0x9c */
	u32 spi0_clk_cfg;	/* 0xa0 */
	u32 spi1_clk_cfg;	/* 0xa4 */
	u32 spi2_clk_cfg;	/* 0xa8 */
	u32 pata_clk_cfg;	/* 0xac */
	u32 ir0_clk_cfg;	/* 0xb0 */
	u32 ir1_clk_cfg;	/* 0xb4 */
	u32 iis_clk_cfg;	/* 0xb8 */
	u32 ac97_clk_cfg;	/* 0xbc */
	u32 spdif_clk_cfg;	/* 0xc0 */
	u32 keypad_clk_cfg;	/* 0xc4 */
	u32 sata_clk_cfg;	/* 0xc8 */
	u32 usb_clk_cfg;	/* 0xcc */
	u32 gps_clk_cfg;	/* 0xd0 */
	u32 spi3_clk_cfg;	/* 0xd4 */
	u8 res5[0x28];
	u32 dram_clk_gate;	/* 0x100 */
	u32 be0_clk_cfg;	/* 0x104 */
	u32 be1_clk_cfg;	/* 0x108 */
	u32 fe0_clk_cfg;	/* 0x10c */
	u32 fe1_clk_cfg;	/* 0x110 */
	u32 mp_clk_cfg;		/* 0x114 */
	u32 lcd0_ch0_clk_cfg;	/* 0x118 */
	u32 lcd1_ch0_clk_cfg;	/* 0x11c */
	u32 csi_isp_clk_cfg;	/* 0x120 */
	u8 res6[0x4];
	u32 tvd_clk_reg;	/* 0x128 */
	u32 lcd0_ch1_clk_cfg;	/* 0x12c */
	u32 lcd1_ch1_clk_cfg;	/* 0x130 */
	u32 csi0_clk_cfg;	/* 0x134 */
	u32 csi1_clk_cfg;	/* 0x138 */
	u32 ve_clk_cfg;		/* 0x13c */
	u32 audio_codec_clk_cfg;	/* 0x140 */
	u32 avs_clk_cfg;	/* 0x144 */
	u32 ace_clk_cfg;	/* 0x148 */
	u32 lvds_clk_cfg;	/* 0x14c */
	u32 hdmi_clk_cfg;	/* 0x150 */
	u32 mali_clk_cfg;	/* 0x154 */
	u8 res7[0x4];
	u32 mbus_clk_cfg;	/* 0x15c */
	u8 res8[0x4];
	u32 gmac_clk_cfg;	/* 0x164 */
};

/* apb1 bit field */
#define APB1_CLK_SRC_OSC24M		(0x0 << 24)
#define APB1_CLK_SRC_PLL6		(0x1 << 24)
#define APB1_CLK_SRC_LOSC		(0x2 << 24)
#define APB1_CLK_SRC_MASK		(0x3 << 24)
#define APB1_CLK_RATE_N_1		(0x0 << 16)
#define APB1_CLK_RATE_N_2		(0x1 << 16)
#define APB1_CLK_RATE_N_4		(0x2 << 16)
#define APB1_CLK_RATE_N_8		(0x3 << 16)
#define APB1_CLK_RATE_N_MASK		(3 << 16)
#define APB1_CLK_RATE_M(m)		(((m)-1) << 0)
#define APB1_CLK_RATE_M_MASK            (0x1f << 0)

/* apb1 gate field */
#define APB1_GATE_UART_SHIFT	(16)
#define APB1_GATE_UART_MASK		(0xff << APB1_GATE_UART_SHIFT)
#define APB1_GATE_TWI_SHIFT	(0)
#define APB1_GATE_TWI_MASK		(0xf << APB1_GATE_TWI_SHIFT)

/* clock divide */
#define AXI_DIV_SHIFT		(0)
#define AXI_DIV_1			0
#define AXI_DIV_2			1
#define AXI_DIV_3			2
#define AXI_DIV_4			3
#define AHB_DIV_SHIFT		(4)
#define AHB_DIV_1			0
#define AHB_DIV_2			1
#define AHB_DIV_4			2
#define AHB_DIV_8			3
#define APB0_DIV_SHIFT		(8)
#define APB0_DIV_1			0
#define APB0_DIV_2			1
#define APB0_DIV_4			2
#define APB0_DIV_8			3
#define CPU_CLK_SRC_SHIFT	(16)
#define CPU_CLK_SRC_OSC24M		1
#define CPU_CLK_SRC_PLL1		2

#define CCM_PLL1_CFG_ENABLE_SHIFT		31
#define CCM_PLL1_CFG_VCO_RST_SHIFT		30
#define CCM_PLL1_CFG_VCO_BIAS_SHIFT		26
#define CCM_PLL1_CFG_PLL4_EXCH_SHIFT		25
#define CCM_PLL1_CFG_BIAS_CUR_SHIFT		20
#define CCM_PLL1_CFG_DIVP_SHIFT			16
#define CCM_PLL1_CFG_LCK_TMR_SHIFT		13
#define CCM_PLL1_CFG_FACTOR_N_SHIFT		8
#define CCM_PLL1_CFG_FACTOR_K_SHIFT		4
#define CCM_PLL1_CFG_SIG_DELT_PAT_IN_SHIFT	3
#define CCM_PLL1_CFG_SIG_DELT_PAT_EN_SHIFT	2
#define CCM_PLL1_CFG_FACTOR_M_SHIFT		0

#define PLL1_CFG_DEFAULT	0xa1005000

#if defined CONFIG_OLD_SUNXI_KERNEL_COMPAT && defined CONFIG_MACH_SUN5I
/*
 * Older linux-sunxi-3.4 kernels override our PLL6 setting with 300 MHz,
 * halving the mbus frequency, so set it to 300 MHz ourselves and base the
 * mbus divider on that.
 */
#define PLL6_CFG_DEFAULT	0xa1009900
#else
#define PLL6_CFG_DEFAULT	0xa1009911
#endif

/* nand clock */
#define NAND_CLK_SRC_OSC24		0
#define NAND_CLK_DIV_N			0
#define NAND_CLK_DIV_M			0

/* gps clock */
#define GPS_SCLK_GATING_OFF		0
#define GPS_RESET			0

/* ahb clock gate bit offset */
#define AHB_GATE_OFFSET_GPS		26
#define AHB_GATE_OFFSET_SATA		25
#define AHB_GATE_OFFSET_PATA		24
#define AHB_GATE_OFFSET_SPI3		23
#define AHB_GATE_OFFSET_SPI2		22
#define AHB_GATE_OFFSET_SPI1		21
#define AHB_GATE_OFFSET_SPI0		20
#define AHB_GATE_OFFSET_TS0		18
#define AHB_GATE_OFFSET_EMAC		17
#define AHB_GATE_OFFSET_ACE		16
#define AHB_GATE_OFFSET_DLL		15
#define AHB_GATE_OFFSET_SDRAM		14
#define AHB_GATE_OFFSET_NAND0		13
#define AHB_GATE_OFFSET_MS		12
#define AHB_GATE_OFFSET_MMC3		11
#define AHB_GATE_OFFSET_MMC2		10
#define AHB_GATE_OFFSET_MMC1		9
#define AHB_GATE_OFFSET_MMC0		8
#define AHB_GATE_OFFSET_MMC(n)		(AHB_GATE_OFFSET_MMC0 + (n))
#define AHB_GATE_OFFSET_BIST		7
#define AHB_GATE_OFFSET_DMA		6
#define AHB_GATE_OFFSET_SS		5
#define AHB_GATE_OFFSET_USB_OHCI1	4
#define AHB_GATE_OFFSET_USB_EHCI1	3
#define AHB_GATE_OFFSET_USB_OHCI0	2
#define AHB_GATE_OFFSET_USB_EHCI0	1
#define AHB_GATE_OFFSET_USB0		0

/* ahb clock gate bit offset (second register) */
#define AHB_GATE_OFFSET_GMAC		17
#define AHB_GATE_OFFSET_DE_FE0		14
#define AHB_GATE_OFFSET_DE_BE0		12
#define AHB_GATE_OFFSET_HDMI		11
#define AHB_GATE_OFFSET_LCD1		5
#define AHB_GATE_OFFSET_LCD0		4
#define AHB_GATE_OFFSET_TVE1		3
#define AHB_GATE_OFFSET_TVE0		2

#define CCM_AHB_GATE_GPS (0x1 << 26)
#define CCM_AHB_GATE_SDRAM (0x1 << 14)
#define CCM_AHB_GATE_DLL (0x1 << 15)
#define CCM_AHB_GATE_ACE (0x1 << 16)

#define CCM_PLL3_CTRL_M_SHIFT		0
#define CCM_PLL3_CTRL_M_MASK		(0x7f << CCM_PLL3_CTRL_M_SHIFT)
#define CCM_PLL3_CTRL_M(n)		(((n) & 0x7f) << 0)
#define CCM_PLL3_CTRL_INTEGER_MODE	(0x1 << 15)
#define CCM_PLL3_CTRL_EN		(0x1 << 31)

#define CCM_PLL5_CTRL_M(n) (((n) & 0x3) << 0)
#define CCM_PLL5_CTRL_M_MASK CCM_PLL5_CTRL_M(0x3)
#define CCM_PLL5_CTRL_M_X(n) ((n) - 1)
#define CCM_PLL5_CTRL_M1(n) (((n) & 0x3) << 2)
#define CCM_PLL5_CTRL_M1_MASK CCM_PLL5_CTRL_M1(0x3)
#define CCM_PLL5_CTRL_M1_X(n) ((n) - 1)
#define CCM_PLL5_CTRL_K(n) (((n) & 0x3) << 4)
#define CCM_PLL5_CTRL_K_SHIFT 4
#define CCM_PLL5_CTRL_K_MASK CCM_PLL5_CTRL_K(0x3)
#define CCM_PLL5_CTRL_K_X(n) ((n) - 1)
#define CCM_PLL5_CTRL_LDO (0x1 << 7)
#define CCM_PLL5_CTRL_N(n) (((n) & 0x1f) << 8)
#define CCM_PLL5_CTRL_N_SHIFT 8
#define CCM_PLL5_CTRL_N_MASK CCM_PLL5_CTRL_N(0x1f)
#define CCM_PLL5_CTRL_N_X(n) (n)
#define CCM_PLL5_CTRL_P(n) (((n) & 0x3) << 16)
#define CCM_PLL5_CTRL_P_SHIFT 16
#define CCM_PLL5_CTRL_P_MASK CCM_PLL5_CTRL_P(0x3)
#define CCM_PLL5_CTRL_P_X(n) ((n) - 1)
#define CCM_PLL5_CTRL_BW (0x1 << 18)
#define CCM_PLL5_CTRL_VCO_GAIN (0x1 << 19)
#define CCM_PLL5_CTRL_BIAS(n) (((n) & 0x1f) << 20)
#define CCM_PLL5_CTRL_BIAS_MASK CCM_PLL5_CTRL_BIAS(0x1f)
#define CCM_PLL5_CTRL_BIAS_X(n) ((n) - 1)
#define CCM_PLL5_CTRL_VCO_BIAS (0x1 << 25)
#define CCM_PLL5_CTRL_DDR_CLK (0x1 << 29)
#define CCM_PLL5_CTRL_BYPASS (0x1 << 30)
#define CCM_PLL5_CTRL_EN (0x1 << 31)

#define CCM_PLL6_CTRL_EN		31
#define CCM_PLL6_CTRL_BYPASS_EN		30
#define CCM_PLL6_CTRL_SATA_EN_SHIFT	14
#define CCM_PLL6_CTRL_N_SHIFT		8
#define CCM_PLL6_CTRL_N_MASK		(0x1f << CCM_PLL6_CTRL_N_SHIFT)
#define CCM_PLL6_CTRL_K_SHIFT		4
#define CCM_PLL6_CTRL_K_MASK		(0x3 << CCM_PLL6_CTRL_K_SHIFT)

#define CCM_GPS_CTRL_RESET (0x1 << 0)
#define CCM_GPS_CTRL_GATE (0x1 << 1)

#define CCM_DRAM_CTRL_DCLK_OUT (0x1 << 15)

#define CCM_MBUS_CTRL_M(n) (((n) & 0xf) << 0)
#define CCM_MBUS_CTRL_M_MASK CCM_MBUS_CTRL_M(0xf)
#define CCM_MBUS_CTRL_M_X(n) ((n) - 1)
#define CCM_MBUS_CTRL_N(n) (((n) & 0xf) << 16)
#define CCM_MBUS_CTRL_N_MASK CCM_MBUS_CTRL_N(0xf)
#define CCM_MBUS_CTRL_N_X(n) (((n) >> 3) ? 3 : (((n) >> 2) ? 2 : (((n) >> 1) ? 1 : 0)))
#define CCM_MBUS_CTRL_CLK_SRC(n) (((n) & 0x3) << 24)
#define CCM_MBUS_CTRL_CLK_SRC_MASK CCM_MBUS_CTRL_CLK_SRC(0x3)
#define CCM_MBUS_CTRL_CLK_SRC_HOSC 0x0
#define CCM_MBUS_CTRL_CLK_SRC_PLL6 0x1
#define CCM_MBUS_CTRL_CLK_SRC_PLL5 0x2
#define CCM_MBUS_CTRL_GATE (0x1 << 31)

#define CCM_NAND_CTRL_M(x)		((x) - 1)
#define CCM_NAND_CTRL_N(x)		((x) << 16)
#define CCM_NAND_CTRL_OSCM24		(0x0 << 24)
#define CCM_NAND_CTRL_PLL6		(0x1 << 24)
#define CCM_NAND_CTRL_PLL5		(0x2 << 24)
#define CCM_NAND_CTRL_ENABLE		(0x1 << 31)

#define CCM_MMC_CTRL_M(x)		((x) - 1)
#define CCM_MMC_CTRL_OCLK_DLY(x)	((x) << 8)
#define CCM_MMC_CTRL_N(x)		((x) << 16)
#define CCM_MMC_CTRL_SCLK_DLY(x)	((x) << 20)
#define CCM_MMC_CTRL_OSCM24		(0x0 << 24)
#define CCM_MMC_CTRL_PLL6		(0x1 << 24)
#define CCM_MMC_CTRL_PLL5		(0x2 << 24)
#define CCM_MMC_CTRL_ENABLE		(0x1 << 31)

#define CCM_DRAM_GATE_OFFSET_DE_FE1	24 /* Note the order of FE1 and */
#define CCM_DRAM_GATE_OFFSET_DE_FE0	25 /* FE0 is swapped ! */
#define CCM_DRAM_GATE_OFFSET_DE_BE0	26
#define CCM_DRAM_GATE_OFFSET_DE_BE1	27

#define CCM_LCD_CH0_CTRL_PLL3		(0 << 24)
#define CCM_LCD_CH0_CTRL_PLL7		(1 << 24)
#define CCM_LCD_CH0_CTRL_PLL3_2X	(2 << 24)
#define CCM_LCD_CH0_CTRL_PLL7_2X	(3 << 24)
#define CCM_LCD_CH0_CTRL_MIPI_PLL	0 /* No mipi pll on sun4i/5i/7i */
#ifdef CONFIG_MACH_SUN5I
#define CCM_LCD_CH0_CTRL_TVE_RST	(0x1 << 29)
#else
#define CCM_LCD_CH0_CTRL_TVE_RST	0 /* No separate tve-rst on sun4i/7i */
#endif
#define CCM_LCD_CH0_CTRL_RST		(0x1 << 30)
#define CCM_LCD_CH0_CTRL_GATE		(0x1 << 31)

#define CCM_LCD_CH1_CTRL_M(n)		((((n) - 1) & 0xf) << 0)
#define CCM_LCD_CH1_CTRL_HALF_SCLK1	(1 << 11)
#define CCM_LCD_CH1_CTRL_PLL3		(0 << 24)
#define CCM_LCD_CH1_CTRL_PLL7		(1 << 24)
#define CCM_LCD_CH1_CTRL_PLL3_2X	(2 << 24)
#define CCM_LCD_CH1_CTRL_PLL7_2X	(3 << 24)
/* Enable / disable both ch1 sclk1 and sclk2 at the same time */
#define CCM_LCD_CH1_CTRL_GATE		(0x1 << 31 | 0x1 << 15)

#define CCM_LVDS_CTRL_RST		(1 << 0)

#define CCM_HDMI_CTRL_M(n)		((((n) - 1) & 0xf) << 0)
#define CCM_HDMI_CTRL_PLL_MASK		(3 << 24)
#define CCM_HDMI_CTRL_PLL3		(0 << 24)
#define CCM_HDMI_CTRL_PLL7		(1 << 24)
#define CCM_HDMI_CTRL_PLL3_2X		(2 << 24)
#define CCM_HDMI_CTRL_PLL7_2X		(3 << 24)
/* No separate ddc gate on sun4i, sun5i and sun7i */
#define CCM_HDMI_CTRL_DDC_GATE		0
#define CCM_HDMI_CTRL_GATE		(0x1 << 31)

#define CCM_GMAC_CTRL_TX_CLK_SRC_MII 0x0
#define CCM_GMAC_CTRL_TX_CLK_SRC_EXT_RGMII 0x1
#define CCM_GMAC_CTRL_TX_CLK_SRC_INT_RGMII 0x2
#define CCM_GMAC_CTRL_GPIT_MII (0x0 << 2)
#define CCM_GMAC_CTRL_GPIT_RGMII (0x1 << 2)
#define CCM_GMAC_CTRL_RX_CLK_DELAY(x)	((x) << 5)
#define CCM_GMAC_CTRL_TX_CLK_DELAY(x)	((x) << 10)

#define CCM_USB_CTRL_PHY0_RST (0x1 << 0)
#define CCM_USB_CTRL_PHY1_RST (0x1 << 1)
#define CCM_USB_CTRL_PHY2_RST (0x1 << 2)
#define CCM_USB_CTRL_OHCI0_CLK (0x1 << 6)
#define CCM_USB_CTRL_OHCI1_CLK (0x1 << 7)
#define CCM_USB_CTRL_PHYGATE (0x1 << 8)
/* These 3 are sun6i only, define them as 0 on sun4i */
#define CCM_USB_CTRL_PHY0_CLK 0
#define CCM_USB_CTRL_PHY1_CLK 0
#define CCM_USB_CTRL_PHY2_CLK 0

/* CCM bits common to all Display Engine (and IEP) clock ctrl regs */
#define CCM_DE_CTRL_M(n)		((((n) - 1) & 0xf) << 0)
#define CCM_DE_CTRL_PLL_MASK		(3 << 24)
#define CCM_DE_CTRL_PLL3		(0 << 24)
#define CCM_DE_CTRL_PLL7		(1 << 24)
#define CCM_DE_CTRL_PLL5P		(2 << 24)
#define CCM_DE_CTRL_RST			(1 << 30)
#define CCM_DE_CTRL_GATE		(1 << 31)

#ifndef __ASSEMBLY__
void clock_set_pll1(unsigned int hz);
void clock_set_pll3(unsigned int hz);
unsigned int clock_get_pll3(void);
unsigned int clock_get_pll5p(void);
unsigned int clock_get_pll6(void);
#endif

#endif /* _SUNXI_CLOCK_SUN4I_H */
