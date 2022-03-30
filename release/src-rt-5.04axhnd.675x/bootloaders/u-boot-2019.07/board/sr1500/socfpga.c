// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015 Stefan Roese <sr@denx.de>
 */

#include <common.h>
#include <asm/arch/reset_manager.h>
#include <asm/gpio.h>
#include <asm/io.h>

int board_early_init_f(void)
{
	int ret;

	/* Reset the Marvell PHY 88E1510 */
	ret = gpio_request(63, "PHY reset");
	if (ret)
		return ret;

	gpio_direction_output(63, 0);
	mdelay(1);
	gpio_set_value(63, 1);
	mdelay(10);

	return 0;
}
