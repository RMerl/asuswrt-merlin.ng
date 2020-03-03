/* Copyright Altera Corporation (C) 2014. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Adopted from dwmac-sti.c
 */

#include <linux/mfd/syscon.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_net.h>
#include <linux/phy.h>
#include <linux/regmap.h>
#include <linux/reset.h>
#include <linux/stmmac.h>

#include "stmmac.h"
#include "stmmac_platform.h"

#define SYSMGR_EMACGRP_CTRL_PHYSEL_ENUM_GMII_MII 0x0
#define SYSMGR_EMACGRP_CTRL_PHYSEL_ENUM_RGMII 0x1
#define SYSMGR_EMACGRP_CTRL_PHYSEL_ENUM_RMII 0x2
#define SYSMGR_EMACGRP_CTRL_PHYSEL_WIDTH 2
#define SYSMGR_EMACGRP_CTRL_PHYSEL_MASK 0x00000003

#define EMAC_SPLITTER_CTRL_REG			0x0
#define EMAC_SPLITTER_CTRL_SPEED_MASK		0x3
#define EMAC_SPLITTER_CTRL_SPEED_10		0x2
#define EMAC_SPLITTER_CTRL_SPEED_100		0x3
#define EMAC_SPLITTER_CTRL_SPEED_1000		0x0

struct socfpga_dwmac {
	int	interface;
	u32	reg_offset;
	u32	reg_shift;
	struct	device *dev;
	struct regmap *sys_mgr_base_addr;
	struct reset_control *stmmac_rst;
	void __iomem *splitter_base;
};

static void socfpga_dwmac_fix_mac_speed(void *priv, unsigned int speed)
{
	struct socfpga_dwmac *dwmac = (struct socfpga_dwmac *)priv;
	void __iomem *splitter_base = dwmac->splitter_base;
	u32 val;

	if (!splitter_base)
		return;

	val = readl(splitter_base + EMAC_SPLITTER_CTRL_REG);
	val &= ~EMAC_SPLITTER_CTRL_SPEED_MASK;

	switch (speed) {
	case 1000:
		val |= EMAC_SPLITTER_CTRL_SPEED_1000;
		break;
	case 100:
		val |= EMAC_SPLITTER_CTRL_SPEED_100;
		break;
	case 10:
		val |= EMAC_SPLITTER_CTRL_SPEED_10;
		break;
	default:
		return;
	}

	writel(val, splitter_base + EMAC_SPLITTER_CTRL_REG);
}

static int socfpga_dwmac_parse_data(struct socfpga_dwmac *dwmac, struct device *dev)
{
	struct device_node *np = dev->of_node;
	struct regmap *sys_mgr_base_addr;
	u32 reg_offset, reg_shift;
	int ret;
	struct device_node *np_splitter;
	struct resource res_splitter;

	dwmac->stmmac_rst = devm_reset_control_get(dev,
						  STMMAC_RESOURCE_NAME);
	if (IS_ERR(dwmac->stmmac_rst)) {
		dev_info(dev, "Could not get reset control!\n");
		if (PTR_ERR(dwmac->stmmac_rst) == -EPROBE_DEFER)
			return -EPROBE_DEFER;
		dwmac->stmmac_rst = NULL;
	}

	dwmac->interface = of_get_phy_mode(np);

	sys_mgr_base_addr = syscon_regmap_lookup_by_phandle(np, "altr,sysmgr-syscon");
	if (IS_ERR(sys_mgr_base_addr)) {
		dev_info(dev, "No sysmgr-syscon node found\n");
		return PTR_ERR(sys_mgr_base_addr);
	}

	ret = of_property_read_u32_index(np, "altr,sysmgr-syscon", 1, &reg_offset);
	if (ret) {
		dev_info(dev, "Could not read reg_offset from sysmgr-syscon!\n");
		return -EINVAL;
	}

	ret = of_property_read_u32_index(np, "altr,sysmgr-syscon", 2, &reg_shift);
	if (ret) {
		dev_info(dev, "Could not read reg_shift from sysmgr-syscon!\n");
		return -EINVAL;
	}

	np_splitter = of_parse_phandle(np, "altr,emac-splitter", 0);
	if (np_splitter) {
		if (of_address_to_resource(np_splitter, 0, &res_splitter)) {
			dev_info(dev, "Missing emac splitter address\n");
			return -EINVAL;
		}

		dwmac->splitter_base = devm_ioremap_resource(dev, &res_splitter);
		if (IS_ERR(dwmac->splitter_base)) {
			dev_info(dev, "Failed to mapping emac splitter\n");
			return PTR_ERR(dwmac->splitter_base);
		}
	}

	dwmac->reg_offset = reg_offset;
	dwmac->reg_shift = reg_shift;
	dwmac->sys_mgr_base_addr = sys_mgr_base_addr;
	dwmac->dev = dev;

	return 0;
}

static int socfpga_dwmac_setup(struct socfpga_dwmac *dwmac)
{
	struct regmap *sys_mgr_base_addr = dwmac->sys_mgr_base_addr;
	int phymode = dwmac->interface;
	u32 reg_offset = dwmac->reg_offset;
	u32 reg_shift = dwmac->reg_shift;
	u32 ctrl, val;

	switch (phymode) {
	case PHY_INTERFACE_MODE_RGMII:
	case PHY_INTERFACE_MODE_RGMII_ID:
		val = SYSMGR_EMACGRP_CTRL_PHYSEL_ENUM_RGMII;
		break;
	case PHY_INTERFACE_MODE_MII:
	case PHY_INTERFACE_MODE_GMII:
		val = SYSMGR_EMACGRP_CTRL_PHYSEL_ENUM_GMII_MII;
		break;
	default:
		dev_err(dwmac->dev, "bad phy mode %d\n", phymode);
		return -EINVAL;
	}

	/* Overwrite val to GMII if splitter core is enabled. The phymode here
	 * is the actual phy mode on phy hardware, but phy interface from
	 * EMAC core is GMII.
	 */
	if (dwmac->splitter_base)
		val = SYSMGR_EMACGRP_CTRL_PHYSEL_ENUM_GMII_MII;

	regmap_read(sys_mgr_base_addr, reg_offset, &ctrl);
	ctrl &= ~(SYSMGR_EMACGRP_CTRL_PHYSEL_MASK << reg_shift);
	ctrl |= val << reg_shift;

	regmap_write(sys_mgr_base_addr, reg_offset, ctrl);
	return 0;
}

static void *socfpga_dwmac_probe(struct platform_device *pdev)
{
	struct device		*dev = &pdev->dev;
	int			ret;
	struct socfpga_dwmac	*dwmac;

	dwmac = devm_kzalloc(dev, sizeof(*dwmac), GFP_KERNEL);
	if (!dwmac)
		return ERR_PTR(-ENOMEM);

	ret = socfpga_dwmac_parse_data(dwmac, dev);
	if (ret) {
		dev_err(dev, "Unable to parse OF data\n");
		return ERR_PTR(ret);
	}

	ret = socfpga_dwmac_setup(dwmac);
	if (ret) {
		dev_err(dev, "couldn't setup SoC glue (%d)\n", ret);
		return ERR_PTR(ret);
	}

	return dwmac;
}

static void socfpga_dwmac_exit(struct platform_device *pdev, void *priv)
{
	struct socfpga_dwmac	*dwmac = priv;

	/* On socfpga platform exit, assert and hold reset to the
	 * enet controller - the default state after a hard reset.
	 */
	if (dwmac->stmmac_rst)
		reset_control_assert(dwmac->stmmac_rst);
}

static int socfpga_dwmac_init(struct platform_device *pdev, void *priv)
{
	struct socfpga_dwmac	*dwmac = priv;
	struct net_device *ndev = platform_get_drvdata(pdev);
	struct stmmac_priv *stpriv = NULL;
	int ret = 0;

	if (ndev)
		stpriv = netdev_priv(ndev);

	/* Assert reset to the enet controller before changing the phy mode */
	if (dwmac->stmmac_rst)
		reset_control_assert(dwmac->stmmac_rst);

	/* Setup the phy mode in the system manager registers according to
	 * devicetree configuration
	 */
	ret = socfpga_dwmac_setup(dwmac);

	/* Deassert reset for the phy configuration to be sampled by
	 * the enet controller, and operation to start in requested mode
	 */
	if (dwmac->stmmac_rst)
		reset_control_deassert(dwmac->stmmac_rst);

	/* Before the enet controller is suspended, the phy is suspended.
	 * This causes the phy clock to be gated. The enet controller is
	 * resumed before the phy, so the clock is still gated "off" when
	 * the enet controller is resumed. This code makes sure the phy
	 * is "resumed" before reinitializing the enet controller since
	 * the enet controller depends on an active phy clock to complete
	 * a DMA reset. A DMA reset will "time out" if executed
	 * with no phy clock input on the Synopsys enet controller.
	 * Verified through Synopsys Case #8000711656.
	 *
	 * Note that the phy clock is also gated when the phy is isolated.
	 * Phy "suspend" and "isolate" controls are located in phy basic
	 * control register 0, and can be modified by the phy driver
	 * framework.
	 */
	if (stpriv && stpriv->phydev)
		phy_resume(stpriv->phydev);

	return ret;
}

const struct stmmac_of_data socfpga_gmac_data = {
	.setup = socfpga_dwmac_probe,
	.init = socfpga_dwmac_init,
	.exit = socfpga_dwmac_exit,
	.fix_mac_speed = socfpga_dwmac_fix_mac_speed,
};
