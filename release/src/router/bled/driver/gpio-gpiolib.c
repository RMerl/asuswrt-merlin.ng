/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 *
 * Copyright 2014, ASUSTeK Inc.
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND ASUS GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 */

#include <asm/gpio.h>
#include "../bled_defs.h"
#include "gpio_api.h"
#ifdef CONFIG_4GAC53U
#include <linux/list.h>
#include <linux/leds.h>

extern struct list_head leds_list;
#endif

/* __gpio_set_value() and __gpio_get_value() are defined and exported
 * by drivers/gpio/gpiolib.c
 */
static void gpio_set(int gpio_nr, int value)
{
#ifdef CONFIG_4GAC53U
	struct led_classdev *led_cdev = NULL;
	char buf[8];

	snprintf(buf, sizeof(buf), "led%d", gpio_nr);

	list_for_each_entry(led_cdev, &leds_list, node) {
		if (!strcmp(led_cdev->name, buf)) {
			led_set_brightness(led_cdev, value);
			return;
		}
	}
#else
	__gpio_set_value(gpio_nr, value);
#endif
}

static int gpio_get(int gpio_nr)
{
#ifdef CONFIG_4GAC53U
	struct led_classdev *led_cdev = NULL;
	char buf[8];
	int brightness = 0;

	snprintf(buf, sizeof(buf), "led%d", gpio_nr);

	list_for_each_entry(led_cdev, &leds_list, node) {
		if (!strcmp(led_cdev->name, buf)) {
			if (led_cdev->brightness > 0)
				brightness = 1;
			break;
		}
	}

	return brightness;
#else
	return __gpio_get_value(gpio_nr);
#endif
}

struct gpio_api_s gpio_api_tbl[GPIO_API_MAX] = {
	[GPIO_API_PLATFORM] =
	{
		.gpio_set = gpio_set,
		.gpio_get = gpio_get
	},
};
