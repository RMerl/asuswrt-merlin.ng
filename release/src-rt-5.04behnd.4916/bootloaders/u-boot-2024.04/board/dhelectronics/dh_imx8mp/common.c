// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2022 Marek Vasut <marex@denx.de>
 */

#include <common.h>
#include <asm/io.h>
#include <asm-generic/gpio.h>

#include "lpddr4_timing.h"

DECLARE_GLOBAL_DATA_PTR;

u8 dh_get_memcfg(void)
{
	struct gpio_desc gpio[4];
	u8 memcfg = 0;
	ofnode node;
	int i, ret;

	node = ofnode_path("/config");
	if (!ofnode_valid(node)) {
		printf("%s: no /config node?\n", __func__);
		return BIT(2) | BIT(0);
	}

	ret = gpio_request_list_by_name_nodev(node,
					      "dh,ram-coding-gpios",
					      gpio, ARRAY_SIZE(gpio),
					      GPIOD_IS_IN);
	for (i = 0; i < ret; i++)
		memcfg |= !!dm_gpio_get_value(&(gpio[i])) << i;

	gpio_free_list_nodev(gpio, ret);

	return memcfg;
}
