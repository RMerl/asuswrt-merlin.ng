/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2016-2017 Rockchip Electronics Co., Ltd
 */

#ifndef _ASM_ARCH_SDRAM_RK3399_H
#define _ASM_ARCH_SDRAM_RK3399_H
#include <asm/arch-rockchip/sdram_common.h>
#include <asm/arch-rockchip/sdram_msch.h>
#ifndef __ASSEMBLY__
#include <linux/bitops.h>
#endif

struct rk3399_ddr_pctl_regs {
	u32 denali_ctl[332];
};

struct rk3399_ddr_publ_regs {
	u32 denali_phy[959];
};

struct rk3399_ddr_pi_regs {
	u32 denali_pi[200];
};

struct rk3399_ddr_cic_regs {
	u32 cic_ctrl0;
	u32 cic_ctrl1;
	u32 cic_idle_th;
	u32 cic_cg_wait_th;
	u32 cic_status0;
	u32 cic_status1;
	u32 cic_ctrl2;
	u32 cic_ctrl3;
	u32 cic_ctrl4;
};

/* DENALI_CTL_00 */
#define START		1

/* DENALI_CTL_68 */
#define PWRUP_SREFRESH_EXIT	BIT(16)

/* DENALI_CTL_274 */
#define MEM_RST_VALID	1

struct msch_regs {
	u32 coreid;
	u32 revisionid;
	u32 ddrconf;
	u32 ddrsize;
	union noc_ddrtiminga0 ddrtiminga0;
	union noc_ddrtimingb0 ddrtimingb0;
	union noc_ddrtimingc0 ddrtimingc0;
	union noc_devtodev0 devtodev0;
	u32 reserved0[(0x110 - 0x20) / 4];
	union noc_ddrmode ddrmode;
	u32 reserved1[(0x1000 - 0x114) / 4];
	u32 agingx0;
};

struct sdram_msch_timings {
	union noc_ddrtiminga0 ddrtiminga0;
	union noc_ddrtimingb0 ddrtimingb0;
	union noc_ddrtimingc0 ddrtimingc0;
	union noc_devtodev0 devtodev0;
	union noc_ddrmode ddrmode;
	u32 agingx0;
};

struct rk3399_sdram_channel {
	struct sdram_cap_info cap_info;
	struct sdram_msch_timings noc_timings;
};

struct rk3399_sdram_params {
	struct rk3399_sdram_channel ch[2];
	struct sdram_base_params base;
	struct rk3399_ddr_pctl_regs pctl_regs;
	struct rk3399_ddr_pi_regs pi_regs;
	struct rk3399_ddr_publ_regs phy_regs;
};

#define PI_CA_TRAINING		BIT(0)
#define PI_WRITE_LEVELING	BIT(1)
#define PI_READ_GATE_TRAINING	BIT(2)
#define PI_READ_LEVELING	BIT(3)
#define PI_WDQ_LEVELING		BIT(4)
#define PI_FULL_TRAINING	0xff

enum {
	STRIDE_128B = 0,
	STRIDE_256B = 1,
	STRIDE_512B = 2,
	STRIDE_4KB = 3,
	UN_STRIDE = 4,
	PART_STRIDE = 5,
};

#endif
