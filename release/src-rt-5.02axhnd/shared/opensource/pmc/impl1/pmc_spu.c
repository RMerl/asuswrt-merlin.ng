/*
<:copyright-BRCM:2018:DUAL/GPL:standard

   Copyright (c) 2018 Broadcom 
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

#ifndef _CFE_
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#endif
#include "pmc_drv.h"
#include "pmc_spu.h"
#include "bcm_ubus4.h"

int pmc_spu_power_up(void)
{
	int err = 0;

    err = PowerOnDevice(PMB_ADDR_CRYPTO);
	if (err) {
		printk("Failed to PowerOnDevice PMB_ADDR_CRYPTO\n");
		return err;
	}

#if defined(CONFIG_BCM96858)
    err = ubus_master_set_token_credits(UBUS_PORT_ID_SPU, 1, 4);
	if (err) {
		printk("Failed to ubus_master_set_token_credits for MST_PORT_NODE_SPU\n");
		return err;
	}
#endif
#if defined(CONFIG_BCM_UBUS_DECODE_REMAP)
	err = ubus_master_remap_port(UBUS_PORT_ID_SPU);
	if (err) {
		printk("Failed to ubus_master_remap_port for MST_PORT_NODE_SPU\n");
		return err;
	}
#endif
	return err;
}

int pmc_spu_power_down(void)
{
    return PowerOffDevice(PMB_ADDR_CRYPTO, 0);
}

#ifndef _CFE_
EXPORT_SYMBOL(pmc_spu_power_up);
EXPORT_SYMBOL(pmc_spu_power_down);
#endif

