// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2019 MediaTek Inc.
 * Author: Mingming Lee <mingming.lee@mediatek.com>
 */

#include <dm.h>

#include "pinctrl-mtk-common.h"

#define PIN_FIELD(_s_pin, _e_pin, _s_addr, _x_addrs, _s_bit, _x_bits)	\
	PIN_FIELD_CALC(_s_pin, _e_pin, _s_addr, _x_addrs, _s_bit,	\
		       _x_bits, 16, false)

static const struct mtk_pin_field_calc mt8518_pin_mode_range[] = {
	PIN_FIELD_CALC(0, 119, 0x300, 0x10, 0, 3, 15, false),
};

static const struct mtk_pin_field_calc mt8518_pin_dir_range[] = {
	PIN_FIELD(0, 119, 0x0, 0x10, 0, 1),
};

static const struct mtk_pin_field_calc mt8518_pin_di_range[] = {
	PIN_FIELD(0, 119, 0x200, 0x10, 0, 1),
};

static const struct mtk_pin_field_calc mt8518_pin_do_range[] = {
	PIN_FIELD(0, 119, 0x100, 0x10, 0, 1),
};

static const struct mtk_pin_field_calc mt8518_pin_ies_range[] = {
	PIN_FIELD(0, 2, 0x900, 0x10, 0, 1),
	PIN_FIELD(3, 3, 0x920, 0x10, 9, 1),
	PIN_FIELD(4, 4, 0x920, 0x10, 8, 1),
	PIN_FIELD(5, 5, 0x920, 0x10, 7, 1),
	PIN_FIELD(6, 6, 0x920, 0x10, 6, 1),
	PIN_FIELD(7, 7, 0x920, 0x10, 10, 1),
	PIN_FIELD(8, 8, 0x920, 0x10, 1, 1),
	PIN_FIELD(9, 9, 0x920, 0x10, 0, 1),
	PIN_FIELD(10, 10, 0x920, 0x10, 5, 1),
	PIN_FIELD(11, 11, 0x920, 0x10, 4, 1),
	PIN_FIELD(12, 12, 0x920, 0x10, 3, 1),
	PIN_FIELD(13, 13, 0x920, 0x10, 2, 1),
	PIN_FIELD(14, 14, 0x900, 0x10, 1, 1),
	PIN_FIELD(15, 15, 0x900, 0x10, 2, 1),
	PIN_FIELD(16, 16, 0x900, 0x10, 3, 1),
	PIN_FIELD(17, 20, 0x900, 0x10, 4, 1),
	PIN_FIELD(21, 22, 0x900, 0x10, 5, 1),
	PIN_FIELD(23, 27, 0x910, 0x10, 15, 1),
	PIN_FIELD(28, 28, 0x900, 0x10, 6, 1),
	PIN_FIELD(29, 29, 0x930, 0x10, 2, 1),
	PIN_FIELD(30, 30, 0x930, 0x10, 1, 1),
	PIN_FIELD(31, 31, 0x930, 0x10, 6, 1),
	PIN_FIELD(32, 32, 0x930, 0x10, 5, 1),
	PIN_FIELD(33, 33, 0x930, 0x10, 4, 1),
	PIN_FIELD(34, 35, 0x930, 0x10, 3, 1),
	PIN_FIELD(36, 39, 0x900, 0x10, 7, 1),
	PIN_FIELD(40, 41, 0x900, 0x10, 8, 1),
	PIN_FIELD(42, 44, 0x900, 0x10, 9, 1),
	PIN_FIELD(45, 47, 0x900, 0x10, 10, 1),
	PIN_FIELD(48, 51, 0x900, 0x10, 11, 1),
	PIN_FIELD(52, 55, 0x900, 0x10, 12, 1),
	PIN_FIELD(56, 56, 0x900, 0x10, 13, 1),
	PIN_FIELD(57, 57, 0x900, 0x10, 14, 1),
	PIN_FIELD(58, 58, 0x900, 0x10, 15, 1),
	PIN_FIELD(59, 60, 0x910, 0x10, 0, 1),

	PIN_FIELD(61, 61, 0x910, 0x10, 1, 1),
	PIN_FIELD(62, 62, 0x910, 0x10, 2, 1),
	PIN_FIELD(63, 69, 0x910, 0x10, 3, 1),
	PIN_FIELD(70, 70, 0x910, 0x10, 4, 1),
	PIN_FIELD(71, 76, 0x910, 0x10, 5, 1),
	PIN_FIELD(77, 80, 0x910, 0x10, 6, 1),
	PIN_FIELD(81, 87, 0x910, 0x10, 7, 1),
	PIN_FIELD(88, 97, 0x910, 0x10, 8, 1),
	PIN_FIELD(98, 103, 0x910, 0x10, 9, 1),
	PIN_FIELD(104, 107, 0x910, 0x10, 10, 1),
	PIN_FIELD(108, 109, 0x910, 0x10, 11, 1),
	PIN_FIELD(110, 111, 0x910, 0x10, 12, 1),
	PIN_FIELD(112, 113, 0x910, 0x10, 13, 1),
	PIN_FIELD(114, 114, 0x920, 0x10, 12, 1),
	PIN_FIELD(115, 115, 0x920, 0x10, 11, 1),
	PIN_FIELD(116, 116, 0x930, 0x10, 0, 1),
	PIN_FIELD(117, 117, 0x920, 0x10, 15, 1),
	PIN_FIELD(118, 118, 0x920, 0x10, 14, 1),
	PIN_FIELD(119, 119, 0x920, 0x10, 13, 1),
};

static const struct mtk_pin_field_calc mt8518_pin_smt_range[] = {
	PIN_FIELD(0, 2, 0xA00, 0x10, 0, 1),
	PIN_FIELD(3, 3, 0xA20, 0x10, 9, 1),
	PIN_FIELD(4, 4, 0xA20, 0x10, 8, 1),
	PIN_FIELD(5, 5, 0xA20, 0x10, 7, 1),
	PIN_FIELD(6, 6, 0xA20, 0x10, 6, 1),
	PIN_FIELD(7, 7, 0xA20, 0x10, 10, 1),
	PIN_FIELD(8, 8, 0xA20, 0x10, 1, 1),
	PIN_FIELD(9, 9, 0xA20, 0x10, 0, 1),
	PIN_FIELD(10, 10, 0xA20, 0x10, 5, 1),
	PIN_FIELD(11, 11, 0xA20, 0x10, 4, 1),
	PIN_FIELD(12, 12, 0xA20, 0x10, 3, 1),
	PIN_FIELD(13, 13, 0xA20, 0x10, 2, 1),
	PIN_FIELD(14, 14, 0xA00, 0x10, 1, 1),
	PIN_FIELD(15, 15, 0xA00, 0x10, 2, 1),
	PIN_FIELD(16, 16, 0xA00, 0x10, 3, 1),
	PIN_FIELD(17, 20, 0xA00, 0x10, 4, 1),
	PIN_FIELD(21, 22, 0xA00, 0x10, 5, 1),
	PIN_FIELD(23, 27, 0xA10, 0x10, 15, 1),
	PIN_FIELD(28, 28, 0xA00, 0x10, 6, 1),
	PIN_FIELD(29, 29, 0xA30, 0x10, 2, 1),
	PIN_FIELD(30, 30, 0xA30, 0x10, 1, 1),
	PIN_FIELD(31, 31, 0xA30, 0x10, 6, 1),
	PIN_FIELD(32, 32, 0xA30, 0x10, 5, 1),
	PIN_FIELD(33, 33, 0xA30, 0x10, 4, 1),
	PIN_FIELD(34, 35, 0xA30, 0x10, 3, 1),
	PIN_FIELD(36, 39, 0xA00, 0x10, 7, 1),
	PIN_FIELD(40, 41, 0xA00, 0x10, 8, 1),
	PIN_FIELD(42, 44, 0xA00, 0x10, 9, 1),
	PIN_FIELD(45, 47, 0xA00, 0x10, 10, 1),
	PIN_FIELD(48, 51, 0xA00, 0x10, 11, 1),
	PIN_FIELD(52, 55, 0xA00, 0x10, 12, 1),
	PIN_FIELD(56, 56, 0xA00, 0x10, 13, 1),
	PIN_FIELD(57, 57, 0xA00, 0x10, 14, 1),
	PIN_FIELD(58, 58, 0xA00, 0x10, 15, 1),
	PIN_FIELD(59, 60, 0xA10, 0x10, 0, 1),

	PIN_FIELD(61, 61, 0xA10, 0x10, 1, 1),
	PIN_FIELD(62, 62, 0xA10, 0x10, 2, 1),
	PIN_FIELD(63, 69, 0xA10, 0x10, 3, 1),
	PIN_FIELD(70, 70, 0xA10, 0x10, 4, 1),
	PIN_FIELD(71, 76, 0xA10, 0x10, 5, 1),
	PIN_FIELD(77, 80, 0xA10, 0x10, 6, 1),
	PIN_FIELD(81, 87, 0xA10, 0x10, 7, 1),
	PIN_FIELD(88, 97, 0xA10, 0x10, 8, 1),
	PIN_FIELD(98, 103, 0xA10, 0x10, 9, 1),
	PIN_FIELD(104, 107, 0xA10, 0x10, 10, 1),
	PIN_FIELD(108, 109, 0xA10, 0x10, 11, 1),
	PIN_FIELD(110, 111, 0xA10, 0x10, 12, 1),
	PIN_FIELD(112, 113, 0xA10, 0x10, 13, 1),
	PIN_FIELD(114, 114, 0xA20, 0x10, 12, 1),
	PIN_FIELD(115, 115, 0xA20, 0x10, 11, 1),
	PIN_FIELD(116, 116, 0xA30, 0x10, 0, 1),
	PIN_FIELD(117, 117, 0xA20, 0x10, 15, 1),
	PIN_FIELD(118, 118, 0xA20, 0x10, 14, 1),
	PIN_FIELD(119, 119, 0xA20, 0x10, 13, 1),
};

static const struct mtk_pin_field_calc mt8518_pin_pullen_range[] = {
	PIN_FIELD(14, 15, 0x500, 0x10, 14, 1),
	PIN_FIELD(16, 28, 0x510, 0x10, 0, 1),
	PIN_FIELD(36, 47, 0x520, 0x10, 4, 1),
	PIN_FIELD(48, 63, 0x530, 0x10, 0, 1),
	PIN_FIELD(64, 79, 0x540, 0x10, 0, 1),
	PIN_FIELD(80, 95, 0x550, 0x10, 0, 1),
	PIN_FIELD(96, 111, 0x560, 0x10, 0, 1),
	PIN_FIELD(112, 113, 0x570, 0x10, 0, 1),
};

static const struct mtk_pin_field_calc mt8518_pin_pullsel_range[] = {
	PIN_FIELD(14, 15, 0x600, 0x10, 14, 1),
	PIN_FIELD(16, 28, 0x610, 0x10, 0, 1),
	PIN_FIELD(36, 47, 0x620, 0x10, 4, 1),
	PIN_FIELD(48, 63, 0x630, 0x10, 0, 1),
	PIN_FIELD(64, 79, 0x640, 0x10, 0, 1),
	PIN_FIELD(80, 95, 0x650, 0x10, 0, 1),
	PIN_FIELD(96, 111, 0x660, 0x10, 0, 1),
	PIN_FIELD(112, 113, 0x670, 0x10, 0, 1),
};

static const struct mtk_pin_field_calc mt8518_pin_drv_range[] = {
	PIN_FIELD(0, 2, 0xd70, 0x10, 8, 4),
	PIN_FIELD(3, 6, 0xd70, 0x10, 0, 4),
	PIN_FIELD(7, 7, 0xd70, 0x10, 4, 4),
	PIN_FIELD(8, 8, 0xd60, 0x10, 8, 4),
	PIN_FIELD(9, 9, 0xd60, 0x10, 12, 4),
	PIN_FIELD(10, 13, 0xd70, 0x10, 0, 4),
	PIN_FIELD(14, 14, 0xd50, 0x10, 8, 4),
	PIN_FIELD(15, 15, 0xd20, 0x10, 4, 4),
	PIN_FIELD(16, 16, 0xd50, 0x10, 8, 4),
	PIN_FIELD(17, 20, 0xd20, 0x10, 12, 4),
	PIN_FIELD(23, 27, 0xd30, 0x10, 8, 4),
	PIN_FIELD(28, 28, 0xd10, 0x10, 0, 4),
	PIN_FIELD(29, 29, 0xd40, 0x10, 12, 4),
	PIN_FIELD(30, 30, 0xd50, 0x10, 0, 4),
	PIN_FIELD(31, 35, 0xd50, 0x10, 4, 4),
	PIN_FIELD(36, 41, 0xd00, 0x10, 0, 4),
	PIN_FIELD(42, 47, 0xd00, 0x10, 4, 4),
	PIN_FIELD(48, 51, 0xd00, 0x10, 8, 4),
	PIN_FIELD(52, 55, 0xd10, 0x10, 12, 4),
	PIN_FIELD(56, 56, 0xdb0, 0x10, 4, 4),
	PIN_FIELD(57, 58, 0xd00, 0x10, 8, 4),
	PIN_FIELD(59, 62, 0xd00, 0x10, 12, 4),
	PIN_FIELD(63, 68, 0xd90, 0x10, 12, 4),
	PIN_FIELD(69, 69, 0xda0, 0x10, 0, 4),
	PIN_FIELD(70, 70, 0xda0, 0x10, 12, 4),
	PIN_FIELD(71, 73, 0xd80, 0x10, 12, 4),
	PIN_FIELD(74, 76, 0xd90, 0x10, 0, 4),
	PIN_FIELD(77, 80, 0xd20, 0x10, 0, 4),
	PIN_FIELD(81, 87, 0xd80, 0x10, 8, 4),
	PIN_FIELD(88, 97, 0xd30, 0x10, 0, 4),
	PIN_FIELD(98, 103, 0xd10, 0x10, 4, 4),
	PIN_FIELD(104, 105, 0xd40, 0x10, 8, 4),
	PIN_FIELD(106, 107, 0xd10, 0x10, 8, 4),
	PIN_FIELD(114, 114, 0xd50, 0x10, 12, 4),
	PIN_FIELD(115, 115, 0xd60, 0x10, 0, 4),
	PIN_FIELD(116, 119, 0xd60, 0x10, 4, 4),
};

static const struct mtk_pin_reg_calc mt8518_reg_cals[] = {
	[PINCTRL_PIN_REG_MODE] = MTK_RANGE(mt8518_pin_mode_range),
	[PINCTRL_PIN_REG_DIR] = MTK_RANGE(mt8518_pin_dir_range),
	[PINCTRL_PIN_REG_DI] = MTK_RANGE(mt8518_pin_di_range),
	[PINCTRL_PIN_REG_DO] = MTK_RANGE(mt8518_pin_do_range),
	[PINCTRL_PIN_REG_IES] = MTK_RANGE(mt8518_pin_ies_range),
	[PINCTRL_PIN_REG_SMT] = MTK_RANGE(mt8518_pin_smt_range),
	[PINCTRL_PIN_REG_PULLSEL] = MTK_RANGE(mt8518_pin_pullsel_range),
	[PINCTRL_PIN_REG_PULLEN] = MTK_RANGE(mt8518_pin_pullen_range),
	[PINCTRL_PIN_REG_DRV] = MTK_RANGE(mt8518_pin_drv_range),
};

static const struct mtk_pin_desc mt8518_pins[] = {
	MTK_PIN(0, "NFI_NCEB0", DRV_GRP4),
	MTK_PIN(1, "NFI_NREB", DRV_GRP4),
	MTK_PIN(2, "NFI_NRNB", DRV_GRP4),
	MTK_PIN(3, "MSDC0_DAT7", DRV_GRP4),
	MTK_PIN(4, "MSDC0_DAT6", DRV_GRP4),
	MTK_PIN(5, "MSDC0_DAT5", DRV_GRP4),
	MTK_PIN(6, "MSDC0_DAT4", DRV_GRP4),
	MTK_PIN(7, "MSDC0_RSTB", DRV_GRP4),
	MTK_PIN(8, "MSDC0_CMD", DRV_GRP4),
	MTK_PIN(9, "MSDC0_CLK", DRV_GRP4),
	MTK_PIN(10, "MSDC0_DAT3", DRV_GRP4),
	MTK_PIN(11, "MSDC0_DAT2", DRV_GRP4),
	MTK_PIN(12, "MSDC0_DAT1", DRV_GRP4),
	MTK_PIN(13, "MSDC0_DAT0", DRV_GRP4),
	MTK_PIN(14, "RTC32K_CK", DRV_GRP2),
	MTK_PIN(15, "WATCHDOG", DRV_GRP2),
	MTK_PIN(16, "SUSPEND", DRV_GRP2),
	MTK_PIN(17, "JTMS", DRV_GRP2),
	MTK_PIN(18, "JTCK", DRV_GRP2),
	MTK_PIN(19, "JTDI", DRV_GRP2),
	MTK_PIN(20, "JTDO", DRV_GRP2),
	MTK_PIN(21, "SDA3", DRV_GRP2),
	MTK_PIN(22, "SCL3", DRV_GRP2),
	MTK_PIN(23, "PWRAP_SPI_CLK", DRV_GRP2),
	MTK_PIN(24, "PWRAP_SPI_CSN", DRV_GRP2),
	MTK_PIN(25, "PWRAP_SPI_MOSI", DRV_GRP2),
	MTK_PIN(26, "PWRAP_SPI_MISO", DRV_GRP2),
	MTK_PIN(27, "PWRAP_INT", DRV_GRP2),
	MTK_PIN(28, "EINT22", DRV_GRP2),
	MTK_PIN(29, "MSDC2_CMD", DRV_GRP4),
	MTK_PIN(30, "MSDC2_CLK", DRV_GRP4),
	MTK_PIN(31, "MSDC2_DAT0", DRV_GRP4),
	MTK_PIN(32, "MSDC2_DAT1", DRV_GRP4),
	MTK_PIN(33, "MSDC2_DAT2", DRV_GRP4),
	MTK_PIN(34, "MSDC2_DAT3", DRV_GRP4),
	MTK_PIN(35, "MSDC2_DS", DRV_GRP4),
	MTK_PIN(36, "EINT0", DRV_GRP0),
	MTK_PIN(37, "EINT1", DRV_GRP0),
	MTK_PIN(38, "EINT2", DRV_GRP0),
	MTK_PIN(39, "EINT3", DRV_GRP0),
	MTK_PIN(40, "EINT4", DRV_GRP0),
	MTK_PIN(41, "EINT5", DRV_GRP0),
	MTK_PIN(42, "EINT6", DRV_GRP0),
	MTK_PIN(43, "EINT7", DRV_GRP0),
	MTK_PIN(44, "EINT8", DRV_GRP0),
	MTK_PIN(45, "EINT9", DRV_GRP0),
	MTK_PIN(46, "EINT10", DRV_GRP0),
	MTK_PIN(47, "EINT11", DRV_GRP0),
	MTK_PIN(48, "EINT12", DRV_GRP0),
	MTK_PIN(49, "EINT13", DRV_GRP0),
	MTK_PIN(50, "EINT14", DRV_GRP0),
	MTK_PIN(51, "EINT15", DRV_GRP0),
	MTK_PIN(52, "URXD1", DRV_GRP0),
	MTK_PIN(53, "UTXD1", DRV_GRP0),
	MTK_PIN(54, "URTS1", DRV_GRP0),
	MTK_PIN(55, "UCTS1", DRV_GRP0),
	MTK_PIN(56, "IR", DRV_GRP0),
	MTK_PIN(57, "EINT16", DRV_GRP0),
	MTK_PIN(58, "EINT17", DRV_GRP0),
	MTK_PIN(59, "EINT18", DRV_GRP0),
	MTK_PIN(60, "EINT19", DRV_GRP0),
	MTK_PIN(61, "EINT20", DRV_GRP0),
	MTK_PIN(62, "EINT21", DRV_GRP0),
	MTK_PIN(63, "I2SO_MCLK", DRV_GRP0),
	MTK_PIN(64, "I2SO_BCK", DRV_GRP0),
	MTK_PIN(65, "I2SO_LRCK", DRV_GRP0),
	MTK_PIN(66, "I2SO_D0", DRV_GRP0),
	MTK_PIN(67, "I2SO_D1", DRV_GRP0),
	MTK_PIN(68, "I2SO_D2", DRV_GRP0),
	MTK_PIN(69, "I2SO_D3", DRV_GRP0),
	MTK_PIN(70, "SPDIF_IN0", DRV_GRP0),
	MTK_PIN(71, "DMIC_CLK0", DRV_GRP0),
	MTK_PIN(72, "DMIC_CLK1", DRV_GRP0),
	MTK_PIN(73, "DMIC_DAT0", DRV_GRP0),
	MTK_PIN(74, "DMIC_DAT1", DRV_GRP0),
	MTK_PIN(75, "DMIC_DAT2", DRV_GRP0),
	MTK_PIN(76, "DMIC_DAT3", DRV_GRP0),
	MTK_PIN(77, "TDM_MCLK", DRV_GRP0),
	MTK_PIN(78, "TDM_BCK", DRV_GRP0),
	MTK_PIN(79, "TDM_LRCK", DRV_GRP0),
	MTK_PIN(80, "TDM_DI", DRV_GRP0),
	MTK_PIN(81, "I2SIN_D0", DRV_GRP0),
	MTK_PIN(82, "I2SIN_D1", DRV_GRP0),
	MTK_PIN(83, "I2SIN_D2", DRV_GRP0),
	MTK_PIN(84, "I2SIN_D3", DRV_GRP0),
	MTK_PIN(85, "I2SIN_MCLK", DRV_GRP0),
	MTK_PIN(86, "I2SIN_BCK", DRV_GRP0),
	MTK_PIN(87, "I2SIN_LRCK", DRV_GRP0),
	MTK_PIN(88, "SPI1_CS", DRV_GRP0),
	MTK_PIN(89, "SPI1_CK", DRV_GRP0),
	MTK_PIN(90, "SPI1_MI", DRV_GRP0),
	MTK_PIN(91, "SPI1_MO", DRV_GRP0),
	MTK_PIN(92, "SPI2_CS", DRV_GRP0),
	MTK_PIN(93, "SPI2_CK", DRV_GRP0),
	MTK_PIN(94, "SPI2_MI0", DRV_GRP0),
	MTK_PIN(95, "SPI2_MI1", DRV_GRP0),
	MTK_PIN(96, "SPI2_MI2", DRV_GRP0),
	MTK_PIN(97, "SPI2_MI3", DRV_GRP0),
	MTK_PIN(98, "SW_RESET_DSP", DRV_GRP0),
	MTK_PIN(99, "GPIO1", DRV_GRP0),
	MTK_PIN(100, "GPIO2", DRV_GRP0),
	MTK_PIN(101, "GPIO3", DRV_GRP0),
	MTK_PIN(102, "GPIO4", DRV_GRP0),
	MTK_PIN(103, "RTC32K_DSP", DRV_GRP0),
	MTK_PIN(104, "URXD0", DRV_GRP2),
	MTK_PIN(105, "UTXD0", DRV_GRP2),
	MTK_PIN(106, "URXD2", DRV_GRP2),
	MTK_PIN(107, "UTXD2", DRV_GRP2),
	MTK_PIN(108, "SDA1", DRV_GRP4),
	MTK_PIN(109, "SCL1", DRV_GRP4),
	MTK_PIN(110, "SDA0", DRV_GRP4),
	MTK_PIN(111, "SCL0", DRV_GRP4),
	MTK_PIN(112, "SDA2", DRV_GRP4),
	MTK_PIN(113, "SCL2", DRV_GRP4),
	MTK_PIN(114, "MSDC1_CMD", DRV_GRP4),
	MTK_PIN(115, "MSDC1_CLK", DRV_GRP4),
	MTK_PIN(116, "MSDC1_DAT0", DRV_GRP4),
	MTK_PIN(117, "MSDC1_DAT1", DRV_GRP4),
	MTK_PIN(118, "MSDC1_DAT2", DRV_GRP4),
	MTK_PIN(119, "MSDC1_DAT3", DRV_GRP4),
};

/* List all groups consisting of these pins dedicated to the enablement of
 * certain hardware block and the corresponding mode for all of the pins.
 * The hardware probably has multiple combinations of these pinouts.
 */

/* UART */
static const int mt8518_uart0_0_rxd_txd_pins[]		= { 104, 105, };
static const int mt8518_uart0_0_rxd_txd_funcs[]		= {  1,  1, };
static const int mt8518_uart1_0_rxd_txd_pins[]		= { 52, 53, };
static const int mt8518_uart1_0_rxd_txd_funcs[]		= {  1,  1, };
static const int mt8518_uart2_0_rxd_txd_pins[]		= { 106, 107, };
static const int mt8518_uart2_0_rxd_txd_funcs[]		= {  1,  1, };

/* Joint those groups owning the same capability in user point of view which
 * allows that people tend to use through the device tree.
 */
static const char *const mt8518_uart_groups[] = { "uart0_0_rxd_txd",
						"uart1_0_rxd_txd",
						"uart2_0_rxd_txd", };

/* MMC0 */
static const int mt8518_msdc0_pins[] = { 3, 4, 5, 6, 7, 8, 9, 10, 11,
					 12, 13, };
static const int mt8518_msdc0_funcs[] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, };

static const struct mtk_group_desc mt8518_groups[] = {
	PINCTRL_PIN_GROUP("uart0_0_rxd_txd", mt8518_uart0_0_rxd_txd),
	PINCTRL_PIN_GROUP("uart1_0_rxd_txd", mt8518_uart1_0_rxd_txd),
	PINCTRL_PIN_GROUP("uart2_0_rxd_txd", mt8518_uart2_0_rxd_txd),

	PINCTRL_PIN_GROUP("msdc0", mt8518_msdc0),
};

static const char *const mt8518_msdc_groups[] = { "msdc0" };

static const struct mtk_function_desc mt8518_functions[] = {
	{"uart", mt8518_uart_groups, ARRAY_SIZE(mt8518_uart_groups)},
	{"msdc", mt8518_msdc_groups, ARRAY_SIZE(mt8518_msdc_groups)},
};

static const struct mtk_pinctrl_soc mt8518_data = {
	.name = "mt8518_pinctrl",
	.reg_cal = mt8518_reg_cals,
	.pins = mt8518_pins,
	.npins = ARRAY_SIZE(mt8518_pins),
	.grps = mt8518_groups,
	.ngrps = ARRAY_SIZE(mt8518_groups),
	.funcs = mt8518_functions,
	.nfuncs = ARRAY_SIZE(mt8518_functions),
	.gpio_mode = 0,
	.rev = MTK_PINCTRL_V1,
};

static int mtk_pinctrl_mt8518_probe(struct udevice *dev)
{
	return mtk_pinctrl_common_probe(dev, &mt8518_data);
}

static const struct udevice_id mt8518_pctrl_match[] = {
	{ .compatible = "mediatek,mt8518-pinctrl" },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(mt8518_pinctrl) = {
	.name = "mt8518_pinctrl",
	.id = UCLASS_PINCTRL,
	.of_match = mt8518_pctrl_match,
	.ops = &mtk_pinctrl_ops,
	.probe = mtk_pinctrl_mt8518_probe,
	.priv_auto	= sizeof(struct mtk_pinctrl_priv),
};
