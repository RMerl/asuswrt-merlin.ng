// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018 Allied Telesis Labs
 */

#include <common.h>
#include <dm.h>
#include <asm/global_data.h>
#include <asm/gpio.h>

DECLARE_GLOBAL_DATA_PTR;

int gpio_hog_list(struct gpio_desc *gpiod, int max_count,
		  const char *node_name, const char *gpio_name, int value)
{
	int node;
	int count;
	int i;

	node = fdt_node_offset_by_compatible(gd->fdt_blob, 0, node_name);
	if (node < 0)
		return -ENODEV;

	if (!dm_gpio_is_valid(gpiod)) {
		count =
		    gpio_request_list_by_name_nodev(offset_to_ofnode(node),
						    gpio_name, gpiod, max_count,
						    GPIOD_IS_OUT);
		if (count < 0)
			return count;

		for (i = 0; i < count; i++)
			dm_gpio_set_value(&gpiod[i], value);
	}

	return 0;
}
