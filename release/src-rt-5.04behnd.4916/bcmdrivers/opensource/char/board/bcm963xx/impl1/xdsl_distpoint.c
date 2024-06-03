 /*
<:copyright-BRCM:2019:DUAL/GPL:standard 

   Copyright (c) 2019 Broadcom 
   All Rights Reserved

Unless you and Broadcom execute a separate written software license
agreement governing use of this software, this software is licensed
to you under the terms of the GNU General Public License version 2
(the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
with the following added to such license:

   As a special exception, the copyright holders of this software give
   you permission to link this software with independent modules, and
   to copy and distribute the resulting executable under terms of your
   choice, provided that you also meet, for each linked independent
   module, the terms and conditions of the license of that module.
   An independent module is a module which is not derived from this
   software.  The special exception does not apply to any modifications
   of the software.

Not withstanding the above, under no circumstances may you combine
this software in any way with any other Broadcom software provided
under a license other than the GPL, without Broadcom's express prior
written consent.

:>
*/

/*
 *  Created on: Nov/2019
 *      Author: ido.brezel@broadcom.com
 */

#include <linux/device.h>
#include <linux/of_gpio.h>
#include <linux/of_gpio.h>
#include <linux/platform_device.h>
#include <linux/version.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0)
#include <linux/gpio/consumer.h>
#include <linux/module.h>
#endif

#define RESET_PREFIX "reset-"

struct xdsldistpoint_gpio
{
    struct gpio_desc *gpio;
    const char *name;
};

struct xdsldistpoint_data
{
    struct xdsldistpoint_gpio *gpios;
    int gpio_amount;
};

static int is_reset_gpio(const char *name)
{
    return !strncmp(RESET_PREFIX, name, strlen(RESET_PREFIX));
}

static int _probe_gpio(struct device *dev)
{
    struct device_node *np = dev->of_node;
    struct xdsldistpoint_data *distpoint = dev_get_drvdata(dev);
    int gpio_count, gpio_name_count, i, err;

    gpio_count = of_gpio_count(np);

    if (gpio_count < 0)
    {
        dev_err(dev, "missing gpios: %d\n", gpio_count);
        return gpio_count;
    }

    gpio_name_count = of_property_count_strings(np, "gpio-names");

    if (gpio_count != gpio_name_count)
    {
        dev_err(dev, "number of gpios does not equal number of gpio names\n");
        return -EINVAL;
    }

    distpoint->gpios = devm_kcalloc(dev, gpio_count, sizeof(*distpoint->gpios), GFP_KERNEL);
    if (!distpoint->gpios)
        return -ENOMEM;

    distpoint->gpio_amount = gpio_count;

    for (i = 0; i < gpio_count; i++)
    {
        distpoint->gpios[i].gpio = devm_gpiod_get_index(dev, NULL, i, GPIOD_ASIS);
        if (IS_ERR(distpoint->gpios[i].gpio))
        {
            dev_err(dev, "Could not get gpio %d\n", i);
            return PTR_ERR(distpoint->gpios[i].gpio);
        }

        err = of_property_read_string_index(np, "gpio-names", i, &(distpoint->gpios[i].name));
        if (err)
        {
            dev_err(dev, "Could not get gpio name %d\n", i);
            return err;
        }

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0)
        gpiod_set_consumer_name(distpoint->gpios[i].gpio, distpoint->gpios[i].name);
#endif

        err = gpiod_export(distpoint->gpios[i].gpio, 0);
        if (err)
            return err;

        err = gpiod_export_link(dev, distpoint->gpios[i].name,
                distpoint->gpios[i].gpio);
        if (err)
            return err;
    }

    return 0;
}

static int _probe(struct platform_device *pdev)
{
    struct device *dev = &pdev->dev;
    struct device_node *np = dev->of_node;
    struct xdsldistpoint_data *distpoint;
    int err, i;
    u32 *defaults;

    if (!np)
    {
        dev_err(dev, "device tree node not found\n");
        return -ENXIO;
    }

    distpoint = devm_kzalloc(dev, sizeof(*distpoint), GFP_KERNEL);
    if (!distpoint)
        return -ENOMEM;

    dev_set_drvdata(dev, distpoint);

    err = _probe_gpio(dev);
    if (err < 0)
    {
        dev_err(dev, "Could not probe GPIOs\n");
        return err;
    }

    defaults = devm_kcalloc(dev, distpoint->gpio_amount, sizeof(*defaults), GFP_KERNEL);
    if (!defaults)
        return -ENOMEM;

    if (of_property_read_u32_array(np, "gpio-default-val", defaults, distpoint->gpio_amount))
    {
        dev_err(dev, "Error reading defaults\n");
        return -ENODEV;
    }

    /* reset pins active first, set other pins to their default */
    for (i = 0; i < distpoint->gpio_amount; i++)
    {
        int val = defaults[i];

        if (is_reset_gpio(distpoint->gpios[i].name))
            val = 1;

        dev_info(dev, "gpio value on %d val %d\n", desc_to_gpio(distpoint->gpios[i].gpio), val);
        gpiod_direction_output(distpoint->gpios[i].gpio, val);
    }

    /* reset pins default values */
    for (i = 0; i < distpoint->gpio_amount; i++)
    {
        if (is_reset_gpio(distpoint->gpios[i].name) && defaults[i] == 0)
        {
            gpiod_direction_output(distpoint->gpios[i].gpio, 0);
            dev_info(dev, "release on init %d val %d\n", desc_to_gpio(distpoint->gpios[i].gpio), defaults[i]);
        }
    }

    devm_kfree(dev, defaults);
    dev_info(dev, "initialized\n");

    return 0;
}

static void _gpio_unexport(struct device *dev)
{
    struct xdsldistpoint_data *distpoint = dev_get_drvdata(dev);
    int i;

    for (i = 0; i < distpoint->gpio_amount; i++)
    {
        sysfs_remove_link(&dev->kobj, distpoint->gpios[i].name);
        gpiod_unexport(distpoint->gpios[i].gpio);
    }
}

static int _remove(struct platform_device *pdev)
{
    struct xdsldistpoint_data *distpoint = platform_get_drvdata(pdev); 
    struct device *dev = &pdev->dev;

    if (!distpoint)
        return 0;

    _gpio_unexport(dev);
    dev_set_drvdata(dev, NULL);

    devm_kfree(dev, distpoint);

    dev_dbg(dev, "Removed...\n");

    return 0;
}

const struct of_device_id of_platform_table[] = {
    { .compatible = "brcm,xdsldistpoint", .data = (void *)0, },
    { /* end of list */ },
};

struct platform_driver of_platform_driver = {
    .driver = {
        .name = "brcm_xdsl_distpoint",
        .of_match_table = of_platform_table,
    },
    .probe = _probe,
    .remove = _remove,
};

module_platform_driver(of_platform_driver);

