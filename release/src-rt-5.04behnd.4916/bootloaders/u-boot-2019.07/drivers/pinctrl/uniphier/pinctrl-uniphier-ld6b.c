// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015-2016 Socionext Inc.
 *   Author: Masahiro Yamada <yamada.masahiro@socionext.com>
 */

#include <common.h>
#include <dm.h>
#include <dm/pinctrl.h>

#include "pinctrl-uniphier.h"

static const unsigned emmc_pins[] = {36, 37, 38, 39, 40, 41, 42};
static const int emmc_muxvals[] = {1, 1, 1, 1, 1, 1, 1};
static const unsigned emmc_dat8_pins[] = {43, 44, 45, 46};
static const int emmc_dat8_muxvals[] = {1, 1, 1, 1};
static const unsigned ether_rgmii_pins[] = {143, 144, 145, 146, 147, 148, 149,
					    150, 151, 152, 153, 154, 155, 156,
					    157, 158};
static const int ether_rgmii_muxvals[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
					  0, 0, 0, 0};
static const unsigned ether_rmii_pins[] = {143, 144, 145, 146, 147, 148, 149,
					   150, 152, 154, 155, 158};
static const int ether_rmii_muxvals[] = {0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1};
static const unsigned i2c0_pins[] = {109, 110};
static const int i2c0_muxvals[] = {0, 0};
static const unsigned i2c1_pins[] = {111, 112};
static const int i2c1_muxvals[] = {0, 0};
static const unsigned i2c2_pins[] = {115, 116};
static const int i2c2_muxvals[] = {1, 1};
static const unsigned i2c3_pins[] = {118, 119};
static const int i2c3_muxvals[] = {1, 1};
static const unsigned nand_pins[] = {30, 31, 32, 33, 34, 35, 36, 39, 40, 41,
				     42, 43, 44, 45, 46};
static const int nand_muxvals[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static const unsigned nand_cs1_pins[] = {37, 38};
static const int nand_cs1_muxvals[] = {0, 0};
static const unsigned sd_pins[] = {47, 48, 49, 50, 51, 52, 53, 54, 55};
static const int sd_muxvals[] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
static const unsigned spi0_pins[] = {199, 200, 201, 202};
static const int spi0_muxvals[] = {8, 8, 8, 8};
static const unsigned spi1_pins[] = {93, 94, 95, 96};
static const int spi1_muxvals[] = {1, 1, 1, 1};
static const unsigned system_bus_pins[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
					   11, 12, 13};
static const int system_bus_muxvals[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
					 0};
static const unsigned system_bus_cs1_pins[] = {14};
static const int system_bus_cs1_muxvals[] = {0};
static const unsigned system_bus_cs2_pins[] = {37};
static const int system_bus_cs2_muxvals[] = {6};
static const unsigned system_bus_cs3_pins[] = {38};
static const int system_bus_cs3_muxvals[] = {6};
static const unsigned system_bus_cs4_pins[] = {115};
static const int system_bus_cs4_muxvals[] = {6};
static const unsigned system_bus_cs5_pins[] = {55};
static const int system_bus_cs5_muxvals[] = {6};
static const unsigned uart0_pins[] = {135, 136};
static const int uart0_muxvals[] = {3, 3};
static const unsigned uart0b_pins[] = {11, 12};
static const int uart0b_muxvals[] = {2, 2};
static const unsigned uart1_pins[] = {115, 116};
static const int uart1_muxvals[] = {0, 0};
static const unsigned uart1b_pins[] = {113, 114};
static const int uart1b_muxvals[] = {1, 1};
static const unsigned uart2_pins[] = {113, 114};
static const int uart2_muxvals[] = {2, 2};
static const unsigned uart2b_pins[] = {86, 87};
static const int uart2b_muxvals[] = {1, 1};
static const unsigned usb0_pins[] = {56, 57};
static const int usb0_muxvals[] = {0, 0};
static const unsigned usb1_pins[] = {58, 59};
static const int usb1_muxvals[] = {0, 0};
static const unsigned usb2_pins[] = {60, 61};
static const int usb2_muxvals[] = {0, 0};
static const unsigned usb3_pins[] = {62, 63};
static const int usb3_muxvals[] = {0, 0};

static const struct uniphier_pinctrl_group uniphier_ld6b_groups[] = {
	UNIPHIER_PINCTRL_GROUP_SPL(emmc),
	UNIPHIER_PINCTRL_GROUP_SPL(emmc_dat8),
	UNIPHIER_PINCTRL_GROUP(ether_rgmii),
	UNIPHIER_PINCTRL_GROUP(ether_rmii),
	UNIPHIER_PINCTRL_GROUP(i2c0),
	UNIPHIER_PINCTRL_GROUP(i2c1),
	UNIPHIER_PINCTRL_GROUP(i2c2),
	UNIPHIER_PINCTRL_GROUP(i2c3),
	UNIPHIER_PINCTRL_GROUP(nand),
	UNIPHIER_PINCTRL_GROUP(nand_cs1),
	UNIPHIER_PINCTRL_GROUP(sd),
	UNIPHIER_PINCTRL_GROUP(spi0),
	UNIPHIER_PINCTRL_GROUP(spi1),
	UNIPHIER_PINCTRL_GROUP(system_bus),
	UNIPHIER_PINCTRL_GROUP(system_bus_cs1),
	UNIPHIER_PINCTRL_GROUP(system_bus_cs2),
	UNIPHIER_PINCTRL_GROUP(system_bus_cs3),
	UNIPHIER_PINCTRL_GROUP(system_bus_cs4),
	UNIPHIER_PINCTRL_GROUP(system_bus_cs5),
	UNIPHIER_PINCTRL_GROUP_SPL(uart0),
	UNIPHIER_PINCTRL_GROUP_SPL(uart0b),
	UNIPHIER_PINCTRL_GROUP_SPL(uart1),
	UNIPHIER_PINCTRL_GROUP_SPL(uart1b),
	UNIPHIER_PINCTRL_GROUP_SPL(uart2),
	UNIPHIER_PINCTRL_GROUP_SPL(uart2b),
	UNIPHIER_PINCTRL_GROUP(usb0),
	UNIPHIER_PINCTRL_GROUP(usb1),
	UNIPHIER_PINCTRL_GROUP(usb2),
	UNIPHIER_PINCTRL_GROUP(usb3),
};

static const char * const uniphier_ld6b_functions[] = {
	UNIPHIER_PINMUX_FUNCTION_SPL(emmc),
	UNIPHIER_PINMUX_FUNCTION(ether_rgmii),
	UNIPHIER_PINMUX_FUNCTION(ether_rmii),
	UNIPHIER_PINMUX_FUNCTION(i2c0),
	UNIPHIER_PINMUX_FUNCTION(i2c1),
	UNIPHIER_PINMUX_FUNCTION(i2c2),
	UNIPHIER_PINMUX_FUNCTION(i2c3),
	UNIPHIER_PINMUX_FUNCTION(nand),
	UNIPHIER_PINMUX_FUNCTION(sd),
	UNIPHIER_PINMUX_FUNCTION(spi0),
	UNIPHIER_PINMUX_FUNCTION(spi1),
	UNIPHIER_PINMUX_FUNCTION(system_bus),
	UNIPHIER_PINMUX_FUNCTION_SPL(uart0),
	UNIPHIER_PINMUX_FUNCTION_SPL(uart1),
	UNIPHIER_PINMUX_FUNCTION_SPL(uart2),
	UNIPHIER_PINMUX_FUNCTION(usb0),
	UNIPHIER_PINMUX_FUNCTION(usb1),
	UNIPHIER_PINMUX_FUNCTION(usb2),
	UNIPHIER_PINMUX_FUNCTION(usb3),
};

static struct uniphier_pinctrl_socdata uniphier_ld6b_pinctrl_socdata = {
	.groups = uniphier_ld6b_groups,
	.groups_count = ARRAY_SIZE(uniphier_ld6b_groups),
	.functions = uniphier_ld6b_functions,
	.functions_count = ARRAY_SIZE(uniphier_ld6b_functions),
	.caps = UNIPHIER_PINCTRL_CAPS_PUPD_SIMPLE,
};

static int uniphier_ld6b_pinctrl_probe(struct udevice *dev)
{
	return uniphier_pinctrl_probe(dev, &uniphier_ld6b_pinctrl_socdata);
}

static const struct udevice_id uniphier_ld6b_pinctrl_match[] = {
	{ .compatible = "socionext,uniphier-ld6b-pinctrl" },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(uniphier_ld6b_pinctrl) = {
	.name = "uniphier-ld6b-pinctrl",
	.id = UCLASS_PINCTRL,
	.of_match = of_match_ptr(uniphier_ld6b_pinctrl_match),
	.probe = uniphier_ld6b_pinctrl_probe,
	.priv_auto_alloc_size = sizeof(struct uniphier_pinctrl_priv),
	.ops = &uniphier_pinctrl_ops,
};
