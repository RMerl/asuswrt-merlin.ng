/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2016 Rockchip Electronics Co., Ltd
 */

#ifndef __ASM_ARCH_CRU_RK3399_H_
#define __ASM_ARCH_CRU_RK3399_H_

/* Private data for the clock driver - used by rockchip_get_cru() */
struct rk3399_clk_priv {
	struct rockchip_cru *cru;
};

struct rk3399_pmuclk_priv {
	struct rk3399_pmucru *pmucru;
};

struct rk3399_pmucru {
	u32 ppll_con[6];
	u32 reserved[0x1a];
	u32 pmucru_clksel[6];
	u32 pmucru_clkfrac_con[2];
	u32 reserved2[0x18];
	u32 pmucru_clkgate_con[3];
	u32 reserved3;
	u32 pmucru_softrst_con[2];
	u32 reserved4[2];
	u32 pmucru_rstnhold_con[2];
	u32 reserved5[2];
	u32 pmucru_gatedis_con[2];
};
check_member(rk3399_pmucru, pmucru_gatedis_con[1], 0x134);

struct rockchip_cru {
	u32 apll_l_con[6];
	u32 reserved[2];
	u32 apll_b_con[6];
	u32 reserved1[2];
	u32 dpll_con[6];
	u32 reserved2[2];
	u32 cpll_con[6];
	u32 reserved3[2];
	u32 gpll_con[6];
	u32 reserved4[2];
	u32 npll_con[6];
	u32 reserved5[2];
	u32 vpll_con[6];
	u32 reserved6[0x0a];
	u32 clksel_con[108];
	u32 reserved7[0x14];
	u32 clkgate_con[35];
	u32 reserved8[0x1d];
	u32 softrst_con[21];
	u32 reserved9[0x2b];
	u32 glb_srst_fst_value;
	u32 glb_srst_snd_value;
	u32 glb_cnt_th;
	u32 misc_con;
	u32 glb_rst_con;
	u32 glb_rst_st;
	u32 reserved10[0x1a];
	u32 sdmmc_con[2];
	u32 sdio0_con[2];
	u32 sdio1_con[2];
};
check_member(rockchip_cru, sdio1_con[1], 0x594);
#define KHz		1000
#define OSC_HZ		(24*MHz)
#define LPLL_HZ		(600*MHz)
#define BPLL_HZ		(600*MHz)
#define GPLL_HZ		(594*MHz)
#define CPLL_HZ		(384*MHz)
#define PPLL_HZ		(676*MHz)

#define PMU_PCLK_HZ	(48*MHz)

#define ACLKM_CORE_L_HZ	(300*MHz)
#define ATCLK_CORE_L_HZ	(300*MHz)
#define PCLK_DBG_L_HZ	(100*MHz)

#define ACLKM_CORE_B_HZ	(300*MHz)
#define ATCLK_CORE_B_HZ	(300*MHz)
#define PCLK_DBG_B_HZ	(100*MHz)

#define PERIHP_ACLK_HZ	(148500*KHz)
#define PERIHP_HCLK_HZ	(148500*KHz)
#define PERIHP_PCLK_HZ	(37125*KHz)

#define PERILP0_ACLK_HZ	(99000*KHz)
#define PERILP0_HCLK_HZ	(99000*KHz)
#define PERILP0_PCLK_HZ	(49500*KHz)

#define PERILP1_HCLK_HZ	(99000*KHz)
#define PERILP1_PCLK_HZ	(49500*KHz)

#define PWM_CLOCK_HZ    PMU_PCLK_HZ

enum apll_l_frequencies {
	APLL_L_1600_MHZ,
	APLL_L_600_MHZ,
};

enum apll_b_frequencies {
	APLL_B_600_MHZ,
};

void rk3399_configure_cpu_l(struct rockchip_cru *cru,
			    enum apll_l_frequencies apll_l_freq);
void rk3399_configure_cpu_b(struct rockchip_cru *cru,
			    enum apll_b_frequencies apll_b_freq);

#endif	/* __ASM_ARCH_CRU_RK3399_H_ */
