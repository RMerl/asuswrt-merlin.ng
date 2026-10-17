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
#include <dt-bindings/clock/sun50i-h6-ccu.h>
#include <dt-bindings/reset/sun50i-h6-ccu.h>
#include <linux/bitops.h>

static struct ccu_clk_gate h6_gates[] = {
	[CLK_PLL_PERIPH0]	= GATE(0x020, BIT(31)),

	[CLK_APB1]		= GATE_DUMMY,

	[CLK_DE]		= GATE(0x600, BIT(31)),
	[CLK_BUS_DE]		= GATE(0x60c, BIT(0)),

	[CLK_NAND0]		= GATE(0x810, BIT(31)),
	[CLK_NAND1]		= GATE(0x814, BIT(31)),
	[CLK_BUS_NAND]		= GATE(0x82c, BIT(0)),

	[CLK_BUS_MMC0]		= GATE(0x84c, BIT(0)),
	[CLK_BUS_MMC1]		= GATE(0x84c, BIT(1)),
	[CLK_BUS_MMC2]		= GATE(0x84c, BIT(2)),
	[CLK_BUS_UART0]		= GATE(0x90c, BIT(0)),
	[CLK_BUS_UART1]		= GATE(0x90c, BIT(1)),
	[CLK_BUS_UART2]		= GATE(0x90c, BIT(2)),
	[CLK_BUS_UART3]		= GATE(0x90c, BIT(3)),

	[CLK_BUS_I2C0]		= GATE(0x91c, BIT(0)),
	[CLK_BUS_I2C1]		= GATE(0x91c, BIT(1)),
	[CLK_BUS_I2C2]		= GATE(0x91c, BIT(2)),
	[CLK_BUS_I2C3]		= GATE(0x91c, BIT(3)),

	[CLK_SPI0]		= GATE(0x940, BIT(31)),
	[CLK_SPI1]		= GATE(0x944, BIT(31)),

	[CLK_BUS_SPI0]		= GATE(0x96c, BIT(0)),
	[CLK_BUS_SPI1]		= GATE(0x96c, BIT(1)),

	[CLK_BUS_EMAC]		= GATE(0x97c, BIT(0)),

	[CLK_USB_PHY0]		= GATE(0xa70, BIT(29)),
	[CLK_USB_OHCI0]		= GATE(0xa70, BIT(31)),

	[CLK_USB_PHY1]		= GATE(0xa74, BIT(29)),

	[CLK_USB_HSIC]		= GATE(0xa7c, BIT(26)),
	[CLK_USB_HSIC_12M]	= GATE(0xa7c, BIT(27)),
	[CLK_USB_PHY3]		= GATE(0xa7c, BIT(29)),
	[CLK_USB_OHCI3]		= GATE(0xa7c, BIT(31)),

	[CLK_BUS_OHCI0]		= GATE(0xa8c, BIT(0)),
	[CLK_BUS_OHCI3]		= GATE(0xa8c, BIT(3)),
	[CLK_BUS_EHCI0]		= GATE(0xa8c, BIT(4)),
	[CLK_BUS_XHCI]		= GATE(0xa8c, BIT(5)),
	[CLK_BUS_EHCI3]		= GATE(0xa8c, BIT(7)),
	[CLK_BUS_OTG]		= GATE(0xa8c, BIT(8)),

	[CLK_HDMI]		= GATE(0xb00, BIT(31)),
	[CLK_HDMI_SLOW]		= GATE(0xb04, BIT(31)),
	[CLK_HDMI_CEC]		= GATE(0xb10, BIT(31)),
	[CLK_BUS_HDMI]		= GATE(0xb1c, BIT(0)),
	[CLK_BUS_TCON_TOP]	= GATE(0xb5c, BIT(0)),
	[CLK_TCON_LCD0]		= GATE(0xb60, BIT(31)),
	[CLK_BUS_TCON_LCD0]	= GATE(0xb7c, BIT(0)),
	[CLK_TCON_TV0]		= GATE(0xb80, BIT(31)),
	[CLK_BUS_TCON_TV0]	= GATE(0xb9c, BIT(0)),
};

static struct ccu_reset h6_resets[] = {
	[RST_BUS_DE]		= RESET(0x60c, BIT(16)),
	[RST_BUS_NAND]		= RESET(0x82c, BIT(16)),

	[RST_BUS_MMC0]		= RESET(0x84c, BIT(16)),
	[RST_BUS_MMC1]		= RESET(0x84c, BIT(17)),
	[RST_BUS_MMC2]		= RESET(0x84c, BIT(18)),
	[RST_BUS_UART0]		= RESET(0x90c, BIT(16)),
	[RST_BUS_UART1]		= RESET(0x90c, BIT(17)),
	[RST_BUS_UART2]		= RESET(0x90c, BIT(18)),
	[RST_BUS_UART3]		= RESET(0x90c, BIT(19)),

	[RST_BUS_I2C0]		= RESET(0x91c, BIT(16)),
	[RST_BUS_I2C1]		= RESET(0x91c, BIT(17)),
	[RST_BUS_I2C2]		= RESET(0x91c, BIT(18)),
	[RST_BUS_I2C3]		= RESET(0x91c, BIT(19)),

	[RST_BUS_SPI0]		= RESET(0x96c, BIT(16)),
	[RST_BUS_SPI1]		= RESET(0x96c, BIT(17)),

	[RST_BUS_EMAC]		= RESET(0x97c, BIT(16)),

	[RST_USB_PHY0]		= RESET(0xa70, BIT(30)),

	[RST_USB_PHY1]		= RESET(0xa74, BIT(30)),

	[RST_USB_HSIC]		= RESET(0xa7c, BIT(28)),
	[RST_USB_PHY3]		= RESET(0xa7c, BIT(30)),

	[RST_BUS_OHCI0]		= RESET(0xa8c, BIT(16)),
	[RST_BUS_OHCI3]		= RESET(0xa8c, BIT(19)),
	[RST_BUS_EHCI0]		= RESET(0xa8c, BIT(20)),
	[RST_BUS_XHCI]		= RESET(0xa8c, BIT(21)),
	[RST_BUS_EHCI3]		= RESET(0xa8c, BIT(23)),
	[RST_BUS_OTG]		= RESET(0xa8c, BIT(24)),

	[RST_BUS_HDMI]		= RESET(0xb1c, BIT(16)),
	[RST_BUS_HDMI_SUB]	= RESET(0xb1c, BIT(17)),
	[RST_BUS_TCON_TOP]	= RESET(0xb5c, BIT(16)),
	[RST_BUS_TCON_LCD0]	= RESET(0xb7c, BIT(16)),
	[RST_BUS_TCON_TV0]	= RESET(0xb9c, BIT(16)),
};

const struct ccu_desc h6_ccu_desc = {
	.gates = h6_gates,
	.resets = h6_resets,
	.num_gates = ARRAY_SIZE(h6_gates),
	.num_resets = ARRAY_SIZE(h6_resets),
};
