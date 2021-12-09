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

static void pmc_pcie_power_set(int index, bool PowerOn)
{
	int status = 0;
	uint32 sr_ctrl;
	int dev;
	
	if(index == 0)
		dev = PMB_ADDR_PCIE_UBUS_0;
	else
		dev = PMB_ADDR_PCIE_UBUS_1;
	
	if(PowerOn)
	{
		sr_ctrl = 0;
		status = PowerOnDevice(dev);
	}
	else
	{
		sr_ctrl = 4; // Only iddq
		status = PowerOffDevice(dev, 0);
	}
	if(status == kPMC_NO_ERROR)
		status = WriteBPCMRegister(dev, 8, sr_ctrl);
	
	BUG_ON(status != kPMC_NO_ERROR);
}


void pmc_pcie_power_up(int unit)
{
	pmc_pcie_power_set(unit, 1);
}

void pmc_pcie_power_down(int unit)
{
	pmc_pcie_power_set(unit, 0);
}

#ifndef _CFE_
EXPORT_SYMBOL(pmc_pcie_power_up);
EXPORT_SYMBOL(pmc_pcie_power_down);
#endif

