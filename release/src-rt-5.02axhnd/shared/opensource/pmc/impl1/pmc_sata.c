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
