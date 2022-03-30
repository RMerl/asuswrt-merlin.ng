// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2010-2016, NVIDIA CORPORATION.
 * (based on tegra_gpio.c)
 */

#include <common.h>
#include <dm.h>
#include <malloc.h>
#include <errno.h>
#include <fdtdec.h>
#include <asm/io.h>
#include <asm/bitops.h>
#include <asm/gpio.h>
#include <dm/device-internal.h>
#include <dt-bindings/gpio/gpio.h>
#include "tegra186_gpio_priv.h"

struct tegra186_gpio_port_data {
	const char *name;
	uint32_t offset;
};

struct tegra186_gpio_ctlr_data {
	const struct tegra186_gpio_port_data *ports;
	uint32_t port_count;
};

struct tegra186_gpio_platdata {
	const char *name;
	uint32_t *regs;
};

static uint32_t *tegra186_gpio_reg(struct udevice *dev, uint32_t reg,
				   uint32_t gpio)
{
	struct tegra186_gpio_platdata *plat = dev->platdata;
	uint32_t index = (reg + (gpio * TEGRA186_GPIO_PER_GPIO_STRIDE)) / 4;

	return &(plat->regs[index]);
}

static int tegra186_gpio_set_out(struct udevice *dev, unsigned offset,
				 bool output)
{
	uint32_t *reg;
	uint32_t rval;

	reg = tegra186_gpio_reg(dev, TEGRA186_GPIO_OUTPUT_CONTROL, offset);
	rval = readl(reg);
	if (output)
		rval &= ~TEGRA186_GPIO_OUTPUT_CONTROL_FLOATED;
	else
		rval |= TEGRA186_GPIO_OUTPUT_CONTROL_FLOATED;
	writel(rval, reg);

	reg = tegra186_gpio_reg(dev, TEGRA186_GPIO_ENABLE_CONFIG, offset);
	rval = readl(reg);
	if (output)
		rval |= TEGRA186_GPIO_ENABLE_CONFIG_OUT;
	else
		rval &= ~TEGRA186_GPIO_ENABLE_CONFIG_OUT;
	rval |= TEGRA186_GPIO_ENABLE_CONFIG_ENABLE;
	writel(rval, reg);

	return 0;
}

static int tegra186_gpio_set_val(struct udevice *dev, unsigned offset, bool val)
{
	uint32_t *reg;
	uint32_t rval;

	reg = tegra186_gpio_reg(dev, TEGRA186_GPIO_OUTPUT_VALUE, offset);
	rval = readl(reg);
	if (val)
		rval |= TEGRA186_GPIO_OUTPUT_VALUE_HIGH;
	else
		rval &= ~TEGRA186_GPIO_OUTPUT_VALUE_HIGH;
	writel(rval, reg);

	return 0;
}

static int tegra186_gpio_direction_input(struct udevice *dev, unsigned offset)
{
	return tegra186_gpio_set_out(dev, offset, false);
}

static int tegra186_gpio_direction_output(struct udevice *dev, unsigned offset,
				       int value)
{
	int ret;

	ret = tegra186_gpio_set_val(dev, offset, value != 0);
	if (ret)
		return ret;
	return tegra186_gpio_set_out(dev, offset, true);
}

static int tegra186_gpio_get_value(struct udevice *dev, unsigned offset)
{
	uint32_t *reg;
	uint32_t rval;

	reg = tegra186_gpio_reg(dev, TEGRA186_GPIO_ENABLE_CONFIG, offset);
	rval = readl(reg);

	if (rval & TEGRA186_GPIO_ENABLE_CONFIG_OUT)
		reg = tegra186_gpio_reg(dev, TEGRA186_GPIO_OUTPUT_VALUE,
					offset);
	else
		reg = tegra186_gpio_reg(dev, TEGRA186_GPIO_INPUT, offset);

	rval = readl(reg);
	return !!rval;
}

static int tegra186_gpio_set_value(struct udevice *dev, unsigned offset,
				   int value)
{
	return tegra186_gpio_set_val(dev, offset, value != 0);
}

static int tegra186_gpio_get_function(struct udevice *dev, unsigned offset)
{
	uint32_t *reg;
	uint32_t rval;

	reg = tegra186_gpio_reg(dev, TEGRA186_GPIO_ENABLE_CONFIG, offset);
	rval = readl(reg);
	if (rval & TEGRA186_GPIO_ENABLE_CONFIG_OUT)
		return GPIOF_OUTPUT;
	else
		return GPIOF_INPUT;
}

static int tegra186_gpio_xlate(struct udevice *dev, struct gpio_desc *desc,
			       struct ofnode_phandle_args *args)
{
	int gpio, port, ret;

	gpio = args->args[0];
	port = gpio / TEGRA186_GPIO_PER_GPIO_COUNT;
	ret = device_get_child(dev, port, &desc->dev);
	if (ret)
		return ret;
	desc->offset = gpio % TEGRA186_GPIO_PER_GPIO_COUNT;
	desc->flags = args->args[1] & GPIO_ACTIVE_LOW ? GPIOD_ACTIVE_LOW : 0;

	return 0;
}

static const struct dm_gpio_ops tegra186_gpio_ops = {
	.direction_input	= tegra186_gpio_direction_input,
	.direction_output	= tegra186_gpio_direction_output,
	.get_value		= tegra186_gpio_get_value,
	.set_value		= tegra186_gpio_set_value,
	.get_function		= tegra186_gpio_get_function,
	.xlate			= tegra186_gpio_xlate,
};

/**
 * We have a top-level GPIO device with no actual GPIOs. It has a child device
 * for each port within the controller.
 */
static int tegra186_gpio_bind(struct udevice *parent)
{
	struct tegra186_gpio_platdata *parent_plat = parent->platdata;
	struct tegra186_gpio_ctlr_data *ctlr_data =
		(struct tegra186_gpio_ctlr_data *)dev_get_driver_data(parent);
	uint32_t *regs;
	int port, ret;

	/* If this is a child device, there is nothing to do here */
	if (parent_plat)
		return 0;

	regs = (uint32_t *)devfdt_get_addr_name(parent, "gpio");
	if (regs == (uint32_t *)FDT_ADDR_T_NONE)
		return -EINVAL;

	for (port = 0; port < ctlr_data->port_count; port++) {
		struct tegra186_gpio_platdata *plat;
		struct udevice *dev;

		plat = calloc(1, sizeof(*plat));
		if (!plat)
			return -ENOMEM;
		plat->name = ctlr_data->ports[port].name;
		plat->regs = &(regs[ctlr_data->ports[port].offset / 4]);

		ret = device_bind(parent, parent->driver, plat->name, plat,
				  -1, &dev);
		if (ret)
			return ret;
		dev_set_of_offset(dev, dev_of_offset(parent));
	}

	return 0;
}

static int tegra186_gpio_probe(struct udevice *dev)
{
	struct tegra186_gpio_platdata *plat = dev->platdata;
	struct gpio_dev_priv *uc_priv = dev_get_uclass_priv(dev);

	/* Only child devices have ports */
	if (!plat)
		return 0;

	uc_priv->gpio_count = TEGRA186_GPIO_PER_GPIO_COUNT;
	uc_priv->bank_name = plat->name;

	return 0;
}

static const struct tegra186_gpio_port_data tegra186_gpio_main_ports[] = {
	{"A",  0x2000},
	{"B",  0x3000},
	{"C",  0x3200},
	{"D",  0x3400},
	{"E",  0x2200},
	{"F",  0x2400},
	{"G",  0x4200},
	{"H",  0x1000},
	{"I",  0x0800},
	{"J",  0x5000},
	{"K",  0x5200},
	{"L",  0x1200},
	{"M",  0x5600},
	{"N",  0x0000},
	{"O",  0x0200},
	{"P",  0x4000},
	{"Q",  0x0400},
	{"R",  0x0a00},
	{"T",  0x0600},
	{"X",  0x1400},
	{"Y",  0x1600},
	{"BB", 0x2600},
	{"CC", 0x5400},
};

static const struct tegra186_gpio_ctlr_data tegra186_gpio_main_data = {
	.ports = tegra186_gpio_main_ports,
	.port_count = ARRAY_SIZE(tegra186_gpio_main_ports),
};

static const struct tegra186_gpio_port_data tegra186_gpio_aon_ports[] = {
	{"S",  0x0200},
	{"U",  0x0400},
	{"V",  0x0800},
	{"W",  0x0a00},
	{"Z",  0x0e00},
	{"AA", 0x0c00},
	{"EE", 0x0600},
	{"FF", 0x0000},
};

static const struct tegra186_gpio_ctlr_data tegra186_gpio_aon_data = {
	.ports = tegra186_gpio_aon_ports,
	.port_count = ARRAY_SIZE(tegra186_gpio_aon_ports),
};

static const struct udevice_id tegra186_gpio_ids[] = {
	{
		.compatible = "nvidia,tegra186-gpio",
		.data = (ulong)&tegra186_gpio_main_data,
	},
	{
		.compatible = "nvidia,tegra186-gpio-aon",
		.data = (ulong)&tegra186_gpio_aon_data,
	},
	{ }
};

U_BOOT_DRIVER(tegra186_gpio) = {
	.name = "tegra186_gpio",
	.id = UCLASS_GPIO,
	.of_match = tegra186_gpio_ids,
	.bind = tegra186_gpio_bind,
	.probe = tegra186_gpio_probe,
	.ops = &tegra186_gpio_ops,
};
