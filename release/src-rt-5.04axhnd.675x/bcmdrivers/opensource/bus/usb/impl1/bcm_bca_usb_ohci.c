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
#include <linux/usb/ohci_pdriver.h>
#include <linux/of_device.h>
#include <linux/of_address.h>

#include <board.h>

struct bcm_bca_ohci_data {
    struct platform_device *pdev;
    struct platform_device *ohci;
};

static struct of_device_id const bcm_bca_usb_ohci_of_match[] = {
    { .compatible = "brcm,bcmbca-ohci" },
    {}
};

MODULE_DEVICE_TABLE(of, bcm_bca_usb_ohci_of_match);
static int bcm_bca_usb_ohci_probe(struct platform_device *pdev)
{
    int ret;
    unsigned int port_id;
    struct usb_ohci_pdata *bca_ohci_pdata = NULL;
    struct bcm_bca_ohci_data *bca_pdata = NULL;
    struct resource bca_res[2];
    bool coherency_enable = false;

    if (of_property_read_u32(pdev->dev.of_node, "usb_port-id", &port_id)) 
    {
        dev_err(&pdev->dev, "Missing usb_port-id OF property\n");
        ret = -EINVAL;
        goto error;
    }

    bca_pdata = devm_kzalloc(&pdev->dev, sizeof(*bca_pdata), GFP_KERNEL);
    if (!bca_pdata)
    {
        ret = -ENOMEM;
        goto error;
    }
    
    platform_set_drvdata(pdev, bca_pdata);

    bca_ohci_pdata = devm_kzalloc(&pdev->dev, sizeof(*bca_ohci_pdata), GFP_KERNEL);
    if (!bca_ohci_pdata)
    {
        ret = -ENOMEM;
        goto error;
    }

    bca_pdata->pdev = pdev;
    bca_pdata->ohci = platform_device_alloc("ohci-platform", port_id);
    if (!bca_pdata->ohci)
    {
        dev_err(&pdev->dev, "Failed to allocate platform device for ohci-platform port %d\n", port_id);
        ret = -ENOMEM;
        goto error;
    }

    memset(bca_res, 0, sizeof(bca_res));

    bca_res[0].name = pdev->resource[0].name;
    bca_res[0].start = pdev->resource[0].start;
    bca_res[0].end = pdev->resource[0].end;
    bca_res[0].flags = pdev->resource[0].flags;

    bca_res[1].name = pdev->resource[1].name;
    bca_res[1].start = pdev->resource[1].start;
    bca_res[1].end = pdev->resource[1].end;
    bca_res[1].flags = pdev->resource[1].flags;

    platform_device_add_resources(bca_pdata->ohci, bca_res, 2);

#if defined(CONFIG_BCM_GLB_COHERENCY) 
    if (of_property_read_bool(pdev->dev.of_node, "coherency_enable"))
        coherency_enable = true;
#endif
    arch_setup_dma_ops(&bca_pdata->ohci->dev, 0,0, NULL, coherency_enable);
    dma_coerce_mask_and_coherent(&bca_pdata->ohci->dev, DMA_BIT_MASK(32));

    bca_pdata->ohci->dev.platform_data = bca_ohci_pdata;

    if (platform_device_add(bca_pdata->ohci))
    {
        dev_err(&pdev->dev, "Failed to add platform device for ohci-platform port %d\n", port_id);

        ret = -EIO;
        goto error;
    }

    dev_info(&pdev->dev, "registered successfully\n");
    return 0;

error:
    if (bca_pdata)
    {
        if (bca_pdata->ohci)
        {
           platform_device_del(bca_pdata->ohci);
           /* Platform data is free by platform_device_release to avoid double free crash */
           bca_ohci_pdata = NULL;
        }

        if (bca_ohci_pdata)
            devm_kfree(&pdev->dev, bca_ohci_pdata);

        platform_set_drvdata(pdev, NULL);
        devm_kfree(&pdev->dev, bca_pdata);
    }

    return ret;
}

static int bcm_bca_usb_ohci_remove(struct platform_device *pdev)
{
    struct bcm_bca_ohci_data *bca_pdata = platform_get_drvdata(pdev);
    platform_device_unregister(bca_pdata->ohci);
    return 0;
}

static struct platform_driver bcm_bca_usb_ohci_driver = {
    .driver = {
        .name = "bcm-bca-usb-ohci",
        .of_match_table = bcm_bca_usb_ohci_of_match,
    },
    .probe = bcm_bca_usb_ohci_probe,
    .remove = bcm_bca_usb_ohci_remove,
};

#ifdef CONFIG_BCM_BCA_USB_MODULE
int bcmbca_usb_ohci_drv_reg(void)
{
	return platform_driver_register(&bcm_bca_usb_ohci_driver);
}
#else /* ! CONFIG_BCM_BCA_USB_MODULE */
static int __init bcmbca_usb_ohci_drv_reg(void)
{
	return platform_driver_register(&bcm_bca_usb_ohci_driver);
}

device_initcall(bcmbca_usb_ohci_drv_reg);

MODULE_AUTHOR("Samyon Furman (samyon.furman@broadcom.com)");
MODULE_DESCRIPTION("Broadcom BCA USB OHCI Driver");
MODULE_LICENSE("GPL v2");
#endif /* CONFIG_BCM_BCA_USB_MODULE */