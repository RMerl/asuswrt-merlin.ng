// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2021 BayLibre, SAS
 */

#include <common.h>
#include <asm/io.h>
#include <dm.h>
#include <phy.h>
#include "designware.h"
#include <dm/device_compat.h>
#include <linux/err.h>

#define ETH_REG_0		0x0
#define ETH_REG_1		0x4
#define ETH_REG_2		0x18
#define ETH_REG_3		0x1c

#define GX_ETH_REG_0_PHY_INTF		BIT(0)
#define GX_ETH_REG_0_TX_PHASE(x)	(((x) & 3) << 5)
#define GX_ETH_REG_0_TX_RATIO(x)	(((x) & 7) << 7)
#define GX_ETH_REG_0_PHY_CLK_EN	BIT(10)
#define GX_ETH_REG_0_INVERT_RMII_CLK	BIT(11)
#define GX_ETH_REG_0_CLK_EN		BIT(12)

#define AXG_ETH_REG_0_PHY_INTF_RGMII	BIT(0)
#define AXG_ETH_REG_0_PHY_INTF_RMII	BIT(2)
#define AXG_ETH_REG_0_TX_PHASE(x)	(((x) & 3) << 5)
#define AXG_ETH_REG_0_TX_RATIO(x)	(((x) & 7) << 7)
#define AXG_ETH_REG_0_PHY_CLK_EN	BIT(10)
#define AXG_ETH_REG_0_INVERT_RMII_CLK	BIT(11)
#define AXG_ETH_REG_0_CLK_EN		BIT(12)

struct dwmac_meson8b_plat {
	struct dw_eth_pdata dw_eth_pdata;
	int (*dwmac_setup)(struct udevice *dev, struct eth_pdata *edata);
	void *regs;
};

static int dwmac_meson8b_of_to_plat(struct udevice *dev)
{
	struct dwmac_meson8b_plat *pdata = dev_get_plat(dev);

	pdata->regs = dev_read_addr_index_ptr(dev, 1);
	if (!pdata->regs)
		return -EINVAL;

	pdata->dwmac_setup = (void *)dev_get_driver_data(dev);
	if (!pdata->dwmac_setup)
		return -EINVAL;

	return designware_eth_of_to_plat(dev);
}

static int dwmac_setup_axg(struct udevice *dev, struct eth_pdata *edata)
{
	struct dwmac_meson8b_plat *plat = dev_get_plat(dev);

	switch (edata->phy_interface) {
	case PHY_INTERFACE_MODE_RGMII:
	case PHY_INTERFACE_MODE_RGMII_ID:
		/* Set RGMII mode */
		setbits_le32(plat->regs + ETH_REG_0, AXG_ETH_REG_0_PHY_INTF_RGMII |
						     AXG_ETH_REG_0_TX_PHASE(1) |
						     AXG_ETH_REG_0_TX_RATIO(4) |
						     AXG_ETH_REG_0_PHY_CLK_EN |
						     AXG_ETH_REG_0_CLK_EN);
		break;

	case PHY_INTERFACE_MODE_RGMII_RXID:
	case PHY_INTERFACE_MODE_RGMII_TXID:
		/* TOFIX: handle amlogic,tx-delay-ns & rx-internal-delay-ps from DT */
		setbits_le32(plat->regs + ETH_REG_0, AXG_ETH_REG_0_PHY_INTF_RGMII |
						     AXG_ETH_REG_0_TX_RATIO(4) |
						     AXG_ETH_REG_0_PHY_CLK_EN |
						     AXG_ETH_REG_0_CLK_EN);
		break;

	case PHY_INTERFACE_MODE_RMII:
		/* Set RMII mode */
		out_le32(plat->regs + ETH_REG_0, AXG_ETH_REG_0_PHY_INTF_RMII |
						 AXG_ETH_REG_0_INVERT_RMII_CLK |
						 AXG_ETH_REG_0_CLK_EN);
		break;
	default:
		dev_err(dev, "Unsupported PHY mode\n");
		return -EINVAL;
	}

	return 0;
}

static int dwmac_setup_gx(struct udevice *dev, struct eth_pdata *edata)
{
	struct dwmac_meson8b_plat *plat = dev_get_plat(dev);

	switch (edata->phy_interface) {
	case PHY_INTERFACE_MODE_RGMII:
	case PHY_INTERFACE_MODE_RGMII_ID:
		/* Set RGMII mode */
		setbits_le32(plat->regs + ETH_REG_0, GX_ETH_REG_0_PHY_INTF |
						     GX_ETH_REG_0_TX_PHASE(1) |
						     GX_ETH_REG_0_TX_RATIO(4) |
						     GX_ETH_REG_0_PHY_CLK_EN |
						     GX_ETH_REG_0_CLK_EN);

		break;

	case PHY_INTERFACE_MODE_RGMII_RXID:
	case PHY_INTERFACE_MODE_RGMII_TXID:
		/* TOFIX: handle amlogic,tx-delay-ns & rx-internal-delay-ps from DT */
		setbits_le32(plat->regs + ETH_REG_0, GX_ETH_REG_0_PHY_INTF |
						     GX_ETH_REG_0_TX_RATIO(4) |
						     GX_ETH_REG_0_PHY_CLK_EN |
						     GX_ETH_REG_0_CLK_EN);

		break;

	case PHY_INTERFACE_MODE_RMII:
		/* Set RMII mode */
		out_le32(plat->regs + ETH_REG_0, GX_ETH_REG_0_INVERT_RMII_CLK |
						 GX_ETH_REG_0_CLK_EN);

		if (!IS_ENABLED(CONFIG_MESON_GXBB))
			writel(0x10110181, plat->regs + ETH_REG_2);

		break;
	default:
		dev_err(dev, "Unsupported PHY mode\n");
		return -EINVAL;
	}

	return 0;
}

static int dwmac_meson8b_probe(struct udevice *dev)
{
	struct dwmac_meson8b_plat *pdata = dev_get_plat(dev);
	struct eth_pdata *edata = &pdata->dw_eth_pdata.eth_pdata;
	int ret;

	ret = pdata->dwmac_setup(dev, edata);
	if (ret)
		return ret;

	return designware_eth_probe(dev);
}

static const struct udevice_id dwmac_meson8b_ids[] = {
	{ .compatible = "amlogic,meson-gxbb-dwmac", .data = (ulong)dwmac_setup_gx },
	{ .compatible = "amlogic,meson-g12a-dwmac", .data = (ulong)dwmac_setup_axg },
	{ .compatible = "amlogic,meson-axg-dwmac", .data = (ulong)dwmac_setup_axg },
	{ }
};

U_BOOT_DRIVER(dwmac_meson8b) = {
	.name		= "dwmac_meson8b",
	.id		= UCLASS_ETH,
	.of_match	= dwmac_meson8b_ids,
	.of_to_plat = dwmac_meson8b_of_to_plat,
	.probe		= dwmac_meson8b_probe,
	.ops		= &designware_eth_ops,
	.priv_auto	= sizeof(struct dw_eth_dev),
	.plat_auto	= sizeof(struct dwmac_meson8b_plat),
	.flags		= DM_FLAG_ALLOC_PRIV_DMA,
};
