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

#define DRV_NAME "brcm-pstn-relay"

struct bcm_pstn_relay_data {
	struct gpio_desc *ctrl_gpio;
	struct device *dev;
};

static struct bcm_pstn_relay_data *priv;

static int pstn_relay_probe(struct platform_device *pdev)
{
	struct device_node *np = pdev->dev.of_node;
	if (!np)
		return -ENODEV;

	priv = devm_kzalloc(&pdev->dev, sizeof(*priv), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	priv->ctrl_gpio = devm_gpiod_get_optional(&pdev->dev, "pstnrelay", 0);
	if (IS_ERR(priv->ctrl_gpio))
		return PTR_ERR(priv->ctrl_gpio);

	priv->dev = &pdev->dev;

	return 0;
}

void voice_pstn_relay_set(int enable)
{
	if (!priv)
		return;

	if (priv->ctrl_gpio) {
		dev_notice(priv->dev,"%sabling relay on pin %d\n", enable ? "en" : "dis",
					desc_to_gpio(priv->ctrl_gpio));
		gpiod_direction_output(priv->ctrl_gpio, enable);
	}
}
EXPORT_SYMBOL(voice_pstn_relay_set);

#ifdef CONFIG_OF
static const struct of_device_id bcm_pstn_relay_match[] = 
{
	{.compatible = "brcm,voice-pstn-relay" },
	{ }
	
};
#endif
static struct platform_driver pstn_relay_driver = {
	.probe = pstn_relay_probe,
	.driver = {
		.name = "pstn_relay",
		.of_match_table = of_match_ptr(bcm_pstn_relay_match),
	},
};

module_platform_driver(pstn_relay_driver);

MODULE_AUTHOR("Kevin Li <kevin-ke.li@broadcom.com>");
MODULE_DESCRIPTION("BCM pstn_relay driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:" DRV_NAME);
