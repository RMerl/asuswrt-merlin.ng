// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2014, NVIDIA CORPORATION.  All rights reserved.
 */

#include <common.h>
#include <errno.h>

#include <asm/arch-tegra/xusb-padctl.h>

struct tegra_xusb_phy * __weak tegra_xusb_phy_get(unsigned int type)
{
	return NULL;
}

int __weak tegra_xusb_phy_prepare(struct tegra_xusb_phy *phy)
{
	return -ENOSYS;
}

int __weak tegra_xusb_phy_enable(struct tegra_xusb_phy *phy)
{
	return -ENOSYS;
}

int __weak tegra_xusb_phy_disable(struct tegra_xusb_phy *phy)
{
	return -ENOSYS;
}

int __weak tegra_xusb_phy_unprepare(struct tegra_xusb_phy *phy)
{
	return -ENOSYS;
}

void __weak tegra_xusb_padctl_init(void)
{
}
