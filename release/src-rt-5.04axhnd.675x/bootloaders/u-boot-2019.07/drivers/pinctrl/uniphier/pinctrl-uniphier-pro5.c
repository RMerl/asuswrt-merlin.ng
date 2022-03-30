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
static const int emmc_muxvals[] = {0, 0, 0, 0, 0, 0, 0};
static const unsigned emmc_dat8_pins[] = {43, 44, 45, 46};
static const int emmc_dat8_muxvals[] = {0, 0, 0, 0};
static const unsigned i2c0_pins[] = {112, 113};
static const int i2c0_muxvals[] = {0, 0};
static const unsigned i2c1_pins[] = {114, 115};
static const int i2c1_muxvals[] = {0, 0};
static const unsigned i2c2_pins[] = {116, 117};
static const int i2c2_muxvals[] = {0, 0};
static const unsigned i2c3_pins[] = {118, 119};
static const int i2c3_muxvals[] = {0, 0};
static const unsigned i2c5_pins[] = {87, 88};
static const int i2c5_muxvals[] = {2, 2};
static const unsigned i2c5b_pins[] = {196, 197};
static const int i2c5b_muxvals[] = {2, 2};
static const unsigned i2c5c_pins[] = {215, 216};
static const int i2c5c_muxvals[] = {2, 2};
static const unsigned i2c6_pins[] = {101, 102};
static const int i2c6_muxvals[] = {2, 2};
static const unsigned nand_pins[] = {19, 20, 21, 22, 23, 24, 25, 28, 29, 30,
				     31, 32, 33, 34, 35};
static const int nand_muxvals[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static const unsigned nand_cs1_pins[] = {26, 27};
static const int nand_cs1_muxvals[] = {0, 0};
static const unsigned sd_pins[] = {250, 251, 252, 253, 254, 255, 256, 257, 258};
static const int sd_muxvals[] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
static const unsigned spi0_pins[] = {120, 121, 122, 123};
static const int spi0_muxvals[] = {0, 0, 0, 0};
static const unsigned spi1_pins[] = {134, 139, 85, 86};
static const int spi1_muxvals[] = {1, 1, 1, 1};
static const unsigned spi2_pins[] = {55, 56, 57, 58, 82, 83, 84};
static const int spi2_muxvals[] = {0, 0, 0, 0, 1, 1, 1};
static const unsigned system_bus_pins[] = {4, 5, 6, 7, 8, 9, 10, 11, 12, 13,
					   14, 15, 16, 17};
static const int system_bus_muxvals[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
					 0};
static const unsigned system_bus_cs0_pins[] = {105};
static const int system_bus_cs0_muxvals[] = {1};
static const unsigned system_bus_cs1_pins[] = {18};
static const int system_bus_cs1_muxvals[] = {0};
static const unsigned system_bus_cs2_pins[] = {106};
static const int system_bus_cs2_muxvals[] = {1};
static const unsigned system_bus_cs3_pins[] = {100};
static const int system_bus_cs3_muxvals[] = {1};
static const unsigned system_bus_cs4_pins[] = {101};
static const int system_bus_cs4_muxvals[] = {1};
static const unsigned system_bus_cs5_pins[] = {102};
static const int system_bus_cs5_muxvals[] = {1};
static const unsigned system_bus_cs6_pins[] = {69};
static const int system_bus_cs6_muxvals[] = {5};
static const unsigned system_bus_cs7_pins[] = {70};
static const int system_bus_cs7_muxvals[] = {5};
static const unsigned uart0_pins[] = {47, 48};
static const int uart0_muxvals[] = {0, 0};
static const unsigned uart0b_pins[] = {227, 228};
static const int uart0b_muxvals[] = {3, 3};
static const unsigned uart1_pins[] = {49, 50};
static const int uart1_muxvals[] = {0, 0};
static const unsigned uart2_pins[] = {51, 52};
static const int uart2_muxvals[] = {0, 0};
static const unsigned uart3_pins[] = {53, 54};
static const int uart3_muxvals[] = {0, 0};
static const unsigned usb0_pins[] = {124, 125};
static const int usb0_muxvals[] = {0, 0};
static const unsigned usb1_pins[] = {126, 127};
static const int usb1_muxvals[] = {0, 0};
static const unsigned usb2_pins[] = {128, 129};
static const int usb2_muxvals[] = {0, 0};

static const struct uniphier_pinctrl_group uniphier_pro5_groups[] = {
	UNIPHIER_PINCTRL_GROUP_SPL(emmc),
	UNIPHIER_PINCTRL_GROUP_SPL(emmc_dat8),
	UNIPHIER_PINCTRL_GROUP(i2c0),
	UNIPHIER_PINCTRL_GROUP(i2c1),
	UNIPHIER_PINCTRL_GROUP(i2c2),
	UNIPHIER_PINCTRL_GROUP(i2c3),
	UNIPHIER_PINCTRL_GROUP(i2c5),
	UNIPHIER_PINCTRL_GROUP(i2c5b),
	UNIPHIER_PINCTRL_GROUP(i2c5c),
	UNIPHIER_PINCTRL_GROUP(i2c6),
	UNIPHIER_PINCTRL_GROUP(nand),
	UNIPHIER_PINCTRL_GROUP(nand_cs1),
	UNIPHIER_PINCTRL_GROUP(sd),
	UNIPHIER_PINCTRL_GROUP(spi0),
	UNIPHIER_PINCTRL_GROUP(spi1),
	UNIPHIER_PINCTRL_GROUP(spi2),
	UNIPHIER_PINCTRL_GROUP(system_bus),
	UNIPHIER_PINCTRL_GROUP(system_bus_cs0),
	UNIPHIER_PINCTRL_GROUP(system_bus_cs1),
	UNIPHIER_PINCTRL_GROUP(system_bus_cs2),
	UNIPHIER_PINCTRL_GROUP(system_bus_cs3),
	UNIPHIER_PINCTRL_GROUP(system_bus_cs4),
	UNIPHIER_PINCTRL_GROUP(system_bus_cs5),
	UNIPHIER_PINCTRL_GROUP(system_bus_cs6),
	UNIPHIER_PINCTRL_GROUP(system_bus_cs7),
	UNIPHIER_PINCTRL_GROUP_SPL(uart0),
	UNIPHIER_PINCTRL_GROUP_SPL(uart0b),
	UNIPHIER_PINCTRL_GROUP_SPL(uart1),
	UNIPHIER_PINCTRL_GROUP_SPL(uart2),
	UNIPHIER_PINCTRL_GROUP_SPL(uart3),
	UNIPHIER_PINCTRL_GROUP(usb0),
	UNIPHIER_PINCTRL_GROUP(usb1),
	UNIPHIER_PINCTRL_GROUP(usb2),
};

static const char * const uniphier_pro5_functions[] = {
	UNIPHIER_PINMUX_FUNCTION_SPL(emmc),
	UNIPHIER_PINMUX_FUNCTION(i2c0),
	UNIPHIER_PINMUX_FUNCTION(i2c1),
	UNIPHIER_PINMUX_FUNCTION(i2c2),
	UNIPHIER_PINMUX_FUNCTION(i2c3),
	UNIPHIER_PINMUX_FUNCTION(i2c5),
	UNIPHIER_PINMUX_FUNCTION(i2c6),
	UNIPHIER_PINMUX_FUNCTION(nand),
	UNIPHIER_PINMUX_FUNCTION(sd),
	UNIPHIER_PINMUX_FUNCTION(spi0),
	UNIPHIER_PINMUX_FUNCTION(spi1),
	UNIPHIER_PINMUX_FUNCTION(spi2),
	UNIPHIER_PINMUX_FUNCTION(system_bus),
	UNIPHIER_PINMUX_FUNCTION_SPL(uart0),
	UNIPHIER_PINMUX_FUNCTION_SPL(uart1),
	UNIPHIER_PINMUX_FUNCTION_SPL(uart2),
	UNIPHIER_PINMUX_FUNCTION_SPL(uart3),
	UNIPHIER_PINMUX_FUNCTION(usb0),
	UNIPHIER_PINMUX_FUNCTION(usb1),
	UNIPHIER_PINMUX_FUNCTION(usb2),
};

static struct uniphier_pinctrl_socdata uniphier_pro5_pinctrl_socdata = {
	.groups = uniphier_pro5_groups,
	.groups_count = ARRAY_SIZE(uniphier_pro5_groups),
	.functions = uniphier_pro5_functions,
	.functions_count = ARRAY_SIZE(uniphier_pro5_functions),
	.caps = UNIPHIER_PINCTRL_CAPS_PUPD_SIMPLE |
		UNIPHIER_PINCTRL_CAPS_DBGMUX_SEPARATE,
};

static int uniphier_pro5_pinctrl_probe(struct udevice *dev)
{
	return uniphier_pinctrl_probe(dev, &uniphier_pro5_pinctrl_socdata);
}

static const struct udevice_id uniphier_pro5_pinctrl_match[] = {
	{ .compatible = "socionext,uniphier-pro5-pinctrl" },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(uniphier_pro5_pinctrl) = {
	.name = "uniphier-pro5-pinctrl",
	.id = UCLASS_PINCTRL,
	.of_match = of_match_ptr(uniphier_pro5_pinctrl_match),
	.probe = uniphier_pro5_pinctrl_probe,
	.priv_auto_alloc_size = sizeof(struct uniphier_pinctrl_priv),
	.ops = &uniphier_pinctrl_ops,
#if !CONFIG_IS_ENABLED(OF_CONTROL)
	.flags = DM_FLAG_PRE_RELOC,
#endif
};
