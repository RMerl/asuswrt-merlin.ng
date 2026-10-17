// SPDX-License-Identifier: GPL-2.0+
/*
 * (C)Copyright 2016 Rockchip Electronics Co., Ltd
 * Authors: Andy Yan <andy.yan@rock-chips.com>
 */

#include <common.h>
#include <init.h>
#include <syscon.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/arch-rockchip/clock.h>
#include <asm/arch-rockchip/grf_rv1108.h>
#include <asm/arch-rockchip/hardware.h>
#include <asm/gpio.h>

DECLARE_GLOBAL_DATA_PTR;

int board_early_init_f(void)
{
	struct rv1108_grf *grf;
	enum {
		GPIO3C3_SHIFT           = 6,
		GPIO3C3_MASK            = 3 << GPIO3C3_SHIFT,

		GPIO3C2_SHIFT           = 4,
		GPIO3C2_MASK            = 3 << GPIO3C2_SHIFT,

		GPIO2D2_SHIFT		= 4,
		GPIO2D2_MASK		= 3 << GPIO2D2_SHIFT,
		GPIO2D2_GPIO            = 0,
		GPIO2D2_UART2_SOUT_M0,

		GPIO2D1_SHIFT		= 2,
		GPIO2D1_MASK		= 3 << GPIO2D1_SHIFT,
		GPIO2D1_GPIO            = 0,
		GPIO2D1_UART2_SIN_M0,
	};

	grf = syscon_get_first_range(ROCKCHIP_SYSCON_GRF);

	/* Elgin board use UART2 m0 for debug*/
	rk_clrsetreg(&grf->gpio2d_iomux,
		     GPIO2D2_MASK | GPIO2D1_MASK,
		     GPIO2D2_UART2_SOUT_M0 << GPIO2D2_SHIFT |
		     GPIO2D1_UART2_SIN_M0 << GPIO2D1_SHIFT);
	rk_clrreg(&grf->gpio3c_iomux, GPIO3C3_MASK | GPIO3C2_MASK);

	return 0;
}

#define MODEM_ENABLE_GPIO 111

int rk_board_late_init(void)
{
	gpio_request(MODEM_ENABLE_GPIO, "modem_enable");
	gpio_direction_output(MODEM_ENABLE_GPIO, 0);

	return 0;
}

int dram_init(void)
{
	gd->ram_size = 0x8000000;

	return 0;
}

int dram_init_banksize(void)
{
	gd->bd->bi_dram[0].start = 0x60000000;
	gd->bd->bi_dram[0].size = 0x8000000;

	return 0;
}
