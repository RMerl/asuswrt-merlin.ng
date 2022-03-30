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
#include <dt-bindings/clock/sun8i-v3s-ccu.h>
#include <dt-bindings/reset/sun8i-v3s-ccu.h>

static struct ccu_clk_gate v3s_gates[] = {
	[CLK_BUS_MMC0]		= GATE(0x060, BIT(8)),
	[CLK_BUS_MMC1]		= GATE(0x060, BIT(9)),
	[CLK_BUS_MMC2]		= GATE(0x060, BIT(10)),
	[CLK_BUS_SPI0]		= GATE(0x060, BIT(20)),
	[CLK_BUS_OTG]		= GATE(0x060, BIT(24)),

	[CLK_BUS_UART0]		= GATE(0x06c, BIT(16)),
	[CLK_BUS_UART1]		= GATE(0x06c, BIT(17)),
	[CLK_BUS_UART2]		= GATE(0x06c, BIT(18)),

	[CLK_SPI0]		= GATE(0x0a0, BIT(31)),

	[CLK_USB_PHY0]          = GATE(0x0cc, BIT(8)),
};

static struct ccu_reset v3s_resets[] = {
	[RST_USB_PHY0]		= RESET(0x0cc, BIT(0)),

	[RST_BUS_MMC0]		= RESET(0x2c0, BIT(8)),
	[RST_BUS_MMC1]		= RESET(0x2c0, BIT(9)),
	[RST_BUS_MMC2]		= RESET(0x2c0, BIT(10)),
	[RST_BUS_SPI0]		= RESET(0x2c0, BIT(20)),
	[RST_BUS_OTG]		= RESET(0x2c0, BIT(24)),

	[RST_BUS_UART0]		= RESET(0x2d8, BIT(16)),
	[RST_BUS_UART1]		= RESET(0x2d8, BIT(17)),
	[RST_BUS_UART2]		= RESET(0x2d8, BIT(18)),
};

static const struct ccu_desc v3s_ccu_desc = {
	.gates = v3s_gates,
	.resets = v3s_resets,
};

static int v3s_clk_bind(struct udevice *dev)
{
	return sunxi_reset_bind(dev, ARRAY_SIZE(v3s_resets));
}

static const struct udevice_id v3s_clk_ids[] = {
	{ .compatible = "allwinner,sun8i-v3s-ccu",
	  .data = (ulong)&v3s_ccu_desc },
	{ }
};

U_BOOT_DRIVER(clk_sun8i_v3s) = {
	.name		= "sun8i_v3s_ccu",
	.id		= UCLASS_CLK,
	.of_match	= v3s_clk_ids,
	.priv_auto_alloc_size	= sizeof(struct ccu_priv),
	.ops		= &sunxi_clk_ops,
	.probe		= sunxi_clk_probe,
	.bind		= v3s_clk_bind,
};
