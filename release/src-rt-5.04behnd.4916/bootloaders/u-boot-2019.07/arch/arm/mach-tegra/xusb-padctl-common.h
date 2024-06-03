/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2014-2015, NVIDIA CORPORATION.  All rights reserved.
 */

#ifndef _TEGRA_XUSB_PADCTL_COMMON_H_
#define _TEGRA_XUSB_PADCTL_COMMON_H_

#include <common.h>
#include <fdtdec.h>
#include <dm/ofnode.h>

#include <asm/io.h>
#include <asm/arch-tegra/xusb-padctl.h>
#include <linux/ioport.h>

struct tegra_xusb_padctl_lane {
	const char *name;

	unsigned int offset;
	unsigned int shift;
	unsigned int mask;
	unsigned int iddq;

	const unsigned int *funcs;
	unsigned int num_funcs;
};

struct tegra_xusb_phy_ops {
	int (*prepare)(struct tegra_xusb_phy *phy);
	int (*enable)(struct tegra_xusb_phy *phy);
	int (*disable)(struct tegra_xusb_phy *phy);
	int (*unprepare)(struct tegra_xusb_phy *phy);
};

struct tegra_xusb_phy {
	unsigned int type;
	const struct tegra_xusb_phy_ops *ops;
	struct tegra_xusb_padctl *padctl;
};

struct tegra_xusb_padctl_pin {
	const struct tegra_xusb_padctl_lane *lane;

	unsigned int func;
	int iddq;
};

#define MAX_GROUPS 5
#define MAX_PINS 7

struct tegra_xusb_padctl_group {
	const char *name;

	const char *pins[MAX_PINS];
	unsigned int num_pins;

	const char *func;
	int iddq;
};

struct tegra_xusb_padctl_soc {
	const struct tegra_xusb_padctl_lane *lanes;
	unsigned int num_lanes;
	const char *const *functions;
	unsigned int num_functions;
	struct tegra_xusb_phy *phys;
	unsigned int num_phys;
};

struct tegra_xusb_padctl_config {
	const char *name;

	struct tegra_xusb_padctl_group groups[MAX_GROUPS];
	unsigned int num_groups;
};

struct tegra_xusb_padctl {
	const struct tegra_xusb_padctl_soc *socdata;
	struct tegra_xusb_padctl_config config;
	struct resource regs;
	unsigned int enable;

};
extern struct tegra_xusb_padctl padctl;

static inline u32 padctl_readl(struct tegra_xusb_padctl *padctl,
			       unsigned long offset)
{
	return readl(padctl->regs.start + offset);
}

static inline void padctl_writel(struct tegra_xusb_padctl *padctl,
				 u32 value, unsigned long offset)
{
	writel(value, padctl->regs.start + offset);
}

int tegra_xusb_process_nodes(ofnode nodes[], unsigned int count,
			     const struct tegra_xusb_padctl_soc *socdata);

#endif
