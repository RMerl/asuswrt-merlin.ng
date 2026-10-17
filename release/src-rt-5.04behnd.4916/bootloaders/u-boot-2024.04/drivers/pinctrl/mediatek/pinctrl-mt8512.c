// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2019 MediaTek Inc.
 * Author: Mingming Lee <mingming.lee@mediatek.com>
 */

#include <dm.h>

#include "pinctrl-mtk-common.h"

#define PIN_FIELD(_s_pin, _e_pin, _s_addr, _x_addrs, _s_bit, _x_bits)	\
	PIN_FIELD_CALC(_s_pin, _e_pin, _s_addr, _x_addrs, _s_bit,	\
		       _x_bits, 32, false)
#define PIN_FIELDS(_s_pin, _e_pin, _s_addr, _x_addrs, _s_bit, _x_bits)	\
	PIN_FIELD_CALC(_s_pin, _e_pin, _s_addr, _x_addrs, _s_bit,	\
		       _x_bits, 32, true)
#define PIN_FIELD30(_s_pin, _e_pin, _s_addr, _x_addrs, _s_bit, _x_bits)	\
	PIN_FIELD_CALC(_s_pin, _e_pin, _s_addr, _x_addrs, _s_bit,	\
			   _x_bits, 30, false)

static const struct mtk_pin_field_calc mt8512_pin_mode_range[] = {
	PIN_FIELD30(0, 115, 0x1E0, 0x10, 0, 3),
};

static const struct mtk_pin_field_calc mt8512_pin_dir_range[] = {
	PIN_FIELD(0, 115, 0x140, 0x10, 0, 1),
};

static const struct mtk_pin_field_calc mt8512_pin_di_range[] = {
	PIN_FIELD(0, 115, 0x000, 0x10, 0, 1),
};

static const struct mtk_pin_field_calc mt8512_pin_do_range[] = {
	PIN_FIELD(0, 115, 0x0A0, 0x10, 0, 1),
};

static const struct mtk_pin_field_calc mt8512_pin_pullen_range[] = {
	PIN_FIELD(0, 115, 0x860, 0x10, 0, 1),
};

static const struct mtk_pin_field_calc mt8512_pin_pullsel_range[] = {
	PIN_FIELD(0, 115, 0x900, 0x10, 0, 1),
};

static const struct mtk_pin_field_calc mt8512_pin_ies_range[] = {
	PIN_FIELDS(0, 2, 0x410, 0x10, 0, 1),
	PIN_FIELDS(3, 5, 0x410, 0x10, 1, 1),
	PIN_FIELDS(6, 7, 0x410, 0x10, 2, 1),
	PIN_FIELDS(8, 11, 0x410, 0x10, 3, 1),
	PIN_FIELDS(12, 15, 0x410, 0x10, 4, 1),
	PIN_FIELDS(16, 19, 0x410, 0x10, 5, 1),
	PIN_FIELD(20, 20, 0x410, 0x10, 6, 1),
	PIN_FIELDS(21, 25, 0x410, 0x10, 7, 1),
	PIN_FIELDS(26, 27, 0x410, 0x10, 8, 1),
	PIN_FIELDS(28, 31, 0x410, 0x10, 9, 1),
	PIN_FIELD(32, 32, 0x410, 0x10, 10, 1),
	PIN_FIELDS(33, 39, 0x410, 0x10, 11, 1),
	PIN_FIELD(40, 40, 0x410, 0x10, 12, 1),
	PIN_FIELDS(41, 43, 0x410, 0x10, 13, 1),
	PIN_FIELDS(44, 47, 0x410, 0x10, 14, 1),
	PIN_FIELDS(48, 51, 0x410, 0x10, 15, 1),
	PIN_FIELDS(52, 53, 0x410, 0x10, 16, 1),
	PIN_FIELDS(54, 57, 0x410, 0x10, 17, 1),
	PIN_FIELDS(58, 63, 0x410, 0x10, 18, 1),
	PIN_FIELDS(64, 65, 0x410, 0x10, 19, 1),
	PIN_FIELDS(66, 67, 0x410, 0x10, 20, 1),
	PIN_FIELDS(68, 69, 0x410, 0x10, 21, 1),
	PIN_FIELD(70, 70, 0x410, 0x10, 22, 1),
	PIN_FIELD(71, 71, 0x410, 0x10, 23, 1),
	PIN_FIELD(72, 72, 0x410, 0x10, 24, 1),
	PIN_FIELD(73, 73, 0x410, 0x10, 25, 1),
	PIN_FIELD(74, 74, 0x410, 0x10, 26, 1),
	PIN_FIELD(75, 75, 0x410, 0x10, 27, 1),
	PIN_FIELD(76, 76, 0x410, 0x10, 28, 1),
	PIN_FIELD(77, 77, 0x410, 0x10, 29, 1),
	PIN_FIELD(78, 78, 0x410, 0x10, 30, 1),
	PIN_FIELD(79, 79, 0x410, 0x10, 31, 1),
	PIN_FIELD(80, 80, 0x420, 0x10, 0, 1),
	PIN_FIELD(81, 81, 0x420, 0x10, 1, 1),
	PIN_FIELD(82, 82, 0x420, 0x10, 2, 1),
	PIN_FIELD(83, 83, 0x420, 0x10, 3, 1),
	PIN_FIELD(84, 84, 0x420, 0x10, 4, 1),
	PIN_FIELDS(85, 86, 0x420, 0x10, 5, 1),
	PIN_FIELD(87, 87, 0x420, 0x10, 6, 1),
	PIN_FIELDS(88, 91, 0x420, 0x10, 7, 1),
	PIN_FIELDS(92, 98, 0x420, 0x10, 8, 1),
	PIN_FIELDS(99, 101, 0x420, 0x10, 9, 1),
	PIN_FIELDS(102, 104, 0x420, 0x10, 10, 1),
	PIN_FIELDS(105, 111, 0x420, 0x10, 11, 1),
	PIN_FIELDS(112, 115, 0x420, 0x10, 12, 1),
};

static const struct mtk_pin_field_calc mt8512_pin_smt_range[] = {
	PIN_FIELDS(0, 2, 0x470, 0x10, 0, 1),
	PIN_FIELDS(3, 5, 0x470, 0x10, 1, 1),
	PIN_FIELDS(6, 7, 0x470, 0x10, 2, 1),
	PIN_FIELDS(8, 11, 0x470, 0x10, 3, 1),
	PIN_FIELDS(12, 15, 0x470, 0x10, 4, 1),
	PIN_FIELDS(16, 19, 0x470, 0x10, 5, 1),
	PIN_FIELD(20, 20, 0x470, 0x10, 6, 1),
	PIN_FIELDS(21, 25, 0x470, 0x10, 7, 1),
	PIN_FIELDS(26, 27, 0x470, 0x10, 8, 1),
	PIN_FIELDS(28, 31, 0x470, 0x10, 9, 1),
	PIN_FIELD(32, 32, 0x470, 0x10, 10, 1),
	PIN_FIELDS(33, 39, 0x470, 0x10, 11, 1),
	PIN_FIELD(40, 40, 0x470, 0x10, 12, 1),
	PIN_FIELDS(41, 43, 0x470, 0x10, 13, 1),
	PIN_FIELDS(44, 47, 0x470, 0x10, 14, 1),
	PIN_FIELDS(48, 51, 0x470, 0x10, 15, 1),
	PIN_FIELDS(52, 53, 0x470, 0x10, 16, 1),
	PIN_FIELDS(54, 57, 0x470, 0x10, 17, 1),
	PIN_FIELDS(58, 63, 0x470, 0x10, 18, 1),
	PIN_FIELDS(64, 65, 0x470, 0x10, 19, 1),
	PIN_FIELDS(66, 67, 0x470, 0x10, 20, 1),
	PIN_FIELDS(68, 69, 0x470, 0x10, 21, 1),
	PIN_FIELD(70, 70, 0x470, 0x10, 22, 1),
	PIN_FIELD(71, 71, 0x470, 0x10, 23, 1),
	PIN_FIELD(72, 72, 0x470, 0x10, 24, 1),
	PIN_FIELD(73, 73, 0x470, 0x10, 25, 1),
	PIN_FIELD(74, 74, 0x470, 0x10, 26, 1),
	PIN_FIELD(75, 75, 0x470, 0x10, 27, 1),
	PIN_FIELD(76, 76, 0x470, 0x10, 28, 1),
	PIN_FIELD(77, 77, 0x470, 0x10, 29, 1),
	PIN_FIELD(78, 78, 0x470, 0x10, 30, 1),
	PIN_FIELD(79, 79, 0x470, 0x10, 31, 1),
	PIN_FIELD(80, 80, 0x480, 0x10, 0, 1),
	PIN_FIELD(81, 81, 0x480, 0x10, 1, 1),
	PIN_FIELD(82, 82, 0x480, 0x10, 2, 1),
	PIN_FIELD(83, 83, 0x480, 0x10, 3, 1),
	PIN_FIELD(84, 84, 0x480, 0x10, 4, 1),
	PIN_FIELDS(85, 86, 0x480, 0x10, 5, 1),
	PIN_FIELD(87, 87, 0x480, 0x10, 6, 1),
	PIN_FIELDS(88, 91, 0x480, 0x10, 7, 1),
	PIN_FIELDS(92, 98, 0x480, 0x10, 8, 1),
	PIN_FIELDS(99, 101, 0x480, 0x10, 9, 1),
	PIN_FIELDS(102, 104, 0x480, 0x10, 10, 1),
	PIN_FIELDS(105, 111, 0x480, 0x10, 11, 1),
	PIN_FIELDS(112, 115, 0x480, 0x10, 12, 1),
};

static const struct mtk_pin_field_calc mt8512_pin_drv_range[] = {
	PIN_FIELDS(0, 2, 0x710, 0x10, 0, 4),
	PIN_FIELDS(3, 5, 0x710, 0x10, 4, 4),
	PIN_FIELDS(6, 7, 0x710, 0x10, 8, 4),
	PIN_FIELDS(8, 11, 0x710, 0x10, 12, 4),
	PIN_FIELDS(12, 15, 0x710, 0x10, 16, 4),
	PIN_FIELDS(16, 19, 0x710, 0x10, 20, 4),
	PIN_FIELD(20, 20, 0x710, 0x10, 24, 4),
	PIN_FIELDS(21, 25, 0x710, 0x10, 28, 4),
	PIN_FIELDS(26, 27, 0x720, 0x10, 0, 4),
	PIN_FIELDS(28, 31, 0x720, 0x10, 4, 4),
	PIN_FIELD(32, 32, 0x720, 0x10, 8, 4),
	PIN_FIELDS(33, 39, 0x720, 0x10, 12, 4),
	PIN_FIELD(40, 40, 0x720, 0x10, 16, 4),
	PIN_FIELDS(41, 43, 0x720, 0x10, 20, 4),
	PIN_FIELDS(44, 47, 0x720, 0x10, 24, 4),
	PIN_FIELDS(48, 51, 0x720, 0x10, 28, 4),
	PIN_FIELDS(52, 53, 0x730, 0x10, 0, 4),
	PIN_FIELDS(54, 57, 0x730, 0x10, 4, 4),
	PIN_FIELDS(58, 63, 0x730, 0x10, 8, 4),
	PIN_FIELDS(64, 65, 0x730, 0x10, 12, 4),
	PIN_FIELDS(66, 67, 0x730, 0x10, 16, 4),
	PIN_FIELDS(68, 69, 0x730, 0x10, 20, 4),
	PIN_FIELD(70, 70, 0x730, 0x10, 24, 4),
	PIN_FIELD(71, 71, 0x730, 0x10, 28, 4),
	PIN_FIELDS(72, 75, 0x740, 0x10, 0, 4),
	PIN_FIELDS(76, 79, 0x740, 0x10, 16, 4),
	PIN_FIELD(80, 80, 0x750, 0x10, 0, 4),
	PIN_FIELD(81, 81, 0x750, 0x10, 4, 4),
	PIN_FIELD(82, 82, 0x750, 0x10, 8, 4),
	PIN_FIELDS(83, 86, 0x740, 0x10, 16, 4),
	PIN_FIELD(87, 87, 0x750, 0x10, 24, 4),
	PIN_FIELDS(88, 91, 0x750, 0x10, 28, 4),
	PIN_FIELDS(92, 98, 0x760, 0x10, 0, 4),
	PIN_FIELDS(99, 101, 0x760, 0x10, 4, 4),
	PIN_FIELDS(102, 104, 0x760, 0x10, 8, 4),
	PIN_FIELDS(105, 111, 0x760, 0x10, 12, 4),
	PIN_FIELDS(112, 115, 0x760, 0x10, 16, 4),
};

static const struct mtk_pin_reg_calc mt8512_reg_cals[] = {
	[PINCTRL_PIN_REG_MODE] = MTK_RANGE(mt8512_pin_mode_range),
	[PINCTRL_PIN_REG_DIR] = MTK_RANGE(mt8512_pin_dir_range),
	[PINCTRL_PIN_REG_DI] = MTK_RANGE(mt8512_pin_di_range),
	[PINCTRL_PIN_REG_DO] = MTK_RANGE(mt8512_pin_do_range),
	[PINCTRL_PIN_REG_IES] = MTK_RANGE(mt8512_pin_ies_range),
	[PINCTRL_PIN_REG_SMT] = MTK_RANGE(mt8512_pin_smt_range),
	[PINCTRL_PIN_REG_PULLSEL] = MTK_RANGE(mt8512_pin_pullsel_range),
	[PINCTRL_PIN_REG_PULLEN] = MTK_RANGE(mt8512_pin_pullen_range),
	[PINCTRL_PIN_REG_DRV] = MTK_RANGE(mt8512_pin_drv_range),
};

static const struct mtk_pin_desc mt8512_pins[] = {
	MTK_PIN(0, "GPIO0", DRV_GRP4),
	MTK_PIN(1, "GPIO1", DRV_GRP4),
	MTK_PIN(2, "GPIO2", DRV_GRP4),
	MTK_PIN(3, "GPIO3", DRV_GRP4),
	MTK_PIN(4, "GPIO4", DRV_GRP4),
	MTK_PIN(5, "GPIO5", DRV_GRP4),
	MTK_PIN(6, "GPIO6", DRV_GRP4),
	MTK_PIN(7, "GPIO7", DRV_GRP4),
	MTK_PIN(8, "GPIO8", DRV_GRP4),
	MTK_PIN(9, "GPIO9", DRV_GRP4),
	MTK_PIN(10, "GPIO10", DRV_GRP4),
	MTK_PIN(11, "GPIO11", DRV_GRP4),
	MTK_PIN(12, "GPIO12", DRV_GRP4),
	MTK_PIN(13, "GPIO13", DRV_GRP4),
	MTK_PIN(14, "GPIO14", DRV_GRP4),
	MTK_PIN(15, "GPIO15", DRV_GRP4),
	MTK_PIN(16, "GPIO16", DRV_GRP4),
	MTK_PIN(17, "GPIO17", DRV_GRP4),
	MTK_PIN(18, "GPIO18", DRV_GRP4),
	MTK_PIN(19, "GPIO19", DRV_GRP4),
	MTK_PIN(20, "GPIO20", DRV_GRP4),
	MTK_PIN(21, "AUDIO_SYNC", DRV_GRP4),
	MTK_PIN(22, "WIFI_INTB", DRV_GRP4),
	MTK_PIN(23, "BT_INTB", DRV_GRP4),
	MTK_PIN(24, "BT_STEREO", DRV_GRP4),
	MTK_PIN(25, "RSTNB", DRV_GRP4),
	MTK_PIN(26, "USB_ID", DRV_GRP4),
	MTK_PIN(27, "USB_DRV", DRV_GRP4),
	MTK_PIN(28, "EINT_GAUGEING", DRV_GRP4),
	MTK_PIN(29, "CHG_IRQ", DRV_GRP4),
	MTK_PIN(30, "CHG_OTG", DRV_GRP4),
	MTK_PIN(31, "CHG_CEB", DRV_GRP4),
	MTK_PIN(32, "FL_EN", DRV_GRP4),
	MTK_PIN(33, "WAN_SMS_RDY", DRV_GRP4),
	MTK_PIN(34, "SOC2WAN_RESET", DRV_GRP4),
	MTK_PIN(35, "WAN_FM_RDY", DRV_GRP4),
	MTK_PIN(36, "WAN_DIS", DRV_GRP4),
	MTK_PIN(37, "WAN_VBUS_EN", DRV_GRP4),
	MTK_PIN(38, "WAN_VBAT_EN", DRV_GRP4),
	MTK_PIN(39, "WAN_PWR_EN", DRV_GRP4),
	MTK_PIN(40, "KPROW0", DRV_GRP4),
	MTK_PIN(41, "KPROW1", DRV_GRP4),
	MTK_PIN(42, "KPCOL0", DRV_GRP4),
	MTK_PIN(43, "KPCOL1", DRV_GRP4),
	MTK_PIN(44, "PWM0", DRV_GRP4),
	MTK_PIN(45, "PWM1", DRV_GRP4),
	MTK_PIN(46, "PWM2", DRV_GRP4),
	MTK_PIN(47, "PWM3", DRV_GRP4),
	MTK_PIN(48, "JTMS", DRV_GRP4),
	MTK_PIN(49, "JTCK", DRV_GRP4),
	MTK_PIN(50, "JTDI", DRV_GRP4),
	MTK_PIN(51, "JTDO", DRV_GRP4),
	MTK_PIN(52, "URXD0", DRV_GRP4),
	MTK_PIN(53, "UTXD0", DRV_GRP4),
	MTK_PIN(54, "URXD1", DRV_GRP4),
	MTK_PIN(55, "UTXD1", DRV_GRP4),
	MTK_PIN(56, "URTS1", DRV_GRP4),
	MTK_PIN(57, "UCTS1", DRV_GRP4),
	MTK_PIN(58, "RTC32K_CK", DRV_GRP4),
	MTK_PIN(59, "PMIC_DVS_REQ0", DRV_GRP4),
	MTK_PIN(60, "PMIC_DVS_REQ1", DRV_GRP4),
	MTK_PIN(61, "WATCHDOG", DRV_GRP4),
	MTK_PIN(62, "PMIC_INT", DRV_GRP4),
	MTK_PIN(63, "SUSPEND", DRV_GRP4),
	MTK_PIN(64, "SDA0", DRV_GRP4),
	MTK_PIN(65, "SCL0", DRV_GRP4),
	MTK_PIN(66, "SDA1", DRV_GRP4),
	MTK_PIN(67, "SCL1", DRV_GRP4),
	MTK_PIN(68, "SDA2", DRV_GRP4),
	MTK_PIN(69, "SCL2", DRV_GRP4),
	MTK_PIN(70, "MSDC1_CMD", DRV_GRP4),
	MTK_PIN(71, "MSDC1_CLK", DRV_GRP4),
	MTK_PIN(72, "MSDC1_DAT0", DRV_GRP4),
	MTK_PIN(73, "MSDC1_DAT1", DRV_GRP4),
	MTK_PIN(74, "MSDC1_DAT2", DRV_GRP4),
	MTK_PIN(75, "MSDC1_DAT3", DRV_GRP4),
	MTK_PIN(76, "MSDC0_DAT7", DRV_GRP4),
	MTK_PIN(77, "MSDC0_DAT6", DRV_GRP4),
	MTK_PIN(78, "MSDC0_DAT5", DRV_GRP4),
	MTK_PIN(79, "MSDC0_DAT4", DRV_GRP4),
	MTK_PIN(80, "MSDC0_RSTB", DRV_GRP4),
	MTK_PIN(81, "MSDC0_CMD", DRV_GRP4),
	MTK_PIN(82, "MSDC0_CLK", DRV_GRP4),
	MTK_PIN(83, "MSDC0_DAT3", DRV_GRP4),
	MTK_PIN(84, "MSDC0_DAT2", DRV_GRP4),
	MTK_PIN(85, "MSDC0_DAT1", DRV_GRP4),
	MTK_PIN(86, "MSDC0_DAT0", DRV_GRP4),
	MTK_PIN(87, "SPDIF", DRV_GRP4),
	MTK_PIN(88, "PCM_CLK", DRV_GRP4),
	MTK_PIN(89, "PCM_SYNC", DRV_GRP4),
	MTK_PIN(90, "PCM_RX", DRV_GRP4),
	MTK_PIN(91, "PCM_TX", DRV_GRP4),
	MTK_PIN(92, "I2SIN_MCLK", DRV_GRP4),
	MTK_PIN(93, "I2SIN_LRCK", DRV_GRP4),
	MTK_PIN(94, "I2SIN_BCK", DRV_GRP4),
	MTK_PIN(95, "I2SIN_DAT0", DRV_GRP4),
	MTK_PIN(96, "I2SIN_DAT1", DRV_GRP4),
	MTK_PIN(97, "I2SIN_DAT2", DRV_GRP4),
	MTK_PIN(98, "I2SIN_DAT3", DRV_GRP4),
	MTK_PIN(99, "DMIC0_CLK", DRV_GRP4),
	MTK_PIN(100, "DMIC0_DAT0", DRV_GRP4),
	MTK_PIN(101, "DMIC0_DAT1", DRV_GRP4),
	MTK_PIN(102, "DMIC1_CLK", DRV_GRP4),
	MTK_PIN(103, "DMIC1_DAT0", DRV_GRP4),
	MTK_PIN(104, "DMIC1_DAT1", DRV_GRP4),
	MTK_PIN(105, "I2SO_BCK", DRV_GRP4),
	MTK_PIN(106, "I2SO_LRCK", DRV_GRP4),
	MTK_PIN(107, "I2SO_MCLK", DRV_GRP4),
	MTK_PIN(108, "I2SO_DAT0", DRV_GRP4),
	MTK_PIN(109, "I2SO_DAT1", DRV_GRP4),
	MTK_PIN(110, "I2SO_DAT2", DRV_GRP4),
	MTK_PIN(111, "I2SO_DAT3", DRV_GRP4),
	MTK_PIN(112, "SPI_CSB", DRV_GRP4),
	MTK_PIN(113, "SPI_CLK", DRV_GRP4),
	MTK_PIN(114, "SPI_MISO", DRV_GRP4),
	MTK_PIN(115, "SPI_MOSI", DRV_GRP4),
};

/* List all groups consisting of these pins dedicated to the enablement of
 * certain hardware block and the corresponding mode for all of the pins.
 * The hardware probably has multiple combinations of these pinouts.
 */

/* UART */
static const int mt8512_uart0_0_rxd_txd_pins[]		= { 52, 53, };
static const int mt8512_uart0_0_rxd_txd_funcs[]		= {  1,  1, };
static const int mt8512_uart1_0_rxd_txd_pins[]		= { 54, 55, };
static const int mt8512_uart1_0_rxd_txd_funcs[]		= {  1,  1, };
static const int mt8512_uart2_0_rxd_txd_pins[]		= { 28, 29, };
static const int mt8512_uart2_0_rxd_txd_funcs[]		= {  1,  1, };

/* Joint those groups owning the same capability in user point of view which
 * allows that people tend to use through the device tree.
 */
static const char *const mt8512_uart_groups[] = { "uart0_0_rxd_txd",
						"uart1_0_rxd_txd",
						"uart2_0_rxd_txd", };

/* SNAND */
static const int mt8512_snfi_pins[] = { 71, 76, 77, 78, 79, 80, };
static const int mt8512_snfi_funcs[] = { 3, 3, 3, 3, 3, 3, };

/* MMC0 */
static const int mt8512_msdc0_pins[] = { 76, 77, 78, 79, 80, 81, 82, 83, 84,
					 85, 86, };
static const int mt8512_msdc0_funcs[] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, };

static const struct mtk_group_desc mt8512_groups[] = {
	PINCTRL_PIN_GROUP("uart0_0_rxd_txd", mt8512_uart0_0_rxd_txd),
	PINCTRL_PIN_GROUP("uart1_0_rxd_txd", mt8512_uart1_0_rxd_txd),
	PINCTRL_PIN_GROUP("uart2_0_rxd_txd", mt8512_uart2_0_rxd_txd),

	PINCTRL_PIN_GROUP("msdc0", mt8512_msdc0),

	PINCTRL_PIN_GROUP("snfi", mt8512_snfi),
};

static const char *const mt8512_msdc_groups[] = { "msdc0" };

static const struct mtk_function_desc mt8512_functions[] = {
	{"uart", mt8512_uart_groups, ARRAY_SIZE(mt8512_uart_groups)},
	{"msdc", mt8512_msdc_groups, ARRAY_SIZE(mt8512_msdc_groups)},
	{"snand", mt8512_msdc_groups, ARRAY_SIZE(mt8512_msdc_groups)},
};

static const struct mtk_pinctrl_soc mt8512_data = {
	.name = "mt8512_pinctrl",
	.reg_cal = mt8512_reg_cals,
	.pins = mt8512_pins,
	.npins = ARRAY_SIZE(mt8512_pins),
	.grps = mt8512_groups,
	.ngrps = ARRAY_SIZE(mt8512_groups),
	.funcs = mt8512_functions,
	.nfuncs = ARRAY_SIZE(mt8512_functions),
};

static int mtk_pinctrl_mt8512_probe(struct udevice *dev)
{
	return mtk_pinctrl_common_probe(dev, &mt8512_data);
}

static const struct udevice_id mt8512_pctrl_match[] = {
	{ .compatible = "mediatek,mt8512-pinctrl" },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(mt8512_pinctrl) = {
	.name = "mt8512_pinctrl",
	.id = UCLASS_PINCTRL,
	.of_match = mt8512_pctrl_match,
	.ops = &mtk_pinctrl_ops,
	.probe = mtk_pinctrl_mt8512_probe,
	.priv_auto	= sizeof(struct mtk_pinctrl_priv),
};
