/*
<:copyright-BRCM:2013:DUAL/GPL:standard 

   Copyright (c) 2013 Broadcom 
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
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/delay.h>

#include "pmc_drv.h"
#include "BPCM.h"
#include "bcm_ubus4.h"

int pmc_sata_power_up(void)
{
	int ret;

	ret = PowerOnDevice(PMB_ADDR_SATA);
	mdelay(1);

#if defined (CONFIG_BCM96858) || defined(_BCM96858_)
    ubus_register_port(UCB_NODE_ID_SLV_SATA);
    ubus_register_port(UCB_NODE_ID_MST_SATA);
#endif
    
#if !defined(CONFIG_BCM963158)
	ret |= WriteBPCMRegister(PMB_ADDR_SATA, BPCMRegOffset(misc_control), 0);
#endif

	ret |= WriteBPCMRegister(PMB_ADDR_SATA, BPCMRegOffset(sr_control), ~0);
	ret |= WriteBPCMRegister(PMB_ADDR_SATA, BPCMRegOffset(sr_control), 0);

#if defined(CONFIG_BCM963158) || defined(CONFIG_BCM96858)
	/* PCIE2 and SATA shared the same master port */
	apply_ubus_credit_each_master(UBUS_PORT_ID_PCIE2);
#endif

#if defined(CONFIG_BCM_UBUS4_DCM)
    ubus_cong_threshold_wr(UBUS_PORT_ID_PCIE2, 0);
#endif

	return ret;
}
EXPORT_SYMBOL(pmc_sata_power_up);

int pmc_sata_power_down(int block)
{
	int ret;

#if defined (CONFIG_BCM96858) || defined(_BCM96858_)
    ubus_deregister_port(UCB_NODE_ID_SLV_SATA);
    ubus_deregister_port(UCB_NODE_ID_MST_SATA);
#endif

#if !defined(CONFIG_BCM963158)
	ret = WriteBPCMRegister(PMB_ADDR_SATA, BPCMRegOffset(misc_control), 1);
#endif
	ret |= PowerOffDevice(PMB_ADDR_SATA, 0);

    return ret;
}
EXPORT_SYMBOL(pmc_sata_power_down);
