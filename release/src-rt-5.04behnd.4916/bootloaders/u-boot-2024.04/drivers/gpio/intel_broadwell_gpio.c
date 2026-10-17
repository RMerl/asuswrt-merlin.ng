// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2012 The Chromium OS Authors.
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <fdtdec.h>
#include <log.h>
#include <pch.h>
#include <pci.h>
#include <syscon.h>
#include <asm/cpu.h>
#include <asm/global_data.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <asm/pci.h>
#include <asm/arch/gpio.h>
#include <dt-bindings/gpio/x86-gpio.h>

DECLARE_GLOBAL_DATA_PTR;

/**
 * struct broadwell_bank_priv - Private driver data
 *
 * @regs:	Pointer to GPIO registers
 * @bank:	Bank number for this bank (0, 1 or 2)
 * @offset:	GPIO offset for this bank (0, 32 or 64)
 */
struct broadwell_bank_priv {
	struct pch_lp_gpio_regs *regs;
	int bank;
	int offset;
};

static int broadwell_gpio_request(struct udevice *dev, unsigned offset,
			     const char *label)
{
	struct broadwell_bank_priv *priv = dev_get_priv(dev);
	struct pch_lp_gpio_regs *regs = priv->regs;
	u32 val;

	/*
	 * Make sure that the GPIO pin we want isn't already in use for some
	 * built-in hardware function. We have to check this for every
	 * requested pin.
	 */
	debug("%s: request bank %d offset %d: ", __func__, priv->bank, offset);
	val = inl(&regs->own[priv->bank]);
	if (!(val & (1UL << offset))) {
		debug("gpio is reserved for internal use\n");
		return -EPERM;
	}
	debug("ok\n");

	return 0;
}

static int broadwell_gpio_direction_input(struct udevice *dev, unsigned offset)
{
	struct broadwell_bank_priv *priv = dev_get_priv(dev);
	struct pch_lp_gpio_regs *regs = priv->regs;

	setio_32(&regs->config[priv->offset + offset], CONFA_DIR_INPUT);

	return 0;
}

static int broadwell_gpio_get_value(struct udevice *dev, unsigned offset)
{
	struct broadwell_bank_priv *priv = dev_get_priv(dev);
	struct pch_lp_gpio_regs *regs = priv->regs;

	return inl(&regs->config[priv->offset + offset]) & CONFA_LEVEL_HIGH ?
		1 : 0;
}

static int broadwell_gpio_set_value(struct udevice *dev, unsigned offset,
				    int value)
{
	struct broadwell_bank_priv *priv = dev_get_priv(dev);
	struct pch_lp_gpio_regs *regs = priv->regs;

	debug("%s: dev=%s, offset=%d, value=%d\n", __func__, dev->name, offset,
	      value);
	clrsetio_32(&regs->config[priv->offset + offset], CONFA_OUTPUT_HIGH,
		      value ? CONFA_OUTPUT_HIGH : 0);

	return 0;
}

static int broadwell_gpio_direction_output(struct udevice *dev, unsigned offset,
					   int value)
{
	struct broadwell_bank_priv *priv = dev_get_priv(dev);
	struct pch_lp_gpio_regs *regs = priv->regs;

	broadwell_gpio_set_value(dev, offset, value);
	clrio_32(&regs->config[priv->offset + offset], CONFA_DIR_INPUT);

	return 0;
}

static int broadwell_gpio_get_function(struct udevice *dev, unsigned offset)
{
	struct broadwell_bank_priv *priv = dev_get_priv(dev);
	struct pch_lp_gpio_regs *regs = priv->regs;
	u32 mask = 1UL << offset;

	if (!(inl(&regs->own[priv->bank]) & mask))
		return GPIOF_FUNC;
	if (inl(&regs->config[priv->offset + offset]) & CONFA_DIR_INPUT)
		return GPIOF_INPUT;
	else
		return GPIOF_OUTPUT;
}

static int broadwell_gpio_probe(struct udevice *dev)
{
	struct broadwell_bank_plat *plat = dev_get_plat(dev);
	struct gpio_dev_priv *uc_priv = dev_get_uclass_priv(dev);
	struct broadwell_bank_priv *priv = dev_get_priv(dev);
	struct udevice *pinctrl;
	int ret;

	/* Set up pin control if available */
	ret = syscon_get_by_driver_data(X86_SYSCON_PINCONF, &pinctrl);
	debug("%s, pinctrl=%p, ret=%d\n", __func__, pinctrl, ret);

	uc_priv->gpio_count = GPIO_PER_BANK;
	uc_priv->bank_name = plat->bank_name;

	priv->regs = (struct pch_lp_gpio_regs *)(uintptr_t)plat->base_addr;
	priv->bank = plat->bank;
	priv->offset = priv->bank * 32;
	debug("%s: probe done, regs %p, bank %d\n", __func__, priv->regs,
	      priv->bank);

	return 0;
}

static int broadwell_gpio_of_to_plat(struct udevice *dev)
{
	struct broadwell_bank_plat *plat = dev_get_plat(dev);
	u32 gpiobase;
	int bank;
	int ret;

	ret = pch_get_gpio_base(dev->parent, &gpiobase);
	if (ret)
		return ret;

	bank = fdtdec_get_int(gd->fdt_blob, dev_of_offset(dev), "reg", -1);
	if (bank == -1) {
		debug("%s: Invalid bank number %d\n", __func__, bank);
		return -EINVAL;
	}
	plat->bank = bank;
	plat->base_addr = gpiobase;
	plat->bank_name = fdt_getprop(gd->fdt_blob, dev_of_offset(dev),
				      "bank-name", NULL);

	return 0;
}

static const struct dm_gpio_ops gpio_broadwell_ops = {
	.request		= broadwell_gpio_request,
	.direction_input	= broadwell_gpio_direction_input,
	.direction_output	= broadwell_gpio_direction_output,
	.get_value		= broadwell_gpio_get_value,
	.set_value		= broadwell_gpio_set_value,
	.get_function		= broadwell_gpio_get_function,
};

static const struct udevice_id intel_broadwell_gpio_ids[] = {
	{ .compatible = "intel,broadwell-gpio" },
	{ }
};

U_BOOT_DRIVER(gpio_broadwell) = {
	.name	= "gpio_broadwell",
	.id	= UCLASS_GPIO,
	.of_match = intel_broadwell_gpio_ids,
	.ops	= &gpio_broadwell_ops,
	.of_to_plat	= broadwell_gpio_of_to_plat,
	.probe	= broadwell_gpio_probe,
	.priv_auto	= sizeof(struct broadwell_bank_priv),
	.plat_auto	= sizeof(struct broadwell_bank_plat),
};
