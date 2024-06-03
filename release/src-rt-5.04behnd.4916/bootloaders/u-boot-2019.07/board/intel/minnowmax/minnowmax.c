// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015, Google, Inc
 */

#include <common.h>
#include <dm.h>
#include <asm/gpio.h>
#include <dm/device-internal.h>
#include <dm/uclass-internal.h>

#define GPIO_BANKE_NAME		"gpioe"

int misc_init_r(void)
{
	struct udevice *dev;
	struct gpio_desc desc;
	int ret;

	/*
	 * Turn on USB VBUS for the two USB ports on the board.
	 * Each port's VBUS is controlled by a GPIO pin.
	 */

	ret = uclass_find_device_by_name(UCLASS_GPIO, GPIO_BANKE_NAME, &dev);
	if (ret) {
		debug("%s: GPIO %s device cannot be not found (ret=%d)\n",
		      __func__, GPIO_BANKE_NAME, ret);
		return ret;
	}

	ret = device_probe(dev);
	if (ret) {
		debug("%s: GPIO %s device probe failed (ret=%d)\n",
		      __func__, GPIO_BANKE_NAME, ret);
		return ret;
	}

	desc.dev = dev;
	desc.flags = GPIOD_IS_OUT;

	/* GPIO E8 controls the bottom port */
	desc.offset = 8;

	ret = dm_gpio_request(&desc, "usb_host_en0");
	if (ret)
		return ret;
	dm_gpio_set_value(&desc, 1);

	/* GPIO E9 controls the upper port */
	desc.offset = 9;

	ret = dm_gpio_request(&desc, "usb_host_en1");
	if (ret)
		return ret;

	dm_gpio_set_value(&desc, 1);

	return 0;
}
