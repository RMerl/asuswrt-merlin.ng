// SPDX-License-Identifier:    GPL-2.0
/*
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * (C) Copyright 2011
 * eInfochips Ltd. <www.einfochips.com>
 * Written-by: Ajay Bhargav <ajay.bhargav@einfochips.com>
 *
 * (C) Copyright 2010
 * Marvell Semiconductor <www.marvell.com>
 */

#include <dm.h>
#include <pci.h>
#include <pci_ids.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <linux/bitfield.h>
#include <linux/compat.h>
#include <dt-bindings/gpio/gpio.h>

/* Returns the bit value to write or read based on the offset */
#define GPIO_BIT(x)		BIT_ULL((x) & 0x3f)

#define GPIO_RX_DAT		0x00
#define GPIO_TX_SET		0x08
#define GPIO_TX_CLR		0x10
#define GPIO_CONST		0x90	/* OcteonTX only */

/* Offset to register-set for 2nd GPIOs (> 63), OcteonTX only */
#define GPIO1_OFFSET		0x1400

/* GPIO_CONST register bits */
#define GPIO_CONST_GPIOS_MASK	GENMASK_ULL(7, 0)

/* GPIO_BIT_CFG register bits */
#define GPIO_BIT_CFG_TX_OE	BIT_ULL(0)
#define GPIO_BIT_CFG_PIN_XOR	BIT_ULL(1)
#define GPIO_BIT_CFG_INT_EN	BIT_ULL(2)
#define GPIO_BIT_CFG_PIN_SEL_MASK GENMASK_ULL(26, 16)

enum {
	PROBE_PCI = 0,		/* PCI based probing */
	PROBE_DT,		/* DT based probing */
};

struct octeon_gpio_data {
	int probe;
	u32 reg_offs;
	u32 gpio_bit_cfg_offs;
};

struct octeon_gpio {
	void __iomem *base;
	const struct octeon_gpio_data *data;
};

/* Returns the offset to the output register based on the offset and value */
static u32 gpio_tx_reg(int offset, int value)
{
	u32 ret;

	ret = value ? GPIO_TX_SET : GPIO_TX_CLR;
	if (offset > 63)
		ret += GPIO1_OFFSET;

	return ret;
}

/* Returns the offset to the input data register based on the offset */
static u32 gpio_rx_dat_reg(int offset)
{
	u32 ret;

	ret = GPIO_RX_DAT;
	if (offset > 63)
		ret += GPIO1_OFFSET;

	return ret;
}

static int octeon_gpio_dir_input(struct udevice *dev, unsigned int offset)
{
	struct octeon_gpio *gpio = dev_get_priv(dev);

	debug("%s(%s, %u)\n", __func__, dev->name, offset);
	clrbits_64(gpio->base + gpio->data->gpio_bit_cfg_offs + 8 * offset,
		   GPIO_BIT_CFG_TX_OE | GPIO_BIT_CFG_PIN_XOR |
		   GPIO_BIT_CFG_INT_EN | GPIO_BIT_CFG_PIN_SEL_MASK);

	return 0;
}

static int octeon_gpio_dir_output(struct udevice *dev, unsigned int offset,
				  int value)
{
	struct octeon_gpio *gpio = dev_get_priv(dev);

	debug("%s(%s, %u, %d)\n", __func__, dev->name, offset, value);
	writeq(GPIO_BIT(offset), gpio->base + gpio->data->reg_offs +
	       gpio_tx_reg(offset, value));

	clrsetbits_64(gpio->base + gpio->data->gpio_bit_cfg_offs + 8 * offset,
		      GPIO_BIT_CFG_PIN_SEL_MASK | GPIO_BIT_CFG_INT_EN,
		      GPIO_BIT_CFG_TX_OE);

	return 0;
}

static int octeon_gpio_get_value(struct udevice *dev, unsigned int offset)
{
	struct octeon_gpio *gpio = dev_get_priv(dev);
	u64 reg = readq(gpio->base + gpio->data->reg_offs +
			gpio_rx_dat_reg(offset));

	debug("%s(%s, %u): value: %d\n", __func__, dev->name, offset,
	      !!(reg & GPIO_BIT(offset)));

	return !!(reg & GPIO_BIT(offset));
}

static int octeon_gpio_set_value(struct udevice *dev,
				 unsigned int offset, int value)
{
	struct octeon_gpio *gpio = dev_get_priv(dev);

	debug("%s(%s, %u, %d)\n", __func__, dev->name, offset, value);
	writeq(GPIO_BIT(offset), gpio->base + gpio->data->reg_offs +
	       gpio_tx_reg(offset, value));

	return 0;
}

static int octeon_gpio_get_function(struct udevice *dev, unsigned int offset)
{
	struct octeon_gpio *gpio = dev_get_priv(dev);
	u64 val = readq(gpio->base + gpio->data->gpio_bit_cfg_offs +
			8 * offset);
	int pin_sel;

	debug("%s(%s, %u): GPIO_BIT_CFG: 0x%llx\n", __func__, dev->name,
	      offset, val);
	pin_sel = FIELD_GET(GPIO_BIT_CFG_PIN_SEL_MASK, val);
	if (pin_sel)
		return GPIOF_FUNC;
	else if (val & GPIO_BIT_CFG_TX_OE)
		return GPIOF_OUTPUT;
	else
		return GPIOF_INPUT;
}

static int octeon_gpio_xlate(struct udevice *dev, struct gpio_desc *desc,
			     struct ofnode_phandle_args *args)
{
	if (args->args_count < 1)
		return -EINVAL;

	desc->offset = args->args[0];
	desc->flags = 0;
	if (args->args_count > 1) {
		if (args->args[1] & GPIO_ACTIVE_LOW)
			desc->flags |= GPIOD_ACTIVE_LOW;
		/* In the future add tri-state flag support */
	}
	return 0;
}

static const struct dm_gpio_ops octeon_gpio_ops = {
	.direction_input	= octeon_gpio_dir_input,
	.direction_output	= octeon_gpio_dir_output,
	.get_value		= octeon_gpio_get_value,
	.set_value		= octeon_gpio_set_value,
	.get_function		= octeon_gpio_get_function,
	.xlate			= octeon_gpio_xlate,
};

static int octeon_gpio_probe(struct udevice *dev)
{
	struct gpio_dev_priv *uc_priv = dev_get_uclass_priv(dev);
	struct octeon_gpio *priv = dev_get_priv(dev);
	char *end;

	priv->data = (const struct octeon_gpio_data *)dev_get_driver_data(dev);

	if (priv->data->probe == PROBE_PCI) {
		priv->base = dm_pci_map_bar(dev, PCI_BASE_ADDRESS_0, 0, 0, PCI_REGION_TYPE,
					    PCI_REGION_MEM);
		uc_priv->gpio_count = readq(priv->base +
					    priv->data->reg_offs + GPIO_CONST) &
			GPIO_CONST_GPIOS_MASK;
	} else {
		priv->base = dev_remap_addr(dev);
		uc_priv->gpio_count = ofnode_read_u32_default(dev_ofnode(dev),
							      "nr-gpios", 32);
	}

	if (!priv->base) {
		debug("%s(%s): Could not get base address\n",
		      __func__, dev->name);
		return -ENODEV;
	}

	uc_priv->bank_name  = strdup(dev->name);
	end = strchr(uc_priv->bank_name, '@');
	end[0] = 'A' + dev_seq(dev);
	end[1] = '\0';

	debug("%s(%s): base address: %p, pin count: %d\n",
	      __func__, dev->name, priv->base, uc_priv->gpio_count);

	return 0;
}

static const struct octeon_gpio_data gpio_octeon_data = {
	.probe = PROBE_DT,
	.reg_offs = 0x80,
	.gpio_bit_cfg_offs = 0x100,
};

static const struct octeon_gpio_data gpio_octeontx_data = {
	.probe = PROBE_PCI,
	.reg_offs = 0x00,
	.gpio_bit_cfg_offs = 0x400,
};

static const struct udevice_id octeon_gpio_ids[] = {
	{ .compatible = "cavium,thunder-8890-gpio",
	  .data = (ulong)&gpio_octeontx_data },
	{ .compatible = "cavium,octeon-7890-gpio",
	  .data = (ulong)&gpio_octeon_data },
	{ }
};

U_BOOT_DRIVER(octeon_gpio) = {
	.name	= "octeon_gpio",
	.id	= UCLASS_GPIO,
	.of_match = of_match_ptr(octeon_gpio_ids),
	.probe = octeon_gpio_probe,
	.priv_auto	= sizeof(struct octeon_gpio),
	.ops	= &octeon_gpio_ops,
	.flags	= DM_FLAG_PRE_RELOC,
};
