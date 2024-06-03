// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2016, NVIDIA CORPORATION.
 */

#include <common.h>
#include <dm.h>
#include <misc.h>
#include <reset-uclass.h>
#include <asm/arch-tegra/bpmp_abi.h>

static int tegra186_reset_request(struct reset_ctl *reset_ctl)
{
	debug("%s(reset_ctl=%p) (dev=%p, id=%lu)\n", __func__, reset_ctl,
	      reset_ctl->dev, reset_ctl->id);

	return 0;
}

static int tegra186_reset_free(struct reset_ctl *reset_ctl)
{
	debug("%s(reset_ctl=%p) (dev=%p, id=%lu)\n", __func__, reset_ctl,
	      reset_ctl->dev, reset_ctl->id);

	return 0;
}

static int tegra186_reset_common(struct reset_ctl *reset_ctl,
				 enum mrq_reset_commands cmd)
{
	struct mrq_reset_request req;
	int ret;

	req.cmd = cmd;
	req.reset_id = reset_ctl->id;

	ret = misc_call(reset_ctl->dev->parent, MRQ_RESET, &req, sizeof(req),
			NULL, 0);
	if (ret < 0)
		return ret;

	return 0;
}

static int tegra186_reset_assert(struct reset_ctl *reset_ctl)
{
	debug("%s(reset_ctl=%p) (dev=%p, id=%lu)\n", __func__, reset_ctl,
	      reset_ctl->dev, reset_ctl->id);

	return tegra186_reset_common(reset_ctl, CMD_RESET_ASSERT);
}

static int tegra186_reset_deassert(struct reset_ctl *reset_ctl)
{
	debug("%s(reset_ctl=%p) (dev=%p, id=%lu)\n", __func__, reset_ctl,
	      reset_ctl->dev, reset_ctl->id);

	return tegra186_reset_common(reset_ctl, CMD_RESET_DEASSERT);
}

struct reset_ops tegra186_reset_ops = {
	.request = tegra186_reset_request,
	.free = tegra186_reset_free,
	.rst_assert = tegra186_reset_assert,
	.rst_deassert = tegra186_reset_deassert,
};

static int tegra186_reset_probe(struct udevice *dev)
{
	debug("%s(dev=%p)\n", __func__, dev);

	return 0;
}

U_BOOT_DRIVER(tegra186_reset) = {
	.name		= "tegra186_reset",
	.id		= UCLASS_RESET,
	.probe		= tegra186_reset_probe,
	.ops = &tegra186_reset_ops,
};
