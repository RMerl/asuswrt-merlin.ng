/*
 * Memory setup for board based on EXYNOS4210
 *
 * Copyright (C) 2013 Samsung Electronics
 * Rajeshwari Shinde <rajeshwari.s@samsung.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <config.h>
#include <asm/arch/dmc.h>
#include "common_setup.h"
#include "exynos4_setup.h"

struct mem_timings mem = {
	.direct_cmd_msr = {
		DIRECT_CMD1, DIRECT_CMD2, DIRECT_CMD3, DIRECT_CMD4
	},
	.timingref = TIMINGREF_VAL,
	.timingrow = TIMINGROW_VAL,
	.timingdata = TIMINGDATA_VAL,
	.timingpower = TIMINGPOWER_VAL,
	.zqcontrol = ZQ_CONTROL_VAL,
	.control0 = CONTROL0_VAL,
	.control1 = CONTROL1_VAL,
	.control2 = CONTROL2_VAL,
	.concontrol = CONCONTROL_VAL,
	.prechconfig = PRECHCONFIG,
	.memcontrol = MEMCONTROL_VAL,
	.memconfig0 = MEMCONFIG0_VAL,
	.memconfig1 = MEMCONFIG1_VAL,
	.dll_resync = FORCE_DLL_RESYNC,
	.dll_on = DLL_CONTROL_ON,
};
static void phy_control_reset(int ctrl_no, struct exynos4_dmc *dmc)
{
	if (ctrl_no) {
		writel((mem.control1 | (1 << mem.dll_resync)),
		       &dmc->phycontrol1);
		writel((mem.control1 | (0 << mem.dll_resync)),
		       &dmc->phycontrol1);
	} else {
		writel((mem.control0 | (0 << mem.dll_on)),
		       &dmc->phycontrol0);
		writel((mem.control0 | (1 << mem.dll_on)),
		       &dmc->phycontrol0);
	}
}

static void dmc_config_mrs(struct exynos4_dmc *dmc, int chip)
{
	int i;
	unsigned long mask = 0;

	if (chip)
		mask = DIRECT_CMD_CHIP1_SHIFT;

	for (i = 0; i < MEM_TIMINGS_MSR_COUNT; i++) {
		writel(mem.direct_cmd_msr[i] | mask,
		       &dmc->directcmd);
	}
}

static void dmc_init(struct exynos4_dmc *dmc)
{
	/*
	 * DLL Parameter Setting:
	 * Termination: Enable R/W
	 * Phase Delay for DQS Cleaning: 180' Shift
	 */
	writel(mem.control1, &dmc->phycontrol1);

	/*
	 * ZQ Calibration
	 * Termination: Disable
	 * Auto Calibration Start: Enable
	 */
	writel(mem.zqcontrol, &dmc->phyzqcontrol);
	sdelay(0x100000);

	/*
	 * Update DLL Information:
	 * Force DLL Resyncronization
	 */
	phy_control_reset(1, dmc);
	phy_control_reset(0, dmc);

	/* Set DLL Parameters */
	writel(mem.control1, &dmc->phycontrol1);

	/* DLL Start */
	writel((mem.control0 | CTRL_START | CTRL_DLL_ON), &dmc->phycontrol0);

	writel(mem.control2, &dmc->phycontrol2);

	/* Set Clock Ratio of Bus clock to Memory Clock */
	writel(mem.concontrol, &dmc->concontrol);

	/*
	 * Memor Burst length: 8
	 * Number of chips: 2
	 * Memory Bus width: 32 bit
	 * Memory Type: DDR3
	 * Additional Latancy for PLL: 1 Cycle
	 */
	writel(mem.memcontrol, &dmc->memcontrol);

	writel(mem.memconfig0, &dmc->memconfig0);
	writel(mem.memconfig1, &dmc->memconfig1);

	/* Config Precharge Policy */
	writel(mem.prechconfig, &dmc->prechconfig);
	/*
	 * TimingAref, TimingRow, TimingData, TimingPower Setting:
	 * Values as per Memory AC Parameters
	 */
	writel(mem.timingref, &dmc->timingref);
	writel(mem.timingrow, &dmc->timingrow);
	writel(mem.timingdata, &dmc->timingdata);
	writel(mem.timingpower, &dmc->timingpower);

	/* Chip0: NOP Command: Assert and Hold CKE to high level */
	writel(DIRECT_CMD_NOP, &dmc->directcmd);
	sdelay(0x100000);

	/* Chip0: EMRS2, EMRS3, EMRS, MRS Commands Using Direct Command */
	dmc_config_mrs(dmc, 0);
	sdelay(0x100000);

	/* Chip0: ZQINIT */
	writel(DIRECT_CMD_ZQ, &dmc->directcmd);
	sdelay(0x100000);

	writel((DIRECT_CMD_NOP | DIRECT_CMD_CHIP1_SHIFT), &dmc->directcmd);
	sdelay(0x100000);

	/* Chip1: EMRS2, EMRS3, EMRS, MRS Commands Using Direct Command */
	dmc_config_mrs(dmc, 1);
	sdelay(0x100000);

	/* Chip1: ZQINIT */
	writel((DIRECT_CMD_ZQ | DIRECT_CMD_CHIP1_SHIFT), &dmc->directcmd);
	sdelay(0x100000);

	phy_control_reset(1, dmc);
	sdelay(0x100000);

	/* turn on DREX0, DREX1 */
	writel((mem.concontrol | AREF_EN), &dmc->concontrol);
}

void mem_ctrl_init(int reset)
{
	struct exynos4_dmc *dmc;

	/*
	 * Async bridge configuration at CPU_core:
	 * 1: half_sync
	 * 0: full_sync
	 */
	writel(1, ASYNC_CONFIG);
#ifdef CONFIG_ORIGEN
	/* Interleave: 2Bit, Interleave_bit1: 0x15, Interleave_bit0: 0x7 */
	writel(APB_SFR_INTERLEAVE_CONF_VAL, EXYNOS4_MIU_BASE +
		APB_SFR_INTERLEAVE_CONF_OFFSET);
	/* Update MIU Configuration */
	writel(APB_SFR_ARBRITATION_CONF_VAL, EXYNOS4_MIU_BASE +
		APB_SFR_ARBRITATION_CONF_OFFSET);
#else
	writel(APB_SFR_INTERLEAVE_CONF_VAL, EXYNOS4_MIU_BASE +
		APB_SFR_INTERLEAVE_CONF_OFFSET);
	writel(INTERLEAVE_ADDR_MAP_START_ADDR, EXYNOS4_MIU_BASE +
		ABP_SFR_INTERLEAVE_ADDRMAP_START_OFFSET);
	writel(INTERLEAVE_ADDR_MAP_END_ADDR, EXYNOS4_MIU_BASE +
		ABP_SFR_INTERLEAVE_ADDRMAP_END_OFFSET);
	writel(INTERLEAVE_ADDR_MAP_EN, EXYNOS4_MIU_BASE +
		ABP_SFR_SLV_ADDRMAP_CONF_OFFSET);
#ifdef CONFIG_MIU_LINEAR
	writel(SLAVE0_SINGLE_ADDR_MAP_START_ADDR, EXYNOS4_MIU_BASE +
		ABP_SFR_SLV0_SINGLE_ADDRMAP_START_OFFSET);
	writel(SLAVE0_SINGLE_ADDR_MAP_END_ADDR, EXYNOS4_MIU_BASE +
		ABP_SFR_SLV0_SINGLE_ADDRMAP_END_OFFSET);
	writel(SLAVE1_SINGLE_ADDR_MAP_START_ADDR, EXYNOS4_MIU_BASE +
		ABP_SFR_SLV1_SINGLE_ADDRMAP_START_OFFSET);
	writel(SLAVE1_SINGLE_ADDR_MAP_END_ADDR, EXYNOS4_MIU_BASE +
		ABP_SFR_SLV1_SINGLE_ADDRMAP_END_OFFSET);
	writel(APB_SFR_SLV_ADDR_MAP_CONF_VAL, EXYNOS4_MIU_BASE +
		ABP_SFR_SLV_ADDRMAP_CONF_OFFSET);
#endif
#endif
	/* DREX0 */
	dmc = (struct exynos4_dmc *)samsung_get_base_dmc_ctrl();
	dmc_init(dmc);
	dmc = (struct exynos4_dmc *)(samsung_get_base_dmc_ctrl()
					+ DMC_OFFSET);
	dmc_init(dmc);
}
