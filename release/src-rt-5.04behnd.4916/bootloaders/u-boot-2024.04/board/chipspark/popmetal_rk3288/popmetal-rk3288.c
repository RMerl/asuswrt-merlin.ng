// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2016 Rockchip Electronics Co., Ltd
 */

#include <common.h>
#include <init.h>
#include <asm/gpio.h>

#define GPIO7A3_HUB_RST	227

int rk3288_board_late_init(void)
{
	int ret;

	ret = gpio_request(GPIO7A3_HUB_RST, "hub_rst");
	if (ret)
		return ret;
	ret = gpio_direction_output(GPIO7A3_HUB_RST, 1);
	if (ret)
		return ret;

	return 0;
}
