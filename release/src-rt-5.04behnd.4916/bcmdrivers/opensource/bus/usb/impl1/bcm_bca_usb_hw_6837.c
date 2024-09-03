/*
<:copyright-BRCM:2022:DUAL/GPL:standard 

   Copyright (c) 2022 Broadcom 
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
#include "bcmtypes.h"

#include <bp3_license.h>

extern bool usb_enable;

/* USB Host contorl regs */
typedef struct usb_ctrl {
    uint32 setup;
#define USBH_IPP                (1<<5)
#define USBH_IOC                (1<<4)
#define USBH_STRAP_IPP_SEL      (1<<25)
#define USB2_OC_DISABLE_PORT0   (1<<28)
#define USB2_OC_DISABLE_PORT1   (1<<29)
#define USB3_OC_DISABLE_PORT0   (1<<30)
#define USB3_OC_DISABLE_PORT1   (1<<31)

    uint32 usb_pm;
#define XHC_SOFT_RESETB         (1<<22)
#define U3XPHY_PWRDWN0			(1<<24)
#define U3XPHY_PWRDWN1			(1<<25)
#define U2PHY_PWRDWN0			(1<<26)
#define U2PHY_PWRDWN1			(1<<27)
#define USB_PWRDWN              (1<<31)
    uint32 ignore1[18];
    uint32 ctlr_cshcr1;
	/*bits 22,23 */
#define HOST_NUM_U3_PORT_MASK	(3<<22)
    uint32 ignore2[2];
    uint32 ctlr_cshcr2;
#define HOST_U3_PORT_DISABLE_P0 (1<<5)
#define HOST_U3_PORT_DISABLE_P1 (1<<21)
} usb_ctrl_t;

int hw_init(struct bcm_bca_usb_ctrl *bca_usb)
{
    usb_ctrl_t *usb_ctrl = (usb_ctrl_t *)bca_usb->usb_ctrl;
    int port0_enable, port1_enable;

    dev_info(&bca_usb->pdev->dev, "---- Powering up USB blocks\n");

    port0_enable = bca_usb->port0_enable &&
        bcm_license_check_msg(BP3_FEATURE_USB0) >= 0 &&
        !pmc_usb_power_up(PMC_USB_PORT_0);
    port1_enable = bca_usb->port1_enable &&
        bcm_license_check_msg(BP3_FEATURE_USB1) >= 0 &&
        !pmc_usb_power_up(PMC_USB_PORT_1);

    if (!port0_enable && !port1_enable)
    {
        usb_enable = 0;
        return 0;
    }

    mdelay(1);

    if (bca_usb->pwrflt_p_high)
        usb_ctrl->setup &= ~(USBH_IOC);
    else
        usb_ctrl->setup |= USBH_IOC;

    /*by default we use strap to determine polarity of port power */
    if (!(usb_ctrl->setup & USBH_STRAP_IPP_SEL)){
        if (bca_usb->pwron_p_high)
            usb_ctrl->setup &= ~(USBH_IPP);
        else
            usb_ctrl->setup |= USBH_IPP;
    }

    mdelay(1);

	/*enable USB PHYs*/
    if (!bca_usb->xhci_enable) {
        usb_ctrl->ctlr_cshcr2 |= (HOST_U3_PORT_DISABLE_P0 | HOST_U3_PORT_DISABLE_P1);
        usb_ctrl->ctlr_cshcr1 &= ~(HOST_NUM_U3_PORT_MASK);
        usb_ctrl->usb_pm |= (U3XPHY_PWRDWN0 | U3XPHY_PWRDWN1);
    }
    if (!port0_enable)
        usb_ctrl->usb_pm |= (U2PHY_PWRDWN0| U3XPHY_PWRDWN0);
    if (!port1_enable)
        usb_ctrl->usb_pm |= (U2PHY_PWRDWN1| U3XPHY_PWRDWN1);
    mdelay(1);
    usb_ctrl->usb_pm &= ~(USB_PWRDWN);
    mdelay(10);
    usb_ctrl->usb_pm |= XHC_SOFT_RESETB;
    mdelay(10);

    return 0;
}

void hw_uninit(struct bcm_bca_usb_ctrl *bca_usb)
{
    pmc_usb_power_down(PMC_USB_HOST_ALL);
    mdelay(1);
}
