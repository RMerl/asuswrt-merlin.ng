/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2017 Jorge Ramirez-Ortiz <jorge.ramirez-ortiz@linaro.org>
 */
#ifndef _CLOCK_QCOM_H
#define _CLOCK_QCOM_H

#include <asm/io.h>

#define CFG_CLK_SRC_CXO   (0 << 8)
#define CFG_CLK_SRC_GPLL0 (1 << 8)
#define CFG_CLK_SRC_GPLL0_EVEN (6 << 8)
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

struct freq_tbl {
	uint freq;
	uint src;
	u8 pre_div;
	u16 m;
	u16 n;
};

#define F(f, s, h, m, n) { (f), (s), (2 * (h) - 1), (m), (n) }

struct gate_clk {
	uintptr_t reg;
	u32 en_val;
	const char *name;
};

#ifdef DEBUG
#define GATE_CLK(clk, reg, val) [clk] = { reg, val, #clk }
#else
#define GATE_CLK(clk, reg, val) [clk] = { reg, val, NULL }
#endif

struct qcom_reset_map {
	unsigned int reg;
	u8 bit;
};

struct clk;

struct msm_clk_data {
	const struct qcom_reset_map	*resets;
	unsigned long			num_resets;
	const struct gate_clk		*clks;
	unsigned long			num_clks;

	int (*enable)(struct clk *clk);
	unsigned long (*set_rate)(struct clk *clk, unsigned long rate);
};

struct msm_clk_priv {
	phys_addr_t		base;
	struct msm_clk_data	*data;
};

int qcom_cc_bind(struct udevice *parent);
void clk_enable_gpll0(phys_addr_t base, const struct pll_vote_clk *gpll0);
void clk_bcr_update(phys_addr_t apps_cmd_rgcr);
void clk_enable_cbc(phys_addr_t cbcr);
void clk_enable_vote_clk(phys_addr_t base, const struct vote_clk *vclk);
const struct freq_tbl *qcom_find_freq(const struct freq_tbl *f, uint rate);
void clk_rcg_set_rate_mnd(phys_addr_t base, const struct bcr_regs *regs,
			  int div, int m, int n, int source, u8 mnd_width);
void clk_rcg_set_rate(phys_addr_t base, const struct bcr_regs *regs, int div,
		      int source);

static inline void qcom_gate_clk_en(const struct msm_clk_priv *priv, unsigned long id)
{
	u32 val;
	if (id >= priv->data->num_clks || priv->data->clks[id].reg == 0)
		return;

	val = readl(priv->base + priv->data->clks[id].reg);
	writel(val | priv->data->clks[id].en_val, priv->base + priv->data->clks[id].reg);
}

#endif
