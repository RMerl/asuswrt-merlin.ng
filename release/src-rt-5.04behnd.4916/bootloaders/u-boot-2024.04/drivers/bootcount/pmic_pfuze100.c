// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018-2022 Denx Software Engineering GmbH
 * Heiko Schocher <hs@denx.de>
 * Philip Oberfichtner <pro@denx.de>
 *
 * A bootcount driver using the registers MEMA - MEMD on the PFUZE100.
 * This works only, if the PMIC is not connected to a battery.
 */

#include <common.h>
#include <bootcount.h>
#include <dm.h>
#include <power/pmic.h>
#include <power/pfuze100_pmic.h>

DECLARE_GLOBAL_DATA_PTR;

#define PFUZE_BC_MAGIC 0xdead

struct bootcount_pmic_priv {
	struct udevice *pmic;
};

static int pfuze100_get_magic(struct udevice *dev, u32 *magic)
{
	int ret;

	ret = pmic_reg_read(dev, PFUZE100_MEMA);
	if (ret < 0)
		return ret;
	*magic = ret;

	ret = pmic_reg_read(dev, PFUZE100_MEMB);
	if (ret < 0)
		return ret;
	*magic += ret << 8;

	return 0;
}

static int pfuze100_set_magic(struct udevice *dev)
{
	int ret;

	ret = pmic_reg_write(dev, PFUZE100_MEMA, PFUZE_BC_MAGIC & 0xff);
	if (ret)
		return ret;

	ret = pmic_reg_write(dev, PFUZE100_MEMB, (PFUZE_BC_MAGIC >> 8) & 0xff);
	return ret;
}

static int pfuze100_get_value(struct udevice *dev, u32 *a)
{
	int ret;

	ret = pmic_reg_read(dev, PFUZE100_MEMC);
	if (ret < 0)
		return ret;
	*a = ret;

	ret = pmic_reg_read(dev, PFUZE100_MEMD);
	if (ret < 0)
		return ret;
	*a += ret << 8;

	return 0;
}

static int pfuze100_set_value(struct udevice *dev, u32 val)
{
	int ret;

	ret = pmic_reg_write(dev, PFUZE100_MEMC, val & 0xff);
	if (ret)
		return ret;

	ret = pmic_reg_write(dev, PFUZE100_MEMD, (val >> 8) & 0xff);
	return ret;
}

static int bootcount_pmic_set(struct udevice *dev, const u32 a)
{
	struct bootcount_pmic_priv *priv = dev_get_priv(dev);

	if (pfuze100_set_magic(priv->pmic)) {
		debug("%s: writing magic failed\n", __func__);
		return -EIO;
	}

	if (pfuze100_set_value(priv->pmic, a)) {
		debug("%s: writing value failed\n", __func__);
		return -EIO;
	}

	return 0;
}

static int bootcount_pmic_get(struct udevice *dev, u32 *a)
{
	struct bootcount_pmic_priv *priv = dev_get_priv(dev);
	u32 magic;

	if (pfuze100_get_magic(priv->pmic, &magic)) {
		debug("%s: reading magic failed\n", __func__);
		return -EIO;
	}

	if (magic != PFUZE_BC_MAGIC) {
		*a = 0;
		return 0;
	}

	if (pfuze100_get_value(priv->pmic, a)) {
		debug("%s: reading value failed\n", __func__);
		return -EIO;
	}

	return 0;
}

static int bootcount_pmic_probe(struct udevice *dev)
{
	struct ofnode_phandle_args phandle_args;
	struct bootcount_pmic_priv *priv = dev_get_priv(dev);
	struct udevice *pmic;

	if (dev_read_phandle_with_args(dev, "pmic", NULL, 0, 0, &phandle_args)) {
		debug("%s: pmic backing device not specified\n", dev->name);
		return -ENOENT;
	}

	if (uclass_get_device_by_ofnode(UCLASS_PMIC, phandle_args.node, &pmic)) {
		debug("%s: could not get backing device\n", dev->name);
		return -ENODEV;
	}

	priv->pmic = pmic;

	return 0;
}

static const struct bootcount_ops bootcount_pmic_ops = {
	.get = bootcount_pmic_get,
	.set = bootcount_pmic_set,
};

static const struct udevice_id bootcount_pmic_ids[] = {
	{ .compatible = "u-boot,bootcount-pmic" },
	{ }
};

U_BOOT_DRIVER(bootcount_pmic) = {
	.name	= "bootcount-pmic",
	.id	= UCLASS_BOOTCOUNT,
	.priv_auto	= sizeof(struct bootcount_pmic_priv),
	.probe	= bootcount_pmic_probe,
	.of_match = bootcount_pmic_ids,
	.ops	= &bootcount_pmic_ops,
};
