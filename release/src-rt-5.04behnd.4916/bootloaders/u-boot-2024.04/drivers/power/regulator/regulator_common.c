// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2019 Disruptive Technologies Research AS
 * Sven Schwermer <sven.svenschwermer@disruptive-technologies.com>
 */

#include <common.h>
#include <dm.h>
#include <log.h>
#include <asm/gpio.h>
#include <linux/delay.h>
#include <power/regulator.h>
#include "regulator_common.h"

#include "regulator_common.h"

int regulator_common_of_to_plat(struct udevice *dev,
				struct regulator_common_plat *plat,
				const char *enable_gpio_name)
{
	struct gpio_desc *gpio;
	int flags = GPIOD_IS_OUT;
	int ret;

	if (!dev_read_bool(dev, "enable-active-high"))
		flags |= GPIOD_ACTIVE_LOW;
	if (dev_read_bool(dev, "regulator-boot-on"))
		flags |= GPIOD_IS_OUT_ACTIVE;

	/* Get optional enable GPIO desc */
	gpio = &plat->gpio;
	ret = gpio_request_by_name(dev, enable_gpio_name, 0, gpio, flags);
	if (ret) {
		debug("Regulator '%s' optional enable GPIO - not found! Error: %d\n",
		      dev->name, ret);
		if (ret != -ENOENT)
			return ret;
	}

	/* Get optional ramp up delay */
	plat->startup_delay_us = dev_read_u32_default(dev,
						      "startup-delay-us", 0);
	plat->off_on_delay_us = dev_read_u32_default(dev, "off-on-delay-us", 0);
	if (!plat->off_on_delay_us) {
		plat->off_on_delay_us =
			dev_read_u32_default(dev, "u-boot,off-on-delay-us", 0);
	}

	return 0;
}

int regulator_common_get_enable(const struct udevice *dev,
	struct regulator_common_plat *plat)
{
	/* Enable GPIO is optional */
	if (!plat->gpio.dev)
		return true;

	return dm_gpio_get_value(&plat->gpio);
}

int regulator_common_set_enable(const struct udevice *dev,
	struct regulator_common_plat *plat, bool enable)
{
	int ret;

	debug("%s: dev='%s', enable=%d, delay=%d, has_gpio=%d\n", __func__,
	      dev->name, enable, plat->startup_delay_us,
	      dm_gpio_is_valid(&plat->gpio));
	/* Enable GPIO is optional */
	if (!dm_gpio_is_valid(&plat->gpio)) {
		if (!enable)
			return -ENOSYS;
		return 0;
	}

	/* If previously enabled, increase count */
	if (enable && plat->enable_count > 0) {
		plat->enable_count++;
		return -EALREADY;
	}

	if (!enable) {
		if (plat->enable_count > 1) {
			/* If enabled multiple times, decrease count */
			plat->enable_count--;
			return -EBUSY;
		} else if (!plat->enable_count) {
			/* If already disabled, do nothing */
			return -EALREADY;
		}
	}

	ret = dm_gpio_set_value(&plat->gpio, enable);
	if (ret) {
		pr_err("Can't set regulator : %s gpio to: %d\n", dev->name,
		      enable);
		return ret;
	}

	if (enable && plat->startup_delay_us)
		udelay(plat->startup_delay_us);
	debug("%s: done\n", __func__);

	if (!enable && plat->off_on_delay_us)
		udelay(plat->off_on_delay_us);

	if (enable)
		plat->enable_count++;
	else
		plat->enable_count--;

	return 0;
}
