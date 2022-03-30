/*********************************************************************
 * voice PSTN relay driver.c
 *
 * Copyright (c) 2020 Broadcom Corporation
 * All Rights Reserved
 *
 * <:label-BRCM:2020:DUAL/GPL:standard
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2, as published by
 * the Free Software Foundation (the "GPL").
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * 
 * A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
 * writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 * 
 * :>
 **********************************************************************/

#include <linux/of.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/slab.h>
#include <sound/initval.h>
#include <sound/pcm.h>
#include <sound/soc.h>
#include <linux/of_gpio.h>
#include <boardparms_voice.h>

#define DRV_NAME "brcm-dect-rf-reset"

struct bcm_dect_reset_data {
	struct gpio_desc *ctrl_gpio;
	struct device *dev;
};

static struct bcm_dect_reset_data *priv;

static int dect_reset_probe(struct platform_device *pdev)
{
	struct device_node *np = pdev->dev.of_node;
	if (!np)
		return -ENODEV;

	priv = devm_kzalloc(&pdev->dev, sizeof(*priv), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	priv->ctrl_gpio = devm_gpiod_get_optional(&pdev->dev, "dectreset", 0);
	if (IS_ERR(priv->ctrl_gpio))
		return PTR_ERR(priv->ctrl_gpio);

	priv->dev = &pdev->dev;

	return 0;
}

int voice_dect_reset(int interval)
{
	if(!priv)
		return -ENOMEM;

	if (priv->ctrl_gpio) {
		gpiod_direction_output(priv->ctrl_gpio, 0);
		msleep(5);
		gpiod_direction_output(priv->ctrl_gpio, 1);
		msleep(interval);
		gpiod_direction_output(priv->ctrl_gpio, 0);
		dev_notice(priv->dev,"DECT reset applied on GPIO %d.\n", desc_to_gpio(priv->ctrl_gpio));
	}
	return 0;

}
EXPORT_SYMBOL(voice_dect_reset);

int voice_get_dect_type(void)
{
	if (!priv->ctrl_gpio)
		return BP_VOICE_NO_DECT;

	return BP_VOICE_INT_DECT;

}
EXPORT_SYMBOL(voice_get_dect_type);

#ifdef CONFIG_OF
static const struct of_device_id bcm_dect_reset_match[] = 
{
	{.compatible = "brcm,voice-dect-reset" },
	{ }
	
};
#endif
static struct platform_driver dect_reset_driver = {
	.probe = dect_reset_probe,
	.driver = {
		.name = "dect_reset",
		.of_match_table = of_match_ptr(bcm_dect_reset_match),
	},
};

module_platform_driver(dect_reset_driver);

MODULE_AUTHOR("Kevin Li <kevin-ke.li@broadcom.com>");
MODULE_DESCRIPTION("BCM dect_reset driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:" DRV_NAME);
