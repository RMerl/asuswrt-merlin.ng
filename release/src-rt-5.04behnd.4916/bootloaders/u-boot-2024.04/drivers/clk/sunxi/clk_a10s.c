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
#include <dt-bindings/clock/sun5i-ccu.h>
#include <dt-bindings/reset/sun5i-ccu.h>
#include <linux/bitops.h>

static struct ccu_clk_gate a10s_gates[] = {
	[CLK_AHB_OTG]		= GATE(0x060, BIT(0)),
	[CLK_AHB_EHCI]		= GATE(0x060, BIT(1)),
	[CLK_AHB_OHCI]		= GATE(0x060, BIT(2)),
	[CLK_AHB_MMC0]		= GATE(0x060, BIT(8)),
	[CLK_AHB_MMC1]		= GATE(0x060, BIT(9)),
	[CLK_AHB_MMC2]		= GATE(0x060, BIT(10)),
	[CLK_AHB_NAND]		= GATE(0x060, BIT(13)),
	[CLK_AHB_EMAC]		= GATE(0x060, BIT(17)),
	[CLK_AHB_SPI0]		= GATE(0x060, BIT(20)),
	[CLK_AHB_SPI1]		= GATE(0x060, BIT(21)),
	[CLK_AHB_SPI2]		= GATE(0x060, BIT(22)),

	[CLK_APB0_PIO]		= GATE(0x068, BIT(5)),

	[CLK_APB1_I2C0]		= GATE(0x06c, BIT(0)),
	[CLK_APB1_I2C1]		= GATE(0x06c, BIT(1)),
	[CLK_APB1_I2C2]		= GATE(0x06c, BIT(2)),
	[CLK_APB1_UART0]	= GATE(0x06c, BIT(16)),
	[CLK_APB1_UART1]	= GATE(0x06c, BIT(17)),
	[CLK_APB1_UART2]	= GATE(0x06c, BIT(18)),
	[CLK_APB1_UART3]	= GATE(0x06c, BIT(19)),

	[CLK_NAND]		= GATE(0x080, BIT(31)),
	[CLK_SPI0]		= GATE(0x0a0, BIT(31)),
	[CLK_SPI1]		= GATE(0x0a4, BIT(31)),
	[CLK_SPI2]		= GATE(0x0a8, BIT(31)),

	[CLK_USB_OHCI]		= GATE(0x0cc, BIT(6)),
	[CLK_USB_PHY0]		= GATE(0x0cc, BIT(8)),
	[CLK_USB_PHY1]		= GATE(0x0cc, BIT(9)),
};

static struct ccu_reset a10s_resets[] = {
	[RST_USB_PHY0]		= RESET(0x0cc, BIT(0)),
	[RST_USB_PHY1]		= RESET(0x0cc, BIT(1)),
};

const struct ccu_desc a10s_ccu_desc = {
	.gates = a10s_gates,
	.resets = a10s_resets,
	.num_gates = ARRAY_SIZE(a10s_gates),
	.num_resets = ARRAY_SIZE(a10s_resets),
};
