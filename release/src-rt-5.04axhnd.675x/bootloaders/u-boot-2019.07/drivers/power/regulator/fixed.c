// SPDX-License-Identifier: GPL-2.0+
/*
 *  Copyright (C) 2015 Samsung Electronics
 *
 *  Przemyslaw Marczak <p.marczak@samsung.com>
 */

#include <common.h>
#include <errno.h>
#include <dm.h>
#include <i2c.h>
#include <asm/gpio.h>
#include <power/pmic.h>
#include <power/regulator.h>

struct fixed_regulator_platdata {
	struct gpio_desc gpio; /* GPIO for regulator enable control */
	unsigned int startup_delay_us;
	unsigned int off_on_delay_us;
};

static int fixed_regulator_ofdata_to_platdata(struct udevice *dev)
{
	struct dm_regulator_uclass_platdata *uc_pdata;
	struct fixed_regulator_platdata *dev_pdata;
	struct gpio_desc *gpio;
	int flags = GPIOD_IS_OUT;
	int ret;

	dev_pdata = dev_get_platdata(dev);
	uc_pdata = dev_get_uclass_platdata(dev);
	if (!uc_pdata)
		return -ENXIO;

	/* Set type to fixed */
	uc_pdata->type = REGULATOR_TYPE_FIXED;

	if (dev_read_bool(dev, "enable-active-high"))
		flags |= GPIOD_IS_OUT_ACTIVE;

	/* Get fixed regulator optional enable GPIO desc */
	gpio = &dev_pdata->gpio;
	ret = gpio_request_by_name(dev, "gpio", 0, gpio, flags);
	if (ret) {
		debug("Fixed regulator optional enable GPIO - not found! Error: %d\n",
		      ret);
		if (ret != -ENOENT)
			return ret;
	}

	/* Get optional ramp up delay */
	dev_pdata->startup_delay_us = dev_read_u32_default(dev,
							"startup-delay-us", 0);
	dev_pdata->off_on_delay_us =
			dev_read_u32_default(dev, "u-boot,off-on-delay-us", 0);

	return 0;
}

static int fixed_regulator_get_value(struct udevice *dev)
{
	struct dm_regulator_uclass_platdata *uc_pdata;

	uc_pdata = dev_get_uclass_platdata(dev);
	if (!uc_pdata)
		return -ENXIO;

	if (uc_pdata->min_uV != uc_pdata->max_uV) {
		debug("Invalid constraints for: %s\n", uc_pdata->name);
		return -EINVAL;
	}

	return uc_pdata->min_uV;
}

static int fixed_regulator_get_current(struct udevice *dev)
{
	struct dm_regulator_uclass_platdata *uc_pdata;

	uc_pdata = dev_get_uclass_platdata(dev);
	if (!uc_pdata)
		return -ENXIO;

	if (uc_pdata->min_uA != uc_pdata->max_uA) {
		debug("Invalid constraints for: %s\n", uc_pdata->name);
		return -EINVAL;
	}

	return uc_pdata->min_uA;
}

static int fixed_regulator_get_enable(struct udevice *dev)
{
	struct fixed_regulator_platdata *dev_pdata = dev_get_platdata(dev);

	/* Enable GPIO is optional */
	if (!dev_pdata->gpio.dev)
		return true;

	return dm_gpio_get_value(&dev_pdata->gpio);
}

static int fixed_regulator_set_enable(struct udevice *dev, bool enable)
{
	struct fixed_regulator_platdata *dev_pdata = dev_get_platdata(dev);
	int ret;

	debug("%s: dev='%s', enable=%d, delay=%d, has_gpio=%d\n", __func__,
	      dev->name, enable, dev_pdata->startup_delay_us,
	      dm_gpio_is_valid(&dev_pdata->gpio));
	/* Enable GPIO is optional */
	if (!dm_gpio_is_valid(&dev_pdata->gpio)) {
		if (!enable)
			return -ENOSYS;
		return 0;
	}

	ret = dm_gpio_set_value(&dev_pdata->gpio, enable);
	if (ret) {
		pr_err("Can't set regulator : %s gpio to: %d\n", dev->name,
		      enable);
		return ret;
	}

	if (enable && dev_pdata->startup_delay_us)
		udelay(dev_pdata->startup_delay_us);
	debug("%s: done\n", __func__);

	if (!enable && dev_pdata->off_on_delay_us)
		udelay(dev_pdata->off_on_delay_us);

	return 0;
}

static const struct dm_regulator_ops fixed_regulator_ops = {
	.get_value	= fixed_regulator_get_value,
	.get_current	= fixed_regulator_get_current,
	.get_enable	= fixed_regulator_get_enable,
	.set_enable	= fixed_regulator_set_enable,
};

static const struct udevice_id fixed_regulator_ids[] = {
	{ .compatible = "regulator-fixed" },
	{ },
};

U_BOOT_DRIVER(fixed_regulator) = {
	.name = "fixed regulator",
	.id = UCLASS_REGULATOR,
	.ops = &fixed_regulator_ops,
	.of_match = fixed_regulator_ids,
	.ofdata_to_platdata = fixed_regulator_ofdata_to_platdata,
	.platdata_auto_alloc_size = sizeof(struct fixed_regulator_platdata),
};
