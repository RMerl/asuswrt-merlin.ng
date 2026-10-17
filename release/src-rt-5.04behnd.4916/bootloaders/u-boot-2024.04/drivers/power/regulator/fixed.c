// SPDX-License-Identifier: GPL-2.0+
/*
 *  Copyright (C) 2015 Samsung Electronics
 *
 *  Przemyslaw Marczak <p.marczak@samsung.com>
 */

#include <common.h>
#include <clk.h>
#include <errno.h>
#include <dm.h>
#include <linux/delay.h>
#include <log.h>
#include <asm/gpio.h>
#include <power/pmic.h>
#include <power/regulator.h>
#include "regulator_common.h"

#include "regulator_common.h"

struct fixed_clock_regulator_plat {
	struct clk *enable_clock;
	unsigned int clk_enable_counter;
};

static int fixed_regulator_of_to_plat(struct udevice *dev)
{
	struct dm_regulator_uclass_plat *uc_pdata;
	struct regulator_common_plat *plat;
	bool gpios;

	plat = dev_get_plat(dev);
	uc_pdata = dev_get_uclass_plat(dev);
	if (!uc_pdata)
		return -ENXIO;

	uc_pdata->type = REGULATOR_TYPE_FIXED;

	gpios = dev_read_bool(dev, "gpios");
	return regulator_common_of_to_plat(dev, plat, gpios ? "gpios" : "gpio");
}

static int fixed_regulator_get_value(struct udevice *dev)
{
	struct dm_regulator_uclass_plat *uc_pdata;

	uc_pdata = dev_get_uclass_plat(dev);
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
	struct dm_regulator_uclass_plat *uc_pdata;

	uc_pdata = dev_get_uclass_plat(dev);
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
	return regulator_common_get_enable(dev, dev_get_plat(dev));
}

static int fixed_regulator_set_enable(struct udevice *dev, bool enable)
{
	return regulator_common_set_enable(dev, dev_get_plat(dev), enable);
}

static int fixed_clock_regulator_get_enable(struct udevice *dev)
{
	struct fixed_clock_regulator_plat *priv = dev_get_priv(dev);

	return priv->clk_enable_counter > 0;
}

static int fixed_clock_regulator_set_enable(struct udevice *dev, bool enable)
{
	struct fixed_clock_regulator_plat *priv = dev_get_priv(dev);
	struct regulator_common_plat *plat = dev_get_plat(dev);
	int ret = 0;

	if (enable) {
		ret = clk_enable(priv->enable_clock);
		priv->clk_enable_counter++;
	} else {
		ret = clk_disable(priv->enable_clock);
		priv->clk_enable_counter--;
	}
	if (ret)
		return ret;

	if (enable && plat->startup_delay_us)
		udelay(plat->startup_delay_us);

	if (!enable && plat->off_on_delay_us)
		udelay(plat->off_on_delay_us);

	return ret;
}

static const struct dm_regulator_ops fixed_regulator_ops = {
	.get_value	= fixed_regulator_get_value,
	.get_current	= fixed_regulator_get_current,
	.get_enable	= fixed_regulator_get_enable,
	.set_enable	= fixed_regulator_set_enable,
};

static const struct dm_regulator_ops fixed_clock_regulator_ops = {
	.get_enable	= fixed_clock_regulator_get_enable,
	.set_enable	= fixed_clock_regulator_set_enable,
};

static const struct udevice_id fixed_regulator_ids[] = {
	{ .compatible = "regulator-fixed" },
	{ },
};

static const struct udevice_id fixed_clock_regulator_ids[] = {
	{ .compatible = "regulator-fixed-clock" },
	{ },
};

U_BOOT_DRIVER(regulator_fixed) = {
	.name = "regulator_fixed",
	.id = UCLASS_REGULATOR,
	.ops = &fixed_regulator_ops,
	.of_match = fixed_regulator_ids,
	.of_to_plat = fixed_regulator_of_to_plat,
	.plat_auto = sizeof(struct regulator_common_plat),
};

U_BOOT_DRIVER(regulator_fixed_clock) = {
	.name = "regulator_fixed_clk",
	.id = UCLASS_REGULATOR,
	.ops = &fixed_clock_regulator_ops,
	.of_match = fixed_clock_regulator_ids,
	.of_to_plat = fixed_regulator_of_to_plat,
	.plat_auto = sizeof(struct fixed_clock_regulator_plat),
};
