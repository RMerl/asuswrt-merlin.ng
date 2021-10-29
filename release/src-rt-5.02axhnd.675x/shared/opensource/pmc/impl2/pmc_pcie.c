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

