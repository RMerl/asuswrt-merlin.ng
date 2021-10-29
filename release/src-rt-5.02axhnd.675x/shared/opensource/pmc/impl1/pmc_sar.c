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
#endif

#include "pmc_drv.h"
#include "pmc_sar.h"
#include "BPCM.h"

int pmc_sar_soft_reset(void)
{
	BPCM_SR_CONTROL sr_ctrl;

	sr_ctrl.Bits.gp = ~0;
	sr_ctrl.Bits.sr = 1;
	WriteBPCMRegister(PMB_ADDR_SAR, BPCMRegOffset(sr_control), sr_ctrl.Reg32);
	sr_ctrl.Bits.sr = 0;
	WriteBPCMRegister(PMB_ADDR_SAR, BPCMRegOffset(sr_control), sr_ctrl.Reg32);
	return 0;
}

int pmc_sar_power_up(void)
{
	int ret;

	ret = PowerOnDevice(PMB_ADDR_SAR);
	if (ret)
		return ret;
	ret = pmc_sar_soft_reset();
	return ret;
}

int pmc_sar_power_down(void)
{
	return PowerOffDevice(PMB_ADDR_SAR, 0);
}

int pmc_sar_power_reset(void)
{
	int ret;

	ret = ResetDevice(PMB_ADDR_SAR);
	if (ret)
		return ret;
	ret = pmc_sar_soft_reset();
	return ret;
}

#ifndef _CFE_
EXPORT_SYMBOL(pmc_sar_power_up);
EXPORT_SYMBOL(pmc_sar_power_down);
EXPORT_SYMBOL(pmc_sar_power_reset);
EXPORT_SYMBOL(pmc_sar_soft_reset);
#endif

