// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017 Socionext Inc.
 *   Author: Masahiro Yamada <yamada.masahiro@socionext.com>
 */

#include <common.h>
#include <dm.h>
#include <dm/pinctrl.h>

#include "pinctrl-uniphier.h"

static const unsigned emmc_pins[] = {31, 32, 33, 34, 35, 36, 37, 38};
static const int emmc_muxvals[] = {0, 0, 0, 0, 0, 0, 0, 0};
static const unsigned emmc_dat8_pins[] = {39, 40, 41, 42};
static const int emmc_dat8_muxvals[] = {0, 0, 0, 0};
static const unsigned ether_rgmii_pins[] = {52, 53, 54, 55, 56, 57, 58, 59, 60,
					    61, 62, 63, 64, 65, 66, 67};
static const int ether_rgmii_muxvals[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
					  0, 0, 0, 0};
static const unsigned ether_rmii_pins[] = {52, 53, 54, 55, 56, 57, 58, 59, 61,
					   63, 64, 67};
static const int ether_rmii_muxvals[] = {0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1};
static const unsigned ether1_rgmii_pins[] = {68, 69, 70, 71, 72, 73, 74, 75, 76,
					     77, 78, 79, 80, 81, 82, 83};
static const int ether1_rgmii_muxvals[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
					   0, 0, 0, 0};
static const unsigned ether1_rmii_pins[] = {68, 69, 70, 71, 72, 73, 74, 75, 77,
					    79, 80, 83};
static const int ether1_rmii_muxvals[] = {0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1};
static const unsigned i2c0_pins[] = {104, 105};
static const int i2c0_muxvals[] = {0, 0};
static const unsigned i2c1_pins[] = {106, 107};
static const int i2c1_muxvals[] = {0, 0};
static const unsigned i2c2_pins[] = {108, 109};
static const int i2c2_muxvals[] = {0, 0};
static const unsigned i2c3_pins[] = {110, 111};
static const int i2c3_muxvals[] = {0, 0};
static const unsigned nand_pins[] = {16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26,
				     27, 28, 29, 30};
static const int nand_muxvals[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static const unsigned sd_pins[] = {43, 44, 45, 46, 47, 48, 49, 50, 51};
static const int sd_muxvals[] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
static const unsigned spi0_pins[] = {100, 101, 102, 103};
static const int spi0_muxvals[] = {0, 0, 0, 0};
static const unsigned spi1_pins[] = {112, 113, 114, 115};
static const int spi1_muxvals[] = {2, 2, 2, 2};
static const unsigned system_bus_pins[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
					   12, 13, 14};
static const int system_bus_muxvals[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
					 0};
static const unsigned system_bus_cs1_pins[] = {15};
static const int system_bus_cs1_muxvals[] = {0};
static const unsigned uart0_pins[] = {92, 93};
static const int uart0_muxvals[] = {0, 0};
static const unsigned uart1_pins[] = {94, 95};
static const int uart1_muxvals[] = {0, 0};
static const unsigned uart2_pins[] = {96, 97};
static const int uart2_muxvals[] = {0, 0};
static const unsigned uart3_pins[] = {98, 99};
static const int uart3_muxvals[] = {0, 0};
static const unsigned usb0_pins[] = {84, 85};
static const int usb0_muxvals[] = {0, 0};
static const unsigned usb1_pins[] = {86, 87};
static const int usb1_muxvals[] = {0, 0};
static const unsigned usb2_pins[] = {88, 89};
static const int usb2_muxvals[] = {0, 0};
static const unsigned usb3_pins[] = {90, 91};
static const int usb3_muxvals[] = {0, 0};

static const struct uniphier_pinctrl_group uniphier_pxs3_groups[] = {
	UNIPHIER_PINCTRL_GROUP(emmc),
	UNIPHIER_PINCTRL_GROUP(emmc_dat8),
	UNIPHIER_PINCTRL_GROUP(ether_rgmii),
	UNIPHIER_PINCTRL_GROUP(ether_rmii),
	UNIPHIER_PINCTRL_GROUP(ether1_rgmii),
	UNIPHIER_PINCTRL_GROUP(ether1_rmii),
	UNIPHIER_PINCTRL_GROUP(i2c0),
	UNIPHIER_PINCTRL_GROUP(i2c1),
	UNIPHIER_PINCTRL_GROUP(i2c2),
	UNIPHIER_PINCTRL_GROUP(i2c3),
	UNIPHIER_PINCTRL_GROUP(nand),
	UNIPHIER_PINCTRL_GROUP(sd),
	UNIPHIER_PINCTRL_GROUP(spi0),
	UNIPHIER_PINCTRL_GROUP(spi1),
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

static const char * const uniphier_pxs3_functions[] = {
	UNIPHIER_PINMUX_FUNCTION(emmc),
	UNIPHIER_PINMUX_FUNCTION(ether_rgmii),
	UNIPHIER_PINMUX_FUNCTION(ether_rmii),
	UNIPHIER_PINMUX_FUNCTION(ether1_rgmii),
	UNIPHIER_PINMUX_FUNCTION(ether1_rmii),
	UNIPHIER_PINMUX_FUNCTION(i2c0),
	UNIPHIER_PINMUX_FUNCTION(i2c1),
	UNIPHIER_PINMUX_FUNCTION(i2c2),
	UNIPHIER_PINMUX_FUNCTION(i2c3),
	UNIPHIER_PINMUX_FUNCTION(nand),
	UNIPHIER_PINMUX_FUNCTION(sd),
	UNIPHIER_PINMUX_FUNCTION(spi0),
	UNIPHIER_PINMUX_FUNCTION(spi1),
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

static struct uniphier_pinctrl_socdata uniphier_pxs3_pinctrl_socdata = {
	.groups = uniphier_pxs3_groups,
	.groups_count = ARRAY_SIZE(uniphier_pxs3_groups),
	.functions = uniphier_pxs3_functions,
	.functions_count = ARRAY_SIZE(uniphier_pxs3_functions),
	.caps = UNIPHIER_PINCTRL_CAPS_PUPD_SIMPLE |
		UNIPHIER_PINCTRL_CAPS_PERPIN_IECTRL,
};

static int uniphier_pxs3_pinctrl_probe(struct udevice *dev)
{
	return uniphier_pinctrl_probe(dev, &uniphier_pxs3_pinctrl_socdata);
}

static const struct udevice_id uniphier_pxs3_pinctrl_match[] = {
	{ .compatible = "socionext,uniphier-pxs3-pinctrl" },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(uniphier_pxs3_pinctrl) = {
	.name = "uniphier-pxs3-pinctrl",
	.id = UCLASS_PINCTRL,
	.of_match = of_match_ptr(uniphier_pxs3_pinctrl_match),
	.probe = uniphier_pxs3_pinctrl_probe,
	.priv_auto_alloc_size = sizeof(struct uniphier_pinctrl_priv),
	.ops = &uniphier_pinctrl_ops,
};
