// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (C) Samuel Holland <samuel@sholland.org>
 */

#include <clk-uclass.h>
#include <dm.h>
#include <clk/sunxi.h>
#include <dt-bindings/clock/sun50i-h6-r-ccu.h>
#include <dt-bindings/reset/sun50i-h6-r-ccu.h>
#include <linux/bitops.h>

static struct ccu_clk_gate h6_r_gates[] = {
	[CLK_R_APB1]		= GATE_DUMMY,

	[CLK_R_APB1_TIMER]	= GATE(0x11c, BIT(0)),
	[CLK_R_APB1_TWD]	= GATE(0x12c, BIT(0)),
	[CLK_R_APB1_PWM]	= GATE(0x13c, BIT(0)),
	[CLK_R_APB2_UART]	= GATE(0x18c, BIT(0)),
	[CLK_R_APB2_I2C]	= GATE(0x19c, BIT(0)),
	[CLK_R_APB2_RSB]	= GATE(0x1bc, BIT(0)),
	[CLK_R_APB1_IR]		= GATE(0x1cc, BIT(0)),
	[CLK_R_APB1_W1]		= GATE(0x1ec, BIT(0)),
};

static struct ccu_reset h6_r_resets[] = {
	[RST_R_APB1_TIMER]	= RESET(0x11c, BIT(16)),
	[RST_R_APB1_TWD]	= RESET(0x12c, BIT(16)),
	[RST_R_APB1_PWM]	= RESET(0x13c, BIT(16)),
	[RST_R_APB2_UART]	= RESET(0x18c, BIT(16)),
	[RST_R_APB2_I2C]	= RESET(0x19c, BIT(16)),
	[RST_R_APB2_RSB]	= RESET(0x1bc, BIT(16)),
	[RST_R_APB1_IR]		= RESET(0x1cc, BIT(16)),
	[RST_R_APB1_W1]		= RESET(0x1ec, BIT(16)),
};

const struct ccu_desc h6_r_ccu_desc = {
	.gates = h6_r_gates,
	.resets = h6_r_resets,
	.num_gates = ARRAY_SIZE(h6_r_gates),
	.num_resets = ARRAY_SIZE(h6_r_resets),
};
