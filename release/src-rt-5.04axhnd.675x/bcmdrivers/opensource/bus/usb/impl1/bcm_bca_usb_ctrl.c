/*
<:copyright-BRCM:2020:DUAL/GPL:standard 

   Copyright (c) 2020 Broadcom 
   All Rights Reserved

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License, version 2, as published by
the Free Software Foundation (the "GPL").

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.


A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.

:>
*/
#include <linux/types.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/dma-mapping.h>
#include <linux/platform_device.h>
#include <linux/clkdev.h>
#include <linux/delay.h>
#include <linux/clk.h>
#include <linux/bug.h>
#include <linux/of_device.h>
#include <linux/of_address.h>

#include <pmc_usb.h>
#include "bcm_bca_usb_ctrl.h"
#include "bcm_bca_usb_utils.h"

#ifdef CONFIG_BCM_BCA_USB_MODULE
static bool usb3_enable = 1;
module_param(usb3_enable, bool, S_IRUGO);
#endif /* CONFIG_BCM_BCA_USB_MODULE */

static struct of_device_id const bcm_bca_usb_ctrl_of_match[] = {
    { .compatible = "brcm,bcmbca-usb-ctrl" },
    {}
};

MODULE_DEVICE_TABLE(of, bcm_bca_usb_ctrl_of_match);

static int bcm_bca_usb_ctrl_probe(struct platform_device *pdev)
{
    struct device *dev = &pdev->dev;
    struct bcm_bca_usb_ctrl *bca_usb = NULL;
    struct resource *res;
    int ret;

    bca_usb = devm_kzalloc(dev, sizeof(*bca_usb), GFP_KERNEL);
    if (!bca_usb)
    {
        ret = -ENOMEM;
        goto error;
    }
    bca_usb->pdev = pdev;
    platform_set_drvdata(pdev, bca_usb);
    
    res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "usb-ctrl");
    if (!res)
    {
        dev_err(dev, "Failed to find usb-ctrl resource\n");
        ret = -EINVAL;
        goto error;
    }
    bca_usb->usb_ctrl = devm_ioremap_resource(dev, res);
    if (IS_ERR(bca_usb->usb_ctrl)) 
    {
        dev_err(dev, "Failed to map the usb-ctrl resource\n");
        ret = -ENXIO;
        goto error;
    }

    bca_usb->xhci_enable = of_property_read_bool(pdev->dev.of_node, "xhci-enable");

#ifdef CONFIG_BCM_BCA_USB_MODULE
    if (!usb3_enable)
        bca_usb->xhci_enable = 0;
    printk("%s: usb3_enable %d xhci_enable %d\n", __FUNCTION__, usb3_enable, bca_usb->xhci_enable);
#endif /* CONFIG_BCM_BCA_USB_MODULE */

    bca_usb->pwrflt_p_high = of_property_read_bool(pdev->dev.of_node, "pwrflt-bias-pull-up");
    bca_usb->pwron_p_high = of_property_read_bool(pdev->dev.of_node, "pwron-bias-pull-up");
    
    if (hw_init(bca_usb))
        goto error;
    
    return 0;
error:
    if(bca_usb)
    {
        platform_set_drvdata(pdev, NULL);
        devm_kfree(dev, bca_usb);
    }
    return ret;
}

static int bcm_bca_usb_ctrl_remove(struct platform_device *pdev)
{
    struct bcm_bca_usb_ctrl *bca_usb = platform_get_drvdata(pdev);
    hw_uninit(bca_usb);
    return 0;
}

static struct platform_driver bcm_bca_usb_ctrl_driver = {
    .driver = {
        .name = "bcm-bca-usb-ctrl",
        .of_match_table = bcm_bca_usb_ctrl_of_match,
    },
    .probe = bcm_bca_usb_ctrl_probe,
    .remove = bcm_bca_usb_ctrl_remove,
};

#ifdef CONFIG_BCM_BCA_USB_MODULE
extern int bcmbca_usb_xhci_drv_reg(void);
extern int bcmbca_usb_ehci_drv_reg(void);
extern int bcmbca_usb_ohci_drv_reg(void);

static int __init bcmbca_usb_ctrl_drv_reg(void)
{
	int ret;

	if ((ret = platform_driver_register(&bcm_bca_usb_ctrl_driver)))
		return ret;

#if defined(CONFIG_BCM94912)
	bcmbca_usb_xhci_drv_reg();
#else
	if (usb3_enable)
		bcmbca_usb_xhci_drv_reg();
#endif
	bcmbca_usb_ehci_drv_reg();
	bcmbca_usb_ohci_drv_reg();

	return ret;
}
module_init(bcmbca_usb_ctrl_drv_reg);
#else /* ! CONFIG_BCM_BCA_USB_MODULE */
static int __init bcmbca_usb_ctrl_drv_reg(void)
{
	return platform_driver_register(&bcm_bca_usb_ctrl_driver);
}

device_initcall(bcmbca_usb_ctrl_drv_reg);
#endif /* CONFIG_BCM_BCA_USB_MODULE */

MODULE_AUTHOR("Samyon Furman (samyon.furman@broadcom.com)");
MODULE_DESCRIPTION("Broadcom BCA USB CTRL Driver");
MODULE_LICENSE("GPL v2");
