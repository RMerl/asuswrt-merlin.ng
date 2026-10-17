// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017 Socionext Inc.
 *   Author: Masahiro Yamada <yamada.masahiro@socionext.com>
 */

#include <clk.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <linux/bug.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <linux/ioport.h>
#include <linux/printk.h>
#include <reset.h>

#include "denali.h"

struct denali_dt_data {
	unsigned int revision;
	unsigned int caps;
	unsigned int oob_skip_bytes;
	const struct nand_ecc_caps *ecc_caps;
};

NAND_ECC_CAPS_SINGLE(denali_socfpga_ecc_caps, denali_calc_ecc_bytes,
		     512, 8, 15);
static const struct denali_dt_data denali_socfpga_data = {
	.caps = DENALI_CAP_HW_ECC_FIXUP,
	.oob_skip_bytes = 2,
	.ecc_caps = &denali_socfpga_ecc_caps,
};

NAND_ECC_CAPS_SINGLE(denali_uniphier_v5a_ecc_caps, denali_calc_ecc_bytes,
		     1024, 8, 16, 24);
static const struct denali_dt_data denali_uniphier_v5a_data = {
	.caps = DENALI_CAP_HW_ECC_FIXUP |
		DENALI_CAP_DMA_64BIT,
	.oob_skip_bytes = 8,
	.ecc_caps = &denali_uniphier_v5a_ecc_caps,
};

NAND_ECC_CAPS_SINGLE(denali_uniphier_v5b_ecc_caps, denali_calc_ecc_bytes,
		     1024, 8, 16);
static const struct denali_dt_data denali_uniphier_v5b_data = {
	.revision = 0x0501,
	.caps = DENALI_CAP_HW_ECC_FIXUP |
		DENALI_CAP_DMA_64BIT,
	.oob_skip_bytes = 8,
	.ecc_caps = &denali_uniphier_v5b_ecc_caps,
};

static const struct udevice_id denali_nand_dt_ids[] = {
	{
		.compatible = "altr,socfpga-denali-nand",
		.data = (unsigned long)&denali_socfpga_data,
	},
	{
		.compatible = "socionext,uniphier-denali-nand-v5a",
		.data = (unsigned long)&denali_uniphier_v5a_data,
	},
	{
		.compatible = "socionext,uniphier-denali-nand-v5b",
		.data = (unsigned long)&denali_uniphier_v5b_data,
	},
	{ /* sentinel */ }
};

static int denali_dt_probe(struct udevice *dev)
{
	struct denali_nand_info *denali = dev_get_priv(dev);
	const struct denali_dt_data *data;
	struct clk clk, clk_x, clk_ecc;
	struct reset_ctl_bulk resets;
	struct resource res;
	int ret;

	data = (void *)dev_get_driver_data(dev);
	if (WARN_ON(!data))
		return -EINVAL;

	denali->revision = data->revision;
	denali->caps = data->caps;
	denali->oob_skip_bytes = data->oob_skip_bytes;
	denali->ecc_caps = data->ecc_caps;

	denali->dev = dev;

	ret = dev_read_resource_byname(dev, "denali_reg", &res);
	if (ret)
		return ret;

	denali->reg = devm_ioremap(dev, res.start, resource_size(&res));

	ret = dev_read_resource_byname(dev, "nand_data", &res);
	if (ret)
		return ret;

	denali->host = devm_ioremap(dev, res.start, resource_size(&res));

	ret = clk_get_by_name(dev, "nand", &clk);
	if (ret)
		ret = clk_get_by_index(dev, 0, &clk);
	if (ret)
		clk.dev = NULL;

	ret = clk_get_by_name(dev, "nand_x", &clk_x);
	if (ret)
		clk_x.dev = NULL;

	ret = clk_get_by_name(dev, "ecc", &clk_ecc);
	if (ret)
		clk_ecc.dev = NULL;

	if (clk.dev) {
		ret = clk_enable(&clk);
		if (ret)
			return ret;
	}

	if (clk_x.dev) {
		ret = clk_enable(&clk_x);
		if (ret)
			return ret;
	}

	if (clk_ecc.dev) {
		ret = clk_enable(&clk_ecc);
		if (ret)
			return ret;
	}

	if (clk_x.dev) {
		denali->clk_rate = clk_get_rate(&clk);
		denali->clk_x_rate = clk_get_rate(&clk_x);
	} else {
		/*
		 * Hardcode the clock rates for the backward compatibility.
		 * This works for both SOCFPGA and UniPhier.
		 */
		dev_notice(dev,
			   "necessary clock is missing. default clock rates are used.\n");
		denali->clk_rate = 50000000;
		denali->clk_x_rate = 200000000;
	}

	ret = reset_get_bulk(dev, &resets);
	if (ret) {
		dev_warn(dev, "Can't get reset: %d\n", ret);
	} else {
		reset_assert_bulk(&resets);
		udelay(2);
		reset_deassert_bulk(&resets);

		/*
		 * When the reset is deasserted, the initialization sequence is
		 * kicked (bootstrap process). The driver must wait until it is
		 * finished. Otherwise, it will result in unpredictable behavior.
		 */
		ret = denali_wait_reset_complete(denali);
		if (ret) {
			dev_err(denali->dev, "reset not completed.\n");
			return ret;
		}
	}

	return denali_init(denali);
}

U_BOOT_DRIVER(denali_nand_dt) = {
	.name = "denali-nand-dt",
	.id = UCLASS_MTD,
	.of_match = denali_nand_dt_ids,
	.probe = denali_dt_probe,
	.priv_auto	= sizeof(struct denali_nand_info),
};

void board_nand_init(void)
{
	struct udevice *dev;
	int ret;

	ret = uclass_get_device_by_driver(UCLASS_MTD,
					  DM_DRIVER_GET(denali_nand_dt),
					  &dev);
	if (ret && ret != -ENODEV)
		pr_err("Failed to initialize Denali NAND controller. (error %d)\n",
		       ret);
}
