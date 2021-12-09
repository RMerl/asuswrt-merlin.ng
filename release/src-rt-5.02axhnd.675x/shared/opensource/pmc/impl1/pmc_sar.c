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

