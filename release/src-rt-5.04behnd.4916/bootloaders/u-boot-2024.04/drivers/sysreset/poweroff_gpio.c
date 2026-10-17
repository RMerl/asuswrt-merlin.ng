// SPDX-License-Identifier: GPL-2.0+
/*
 * Toggles a GPIO pin to power down a device
 *
 * Created using the Linux driver as reference, which
 * has been written by:
 *
 * Jamie Lentin <jm@lentin.co.uk>
 * Andrew Lunn <andrew@lunn.ch>
 *
 * Copyright (C) 2012 Jamie Lentin
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <log.h>
#include <sysreset.h>

#include <asm/gpio.h>
#include <linux/delay.h>

struct poweroff_gpio_info {
	struct gpio_desc gpio;
	u32 active_delay_ms;
	u32 inactive_delay_ms;
	u32 timeout_ms;
};

static int poweroff_gpio_request(struct udevice *dev, enum sysreset_t type)
{
	struct poweroff_gpio_info *priv = dev_get_priv(dev);
	int r;

	if (type != SYSRESET_POWER_OFF)
		return -EPROTONOSUPPORT;

	debug("GPIO poweroff\n");

	/* drive it active, also inactive->active edge */
	r = dm_gpio_set_value(&priv->gpio, 1);
	if (r < 0)
		return r;
	mdelay(priv->active_delay_ms);

	/* drive inactive, also active->inactive edge */
	r = dm_gpio_set_value(&priv->gpio, 0);
	if (r < 0)
		return r;
	mdelay(priv->inactive_delay_ms);

	/* drive it active, also inactive->active edge */
	r = dm_gpio_set_value(&priv->gpio, 1);
	if (r < 0)
		return r;

	/* give it some time */
	mdelay(priv->timeout_ms);

	return -EINPROGRESS;
}

static int poweroff_gpio_probe(struct udevice *dev)
{
	struct poweroff_gpio_info *priv = dev_get_priv(dev);
	int flags;

	flags = dev_read_bool(dev, "input") ? GPIOD_IS_IN : GPIOD_IS_OUT;
	priv->active_delay_ms = dev_read_u32_default(dev, "active-delay-ms", 100);
	priv->inactive_delay_ms = dev_read_u32_default(dev, "inactive-delay-ms", 100);
	priv->timeout_ms = dev_read_u32_default(dev, "timeout-ms", 3000);

	return gpio_request_by_name(dev, "gpios", 0, &priv->gpio, flags);
}

static struct sysreset_ops poweroff_gpio_ops = {
	.request = poweroff_gpio_request,
};

static const struct udevice_id poweroff_gpio_ids[] = {
	{ .compatible = "gpio-poweroff", },
	{},
};

U_BOOT_DRIVER(poweroff_gpio) = {
	.name		= "poweroff-gpio",
	.id		= UCLASS_SYSRESET,
	.ops		= &poweroff_gpio_ops,
	.probe		= poweroff_gpio_probe,
	.priv_auto	= sizeof(struct poweroff_gpio_info),
	.of_match	= poweroff_gpio_ids,
};
