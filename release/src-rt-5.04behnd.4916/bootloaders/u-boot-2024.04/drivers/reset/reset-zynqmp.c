// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2021 Xilinx, Inc. - Michal Simek
 */

#define LOG_CATEGORY UCLASS_RESET

#include <common.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <reset-uclass.h>
#include <zynqmp_firmware.h>

#define ZYNQMP_NR_RESETS	(ZYNQMP_PM_RESET_END - ZYNQMP_PM_RESET_START)
#define ZYNQMP_RESET_ID		ZYNQMP_PM_RESET_START

struct zynqmp_reset_priv {
	u32 reset_id;
	u32 nr_reset;
};

static int zynqmp_pm_reset_assert(const u32 reset,
				  const enum zynqmp_pm_reset_action assert_flag)
{
	return xilinx_pm_request(PM_RESET_ASSERT, reset, assert_flag, 0, 0,
				 NULL);
}

static int zynqmp_reset_assert(struct reset_ctl *rst)
{
	struct zynqmp_reset_priv *priv = dev_get_priv(rst->dev);

	dev_dbg(rst->dev, "%s(rst=%p) (id=%lu)\n", __func__, rst, rst->id);

	return zynqmp_pm_reset_assert(priv->reset_id + rst->id,
				      PM_RESET_ACTION_ASSERT);
}

static int zynqmp_reset_deassert(struct reset_ctl *rst)
{
	struct zynqmp_reset_priv *priv = dev_get_priv(rst->dev);

	dev_dbg(rst->dev, "%s(rst=%p) (id=%lu)\n", __func__, rst, rst->id);

	return zynqmp_pm_reset_assert(priv->reset_id + rst->id,
				      PM_RESET_ACTION_RELEASE);
}

static int zynqmp_reset_request(struct reset_ctl *rst)
{
	struct zynqmp_reset_priv *priv = dev_get_priv(rst->dev);

	dev_dbg(rst->dev, "%s(rst=%p) (id=%lu) (nr_reset=%d)\n", __func__,
		rst, rst->id, priv->nr_reset);

	if (priv->nr_reset && rst->id > priv->nr_reset)
		return -EINVAL;

	return 0;
}

static int zynqmp_reset_probe(struct udevice *dev)
{
	struct zynqmp_reset_priv *priv = dev_get_priv(dev);

	if (device_is_compatible(dev, "xlnx,zynqmp-reset")) {
		priv->reset_id = ZYNQMP_RESET_ID;
		priv->nr_reset = ZYNQMP_NR_RESETS;
	}

	return 0;
}

const struct reset_ops zynqmp_reset_ops = {
	.request = zynqmp_reset_request,
	.rst_assert = zynqmp_reset_assert,
	.rst_deassert = zynqmp_reset_deassert,
};

static const struct udevice_id zynqmp_reset_ids[] = {
	{ .compatible = "xlnx,zynqmp-reset" },
	{ .compatible = "xlnx,versal-reset" },
	{ .compatible = "xlnx,versal-net-reset" },
	{ }
};

U_BOOT_DRIVER(zynqmp_reset) = {
	.name		= "zynqmp_reset",
	.id		= UCLASS_RESET,
	.of_match	= zynqmp_reset_ids,
	.ops		= &zynqmp_reset_ops,
	.probe		= zynqmp_reset_probe,
	.priv_auto	= sizeof(struct zynqmp_reset_priv),
};
