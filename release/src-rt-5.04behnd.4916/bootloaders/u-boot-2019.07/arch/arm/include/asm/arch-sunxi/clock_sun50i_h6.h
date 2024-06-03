/*
 * Allwinner H6 clock register definitions
 *
 * (C) Copyright 2017 Icenowy Zheng <icenowy@aosc.io>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _SUNXI_CLOCK_SUN50I_H6_H
#define _SUNXI_CLOCK_SUN50I_H6_H

struct sunxi_ccm_reg {
	u32 pll1_cfg;		/* 0x000 pll1 (cpux) control */
	u8 reserved_0x004[12];
	u32 pll5_cfg;		/* 0x010 pll5 (ddr) control */
	u8 reserved_0x014[12];
	u32 pll6_cfg;		/* 0x020 pll6 (periph0) control */
	u8 reserved_0x020[4];
	u32 pll_periph1_cfg;	/* 0x028 pll periph1 control */
	u8 reserved_0x028[4];
	u32 pll7_cfg;		/* 0x030 pll7 (gpu) control */
	u8 reserved_0x034[12];
	u32 pll3_cfg;		/* 0x040 pll3 (video0) control */
	u8 reserved_0x044[4];
	u32 pll_video1_cfg;	/* 0x048 pll video1 control */
	u8 reserved_0x04c[12];
	u32 pll4_cfg;		/* 0x058 pll4 (ve) control */
	u8 reserved_0x05c[4];
	u32 pll10_cfg;		/* 0x060 pll10 (de) control */
	u8 reserved_0x064[12];
	u32 pll9_cfg;		/* 0x070 pll9 (hsic) control */
	u8 reserved_0x074[4];
	u32 pll2_cfg;		/* 0x078 pll2 (audio) control */
	u8 reserved_0x07c[148];
	u32 pll5_pat;		/* 0x110 pll5 (ddr) pattern */
	u8 reserved_0x114[20];
	u32 pll_periph1_pat0;	/* 0x128 pll periph1 pattern0 */
	u32 pll_periph1_pat1;	/* 0x12c pll periph1 pattern1 */
	u32 pll7_pat0;		/* 0x130 pll7 (gpu) pattern0 */
	u32 pll7_pat1;		/* 0x134 pll7 (gpu) pattern1 */
	u8 reserved_0x138[8];
	u32 pll3_pat0;		/* 0x140 pll3 (video0) pattern0 */
	u32 pll3_pat1;		/* 0x144 pll3 (video0) pattern1 */
	u32 pll_video1_pat0;	/* 0x148 pll video1 pattern0 */
	u32 pll_video1_pat1;	/* 0x14c pll video1 pattern1 */
	u8 reserved_0x150[8];
	u32 pll4_pat0;		/* 0x158 pll4 (ve) pattern0 */
	u32 pll4_pat1;		/* 0x15c pll4 (ve) pattern1 */
	u32 pll10_pat0;		/* 0x160 pll10 (de) pattern0 */
	u32 pll10_pat1;		/* 0x164 pll10 (de) pattern1 */
	u8 reserved_0x168[8];
	u32 pll9_pat0;		/* 0x170 pll9 (hsic) pattern0 */
	u32 pll9_pat1;		/* 0x174 pll9 (hsic) pattern1 */
	u32 pll2_pat0;		/* 0x178 pll2 (audio) pattern0 */
	u32 pll2_pat1;		/* 0x17c pll2 (audio) pattern1 */
	u8 reserved_0x180[384];
	u32 pll1_bias;		/* 0x300 pll1 (cpux) bias */
	u8 reserved_0x304[12];
	u32 pll5_bias;		/* 0x310 pll5 (ddr) bias */
	u8 reserved_0x314[12];
	u32 pll6_bias;		/* 0x320 pll6 (periph0) bias */
	u8 reserved_0x324[4];
	u32 pll_periph1_bias;	/* 0x328 pll periph1 bias */
	u8 reserved_0x32c[4];
	u32 pll7_bias;		/* 0x330 pll7 (gpu) bias */
	u8 reserved_0x334[12];
	u32 pll3_bias;		/* 0x340 pll3 (video0) bias */
	u8 reserved_0x344[4];
	u32 pll_video1_bias;	/* 0x348 pll video1 bias */
	u8 reserved_0x34c[12];
	u32 pll4_bias;		/* 0x358 pll4 (ve) bias */
	u8 reserved_0x35c[4];
	u32 pll10_bias;		/* 0x360 pll10 (de) bias */
	u8 reserved_0x364[12];
	u32 pll9_bias;		/* 0x370 pll9 (hsic) bias */
	u8 reserved_0x374[4];
	u32 pll2_bias;		/* 0x378 pll2 (audio) bias */
	u8 reserved_0x37c[132];
	u32 pll1_tun;		/* 0x400 pll1 (cpux) tunning */
	u8 reserved_0x404[252];
	u32 cpu_axi_cfg;	/* 0x500 CPUX/AXI clock control*/
	u8 reserved_0x504[12];
	u32 psi_ahb1_ahb2_cfg;	/* 0x510 PSI/AHB1/AHB2 clock control */
	u8 reserved_0x514[8];
	u32 ahb3_cfg;		/* 0x51c AHB3 clock control */
	u32 apb1_cfg;		/* 0x520 APB1 clock control */
	u32 apb2_cfg;		/* 0x524 APB2 clock control */
	u8 reserved_0x528[24];
	u32 mbus_cfg;		/* 0x540 MBUS clock control */
	u8 reserved_0x544[188];
	u32 de_clk_cfg;		/* 0x600 DE clock control */
	u8 reserved_0x604[8];
	u32 de_gate_reset;	/* 0x60c DE gate/reset control */
	u8 reserved_0x610[16];
	u32 di_clk_cfg;		/* 0x620 DI clock control */
	u8 reserved_0x024[8];
	u32 di_gate_reset;	/* 0x62c DI gate/reset control */
	u8 reserved_0x630[64];
	u32 gpu_clk_cfg;	/* 0x670 GPU clock control */
	u8 reserved_0x674[8];
	u32 gpu_gate_reset;	/* 0x67c GPU gate/reset control */
	u32 ce_clk_cfg;		/* 0x680 CE clock control */
	u8 reserved_0x684[8];
	u32 ce_gate_reset;	/* 0x68c CE gate/reset control */
	u32 ve_clk_cfg;		/* 0x690 VE clock control */
	u8 reserved_0x694[8];
	u32 ve_gate_reset;	/* 0x69c VE gate/reset control */
	u8 reserved_0x6a0[16];
	u32 emce_clk_cfg;	/* 0x6b0 EMCE clock control */
	u8 reserved_0x6b4[8];
	u32 emce_gate_reset;	/* 0x6bc EMCE gate/reset control */
	u32 vp9_clk_cfg;	/* 0x6c0 VP9 clock control */
	u8 reserved_0x6c4[8];
	u32 vp9_gate_reset;	/* 0x6cc VP9 gate/reset control */
	u8 reserved_0x6d0[60];
	u32 dma_gate_reset;	/* 0x70c DMA gate/reset control */
	u8 reserved_0x710[12];
	u32 msgbox_gate_reset;	/* 0x71c Message Box gate/reset control */
	u8 reserved_0x720[12];
	u32 spinlock_gate_reset;/* 0x72c Spinlock gate/reset control */
	u8 reserved_0x730[12];
	u32 hstimer_gate_reset;	/* 0x73c HS Timer gate/reset control */
	u32 avs_gate_reset;	/* 0x740 AVS gate/reset control */
	u8 reserved_0x744[72];
	u32 dbgsys_gate_reset;	/* 0x78c Debugging system gate/reset control */
	u8 reserved_0x790[12];
	u32 psi_gate_reset;	/* 0x79c PSI gate/reset control */
	u8 reserved_0x7a0[12];
	u32 pwm_gate_reset;	/* 0x7ac PWM gate/reset control */
	u8 reserved_0x7b0[12];
	u32 iommu_gate_reset;	/* 0x7bc IOMMU gate/reset control */
	u8 reserved_0x7c0[64];
	u32 dram_clk_cfg;		/* 0x800 DRAM clock control */
	u32 mbus_gate;		/* 0x804 MBUS gate control */
	u8 reserved_0x808[4];
	u32 dram_gate_reset;	/* 0x80c DRAM gate/reset control */
	u32 nand0_clk_cfg;	/* 0x810 NAND0 clock control */
	u32 nand1_clk_cfg;	/* 0x814 NAND1 clock control */
	u8 reserved_0x818[20];
	u32 nand_gate_reset;	/* 0x82c NAND gate/reset control */
	u32 sd0_clk_cfg;	/* 0x830 MMC0 clock control */
	u32 sd1_clk_cfg;	/* 0x834 MMC1 clock control */
	u32 sd2_clk_cfg;	/* 0x838 MMC2 clock control */
	u8 reserved_0x83c[16];
	u32 sd_gate_reset;	/* 0x84c MMC gate/reset control */
	u8 reserved_0x850[188];
	u32 uart_gate_reset;	/* 0x90c UART gate/reset control */
	u8 reserved_0x910[12];
	u32 twi_gate_reset;	/* 0x91c I2C gate/reset control */
	u8 reserved_0x920[28];
	u32 scr_gate_reset;	/* 0x93c SCR gate/reset control */
	u32 spi0_clk_cfg;	/* 0x940 SPI0 clock control */
	u32 spi1_clk_cfg;	/* 0x944 SPI1 clock control */
	u8 reserved_0x948[36];
	u32 spi_gate_reset;	/* 0x96c SPI gate/reset control */
	u8 reserved_0x970[12];
	u32 emac_gate_reset;	/* 0x97c EMAC gate/reset control */
	u8 reserved_0x980[48];
	u32 ts_clk_cfg;		/* 0x9b0 TS clock control */
	u8 reserved_0x9b4[8];
	u32 ts_gate_reset;	/* 0x9bc TS gate/reset control */
	u32 irtx_clk_cfg;	/* 0x9c0 IR TX clock control */
	u8 reserved_0x9c4[8];
	u32 irtx_gate_reset;	/* 0x9cc IR TX gate/reset control */
	u8 reserved_0x9d0[44];
	u32 ths_gate_reset;	/* 0x9fc THS gate/reset control */
	u8 reserved_0xa00[12];
	u32 i2s3_clk_cfg;	/* 0xa0c I2S3 clock control */
	u32 i2s0_clk_cfg;	/* 0xa10 I2S0 clock control */
	u32 i2s1_clk_cfg;	/* 0xa14 I2S1 clock control */
	u32 i2s2_clk_cfg;	/* 0xa18 I2S2 clock control */
	u32 i2s_gate_reset;	/* 0xa1c I2S gate/reset control */
	u32 spdif_clk_cfg;	/* 0xa20 SPDIF clock control */
	u8 reserved_0xa24[8];
	u32 spdif_gate_reset;	/* 0xa2c SPDIF gate/reset control */
	u8 reserved_0xa30[16];
	u32 dmic_clk_cfg;	/* 0xa40 DMIC clock control */
	u8 reserved_0xa44[8];
	u32 dmic_gate_reset;	/* 0xa4c DMIC gate/reset control */
	u8 reserved_0xa50[16];
	u32 ahub_clk_cfg;	/* 0xa60 Audio HUB clock control */
	u8 reserved_0xa64[8];
	u32 ahub_gate_reset;	/* 0xa6c Audio HUB gate/reset control */
	u32 usb0_clk_cfg;	/* 0xa70 USB0(OTG) clock control */
	u32 usb1_clk_cfg;	/* 0xa74 USB1(XHCI) clock control */
	u8 reserved_0xa78[4];
	u32 usb3_clk_cfg;	/* 0xa78 USB3 clock control */
	u8 reserved_0xa80[12];
	u32 usb_gate_reset;	/* 0xa8c USB gate/reset control */
	u8 reserved_0xa90[32];
	u32 pcie_ref_clk_cfg;	/* 0xab0 PCIE REF clock control */
	u32 pcie_axi_clk_cfg;	/* 0xab4 PCIE AXI clock control */
	u32 pcie_aux_clk_cfg;	/* 0xab8 PCIE AUX clock control */
	u32 pcie_gate_reset;	/* 0xabc PCIE gate/reset control */
	u8 reserved_0xac0[64];
	u32 hdmi_clk_cfg;	/* 0xb00 HDMI clock control */
	u32 hdmi_slow_clk_cfg;	/* 0xb04 HDMI slow clock control */
	u8 reserved_0xb08[8];
	u32 hdmi_cec_clk_cfg;	/* 0xb10 HDMI CEC clock control */
	u8 reserved_0xb14[8];
	u32 hdmi_gate_reset;	/* 0xb1c HDMI gate/reset control */
	u8 reserved_0xb20[60];
	u32 tcon_top_gate_reset;/* 0xb5c TCON TOP gate/reset control */
	u32 tcon_lcd0_clk_cfg;	/* 0xb60 TCON LCD0 clock control */
	u8 reserved_0xb64[24];
	u32 tcon_lcd_gate_reset;/* 0xb7c TCON LCD gate/reset control */
	u32 tcon_tv0_clk_cfg;	/* 0xb80 TCON TV0 clock control */
	u8 reserved_0xb84[24];
	u32 tcon_tv_gate_reset;	/* 0xb9c TCON TV gate/reset control */
	u8 reserved_0xba0[96];
	u32 csi_misc_clk_cfg;	/* 0xc00 CSI MISC clock control */
	u32 csi_top_clk_cfg;	/* 0xc04 CSI TOP clock control */
	u32 csi_mclk_cfg;	/* 0xc08 CSI Master clock control */
	u8 reserved_0xc0c[32];
	u32 csi_gate_reset;	/* 0xc2c CSI gate/reset control */
	u8 reserved_0xc30[16];
	u32 hdcp_clk_cfg;	/* 0xc40 HDCP clock control */
	u8 reserved_0xc44[8];
	u32 hdcp_gate_reset;	/* 0xc4c HDCP gate/reset control */
	u8 reserved_0xc50[688];
	u32 ccu_sec_switch;	/* 0xf00 CCU security switch */
	u32 pll_lock_dbg_ctrl;	/* 0xf04 PLL lock debugging control */
};

/* pll1 bit field */
#define CCM_PLL1_CTRL_EN		BIT(31)
#define CCM_PLL1_LOCK_EN		BIT(29)
#define CCM_PLL1_LOCK			BIT(28)
#define CCM_PLL1_CLOCK_TIME_2		(2 << 24)
#define CCM_PLL1_CTRL_P(p)		((p) << 16)
#define CCM_PLL1_CTRL_N(n)		((n) << 8)

/* pll5 bit field */
#define CCM_PLL5_CTRL_EN		BIT(31)
#define CCM_PLL5_LOCK_EN		BIT(29)
#define CCM_PLL5_LOCK			BIT(28)
#define CCM_PLL5_CTRL_N(n)		((n) << 8)
#define CCM_PLL5_CTRL_DIV1(div1)	((div1) << 0)
#define CCM_PLL5_CTRL_DIV2(div0)	((div0) << 1)

/* pll6 bit field */
#define CCM_PLL6_CTRL_EN		BIT(31)
#define CCM_PLL6_LOCK_EN		BIT(29)
#define CCM_PLL6_LOCK			BIT(28)
#define CCM_PLL6_CTRL_N_SHIFT		8
#define CCM_PLL6_CTRL_N_MASK		(0xff << CCM_PLL6_CTRL_N_SHIFT)
#define CCM_PLL6_CTRL_DIV1_SHIFT	0
#define CCM_PLL6_CTRL_DIV1_MASK		(0x1 << CCM_PLL6_CTRL_DIV1_SHIFT)
#define CCM_PLL6_CTRL_DIV2_SHIFT	1
#define CCM_PLL6_CTRL_DIV2_MASK		(0x1 << CCM_PLL6_CTRL_DIV2_SHIFT)
#define CCM_PLL6_DEFAULT		0xa0006300

/* cpu_axi bit field*/
#define CCM_CPU_AXI_MUX_MASK		(0x3 << 24)
#define CCM_CPU_AXI_MUX_OSC24M		(0x0 << 24)
#define CCM_CPU_AXI_MUX_PLL_CPUX	(0x3 << 24)
#define CCM_CPU_AXI_APB_MASK		0x300
#define CCM_CPU_AXI_AXI_MASK		0x3
#define CCM_CPU_AXI_DEFAULT_FACTORS	0x301

/* psi_ahb1_ahb2 bit field */
#define CCM_PSI_AHB1_AHB2_DEFAULT	0x03000102

/* ahb3 bit field */
#define CCM_AHB3_DEFAULT		0x03000002

/* apb1 bit field */
#define CCM_APB1_DEFAULT		0x03000102

/* apb2 bit field */
#define APB2_CLK_SRC_OSC24M		(0x0 << 24)
#define APB2_CLK_SRC_OSC32K		(0x1 << 24)
#define APB2_CLK_SRC_PSI		(0x2 << 24)
#define APB2_CLK_SRC_PLL6		(0x3 << 24)
#define APB2_CLK_SRC_MASK		(0x3 << 24)
#define APB2_CLK_RATE_N_1		(0x0 << 8)
#define APB2_CLK_RATE_N_2		(0x1 << 8)
#define APB2_CLK_RATE_N_4		(0x2 << 8)
#define APB2_CLK_RATE_N_8		(0x3 << 8)
#define APB2_CLK_RATE_N_MASK		(3 << 8)
#define APB2_CLK_RATE_M(m)		(((m)-1) << 0)
#define APB2_CLK_RATE_M_MASK            (3 << 0)

/* MBUS clock bit field */
#define MBUS_ENABLE			BIT(31)
#define MBUS_RESET			BIT(30)
#define MBUS_CLK_SRC_MASK		GENMASK(25, 24)
#define MBUS_CLK_SRC_OSCM24		(0 << 24)
#define MBUS_CLK_SRC_PLL6X2		(1 << 24)
#define MBUS_CLK_SRC_PLL5		(2 << 24)
#define MBUS_CLK_SRC_PLL6X4		(3 << 24)
#define MBUS_CLK_M(m)			(((m)-1) << 0)

/* Module gate/reset shift*/
#define RESET_SHIFT			(16)

/* DRAM clock bit field */
#define DRAM_MOD_RESET			BIT(30)
#define DRAM_CLK_UPDATE			BIT(27)
#define DRAM_CLK_SRC_MASK		GENMASK(25, 24)
#define DRAM_CLK_SRC_PLL5		(0 << 24)
#define DRAM_CLK_M(m)			(((m)-1) << 0)

/* MMC clock bit field */
#define CCM_MMC_CTRL_M(x)		((x) - 1)
#define CCM_MMC_CTRL_N(x)		((x) << 8)
#define CCM_MMC_CTRL_OSCM24		(0x0 << 24)
#define CCM_MMC_CTRL_PLL6X2		(0x1 << 24)
#define CCM_MMC_CTRL_PLL_PERIPH2X2	(0x2 << 24)
#define CCM_MMC_CTRL_ENABLE		(0x1 << 31)
/* H6 doesn't have these delays */
#define CCM_MMC_CTRL_OCLK_DLY(a)	((void) (a), 0)
#define CCM_MMC_CTRL_SCLK_DLY(a)	((void) (a), 0)

#ifndef __ASSEMBLY__
void clock_set_pll1(unsigned int hz);
unsigned int clock_get_pll6(void);
#endif

#endif /* _SUNXI_CLOCK_SUN50I_H6_H */
