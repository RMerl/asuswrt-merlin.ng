// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (C) 2018 Amarula Solutions B.V.
 * Author: Jagan Teki <jagan@amarulasolutions.com>
 */

#include <common.h>
#include <clk-uclass.h>
#include <dm.h>
#include <errno.h>
#include <clk/sunxi.h>
#include <dt-bindings/clock/sun6i-a31-ccu.h>
#include <dt-bindings/reset/sun6i-a31-ccu.h>
#include <linux/bitops.h>

static struct ccu_clk_gate a31_gates[] = {
	[CLK_AHB1_MMC0]		= GATE(0x060, BIT(8)),
	[CLK_AHB1_MMC1]		= GATE(0x060, BIT(9)),
	[CLK_AHB1_MMC2]		= GATE(0x060, BIT(10)),
	[CLK_AHB1_MMC3]		= GATE(0x060, BIT(11)),
	[CLK_AHB1_NAND1]	= GATE(0x060, BIT(12)),
	[CLK_AHB1_NAND0]	= GATE(0x060, BIT(13)),
	[CLK_AHB1_EMAC]		= GATE(0x060, BIT(17)),
	[CLK_AHB1_SPI0]		= GATE(0x060, BIT(20)),
	[CLK_AHB1_SPI1]		= GATE(0x060, BIT(21)),
	[CLK_AHB1_SPI2]		= GATE(0x060, BIT(22)),
	[CLK_AHB1_SPI3]		= GATE(0x060, BIT(23)),
	[CLK_AHB1_OTG]		= GATE(0x060, BIT(24)),
	[CLK_AHB1_EHCI0]	= GATE(0x060, BIT(26)),
	[CLK_AHB1_EHCI1]	= GATE(0x060, BIT(27)),
	[CLK_AHB1_OHCI0]	= GATE(0x060, BIT(29)),
	[CLK_AHB1_OHCI1]	= GATE(0x060, BIT(30)),
	[CLK_AHB1_OHCI2]	= GATE(0x060, BIT(31)),

	[CLK_APB1_PIO]		= GATE(0x068, BIT(5)),

	[CLK_APB2_I2C0]		= GATE(0x06c, BIT(0)),
	[CLK_APB2_I2C1]		= GATE(0x06c, BIT(1)),
	[CLK_APB2_I2C2]		= GATE(0x06c, BIT(2)),
	[CLK_APB2_I2C3]		= GATE(0x06c, BIT(3)),
	[CLK_APB2_UART0]	= GATE(0x06c, BIT(16)),
	[CLK_APB2_UART1]	= GATE(0x06c, BIT(17)),
	[CLK_APB2_UART2]	= GATE(0x06c, BIT(18)),
	[CLK_APB2_UART3]	= GATE(0x06c, BIT(19)),
	[CLK_APB2_UART4]	= GATE(0x06c, BIT(20)),
	[CLK_APB2_UART5]	= GATE(0x06c, BIT(21)),

	[CLK_NAND0]		= GATE(0x080, BIT(31)),
	[CLK_NAND1]		= GATE(0x084, BIT(31)),
	[CLK_SPI0]		= GATE(0x0a0, BIT(31)),
	[CLK_SPI1]		= GATE(0x0a4, BIT(31)),
	[CLK_SPI2]		= GATE(0x0a8, BIT(31)),
	[CLK_SPI3]		= GATE(0x0ac, BIT(31)),

	[CLK_USB_PHY0]		= GATE(0x0cc, BIT(8)),
	[CLK_USB_PHY1]		= GATE(0x0cc, BIT(9)),
	[CLK_USB_PHY2]		= GATE(0x0cc, BIT(10)),
	[CLK_USB_OHCI0]		= GATE(0x0cc, BIT(16)),
	[CLK_USB_OHCI1]		= GATE(0x0cc, BIT(17)),
	[CLK_USB_OHCI2]		= GATE(0x0cc, BIT(18)),
};

static struct ccu_reset a31_resets[] = {
	[RST_USB_PHY0]		= RESET(0x0cc, BIT(0)),
	[RST_USB_PHY1]		= RESET(0x0cc, BIT(1)),
	[RST_USB_PHY2]		= RESET(0x0cc, BIT(2)),

	[RST_AHB1_MMC0]		= RESET(0x2c0, BIT(8)),
	[RST_AHB1_MMC1]		= RESET(0x2c0, BIT(9)),
	[RST_AHB1_MMC2]		= RESET(0x2c0, BIT(10)),
	[RST_AHB1_MMC3]		= RESET(0x2c0, BIT(11)),
	[RST_AHB1_NAND1]	= RESET(0x2c0, BIT(12)),
	[RST_AHB1_NAND0]	= RESET(0x2c0, BIT(13)),
	[RST_AHB1_EMAC]		= RESET(0x2c0, BIT(17)),
	[RST_AHB1_SPI0]		= RESET(0x2c0, BIT(20)),
	[RST_AHB1_SPI1]		= RESET(0x2c0, BIT(21)),
	[RST_AHB1_SPI2]		= RESET(0x2c0, BIT(22)),
	[RST_AHB1_SPI3]		= RESET(0x2c0, BIT(23)),
	[RST_AHB1_OTG]		= RESET(0x2c0, BIT(24)),
	[RST_AHB1_EHCI0]	= RESET(0x2c0, BIT(26)),
	[RST_AHB1_EHCI1]	= RESET(0x2c0, BIT(27)),
	[RST_AHB1_OHCI0]	= RESET(0x2c0, BIT(29)),
	[RST_AHB1_OHCI1]	= RESET(0x2c0, BIT(30)),
	[RST_AHB1_OHCI2]	= RESET(0x2c0, BIT(31)),

	[RST_APB2_I2C0]		= RESET(0x2d8, BIT(0)),
	[RST_APB2_I2C1]		= RESET(0x2d8, BIT(1)),
	[RST_APB2_I2C2]		= RESET(0x2d8, BIT(2)),
	[RST_APB2_I2C3]		= RESET(0x2d8, BIT(3)),
	[RST_APB2_UART0]	= RESET(0x2d8, BIT(16)),
	[RST_APB2_UART1]	= RESET(0x2d8, BIT(17)),
	[RST_APB2_UART2]	= RESET(0x2d8, BIT(18)),
	[RST_APB2_UART3]	= RESET(0x2d8, BIT(19)),
	[RST_APB2_UART4]	= RESET(0x2d8, BIT(20)),
	[RST_APB2_UART5]	= RESET(0x2d8, BIT(21)),
};

const struct ccu_desc a31_ccu_desc = {
	.gates = a31_gates,
	.resets = a31_resets,
	.num_gates = ARRAY_SIZE(a31_gates),
	.num_resets = ARRAY_SIZE(a31_resets),
};
