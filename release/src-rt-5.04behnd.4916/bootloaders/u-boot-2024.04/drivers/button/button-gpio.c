// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2020 Philippe Reynes <philippe.reynes@softathome.com>
 */

#include <common.h>
#include <button.h>
#include <dm.h>
#include <dm/lists.h>
#include <dm/uclass-internal.h>
#include <log.h>
#include <asm/gpio.h>

struct button_gpio_priv {
	struct gpio_desc gpio;
	int linux_code;
};

static enum button_state_t button_gpio_get_state(struct udevice *dev)
{
	struct button_gpio_priv *priv = dev_get_priv(dev);
	int ret;

	if (!dm_gpio_is_valid(&priv->gpio))
		return -EREMOTEIO;
	ret = dm_gpio_get_value(&priv->gpio);
	if (ret < 0)
		return ret;

	return ret ? BUTTON_ON : BUTTON_OFF;
}

static int button_gpio_get_code(struct udevice *dev)
{
	struct button_gpio_priv *priv = dev_get_priv(dev);
	int code = priv->linux_code;

	if (!code)
		return -ENODATA;

	return code;
}

static int button_gpio_probe(struct udevice *dev)
{
	struct button_uc_plat *uc_plat = dev_get_uclass_plat(dev);
	struct button_gpio_priv *priv = dev_get_priv(dev);
	int ret;

	/* Ignore the top-level button node */
	if (!uc_plat->label)
		return 0;

	ret = gpio_request_by_name(dev, "gpios", 0, &priv->gpio, GPIOD_IS_IN);
	if (ret)
		return ret;

	ret = dev_read_u32(dev, "linux,code", &priv->linux_code);

	return ret;
}

static int button_gpio_remove(struct udevice *dev)
{
	/*
	 * The GPIO driver may have already been removed. We will need to
	 * address this more generally.
	 */
	if (!IS_ENABLED(CONFIG_SANDBOX)) {
		struct button_gpio_priv *priv = dev_get_priv(dev);

		if (dm_gpio_is_valid(&priv->gpio))
			dm_gpio_free(dev, &priv->gpio);
	}

	return 0;
}

static int button_gpio_bind(struct udevice *parent)
{
	struct udevice *dev;
	ofnode node;
	int ret;

	dev_for_each_subnode(node, parent) {
		struct button_uc_plat *uc_plat;
		const char *label;

		label = ofnode_read_string(node, "label");
		if (!label) {
			debug("%s: node %s has no label\n", __func__,
			      ofnode_get_name(node));
			return -EINVAL;
		}
		ret = device_bind_driver_to_node(parent, "button_gpio",
						 ofnode_get_name(node),
						 node, &dev);
		if (ret)
			return ret;
		uc_plat = dev_get_uclass_plat(dev);
		uc_plat->label = label;
	}

	return 0;
}

static const struct button_ops button_gpio_ops = {
	.get_state	= button_gpio_get_state,
	.get_code	= button_gpio_get_code,
};

static const struct udevice_id button_gpio_ids[] = {
	{ .compatible = "gpio-keys" },
	{ .compatible = "gpio-keys-polled" },
	{ }
};

U_BOOT_DRIVER(button_gpio) = {
	.name		= "button_gpio",
	.id		= UCLASS_BUTTON,
	.of_match	= button_gpio_ids,
	.ops		= &button_gpio_ops,
	.priv_auto	= sizeof(struct button_gpio_priv),
	.bind		= button_gpio_bind,
	.probe		= button_gpio_probe,
	.remove		= button_gpio_remove,
};
