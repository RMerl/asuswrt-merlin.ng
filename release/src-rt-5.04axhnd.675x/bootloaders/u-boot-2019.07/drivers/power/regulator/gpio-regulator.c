// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2016 Texas Instruments Incorporated, <www.ti.com>
 * Keerthy <j-keerthy@ti.com>
 */

#include <common.h>
#include <fdtdec.h>
#include <errno.h>
#include <dm.h>
#include <i2c.h>
#include <asm/gpio.h>
#include <power/pmic.h>
#include <power/regulator.h>

#define GPIO_REGULATOR_MAX_STATES	2

DECLARE_GLOBAL_DATA_PTR;

struct gpio_regulator_platdata {
	struct gpio_desc gpio; /* GPIO for regulator voltage control */
	int states[GPIO_REGULATOR_MAX_STATES];
	int voltages[GPIO_REGULATOR_MAX_STATES];
};

static int gpio_regulator_ofdata_to_platdata(struct udevice *dev)
{
	struct dm_regulator_uclass_platdata *uc_pdata;
	struct gpio_regulator_platdata *dev_pdata;
	struct gpio_desc *gpio;
	const void *blob = gd->fdt_blob;
	int node = dev_of_offset(dev);
	int ret, count, i, j;
	u32 states_array[8];

	dev_pdata = dev_get_platdata(dev);
	uc_pdata = dev_get_uclass_platdata(dev);
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
	gpio = &dev_pdata->gpio;
	ret = gpio_request_by_name(dev, "gpios", 0, gpio, GPIOD_IS_OUT);
	if (ret)
		debug("regulator gpio - not found! Error: %d", ret);

	count = fdtdec_get_int_array_count(blob, node, "states",
					   states_array, 8);

	if (!count)
		return -EINVAL;

	for (i = 0, j = 0; i < count; i += 2) {
		dev_pdata->voltages[j] = states_array[i];
		dev_pdata->states[j] = states_array[i + 1];
		j++;
	}

	return 0;
}

static int gpio_regulator_get_value(struct udevice *dev)
{
	struct dm_regulator_uclass_platdata *uc_pdata;
	struct gpio_regulator_platdata *dev_pdata = dev_get_platdata(dev);
	int enable;

	if (!dev_pdata->gpio.dev)
		return -ENOSYS;

	uc_pdata = dev_get_uclass_platdata(dev);
	if (uc_pdata->min_uV > uc_pdata->max_uV) {
		debug("Invalid constraints for: %s\n", uc_pdata->name);
		return -EINVAL;
	}

	enable = dm_gpio_get_value(&dev_pdata->gpio);
	if (enable == dev_pdata->states[0])
		return dev_pdata->voltages[0];
	else
		return dev_pdata->voltages[1];
}

static int gpio_regulator_set_value(struct udevice *dev, int uV)
{
	struct gpio_regulator_platdata *dev_pdata = dev_get_platdata(dev);
	int ret;
	bool enable;

	if (!dev_pdata->gpio.dev)
		return -ENOSYS;

	if (uV == dev_pdata->voltages[0])
		enable = dev_pdata->states[0];
	else if (uV == dev_pdata->voltages[1])
		enable = dev_pdata->states[1];
	else
		return -EINVAL;

	ret = dm_gpio_set_value(&dev_pdata->gpio, enable);
	if (ret) {
		pr_err("Can't set regulator : %s gpio to: %d\n", dev->name,
		      enable);
		return ret;
	}

	return 0;
}

static const struct dm_regulator_ops gpio_regulator_ops = {
	.get_value	= gpio_regulator_get_value,
	.set_value	= gpio_regulator_set_value,
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
	.ofdata_to_platdata = gpio_regulator_ofdata_to_platdata,
	.platdata_auto_alloc_size = sizeof(struct gpio_regulator_platdata),
};
