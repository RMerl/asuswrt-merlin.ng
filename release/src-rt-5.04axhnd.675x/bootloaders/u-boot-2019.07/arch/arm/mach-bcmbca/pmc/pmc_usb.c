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
#include "pmc_drv.h"
#include "asm/arch/BPCM.h"
#include "pmc_usb.h"
#include "bcm_ubus4.h"

/*TODO add functions to power on/off the USB blocks selectively */
int pmc_usb_power_up(unsigned int usb_block)
{
    int ret;

#if IS_BCMCHIP(63138) || IS_BCMCHIP(63148)
    if( usb_block == PMC_USB_HOST_ALL)
    {
        ret = PowerOnDevice(PMB_ADDR_USB30_2X);
    }
#elif IS_BCMCHIP(4908)
    if( usb_block == PMC_USB_HOST_ALL)
    {
        /*TODO check if this will power on USB device also */
        ret = PowerOnDevice(PMB_ADDR_USB30_2X);
    }
#elif IS_BCMCHIP(6858) || IS_BCMCHIP(6856)
    if( usb_block == PMC_USB_HOST_ALL)
    {
        ret = PowerOnDevice(PMB_ADDR_USB30_2X);
        ubus_register_port(UCB_NODE_ID_SLV_USB);
        ubus_register_port(UCB_NODE_ID_MST_USB);
    }
#elif IS_BCMCHIP(6846) || IS_BCMCHIP(6878) || IS_BCMCHIP(6855)
    if( usb_block == PMC_USB_HOST_ALL)
    {
        ret = PowerOnDevice(PMB_ADDR_USB20_2X);
    }
#elif IS_BCMCHIP(63158) || IS_BCMCHIP(63178) || IS_BCMCHIP(63146) || IS_BCMCHIP(4912)\
    || IS_BCMCHIP(6813)
    if( usb_block == PMC_USB_HOST_ALL)
    {
        ret = PowerOnDevice(PMB_ADDR_USB30_2X);
    }
#elif IS_BCMCHIP(47622) || IS_BCMCHIP(6756)
    if( usb_block == PMC_USB_HOST_ALL)
    {
        ret = PowerOnDevice(PMB_ADDR_USB31_20);
    }
#endif
    else 
    {
        printk("pmc_usb_power_up: Error unsupported usb_block=%u\n",usb_block);
        return -1;
    }

#if  IS_BCMCHIP(63158) || IS_BCMCHIP(6858)
    apply_ubus_credit_each_master(UBUS_PORT_ID_USB);
#endif

#if defined(CONFIG_BCM_UBUS4_DCM)
    ubus_cong_threshold_wr(UBUS_PORT_ID_USB, 0);
#endif

#if defined(CONFIG_BCM_UBUS_DECODE_REMAP)
    ubus_master_remap_port(UBUS_PORT_ID_USB);
#endif

    return ret;
}

int pmc_usb_power_down(unsigned int usb_block)
{
#if IS_BCMCHIP(63138) || IS_BCMCHIP(63148)
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
#elif IS_BCMCHIP(4908)
    if( usb_block == PMC_USB_HOST_ALL)
    {
        /*TODO check if this will power off USB device also */
        return PowerOffDevice(PMB_ADDR_USB30_2X, 0);
    }
#elif IS_BCMCHIP(6858) || IS_BCMCHIP(6856)
    if( usb_block == PMC_USB_HOST_ALL)
    {
        ubus_deregister_port(UCB_NODE_ID_SLV_USB);
        ubus_deregister_port(UCB_NODE_ID_MST_USB);
        return PowerOffDevice(PMB_ADDR_USB30_2X, 0);
    }
#elif IS_BCMCHIP(6846) || IS_BCMCHIP(6878) || IS_BCMCHIP(6855)
    if( usb_block == PMC_USB_HOST_ALL)
    {
        return PowerOffDevice(PMB_ADDR_USB20_2X,0);
    }
#elif IS_BCMCHIP(63158) || IS_BCMCHIP(63178) || IS_BCMCHIP(63146) || IS_BCMCHIP(4912)\
    || IS_BCMCHIP(6813)
    if( usb_block == PMC_USB_HOST_ALL)
    {
        return PowerOffDevice(PMB_ADDR_USB30_2X, 0);
    }
#elif IS_BCMCHIP(47622) || IS_BCMCHIP(6756)
    if( usb_block == PMC_USB_HOST_ALL)
    {
        return PowerOffDevice(PMB_ADDR_USB31_20, 0);
    }
#endif
    else
    {
        printk("pmc_usb_power_down: Error unsupported usb_block=%u\n",usb_block);
        return -1;
    }
}
