/*
 * Toggles a GPIO pin to power down a device
 *
 * Jamie Lentin <jm@lentin.co.uk>
 * Andrew Lunn <andrew@lunn.ch>
 *
 * Copyright (C) 2012 Jamie Lentin
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/gpio/consumer.h>
#include <linux/of_platform.h>
#include <linux/module.h>

/*
 * Hold configuration here, cannot be more than one instance of the driver
 * since pm_power_off itself is global.
 */
static struct gpio_desc *reset_gpio;

static void gpio_poweroff_do_poweroff(void)
{
	BUG_ON(!reset_gpio);

	/* drive it active, also inactive->active edge */
	gpiod_direction_output(reset_gpio, 1);
	mdelay(100);
	/* drive inactive, also active->inactive edge */
	gpiod_set_value(reset_gpio, 0);
	mdelay(100);

	/* drive it active, also inactive->active edge */
	gpiod_set_value(reset_gpio, 1);

	/* give it some time */
	mdelay(3000);

	WARN_ON(1);
}

static int gpio_poweroff_probe(struct platform_device *pdev)
{
	bool input = false;

	/* If a pm_power_off function has already been added, leave it alone */
	if (pm_power_off != NULL) {
		dev_err(&pdev->dev,
			"%s: pm_power_off function already registered",
		       __func__);
		return -EBUSY;
	}

	reset_gpio = devm_gpiod_get(&pdev->dev, NULL);
	if (IS_ERR(reset_gpio))
		return PTR_ERR(reset_gpio);

	input = of_property_read_bool(pdev->dev.of_node, "input");

	if (input) {
		if (gpiod_direction_input(reset_gpio)) {
			dev_err(&pdev->dev,
				"Could not set direction of reset GPIO to input\n");
			return -ENODEV;
		}
	} else {
		if (gpiod_direction_output(reset_gpio, 0)) {
			dev_err(&pdev->dev,
				"Could not set direction of reset GPIO\n");
			return -ENODEV;
		}
	}

	pm_power_off = &gpio_poweroff_do_poweroff;
	return 0;
}

static int gpio_poweroff_remove(struct platform_device *pdev)
{
	if (pm_power_off == &gpio_poweroff_do_poweroff)
		pm_power_off = NULL;

	return 0;
}

static const struct of_device_id of_gpio_poweroff_match[] = {
	{ .compatible = "gpio-poweroff", },
	{},
};

static struct platform_driver gpio_poweroff_driver = {
	.probe = gpio_poweroff_probe,
	.remove = gpio_poweroff_remove,
	.driver = {
		.name = "poweroff-gpio",
		.of_match_table = of_gpio_poweroff_match,
	},
};

module_platform_driver(gpio_poweroff_driver);

MODULE_AUTHOR("Jamie Lentin <jm@lentin.co.uk>");
MODULE_DESCRIPTION("GPIO poweroff driver");
MODULE_LICENSE("GPL v2");
MODULE_ALIAS("platform:poweroff-gpio");
