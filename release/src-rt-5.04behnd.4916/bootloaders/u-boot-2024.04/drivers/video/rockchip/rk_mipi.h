/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2017, Fuzhou Rockchip Electronics Co., Ltd
 * Author: Eric Gao <eric.gao@rock-chips.com>
 */

#ifndef __RK_MIPI_H
#define __RK_MIPI_H

struct rk_mipi_priv {
	uintptr_t regs;
	void *grf;
	struct udevice *panel;
	struct mipi_dsi *dsi;
	u32 ref_clk;
	u32 sys_clk;
	u32 pix_clk;
	u32 phy_clk;
	u32 txbyte_clk;
	u32 txesc_clk;
};

int rk_mipi_read_timing(struct udevice *dev,
			       struct display_timing *timing);

int rk_mipi_dsi_enable(struct udevice *dev,
			      const struct display_timing *timing);

int rk_mipi_phy_enable(struct udevice *dev);


#endif
