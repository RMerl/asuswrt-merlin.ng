// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2018 MediaTek Inc.
 * Author: Ryder Lee <ryder.lee@mediatek.com>
 */

#include <dm.h>

#include "pinctrl-mtk-common.h"

#define PIN_BOND_REG0		0xb10
#define PIN_BOND_REG1		0xf20
#define PIN_BOND_REG2		0xef0
#define BOND_PCIE_CLR		(0x77 << 3)
#define BOND_I2S_CLR		0x3
#define BOND_MSDC0E_CLR		0x1

#define PIN_FIELD15(_s_pin, _e_pin, _s_addr, _x_addrs, _s_bit, _x_bits)	\
	PIN_FIELD_CALC(_s_pin, _e_pin, _s_addr, _x_addrs, _s_bit,	\
		       _x_bits, 15, false)

#define PIN_FIELD16(_s_pin, _e_pin, _s_addr, _x_addrs, _s_bit, _x_bits)	\
	PIN_FIELD_CALC(_s_pin, _e_pin, _s_addr, _x_addrs, _s_bit,	\
		       _x_bits, 16, false)

#define PINS_FIELD16(_s_pin, _e_pin, _s_addr, _x_addrs, _s_bit, _x_bits)\
	PIN_FIELD_CALC(_s_pin, _e_pin, _s_addr, _x_addrs, _s_bit,	\
		       _x_bits, 16, true)

static const struct mtk_pin_field_calc mt7623_pin_mode_range[] = {
	PIN_FIELD15(0, 278, 0x760, 0x10, 0, 3),
};

static const struct mtk_pin_field_calc mt7623_pin_dir_range[] = {
	PIN_FIELD16(0, 175, 0x0, 0x10, 0, 1),
	PIN_FIELD16(176, 278, 0xc0, 0x10, 0, 1),
};

static const struct mtk_pin_field_calc mt7623_pin_di_range[] = {
	PIN_FIELD16(0, 278, 0x630, 0x10, 0, 1),
};

static const struct mtk_pin_field_calc mt7623_pin_do_range[] = {
	PIN_FIELD16(0, 278, 0x500, 0x10, 0, 1),
};

static const struct mtk_pin_field_calc mt7623_pin_ies_range[] = {
	PINS_FIELD16(0, 6, 0xb20, 0x10, 0, 1),
	PINS_FIELD16(7, 9, 0xb20, 0x10, 1, 1),
	PINS_FIELD16(10, 13, 0xb30, 0x10, 3, 1),
	PINS_FIELD16(14, 15, 0xb30, 0x10, 13, 1),
	PINS_FIELD16(16, 17, 0xb40, 0x10, 7, 1),
	PINS_FIELD16(18, 29, 0xb40, 0x10, 13, 1),
	PINS_FIELD16(30, 32, 0xb40, 0x10, 7, 1),
	PINS_FIELD16(33, 37, 0xb40, 0x10, 13, 1),
	PIN_FIELD16(38, 38, 0xb20, 0x10, 13, 1),
	PINS_FIELD16(39, 42, 0xb40, 0x10, 13, 1),
	PINS_FIELD16(43, 45, 0xb20, 0x10, 10, 1),
	PINS_FIELD16(47, 48, 0xb20, 0x10, 11, 1),
	PIN_FIELD16(49, 49, 0xb20, 0x10, 12, 1),
	PINS_FIELD16(50, 52, 0xb20, 0x10, 13, 1),
	PINS_FIELD16(53, 56, 0xb20, 0x10, 14, 1),
	PINS_FIELD16(57, 58, 0xb20, 0x10, 15, 1),
	PIN_FIELD16(59, 59, 0xb30, 0x10, 10, 1),
	PINS_FIELD16(60, 62, 0xb30, 0x10, 0, 1),
	PINS_FIELD16(63, 65, 0xb30, 0x10, 1, 1),
	PINS_FIELD16(66, 71, 0xb30, 0x10, 2, 1),
	PINS_FIELD16(72, 74, 0xb20, 0x10, 12, 1),
	PINS_FIELD16(75, 76, 0xb30, 0x10, 3, 1),
	PINS_FIELD16(77, 78, 0xb30, 0x10, 4, 1),
	PINS_FIELD16(79, 82, 0xb30, 0x10, 5, 1),
	PINS_FIELD16(83, 84, 0xb30, 0x10, 2, 1),
	PIN_FIELD16(85, 85, 0xda0, 0x10, 4, 1),
	PIN_FIELD16(86, 86, 0xd90, 0x10, 4, 1),
	PINS_FIELD16(87, 90, 0xdb0, 0x10, 4, 1),
	PINS_FIELD16(101, 104, 0xb30, 0x10, 6, 1),
	PIN_FIELD16(105, 105, 0xd40, 0x10, 4, 1),
	PIN_FIELD16(106, 106, 0xd30, 0x10, 4, 1),
	PINS_FIELD16(107, 110, 0xd50, 0x10, 4, 1),
	PINS_FIELD16(111, 115, 0xce0, 0x10, 4, 1),
	PIN_FIELD16(116, 116, 0xcd0, 0x10, 4, 1),
	PIN_FIELD16(117, 117, 0xcc0, 0x10, 4, 1),
	PINS_FIELD16(118, 121, 0xce0, 0x10, 4, 1),
	PINS_FIELD16(122, 125, 0xb30, 0x10, 7, 1),
	PIN_FIELD16(126, 126, 0xb20, 0x10, 12, 1),
	PINS_FIELD16(127, 142, 0xb30, 0x10, 9, 1),
	PINS_FIELD16(143, 160, 0xb30, 0x10, 10, 1),
	PINS_FIELD16(161, 168, 0xb30, 0x10, 12, 1),
	PINS_FIELD16(169, 183, 0xb30, 0x10, 10, 1),
	PINS_FIELD16(184, 186, 0xb30, 0x10, 9, 1),
	PIN_FIELD16(187, 187, 0xb30, 0x10, 14, 1),
	PIN_FIELD16(188, 188, 0xb20, 0x10, 13, 1),
	PINS_FIELD16(189, 193, 0xb30, 0x10, 15, 1),
	PINS_FIELD16(194, 198, 0xb40, 0x10, 0, 1),
	PIN_FIELD16(199, 199, 0xb20, 0x10, 1, 1),
	PINS_FIELD16(200, 202, 0xb40, 0x10, 1, 1),
	PINS_FIELD16(203, 207, 0xb40, 0x10, 2, 1),
	PINS_FIELD16(208, 209, 0xb40, 0x10, 3, 1),
	PIN_FIELD16(210, 210, 0xb40, 0x10, 4, 1),
	PINS_FIELD16(211, 235, 0xb40, 0x10, 5, 1),
	PINS_FIELD16(236, 241, 0xb40, 0x10, 6, 1),
	PINS_FIELD16(242, 243, 0xb40, 0x10, 7, 1),
	PINS_FIELD16(244, 247, 0xb40, 0x10, 8, 1),
	PIN_FIELD16(248, 248, 0xb40, 0x10, 9, 1),
	PINS_FIELD16(249, 257, 0xfc0, 0x10, 4, 1),
	PIN_FIELD16(258, 258, 0xcb0, 0x10, 4, 1),
	PIN_FIELD16(259, 259, 0xc90, 0x10, 4, 1),
	PIN_FIELD16(260, 260, 0x3a0, 0x10, 4, 1),
	PIN_FIELD16(261, 261, 0xd50, 0x10, 4, 1),
	PINS_FIELD16(262, 277, 0xb40, 0x10, 12, 1),
	PIN_FIELD16(278, 278, 0xb40, 0x10, 13, 1),
};

static const struct mtk_pin_field_calc mt7623_pin_smt_range[] = {
	PINS_FIELD16(0, 6, 0xb50, 0x10, 0, 1),
	PINS_FIELD16(7, 9, 0xb50, 0x10, 1, 1),
	PINS_FIELD16(10, 13, 0xb60, 0x10, 3, 1),
	PINS_FIELD16(14, 15, 0xb60, 0x10, 13, 1),
	PINS_FIELD16(16, 17, 0xb70, 0x10, 7, 1),
	PINS_FIELD16(18, 29, 0xb70, 0x10, 13, 1),
	PINS_FIELD16(30, 32, 0xb70, 0x10, 7, 1),
	PINS_FIELD16(33, 37, 0xb70, 0x10, 13, 1),
	PIN_FIELD16(38, 38, 0xb50, 0x10, 13, 1),
	PINS_FIELD16(39, 42, 0xb70, 0x10, 13, 1),
	PINS_FIELD16(43, 45, 0xb50, 0x10, 10, 1),
	PINS_FIELD16(47, 48, 0xb50, 0x10, 11, 1),
	PIN_FIELD16(49, 49, 0xb50, 0x10, 12, 1),
	PINS_FIELD16(50, 52, 0xb50, 0x10, 13, 1),
	PINS_FIELD16(53, 56, 0xb50, 0x10, 14, 1),
	PINS_FIELD16(57, 58, 0xb50, 0x10, 15, 1),
	PIN_FIELD16(59, 59, 0xb60, 0x10, 10, 1),
	PINS_FIELD16(60, 62, 0xb60, 0x10, 0, 1),
	PINS_FIELD16(63, 65, 0xb60, 0x10, 1, 1),
	PINS_FIELD16(66, 71, 0xb60, 0x10, 2, 1),
	PINS_FIELD16(72, 74, 0xb50, 0x10, 12, 1),
	PINS_FIELD16(75, 76, 0xb60, 0x10, 3, 1),
	PINS_FIELD16(77, 78, 0xb60, 0x10, 4, 1),
	PINS_FIELD16(79, 82, 0xb60, 0x10, 5, 1),
	PINS_FIELD16(83, 84, 0xb60, 0x10, 2, 1),
	PIN_FIELD16(85, 85, 0xda0, 0x10, 11, 1),
	PIN_FIELD16(86, 86, 0xd90, 0x10, 11, 1),
	PIN_FIELD16(87, 87, 0xdc0, 0x10, 3, 1),
	PIN_FIELD16(88, 88, 0xdc0, 0x10, 7, 1),
	PIN_FIELD16(89, 89, 0xdc0, 0x10, 11, 1),
	PIN_FIELD16(90, 90, 0xdc0, 0x10, 15, 1),
	PINS_FIELD16(101, 104, 0xb60, 0x10, 6, 1),
	PIN_FIELD16(105, 105, 0xd40, 0x10, 11, 1),
	PIN_FIELD16(106, 106, 0xd30, 0x10, 11, 1),
	PIN_FIELD16(107, 107, 0xd60, 0x10, 3, 1),
	PIN_FIELD16(108, 108, 0xd60, 0x10, 7, 1),
	PIN_FIELD16(109, 109, 0xd60, 0x10, 11, 1),
	PIN_FIELD16(110, 110, 0xd60, 0x10, 15, 1),
	PIN_FIELD16(111, 111, 0xd00, 0x10, 15, 1),
	PIN_FIELD16(112, 112, 0xd00, 0x10, 11, 1),
	PIN_FIELD16(113, 113, 0xd00, 0x10, 7, 1),
	PIN_FIELD16(114, 114, 0xd00, 0x10, 3, 1),
	PIN_FIELD16(115, 115, 0xd10, 0x10, 3, 1),
	PIN_FIELD16(116, 116, 0xcd0, 0x10, 11, 1),
	PIN_FIELD16(117, 117, 0xcc0, 0x10, 11, 1),
	PIN_FIELD16(118, 118, 0xcf0, 0x10, 15, 1),
	PIN_FIELD16(119, 119, 0xcf0, 0x10, 7, 1),
	PIN_FIELD16(120, 120, 0xcf0, 0x10, 3, 1),
	PIN_FIELD16(121, 121, 0xcf0, 0x10, 7, 1),
	PINS_FIELD16(122, 125, 0xb60, 0x10, 7, 1),
	PIN_FIELD16(126, 126, 0xb50, 0x10, 12, 1),
	PINS_FIELD16(127, 142, 0xb60, 0x10, 9, 1),
	PINS_FIELD16(143, 160, 0xb60, 0x10, 10, 1),
	PINS_FIELD16(161, 168, 0xb60, 0x10, 12, 1),
	PINS_FIELD16(169, 183, 0xb60, 0x10, 10, 1),
	PINS_FIELD16(184, 186, 0xb60, 0x10, 9, 1),
	PIN_FIELD16(187, 187, 0xb60, 0x10, 14, 1),
	PIN_FIELD16(188, 188, 0xb50, 0x10, 13, 1),
	PINS_FIELD16(189, 193, 0xb60, 0x10, 15, 1),
	PINS_FIELD16(194, 198, 0xb70, 0x10, 0, 1),
	PIN_FIELD16(199, 199, 0xb50, 0x10, 1, 1),
	PINS_FIELD16(200, 202, 0xb70, 0x10, 1, 1),
	PINS_FIELD16(203, 207, 0xb70, 0x10, 2, 1),
	PINS_FIELD16(208, 209, 0xb70, 0x10, 3, 1),
	PIN_FIELD16(210, 210, 0xb70, 0x10, 4, 1),
	PINS_FIELD16(211, 235, 0xb70, 0x10, 5, 1),
	PINS_FIELD16(236, 241, 0xb70, 0x10, 6, 1),
	PINS_FIELD16(242, 243, 0xb70, 0x10, 7, 1),
	PINS_FIELD16(244, 247, 0xb70, 0x10, 8, 1),
	PIN_FIELD16(248, 248, 0xb70, 0x10, 9, 10),
	PIN_FIELD16(249, 249, 0x140, 0x10, 3, 1),
	PIN_FIELD16(250, 250, 0x130, 0x10, 15, 1),
	PIN_FIELD16(251, 251, 0x130, 0x10, 11, 1),
	PIN_FIELD16(252, 252, 0x130, 0x10, 7, 1),
	PIN_FIELD16(253, 253, 0x130, 0x10, 3, 1),
	PIN_FIELD16(254, 254, 0xf40, 0x10, 15, 1),
	PIN_FIELD16(255, 255, 0xf40, 0x10, 11, 1),
	PIN_FIELD16(256, 256, 0xf40, 0x10, 7, 1),
	PIN_FIELD16(257, 257, 0xf40, 0x10, 3, 1),
	PIN_FIELD16(258, 258, 0xcb0, 0x10, 11, 1),
	PIN_FIELD16(259, 259, 0xc90, 0x10, 11, 1),
	PIN_FIELD16(260, 260, 0x3a0, 0x10, 11, 1),
	PIN_FIELD16(261, 261, 0x0b0, 0x10, 3, 1),
	PINS_FIELD16(262, 277, 0xb70, 0x10, 12, 1),
	PIN_FIELD16(278, 278, 0xb70, 0x10, 13, 1),
};

static const struct mtk_pin_field_calc mt7623_pin_pullen_range[] = {
	PIN_FIELD16(0, 278, 0x150, 0x10, 0, 1),
};

static const struct mtk_pin_field_calc mt7623_pin_pullsel_range[] = {
	PIN_FIELD16(0, 278, 0x280, 0x10, 0, 1),
};

static const struct mtk_pin_field_calc mt7623_pin_drv_range[] = {
	PINS_FIELD16(0, 6, 0xf50, 0x10, 0, 4),
	PINS_FIELD16(7, 9, 0xf50, 0x10, 4, 4),
	PINS_FIELD16(10, 13, 0xf50, 0x10, 4, 4),
	PINS_FIELD16(14, 15, 0xf50, 0x10, 12, 4),
	PINS_FIELD16(16, 17, 0xf60, 0x10, 0, 4),
	PINS_FIELD16(18, 21, 0xf60, 0x10, 0, 4),
	PINS_FIELD16(22, 26, 0xf60, 0x10, 8, 4),
	PINS_FIELD16(27, 29, 0xf60, 0x10, 12, 4),
	PINS_FIELD16(30, 32, 0xf60, 0x10, 0, 4),
	PINS_FIELD16(33, 37, 0xf70, 0x10, 0, 4),
	PIN_FIELD16(38, 38, 0xf70, 0x10, 4, 4),
	PINS_FIELD16(39, 42, 0xf70, 0x10, 8, 4),
	PINS_FIELD16(43, 45, 0xf70, 0x10, 12, 4),
	PINS_FIELD16(47, 48, 0xf80, 0x10, 0, 4),
	PIN_FIELD16(49, 49, 0xf80, 0x10, 4, 4),
	PINS_FIELD16(50, 52, 0xf70, 0x10, 4, 4),
	PINS_FIELD16(53, 56, 0xf80, 0x10, 12, 4),
	PINS_FIELD16(60, 62, 0xf90, 0x10, 8, 4),
	PINS_FIELD16(63, 65, 0xf90, 0x10, 12, 4),
	PINS_FIELD16(66, 71, 0xfa0, 0x10, 0, 4),
	PINS_FIELD16(72, 74, 0xf80, 0x10, 4, 4),
	PIN_FIELD16(85, 85, 0xda0, 0x10, 0, 4),
	PIN_FIELD16(86, 86, 0xd90, 0x10, 0, 4),
	PINS_FIELD16(87, 90, 0xdb0, 0x10, 0, 4),
	PIN_FIELD16(105, 105, 0xd40, 0x10, 0, 4),
	PIN_FIELD16(106, 106, 0xd30, 0x10, 0, 4),
	PINS_FIELD16(107, 110, 0xd50, 0x10, 0, 4),
	PINS_FIELD16(111, 115, 0xce0, 0x10, 0, 4),
	PIN_FIELD16(116, 116, 0xcd0, 0x10, 0, 4),
	PIN_FIELD16(117, 117, 0xcc0, 0x10, 0, 4),
	PINS_FIELD16(118, 121, 0xce0, 0x10, 0, 4),
	PIN_FIELD16(126, 126, 0xf80, 0x10, 4, 4),
	PIN_FIELD16(188, 188, 0xf70, 0x10, 4, 4),
	PINS_FIELD16(189, 193, 0xfe0, 0x10, 8, 4),
	PINS_FIELD16(194, 198, 0xfe0, 0x10, 12, 4),
	PIN_FIELD16(199, 199, 0xf50, 0x10, 4, 4),
	PINS_FIELD16(200, 202, 0xfd0, 0x10, 0, 4),
	PINS_FIELD16(203, 207, 0xfd0, 0x10, 4, 4),
	PINS_FIELD16(208, 209, 0xfd0, 0x10, 8, 4),
	PIN_FIELD16(210, 210, 0xfd0, 0x10, 12, 4),
	PINS_FIELD16(211, 235, 0xff0, 0x10, 0, 4),
	PINS_FIELD16(236, 241, 0xff0, 0x10, 4, 4),
	PINS_FIELD16(242, 243, 0xff0, 0x10, 8, 4),
	PIN_FIELD16(248, 248, 0xf00, 0x10, 0, 4),
	PINS_FIELD16(249, 256, 0xfc0, 0x10, 0, 4),
	PIN_FIELD16(257, 257, 0xce0, 0x10, 0, 4),
	PIN_FIELD16(258, 258, 0xcb0, 0x10, 0, 4),
	PIN_FIELD16(259, 259, 0xc90, 0x10, 0, 4),
	PIN_FIELD16(260, 260, 0x3a0, 0x10, 0, 4),
	PIN_FIELD16(261, 261, 0xd50, 0x10, 0, 4),
	PINS_FIELD16(262, 277, 0xf00, 0x10, 8, 4),
	PIN_FIELD16(278, 278, 0xf70, 0x10, 8, 4),
};

static const struct mtk_pin_reg_calc mt7623_reg_cals[] = {
	[PINCTRL_PIN_REG_MODE] = MTK_RANGE(mt7623_pin_mode_range),
	[PINCTRL_PIN_REG_DIR] = MTK_RANGE(mt7623_pin_dir_range),
	[PINCTRL_PIN_REG_DI] = MTK_RANGE(mt7623_pin_di_range),
	[PINCTRL_PIN_REG_DO] = MTK_RANGE(mt7623_pin_do_range),
	[PINCTRL_PIN_REG_IES] = MTK_RANGE(mt7623_pin_ies_range),
	[PINCTRL_PIN_REG_SMT] = MTK_RANGE(mt7623_pin_smt_range),
	[PINCTRL_PIN_REG_PULLSEL] = MTK_RANGE(mt7623_pin_pullsel_range),
	[PINCTRL_PIN_REG_PULLEN] = MTK_RANGE(mt7623_pin_pullen_range),
	[PINCTRL_PIN_REG_DRV] = MTK_RANGE(mt7623_pin_drv_range),
};

static const struct mtk_pin_desc mt7623_pins[] = {
	MTK_PIN(0, "PWRAP_SPI0_MI", DRV_GRP3),
	MTK_PIN(1, "PWRAP_SPI0_MO", DRV_GRP3),
	MTK_PIN(2, "PWRAP_INT", DRV_GRP3),
	MTK_PIN(3, "PWRAP_SPI0_CK", DRV_GRP3),
	MTK_PIN(4, "PWRAP_SPI0_CSN", DRV_GRP3),
	MTK_PIN(5, "PWRAP_SPI0_CK2", DRV_GRP3),
	MTK_PIN(6, "PWRAP_SPI0_CSN2", DRV_GRP3),
	MTK_PIN(7, "SPI1_CSN", DRV_GRP3),
	MTK_PIN(8, "SPI1_MI", DRV_GRP3),
	MTK_PIN(9, "SPI1_MO", DRV_GRP3),
	MTK_PIN(10, "RTC32K_CK", DRV_GRP3),
	MTK_PIN(11, "WATCHDOG", DRV_GRP3),
	MTK_PIN(12, "SRCLKENA", DRV_GRP3),
	MTK_PIN(13, "SRCLKENAI", DRV_GRP3),
	MTK_PIN(14, "URXD2", DRV_GRP1),
	MTK_PIN(15, "UTXD2", DRV_GRP1),
	MTK_PIN(16, "I2S5_DATA_IN", DRV_GRP1),
	MTK_PIN(17, "I2S5_BCK", DRV_GRP1),
	MTK_PIN(18, "PCM_CLK", DRV_GRP1),
	MTK_PIN(19, "PCM_SYNC", DRV_GRP1),
	MTK_PIN(20, "PCM_RX", DRV_GRP1),
	MTK_PIN(21, "PCM_TX", DRV_GRP1),
	MTK_PIN(22, "EINT0", DRV_GRP1),
	MTK_PIN(23, "EINT1", DRV_GRP1),
	MTK_PIN(24, "EINT2", DRV_GRP1),
	MTK_PIN(25, "EINT3", DRV_GRP1),
	MTK_PIN(26, "EINT4", DRV_GRP1),
	MTK_PIN(27, "EINT5", DRV_GRP1),
	MTK_PIN(28, "EINT6", DRV_GRP1),
	MTK_PIN(29, "EINT7", DRV_GRP1),
	MTK_PIN(30, "I2S5_LRCK", DRV_GRP1),
	MTK_PIN(31, "I2S5_MCLK", DRV_GRP1),
	MTK_PIN(32, "I2S5_DATA", DRV_GRP1),
	MTK_PIN(33, "I2S1_DATA", DRV_GRP1),
	MTK_PIN(34, "I2S1_DATA_IN", DRV_GRP1),
	MTK_PIN(35, "I2S1_BCK", DRV_GRP1),
	MTK_PIN(36, "I2S1_LRCK", DRV_GRP1),
	MTK_PIN(37, "I2S1_MCLK", DRV_GRP1),
	MTK_PIN(38, "I2S2_DATA", DRV_GRP1),
	MTK_PIN(39, "JTMS", DRV_GRP3),
	MTK_PIN(40, "JTCK", DRV_GRP3),
	MTK_PIN(41, "JTDI", DRV_GRP3),
	MTK_PIN(42, "JTDO", DRV_GRP3),
	MTK_PIN(43, "NCLE", DRV_GRP1),
	MTK_PIN(44, "NCEB1", DRV_GRP1),
	MTK_PIN(45, "NCEB0", DRV_GRP1),
	MTK_PIN(46, "IR", DRV_FIXED),
	MTK_PIN(47, "NREB", DRV_GRP1),
	MTK_PIN(48, "NRNB", DRV_GRP1),
	MTK_PIN(49, "I2S0_DATA", DRV_GRP1),
	MTK_PIN(50, "I2S2_BCK", DRV_GRP1),
	MTK_PIN(51, "I2S2_DATA_IN", DRV_GRP1),
	MTK_PIN(52, "I2S2_LRCK", DRV_GRP1),
	MTK_PIN(53, "SPI0_CSN", DRV_GRP1),
	MTK_PIN(54, "SPI0_CK", DRV_GRP1),
	MTK_PIN(55, "SPI0_MI", DRV_GRP1),
	MTK_PIN(56, "SPI0_MO", DRV_GRP1),
	MTK_PIN(57, "SDA1", DRV_FIXED),
	MTK_PIN(58, "SCL1", DRV_FIXED),
	MTK_PIN(59, "RAMBUF_I_CLK", DRV_FIXED),
	MTK_PIN(60, "WB_RSTB", DRV_GRP3),
	MTK_PIN(61, "F2W_DATA", DRV_GRP3),
	MTK_PIN(62, "F2W_CLK", DRV_GRP3),
	MTK_PIN(63, "WB_SCLK", DRV_GRP3),
	MTK_PIN(64, "WB_SDATA", DRV_GRP3),
	MTK_PIN(65, "WB_SEN", DRV_GRP3),
	MTK_PIN(66, "WB_CRTL0", DRV_GRP3),
	MTK_PIN(67, "WB_CRTL1", DRV_GRP3),
	MTK_PIN(68, "WB_CRTL2", DRV_GRP3),
	MTK_PIN(69, "WB_CRTL3", DRV_GRP3),
	MTK_PIN(70, "WB_CRTL4", DRV_GRP3),
	MTK_PIN(71, "WB_CRTL5", DRV_GRP3),
	MTK_PIN(72, "I2S0_DATA_IN", DRV_GRP1),
	MTK_PIN(73, "I2S0_LRCK", DRV_GRP1),
	MTK_PIN(74, "I2S0_BCK", DRV_GRP1),
	MTK_PIN(75, "SDA0", DRV_FIXED),
	MTK_PIN(76, "SCL0", DRV_FIXED),
	MTK_PIN(77, "SDA2", DRV_FIXED),
	MTK_PIN(78, "SCL2", DRV_FIXED),
	MTK_PIN(79, "URXD0", DRV_FIXED),
	MTK_PIN(80, "UTXD0", DRV_FIXED),
	MTK_PIN(81, "URXD1", DRV_FIXED),
	MTK_PIN(82, "UTXD1", DRV_FIXED),
	MTK_PIN(83, "LCM_RST", DRV_FIXED),
	MTK_PIN(84, "DSI_TE", DRV_FIXED),
	MTK_PIN(85, "MSDC2_CMD", DRV_GRP4),
	MTK_PIN(86, "MSDC2_CLK", DRV_GRP4),
	MTK_PIN(87, "MSDC2_DAT0", DRV_GRP4),
	MTK_PIN(88, "MSDC2_DAT1", DRV_GRP4),
	MTK_PIN(89, "MSDC2_DAT2", DRV_GRP4),
	MTK_PIN(90, "MSDC2_DAT3", DRV_GRP4),
	MTK_PIN(91, "TDN3", DRV_FIXED),
	MTK_PIN(92, "TDP3", DRV_FIXED),
	MTK_PIN(93, "TDN2", DRV_FIXED),
	MTK_PIN(94, "TDP2", DRV_FIXED),
	MTK_PIN(95, "TCN", DRV_FIXED),
	MTK_PIN(96, "TCP", DRV_FIXED),
	MTK_PIN(97, "TDN1", DRV_FIXED),
	MTK_PIN(98, "TDP1", DRV_FIXED),
	MTK_PIN(99, "TDN0", DRV_FIXED),
	MTK_PIN(100, "TDP0", DRV_FIXED),
	MTK_PIN(101, "SPI2_CSN", DRV_FIXED),
	MTK_PIN(102, "SPI2_MI", DRV_FIXED),
	MTK_PIN(103, "SPI2_MO", DRV_FIXED),
	MTK_PIN(104, "SPI2_CLK", DRV_FIXED),
	MTK_PIN(105, "MSDC1_CMD", DRV_GRP4),
	MTK_PIN(106, "MSDC1_CLK", DRV_GRP4),
	MTK_PIN(107, "MSDC1_DAT0", DRV_GRP4),
	MTK_PIN(108, "MSDC1_DAT1", DRV_GRP4),
	MTK_PIN(109, "MSDC1_DAT2", DRV_GRP4),
	MTK_PIN(110, "MSDC1_DAT3", DRV_GRP4),
	MTK_PIN(111, "MSDC0_DAT7", DRV_GRP4),
	MTK_PIN(112, "MSDC0_DAT6", DRV_GRP4),
	MTK_PIN(113, "MSDC0_DAT5", DRV_GRP4),
	MTK_PIN(114, "MSDC0_DAT4", DRV_GRP4),
	MTK_PIN(115, "MSDC0_RSTB", DRV_GRP4),
	MTK_PIN(116, "MSDC0_CMD", DRV_GRP4),
	MTK_PIN(117, "MSDC0_CLK", DRV_GRP4),
	MTK_PIN(118, "MSDC0_DAT3", DRV_GRP4),
	MTK_PIN(119, "MSDC0_DAT2", DRV_GRP4),
	MTK_PIN(120, "MSDC0_DAT1", DRV_GRP4),
	MTK_PIN(121, "MSDC0_DAT0", DRV_GRP4),
	MTK_PIN(122, "CEC", DRV_FIXED),
	MTK_PIN(123, "HTPLG", DRV_FIXED),
	MTK_PIN(124, "HDMISCK", DRV_FIXED),
	MTK_PIN(125, "HDMISD", DRV_FIXED),
	MTK_PIN(126, "I2S0_MCLK", DRV_GRP1),
	MTK_PIN(127, "RAMBUF_IDATA0", DRV_FIXED),
	MTK_PIN(128, "RAMBUF_IDATA1", DRV_FIXED),
	MTK_PIN(129, "RAMBUF_IDATA2", DRV_FIXED),
	MTK_PIN(130, "RAMBUF_IDATA3", DRV_FIXED),
	MTK_PIN(131, "RAMBUF_IDATA4", DRV_FIXED),
	MTK_PIN(132, "RAMBUF_IDATA5", DRV_FIXED),
	MTK_PIN(133, "RAMBUF_IDATA6", DRV_FIXED),
	MTK_PIN(134, "RAMBUF_IDATA7", DRV_FIXED),
	MTK_PIN(135, "RAMBUF_IDATA8", DRV_FIXED),
	MTK_PIN(136, "RAMBUF_IDATA9", DRV_FIXED),
	MTK_PIN(137, "RAMBUF_IDATA10", DRV_FIXED),
	MTK_PIN(138, "RAMBUF_IDATA11", DRV_FIXED),
	MTK_PIN(139, "RAMBUF_IDATA12", DRV_FIXED),
	MTK_PIN(140, "RAMBUF_IDATA13", DRV_FIXED),
	MTK_PIN(141, "RAMBUF_IDATA14", DRV_FIXED),
	MTK_PIN(142, "RAMBUF_IDATA15", DRV_FIXED),
	MTK_PIN(143, "RAMBUF_ODATA0", DRV_FIXED),
	MTK_PIN(144, "RAMBUF_ODATA1", DRV_FIXED),
	MTK_PIN(145, "RAMBUF_ODATA2", DRV_FIXED),
	MTK_PIN(146, "RAMBUF_ODATA3", DRV_FIXED),
	MTK_PIN(147, "RAMBUF_ODATA4", DRV_FIXED),
	MTK_PIN(148, "RAMBUF_ODATA5", DRV_FIXED),
	MTK_PIN(149, "RAMBUF_ODATA6", DRV_FIXED),
	MTK_PIN(150, "RAMBUF_ODATA7", DRV_FIXED),
	MTK_PIN(151, "RAMBUF_ODATA8", DRV_FIXED),
	MTK_PIN(152, "RAMBUF_ODATA9", DRV_FIXED),
	MTK_PIN(153, "RAMBUF_ODATA10", DRV_FIXED),
	MTK_PIN(154, "RAMBUF_ODATA11", DRV_FIXED),
	MTK_PIN(155, "RAMBUF_ODATA12", DRV_FIXED),
	MTK_PIN(156, "RAMBUF_ODATA13", DRV_FIXED),
	MTK_PIN(157, "RAMBUF_ODATA14", DRV_FIXED),
	MTK_PIN(158, "RAMBUF_ODATA15", DRV_FIXED),
	MTK_PIN(159, "RAMBUF_BE0", DRV_FIXED),
	MTK_PIN(160, "RAMBUF_BE1", DRV_FIXED),
	MTK_PIN(161, "AP2PT_INT", DRV_FIXED),
	MTK_PIN(162, "AP2PT_INT_CLR", DRV_FIXED),
	MTK_PIN(163, "PT2AP_INT", DRV_FIXED),
	MTK_PIN(164, "PT2AP_INT_CLR", DRV_FIXED),
	MTK_PIN(165, "AP2UP_INT", DRV_FIXED),
	MTK_PIN(166, "AP2UP_INT_CLR", DRV_FIXED),
	MTK_PIN(167, "UP2AP_INT", DRV_FIXED),
	MTK_PIN(168, "UP2AP_INT_CLR", DRV_FIXED),
	MTK_PIN(169, "RAMBUF_ADDR0", DRV_FIXED),
	MTK_PIN(170, "RAMBUF_ADDR1", DRV_FIXED),
	MTK_PIN(171, "RAMBUF_ADDR2", DRV_FIXED),
	MTK_PIN(172, "RAMBUF_ADDR3", DRV_FIXED),
	MTK_PIN(173, "RAMBUF_ADDR4", DRV_FIXED),
	MTK_PIN(174, "RAMBUF_ADDR5", DRV_FIXED),
	MTK_PIN(175, "RAMBUF_ADDR6", DRV_FIXED),
	MTK_PIN(176, "RAMBUF_ADDR7", DRV_FIXED),
	MTK_PIN(177, "RAMBUF_ADDR8", DRV_FIXED),
	MTK_PIN(178, "RAMBUF_ADDR9", DRV_FIXED),
	MTK_PIN(179, "RAMBUF_ADDR10", DRV_FIXED),
	MTK_PIN(180, "RAMBUF_RW", DRV_FIXED),
	MTK_PIN(181, "RAMBUF_LAST", DRV_FIXED),
	MTK_PIN(182, "RAMBUF_HP", DRV_FIXED),
	MTK_PIN(183, "RAMBUF_REQ", DRV_FIXED),
	MTK_PIN(184, "RAMBUF_ALE", DRV_FIXED),
	MTK_PIN(185, "RAMBUF_DLE", DRV_FIXED),
	MTK_PIN(186, "RAMBUF_WDLE", DRV_FIXED),
	MTK_PIN(187, "RAMBUF_O_CLK", DRV_FIXED),
	MTK_PIN(188, "I2S2_MCLK", DRV_GRP1),
	MTK_PIN(189, "I2S3_DATA", DRV_GRP1),
	MTK_PIN(190, "I2S3_DATA_IN", DRV_GRP1),
	MTK_PIN(191, "I2S3_BCK", DRV_GRP1),
	MTK_PIN(192, "I2S3_LRCK", DRV_GRP1),
	MTK_PIN(193, "I2S3_MCLK", DRV_GRP1),
	MTK_PIN(194, "I2S4_DATA", DRV_GRP1),
	MTK_PIN(195, "I2S4_DATA_IN", DRV_GRP1),
	MTK_PIN(196, "I2S4_BCK", DRV_GRP1),
	MTK_PIN(197, "I2S4_LRCK", DRV_GRP1),
	MTK_PIN(198, "I2S4_MCLK", DRV_GRP1),
	MTK_PIN(199, "SPI1_CLK", DRV_GRP3),
	MTK_PIN(200, "SPDIF_OUT", DRV_GRP1),
	MTK_PIN(201, "SPDIF_IN0", DRV_GRP1),
	MTK_PIN(202, "SPDIF_IN1", DRV_GRP1),
	MTK_PIN(203, "PWM0", DRV_GRP1),
	MTK_PIN(204, "PWM1", DRV_GRP1),
	MTK_PIN(205, "PWM2", DRV_GRP1),
	MTK_PIN(206, "PWM3", DRV_GRP1),
	MTK_PIN(207, "PWM4", DRV_GRP1),
	MTK_PIN(208, "AUD_EXT_CK1", DRV_GRP1),
	MTK_PIN(209, "AUD_EXT_CK2", DRV_GRP1),
	MTK_PIN(210, "AUD_CLOCK", DRV_GRP3),
	MTK_PIN(211, "DVP_RESET", DRV_GRP3),
	MTK_PIN(212, "DVP_CLOCK", DRV_GRP3),
	MTK_PIN(213, "DVP_CS", DRV_GRP3),
	MTK_PIN(214, "DVP_CK", DRV_GRP3),
	MTK_PIN(215, "DVP_DI", DRV_GRP3),
	MTK_PIN(216, "DVP_DO", DRV_GRP3),
	MTK_PIN(217, "AP_CS", DRV_GRP3),
	MTK_PIN(218, "AP_CK", DRV_GRP3),
	MTK_PIN(219, "AP_DI", DRV_GRP3),
	MTK_PIN(220, "AP_DO", DRV_GRP3),
	MTK_PIN(221, "DVD_BCLK", DRV_GRP3),
	MTK_PIN(222, "T8032_CLK", DRV_GRP3),
	MTK_PIN(223, "AP_BCLK", DRV_GRP3),
	MTK_PIN(224, "HOST_CS", DRV_GRP3),
	MTK_PIN(225, "HOST_CK", DRV_GRP3),
	MTK_PIN(226, "HOST_DO0", DRV_GRP3),
	MTK_PIN(227, "HOST_DO1", DRV_GRP3),
	MTK_PIN(228, "SLV_CS", DRV_GRP3),
	MTK_PIN(229, "SLV_CK", DRV_GRP3),
	MTK_PIN(230, "SLV_DI0", DRV_GRP3),
	MTK_PIN(231, "SLV_DI1", DRV_GRP3),
	MTK_PIN(232, "AP2DSP_INT", DRV_GRP3),
	MTK_PIN(233, "AP2DSP_INT_CLR", DRV_GRP3),
	MTK_PIN(234, "DSP2AP_INT", DRV_GRP3),
	MTK_PIN(235, "DSP2AP_INT_CLR", DRV_GRP3),
	MTK_PIN(236, "EXT_SDIO3", DRV_GRP1),
	MTK_PIN(237, "EXT_SDIO2", DRV_GRP1),
	MTK_PIN(238, "EXT_SDIO1", DRV_GRP1),
	MTK_PIN(239, "EXT_SDIO0", DRV_GRP1),
	MTK_PIN(240, "EXT_XCS", DRV_GRP1),
	MTK_PIN(241, "EXT_SCK", DRV_GRP1),
	MTK_PIN(242, "URTS2", DRV_GRP1),
	MTK_PIN(243, "UCTS2", DRV_GRP1),
	MTK_PIN(244, "HDMI_SDA_RX", DRV_FIXED),
	MTK_PIN(245, "HDMI_SCL_RX", DRV_FIXED),
	MTK_PIN(246, "MHL_SENCE", DRV_FIXED),
	MTK_PIN(247, "HDMI_HPD_CBUS_RX", DRV_FIXED),
	MTK_PIN(248, "HDMI_TESTOUTP_RX", DRV_GRP1),
	MTK_PIN(249, "MSDC0E_RSTB", DRV_GRP4),
	MTK_PIN(250, "MSDC0E_DAT7", DRV_GRP4),
	MTK_PIN(251, "MSDC0E_DAT6", DRV_GRP4),
	MTK_PIN(252, "MSDC0E_DAT5", DRV_GRP4),
	MTK_PIN(253, "MSDC0E_DAT4", DRV_GRP4),
	MTK_PIN(254, "MSDC0E_DAT3", DRV_GRP4),
	MTK_PIN(255, "MSDC0E_DAT2", DRV_GRP4),
	MTK_PIN(256, "MSDC0E_DAT1", DRV_GRP4),
	MTK_PIN(257, "MSDC0E_DAT0", DRV_GRP4),
	MTK_PIN(258, "MSDC0E_CMD", DRV_GRP4),
	MTK_PIN(259, "MSDC0E_CLK", DRV_GRP4),
	MTK_PIN(260, "MSDC0E_DSL", DRV_GRP4),
	MTK_PIN(261, "MSDC1_INS", DRV_GRP4),
	MTK_PIN(262, "G2_TXEN", DRV_GRP1),
	MTK_PIN(263, "G2_TXD3", DRV_GRP1),
	MTK_PIN(264, "G2_TXD2", DRV_GRP1),
	MTK_PIN(265, "G2_TXD1", DRV_GRP1),
	MTK_PIN(266, "G2_TXD0", DRV_GRP1),
	MTK_PIN(267, "G2_TXC", DRV_GRP1),
	MTK_PIN(268, "G2_RXC", DRV_GRP1),
	MTK_PIN(269, "G2_RXD0", DRV_GRP1),
	MTK_PIN(270, "G2_RXD1", DRV_GRP1),
	MTK_PIN(271, "G2_RXD2", DRV_GRP1),
	MTK_PIN(272, "G2_RXD3", DRV_GRP1),
	MTK_PIN(273, "ESW_INT", DRV_GRP1),
	MTK_PIN(274, "G2_RXDV", DRV_GRP1),
	MTK_PIN(275, "MDC", DRV_GRP1),
	MTK_PIN(276, "MDIO", DRV_GRP1),
	MTK_PIN(277, "ESW_RST", DRV_GRP1),
	MTK_PIN(278, "JTAG_RESET", DRV_GRP3),
	MTK_PIN(279, "USB3_RES_BOND", DRV_GRP1),
};

/* List all groups consisting of these pins dedicated to the enablement of
 * certain hardware block and the corresponding mode for all of the pins.
 * The hardware probably has multiple combinations of these pinouts.
 */

/* AUDIO EXT CLK */
static int mt7623_aud_ext_clk0_pins[] = { 208, };
static int mt7623_aud_ext_clk0_funcs[] = { 1, };
static int mt7623_aud_ext_clk1_pins[] = { 209, };
static int mt7623_aud_ext_clk1_funcs[] = { 1, };

/* DISP PWM */
static int mt7623_disp_pwm_0_pins[] = { 72, };
static int mt7623_disp_pwm_0_funcs[] = { 5, };
static int mt7623_disp_pwm_1_pins[] = { 203, };
static int mt7623_disp_pwm_1_funcs[] = { 2, };
static int mt7623_disp_pwm_2_pins[] = { 208, };
static int mt7623_disp_pwm_2_funcs[] = { 5, };

/* ESW */
static int mt7623_esw_int_pins[] = { 273, };
static int mt7623_esw_int_funcs[] = { 1, };
static int mt7623_esw_rst_pins[] = { 277, };
static int mt7623_esw_rst_funcs[] = { 1, };

/* EPHY */
static int mt7623_ephy_pins[] = { 262, 263, 264, 265, 266, 267, 268,
				  269, 270, 271, 272, 274, };
static int mt7623_ephy_funcs[] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, };

/* EXT_SDIO */
static int mt7623_ext_sdio_pins[] = { 236, 237, 238, 239, 240, 241, };
static int mt7623_ext_sdio_funcs[] = { 1, 1, 1, 1, 1, 1, };

/* HDMI RX */
static int mt7623_hdmi_rx_pins[] = { 247, 248, };
static int mt7623_hdmi_rx_funcs[] = { 1, 1 };
static int mt7623_hdmi_rx_i2c_pins[] = { 244, 245, };
static int mt7623_hdmi_rx_i2c_funcs[] = { 1, 1 };

/* HDMI TX */
static int mt7623_hdmi_cec_pins[] = { 122, };
static int mt7623_hdmi_cec_funcs[] = { 1, };
static int mt7623_hdmi_htplg_pins[] = { 123, };
static int mt7623_hdmi_htplg_funcs[] = { 1, };
static int mt7623_hdmi_i2c_pins[] = { 124, 125, };
static int mt7623_hdmi_i2c_funcs[] = { 1, 1 };

/* I2C */
static int mt7623_i2c0_pins[] = { 75, 76, };
static int mt7623_i2c0_funcs[] = { 1, 1, };
static int mt7623_i2c1_0_pins[] = { 57, 58, };
static int mt7623_i2c1_0_funcs[] = { 1, 1, };
static int mt7623_i2c1_1_pins[] = { 242, 243, };
static int mt7623_i2c1_1_funcs[] = { 4, 4, };
static int mt7623_i2c1_2_pins[] = { 85, 86, };
static int mt7623_i2c1_2_funcs[] = { 3, 3, };
static int mt7623_i2c1_3_pins[] = { 105, 106, };
static int mt7623_i2c1_3_funcs[] = { 3, 3, };
static int mt7623_i2c1_4_pins[] = { 124, 125, };
static int mt7623_i2c1_4_funcs[] = { 4, 4, };
static int mt7623_i2c2_0_pins[] = { 77, 78, };
static int mt7623_i2c2_0_funcs[] = { 1, 1, };
static int mt7623_i2c2_1_pins[] = { 89, 90, };
static int mt7623_i2c2_1_funcs[] = { 3, 3, };
static int mt7623_i2c2_2_pins[] = { 109, 110, };
static int mt7623_i2c2_2_funcs[] = { 3, 3, };
static int mt7623_i2c2_3_pins[] = { 122, 123, };
static int mt7623_i2c2_3_funcs[] = { 4, 4, };

/* I2S */
static int mt7623_i2s0_pins[] = { 49, 72, 73, 74, 126, };
static int mt7623_i2s0_funcs[] = { 1, 1, 1, 1, 1, };
static int mt7623_i2s1_pins[] = { 33, 34, 35, 36, 37, };
static int mt7623_i2s1_funcs[] = { 1, 1, 1, 1, 1, };
static int mt7623_i2s2_bclk_lrclk_mclk_pins[] = { 50, 52, 188, };
static int mt7623_i2s2_bclk_lrclk_mclk_funcs[] = { 1, 1, 1, };
static int mt7623_i2s2_data_in_pins[] = { 51, };
static int mt7623_i2s2_data_in_funcs[] = { 1, };
static int mt7623_i2s2_data_0_pins[] = { 203, };
static int mt7623_i2s2_data_0_funcs[] = { 9, };
static int mt7623_i2s2_data_1_pins[] = { 38,  };
static int mt7623_i2s2_data_1_funcs[] = { 4, };
static int mt7623_i2s3_bclk_lrclk_mclk_pins[] = { 191, 192, 193, };
static int mt7623_i2s3_bclk_lrclk_mclk_funcs[] = { 1, 1, 1, };
static int mt7623_i2s3_data_in_pins[] = { 190, };
static int mt7623_i2s3_data_in_funcs[] = { 1, };
static int mt7623_i2s3_data_0_pins[] = { 204, };
static int mt7623_i2s3_data_0_funcs[] = { 9, };
static int mt7623_i2s3_data_1_pins[] = { 2, };
static int mt7623_i2s3_data_1_funcs[] = { 0, };
static int mt7623_i2s4_pins[] = { 194, 195, 196, 197, 198, };
static int mt7623_i2s4_funcs[] = { 1, 1, 1, 1, 1, };
static int mt7623_i2s5_pins[] = { 16, 17, 30, 31, 32, };
static int mt7623_i2s5_funcs[] = { 1, 1, 1, 1, 1, };

/* IR */
static int mt7623_ir_pins[] = { 46, };
static int mt7623_ir_funcs[] = { 1, };

/* LCD */
static int mt7623_mipi_tx_pins[] = { 91, 92, 93, 94, 95, 96, 97, 98,
				     99, 100, };
static int mt7623_mipi_tx_funcs[] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, };
static int mt7623_dsi_te_pins[] = { 84, };
static int mt7623_dsi_te_funcs[] = { 1, };
static int mt7623_lcm_rst_pins[] = { 83, };
static int mt7623_lcm_rst_funcs[] = { 1, };

/* MDC/MDIO */
static int mt7623_mdc_mdio_pins[] = { 275, 276, };
static int mt7623_mdc_mdio_funcs[] = { 1, 1, };

/* MSDC */
static int mt7623_msdc0_pins[] = { 111, 112, 113, 114, 115, 116, 117, 118,
				   119, 120, 121, };
static int mt7623_msdc0_funcs[] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, };
static int mt7623_msdc1_pins[] = { 105, 106, 107, 108, 109, 110, };
static int mt7623_msdc1_funcs[] = { 1, 1, 1, 1, 1, 1, };
static int mt7623_msdc1_ins_pins[] = { 261, };
static int mt7623_msdc1_ins_funcs[] = { 1, };
static int mt7623_msdc1_wp_0_pins[] = { 29, };
static int mt7623_msdc1_wp_0_funcs[] = { 1, };
static int mt7623_msdc1_wp_1_pins[] = { 55, };
static int mt7623_msdc1_wp_1_funcs[] = { 3, };
static int mt7623_msdc1_wp_2_pins[] = { 209, };
static int mt7623_msdc1_wp_2_funcs[] = { 2, };
static int mt7623_msdc2_pins[] = { 85, 86, 87, 88, 89, 90, };
static int mt7623_msdc2_funcs[] = { 1, 1, 1, 1, 1, 1, };
static int mt7623_msdc3_pins[] = { 249, 250, 251, 252, 253, 254, 255, 256,
				   257, 258, 259, 260, };
static int mt7623_msdc3_funcs[] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, };

/* NAND */
static int mt7623_nandc_pins[] = { 43, 47, 48, 111, 112, 113, 114, 115,
				   116, 117, 118, 119, 120, 121, };
static int mt7623_nandc_funcs[] = { 1, 1, 1, 4, 4, 4, 4, 4, 4, 4, 4, 4,
				   4, 4, };
static int mt7623_nandc_ceb0_pins[] = { 45, };
static int mt7623_nandc_ceb0_funcs[] = { 1, };
static int mt7623_nandc_ceb1_pins[] = { 44, };
static int mt7623_nandc_ceb1_funcs[] = { 1, };

/* RTC */
static int mt7623_rtc_pins[] = { 10, };
static int mt7623_rtc_funcs[] = { 1, };

/* OTG */
static int mt7623_otg_iddig0_0_pins[] = { 29, };
static int mt7623_otg_iddig0_0_funcs[] = { 1, };
static int mt7623_otg_iddig0_1_pins[] = { 44, };
static int mt7623_otg_iddig0_1_funcs[] = { 2, };
static int mt7623_otg_iddig0_2_pins[] = { 236, };
static int mt7623_otg_iddig0_2_funcs[] = { 2, };
static int mt7623_otg_iddig1_0_pins[] = { 27, };
static int mt7623_otg_iddig1_0_funcs[] = { 2, };
static int mt7623_otg_iddig1_1_pins[] = { 47, };
static int mt7623_otg_iddig1_1_funcs[] = { 2, };
static int mt7623_otg_iddig1_2_pins[] = { 238, };
static int mt7623_otg_iddig1_2_funcs[] = { 2, };
static int mt7623_otg_drv_vbus0_0_pins[] = { 28, };
static int mt7623_otg_drv_vbus0_0_funcs[] = { 1, };
static int mt7623_otg_drv_vbus0_1_pins[] = { 45, };
static int mt7623_otg_drv_vbus0_1_funcs[] = { 2, };
static int mt7623_otg_drv_vbus0_2_pins[] = { 237, };
static int mt7623_otg_drv_vbus0_2_funcs[] = { 2, };
static int mt7623_otg_drv_vbus1_0_pins[] = { 26, };
static int mt7623_otg_drv_vbus1_0_funcs[] = { 2, };
static int mt7623_otg_drv_vbus1_1_pins[] = { 48, };
static int mt7623_otg_drv_vbus1_1_funcs[] = { 2, };
static int mt7623_otg_drv_vbus1_2_pins[] = { 239, };
static int mt7623_otg_drv_vbus1_2_funcs[] = { 2, };

/* PCIE */
static int mt7623_pcie0_0_perst_pins[] = { 208, };
static int mt7623_pcie0_0_perst_funcs[] = { 3, };
static int mt7623_pcie0_1_perst_pins[] = { 22, };
static int mt7623_pcie0_1_perst_funcs[] = { 2, };
static int mt7623_pcie1_0_perst_pins[] = { 209, };
static int mt7623_pcie1_0_perst_funcs[] = { 3, };
static int mt7623_pcie1_1_perst_pins[] = { 23, };
static int mt7623_pcie1_1_perst_funcs[] = { 2, };
static int mt7623_pcie2_0_perst_pins[] = { 24, };
static int mt7623_pcie2_0_perst_funcs[] = { 2, };
static int mt7623_pcie2_1_perst_pins[] = { 29, };
static int mt7623_pcie2_1_perst_funcs[] = { 6, };
static int mt7623_pcie0_0_wake_pins[] = { 28, };
static int mt7623_pcie0_0_wake_funcs[] = { 6, };
static int mt7623_pcie0_1_wake_pins[] = { 251, };
static int mt7623_pcie0_1_wake_funcs[] = { 6, };
static int mt7623_pcie1_0_wake_pins[] = { 27, };
static int mt7623_pcie1_0_wake_funcs[] = { 6, };
static int mt7623_pcie1_1_wake_pins[] = { 253, };
static int mt7623_pcie1_1_wake_funcs[] = { 6, };
static int mt7623_pcie2_0_wake_pins[] = { 26, };
static int mt7623_pcie2_0_wake_funcs[] = { 6, };
static int mt7623_pcie2_1_wake_pins[] = { 255, };
static int mt7623_pcie2_1_wake_funcs[] = { 6, };
static int mt7623_pcie0_clkreq_pins[] = { 250, };
static int mt7623_pcie0_clkreq_funcs[] = { 6, };
static int mt7623_pcie1_clkreq_pins[] = { 252, };
static int mt7623_pcie1_clkreq_funcs[] = { 6, };
static int mt7623_pcie2_clkreq_pins[] = { 254, };
static int mt7623_pcie2_clkreq_funcs[] = { 6, };
/* the pcie_*_rev are only used for MT7623 */
static int mt7623_pcie0_0_rev_perst_pins[] = { 208, };
static int mt7623_pcie0_0_rev_perst_funcs[] = { 11, };
static int mt7623_pcie0_1_rev_perst_pins[] = { 22, };
static int mt7623_pcie0_1_rev_perst_funcs[] = { 10, };
static int mt7623_pcie1_0_rev_perst_pins[] = { 209, };
static int mt7623_pcie1_0_rev_perst_funcs[] = { 11, };
static int mt7623_pcie1_1_rev_perst_pins[] = { 23, };
static int mt7623_pcie1_1_rev_perst_funcs[] = { 10, };
static int mt7623_pcie2_0_rev_perst_pins[] = { 24, };
static int mt7623_pcie2_0_rev_perst_funcs[] = { 11, };
static int mt7623_pcie2_1_rev_perst_pins[] = { 29, };
static int mt7623_pcie2_1_rev_perst_funcs[] = { 14, };

/* PCM */
static int mt7623_pcm_clk_0_pins[] = { 18, };
static int mt7623_pcm_clk_0_funcs[] = { 1, };
static int mt7623_pcm_clk_1_pins[] = { 17, };
static int mt7623_pcm_clk_1_funcs[] = { 3, };
static int mt7623_pcm_clk_2_pins[] = { 35, };
static int mt7623_pcm_clk_2_funcs[] = { 3, };
static int mt7623_pcm_clk_3_pins[] = { 50, };
static int mt7623_pcm_clk_3_funcs[] = { 3, };
static int mt7623_pcm_clk_4_pins[] = { 74, };
static int mt7623_pcm_clk_4_funcs[] = { 3, };
static int mt7623_pcm_clk_5_pins[] = { 191, };
static int mt7623_pcm_clk_5_funcs[] = { 3, };
static int mt7623_pcm_clk_6_pins[] = { 196, };
static int mt7623_pcm_clk_6_funcs[] = { 3, };
static int mt7623_pcm_sync_0_pins[] = { 19, };
static int mt7623_pcm_sync_0_funcs[] = { 1, };
static int mt7623_pcm_sync_1_pins[] = { 30, };
static int mt7623_pcm_sync_1_funcs[] = { 3, };
static int mt7623_pcm_sync_2_pins[] = { 36, };
static int mt7623_pcm_sync_2_funcs[] = { 3, };
static int mt7623_pcm_sync_3_pins[] = { 52, };
static int mt7623_pcm_sync_3_funcs[] = { 31, };
static int mt7623_pcm_sync_4_pins[] = { 73, };
static int mt7623_pcm_sync_4_funcs[] = { 3, };
static int mt7623_pcm_sync_5_pins[] = { 192, };
static int mt7623_pcm_sync_5_funcs[] = { 3, };
static int mt7623_pcm_sync_6_pins[] = { 197, };
static int mt7623_pcm_sync_6_funcs[] = { 3, };
static int mt7623_pcm_rx_0_pins[] = { 20, };
static int mt7623_pcm_rx_0_funcs[] = { 1, };
static int mt7623_pcm_rx_1_pins[] = { 16, };
static int mt7623_pcm_rx_1_funcs[] = { 3, };
static int mt7623_pcm_rx_2_pins[] = { 34, };
static int mt7623_pcm_rx_2_funcs[] = { 3, };
static int mt7623_pcm_rx_3_pins[] = { 51, };
static int mt7623_pcm_rx_3_funcs[] = { 3, };
static int mt7623_pcm_rx_4_pins[] = { 72, };
static int mt7623_pcm_rx_4_funcs[] = { 3, };
static int mt7623_pcm_rx_5_pins[] = { 190, };
static int mt7623_pcm_rx_5_funcs[] = { 3, };
static int mt7623_pcm_rx_6_pins[] = { 195, };
static int mt7623_pcm_rx_6_funcs[] = { 3, };
static int mt7623_pcm_tx_0_pins[] = { 21, };
static int mt7623_pcm_tx_0_funcs[] = { 1, };
static int mt7623_pcm_tx_1_pins[] = { 32, };
static int mt7623_pcm_tx_1_funcs[] = { 3, };
static int mt7623_pcm_tx_2_pins[] = { 33, };
static int mt7623_pcm_tx_2_funcs[] = { 3, };
static int mt7623_pcm_tx_3_pins[] = { 38, };
static int mt7623_pcm_tx_3_funcs[] = { 3, };
static int mt7623_pcm_tx_4_pins[] = { 49, };
static int mt7623_pcm_tx_4_funcs[] = { 3, };
static int mt7623_pcm_tx_5_pins[] = { 189, };
static int mt7623_pcm_tx_5_funcs[] = { 3, };
static int mt7623_pcm_tx_6_pins[] = { 194, };
static int mt7623_pcm_tx_6_funcs[] = { 3, };

/* PWM */
static int mt7623_pwm_ch1_0_pins[] = { 203, };
static int mt7623_pwm_ch1_0_funcs[] = { 1, };
static int mt7623_pwm_ch1_1_pins[] = { 208, };
static int mt7623_pwm_ch1_1_funcs[] = { 2, };
static int mt7623_pwm_ch1_2_pins[] = { 72, };
static int mt7623_pwm_ch1_2_funcs[] = { 4, };
static int mt7623_pwm_ch1_3_pins[] = { 88, };
static int mt7623_pwm_ch1_3_funcs[] = { 3, };
static int mt7623_pwm_ch1_4_pins[] = { 108, };
static int mt7623_pwm_ch1_4_funcs[] = { 3, };
static int mt7623_pwm_ch2_0_pins[] = { 204, };
static int mt7623_pwm_ch2_0_funcs[] = { 1, };
static int mt7623_pwm_ch2_1_pins[] = { 53, };
static int mt7623_pwm_ch2_1_funcs[] = { 5, };
static int mt7623_pwm_ch2_2_pins[] = { 88, };
static int mt7623_pwm_ch2_2_funcs[] = { 6, };
static int mt7623_pwm_ch2_3_pins[] = { 108, };
static int mt7623_pwm_ch2_3_funcs[] = { 6, };
static int mt7623_pwm_ch2_4_pins[] = { 209, };
static int mt7623_pwm_ch2_4_funcs[] = { 5, };
static int mt7623_pwm_ch3_0_pins[] = { 205, };
static int mt7623_pwm_ch3_0_funcs[] = { 1, };
static int mt7623_pwm_ch3_1_pins[] = { 55, };
static int mt7623_pwm_ch3_1_funcs[] = { 5, };
static int mt7623_pwm_ch3_2_pins[] = { 89, };
static int mt7623_pwm_ch3_2_funcs[] = { 6, };
static int mt7623_pwm_ch3_3_pins[] = { 109, };
static int mt7623_pwm_ch3_3_funcs[] = { 6, };
static int mt7623_pwm_ch4_0_pins[] = { 206, };
static int mt7623_pwm_ch4_0_funcs[] = { 1, };
static int mt7623_pwm_ch4_1_pins[] = { 90, };
static int mt7623_pwm_ch4_1_funcs[] = { 6, };
static int mt7623_pwm_ch4_2_pins[] = { 110, };
static int mt7623_pwm_ch4_2_funcs[] = { 6, };
static int mt7623_pwm_ch4_3_pins[] = { 124, };
static int mt7623_pwm_ch4_3_funcs[] = { 5, };
static int mt7623_pwm_ch5_0_pins[] = { 207, };
static int mt7623_pwm_ch5_0_funcs[] = { 1, };
static int mt7623_pwm_ch5_1_pins[] = { 125, };
static int mt7623_pwm_ch5_1_funcs[] = { 5, };

/* PWRAP */
static int mt7623_pwrap_pins[] = { 0, 1, 2, 3, 4, 5, 6, };
static int mt7623_pwrap_funcs[] = { 1, 1, 1, 1, 1, 1, 1, };

/* SPDIF */
static int mt7623_spdif_in0_0_pins[] = { 56, };
static int mt7623_spdif_in0_0_funcs[] = { 3, };
static int mt7623_spdif_in0_1_pins[] = { 201, };
static int mt7623_spdif_in0_1_funcs[] = { 1, };
static int mt7623_spdif_in1_0_pins[] = { 54, };
static int mt7623_spdif_in1_0_funcs[] = { 3, };
static int mt7623_spdif_in1_1_pins[] = { 202, };
static int mt7623_spdif_in1_1_funcs[] = { 1, };
static int mt7623_spdif_out_pins[] = { 202, };
static int mt7623_spdif_out_funcs[] = { 1, };

/* SPI */
static int mt7623_spi0_pins[] = { 53, 54, 55, 56, };
static int mt7623_spi0_funcs[] = { 1, 1, 1, 1, };
static int mt7623_spi1_pins[] = { 7, 199, 8, 9, };
static int mt7623_spi1_funcs[] = { 1, 1, 1, 1, };
static int mt7623_spi2_pins[] = { 101, 104, 102, 103, };
static int mt7623_spi2_funcs[] = { 1, 1, 1, 1, };

/* UART */
static int mt7623_uart0_0_txd_rxd_pins[] = { 79, 80, };
static int mt7623_uart0_0_txd_rxd_funcs[] = { 1, 1, };
static int mt7623_uart0_1_txd_rxd_pins[] = { 87, 88, };
static int mt7623_uart0_1_txd_rxd_funcs[] = { 5, 5, };
static int mt7623_uart0_2_txd_rxd_pins[] = { 107, 108, };
static int mt7623_uart0_2_txd_rxd_funcs[] = { 5, 5, };
static int mt7623_uart0_3_txd_rxd_pins[] = { 123, 122, };
static int mt7623_uart0_3_txd_rxd_funcs[] = { 5, 5, };
static int mt7623_uart0_rts_cts_pins[] = { 22, 23, };
static int mt7623_uart0_rts_cts_funcs[] = { 1, 1, };
static int mt7623_uart1_0_txd_rxd_pins[] = { 81, 82, };
static int mt7623_uart1_0_txd_rxd_funcs[] = { 1, 1, };
static int mt7623_uart1_1_txd_rxd_pins[] = { 89, 90, };
static int mt7623_uart1_1_txd_rxd_funcs[] = { 5, 5, };
static int mt7623_uart1_2_txd_rxd_pins[] = { 109, 110, };
static int mt7623_uart1_2_txd_rxd_funcs[] = { 5, 5, };
static int mt7623_uart1_rts_cts_pins[] = { 24, 25, };
static int mt7623_uart1_rts_cts_funcs[] = { 1, 1, };
static int mt7623_uart2_0_txd_rxd_pins[] = { 14, 15, };
static int mt7623_uart2_0_txd_rxd_funcs[] = { 1, 1, };
static int mt7623_uart2_1_txd_rxd_pins[] = { 200, 201, };
static int mt7623_uart2_1_txd_rxd_funcs[] = { 6, 6, };
static int mt7623_uart2_rts_cts_pins[] = { 242, 243, };
static int mt7623_uart2_rts_cts_funcs[] = { 1, 1, };
static int mt7623_uart3_txd_rxd_pins[] = { 242, 243, };
static int mt7623_uart3_txd_rxd_funcs[] = { 2, 2, };
static int mt7623_uart3_rts_cts_pins[] = { 26, 27, };
static int mt7623_uart3_rts_cts_funcs[] = { 1, 1, };

/* Watchdog */
static int mt7623_watchdog_0_pins[] = { 11, };
static int mt7623_watchdog_0_funcs[] = { 1, };
static int mt7623_watchdog_1_pins[] = { 121, };
static int mt7623_watchdog_1_funcs[] = { 5, };

static const struct mtk_group_desc mt7623_groups[] = {
	PINCTRL_PIN_GROUP("aud_ext_clk0", mt7623_aud_ext_clk0),
	PINCTRL_PIN_GROUP("aud_ext_clk1", mt7623_aud_ext_clk1),
	PINCTRL_PIN_GROUP("dsi_te", mt7623_dsi_te),
	PINCTRL_PIN_GROUP("disp_pwm_0", mt7623_disp_pwm_0),
	PINCTRL_PIN_GROUP("disp_pwm_1", mt7623_disp_pwm_1),
	PINCTRL_PIN_GROUP("disp_pwm_2", mt7623_disp_pwm_2),
	PINCTRL_PIN_GROUP("ephy", mt7623_ephy),
	PINCTRL_PIN_GROUP("esw_int", mt7623_esw_int),
	PINCTRL_PIN_GROUP("esw_rst", mt7623_esw_rst),
	PINCTRL_PIN_GROUP("ext_sdio", mt7623_ext_sdio),
	PINCTRL_PIN_GROUP("hdmi_cec", mt7623_hdmi_cec),
	PINCTRL_PIN_GROUP("hdmi_htplg", mt7623_hdmi_htplg),
	PINCTRL_PIN_GROUP("hdmi_i2c", mt7623_hdmi_i2c),
	PINCTRL_PIN_GROUP("hdmi_rx", mt7623_hdmi_rx),
	PINCTRL_PIN_GROUP("hdmi_rx_i2c", mt7623_hdmi_rx_i2c),
	PINCTRL_PIN_GROUP("i2c0", mt7623_i2c0),
	PINCTRL_PIN_GROUP("i2c1_0", mt7623_i2c1_0),
	PINCTRL_PIN_GROUP("i2c1_1", mt7623_i2c1_1),
	PINCTRL_PIN_GROUP("i2c1_2", mt7623_i2c1_2),
	PINCTRL_PIN_GROUP("i2c1_3", mt7623_i2c1_3),
	PINCTRL_PIN_GROUP("i2c1_4", mt7623_i2c1_4),
	PINCTRL_PIN_GROUP("i2c2_0", mt7623_i2c2_0),
	PINCTRL_PIN_GROUP("i2c2_1", mt7623_i2c2_1),
	PINCTRL_PIN_GROUP("i2c2_2", mt7623_i2c2_2),
	PINCTRL_PIN_GROUP("i2c2_3", mt7623_i2c2_3),
	PINCTRL_PIN_GROUP("i2s0", mt7623_i2s0),
	PINCTRL_PIN_GROUP("i2s1", mt7623_i2s1),
	PINCTRL_PIN_GROUP("i2s4", mt7623_i2s4),
	PINCTRL_PIN_GROUP("i2s5", mt7623_i2s5),
	PINCTRL_PIN_GROUP("i2s2_bclk_lrclk_mclk", mt7623_i2s2_bclk_lrclk_mclk),
	PINCTRL_PIN_GROUP("i2s3_bclk_lrclk_mclk", mt7623_i2s3_bclk_lrclk_mclk),
	PINCTRL_PIN_GROUP("i2s2_data_in", mt7623_i2s2_data_in),
	PINCTRL_PIN_GROUP("i2s3_data_in", mt7623_i2s3_data_in),
	PINCTRL_PIN_GROUP("i2s2_data_0", mt7623_i2s2_data_0),
	PINCTRL_PIN_GROUP("i2s2_data_1", mt7623_i2s2_data_1),
	PINCTRL_PIN_GROUP("i2s3_data_0", mt7623_i2s3_data_0),
	PINCTRL_PIN_GROUP("i2s3_data_1", mt7623_i2s3_data_1),
	PINCTRL_PIN_GROUP("ir", mt7623_ir),
	PINCTRL_PIN_GROUP("lcm_rst", mt7623_lcm_rst),
	PINCTRL_PIN_GROUP("mdc_mdio", mt7623_mdc_mdio),
	PINCTRL_PIN_GROUP("mipi_tx", mt7623_mipi_tx),
	PINCTRL_PIN_GROUP("msdc0", mt7623_msdc0),
	PINCTRL_PIN_GROUP("msdc1", mt7623_msdc1),
	PINCTRL_PIN_GROUP("msdc1_ins", mt7623_msdc1_ins),
	PINCTRL_PIN_GROUP("msdc1_wp_0", mt7623_msdc1_wp_0),
	PINCTRL_PIN_GROUP("msdc1_wp_1", mt7623_msdc1_wp_1),
	PINCTRL_PIN_GROUP("msdc1_wp_2", mt7623_msdc1_wp_2),
	PINCTRL_PIN_GROUP("msdc2", mt7623_msdc2),
	PINCTRL_PIN_GROUP("msdc3", mt7623_msdc3),
	PINCTRL_PIN_GROUP("nandc", mt7623_nandc),
	PINCTRL_PIN_GROUP("nandc_ceb0", mt7623_nandc_ceb0),
	PINCTRL_PIN_GROUP("nandc_ceb1", mt7623_nandc_ceb1),
	PINCTRL_PIN_GROUP("otg_iddig0_0", mt7623_otg_iddig0_0),
	PINCTRL_PIN_GROUP("otg_iddig0_1", mt7623_otg_iddig0_1),
	PINCTRL_PIN_GROUP("otg_iddig0_2", mt7623_otg_iddig0_2),
	PINCTRL_PIN_GROUP("otg_iddig1_0", mt7623_otg_iddig1_0),
	PINCTRL_PIN_GROUP("otg_iddig1_1", mt7623_otg_iddig1_1),
	PINCTRL_PIN_GROUP("otg_iddig1_2", mt7623_otg_iddig1_2),
	PINCTRL_PIN_GROUP("otg_drv_vbus0_0", mt7623_otg_drv_vbus0_0),
	PINCTRL_PIN_GROUP("otg_drv_vbus0_1", mt7623_otg_drv_vbus0_1),
	PINCTRL_PIN_GROUP("otg_drv_vbus0_2", mt7623_otg_drv_vbus0_2),
	PINCTRL_PIN_GROUP("otg_drv_vbus1_0", mt7623_otg_drv_vbus1_0),
	PINCTRL_PIN_GROUP("otg_drv_vbus1_1", mt7623_otg_drv_vbus1_1),
	PINCTRL_PIN_GROUP("otg_drv_vbus1_2", mt7623_otg_drv_vbus1_2),
	PINCTRL_PIN_GROUP("pcie0_0_perst", mt7623_pcie0_0_perst),
	PINCTRL_PIN_GROUP("pcie0_1_perst", mt7623_pcie0_1_perst),
	PINCTRL_PIN_GROUP("pcie1_0_perst", mt7623_pcie1_0_perst),
	PINCTRL_PIN_GROUP("pcie1_1_perst", mt7623_pcie1_1_perst),
	PINCTRL_PIN_GROUP("pcie1_1_perst", mt7623_pcie1_1_perst),
	PINCTRL_PIN_GROUP("pcie0_0_rev_perst", mt7623_pcie0_0_rev_perst),
	PINCTRL_PIN_GROUP("pcie0_1_rev_perst", mt7623_pcie0_1_rev_perst),
	PINCTRL_PIN_GROUP("pcie1_0_rev_perst", mt7623_pcie1_0_rev_perst),
	PINCTRL_PIN_GROUP("pcie1_1_rev_perst", mt7623_pcie1_1_rev_perst),
	PINCTRL_PIN_GROUP("pcie2_0_rev_perst", mt7623_pcie2_0_rev_perst),
	PINCTRL_PIN_GROUP("pcie2_1_rev_perst", mt7623_pcie2_1_rev_perst),
	PINCTRL_PIN_GROUP("pcie2_0_perst", mt7623_pcie2_0_perst),
	PINCTRL_PIN_GROUP("pcie2_1_perst", mt7623_pcie2_1_perst),
	PINCTRL_PIN_GROUP("pcie0_0_wake", mt7623_pcie0_0_wake),
	PINCTRL_PIN_GROUP("pcie0_1_wake", mt7623_pcie0_1_wake),
	PINCTRL_PIN_GROUP("pcie1_0_wake", mt7623_pcie1_0_wake),
	PINCTRL_PIN_GROUP("pcie1_1_wake", mt7623_pcie1_1_wake),
	PINCTRL_PIN_GROUP("pcie2_0_wake", mt7623_pcie2_0_wake),
	PINCTRL_PIN_GROUP("pcie2_1_wake", mt7623_pcie2_1_wake),
	PINCTRL_PIN_GROUP("pcie0_clkreq", mt7623_pcie0_clkreq),
	PINCTRL_PIN_GROUP("pcie1_clkreq", mt7623_pcie1_clkreq),
	PINCTRL_PIN_GROUP("pcie2_clkreq", mt7623_pcie2_clkreq),
	PINCTRL_PIN_GROUP("pcm_clk_0", mt7623_pcm_clk_0),
	PINCTRL_PIN_GROUP("pcm_clk_1", mt7623_pcm_clk_1),
	PINCTRL_PIN_GROUP("pcm_clk_2", mt7623_pcm_clk_2),
	PINCTRL_PIN_GROUP("pcm_clk_3", mt7623_pcm_clk_3),
	PINCTRL_PIN_GROUP("pcm_clk_4", mt7623_pcm_clk_4),
	PINCTRL_PIN_GROUP("pcm_clk_5", mt7623_pcm_clk_5),
	PINCTRL_PIN_GROUP("pcm_clk_6", mt7623_pcm_clk_6),
	PINCTRL_PIN_GROUP("pcm_sync_0", mt7623_pcm_sync_0),
	PINCTRL_PIN_GROUP("pcm_sync_1", mt7623_pcm_sync_1),
	PINCTRL_PIN_GROUP("pcm_sync_2", mt7623_pcm_sync_2),
	PINCTRL_PIN_GROUP("pcm_sync_3", mt7623_pcm_sync_3),
	PINCTRL_PIN_GROUP("pcm_sync_4", mt7623_pcm_sync_4),
	PINCTRL_PIN_GROUP("pcm_sync_5", mt7623_pcm_sync_5),
	PINCTRL_PIN_GROUP("pcm_sync_6", mt7623_pcm_sync_6),
	PINCTRL_PIN_GROUP("pcm_rx_0", mt7623_pcm_rx_0),
	PINCTRL_PIN_GROUP("pcm_rx_1", mt7623_pcm_rx_1),
	PINCTRL_PIN_GROUP("pcm_rx_2", mt7623_pcm_rx_2),
	PINCTRL_PIN_GROUP("pcm_rx_3", mt7623_pcm_rx_3),
	PINCTRL_PIN_GROUP("pcm_rx_4", mt7623_pcm_rx_4),
	PINCTRL_PIN_GROUP("pcm_rx_5", mt7623_pcm_rx_5),
	PINCTRL_PIN_GROUP("pcm_rx_6", mt7623_pcm_rx_6),
	PINCTRL_PIN_GROUP("pcm_tx_0", mt7623_pcm_tx_0),
	PINCTRL_PIN_GROUP("pcm_tx_1", mt7623_pcm_tx_1),
	PINCTRL_PIN_GROUP("pcm_tx_2", mt7623_pcm_tx_2),
	PINCTRL_PIN_GROUP("pcm_tx_3", mt7623_pcm_tx_3),
	PINCTRL_PIN_GROUP("pcm_tx_4", mt7623_pcm_tx_4),
	PINCTRL_PIN_GROUP("pcm_tx_5", mt7623_pcm_tx_5),
	PINCTRL_PIN_GROUP("pcm_tx_6", mt7623_pcm_tx_6),
	PINCTRL_PIN_GROUP("pwm_ch1_0", mt7623_pwm_ch1_0),
	PINCTRL_PIN_GROUP("pwm_ch1_1", mt7623_pwm_ch1_1),
	PINCTRL_PIN_GROUP("pwm_ch1_2", mt7623_pwm_ch1_2),
	PINCTRL_PIN_GROUP("pwm_ch1_3", mt7623_pwm_ch1_3),
	PINCTRL_PIN_GROUP("pwm_ch1_4", mt7623_pwm_ch1_4),
	PINCTRL_PIN_GROUP("pwm_ch2_0", mt7623_pwm_ch2_0),
	PINCTRL_PIN_GROUP("pwm_ch2_1", mt7623_pwm_ch2_1),
	PINCTRL_PIN_GROUP("pwm_ch2_2", mt7623_pwm_ch2_2),
	PINCTRL_PIN_GROUP("pwm_ch2_3", mt7623_pwm_ch2_3),
	PINCTRL_PIN_GROUP("pwm_ch2_4", mt7623_pwm_ch2_4),
	PINCTRL_PIN_GROUP("pwm_ch3_0", mt7623_pwm_ch3_0),
	PINCTRL_PIN_GROUP("pwm_ch3_1", mt7623_pwm_ch3_1),
	PINCTRL_PIN_GROUP("pwm_ch3_2", mt7623_pwm_ch3_2),
	PINCTRL_PIN_GROUP("pwm_ch3_3", mt7623_pwm_ch3_3),
	PINCTRL_PIN_GROUP("pwm_ch4_0", mt7623_pwm_ch4_0),
	PINCTRL_PIN_GROUP("pwm_ch4_1", mt7623_pwm_ch4_1),
	PINCTRL_PIN_GROUP("pwm_ch4_2", mt7623_pwm_ch4_2),
	PINCTRL_PIN_GROUP("pwm_ch4_3", mt7623_pwm_ch4_3),
	PINCTRL_PIN_GROUP("pwm_ch5_0", mt7623_pwm_ch5_0),
	PINCTRL_PIN_GROUP("pwm_ch5_1", mt7623_pwm_ch5_1),
	PINCTRL_PIN_GROUP("pwrap", mt7623_pwrap),
	PINCTRL_PIN_GROUP("rtc", mt7623_rtc),
	PINCTRL_PIN_GROUP("spdif_in0_0", mt7623_spdif_in0_0),
	PINCTRL_PIN_GROUP("spdif_in0_1", mt7623_spdif_in0_1),
	PINCTRL_PIN_GROUP("spdif_in1_0", mt7623_spdif_in1_0),
	PINCTRL_PIN_GROUP("spdif_in1_1", mt7623_spdif_in1_1),
	PINCTRL_PIN_GROUP("spdif_out", mt7623_spdif_out),
	PINCTRL_PIN_GROUP("spi0", mt7623_spi0),
	PINCTRL_PIN_GROUP("spi1", mt7623_spi1),
	PINCTRL_PIN_GROUP("spi2", mt7623_spi2),
	PINCTRL_PIN_GROUP("uart0_0_txd_rxd",  mt7623_uart0_0_txd_rxd),
	PINCTRL_PIN_GROUP("uart0_1_txd_rxd",  mt7623_uart0_1_txd_rxd),
	PINCTRL_PIN_GROUP("uart0_2_txd_rxd",  mt7623_uart0_2_txd_rxd),
	PINCTRL_PIN_GROUP("uart0_3_txd_rxd",  mt7623_uart0_3_txd_rxd),
	PINCTRL_PIN_GROUP("uart1_0_txd_rxd",  mt7623_uart1_0_txd_rxd),
	PINCTRL_PIN_GROUP("uart1_1_txd_rxd",  mt7623_uart1_1_txd_rxd),
	PINCTRL_PIN_GROUP("uart1_2_txd_rxd",  mt7623_uart1_2_txd_rxd),
	PINCTRL_PIN_GROUP("uart2_0_txd_rxd",  mt7623_uart2_0_txd_rxd),
	PINCTRL_PIN_GROUP("uart2_1_txd_rxd",  mt7623_uart2_1_txd_rxd),
	PINCTRL_PIN_GROUP("uart3_txd_rxd",  mt7623_uart3_txd_rxd),
	PINCTRL_PIN_GROUP("uart0_rts_cts",  mt7623_uart0_rts_cts),
	PINCTRL_PIN_GROUP("uart1_rts_cts",  mt7623_uart1_rts_cts),
	PINCTRL_PIN_GROUP("uart2_rts_cts",  mt7623_uart2_rts_cts),
	PINCTRL_PIN_GROUP("uart3_rts_cts",  mt7623_uart3_rts_cts),
	PINCTRL_PIN_GROUP("watchdog_0", mt7623_watchdog_0),
	PINCTRL_PIN_GROUP("watchdog_1", mt7623_watchdog_1),
};

/* Joint those groups owning the same capability in user point of view which
 * allows that people tend to use through the device tree.
 */

static const char *const mt7623_aud_clk_groups[] = { "aud_ext_clk0",
						"aud_ext_clk1", };
static const char *const mt7623_disp_pwm_groups[] = { "disp_pwm_0",
						"disp_pwm_1",
						"disp_pwm_2", };
static const char *const mt7623_ethernet_groups[] = { "esw_int", "esw_rst",
						"ephy", "mdc_mdio", };
static const char *const mt7623_ext_sdio_groups[] = { "ext_sdio", };
static const char *const mt7623_hdmi_groups[] = { "hdmi_cec", "hdmi_htplg",
						"hdmi_i2c", "hdmi_rx",
						"hdmi_rx_i2c", };
static const char *const mt7623_i2c_groups[] = { "i2c0", "i2c1_0", "i2c1_1",
						"i2c1_2", "i2c1_3", "i2c1_4",
						"i2c2_0", "i2c2_1", "i2c2_2",
						"i2c2_3", };
static const char *const mt7623_i2s_groups[] = { "i2s0", "i2s1",
						"i2s2_bclk_lrclk_mclk",
						"i2s3_bclk_lrclk_mclk",
						"i2s4", "i2s5",
						"i2s2_data_in", "i2s3_data_in",
						"i2s2_data_0", "i2s2_data_1",
						"i2s3_data_0", "i2s3_data_1",};
static const char *const mt7623_ir_groups[] = { "ir", };
static const char *const mt7623_lcd_groups[] = { "dsi_te", "lcm_rst",
						"mipi_tx", };
static const char *const mt7623_msdc_groups[] = { "msdc0", "msdc1",
						"msdc1_ins", "msdc1_wp_0",
						"msdc1_wp_1", "msdc1_wp_2",
						"msdc2", "msdc3", };
static const char *const mt7623_nandc_groups[] = { "nandc", "nandc_ceb0",
						"nandc_ceb1", };
static const char *const mt7623_otg_groups[] = { "otg_iddig0_0",
						"otg_iddig0_1",
						"otg_iddig0_2",
						"otg_iddig1_0",
						"otg_iddig1_1",
						"otg_iddig1_2",
						"otg_drv_vbus0_0",
						"otg_drv_vbus0_1",
						"otg_drv_vbus0_2",
						"otg_drv_vbus1_0",
						"otg_drv_vbus1_1",
						"otg_drv_vbus1_2", };
static const char *const mt7623_pcie_groups[] = { "pcie0_0_perst",
						"pcie0_1_perst",
						"pcie1_0_perst",
						"pcie1_1_perst",
						"pcie2_0_perst",
						"pcie2_1_perst",
						"pcie0_0_rev_perst",
						"pcie0_1_rev_perst",
						"pcie1_0_rev_perst",
						"pcie1_1_rev_perst",
						"pcie2_0_rev_perst",
						"pcie2_1_rev_perst",
						"pcie0_0_wake", "pcie0_1_wake",
						"pcie2_0_wake", "pcie2_1_wake",
						"pcie0_clkreq", "pcie1_clkreq",
						"pcie2_clkreq", };
static const char *const mt7623_pcm_groups[] = { "pcm_clk_0", "pcm_clk_1",
						"pcm_clk_2", "pcm_clk_3",
						"pcm_clk_4", "pcm_clk_5",
						"pcm_clk_6", "pcm_sync_0",
						"pcm_sync_1", "pcm_sync_2",
						"pcm_sync_3", "pcm_sync_4",
						"pcm_sync_5", "pcm_sync_6",
						"pcm_rx_0", "pcm_rx_1",
						"pcm_rx_2", "pcm_rx_3",
						"pcm_rx_4", "pcm_rx_5",
						"pcm_rx_6", "pcm_tx_0",
						"pcm_tx_1", "pcm_tx_2",
						"pcm_tx_3", "pcm_tx_4",
						"pcm_tx_5", "pcm_tx_6", };
static const char *const mt7623_pwm_groups[] = { "pwm_ch1_0", "pwm_ch1_1",
						"pwm_ch1_2", "pwm_ch2_0",
						"pwm_ch2_1", "pwm_ch2_2",
						"pwm_ch3_0", "pwm_ch3_1",
						"pwm_ch3_2", "pwm_ch4_0",
						"pwm_ch4_1", "pwm_ch4_2",
						"pwm_ch4_3", "pwm_ch5_0",
						"pwm_ch5_1", "pwm_ch5_2",
						"pwm_ch6_0", "pwm_ch6_1",
						"pwm_ch6_2", "pwm_ch6_3",
						"pwm_ch7_0", "pwm_ch7_1",
						"pwm_ch7_2", };
static const char *const mt7623_pwrap_groups[] = { "pwrap", };
static const char *const mt7623_rtc_groups[] = { "rtc", };
static const char *const mt7623_spi_groups[] = { "spi0", "spi2", "spi2", };
static const char *const mt7623_spdif_groups[] = { "spdif_in0_0",
						"spdif_in0_1", "spdif_in1_0",
						"spdif_in1_1", "spdif_out", };
static const char *const mt7623_uart_groups[] = { "uart0_0_txd_rxd",
						"uart0_1_txd_rxd",
						"uart0_2_txd_rxd",
						"uart0_3_txd_rxd",
						"uart1_0_txd_rxd",
						"uart1_1_txd_rxd",
						"uart1_2_txd_rxd",
						"uart2_0_txd_rxd",
						"uart2_1_txd_rxd",
						"uart3_txd_rxd",
						"uart0_rts_cts",
						"uart1_rts_cts",
						"uart2_rts_cts",
						"uart3_rts_cts", };
static const char *const mt7623_wdt_groups[] = { "watchdog_0", "watchdog_1", };

static const struct mtk_function_desc mt7623_functions[] = {
	{"audck", mt7623_aud_clk_groups, ARRAY_SIZE(mt7623_aud_clk_groups)},
	{"disp", mt7623_disp_pwm_groups, ARRAY_SIZE(mt7623_disp_pwm_groups)},
	{"eth",	mt7623_ethernet_groups, ARRAY_SIZE(mt7623_ethernet_groups)},
	{"sdio", mt7623_ext_sdio_groups, ARRAY_SIZE(mt7623_ext_sdio_groups)},
	{"hdmi", mt7623_hdmi_groups, ARRAY_SIZE(mt7623_hdmi_groups)},
	{"i2c", mt7623_i2c_groups, ARRAY_SIZE(mt7623_i2c_groups)},
	{"i2s",	mt7623_i2s_groups, ARRAY_SIZE(mt7623_i2s_groups)},
	{"ir",	mt7623_ir_groups, ARRAY_SIZE(mt7623_ir_groups)},
	{"lcd", mt7623_lcd_groups, ARRAY_SIZE(mt7623_lcd_groups)},
	{"msdc", mt7623_msdc_groups, ARRAY_SIZE(mt7623_msdc_groups)},
	{"nand", mt7623_nandc_groups, ARRAY_SIZE(mt7623_nandc_groups)},
	{"otg", mt7623_otg_groups, ARRAY_SIZE(mt7623_otg_groups)},
	{"pcie", mt7623_pcie_groups, ARRAY_SIZE(mt7623_pcie_groups)},
	{"pcm",	mt7623_pcm_groups, ARRAY_SIZE(mt7623_pcm_groups)},
	{"pwm",	mt7623_pwm_groups, ARRAY_SIZE(mt7623_pwm_groups)},
	{"pwrap", mt7623_pwrap_groups, ARRAY_SIZE(mt7623_pwrap_groups)},
	{"rtc", mt7623_rtc_groups, ARRAY_SIZE(mt7623_rtc_groups)},
	{"spi",	mt7623_spi_groups, ARRAY_SIZE(mt7623_spi_groups)},
	{"spdif", mt7623_spdif_groups, ARRAY_SIZE(mt7623_spdif_groups)},
	{"uart", mt7623_uart_groups, ARRAY_SIZE(mt7623_uart_groups)},
	{"watchdog", mt7623_wdt_groups, ARRAY_SIZE(mt7623_wdt_groups)},
};

static struct mtk_pinctrl_soc mt7623_data = {
	.name = "mt7623_pinctrl",
	.reg_cal = mt7623_reg_cals,
	.pins = mt7623_pins,
	.npins = ARRAY_SIZE(mt7623_pins),
	.grps = mt7623_groups,
	.ngrps = ARRAY_SIZE(mt7623_groups),
	.funcs = mt7623_functions,
	.nfuncs = ARRAY_SIZE(mt7623_functions),
};

/*
 * There are some specific pins have mux functions greater than 8,
 * and if we want to switch thees high modes we need to disable
 * bonding constraints firstly.
 */
static void mt7623_bonding_disable(struct udevice *dev)
{
	mtk_rmw(dev, PIN_BOND_REG0, BOND_PCIE_CLR, BOND_PCIE_CLR);
	mtk_rmw(dev, PIN_BOND_REG1, BOND_I2S_CLR, BOND_I2S_CLR);
	mtk_rmw(dev, PIN_BOND_REG2, BOND_MSDC0E_CLR, BOND_MSDC0E_CLR);
}

static int mtk_pinctrl_mt7623_probe(struct udevice *dev)
{
	int err;

	err = mtk_pinctrl_common_probe(dev, &mt7623_data);
	if (err)
		return err;

	mt7623_bonding_disable(dev);

	return 0;
}

static const struct udevice_id mt7623_pctrl_match[] = {
	{ .compatible = "mediatek,mt7623-pinctrl", },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(mt7623_pinctrl) = {
	.name = "mt7623_pinctrl",
	.id = UCLASS_PINCTRL,
	.of_match = mt7623_pctrl_match,
	.ops = &mtk_pinctrl_ops,
	.probe = mtk_pinctrl_mt7623_probe,
	.priv_auto_alloc_size = sizeof(struct mtk_pinctrl_priv),
};
