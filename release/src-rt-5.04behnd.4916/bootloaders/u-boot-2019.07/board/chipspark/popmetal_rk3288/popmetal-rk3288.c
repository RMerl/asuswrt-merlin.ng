// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2016 Rockchip Electronics Co., Ltd
 */

#include <common.h>
#include <spl.h>
#include <asm/gpio.h>

void board_boot_order(u32 *spl_boot_list)
{
	/* eMMC prior to sdcard */
	spl_boot_list[0] = BOOT_DEVICE_MMC2;
	spl_boot_list[1] = BOOT_DEVICE_MMC1;
}

#define GPIO7A3_HUB_RST	227

int rk_board_late_init(void)
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
