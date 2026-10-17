// SPDX-License-Identifier: GPL-2.0+
/*
 * I2C multiplexer using GPIO API
 *
 * Copyright 2017 NXP
 *
 * Peng Fan <peng.fan@nxp.com>
 */

#include <asm/global_data.h>
#include <asm/io.h>
#include <asm-generic/gpio.h>
#include <common.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <dm/devres.h>
#include <dm/pinctrl.h>
#include <fdtdec.h>
#include <i2c.h>
#include <linux/errno.h>
#include <linux/libfdt.h>

DECLARE_GLOBAL_DATA_PTR;

/**
 * struct i2c_mux_gpio_priv - private data for i2c mux gpio
 *
 * @values: the reg value of each child node
 * @n_values: num of regs
 * @gpios: the mux-gpios array
 * @n_gpios: num of gpios in mux-gpios
 * @idle: the value of idle-state
 */
struct i2c_mux_gpio_priv {
	u32 *values;
	int n_values;
	struct gpio_desc *gpios;
	int n_gpios;
	u32 idle;
};


static int i2c_mux_gpio_select(struct udevice *dev, struct udevice *bus,
			       uint channel)
{
	struct i2c_mux_gpio_priv *priv = dev_get_priv(dev);
	int i, ret;

	for (i = 0; i < priv->n_gpios; i++) {
		ret = dm_gpio_set_value(&priv->gpios[i], (channel >> i) & 1);
		if (ret)
			return ret;
	}

	return 0;
}

static int i2c_mux_gpio_deselect(struct udevice *dev, struct udevice *bus,
				 uint channel)
{
	struct i2c_mux_gpio_priv *priv = dev_get_priv(dev);
	int i, ret;

	for (i = 0; i < priv->n_gpios; i++) {
		ret = dm_gpio_set_value(&priv->gpios[i], (priv->idle >> i) & 1);
		if (ret)
			return ret;
	}

	return 0;
}

static int i2c_mux_gpio_probe(struct udevice *dev)
{
	const void *fdt = gd->fdt_blob;
	int node = dev_of_offset(dev);
	struct i2c_mux_gpio_priv *mux = dev_get_priv(dev);
	struct gpio_desc *gpios;
	u32 *values;
	int i = 0, subnode, ret;

	mux->n_values = fdtdec_get_child_count(fdt, node);
	values = devm_kzalloc(dev, sizeof(*mux->values) * mux->n_values,
			      GFP_KERNEL);
	if (!values) {
		dev_err(dev, "Cannot alloc values array");
		return -ENOMEM;
	}

	fdt_for_each_subnode(subnode, fdt, node) {
		*(values + i) = fdtdec_get_uint(fdt, subnode, "reg", -1);
		i++;
	}

	mux->values = values;

	mux->idle = fdtdec_get_uint(fdt, node, "idle-state", -1);

	mux->n_gpios = gpio_get_list_count(dev, "mux-gpios");
	if (mux->n_gpios < 0) {
		dev_err(dev, "Missing mux-gpios property\n");
		return -EINVAL;
	}

	gpios = devm_kzalloc(dev, sizeof(struct gpio_desc) * mux->n_gpios,
			     GFP_KERNEL);
	if (!gpios) {
		dev_err(dev, "Cannot allocate gpios array\n");
		return -ENOMEM;
	}

	ret = gpio_request_list_by_name(dev, "mux-gpios", gpios, mux->n_gpios,
					GPIOD_IS_OUT | GPIOD_IS_OUT_ACTIVE);
	if (ret <= 0) {
		dev_err(dev, "Failed to request mux-gpios\n");
		return ret;
	}

	mux->gpios = gpios;

	return 0;
}

static const struct i2c_mux_ops i2c_mux_gpio_ops = {
	.select = i2c_mux_gpio_select,
	.deselect = i2c_mux_gpio_deselect,
};

static const struct udevice_id i2c_mux_gpio_ids[] = {
	{ .compatible = "i2c-mux-gpio", },
	{}
};

U_BOOT_DRIVER(i2c_mux_gpio) = {
	.name = "i2c_mux_gpio",
	.id = UCLASS_I2C_MUX,
	.of_match = i2c_mux_gpio_ids,
	.ops = &i2c_mux_gpio_ops,
	.probe = i2c_mux_gpio_probe,
	.priv_auto	= sizeof(struct i2c_mux_gpio_priv),
};
