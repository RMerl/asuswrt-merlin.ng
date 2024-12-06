/*
 * <:copyright-BRCM:2023:DUAL/GPL:standard
 * 
 *    Copyright (c) 2023 Broadcom 
 *    All Rights Reserved
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
