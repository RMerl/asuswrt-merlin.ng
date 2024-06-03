// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2009
 * Marvell Semiconductor <www.marvell.com>
 * Written-by: Prafulla Wadaskar <prafulla@marvell.com>
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/cpu.h>
#include <asm/arch/soc.h>

/*
 * mvebu_config_gpio - GPIO configuration
 */
void mvebu_config_gpio(u32 gpp0_oe_val, u32 gpp1_oe_val,
		       u32 gpp0_oe, u32 gpp1_oe)
{
	struct kwgpio_registers *gpio0reg =
		(struct kwgpio_registers *)MVEBU_GPIO0_BASE;
	struct kwgpio_registers *gpio1reg =
		(struct kwgpio_registers *)MVEBU_GPIO1_BASE;

	/* Init GPIOS to default values as per board requirement */
	writel(gpp0_oe_val, &gpio0reg->dout);
	writel(gpp1_oe_val, &gpio1reg->dout);
	writel(gpp0_oe, &gpio0reg->oe);
	writel(gpp1_oe, &gpio1reg->oe);
}
