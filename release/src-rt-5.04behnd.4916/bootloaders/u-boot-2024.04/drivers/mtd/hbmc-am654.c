// SPDX-License-Identifier: GPL-2.0
//
// Copyright (C) 2019 Texas Instruments Incorporated - https://www.ti.com/
// Author: Vignesh Raghavendra <vigneshr@ti.com>

#include <common.h>
#include <asm/io.h>
#include <dm.h>
#include <regmap.h>
#include <syscon.h>
#include <dm/device_compat.h>

#define FSS_SYSC_REG	0x4

#define HYPERBUS_CALIB_COUNT 25

struct am654_hbmc_priv {
	void __iomem *mmiobase;
	bool calibrated;
};

/* Calibrate by looking for "QRY" string within the CFI space */
static int am654_hyperbus_calibrate(struct udevice *dev)
{
	struct am654_hbmc_priv *priv = dev_get_priv(dev);
	int count = HYPERBUS_CALIB_COUNT;
	int pass_count = 0;
	u16 qry[3];

	if (priv->calibrated)
		return 0;

	writew(0xF0, priv->mmiobase);
	writew(0x98, priv->mmiobase + 0xaa);

	while (count--) {
		qry[0] = readw(priv->mmiobase + 0x20);
		qry[1] = readw(priv->mmiobase + 0x22);
		qry[2] = readw(priv->mmiobase + 0x24);

		if (qry[0] == 'Q' && qry[1] == 'R' && qry[2] == 'Y')
			pass_count++;
		else
			pass_count = 0;
		if (pass_count == 5)
			break;
	}
	writew(0xF0, priv->mmiobase);
	writew(0xFF, priv->mmiobase);

	return pass_count == 5;
}

static int am654_select_hbmc(struct udevice *dev)
{
	struct regmap *regmap = syscon_get_regmap(dev_get_parent(dev));

	return regmap_update_bits(regmap, FSS_SYSC_REG, 0x2, 0x2);
}

static int am654_hbmc_bind(struct udevice *dev)
{
	return dm_scan_fdt_dev(dev);
}

static int am654_hbmc_probe(struct udevice *dev)
{
	struct am654_hbmc_priv *priv = dev_get_priv(dev);
	int ret;

	priv->mmiobase = devfdt_remap_addr_index(dev, 1);
	if (dev_read_bool(dev, "mux-controls")) {
		ret = am654_select_hbmc(dev);
		if (ret) {
			dev_err(dev, "Failed to select HBMC mux\n");
			return ret;
		}
	}

	if (!priv->calibrated) {
		ret = am654_hyperbus_calibrate(dev);
		if (!ret) {
			dev_err(dev, "Calibration Failed\n");
			return -EIO;
		}
	}
	priv->calibrated = true;

	return 0;
}

static const struct udevice_id am654_hbmc_dt_ids[] = {
	{
		.compatible = "ti,am654-hbmc",
	},
	{ /* end of table */ }
};

U_BOOT_DRIVER(hbmc_am654) = {
	.name	= "hbmc-am654",
	.id	= UCLASS_MTD,
	.of_match = am654_hbmc_dt_ids,
	.probe = am654_hbmc_probe,
	.bind = am654_hbmc_bind,
	.priv_auto	= sizeof(struct am654_hbmc_priv),
};
