// SPDX-License-Identifier: BSD-3-Clause
/*
 * Clock drivers for Qualcomm APQ8016
 *
 * (C) Copyright 2015 Mateusz Kulikowski <mateusz.kulikowski@gmail.com>
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
#define GPLL0_STATUS			(0x2101C)
#define APCS_GPLL_ENA_VOTE		(0x45000)
#define APCS_CLOCK_BRANCH_ENA_VOTE (0x45004)

#define SDCC_BCR(n)			((n * 0x1000) + 0x41000)
#define SDCC_CMD_RCGR(n)		((n * 0x1000) + 0x41004)
#define SDCC_CFG_RCGR(n)		((n * 0x1000) + 0x41008)
#define SDCC_M(n)			((n * 0x1000) + 0x4100C)
#define SDCC_N(n)			((n * 0x1000) + 0x41010)
#define SDCC_D(n)			((n * 0x1000) + 0x41014)
#define SDCC_APPS_CBCR(n)		((n * 0x1000) + 0x41018)
#define SDCC_AHB_CBCR(n)		((n * 0x1000) + 0x4101C)

/* BLSP1 AHB clock (root clock for BLSP) */
#define BLSP1_AHB_CBCR			0x1008

/* Uart clock control registers */
#define BLSP1_UART2_BCR			(0x3028)
#define BLSP1_UART2_APPS_CBCR		(0x302C)
#define BLSP1_UART2_APPS_CMD_RCGR	(0x3034)
#define BLSP1_UART2_APPS_CFG_RCGR	(0x3038)
#define BLSP1_UART2_APPS_M		(0x303C)
#define BLSP1_UART2_APPS_N		(0x3040)
#define BLSP1_UART2_APPS_D		(0x3044)

/* GPLL0 clock control registers */
#define GPLL0_STATUS_ACTIVE BIT(17)

static const struct bcr_regs sdc_regs[] = {
	{
	.cfg_rcgr = SDCC_CFG_RCGR(1),
	.cmd_rcgr = SDCC_CMD_RCGR(1),
	.M = SDCC_M(1),
	.N = SDCC_N(1),
	.D = SDCC_D(1),
	},
	{
	.cfg_rcgr = SDCC_CFG_RCGR(2),
	.cmd_rcgr = SDCC_CMD_RCGR(2),
	.M = SDCC_M(2),
	.N = SDCC_N(2),
	.D = SDCC_D(2),
	}
};

static struct pll_vote_clk gpll0_vote_clk = {
	.status = GPLL0_STATUS,
	.status_bit = GPLL0_STATUS_ACTIVE,
	.ena_vote = APCS_GPLL_ENA_VOTE,
	.vote_bit = BIT(0),
};

static struct vote_clk gcc_blsp1_ahb_clk = {
	.cbcr_reg = BLSP1_AHB_CBCR,
	.ena_vote = APCS_CLOCK_BRANCH_ENA_VOTE,
	.vote_bit = BIT(10),
};

/* SDHCI */
static int clk_init_sdc(struct msm_clk_priv *priv, int slot, uint rate)
{
	int div = 15; /* 100MHz default */

	if (rate == 200000000)
		div = 4;

	clk_enable_cbc(priv->base + SDCC_AHB_CBCR(slot));
	/* 800Mhz/div, gpll0 */
	clk_rcg_set_rate_mnd(priv->base, &sdc_regs[slot], div, 0, 0,
			     CFG_CLK_SRC_GPLL0, 8);
	clk_enable_gpll0(priv->base, &gpll0_vote_clk);
	clk_enable_cbc(priv->base + SDCC_APPS_CBCR(slot));

	return rate;
}

static const struct bcr_regs uart2_regs = {
	.cfg_rcgr = BLSP1_UART2_APPS_CFG_RCGR,
	.cmd_rcgr = BLSP1_UART2_APPS_CMD_RCGR,
	.M = BLSP1_UART2_APPS_M,
	.N = BLSP1_UART2_APPS_N,
	.D = BLSP1_UART2_APPS_D,
};

/* UART: 115200 */
static int clk_init_uart(struct msm_clk_priv *priv)
{
	/* Enable AHB clock */
	clk_enable_vote_clk(priv->base, &gcc_blsp1_ahb_clk);

	/* 7372800 uart block clock @ GPLL0 */
	clk_rcg_set_rate_mnd(priv->base, &uart2_regs, 1, 144, 15625,
			     CFG_CLK_SRC_GPLL0, 16);

	/* Vote for gpll0 clock */
	clk_enable_gpll0(priv->base, &gpll0_vote_clk);

	/* Enable core clk */
	clk_enable_cbc(priv->base + BLSP1_UART2_APPS_CBCR);

	return 0;
}

static ulong apq8016_clk_set_rate(struct clk *clk, ulong rate)
{
	struct msm_clk_priv *priv = dev_get_priv(clk->dev);

	switch (clk->id) {
	case 0: /* SDC1 */
		return clk_init_sdc(priv, 0, rate);
		break;
	case 1: /* SDC2 */
		return clk_init_sdc(priv, 1, rate);
		break;
	case 4: /* UART2 */
		return clk_init_uart(priv);
		break;
	default:
		return 0;
	}
}

static struct msm_clk_data apq8016_clk_data = {
	.set_rate = apq8016_clk_set_rate,
};

static const struct udevice_id gcc_apq8016_of_match[] = {
	{
		.compatible = "qcom,gcc-apq8016",
		.data = (ulong)&apq8016_clk_data,
	},
	{ }
};

U_BOOT_DRIVER(gcc_apq8016) = {
	.name		= "gcc_apq8016",
	.id		= UCLASS_NOP,
	.of_match	= gcc_apq8016_of_match,
	.bind		= qcom_cc_bind,
	.flags		= DM_FLAG_PRE_RELOC,
};
