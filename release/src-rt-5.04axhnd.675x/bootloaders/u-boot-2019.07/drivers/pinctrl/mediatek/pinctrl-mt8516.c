// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2018 BayLibre, SAS
 * Author: Fabien Parent <fparent@baylibre.com>
 */

#include <dm.h>

#include "pinctrl-mtk-common.h"

#define PIN_FIELD(_s_pin, _e_pin, _s_addr, _x_addrs, _s_bit, _x_bits)	\
	PIN_FIELD_CALC(_s_pin, _e_pin, _s_addr, _x_addrs, _s_bit,	\
		       _x_bits, 16, false)

static const struct mtk_pin_field_calc mt8516_pin_mode_range[] = {
	PIN_FIELD_CALC(0, 124, 0x300, 0x10, 0, 3, 15, false),
};

static const struct mtk_pin_field_calc mt8516_pin_dir_range[] = {
	PIN_FIELD(0, 124, 0x0, 0x10, 0, 1),
};

static const struct mtk_pin_field_calc mt8516_pin_di_range[] = {
	PIN_FIELD(0, 124, 0x200, 0x10, 0, 1),
};

static const struct mtk_pin_field_calc mt8516_pin_do_range[] = {
	PIN_FIELD(0, 124, 0x100, 0x10, 0, 1),
};

static const struct mtk_pin_field_calc mt8516_pin_ies_range[] = {
	PIN_FIELD(0, 6, 0x900, 0x10, 2, 1),
	PIN_FIELD(7, 10, 0x900, 0x10, 3, 1),
	PIN_FIELD(11, 13, 0x900, 0x10, 12, 1),
	PIN_FIELD(14, 17, 0x900, 0x10, 13, 1),
	PIN_FIELD(18, 20, 0x910, 0x10, 10, 1),
	PIN_FIELD(21, 23, 0x900, 0x10, 13, 1),
	PIN_FIELD(24, 25, 0x900, 0x10, 12, 1),
	PIN_FIELD(26, 30, 0x900, 0x10, 0, 1),
	PIN_FIELD(31, 33, 0x900, 0x10, 1, 1),
	PIN_FIELD(34, 39, 0x900, 0x10, 2, 1),
	PIN_FIELD(40, 40, 0x910, 0x10, 11, 1),
	PIN_FIELD(41, 43, 0x900, 0x10, 10, 1),
	PIN_FIELD(44, 47, 0x900, 0x10, 11, 1),
	PIN_FIELD(48, 51, 0x900, 0x10, 14, 1),
	PIN_FIELD(52, 53, 0x910, 0x10, 0, 1),
	PIN_FIELD(54, 54, 0x910, 0x10, 2, 1),
	PIN_FIELD(55, 57, 0x910, 0x10, 4, 1),
	PIN_FIELD(58, 59, 0x900, 0x10, 15, 1),
	PIN_FIELD(60, 61, 0x910, 0x10, 1, 1),
	PIN_FIELD(62, 65, 0x910, 0x10, 5, 1),
	PIN_FIELD(66, 67, 0x910, 0x10, 6, 1),
	PIN_FIELD(68, 68, 0x930, 0x10, 2, 1),
	PIN_FIELD(69, 69, 0x930, 0x10, 1, 1),
	PIN_FIELD(70, 70, 0x930, 0x10, 6, 1),
	PIN_FIELD(71, 71, 0x930, 0x10, 5, 1),
	PIN_FIELD(72, 72, 0x930, 0x10, 4, 1),
	PIN_FIELD(73, 73, 0x930, 0x10, 3, 1),

	PIN_FIELD(100, 103, 0x910, 0x10, 7, 1),
	PIN_FIELD(104, 104, 0x920, 0x10, 12, 1),
	PIN_FIELD(105, 105, 0x920, 0x10, 11, 1),
	PIN_FIELD(106, 106, 0x930, 0x10, 0, 1),
	PIN_FIELD(107, 107, 0x920, 0x10, 15, 1),
	PIN_FIELD(108, 108, 0x920, 0x10, 14, 1),
	PIN_FIELD(109, 109, 0x920, 0x10, 13, 1),
	PIN_FIELD(110, 110, 0x920, 0x10, 9, 1),
	PIN_FIELD(111, 111, 0x920, 0x10, 8, 1),
	PIN_FIELD(112, 112, 0x920, 0x10, 7, 1),
	PIN_FIELD(113, 113, 0x920, 0x10, 6, 1),
	PIN_FIELD(114, 114, 0x920, 0x10, 10, 1),
	PIN_FIELD(115, 115, 0x920, 0x10, 1, 1),
	PIN_FIELD(116, 116, 0x920, 0x10, 0, 1),
	PIN_FIELD(117, 117, 0x920, 0x10, 5, 1),
	PIN_FIELD(118, 118, 0x920, 0x10, 4, 1),
	PIN_FIELD(119, 119, 0x920, 0x10, 3, 1),
	PIN_FIELD(120, 120, 0x920, 0x10, 2, 1),
	PIN_FIELD(121, 124, 0x910, 0x10, 9, 1),
};

static const struct mtk_pin_field_calc mt8516_pin_smt_range[] = {
	PIN_FIELD(0, 6, 0xA00, 0x10, 2, 1),
	PIN_FIELD(7, 10, 0xA00, 0x10, 3, 1),
	PIN_FIELD(11, 13, 0xA00, 0x10, 12, 1),
	PIN_FIELD(14, 17, 0xA00, 0x10, 13, 1),
	PIN_FIELD(18, 20, 0xA10, 0x10, 10, 1),
	PIN_FIELD(21, 23, 0xA00, 0x10, 13, 1),
	PIN_FIELD(24, 25, 0xA00, 0x10, 12, 1),
	PIN_FIELD(26, 30, 0xA00, 0x10, 0, 1),
	PIN_FIELD(31, 33, 0xA00, 0x10, 1, 1),
	PIN_FIELD(40, 40, 0xA10, 0x10, 11, 1),
	PIN_FIELD(41, 43, 0xA00, 0x10, 10, 1),
	PIN_FIELD(44, 47, 0xA00, 0x10, 11, 1),
	PIN_FIELD(48, 51, 0xA00, 0x10, 14, 1),
	PIN_FIELD(52, 53, 0xA10, 0x10, 0, 1),
	PIN_FIELD(54, 54, 0xA10, 0x10, 2, 1),
	PIN_FIELD(55, 57, 0xA10, 0x10, 4, 1),
	PIN_FIELD(58, 59, 0xA00, 0x10, 15, 1),
	PIN_FIELD(60, 61, 0xA10, 0x10, 1, 1),
	PIN_FIELD(62, 65, 0xA10, 0x10, 5, 1),
	PIN_FIELD(66, 67, 0xA10, 0x10, 6, 1),
	PIN_FIELD(68, 68, 0xA30, 0x10, 2, 1),
	PIN_FIELD(69, 69, 0xA30, 0x10, 1, 1),
	PIN_FIELD(70, 70, 0xA30, 0x10, 3, 1),
	PIN_FIELD(71, 71, 0xA30, 0x10, 4, 1),
	PIN_FIELD(72, 72, 0xA30, 0x10, 5, 1),
	PIN_FIELD(73, 73, 0xA30, 0x10, 6, 1),

	PIN_FIELD(100, 103, 0xA10, 0x10, 7, 1),
	PIN_FIELD(104, 104, 0xA20, 0x10, 12, 1),
	PIN_FIELD(105, 105, 0xA20, 0x10, 11, 1),
	PIN_FIELD(106, 106, 0xA30, 0x10, 13, 1),
	PIN_FIELD(107, 107, 0xA20, 0x10, 14, 1),
	PIN_FIELD(108, 108, 0xA20, 0x10, 15, 1),
	PIN_FIELD(109, 109, 0xA30, 0x10, 0, 1),
	PIN_FIELD(110, 110, 0xA20, 0x10, 9, 1),
	PIN_FIELD(111, 111, 0xA20, 0x10, 8, 1),
	PIN_FIELD(112, 112, 0xA20, 0x10, 7, 1),
	PIN_FIELD(113, 113, 0xA20, 0x10, 6, 1),
	PIN_FIELD(114, 114, 0xA20, 0x10, 10, 1),
	PIN_FIELD(115, 115, 0xA20, 0x10, 1, 1),
	PIN_FIELD(116, 116, 0xA20, 0x10, 0, 1),
	PIN_FIELD(117, 117, 0xA20, 0x10, 5, 1),
	PIN_FIELD(118, 118, 0xA20, 0x10, 4, 1),
	PIN_FIELD(119, 119, 0xA20, 0x10, 3, 1),
	PIN_FIELD(120, 120, 0xA20, 0x10, 2, 1),
	PIN_FIELD(121, 124, 0xA10, 0x10, 9, 1),
};

static const struct mtk_pin_field_calc mt8516_pin_pullen_range[] = {
	PIN_FIELD(0, 13, 0x500, 0x10, 0, 1),
	PIN_FIELD(18, 20, 0x510, 0x10, 2, 1),
	PIN_FIELD(24, 31, 0x510, 0x10, 8, 1),
	PIN_FIELD(32, 39, 0x520, 0x10, 0, 1),
	PIN_FIELD(44, 47, 0x520, 0x10, 12, 1),
	PIN_FIELD(48, 63, 0x530, 0x10, 0, 1),
	PIN_FIELD(64, 67, 0x540, 0x10, 0, 1),
	PIN_FIELD(100, 103, 0x560, 0x10, 4, 1),
	PIN_FIELD(121, 124, 0x570, 0x10, 9, 1),
};

static const struct mtk_pin_field_calc mt8516_pin_pullsel_range[] = {
	PIN_FIELD(0, 13, 0x600, 0x10, 0, 1),
	PIN_FIELD(18, 20, 0x610, 0x10, 2, 1),
	PIN_FIELD(24, 31, 0x610, 0x10, 8, 1),
	PIN_FIELD(32, 39, 0x620, 0x10, 0, 1),
	PIN_FIELD(44, 47, 0x620, 0x10, 12, 1),
	PIN_FIELD(48, 63, 0x630, 0x10, 0, 1),
	PIN_FIELD(64, 67, 0x640, 0x10, 0, 1),
	PIN_FIELD(100, 103, 0x660, 0x10, 4, 1),
	PIN_FIELD(121, 124, 0x670, 0x10, 9, 1),
};

static const struct mtk_pin_field_calc mt8516_pin_drv_range[] = {
	PIN_FIELD(0, 4, 0xd00, 0x10, 0, 4),
	PIN_FIELD(5, 10, 0xd00, 0x10, 4, 4),
	PIN_FIELD(11, 13, 0xd00, 0x10, 8, 4),
	PIN_FIELD(14, 17, 0xd00, 0x10, 12, 4),
	PIN_FIELD(18, 20, 0xd10, 0x10, 0, 4),
	PIN_FIELD(21, 23, 0xd00, 0x10, 12, 4),
	PIN_FIELD(24, 25, 0xd00, 0x10, 8, 4),
	PIN_FIELD(26, 30, 0xd10, 0x10, 4, 4),
	PIN_FIELD(31, 33, 0xd10, 0x10, 8, 4),
	PIN_FIELD(34, 35, 0xd10, 0x10, 12, 4),
	PIN_FIELD(36, 39, 0xd20, 0x10, 0, 4),
	PIN_FIELD(40, 40, 0xd20, 0x10, 4, 4),
	PIN_FIELD(41, 43, 0xd20, 0x10, 8, 4),
	PIN_FIELD(44, 47, 0xd20, 0x10, 12, 4),
	PIN_FIELD(48, 51, 0xd30, 0x10, 0, 4),
	PIN_FIELD(54, 54, 0xd30, 0x10, 8, 4),
	PIN_FIELD(55, 57, 0xd30, 0x10, 12, 4),
	PIN_FIELD(62, 67, 0xd40, 0x10, 8, 4),
	PIN_FIELD(68, 68, 0xd40, 0x10, 12, 4),
	PIN_FIELD(69, 69, 0xd50, 0x10, 0, 4),
	PIN_FIELD(70, 73, 0xd50, 0x10, 4, 4),
	PIN_FIELD(100, 103, 0xd50, 0x10, 8, 4),
	PIN_FIELD(104, 104, 0xd50, 0x10, 12, 4),
	PIN_FIELD(105, 105, 0xd60, 0x10, 0, 4),
	PIN_FIELD(106, 109, 0xd60, 0x10, 4, 4),
	PIN_FIELD(110, 113, 0xd70, 0x10, 0, 4),
	PIN_FIELD(114, 114, 0xd70, 0x10, 4, 4),
	PIN_FIELD(115, 115, 0xd60, 0x10, 12, 4),
	PIN_FIELD(116, 116, 0xd60, 0x10, 8, 4),
	PIN_FIELD(117, 120, 0xd70, 0x10, 0, 4),
};

static const struct mtk_pin_reg_calc mt8516_reg_cals[] = {
	[PINCTRL_PIN_REG_MODE] = MTK_RANGE(mt8516_pin_mode_range),
	[PINCTRL_PIN_REG_DIR] = MTK_RANGE(mt8516_pin_dir_range),
	[PINCTRL_PIN_REG_DI] = MTK_RANGE(mt8516_pin_di_range),
	[PINCTRL_PIN_REG_DO] = MTK_RANGE(mt8516_pin_do_range),
	[PINCTRL_PIN_REG_IES] = MTK_RANGE(mt8516_pin_ies_range),
	[PINCTRL_PIN_REG_SMT] = MTK_RANGE(mt8516_pin_smt_range),
	[PINCTRL_PIN_REG_PULLSEL] = MTK_RANGE(mt8516_pin_pullsel_range),
	[PINCTRL_PIN_REG_PULLEN] = MTK_RANGE(mt8516_pin_pullen_range),
	[PINCTRL_PIN_REG_DRV] = MTK_RANGE(mt8516_pin_drv_range),
};

static const struct mtk_pin_desc mt8516_pins[] = {
	MTK_PIN(0, "EINT0", DRV_GRP0),
	MTK_PIN(1, "EINT1", DRV_GRP0),
	MTK_PIN(2, "EINT2", DRV_GRP0),
	MTK_PIN(3, "EINT3", DRV_GRP0),
	MTK_PIN(4, "EINT4", DRV_GRP0),
	MTK_PIN(5, "EINT5", DRV_GRP0),
	MTK_PIN(6, "EINT6", DRV_GRP0),
	MTK_PIN(7, "EINT7", DRV_GRP0),
	MTK_PIN(8, "EINT8", DRV_GRP0),
	MTK_PIN(9, "EINT9", DRV_GRP0),
	MTK_PIN(10, "EINT10", DRV_GRP0),
	MTK_PIN(11, "EINT11", DRV_GRP0),
	MTK_PIN(12, "EINT12", DRV_GRP0),
	MTK_PIN(13, "EINT13", DRV_GRP0),
	MTK_PIN(14, "EINT14", DRV_GRP2),
	MTK_PIN(15, "EINT15", DRV_GRP2),
	MTK_PIN(16, "EINT16", DRV_GRP2),
	MTK_PIN(17, "EINT17", DRV_GRP2),
	MTK_PIN(18, "EINT18", DRV_GRP0),
	MTK_PIN(19, "EINT19", DRV_GRP0),
	MTK_PIN(20, "EINT20", DRV_GRP0),
	MTK_PIN(21, "EINT21", DRV_GRP2),
	MTK_PIN(22, "EINT22", DRV_GRP2),
	MTK_PIN(23, "EINT23", DRV_GRP2),
	MTK_PIN(24, "EINT24", DRV_GRP0),
	MTK_PIN(25, "EINT25", DRV_GRP0),
	MTK_PIN(26, "PWRAP_SPI0_MI", DRV_GRP4),
	MTK_PIN(27, "PWRAP_SPI0_MO", DRV_GRP4),
	MTK_PIN(28, "PWRAP_INT", DRV_GRP4),
	MTK_PIN(29, "PWRAP_SPIO0_CK", DRV_GRP4),
	MTK_PIN(30, "PWARP_SPI0_CSN", DRV_GRP4),
	MTK_PIN(31, "RTC32K_CK", DRV_GRP4),
	MTK_PIN(32, "WATCHDOG", DRV_GRP4),
	MTK_PIN(33, "SRCLKENA0", DRV_GRP4),
	MTK_PIN(34, "URXD2", DRV_GRP0),
	MTK_PIN(35, "UTXD2", DRV_GRP0),
	MTK_PIN(36, "MRG_CLK", DRV_GRP0),
	MTK_PIN(37, "MRG_SYNC", DRV_GRP0),
	MTK_PIN(38, "MRG_DI", DRV_GRP0),
	MTK_PIN(39, "MRG_DO", DRV_GRP0),
	MTK_PIN(40, "KPROW0", DRV_GRP2),
	MTK_PIN(41, "KPROW1", DRV_GRP2),
	MTK_PIN(42, "KPCOL0", DRV_GRP2),
	MTK_PIN(43, "KPCOL1", DRV_GRP2),
	MTK_PIN(44, "JMTS", DRV_GRP2),
	MTK_PIN(45, "JTCK", DRV_GRP2),
	MTK_PIN(46, "JTDI", DRV_GRP2),
	MTK_PIN(47, "JTDO", DRV_GRP2),
	MTK_PIN(48, "SPI_CS", DRV_GRP2),
	MTK_PIN(49, "SPI_CK", DRV_GRP2),
	MTK_PIN(50, "SPI_MI", DRV_GRP2),
	MTK_PIN(51, "SPI_MO", DRV_GRP2),
	MTK_PIN(52, "SDA1", DRV_GRP2),
	MTK_PIN(53, "SCL1", DRV_GRP2),
	MTK_PIN(54, "DISP_PWM", DRV_GRP2),
	MTK_PIN(55, "I2S_DATA_IN", DRV_GRP2),
	MTK_PIN(56, "I2S_LRCK", DRV_GRP2),
	MTK_PIN(57, "I2S_BCK", DRV_GRP2),
	MTK_PIN(58, "SDA0", DRV_GRP2),
	MTK_PIN(59, "SCL0", DRV_GRP2),
	MTK_PIN(60, "SDA2", DRV_GRP2),
	MTK_PIN(61, "SCL2", DRV_GRP2),
	MTK_PIN(62, "URXD0", DRV_GRP2),
	MTK_PIN(63, "UTXD0", DRV_GRP2),
	MTK_PIN(64, "URXD1", DRV_GRP2),
	MTK_PIN(65, "UTXD1", DRV_GRP2),
	MTK_PIN(66, "LCM_RST", DRV_GRP2),
	MTK_PIN(67, "DSI_TE", DRV_GRP2),
	MTK_PIN(68, "MSDC2_CMD", DRV_GRP4),
	MTK_PIN(69, "MSDC2_CLK", DRV_GRP4),
	MTK_PIN(70, "MSDC2_DAT0", DRV_GRP4),
	MTK_PIN(71, "MSDC2_DAT1", DRV_GRP4),
	MTK_PIN(72, "MSDC2_DAT2", DRV_GRP4),
	MTK_PIN(73, "MSDC2_DAT3", DRV_GRP4),
	MTK_PIN(74, "TDN3", DRV_GRP0),
	MTK_PIN(75, "TDP3", DRV_GRP0),
	MTK_PIN(76, "TDN2", DRV_GRP0),
	MTK_PIN(77, "TDP2", DRV_GRP0),
	MTK_PIN(78, "TCN", DRV_GRP0),
	MTK_PIN(79, "TCP", DRV_GRP0),
	MTK_PIN(80, "TDN1", DRV_GRP0),
	MTK_PIN(81, "TDP1", DRV_GRP0),
	MTK_PIN(82, "TDN0", DRV_GRP0),
	MTK_PIN(83, "TDP0", DRV_GRP0),
	MTK_PIN(84, "RDN0", DRV_GRP0),
	MTK_PIN(85, "RDP0", DRV_GRP0),
	MTK_PIN(86, "RDN1", DRV_GRP0),
	MTK_PIN(87, "RDP1", DRV_GRP0),
	MTK_PIN(88, "RCN", DRV_GRP0),
	MTK_PIN(89, "RCP", DRV_GRP0),
	MTK_PIN(90, "RDN2", DRV_GRP0),
	MTK_PIN(91, "RDP2", DRV_GRP0),
	MTK_PIN(92, "RDN3", DRV_GRP0),
	MTK_PIN(93, "RDP3", DRV_GRP0),
	MTK_PIN(94, "RCN_A", DRV_GRP0),
	MTK_PIN(95, "RCP_A", DRV_GRP0),
	MTK_PIN(96, "RDN1_A", DRV_GRP0),
	MTK_PIN(97, "RDP1_A", DRV_GRP0),
	MTK_PIN(98, "RDN0_A", DRV_GRP0),
	MTK_PIN(99, "RDP0_A", DRV_GRP0),
	MTK_PIN(100, "CMDDAT0", DRV_GRP2),
	MTK_PIN(101, "CMDDAT1", DRV_GRP2),
	MTK_PIN(102, "CMMCLK", DRV_GRP2),
	MTK_PIN(103, "CMPCLK", DRV_GRP2),
	MTK_PIN(104, "MSDC1_CMD", DRV_GRP4),
	MTK_PIN(105, "MSDC1_CLK", DRV_GRP4),
	MTK_PIN(106, "MSDC1_DAT0", DRV_GRP4),
	MTK_PIN(107, "MSDC1_DAT1", DRV_GRP4),
	MTK_PIN(108, "MSDC1_DAT2", DRV_GRP4),
	MTK_PIN(109, "MSDC1_DAT3", DRV_GRP4),
	MTK_PIN(110, "MSDC0_DAT7", DRV_GRP4),
	MTK_PIN(111, "MSDC0_DAT6", DRV_GRP4),
	MTK_PIN(112, "MSDC0_DAT5", DRV_GRP4),
	MTK_PIN(113, "MSDC0_DAT4", DRV_GRP4),
	MTK_PIN(114, "MSDC0_RSTB", DRV_GRP4),
	MTK_PIN(115, "MSDC0_CMD", DRV_GRP4),
	MTK_PIN(116, "MSDC0_CLK", DRV_GRP4),
	MTK_PIN(117, "MSDC0_DAT3", DRV_GRP4),
	MTK_PIN(118, "MSDC0_DAT2", DRV_GRP4),
	MTK_PIN(119, "MSDC0_DAT1", DRV_GRP4),
	MTK_PIN(120, "MSDC0_DAT0", DRV_GRP4),
};

/* List all groups consisting of these pins dedicated to the enablement of
 * certain hardware block and the corresponding mode for all of the pins.
 * The hardware probably has multiple combinations of these pinouts.
 */

/* UART */
static int mt8516_uart0_0_rxd_txd_pins[]		= { 62, 63, };
static int mt8516_uart0_0_rxd_txd_funcs[]		= {  1,  1, };
static int mt8516_uart1_0_rxd_txd_pins[]		= { 64, 65, };
static int mt8516_uart1_0_rxd_txd_funcs[]		= {  1,  1, };
static int mt8516_uart2_0_rxd_txd_pins[]		= { 34, 35, };
static int mt8516_uart2_0_rxd_txd_funcs[]		= {  1,  1, };

/* Joint those groups owning the same capability in user point of view which
 * allows that people tend to use through the device tree.
 */
static const char *const mt8516_uart_groups[] = { "uart0_0_rxd_txd",
						"uart1_0_rxd_txd",
						"uart2_0_rxd_txd", };

/* MMC0 */
static int mt8516_msdc0_pins[] = { 110, 111, 112, 113, 114, 115, 116, 117, 118,
				   119, 120, };
static int mt8516_msdc0_funcs[] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, };

static const struct mtk_group_desc mt8516_groups[] = {
	PINCTRL_PIN_GROUP("uart0_0_rxd_txd", mt8516_uart0_0_rxd_txd),
	PINCTRL_PIN_GROUP("uart1_0_rxd_txd", mt8516_uart1_0_rxd_txd),
	PINCTRL_PIN_GROUP("uart2_0_rxd_txd", mt8516_uart2_0_rxd_txd),

	PINCTRL_PIN_GROUP("msdc0", mt8516_msdc0),
};

static const char *const mt8516_msdc_groups[] = { "msdc0" };

static const struct mtk_function_desc mt8516_functions[] = {
	{"uart", mt8516_uart_groups, ARRAY_SIZE(mt8516_uart_groups)},
	{"msdc", mt8516_msdc_groups, ARRAY_SIZE(mt8516_msdc_groups)},
};

static struct mtk_pinctrl_soc mt8516_data = {
	.name = "mt8516_pinctrl",
	.reg_cal = mt8516_reg_cals,
	.pins = mt8516_pins,
	.npins = ARRAY_SIZE(mt8516_pins),
	.grps = mt8516_groups,
	.ngrps = ARRAY_SIZE(mt8516_groups),
	.funcs = mt8516_functions,
	.nfuncs = ARRAY_SIZE(mt8516_functions),
};

static int mtk_pinctrl_mt8516_probe(struct udevice *dev)
{
	return mtk_pinctrl_common_probe(dev, &mt8516_data);
}

static const struct udevice_id mt8516_pctrl_match[] = {
	{ .compatible = "mediatek,mt8516-pinctrl" },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(mt8516_pinctrl) = {
	.name = "mt8516_pinctrl",
	.id = UCLASS_PINCTRL,
	.of_match = mt8516_pctrl_match,
	.ops = &mtk_pinctrl_ops,
	.probe = mtk_pinctrl_mt8516_probe,
	.priv_auto_alloc_size = sizeof(struct mtk_pinctrl_priv),
};
