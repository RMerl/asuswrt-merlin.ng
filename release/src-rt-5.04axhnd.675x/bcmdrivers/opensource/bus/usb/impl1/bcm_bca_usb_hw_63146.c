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

#include <pmc_usb.h>
#include "bcm_bca_usb_ctrl.h"
#include "bcm_bca_usb_utils.h"
#include "bcmtypes.h"

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
#define XHC_SOFT_RESETB         (1<<21)
#define U3XPHY_PWRDWN			(1<<23)
#define USB_PWRDWN              (1<<31)
    uint32 usb_pm_status;
    uint32 ignore1[25];
    uint32 p0_u2phy_cfg1;
#define USB2_PHY_AUTORSMENB0_P0 (1<<16)
    uint32 ignore2[21];
    uint32 p1_u2phy_cfg1;
#define USB2_PHY_AUTORSMENB0_P1	(1<<16)
    uint32 ignore3[22];
} usb_ctrl_t;

int hw_init(struct bcm_bca_usb_ctrl *bca_usb)
{
	usb_ctrl_t *usb_ctrl = (usb_ctrl_t *)bca_usb->usb_ctrl;

	dev_info(&bca_usb->pdev->dev, "---- Powering up USB blocks\n");

	if(pmc_usb_power_up(PMC_USB_HOST_ALL)){
		dev_err(&bca_usb->pdev->dev,"+++ Failed to Power Up USB Host\n");
		return -1;
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
#if (CONFIG_BRCM_CHIP_REV == 0x63146A0)
	/* disable autoresume for Full speed/OHCI */
	usb_ctrl->p0_u2phy_cfg1 &= ~(USB2_PHY_AUTORSMENB0_P0);
	usb_ctrl->p1_u2phy_cfg1 &= ~(USB2_PHY_AUTORSMENB0_P1);
#endif

	mdelay(1);

	/*enable USB PHYs*/
	if (!bca_usb->xhci_enable){
		/* TODO maybe rename xhci_enable to usb3x_enable or ss_enable*/
		usb_ctrl->usb_pm |= (U3XPHY_PWRDWN);
	}
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
