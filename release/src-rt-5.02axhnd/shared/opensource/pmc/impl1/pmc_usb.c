/*
<:copyright-BRCM:2013:DUAL/GPL:standard 

   Copyright (c) 2013 Broadcom 
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
#ifdef _CFE_
#include "lib_printf.h"
#define printk	printf
#else
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#endif

#include "pmc_drv.h"
#include "BPCM.h"
#include "pmc_usb.h"
#include "bcm_ubus4.h"

/*TODO add functions to power on/off the USB blocks selectively */
int pmc_usb_power_up(unsigned int usb_block)
{
    int ret;
#if defined CONFIG_BCM963138 || defined CONFIG_BCM963148
    if( usb_block == PMC_USB_HOST_ALL)
    {
        ret = PowerOnDevice(PMB_ADDR_USB30_2X);
    }
#elif defined CONFIG_BCM963381
    if( usb_block == PMC_USB_HOST_20)
    {
        ret = PowerOnDevice(PMB_ADDR_USB2X);
    }
    else if( usb_block == PMC_USB_HOST_30)
    {
        ret = PowerOnDevice(PMB_ADDR_USB30);
    }
#elif defined CONFIG_BCM96848
    if( usb_block == PMC_USB_HOST_20)
    {
        ret = PowerOnDevice(PMB_ADDR_USB2X);
    }
#elif defined CONFIG_BCM94908
    if( usb_block == PMC_USB_HOST_ALL)
    {
        /*TODO check if this will power on USB device also */
        ret = PowerOnDevice(PMB_ADDR_USB);
    }
#elif defined(CONFIG_BCM96858) || defined(CONFIG_BCM96836) || defined(CONFIG_BCM96856)
    if( usb_block == PMC_USB_HOST_ALL)
    {
        ret = PowerOnDevice(PMB_ADDR_USB30_2X);
        ubus_register_port(UCB_NODE_ID_SLV_USB);
        ubus_register_port(UCB_NODE_ID_MST_USB);
    }
#elif defined CONFIG_BCM96846
    if( usb_block == PMC_USB_HOST_ALL)
    {
        ret = PowerOnDevice(PMB_ADDR_USB20_2X);
        printk("Need to open after OTP func=%s line=%d",__FUNCTION__,__LINE__);
        /*ubus_register_port(UCB_NODE_ID_SLV_USB);
        ubus_register_port(UCB_NODE_ID_MST_USB);*/
    }
#elif defined CONFIG_BCM963158 
    if( usb_block == PMC_USB_HOST_ALL)
    {
        ret = PowerOnDevice(PMB_ADDR_USB30_2X);
    }
#endif
    else {
        printk("pmc_usb_power_up: Error unsupported usb_block=%u\n",usb_block);
        return -1;
    }

#if defined CONFIG_BCM963158 || defined CONFIG_BCM96858
    apply_ubus_credit_each_master(UBUS_PORT_ID_USB);
#endif

#if defined(CONFIG_BCM_UBUS4_DCM)
    ubus_cong_threshold_wr(UBUS_PORT_ID_USB, 0);
#endif

    return ret;
}
#ifndef _CFE_
EXPORT_SYMBOL(pmc_usb_power_up);

int pmc_usb_power_down(unsigned int usb_block)
{
#if defined CONFIG_BCM963138 || defined CONFIG_BCM963148
	// Powering-down zone 0 doesn't power-down the whole device;
	// Individually power-down each zone
	int zone, status;

    if( usb_block == PMC_USB_HOST_ALL)
    {
        for (zone = 0; zone < PMB_ZONES_USB30_2X; zone++) {
            status = PowerOffZone(PMB_ADDR_USB30_2X, zone);
            if (status) return status;
        }
        return 0;
    }
#elif defined CONFIG_BCM963381
    if( usb_block == PMC_USB_HOST_20)
    {
        return PowerOffDevice(PMB_ADDR_USB2X, 0);
    }
    else if( usb_block == PMC_USB_HOST_30)
    {
        return PowerOffDevice(PMB_ADDR_USB30, 0);
    }
#elif defined CONFIG_BCM96848
    if( usb_block == PMC_USB_HOST_20)
    {
        return PowerOffDevice(PMB_ADDR_USB2X, 0);
    }
#elif defined CONFIG_BCM94908
    if( usb_block == PMC_USB_HOST_ALL)
    {
        /*TODO check if this will power off USB device also */
        return PowerOffDevice(PMB_ADDR_USB, 0);
    }
#elif defined(CONFIG_BCM96858) || defined(CONFIG_BCM96836) || defined(CONFIG_BCM96856)
    if( usb_block == PMC_USB_HOST_ALL)
    {
        ubus_deregister_port(UCB_NODE_ID_SLV_USB);
        ubus_deregister_port(UCB_NODE_ID_MST_USB);
        return PowerOffDevice(PMB_ADDR_USB30_2X, 0);
    }
#elif defined CONFIG_BCM96846
    if( usb_block == PMC_USB_HOST_ALL)
    {
        printk("Need to open after OTP func=%s line=%d",__FUNCTION__,__LINE__);
        /*ubus_deregister_port(UCB_NODE_ID_SLV_USB);
        ubus_deregister_port(UCB_NODE_ID_MST_USB);*/
        return PowerOffDevice(PMB_ADDR_USB20_2X,0);
    }
#elif defined CONFIG_BCM963158
    if( usb_block == PMC_USB_HOST_ALL)
    {
        return PowerOffDevice(PMB_ADDR_USB30_2X, 0);
    }
#endif
    {
        printk("pmc_usb_power_down: Error unsupported usb_block=%u\n",usb_block);
        return -1;
    }
}
EXPORT_SYMBOL(pmc_usb_power_down);
#endif
