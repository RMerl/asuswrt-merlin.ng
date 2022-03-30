// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2016, NVIDIA CORPORATION.
 */

#include <common.h>
#include <clk-uclass.h>
#include <dm.h>
#include <misc.h>
#include <asm/arch-tegra/bpmp_abi.h>

static ulong tegra186_clk_get_rate(struct clk *clk)
{
	struct mrq_clk_request req;
	struct mrq_clk_response resp;
	int ret;

	debug("%s(clk=%p) (dev=%p, id=%lu)\n", __func__, clk, clk->dev,
	      clk->id);

	req.cmd_and_id = (CMD_CLK_GET_RATE << 24) | clk->id;

	ret = misc_call(clk->dev->parent, MRQ_CLK, &req, sizeof(req), &resp,
			sizeof(resp));
	if (ret < 0)
		return ret;

	return resp.clk_get_rate.rate;
}

static ulong tegra186_clk_set_rate(struct clk *clk, ulong rate)
{
	struct mrq_clk_request req;
	struct mrq_clk_response resp;
	int ret;

	debug("%s(clk=%p, rate=%lu) (dev=%p, id=%lu)\n", __func__, clk, rate,
	      clk->dev, clk->id);

	req.cmd_and_id = (CMD_CLK_SET_RATE << 24) | clk->id;
	req.clk_set_rate.rate = rate;

	ret = misc_call(clk->dev->parent, MRQ_CLK, &req, sizeof(req), &resp,
			sizeof(resp));
	if (ret < 0)
		return ret;

	return resp.clk_set_rate.rate;
}

static int tegra186_clk_en_dis(struct clk *clk,
			       enum mrq_reset_commands cmd)
{
	struct mrq_clk_request req;
	struct mrq_clk_response resp;
	int ret;

	req.cmd_and_id = (cmd << 24) | clk->id;

	ret = misc_call(clk->dev->parent, MRQ_CLK, &req, sizeof(req), &resp,
			sizeof(resp));
	if (ret < 0)
		return ret;

	return 0;
}

static int tegra186_clk_enable(struct clk *clk)
{
	debug("%s(clk=%p) (dev=%p, id=%lu)\n", __func__, clk, clk->dev,
	      clk->id);

	return tegra186_clk_en_dis(clk, CMD_CLK_ENABLE);
}

static int tegra186_clk_disable(struct clk *clk)
{
	debug("%s(clk=%p) (dev=%p, id=%lu)\n", __func__, clk, clk->dev,
	      clk->id);

	return tegra186_clk_en_dis(clk, CMD_CLK_DISABLE);
}

static struct clk_ops tegra186_clk_ops = {
	.get_rate = tegra186_clk_get_rate,
	.set_rate = tegra186_clk_set_rate,
	.enable = tegra186_clk_enable,
	.disable = tegra186_clk_disable,
};

static int tegra186_clk_probe(struct udevice *dev)
{
	debug("%s(dev=%p)\n", __func__, dev);

	return 0;
}

U_BOOT_DRIVER(tegra186_clk) = {
	.name		= "tegra186_clk",
	.id		= UCLASS_CLK,
	.probe		= tegra186_clk_probe,
	.ops = &tegra186_clk_ops,
};
