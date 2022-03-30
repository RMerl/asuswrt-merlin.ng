// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015 - 2016 Xilinx, Inc.
 * Copyright (C) 2017 National Instruments Corp
 * Written by Michal Simek
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <i2c.h>

#include <asm-generic/gpio.h>

DECLARE_GLOBAL_DATA_PTR;

enum pca_type {
	PCA9543,
	PCA9544,
	PCA9547,
	PCA9548,
	PCA9646
};

struct chip_desc {
	u8 enable; /* Enable mask in ctl register (used for muxes only) */
	enum muxtype {
		pca954x_ismux = 0,
		pca954x_isswi,
	} muxtype;
	u32 width;
};

struct pca954x_priv {
	u32 addr; /* I2C mux address */
	u32 width; /* I2C mux width - number of busses */
	struct gpio_desc gpio_mux_reset;
};

static const struct chip_desc chips[] = {
	[PCA9543] = {
		.muxtype = pca954x_isswi,
		.width = 2,
	},
	[PCA9544] = {
		.enable = 0x4,
		.muxtype = pca954x_ismux,
		.width = 4,
	},
	[PCA9547] = {
		.enable = 0x8,
		.muxtype = pca954x_ismux,
		.width = 8,
	},
	[PCA9548] = {
		.muxtype = pca954x_isswi,
		.width = 8,
	},
	[PCA9646] = {
		.muxtype = pca954x_isswi,
		.width = 4,
	},
};

static int pca954x_deselect(struct udevice *mux, struct udevice *bus,
			    uint channel)
{
	struct pca954x_priv *priv = dev_get_priv(mux);
	uchar byte = 0;

	return dm_i2c_write(mux, priv->addr, &byte, 1);
}

static int pca954x_select(struct udevice *mux, struct udevice *bus,
			  uint channel)
{
	struct pca954x_priv *priv = dev_get_priv(mux);
	const struct chip_desc *chip = &chips[dev_get_driver_data(mux)];
	uchar byte;

	if (chip->muxtype == pca954x_ismux)
		byte = channel | chip->enable;
	else
		byte = 1 << channel;

	return dm_i2c_write(mux, priv->addr, &byte, 1);
}

static const struct i2c_mux_ops pca954x_ops = {
	.select = pca954x_select,
	.deselect = pca954x_deselect,
};

static const struct udevice_id pca954x_ids[] = {
	{ .compatible = "nxp,pca9543", .data = PCA9543 },
	{ .compatible = "nxp,pca9544", .data = PCA9544 },
	{ .compatible = "nxp,pca9547", .data = PCA9547 },
	{ .compatible = "nxp,pca9548", .data = PCA9548 },
	{ .compatible = "nxp,pca9646", .data = PCA9646 },
	{ }
};

static int pca954x_ofdata_to_platdata(struct udevice *dev)
{
	struct pca954x_priv *priv = dev_get_priv(dev);
	const struct chip_desc *chip = &chips[dev_get_driver_data(dev)];

	priv->addr = dev_read_u32_default(dev, "reg", 0);
	if (!priv->addr) {
		debug("MUX not found\n");
		return -ENODEV;
	}
	priv->width = chip->width;

	if (!priv->width) {
		debug("No I2C MUX width specified\n");
		return -EINVAL;
	}

	debug("Device %s at 0x%x with width %d\n",
	      dev->name, priv->addr, priv->width);

	return 0;
}

static int pca954x_probe(struct udevice *dev)
{
	if (IS_ENABLED(CONFIG_DM_GPIO)) {
		struct pca954x_priv *priv = dev_get_priv(dev);
		int err;

		err = gpio_request_by_name(dev, "reset-gpios", 0,
				&priv->gpio_mux_reset, GPIOD_IS_OUT);

		/* it's optional so only bail if we get a real error */
		if (err && (err != -ENOENT))
			return err;

		/* dm will take care of polarity */
		if (dm_gpio_is_valid(&priv->gpio_mux_reset))
			dm_gpio_set_value(&priv->gpio_mux_reset, 0);
	}

	return 0;
}

static int pca954x_remove(struct udevice *dev)
{
	if (IS_ENABLED(CONFIG_DM_GPIO)) {
		struct pca954x_priv *priv = dev_get_priv(dev);

		if (dm_gpio_is_valid(&priv->gpio_mux_reset))
			dm_gpio_free(dev, &priv->gpio_mux_reset);
	}

	return 0;
}

U_BOOT_DRIVER(pca954x) = {
	.name = "pca954x",
	.id = UCLASS_I2C_MUX,
	.of_match = pca954x_ids,
	.probe = pca954x_probe,
	.remove = pca954x_remove,
	.ops = &pca954x_ops,
	.ofdata_to_platdata = pca954x_ofdata_to_platdata,
	.priv_auto_alloc_size = sizeof(struct pca954x_priv),
};
