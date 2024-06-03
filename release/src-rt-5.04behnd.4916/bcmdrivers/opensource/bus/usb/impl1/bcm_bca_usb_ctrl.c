/*
<:copyright-BRCM:2020:DUAL/GPL:standard 

   Copyright (c) 2020 Broadcom 
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

#include "bcm_bca_usb_ctrl.h"
#include "bcm_bca_usb_utils.h"

bool usb_enable = 1;
#if defined(CONFIG_USB_XHCI_PLATFORM) || defined(CONFIG_USB_XHCI_PLATFORM_MODULE)
bool usb3_enable = 1;
#else
bool usb3_enable = 0;
#endif

module_param(usb3_enable, bool, S_IRUGO);

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

    /* if usb3 is disabled in device tree keep it disabled
     * but if it's enabled, then use the module param to enable or disable usb3 
     */
    if (bca_usb->xhci_enable)
    {
        if (!usb3_enable)
            bca_usb->xhci_enable = 0;
    }
    else
        /*clear this to avoid xhci init when ehci/ohci are present */
        usb3_enable = 0; 

    bca_usb->pwrflt_p_high = of_property_read_bool(pdev->dev.of_node, "pwrflt-bias-pull-up");
    bca_usb->pwron_p_high = of_property_read_bool(pdev->dev.of_node, "pwron-bias-pull-up");
    
    bca_usb->port0_enable = !of_property_read_bool(pdev->dev.of_node, "port0-disabled");
    bca_usb->port1_enable = !of_property_read_bool(pdev->dev.of_node, "port1-disabled");

    if (hw_init(bca_usb))
    {
        usb_enable = 0; 
        goto error;
    }
    
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
    mdelay(1);
    platform_set_drvdata(pdev, NULL);
    devm_kfree(&pdev->dev, bca_usb);

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

int __init bcmbca_usb_xhci_drv_reg(void);

static int __init bcmbca_usb_ctrl_drv_reg(void)
{
    int rc = 0;

    rc = platform_driver_register(&bcm_bca_usb_ctrl_driver);
    rc = rc ? rc : bcmbca_usb_xhci_drv_reg();

	return rc; 
}

void bcmbca_usb_xhci_drv_unreg(void);

static void bcmbca_usb_ctrl_drv_cleanup(void)
{
    bcmbca_usb_xhci_drv_unreg();
    mdelay(1);
    platform_driver_unregister(&bcm_bca_usb_ctrl_driver);
}

module_init(bcmbca_usb_ctrl_drv_reg);
module_exit(bcmbca_usb_ctrl_drv_cleanup);

MODULE_AUTHOR("Samyon Furman (samyon.furman@broadcom.com)");
MODULE_DESCRIPTION("Broadcom BCA USB CTRL Driver");
MODULE_LICENSE("GPL v2");
