/*
 * <:copyright-BRCM:2023:DUAL/GPL:standard
 * 
 *    Copyright (c) 2023 Broadcom 
 *    All Rights Reserved
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
 */

#include <linux/io.h>
#include <linux/of_device.h>
#include <linux/of_address.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/gpio/consumer.h>
#include <linux/version.h>
#include <ba_rpc_svc.h>

static const struct of_device_id ext_pwr_ctrl_of_match[] = {
    { .compatible = "brcm,ext_pwr_ctrl", .data = NULL, },
    {},
};

MODULE_DEVICE_TABLE(of, ext_pwr_ctrl_of_match);


static int ext_pwr_ctrl_probe(struct platform_device *pdev)
{
    int i = 0;
    struct gpio_descs *gpios = NULL;
    struct gpio_desc *gpiod = NULL;

    gpios = devm_gpiod_get_array_optional(&pdev->dev, "pwr-ctrl", GPIOD_OUT_HIGH);
    if (IS_ERR_OR_NULL(gpios))
        goto exit;

    for (i = 0; i < gpios->ndescs; i++)
    {
        int hw_gpio;
        int polarity;

        gpiod = gpios->desc[i]; 
        hw_gpio = desc_to_gpio(gpiod);
        polarity = gpiod_is_active_low(gpiod) ? 0:1;

        printk("Setting up EXT PWR PIN %d polarity %s\n", hw_gpio, polarity ? "ACTIVE_HIGH" : "ACTIVE_LOW");

        if (bcm_rpc_ba_setup_pwr_pin(hw_gpio, polarity) != 0)
            goto exit;
    }

exit:
    return 0;
}

static struct platform_driver ext_pwr_ctrl_driver = {
	.probe = ext_pwr_ctrl_probe,
	.driver = {
		.name = "ext-pwr-ctrl",
		.of_match_table = ext_pwr_ctrl_of_match,
	},
};

static int __init ext_pwr_ctrl_drv_reg(void)
{
	return platform_driver_register(&ext_pwr_ctrl_driver);
}

late_initcall(ext_pwr_ctrl_drv_reg);

MODULE_AUTHOR("Samyon Furman (samyon.furman@broadcom.com)");
MODULE_DESCRIPTION("Broadcom EXT PWR CTRL Driver");
MODULE_LICENSE("GPL v2");
