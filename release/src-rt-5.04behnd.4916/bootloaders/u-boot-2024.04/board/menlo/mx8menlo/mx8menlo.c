// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2021-2022 Marek Vasut <marex@denx.de>
 */

#include <common.h>
#include <asm/arch/clock.h>
#include <asm/arch/imx8mm_pins.h>
#include <asm/arch/sys_proto.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/mach-imx/iomux-v3.h>
#include <spl.h>

void board_early_init(void)
{
	init_uart_clk(0);
}
