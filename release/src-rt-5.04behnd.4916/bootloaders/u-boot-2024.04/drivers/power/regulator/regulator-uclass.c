// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2014-2015 Samsung Electronics
 * Przemyslaw Marczak <p.marczak@samsung.com>
 */

#define LOG_CATEGORY UCLASS_REGULATOR

#include <common.h>
#include <errno.h>
#include <dm.h>
#include <log.h>
#include <dm/uclass-internal.h>
#include <linux/delay.h>
#include <power/pmic.h>
#include <power/regulator.h>

int regulator_mode(struct udevice *dev, struct dm_regulator_mode **modep)
{
	struct dm_regulator_uclass_plat *uc_pdata;

	*modep = NULL;

	uc_pdata = dev_get_uclass_plat(dev);
	if (!uc_pdata)
		return -ENXIO;

	*modep = uc_pdata->mode;
	return uc_pdata->mode_count;
}

int regulator_get_value(struct udevice *dev)
{
	const struct dm_regulator_ops *ops = dev_get_driver_ops(dev);

	if (!ops || !ops->get_value)
		return -ENOSYS;

	return ops->get_value(dev);
}

static void regulator_set_value_ramp_delay(struct udevice *dev, int old_uV,
					   int new_uV, unsigned int ramp_delay)
{
	int delay = DIV_ROUND_UP(abs(new_uV - old_uV), ramp_delay);

	debug("regulator %s: delay %u us (%d uV -> %d uV)\n", dev->name, delay,
	      old_uV, new_uV);

	udelay(delay);
}

int regulator_set_value(struct udevice *dev, int uV)
{
	const struct dm_regulator_ops *ops = dev_get_driver_ops(dev);
	struct dm_regulator_uclass_plat *uc_pdata;
	int ret, old_uV = uV, is_enabled = 0;

	uc_pdata = dev_get_uclass_plat(dev);
	if (uc_pdata->min_uV != -ENODATA && uV < uc_pdata->min_uV)
		return -EINVAL;
	if (uc_pdata->max_uV != -ENODATA && uV > uc_pdata->max_uV)
		return -EINVAL;

	if (!ops || !ops->set_value)
		return -ENOSYS;

	if (uc_pdata->ramp_delay) {
		is_enabled = regulator_get_enable(dev);
		old_uV = regulator_get_value(dev);
	}

	ret = ops->set_value(dev, uV);

	if (!ret) {
		if (uc_pdata->ramp_delay && old_uV > 0 && is_enabled)
			regulator_set_value_ramp_delay(dev, old_uV, uV,
						       uc_pdata->ramp_delay);
	}

	return ret;
}

int regulator_set_suspend_value(struct udevice *dev, int uV)
{
	const struct dm_regulator_ops *ops = dev_get_driver_ops(dev);
	struct dm_regulator_uclass_plat *uc_pdata;

	uc_pdata = dev_get_uclass_plat(dev);
	if (uc_pdata->min_uV != -ENODATA && uV < uc_pdata->min_uV)
		return -EINVAL;
	if (uc_pdata->max_uV != -ENODATA && uV > uc_pdata->max_uV)
		return -EINVAL;

	if (!ops->set_suspend_value)
		return -ENOSYS;

	return ops->set_suspend_value(dev, uV);
}

int regulator_get_suspend_value(struct udevice *dev)
{
	const struct dm_regulator_ops *ops = dev_get_driver_ops(dev);

	if (!ops->get_suspend_value)
		return -ENOSYS;

	return ops->get_suspend_value(dev);
}

/*
 * To be called with at most caution as there is no check
 * before setting the actual voltage value.
 */
int regulator_set_value_force(struct udevice *dev, int uV)
{
	const struct dm_regulator_ops *ops = dev_get_driver_ops(dev);

	if (!ops || !ops->set_value)
		return -ENOSYS;

	return ops->set_value(dev, uV);
}

int regulator_get_current(struct udevice *dev)
{
	const struct dm_regulator_ops *ops = dev_get_driver_ops(dev);

	if (!ops || !ops->get_current)
		return -ENOSYS;

	return ops->get_current(dev);
}

int regulator_set_current(struct udevice *dev, int uA)
{
	const struct dm_regulator_ops *ops = dev_get_driver_ops(dev);
	struct dm_regulator_uclass_plat *uc_pdata;

	uc_pdata = dev_get_uclass_plat(dev);
	if (uc_pdata->min_uA != -ENODATA && uA < uc_pdata->min_uA)
		return -EINVAL;
	if (uc_pdata->max_uA != -ENODATA && uA > uc_pdata->max_uA)
		return -EINVAL;

	if (!ops || !ops->set_current)
		return -ENOSYS;

	return ops->set_current(dev, uA);
}

int regulator_get_enable(struct udevice *dev)
{
	const struct dm_regulator_ops *ops = dev_get_driver_ops(dev);

	if (!ops || !ops->get_enable)
		return -ENOSYS;

	return ops->get_enable(dev);
}

int regulator_set_enable(struct udevice *dev, bool enable)
{
	const struct dm_regulator_ops *ops = dev_get_driver_ops(dev);
	struct dm_regulator_uclass_plat *uc_pdata;
	int ret, old_enable = 0;

	if (!ops || !ops->set_enable)
		return -ENOSYS;

	uc_pdata = dev_get_uclass_plat(dev);
	if (!enable && uc_pdata->always_on)
		return -EACCES;

	if (uc_pdata->ramp_delay)
		old_enable = regulator_get_enable(dev);

	ret = ops->set_enable(dev, enable);
	if (!ret) {
		if (uc_pdata->ramp_delay && !old_enable && enable) {
			int uV = regulator_get_value(dev);

			if (uV > 0) {
				regulator_set_value_ramp_delay(dev, 0, uV,
							       uc_pdata->ramp_delay);
			}
		}
	}

	return ret;
}

int regulator_set_enable_if_allowed(struct udevice *dev, bool enable)
{
	int ret;

	ret = regulator_set_enable(dev, enable);
	if (ret == -ENOSYS || ret == -EACCES)
		return 0;
	/* if we want to disable but it's in use by someone else */
	if (!enable && ret == -EBUSY)
		return 0;
	/* if it's already enabled/disabled */
	if (ret == -EALREADY)
		return 0;

	return ret;
}

int regulator_set_suspend_enable(struct udevice *dev, bool enable)
{
	const struct dm_regulator_ops *ops = dev_get_driver_ops(dev);

	if (!ops->set_suspend_enable)
		return -ENOSYS;

	return ops->set_suspend_enable(dev, enable);
}

int regulator_get_suspend_enable(struct udevice *dev)
{
	const struct dm_regulator_ops *ops = dev_get_driver_ops(dev);

	if (!ops->get_suspend_enable)
		return -ENOSYS;

	return ops->get_suspend_enable(dev);
}

int regulator_get_mode(struct udevice *dev)
{
	const struct dm_regulator_ops *ops = dev_get_driver_ops(dev);

	if (!ops || !ops->get_mode)
		return -ENOSYS;

	return ops->get_mode(dev);
}

int regulator_set_mode(struct udevice *dev, int mode)
{
	const struct dm_regulator_ops *ops = dev_get_driver_ops(dev);

	if (!ops || !ops->set_mode)
		return -ENOSYS;

	return ops->set_mode(dev, mode);
}

int regulator_get_by_platname(const char *plat_name, struct udevice **devp)
{
	struct dm_regulator_uclass_plat *uc_pdata;
	struct udevice *dev;
	int ret;

	*devp = NULL;

	for (ret = uclass_find_first_device(UCLASS_REGULATOR, &dev); dev;
	     ret = uclass_find_next_device(&dev)) {
		if (ret) {
			debug("regulator %s, ret=%d\n", dev->name, ret);
			continue;
		}

		uc_pdata = dev_get_uclass_plat(dev);
		if (!uc_pdata || strcmp(plat_name, uc_pdata->name))
			continue;

		return uclass_get_device_tail(dev, 0, devp);
	}

	debug("%s: can't find: %s, ret=%d\n", __func__, plat_name, ret);

	return -ENODEV;
}

int regulator_get_by_devname(const char *devname, struct udevice **devp)
{
	return uclass_get_device_by_name(UCLASS_REGULATOR, devname, devp);
}

int device_get_supply_regulator(struct udevice *dev, const char *supply_name,
				struct udevice **devp)
{
	return uclass_get_device_by_phandle(UCLASS_REGULATOR, dev,
					    supply_name, devp);
}

int regulator_autoset(struct udevice *dev)
{
	struct dm_regulator_uclass_plat *uc_pdata;
	int ret = 0;

	uc_pdata = dev_get_uclass_plat(dev);

	if (uc_pdata->flags & REGULATOR_FLAG_AUTOSET_DONE)
		return -EALREADY;

	ret = regulator_set_suspend_enable(dev, uc_pdata->suspend_on);
	if (ret == -ENOSYS)
		ret = 0;

	if (!ret && uc_pdata->suspend_on) {
		ret = regulator_set_suspend_value(dev, uc_pdata->suspend_uV);
		if (ret == -ENOSYS)
			ret = 0;

		if (ret)
			return ret;
	}

	if (!uc_pdata->always_on && !uc_pdata->boot_on) {
		ret = -EMEDIUMTYPE;
		goto out;
	}

	if (uc_pdata->type == REGULATOR_TYPE_FIXED) {
		ret = regulator_set_enable(dev, true);
		goto out;
	}

	if (uc_pdata->flags & REGULATOR_FLAG_AUTOSET_UV)
		ret = regulator_set_value(dev, uc_pdata->min_uV);
	if (uc_pdata->init_uV > 0)
		ret = regulator_set_value(dev, uc_pdata->init_uV);
	if (!ret && (uc_pdata->flags & REGULATOR_FLAG_AUTOSET_UA))
		ret = regulator_set_current(dev, uc_pdata->min_uA);

	if (!ret)
		ret = regulator_set_enable(dev, true);

out:
	uc_pdata->flags |= REGULATOR_FLAG_AUTOSET_DONE;

	return ret;
}

int regulator_unset(struct udevice *dev)
{
	struct dm_regulator_uclass_plat *uc_pdata;

	uc_pdata = dev_get_uclass_plat(dev);
	if (uc_pdata && uc_pdata->force_off)
		return regulator_set_enable(dev, false);

	return -EMEDIUMTYPE;
}

static void regulator_show(struct udevice *dev, int ret)
{
	struct dm_regulator_uclass_plat *uc_pdata;

	uc_pdata = dev_get_uclass_plat(dev);

	printf("%s@%s: ", dev->name, uc_pdata->name);
	if (uc_pdata->flags & REGULATOR_FLAG_AUTOSET_UV)
		printf("set %d uV", uc_pdata->min_uV);
	if (uc_pdata->flags & REGULATOR_FLAG_AUTOSET_UA)
		printf("; set %d uA", uc_pdata->min_uA);
	printf("; enabling");
	if (ret)
		printf(" (ret: %d)", ret);
	printf("\n");
}

int regulator_autoset_by_name(const char *platname, struct udevice **devp)
{
	struct udevice *dev;
	int ret;

	ret = regulator_get_by_platname(platname, &dev);
	if (devp)
		*devp = dev;
	if (ret) {
		debug("Can get the regulator: %s (err=%d)\n", platname, ret);
		return ret;
	}

	return regulator_autoset(dev);
}

int regulator_list_autoset(const char *list_platname[],
			   struct udevice *list_devp[],
			   bool verbose)
{
	struct udevice *dev;
	int error = 0, i = 0, ret;

	while (list_platname[i]) {
		ret = regulator_autoset_by_name(list_platname[i], &dev);
		if (ret != -EMEDIUMTYPE && verbose)
			regulator_show(dev, ret);
		if (ret & !error)
			error = ret;

		if (list_devp)
			list_devp[i] = dev;

		i++;
	}

	return error;
}

static bool regulator_name_is_unique(struct udevice *check_dev,
				     const char *check_name)
{
	struct dm_regulator_uclass_plat *uc_pdata;
	struct udevice *dev;
	int check_len = strlen(check_name);
	int ret;
	int len;

	for (ret = uclass_find_first_device(UCLASS_REGULATOR, &dev); dev;
	     ret = uclass_find_next_device(&dev)) {
		if (ret || dev == check_dev)
			continue;

		uc_pdata = dev_get_uclass_plat(dev);
		len = strlen(uc_pdata->name);
		if (len != check_len)
			continue;

		if (!strcmp(uc_pdata->name, check_name))
			return false;
	}

	return true;
}

static int regulator_post_bind(struct udevice *dev)
{
	struct dm_regulator_uclass_plat *uc_pdata;
	const char *property = "regulator-name";

	uc_pdata = dev_get_uclass_plat(dev);

	/* Regulator's mandatory constraint */
	uc_pdata->name = dev_read_string(dev, property);
	if (!uc_pdata->name) {
		debug("%s: dev '%s' has no property '%s'\n",
		      __func__, dev->name, property);
		uc_pdata->name = dev_read_name(dev);
		if (!uc_pdata->name)
			return -EINVAL;
	}

	if (regulator_name_is_unique(dev, uc_pdata->name))
		return 0;

	debug("'%s' of dev: '%s', has nonunique value: '%s\n",
	      property, dev->name, uc_pdata->name);

	return -EINVAL;
}

static int regulator_pre_probe(struct udevice *dev)
{
	struct dm_regulator_uclass_plat *uc_pdata;
	ofnode node;

	uc_pdata = dev_get_uclass_plat(dev);
	if (!uc_pdata)
		return -ENXIO;

	/* Regulator's optional constraints */
	uc_pdata->min_uV = dev_read_u32_default(dev, "regulator-min-microvolt",
						-ENODATA);
	uc_pdata->max_uV = dev_read_u32_default(dev, "regulator-max-microvolt",
						-ENODATA);
	uc_pdata->init_uV = dev_read_u32_default(dev, "regulator-init-microvolt",
						 -ENODATA);
	uc_pdata->min_uA = dev_read_u32_default(dev, "regulator-min-microamp",
						-ENODATA);
	uc_pdata->max_uA = dev_read_u32_default(dev, "regulator-max-microamp",
						-ENODATA);
	uc_pdata->always_on = dev_read_bool(dev, "regulator-always-on");
	uc_pdata->boot_on = dev_read_bool(dev, "regulator-boot-on");
	uc_pdata->ramp_delay = dev_read_u32_default(dev, "regulator-ramp-delay",
						    0);
	uc_pdata->force_off = dev_read_bool(dev, "regulator-force-boot-off");

	node = dev_read_subnode(dev, "regulator-state-mem");
	if (ofnode_valid(node)) {
		uc_pdata->suspend_on = !ofnode_read_bool(node, "regulator-off-in-suspend");
		if (ofnode_read_u32(node, "regulator-suspend-microvolt", &uc_pdata->suspend_uV))
			uc_pdata->suspend_uV = uc_pdata->max_uV;
	} else {
		uc_pdata->suspend_on = true;
		uc_pdata->suspend_uV = uc_pdata->max_uV;
	}

	/* Those values are optional (-ENODATA if unset) */
	if ((uc_pdata->min_uV != -ENODATA) &&
	    (uc_pdata->max_uV != -ENODATA) &&
	    (uc_pdata->min_uV == uc_pdata->max_uV))
		uc_pdata->flags |= REGULATOR_FLAG_AUTOSET_UV;

	/* Those values are optional (-ENODATA if unset) */
	if ((uc_pdata->min_uA != -ENODATA) &&
	    (uc_pdata->max_uA != -ENODATA) &&
	    (uc_pdata->min_uA == uc_pdata->max_uA))
		uc_pdata->flags |= REGULATOR_FLAG_AUTOSET_UA;

	return 0;
}

int regulators_enable_boot_on(bool verbose)
{
	struct udevice *dev;
	struct uclass *uc;
	int ret;

	ret = uclass_get(UCLASS_REGULATOR, &uc);
	if (ret)
		return ret;
	for (uclass_first_device(UCLASS_REGULATOR, &dev);
	     dev;
	     uclass_next_device(&dev)) {
		ret = regulator_autoset(dev);
		if (ret == -EMEDIUMTYPE) {
			ret = 0;
			continue;
		}
		if (verbose)
			regulator_show(dev, ret);
		if (ret == -ENOSYS)
			ret = 0;
	}

	return ret;
}

int regulators_enable_boot_off(bool verbose)
{
	struct udevice *dev;
	struct uclass *uc;
	int ret;

	ret = uclass_get(UCLASS_REGULATOR, &uc);
	if (ret)
		return ret;
	for (uclass_first_device(UCLASS_REGULATOR, &dev);
	     dev;
	     uclass_next_device(&dev)) {
		ret = regulator_unset(dev);
		if (ret == -EMEDIUMTYPE) {
			ret = 0;
			continue;
		}
		if (verbose)
			regulator_show(dev, ret);
		if (ret == -ENOSYS)
			ret = 0;
	}

	return ret;
}

UCLASS_DRIVER(regulator) = {
	.id		= UCLASS_REGULATOR,
	.name		= "regulator",
	.post_bind	= regulator_post_bind,
	.pre_probe	= regulator_pre_probe,
	.per_device_plat_auto	= sizeof(struct dm_regulator_uclass_plat),
};
