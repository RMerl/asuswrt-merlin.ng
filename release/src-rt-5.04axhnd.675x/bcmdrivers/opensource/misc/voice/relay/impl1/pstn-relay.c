/*********************************************************************
 * voice PSTN relay driver.c
 *
 * Copyright (c) 2020 Broadcom Corporation
 * All Rights Reserved
 *
 * <:label-BRCM:2020:DUAL/GPL:standard
 * 
 * Unless you and Broadcom execute a separate written software license
 * agreement governing use of this software, this software is licensed
 * to you under the terms of the GNU General Public License version 2
 * (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
 * with the following added to such license:
 * 
 *    As a special exception, the copyright holders of this software give
 *    you permission to link this software with independent modules, and
 *    to copy and distribute the resulting executable under terms of your
 *    choice, provided that you also meet, for each linked independent
 *    module, the terms and conditions of the license of that module.
 *    An independent module is a module which is not derived from this
 *    software.  The special exception does not apply to any modifications
 *    of the software.
 * 
 * Not withstanding the above, under no circumstances may you combine
 * this software in any way with any other Broadcom software provided
 * under a license other than the GPL, without Broadcom's express prior
 * written consent.
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
