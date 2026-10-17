// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2016 Texas Instruments Incorporated, <www.ti.com>
 * Keerthy <j-keerthy@ti.com>
 */

#include <common.h>
#include <fdtdec.h>
#include <errno.h>
#include <dm.h>
#include <log.h>
#include <asm/gpio.h>
#include <linux/printk.h>
#include <power/pmic.h>
#include <power/regulator.h>
#include "regulator_common.h"

#include "regulator_common.h"

#define GPIO_REGULATOR_MAX_STATES	2

struct gpio_regulator_plat {
	struct regulator_common_plat common;
	struct gpio_desc gpio; /* GPIO for regulator voltage control */
	int states[GPIO_REGULATOR_MAX_STATES];
	int voltages[GPIO_REGULATOR_MAX_STATES];
};

static int gpio_regulator_of_to_plat(struct udevice *dev)
{
	struct dm_regulator_uclass_plat *uc_pdata;
	struct gpio_regulator_plat *plat;
	struct gpio_desc *gpio;
	int ret, count, i, j;
	u32 states_array[GPIO_REGULATOR_MAX_STATES * 2];

	plat = dev_get_plat(dev);
	uc_pdata = dev_get_uclass_plat(dev);
	if (!uc_pdata)
		return -ENXIO;

	/* Set type to gpio */
	uc_pdata->type = REGULATOR_TYPE_GPIO;

	/*
	 * Get gpio regulator gpio desc
	 * Assuming one GPIO per regulator.
	 * Can be extended later to multiple GPIOs
	 * per gpio-regulator. As of now no instance with multiple
	 * gpios is presnt
	 */
	gpio = &plat->gpio;
	ret = gpio_request_by_name(dev, "gpios", 0, gpio, GPIOD_IS_OUT);
	if (ret)
		debug("regulator gpio - not found! Error: %d", ret);

	ret = dev_read_size(dev, "states");
	if (ret < 0)
		return ret;

	count = ret / sizeof(states_array[0]);
	if (count > ARRAY_SIZE(states_array)) {
		debug("regulator gpio - to many states (%d > %d)",
		      count / 2, GPIO_REGULATOR_MAX_STATES);
		count = ARRAY_SIZE(states_array);
	}

	ret = dev_read_u32_array(dev, "states", states_array, count);
	if (ret < 0)
		return ret;

	for (i = 0, j = 0; i < count; i += 2) {
		plat->voltages[j] = states_array[i];
		plat->states[j] = states_array[i + 1];
		j++;
	}

	return regulator_common_of_to_plat(dev, &plat->common, "enable-gpios");
}

static int gpio_regulator_get_value(struct udevice *dev)
{
	struct dm_regulator_uclass_plat *uc_pdata;
	struct gpio_regulator_plat *plat = dev_get_plat(dev);
	int enable;

	if (!plat->gpio.dev)
		return -ENOSYS;

	uc_pdata = dev_get_uclass_plat(dev);
	if (uc_pdata->min_uV > uc_pdata->max_uV) {
		debug("Invalid constraints for: %s\n", uc_pdata->name);
		return -EINVAL;
	}

	enable = dm_gpio_get_value(&plat->gpio);
	if (enable == plat->states[0])
		return plat->voltages[0];
	else
		return plat->voltages[1];
}

static int gpio_regulator_set_value(struct udevice *dev, int uV)
{
	struct gpio_regulator_plat *plat = dev_get_plat(dev);
	int ret;
	bool enable;

	if (!plat->gpio.dev)
		return -ENOSYS;

	if (uV == plat->voltages[0])
		enable = plat->states[0];
	else if (uV == plat->voltages[1])
		enable = plat->states[1];
	else
		return -EINVAL;

	ret = dm_gpio_set_value(&plat->gpio, enable);
	if (ret) {
		pr_err("Can't set regulator : %s gpio to: %d\n", dev->name,
		      enable);
		return ret;
	}

	return 0;
}

static int gpio_regulator_get_enable(struct udevice *dev)
{
	struct gpio_regulator_plat *plat = dev_get_plat(dev);
	return regulator_common_get_enable(dev, &plat->common);
}

static int gpio_regulator_set_enable(struct udevice *dev, bool enable)
{
	struct gpio_regulator_plat *plat = dev_get_plat(dev);
	return regulator_common_set_enable(dev, &plat->common, enable);
}

static const struct dm_regulator_ops gpio_regulator_ops = {
	.get_value	= gpio_regulator_get_value,
	.set_value	= gpio_regulator_set_value,
	.get_enable	= gpio_regulator_get_enable,
	.set_enable	= gpio_regulator_set_enable,
};

static const struct udevice_id gpio_regulator_ids[] = {
	{ .compatible = "regulator-gpio" },
	{ },
};

U_BOOT_DRIVER(gpio_regulator) = {
	.name = "gpio regulator",
	.id = UCLASS_REGULATOR,
	.ops = &gpio_regulator_ops,
	.of_match = gpio_regulator_ids,
	.of_to_plat = gpio_regulator_of_to_plat,
	.plat_auto	= sizeof(struct gpio_regulator_plat),
};
