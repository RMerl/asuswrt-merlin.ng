/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2016 Heiko Stuebner <heiko@sntech.de>
 */

#ifndef _ASM_ARCH_PMU_RK3188_H
#define _ASM_ARCH_PMU_RK3188_H

struct rk3188_pmu {
	u32 wakeup_cfg[2];
	u32 pwrdn_con;
	u32 pwrdn_st;

	u32 int_con;
	u32 int_st;
	u32 misc_con;

	u32 osc_cnt;
	u32 pll_cnt;
	u32 pmu_cnt;
	u32 ddrio_pwron_cnt;
	u32 wakeup_rst_clr_cnt;
	u32 scu_pwrdwn_cnt;
	u32 scu_pwrup_cnt;
	u32 misc_con1;
	u32 gpio0_con;

	u32 sys_reg[4];
	u32 reserved0[4];
	u32 stop_int_dly;
	u32 gpio0_p[2];
};
check_member(rk3188_pmu, gpio0_p[1], 0x0068);

#endif
