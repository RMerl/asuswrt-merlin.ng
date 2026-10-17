// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright (C) 2019, STMicroelectronics - All Rights Reserved
 */

#include <common.h>
#include <display.h>
#include <dm.h>
#include <dsi_host.h>

/**
 * struct sandbox_dsi_host_priv - private data for driver
 * @device: DSI peripheral device
 * @timing: Display timings
 * @max_data_lanes: maximum number of data lines
 * @phy_ops: set of function pointers for performing physical operations
 */
struct sandbox_dsi_host_priv {
	struct mipi_dsi_device *device;
	struct display_timing *timings;
	unsigned int max_data_lanes;
	const struct mipi_dsi_phy_ops *phy_ops;
};

static int sandbox_dsi_host_init(struct udevice *dev,
				 struct mipi_dsi_device *device,
				 struct display_timing *timings,
				 unsigned int max_data_lanes,
				 const struct mipi_dsi_phy_ops *phy_ops)
{
	struct sandbox_dsi_host_priv *priv = dev_get_priv(dev);

	if (!device)
		return -1;

	if (!timings)
		return -2;

	if (max_data_lanes == 0)
		return -3;

	if (!phy_ops)
		return -4;

	if (!phy_ops->init || !phy_ops->get_lane_mbps ||
	    !phy_ops->post_set_mode)
		return -5;

	priv->max_data_lanes = max_data_lanes;
	priv->phy_ops = phy_ops;
	priv->timings = timings;
	priv->device = device;

	return 0;
}

static int sandbox_dsi_host_enable(struct udevice *dev)
{
	struct sandbox_dsi_host_priv *priv = dev_get_priv(dev);
	unsigned int lane_mbps;
	int ret;

	priv->phy_ops->init(priv->device);
	ret = priv->phy_ops->get_lane_mbps(priv->device, priv->timings, 2,
					   MIPI_DSI_FMT_RGB888, &lane_mbps);
	if (ret)
		return -1;

	priv->phy_ops->post_set_mode(priv->device, MIPI_DSI_MODE_VIDEO);

	return 0;
}

struct dsi_host_ops sandbox_dsi_host_ops = {
	.init = sandbox_dsi_host_init,
	.enable = sandbox_dsi_host_enable,
};

static const struct udevice_id sandbox_dsi_host_ids[] = {
	{ .compatible = "sandbox,dsi-host"},
	{ }
};

U_BOOT_DRIVER(sandbox_dsi_host) = {
	.name		      = "sandbox-dsi-host",
	.id		      = UCLASS_DSI_HOST,
	.of_match	      = sandbox_dsi_host_ids,
	.ops		      = &sandbox_dsi_host_ops,
	.priv_auto	= sizeof(struct sandbox_dsi_host_priv),
};
