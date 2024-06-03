/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2012
 * Gumstix Incorporated, <www.gumstix.com>
 * Maintainer: Ash Charles <ash@gumstix.com>
 */
#ifndef _DUOVERO_MUX_DATA_H_
#define _DUOVERO_MUX_DATA_H_

#include <asm/arch/mux_omap4.h>

const struct pad_conf_entry core_padconf_array_essential[] = {
	{SDMMC1_CLK, (PTU | OFF_EN | OFF_OUT_PTD | M0)},	 /* sdmmc1_clk */
	{SDMMC1_CMD, (PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M0)}, /* sdmmc1_cmd */
	{SDMMC1_DAT0, (PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M0)}, /* sdmmc1_dat0 */
	{SDMMC1_DAT1, (PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M0)}, /* sdmmc1_dat1 */
	{SDMMC1_DAT2, (PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M0)}, /* sdmmc1_dat2 */
	{SDMMC1_DAT3, (PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M0)}, /* sdmmc1_dat3 */
	{I2C1_SCL, (PTU | IEN | M0)},				/* i2c1_scl */
	{I2C1_SDA, (PTU | IEN | M0)},				/* i2c1_sda */
	{I2C2_SCL, (PTU | IEN | M0)},				/* i2c2_scl */
	{I2C2_SDA, (PTU | IEN | M0)},				/* i2c2_sda */
	{I2C3_SCL, (PTU | IEN | M0)},				/* i2c3_scl */
	{I2C3_SDA, (PTU | IEN | M0)},				/* i2c3_sda */
	{I2C4_SCL, (PTU | IEN | M0)},				/* i2c4_scl */
	{I2C4_SDA, (PTU | IEN | M0)},				/* i2c4_sda */
	{UART3_CTS_RCTX, (PTU | IEN | M0)},			/* uart3_tx */
	{UART3_RTS_SD, (M0)},					/* uart3_rts_sd */
	{UART3_RX_IRRX, (PTU | IEN | M0)},			/* uart3_rx */
	{UART3_TX_IRTX, (M0)}					/* uart3_tx */
};

const struct pad_conf_entry wkup_padconf_array_essential[] = {
	{PAD1_SR_SCL, (PTU | IEN | M0)},			/* sr_scl */
	{PAD0_SR_SDA, (PTU | IEN | M0)},			/* sr_sda */
	{PAD1_SYS_32K, (IEN | M0)}				/* sys_32k */
};

const struct pad_conf_entry core_padconf_array_non_essential[] = {
	{GPMC_AD0, (PTU | IEN | M0)},				/* gpmc_ad0 */
	{GPMC_AD1, (PTU | IEN | M0)},				/* gpmc_ad1 */
	{GPMC_AD2, (PTU | IEN | M0)},				/* gpmc_ad2 */
	{GPMC_AD3, (PTU | IEN | M0)},				/* gpmc_ad3 */
	{GPMC_AD4, (PTU | IEN | M0)},				/* gpmc_ad4 */
	{GPMC_AD5, (PTU | IEN | M0)},				/* gpmc_ad5 */
	{GPMC_AD6, (PTU | IEN | M0)},				/* gpmc_ad6 */
	{GPMC_AD7, (PTU | IEN | M0)},				/* gpmc_ad7 */
	{GPMC_AD8, (PTU | IEN | M0)},				/* gpmc_ad8 */
	{GPMC_AD9, (PTU | IEN | M0)},				/* gpmc_ad9 */
	{GPMC_AD10, (PTU | IEN | M0)},				/* gpmc_ad10 */
	{GPMC_AD11, (PTU | IEN | M0)},				/* gpmc_ad11 */
	{GPMC_AD12, (PTU | IEN | M0)},				/* gpmc_ad12 */
	{GPMC_AD13, (PTU | IEN | M0)},				/* gpmc_ad13 */
	{GPMC_AD14, (PTU | IEN | M0)},				/* gpmc_ad14 */
	{GPMC_AD15, (PTU | IEN | M0)},				/* gpmc_ad15 */
	{GPMC_A16, (PTU | IEN | M3)},				/* gpio_40 */
	{GPMC_A17, (PTU | IEN | M3)},				/* gpio_41 - hdmi_ls_oe */
	{GPMC_A18, (PTU | IEN | M3)},				/* gpio_42 */
	{GPMC_A19, (PTU | IEN | M3)},				/* gpio_43 - wifi_en */
	{GPMC_A20, (PTU | IEN | M3)},				/* gpio_44 - eth_irq */
	{GPMC_A21, (PTU | IEN | M3)},				/* gpio_45 - eth_nreset */
	{GPMC_A22, (PTU | IEN | M3)},				/* gpio_46 - eth_pme */
	{GPMC_A23, (PTU | IEN | M3)},				/* gpio_47 */
	{GPMC_A24, (PTU | IEN | M3)},				/* gpio_48 - eth_mdix */
	{GPMC_A25, (PTU | IEN | M3)},				/* gpio_49 - bt_wakeup */
	{GPMC_NCS0, (PTU | M0)},				/* gpmc_ncs0 */
	{GPMC_NCS1, (PTU | M0)},				/* gpmc_ncs1 */
	{GPMC_NCS2, (PTU | M0)},				/* gpmc_ncs2 */
	{GPMC_NCS3, (PTU | IEN | M3)},				/* gpio_53  */
	{C2C_DATA12, (PTU | M0)},				/* gpmc_ncs4 */
	{C2C_DATA13, (PTU | M0)},				/* gpmc_ncs5 - eth_cs */
	{GPMC_NWP, (PTU | IEN | M0)},				/* gpmc_nwp */
	{GPMC_CLK, (PTU | IEN | M0)},				/* gpmc_clk */
	{GPMC_NADV_ALE, (PTU | M0)},				/* gpmc_nadv_ale */
	{GPMC_NBE0_CLE, (PTU | M0)},				/* gpmc_nbe0_cle */
	{GPMC_NBE1, (PTU | M0)},				/* gpmc_nbe1 */
	{GPMC_WAIT0, (PTU | IEN | M0)},				/* gpmc_wait0 */
	{GPMC_WAIT1,  (PTU | IEN | M0)},			/* gpio_62 - usbh_nreset */
	{GPMC_NOE, (PTU | M0)},					/* gpmc_noe */
	{GPMC_NWE, (PTU | M0)},					/* gpmc_nwe */
	{HDMI_HPD, (PTD | IEN | M3)},				/* gpio_63 - hdmi_hpd */
	{HDMI_CEC, (PTU | IEN | M0)},				/* hdmi_cec */
	{HDMI_DDC_SCL, (M0)},					/* hdmi_ddc_scl */
	{HDMI_DDC_SDA, (IEN | M0)},				/* hdmi_ddc_sda */
	{CSI21_DX0, (IEN | M0)},				/* csi21_dx0 */
	{CSI21_DY0, (IEN | M0)},				/* csi21_dy0 */
	{CSI21_DX1, (IEN | M0)},				/* csi21_dx1 */
	{CSI21_DY1, (IEN | M0)},				/* csi21_dy1 */
	{CSI21_DX2, (IEN | M0)},				/* csi21_dx2 */
	{CSI21_DY2, (IEN | M0)},				/* csi21_dy2 */
	{CSI21_DX3, (IEN | M0)},				/* csi21_dx3 */
	{CSI21_DY3, (IEN | M0)},				/* csi21_dy3 */
	{CSI21_DX4, (IEN | M0)},				/* csi21_dx4 */
	{CSI21_DY4, (IEN | M0)},				/* csi21_dy4 */
	{CSI22_DX0, (IEN | M0)},				/* csi22_dx0 */
	{CSI22_DY0, (IEN | M0)},				/* csi22_dy0 */
	{CSI22_DX1, (IEN | M0)},				/* csi22_dx1 */
	{CSI22_DY1, (IEN | M0)},				/* csi22_dy1 */
	{USBB1_ULPITLL_CLK, (PTD | IEN | OFF_EN | OFF_PD | OFF_IN | M4)},/* usbb1_ulpiphy_clk */
	{USBB1_ULPITLL_STP, (OFF_EN | OFF_OUT_PTD | M4)},		/* usbb1_ulpiphy_stp */
	{USBB1_ULPITLL_DIR, (IEN | OFF_EN | OFF_PD | OFF_IN | M4)},	/* usbb1_ulpiphy_dir */
	{USBB1_ULPITLL_NXT, (IEN | OFF_EN | OFF_PD | OFF_IN | M4)},	/* usbb1_ulpiphy_nxt */
	{USBB1_ULPITLL_DAT0, (IEN | OFF_EN | OFF_PD | OFF_IN | M4)},	/* usbb1_ulpiphy_dat0 */
	{USBB1_ULPITLL_DAT1, (IEN | OFF_EN | OFF_PD | OFF_IN | M4)},	/* usbb1_ulpiphy_dat1 */
	{USBB1_ULPITLL_DAT2, (IEN | OFF_EN | OFF_PD | OFF_IN | M4)},	/* usbb1_ulpiphy_dat2 */
	{USBB1_ULPITLL_DAT3, (IEN | OFF_EN | OFF_PD | OFF_IN | M4)},	/* usbb1_ulpiphy_dat3 */
	{USBB1_ULPITLL_DAT4, (IEN | OFF_EN | OFF_PD | OFF_IN | M4)},	/* usbb1_ulpiphy_dat4 */
	{USBB1_ULPITLL_DAT5, (IEN | OFF_EN | OFF_PD | OFF_IN | M4)},	/* usbb1_ulpiphy_dat5 */
	{USBB1_ULPITLL_DAT6, (IEN | OFF_EN | OFF_PD | OFF_IN | M4)},	/* usbb1_ulpiphy_dat6 */
	{USBB1_ULPITLL_DAT7, (IEN | OFF_EN | OFF_PD | OFF_IN | M4)},	/* usbb1_ulpiphy_dat7 */
	{USBB1_HSIC_DATA, (PTU | IEN | M3)},				/* gpio_96 - usbh_cpen */
	{USBB1_HSIC_STROBE, (PTU | IEN | M3)},				/* gpio_97 - usbh_reset */
	{ABE_MCBSP2_CLKX, (IEN | OFF_EN | OFF_PD | OFF_IN | M0)},	/* abe_mcbsp2_clkx */
	{ABE_MCBSP2_DR, (IEN | OFF_EN | OFF_OUT_PTD | M0)},		/* abe_mcbsp2_dr */
	{ABE_MCBSP2_DX, (OFF_EN | OFF_OUT_PTD | M0)},			/* abe_mcbsp2_dx */
	{ABE_MCBSP2_FSX, (IEN | OFF_EN | OFF_PD | OFF_IN | M0)},	/* abe_mcbsp2_fsx */
	{ABE_PDM_UL_DATA, (PTD | IEN | OFF_EN | OFF_PD | OFF_IN | M0)},	/* abe_pdm_ul_data */
	{ABE_PDM_DL_DATA, (PTD | IEN | OFF_EN | OFF_PD | OFF_IN | M0)},	/* abe_pdm_dl_data */
	{ABE_PDM_FRAME, (PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M0)},	/* abe_pdm_frame */
	{ABE_PDM_LB_CLK, (PTD | IEN | OFF_EN | OFF_PD | OFF_IN | M0)},	/* abe_pdm_lb_clk */
	{ABE_CLKS, (PTD | IEN | OFF_EN | OFF_PD | OFF_IN | M0)},	/* abe_clks */
	{ABE_DMIC_CLK1, (M0)},						/* abe_dmic_clk1 */
	{ABE_DMIC_DIN1, (IEN | M0)},					/* abe_dmic_din1 */
	{ABE_DMIC_DIN2, (IEN | M0)},					/* abe_dmic_din2 */
	{ABE_DMIC_DIN3, (IEN | M0)},					/* abe_dmic_din3 */
	{UART2_CTS, (PTU | IEN | M0)},					/* uart2_cts */
	{UART2_RTS, (M0)},						/* uart2_rts */
	{UART2_RX, (PTU | IEN | M0)},					/* uart2_rx */
	{UART2_TX, (M0)},						/* uart2_tx */
	{HDQ_SIO, (M0)},						/* hdq-sio */
	{MCSPI1_CLK, (IEN | OFF_EN | OFF_PD | OFF_IN | M0)},		/* mcspi1_clk */
	{MCSPI1_SOMI, (IEN | OFF_EN | OFF_PD | OFF_IN | M0)},		/* mcspi1_somi */
	{MCSPI1_SIMO, (IEN | OFF_EN | OFF_PD | OFF_IN | M0)},		/* mcspi1_simo */
	{MCSPI1_CS0, (PTD | IEN | OFF_EN | OFF_PD | OFF_IN | M0)},	/* mcspi1_cs0 */
	{MCSPI1_CS1, (PTD | IEN | OFF_EN | OFF_PD | OFF_IN | M0)},	/* mcspi1_cs1 */
	{SDMMC5_CLK, (PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M0)},	/* sdmmc5_clk */
	{SDMMC5_CMD, (PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M0)},	/* sdmmc5_cmd */
	{SDMMC5_DAT0, (PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M0)},	/* sdmmc5_dat0 */
	{SDMMC5_DAT1, (PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M0)},	/* sdmmc5_dat1 */
	{SDMMC5_DAT2, (PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M0)},	/* sdmmc5_dat2 */
	{SDMMC5_DAT3, (PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M0)},	/* sdmmc5_dat3 */
	{MCSPI4_CLK, (IEN | OFF_EN | OFF_PD | OFF_IN | M0)},		/* mcspi4_clk */
	{MCSPI4_SIMO, (IEN | OFF_EN | OFF_PD | OFF_IN | M0)},		/* mcspi4_simo */
	{MCSPI4_SOMI, (IEN | OFF_EN | OFF_PD | OFF_IN | M0)},		/* mcspi4_somi */
	{MCSPI4_CS0, (PTD | IEN | OFF_EN | OFF_PD | OFF_IN | M0)},	/* mcspi4_cs0 */
	{UART4_RX, (IEN | PTU | M0)},					/* uart4_rx */
	{UART4_TX, (M0)},						/* uart4_tx */
	{USBB2_ULPITLL_CLK, (PTU | IEN | M3)},				/* gpio_157 - start_adc */
	{USBB2_ULPITLL_STP, (PTU | IEN | M3)},				/* gpio_158 - spi_nirq */
	{USBB2_ULPITLL_DIR, (PTU | IEN | M3)},				/* gpio_159 - bt_nreset */
	{USBB2_ULPITLL_NXT, (PTU | IEN | M3)},				/* gpio_160 - audio_pwron*/
	{USBB2_ULPITLL_DAT0, (PTU | IEN | M3)},				/* gpio_161 - bid_0 */
	{USBB2_ULPITLL_DAT1, (PTU | IEN | M3)},				/* gpio_162 - bid_1 */
	{USBB2_ULPITLL_DAT2, (PTU | IEN | M3)},				/* gpio_163 - bid_2 */
	{USBB2_ULPITLL_DAT3, (PTU | IEN | M3)},				/* gpio_164 - bid_3 */
	{USBB2_ULPITLL_DAT4, (PTU | IEN | M3)},				/* gpio_165 - bid_4 */
	{USBB2_ULPITLL_DAT5, (PTU | IEN | M3)},				/* gpio_166 - ts_irq*/
	{USBB2_ULPITLL_DAT6, (PTU | IEN | M3)},				/* gpio_167 - gps_pps */
	{USBB2_ULPITLL_DAT7, (PTU | IEN | M3)},				/* gpio_168 */
	{USBB2_HSIC_DATA, (PTU | IEN | M3)},				/* gpio_169 */
	{USBB2_HSIC_STROBE, (PTU | IEN | M3)},				/* gpio_170 */
	{UNIPRO_TX1, (PTU | IEN | M3)},					/* gpio_173 */
	{USBA0_OTG_CE, (PTD | OFF_EN | OFF_PD | OFF_OUT_PTD | M0)},	/* usba0_otg_ce */
	{USBA0_OTG_DP, (IEN | OFF_EN | OFF_PD | OFF_IN | M0)},		/* usba0_otg_dp */
	{USBA0_OTG_DM, (IEN | OFF_EN | OFF_PD | OFF_IN | M0)},		/* usba0_otg_dm */
	{SYS_NIRQ1, (PTU | IEN | M0)},					/* sys_nirq1 */
	{SYS_NIRQ2, (PTU | IEN | M0)},					/* sys_nirq2 */
	{SYS_BOOT0, (M0)},						/* sys_boot0 */
	{SYS_BOOT1, (M0)},						/* sys_boot1 */
	{SYS_BOOT2, (M0)},						/* sys_boot2 */
	{SYS_BOOT3, (M0)},						/* sys_boot3 */
	{SYS_BOOT4, (M0)},						/* sys_boot4 */
	{SYS_BOOT5, (M0)},						/* sys_boot5 */
	{DPM_EMU0, (IEN | M0)},						/* dpm_emu0 */
	{DPM_EMU1, (IEN | M0)},						/* dpm_emu1 */
	{DPM_EMU16, (PTU | IEN | M3)},					/* gpio_27 */
	{DPM_EMU17, (PTU | IEN | M3)},					/* gpio_28 */
	{DPM_EMU18, (PTU | IEN | M3)},					/* gpio_29 */
	{DPM_EMU19, (PTU | IEN | M3)},					/* gpio_30 */
};

const struct pad_conf_entry wkup_padconf_array_non_essential[] = {
	{PAD1_FREF_XTAL_IN, (M0)},					/* fref_xtal_in  */
	{PAD0_FREF_SLICER_IN, (M0)},					/* fref_slicer_in */
	{PAD1_FREF_CLK_IOREQ, (M0)},					/* fref_clk_ioreq */
	{PAD0_FREF_CLK0_OUT, (M7)},					/* safe mode */
	{PAD1_FREF_CLK3_REQ, M7},					/* safe mode */
	{PAD0_FREF_CLK3_OUT, (M0)},					/* fref_clk3_out */
	{PAD0_SYS_NRESPWRON, (M0)},					/* sys_nrespwron */
	{PAD1_SYS_NRESWARM, (M0)},					/* sys_nreswarm */
	{PAD0_SYS_PWR_REQ, (PTU | M0)},					/* sys_pwr_req */
	{PAD1_SYS_PWRON_RESET, (M3)},					/* gpio_wk29 */
	{PAD0_SYS_BOOT6, (M0)},						/* sys_boot6 */
	{PAD1_SYS_BOOT7, (M0)},						/* sys_boot7 */
};


#endif /* _DUOVERO_MUX_DATA_H_ */
