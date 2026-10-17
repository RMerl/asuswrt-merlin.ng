// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2019 Texas Instruments Incorporated - https://www.ti.com/
 */

#include <asm/io.h>
#include <clk.h>
#include <common.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <linux/bitops.h>
#include <linux/err.h>

#define UFS_SS_CTRL             0x4
#define UFS_SS_RST_N_PCS        BIT(0)
#define UFS_SS_CLK_26MHZ        BIT(4)

static int ti_j721e_ufs_probe(struct udevice *dev)
{
	void __iomem *base;
	unsigned int clock;
	struct clk clk;
	u32 reg = 0;
	int ret;

	ret = clk_get_by_index(dev, 0, &clk);
	if (ret) {
		dev_err(dev, "failed to get M-PHY clock\n");
		return ret;
	}

	clock = clk_get_rate(&clk);
	if (IS_ERR_VALUE(clock)) {
		dev_err(dev, "failed to get rate\n");
		return ret;
	}

	base = dev_remap_addr_index(dev, 0);

	if (clock == 26000000)
		reg |= UFS_SS_CLK_26MHZ;
	/* Take UFS slave device out of reset */
	reg |= UFS_SS_RST_N_PCS;
	writel(reg, base + UFS_SS_CTRL);

	return 0;
}

static int ti_j721e_ufs_remove(struct udevice *dev)
{
	void __iomem *base = dev_remap_addr_index(dev, 0);
	u32 reg = readl(base + UFS_SS_CTRL);

	reg &= ~UFS_SS_RST_N_PCS;
	writel(reg, base + UFS_SS_CTRL);

	return 0;
}

static const struct udevice_id ti_j721e_ufs_ids[] = {
	{
		.compatible = "ti,j721e-ufs",
	},
	{},
};

U_BOOT_DRIVER(ti_j721e_ufs) = {
	.name			= "ti-j721e-ufs",
	.id			= UCLASS_MISC,
	.of_match		= ti_j721e_ufs_ids,
	.probe			= ti_j721e_ufs_probe,
	.remove			= ti_j721e_ufs_remove,
	.flags			= DM_FLAG_OS_PREPARE,
};
