/*
 * <:copyright-BRCM:2019:DUAL/GPL:standard
 * 
 *    Copyright (c) 2019 Broadcom 
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
#include <linux/leds.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#include <linux/of_address.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/version.h>
#include <linux/notifier.h>
#include <linux/reboot.h>

static const struct of_device_id bca_vreg_sync_of_match[] = {
    { .compatible = "brcm,vreg-sync", .data = NULL, },
    {},
};

MODULE_DEVICE_TABLE(of, bca_vreg_sync_of_match);

struct vreg_sync_info
{
	struct notifier_block reboot_nb;
	int gpio;
}vreg_struct_info;

static int vreg_reboot_handler(struct notifier_block *this,
		unsigned long mode, void *cmd)
{

	struct vreg_sync_info *vsi;

	vsi=container_of(this, struct vreg_sync_info, reboot_nb);
	if(vsi != NULL && vsi->gpio != -1)
	{
		gpio_request(vsi->gpio, "vreg_sync_gpio");
	}
	return NOTIFY_DONE;
}
static struct notifier_block reboot_nb = {
	.notifier_call = vreg_reboot_handler,
	.priority = 80,
};

static int bca_vreg_sync_probe(struct platform_device *pdev)
{
struct gpio_desc *desc;

	desc = devm_gpiod_get_optional(&pdev->dev, "power_sync", 0);
	if (desc)
	{
		gpiod_direction_output(desc, 1);
	}


	if(of_property_read_u32(pdev->dev.of_node, "vreg_sync_gpio", &vreg_struct_info.gpio))
			vreg_struct_info.gpio=-1;

	vreg_struct_info.reboot_nb = reboot_nb;
	register_reboot_notifier(&vreg_struct_info.reboot_nb);

	return 0;
}

static struct platform_driver bcm_bca_vreg_sync_driver = {
	.probe = bca_vreg_sync_probe,
	.driver = {
		.name = "bcm-bca-vreg-sync",
		.of_match_table = bca_vreg_sync_of_match,
	},
};
extern void bcm_set_vreg_sync(void);
static int __init bcmbca_vreg_sync_drv_reg(void)
{
	bcm_set_vreg_sync();
	return platform_driver_register(&bcm_bca_vreg_sync_driver);
}

subsys_initcall_sync(bcmbca_vreg_sync_drv_reg);

MODULE_AUTHOR("Anand Gore (anand.gore@broadcom.com)");
MODULE_DESCRIPTION("Broadcom BCA Legacy Vreg Sync Driver");
MODULE_LICENSE("GPL v2");
