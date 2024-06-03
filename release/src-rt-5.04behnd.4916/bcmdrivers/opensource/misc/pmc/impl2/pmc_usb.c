/*
<:copyright-BRCM:2022:DUAL/GPL:standard

   Copyright (c) 2022 Broadcom
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
/****************************************************************************
 *
 * Author: Dima Mamut <dima.mamut@broadcom.com>
*****************************************************************************/
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include "pmc_drv.h"
#include "pmc_usb.h"
#include "bcm_ubus4.h"
#include <board.h>

char *usb_domain_names_20[PWR_DOMAIN_NAME_MAX_LEN]  = {"usb2p0","usb2p1"};
char *usb_domain_names_30[PWR_DOMAIN_NAME_MAX_LEN]  = {"usb3p0","usb3p1"};
char *usb_domain_names_port0[PWR_DOMAIN_NAME_MAX_LEN] = {"usb2p0", "usb3p0"};
char *usb_domain_names_port1[PWR_DOMAIN_NAME_MAX_LEN] = {"usb2p1", "usb3p1"};

#define USB_DOMAIN_MAX_NUM      (2)

int pmc_usb_power_up(unsigned int usb_block)
{
    char *domain_name;
    uint32_t usb_domain_idx = 0;
    int32_t result = 0;
    char **usb_domain_ptr;

    if (usb_block >= PMC_USB_MAX)
    {
        printk("[%s:%d] ERROR:wrong usb_block number[%d]\n",__FUNCTION__,__LINE__,usb_block);
        return -1;
    }

    switch (usb_block)
    {
        case PMC_USB_ALL:
        case PMC_USB_HOST_ALL:
            result = bcm_rpc_pwr_set_domain_state("usb", PWR_DOMAIN_NAME_MAX_LEN, PWR_DOM_STATE_ON, PWR_DOM_RESET_DEASSERT);
            return result;

        case PMC_USB_PORT_2_P0:
            result = bcm_rpc_pwr_set_domain_state("usb2p0", PWR_DOMAIN_NAME_MAX_LEN, PWR_DOM_STATE_ON, PWR_DOM_RESET_DEASSERT);
            return result;

        case PMC_USB_PORT_3_P0:
            result = bcm_rpc_pwr_set_domain_state("usb3p0", PWR_DOMAIN_NAME_MAX_LEN, PWR_DOM_STATE_ON, PWR_DOM_RESET_DEASSERT);
            return result;

        case PMC_USB_PORT_2_P1:
            result = bcm_rpc_pwr_set_domain_state("usb2p1", PWR_DOMAIN_NAME_MAX_LEN, PWR_DOM_STATE_ON, PWR_DOM_RESET_DEASSERT);
            return result;

        case PMC_USB_PORT_3_P1:
            result = bcm_rpc_pwr_set_domain_state("usb3p1", PWR_DOMAIN_NAME_MAX_LEN, PWR_DOM_STATE_ON, PWR_DOM_RESET_DEASSERT);
            return result;

        case PMC_USB_HOST_20:
            usb_domain_ptr = usb_domain_names_20;
        break;

        case PMC_USB_HOST_30:
            usb_domain_ptr = usb_domain_names_30;
        break;

        case PMC_USB_PORT_0:
            usb_domain_ptr = usb_domain_names_port0;
            break;

        case PMC_USB_PORT_1:
            usb_domain_ptr = usb_domain_names_port1;
        break;

        case PMC_USB_DEVICE:
            printk("Failed PMC_USB_DEVICE NOT SUPPORTED\n");
            return -1;
    }

    for (usb_domain_idx = 0; usb_domain_idx < USB_DOMAIN_MAX_NUM; usb_domain_idx++)
    {
        domain_name = usb_domain_ptr[usb_domain_idx];

        result = bcm_rpc_pwr_set_domain_state(domain_name, PWR_DOMAIN_NAME_MAX_LEN, PWR_DOM_STATE_ON, PWR_DOM_RESET_DEASSERT);
        if (result < 0)
        {
            printk("Failed to set pwr domain state of USB[%d]to ON\n", usb_block);
            return -1;
        }
    }

    return result;
}
EXPORT_SYMBOL(pmc_usb_power_up);

int pmc_usb_power_down(unsigned int usb_block)
{
    char *domain_name;
    uint32_t usb_domain_idx = 0;
    int32_t result = 0;
    char **usb_domain_ptr;

    if (usb_block >= PMC_USB_MAX)
    {
        printk("[%s:%d] ERROR:wrong usb_block number[%d]\n",__FUNCTION__,__LINE__,usb_block);
        return -1;
    }

    switch (usb_block)
    {
        case PMC_USB_ALL:
        case PMC_USB_HOST_ALL:
            result = bcm_rpc_pwr_set_domain_state("usb", PWR_DOMAIN_NAME_MAX_LEN, PWR_DOM_STATE_OFF, PWR_DOM_RESET_ASSERT);
            return result;

        case PMC_USB_PORT_2_P0:
            result = bcm_rpc_pwr_set_domain_state("usb2p0", PWR_DOMAIN_NAME_MAX_LEN, PWR_DOM_STATE_OFF, PWR_DOM_RESET_ASSERT);
            return result;

        case PMC_USB_PORT_3_P0:
            result = bcm_rpc_pwr_set_domain_state("usb3p0", PWR_DOMAIN_NAME_MAX_LEN, PWR_DOM_STATE_OFF, PWR_DOM_RESET_ASSERT);
            return result;

        case PMC_USB_PORT_2_P1:
            result = bcm_rpc_pwr_set_domain_state("usb2p1", PWR_DOMAIN_NAME_MAX_LEN, PWR_DOM_STATE_OFF, PWR_DOM_RESET_ASSERT);
            return result;

        case PMC_USB_PORT_3_P1:
            result = bcm_rpc_pwr_set_domain_state("usb3p1", PWR_DOMAIN_NAME_MAX_LEN, PWR_DOM_STATE_OFF, PWR_DOM_RESET_ASSERT);
            return result;

        case PMC_USB_HOST_20:
            usb_domain_ptr = usb_domain_names_20;
        break;

        case PMC_USB_HOST_30:
            usb_domain_ptr = usb_domain_names_30;
        break;

        case PMC_USB_PORT_0:
            usb_domain_ptr = usb_domain_names_port0;
            break;

        case PMC_USB_PORT_1:
            usb_domain_ptr = usb_domain_names_port1;
        break;

        case PMC_USB_DEVICE:
            printk("Failed PMC_USB_DEVICE NOT SUPPORTED\n");
            return -1;
    }

    for (usb_domain_idx = 0; usb_domain_idx < USB_DOMAIN_MAX_NUM; usb_domain_idx++)
    {
        domain_name = usb_domain_ptr[usb_domain_idx];

        result = bcm_rpc_pwr_set_domain_state(domain_name, PWR_DOMAIN_NAME_MAX_LEN, PWR_DOM_STATE_OFF, PWR_DOM_RESET_ASSERT);
        if (result < 0)
        {
            printk("Failed to set pwr domain state of USB[%d]to OFF\n", usb_block);
            return -1;
        }
    }

    return result;
}
EXPORT_SYMBOL(pmc_usb_power_down);
