/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2015
 * Toradex, Inc.
 *
 * Authors: Stefan Agner
 *          Sanchayan Maity
 */

#ifndef __ASM_ARCH_VF610_DDRMC_H
#define __ASM_ARCH_VF610_DDRMC_H

#include <asm/arch/iomux-vf610.h>

struct ddr3_jedec_timings {
	u8 tinit;
	u32 trst_pwron;
	u32 cke_inactive;
	u8 wrlat;
	u8 caslat_lin;
	u8 trc;
	u8 trrd;
	u8 tccd;
	u8 tbst_int_interval;
	u8 tfaw;
	u8 trp;
	u8 twtr;
	u8 tras_min;
	u8 tmrd;
	u8 trtp;
	u32 tras_max;
	u8 tmod;
	u8 tckesr;
	u8 tcke;
	u8 trcd_int;
	u8 tras_lockout;
	u8 tdal;
	u8 bstlen;
	u16 tdll;
	u8 trp_ab;
	u16 tref;
	u8 trfc;
	u16 tref_int;
	u8 tpdex;
	u8 txpdll;
	u8 txsnr;
	u16 txsr;
	u8 cksrx;
	u8 cksre;
	u8 freq_chg_en;
	u16 zqcl;
	u16 zqinit;
	u8 zqcs;
	u8 ref_per_zq;
	u8 zqcs_rotate;
	u8 aprebit;
	u8 cmd_age_cnt;
	u8 age_cnt;
	u8 q_fullness;
	u8 odt_rd_mapcs0;
	u8 odt_wr_mapcs0;
	u8 wlmrd;
	u8 wldqsen;
};

struct ddrmc_cr_setting {
	u32	setting;
	int	cr_rnum; /* CR register ; -1 for last entry */
};

struct ddrmc_phy_setting {
	u32	setting;
	int	phy_rnum; /* PHY register ; -1 for last entry */
};

void ddrmc_setup_iomux(const iomux_v3_cfg_t *pads, int pads_count);
void ddrmc_phy_init(void);
void ddrmc_ctrl_init_ddr3(struct ddr3_jedec_timings const *timings,
			  struct ddrmc_cr_setting *board_cr_settings,
			  struct ddrmc_phy_setting *board_phy_settings,
			  int col_diff, int row_diff);

#endif
