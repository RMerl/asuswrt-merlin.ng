/*
 * pinctrl pads, groups, functions for CSR SiRFatlasVI
 *
 * Copyright (c) 2011 - 2014 Cambridge Silicon Radio Limited, a CSR plc group
 * company.
 *
 * Licensed under GPLv2 or later.
 */

#include <linux/pinctrl/pinctrl.h>
#include <linux/bitops.h>

#include "pinctrl-sirf.h"

/*
 * pad list for the pinmux subsystem
 * refer to atlasVI_io_table_v0.93.xls
 */
static const struct pinctrl_pin_desc sirfsoc_pads[] = {
	PINCTRL_PIN(0, "gpio0-0"),
	PINCTRL_PIN(1, "gpio0-1"),
	PINCTRL_PIN(2, "gpio0-2"),
	PINCTRL_PIN(3, "gpio0-3"),
	PINCTRL_PIN(4, "pwm0"),
	PINCTRL_PIN(5, "pwm1"),
	PINCTRL_PIN(6, "pwm2"),
	PINCTRL_PIN(7, "pwm3"),
	PINCTRL_PIN(8, "warm_rst_b"),
	PINCTRL_PIN(9, "odo_0"),
	PINCTRL_PIN(10, "odo_1"),
	PINCTRL_PIN(11, "dr_dir"),
	PINCTRL_PIN(12, "rts_0"),
	PINCTRL_PIN(13, "scl_1"),
	PINCTRL_PIN(14, "ntrst"),
	PINCTRL_PIN(15, "sda_1"),
	PINCTRL_PIN(16, "x_ldd[16]"),
	PINCTRL_PIN(17, "x_ldd[17]"),
	PINCTRL_PIN(18, "x_ldd[18]"),
	PINCTRL_PIN(19, "x_ldd[19]"),
	PINCTRL_PIN(20, "x_ldd[20]"),
	PINCTRL_PIN(21, "x_ldd[21]"),
	PINCTRL_PIN(22, "x_ldd[22]"),
	PINCTRL_PIN(23, "x_ldd[23]"),
	PINCTRL_PIN(24, "gps_sgn"),
	PINCTRL_PIN(25, "gps_mag"),
	PINCTRL_PIN(26, "gps_clk"),
	PINCTRL_PIN(27,	"sd_cd_b_2"),
	PINCTRL_PIN(28, "sd_vcc_on_2"),
	PINCTRL_PIN(29, "sd_wp_b_2"),
	PINCTRL_PIN(30, "sd_clk_3"),
	PINCTRL_PIN(31, "sd_cmd_3"),

	PINCTRL_PIN(32, "x_sd_dat_3[0]"),
	PINCTRL_PIN(33, "x_sd_dat_3[1]"),
	PINCTRL_PIN(34, "x_sd_dat_3[2]"),
	PINCTRL_PIN(35, "x_sd_dat_3[3]"),
	PINCTRL_PIN(36, "usb_clk"),
	PINCTRL_PIN(37, "usb_dir"),
	PINCTRL_PIN(38, "usb_nxt"),
	PINCTRL_PIN(39, "usb_stp"),
	PINCTRL_PIN(40, "usb_dat[7]"),
	PINCTRL_PIN(41, "usb_dat[6]"),
	PINCTRL_PIN(42, "x_cko_1"),
	PINCTRL_PIN(43, "spi_clk_1"),
	PINCTRL_PIN(44, "spi_dout_1"),
	PINCTRL_PIN(45, "spi_din_1"),
	PINCTRL_PIN(46, "spi_en_1"),
	PINCTRL_PIN(47, "x_txd_1"),
	PINCTRL_PIN(48, "x_txd_2"),
	PINCTRL_PIN(49, "x_rxd_1"),
	PINCTRL_PIN(50, "x_rxd_2"),
	PINCTRL_PIN(51, "x_usclk_0"),
	PINCTRL_PIN(52, "x_utxd_0"),
	PINCTRL_PIN(53, "x_urxd_0"),
	PINCTRL_PIN(54, "x_utfs_0"),
	PINCTRL_PIN(55, "x_urfs_0"),
	PINCTRL_PIN(56, "usb_dat5"),
	PINCTRL_PIN(57, "usb_dat4"),
	PINCTRL_PIN(58, "usb_dat3"),
	PINCTRL_PIN(59, "usb_dat2"),
	PINCTRL_PIN(60, "usb_dat1"),
	PINCTRL_PIN(61, "usb_dat0"),
	PINCTRL_PIN(62, "x_ldd[14]"),
	PINCTRL_PIN(63, "x_ldd[15]"),

	PINCTRL_PIN(64, "x_gps_gpio"),
	PINCTRL_PIN(65, "x_ldd[13]"),
	PINCTRL_PIN(66, "x_df_we_b"),
	PINCTRL_PIN(67, "x_df_re_b"),
	PINCTRL_PIN(68, "x_txd_0"),
	PINCTRL_PIN(69, "x_rxd_0"),
	PINCTRL_PIN(70, "x_l_lck"),
	PINCTRL_PIN(71, "x_l_fck"),
	PINCTRL_PIN(72, "x_l_de"),
	PINCTRL_PIN(73, "x_ldd[0]"),
	PINCTRL_PIN(74, "x_ldd[1]"),
	PINCTRL_PIN(75, "x_ldd[2]"),
	PINCTRL_PIN(76, "x_ldd[3]"),
	PINCTRL_PIN(77, "x_ldd[4]"),
	PINCTRL_PIN(78, "x_cko_0"),
	PINCTRL_PIN(79, "x_ldd[5]"),
	PINCTRL_PIN(80, "x_ldd[6]"),
	PINCTRL_PIN(81, "x_ldd[7]"),
	PINCTRL_PIN(82, "x_ldd[8]"),
	PINCTRL_PIN(83, "x_ldd[9]"),
	PINCTRL_PIN(84, "x_ldd[10]"),
	PINCTRL_PIN(85, "x_ldd[11]"),
	PINCTRL_PIN(86, "x_ldd[12]"),
	PINCTRL_PIN(87, "x_vip_vsync"),
	PINCTRL_PIN(88, "x_vip_hsync"),
	PINCTRL_PIN(89, "x_vip_pxclk"),
	PINCTRL_PIN(90, "x_sda_0"),
	PINCTRL_PIN(91, "x_scl_0"),
	PINCTRL_PIN(92, "x_df_ry_by"),
	PINCTRL_PIN(93, "x_df_cs_b[1]"),
	PINCTRL_PIN(94, "x_df_cs_b[0]"),
	PINCTRL_PIN(95, "x_l_pclk"),

	PINCTRL_PIN(96, "x_df_dqs"),
	PINCTRL_PIN(97, "x_df_wp_b"),
	PINCTRL_PIN(98, "ac97_sync"),
	PINCTRL_PIN(99, "ac97_bit_clk "),
	PINCTRL_PIN(100, "ac97_dout"),
	PINCTRL_PIN(101, "ac97_din"),
	PINCTRL_PIN(102, "x_rtc_io"),

	PINCTRL_PIN(103, "x_usb1_dp"),
	PINCTRL_PIN(104, "x_usb1_dn"),
};

static const struct sirfsoc_muxmask lcd_16bits_sirfsoc_muxmask[] = {
	{
		.group = 1,
		.mask = BIT(30) | BIT(31),
	}, {
		.group = 2,
		.mask = BIT(1) | BIT(6) | BIT(7) | BIT(8) | BIT(9) |
			BIT(10) | BIT(11) | BIT(12) | BIT(13) | BIT(15) |
			BIT(16) | BIT(17) | BIT(18) | BIT(19) |
			BIT(20) | BIT(21) | BIT(22) | BIT(31),
	},
};

static const struct sirfsoc_padmux lcd_16bits_padmux = {
	.muxmask_counts = ARRAY_SIZE(lcd_16bits_sirfsoc_muxmask),
	.muxmask = lcd_16bits_sirfsoc_muxmask,
	.ctrlreg = SIRFSOC_RSC_PIN_MUX,
	.funcmask = BIT(4),
	.funcval = 0,
};

static const unsigned lcd_16bits_pins[] = { 62, 63, 65, 70, 71, 72, 73, 74, 75,
	76, 77, 79, 80, 81, 82, 83, 84, 85, 86, 95 };

static const struct sirfsoc_muxmask lcd_18bits_muxmask[] = {
	{
		.group = 2,
		.mask = BIT(1) | BIT(6) | BIT(7) | BIT(8) | BIT(9) |
			BIT(10) | BIT(11) | BIT(12) | BIT(13) | BIT(15) |
			BIT(16) | BIT(17) | BIT(18) | BIT(19) |
			BIT(20) | BIT(21) | BIT(22) | BIT(31),
	}, {
		.group = 1,
		.mask = BIT(30) | BIT(31),
	}, {
		.group = 0,
		.mask = BIT(16) | BIT(17),
	},
};

static const struct sirfsoc_padmux lcd_18bits_padmux = {
	.muxmask_counts = ARRAY_SIZE(lcd_18bits_muxmask),
	.muxmask = lcd_18bits_muxmask,
	.ctrlreg = SIRFSOC_RSC_PIN_MUX,
	.funcmask = BIT(4) | BIT(15),
	.funcval = 0,
};

static const unsigned lcd_18bits_pins[] = { 16, 17, 62, 63, 65, 70, 71, 72, 73,
	74, 75, 76, 77, 79, 80, 81, 82, 83, 84, 85, 86, 95 };

static const struct sirfsoc_muxmask lcd_24bits_muxmask[] = {
	{
		.group = 2,
		.mask = BIT(1) | BIT(6) | BIT(7) | BIT(8) | BIT(9) |
			BIT(10) | BIT(11) | BIT(12) | BIT(13) | BIT(15) |
			BIT(16) | BIT(17) | BIT(18) | BIT(19) |
			BIT(20) | BIT(21) | BIT(22) | BIT(31),
	}, {
		.group = 1,
		.mask = BIT(30) | BIT(31),
	}, {
		.group = 0,
		.mask = BIT(16) | BIT(17) | BIT(18) | BIT(19) | BIT(20) |
			BIT(21) | BIT(22) | BIT(23),
	},
};

static const struct sirfsoc_padmux lcd_24bits_padmux = {
	.muxmask_counts = ARRAY_SIZE(lcd_24bits_muxmask),
	.muxmask = lcd_24bits_muxmask,
	.ctrlreg = SIRFSOC_RSC_PIN_MUX,
	.funcmask = BIT(4) | BIT(15),
	.funcval = 0,
};

static const unsigned lcd_24bits_pins[] = { 16, 17, 18, 19, 20, 21, 22, 23, 62,
	63, 65, 70, 71, 72, 73, 74, 75, 76, 77, 79, 80, 81, 82, 83, 84,
	85, 86, 95};

static const struct sirfsoc_muxmask lcdrom_muxmask[] = {
	{
		.group = 2,
		.mask = BIT(1) | BIT(6) | BIT(7) | BIT(8) | BIT(9) | BIT(10) |
			BIT(11) | BIT(12) | BIT(13) | BIT(15) | BIT(16) |
			BIT(17) | BIT(18) | BIT(19) |
			BIT(20) | BIT(21) | BIT(22) | BIT(31),
	}, {
		.group = 1,
		.mask = BIT(30) | BIT(31),
	}, {
		.group = 0,
		.mask = BIT(8),
	},
};

static const struct sirfsoc_padmux lcdrom_padmux = {
	.muxmask_counts = ARRAY_SIZE(lcdrom_muxmask),
	.muxmask = lcdrom_muxmask,
	.ctrlreg = SIRFSOC_RSC_PIN_MUX,
	.funcmask = BIT(4),
	.funcval = BIT(4),
};

static const unsigned lcdrom_pins[] = { 8, 62, 63, 65, 70, 71, 72, 73, 74, 75,
	76, 77, 79, 80, 81, 82, 83, 84, 85, 86, 95};

static const struct sirfsoc_muxmask uart0_muxmask[] = {
	{
		.group = 0,
		.mask = BIT(12),
	}, {
		.group = 1,
		.mask = BIT(23),
	}, {
		.group = 2,
		.mask = BIT(4) | BIT(5),
	},
};

static const struct sirfsoc_padmux uart0_padmux = {
	.muxmask_counts = ARRAY_SIZE(uart0_muxmask),
	.muxmask = uart0_muxmask,
	.ctrlreg = SIRFSOC_RSC_PIN_MUX,
	.funcmask = BIT(9),
	.funcval = BIT(9),
};

static const unsigned uart0_pins[] = { 12, 55, 68, 69 };

static const struct sirfsoc_muxmask uart0_nostreamctrl_muxmask[] = {
	{
		.group = 2,
		.mask = BIT(4) | BIT(5),
	},
};

static const struct sirfsoc_padmux uart0_nostreamctrl_padmux = {
	.muxmask_counts = ARRAY_SIZE(uart0_nostreamctrl_muxmask),
	.muxmask = uart0_nostreamctrl_muxmask,
};

static const unsigned uart0_nostreamctrl_pins[] = { 68, 69 };

static const struct sirfsoc_muxmask uart1_muxmask[] = {
	{
		.group = 1,
		.mask = BIT(15) | BIT(17),
	},
};

static const struct sirfsoc_padmux uart1_padmux = {
	.muxmask_counts = ARRAY_SIZE(uart1_muxmask),
	.muxmask = uart1_muxmask,
};

static const unsigned uart1_pins[] = { 47, 49 };

static const struct sirfsoc_muxmask uart2_muxmask[] = {
	{
		.group = 0,
		.mask = BIT(10) | BIT(14),
	}, {
		.group = 1,
		.mask = BIT(16) | BIT(18),
	},
};

static const struct sirfsoc_padmux uart2_padmux = {
	.muxmask_counts = ARRAY_SIZE(uart2_muxmask),
	.muxmask = uart2_muxmask,
	.ctrlreg = SIRFSOC_RSC_PIN_MUX,
	.funcmask = BIT(10),
	.funcval = BIT(10),
};

static const unsigned uart2_pins[] = { 10, 14, 48, 50 };

static const struct sirfsoc_muxmask uart2_nostreamctrl_muxmask[] = {
	{
		.group = 1,
		.mask = BIT(16) | BIT(18),
	},
};

static const struct sirfsoc_padmux uart2_nostreamctrl_padmux = {
	.muxmask_counts = ARRAY_SIZE(uart2_nostreamctrl_muxmask),
	.muxmask = uart2_nostreamctrl_muxmask,
};

static const unsigned uart2_nostreamctrl_pins[] = { 48, 50 };

static const struct sirfsoc_muxmask sdmmc3_muxmask[] = {
	{
		.group = 0,
		.mask = BIT(30) | BIT(31),
	}, {
		.group = 1,
		.mask = BIT(0) | BIT(1) | BIT(2) | BIT(3),
	},
};

static const struct sirfsoc_padmux sdmmc3_padmux = {
	.muxmask_counts = ARRAY_SIZE(sdmmc3_muxmask),
	.muxmask = sdmmc3_muxmask,
	.ctrlreg = SIRFSOC_RSC_PIN_MUX,
	.funcmask = BIT(7),
	.funcval = 0,
};

static const unsigned sdmmc3_pins[] = { 30, 31, 32, 33, 34, 35 };

static const struct sirfsoc_muxmask spi0_muxmask[] = {
	{
		.group = 0,
		.mask = BIT(30),
	}, {
		.group = 1,
		.mask = BIT(0) | BIT(2) | BIT(3),
	},
};

static const struct sirfsoc_padmux spi0_padmux = {
	.muxmask_counts = ARRAY_SIZE(spi0_muxmask),
	.muxmask = spi0_muxmask,
	.ctrlreg = SIRFSOC_RSC_PIN_MUX,
	.funcmask = BIT(7),
	.funcval = BIT(7),
};

static const unsigned spi0_pins[] = { 30, 32, 34, 35 };

static const struct sirfsoc_muxmask cko1_muxmask[] = {
	{
		.group = 1,
		.mask = BIT(10),
	},
};

static const struct sirfsoc_padmux cko1_padmux = {
	.muxmask_counts = ARRAY_SIZE(cko1_muxmask),
	.muxmask = cko1_muxmask,
	.ctrlreg = SIRFSOC_RSC_PIN_MUX,
	.funcmask = BIT(3),
	.funcval = 0,
};

static const unsigned cko1_pins[] = { 42 };

static const struct sirfsoc_muxmask i2s_mclk_muxmask[] = {
	{
		.group = 1,
		.mask = BIT(10),
	},
};

static const struct sirfsoc_padmux i2s_mclk_padmux = {
	.muxmask_counts = ARRAY_SIZE(i2s_mclk_muxmask),
	.muxmask = i2s_mclk_muxmask,
	.ctrlreg = SIRFSOC_RSC_PIN_MUX,
	.funcmask = BIT(3),
	.funcval = BIT(3),
};

static const unsigned i2s_mclk_pins[] = { 42 };

static const struct sirfsoc_muxmask i2s_ext_clk_input_muxmask[] = {
	{
		.group = 1,
		.mask = BIT(19),
	},
};

static const struct sirfsoc_padmux i2s_ext_clk_input_padmux = {
	.muxmask_counts = ARRAY_SIZE(i2s_ext_clk_input_muxmask),
	.muxmask = i2s_ext_clk_input_muxmask,
	.ctrlreg = SIRFSOC_RSC_PIN_MUX,
	.funcmask = BIT(2),
	.funcval = BIT(2),
};

static const unsigned i2s_ext_clk_input_pins[] = { 51 };

static const struct sirfsoc_muxmask i2s_muxmask[] = {
	{
		.group = 3,
		.mask = BIT(2) | BIT(3) | BIT(4) | BIT(5),
	},
};

static const struct sirfsoc_padmux i2s_padmux = {
	.muxmask_counts = ARRAY_SIZE(i2s_muxmask),
	.muxmask = i2s_muxmask,
	.ctrlreg = SIRFSOC_RSC_PIN_MUX,
};

static const unsigned i2s_pins[] = { 98, 99, 100, 101 };

static const struct sirfsoc_muxmask i2s_no_din_muxmask[] = {
	{
		.group = 3,
		.mask = BIT(2) | BIT(3) | BIT(4),
	},
};

static const struct sirfsoc_padmux i2s_no_din_padmux = {
	.muxmask_counts = ARRAY_SIZE(i2s_no_din_muxmask),
	.muxmask = i2s_no_din_muxmask,
	.ctrlreg = SIRFSOC_RSC_PIN_MUX,
};

static const unsigned i2s_no_din_pins[] = { 98, 99, 100 };

static const struct sirfsoc_muxmask i2s_6chn_muxmask[] = {
	{
		.group = 3,
		.mask = BIT(2) | BIT(3) | BIT(4) | BIT(5),
	},
};

static const struct sirfsoc_padmux i2s_6chn_padmux = {
	.muxmask_counts = ARRAY_SIZE(i2s_6chn_muxmask),
	.muxmask = i2s_6chn_muxmask,
	.ctrlreg = SIRFSOC_RSC_PIN_MUX,
	.funcmask = BIT(1) | BIT(9),
	.funcval = BIT(1) | BIT(9),
};

static const unsigned i2s_6chn_pins[] = { 52, 55, 98, 99, 100, 101 };

static const struct sirfsoc_muxmask ac97_muxmask[] = {
	{
		.group = 3,
		.mask = BIT(2) | BIT(3) | BIT(4) | BIT(5),
	},
};

static const struct sirfsoc_padmux ac97_padmux = {
	.muxmask_counts = ARRAY_SIZE(ac97_muxmask),
	.muxmask = ac97_muxmask,
};

static const unsigned ac97_pins[] = { 98, 99, 100, 101 };

static const struct sirfsoc_muxmask spi1_muxmask[] = {
	{
		.group = 1,
		.mask = BIT(11) | BIT(12) | BIT(13) | BIT(14),
	},
};

static const struct sirfsoc_padmux spi1_padmux = {
	.muxmask_counts = ARRAY_SIZE(spi1_muxmask),
	.muxmask = spi1_muxmask,
	.ctrlreg = SIRFSOC_RSC_PIN_MUX,
	.funcmask = BIT(16),
	.funcval = 0,
};

static const unsigned spi1_pins[] = { 43, 44, 45, 46 };

static const struct sirfsoc_muxmask sdmmc1_muxmask[] = {
	{
		.group = 2,
		.mask = BIT(2) | BIT(3),
	},
};

static const struct sirfsoc_padmux sdmmc1_padmux = {
	.muxmask_counts = ARRAY_SIZE(sdmmc1_muxmask),
	.muxmask = sdmmc1_muxmask,
	.ctrlreg = SIRFSOC_RSC_PIN_MUX,
	.funcmask = BIT(5),
	.funcval = BIT(5),
};

static const unsigned sdmmc1_pins[] = { 66, 67 };

static const struct sirfsoc_muxmask gps_muxmask[] = {
	{
		.group = 0,
		.mask = BIT(24) | BIT(25) | BIT(26),
	},
};

static const struct sirfsoc_padmux gps_padmux = {
	.muxmask_counts = ARRAY_SIZE(gps_muxmask),
	.muxmask = gps_muxmask,
	.ctrlreg = SIRFSOC_RSC_PIN_MUX,
	.funcmask = BIT(13),
	.funcval = 0,
};

static const unsigned gps_pins[] = { 24, 25, 26 };

static const struct sirfsoc_muxmask sdmmc5_muxmask[] = {
	{
		.group = 0,
		.mask = BIT(24) | BIT(25) | BIT(26),
	},
};

static const struct sirfsoc_padmux sdmmc5_padmux = {
	.muxmask_counts = ARRAY_SIZE(sdmmc5_muxmask),
	.muxmask = sdmmc5_muxmask,
	.ctrlreg = SIRFSOC_RSC_PIN_MUX,
	.funcmask = BIT(13),
	.funcval = BIT(13),
};

static const unsigned sdmmc5_pins[] = { 24, 25, 26 };

static const struct sirfsoc_muxmask usp0_muxmask[] = {
	{
		.group = 1,
		.mask = BIT(19) | BIT(20) | BIT(21) | BIT(22) | BIT(23),
	},
};

static const struct sirfsoc_padmux usp0_padmux = {
	.muxmask_counts = ARRAY_SIZE(usp0_muxmask),
	.muxmask = usp0_muxmask,
	.ctrlreg = SIRFSOC_RSC_PIN_MUX,
	.funcmask = BIT(1) | BIT(2) | BIT(9),
	.funcval = 0,
};

static const unsigned usp0_pins[] = { 51, 52, 53, 54, 55 };

static const struct sirfsoc_muxmask usp0_only_utfs_muxmask[] = {
	{
		.group = 1,
		.mask = BIT(19) | BIT(20) | BIT(21) | BIT(22),
	},
};

static const struct sirfsoc_padmux usp0_only_utfs_padmux = {
	.muxmask_counts = ARRAY_SIZE(usp0_only_utfs_muxmask),
	.muxmask = usp0_only_utfs_muxmask,
	.ctrlreg = SIRFSOC_RSC_PIN_MUX,
	.funcmask = BIT(1) | BIT(2) | BIT(6),
	.funcval = 0,
};

static const unsigned usp0_only_utfs_pins[] = { 51, 52, 53, 54 };

static const struct sirfsoc_muxmask usp0_only_urfs_muxmask[] = {
	{
		.group = 1,
		.mask = BIT(19) | BIT(20) | BIT(21) | BIT(23),
	},
};

static const struct sirfsoc_padmux usp0_only_urfs_padmux = {
	.muxmask_counts = ARRAY_SIZE(usp0_only_urfs_muxmask),
	.muxmask = usp0_only_urfs_muxmask,
	.ctrlreg = SIRFSOC_RSC_PIN_MUX,
	.funcmask = BIT(1) | BIT(2) | BIT(9),
	.funcval = 0,
};

static const unsigned usp0_only_urfs_pins[] = { 51, 52, 53, 55 };

static const struct sirfsoc_muxmask usp0_uart_nostreamctrl_muxmask[] = {
	{
		.group = 1,
		.mask = BIT(20) | BIT(21),
	},
};

static const struct sirfsoc_padmux usp0_uart_nostreamctrl_padmux = {
	.muxmask_counts = ARRAY_SIZE(usp0_uart_nostreamctrl_muxmask),
	.muxmask = usp0_uart_nostreamctrl_muxmask,
};

static const unsigned usp0_uart_nostreamctrl_pins[] = { 52, 53 };
static const struct sirfsoc_muxmask usp1_muxmask[] = {
	{
		.group = 0,
		.mask = BIT(15),
	}, {
		.group = 1,
		.mask = BIT(11) | BIT(12) | BIT(13) | BIT(14),
	},
};

static const struct sirfsoc_padmux usp1_padmux = {
	.muxmask_counts = ARRAY_SIZE(usp1_muxmask),
	.muxmask = usp1_muxmask,
	.ctrlreg = SIRFSOC_RSC_PIN_MUX,
	.funcmask = BIT(16),
	.funcval = BIT(16),
};

static const unsigned usp1_pins[] = { 15, 43, 44, 45, 46 };

static const struct sirfsoc_muxmask usp1_uart_nostreamctrl_muxmask[] = {
	{
		.group = 1,
		.mask = BIT(12) | BIT(13),
	},
};

static const struct sirfsoc_padmux usp1_uart_nostreamctrl_padmux = {
	.muxmask_counts = ARRAY_SIZE(usp1_uart_nostreamctrl_muxmask),
	.muxmask = usp1_uart_nostreamctrl_muxmask,
	.ctrlreg = SIRFSOC_RSC_PIN_MUX,
	.funcmask = BIT(16),
	.funcval = BIT(16),
};

static const unsigned usp1_uart_nostreamctrl_pins[] = { 44, 45 };

static const struct sirfsoc_muxmask nand_muxmask[] = {
	{
		.group = 2,
		.mask = BIT(2) | BIT(3) | BIT(28) | BIT(29) | BIT(30),
	}, {
		.group = 3,
		.mask = BIT(0) | BIT(1),
	},
};

static const struct sirfsoc_padmux nand_padmux = {
	.muxmask_counts = ARRAY_SIZE(nand_muxmask),
	.muxmask = nand_muxmask,
	.ctrlreg = SIRFSOC_RSC_PIN_MUX,
	.funcmask = BIT(5) | BIT(19),
	.funcval = 0,
};

static const unsigned nand_pins[] = { 66, 67, 92, 93, 94, 96, 97 };

static const struct sirfsoc_muxmask sdmmc0_muxmask[] = {
	{
		.group = 3,
		.mask = BIT(1),
	},
};

static const struct sirfsoc_padmux sdmmc0_padmux = {
	.muxmask_counts = ARRAY_SIZE(sdmmc0_muxmask),
	.muxmask = sdmmc0_muxmask,
	.ctrlreg = SIRFSOC_RSC_PIN_MUX,
	.funcmask = BIT(5) | BIT(19),
	.funcval = BIT(19),
};

static const unsigned sdmmc0_pins[] = { 97 };

static const struct sirfsoc_muxmask sdmmc2_muxmask[] = {
	{
		.group = 0,
		.mask = BIT(27) | BIT(28) | BIT(29),
	},
};

static const struct sirfsoc_padmux sdmmc2_padmux = {
	.muxmask_counts = ARRAY_SIZE(sdmmc2_muxmask),
	.muxmask = sdmmc2_muxmask,
	.ctrlreg = SIRFSOC_RSC_PIN_MUX,
	.funcmask = BIT(11),
	.funcval = 0,
};

static const unsigned sdmmc2_pins[] = { 27, 28, 29 };

static const struct sirfsoc_muxmask sdmmc2_nowp_muxmask[] = {
	{
		.group = 0,
		.mask = BIT(27) | BIT(28),
	},
};

static const struct sirfsoc_padmux sdmmc2_nowp_padmux = {
	.muxmask_counts = ARRAY_SIZE(sdmmc2_nowp_muxmask),
	.muxmask = sdmmc2_nowp_muxmask,
	.ctrlreg = SIRFSOC_RSC_PIN_MUX,
	.funcmask = BIT(11),
	.funcval = 0,
};

static const unsigned sdmmc2_nowp_pins[] = { 27, 28 };

static const struct sirfsoc_muxmask cko0_muxmask[] = {
	{
		.group = 2,
		.mask = BIT(14),
	},
};

static const struct sirfsoc_padmux cko0_padmux = {
	.muxmask_counts = ARRAY_SIZE(cko0_muxmask),
	.muxmask = cko0_muxmask,
};

static const unsigned cko0_pins[] = { 78 };

static const struct sirfsoc_muxmask vip_muxmask[] = {
	{
		.group = 1,
		.mask = BIT(4) | BIT(5) | BIT(6) | BIT(8) | BIT(9)
			| BIT(24) | BIT(25) | BIT(26) | BIT(27) | BIT(28) |
			BIT(29),
	},
};

static const struct sirfsoc_padmux vip_padmux = {
	.muxmask_counts = ARRAY_SIZE(vip_muxmask),
	.muxmask = vip_muxmask,
	.ctrlreg = SIRFSOC_RSC_PIN_MUX,
	.funcmask = BIT(18),
	.funcval = BIT(18),
};

static const unsigned vip_pins[] = { 36, 37, 38, 40, 41, 56, 57, 58, 59,
	60, 61 };

static const struct sirfsoc_muxmask vip_noupli_muxmask[] = {
	{
		.group = 0,
		.mask = BIT(16) | BIT(17) | BIT(18) | BIT(19) | BIT(20)
			| BIT(21) | BIT(22) | BIT(23),
	}, {
		.group = 2,
		.mask = BIT(23) | BIT(24) | BIT(25),
	},
};

static const struct sirfsoc_padmux vip_noupli_padmux = {
	.muxmask_counts = ARRAY_SIZE(vip_noupli_muxmask),
	.muxmask = vip_noupli_muxmask,
	.ctrlreg = SIRFSOC_RSC_PIN_MUX,
	.funcmask = BIT(15),
	.funcval = BIT(15),
};

static const unsigned vip_noupli_pins[] = { 16, 17, 18, 19, 20, 21, 22, 23,
	87, 88, 89 };

static const struct sirfsoc_muxmask i2c0_muxmask[] = {
	{
		.group = 2,
		.mask = BIT(26) | BIT(27),
	},
};

static const struct sirfsoc_padmux i2c0_padmux = {
	.muxmask_counts = ARRAY_SIZE(i2c0_muxmask),
	.muxmask = i2c0_muxmask,
};

static const unsigned i2c0_pins[] = { 90, 91 };

static const struct sirfsoc_muxmask i2c1_muxmask[] = {
	{
		.group = 0,
		.mask = BIT(13) | BIT(15),
	},
};

static const struct sirfsoc_padmux i2c1_padmux = {
	.muxmask_counts = ARRAY_SIZE(i2c1_muxmask),
	.muxmask = i2c1_muxmask,
	.ctrlreg = SIRFSOC_RSC_PIN_MUX,
	.funcmask = BIT(16),
	.funcval = 0,
};

static const unsigned i2c1_pins[] = { 13, 15 };

static const struct sirfsoc_muxmask pwm0_muxmask[] = {
	{
		.group = 0,
		.mask = BIT(4),
	},
};

static const struct sirfsoc_padmux pwm0_padmux = {
	.muxmask_counts = ARRAY_SIZE(pwm0_muxmask),
	.muxmask = pwm0_muxmask,
	.ctrlreg = SIRFSOC_RSC_PIN_MUX,
	.funcmask = BIT(12),
	.funcval = 0,
};

static const unsigned pwm0_pins[] = { 4 };

static const struct sirfsoc_muxmask pwm1_muxmask[] = {
	{
		.group = 0,
		.mask = BIT(5),
	},
};

static const struct sirfsoc_padmux pwm1_padmux = {
	.muxmask_counts = ARRAY_SIZE(pwm1_muxmask),
	.muxmask = pwm1_muxmask,
};

static const unsigned pwm1_pins[] = { 5 };

static const struct sirfsoc_muxmask pwm2_muxmask[] = {
	{
		.group = 0,
		.mask = BIT(6),
	},
};

static const struct sirfsoc_padmux pwm2_padmux = {
	.muxmask_counts = ARRAY_SIZE(pwm2_muxmask),
	.muxmask = pwm2_muxmask,
};

static const unsigned pwm2_pins[] = { 6 };

static const struct sirfsoc_muxmask pwm3_muxmask[] = {
	{
		.group = 0,
		.mask = BIT(7),
	},
};

static const struct sirfsoc_padmux pwm3_padmux = {
	.muxmask_counts = ARRAY_SIZE(pwm3_muxmask),
	.muxmask = pwm3_muxmask,
};

static const unsigned pwm3_pins[] = { 7 };

static const struct sirfsoc_muxmask pwm4_muxmask[] = {
	{
		.group = 2,
		.mask = BIT(14),
	},
};

static const struct sirfsoc_padmux pwm4_padmux = {
	.muxmask_counts = ARRAY_SIZE(pwm4_muxmask),
	.muxmask = pwm4_muxmask,
};

static const unsigned pwm4_pins[] = { 78 };

static const struct sirfsoc_muxmask warm_rst_muxmask[] = {
	{
		.group = 0,
		.mask = BIT(8),
	},
};

static const struct sirfsoc_padmux warm_rst_padmux = {
	.muxmask_counts = ARRAY_SIZE(warm_rst_muxmask),
	.muxmask = warm_rst_muxmask,
	.ctrlreg = SIRFSOC_RSC_PIN_MUX,
	.funcmask = BIT(4),
	.funcval = 0,
};

static const unsigned warm_rst_pins[] = { 8 };

static const struct sirfsoc_muxmask usb0_upli_drvbus_muxmask[] = {
	{
		.group = 1,
		.mask = BIT(4) | BIT(5) | BIT(6) | BIT(7) | BIT(8)
			| BIT(9) | BIT(24) | BIT(25) | BIT(26) |
			BIT(27) | BIT(28) | BIT(29),
	},
};
static const struct sirfsoc_padmux usb0_upli_drvbus_padmux = {
	.muxmask_counts = ARRAY_SIZE(usb0_upli_drvbus_muxmask),
	.muxmask = usb0_upli_drvbus_muxmask,
	.ctrlreg = SIRFSOC_RSC_PIN_MUX,
	.funcmask = BIT(18),
	.funcval = 0,
};

static const unsigned usb0_upli_drvbus_pins[] = { 36, 37, 38, 39, 40,
	41, 56, 57, 58, 59, 60, 61 };

static const struct sirfsoc_muxmask usb1_utmi_drvbus_muxmask[] = {
	{
		.group = 0,
		.mask = BIT(28),
	},
};

static const struct sirfsoc_padmux usb1_utmi_drvbus_padmux = {
	.muxmask_counts = ARRAY_SIZE(usb1_utmi_drvbus_muxmask),
	.muxmask = usb1_utmi_drvbus_muxmask,
	.ctrlreg = SIRFSOC_RSC_PIN_MUX,
	.funcmask = BIT(11),
	.funcval = BIT(11), /* refer to PAD_UTMI_DRVVBUS1_ENABLE */
};

static const unsigned usb1_utmi_drvbus_pins[] = { 28 };

static const struct sirfsoc_padmux usb1_dp_dn_padmux = {
	.muxmask_counts = 0,
	.ctrlreg = SIRFSOC_RSC_USB_UART_SHARE,
	.funcmask = BIT(2),
	.funcval = BIT(2),
};

static const unsigned usb1_dp_dn_pins[] = { 103, 104 };

static const struct sirfsoc_padmux uart1_route_io_usb1_padmux = {
	.muxmask_counts = 0,
	.ctrlreg = SIRFSOC_RSC_USB_UART_SHARE,
	.funcmask = BIT(2),
	.funcval = 0,
};

static const unsigned uart1_route_io_usb1_pins[] = { 103, 104 };

static const struct sirfsoc_muxmask pulse_count_muxmask[] = {
	{
		.group = 0,
		.mask = BIT(9) | BIT(10) | BIT(11),
	},
};

static const struct sirfsoc_padmux pulse_count_padmux = {
	.muxmask_counts = ARRAY_SIZE(pulse_count_muxmask),
	.muxmask = pulse_count_muxmask,
};

static const unsigned pulse_count_pins[] = { 9, 10, 11 };

static const struct sirfsoc_pin_group sirfsoc_pin_groups[] = {
	SIRFSOC_PIN_GROUP("lcd_16bitsgrp", lcd_16bits_pins),
	SIRFSOC_PIN_GROUP("lcd_18bitsgrp", lcd_18bits_pins),
	SIRFSOC_PIN_GROUP("lcd_24bitsgrp", lcd_24bits_pins),
	SIRFSOC_PIN_GROUP("lcdrom_grp", lcdrom_pins),
	SIRFSOC_PIN_GROUP("uart0grp", uart0_pins),
	SIRFSOC_PIN_GROUP("uart0_nostreamctrlgrp", uart0_nostreamctrl_pins),
	SIRFSOC_PIN_GROUP("uart1grp", uart1_pins),
	SIRFSOC_PIN_GROUP("uart2grp", uart2_pins),
	SIRFSOC_PIN_GROUP("uart2_nostreamctrlgrp", uart2_nostreamctrl_pins),
	SIRFSOC_PIN_GROUP("usp0grp", usp0_pins),
	SIRFSOC_PIN_GROUP("usp0_uart_nostreamctrl_grp",
					usp0_uart_nostreamctrl_pins),
	SIRFSOC_PIN_GROUP("usp0_only_utfs_grp", usp0_only_utfs_pins),
	SIRFSOC_PIN_GROUP("usp0_only_urfs_grp", usp0_only_urfs_pins),
	SIRFSOC_PIN_GROUP("usp1grp", usp1_pins),
	SIRFSOC_PIN_GROUP("usp1_uart_nostreamctrl_grp",
					usp1_uart_nostreamctrl_pins),
	SIRFSOC_PIN_GROUP("i2c0grp", i2c0_pins),
	SIRFSOC_PIN_GROUP("i2c1grp", i2c1_pins),
	SIRFSOC_PIN_GROUP("pwm0grp", pwm0_pins),
	SIRFSOC_PIN_GROUP("pwm1grp", pwm1_pins),
	SIRFSOC_PIN_GROUP("pwm2grp", pwm2_pins),
	SIRFSOC_PIN_GROUP("pwm3grp", pwm3_pins),
	SIRFSOC_PIN_GROUP("pwm4grp", pwm4_pins),
	SIRFSOC_PIN_GROUP("vipgrp", vip_pins),
	SIRFSOC_PIN_GROUP("vip_noupligrp", vip_noupli_pins),
	SIRFSOC_PIN_GROUP("warm_rstgrp", warm_rst_pins),
	SIRFSOC_PIN_GROUP("cko0grp", cko0_pins),
	SIRFSOC_PIN_GROUP("cko1grp", cko1_pins),
	SIRFSOC_PIN_GROUP("sdmmc0grp", sdmmc0_pins),
	SIRFSOC_PIN_GROUP("sdmmc1grp", sdmmc1_pins),
	SIRFSOC_PIN_GROUP("sdmmc2grp", sdmmc2_pins),
	SIRFSOC_PIN_GROUP("sdmmc2_nowpgrp", sdmmc2_nowp_pins),
	SIRFSOC_PIN_GROUP("sdmmc3grp", sdmmc3_pins),
	SIRFSOC_PIN_GROUP("sdmmc5grp", sdmmc5_pins),
	SIRFSOC_PIN_GROUP("usb0_upli_drvbusgrp", usb0_upli_drvbus_pins),
	SIRFSOC_PIN_GROUP("usb1_utmi_drvbusgrp", usb1_utmi_drvbus_pins),
	SIRFSOC_PIN_GROUP("usb1_dp_dngrp", usb1_dp_dn_pins),
	SIRFSOC_PIN_GROUP("uart1_route_io_usb1grp", uart1_route_io_usb1_pins),
	SIRFSOC_PIN_GROUP("pulse_countgrp", pulse_count_pins),
	SIRFSOC_PIN_GROUP("i2smclkgrp", i2s_mclk_pins),
	SIRFSOC_PIN_GROUP("i2s_ext_clk_inputgrp", i2s_ext_clk_input_pins),
	SIRFSOC_PIN_GROUP("i2sgrp", i2s_pins),
	SIRFSOC_PIN_GROUP("i2s_no_dingrp", i2s_no_din_pins),
	SIRFSOC_PIN_GROUP("i2s_6chngrp", i2s_6chn_pins),
	SIRFSOC_PIN_GROUP("ac97grp", ac97_pins),
	SIRFSOC_PIN_GROUP("nandgrp", nand_pins),
	SIRFSOC_PIN_GROUP("spi0grp", spi0_pins),
	SIRFSOC_PIN_GROUP("spi1grp", spi1_pins),
	SIRFSOC_PIN_GROUP("gpsgrp", gps_pins),
};

static const char * const lcd_16bitsgrp[] = { "lcd_16bitsgrp" };
static const char * const lcd_18bitsgrp[] = { "lcd_18bitsgrp" };
static const char * const lcd_24bitsgrp[] = { "lcd_24bitsgrp" };
static const char * const lcdromgrp[] = { "lcdromgrp" };
static const char * const uart0grp[] = { "uart0grp" };
static const char * const uart0_nostreamctrlgrp[] = { "uart0_nostreamctrlgrp" };
static const char * const uart1grp[] = { "uart1grp" };
static const char * const uart2grp[] = { "uart2grp" };
static const char * const uart2_nostreamctrlgrp[] = { "uart2_nostreamctrlgrp" };
static const char * const usp0_uart_nostreamctrl_grp[] = {
					"usp0_uart_nostreamctrl_grp" };
static const char * const usp0grp[] = { "usp0grp" };
static const char * const usp0_only_utfs_grp[] = { "usp0_only_utfs_grp" };
static const char * const usp0_only_urfs_grp[] = { "usp0_only_urfs_grp" };

static const char * const usp1grp[] = { "usp1grp" };
static const char * const usp1_uart_nostreamctrl_grp[] = {
					"usp1_uart_nostreamctrl_grp" };
static const char * const i2c0grp[] = { "i2c0grp" };
static const char * const i2c1grp[] = { "i2c1grp" };
static const char * const pwm0grp[] = { "pwm0grp" };
static const char * const pwm1grp[] = { "pwm1grp" };
static const char * const pwm2grp[] = { "pwm2grp" };
static const char * const pwm3grp[] = { "pwm3grp" };
static const char * const pwm4grp[] = { "pwm4grp" };
static const char * const vipgrp[] = { "vipgrp" };
static const char * const vip_noupligrp[] = { "vip_noupligrp" };
static const char * const warm_rstgrp[] = { "warm_rstgrp" };
static const char * const cko0grp[] = { "cko0grp" };
static const char * const cko1grp[] = { "cko1grp" };
static const char * const sdmmc0grp[] = { "sdmmc0grp" };
static const char * const sdmmc1grp[] = { "sdmmc1grp" };
static const char * const sdmmc2grp[] = { "sdmmc2grp" };
static const char * const sdmmc3grp[] = { "sdmmc3grp" };
static const char * const sdmmc5grp[] = { "sdmmc5grp" };
static const char * const sdmmc2_nowpgrp[] = { "sdmmc2_nowpgrp" };
static const char * const usb0_upli_drvbusgrp[] = { "usb0_upli_drvbusgrp" };
static const char * const usb1_utmi_drvbusgrp[] = { "usb1_utmi_drvbusgrp" };
static const char * const usb1_dp_dngrp[] = { "usb1_dp_dngrp" };
static const char * const
	uart1_route_io_usb1grp[] = { "uart1_route_io_usb1grp" };
static const char * const pulse_countgrp[] = { "pulse_countgrp" };
static const char * const i2smclkgrp[] = { "i2smclkgrp" };
static const char * const i2s_ext_clk_inputgrp[] = { "i2s_ext_clk_inputgrp" };
static const char * const i2sgrp[] = { "i2sgrp" };
static const char * const i2s_no_dingrp[] = { "i2s_no_dingrp" };
static const char * const i2s_6chngrp[] = { "i2s_6chngrp" };
static const char * const ac97grp[] = { "ac97grp" };
static const char * const nandgrp[] = { "nandgrp" };
static const char * const spi0grp[] = { "spi0grp" };
static const char * const spi1grp[] = { "spi1grp" };
static const char * const gpsgrp[] = { "gpsgrp" };

static const struct sirfsoc_pmx_func sirfsoc_pmx_functions[] = {
	SIRFSOC_PMX_FUNCTION("lcd_16bits", lcd_16bitsgrp, lcd_16bits_padmux),
	SIRFSOC_PMX_FUNCTION("lcd_18bits", lcd_18bitsgrp, lcd_18bits_padmux),
	SIRFSOC_PMX_FUNCTION("lcd_24bits", lcd_24bitsgrp, lcd_24bits_padmux),
	SIRFSOC_PMX_FUNCTION("lcdrom", lcdromgrp, lcdrom_padmux),
	SIRFSOC_PMX_FUNCTION("uart0", uart0grp, uart0_padmux),
	SIRFSOC_PMX_FUNCTION("uart0_nostreamctrl", uart0_nostreamctrlgrp,
						uart0_nostreamctrl_padmux),
	SIRFSOC_PMX_FUNCTION("uart1", uart1grp, uart1_padmux),
	SIRFSOC_PMX_FUNCTION("uart2", uart2grp, uart2_padmux),
	SIRFSOC_PMX_FUNCTION("uart2_nostreamctrl",
		uart2_nostreamctrlgrp, uart2_nostreamctrl_padmux),
	SIRFSOC_PMX_FUNCTION("usp0", usp0grp, usp0_padmux),
	SIRFSOC_PMX_FUNCTION("usp0_uart_nostreamctrl",
						usp0_uart_nostreamctrl_grp,
						usp0_uart_nostreamctrl_padmux),
	SIRFSOC_PMX_FUNCTION("usp0_only_utfs", usp0_only_utfs_grp,
						usp0_only_utfs_padmux),
	SIRFSOC_PMX_FUNCTION("usp0_only_urfs", usp0_only_urfs_grp,
						usp0_only_urfs_padmux),
	SIRFSOC_PMX_FUNCTION("usp1", usp1grp, usp1_padmux),
	SIRFSOC_PMX_FUNCTION("usp1_uart_nostreamctrl",
						usp1_uart_nostreamctrl_grp,
						usp1_uart_nostreamctrl_padmux),
	SIRFSOC_PMX_FUNCTION("i2c0", i2c0grp, i2c0_padmux),
	SIRFSOC_PMX_FUNCTION("i2c1", i2c1grp, i2c1_padmux),
	SIRFSOC_PMX_FUNCTION("pwm0", pwm0grp, pwm0_padmux),
	SIRFSOC_PMX_FUNCTION("pwm1", pwm1grp, pwm1_padmux),
	SIRFSOC_PMX_FUNCTION("pwm2", pwm2grp, pwm2_padmux),
	SIRFSOC_PMX_FUNCTION("pwm3", pwm3grp, pwm3_padmux),
	SIRFSOC_PMX_FUNCTION("pwm4", pwm4grp, pwm4_padmux),
	SIRFSOC_PMX_FUNCTION("vip", vipgrp, vip_padmux),
	SIRFSOC_PMX_FUNCTION("vip_noupli", vip_noupligrp, vip_noupli_padmux),
	SIRFSOC_PMX_FUNCTION("warm_rst", warm_rstgrp, warm_rst_padmux),
	SIRFSOC_PMX_FUNCTION("cko0", cko0grp, cko0_padmux),
	SIRFSOC_PMX_FUNCTION("cko1", cko1grp, cko1_padmux),
	SIRFSOC_PMX_FUNCTION("sdmmc0", sdmmc0grp, sdmmc0_padmux),
	SIRFSOC_PMX_FUNCTION("sdmmc1", sdmmc1grp, sdmmc1_padmux),
	SIRFSOC_PMX_FUNCTION("sdmmc2", sdmmc2grp, sdmmc2_padmux),
	SIRFSOC_PMX_FUNCTION("sdmmc3", sdmmc3grp, sdmmc3_padmux),
	SIRFSOC_PMX_FUNCTION("sdmmc5", sdmmc5grp, sdmmc5_padmux),
	SIRFSOC_PMX_FUNCTION("sdmmc2_nowp",
		sdmmc2_nowpgrp, sdmmc2_nowp_padmux),
	SIRFSOC_PMX_FUNCTION("usb0_upli_drvbus",
		usb0_upli_drvbusgrp, usb0_upli_drvbus_padmux),
	SIRFSOC_PMX_FUNCTION("usb1_utmi_drvbus",
		usb1_utmi_drvbusgrp, usb1_utmi_drvbus_padmux),
	SIRFSOC_PMX_FUNCTION("usb1_dp_dn", usb1_dp_dngrp, usb1_dp_dn_padmux),
	SIRFSOC_PMX_FUNCTION("uart1_route_io_usb1",
		uart1_route_io_usb1grp, uart1_route_io_usb1_padmux),
	SIRFSOC_PMX_FUNCTION("pulse_count", pulse_countgrp, pulse_count_padmux),
	SIRFSOC_PMX_FUNCTION("i2s_mclk", i2smclkgrp, i2s_mclk_padmux),
	SIRFSOC_PMX_FUNCTION("i2s_ext_clk_input", i2s_ext_clk_inputgrp,
						i2s_ext_clk_input_padmux),
	SIRFSOC_PMX_FUNCTION("i2s", i2sgrp, i2s_padmux),
	SIRFSOC_PMX_FUNCTION("i2s_no_din", i2s_no_dingrp, i2s_no_din_padmux),
	SIRFSOC_PMX_FUNCTION("i2s_6chn", i2s_6chngrp, i2s_6chn_padmux),
	SIRFSOC_PMX_FUNCTION("ac97", ac97grp, ac97_padmux),
	SIRFSOC_PMX_FUNCTION("nand", nandgrp, nand_padmux),
	SIRFSOC_PMX_FUNCTION("spi0", spi0grp, spi0_padmux),
	SIRFSOC_PMX_FUNCTION("spi1", spi1grp, spi1_padmux),
	SIRFSOC_PMX_FUNCTION("gps", gpsgrp, gps_padmux),
};

struct sirfsoc_pinctrl_data atlas6_pinctrl_data = {
	(struct pinctrl_pin_desc *)sirfsoc_pads,
	ARRAY_SIZE(sirfsoc_pads),
	(struct sirfsoc_pin_group *)sirfsoc_pin_groups,
	ARRAY_SIZE(sirfsoc_pin_groups),
	(struct sirfsoc_pmx_func *)sirfsoc_pmx_functions,
	ARRAY_SIZE(sirfsoc_pmx_functions),
};

