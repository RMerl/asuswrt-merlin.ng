// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2019 STMicroelectronics - All Rights Reserved
 * Author(s): Yannick Fertre <yannick.fertre@st.com> for STMicroelectronics.
 *
 */

#define LOG_CATEGORY UCLASS_DSI_HOST

#include <common.h>
#include <dm.h>
#include <dsi_host.h>

int dsi_host_init(struct udevice *dev,
		  struct mipi_dsi_device *device,
		  struct display_timing *timings,
		  unsigned int max_data_lanes,
		  const struct mipi_dsi_phy_ops *phy_ops)
{
	struct dsi_host_ops *ops = dsi_host_get_ops(dev);

	if (!ops->init)
		return -ENOSYS;

	return ops->init(dev, device, timings, max_data_lanes, phy_ops);
}

int dsi_host_enable(struct udevice *dev)
{
	struct dsi_host_ops *ops = dsi_host_get_ops(dev);

	if (!ops->enable)
		return -ENOSYS;

	return ops->enable(dev);
}

UCLASS_DRIVER(dsi_host) = {
	.id		= UCLASS_DSI_HOST,
	.name		= "dsi_host",
};
