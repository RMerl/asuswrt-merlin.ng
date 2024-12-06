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

typedef struct usb_ctrl{
    uint32_t setup;
#define USBH_IPP                (1<<5)
#define USBH_IOC                (1<<4)
#define USBH_STRAP_IPP_SEL      (1<<25)
#define USB2_OC_DISABLE_PORT0   (1<<28)
#define USB2_OC_DISABLE_PORT1   (1<<29)
#define USB3_OC_DISABLE_PORT0   (1<<30)
#define USB3_OC_DISABLE_PORT1   (1<<31)
    uint32_t pll_ctl;
    uint32_t fladj_value;
    uint32_t bridge_ctl;
#define USB_BRCTL_OHCI_MEM_REQ_DIS (1<<16)
    uint32_t spare1;
    uint32_t mdio;
    uint32_t mdio2;
    uint32_t test_port_control;
    uint32_t usb_simctl;
    uint32_t usb_testctl;
    uint32_t usb_testmon;
    uint32_t utmi_ctl_1;
    uint32_t utmi_ctl_2;
    uint32_t usb_pm;
#define XHC_SOFT_RESETB         (1<<22)
#define USB_PWRDWN              (1<<31)
    uint32_t usb_pm_status;
    uint32_t spare3;
    uint32_t pll_ldo_ctl;
    uint32_t pll_ldo_pllbias;
    uint32_t pll_afe_bg_cntl;
    uint32_t afe_usbio_tst;
    uint32_t pll_ndiv_frac;
    uint32_t tp_diag;
    uint32_t ahb_capture_fifo;
    uint32_t spare4;
    uint32_t usb30_ctl1;
#define PHY3_PLL_SEQ_START      (1<<4)
    uint32_t usb30_ctl2;
    uint32_t usb30_ctl3;
    uint32_t usb30_ctl4;
    uint32_t usb30_pctl;
    uint32_t usb30_ctl5;
    uint32_t spare5;
    uint32_t spare6;
    uint32_t spare7;
    uint32_t unsused1[3];
    uint32_t usb_device_ctl1;
    uint32_t usb_device_ctl2;
    uint32_t unsused2[22];
    uint32_t usb20_id;
    uint32_t usb30_id;
    uint32_t bdc_coreid;
    uint32_t usb_revid;
} usb_ctrl_t;

int hw_init(struct bcm_bca_usb_ctrl *bca_usb)
{
    usb_ctrl_t *usb_ctrl = (usb_ctrl_t *)bca_usb->usb_ctrl;
    uint32_t val;
    dev_info(&bca_usb->pdev->dev, "---- Powering up USB blocks\n");

    if(pmc_usb_power_up(PMC_USB_HOST_ALL))
    {
        dev_err(&bca_usb->pdev->dev,"+++ Failed to Power Up USB Host\n");
        return -1;
    }

    mdelay(1);

    /* adjust over current & port power polarity */
    if (bca_usb->pwrflt_p_high)
        usb_ctrl->setup &= ~(USBH_IOC);
    else
        usb_ctrl->setup |= USBH_IOC;
    
    /*overide strap for IPP*/
    val = usb_ctrl->setup;
    val &= ~(USBH_STRAP_IPP_SEL);
    val |= (USBH_IPP);
    usb_ctrl->setup = val;

    if (bca_usb->pwron_p_high)
	usb_ctrl->setup &= ~(USBH_IPP);
    else
	usb_ctrl->setup |= USBH_IPP;

    /*enable USB PHYs*/
    mdelay(1);
    usb_ctrl->usb_pm &= ~(USB_PWRDWN);
    mdelay(1);

    if (bca_usb->xhci_enable)
    {
        /*enable SSC for usb3.0 */
        usb3_ssc_enable(&usb_ctrl->mdio);

        usb3_enable_pipe_reset(&usb_ctrl->mdio);
        usb3_enable_sigdet(&usb_ctrl->mdio);
        usb3_enable_skip_align(&usb_ctrl->mdio);
        mdelay(300);
        /*initialize XHCI settings*/

        usb_ctrl->usb30_ctl1 |= PHY3_PLL_SEQ_START;
        usb_ctrl->usb_pm |= XHC_SOFT_RESETB;
    }
    else
    {
        mdelay(300);
        usb_ctrl->usb_pm &= ~XHC_SOFT_RESETB;
    }

    /*adjust the default AFE settings for better eye diagrams */
    usb2_eye_fix(&usb_ctrl->mdio);

    /*initialize EHCI & OHCI settings*/
    /* no swap for data & desciptors */    
    usb_ctrl->bridge_ctl &= ~(0xf); /*clear lower 4 bits */

    /* reset host controllers for possible fake overcurrent indications */ 
    val = usb_ctrl->usb_pm;
    usb_ctrl->usb_pm = 0;
    usb_ctrl->usb_pm = val;
    mdelay(1);

    return 0;
}

void hw_uninit(struct bcm_bca_usb_ctrl *bca_usb)
{
    dev_info(&bca_usb->pdev->dev, "---- Powering DOWN USB blocks\n");
    pmc_usb_power_down(PMC_USB_HOST_ALL);
    mdelay(1);
}
