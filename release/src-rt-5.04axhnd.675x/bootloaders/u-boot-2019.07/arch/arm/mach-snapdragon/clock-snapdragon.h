/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Qualcomm APQ8016, APQ8096
 *
 * (C) Copyright 2017 Jorge Ramirez-Ortiz <jorge.ramirez-ortiz@linaro.org>
 */
#ifndef _CLOCK_SNAPDRAGON_H
#define _CLOCK_SNAPDRAGON_H

#define CFG_CLK_SRC_CXO   (0 << 8)
#define CFG_CLK_SRC_GPLL0 (1 << 8)
#define CFG_CLK_SRC_MASK  (7 << 8)

struct pll_vote_clk {
	uintptr_t status;
	int status_bit;
	uintptr_t ena_vote;
	int vote_bit;
};

struct vote_clk {
	uintptr_t cbcr_reg;
	uintptr_t ena_vote;
	int vote_bit;
};
struct bcr_regs {
	uintptr_t cfg_rcgr;
	uintptr_t cmd_rcgr;
	uintptr_t M;
	uintptr_t N;
	uintptr_t D;
};

struct msm_clk_priv {
	phys_addr_t base;
};

void clk_enable_gpll0(phys_addr_t base, const struct pll_vote_clk *gpll0);
void clk_bcr_update(phys_addr_t apps_cmd_rgcr);
void clk_enable_cbc(phys_addr_t cbcr);
void clk_enable_vote_clk(phys_addr_t base, const struct vote_clk *vclk);
void clk_rcg_set_rate_mnd(phys_addr_t base, const struct bcr_regs *regs,
			  int div, int m, int n, int source);

#endif
