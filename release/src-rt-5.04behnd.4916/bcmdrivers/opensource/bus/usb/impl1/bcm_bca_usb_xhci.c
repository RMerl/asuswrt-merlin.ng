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
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,20,0))
#include <linux/dma-map-ops.h>
#endif

#include <pmc_usb.h>
#include <board.h>
#include "bcm_bca_usb_utils.h"
#include "bcmtypes.h"

#define USB_XHCI_GBL_GUCTL 0x2c
#define XHCI_REFCLKPER_MASK 	0xffc00000
#define XHCI_REFCLKPER_125ms 	0x0a000000

#define USB_XHCI_GBL_GFLADJ 0x530
#define REFCLK_LPM_SEL  (1<<23)

#define USB_XHCI_GBL_GUCTL4 0x534
#define CSR_TIMEOUT_VAL  0xefff 

struct bcm_bca_xhci_data {
    struct platform_device *pdev;
    struct platform_device *xhci;
};

extern bool usb_enable;
extern bool usb3_enable;

static void erdy_nump_bypass(void __iomem *base_addr)
{
    uint32_t value;

    value = xhci_ecira_read((uint32_t *)base_addr, 0xa20c);
    value |= 0x10000;
    xhci_ecira_write((uint32_t *)base_addr, 0xa20c, value);
}

#if defined(CONFIG_USB_UAS) || defined(CONFIG_USB_UAS_MODULE)
static void  uas_fix(void __iomem *base_addr)
{
    /* workaround for UAS to work.*/
    volatile uint32_t* addr = &((uint32_t*)base_addr)[4];
    *addr &= 0x7fffffff;
}
#endif

static void enable_recovery_pipe_reset(void __iomem *base_addr)
{
    uint32_t value, reg;
    int ii;

    reg = 0xc410;
    for (ii = 0; ii < 2; ++ii)
    {
        value = xhci_ecira_read((uint32_t *)base_addr, reg);
        value |= (1 << 29);
        xhci_ecira_write((uint32_t *)base_addr, reg, value);
	reg += 0x40;
    }
}

static struct of_device_id const bcm_bca_usb_xhci_of_match[] = {
    { .compatible = "brcm,bcmbca-xhci" },
    {}
};

MODULE_DEVICE_TABLE(of, bcm_bca_usb_xhci_of_match);
static int bcm_bca_usb_xhci_probe(struct platform_device *pdev)
{
    int ret;
    struct bcm_bca_xhci_data *bca_pdata = NULL;
    struct resource *res;
    bool coherency_enable = false;
    void __iomem *xhci_ecira_base = NULL;

    static const struct property_entry properties[] = {
        PROPERTY_ENTRY_BOOL("usb3-lpm-capable"),
        { }
    };
#if (LINUX_VERSION_CODE > KERNEL_VERSION(4, 20, 0))

    static const struct software_node usb_software_node = {
        .properties = properties,
    };
#endif

    if (!usb_enable)
        return 0;

    /* when EHCI & OHCI companion controllers are present and usb3 is disabled
     * no need to register xhci
     */ 
    if(!of_property_read_bool(pdev->dev.of_node, "xhci-no-companion") && !usb3_enable)
        return 0;

    bca_pdata = devm_kzalloc(&pdev->dev, sizeof(*bca_pdata), GFP_KERNEL);
    if (!bca_pdata)
    {
        ret = -ENOMEM;
        goto error;
    }

    platform_set_drvdata(pdev, bca_pdata);

    bca_pdata->pdev = pdev;
    bca_pdata->xhci = platform_device_alloc("xhci-hcd", 0);
    if (!bca_pdata->xhci)
    {
        dev_err(&pdev->dev, "Failed to allocate platform device for xhci-platform port\n");
        ret = -ENOMEM;
        goto error;
    }

    res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "xhci-gbl");
    if (res)
    {
        volatile void __iomem *xhci_gbl;
        uint32_t *regaddr;
        uint32_t regval;

        xhci_gbl = devm_ioremap_resource(&pdev->dev, res);
        if (IS_ERR((void*)xhci_gbl))
        {
           dev_err(&pdev->dev, "Failed to map the xhci_gbl resource\n");
           ret = -ENXIO;
           goto error;
        }

        if(!usb3_enable) {
            regaddr = (uint32_t *)(xhci_gbl + USB_XHCI_GBL_GFLADJ);
            regval = *regaddr;
            regval &= ~REFCLK_LPM_SEL;
            *regaddr = regval;
        }

        /* adjust csr timeout to be shorted than ubus timeout */
        regaddr = (uint32_t *)(xhci_gbl + USB_XHCI_GBL_GUCTL4);
        *regaddr = CSR_TIMEOUT_VAL;
    }
    
    res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "usb-xhci");
    if (!res)
    {
        dev_err(&pdev->dev, "Failed to find usb-xhci resource\n");
        ret = -EINVAL;
        goto error;
    }

    platform_device_add_resources(bca_pdata->xhci, pdev->resource, pdev->num_resources);
    
    if(of_property_read_bool(pdev->dev.of_node, "usb3-lpm-capable"))
    {
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 20, 0))
        platform_device_add_properties(bca_pdata->xhci, properties);
#else
        device_add_software_node(&bca_pdata->xhci->dev, &usb_software_node);
#endif
    }

#if defined(CONFIG_BCM_GLB_COHERENCY) 
    if (of_property_read_bool(pdev->dev.of_node, "coherency_enable"))
        coherency_enable = true;
#endif
    arch_setup_dma_ops(&bca_pdata->xhci->dev, 0,0, NULL, coherency_enable);
    dma_coerce_mask_and_coherent(&bca_pdata->xhci->dev, DMA_BIT_MASK(32));

    bca_pdata->xhci->dev.parent = &pdev->dev;

    if (platform_device_add(bca_pdata->xhci))
    {
        dev_err(&pdev->dev, "Failed to add platform device for xhci-platform\n");

        ret = -EIO;
        goto error;
    }

	/* xchi-no-companion is set for new XHCI controllers with
	 * integrated EHCI/OHCI support */
	if(!of_property_read_bool(pdev->dev.of_node, "xhci-no-companion"))
	{
		xhci_ecira_base = ioremap(res->start +0xf90, 0x14);
		if (IS_ERR(xhci_ecira_base)) 
		{
			dev_err(&pdev->dev, "Failed to map the xhci_ecira resource\n");
			ret = -ENXIO;
			goto error;
		}

		if (!of_property_read_bool(pdev->dev.of_node, "skip_erdy_nump_bypass"))
			erdy_nump_bypass(xhci_ecira_base);

#if defined(CONFIG_USB_UAS) || defined(CONFIG_USB_UAS_MODULE)
		uas_fix(xhci_ecira_base);
#endif

		enable_recovery_pipe_reset(xhci_ecira_base);
	}

    dev_info(&pdev->dev, "registered successfully\n");
    return 0;

error:
    if (bca_pdata)
    {
        if (bca_pdata->xhci)
        {
           platform_device_del(bca_pdata->xhci);
           /* Platform data is free by platform_device_release to avoid double free crash */
        }

        platform_set_drvdata(pdev, NULL);
        devm_kfree(&pdev->dev, bca_pdata);
    }

    return ret;
}

static int bcm_bca_usb_xhci_remove(struct platform_device *pdev)
{
    struct bcm_bca_xhci_data *bca_pdata = platform_get_drvdata(pdev);

    if(bca_pdata) 
    {
        platform_device_del(bca_pdata->xhci);
        platform_set_drvdata(pdev, NULL);
        devm_kfree(&pdev->dev, bca_pdata);
    }

    return 0;
}

static struct platform_driver bcm_bca_usb_xhci_driver = {
    .driver = {
        .name = "bcm-bca-usb-xhci",
        .of_match_table = bcm_bca_usb_xhci_of_match,
    },
    .probe = bcm_bca_usb_xhci_probe,
    .remove = bcm_bca_usb_xhci_remove,
};

int __init bcmbca_usb_xhci_drv_reg(void)
{
	return platform_driver_register(&bcm_bca_usb_xhci_driver);
}

void bcmbca_usb_xhci_drv_unreg(void)
{
    platform_driver_unregister(&bcm_bca_usb_xhci_driver);
}

MODULE_AUTHOR("Samyon Furman (samyon.furman@broadcom.com)");
MODULE_DESCRIPTION("Broadcom BCA USB XHCI Driver");
MODULE_LICENSE("GPL v2");
