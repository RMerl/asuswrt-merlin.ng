// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (C) 2018 Amarula Solutions B.V.
 * Author: Jagan Teki <jagan@amarulasolutions.com>
 */

#include <common.h>
#include <clk-uclass.h>
#include <dm.h>
#include <errno.h>
#include <asm/arch/ccu.h>
#include <dt-bindings/clock/sun8i-a23-a33-ccu.h>
#include <dt-bindings/reset/sun8i-a23-a33-ccu.h>

static struct ccu_clk_gate a23_gates[] = {
	[CLK_BUS_MMC0]		= GATE(0x060, BIT(8)),
	[CLK_BUS_MMC1]		= GATE(0x060, BIT(9)),
	[CLK_BUS_MMC2]		= GATE(0x060, BIT(10)),
	[CLK_BUS_SPI0]		= GATE(0x060, BIT(20)),
	[CLK_BUS_SPI1]		= GATE(0x060, BIT(21)),
	[CLK_BUS_OTG]		= GATE(0x060, BIT(24)),
	[CLK_BUS_EHCI]		= GATE(0x060, BIT(26)),
	[CLK_BUS_OHCI]		= GATE(0x060, BIT(29)),

	[CLK_BUS_UART0]		= GATE(0x06c, BIT(16)),
	[CLK_BUS_UART1]		= GATE(0x06c, BIT(17)),
	[CLK_BUS_UART2]		= GATE(0x06c, BIT(18)),
	[CLK_BUS_UART3]		= GATE(0x06c, BIT(19)),
	[CLK_BUS_UART4]		= GATE(0x06c, BIT(20)),

	[CLK_SPI0]		= GATE(0x0a0, BIT(31)),
	[CLK_SPI1]		= GATE(0x0a4, BIT(31)),

	[CLK_USB_PHY0]		= GATE(0x0cc, BIT(8)),
	[CLK_USB_PHY1]		= GATE(0x0cc, BIT(9)),
	[CLK_USB_HSIC]		= GATE(0x0cc, BIT(10)),
	[CLK_USB_HSIC_12M]	= GATE(0x0cc, BIT(11)),
	[CLK_USB_OHCI]		= GATE(0x0cc, BIT(16)),
};

static struct ccu_reset a23_resets[] = {
	[RST_USB_PHY0]		= RESET(0x0cc, BIT(0)),
	[RST_USB_PHY1]		= RESET(0x0cc, BIT(1)),
	[RST_USB_HSIC]		= RESET(0x0cc, BIT(2)),

	[RST_BUS_MMC0]		= RESET(0x2c0, BIT(8)),
	[RST_BUS_MMC1]		= RESET(0x2c0, BIT(9)),
	[RST_BUS_MMC2]		= RESET(0x2c0, BIT(10)),
	[RST_BUS_SPI0]		= RESET(0x2c0, BIT(20)),
	[RST_BUS_SPI1]		= RESET(0x2c0, BIT(21)),
	[RST_BUS_OTG]		= RESET(0x2c0, BIT(24)),
	[RST_BUS_EHCI]		= RESET(0x2c0, BIT(26)),
	[RST_BUS_OHCI]		= RESET(0x2c0, BIT(29)),

	[RST_BUS_UART0]		= RESET(0x2d8, BIT(16)),
	[RST_BUS_UART1]		= RESET(0x2d8, BIT(17)),
	[RST_BUS_UART2]		= RESET(0x2d8, BIT(18)),
	[RST_BUS_UART3]		= RESET(0x2d8, BIT(19)),
	[RST_BUS_UART4]		= RESET(0x2d8, BIT(20)),
};

static const struct ccu_desc a23_ccu_desc = {
	.gates = a23_gates,
	.resets = a23_resets,
};

static int a23_clk_bind(struct udevice *dev)
{
	return sunxi_reset_bind(dev, ARRAY_SIZE(a23_resets));
}

static const struct udevice_id a23_clk_ids[] = {
	{ .compatible = "allwinner,sun8i-a23-ccu",
	  .data = (ulong)&a23_ccu_desc },
	{ .compatible = "allwinner,sun8i-a33-ccu",
	  .data = (ulong)&a23_ccu_desc },
	{ }
};

U_BOOT_DRIVER(clk_sun8i_a23) = {
	.name		= "sun8i_a23_ccu",
	.id		= UCLASS_CLK,
	.of_match	= a23_clk_ids,
	.priv_auto_alloc_size	= sizeof(struct ccu_priv),
	.ops		= &sunxi_clk_ops,
	.probe		= sunxi_clk_probe,
	.bind		= a23_clk_bind,
};
