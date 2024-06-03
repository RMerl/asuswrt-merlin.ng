/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Amazon Kindle Fire (first generation) codename kc1 config
 *
 * Copyright (C) 2016 Paul Kocialkowski <contact@paulk.fr>
 */

#ifndef _KC1_H_
#define _KC1_H_

#include <asm/arch/mux_omap4.h>

#define KC1_GPIO_USB_ID		52
#define KC1_GPIO_MBID1		173
#define KC1_GPIO_MBID0		174
#define KC1_GPIO_MBID3		177
#define KC1_GPIO_MBID2		178

const struct pad_conf_entry core_padconf_array[] = {
	/* GPMC */
	{ GPMC_AD0,		(IEN  | PTU | M1) }, /* sdmmc2_dat0 */
	{ GPMC_AD1,		(IEN  | PTU | M1) }, /* sdmmc2_dat1 */
	{ GPMC_AD2,		(IEN  | PTU | M1) }, /* sdmmc2_dat2 */
	{ GPMC_AD3,		(IEN  | PTU | M1) }, /* sdmmc2_dat3 */
	{ GPMC_AD4,		(IEN  | PTU | M1) }, /* sdmmc2_dat4 */
	{ GPMC_AD5,		(IEN  | PTU | M1) }, /* sdmmc2_dat5 */
	{ GPMC_AD6,		(IEN  | PTU | M1) }, /* sdmmc2_dat6 */
	{ GPMC_AD7,		(IEN  | PTU | M1) }, /* sdmmc2_dat7 */
	{ GPMC_NOE,		(IEN  | PTU | M1) }, /* sdmmc2_clk */
	{ GPMC_NWE,		(IEN  | PTU | M1) }, /* sdmmc2_cmd */
	{ GPMC_NCS2,		(IEN  | PTD | M3) }, /* gpio_52 */
	/* CAM */
	{ CAM_SHUTTER,		(IDIS | DIS | M7) }, /* safe_mode */
	{ CAM_STROBE,		(IDIS | DIS | M7) }, /* safe_mode */
	{ CAM_GLOBALRESET,	(IDIS | DIS | M7) }, /* safe_mode */
	/* HDQ */
	{ HDQ_SIO,		(IDIS | DIS | M7) }, /* safe_mode */
	/* I2C1 */
	{ I2C1_SCL,		(IEN  | PTU | M0) }, /* i2c1_scl */
	{ I2C1_SDA,		(IEN  | PTU | M0) }, /* i2c1_sda */
	/* I2C2 */
	{ I2C2_SCL,		(IEN  | PTU | M0) }, /* i2c2_scl */
	{ I2C2_SDA,		(IEN  | PTU | M0) }, /* i2c2_sda */
	/* I2C3 */
	{ I2C3_SCL,		(IEN  | PTU | M0) }, /* i2c3_scl */
	{ I2C3_SDA,		(IEN  | PTU | M0) }, /* i2c3_sda */
	/* I2C4 */
	{ I2C4_SCL,		(IEN  | PTU | M0) }, /* i2c4_scl */
	{ I2C4_SDA,		(IEN  | PTU | M0) }, /* i2c4_sda */
	/* MCSPI1 */
	{ MCSPI1_CLK,		(IDIS | DIS | M7) }, /* safe_mode */
	{ MCSPI1_SOMI,		(IDIS | DIS | M7) }, /* safe_mode */
	{ MCSPI1_SIMO,		(IDIS | DIS | M7) }, /* safe_mode */
	{ MCSPI1_CS0,		(IDIS | DIS | M7) }, /* safe_mode */
	{ MCSPI1_CS1,		(IDIS | DIS | M7) }, /* safe_mode */
	{ MCSPI1_CS2,		(IDIS | DIS | M7) }, /* safe_mode */
	{ MCSPI1_CS3,		(IDIS | DIS | M7) }, /* safe_mode */
	/* UART3 */
	{ UART3_CTS_RCTX,	(IDIS | DIS | M7) }, /* safe_mode */
	{ UART3_RTS_SD,		(IDIS | DIS | M7) }, /* safe_mode */
	{ UART3_RX_IRRX,	(IEN  | DIS | M0) }, /* uart3_rx_irrx */
	{ UART3_TX_IRTX,	(IDIS | DIS | M0) }, /* uart3_tx_irtx */
	/* SDMMC5 */
	{ SDMMC5_CLK,		(IEN  | PTU | M0) }, /* sdmmc5_clk */
	{ SDMMC5_CMD,		(IEN  | PTU | M0) }, /* sdmmc5_cmd */
	{ SDMMC5_DAT0,		(IEN  | PTU | M0) }, /* sdmmc5_dat0 */
	{ SDMMC5_DAT1,		(IEN  | PTU | M0) }, /* sdmmc5_dat1 */
	{ SDMMC5_DAT2,		(IEN  | PTU | M0) }, /* sdmmc5_dat2 */
	{ SDMMC5_DAT3,		(IEN  | PTU | M0) }, /* sdmmc5_dat3 */
	/* MCSPI4 */
	{ MCSPI4_CLK,		(IEN  | DIS | M0) }, /* mcspi4_clk */
	{ MCSPI4_SIMO,		(IEN  | DIS | M0) }, /* mcspi4_simo */
	{ MCSPI4_SOMI,		(IEN  | DIS | M0) }, /* mcspi4_somi */
	{ MCSPI4_CS0,		(IEN  | PTD | M0) }, /* mcspi4_cs0 */
	/* UART4 */
	{ UART4_RX,		(IDIS | DIS | M4) }, /* gpio_155 */
	{ UART4_TX,		(IDIS | DIS | M7) }, /* safe_mode */
	/* UNIPRO */
	{ UNIPRO_TX0,		(IDIS | DIS | M7) }, /* safe_mode */
	{ UNIPRO_TY0,		(IDIS | DIS | M7) }, /* safe_mode */
	{ UNIPRO_TX1,		(IEN  | DIS | M3) }, /* gpio_173 */
	{ UNIPRO_TY1,		(IEN  | DIS | M3) }, /* gpio_174 */
	{ UNIPRO_TX2,		(IDIS | DIS | M7) }, /* safe_mode */
	{ UNIPRO_TY2,		(IDIS | DIS | M7) }, /* safe_mode */
	{ UNIPRO_RX0,		(IEN  | DIS | M3) }, /* gpio_175 */
	{ UNIPRO_RY0,		(IEN  | DIS | M3) }, /* gpio_176 */
	{ UNIPRO_RX1,		(IEN  | DIS | M3) }, /* gpio_177 */
	{ UNIPRO_RY1,		(IEN  | DIS | M3) }, /* gpio_178 */
	{ UNIPRO_RX2,		(IDIS | DIS | M7) }, /* safe_mode */
	{ UNIPRO_RY2,		(IDIS | DIS | M7) }, /* safe_mode */
	/* USBA0_OTG */
	{ USBA0_OTG_CE,		(IDIS | PTD | M0) }, /* usba0_otg_ce */
	{ USBA0_OTG_DP,		(IEN  | DIS | M0) }, /* usba0_otg_dp */
	{ USBA0_OTG_DM,		(IEN  | DIS | M0) }, /* usba0_otg_dm */
};

#endif
