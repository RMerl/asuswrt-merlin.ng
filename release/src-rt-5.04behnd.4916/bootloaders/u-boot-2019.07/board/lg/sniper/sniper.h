/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * LG Optimus Black codename sniper board
 *
 * Copyright (C) 2015 Paul Kocialkowski <contact@paulk.fr>
 */

#ifndef _SNIPER_H_
#define _SNIPER_H_

#include <asm/arch/mux.h>

#define MUX_SNIPER() \
	/* SDRC */ \
	MUX_VAL(CP(SDRC_D0),		(IEN  | PTD | DIS | M0)) /* sdrc_d0 */\
	MUX_VAL(CP(SDRC_D1),		(IEN  | PTD | DIS | M0)) /* sdrc_d1 */\
	MUX_VAL(CP(SDRC_D2),		(IEN  | PTD | DIS | M0)) /* sdrc_d2 */\
	MUX_VAL(CP(SDRC_D3),		(IEN  | PTD | DIS | M0)) /* sdrc_d3 */\
	MUX_VAL(CP(SDRC_D4),		(IEN  | PTD | DIS | M0)) /* sdrc_d4 */\
	MUX_VAL(CP(SDRC_D5),		(IEN  | PTD | DIS | M0)) /* sdrc_d5 */\
	MUX_VAL(CP(SDRC_D6),		(IEN  | PTD | DIS | M0)) /* sdrc_d6 */\
	MUX_VAL(CP(SDRC_D7),		(IEN  | PTD | DIS | M0)) /* sdrc_d7 */\
	MUX_VAL(CP(SDRC_D8),		(IEN  | PTD | DIS | M0)) /* sdrc_d8 */\
	MUX_VAL(CP(SDRC_D9),		(IEN  | PTD | DIS | M0)) /* sdrc_d9 */\
	MUX_VAL(CP(SDRC_D10),		(IEN  | PTD | DIS | M0)) /* sdrc_d10 */\
	MUX_VAL(CP(SDRC_D11),		(IEN  | PTD | DIS | M0)) /* sdrc_d11 */\
	MUX_VAL(CP(SDRC_D12),		(IEN  | PTD | DIS | M0)) /* sdrc_d12 */\
	MUX_VAL(CP(SDRC_D13),		(IEN  | PTD | DIS | M0)) /* sdrc_d13 */\
	MUX_VAL(CP(SDRC_D14),		(IEN  | PTD | DIS | M0)) /* sdrc_d14 */\
	MUX_VAL(CP(SDRC_D15),		(IEN  | PTD | DIS | M0)) /* sdrc_d15 */\
	MUX_VAL(CP(SDRC_D16),		(IEN  | PTD | DIS | M0)) /* sdrc_d16 */\
	MUX_VAL(CP(SDRC_D17),		(IEN  | PTD | DIS | M0)) /* sdrc_d17 */\
	MUX_VAL(CP(SDRC_D18),		(IEN  | PTD | DIS | M0)) /* sdrc_d18 */\
	MUX_VAL(CP(SDRC_D19),		(IEN  | PTD | DIS | M0)) /* sdrc_d19 */\
	MUX_VAL(CP(SDRC_D20),		(IEN  | PTD | DIS | M0)) /* sdrc_d20 */\
	MUX_VAL(CP(SDRC_D21),		(IEN  | PTD | DIS | M0)) /* sdrc_d21 */\
	MUX_VAL(CP(SDRC_D22),		(IEN  | PTD | DIS | M0)) /* sdrc_d22 */\
	MUX_VAL(CP(SDRC_D23),		(IEN  | PTD | DIS | M0)) /* sdrc_d23 */\
	MUX_VAL(CP(SDRC_D24),		(IEN  | PTD | DIS | M0)) /* sdrc_d24 */\
	MUX_VAL(CP(SDRC_D25),		(IEN  | PTD | DIS | M0)) /* sdrc_d25 */\
	MUX_VAL(CP(SDRC_D26),		(IEN  | PTD | DIS | M0)) /* sdrc_d26 */\
	MUX_VAL(CP(SDRC_D27),		(IEN  | PTD | DIS | M0)) /* sdrc_d27 */\
	MUX_VAL(CP(SDRC_D28),		(IEN  | PTD | DIS | M0)) /* sdrc_d28 */\
	MUX_VAL(CP(SDRC_D29),		(IEN  | PTD | DIS | M0)) /* sdrc_d29 */\
	MUX_VAL(CP(SDRC_D30),		(IEN  | PTD | DIS | M0)) /* sdrc_d30 */\
	MUX_VAL(CP(SDRC_D31),		(IEN  | PTD | DIS | M0)) /* sdrc_d31 */\
	MUX_VAL(CP(SDRC_CLK),		(IEN  | PTD | DIS | M0)) /* sdrc_clk */\
	MUX_VAL(CP(SDRC_DQS0),		(IEN  | PTD | DIS | M0)) /* sdrc_dqs0 */\
	MUX_VAL(CP(SDRC_DQS1),		(IEN  | PTD | DIS | M0)) /* sdrc_dqs1 */\
	MUX_VAL(CP(SDRC_DQS2),		(IEN  | PTD | DIS | M0)) /* sdrc_dqs2 */\
	MUX_VAL(CP(SDRC_DQS3),		(IEN  | PTD | DIS | M0)) /* sdrc_dqs3 */ \
	/* GPMC */ \
	MUX_VAL(CP(GPMC_A1),		(IDIS | PTD | DIS | M4)) /* gpio_34 */ \
	MUX_VAL(CP(GPMC_A2),		(IEN  | PTD | DIS | M4)) /* gpio_35 */ \
	MUX_VAL(CP(GPMC_A3),		(IDIS | PTD | DIS | M4)) /* gpio_36 */ \
	MUX_VAL(CP(GPMC_A4),		(IDIS | PTD | DIS | M4)) /* gpio_37 */\
	MUX_VAL(CP(GPMC_A5),		(IEN  | PTD | DIS | M4)) /* gpio_38 */\
	MUX_VAL(CP(GPMC_A6),		(IDIS | PTD | DIS | M4)) /* gpio_39 */\
	MUX_VAL(CP(GPMC_A7),		(IEN  | PTD | DIS | M4)) /* gpio_40 */\
	MUX_VAL(CP(GPMC_A8),		(IEN  | PTD | DIS | M4)) /* gpio_41 */\
	MUX_VAL(CP(GPMC_A9),		(IEN  | PTD | EN  | M4)) /* gpio_42 */\
	MUX_VAL(CP(GPMC_A10),		(IEN  | PTD | DIS | M4)) /* gpio_43 */\
	MUX_VAL(CP(GPMC_D0),		(IEN  | PTD | DIS | M0)) /* gpmc_d0 */ \
	MUX_VAL(CP(GPMC_D1),		(IEN  | PTD | DIS | M0)) /* gpmc_d1 */ \
	MUX_VAL(CP(GPMC_D2),		(IEN  | PTD | DIS | M0)) /* gpmc_d2 */ \
	MUX_VAL(CP(GPMC_D3),		(IEN  | PTD | DIS | M0)) /* gpmc_d3 */ \
	MUX_VAL(CP(GPMC_D4),		(IEN  | PTD | DIS | M0)) /* gpmc_d4 */ \
	MUX_VAL(CP(GPMC_D5),		(IEN  | PTD | DIS | M0)) /* gpmc_d5 */ \
	MUX_VAL(CP(GPMC_D6),		(IEN  | PTD | DIS | M0)) /* gpmc_d6 */ \
	MUX_VAL(CP(GPMC_D7),		(IEN  | PTD | DIS | M0)) /* gpmc_d7 */ \
	MUX_VAL(CP(GPMC_D8),		(IEN  | PTD | DIS | M0)) /* gpmc_d8 */ \
	MUX_VAL(CP(GPMC_D9),		(IEN  | PTD | DIS | M0)) /* gpmc_d9 */ \
	MUX_VAL(CP(GPMC_D10),		(IEN  | PTD | DIS | M0)) /* gpmc_d10 */ \
	MUX_VAL(CP(GPMC_D11),		(IEN  | PTD | DIS | M0)) /* gpmc_d11 */ \
	MUX_VAL(CP(GPMC_D12),		(IEN  | PTD | DIS | M0)) /* gpmc_d12 */ \
	MUX_VAL(CP(GPMC_D13),		(IEN  | PTD | DIS | M0)) /* gpmc_d13 */ \
	MUX_VAL(CP(GPMC_D14),		(IEN  | PTD | DIS | M0)) /* gpmc_d14 */ \
	MUX_VAL(CP(GPMC_NCS0),		(IDIS | PTD | DIS | M7)) \
	MUX_VAL(CP(GPMC_NCS1),		(IDIS | PTD | DIS | M4)) /* gpio_52 */ \
	MUX_VAL(CP(GPMC_NCS2),		(IEN  | PTD | DIS | M4)) /* gpio_53 */ \
	MUX_VAL(CP(GPMC_NCS3),		(IDIS | PTD | DIS | M4)) /* gpio_54 */ \
	MUX_VAL(CP(GPMC_NCS4),		(IDIS | PTD | DIS | M4)) /* gpio_55 */ \
	MUX_VAL(CP(GPMC_NCS5),		(IDIS | PTD | DIS | M3)) /* gpio_56 */ \
	MUX_VAL(CP(GPMC_NCS6),		(IDIS | PTD | DIS | M4)) /* gpio_57 */ \
	MUX_VAL(CP(GPMC_NCS7),		(IEN  | PTD | DIS | M4)) /* gpio_58 */ \
	MUX_VAL(CP(GPMC_CLK),		(IDIS | PTD | DIS | M7)) /* safe_mode */ \
	MUX_VAL(CP(GPMC_NADV_ALE),	(IDIS | PTD | DIS | M7)) \
	MUX_VAL(CP(GPMC_NOE),		(IDIS | PTD | DIS | M7)) \
	MUX_VAL(CP(GPMC_NWE),		(IDIS | PTD | DIS | M7)) \
	MUX_VAL(CP(GPMC_NBE0_CLE),	(IDIS | PTD | DIS | M4)) /* gpio_60 */ \
	MUX_VAL(CP(GPMC_NBE1),		(IDIS | PTD | DIS | M4)) /* gpio_61 */ \
	MUX_VAL(CP(GPMC_NWP),		(IDIS | PTD | DIS | M4)) /* gpio_62 */ \
	MUX_VAL(CP(GPMC_WAIT0),		(IEN  | PTU | EN  | M4)) \
	MUX_VAL(CP(GPMC_WAIT1),		(IEN  | PTD | DIS | M4)) /* gpio_63 */ \
	MUX_VAL(CP(GPMC_WAIT2),		(IDIS | PTD | DIS | M2)) /* gpio_64 */ \
	MUX_VAL(CP(GPMC_WAIT3),		(IEN  | PTD | DIS | M2)) /* gpio_65 */ \
	/* DSS */ \
	MUX_VAL(CP(DSS_PCLK),		(IEN  | PTD | EN  | M7)) /* safe_mode */ \
	MUX_VAL(CP(DSS_HSYNC),		(IEN  | PTD | EN  | M7)) /* safe_mode */ \
	MUX_VAL(CP(DSS_VSYNC),		(IEN  | PTD | EN  | M7)) /* safe_mode */ \
	MUX_VAL(CP(DSS_ACBIAS),		(IEN  | PTD | EN  | M7)) /* safe_mode */ \
	MUX_VAL(CP(DSS_DATA0),		(IDIS | PTD | DIS | M1)) /* dsi_dx0 */ \
	MUX_VAL(CP(DSS_DATA1),		(IDIS | PTD | DIS | M1)) /* dsi_dy0 */ \
	MUX_VAL(CP(DSS_DATA2),		(IDIS | PTD | DIS | M1)) /* dsi_dx1 */ \
	MUX_VAL(CP(DSS_DATA3),		(IDIS | PTD | DIS | M1)) /* dsi_dy1 */ \
	MUX_VAL(CP(DSS_DATA4),		(IDIS | PTD | DIS | M1)) /* dsi_dx2 */ \
	MUX_VAL(CP(DSS_DATA5),		(IDIS | PTD | DIS | M1)) /* dsi_dy2 */ \
	MUX_VAL(CP(DSS_DATA6),		(IEN  | PTD | EN  | M7)) /* safe_mode */ \
	MUX_VAL(CP(DSS_DATA7),		(IEN  | PTD | EN  | M7)) /* safe_mode */ \
	MUX_VAL(CP(DSS_DATA8),		(IEN  | PTD | EN  | M7)) /* safe_mode */ \
	MUX_VAL(CP(DSS_DATA9),		(IEN  | PTD | EN  | M7)) /* safe_mode */ \
	MUX_VAL(CP(DSS_DATA10),		(IEN  | PTD | EN  | M7)) /* safe_mode */ \
	MUX_VAL(CP(DSS_DATA11),		(IEN  | PTD | EN  | M7)) /* safe_mode */ \
	MUX_VAL(CP(DSS_DATA12),		(IEN  | PTD | EN  | M7)) /* safe_mode */ \
	MUX_VAL(CP(DSS_DATA13),		(IEN  | PTD | EN  | M7)) /* safe_mode */ \
	MUX_VAL(CP(DSS_DATA14),		(IEN  | PTD | EN  | M7)) /* safe_mode */ \
	MUX_VAL(CP(DSS_DATA15),		(IEN  | PTD | EN  | M7)) /* safe_mode */ \
	MUX_VAL(CP(DSS_DATA16),		(IEN  | PTD | EN  | M7)) /* safe_mode */ \
	MUX_VAL(CP(DSS_DATA17),		(IDIS | PTD | DIS | M4)) /* gpio_87 */ \
	MUX_VAL(CP(DSS_DATA18),		(IEN  | PTD | EN  | M7)) /* safe_mode */ \
	MUX_VAL(CP(DSS_DATA19),		(IEN  | PTD | EN  | M7)) /* safe_mode */ \
	MUX_VAL(CP(DSS_DATA20),		(IEN  | PTD | EN  | M7)) /* safe_mode */ \
	MUX_VAL(CP(DSS_DATA21),		(IEN  | PTD | EN  | M7)) /* safe_mode */ \
	MUX_VAL(CP(DSS_DATA22),		(IEN  | PTD | EN  | M7)) /* safe_mode */ \
	MUX_VAL(CP(DSS_DATA23),		(IEN  | PTD | EN  | M7)) /* safe_mode */ \
	/* CAM */ \
	MUX_VAL(CP(CAM_HS),		(IEN  | PTD | EN  | M0)) /* cam_hs */ \
	MUX_VAL(CP(CAM_VS),		(IEN  | PTD | EN  | M0)) /* cam_vs */ \
	MUX_VAL(CP(CAM_XCLKA),		(IDIS | PTD | DIS | M0)) /* cam_xclka */ \
	MUX_VAL(CP(CAM_PCLK),		(IEN  | PTD | EN  | M0)) /* cam_pclk */ \
	MUX_VAL(CP(CAM_FLD),		(IDIS | PTD | DIS | M4)) /* gpio_98 */ \
	MUX_VAL(CP(CAM_D0),		(IEN  | PTD | DIS | M2)) /* csi2_dx2 */ \
	MUX_VAL(CP(CAM_D1),		(IEN  | PTD | DIS | M2)) /* csi2_dy2 */ \
	MUX_VAL(CP(CAM_D2),		(IDIS | PTD | EN  | M4)) /* gpio_101 */ \
	MUX_VAL(CP(CAM_D3),		(IDIS | PTD | DIS | M7)) /* safe_mode */ \
	MUX_VAL(CP(CAM_D4),		(IEN  | PTD | DIS | M0)) /* cam_d4 */ \
	MUX_VAL(CP(CAM_D5),		(IEN  | PTD | DIS | M0)) /* cam_d5 */ \
	MUX_VAL(CP(CAM_D6),		(IEN  | PTD | DIS | M0)) /* cam_d6 */ \
	MUX_VAL(CP(CAM_D7),		(IEN  | PTD | DIS | M0)) /* cam_d7 */ \
	MUX_VAL(CP(CAM_D8),		(IEN  | PTD | DIS | M0)) /* cam_d8 */ \
	MUX_VAL(CP(CAM_D9),		(IEN  | PTD | DIS | M0)) /* cam_d9 */ \
	MUX_VAL(CP(CAM_D10),		(IEN  | PTD | DIS | M0)) /* cam_d10 */ \
	MUX_VAL(CP(CAM_D11),		(IEN  | PTD | DIS | M0)) /* cam_d11 */ \
	MUX_VAL(CP(CAM_XCLKB),		(IEN  | PTD | DIS | M0)) /* cam_xclkb */ \
	MUX_VAL(CP(CAM_WEN),		(IDIS | PTD | DIS | M4)) /* gpio_167 */ \
	MUX_VAL(CP(CAM_STROBE),		(IEN  | PTD | DIS | M7)) /* safe_mode */ \
	/* CSI2 */ \
	MUX_VAL(CP(CSI2_DX0),		(IEN  | PTD | DIS | M0)) /* csi2_dx0 */ \
	MUX_VAL(CP(CSI2_DY0),		(IEN  | PTD | DIS | M0)) /* csi2_dy0 */ \
	MUX_VAL(CP(CSI2_DX1),		(IEN  | PTD | DIS | M0)) /* csi2_dx1 */ \
	MUX_VAL(CP(CSI2_DY1),		(IEN  | PTD | DIS | M0)) /* csi2_dy1 */ \
	/* MCBSP2 */ \
	MUX_VAL(CP(MCBSP2_FSX),		(IEN  | PTD | DIS | M0)) /* mcbsp2_fsx */ \
	MUX_VAL(CP(MCBSP2_CLKX),	(IEN  | PTD | DIS | M0)) /* mcbsp2_clkx */ \
	MUX_VAL(CP(MCBSP2_DR),		(IEN  | PTD | DIS | M0)) /* mcbsp2_dr */ \
	MUX_VAL(CP(MCBSP2_DX),		(IDIS | PTD | DIS | M0)) /* mcbsp2_dx */ \
	/* MMC1 */ \
	MUX_VAL(CP(MMC1_CLK),		(IEN  | PTD | DIS | M0)) /* mmc1_clk */ \
	MUX_VAL(CP(MMC1_CMD),		(IEN  | PTD | DIS | M0)) /* mmc1_cmd */ \
	MUX_VAL(CP(MMC1_DAT0),		(IEN  | PTD | DIS | M0)) /* mmc1_dat0 */ \
	MUX_VAL(CP(MMC1_DAT1),		(IEN  | PTD | DIS | M0)) /* mmc1_dat1 */ \
	MUX_VAL(CP(MMC1_DAT2),		(IEN  | PTD | DIS | M0)) /* mmc1_dat2 */ \
	MUX_VAL(CP(MMC1_DAT3),		(IEN  | PTD | DIS | M0)) /* mmc1_dat3 */ \
	MUX_VAL(CP(MMC1_DAT4),		(IEN  | PTD | DIS | M7)) /* safe_mode */ \
	MUX_VAL(CP(MMC1_DAT5),		(IEN  | PTD | DIS | M7)) /* safe_mode */ \
	MUX_VAL(CP(MMC1_DAT6),		(IEN  | PTD | DIS | M7)) /* safe_mode */ \
	MUX_VAL(CP(MMC1_DAT7),		(IEN  | PTD | DIS | M7)) /* safe_mode */ \
	/* MMC2 */ \
	MUX_VAL(CP(MMC2_CLK),		(IEN  | PTD | DIS | M0)) /* mmc2_clk */ \
	MUX_VAL(CP(MMC2_CMD),		(IEN  | PTD | DIS | M0)) /* mmc2_cmd */ \
	MUX_VAL(CP(MMC2_DAT0),		(IEN  | PTD | DIS | M0)) /* mmc2_dat0 */ \
	MUX_VAL(CP(MMC2_DAT1),		(IEN  | PTD | DIS | M0)) /* mmc2_dat1 */ \
	MUX_VAL(CP(MMC2_DAT2),		(IEN  | PTD | DIS | M0)) /* mmc2_dat2 */ \
	MUX_VAL(CP(MMC2_DAT3),		(IEN  | PTD | DIS | M0)) /* mmc2_dat3 */ \
	MUX_VAL(CP(MMC2_DAT4),		(IEN  | PTD | DIS | M0)) /* mmc2_dat4 */ \
	MUX_VAL(CP(MMC2_DAT5),		(IEN  | PTD | DIS | M0)) /* mmc2_dat5 */ \
	MUX_VAL(CP(MMC2_DAT6),		(IEN  | PTD | DIS | M0)) /* mmc2_dat6 */ \
	MUX_VAL(CP(MMC2_DAT7),		(IEN  | PTD | DIS | M0)) /* mmc2_dat7 */ \
	/* MCBSP3 */ \
	MUX_VAL(CP(MCBSP3_DX),		(IDIS | PTD | DIS | M0)) /* mcbsp3_dx */ \
	MUX_VAL(CP(MCBSP3_DR),		(IEN  | PTD | DIS | M0)) /* mcbsp3_dr */ \
	MUX_VAL(CP(MCBSP3_CLKX),	(IEN  | PTD | DIS | M0)) /* mcbsp3_clkx */ \
	MUX_VAL(CP(MCBSP3_FSX),		(IEN  | PTD | DIS | M0)) /* mcbsp3_fsx */ \
	/* UART2 */ \
	MUX_VAL(CP(UART2_CTS),		(IEN  | PTD | DIS | M0)) /* uart2_cts */ \
	MUX_VAL(CP(UART2_RTS),		(IDIS | PTD | DIS | M0)) /* uart2_rts */ \
	MUX_VAL(CP(UART2_TX),		(IDIS | PTD | DIS | M0)) /* uart2_tx */ \
	MUX_VAL(CP(UART2_RX),		(IEN  | PTD | DIS | M0)) /* uart2_rx */ \
	/* UART1 */ \
	MUX_VAL(CP(UART1_TX),		(IDIS | PTD | DIS | M0)) /* uart1_tx */ \
	MUX_VAL(CP(UART1_RTS),		(IDIS | PTD | DIS | M0)) /* uart1_rts */ \
	MUX_VAL(CP(UART1_CTS),		(IEN  | PTD | DIS | M0)) /* uart1_cts */ \
	MUX_VAL(CP(UART1_RX),		(IEN  | PTD | DIS | M0)) /* uart1_rx */ \
	/* MCBSP4 */ \
	MUX_VAL(CP(MCBSP4_CLKX),	(IDIS | PTD | DIS | M4)) /* gpio_152 */ \
	MUX_VAL(CP(MCBSP4_DR),		(IDIS | PTD | DIS | M4)) /* gpio_153 */ \
	MUX_VAL(CP(MCBSP4_DX),		(IDIS | PTD | DIS | M4)) /* gpio_154 */ \
	MUX_VAL(CP(MCBSP4_FSX),		(IDIS | PTD | DIS | M4)) /* gpio_155 */ \
	/* MCBSP1 */ \
	MUX_VAL(CP(MCBSP1_CLKR),	(IEN  | PTD | DIS | M0)) /* mcbsp1_clkr */ \
	MUX_VAL(CP(MCBSP1_FSR),		(IEN  | PTD | DIS | M0)) /* mcbsp1_fsr */ \
	MUX_VAL(CP(MCBSP1_DX),		(IDIS | PTD | DIS | M0)) /* mcbsp1_dx */ \
	MUX_VAL(CP(MCBSP1_DR),		(IEN  | PTD | DIS | M0)) /* mcbsp1_dr */ \
	MUX_VAL(CP(MCBSP_CLKS),		(IDIS | PTD | DIS | M7)) /* safe_mode */ \
	MUX_VAL(CP(MCBSP1_FSX),		(IDIS | PTD | DIS | M4)) /* gpio_161 */ \
	MUX_VAL(CP(MCBSP1_CLKX),	(IDIS | PTD | DIS | M4)) /* gpio_162 */ \
	/* UART3 */ \
	MUX_VAL(CP(UART3_CTS_RCTX),	(IEN  | PTD | EN  | M4)) /* gpio_163 */ \
	MUX_VAL(CP(UART3_RTS_SD),	(IEN  | PTD | EN  | M7)) /* safe_mode */ \
	MUX_VAL(CP(UART3_RX_IRRX),	(IEN  | PTD | DIS | M0)) /* uart3_rx_irrx */ \
	MUX_VAL(CP(UART3_TX_IRTX),	(IDIS | PTD | DIS | M0)) /* uart3_tx_irtx */ \
	/* HSUSB0 */ \
	MUX_VAL(CP(HSUSB0_CLK),		(IEN  | PTD | EN  | M0)) /* hsusb0_clk */\
	MUX_VAL(CP(HSUSB0_STP),		(IDIS | PTD | DIS | M0)) /* hsusb0_stp */\
	MUX_VAL(CP(HSUSB0_DIR),		(IEN  | PTD | EN  | M0)) /* hsusb0_dir */\
	MUX_VAL(CP(HSUSB0_NXT),		(IEN  | PTD | EN  | M0)) /* hsusb0_nxt */\
	MUX_VAL(CP(HSUSB0_DATA0),	(IEN  | PTD | EN  | M0)) /* hsusb0_data0 */\
	MUX_VAL(CP(HSUSB0_DATA1),	(IEN  | PTD | EN  | M0)) /* hsusb0_data1 */\
	MUX_VAL(CP(HSUSB0_DATA2),	(IEN  | PTD | EN  | M0)) /* hsusb0_data2 */\
	MUX_VAL(CP(HSUSB0_DATA3),	(IEN  | PTD | EN  | M0)) /* hsusb0_data3 */\
	MUX_VAL(CP(HSUSB0_DATA4),	(IEN  | PTD | EN  | M0)) /* hsusb0_data4 */\
	MUX_VAL(CP(HSUSB0_DATA5),	(IEN  | PTD | EN  | M0)) /* hsusb0_data5 */\
	MUX_VAL(CP(HSUSB0_DATA6),	(IEN  | PTD | EN  | M0)) /* hsusb0_data6 */\
	MUX_VAL(CP(HSUSB0_DATA7),	(IEN  | PTD | EN  | M0)) /* hsusb0_data7 */ \
	/* I2C1 */ \
	MUX_VAL(CP(I2C1_SCL),		(IEN  | PTU | EN  | M0)) /* i2c1_scl */ \
	MUX_VAL(CP(I2C1_SDA),		(IEN  | PTU | EN  | M0)) /* i2c1_sda */ \
	/* I2C2 */ \
	MUX_VAL(CP(I2C2_SCL),		(IEN  | PTD | DIS | M0)) /* i2c2_scl */ \
	MUX_VAL(CP(I2C2_SDA),		(IEN  | PTD | DIS | M0)) /* i2c2_sda */ \
	/* I2C3 */ \
	MUX_VAL(CP(I2C3_SCL),		(IEN  | PTD | DIS | M0)) /* i2c3_scl */ \
	MUX_VAL(CP(I2C3_SDA),		(IEN  | PTD | DIS | M0)) /* i2c3_sda */ \
	/* I2C4 */ \
	MUX_VAL(CP(I2C4_SCL),		(IEN  | PTU | EN  | M0)) /* i2c4_scl */ \
	MUX_VAL(CP(I2C4_SDA),		(IEN  | PTU | EN  | M0)) /* i2c4_sda */ \
	/* HDQ */ \
	MUX_VAL(CP(HDQ_SIO),		(IEN  | PTD | EN  | M4)) /* gpio_170 */ \
	/* MCSPI1 */ \
	MUX_VAL(CP(MCSPI1_CLK),		(IEN  | PTD | EN  | M7)) /* safe_mode */ \
	MUX_VAL(CP(MCSPI1_SIMO),	(IEN  | PTD | EN  | M7)) /* safe_mode */ \
	MUX_VAL(CP(MCSPI1_SOMI),	(IEN  | PTD | EN  | M7)) /* safe_mode */ \
	MUX_VAL(CP(MCSPI1_CS0),		(IEN  | PTD | EN  | M7)) /* safe_mode */ \
	MUX_VAL(CP(MCSPI1_CS1),		(IEN  | PTD | DIS | M4)) /* gpio_175 */ \
	MUX_VAL(CP(MCSPI1_CS2),		(IEN  | PTD | DIS | M4)) /* gpio_176 */ \
	MUX_VAL(CP(MCSPI1_CS3),		(IDIS | PTD | DIS | M4)) /* gpio_177 */ \
	MUX_VAL(CP(MCSPI2_CLK),		(IEN  | PTD | EN  | M0)) /* mcspi2_clk */ \
	MUX_VAL(CP(MCSPI2_SIMO),	(IDIS | PTD | DIS | M0)) /* mcspi2_simo */ \
	MUX_VAL(CP(MCSPI2_SOMI),	(IEN  | PTD | DIS | M0)) /* mcspi2_somi */ \
	MUX_VAL(CP(MCSPI2_CS0),		(IDIS | PTD | DIS | M4)) /* gpio_181 */ \
	MUX_VAL(CP(MCSPI2_CS1),		(IDIS | PTD | DIS | M4)) /* gpio_182 */ \
	/* SYS */ \
	MUX_VAL(CP(SYS_32K),		(IEN  | PTD | DIS | M0)) /* sys_32k */ \
	MUX_VAL(CP(SYS_CLKREQ),		(IEN  | PTD | DIS | M0)) /* sys_clkreq */ \
	MUX_VAL(CP(SYS_NIRQ),		(IEN  | PTU | EN  | M0)) /* sys_nirq */ \
	MUX_VAL(CP(SYS_BOOT0),		(IEN  | PTU | EN  | M7)) /* safe_mode */ \
	MUX_VAL(CP(SYS_BOOT1),		(IEN  | PTD | EN  | M7)) /* safe_mode */ \
	MUX_VAL(CP(SYS_BOOT2),		(IEN  | PTU | EN  | M7)) /* safe_mode */ \
	MUX_VAL(CP(SYS_BOOT3),		(IEN  | PTD | EN  | M7)) /* safe_mode */ \
	MUX_VAL(CP(SYS_BOOT4),		(IEN  | PTD | EN  | M7)) /* safe_mode */ \
	MUX_VAL(CP(SYS_BOOT5),		(IEN  | PTD | EN  | M7)) /* safe_mode */ \
	MUX_VAL(CP(SYS_BOOT6),		(IEN  | PTU | EN  | M7)) /* safe_mode */ \
	MUX_VAL(CP(SYS_OFF_MODE),	(IDIS | PTD | DIS | M0)) /* sys_off_mode */ \
	MUX_VAL(CP(SYS_CLKOUT1),	(IEN  | PTD | DIS | M4)) /* gpio_10 */ \
	MUX_VAL(CP(SYS_CLKOUT2),	(IDIS | PTD | EN  | M7)) /* safe_mode */ \
	/* JTAG */ \
	MUX_VAL(CP(JTAG_NTRST),		(IEN  | PTD | DIS | M0)) /* jtag_ntrst */ \
	MUX_VAL(CP(JTAG_TCK),		(IEN  | PTD | DIS | M0)) /* jtag_tck */ \
	MUX_VAL(CP(JTAG_TMS),		(IEN  | PTU | EN  | M0)) /* jtag_tms */ \
	MUX_VAL(CP(JTAG_TDI),		(IEN  | PTU | EN  | M0)) /* jtag_tdi */ \
	MUX_VAL(CP(JTAG_EMU0),		(IEN  | PTD | DIS | M0)) /* jtag_emu0 */ \
	MUX_VAL(CP(JTAG_EMU1),		(IEN  | PTD | DIS | M0)) /* jtag_emu1 */ \
	/* ETK */ \
	MUX_VAL(CP(ETK_CLK_ES2),	(IEN  | PTD | DIS | M2)) /* sdmmc3_clk */ \
	MUX_VAL(CP(ETK_CTL_ES2),	(IEN  | PTU | EN  | M2)) /* sdmmc3_cmd */ \
	MUX_VAL(CP(ETK_D0_ES2),		(IEN  | PTD | EN  | M4)) /* gpio_14 */ \
	MUX_VAL(CP(ETK_D1_ES2),		(IEN  | PTD | DIS | M4)) /* gpio_15 */ \
	MUX_VAL(CP(ETK_D2_ES2),		(IEN  | PTD | DIS | M4)) /* gpio_16 */ \
	MUX_VAL(CP(ETK_D3_ES2),		(IEN  | PTD | DIS | M2)) /* sdmmc3_dat3 */ \
	MUX_VAL(CP(ETK_D4_ES2),		(IEN  | PTD | DIS | M2)) /* sdmmc3_dat0 */ \
	MUX_VAL(CP(ETK_D5_ES2),		(IEN  | PTD | DIS | M2)) /* sdmmc3_dat1 */ \
	MUX_VAL(CP(ETK_D6_ES2),		(IEN  | PTD | DIS | M2)) /* sdmmc3_dat2 */ \
	MUX_VAL(CP(ETK_D7_ES2),		(IEN  | PTD | EN  | M4)) /* gpio_21 */ \
	MUX_VAL(CP(ETK_D8_ES2),		(IDIS | PTD | DIS | M4)) /* gpio_22 */ \
	MUX_VAL(CP(ETK_D9_ES2),		(IDIS | PTD | DIS | M4)) /* gpio_23 */ \
	MUX_VAL(CP(ETK_D10_ES2),	(IEN  | PTD | EN  | M4)) /* gpio_24 */ \
	MUX_VAL(CP(ETK_D11_ES2),	(IDIS | PTD | DIS | M4)) /* gpio_25 */ \
	MUX_VAL(CP(ETK_D12_ES2),	(IDIS | PTD | DIS | M4)) /* gpio_26 */ \
	MUX_VAL(CP(ETK_D13_ES2),	(IDIS | PTD | DIS | M4)) /* gpio_27 */ \
	MUX_VAL(CP(ETK_D14_ES2),	(IEN  | PTU | EN  | M4)) /* gpio_28 */ \
	MUX_VAL(CP(ETK_D15_ES2),	(IEN  | PTU | EN  | M4)) /* gpio_29 */ \
	/* D2D */ \
	MUX_VAL(CP(D2D_MCAD0),		(IEN  | PTD | EN  | M0)) /* d2d_mcad0 */ \
	MUX_VAL(CP(D2D_MCAD1),		(IEN  | PTD | EN  | M0)) /* d2d_mcad1 */ \
	MUX_VAL(CP(D2D_MCAD2),		(IEN  | PTD | EN  | M0)) /* d2d_mcad2 */ \
	MUX_VAL(CP(D2D_MCAD3),		(IEN  | PTD | EN  | M0)) /* d2d_mcad3 */ \
	MUX_VAL(CP(D2D_MCAD4),		(IEN  | PTD | EN  | M0)) /* d2d_mcad4 */ \
	MUX_VAL(CP(D2D_MCAD5),		(IEN  | PTD | EN  | M0)) /* d2d_mcad5 */ \
	MUX_VAL(CP(D2D_MCAD6),		(IEN  | PTD | EN  | M0)) /* d2d_mcad6 */ \
	MUX_VAL(CP(D2D_MCAD7),		(IEN  | PTD | EN  | M0)) /* d2d_mcad7 */ \
	MUX_VAL(CP(D2D_MCAD8),		(IEN  | PTD | EN  | M0)) /* d2d_mcad8 */ \
	MUX_VAL(CP(D2D_MCAD9),		(IEN  | PTD | EN  | M0)) /* d2d_mcad9 */ \
	MUX_VAL(CP(D2D_MCAD10),		(IEN  | PTD | EN  | M0)) /* d2d_mcad10 */ \
	MUX_VAL(CP(D2D_MCAD11),		(IEN  | PTD | EN  | M0)) /* d2d_mcad11 */ \
	MUX_VAL(CP(D2D_MCAD12),		(IEN  | PTD | EN  | M0)) /* d2d_mcad12 */ \
	MUX_VAL(CP(D2D_MCAD13),		(IEN  | PTD | EN  | M0)) /* d2d_mcad13 */ \
	MUX_VAL(CP(D2D_MCAD14),		(IEN  | PTD | EN  | M0)) /* d2d_mcad14 */ \
	MUX_VAL(CP(D2D_MCAD15),		(IEN  | PTD | EN  | M0)) /* d2d_mcad15 */ \
	MUX_VAL(CP(D2D_MCAD16),		(IEN  | PTD | EN  | M0)) /* d2d_mcad16 */ \
	MUX_VAL(CP(D2D_MCAD17),		(IEN  | PTD | EN  | M0)) /* d2d_mcad17 */ \
	MUX_VAL(CP(D2D_MCAD18),		(IEN  | PTD | EN  | M0)) /* d2d_mcad18 */ \
	MUX_VAL(CP(D2D_MCAD19),		(IEN  | PTD | EN  | M0)) /* d2d_mcad19 */ \
	MUX_VAL(CP(D2D_MCAD20),		(IEN  | PTD | EN  | M0)) /* d2d_mcad20 */ \
	MUX_VAL(CP(D2D_MCAD21),		(IEN  | PTD | EN  | M0)) /* d2d_mcad21 */ \
	MUX_VAL(CP(D2D_MCAD22),		(IEN  | PTD | EN  | M0)) /* d2d_mcad22 */ \
	MUX_VAL(CP(D2D_MCAD23),		(IEN  | PTD | EN  | M0)) /* d2d_mcad23 */ \
	MUX_VAL(CP(D2D_MCAD24),		(IEN  | PTD | EN  | M0)) /* d2d_mcad24 */ \
	MUX_VAL(CP(D2D_MCAD25),		(IEN  | PTD | EN  | M0)) /* d2d_mcad25 */ \
	MUX_VAL(CP(D2D_MCAD26),		(IEN  | PTD | EN  | M0)) /* d2d_mcad26 */ \
	MUX_VAL(CP(D2D_MCAD27),		(IEN  | PTD | EN  | M0)) /* d2d_mcad27 */ \
	MUX_VAL(CP(D2D_MCAD28),		(IEN  | PTD | EN  | M0)) /* d2d_mcad28 */ \
	MUX_VAL(CP(D2D_MCAD29),		(IEN  | PTD | EN  | M0)) /* d2d_mcad29 */ \
	MUX_VAL(CP(D2D_MCAD30),		(IEN  | PTD | EN  | M0)) /* d2d_mcad30 */ \
	MUX_VAL(CP(D2D_MCAD31),		(IEN  | PTD | EN  | M0)) /* d2d_mcad31 */ \
	MUX_VAL(CP(D2D_MCAD32),		(IEN  | PTD | EN  | M0)) /* d2d_mcad32 */ \
	MUX_VAL(CP(D2D_MCAD33),		(IEN  | PTD | EN  | M0)) /* d2d_mcad33 */ \
	MUX_VAL(CP(D2D_MCAD34),		(IEN  | PTD | EN  | M0)) /* d2d_mcad34 */ \
	MUX_VAL(CP(D2D_MCAD35),		(IEN  | PTD | EN  | M0)) /* d2d_mcad35 */ \
	MUX_VAL(CP(D2D_MCAD36),		(IEN  | PTD | EN  | M0)) /* d2d_mcad36 */ \
	MUX_VAL(CP(D2D_CLK26MI),	(IDIS | PTD | DIS | M0)) /* d2d_clk26mi */ \
	MUX_VAL(CP(D2D_NRESPWRON),	(IEN  | PTU | EN  | M0)) /* d2d_nrespwron */ \
	MUX_VAL(CP(D2D_NRESWARM),	(IDIS | PTD | DIS | M0)) /* d2d_nreswarm */ \
	MUX_VAL(CP(D2D_ARM9NIRQ),	(IDIS | PTD | DIS | M0)) /* d2d_arm9nirq */ \
	MUX_VAL(CP(D2D_UMA2P6FIQ),	(IDIS | PTD | DIS | M0)) /* d2d_uma2p6fiq */ \
	MUX_VAL(CP(D2D_SPINT),		(IEN  | PTD | DIS | M0)) /* d2d_spint */ \
	MUX_VAL(CP(D2D_FRINT),		(IEN  | PTD | DIS | M0)) /* d2d_frint */ \
	MUX_VAL(CP(D2D_DMAREQ0),	(IDIS | PTD | DIS | M0)) /* d2d_dmareq0 */ \
	MUX_VAL(CP(D2D_DMAREQ1),	(IDIS | PTD | DIS | M0)) /* d2d_dmareq1 */ \
	MUX_VAL(CP(D2D_DMAREQ2),	(IDIS | PTD | DIS | M0)) /* d2d_dmareq2 */ \
	MUX_VAL(CP(D2D_DMAREQ3),	(IDIS | PTD | DIS | M0)) /* d2d_dmareq3 */ \
	MUX_VAL(CP(D2D_N3GTRST),	(IEN  | PTD | DIS | M0)) /* d2d_n3gtrst */ \
	MUX_VAL(CP(D2D_N3GTDI),		(IEN  | PTU | EN  | M0)) /* d2d_n3gtdi */ \
	MUX_VAL(CP(D2D_N3GTDO),		(IDIS | PTD | DIS | M0)) /* d2d_n3gtdo */ \
	MUX_VAL(CP(D2D_N3GTMS),		(IEN  | PTU | EN  | M0)) /* d2d_n3gtms */ \
	MUX_VAL(CP(D2D_N3GTCK),		(IEN  | PTD | DIS | M0)) /* d2d_n3gtck */ \
	MUX_VAL(CP(D2D_N3GRTCK),	(IEN  | PTD | DIS | M0)) /* d2d_n3grtck */ \
	MUX_VAL(CP(D2D_MSTDBY),		(IEN  | PTU | EN  | M0)) /* d2d_mstdby */ \
	MUX_VAL(CP(D2D_SWAKEUP),	(IEN  | PTD | EN  | M0)) /* d2d_swakeup */ \
	MUX_VAL(CP(D2D_IDLEREQ),	(IEN  | PTD | DIS | M0)) /* d2d_idlereq */ \
	MUX_VAL(CP(D2D_IDLEACK),	(IEN  | PTU | EN  | M0)) /* d2d_idleack */ \
	MUX_VAL(CP(D2D_MWRITE),		(IEN  | PTD | DIS | M0)) /* d2d_mwrite */ \
	MUX_VAL(CP(D2D_SWRITE),		(IEN  | PTD | DIS | M0)) /* d2d_swrite */ \
	MUX_VAL(CP(D2D_MREAD),		(IEN  | PTD | DIS | M0)) /* d2d_mread */ \
	MUX_VAL(CP(D2D_SREAD),		(IEN  | PTD | DIS | M0)) /* d2d_sread */ \
	MUX_VAL(CP(D2D_MBUSFLAG),	(IEN  | PTD | DIS | M0)) /* d2d_mbusflag */ \
	MUX_VAL(CP(D2D_SBUSFLAG),	(IEN  | PTD | DIS | M0)) /* d2d_sbusflag */ \
	MUX_VAL(CP(SDRC_CKE0),		(IDIS | PTD | DIS | M0)) /* sdrc_cke0 */ \
	MUX_VAL(CP(SDRC_CKE1),		(IDIS | PTD | DIS | M0)) /* sdrc_cke1 */ \
	MUX_VAL(CP(GPIO127),		(IEN  | PTD | DIS | M7)) /* safe_mode */ \
	MUX_VAL(CP(GPIO126),		(IDIS | PTD | DIS | M4)) /* gpio_126 */ \
	MUX_VAL(CP(GPIO128),		(IDIS | PTD | DIS | M4)) /* gpio_128 */ \
	MUX_VAL(CP(GPIO129),		(IEN  | PTD | DIS | M4)) /* gpio_129 */

#endif
