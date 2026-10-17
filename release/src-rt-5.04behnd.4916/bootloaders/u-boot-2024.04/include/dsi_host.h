/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2019 STMicroelectronics - All Rights Reserved
 * Author(s): Yannick Fertre <yannick.fertre@st.com> for STMicroelectronics.
 *
 */

#ifndef _DSI_HOST_H
#define _DSI_HOST_H

#include <mipi_dsi.h>

struct dsi_host_ops {
	/**
	 * init() - initialized the dsi_host
	 *
	 * @dev: dsi host device
	 * @device: DSI peripheral device
	 * @timing: Display timings
	 * @max_data_lanes: maximum number of data lines
	 * @phy_ops: set of function pointers for performing physical operations
	 * @return 0 if OK, -ve on error
	 */
	int (*init)(struct udevice *dev,
		    struct mipi_dsi_device *device,
		    struct display_timing *timings,
		    unsigned int max_data_lanes,
		    const struct mipi_dsi_phy_ops *phy_ops);

	/**
	 * enable() - Enable the dsi_host
	 *
	 * @dev: dsi host device
	 * @return 0 if OK, -ve on error
	 */
	int (*enable)(struct udevice *dev);

	/**
	 * disable() - Disable the dsi_host
	 *
	 * @dev: dsi host device
	 * @return 0 if OK, -ve on error
	 */
	int (*disable)(struct udevice *dev);
};

#define dsi_host_get_ops(dev)	((struct dsi_host_ops *)(dev)->driver->ops)

/**
 * dsi_host_init
 *
 * @dev: dsi host device
 * @device: DSI peripheral device
 * @timing: Display timings
 * @max_data_lanes: maximum number of data lines
 * @phy_ops: set of function pointers for performing physical operations
 * Return: 0 if OK, -ve on error
 */
int dsi_host_init(struct udevice *dev,
		  struct mipi_dsi_device *device,
		  struct display_timing *timings,
		  unsigned int max_data_lanes,
		  const struct mipi_dsi_phy_ops *phy_ops);

/**
 * dsi_host_enable
 *
 * @dev:	dsi host device
 * Return: 0 if OK, -ve on error
 */
int dsi_host_enable(struct udevice *dev);

#endif
