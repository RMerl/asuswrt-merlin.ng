// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c), Vaisala Oyj
 */

#include <common.h>
#include <asm/gpio.h>
#include <dm.h>
#include <dm/devres.h>
#include <errno.h>
#include <reboot-mode/reboot-mode-gpio.h>
#include <reboot-mode/reboot-mode.h>

DECLARE_GLOBAL_DATA_PTR;

static int reboot_mode_get(struct udevice *dev, u32 *buf)
{
	int ret;
	struct reboot_mode_gpio_platdata *plat_data;

	if (!buf)
		return -EINVAL;

	plat_data = dev_get_plat(dev);
	if (!plat_data)
		return -EINVAL;

	ret = dm_gpio_get_values_as_int(plat_data->gpio_desc,
					plat_data->gpio_count);
	if (ret < 0)
		return ret;

	*buf = ret;

	return 0;
}

static int reboot_mode_probe(struct udevice *dev)
{
	struct reboot_mode_gpio_platdata *plat_data;

	plat_data = dev_get_plat(dev);
	if (!plat_data)
		return -EINVAL;

	int ret;

#if CONFIG_IS_ENABLED(OF_CONTROL)
	ret = gpio_get_list_count(dev, "gpios");
	if (ret < 0)
		return ret;

	plat_data->gpio_count = ret;
#endif

	if (plat_data->gpio_count <= 0)
		return -EINVAL;

	plat_data->gpio_desc = devm_kcalloc(dev, plat_data->gpio_count,
					    sizeof(struct gpio_desc), 0);
	if (!plat_data->gpio_desc)
		return -ENOMEM;

#if CONFIG_IS_ENABLED(OF_CONTROL)
	ret = gpio_request_list_by_name(dev, "gpios", plat_data->gpio_desc,
					plat_data->gpio_count, GPIOD_IS_IN);
	if (ret < 0)
		return ret;
#else
	for (int i = 0; i < plat_data->gpio_count; i++) {
		struct reboot_mode_gpio_config *gpio =
			plat_data->gpios_config + i;
		struct gpio_desc *desc = plat_data->gpio_desc + i;

		ret = uclass_get_device_by_seq(UCLASS_GPIO,
					       gpio->gpio_dev_offset,
					       &desc->dev);
		if (ret < 0)
			return ret;

		desc->flags = gpio->flags;
		desc->offset = gpio->gpio_offset;

		ret = dm_gpio_request(desc, "");
		if (ret < 0)
			return ret;

		ret = dm_gpio_set_dir(desc);
		if (ret < 0)
			return ret;
	}
#endif
	return 0;
}

static int reboot_mode_remove(struct udevice *dev)
{
	struct reboot_mode_gpio_platdata *plat_data;

	plat_data = dev_get_plat(dev);
	if (!plat_data)
		return -EINVAL;

	return gpio_free_list(dev, plat_data->gpio_desc, plat_data->gpio_count);
}

#if CONFIG_IS_ENABLED(OF_CONTROL)
static const struct udevice_id reboot_mode_ids[] = {
	{ .compatible = "reboot-mode-gpio", 0 },
	{ }
};
#endif

static const struct reboot_mode_ops reboot_mode_gpio_ops = {
	.get = reboot_mode_get,
};

U_BOOT_DRIVER(reboot_mode_gpio) = {
	.name = "reboot-mode-gpio",
	.id = UCLASS_REBOOT_MODE,
	.probe = reboot_mode_probe,
	.remove = reboot_mode_remove,
#if CONFIG_IS_ENABLED(OF_CONTROL)
	.of_match = reboot_mode_ids,
#endif
	.plat_auto = sizeof(struct reboot_mode_gpio_platdata),
	.ops = &reboot_mode_gpio_ops,
};
