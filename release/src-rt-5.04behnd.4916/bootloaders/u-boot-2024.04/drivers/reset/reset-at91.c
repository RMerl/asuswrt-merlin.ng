// SPDX-License-Identifier: GPL-2.0+
/*
 * Support for Atmel/Microchip Reset Controller.
 *
 * Copyright (C) 2022 Microchip Technology Inc. and its subsidiaries
 *
 * Author: Sergiu Moga <sergiu.moga@microchip.com>
 */

#include <clk.h>
#include <asm/io.h>
#include <dm.h>
#include <dm/lists.h>
#include <reset-uclass.h>
#include <asm/arch/at91_rstc.h>
#include <dt-bindings/reset/sama7g5-reset.h>

struct at91_reset {
	void __iomem *dev_base;
	struct at91_reset_data *data;
};

struct at91_reset_data {
	u32 n_device_reset;
	u8 device_reset_min_id;
	u8 device_reset_max_id;
};

static const struct at91_reset_data sama7g5_data = {
	.n_device_reset = 3,
	.device_reset_min_id = SAMA7G5_RESET_USB_PHY1,
	.device_reset_max_id = SAMA7G5_RESET_USB_PHY3,
};

static int at91_rst_update(struct at91_reset *reset, unsigned long id,
			   bool assert)
{
	u32 val;

	if (!reset->dev_base)
		return 0;

	val = readl(reset->dev_base);
	if (assert)
		val |= BIT(id);
	else
		val &= ~BIT(id);
	writel(val, reset->dev_base);

	return 0;
}

static int at91_reset_of_xlate(struct reset_ctl *reset_ctl,
			       struct ofnode_phandle_args *args)
{
	struct at91_reset *reset = dev_get_priv(reset_ctl->dev);

	if (!reset->data->n_device_reset ||
	    args->args[0] < reset->data->device_reset_min_id ||
	    args->args[0] > reset->data->device_reset_max_id)
		return -EINVAL;

	reset_ctl->id = args->args[0];

	return 0;
}

static int at91_rst_assert(struct reset_ctl *reset_ctl)
{
	struct at91_reset *reset = dev_get_priv(reset_ctl->dev);

	return at91_rst_update(reset, reset_ctl->id, true);
}

static int at91_rst_deassert(struct reset_ctl *reset_ctl)
{
	struct at91_reset *reset = dev_get_priv(reset_ctl->dev);

	return at91_rst_update(reset, reset_ctl->id, false);
}

struct reset_ops at91_reset_ops = {
	.of_xlate = at91_reset_of_xlate,
	.rst_assert = at91_rst_assert,
	.rst_deassert = at91_rst_deassert,
};

static int at91_reset_probe(struct udevice *dev)
{
	struct at91_reset *reset = dev_get_priv(dev);
	struct clk sclk;
	int ret;

	reset->data = (struct at91_reset_data *)dev_get_driver_data(dev);
	reset->dev_base = dev_remap_addr_index(dev, 1);
	if (reset->data && reset->data->n_device_reset && !reset->dev_base)
		return -EINVAL;

	ret = clk_get_by_index(dev, 0, &sclk);
	if (ret)
		return ret;

	return clk_prepare_enable(&sclk);
}

static int at91_reset_bind(struct udevice *dev)
{
	struct udevice *at91_sysreset;

	if (CONFIG_IS_ENABLED(SYSRESET_AT91))
		return device_bind_driver_to_node(dev, "at91_sysreset",
						  "at91_sysreset",
						  dev_ofnode(dev),
						  &at91_sysreset);

	return 0;
}

static const struct udevice_id at91_reset_ids[] = {
	{
		.compatible = "microchip,sama7g5-rstc",
		.data = (ulong)&sama7g5_data,
	},
	{
		.compatible = "atmel,sama5d3-rstc",
	},
	{
		.compatible = "microchip,sam9x60-rstc",
	},
	{ }
};

U_BOOT_DRIVER(at91_reset) = {
	.name = "at91_reset",
	.id = UCLASS_RESET,
	.of_match = at91_reset_ids,
	.bind = at91_reset_bind,
	.probe = at91_reset_probe,
	.priv_auto = sizeof(struct at91_reset),
	.ops = &at91_reset_ops,
};
