// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2012 The Chromium OS Authors.
 */

/*
 * This is a GPIO driver for Intel ICH6 and later. The x86 GPIOs are accessed
 * through the PCI bus. Each PCI device has 256 bytes of configuration space,
 * consisting of a standard header and a device-specific set of registers. PCI
 * bus 0, device 31, function 0 gives us access to the chipset GPIOs (among
 * other things). Within the PCI configuration space, the GPIOBASE register
 * tells us where in the device's I/O region we can find more registers to
 * actually access the GPIOs.
 *
 * PCI bus/device/function 0:1f:0  => PCI config registers
 *   PCI config register "GPIOBASE"
 *     PCI I/O space + [GPIOBASE]  => start of GPIO registers
 *       GPIO registers => gpio pin function, direction, value
 *
 *
 * Danger Will Robinson! Bank 0 (GPIOs 0-31) seems to be fairly stable. Most
 * ICH versions have more, but the decoding the matrix that describes them is
 * absurdly complex and constantly changing. We'll provide Bank 1 and Bank 2,
 * but they will ONLY work for certain unspecified chipsets because the offset
 * from GPIOBASE changes randomly. Even then, many GPIOs are unimplemented or
 * reserved or subject to arcane restrictions.
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <fdtdec.h>
#include <pch.h>
#include <pci.h>
#include <asm/cpu.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <asm/pci.h>

DECLARE_GLOBAL_DATA_PTR;

#define GPIO_PER_BANK	32

struct ich6_bank_priv {
	/* These are I/O addresses */
	uint16_t use_sel;
	uint16_t io_sel;
	uint16_t lvl;
	u32 lvl_write_cache;
	bool use_lvl_write_cache;
};

#define GPIO_USESEL_OFFSET(x)	(x)
#define GPIO_IOSEL_OFFSET(x)	(x + 4)
#define GPIO_LVL_OFFSET(x)	(x + 8)

static int _ich6_gpio_set_value(struct ich6_bank_priv *bank, unsigned offset,
				int value)
{
	u32 val;

	if (bank->use_lvl_write_cache)
		val = bank->lvl_write_cache;
	else
		val = inl(bank->lvl);

	if (value)
		val |= (1UL << offset);
	else
		val &= ~(1UL << offset);
	outl(val, bank->lvl);
	if (bank->use_lvl_write_cache)
		bank->lvl_write_cache = val;

	return 0;
}

static int _ich6_gpio_set_direction(uint16_t base, unsigned offset, int dir)
{
	u32 val;

	if (!dir) {
		val = inl(base);
		val |= (1UL << offset);
		outl(val, base);
	} else {
		val = inl(base);
		val &= ~(1UL << offset);
		outl(val, base);
	}

	return 0;
}

static int gpio_ich6_ofdata_to_platdata(struct udevice *dev)
{
	struct ich6_bank_platdata *plat = dev_get_platdata(dev);
	u32 gpiobase;
	int offset;
	int ret;

	ret = pch_get_gpio_base(dev->parent, &gpiobase);
	if (ret)
		return ret;

	offset = fdtdec_get_int(gd->fdt_blob, dev_of_offset(dev), "reg", -1);
	if (offset == -1) {
		debug("%s: Invalid register offset %d\n", __func__, offset);
		return -EINVAL;
	}
	plat->offset = offset;
	plat->base_addr = gpiobase + offset;
	plat->bank_name = fdt_getprop(gd->fdt_blob, dev_of_offset(dev),
				      "bank-name", NULL);

	return 0;
}

static int ich6_gpio_probe(struct udevice *dev)
{
	struct ich6_bank_platdata *plat = dev_get_platdata(dev);
	struct gpio_dev_priv *uc_priv = dev_get_uclass_priv(dev);
	struct ich6_bank_priv *bank = dev_get_priv(dev);
	const void *prop;

	uc_priv->gpio_count = GPIO_PER_BANK;
	uc_priv->bank_name = plat->bank_name;
	bank->use_sel = plat->base_addr;
	bank->io_sel = plat->base_addr + 4;
	bank->lvl = plat->base_addr + 8;

	prop = fdt_getprop(gd->fdt_blob, dev_of_offset(dev),
			   "use-lvl-write-cache", NULL);
	if (prop)
		bank->use_lvl_write_cache = true;
	else
		bank->use_lvl_write_cache = false;
	bank->lvl_write_cache = 0;

	return 0;
}

static int ich6_gpio_request(struct udevice *dev, unsigned offset,
			     const char *label)
{
	struct ich6_bank_priv *bank = dev_get_priv(dev);
	u32 tmplong;

	/*
	 * Make sure that the GPIO pin we want isn't already in use for some
	 * built-in hardware function. We have to check this for every
	 * requested pin.
	 */
	tmplong = inl(bank->use_sel);
	if (!(tmplong & (1UL << offset))) {
		debug("%s: gpio %d is reserved for internal use\n", __func__,
		      offset);
		return -EPERM;
	}

	return 0;
}

static int ich6_gpio_direction_input(struct udevice *dev, unsigned offset)
{
	struct ich6_bank_priv *bank = dev_get_priv(dev);

	return _ich6_gpio_set_direction(bank->io_sel, offset, 0);
}

static int ich6_gpio_direction_output(struct udevice *dev, unsigned offset,
				       int value)
{
	int ret;
	struct ich6_bank_priv *bank = dev_get_priv(dev);

	ret = _ich6_gpio_set_direction(bank->io_sel, offset, 1);
	if (ret)
		return ret;

	return _ich6_gpio_set_value(bank, offset, value);
}

static int ich6_gpio_get_value(struct udevice *dev, unsigned offset)
{
	struct ich6_bank_priv *bank = dev_get_priv(dev);
	u32 tmplong;
	int r;

	tmplong = inl(bank->lvl);
	if (bank->use_lvl_write_cache)
		tmplong |= bank->lvl_write_cache;
	r = (tmplong & (1UL << offset)) ? 1 : 0;
	return r;
}

static int ich6_gpio_set_value(struct udevice *dev, unsigned offset,
			       int value)
{
	struct ich6_bank_priv *bank = dev_get_priv(dev);
	return _ich6_gpio_set_value(bank, offset, value);
}

static int ich6_gpio_get_function(struct udevice *dev, unsigned offset)
{
	struct ich6_bank_priv *bank = dev_get_priv(dev);
	u32 mask = 1UL << offset;

	if (!(inl(bank->use_sel) & mask))
		return GPIOF_FUNC;
	if (inl(bank->io_sel) & mask)
		return GPIOF_INPUT;
	else
		return GPIOF_OUTPUT;
}

static const struct dm_gpio_ops gpio_ich6_ops = {
	.request		= ich6_gpio_request,
	.direction_input	= ich6_gpio_direction_input,
	.direction_output	= ich6_gpio_direction_output,
	.get_value		= ich6_gpio_get_value,
	.set_value		= ich6_gpio_set_value,
	.get_function		= ich6_gpio_get_function,
};

static const struct udevice_id intel_ich6_gpio_ids[] = {
	{ .compatible = "intel,ich6-gpio" },
	{ }
};

U_BOOT_DRIVER(gpio_ich6) = {
	.name	= "gpio_ich6",
	.id	= UCLASS_GPIO,
	.of_match = intel_ich6_gpio_ids,
	.ops	= &gpio_ich6_ops,
	.ofdata_to_platdata	= gpio_ich6_ofdata_to_platdata,
	.probe	= ich6_gpio_probe,
	.priv_auto_alloc_size = sizeof(struct ich6_bank_priv),
	.platdata_auto_alloc_size = sizeof(struct ich6_bank_platdata),
};
