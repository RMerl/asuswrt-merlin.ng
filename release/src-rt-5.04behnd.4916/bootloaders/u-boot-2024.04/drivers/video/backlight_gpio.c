// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017, STMicroelectronics - All Rights Reserved
 * Author: Patrick Delaunay <patrick.delaunay@foss.st.com>
 */

#include <common.h>
#include <dm.h>
#include <backlight.h>
#include <log.h>
#include <asm/gpio.h>

struct gpio_backlight_priv {
	struct gpio_desc gpio;
	bool def_value;
};

static int gpio_backlight_enable(struct udevice *dev)
{
	struct gpio_backlight_priv *priv = dev_get_priv(dev);

	dm_gpio_set_value(&priv->gpio, 1);

	return 0;
}

static int gpio_backlight_of_to_plat(struct udevice *dev)
{
	struct gpio_backlight_priv *priv = dev_get_priv(dev);
	int ret;

	ret = gpio_request_by_name(dev, "gpios", 0, &priv->gpio,
				   GPIOD_IS_OUT);
	if (ret) {
		debug("%s: Warning: cannot get GPIO: ret=%d\n",
		      __func__, ret);
		return ret;
	}

	priv->def_value = dev_read_bool(dev, "default-on");

	return 0;
}

static int gpio_backlight_probe(struct udevice *dev)
{
	struct gpio_backlight_priv *priv = dev_get_priv(dev);

	if (priv->def_value)
		gpio_backlight_enable(dev);

	return 0;
}

static const struct backlight_ops gpio_backlight_ops = {
	.enable	= gpio_backlight_enable,
};

static const struct udevice_id gpio_backlight_ids[] = {
	{ .compatible = "gpio-backlight" },
	{ }
};

U_BOOT_DRIVER(gpio_backlight) = {
	.name	= "gpio_backlight",
	.id	= UCLASS_PANEL_BACKLIGHT,
	.of_match = gpio_backlight_ids,
	.ops	= &gpio_backlight_ops,
	.of_to_plat	= gpio_backlight_of_to_plat,
	.probe		= gpio_backlight_probe,
	.priv_auto	= sizeof(struct gpio_backlight_priv),
};
