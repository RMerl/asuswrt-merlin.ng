// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c), Vaisala Oyj
 */

#include <common.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <dm/devres.h>
#include <exports.h>
#include <reboot-mode/reboot-mode.h>

int dm_reboot_mode_update(struct udevice *dev)
{
	struct reboot_mode_ops *ops = reboot_mode_get_ops(dev);
	u32 rebootmode;
	int ret, i;

	assert(ops);

	if (!ops->get)
		return -ENOSYS;

	ret = ops->get(dev, &rebootmode);
	if (ret < 0) {
		dev_err(dev, "Failed to retrieve the reboot mode value\n");
		return ret;
	}

	const struct reboot_mode_uclass_platdata *plat_data =
		dev_get_uclass_plat(dev);

	for (i = 0; i < plat_data->count; i++) {
		if (plat_data->modes[i].mode_id == rebootmode) {
			ret = env_set(plat_data->env_variable,
				      plat_data->modes[i].mode_name);
			if (ret) {
				dev_err(dev, "Failed to set env: %s\n",
					plat_data->env_variable);
				return ret;
			}
		}
	}

	if (ops->set) {
		/* Clear the value */
		rebootmode = 0;
		ret = ops->set(dev, rebootmode);
		if (ret) {
			dev_err(dev, "Failed to clear the reboot mode\n");
			return ret;
		}
	}

	return 0;
}

int dm_reboot_mode_pre_probe(struct udevice *dev)
{
	struct reboot_mode_uclass_platdata *plat_data;

	plat_data = dev_get_uclass_plat(dev);
	if (!plat_data)
		return -EINVAL;

#if CONFIG_IS_ENABLED(OF_CONTROL)
	const char *mode_prefix = "mode-";
	const int mode_prefix_len = strlen(mode_prefix);
	struct ofprop property;
	const u32 *propvalue;
	const char *propname;

	plat_data->env_variable = dev_read_string(dev, "u-boot,env-variable");
	if (!plat_data->env_variable)
		plat_data->env_variable = "reboot-mode";

	plat_data->count = 0;

	dev_for_each_property(property, dev) {
		propvalue = dev_read_prop_by_prop(&property, &propname, NULL);
		if (!propvalue) {
			dev_err(dev, "Could not get the value for property %s\n",
				propname);
			return -EINVAL;
		}

		if (!strncmp(propname, mode_prefix, mode_prefix_len))
			plat_data->count++;
	}

	plat_data->modes = devm_kcalloc(dev, plat_data->count,
					sizeof(struct reboot_mode_mode), 0);

	struct reboot_mode_mode *next = plat_data->modes;

	dev_for_each_property(property, dev) {
		propvalue = dev_read_prop_by_prop(&property, &propname, NULL);
		if (!propvalue) {
			dev_err(dev, "Could not get the value for property %s\n",
				propname);
			return -EINVAL;
		}

		if (!strncmp(propname, mode_prefix, mode_prefix_len)) {
			next->mode_name = &propname[mode_prefix_len];
			next->mode_id = fdt32_to_cpu(*propvalue);

			next++;
		}
	}
#else
	if (!plat_data->env_variable)
		plat_data->env_variable = "reboot-mode";

#endif

	return 0;
}

UCLASS_DRIVER(reboot_mode) = {
	.name	= "reboot-mode",
	.id	= UCLASS_REBOOT_MODE,
	.pre_probe	= dm_reboot_mode_pre_probe,
	.per_device_plat_auto =
		sizeof(struct reboot_mode_uclass_platdata),
};
