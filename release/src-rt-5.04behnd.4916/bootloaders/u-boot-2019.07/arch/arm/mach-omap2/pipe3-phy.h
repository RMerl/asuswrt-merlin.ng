/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * TI PIPE3 PHY
 *
 * (C) Copyright 2013
 * Texas Instruments, <www.ti.com>
 */

#ifndef __OMAP_PIPE3_PHY_H
#define __OMAP_PIPE3_PHY_H

struct pipe3_dpll_params {
	u16     m;
	u8      n;
	u8      freq:3;
	u8      sd;
	u32     mf;
};

struct pipe3_dpll_map {
	unsigned long rate;
	struct pipe3_dpll_params params;
};

struct omap_pipe3 {
	void __iomem            *pll_ctrl_base;
	void __iomem		*power_reg;
	struct pipe3_dpll_map   *dpll_map;
};


int phy_pipe3_power_on(struct omap_pipe3 *phy);
int phy_pipe3_power_off(struct omap_pipe3 *pipe3);

#endif /* __OMAP_PIPE3_PHY_H */
