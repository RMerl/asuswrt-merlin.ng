// SPDX-License-Identifier: GPL-2.0+
/*
 * Texas Instruments CDCE913/925/937/949 clock synthesizer driver
 *
 * Copyright (C) 2019 Texas Instruments Incorporated - https://www.ti.com/
 *	Tero Kristo <t-kristo@ti.com>
 *
 * Based on Linux kernel clk-cdce925.c.
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <clk-uclass.h>
#include <i2c.h>
#include <dm/device_compat.h>
#include <linux/bitops.h>

#define MAX_NUMBER_OF_PLLS		4
#define MAX_NUMER_OF_OUTPUTS		9

#define CDCE9XX_REG_GLOBAL1		0x01
#define CDCE9XX_REG_Y1SPIPDIVH		0x02
#define CDCE9XX_REG_PDIV1L		0x03
#define CDCE9XX_REG_XCSEL		0x05

#define CDCE9XX_PDIV1_H_MASK		0x3

#define CDCE9XX_REG_PDIV(clk)		(0x16 + (((clk) - 1) & 1) + \
					 ((clk) - 1) / 2 * 0x10)

#define CDCE9XX_PDIV_MASK		0x7f

#define CDCE9XX_BYTE_TRANSFER		BIT(7)

struct cdce9xx_chip_info {
	int num_plls;
	int num_outputs;
};

struct cdce9xx_clk_data {
	struct udevice *i2c;
	struct cdce9xx_chip_info *chip;
	u32 xtal_rate;
};

static const struct cdce9xx_chip_info cdce913_chip_info = {
	.num_plls = 1, .num_outputs = 3,
};

static const struct cdce9xx_chip_info cdce925_chip_info = {
	.num_plls = 2, .num_outputs = 5,
};

static const struct cdce9xx_chip_info cdce937_chip_info = {
	.num_plls = 3, .num_outputs = 7,
};

static const struct cdce9xx_chip_info cdce949_chip_info = {
	.num_plls = 4, .num_outputs = 9,
};

static int cdce9xx_reg_read(struct udevice *dev, u8 addr, u8 *buf)
{
	struct cdce9xx_clk_data *data = dev_get_priv(dev);
	int ret;

	ret = dm_i2c_read(data->i2c, addr | CDCE9XX_BYTE_TRANSFER, buf, 1);
	if (ret)
		dev_err(dev, "%s: failed for addr:%x, ret:%d\n", __func__,
			addr, ret);

	return ret;
}

static int cdce9xx_reg_write(struct udevice *dev, u8 addr, u8 val)
{
	struct cdce9xx_clk_data *data = dev_get_priv(dev);
	int ret;

	ret = dm_i2c_write(data->i2c, addr | CDCE9XX_BYTE_TRANSFER, &val, 1);
	if (ret)
		dev_err(dev, "%s: failed for addr:%x, ret:%d\n", __func__,
			addr, ret);

	return ret;
}

static int cdce9xx_clk_request(struct clk *clk)
{
	struct cdce9xx_clk_data *data = dev_get_priv(clk->dev);

	if (clk->id > data->chip->num_outputs)
		return -EINVAL;

	return 0;
}

static int cdce9xx_clk_probe(struct udevice *dev)
{
	struct cdce9xx_clk_data *data = dev_get_priv(dev);
	struct cdce9xx_chip_info *chip = (void *)dev_get_driver_data(dev);
	int ret;
	u32 val;
	struct clk clk;

	val = (u32)dev_read_addr_ptr(dev);

	ret = i2c_get_chip(dev->parent, val, 1, &data->i2c);
	if (ret) {
		dev_err(dev, "I2C probe failed.\n");
		return ret;
	}

	data->chip = chip;

	ret = clk_get_by_index(dev, 0, &clk);
	data->xtal_rate = clk_get_rate(&clk);

	val = dev_read_u32_default(dev, "xtal-load-pf", -1);
	if (val >= 0)
		cdce9xx_reg_write(dev, CDCE9XX_REG_XCSEL, val << 3);

	return 0;
}

static u16 cdce9xx_clk_get_pdiv(struct clk *clk)
{
	u8 val;
	u16 pdiv;
	int ret;

	if (clk->id == 0) {
		ret = cdce9xx_reg_read(clk->dev, CDCE9XX_REG_Y1SPIPDIVH, &val);
		if (ret)
			return 0;

		pdiv = (val & CDCE9XX_PDIV1_H_MASK) << 8;

		ret = cdce9xx_reg_read(clk->dev, CDCE9XX_REG_PDIV1L, &val);
		if (ret)
			return 0;

		pdiv |= val;
	} else {
		ret = cdce9xx_reg_read(clk->dev, CDCE9XX_REG_PDIV(clk->id),
				       &val);
		if (ret)
			return 0;

		pdiv = val & CDCE9XX_PDIV_MASK;
	}

	return pdiv;
}

static u32 cdce9xx_clk_get_parent_rate(struct clk *clk)
{
	struct cdce9xx_clk_data *data = dev_get_priv(clk->dev);

	return data->xtal_rate;
}

static ulong cdce9xx_clk_get_rate(struct clk *clk)
{
	u32 parent_rate;
	u16 pdiv;

	parent_rate = cdce9xx_clk_get_parent_rate(clk);

	pdiv = cdce9xx_clk_get_pdiv(clk);

	return parent_rate / pdiv;
}

static ulong cdce9xx_clk_set_rate(struct clk *clk, ulong rate)
{
	u32 parent_rate;
	int pdiv;
	u32 diff;
	u8 val;
	int ret;

	parent_rate = cdce9xx_clk_get_parent_rate(clk);

	pdiv = parent_rate / rate;

	diff = rate - parent_rate / pdiv;

	if (rate - parent_rate / (pdiv + 1) < diff)
		pdiv++;

	if (clk->id == 0) {
		ret = cdce9xx_reg_read(clk->dev, CDCE9XX_REG_Y1SPIPDIVH, &val);
		if (ret)
			return ret;

		val &= ~CDCE9XX_PDIV1_H_MASK;

		val |= (pdiv >> 8);

		ret = cdce9xx_reg_write(clk->dev, CDCE9XX_REG_Y1SPIPDIVH, val);
		if (ret)
			return ret;

		ret = cdce9xx_reg_write(clk->dev, CDCE9XX_REG_PDIV1L,
					(pdiv & 0xff));
		if (ret)
			return ret;
	} else {
		ret = cdce9xx_reg_read(clk->dev, CDCE9XX_REG_PDIV(clk->id),
				       &val);
		if (ret)
			return ret;

		val &= ~CDCE9XX_PDIV_MASK;

		val |= pdiv;

		ret = cdce9xx_reg_write(clk->dev, CDCE9XX_REG_PDIV(clk->id),
					val);
		if (ret)
			return ret;
	}

	return 0;
}

static const struct udevice_id cdce9xx_clk_of_match[] = {
	{ .compatible = "ti,cdce913", .data = (u32)&cdce913_chip_info },
	{ .compatible = "ti,cdce925", .data = (u32)&cdce925_chip_info },
	{ .compatible = "ti,cdce937", .data = (u32)&cdce937_chip_info },
	{ .compatible = "ti,cdce949", .data = (u32)&cdce949_chip_info },
	{ /* sentinel */ },
};

static const struct clk_ops cdce9xx_clk_ops = {
	.request = cdce9xx_clk_request,
	.get_rate = cdce9xx_clk_get_rate,
	.set_rate = cdce9xx_clk_set_rate,
};

U_BOOT_DRIVER(cdce9xx_clk) = {
	.name = "cdce9xx-clk",
	.id = UCLASS_CLK,
	.of_match = cdce9xx_clk_of_match,
	.probe = cdce9xx_clk_probe,
	.priv_auto	= sizeof(struct cdce9xx_clk_data),
	.ops = &cdce9xx_clk_ops,
};
