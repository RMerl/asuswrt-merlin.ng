// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018 Amarula Solutions.
 * Author: Jagan Teki <jagan@amarulasolutions.com>
 */

#include <common.h>
#include <clk-uclass.h>
#include <dm.h>
#include <errno.h>
#include <clk/sunxi.h>
#include <dt-bindings/clock/sun9i-a80-ccu.h>
#include <dt-bindings/reset/sun9i-a80-ccu.h>
#include <linux/bitops.h>

static const struct ccu_clk_gate a80_gates[] = {
	[CLK_NAND0_0]		= GATE(0x400, BIT(31)),
	[CLK_NAND0_1]		= GATE(0x404, BIT(31)),
	[CLK_NAND1_0]		= GATE(0x408, BIT(31)),
	[CLK_NAND1_1]		= GATE(0x40c, BIT(31)),
	[CLK_SPI0]		= GATE(0x430, BIT(31)),
	[CLK_SPI1]		= GATE(0x434, BIT(31)),
	[CLK_SPI2]		= GATE(0x438, BIT(31)),
	[CLK_SPI3]		= GATE(0x43c, BIT(31)),

	[CLK_BUS_MMC]		= GATE(0x580, BIT(8)),
	[CLK_BUS_NAND0]		= GATE(0x580, BIT(13)),
	[CLK_BUS_NAND1]		= GATE(0x580, BIT(12)),
	[CLK_BUS_SPI0]		= GATE(0x580, BIT(20)),
	[CLK_BUS_SPI1]		= GATE(0x580, BIT(21)),
	[CLK_BUS_SPI2]		= GATE(0x580, BIT(22)),
	[CLK_BUS_SPI3]		= GATE(0x580, BIT(23)),

	[CLK_BUS_PIO]           = GATE(0x590, BIT(5)),

	[CLK_BUS_I2C0]		= GATE(0x594, BIT(0)),
	[CLK_BUS_I2C1]		= GATE(0x594, BIT(1)),
	[CLK_BUS_I2C2]		= GATE(0x594, BIT(2)),
	[CLK_BUS_I2C3]		= GATE(0x594, BIT(3)),
	[CLK_BUS_I2C4]		= GATE(0x594, BIT(4)),
	[CLK_BUS_UART0]		= GATE(0x594, BIT(16)),
	[CLK_BUS_UART1]		= GATE(0x594, BIT(17)),
	[CLK_BUS_UART2]		= GATE(0x594, BIT(18)),
	[CLK_BUS_UART3]		= GATE(0x594, BIT(19)),
	[CLK_BUS_UART4]		= GATE(0x594, BIT(20)),
	[CLK_BUS_UART5]		= GATE(0x594, BIT(21)),
};

static const struct ccu_reset a80_resets[] = {
	[RST_BUS_MMC]		= RESET(0x5a0, BIT(8)),
	[RST_BUS_NAND0]		= RESET(0x5a0, BIT(13)),
	[RST_BUS_NAND1]		= RESET(0x5a0, BIT(12)),
	[RST_BUS_SPI0]		= RESET(0x5a0, BIT(20)),
	[RST_BUS_SPI1]		= RESET(0x5a0, BIT(21)),
	[RST_BUS_SPI2]		= RESET(0x5a0, BIT(22)),
	[RST_BUS_SPI3]		= RESET(0x5a0, BIT(23)),

	[RST_BUS_I2C0]		= RESET(0x5b4, BIT(0)),
	[RST_BUS_I2C1]		= RESET(0x5b4, BIT(1)),
	[RST_BUS_I2C2]		= RESET(0x5b4, BIT(2)),
	[RST_BUS_I2C3]		= RESET(0x5b4, BIT(3)),
	[RST_BUS_I2C4]		= RESET(0x5b4, BIT(4)),
	[RST_BUS_UART0]		= RESET(0x5b4, BIT(16)),
	[RST_BUS_UART1]		= RESET(0x5b4, BIT(17)),
	[RST_BUS_UART2]		= RESET(0x5b4, BIT(18)),
	[RST_BUS_UART3]		= RESET(0x5b4, BIT(19)),
	[RST_BUS_UART4]		= RESET(0x5b4, BIT(20)),
	[RST_BUS_UART5]		= RESET(0x5b4, BIT(21)),
};

static const struct ccu_clk_gate a80_mmc_gates[] = {
	[0]			= GATE(0x0, BIT(16)),
	[1]			= GATE(0x4, BIT(16)),
	[2]			= GATE(0x8, BIT(16)),
	[3]			= GATE(0xc, BIT(16)),
};

static const struct ccu_reset a80_mmc_resets[] = {
	[0]			= GATE(0x0, BIT(18)),
	[1]			= GATE(0x4, BIT(18)),
	[2]			= GATE(0x8, BIT(18)),
	[3]			= GATE(0xc, BIT(18)),
};

const struct ccu_desc a80_ccu_desc = {
	.gates = a80_gates,
	.resets = a80_resets,
	.num_gates = ARRAY_SIZE(a80_gates),
	.num_resets = ARRAY_SIZE(a80_resets),
};

const struct ccu_desc a80_mmc_clk_desc = {
	.gates = a80_mmc_gates,
	.resets = a80_mmc_resets,
	.num_gates = ARRAY_SIZE(a80_mmc_gates),
	.num_resets = ARRAY_SIZE(a80_mmc_resets),
};
