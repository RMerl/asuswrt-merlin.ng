/*
<:copyright-BRCM:2020:DUAL/GPL:standard 

   Copyright (c) 2020 Broadcom 
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
#include <debug.h>
#include <platform_def.h>
#include <delay_timer.h>
#include "pmc_drv.h"
#include "pmc_cpu_core.h"
#include "BPCM.h"

#if defined (PMB_ADDR_ORION_CPU0)
static uint32_t pmb_cpu_id[] = {
  PMB_ADDR_ORION_CPU0,
  PMB_ADDR_ORION_CPU1,
#if defined(PMB_ADDR_ORION_CPU2)
  PMB_ADDR_ORION_CPU2,
#endif
#if defined(PMB_ADDR_ORION_CPU3)
  PMB_ADDR_ORION_CPU3
#endif
};
#endif

int pmc_cpu_core_power_up(unsigned cpu)
{
#if defined (PMB_ADDR_ORION_CPU0)
	int error;
	uint32_t arm_control;

#if defined (PMB_ADDR_ORION_CPU0)
	error = ResetDevice(pmb_cpu_id[cpu]);
	if (error) {
		ERROR("unable to power on CPU%d\n", cpu);
		return error;
	}
#endif

	error = ReadBPCMRegister(PMB_ADDR_BIU_BPCM,
					ARMBPCMRegOffset(arm_control),
					&arm_control);
	if (error) {
		ERROR("unable to release CPU%d reset.\n", cpu);
		return error;
	}
	arm_control &= ~(1 << cpu);
	error = WriteBPCMRegister(PMB_ADDR_BIU_BPCM,
					ARMBPCMRegOffset(arm_control),
					arm_control);
	if (error) {
		ERROR("unable to release CPU%d reset.\n", cpu);
		return error;
	}
	udelay(100); // wait for cpu to come out of reset
	INFO("pmc_cpu: powered up CPU%d\n", cpu);
#elif defined (BIUCTRL_BASE)
	if (BIUCTRL->cpu_pwr_zone_ctrl[cpu] & BIU_CPU_CTRL_PWR_ZONE_CTRL_ZONE_RESET) {
		BIUCTRL->power_cfg |= BIU_CPU_CTRL_PWR_CFG_CPU0_BPCM_INIT_ON << cpu;
		BIUCTRL->cpu_pwr_zone_ctrl[cpu] = BIU_CPU_CTRL_PWR_ZONE_CTRL_PWR_UP_REQ |
			(BIUCTRL->cpu_pwr_zone_ctrl[cpu] & ~BIU_CPU_CTRL_PWR_ZONE_CTRL_PWR_DN_REQ);

		udelay(100); // wait for cpu to come out of reset
	}
#endif
	return 0;
}

int pmc_cpu_core_power_down(unsigned cpu)
{
#if defined (BIUCTRL_BASE)
	BIUCTRL->power_cfg &= ~(BIU_CPU_CTRL_PWR_CFG_CPU0_BPCM_INIT_ON << cpu);
	BIUCTRL->cpu_pwr_zone_ctrl[cpu] = BIU_CPU_CTRL_PWR_ZONE_CTRL_PWR_DN_REQ |
		(BIUCTRL->cpu_pwr_zone_ctrl[cpu] & ~BIU_CPU_CTRL_PWR_ZONE_CTRL_PWR_UP_REQ);
#endif
    return 0;
}
