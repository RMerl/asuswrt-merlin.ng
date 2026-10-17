// SPDX-License-Identifier: (GPL-2.0+)
/*
 * Copyright (C) 2019 George Hilliard <thirtythreeforty@gmail.com>.
 */

#include <common.h>
#include <clk-uclass.h>
#include <dm.h>
#include <errno.h>
#include <clk/sunxi.h>
#include <dt-bindings/clock/suniv-ccu-f1c100s.h>
#include <dt-bindings/reset/suniv-ccu-f1c100s.h>

static struct ccu_clk_gate f1c100s_gates[] = {
	[CLK_BUS_MMC0]		= GATE(0x060, BIT(8)),
	[CLK_BUS_MMC1]		= GATE(0x060, BIT(9)),
	[CLK_BUS_SPI0]		= GATE(0x060, BIT(20)),
	[CLK_BUS_SPI1]		= GATE(0x060, BIT(21)),
	[CLK_BUS_OTG]		= GATE(0x060, BIT(24)),

	[CLK_BUS_I2C0]		= GATE(0x068, BIT(16)),
	[CLK_BUS_I2C1]		= GATE(0x068, BIT(17)),
	[CLK_BUS_I2C2]		= GATE(0x068, BIT(18)),
	[CLK_BUS_PIO]		= GATE(0x068, BIT(19)),

	[CLK_BUS_UART0]		= GATE(0x06c, BIT(20)),
	[CLK_BUS_UART1]		= GATE(0x06c, BIT(21)),
	[CLK_BUS_UART2]		= GATE(0x06c, BIT(22)),

	[CLK_USB_PHY0]          = GATE(0x0cc, BIT(1)),
};

static struct ccu_reset f1c100s_resets[] = {
	[RST_USB_PHY0]		= RESET(0x0cc, BIT(0)),

	[RST_BUS_MMC0]		= RESET(0x2c0, BIT(8)),
	[RST_BUS_MMC1]		= RESET(0x2c0, BIT(9)),
	[RST_BUS_SPI0]		= RESET(0x2c0, BIT(20)),
	[RST_BUS_SPI1]		= RESET(0x2c0, BIT(21)),
	[RST_BUS_OTG]		= RESET(0x2c0, BIT(24)),

	[RST_BUS_I2C0]		= RESET(0x2d0, BIT(16)),
	[RST_BUS_I2C1]		= RESET(0x2d0, BIT(17)),
	[RST_BUS_I2C2]		= RESET(0x2d0, BIT(18)),
	[RST_BUS_UART0]		= RESET(0x2d0, BIT(20)),
	[RST_BUS_UART1]		= RESET(0x2d0, BIT(21)),
	[RST_BUS_UART2]		= RESET(0x2d0, BIT(22)),
};

const struct ccu_desc f1c100s_ccu_desc = {
	.gates = f1c100s_gates,
	.resets = f1c100s_resets,
	.num_gates = ARRAY_SIZE(f1c100s_gates),
	.num_resets = ARRAY_SIZE(f1c100s_resets),
};
