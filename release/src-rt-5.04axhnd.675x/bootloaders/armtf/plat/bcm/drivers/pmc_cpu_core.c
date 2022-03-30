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

void udelay(uint32_t usec);

#if defined (PLATFORM_FLAVOR_63138)
/* this power up function is implemented with assumption that cpu#0 will always
 * be the first one that's powered up. */
int pmc_cpu_core_power_up(unsigned cpu)
{
	int ret;
	ARM_CONTROL_REG arm_ctrl;
	ARM_CPUx_PWR_CTRL_REG arm_pwr_ctrl;

	if (cpu == 0) {
		/* in 63138, cpu#0 should've been powered on either by default or by PMC ROM.
		 * This code is for the future if PMC can shut down all the CPUs in
		 * hibernation and power them back up from other blocks. */

		/* 1) Power on PLL */
		ret = ReadBPCMRegister(PMB_ADDR_AIP,
				ARMBPCMRegOffset(arm_control),
				&arm_ctrl.Reg32);
		if (ret)
			return ret;
		arm_ctrl.Bits.pll_ldo_pwr_on = 1;

		ret = WriteBPCMRegister(PMB_ADDR_AIP,
				ARMBPCMRegOffset(arm_control),
				arm_ctrl.Reg32);
		if (ret)
			return ret;

		/* wait at least 1.0 usec */
		udelay(2);

		arm_ctrl.Bits.pll_pwr_on = 1;
		ret = WriteBPCMRegister(PMB_ADDR_AIP,
				ARMBPCMRegOffset(arm_control),
				arm_ctrl.Reg32);
		if (ret)
			return ret;

		/* 2) Power up CPU0 */
		ret = ReadBPCMRegister(PMB_ADDR_AIP,
				ARMBPCMRegOffset(arm_pwr_ctrl_0),
				&arm_pwr_ctrl.Reg32);
		if (ret)
			return ret;
		arm_pwr_ctrl.Bits.pwr_on = 0xf;

		ret = WriteBPCMRegister(PMB_ADDR_AIP,
				ARMBPCMRegOffset(arm_pwr_ctrl_0),
				arm_pwr_ctrl.Reg32);
		if (ret)
			return ret;

		do {
			ret = ReadBPCMRegister(PMB_ADDR_AIP,
					ARMBPCMRegOffset(arm_pwr_ctrl_0),
					&arm_pwr_ctrl.Reg32);
			if (ret)
				return ret;
		} while (arm_pwr_ctrl.Bits.pwr_on_status != 0xf);

		arm_pwr_ctrl.Bits.pwr_ok = 0xf;

		ret = WriteBPCMRegister(PMB_ADDR_AIP,
				ARMBPCMRegOffset(arm_pwr_ctrl_0),
				arm_pwr_ctrl.Reg32);
		if (ret)
			return ret;

		do {
			ret = ReadBPCMRegister(PMB_ADDR_AIP,
					ARMBPCMRegOffset(arm_pwr_ctrl_0),
					&arm_pwr_ctrl.Reg32);
			if (ret)
				return ret;
		} while (arm_pwr_ctrl.Bits.pwr_ok_status != 0xf);

		arm_ctrl.Bits.pll_clamp_on = 0;

		ret = WriteBPCMRegister(PMB_ADDR_AIP,
				ARMBPCMRegOffset(arm_control),
				arm_ctrl.Reg32);
		if (ret)
			return ret;

		arm_pwr_ctrl.Bits.clamp_on = 0;

		ret = WriteBPCMRegister(PMB_ADDR_AIP,
				ARMBPCMRegOffset(arm_pwr_ctrl_0),
				arm_pwr_ctrl.Reg32);
		if (ret)
			return ret;

		/* 3) Power up CPU0 RAM */
		arm_pwr_ctrl.Bits.mem_pda &= 0xe;

		ret = WriteBPCMRegister(PMB_ADDR_AIP,
				ARMBPCMRegOffset(arm_pwr_ctrl_0), arm_pwr_ctrl.Reg32);
		if (ret)
			return ret;

		arm_pwr_ctrl.Bits.mem_pwr_on = 1;

		ret = WriteBPCMRegister(PMB_ADDR_AIP,
				ARMBPCMRegOffset(arm_pwr_ctrl_0), arm_pwr_ctrl.Reg32);
		if (ret)
			return ret;

		do {
			ret = ReadBPCMRegister(PMB_ADDR_AIP,
					ARMBPCMRegOffset(arm_pwr_ctrl_0), &arm_pwr_ctrl.Reg32);
			if (ret)
				return ret;
		} while (arm_pwr_ctrl.Bits.mem_pwr_on_status == 0);

		arm_pwr_ctrl.Bits.mem_pwr_ok = 1;

		ret = WriteBPCMRegister(PMB_ADDR_AIP,
				ARMBPCMRegOffset(arm_pwr_ctrl_0), arm_pwr_ctrl.Reg32);
		if (ret)
			return ret;

		do {
			ret = ReadBPCMRegister(PMB_ADDR_AIP,
					ARMBPCMRegOffset(arm_pwr_ctrl_0), &arm_pwr_ctrl.Reg32);
			if (ret)
				return ret;
		} while (arm_pwr_ctrl.Bits.mem_pwr_ok_status == 0);

		arm_pwr_ctrl.Bits.mem_clamp_on = 0;

		ret = WriteBPCMRegister(PMB_ADDR_AIP,
				ARMBPCMRegOffset(arm_pwr_ctrl_0),
				arm_pwr_ctrl.Reg32);
		if (ret)
			return ret;

		/* 4) Power up L2 cache */
		pmc_cpu_l2cache_power_up();

		/* 5) de-assert CPU0 reset */
		arm_ctrl.Bits.cpu0_reset_n = 1;

		ret = WriteBPCMRegister(PMB_ADDR_AIP,
				ARMBPCMRegOffset(arm_control),
				arm_ctrl.Reg32);
		if (ret)
			return ret;
	} else if (cpu == 1) {
		/* check whether CPU1 is up and running already.
		 * Assuming once cpu1_reset_n is set, cpu1 is powered on
		 * and running */
		ret = ReadBPCMRegister(PMB_ADDR_AIP,
				ARMBPCMRegOffset(arm_control),
				&arm_ctrl.Reg32);
		if (ret)
			return ret;
		if (arm_ctrl.Bits.cpu1_reset_n == 1)
			return 0;

		/* 1) Power up CPU1 */
		ret = ReadBPCMRegister(PMB_ADDR_AIP,
				ARMBPCMRegOffset(arm_pwr_ctrl_1),
				&arm_pwr_ctrl.Reg32);
		if (ret)
			return ret;
		arm_pwr_ctrl.Bits.pwr_on |= 0x3;

		ret = WriteBPCMRegister(PMB_ADDR_AIP,
				ARMBPCMRegOffset(arm_pwr_ctrl_1),
				arm_pwr_ctrl.Reg32);
		if (ret)
			return ret;

		do {
			ret = ReadBPCMRegister(PMB_ADDR_AIP,
					ARMBPCMRegOffset(arm_pwr_ctrl_1),
					&arm_pwr_ctrl.Reg32);
			if (ret)
				return ret;
		} while ((arm_pwr_ctrl.Bits.pwr_on_status & 0x3) != 0x3);

		arm_pwr_ctrl.Bits.pwr_ok |= 0x3;

		ret = WriteBPCMRegister(PMB_ADDR_AIP,
				ARMBPCMRegOffset(arm_pwr_ctrl_1),
				arm_pwr_ctrl.Reg32);
		if (ret)
			return ret;

		do {
			ret = ReadBPCMRegister(PMB_ADDR_AIP,
					ARMBPCMRegOffset(arm_pwr_ctrl_1),
					&arm_pwr_ctrl.Reg32);
			if (ret)
				return ret;
		} while ((arm_pwr_ctrl.Bits.pwr_ok_status & 0x3) != 0x3);

		arm_pwr_ctrl.Bits.clamp_on = 0;

		ret = WriteBPCMRegister(PMB_ADDR_AIP,
				ARMBPCMRegOffset(arm_pwr_ctrl_1),
				arm_pwr_ctrl.Reg32);
		if (ret)
			return ret;

		/* 2) Power up CPU1 RAM */
		arm_pwr_ctrl.Bits.mem_pda &= 0xe;

		ret = WriteBPCMRegister(PMB_ADDR_AIP,
				ARMBPCMRegOffset(arm_pwr_ctrl_1),
				arm_pwr_ctrl.Reg32);
		if (ret)
			return ret;

		arm_pwr_ctrl.Bits.mem_pwr_on = 1;

		ret = WriteBPCMRegister(PMB_ADDR_AIP,
				ARMBPCMRegOffset(arm_pwr_ctrl_1),
				arm_pwr_ctrl.Reg32);
		if (ret)
			return ret;

		do {
			ret = ReadBPCMRegister(PMB_ADDR_AIP,
					ARMBPCMRegOffset(arm_pwr_ctrl_1),
					&arm_pwr_ctrl.Reg32);
			if (ret)
				return ret;
		} while (arm_pwr_ctrl.Bits.mem_pwr_on_status == 0);

		arm_pwr_ctrl.Bits.mem_pwr_ok = 1;

		ret = WriteBPCMRegister(PMB_ADDR_AIP,
				ARMBPCMRegOffset(arm_pwr_ctrl_1),
				arm_pwr_ctrl.Reg32);
		if (ret)
			return ret;

		do {
			ret = ReadBPCMRegister(PMB_ADDR_AIP,
					ARMBPCMRegOffset(arm_pwr_ctrl_1),
					&arm_pwr_ctrl.Reg32);
			if (ret)
				return ret;
		} while (arm_pwr_ctrl.Bits.mem_pwr_ok_status == 0);

		arm_pwr_ctrl.Bits.mem_clamp_on = 0;

		ret = WriteBPCMRegister(PMB_ADDR_AIP,
				ARMBPCMRegOffset(arm_pwr_ctrl_1),
				arm_pwr_ctrl.Reg32);
		if (ret)
			return ret;

		/* 3) de-assert reset to CPU1 */
		arm_ctrl.Bits.cpu1_reset_n = 1;

		ret = WriteBPCMRegister(PMB_ADDR_AIP,
				ARMBPCMRegOffset(arm_control),
				arm_ctrl.Reg32);
		if (ret)
			return ret;
	} else {
		printk("error in %s: we do not have CPU#%d\n", __func__, cpu);
		return -1;
	}

	return 0;
}

/* this power_down function is implemented with assumption that CPU#0 will
 * always be the last one to be powered down */
/* Note: all the power_down codes have never been tested in 63138 */
int pmc_cpu_core_power_down(unsigned cpu)
{
	int ret;
	ARM_CONTROL_REG arm_ctrl;
	ARM_CPUx_PWR_CTRL_REG arm_pwr_ctrl;

	if (cpu == 0) {
		/* 1) assert reset to CPU0 */
		ret = ReadBPCMRegister(PMB_ADDR_AIP,
				ARMBPCMRegOffset(arm_control),
				&arm_ctrl.Reg32);
		if (ret)
			return ret;
		arm_ctrl.Bits.cpu0_reset_n = 0;

		ret = WriteBPCMRegister(PMB_ADDR_AIP,
				ARMBPCMRegOffset(arm_control),
				arm_ctrl.Reg32);
		if (ret)
			return ret;

		/* 2) power down L2 cache */
		pmc_cpu_l2cache_power_down();

		/* 3) power down CPU0 RAM */
		ret = ReadBPCMRegister(PMB_ADDR_AIP,
				ARMBPCMRegOffset(arm_pwr_ctrl_0),
				&arm_pwr_ctrl.Reg32);
		if (ret)
			return ret;
		arm_pwr_ctrl.Bits.mem_clamp_on = 1;

		ret = WriteBPCMRegister(PMB_ADDR_AIP,
				ARMBPCMRegOffset(arm_pwr_ctrl_0),
				arm_pwr_ctrl.Reg32);
		if (ret)
			return ret;

		arm_pwr_ctrl.Bits.mem_pwr_ok = 0;

		ret = WriteBPCMRegister(PMB_ADDR_AIP,
				ARMBPCMRegOffset(arm_pwr_ctrl_0),
				arm_pwr_ctrl.Reg32);
		if (ret)
			return ret;

		do {
			ret = ReadBPCMRegister(PMB_ADDR_AIP,
					ARMBPCMRegOffset(arm_pwr_ctrl_0),
					&arm_pwr_ctrl.Reg32);
			if (ret)
				return ret;
		} while (arm_pwr_ctrl.Bits.mem_pwr_ok_status == 0);

		arm_pwr_ctrl.Bits.mem_pwr_on = 0;

		ret = WriteBPCMRegister(PMB_ADDR_AIP,
				ARMBPCMRegOffset(arm_pwr_ctrl_0),
				arm_pwr_ctrl.Reg32);
		if (ret)
			return ret;

		do {
			ret = ReadBPCMRegister(PMB_ADDR_AIP,
					ARMBPCMRegOffset(arm_pwr_ctrl_0),
					&arm_pwr_ctrl.Reg32);
			if (ret)
				return ret;
		} while (arm_pwr_ctrl.Bits.mem_pwr_on_status == 0);

		arm_pwr_ctrl.Bits.mem_pda |= 0x1;

		ret = WriteBPCMRegister(PMB_ADDR_AIP,
				ARMBPCMRegOffset(arm_pwr_ctrl_0),
				arm_pwr_ctrl.Reg32);
		if (ret)
			return ret;

		/* 4) Power down CPU0 */

		arm_pwr_ctrl.Bits.clamp_on = 1;

		ret = WriteBPCMRegister(PMB_ADDR_AIP,
				ARMBPCMRegOffset(arm_pwr_ctrl_0),
				arm_pwr_ctrl.Reg32);
		if (ret)
			return ret;

		arm_ctrl.Bits.pll_clamp_on = 1;

		ret = WriteBPCMRegister(PMB_ADDR_AIP,
				ARMBPCMRegOffset(arm_control),
				arm_ctrl.Reg32);
		if (ret)
			return ret;

		arm_pwr_ctrl.Bits.pwr_ok = 0x0;

		ret = WriteBPCMRegister(PMB_ADDR_AIP,
				ARMBPCMRegOffset(arm_pwr_ctrl_0),
				arm_pwr_ctrl.Reg32);
		if (ret)
			return ret;

		do {
			ret = ReadBPCMRegister(PMB_ADDR_AIP,
					ARMBPCMRegOffset(arm_pwr_ctrl_0),
					&arm_pwr_ctrl.Reg32);
			if (ret)
				return ret;
		} while (arm_pwr_ctrl.Bits.pwr_ok_status != 0xf);

		arm_pwr_ctrl.Bits.pwr_on = 0x0;

		ret = WriteBPCMRegister(PMB_ADDR_AIP,
				ARMBPCMRegOffset(arm_pwr_ctrl_0),
				arm_pwr_ctrl.Reg32);
		if (ret)
			return ret;

		do {
			ret = ReadBPCMRegister(PMB_ADDR_AIP,
					ARMBPCMRegOffset(arm_pwr_ctrl_0),
					&arm_pwr_ctrl.Reg32);
			if (ret)
				return ret;
		} while (arm_pwr_ctrl.Bits.pwr_on_status != 0xf);

		/* 5) power down PLL */
		arm_ctrl.Bits.pll_pwr_on = 0;
		ret = WriteBPCMRegister(PMB_ADDR_AIP,
				ARMBPCMRegOffset(arm_control),
				arm_ctrl.Reg32);
		if (ret)
			return ret;

		/* wait at least 1.0 usec */
		udelay(2);

		arm_ctrl.Bits.pll_ldo_pwr_on = 0;
		ret = WriteBPCMRegister(PMB_ADDR_AIP,
				ARMBPCMRegOffset(arm_control),
				arm_ctrl.Reg32);
		if (ret)
			return ret;
	} else if (cpu == 1) {
		/* 1) assert reset to CPU1 */
		ret = ReadBPCMRegister(PMB_ADDR_AIP,
				ARMBPCMRegOffset(arm_control),
				&arm_ctrl.Reg32);
		if (ret)
			return ret;
		arm_ctrl.Bits.cpu1_reset_n = 0;

		ret = WriteBPCMRegister(PMB_ADDR_AIP,
				ARMBPCMRegOffset(arm_control),
				arm_ctrl.Reg32);
		if (ret)
			return ret;

		/* 2) Power down RAM CPU1 */
		ret = ReadBPCMRegister(PMB_ADDR_AIP,
				ARMBPCMRegOffset(arm_pwr_ctrl_1),
				&arm_pwr_ctrl.Reg32);
		if (ret)
			return ret;
		arm_pwr_ctrl.Bits.mem_clamp_on = 1;

		ret = WriteBPCMRegister(PMB_ADDR_AIP,
				ARMBPCMRegOffset(arm_pwr_ctrl_1),
				arm_pwr_ctrl.Reg32);
		if (ret)
			return ret;

		arm_pwr_ctrl.Bits.mem_pwr_ok = 0;

		ret = WriteBPCMRegister(PMB_ADDR_AIP,
				ARMBPCMRegOffset(arm_pwr_ctrl_1),
				arm_pwr_ctrl.Reg32);
		if (ret)
			return ret;

		do {
			ret = ReadBPCMRegister(PMB_ADDR_AIP,
					ARMBPCMRegOffset(arm_pwr_ctrl_1),
					&arm_pwr_ctrl.Reg32);
			if (ret)
				return ret;
		} while (arm_pwr_ctrl.Bits.mem_pwr_ok_status == 0);

		arm_pwr_ctrl.Bits.mem_pwr_on = 0;

		ret = WriteBPCMRegister(PMB_ADDR_AIP,
				ARMBPCMRegOffset(arm_pwr_ctrl_1),
				arm_pwr_ctrl.Reg32);
		if (ret)
			return ret;

		do {
			ret = ReadBPCMRegister(PMB_ADDR_AIP,
					ARMBPCMRegOffset(arm_pwr_ctrl_1),
					&arm_pwr_ctrl.Reg32);
			if (ret)
				return ret;
		} while (arm_pwr_ctrl.Bits.mem_pwr_on_status == 0);

		arm_pwr_ctrl.Bits.mem_pda |= 0x1;

		ret = WriteBPCMRegister(PMB_ADDR_AIP,
				ARMBPCMRegOffset(arm_pwr_ctrl_1),
				arm_pwr_ctrl.Reg32);
		if (ret)
			return ret;

		/* 3) Power Down CPU1 */
		arm_pwr_ctrl.Bits.clamp_on = 1;

		ret = WriteBPCMRegister(PMB_ADDR_AIP,
				ARMBPCMRegOffset(arm_pwr_ctrl_1),
				arm_pwr_ctrl.Reg32);
		if (ret)
			return ret;

		arm_pwr_ctrl.Bits.pwr_ok &= 0xc;

		ret = WriteBPCMRegister(PMB_ADDR_AIP,
				ARMBPCMRegOffset(arm_pwr_ctrl_1),
				arm_pwr_ctrl.Reg32);
		if (ret)
			return ret;

		do {
			ret = ReadBPCMRegister(PMB_ADDR_AIP,
					ARMBPCMRegOffset(arm_pwr_ctrl_1),
					&arm_pwr_ctrl.Reg32);
			if (ret)
				return ret;
		} while ((arm_pwr_ctrl.Bits.pwr_ok_status & 0x3) != 0x3);

		arm_pwr_ctrl.Bits.pwr_on &= 0xc;

		ret = WriteBPCMRegister(PMB_ADDR_AIP,
				ARMBPCMRegOffset(arm_pwr_ctrl_1),
				arm_pwr_ctrl.Reg32);
		if (ret)
			return ret;

		do {
			ret = ReadBPCMRegister(PMB_ADDR_AIP,
					ARMBPCMRegOffset(arm_pwr_ctrl_1),
					&arm_pwr_ctrl.Reg32);
			if (ret)
				return ret;
		} while ((arm_pwr_ctrl.Bits.pwr_on_status & 0x3) != 0x3);
	} else {
		printk("error in %s: we do not have CPU#%d\n", __func__, cpu);
		return -1;
	}

	return 0;
}

int pmc_cpu_l2cache_power_up(void)
{
	int ret;
	ARM_CPUx_PWR_CTRL_REG arm_pwr_ctrl;

	ret = ReadBPCMRegister(PMB_ADDR_AIP,
			ARMBPCMRegOffset(arm_neon_l2),
			&arm_pwr_ctrl.Reg32);
	if (ret)
		return ret;

	/* check if L2 cache is already power on. If it is, then just return 0 */
	if ((arm_pwr_ctrl.Bits.mem_clamp_on == 0) &&
			(arm_pwr_ctrl.Bits.mem_clamp_on == 0) &&
			(arm_pwr_ctrl.Bits.mem_pwr_ok == 0x1) &&
			(arm_pwr_ctrl.Bits.mem_pwr_on == 0x1))
		return 0;

	arm_pwr_ctrl.Bits.mem_pda = 0x0;	/* set 0 to bit#8:11 */

	ret = WriteBPCMRegister(PMB_ADDR_AIP,
			ARMBPCMRegOffset(arm_neon_l2),
			arm_pwr_ctrl.Reg32);
	if (ret)
		return ret;

	arm_pwr_ctrl.Bits.mem_pwr_on = 1;

	ret = WriteBPCMRegister(PMB_ADDR_AIP,
			ARMBPCMRegOffset(arm_neon_l2),
			arm_pwr_ctrl.Reg32);
	if (ret)
		return ret;

	do {
		ret = ReadBPCMRegister(PMB_ADDR_AIP,
				ARMBPCMRegOffset(arm_neon_l2),
				&arm_pwr_ctrl.Reg32);
		if (ret)
			return ret;
	} while (arm_pwr_ctrl.Bits.mem_pwr_on_status == 0);

	arm_pwr_ctrl.Bits.mem_pwr_ok = 1;

	ret = WriteBPCMRegister(PMB_ADDR_AIP,
			ARMBPCMRegOffset(arm_neon_l2),
			arm_pwr_ctrl.Reg32);
	if (ret)
		return ret;

	do {
		ret = ReadBPCMRegister(PMB_ADDR_AIP,
				ARMBPCMRegOffset(arm_neon_l2),
				&arm_pwr_ctrl.Reg32);
		if (ret)
			return ret;
	} while (arm_pwr_ctrl.Bits.mem_pwr_ok_status == 0);

	arm_pwr_ctrl.Bits.mem_clamp_on = 0;

	ret = WriteBPCMRegister(PMB_ADDR_AIP,
			ARMBPCMRegOffset(arm_neon_l2),
			arm_pwr_ctrl.Reg32);
	if (ret)
		return ret;

	return 0;
}

int pmc_cpu_l2cache_power_down(void)
{
	int ret;
	ARM_CPUx_PWR_CTRL_REG arm_pwr_ctrl;

	ret = ReadBPCMRegister(PMB_ADDR_AIP,
			ARMBPCMRegOffset(arm_neon_l2),
			&arm_pwr_ctrl.Reg32);
	if (ret)
		return ret;

	arm_pwr_ctrl.Bits.mem_clamp_on = 1;

	ret = WriteBPCMRegister(PMB_ADDR_AIP,
			ARMBPCMRegOffset(arm_neon_l2),
			arm_pwr_ctrl.Reg32);
	if (ret)
		return ret;

	arm_pwr_ctrl.Bits.mem_pwr_ok = 0;

	ret = WriteBPCMRegister(PMB_ADDR_AIP,
			ARMBPCMRegOffset(arm_neon_l2),
			arm_pwr_ctrl.Reg32);
	if (ret)
		return ret;

	do {
		ret = ReadBPCMRegister(PMB_ADDR_AIP,
				ARMBPCMRegOffset(arm_neon_l2),
				&arm_pwr_ctrl.Reg32);
		if (ret)
			return ret;
	} while (arm_pwr_ctrl.Bits.mem_pwr_ok_status == 0);

	arm_pwr_ctrl.Bits.mem_pwr_on = 0;

	ret = WriteBPCMRegister(PMB_ADDR_AIP,
			ARMBPCMRegOffset(arm_neon_l2),
			arm_pwr_ctrl.Reg32);
	if (ret)
		return ret;

	do {
		ret = ReadBPCMRegister(PMB_ADDR_AIP,
				ARMBPCMRegOffset(arm_neon_l2),
				&arm_pwr_ctrl.Reg32);
		if (ret)
			return ret;
	} while (arm_pwr_ctrl.Bits.mem_pwr_on_status == 0);

	arm_pwr_ctrl.Bits.mem_pda = 0xf;	/* set 0xf to bit#8:11 */

	ret = WriteBPCMRegister(PMB_ADDR_AIP,
			ARMBPCMRegOffset(arm_neon_l2),
			arm_pwr_ctrl.Reg32);
	if (ret)
		return ret;

	return 0;
}

int pmc_cpu_neon_power_up(unsigned cpu)
{
	int ret;
	ARM_CONTROL_REG arm_ctrl;
	ARM_CPUx_PWR_CTRL_REG arm_pwr_ctrl;

	ret = ReadBPCMRegister(PMB_ADDR_AIP,
			ARMBPCMRegOffset(arm_control),
			&arm_ctrl.Reg32);
	if (ret)
		return ret;

	ret = ReadBPCMRegister(PMB_ADDR_AIP,
			ARMBPCMRegOffset(arm_neon_l2),
			&arm_pwr_ctrl.Reg32);
	if (ret)
		return ret;

	/* check if neon is on and running aready. */
	if ((arm_ctrl.Bits.neon_reset_n == 1) &&
			(arm_pwr_ctrl.Bits.clamp_on == 0) &&
			(arm_pwr_ctrl.Bits.pwr_ok & 0x1) &&
			(arm_pwr_ctrl.Bits.pwr_on & 0x1))
		return 0;

	/* 1) Power up Neon */
	arm_pwr_ctrl.Bits.pwr_on |= 1;

	ret = WriteBPCMRegister(PMB_ADDR_AIP,
			ARMBPCMRegOffset(arm_neon_l2),
			arm_pwr_ctrl.Reg32);
	if (ret)
		return ret;

	do {
		ret = ReadBPCMRegister(PMB_ADDR_AIP,
				ARMBPCMRegOffset(arm_neon_l2),
				&arm_pwr_ctrl.Reg32);
		if (ret)
			return ret;
	} while ((arm_pwr_ctrl.Bits.pwr_on_status & 0x1) == 0);

	arm_pwr_ctrl.Bits.pwr_ok |= 1;

	ret = WriteBPCMRegister(PMB_ADDR_AIP,
			ARMBPCMRegOffset(arm_neon_l2),
			arm_pwr_ctrl.Reg32);
	if (ret)
		return ret;

	do {
		ret = ReadBPCMRegister(PMB_ADDR_AIP,
				ARMBPCMRegOffset(arm_neon_l2),
				&arm_pwr_ctrl.Reg32);
		if (ret)
			return ret;
	} while ((arm_pwr_ctrl.Bits.pwr_ok_status & 0x1) == 0);

	arm_pwr_ctrl.Bits.clamp_on = 0;

	ret = WriteBPCMRegister(PMB_ADDR_AIP,
			ARMBPCMRegOffset(arm_neon_l2),
			arm_pwr_ctrl.Reg32);
	if (ret)
		return ret;

	/* 2) De-assert reset to Neon */
	arm_ctrl.Bits.neon_reset_n = 1;

	ret = WriteBPCMRegister(PMB_ADDR_AIP,
			ARMBPCMRegOffset(arm_control),
			arm_ctrl.Reg32);
	if (ret)
		return ret;

	return 0;
}

int pmc_cpu_neon_power_down(unsigned cpu)
{
	int ret;
	ARM_CONTROL_REG arm_ctrl;
	ARM_CPUx_PWR_CTRL_REG arm_pwr_ctrl;

	ret = ReadBPCMRegister(PMB_ADDR_AIP,
			ARMBPCMRegOffset(arm_control),
			&arm_ctrl.Reg32);
	if (ret)
		return ret;

	ret = ReadBPCMRegister(PMB_ADDR_AIP,
			ARMBPCMRegOffset(arm_neon_l2),
			&arm_pwr_ctrl.Reg32);
	if (ret)
		return ret;

	/* 1) assert reset to Neon */
	arm_ctrl.Bits.neon_reset_n = 0;

	ret = WriteBPCMRegister(PMB_ADDR_AIP,
			ARMBPCMRegOffset(arm_control),
			arm_ctrl.Reg32);
	if (ret)
		return ret;

	/* 2) Power down Neon */
	arm_pwr_ctrl.Bits.clamp_on = 1;

	ret = WriteBPCMRegister(PMB_ADDR_AIP,
			ARMBPCMRegOffset(arm_neon_l2),
			arm_pwr_ctrl.Reg32);
	if (ret)
		return ret;

	arm_pwr_ctrl.Bits.pwr_ok &= 0xe;

	ret = WriteBPCMRegister(PMB_ADDR_AIP,
			ARMBPCMRegOffset(arm_neon_l2),
			arm_pwr_ctrl.Reg32);
	if (ret)
		return ret;

	do {
		ret = ReadBPCMRegister(PMB_ADDR_AIP,
				ARMBPCMRegOffset(arm_neon_l2),
				&arm_pwr_ctrl.Reg32);
		if (ret)
			return ret;
	} while ((arm_pwr_ctrl.Bits.pwr_ok_status & 0x1) == 1);

	arm_pwr_ctrl.Bits.pwr_on &= 0xe;

	ret = WriteBPCMRegister(PMB_ADDR_AIP,
			ARMBPCMRegOffset(arm_neon_l2),
			arm_pwr_ctrl.Reg32);
	if (ret)
		return ret;

	do {
		ret = ReadBPCMRegister(PMB_ADDR_AIP,
				ARMBPCMRegOffset(arm_neon_l2),
				&arm_pwr_ctrl.Reg32);
		if (ret)
			return ret;
	} while ((arm_pwr_ctrl.Bits.pwr_on_status & 0x1) == 1);

	return 0;
}

#else

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
#if defined (PLATFORM_FLAVOR_63148)
	B15CTRL->cpu_ctrl.cpu1_pwr_zone_ctrl |= 0x400;
	B15CTRL->cpu_ctrl.reset_cfg &= 0xfffffffd;
#elif defined (PMB_ADDR_ORION_CPU0) || defined (PLATFORM_FLAVOR_6846)
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
#endif /* PLATFORM_FLAVOR_63138 */
