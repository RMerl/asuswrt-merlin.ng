// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016 Socionext Inc.
 *   Author: Masahiro Yamada <yamada.masahiro@socionext.com>
 */

#include <common.h>
#include <dm.h>
#include <dm/pinctrl.h>

#include "pinctrl-uniphier.h"

static const struct uniphier_pinctrl_pin uniphier_ld20_pins[] = {
	UNIPHIER_PINCTRL_PIN(40, "RGMII_TXCLK", 28, UNIPHIER_PIN_DRV_3BIT),
	UNIPHIER_PINCTRL_PIN(41, "RGMII_TXD0", 29, UNIPHIER_PIN_DRV_3BIT),
	UNIPHIER_PINCTRL_PIN(42, "RGMII_TXD1", 30, UNIPHIER_PIN_DRV_3BIT),
	UNIPHIER_PINCTRL_PIN(43, "RGMII_TXD2", 31, UNIPHIER_PIN_DRV_3BIT),
	UNIPHIER_PINCTRL_PIN(44, "RGMII_TXD3", 32, UNIPHIER_PIN_DRV_3BIT),
	UNIPHIER_PINCTRL_PIN(45, "RGMII_TXCTL", 33, UNIPHIER_PIN_DRV_3BIT),
};

static const unsigned emmc_pins[] = {18, 19, 20, 21, 22, 23, 24, 25};
static const int emmc_muxvals[] = {0, 0, 0, 0, 0, 0, 0, 0};
static const unsigned emmc_dat8_pins[] = {26, 27, 28, 29};
static const int emmc_dat8_muxvals[] = {0, 0, 0, 0};
static const unsigned ether_rgmii_pins[] = {30, 31, 32, 33, 34, 35, 36, 37, 38,
					    39, 40, 41, 42, 43, 44, 45};
static const int ether_rgmii_muxvals[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
					  0, 0, 0, 0};
static const unsigned ether_rmii_pins[] = {30, 31, 32, 33, 34, 35, 36, 37, 39,
					   41, 42, 45};
static const int ether_rmii_muxvals[] = {0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1};
static const unsigned i2c0_pins[] = {63, 64};
static const int i2c0_muxvals[] = {0, 0};
static const unsigned i2c1_pins[] = {65, 66};
static const int i2c1_muxvals[] = {0, 0};
static const unsigned i2c3_pins[] = {67, 68};
static const int i2c3_muxvals[] = {1, 1};
static const unsigned i2c4_pins[] = {61, 62};
static const int i2c4_muxvals[] = {1, 1};
static const unsigned nand_pins[] = {3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14,
				     15, 16, 17};
static const int nand_muxvals[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static const unsigned sd_pins[] = {10, 11, 12, 13, 14, 15, 16, 17};
static const int sd_muxvals[] = {3, 3, 3, 3, 3, 3, 3, 3};  /* No SDVOLC */
static const unsigned spi0_pins[] = {56, 57, 58, 59};
static const int spi0_muxvals[] = {0, 0, 0, 0};
static const unsigned spi1_pins[] = {169, 170, 171, 172};
static const int spi1_muxvals[] = {1, 1, 1, 1};
static const unsigned spi2_pins[] = {86, 87, 88, 89};
static const int spi2_muxvals[] = {1, 1, 1, 1};
static const unsigned spi3_pins[] = {74, 75, 76, 77};
static const int spi3_muxvals[] = {1, 1, 1, 1};
static const unsigned system_bus_pins[] = {1, 2, 6, 7, 8, 9, 10, 11, 12, 13,
					   14, 15, 16, 17};
static const int system_bus_muxvals[] = {0, 0, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
					 2};
static const unsigned system_bus_cs1_pins[] = {0};
static const int system_bus_cs1_muxvals[] = {0};
static const unsigned uart0_pins[] = {54, 55};
static const int uart0_muxvals[] = {0, 0};
static const unsigned uart1_pins[] = {58, 59};
static const int uart1_muxvals[] = {1, 1};
static const unsigned uart2_pins[] = {90, 91};
static const int uart2_muxvals[] = {1, 1};
static const unsigned uart3_pins[] = {94, 95};
static const int uart3_muxvals[] = {1, 1};
static const unsigned usb0_pins[] = {46, 47};
static const int usb0_muxvals[] = {0, 0};
static const unsigned usb1_pins[] = {48, 49};
static const int usb1_muxvals[] = {0, 0};
static const unsigned usb2_pins[] = {50, 51};
static const int usb2_muxvals[] = {0, 0};
static const unsigned usb3_pins[] = {52, 53};
static const int usb3_muxvals[] = {0, 0};

static const struct uniphier_pinctrl_group uniphier_ld20_groups[] = {
	UNIPHIER_PINCTRL_GROUP(emmc),
	UNIPHIER_PINCTRL_GROUP(emmc_dat8),
	UNIPHIER_PINCTRL_GROUP(ether_rgmii),
	UNIPHIER_PINCTRL_GROUP(ether_rmii),
	UNIPHIER_PINCTRL_GROUP(i2c0),
	UNIPHIER_PINCTRL_GROUP(i2c1),
	UNIPHIER_PINCTRL_GROUP(i2c3),
	UNIPHIER_PINCTRL_GROUP(i2c4),
	UNIPHIER_PINCTRL_GROUP(nand),
	UNIPHIER_PINCTRL_GROUP(sd),
	UNIPHIER_PINCTRL_GROUP(spi0),
	UNIPHIER_PINCTRL_GROUP(spi1),
	UNIPHIER_PINCTRL_GROUP(spi2),
	UNIPHIER_PINCTRL_GROUP(spi3),
	UNIPHIER_PINCTRL_GROUP(system_bus),
	UNIPHIER_PINCTRL_GROUP(system_bus_cs1),
	UNIPHIER_PINCTRL_GROUP(uart0),
	UNIPHIER_PINCTRL_GROUP(uart1),
	UNIPHIER_PINCTRL_GROUP(uart2),
	UNIPHIER_PINCTRL_GROUP(uart3),
	UNIPHIER_PINCTRL_GROUP(usb0),
	UNIPHIER_PINCTRL_GROUP(usb1),
	UNIPHIER_PINCTRL_GROUP(usb2),
	UNIPHIER_PINCTRL_GROUP(usb3),
};

static const char * const uniphier_ld20_functions[] = {
	UNIPHIER_PINMUX_FUNCTION(emmc),
	UNIPHIER_PINMUX_FUNCTION(ether_rgmii),
	UNIPHIER_PINMUX_FUNCTION(ether_rmii),
	UNIPHIER_PINMUX_FUNCTION(i2c0),
	UNIPHIER_PINMUX_FUNCTION(i2c1),
	UNIPHIER_PINMUX_FUNCTION(i2c3),
	UNIPHIER_PINMUX_FUNCTION(i2c4),
	UNIPHIER_PINMUX_FUNCTION(nand),
	UNIPHIER_PINMUX_FUNCTION(sd),
	UNIPHIER_PINMUX_FUNCTION(spi0),
	UNIPHIER_PINMUX_FUNCTION(spi1),
	UNIPHIER_PINMUX_FUNCTION(spi2),
	UNIPHIER_PINMUX_FUNCTION(spi3),
	UNIPHIER_PINMUX_FUNCTION(system_bus),
	UNIPHIER_PINMUX_FUNCTION(uart0),
	UNIPHIER_PINMUX_FUNCTION(uart1),
	UNIPHIER_PINMUX_FUNCTION(uart2),
	UNIPHIER_PINMUX_FUNCTION(uart3),
	UNIPHIER_PINMUX_FUNCTION(usb0),
	UNIPHIER_PINMUX_FUNCTION(usb1),
	UNIPHIER_PINMUX_FUNCTION(usb2),
	UNIPHIER_PINMUX_FUNCTION(usb3),
};

static struct uniphier_pinctrl_socdata uniphier_ld20_pinctrl_socdata = {
	.pins = uniphier_ld20_pins,
	.pins_count = ARRAY_SIZE(uniphier_ld20_pins),
	.groups = uniphier_ld20_groups,
	.groups_count = ARRAY_SIZE(uniphier_ld20_groups),
	.functions = uniphier_ld20_functions,
	.functions_count = ARRAY_SIZE(uniphier_ld20_functions),
	.caps = UNIPHIER_PINCTRL_CAPS_PUPD_SIMPLE |
		UNIPHIER_PINCTRL_CAPS_PERPIN_IECTRL,
};

static int uniphier_ld20_pinctrl_probe(struct udevice *dev)
{
	return uniphier_pinctrl_probe(dev, &uniphier_ld20_pinctrl_socdata);
}

static const struct udevice_id uniphier_ld20_pinctrl_match[] = {
	{ .compatible = "socionext,uniphier-ld20-pinctrl" },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(uniphier_ld20_pinctrl) = {
	.name = "uniphier-ld20-pinctrl",
	.id = UCLASS_PINCTRL,
	.of_match = of_match_ptr(uniphier_ld20_pinctrl_match),
	.probe = uniphier_ld20_pinctrl_probe,
	.priv_auto	= sizeof(struct uniphier_pinctrl_priv),
	.ops = &uniphier_pinctrl_ops,
};
