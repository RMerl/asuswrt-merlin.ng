/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2017 Allied Telesis Labs
 */

int gpio_hog_list(struct gpio_desc *gpiod, int max_count, const char *node_name,
		  const char *gpio_name, int value);

static inline int gpio_hog(struct gpio_desc *gpiod, const char *node_name,
			   const char *gpio_name, int value)
{
	return gpio_hog_list(gpiod, 1, node_name, gpio_name, value);
}
