/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2015 Google, Inc
 *
 * Copyright 2014 Rockchip Inc.
 */

#ifndef _ASM_ARCH_PMU_RK3288_H
#define _ASM_ARCH_PMU_RK3288_H

struct rk3288_pmu {
	u32 wakeup_cfg[2];
	u32 pwrdn_con;
	u32 pwrdn_st;

	u32 idle_req;
	u32 idle_st;
	u32 pwrmode_con;
	u32 pwr_state;

	u32 osc_cnt;
	u32 pll_cnt;
	u32 stabl_cnt;
	u32 ddr0io_pwron_cnt;

	u32 ddr1io_pwron_cnt;
	u32 core_pwrdn_cnt;
	u32 core_pwrup_cnt;
	u32 gpu_pwrdn_cnt;

	u32 gpu_pwrup_cnt;
	u32 wakeup_rst_clr_cnt;
	u32 sft_con;
	u32 ddr_sref_st;

	u32 int_con;
	u32 int_st;
	u32 boot_addr_sel;
	u32 grf_con;

	u32 gpio_sr;
	u32 gpio0pull[3];

	u32 gpio0drv[3];
	u32 gpio_op;

	u32 gpio0_sel18;	/* 0x80 */
	u32 gpio0_iomux[4];	/* a, b, c, d */
	u32 sys_reg[4];
};
check_member(rk3288_pmu, sys_reg[3], 0x00a0);

enum {
	PMU_GPIO0_A	= 0,
	PMU_GPIO0_B,
	PMU_GPIO0_C,
	PMU_GPIO0_D,
};

/* PMU_GPIO0_B_IOMUX */
enum {
	GPIO0_B7_SHIFT		= 14,
	GPIO0_B7_MASK		= 1,
	GPIO0_B7_GPIOB7		= 0,
	GPIO0_B7_I2C0PMU_SDA,

	GPIO0_B5_SHIFT		= 10,
	GPIO0_B5_MASK		= 1,
	GPIO0_B5_GPIOB5		= 0,
	GPIO0_B5_CLK_27M,

	GPIO0_B2_SHIFT		= 4,
	GPIO0_B2_MASK		= 1,
	GPIO0_B2_GPIOB2		= 0,
	GPIO0_B2_TSADC_INT,
};

/* PMU_GPIO0_C_IOMUX */
enum {
	GPIO0_C1_SHIFT		= 2,
	GPIO0_C1_MASK		= 3,
	GPIO0_C1_GPIOC1		= 0,
	GPIO0_C1_TEST_CLKOUT,
	GPIO0_C1_CLKT1_27M,

	GPIO0_C0_SHIFT		= 0,
	GPIO0_C0_MASK		= 1,
	GPIO0_C0_GPIOC0		= 0,
	GPIO0_C0_I2C0PMU_SCL,
};

#endif
