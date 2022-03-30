// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (C) 2018 Amarula Solutions.
 * Author: Jagan Teki <jagan@amarulasolutions.com>
 */

#include <common.h>
#include <clk-uclass.h>
#include <dm.h>
#include <errno.h>
#include <asm/arch/ccu.h>
#include <dt-bindings/clock/sun8i-h3-ccu.h>
#include <dt-bindings/reset/sun8i-h3-ccu.h>

static struct ccu_clk_gate h3_gates[] = {
	[CLK_BUS_MMC0]		= GATE(0x060, BIT(8)),
	[CLK_BUS_MMC1]		= GATE(0x060, BIT(9)),
	[CLK_BUS_MMC2]		= GATE(0x060, BIT(10)),
	[CLK_BUS_EMAC]		= GATE(0x060, BIT(17)),
	[CLK_BUS_SPI0]		= GATE(0x060, BIT(20)),
	[CLK_BUS_SPI1]		= GATE(0x060, BIT(21)),
	[CLK_BUS_OTG]		= GATE(0x060, BIT(23)),
	[CLK_BUS_EHCI0]		= GATE(0x060, BIT(24)),
	[CLK_BUS_EHCI1]		= GATE(0x060, BIT(25)),
	[CLK_BUS_EHCI2]		= GATE(0x060, BIT(26)),
	[CLK_BUS_EHCI3]		= GATE(0x060, BIT(27)),
	[CLK_BUS_OHCI0]		= GATE(0x060, BIT(28)),
	[CLK_BUS_OHCI1]		= GATE(0x060, BIT(29)),
	[CLK_BUS_OHCI2]		= GATE(0x060, BIT(30)),
	[CLK_BUS_OHCI3]		= GATE(0x060, BIT(31)),

	[CLK_BUS_UART0]		= GATE(0x06c, BIT(16)),
	[CLK_BUS_UART1]		= GATE(0x06c, BIT(17)),
	[CLK_BUS_UART2]		= GATE(0x06c, BIT(18)),
	[CLK_BUS_UART3]		= GATE(0x06c, BIT(19)),

	[CLK_BUS_EPHY]		= GATE(0x070, BIT(0)),

	[CLK_SPI0]		= GATE(0x0a0, BIT(31)),
	[CLK_SPI1]		= GATE(0x0a4, BIT(31)),

	[CLK_USB_PHY0]		= GATE(0x0cc, BIT(8)),
	[CLK_USB_PHY1]		= GATE(0x0cc, BIT(9)),
	[CLK_USB_PHY2]		= GATE(0x0cc, BIT(10)),
	[CLK_USB_PHY3]		= GATE(0x0cc, BIT(11)),
	[CLK_USB_OHCI0]		= GATE(0x0cc, BIT(16)),
	[CLK_USB_OHCI1]		= GATE(0x0cc, BIT(17)),
	[CLK_USB_OHCI2]		= GATE(0x0cc, BIT(18)),
	[CLK_USB_OHCI3]		= GATE(0x0cc, BIT(19)),
};

static struct ccu_reset h3_resets[] = {
	[RST_USB_PHY0]		= RESET(0x0cc, BIT(0)),
	[RST_USB_PHY1]		= RESET(0x0cc, BIT(1)),
	[RST_USB_PHY2]		= RESET(0x0cc, BIT(2)),
	[RST_USB_PHY3]		= RESET(0x0cc, BIT(3)),

	[RST_BUS_MMC0]		= RESET(0x2c0, BIT(8)),
	[RST_BUS_MMC1]		= RESET(0x2c0, BIT(9)),
	[RST_BUS_MMC2]		= RESET(0x2c0, BIT(10)),
	[RST_BUS_EMAC]		= RESET(0x2c0, BIT(17)),
	[RST_BUS_SPI0]		= RESET(0x2c0, BIT(20)),
	[RST_BUS_SPI1]		= RESET(0x2c0, BIT(21)),
	[RST_BUS_OTG]		= RESET(0x2c0, BIT(23)),
	[RST_BUS_EHCI0]		= RESET(0x2c0, BIT(24)),
	[RST_BUS_EHCI1]		= RESET(0x2c0, BIT(25)),
	[RST_BUS_EHCI2]		= RESET(0x2c0, BIT(26)),
	[RST_BUS_EHCI3]		= RESET(0x2c0, BIT(27)),
	[RST_BUS_OHCI0]		= RESET(0x2c0, BIT(28)),
	[RST_BUS_OHCI1]		= RESET(0x2c0, BIT(29)),
	[RST_BUS_OHCI2]		= RESET(0x2c0, BIT(30)),
	[RST_BUS_OHCI3]		= RESET(0x2c0, BIT(31)),

	[RST_BUS_EPHY]		= RESET(0x2c8, BIT(2)),

	[RST_BUS_UART0]		= RESET(0x2d8, BIT(16)),
	[RST_BUS_UART1]		= RESET(0x2d8, BIT(17)),
	[RST_BUS_UART2]		= RESET(0x2d8, BIT(18)),
	[RST_BUS_UART3]		= RESET(0x2d8, BIT(19)),
};

static const struct ccu_desc h3_ccu_desc = {
	.gates = h3_gates,
	.resets = h3_resets,
};

static int h3_clk_bind(struct udevice *dev)
{
	return sunxi_reset_bind(dev, ARRAY_SIZE(h3_resets));
}

static const struct udevice_id h3_ccu_ids[] = {
	{ .compatible = "allwinner,sun8i-h3-ccu",
	  .data = (ulong)&h3_ccu_desc },
	{ .compatible = "allwinner,sun50i-h5-ccu",
	  .data = (ulong)&h3_ccu_desc },
	{ }
};

U_BOOT_DRIVER(clk_sun8i_h3) = {
	.name		= "sun8i_h3_ccu",
	.id		= UCLASS_CLK,
	.of_match	= h3_ccu_ids,
	.priv_auto_alloc_size	= sizeof(struct ccu_priv),
	.ops		= &sunxi_clk_ops,
	.probe		= sunxi_clk_probe,
	.bind		= h3_clk_bind,
};
