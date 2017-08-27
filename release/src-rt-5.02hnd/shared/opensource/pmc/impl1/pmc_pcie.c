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
#ifndef _CFE_
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/delay.h>
#endif

#include "pmc_drv.h"
#include "pmc_pcie.h"
#include "BPCM.h"

static const int pmc_pcie_pmb_addr[] = {
#if defined(CONFIG_BCM963138) || defined(CONFIG_BCM963148)
	PMB_ADDR_PCIE0,
	PMB_ADDR_PCIE1
#elif defined CONFIG_BCM94908
	PMB_ADDR_PCIE0,
	PMB_ADDR_PCIE1,
	PMB_ADDR_PCIE2
#elif defined(CONFIG_BCM96858)
	PMB_ADDR_PCIE0,
	PMB_ADDR_PCIE1,
	PMB_ADDR_PCIE_UBUS
#else /* CONFIG_BCM963381, CONFIG_BCM96848 */
	PMB_ADDR_PCIE0
#endif
};

#define MAX_PCIE_UNIT	(sizeof(pmc_pcie_pmb_addr)/sizeof(int))
void pmc_pcie_power_up(int unit)
{
	BPCM_SR_CONTROL sr_ctrl = {
		.Bits.sr = 0, // Only iddq
	};
	int addr;

	if (unit >= MAX_PCIE_UNIT)
		BUG_ON(1);

	addr = pmc_pcie_pmb_addr[unit];

	if (PowerOnZone(addr, 0))
		BUG_ON(1);

	mdelay(10);

	if (WriteBPCMRegister(addr, BPCMRegOffset(sr_control), sr_ctrl.Reg32))
		BUG_ON(1);
}

void pmc_pcie_power_down(int unit)
{
	BPCM_SR_CONTROL sr_ctrl = {
		.Bits.sr = 4, // Only iddq
	};
	int addr;

	if (unit >= MAX_PCIE_UNIT)
		BUG_ON(1);

	addr = pmc_pcie_pmb_addr[unit];

	if (WriteBPCMRegister(addr, BPCMRegOffset(sr_control), sr_ctrl.Reg32))
		BUG_ON(1);

	mdelay(10);

	if (PowerOffZone(addr, 0))
		BUG_ON(1);
}

#ifndef _CFE_
EXPORT_SYMBOL(pmc_pcie_power_up);
EXPORT_SYMBOL(pmc_pcie_power_down);
#endif
