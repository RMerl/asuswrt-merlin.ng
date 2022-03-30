// SPDX-License-Identifier: BSD-3-Clause
/*
 * Clock drivers for Qualcomm APQ8096
 *
 * (C) Copyright 2017 Jorge Ramirez Ortiz <jorge.ramirez-ortiz@linaro.org>
 *
 * Based on Little Kernel driver, simplified
 */

#include <common.h>
#include <clk-uclass.h>
#include <dm.h>
#include <errno.h>
#include <asm/io.h>
#include <linux/bitops.h>
#include "clock-snapdragon.h"

/* GPLL0 clock control registers */
#define GPLL0_STATUS_ACTIVE		BIT(30)
#define APCS_GPLL_ENA_VOTE_GPLL0	BIT(0)

static const struct bcr_regs sdc_regs = {
	.cfg_rcgr = SDCC2_CFG_RCGR,
	.cmd_rcgr = SDCC2_CMD_RCGR,
	.M = SDCC2_M,
	.N = SDCC2_N,
	.D = SDCC2_D,
};

static const struct pll_vote_clk gpll0_vote_clk = {
	.status = GPLL0_STATUS,
	.status_bit = GPLL0_STATUS_ACTIVE,
	.ena_vote = APCS_GPLL_ENA_VOTE,
	.vote_bit = APCS_GPLL_ENA_VOTE_GPLL0,
};

static struct vote_clk gcc_blsp2_ahb_clk = {
	.cbcr_reg = BLSP2_AHB_CBCR,
	.ena_vote = APCS_CLOCK_BRANCH_ENA_VOTE,
	.vote_bit = BIT(15),
};

static int clk_init_sdc(struct msm_clk_priv *priv, uint rate)
{
	int div = 3;

	clk_enable_cbc(priv->base + SDCC2_AHB_CBCR);
	clk_rcg_set_rate_mnd(priv->base, &sdc_regs, div, 0, 0,
			     CFG_CLK_SRC_GPLL0);
	clk_enable_gpll0(priv->base, &gpll0_vote_clk);
	clk_enable_cbc(priv->base + SDCC2_APPS_CBCR);

	return rate;
}

static const struct bcr_regs uart2_regs = {
	.cfg_rcgr = BLSP2_UART2_APPS_CFG_RCGR,
	.cmd_rcgr = BLSP2_UART2_APPS_CMD_RCGR,
	.M = BLSP2_UART2_APPS_M,
	.N = BLSP2_UART2_APPS_N,
	.D = BLSP2_UART2_APPS_D,
};

static int clk_init_uart(struct msm_clk_priv *priv)
{
	/* Enable AHB clock */
	clk_enable_vote_clk(priv->base, &gcc_blsp2_ahb_clk);

	/* 7372800 uart block clock @ GPLL0 */
	clk_rcg_set_rate_mnd(priv->base, &uart2_regs, 1, 192, 15625,
			     CFG_CLK_SRC_GPLL0);

	/* Vote for gpll0 clock */
	clk_enable_gpll0(priv->base, &gpll0_vote_clk);

	/* Enable core clk */
	clk_enable_cbc(priv->base + BLSP2_UART2_APPS_CBCR);

	return 0;
}

ulong msm_set_rate(struct clk *clk, ulong rate)
{
	struct msm_clk_priv *priv = dev_get_priv(clk->dev);

	switch (clk->id) {
	case 0: /* SDC1 */
		return clk_init_sdc(priv, rate);
		break;
	case 4: /*UART2*/
		return clk_init_uart(priv);
	default:
		return 0;
	}
}
