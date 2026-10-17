// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (C) 2018 Amarula Solutions.
 * Author: Jagan Teki <jagan@amarulasolutions.com>
 */

#include <common.h>
#include <clk-uclass.h>
#include <dm.h>
#include <errno.h>
#include <clk/sunxi.h>
#include <dt-bindings/clock/sun8i-v3s-ccu.h>
#include <dt-bindings/reset/sun8i-v3s-ccu.h>
#include <linux/bitops.h>

static struct ccu_clk_gate v3s_gates[] = {
	[CLK_BUS_MMC0]		= GATE(0x060, BIT(8)),
	[CLK_BUS_MMC1]		= GATE(0x060, BIT(9)),
	[CLK_BUS_MMC2]		= GATE(0x060, BIT(10)),
	[CLK_BUS_SPI0]		= GATE(0x060, BIT(20)),
	[CLK_BUS_OTG]		= GATE(0x060, BIT(24)),

	[CLK_BUS_TCON0]		= GATE(0x064, BIT(4)),
	[CLK_BUS_DE]		= GATE(0x064, BIT(12)),

	[CLK_BUS_PIO]		= GATE(0x068, BIT(5)),

	[CLK_BUS_I2C0]		= GATE(0x06c, BIT(0)),
	[CLK_BUS_I2C1]		= GATE(0x06c, BIT(1)),
	[CLK_BUS_UART0]		= GATE(0x06c, BIT(16)),
	[CLK_BUS_UART1]		= GATE(0x06c, BIT(17)),
	[CLK_BUS_UART2]		= GATE(0x06c, BIT(18)),

	[CLK_SPI0]		= GATE(0x0a0, BIT(31)),

	[CLK_USB_PHY0]          = GATE(0x0cc, BIT(8)),

	[CLK_DE]		= GATE(0x104, BIT(31)),
	[CLK_TCON0]		= GATE(0x118, BIT(31)),
};

static struct ccu_reset v3s_resets[] = {
	[RST_USB_PHY0]		= RESET(0x0cc, BIT(0)),

	[RST_BUS_MMC0]		= RESET(0x2c0, BIT(8)),
	[RST_BUS_MMC1]		= RESET(0x2c0, BIT(9)),
	[RST_BUS_MMC2]		= RESET(0x2c0, BIT(10)),
	[RST_BUS_SPI0]		= RESET(0x2c0, BIT(20)),
	[RST_BUS_OTG]		= RESET(0x2c0, BIT(24)),

	[RST_BUS_TCON0]		= RESET(0x2c4, BIT(4)),
	[RST_BUS_DE]		= RESET(0x2c4, BIT(12)),

	[RST_BUS_I2C0]		= RESET(0x2d8, BIT(0)),
	[RST_BUS_I2C1]		= RESET(0x2d8, BIT(1)),
	[RST_BUS_UART0]		= RESET(0x2d8, BIT(16)),
	[RST_BUS_UART1]		= RESET(0x2d8, BIT(17)),
	[RST_BUS_UART2]		= RESET(0x2d8, BIT(18)),
};

const struct ccu_desc v3s_ccu_desc = {
	.gates = v3s_gates,
	.resets = v3s_resets,
	.num_gates = ARRAY_SIZE(v3s_gates),
	.num_resets = ARRAY_SIZE(v3s_resets),
};
