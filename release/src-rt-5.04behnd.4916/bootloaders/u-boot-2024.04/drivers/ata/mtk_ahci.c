// SPDX-License-Identifier: GPL-2.0+
/*
 * MTK SATA platform driver
 *
 * Copyright (C) 2020 MediaTek Inc.
 *
 * Author: Ryder Lee <ryder.lee@mediatek.com>
 * Author: Frank Wunderlich <frank-w@public-files.de>
 */

#include <common.h>
#include <ahci.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <dm.h>
#include <dm/of_access.h>
#include <generic-phy.h>
#include <linux/err.h>
#include <regmap.h>
#include <reset.h>
#include <sata.h>
#include <scsi.h>
#include <syscon.h>
#include <dm/device_compat.h>

#define SYS_CFG			0x14
#define SYS_CFG_SATA_MSK	GENMASK(31, 30)
#define SYS_CFG_SATA_EN		BIT(31)

struct mtk_ahci_priv {
	void *base;

	struct ahci_uc_priv ahci_priv;
	struct regmap *mode;
	struct reset_ctl_bulk rst_bulk;
};

static int mtk_ahci_bind(struct udevice *dev)
{
	struct udevice *scsi_dev;

	return ahci_bind_scsi(dev, &scsi_dev);
}

static int mtk_ahci_of_to_plat(struct udevice *dev)
{
	struct mtk_ahci_priv *priv = dev_get_priv(dev);

	priv->base = devfdt_remap_addr_index(dev, 0);

	return 0;
}

static int mtk_ahci_parse_property(struct ahci_uc_priv *hpriv,
				   struct udevice *dev)
{
	struct mtk_ahci_priv *plat = dev_get_priv(dev);
	const void *fdt = gd->fdt_blob;

	/* enable SATA function if needed */
	if (fdt_get_property(fdt, dev_of_offset(dev),
			     "mediatek,phy-mode", NULL)) {
		plat->mode = syscon_regmap_lookup_by_phandle(dev,
						"mediatek,phy-mode");
		if (IS_ERR(plat->mode)) {
			dev_err(dev, "missing phy-mode phandle\n");
			return PTR_ERR(plat->mode);
		}
		regmap_update_bits(plat->mode, SYS_CFG,
				   SYS_CFG_SATA_MSK, SYS_CFG_SATA_EN);
	}

	ofnode_read_u32(dev_ofnode(dev), "ports-implemented",
			&hpriv->port_map);
	return 0;
}

static int mtk_ahci_probe(struct udevice *dev)
{
	struct mtk_ahci_priv *priv = dev_get_priv(dev);
	int ret;
	struct phy phy;

	ret = mtk_ahci_parse_property(&priv->ahci_priv, dev);
	if (ret)
		return ret;

	ret = reset_get_bulk(dev, &priv->rst_bulk);
	if (!ret) {
		reset_assert_bulk(&priv->rst_bulk);
		reset_deassert_bulk(&priv->rst_bulk);
	} else {
		dev_err(dev, "Failed to get reset: %d\n", ret);
	}

	ret = generic_phy_get_by_name(dev, "sata-phy", &phy);
	if (ret) {
		pr_err("can't get the phy from DT\n");
		return ret;
	}

	ret = generic_phy_init(&phy);
	if (ret) {
		pr_err("unable to initialize the sata phy\n");
		return ret;
	}

	ret = generic_phy_power_on(&phy);
	if (ret) {
		pr_err("unable to power on the sata phy\n");
		return ret;
	}

	return ahci_probe_scsi(dev, (ulong)priv->base);
}

static const struct udevice_id mtk_ahci_ids[] = {
	{ .compatible = "mediatek,mtk-ahci" },
	{ }
};

U_BOOT_DRIVER(mtk_ahci) = {
	.name	= "mtk_ahci",
	.id	= UCLASS_AHCI,
	.of_match = mtk_ahci_ids,
	.bind	= mtk_ahci_bind,
	.of_to_plat = mtk_ahci_of_to_plat,
	.ops	= &scsi_ops,
	.probe	= mtk_ahci_probe,
	.priv_auto	= sizeof(struct mtk_ahci_priv),
};
