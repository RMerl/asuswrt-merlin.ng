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
#define USB_DEVICE_SEL          (1<<11)
#define USB3_OC_DISABLE_PORT0   (1<<30)
#define USB3_OC_DISABLE_PORT1   (1<<31)
    uint32_t pll_ctl;
    uint32_t fladj_value;
    uint32_t bridge_ctl;
#define EHCI_ENDIAN_SWAP        (1<<3)
#define EHCI_DATA_SWAP          (1<<2)
#define OHCI_ENDIAN_SWAP        (1<<1)
#define OHCI_DATA_SWAP          (1<<0)
    uint32_t spare1;
    uint32_t mdio;
    uint32_t mdio2;
    uint32_t test_port_control;
    uint32_t usb_simctl;
#define USBH_OHCI_MEM_REQ_DIS   (1<<1)
    uint32_t usb_testctl;
    uint32_t usb_testmon;
    uint32_t utmi_ctl_1;
    uint32_t spare2;
    uint32_t usb_pm;
#define XHC_SOFT_RESETB         (1<<30)
    uint32_t usb_pm_status;
    uint32_t spare3;
    uint32_t pll_ldo_ctl;
    uint32_t pll_ldo_pllbias;
    uint32_t pll_afe_bg_cntl;
    uint32_t afe_usbio_tst;
    uint32_t pll_ndiv_frac;
    uint32_t spare4[3];
    uint32_t usb30_ctl1;
#define PHY3_PLL_SEQ_START      (1<<4)
    uint32_t usb30_ctl2;
    uint32_t usb30_ctl3;
    uint32_t usb30_ctl4;
    uint32_t usb30_pctl;
    uint32_t usb30_ctl5;
#define GC_PLL_SUSPEND_EN       (1<<1)
    uint32_t spare5;
    uint32_t spare6;
    uint32_t spare7;
    uint32_t unsused1[3];
    uint32_t usb_device_ctl1;
    uint32_t unsused2[26];
    uint32_t usb_revid;
} usb_ctrl_t;

static void manual_usb_ldo_start(usb_ctrl_t *usb_ctrl)
{
    usb_ctrl->pll_ctl &= ~(1 << 30); /*pll_resetb=0*/
    usb_ctrl->utmi_ctl_1 = 0; 
    usb_ctrl->pll_ldo_ctl = 4; /*ldo_ctl=core_rdy */
    usb_ctrl->pll_ctl |= ( 1 << 31); /*pll_iddq=1*/
    mdelay(10);
    usb_ctrl->pll_ctl &= ~( 1 << 31); /*pll_iddq=0*/
    usb_ctrl->pll_ldo_ctl |= 1; /*ldo_ctl.AFE_LDO_PWRDWNB=1*/
    usb_ctrl->pll_ldo_ctl |= 2; /*ldo_ctl.AFE_BG_PWRDWNB=1*/
    mdelay(1);
    usb_ctrl->utmi_ctl_1 = 0x00020002;/* utmi_resetb &ref_clk_sel=0; */ 
    usb_ctrl->pll_ctl |= ( 1 << 30); /*pll_resetb=1*/
    mdelay(10);
}    

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

    if (bca_usb->pwrflt_p_high)
        usb_ctrl->setup &= ~(USBH_IOC);
    else
        usb_ctrl->setup |= USBH_IOC;

    if (bca_usb->pwron_p_high)
        usb_ctrl->setup &= ~(USBH_IPP);
    else
        usb_ctrl->setup |= USBH_IPP;

    if (bca_usb->xhci_enable)
    {
        /*enable SSC for usb3.0 */
        usb3_ssc_enable(&usb_ctrl->mdio);

        mdelay(300);
        manual_usb_ldo_start(usb_ctrl);
        /*initialize XHCI settings*/
        usb_ctrl->usb_pm |= XHC_SOFT_RESETB;
        usb_ctrl->usb30_ctl1 &= ~PHY3_PLL_SEQ_START;
        usb_ctrl->usb30_ctl1 |= PHY3_PLL_SEQ_START;
    }
    else
    {
        mdelay(300);
        manual_usb_ldo_start(usb_ctrl);
    }

    /*adjust the default AFE settings for better eye diagrams */
    usb2_eye_fix(&usb_ctrl->mdio);

    /*initialize EHCI & OHCI settings*/
    usb_ctrl->bridge_ctl &= ~(EHCI_ENDIAN_SWAP | OHCI_ENDIAN_SWAP);

    /* reset host controllers for possible fake overcurrent indications */ 
    val = usb_ctrl->usb_pm;
    usb_ctrl->usb_pm = 0;
    usb_ctrl->usb_pm = val;
    mdelay(1);

    return 0;
}

void hw_uninit(struct bcm_bca_usb_ctrl *bca_usb)
{
    pmc_usb_power_down(PMC_USB_HOST_ALL);
    mdelay(1);
}

