// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018 Stefan Roese <sr@denx.de>
 *
 * Based on the Linux driver version which is:
 *   Copyright (C) 2009-2011 Gabor Juhos <juhosg@openwrt.org>
 *   Copyright (C) 2013 John Crispin <blogic@openwrt.org>
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <fdtdec.h>
#include <malloc.h>
#include <linux/bitops.h>
#include <linux/io.h>
#include <asm/io.h>
#include <asm/gpio.h>
#include <dm/device-internal.h>
#include <dt-bindings/gpio/gpio.h>

#define MTK_MAX_BANK		3
#define MTK_BANK_WIDTH		32

enum mediatek_gpio_reg {
	GPIO_REG_CTRL = 0,
	GPIO_REG_POL,
	GPIO_REG_DATA,
	GPIO_REG_DSET,
	GPIO_REG_DCLR,
	GPIO_REG_REDGE,
	GPIO_REG_FEDGE,
	GPIO_REG_HLVL,
	GPIO_REG_LLVL,
	GPIO_REG_STAT,
	GPIO_REG_EDGE,
};

static void __iomem *mediatek_gpio_membase;

struct mediatek_gpio_plat {
	char bank_name[3];	/* Name of bank, e.g. "PA", "PB" etc */
	int gpio_count;
	int bank;
};

static u32 reg_offs(struct mediatek_gpio_plat *plat, int reg)
{
	return (reg * 0x10) + (plat->bank * 0x4);
}

static int mediatek_gpio_get_value(struct udevice *dev, unsigned int offset)
{
	struct mediatek_gpio_plat *plat = dev_get_plat(dev);

	return !!(ioread32(mediatek_gpio_membase +
			   reg_offs(plat, GPIO_REG_DATA)) & BIT(offset));
}

static int mediatek_gpio_set_value(struct udevice *dev, unsigned int offset,
				   int value)
{
	struct mediatek_gpio_plat *plat = dev_get_plat(dev);

	iowrite32(BIT(offset), mediatek_gpio_membase +
		  reg_offs(plat, value ? GPIO_REG_DSET : GPIO_REG_DCLR));

	return 0;
}

static int mediatek_gpio_direction_input(struct udevice *dev, unsigned int offset)
{
	struct mediatek_gpio_plat *plat = dev_get_plat(dev);

	clrbits_le32(mediatek_gpio_membase + reg_offs(plat, GPIO_REG_CTRL),
		     BIT(offset));

	return 0;
}

static int mediatek_gpio_direction_output(struct udevice *dev, unsigned int offset,
					  int value)
{
	struct mediatek_gpio_plat *plat = dev_get_plat(dev);

	setbits_le32(mediatek_gpio_membase + reg_offs(plat, GPIO_REG_CTRL),
		     BIT(offset));
	mediatek_gpio_set_value(dev, offset, value);

	return 0;
}

static int mediatek_gpio_get_function(struct udevice *dev, unsigned int offset)
{
	struct mediatek_gpio_plat *plat = dev_get_plat(dev);
	u32 t;

	t = ioread32(mediatek_gpio_membase + reg_offs(plat, GPIO_REG_CTRL));
	if (t & BIT(offset))
		return GPIOF_OUTPUT;

	return GPIOF_INPUT;
}

static const struct dm_gpio_ops gpio_mediatek_ops = {
	.direction_input	= mediatek_gpio_direction_input,
	.direction_output	= mediatek_gpio_direction_output,
	.get_value		= mediatek_gpio_get_value,
	.set_value		= mediatek_gpio_set_value,
	.get_function		= mediatek_gpio_get_function,
};

static int gpio_mediatek_probe(struct udevice *dev)
{
	struct mediatek_gpio_plat *plat = dev_get_plat(dev);
	struct gpio_dev_priv *uc_priv = dev_get_uclass_priv(dev);

	/* Tell the uclass how many GPIOs we have */
	if (plat) {
		uc_priv->gpio_count = plat->gpio_count;
		uc_priv->bank_name = plat->bank_name;
	}

	return 0;
}

/**
 * We have a top-level GPIO device with no actual GPIOs. It has a child
 * device for each Mediatek bank.
 */
static int gpio_mediatek_bind(struct udevice *parent)
{
	struct mediatek_gpio_plat *plat = dev_get_plat(parent);
	ofnode node;
	int bank = 0;
	int ret;

	/* If this is a child device, there is nothing to do here */
	if (plat)
		return 0;

	mediatek_gpio_membase = dev_remap_addr(parent);
	if (!mediatek_gpio_membase)
		return -EINVAL;

	for (node = dev_read_first_subnode(parent); ofnode_valid(node);
	     node = dev_read_next_subnode(node)) {
		struct mediatek_gpio_plat *plat;
		struct udevice *dev;

		plat = calloc(1, sizeof(*plat));
		if (!plat)
			return -ENOMEM;
		plat->bank_name[0] = 'P';
		plat->bank_name[1] = 'A' + bank;
		plat->bank_name[2] = '\0';
		plat->gpio_count = MTK_BANK_WIDTH;
		plat->bank = bank;

		ret = device_bind(parent, parent->driver, plat->bank_name, plat,
				  node, &dev);
		if (ret)
			return ret;

		bank++;
	}

	return 0;
}

static const struct udevice_id mediatek_gpio_ids[] = {
	{ .compatible = "mtk,mt7621-gpio" },
	{ }
};

U_BOOT_DRIVER(gpio_mediatek) = {
	.name	= "gpio_mediatek",
	.id	= UCLASS_GPIO,
	.ops	= &gpio_mediatek_ops,
	.of_match = mediatek_gpio_ids,
	.bind	= gpio_mediatek_bind,
	.probe	= gpio_mediatek_probe,
};
