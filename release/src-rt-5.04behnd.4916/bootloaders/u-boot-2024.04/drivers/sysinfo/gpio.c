// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2021 Sean Anderson <sean.anderson@seco.com>
 */

#include <common.h>
#include <dm.h>
#include <log.h>
#include <sysinfo.h>
#include <asm/gpio.h>
#include <dm/device_compat.h>

/**
 * struct sysinfo_gpio_priv - GPIO sysinfo private data
 * @gpios: List of GPIOs used to detect the revision
 * @gpio_num: The number of GPIOs in @gpios
 * @revision: The revision as detected from the GPIOs.
 */
struct sysinfo_gpio_priv {
	struct gpio_desc *gpios;
	int gpio_num, revision;
};

static int sysinfo_gpio_detect(struct udevice *dev)
{
	int ret;
	struct sysinfo_gpio_priv *priv = dev_get_priv(dev);

	ret = dm_gpio_get_values_as_int_base3(priv->gpios, priv->gpio_num);
	if (ret < 0)
		return ret;

	priv->revision = ret;
	return 0;
}

static int sysinfo_gpio_get_int(struct udevice *dev, int id, int *val)
{
	struct sysinfo_gpio_priv *priv = dev_get_priv(dev);

	switch (id) {
	case SYSINFO_ID_BOARD_MODEL:
		*val = priv->revision;
		return 0;
	default:
		return -EINVAL;
	};
}

static int sysinfo_gpio_get_str(struct udevice *dev, int id, size_t size, char *val)
{
	struct sysinfo_gpio_priv *priv = dev_get_priv(dev);

	switch (id) {
	case SYSINFO_ID_BOARD_MODEL: {
		const char *name = NULL;
		int i, ret;
		u32 revision;

		for (i = 0; ; i++) {
			ret = dev_read_u32_index(dev, "revisions", i,
						 &revision);
			if (ret) {
				if (ret != -EOVERFLOW)
					return ret;
				break;
			}

			if (revision == priv->revision) {
				ret = dev_read_string_index(dev, "names", i,
							    &name);
				if (ret < 0)
					return ret;
				break;
			}
		}
		if (!name)
			name = "unknown";

		strncpy(val, name, size);
		val[size - 1] = '\0';
		return 0;
	}
	default:
		return -EINVAL;
	};
}

static const struct sysinfo_ops sysinfo_gpio_ops = {
	.detect = sysinfo_gpio_detect,
	.get_int = sysinfo_gpio_get_int,
	.get_str = sysinfo_gpio_get_str,
};

static int sysinfo_gpio_probe(struct udevice *dev)
{
	int ret;
	struct sysinfo_gpio_priv *priv = dev_get_priv(dev);

	priv->gpio_num = gpio_get_list_count(dev, "gpios");
	if (priv->gpio_num < 0) {
		dev_err(dev, "could not get gpios length (err = %d)\n",
			priv->gpio_num);
		return priv->gpio_num;
	}

	priv->gpios = calloc(priv->gpio_num, sizeof(*priv->gpios));
	if (!priv->gpios) {
		dev_err(dev, "could not allocate memory for %d gpios\n",
			priv->gpio_num);
		return -ENOMEM;
	}

	ret = gpio_request_list_by_name(dev, "gpios", priv->gpios,
					priv->gpio_num, GPIOD_IS_IN);
	if (ret != priv->gpio_num) {
		dev_err(dev, "could not get gpios (err = %d)\n",
			priv->gpio_num);
		return ret;
	}

	if (!dev_read_bool(dev, "revisions") || !dev_read_bool(dev, "names")) {
		dev_err(dev, "revisions or names properties missing\n");
		return -ENOENT;
	}

	return 0;
}

static const struct udevice_id sysinfo_gpio_ids[] = {
	{ .compatible = "gpio-sysinfo" },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(sysinfo_gpio) = {
	.name           = "sysinfo_gpio",
	.id             = UCLASS_SYSINFO,
	.of_match       = sysinfo_gpio_ids,
	.ops		= &sysinfo_gpio_ops,
	.priv_auto	= sizeof(struct sysinfo_gpio_priv),
	.probe          = sysinfo_gpio_probe,
};
