/*
<:copyright-BRCM:2018:DUAL/GPL:standard

   Copyright (c) 2018 Broadcom 
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

