// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2015 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <led.h>
#include <log.h>
#include <malloc.h>
#include <asm/gpio.h>

struct led_gpio_priv {
	struct gpio_desc gpio;
};

static int gpio_led_set_state(struct udevice *dev, enum led_state_t state)
{
	struct led_gpio_priv *priv = dev_get_priv(dev);
	int ret;

	if (!dm_gpio_is_valid(&priv->gpio))
		return -EREMOTEIO;
	switch (state) {
	case LEDST_OFF:
	case LEDST_ON:
		break;
	case LEDST_TOGGLE:
		ret = dm_gpio_get_value(&priv->gpio);
		if (ret < 0)
			return ret;
		state = !ret;
		break;
	default:
		return -ENOSYS;
	}

	return dm_gpio_set_value(&priv->gpio, state);
}

static enum led_state_t gpio_led_get_state(struct udevice *dev)
{
	struct led_gpio_priv *priv = dev_get_priv(dev);
	int ret;

	if (!dm_gpio_is_valid(&priv->gpio))
		return -EREMOTEIO;
	ret = dm_gpio_get_value(&priv->gpio);
	if (ret < 0)
		return ret;

	return ret ? LEDST_ON : LEDST_OFF;
}

static int led_gpio_probe(struct udevice *dev)
{
	struct led_gpio_priv *priv = dev_get_priv(dev);

	return gpio_request_by_name(dev, "gpios", 0, &priv->gpio, GPIOD_IS_OUT);
}

static int led_gpio_remove(struct udevice *dev)
{
	/*
	 * The GPIO driver may have already been removed. We will need to
	 * address this more generally.
	 */
#ifndef CONFIG_SANDBOX
	struct led_gpio_priv *priv = dev_get_priv(dev);

	if (dm_gpio_is_valid(&priv->gpio))
		dm_gpio_free(dev, &priv->gpio);
#endif

	return 0;
}

static int led_gpio_bind(struct udevice *parent)
{
	return led_bind_generic(parent, "gpio_led");
}

static const struct led_ops gpio_led_ops = {
	.set_state	= gpio_led_set_state,
	.get_state	= gpio_led_get_state,
};

U_BOOT_DRIVER(led_gpio) = {
	.name	= "gpio_led",
	.id	= UCLASS_LED,
	.ops	= &gpio_led_ops,
	.priv_auto	= sizeof(struct led_gpio_priv),
	.probe	= led_gpio_probe,
	.remove	= led_gpio_remove,
};

static const struct udevice_id led_gpio_ids[] = {
	{ .compatible = "gpio-leds" },
	{ }
};

U_BOOT_DRIVER(led_gpio_wrap) = {
	.name	= "gpio_led_wrap",
	.id	= UCLASS_NOP,
	.of_match = led_gpio_ids,
	.bind	= led_gpio_bind,
};
