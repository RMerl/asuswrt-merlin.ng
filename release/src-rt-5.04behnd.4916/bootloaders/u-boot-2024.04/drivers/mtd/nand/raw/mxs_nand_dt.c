/*
 * NXP GPMI NAND flash driver (DT initialization)
 *
 * Copyright (C) 2018 Toradex
 * Copyright 2019 NXP
 *
 * Authors:
 * Stefan Agner <stefan.agner@toradex.com>
 *
 * Based on denali_dt.c
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <dm.h>
#include <linux/io.h>
#include <linux/ioport.h>
#include <linux/printk.h>
#include <clk.h>

#include <mxs_nand.h>

struct mxs_nand_dt_data {
	unsigned int max_ecc_strength_supported;
	int max_chain_delay; /* See the async EDO mode */
};

static const struct mxs_nand_dt_data mxs_nand_imx6q_data = {
	.max_ecc_strength_supported = 40,
	.max_chain_delay = 12000,
};

static const struct mxs_nand_dt_data mxs_nand_imx6sx_data = {
	.max_ecc_strength_supported = 62,
	.max_chain_delay = 12000,
};

static const struct mxs_nand_dt_data mxs_nand_imx7d_data = {
	.max_ecc_strength_supported = 62,
	.max_chain_delay = 12000,
};

static const struct mxs_nand_dt_data mxs_nand_imx8qxp_data = {
	.max_ecc_strength_supported = 62,
	.max_chain_delay = 12000,
};

static const struct udevice_id mxs_nand_dt_ids[] = {
	{
		.compatible = "fsl,imx6q-gpmi-nand",
		.data = (unsigned long)&mxs_nand_imx6q_data,
	},
	{
		.compatible = "fsl,imx6qp-gpmi-nand",
		.data = (unsigned long)&mxs_nand_imx6q_data,
	},
	{
		.compatible = "fsl,imx6sx-gpmi-nand",
		.data = (unsigned long)&mxs_nand_imx6sx_data,
	},
	{
		.compatible = "fsl,imx7d-gpmi-nand",
		.data = (unsigned long)&mxs_nand_imx7d_data,
	},
	{
		.compatible = "fsl,imx8qxp-gpmi-nand",
		.data = (unsigned long)&mxs_nand_imx8qxp_data,
	},
	{ /* sentinel */ }
};

static int mxs_nand_dt_probe(struct udevice *dev)
{
	struct mxs_nand_info *info = dev_get_priv(dev);
	const struct mxs_nand_dt_data *data;
	struct resource res;
	int ret;

	data = (void *)dev_get_driver_data(dev);
	if (data) {
		info->max_ecc_strength_supported = data->max_ecc_strength_supported;
		info->max_chain_delay = data->max_chain_delay;
	}

	info->dev = dev;

	ret = dev_read_resource_byname(dev, "gpmi-nand", &res);
	if (ret)
		return ret;

	info->gpmi_regs = devm_ioremap(dev, res.start, resource_size(&res));


	ret = dev_read_resource_byname(dev, "bch", &res);
	if (ret)
		return ret;

	info->bch_regs = devm_ioremap(dev, res.start, resource_size(&res));

	info->use_minimum_ecc = dev_read_bool(dev, "fsl,use-minimum-ecc");

	if (IS_ENABLED(CONFIG_CLK) &&
	    (IS_ENABLED(CONFIG_IMX8) || IS_ENABLED(CONFIG_IMX8M))) {
		/* Assigned clock already set clock */
		struct clk gpmi_clk;

		info->gpmi_clk = devm_clk_get(dev, "gpmi_io");

		if (IS_ERR(info->gpmi_clk)) {
			ret = PTR_ERR(info->gpmi_clk);
			debug("Can't get gpmi io clk: %d\n", ret);
			return ret;
		}

		ret = clk_enable(info->gpmi_clk);
		if (ret < 0) {
			debug("Can't enable gpmi io clk: %d\n", ret);
			return ret;
		}

		if (IS_ENABLED(CONFIG_IMX8)) {
			ret = clk_get_by_name(dev, "gpmi_apb", &gpmi_clk);
			if (ret < 0) {
				debug("Can't get gpmi_apb clk: %d\n", ret);
				return ret;
			}

			ret = clk_enable(&gpmi_clk);
			if (ret < 0) {
				debug("Can't enable gpmi_apb clk: %d\n", ret);
				return ret;
			}

			ret = clk_get_by_name(dev, "gpmi_bch", &gpmi_clk);
			if (ret < 0) {
				debug("Can't get gpmi_bch clk: %d\n", ret);
				return ret;
			}

			ret = clk_enable(&gpmi_clk);
			if (ret < 0) {
				debug("Can't enable gpmi_bch clk: %d\n", ret);
				return ret;
			}
		}

		ret = clk_get_by_name(dev, "gpmi_bch_apb", &gpmi_clk);
		if (ret < 0) {
			debug("Can't get gpmi_bch_apb clk: %d\n", ret);
			return ret;
		}

		ret = clk_enable(&gpmi_clk);
		if (ret < 0) {
			debug("Can't enable gpmi_bch_apb clk: %d\n", ret);
			return ret;
		}
	}

	return mxs_nand_init_ctrl(info);
}

U_BOOT_DRIVER(mxs_nand_dt) = {
	.name = "mxs-nand-dt",
	.id = UCLASS_MTD,
	.of_match = mxs_nand_dt_ids,
	.probe = mxs_nand_dt_probe,
	.priv_auto	= sizeof(struct mxs_nand_info),
};

void board_nand_init(void)
{
	struct udevice *dev;
	int ret;

	ret = uclass_get_device_by_driver(UCLASS_MTD,
					  DM_DRIVER_GET(mxs_nand_dt),
					  &dev);
	if (ret && ret != -ENODEV)
		pr_err("Failed to initialize MXS NAND controller. (error %d)\n",
		       ret);
}
