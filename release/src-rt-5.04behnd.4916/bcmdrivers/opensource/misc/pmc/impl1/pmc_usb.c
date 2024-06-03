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
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>

#include "pmc_drv.h"
#include "BPCM.h"
#include "pmc_usb.h"
#include "bcm_ubus4.h"
#include <board.h>

/*TODO add functions to power on/off the USB blocks selectively */
int pmc_usb_power_up(unsigned int usb_block)
{
    int ret;

    if( usb_block == PMC_USB_HOST_ALL)
    {
#if defined CONFIG_BCM963138 || defined CONFIG_BCM963148
        ret = PowerOnDevice(PMB_ADDR_USB30_2X);
#elif defined CONFIG_BCM94908
        /*TODO check if this will power on USB device also */
        ret = PowerOnDevice(PMB_ADDR_USB30_2X);
#elif defined(CONFIG_BCM96858) || defined(CONFIG_BCM96856)
        ret = PowerOnDevice(PMB_ADDR_USB30_2X);
        ubus_register_port(UCB_NODE_ID_SLV_USB);
        ubus_register_port(UCB_NODE_ID_MST_USB);
#elif defined(CONFIG_BCM96846) || defined(CONFIG_BCM96878) || defined(CONFIG_BCM96855)
        ret = PowerOnDevice(PMB_ADDR_USB20_2X);
#elif defined(CONFIG_BCM963158) || defined(CONFIG_BCM963178) || defined(CONFIG_BCM963146)|| defined(CONFIG_BCM94912) || defined(CONFIG_BCM96813)
        ret = PowerOnDevice(PMB_ADDR_USB30_2X);
#elif defined(CONFIG_BCM947622) || defined(CONFIG_BCM96756)
        ret = PowerOnDevice(PMB_ADDR_USB31_20);
#elif defined(CONFIG_BCM96888) || defined(CONFIG_BCM968880) || defined(CONFIG_BCM96837) || defined(CONFIG_BCM96765)
        ret = PowerOnDevice(PMB_ADDR_USB);
#else
#error Platform is not supported
#endif
    }
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

#if defined(CONFIG_BCM_UBUS_DECODE_REMAP)
    ubus_master_remap_port(UBUS_PORT_ID_USB);
#endif

    return ret;
}
EXPORT_SYMBOL(pmc_usb_power_up);

int pmc_usb_power_down(unsigned int usb_block)
{
    int zone, status, dev_addr;

    if( usb_block == PMC_USB_HOST_ALL)
    {
#if defined PMB_ADDR_USB30_2X
        dev_addr = PMB_ADDR_USB30_2X;
        zone = PMB_ZONES_USB30_2X;
#if defined(CONFIG_BCM96858) || defined(CONFIG_BCM96856)
        ubus_deregister_port(UCB_NODE_ID_SLV_USB);
        ubus_deregister_port(UCB_NODE_ID_MST_USB);
#endif
#elif defined(PMB_ADDR_USB20_2X)
        dev_addr = PMB_ADDR_USB20_2X;
        zone = PMB_ZONES_USB20_2X;
#elif defined(PMB_ADDR_USB31_20)
        dev_addr = PMB_ADDR_USB31_20;
        zone = PMB_ZONES_USB31_20;
#elif defined(PMB_ADDR_USB)
        dev_addr = PMB_ADDR_USB;
        zone = PMB_ZONES_USB;
#else
#error Platform is not supported
#endif

        // Powering-down zone 0 doesn't power-down the whole device;
        // Individually power-down each zone

        for (zone-=1; zone >=0 ; zone--)
        {
            status = PowerOffZone(dev_addr, zone);
            if (status)
                return status;
        }
        return 0;
    }
#if defined(PMB_ADDR_USB)
    /*
      Shutting down Zone0 disables USB functionality on port0 (Use only when disables whole USB subsystem)
      Shutting down Zone1 disables USB functionality on port1
      Shutting down Zone2 disables USB3 functionality on Port0
      Shutting down Zone3 disables USBD functionality
    */
    else if (usb_block == PMC_USB_PORT_1)
    {
    	PowerOffZone(PMB_ADDR_USB, 1);
    }
#endif
    else
    {
        printk("pmc_usb_power_down: Error unsupported usb_block=%u\n",usb_block);
        return -1;
    }
    return 0;
}
EXPORT_SYMBOL(pmc_usb_power_down);
