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
#include <dt-bindings/clock/sun50i-h6-ccu.h>
#include <dt-bindings/reset/sun50i-h6-ccu.h>

static struct ccu_clk_gate h6_gates[] = {
	[CLK_BUS_MMC0]		= GATE(0x84c, BIT(0)),
	[CLK_BUS_MMC1]		= GATE(0x84c, BIT(1)),
	[CLK_BUS_MMC2]		= GATE(0x84c, BIT(2)),
	[CLK_BUS_UART0]		= GATE(0x90c, BIT(0)),
	[CLK_BUS_UART1]		= GATE(0x90c, BIT(1)),
	[CLK_BUS_UART2]		= GATE(0x90c, BIT(2)),
	[CLK_BUS_UART3]		= GATE(0x90c, BIT(3)),

	[CLK_SPI0]		= GATE(0x940, BIT(31)),
	[CLK_SPI1]		= GATE(0x944, BIT(31)),

	[CLK_BUS_SPI0]		= GATE(0x96c, BIT(0)),
	[CLK_BUS_SPI1]		= GATE(0x96c, BIT(1)),

	[CLK_BUS_EMAC]		= GATE(0x97c, BIT(0)),
};

static struct ccu_reset h6_resets[] = {
	[RST_BUS_MMC0]		= RESET(0x84c, BIT(16)),
	[RST_BUS_MMC1]		= RESET(0x84c, BIT(17)),
	[RST_BUS_MMC2]		= RESET(0x84c, BIT(18)),
	[RST_BUS_UART0]		= RESET(0x90c, BIT(16)),
	[RST_BUS_UART1]		= RESET(0x90c, BIT(17)),
	[RST_BUS_UART2]		= RESET(0x90c, BIT(18)),
	[RST_BUS_UART3]		= RESET(0x90c, BIT(19)),

	[RST_BUS_SPI0]		= RESET(0x96c, BIT(16)),
	[RST_BUS_SPI1]		= RESET(0x96c, BIT(17)),

	[RST_BUS_EMAC]		= RESET(0x97c, BIT(16)),
};

static const struct ccu_desc h6_ccu_desc = {
	.gates = h6_gates,
	.resets = h6_resets,
};

static int h6_clk_bind(struct udevice *dev)
{
	return sunxi_reset_bind(dev, ARRAY_SIZE(h6_resets));
}

static const struct udevice_id h6_ccu_ids[] = {
	{ .compatible = "allwinner,sun50i-h6-ccu",
	  .data = (ulong)&h6_ccu_desc },
	{ }
};

U_BOOT_DRIVER(clk_sun50i_h6) = {
	.name		= "sun50i_h6_ccu",
	.id		= UCLASS_CLK,
	.of_match	= h6_ccu_ids,
	.priv_auto_alloc_size	= sizeof(struct ccu_priv),
	.ops		= &sunxi_clk_ops,
	.probe		= sunxi_clk_probe,
	.bind		= h6_clk_bind,
};
