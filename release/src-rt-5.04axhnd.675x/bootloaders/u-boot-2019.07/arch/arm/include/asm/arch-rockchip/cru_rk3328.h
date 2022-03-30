/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2016 Rockchip Electronics Co., Ltd
 */

#ifndef __ASM_ARCH_CRU_RK3328_H_
#define __ASM_ARCH_CRU_RK3328_H_

#include <common.h>

struct rk3328_clk_priv {
	struct rk3328_cru *cru;
	ulong rate;
};

struct rk3328_cru {
	u32 apll_con[5];
	u32 reserved1[3];
	u32 dpll_con[5];
	u32 reserved2[3];
	u32 cpll_con[5];
	u32 reserved3[3];
	u32 gpll_con[5];
	u32 reserved4[3];
	u32 mode_con;
	u32 misc;
	u32 reserved5[2];
	u32 glb_cnt_th;
	u32 glb_rst_st;
	u32 glb_srst_snd_value;
	u32 glb_srst_fst_value;
	u32 npll_con[5];
	u32 reserved6[(0x100 - 0xb4) / 4];
	u32 clksel_con[53];
	u32 reserved7[(0x200 - 0x1d4) / 4];
	u32 clkgate_con[29];
	u32 reserved8[3];
	u32 ssgtbl[32];
	u32 softrst_con[12];
	u32 reserved9[(0x380 - 0x330) / 4];
	u32 sdmmc_con[2];
	u32 sdio_con[2];
	u32 emmc_con[2];
	u32 sdmmc_ext_con[2];
};
check_member(rk3328_cru, sdmmc_ext_con[1], 0x39c);
#define MHz		1000000
#define KHz		1000
#define OSC_HZ		(24 * MHz)
#define APLL_HZ		(600 * MHz)
#define GPLL_HZ		(576 * MHz)
#define CPLL_HZ		(594 * MHz)

#define CLK_CORE_HZ	(600 * MHz)
#define ACLKM_CORE_HZ	(300 * MHz)
#define PCLK_DBG_HZ	(300 * MHz)

#define PERIHP_ACLK_HZ	(144000 * KHz)
#define PERIHP_HCLK_HZ	(72000 * KHz)
#define PERIHP_PCLK_HZ	(72000 * KHz)

#define PWM_CLOCK_HZ    (74 * MHz)

enum apll_frequencies {
	APLL_816_MHZ,
	APLL_600_MHZ,
};

#endif	/* __ASM_ARCH_CRU_RK3328_H_ */
