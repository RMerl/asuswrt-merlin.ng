// SPDX-License-Identifier: BSD-3-Clause
/*
 * Clock drivers for Qualcomm APQ8016, APQ8096
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
#include "clock-snapdragon.h"

/* CBCR register fields */
#define CBCR_BRANCH_ENABLE_BIT  BIT(0)
#define CBCR_BRANCH_OFF_BIT     BIT(31)

extern ulong msm_set_rate(struct clk *clk, ulong rate);

/* Enable clock controlled by CBC soft macro */
void clk_enable_cbc(phys_addr_t cbcr)
{
	setbits_le32(cbcr, CBCR_BRANCH_ENABLE_BIT);

	while (readl(cbcr) & CBCR_BRANCH_OFF_BIT)
		;
}

void clk_enable_gpll0(phys_addr_t base, const struct pll_vote_clk *gpll0)
{
	if (readl(base + gpll0->status) & gpll0->status_bit)
		return; /* clock already enabled */

	setbits_le32(base + gpll0->ena_vote, gpll0->vote_bit);

	while ((readl(base + gpll0->status) & gpll0->status_bit) == 0)
		;
}

#define BRANCH_ON_VAL (0)
#define BRANCH_NOC_FSM_ON_VAL BIT(29)
#define BRANCH_CHECK_MASK GENMASK(31, 28)

void clk_enable_vote_clk(phys_addr_t base, const struct vote_clk *vclk)
{
	u32 val;

	setbits_le32(base + vclk->ena_vote, vclk->vote_bit);
	do {
		val = readl(base + vclk->cbcr_reg);
		val &= BRANCH_CHECK_MASK;
	} while ((val != BRANCH_ON_VAL) && (val != BRANCH_NOC_FSM_ON_VAL));
}

#define APPS_CMD_RGCR_UPDATE BIT(0)

/* Update clock command via CMD_RGCR */
void clk_bcr_update(phys_addr_t apps_cmd_rgcr)
{
	setbits_le32(apps_cmd_rgcr, APPS_CMD_RGCR_UPDATE);

	/* Wait for frequency to be updated. */
	while (readl(apps_cmd_rgcr) & APPS_CMD_RGCR_UPDATE)
		;
}

#define CFG_MODE_DUAL_EDGE (0x2 << 12) /* Counter mode */

#define CFG_MASK 0x3FFF

#define CFG_DIVIDER_MASK 0x1F

/* root set rate for clocks with half integer and MND divider */
void clk_rcg_set_rate_mnd(phys_addr_t base, const struct bcr_regs *regs,
			  int div, int m, int n, int source)
{
	u32 cfg;
	/* M value for MND divider. */
	u32 m_val = m;
	/* NOT(N-M) value for MND divider. */
	u32 n_val = ~((n) - (m)) * !!(n);
	/* NOT 2D value for MND divider. */
	u32 d_val = ~(n);

	/* Program MND values */
	writel(m_val, base + regs->M);
	writel(n_val, base + regs->N);
	writel(d_val, base + regs->D);

	/* setup src select and divider */
	cfg  = readl(base + regs->cfg_rcgr);
	cfg &= ~CFG_MASK;
	cfg |= source & CFG_CLK_SRC_MASK; /* Select clock source */

	/* Set the divider; HW permits fraction dividers (+0.5), but
	   for simplicity, we will support integers only */
	if (div)
		cfg |= (2 * div - 1) & CFG_DIVIDER_MASK;

	if (n_val)
		cfg |= CFG_MODE_DUAL_EDGE;

	writel(cfg, base + regs->cfg_rcgr); /* Write new clock configuration */

	/* Inform h/w to start using the new config. */
	clk_bcr_update(base + regs->cmd_rcgr);
}

static int msm_clk_probe(struct udevice *dev)
{
	struct msm_clk_priv *priv = dev_get_priv(dev);

	priv->base = devfdt_get_addr(dev);
	if (priv->base == FDT_ADDR_T_NONE)
		return -EINVAL;

	return 0;
}

static ulong msm_clk_set_rate(struct clk *clk, ulong rate)
{
	return msm_set_rate(clk, rate);
}

static struct clk_ops msm_clk_ops = {
	.set_rate = msm_clk_set_rate,
};

static const struct udevice_id msm_clk_ids[] = {
	{ .compatible = "qcom,gcc-msm8916" },
	{ .compatible = "qcom,gcc-apq8016" },
	{ .compatible = "qcom,gcc-msm8996" },
	{ .compatible = "qcom,gcc-apq8096" },
	{ }
};

U_BOOT_DRIVER(clk_msm) = {
	.name		= "clk_msm",
	.id		= UCLASS_CLK,
	.of_match	= msm_clk_ids,
	.ops		= &msm_clk_ops,
	.priv_auto_alloc_size = sizeof(struct msm_clk_priv),
	.probe		= msm_clk_probe,
};
