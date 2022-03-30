// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2018 MediaTek Inc.
 * Author: Ryder Lee <ryder.lee@mediatek.com>
 */

#include <dm.h>

#include "pinctrl-mtk-common.h"

#define PIN_FIELD(_s_pin, _e_pin, _s_addr, _x_addrs, _s_bit, _x_bits)	\
	PIN_FIELD_CALC(_s_pin, _e_pin, _s_addr, _x_addrs, _s_bit,	\
		       _x_bits, 32, false)

#define MT7629_PIN(_number, _name)	MTK_PIN(_number, _name, DRV_GRP1)

static const struct mtk_pin_field_calc mt7629_pin_mode_range[] = {
	PIN_FIELD(0, 78, 0x300, 0x10, 0, 4),
};

static const struct mtk_pin_field_calc mt7629_pin_dir_range[] = {
	PIN_FIELD(0, 78, 0x0, 0x10, 0, 1),
};

static const struct mtk_pin_field_calc mt7629_pin_di_range[] = {
	PIN_FIELD(0, 78, 0x200, 0x10, 0, 1),
};

static const struct mtk_pin_field_calc mt7629_pin_do_range[] = {
	PIN_FIELD(0, 78, 0x100, 0x10, 0, 1),
};

static const struct mtk_pin_field_calc mt7629_pin_ies_range[] = {
	PIN_FIELD(0, 10, 0x1000, 0x10, 0, 1),
	PIN_FIELD(11, 18, 0x2000, 0x10, 0, 1),
	PIN_FIELD(19, 32, 0x3000, 0x10, 0, 1),
	PIN_FIELD(33, 48, 0x4000, 0x10, 0, 1),
	PIN_FIELD(49, 50, 0x5000, 0x10, 0, 1),
	PIN_FIELD(51, 69, 0x6000, 0x10, 0, 1),
	PIN_FIELD(70, 78, 0x7000, 0x10, 0, 1),
};

static const struct mtk_pin_field_calc mt7629_pin_smt_range[] = {
	PIN_FIELD(0, 10, 0x1100, 0x10, 0, 1),
	PIN_FIELD(11, 18, 0x2100, 0x10, 0, 1),
	PIN_FIELD(19, 32, 0x3100, 0x10, 0, 1),
	PIN_FIELD(33, 48, 0x4100, 0x10, 0, 1),
	PIN_FIELD(49, 50, 0x5100, 0x10, 0, 1),
	PIN_FIELD(51, 69, 0x6100, 0x10, 0, 1),
	PIN_FIELD(70, 78, 0x7100, 0x10, 0, 1),
};

static const struct mtk_pin_field_calc mt7629_pin_pullen_range[] = {
	PIN_FIELD(0, 10, 0x1400, 0x10, 0, 1),
	PIN_FIELD(11, 18, 0x2400, 0x10, 0, 1),
	PIN_FIELD(19, 32, 0x3400, 0x10, 0, 1),
	PIN_FIELD(33, 48, 0x4400, 0x10, 0, 1),
	PIN_FIELD(49, 50, 0x5400, 0x10, 0, 1),
	PIN_FIELD(51, 69, 0x6400, 0x10, 0, 1),
	PIN_FIELD(70, 78, 0x7400, 0x10, 0, 1),
};

static const struct mtk_pin_field_calc mt7629_pin_pullsel_range[] = {
	PIN_FIELD(0, 10, 0x1500, 0x10, 0, 1),
	PIN_FIELD(11, 18, 0x2500, 0x10, 0, 1),
	PIN_FIELD(19, 32, 0x3500, 0x10, 0, 1),
	PIN_FIELD(33, 48, 0x4500, 0x10, 0, 1),
	PIN_FIELD(49, 50, 0x5500, 0x10, 0, 1),
	PIN_FIELD(51, 69, 0x6500, 0x10, 0, 1),
	PIN_FIELD(70, 78, 0x7500, 0x10, 0, 1),
};

static const struct mtk_pin_field_calc mt7629_pin_drv_range[] = {
	PIN_FIELD(0, 10, 0x1600, 0x10, 0, 4),
	PIN_FIELD(11, 18, 0x2600, 0x10, 0, 4),
	PIN_FIELD(19, 32, 0x3600, 0x10, 0, 4),
	PIN_FIELD(33, 48, 0x4600, 0x10, 0, 4),
	PIN_FIELD(49, 50, 0x5600, 0x10, 0, 4),
	PIN_FIELD(51, 69, 0x6600, 0x10, 0, 4),
	PIN_FIELD(70, 78, 0x7600, 0x10, 0, 4),
};

static const struct mtk_pin_reg_calc mt7629_reg_cals[] = {
	[PINCTRL_PIN_REG_MODE] = MTK_RANGE(mt7629_pin_mode_range),
	[PINCTRL_PIN_REG_DIR] = MTK_RANGE(mt7629_pin_dir_range),
	[PINCTRL_PIN_REG_DI] = MTK_RANGE(mt7629_pin_di_range),
	[PINCTRL_PIN_REG_DO] = MTK_RANGE(mt7629_pin_do_range),
	[PINCTRL_PIN_REG_IES] = MTK_RANGE(mt7629_pin_ies_range),
	[PINCTRL_PIN_REG_SMT] = MTK_RANGE(mt7629_pin_smt_range),
	[PINCTRL_PIN_REG_PULLSEL] = MTK_RANGE(mt7629_pin_pullsel_range),
	[PINCTRL_PIN_REG_PULLEN] = MTK_RANGE(mt7629_pin_pullen_range),
	[PINCTRL_PIN_REG_DRV] = MTK_RANGE(mt7629_pin_drv_range),
};

static const struct mtk_pin_desc mt7629_pins[] = {
	MT7629_PIN(0, "TOP_5G_CLK"),
	MT7629_PIN(1, "TOP_5G_DATA"),
	MT7629_PIN(2, "WF0_5G_HB0"),
	MT7629_PIN(3, "WF0_5G_HB1"),
	MT7629_PIN(4, "WF0_5G_HB2"),
	MT7629_PIN(5, "WF0_5G_HB3"),
	MT7629_PIN(6, "WF0_5G_HB4"),
	MT7629_PIN(7, "WF0_5G_HB5"),
	MT7629_PIN(8, "WF0_5G_HB6"),
	MT7629_PIN(9, "XO_REQ"),
	MT7629_PIN(10, "TOP_RST_N"),
	MT7629_PIN(11, "SYS_WATCHDOG"),
	MT7629_PIN(12, "EPHY_LED0_N_JTDO"),
	MT7629_PIN(13, "EPHY_LED1_N_JTDI"),
	MT7629_PIN(14, "EPHY_LED2_N_JTMS"),
	MT7629_PIN(15, "EPHY_LED3_N_JTCLK"),
	MT7629_PIN(16, "EPHY_LED4_N_JTRST_N"),
	MT7629_PIN(17, "WF2G_LED_N"),
	MT7629_PIN(18, "WF5G_LED_N"),
	MT7629_PIN(19, "I2C_SDA"),
	MT7629_PIN(20, "I2C_SCL"),
	MT7629_PIN(21, "GPIO_9"),
	MT7629_PIN(22, "GPIO_10"),
	MT7629_PIN(23, "GPIO_11"),
	MT7629_PIN(24, "GPIO_12"),
	MT7629_PIN(25, "UART1_TXD"),
	MT7629_PIN(26, "UART1_RXD"),
	MT7629_PIN(27, "UART1_CTS"),
	MT7629_PIN(28, "UART1_RTS"),
	MT7629_PIN(29, "UART2_TXD"),
	MT7629_PIN(30, "UART2_RXD"),
	MT7629_PIN(31, "UART2_CTS"),
	MT7629_PIN(32, "UART2_RTS"),
	MT7629_PIN(33, "MDI_TP_P1"),
	MT7629_PIN(34, "MDI_TN_P1"),
	MT7629_PIN(35, "MDI_RP_P1"),
	MT7629_PIN(36, "MDI_RN_P1"),
	MT7629_PIN(37, "MDI_RP_P2"),
	MT7629_PIN(38, "MDI_RN_P2"),
	MT7629_PIN(39, "MDI_TP_P2"),
	MT7629_PIN(40, "MDI_TN_P2"),
	MT7629_PIN(41, "MDI_TP_P3"),
	MT7629_PIN(42, "MDI_TN_P3"),
	MT7629_PIN(43, "MDI_RP_P3"),
	MT7629_PIN(44, "MDI_RN_P3"),
	MT7629_PIN(45, "MDI_RP_P4"),
	MT7629_PIN(46, "MDI_RN_P4"),
	MT7629_PIN(47, "MDI_TP_P4"),
	MT7629_PIN(48, "MDI_TN_P4"),
	MT7629_PIN(49, "SMI_MDC"),
	MT7629_PIN(50, "SMI_MDIO"),
	MT7629_PIN(51, "PCIE_PERESET_N"),
	MT7629_PIN(52, "PWM_0"),
	MT7629_PIN(53, "GPIO_0"),
	MT7629_PIN(54, "GPIO_1"),
	MT7629_PIN(55, "GPIO_2"),
	MT7629_PIN(56, "GPIO_3"),
	MT7629_PIN(57, "GPIO_4"),
	MT7629_PIN(58, "GPIO_5"),
	MT7629_PIN(59, "GPIO_6"),
	MT7629_PIN(60, "GPIO_7"),
	MT7629_PIN(61, "GPIO_8"),
	MT7629_PIN(62, "SPI_CLK"),
	MT7629_PIN(63, "SPI_CS"),
	MT7629_PIN(64, "SPI_MOSI"),
	MT7629_PIN(65, "SPI_MISO"),
	MT7629_PIN(66, "SPI_WP"),
	MT7629_PIN(67, "SPI_HOLD"),
	MT7629_PIN(68, "UART0_TXD"),
	MT7629_PIN(69, "UART0_RXD"),
	MT7629_PIN(70, "TOP_2G_CLK"),
	MT7629_PIN(71, "TOP_2G_DATA"),
	MT7629_PIN(72, "WF0_2G_HB0"),
	MT7629_PIN(73, "WF0_2G_HB1"),
	MT7629_PIN(74, "WF0_2G_HB2"),
	MT7629_PIN(75, "WF0_2G_HB3"),
	MT7629_PIN(76, "WF0_2G_HB4"),
	MT7629_PIN(77, "WF0_2G_HB5"),
	MT7629_PIN(78, "WF0_2G_HB6"),
};

/* List all groups consisting of these pins dedicated to the enablement of
 * certain hardware block and the corresponding mode for all of the pins.
 * The hardware probably has multiple combinations of these pinouts.
 */

/* WF 5G */
static int mt7629_wf0_5g_pins[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, };
static int mt7629_wf0_5g_funcs[] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, };

/* LED for EPHY */
static int mt7629_ephy_leds_pins[] = { 12, 13, 14, 15, 16, 17, 18, };
static int mt7629_ephy_leds_funcs[] = { 1, 1, 1, 1, 1, 1, 1, };
static int mt7629_ephy_led0_pins[] = { 12, };
static int mt7629_ephy_led0_funcs[] = { 1, };
static int mt7629_ephy_led1_pins[] = { 13, };
static int mt7629_ephy_led1_funcs[] = { 1, };
static int mt7629_ephy_led2_pins[] = { 14, };
static int mt7629_ephy_led2_funcs[] = { 1, };
static int mt7629_ephy_led3_pins[] = { 15, };
static int mt7629_ephy_led3_funcs[] = { 1, };
static int mt7629_ephy_led4_pins[] = { 16, };
static int mt7629_ephy_led4_funcs[] = { 1, };
static int mt7629_wf2g_led_pins[] = { 17, };
static int mt7629_wf2g_led_funcs[] = { 1, };
static int mt7629_wf5g_led_pins[] = { 18, };
static int mt7629_wf5g_led_funcs[] = { 1, };

/* Watchdog */
static int mt7629_watchdog_pins[] = { 11, };
static int mt7629_watchdog_funcs[] = { 1, };

/* LED for GPHY */
static int mt7629_gphy_leds_0_pins[] = { 21, 22, 23, };
static int mt7629_gphy_leds_0_funcs[] = { 2, 2, 2, };
static int mt7629_gphy_led1_0_pins[] = { 21, };
static int mt7629_gphy_led1_0_funcs[] = { 2, };
static int mt7629_gphy_led2_0_pins[] = { 22, };
static int mt7629_gphy_led2_0_funcs[] = { 2, };
static int mt7629_gphy_led3_0_pins[] = { 23, };
static int mt7629_gphy_led3_0_funcs[] = { 2, };
static int mt7629_gphy_leds_1_pins[] = { 57, 58, 59, };
static int mt7629_gphy_leds_1_funcs[] = { 1, 1, 1, };
static int mt7629_gphy_led1_1_pins[] = { 57, };
static int mt7629_gphy_led1_1_funcs[] = { 1, };
static int mt7629_gphy_led2_1_pins[] = { 58, };
static int mt7629_gphy_led2_1_funcs[] = { 1, };
static int mt7629_gphy_led3_1_pins[] = { 59, };
static int mt7629_gphy_led3_1_funcs[] = { 1, };

/* I2C */
static int mt7629_i2c_0_pins[] = { 19, 20, };
static int mt7629_i2c_0_funcs[] = { 1, 1, };
static int mt7629_i2c_1_pins[] = { 53, 54, };
static int mt7629_i2c_1_funcs[] = { 1, 1, };

/* SPI */
static int mt7629_spi_0_pins[] = { 21, 22, 23, 24, };
static int mt7629_spi_0_funcs[] = { 1, 1, 1, 1, };
static int mt7629_spi_1_pins[] = { 62, 63, 64, 65, };
static int mt7629_spi_1_funcs[] = { 1, 1, 1, 1, };
static int mt7629_spi_wp_pins[] = { 66, };
static int mt7629_spi_wp_funcs[] = { 1, };
static int mt7629_spi_hold_pins[] = { 67, };
static int mt7629_spi_hold_funcs[] = { 1, };

/* UART */
static int mt7629_uart1_0_txd_rxd_pins[] = { 25, 26, };
static int mt7629_uart1_0_txd_rxd_funcs[] = { 1, 1, };
static int mt7629_uart1_1_txd_rxd_pins[] = { 53, 54, };
static int mt7629_uart1_1_txd_rxd_funcs[] = { 2, 2, };
static int mt7629_uart2_0_txd_rxd_pins[] = { 29, 30, };
static int mt7629_uart2_0_txd_rxd_funcs[] = { 1, 1, };
static int mt7629_uart2_1_txd_rxd_pins[] = { 57, 58, };
static int mt7629_uart2_1_txd_rxd_funcs[] = { 2, 2, };
static int mt7629_uart1_0_cts_rts_pins[] = { 27, 28, };
static int mt7629_uart1_0_cts_rts_funcs[] = { 1, 1, };
static int mt7629_uart1_1_cts_rts_pins[] = { 55, 56, };
static int mt7629_uart1_1_cts_rts_funcs[] = { 2, 2, };
static int mt7629_uart2_0_cts_rts_pins[] = { 31, 32, };
static int mt7629_uart2_0_cts_rts_funcs[] = { 1, 1, };
static int mt7629_uart2_1_cts_rts_pins[] = { 59, 60, };
static int mt7629_uart2_1_cts_rts_funcs[] = { 2, 2, };
static int mt7629_uart0_txd_rxd_pins[] = { 68, 69, };
static int mt7629_uart0_txd_rxd_funcs[] = { 1, 1, };

/* MDC/MDIO */
static int mt7629_mdc_mdio_pins[] = { 49, 50, };
static int mt7629_mdc_mdio_funcs[] = { 1, 1, };

/* PCIE */
static int mt7629_pcie_pereset_pins[] = { 51, };
static int mt7629_pcie_pereset_funcs[] = { 1, };
static int mt7629_pcie_wake_pins[] = { 55, };
static int mt7629_pcie_wake_funcs[] = { 1, };
static int mt7629_pcie_clkreq_pins[] = { 56, };
static int mt7629_pcie_clkreq_funcs[] = { 1, };

/* PWM */
static int mt7629_pwm_0_pins[] = { 52, };
static int mt7629_pwm_0_funcs[] = { 1, };
static int mt7629_pwm_1_pins[] = { 61, };
static int mt7629_pwm_1_funcs[] = { 2, };

/* WF 2G */
static int mt7629_wf0_2g_pins[] = { 70, 71, 72, 73, 74, 75, 76, 77, 78, };
static int mt7629_wf0_2g_funcs[] = { 1, 1, 1, 1, 1, 1, 1, 1, };

/* SNFI */
static int mt7629_snfi_pins[] = { 62, 63, 64, 65, 66, 67 };
static int mt7629_snfi_funcs[] = { 2, 2, 2, 2, 2, 2 };

/* SPI NOR */
static int mt7629_snor_pins[] = { 62, 63, 64, 65, 66, 67 };
static int mt7629_snor_funcs[] = { 1, 1, 1, 1, 1, 1 };

static const struct mtk_group_desc mt7629_groups[] = {
	PINCTRL_PIN_GROUP("wf0_5g", mt7629_wf0_5g),
	PINCTRL_PIN_GROUP("ephy_leds", mt7629_ephy_leds),
	PINCTRL_PIN_GROUP("ephy_led0", mt7629_ephy_led0),
	PINCTRL_PIN_GROUP("ephy_led1", mt7629_ephy_led1),
	PINCTRL_PIN_GROUP("ephy_led2", mt7629_ephy_led2),
	PINCTRL_PIN_GROUP("ephy_led3", mt7629_ephy_led3),
	PINCTRL_PIN_GROUP("ephy_led4", mt7629_ephy_led4),
	PINCTRL_PIN_GROUP("wf2g_led", mt7629_wf2g_led),
	PINCTRL_PIN_GROUP("wf5g_led", mt7629_wf5g_led),
	PINCTRL_PIN_GROUP("watchdog", mt7629_watchdog),
	PINCTRL_PIN_GROUP("gphy_leds_0", mt7629_gphy_leds_0),
	PINCTRL_PIN_GROUP("gphy_led1_0", mt7629_gphy_led1_0),
	PINCTRL_PIN_GROUP("gphy_led2_0", mt7629_gphy_led2_0),
	PINCTRL_PIN_GROUP("gphy_led3_0", mt7629_gphy_led3_0),
	PINCTRL_PIN_GROUP("gphy_leds_1", mt7629_gphy_leds_1),
	PINCTRL_PIN_GROUP("gphy_led1_1", mt7629_gphy_led1_1),
	PINCTRL_PIN_GROUP("gphy_led2_1", mt7629_gphy_led2_1),
	PINCTRL_PIN_GROUP("gphy_led3_1", mt7629_gphy_led3_1),
	PINCTRL_PIN_GROUP("i2c_0", mt7629_i2c_0),
	PINCTRL_PIN_GROUP("i2c_1", mt7629_i2c_1),
	PINCTRL_PIN_GROUP("spi_0", mt7629_spi_0),
	PINCTRL_PIN_GROUP("spi_1", mt7629_spi_1),
	PINCTRL_PIN_GROUP("spi_wp", mt7629_spi_wp),
	PINCTRL_PIN_GROUP("spi_hold", mt7629_spi_hold),
	PINCTRL_PIN_GROUP("uart1_0_txd_rxd", mt7629_uart1_0_txd_rxd),
	PINCTRL_PIN_GROUP("uart1_1_txd_rxd", mt7629_uart1_1_txd_rxd),
	PINCTRL_PIN_GROUP("uart2_0_txd_rxd", mt7629_uart2_0_txd_rxd),
	PINCTRL_PIN_GROUP("uart2_1_txd_rxd", mt7629_uart2_1_txd_rxd),
	PINCTRL_PIN_GROUP("uart1_0_cts_rts", mt7629_uart1_0_cts_rts),
	PINCTRL_PIN_GROUP("uart1_1_cts_rts", mt7629_uart1_1_cts_rts),
	PINCTRL_PIN_GROUP("uart2_0_cts_rts", mt7629_uart2_0_cts_rts),
	PINCTRL_PIN_GROUP("uart2_1_cts_rts", mt7629_uart2_1_cts_rts),
	PINCTRL_PIN_GROUP("uart0_txd_rxd", mt7629_uart0_txd_rxd),
	PINCTRL_PIN_GROUP("mdc_mdio", mt7629_mdc_mdio),
	PINCTRL_PIN_GROUP("pcie_pereset", mt7629_pcie_pereset),
	PINCTRL_PIN_GROUP("pcie_wake", mt7629_pcie_wake),
	PINCTRL_PIN_GROUP("pcie_clkreq", mt7629_pcie_clkreq),
	PINCTRL_PIN_GROUP("pwm_0", mt7629_pwm_0),
	PINCTRL_PIN_GROUP("pwm_1", mt7629_pwm_1),
	PINCTRL_PIN_GROUP("wf0_2g", mt7629_wf0_2g),
	PINCTRL_PIN_GROUP("snfi", mt7629_snfi),
	PINCTRL_PIN_GROUP("spi_nor", mt7629_snor),
};

/* Joint those groups owning the same capability in user point of view which
 * allows that people tend to use through the device tree.
 */
static const char *const mt7629_ethernet_groups[] = { "mdc_mdio", };
static const char *const mt7629_i2c_groups[] = { "i2c_0", "i2c_1", };
static const char *const mt7629_led_groups[] = { "ephy_leds", "ephy_led0",
						"ephy_led1", "ephy_led2",
						"ephy_led3", "ephy_led4",
						"wf2g_led", "wf5g_led",
						"gphy_leds_0", "gphy_led1_0",
						"gphy_led2_0", "gphy_led3_0",
						"gphy_leds_1", "gphy_led1_1",
						"gphy_led2_1", "gphy_led3_1",};
static const char *const mt7629_pcie_groups[] = { "pcie_pereset", "pcie_wake",
						"pcie_clkreq", };
static const char *const mt7629_pwm_groups[] = { "pwm_0", "pwm_1", };
static const char *const mt7629_spi_groups[] = { "spi_0", "spi_1", "spi_wp",
						"spi_hold", };
static const char *const mt7629_uart_groups[] = { "uart1_0_txd_rxd",
						"uart1_1_txd_rxd",
						"uart2_0_txd_rxd",
						"uart2_1_txd_rxd",
						"uart1_0_cts_rts",
						"uart1_1_cts_rts",
						"uart2_0_cts_rts",
						"uart2_1_cts_rts",
						"uart0_txd_rxd", };
static const char *const mt7629_wdt_groups[] = { "watchdog", };
static const char *const mt7629_wifi_groups[] = { "wf0_5g", "wf0_2g", };
static const char *const mt7629_flash_groups[] = { "snfi", "spi_nor" };

static const struct mtk_function_desc mt7629_functions[] = {
	{"eth",	mt7629_ethernet_groups, ARRAY_SIZE(mt7629_ethernet_groups)},
	{"i2c", mt7629_i2c_groups, ARRAY_SIZE(mt7629_i2c_groups)},
	{"led",	mt7629_led_groups, ARRAY_SIZE(mt7629_led_groups)},
	{"pcie", mt7629_pcie_groups, ARRAY_SIZE(mt7629_pcie_groups)},
	{"pwm",	mt7629_pwm_groups, ARRAY_SIZE(mt7629_pwm_groups)},
	{"spi",	mt7629_spi_groups, ARRAY_SIZE(mt7629_spi_groups)},
	{"uart", mt7629_uart_groups, ARRAY_SIZE(mt7629_uart_groups)},
	{"watchdog", mt7629_wdt_groups, ARRAY_SIZE(mt7629_wdt_groups)},
	{"wifi", mt7629_wifi_groups, ARRAY_SIZE(mt7629_wifi_groups)},
	{"flash", mt7629_flash_groups, ARRAY_SIZE(mt7629_flash_groups)},
};

static struct mtk_pinctrl_soc mt7629_data = {
	.name = "mt7629_pinctrl",
	.reg_cal = mt7629_reg_cals,
	.pins = mt7629_pins,
	.npins = ARRAY_SIZE(mt7629_pins),
	.grps = mt7629_groups,
	.ngrps = ARRAY_SIZE(mt7629_groups),
	.funcs = mt7629_functions,
	.nfuncs = ARRAY_SIZE(mt7629_functions),
};

static int mtk_pinctrl_mt7629_probe(struct udevice *dev)
{
	return mtk_pinctrl_common_probe(dev, &mt7629_data);
}

static const struct udevice_id mt7629_pctrl_match[] = {
	{ .compatible = "mediatek,mt7629-pinctrl" },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(mt7629_pinctrl) = {
	.name = "mt7629_pinctrl",
	.id = UCLASS_PINCTRL,
	.of_match = mt7629_pctrl_match,
	.ops = &mtk_pinctrl_ops,
	.probe = mtk_pinctrl_mt7629_probe,
	.priv_auto_alloc_size = sizeof(struct mtk_pinctrl_priv),
};
