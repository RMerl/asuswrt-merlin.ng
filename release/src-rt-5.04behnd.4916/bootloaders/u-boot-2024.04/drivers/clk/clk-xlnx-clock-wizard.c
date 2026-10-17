// SPDX-License-Identifier: GPL-2.0
/*
 * Xilinx 'Clocking Wizard' driver
 *
 * Copyright (c) 2021 Macronix Inc.
 *
 * Author: Zhengxun Li <zhengxunli@mxic.com.tw>
 */

#include <common.h>
#include <clk-uclass.h>
#include <dm.h>
#include <div64.h>
#include <dm/device_compat.h>
#include <linux/iopoll.h>

#include <linux/bitfield.h>

#define SRR			0x0

#define SR			0x4
#define SR_LOCKED		BIT(0)

#define CCR(x)			(0x200 + ((x) * 4))

#define FBOUT_CFG		CCR(0)
#define FBOUT_DIV(x)		(x)
#define FBOUT_DIV_MASK		GENMASK(7, 0)
#define FBOUT_GET_DIV(x)	FIELD_GET(FBOUT_DIV_MASK, x)
#define FBOUT_MUL(x)		((x) << 8)
#define FBOUT_MUL_MASK		GENMASK(15, 8)
#define FBOUT_GET_MUL(x)	FIELD_GET(FBOUT_MUL_MASK, x)
#define FBOUT_FRAC(x)		((x) << 16)
#define FBOUT_FRAC_MASK		GENMASK(25, 16)
#define FBOUT_GET_FRAC(x)	FIELD_GET(FBOUT_FRAC_MASK, x)
#define FBOUT_FRAC_EN		BIT(26)

#define FBOUT_PHASE		CCR(1)

#define OUT_CFG(x)		CCR(2 + ((x) * 3))
#define OUT_DIV(x)		(x)
#define OUT_DIV_MASK		GENMASK(7, 0)
#define OUT_GET_DIV(x)		FIELD_GET(OUT_DIV_MASK, x)
#define OUT_FRAC(x)		((x) << 8)
#define OUT_GET_MASK		GENMASK(17, 8)
#define OUT_GET_FRAC(x)		FIELD_GET(OUT_GET_MASK, x)
#define OUT_FRAC_EN		BIT(18)

#define OUT_PHASE(x)		CCR(3 + ((x) * 3))
#define OUT_DUTY(x)		CCR(4 + ((x) * 3))

#define CTRL			CCR(23)
#define CTRL_SEN		BIT(2)
#define CTRL_SADDR		BIT(1)
#define CTRL_LOAD		BIT(0)

/**
 * struct clkwzrd - Clock wizard private data structure
 *
 * @base:		memory base
 * @vco_clk:		voltage-controlled oscillator frequency
 *
 */
struct clkwzd {
	void *base;
	u64 vco_clk;
};

struct clkwzd_plat {
	fdt_addr_t addr;
};

static int clk_wzrd_enable(struct clk *clk)
{
	struct clkwzd *priv = dev_get_priv(clk->dev);
	int ret;
	u32 val;

	ret = readl_poll_sleep_timeout(priv->base + SR, val, val & SR_LOCKED,
				       1, 100);
	if (!ret) {
		writel(CTRL_SEN | CTRL_SADDR | CTRL_LOAD, priv->base + CTRL);
		writel(CTRL_SADDR, priv->base + CTRL);
		ret = readl_poll_sleep_timeout(priv->base + SR, val,
					       val & SR_LOCKED, 1, 100);
	}

	return ret;
}

static unsigned long clk_wzrd_set_rate(struct clk *clk, ulong rate)
{
	struct clkwzd *priv = dev_get_priv(clk->dev);
	u64 div;
	u32 cfg;

	/* Get output clock divide value */
	div = DIV_ROUND_DOWN_ULL(priv->vco_clk * 1000, rate);
	if (div < 1000 || div > 255999)
		return -EINVAL;

	cfg = OUT_DIV((u32)div / 1000);

	writel(cfg, priv->base + OUT_CFG(clk->id));

	return 0;
}

static struct clk_ops clk_wzrd_ops = {
	.enable = clk_wzrd_enable,
	.set_rate = clk_wzrd_set_rate,
};

static int clk_wzrd_probe(struct udevice *dev)
{
	struct clkwzd_plat *plat = dev_get_plat(dev);
	struct clkwzd *priv = dev_get_priv(dev);
	struct clk clk_in1;
	u64 clock, vco_clk;
	u32 cfg;
	int ret;

	priv->base = (void *)plat->addr;

	ret = clk_get_by_name(dev, "clk_in1", &clk_in1);
	if (ret < 0) {
		dev_err(dev, "failed to get clock\n");
		return ret;
	}

	clock = clk_get_rate(&clk_in1);
	if (IS_ERR_VALUE(clock)) {
		dev_err(dev, "failed to get rate\n");
		return clock;
	}

	ret = clk_enable(&clk_in1);
	if (ret) {
		dev_err(dev, "failed to enable clock\n");
		return ret;
	}

	/* Read clock configuration registers */
	cfg = readl(priv->base + FBOUT_CFG);

	/* Recalculate VCO rate */
	if (cfg & FBOUT_FRAC_EN)
		vco_clk = DIV_ROUND_DOWN_ULL(clock *
					     ((FBOUT_GET_MUL(cfg) * 1000) +
					      FBOUT_GET_FRAC(cfg)),
					     1000);
	else
		vco_clk = clock * FBOUT_GET_MUL(cfg);

	priv->vco_clk = DIV_ROUND_DOWN_ULL(vco_clk, FBOUT_GET_DIV(cfg));

	return 0;
}

static int clk_wzrd_of_to_plat(struct udevice *dev)
{
	struct clkwzd_plat *plat = dev_get_plat(dev);

	plat->addr = dev_read_addr(dev);
	if (plat->addr == FDT_ADDR_T_NONE)
		return -EINVAL;

	return 0;
}

static const struct udevice_id clk_wzrd_ids[] = {
	{ .compatible = "xlnx,clocking-wizard" },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(clk_wzrd) = {
	.name = "zynq-clk-wizard",
	.id = UCLASS_CLK,
	.of_match = clk_wzrd_ids,
	.ops = &clk_wzrd_ops,
	.probe = clk_wzrd_probe,
	.of_to_plat = clk_wzrd_of_to_plat,
	.priv_auto = sizeof(struct clkwzd),
	.plat_auto = sizeof(struct clkwzd_plat),
};
