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

#include "clock-qcom.h"

/* Clocks: (from CLK_CTL_BASE)  */
#define GPLL0_STATUS			(0x0000)
#define APCS_GPLL_ENA_VOTE		(0x52000)
#define APCS_CLOCK_BRANCH_ENA_VOTE	(0x52004)

#define SDCC2_BCR			(0x14000) /* block reset */
#define SDCC2_APPS_CBCR			(0x14004) /* branch control */
#define SDCC2_AHB_CBCR			(0x14008)
#define SDCC2_CMD_RCGR			(0x14010)
#define SDCC2_CFG_RCGR			(0x14014)
#define SDCC2_M				(0x14018)
#define SDCC2_N				(0x1401C)
#define SDCC2_D				(0x14020)

#define BLSP2_AHB_CBCR			(0x25004)
#define BLSP2_UART2_APPS_CBCR		(0x29004)
#define BLSP2_UART2_APPS_CMD_RCGR	(0x2900C)
#define BLSP2_UART2_APPS_CFG_RCGR	(0x29010)
#define BLSP2_UART2_APPS_M		(0x29014)
#define BLSP2_UART2_APPS_N		(0x29018)
#define BLSP2_UART2_APPS_D		(0x2901C)

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
	int div = 5;

	clk_enable_cbc(priv->base + SDCC2_AHB_CBCR);
	clk_rcg_set_rate_mnd(priv->base, &sdc_regs, div, 0, 0,
			     CFG_CLK_SRC_GPLL0, 8);
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
			     CFG_CLK_SRC_GPLL0, 16);

	/* Vote for gpll0 clock */
	clk_enable_gpll0(priv->base, &gpll0_vote_clk);

	/* Enable core clk */
	clk_enable_cbc(priv->base + BLSP2_UART2_APPS_CBCR);

	return 0;
}

static ulong apq8096_clk_set_rate(struct clk *clk, ulong rate)
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

static struct msm_clk_data apq8096_clk_data = {
	.set_rate = apq8096_clk_set_rate,
};

static const struct udevice_id gcc_apq8096_of_match[] = {
	{
		.compatible = "qcom,gcc-apq8096",
		.data = (ulong)&apq8096_clk_data,
	},
	{ }
};

U_BOOT_DRIVER(gcc_apq8096) = {
	.name		= "gcc_apq8096",
	.id		= UCLASS_NOP,
	.of_match	= gcc_apq8096_of_match,
	.bind		= qcom_cc_bind,
	.flags		= DM_FLAG_PRE_RELOC,
};
