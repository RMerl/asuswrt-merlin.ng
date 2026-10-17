// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2019 Texas Instruments Incorporated - https://www.ti.com/
 *      Tero Kristo <t-kristo@ti.com>
 */

#include <common.h>
#include <dm.h>
#include <i2c.h>
#include <dm/device_compat.h>
#include <power/regulator.h>

#define TPS62360_REG_SET0	0

#define TPS62360_I2C_CHIP	0x60

#define TPS62360_VSEL_STEPSIZE	10000 /* In uV */

struct tps62360_regulator_config {
	u32 vmin;
	u32 vmax;
};

struct tps62360_regulator_pdata {
	u8 vsel_offset;
	struct udevice *i2c;
	struct tps62360_regulator_config *config;
};

/*
 * TPS62362/TPS62363 are just re-using these values for now, their preset
 * voltage values are just different compared to TPS62360/TPS62361.
 */
static struct tps62360_regulator_config tps62360_data = {
	.vmin = 770000,
	.vmax = 1400000,
};

static struct tps62360_regulator_config tps62361_data = {
	.vmin = 500000,
	.vmax = 1770000,
};

static int tps62360_regulator_set_value(struct udevice *dev, int uV)
{
	struct tps62360_regulator_pdata *pdata = dev_get_plat(dev);
	u8 regval;

	if (uV < pdata->config->vmin || uV > pdata->config->vmax)
		return -EINVAL;

	uV -= pdata->config->vmin;

	uV = DIV_ROUND_UP(uV, TPS62360_VSEL_STEPSIZE);

	if (uV > U8_MAX)
		return -EINVAL;

	regval = (u8)uV;

	return dm_i2c_write(pdata->i2c, TPS62360_REG_SET0 + pdata->vsel_offset,
			    &regval, 1);
}

static int tps62360_regulator_get_value(struct udevice *dev)
{
	u8 regval;
	int ret;
	struct tps62360_regulator_pdata *pdata = dev_get_plat(dev);

	ret = dm_i2c_read(pdata->i2c, TPS62360_REG_SET0 + pdata->vsel_offset,
			  &regval, 1);
	if (ret) {
		dev_err(dev, "i2c read failed: %d\n", ret);
		return ret;
	}

	return (u32)regval * TPS62360_VSEL_STEPSIZE + pdata->config->vmin;
}

static int tps62360_regulator_probe(struct udevice *dev)
{
	struct tps62360_regulator_pdata *pdata = dev_get_plat(dev);
	u8 vsel0;
	u8 vsel1;
	int ret;

	pdata->config = (void *)dev_get_driver_data(dev);

	vsel0 = dev_read_bool(dev, "ti,vsel0-state-high");
	vsel1 = dev_read_bool(dev, "ti,vsel1-state-high");

	pdata->vsel_offset = vsel0 + vsel1 * 2;

	ret = i2c_get_chip(dev->parent, TPS62360_I2C_CHIP, 1, &pdata->i2c);
	if (ret) {
		dev_err(dev, "i2c dev get failed.\n");
		return ret;
	}

	return 0;
}

static const struct dm_regulator_ops tps62360_regulator_ops = {
	.get_value  = tps62360_regulator_get_value,
	.set_value  = tps62360_regulator_set_value,
};

static const struct udevice_id tps62360_regulator_ids[] = {
	{ .compatible = "ti,tps62360", .data = (ulong)&tps62360_data },
	{ .compatible = "ti,tps62361", .data = (ulong)&tps62361_data },
	{ .compatible = "ti,tps62362", .data = (ulong)&tps62360_data },
	{ .compatible = "ti,tps62363", .data = (ulong)&tps62361_data },
	{ },
};

U_BOOT_DRIVER(tps62360_regulator) = {
	.name = "tps62360_regulator",
	.id = UCLASS_REGULATOR,
	.ops = &tps62360_regulator_ops,
	.of_match = tps62360_regulator_ids,
	.plat_auto	= sizeof(struct tps62360_regulator_pdata),
	.probe = tps62360_regulator_probe,
};
