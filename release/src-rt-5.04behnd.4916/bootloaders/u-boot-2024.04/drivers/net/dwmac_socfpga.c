// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018 Marek Vasut <marex@denx.de>
 *
 * Altera SoCFPGA EMAC extras
 */

#include <common.h>
#include <asm/arch/secure_reg_helper.h>
#include <asm/arch/system_manager.h>
#include <asm/io.h>
#include <dm.h>
#include <clk.h>
#include <phy.h>
#include <regmap.h>
#include <reset.h>
#include <syscon.h>
#include "designware.h"
#include <dm/device_compat.h>
#include <linux/err.h>

struct dwmac_socfpga_plat {
	struct dw_eth_pdata	dw_eth_pdata;
	void			*phy_intf;
	u32			reg_shift;
};

static int dwmac_socfpga_of_to_plat(struct udevice *dev)
{
	struct dwmac_socfpga_plat *pdata = dev_get_plat(dev);
	struct regmap *regmap;
	struct ofnode_phandle_args args;
	void *range;
	int ret;

	ret = dev_read_phandle_with_args(dev, "altr,sysmgr-syscon", NULL,
					 2, 0, &args);
	if (ret) {
		dev_err(dev, "Failed to get syscon: %d\n", ret);
		return ret;
	}

	if (args.args_count != 2) {
		dev_err(dev, "Invalid number of syscon args\n");
		return -EINVAL;
	}

	regmap = syscon_node_to_regmap(args.node);
	if (IS_ERR(regmap)) {
		ret = PTR_ERR(regmap);
		dev_err(dev, "Failed to get regmap: %d\n", ret);
		return ret;
	}

	range = regmap_get_range(regmap, 0);
	if (!range) {
		dev_err(dev, "Failed to get regmap range\n");
		return -ENOMEM;
	}

	pdata->phy_intf = range + args.args[0];
	pdata->reg_shift = args.args[1];

	return designware_eth_of_to_plat(dev);
}

static int dwmac_socfpga_do_setphy(struct udevice *dev, u32 modereg)
{
	struct dwmac_socfpga_plat *pdata = dev_get_plat(dev);
	u32 modemask = SYSMGR_EMACGRP_CTRL_PHYSEL_MASK << pdata->reg_shift;

#if !defined(CONFIG_SPL_BUILD) && defined(CONFIG_SPL_ATF)
	u32 index = ((u64)pdata->phy_intf - socfpga_get_sysmgr_addr() -
		     SYSMGR_SOC64_EMAC0) >> 2;

	u32 id = SOCFPGA_SECURE_REG_SYSMGR_SOC64_EMAC0 + index;

	int ret = socfpga_secure_reg_update32(id,
					     modemask,
					     modereg << pdata->reg_shift);
	if (ret) {
		dev_err(dev, "Failed to set PHY register via SMC call\n");
		return ret;
	}
#else
	clrsetbits_le32(pdata->phy_intf, modemask,
			modereg << pdata->reg_shift);
#endif

	return 0;
}

static int dwmac_socfpga_probe(struct udevice *dev)
{
	struct dwmac_socfpga_plat *pdata = dev_get_plat(dev);
	struct eth_pdata *edata = &pdata->dw_eth_pdata.eth_pdata;
	struct reset_ctl_bulk reset_bulk;
	int ret;
	u32 modereg;

	switch (edata->phy_interface) {
	case PHY_INTERFACE_MODE_MII:
	case PHY_INTERFACE_MODE_GMII:
		modereg = SYSMGR_EMACGRP_CTRL_PHYSEL_ENUM_GMII_MII;
		break;
	case PHY_INTERFACE_MODE_RMII:
		modereg = SYSMGR_EMACGRP_CTRL_PHYSEL_ENUM_RMII;
		break;
	case PHY_INTERFACE_MODE_RGMII:
		modereg = SYSMGR_EMACGRP_CTRL_PHYSEL_ENUM_RGMII;
		break;
	default:
		dev_err(dev, "Unsupported PHY mode\n");
		return -EINVAL;
	}

	ret = reset_get_bulk(dev, &reset_bulk);
	if (ret) {
		dev_err(dev, "Failed to get reset: %d\n", ret);
		return ret;
	}

	reset_assert_bulk(&reset_bulk);

	ret = dwmac_socfpga_do_setphy(dev, modereg);
	if (ret)
		return ret;

	reset_release_bulk(&reset_bulk);

	return designware_eth_probe(dev);
}

static const struct udevice_id dwmac_socfpga_ids[] = {
	{ .compatible = "altr,socfpga-stmmac" },
	{ }
};

U_BOOT_DRIVER(dwmac_socfpga) = {
	.name		= "dwmac_socfpga",
	.id		= UCLASS_ETH,
	.of_match	= dwmac_socfpga_ids,
	.of_to_plat = dwmac_socfpga_of_to_plat,
	.probe		= dwmac_socfpga_probe,
	.ops		= &designware_eth_ops,
	.priv_auto	= sizeof(struct dw_eth_dev),
	.plat_auto	= sizeof(struct dwmac_socfpga_plat),
	.flags		= DM_FLAG_ALLOC_PRIV_DMA,
};
