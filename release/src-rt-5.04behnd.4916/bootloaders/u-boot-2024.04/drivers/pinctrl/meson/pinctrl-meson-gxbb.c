// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2016 - Beniamino Galvani <b.galvani@gmail.com>
 *
 * Based on code from Linux kernel:
 *   Copyright (C) 2016 Endless Mobile, Inc.
 */

#include <common.h>
#include <dm.h>
#include <dm/pinctrl.h>
#include <dt-bindings/gpio/meson-gxbb-gpio.h>

#include "pinctrl-meson-gx.h"

#define EE_OFF	15

static const unsigned int emmc_nand_d07_pins[] = {
	PIN(BOOT_0, EE_OFF), PIN(BOOT_1, EE_OFF), PIN(BOOT_2, EE_OFF),
	PIN(BOOT_3, EE_OFF), PIN(BOOT_4, EE_OFF), PIN(BOOT_5, EE_OFF),
	PIN(BOOT_6, EE_OFF), PIN(BOOT_7, EE_OFF),
};
static const unsigned int emmc_clk_pins[] = { PIN(BOOT_8, EE_OFF) };
static const unsigned int emmc_cmd_pins[] = { PIN(BOOT_10, EE_OFF) };
static const unsigned int emmc_ds_pins[] = { PIN(BOOT_15, EE_OFF) };

static const unsigned int sdcard_d0_pins[] = { PIN(CARD_1, EE_OFF) };
static const unsigned int sdcard_d1_pins[] = { PIN(CARD_0, EE_OFF) };
static const unsigned int sdcard_d2_pins[] = { PIN(CARD_5, EE_OFF) };
static const unsigned int sdcard_d3_pins[] = { PIN(CARD_4, EE_OFF) };
static const unsigned int sdcard_cmd_pins[] = { PIN(CARD_3, EE_OFF) };
static const unsigned int sdcard_clk_pins[] = { PIN(CARD_2, EE_OFF) };

static const unsigned int uart_tx_a_pins[]	= { PIN(GPIOX_12, EE_OFF) };
static const unsigned int uart_rx_a_pins[]	= { PIN(GPIOX_13, EE_OFF) };
static const unsigned int uart_cts_a_pins[]	= { PIN(GPIOX_14, EE_OFF) };
static const unsigned int uart_rts_a_pins[]	= { PIN(GPIOX_15, EE_OFF) };

static const unsigned int uart_tx_b_pins[]	= { PIN(GPIODV_24, EE_OFF) };
static const unsigned int uart_rx_b_pins[]	= { PIN(GPIODV_25, EE_OFF) };
static const unsigned int uart_cts_b_pins[]	= { PIN(GPIODV_26, EE_OFF) };
static const unsigned int uart_rts_b_pins[]	= { PIN(GPIODV_27, EE_OFF) };

static const unsigned int uart_tx_c_pins[]	= { PIN(GPIOY_13, EE_OFF) };
static const unsigned int uart_rx_c_pins[]	= { PIN(GPIOY_14, EE_OFF) };
static const unsigned int uart_cts_c_pins[]	= { PIN(GPIOX_11, EE_OFF) };
static const unsigned int uart_rts_c_pins[]	= { PIN(GPIOX_12, EE_OFF) };

static const unsigned int eth_mdio_pins[]	= { PIN(GPIOZ_0, EE_OFF) };
static const unsigned int eth_mdc_pins[]	= { PIN(GPIOZ_1, EE_OFF) };
static const unsigned int eth_clk_rx_clk_pins[]	= { PIN(GPIOZ_2, EE_OFF) };
static const unsigned int eth_rx_dv_pins[]	= { PIN(GPIOZ_3, EE_OFF) };
static const unsigned int eth_rxd0_pins[]	= { PIN(GPIOZ_4, EE_OFF) };
static const unsigned int eth_rxd1_pins[]	= { PIN(GPIOZ_5, EE_OFF) };
static const unsigned int eth_rxd2_pins[]	= { PIN(GPIOZ_6, EE_OFF) };
static const unsigned int eth_rxd3_pins[]	= { PIN(GPIOZ_7, EE_OFF) };
static const unsigned int eth_rgmii_tx_clk_pins[] = { PIN(GPIOZ_8, EE_OFF) };
static const unsigned int eth_tx_en_pins[]	= { PIN(GPIOZ_9, EE_OFF) };
static const unsigned int eth_txd0_pins[]	= { PIN(GPIOZ_10, EE_OFF) };
static const unsigned int eth_txd1_pins[]	= { PIN(GPIOZ_11, EE_OFF) };
static const unsigned int eth_txd2_pins[]	= { PIN(GPIOZ_12, EE_OFF) };
static const unsigned int eth_txd3_pins[]	= { PIN(GPIOZ_13, EE_OFF) };

static const unsigned int hdmi_hpd_pins[]	= { PIN(GPIOH_0, EE_OFF) };
static const unsigned int hdmi_sda_pins[]	= { PIN(GPIOH_1, EE_OFF) };
static const unsigned int hdmi_scl_pins[]	= { PIN(GPIOH_2, EE_OFF) };

static const unsigned int uart_tx_ao_a_pins[]	= { PIN(GPIOAO_0, 0) };
static const unsigned int uart_rx_ao_a_pins[]	= { PIN(GPIOAO_1, 0) };
static const unsigned int uart_cts_ao_a_pins[]	= { PIN(GPIOAO_2, 0) };
static const unsigned int uart_rts_ao_a_pins[]	= { PIN(GPIOAO_3, 0) };
static const unsigned int uart_tx_ao_b_pins[]	= { PIN(GPIOAO_0, 0) };
static const unsigned int uart_rx_ao_b_pins[]	= { PIN(GPIOAO_1, 0),
						    PIN(GPIOAO_5, 0) };
static const unsigned int uart_cts_ao_b_pins[]	= { PIN(GPIOAO_2, 0) };
static const unsigned int uart_rts_ao_b_pins[]	= { PIN(GPIOAO_3, 0) };

static const unsigned int i2c_sck_ao_pins[] = {PIN(GPIOAO_4, 0) };
static const unsigned int i2c_sda_ao_pins[] = {PIN(GPIOAO_5, 0) };
static const unsigned int i2c_slave_sck_ao_pins[] = {PIN(GPIOAO_4, 0) };
static const unsigned int i2c_slave_sda_ao_pins[] = {PIN(GPIOAO_5, 0) };

static struct meson_pmx_group meson_gxbb_periphs_groups[] = {
	GPIO_GROUP(GPIOZ_0, EE_OFF),
	GPIO_GROUP(GPIOZ_1, EE_OFF),
	GPIO_GROUP(GPIOZ_2, EE_OFF),
	GPIO_GROUP(GPIOZ_3, EE_OFF),
	GPIO_GROUP(GPIOZ_4, EE_OFF),
	GPIO_GROUP(GPIOZ_5, EE_OFF),
	GPIO_GROUP(GPIOZ_6, EE_OFF),
	GPIO_GROUP(GPIOZ_7, EE_OFF),
	GPIO_GROUP(GPIOZ_8, EE_OFF),
	GPIO_GROUP(GPIOZ_9, EE_OFF),
	GPIO_GROUP(GPIOZ_10, EE_OFF),
	GPIO_GROUP(GPIOZ_11, EE_OFF),
	GPIO_GROUP(GPIOZ_12, EE_OFF),
	GPIO_GROUP(GPIOZ_13, EE_OFF),
	GPIO_GROUP(GPIOZ_14, EE_OFF),
	GPIO_GROUP(GPIOZ_15, EE_OFF),

	GPIO_GROUP(GPIOH_0, EE_OFF),
	GPIO_GROUP(GPIOH_1, EE_OFF),
	GPIO_GROUP(GPIOH_2, EE_OFF),
	GPIO_GROUP(GPIOH_3, EE_OFF),

	GPIO_GROUP(BOOT_0, EE_OFF),
	GPIO_GROUP(BOOT_1, EE_OFF),
	GPIO_GROUP(BOOT_2, EE_OFF),
	GPIO_GROUP(BOOT_3, EE_OFF),
	GPIO_GROUP(BOOT_4, EE_OFF),
	GPIO_GROUP(BOOT_5, EE_OFF),
	GPIO_GROUP(BOOT_6, EE_OFF),
	GPIO_GROUP(BOOT_7, EE_OFF),
	GPIO_GROUP(BOOT_8, EE_OFF),
	GPIO_GROUP(BOOT_9, EE_OFF),
	GPIO_GROUP(BOOT_10, EE_OFF),
	GPIO_GROUP(BOOT_11, EE_OFF),
	GPIO_GROUP(BOOT_12, EE_OFF),
	GPIO_GROUP(BOOT_13, EE_OFF),
	GPIO_GROUP(BOOT_14, EE_OFF),
	GPIO_GROUP(BOOT_15, EE_OFF),
	GPIO_GROUP(BOOT_16, EE_OFF),
	GPIO_GROUP(BOOT_17, EE_OFF),

	GPIO_GROUP(CARD_0, EE_OFF),
	GPIO_GROUP(CARD_1, EE_OFF),
	GPIO_GROUP(CARD_2, EE_OFF),
	GPIO_GROUP(CARD_3, EE_OFF),
	GPIO_GROUP(CARD_4, EE_OFF),
	GPIO_GROUP(CARD_5, EE_OFF),
	GPIO_GROUP(CARD_6, EE_OFF),

	GPIO_GROUP(GPIODV_0, EE_OFF),
	GPIO_GROUP(GPIODV_1, EE_OFF),
	GPIO_GROUP(GPIODV_2, EE_OFF),
	GPIO_GROUP(GPIODV_3, EE_OFF),
	GPIO_GROUP(GPIODV_4, EE_OFF),
	GPIO_GROUP(GPIODV_5, EE_OFF),
	GPIO_GROUP(GPIODV_6, EE_OFF),
	GPIO_GROUP(GPIODV_7, EE_OFF),
	GPIO_GROUP(GPIODV_8, EE_OFF),
	GPIO_GROUP(GPIODV_9, EE_OFF),
	GPIO_GROUP(GPIODV_10, EE_OFF),
	GPIO_GROUP(GPIODV_11, EE_OFF),
	GPIO_GROUP(GPIODV_12, EE_OFF),
	GPIO_GROUP(GPIODV_13, EE_OFF),
	GPIO_GROUP(GPIODV_14, EE_OFF),
	GPIO_GROUP(GPIODV_15, EE_OFF),
	GPIO_GROUP(GPIODV_16, EE_OFF),
	GPIO_GROUP(GPIODV_17, EE_OFF),
	GPIO_GROUP(GPIODV_18, EE_OFF),
	GPIO_GROUP(GPIODV_19, EE_OFF),
	GPIO_GROUP(GPIODV_20, EE_OFF),
	GPIO_GROUP(GPIODV_21, EE_OFF),
	GPIO_GROUP(GPIODV_22, EE_OFF),
	GPIO_GROUP(GPIODV_23, EE_OFF),
	GPIO_GROUP(GPIODV_24, EE_OFF),
	GPIO_GROUP(GPIODV_25, EE_OFF),
	GPIO_GROUP(GPIODV_26, EE_OFF),
	GPIO_GROUP(GPIODV_27, EE_OFF),
	GPIO_GROUP(GPIODV_28, EE_OFF),
	GPIO_GROUP(GPIODV_29, EE_OFF),

	GPIO_GROUP(GPIOY_0, EE_OFF),
	GPIO_GROUP(GPIOY_1, EE_OFF),
	GPIO_GROUP(GPIOY_2, EE_OFF),
	GPIO_GROUP(GPIOY_3, EE_OFF),
	GPIO_GROUP(GPIOY_4, EE_OFF),
	GPIO_GROUP(GPIOY_5, EE_OFF),
	GPIO_GROUP(GPIOY_6, EE_OFF),
	GPIO_GROUP(GPIOY_7, EE_OFF),
	GPIO_GROUP(GPIOY_8, EE_OFF),
	GPIO_GROUP(GPIOY_9, EE_OFF),
	GPIO_GROUP(GPIOY_10, EE_OFF),
	GPIO_GROUP(GPIOY_11, EE_OFF),
	GPIO_GROUP(GPIOY_12, EE_OFF),
	GPIO_GROUP(GPIOY_13, EE_OFF),
	GPIO_GROUP(GPIOY_14, EE_OFF),
	GPIO_GROUP(GPIOY_15, EE_OFF),
	GPIO_GROUP(GPIOY_16, EE_OFF),

	GPIO_GROUP(GPIOX_0, EE_OFF),
	GPIO_GROUP(GPIOX_1, EE_OFF),
	GPIO_GROUP(GPIOX_2, EE_OFF),
	GPIO_GROUP(GPIOX_3, EE_OFF),
	GPIO_GROUP(GPIOX_4, EE_OFF),
	GPIO_GROUP(GPIOX_5, EE_OFF),
	GPIO_GROUP(GPIOX_6, EE_OFF),
	GPIO_GROUP(GPIOX_7, EE_OFF),
	GPIO_GROUP(GPIOX_8, EE_OFF),
	GPIO_GROUP(GPIOX_9, EE_OFF),
	GPIO_GROUP(GPIOX_10, EE_OFF),
	GPIO_GROUP(GPIOX_11, EE_OFF),
	GPIO_GROUP(GPIOX_12, EE_OFF),
	GPIO_GROUP(GPIOX_13, EE_OFF),
	GPIO_GROUP(GPIOX_14, EE_OFF),
	GPIO_GROUP(GPIOX_15, EE_OFF),
	GPIO_GROUP(GPIOX_16, EE_OFF),
	GPIO_GROUP(GPIOX_17, EE_OFF),
	GPIO_GROUP(GPIOX_18, EE_OFF),
	GPIO_GROUP(GPIOX_19, EE_OFF),
	GPIO_GROUP(GPIOX_20, EE_OFF),
	GPIO_GROUP(GPIOX_21, EE_OFF),
	GPIO_GROUP(GPIOX_22, EE_OFF),

	GPIO_GROUP(GPIOCLK_0, EE_OFF),
	GPIO_GROUP(GPIOCLK_1, EE_OFF),
	GPIO_GROUP(GPIOCLK_2, EE_OFF),
	GPIO_GROUP(GPIOCLK_3, EE_OFF),

	/* Bank X */
	GROUP(uart_tx_a,	4,	13),
	GROUP(uart_rx_a,	4,	12),
	GROUP(uart_cts_a,	4,	11),
	GROUP(uart_rts_a,	4,	10),

	/* Bank Y */
	GROUP(uart_cts_c,	1,	19),
	GROUP(uart_rts_c,	1,	18),
	GROUP(uart_tx_c,	1,	17),
	GROUP(uart_rx_c,	1,	16),

	/* Bank Z */
	GROUP(eth_mdio,		6,	1),
	GROUP(eth_mdc,		6,	0),
	GROUP(eth_clk_rx_clk,	6,	13),
	GROUP(eth_rx_dv,	6,	12),
	GROUP(eth_rxd0,		6,	11),
	GROUP(eth_rxd1,		6,	10),
	GROUP(eth_rxd2,		6,	9),
	GROUP(eth_rxd3,		6,	8),
	GROUP(eth_rgmii_tx_clk,	6,	7),
	GROUP(eth_tx_en,	6,	6),
	GROUP(eth_txd0,		6,	5),
	GROUP(eth_txd1,		6,	4),
	GROUP(eth_txd2,		6,	3),
	GROUP(eth_txd3,		6,	2),

	/* Bank H */
	GROUP(hdmi_hpd,		1,	26),
	GROUP(hdmi_sda,		1,	25),
	GROUP(hdmi_scl,		1,	24),

	/* Bank DV */
	GROUP(uart_tx_b,	2,	29),
	GROUP(uart_rx_b,	2,	28),
	GROUP(uart_cts_b,	2,	27),
	GROUP(uart_rts_b,	2,	26),

	/* Bank BOOT */
	GROUP(emmc_nand_d07,	4,	30),
	GROUP(emmc_clk,		4,	18),
	GROUP(emmc_cmd,		4,	19),
	GROUP(emmc_ds,		4,	31),

	/* Bank CARD */
	GROUP(sdcard_d1,	2,	14),
	GROUP(sdcard_d0,	2,	15),
	GROUP(sdcard_d3,	2,	12),
	GROUP(sdcard_d2,	2,	13),
	GROUP(sdcard_cmd,	2,	10),
	GROUP(sdcard_clk,	2,	11),
};

static struct meson_pmx_group meson_gxbb_aobus_groups[] = {
	GPIO_GROUP(GPIOAO_0, 0),
	GPIO_GROUP(GPIOAO_1, 0),
	GPIO_GROUP(GPIOAO_2, 0),
	GPIO_GROUP(GPIOAO_3, 0),
	GPIO_GROUP(GPIOAO_4, 0),
	GPIO_GROUP(GPIOAO_5, 0),
	GPIO_GROUP(GPIOAO_6, 0),
	GPIO_GROUP(GPIOAO_7, 0),
	GPIO_GROUP(GPIOAO_8, 0),
	GPIO_GROUP(GPIOAO_9, 0),
	GPIO_GROUP(GPIOAO_10, 0),
	GPIO_GROUP(GPIOAO_11, 0),
	GPIO_GROUP(GPIOAO_12, 0),
	GPIO_GROUP(GPIOAO_13, 0),

	GPIO_GROUP(GPIO_TEST_N, 0),

	/* bank AO */
	GROUP(uart_tx_ao_b,	0,	26),
	GROUP(uart_rx_ao_b,	0,	25),
	GROUP(uart_tx_ao_a,	0,	12),
	GROUP(uart_rx_ao_a,	0,	11),
	GROUP(uart_cts_ao_a,	0,	10),
	GROUP(uart_rts_ao_a,	0,	9),
	GROUP(uart_cts_ao_b,	0,	8),
	GROUP(uart_rts_ao_b,	0,	7),
	GROUP(i2c_sck_ao,	0,	6),
	GROUP(i2c_sda_ao,	0,	5),
	GROUP(i2c_slave_sck_ao, 0,	2),
	GROUP(i2c_slave_sda_ao, 0,	1),
};

static const char * const gpio_periphs_groups[] = {
	"GPIOZ_0", "GPIOZ_1", "GPIOZ_2", "GPIOZ_3", "GPIOZ_4",
	"GPIOZ_5", "GPIOZ_6", "GPIOZ_7", "GPIOZ_8", "GPIOZ_9",
	"GPIOZ_10", "GPIOZ_11", "GPIOZ_12", "GPIOZ_13", "GPIOZ_14",
	"GPIOZ_15",

	"GPIOH_0", "GPIOH_1", "GPIOH_2", "GPIOH_3",

	"BOOT_0", "BOOT_1", "BOOT_2", "BOOT_3", "BOOT_4",
	"BOOT_5", "BOOT_6", "BOOT_7", "BOOT_8", "BOOT_9",
	"BOOT_10", "BOOT_11", "BOOT_12", "BOOT_13", "BOOT_14",
	"BOOT_15", "BOOT_16", "BOOT_17",

	"CARD_0", "CARD_1", "CARD_2", "CARD_3", "CARD_4",
	"CARD_5", "CARD_6",

	"GPIODV_0", "GPIODV_1", "GPIODV_2", "GPIODV_3", "GPIODV_4",
	"GPIODV_5", "GPIODV_6", "GPIODV_7", "GPIODV_8", "GPIODV_9",
	"GPIODV_10", "GPIODV_11", "GPIODV_12", "GPIODV_13", "GPIODV_14",
	"GPIODV_15", "GPIODV_16", "GPIODV_17", "GPIODV_18", "GPIODV_19",
	"GPIODV_20", "GPIODV_21", "GPIODV_22", "GPIODV_23", "GPIODV_24",
	"GPIODV_25", "GPIODV_26", "GPIODV_27", "GPIODV_28", "GPIODV_29",

	"GPIOY_0", "GPIOY_1", "GPIOY_2", "GPIOY_3", "GPIOY_4",
	"GPIOY_5", "GPIOY_6", "GPIOY_7", "GPIOY_8", "GPIOY_9",
	"GPIOY_10", "GPIOY_11", "GPIOY_12", "GPIOY_13", "GPIOY_14",
	"GPIOY_15", "GPIOY_16",

	"GPIOX_0", "GPIOX_1", "GPIOX_2", "GPIOX_3", "GPIOX_4",
	"GPIOX_5", "GPIOX_6", "GPIOX_7", "GPIOX_8", "GPIOX_9",
	"GPIOX_10", "GPIOX_11", "GPIOX_12", "GPIOX_13", "GPIOX_14",
	"GPIOX_15", "GPIOX_16", "GPIOX_17", "GPIOX_18", "GPIOX_19",
	"GPIOX_20", "GPIOX_21", "GPIOX_22",

	"GPIOCLK_0", "GPIOCLK_1", "GPIOCLK_2", "GPIOCLK_3",
};

static const char * const emmc_groups[] = {
	"emmc_nand_d07", "emmc_clk", "emmc_cmd", "emmc_ds",
};

static const char * const sdcard_groups[] = {
	"sdcard_d0", "sdcard_d1", "sdcard_d2", "sdcard_d3",
	"sdcard_cmd", "sdcard_clk",
};

static const char * const uart_a_groups[] = {
	"uart_tx_a", "uart_rx_a", "uart_cts_a", "uart_rts_a",
};

static const char * const uart_b_groups[] = {
	"uart_tx_b", "uart_rx_b", "uart_cts_b", "uart_rts_b",
};

static const char * const uart_c_groups[] = {
	"uart_tx_c", "uart_rx_c", "uart_cts_c", "uart_rts_c",
};

static const char * const eth_groups[] = {
	"eth_mdio", "eth_mdc", "eth_clk_rx_clk", "eth_rx_dv",
	"eth_rxd0", "eth_rxd1", "eth_rxd2", "eth_rxd3",
	"eth_rgmii_tx_clk", "eth_tx_en",
	"eth_txd0", "eth_txd1", "eth_txd2", "eth_txd3",
};

static const char * const hdmi_hpd_groups[] = {
	"hdmi_hpd",
};

static const char * const hdmi_i2c_groups[] = {
	"hdmi_sda", "hdmi_scl",
};

static const char * const gpio_aobus_groups[] = {
	"GPIOAO_0", "GPIOAO_1", "GPIOAO_2", "GPIOAO_3", "GPIOAO_4",
	"GPIOAO_5", "GPIOAO_6", "GPIOAO_7", "GPIOAO_8", "GPIOAO_9",
	"GPIOAO_10", "GPIOAO_11", "GPIOAO_12", "GPIOAO_13",

	"GPIO_TEST_N",
};

static const char * const uart_ao_groups[] = {
	"uart_tx_ao_a", "uart_rx_ao_a", "uart_cts_ao_a", "uart_rts_ao_a",
};

static const char * const uart_ao_b_groups[] = {
	"uart_tx_ao_b", "uart_rx_ao_b", "uart_cts_ao_b", "uart_rts_ao_b",
};

static const char * const i2c_ao_groups[] = {
	"i2c_sdk_ao", "i2c_sda_ao",
};

static const char * const i2c_slave_ao_groups[] = {
	"i2c_slave_sdk_ao", "i2c_slave_sda_ao",
};

static struct meson_pmx_func meson_gxbb_periphs_functions[] = {
	FUNCTION(gpio_periphs),
	FUNCTION(emmc),
	FUNCTION(sdcard),
	FUNCTION(uart_a),
	FUNCTION(uart_b),
	FUNCTION(uart_c),
	FUNCTION(eth),
	FUNCTION(hdmi_hpd),
	FUNCTION(hdmi_i2c),
};

static struct meson_pmx_func meson_gxbb_aobus_functions[] = {
	FUNCTION(gpio_aobus),
	FUNCTION(uart_ao),
	FUNCTION(uart_ao_b),
	FUNCTION(i2c_ao),
	FUNCTION(i2c_slave_ao),
};

static struct meson_bank meson_gxbb_periphs_banks[] = {
	/*   name    first                      last                    pullen  pull    dir     out     in  */
	BANK("X",    PIN(GPIOX_0, EE_OFF),	PIN(GPIOX_22, EE_OFF),  4,  0,  4,  0,  12, 0,  13, 0,  14, 0),
	BANK("Y",    PIN(GPIOY_0, EE_OFF),	PIN(GPIOY_16, EE_OFF),  1,  0,  1,  0,  3,  0,  4,  0,  5,  0),
	BANK("DV",   PIN(GPIODV_0, EE_OFF),	PIN(GPIODV_29, EE_OFF), 0,  0,  0,  0,  0,  0,  1,  0,  2,  0),
	BANK("H",    PIN(GPIOH_0, EE_OFF),	PIN(GPIOH_3, EE_OFF),   1, 20,  1, 20,  3, 20,  4, 20,  5, 20),
	BANK("Z",    PIN(GPIOZ_0, EE_OFF),	PIN(GPIOZ_15, EE_OFF),  3,  0,  3,  0,  9,  0,  10, 0, 11,  0),
	BANK("CARD", PIN(CARD_0, EE_OFF),	PIN(CARD_6, EE_OFF),    2, 20,  2, 20,  6, 20,  7, 20,  8, 20),
	BANK("BOOT", PIN(BOOT_0, EE_OFF),	PIN(BOOT_17, EE_OFF),   2,  0,  2,  0,  6,  0,  7,  0,  8,  0),
	BANK("CLK",  PIN(GPIOCLK_0, EE_OFF),	PIN(GPIOCLK_3, EE_OFF), 3, 28,  3, 28,  9, 28, 10, 28, 11, 28),
};

static struct meson_bank meson_gxbb_aobus_banks[] = {
	/*   name    first              last               pullen  pull    dir     out     in  */
	BANK("AO",   PIN(GPIOAO_0, 0),  PIN(GPIOAO_13, 0), 0,  0,  0, 16,  0,  0,  0, 16,  1,  0),
};

struct meson_pinctrl_data meson_gxbb_periphs_pinctrl_data = {
	.name		= "periphs-banks",
	.pin_base	= 15,
	.groups		= meson_gxbb_periphs_groups,
	.funcs		= meson_gxbb_periphs_functions,
	.banks		= meson_gxbb_periphs_banks,
	.num_pins	= 119,
	.num_groups	= ARRAY_SIZE(meson_gxbb_periphs_groups),
	.num_funcs	= ARRAY_SIZE(meson_gxbb_periphs_functions),
	.num_banks	= ARRAY_SIZE(meson_gxbb_periphs_banks),
	.gpio_driver	= DM_DRIVER_REF(meson_gx_gpio_driver),
};

struct meson_pinctrl_data meson_gxbb_aobus_pinctrl_data = {
	.name		= "aobus-banks",
	.pin_base	= 0,
	.groups		= meson_gxbb_aobus_groups,
	.funcs		= meson_gxbb_aobus_functions,
	.banks		= meson_gxbb_aobus_banks,
	.num_pins	= 15,
	.num_groups	= ARRAY_SIZE(meson_gxbb_aobus_groups),
	.num_funcs	= ARRAY_SIZE(meson_gxbb_aobus_functions),
	.num_banks	= ARRAY_SIZE(meson_gxbb_aobus_banks),
	.gpio_driver	= DM_DRIVER_REF(meson_gx_gpio_driver),
};

static const struct udevice_id meson_gxbb_pinctrl_match[] = {
	{
		.compatible = "amlogic,meson-gxbb-periphs-pinctrl",
		.data = (ulong)&meson_gxbb_periphs_pinctrl_data,
	},
	{
		.compatible = "amlogic,meson-gxbb-aobus-pinctrl",
		.data = (ulong)&meson_gxbb_aobus_pinctrl_data,
	},
	{ /* sentinel */ }
};

U_BOOT_DRIVER(meson_gxbb_pinctrl) = {
	.name = "meson-gxbb-pinctrl",
	.id = UCLASS_PINCTRL,
	.of_match = of_match_ptr(meson_gxbb_pinctrl_match),
	.probe = meson_pinctrl_probe,
	.priv_auto	= sizeof(struct meson_pinctrl),
	.ops = &meson_gx_pinctrl_ops,
};
