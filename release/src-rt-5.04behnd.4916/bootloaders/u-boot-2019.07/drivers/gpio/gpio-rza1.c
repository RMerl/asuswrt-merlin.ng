// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2019 Marek Vasut <marek.vasut@gmail.com>
 */

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <errno.h>
#include <asm/gpio.h>
#include <asm/io.h>

#define P(bank)			(0x0000 + (bank) * 4)
#define PSR(bank)		(0x0100 + (bank) * 4)
#define PPR(bank)		(0x0200 + (bank) * 4)
#define PM(bank)		(0x0300 + (bank) * 4)
#define PMC(bank)		(0x0400 + (bank) * 4)
#define PFC(bank)		(0x0500 + (bank) * 4)
#define PFCE(bank)		(0x0600 + (bank) * 4)
#define PNOT(bank)		(0x0700 + (bank) * 4)
#define PMSR(bank)		(0x0800 + (bank) * 4)
#define PMCSR(bank)		(0x0900 + (bank) * 4)
#define PFCAE(bank)		(0x0A00 + (bank) * 4)
#define PIBC(bank)		(0x4000 + (bank) * 4)
#define PBDC(bank)		(0x4100 + (bank) * 4)
#define PIPC(bank)		(0x4200 + (bank) * 4)

#define RZA1_MAX_GPIO_PER_BANK	16

DECLARE_GLOBAL_DATA_PTR;

struct r7s72100_gpio_priv {
	void __iomem		*regs;
	int			bank;
};

static int r7s72100_gpio_get_value(struct udevice *dev, unsigned offset)
{
	struct r7s72100_gpio_priv *priv = dev_get_priv(dev);

	return !!(readw(priv->regs + PPR(priv->bank)) & BIT(offset));
}

static int r7s72100_gpio_set_value(struct udevice *dev, unsigned line,
			       int value)
{
	struct r7s72100_gpio_priv *priv = dev_get_priv(dev);

	writel(BIT(line + 16) | (value ? BIT(line) : 0),
	       priv->regs + PSR(priv->bank));

	return 0;
}

static void r7s72100_gpio_set_direction(struct udevice *dev, unsigned line,
					bool output)
{
	struct r7s72100_gpio_priv *priv = dev_get_priv(dev);

	writel(BIT(line + 16), priv->regs + PMCSR(priv->bank));
	writel(BIT(line + 16) | (output ? 0 : BIT(line)),
	       priv->regs + PMSR(priv->bank));

	clrsetbits_le16(priv->regs + PIBC(priv->bank), BIT(line),
			output ? 0 : BIT(line));
}

static int r7s72100_gpio_direction_input(struct udevice *dev, unsigned offset)
{
	r7s72100_gpio_set_direction(dev, offset, false);
	return 0;
}

static int r7s72100_gpio_direction_output(struct udevice *dev, unsigned offset,
				      int value)
{
	/* write GPIO value to output before selecting output mode of pin */
	r7s72100_gpio_set_value(dev, offset, value);
	r7s72100_gpio_set_direction(dev, offset, true);

	return 0;
}

static int r7s72100_gpio_get_function(struct udevice *dev, unsigned offset)
{
	struct r7s72100_gpio_priv *priv = dev_get_priv(dev);

	if (readw(priv->regs + PM(priv->bank)) & BIT(offset))
		return GPIOF_INPUT;
	else
		return GPIOF_OUTPUT;
}

static const struct dm_gpio_ops r7s72100_gpio_ops = {
	.direction_input	= r7s72100_gpio_direction_input,
	.direction_output	= r7s72100_gpio_direction_output,
	.get_value		= r7s72100_gpio_get_value,
	.set_value		= r7s72100_gpio_set_value,
	.get_function		= r7s72100_gpio_get_function,
};

static int r7s72100_gpio_probe(struct udevice *dev)
{
	struct gpio_dev_priv *uc_priv = dev_get_uclass_priv(dev);
	struct r7s72100_gpio_priv *priv = dev_get_priv(dev);
	struct fdtdec_phandle_args args;
	int node = dev_of_offset(dev);
	int ret;

	fdt_addr_t addr_base;

	uc_priv->bank_name = dev->name;
	dev = dev_get_parent(dev);
	addr_base = devfdt_get_addr(dev);
	if (addr_base == FDT_ADDR_T_NONE)
		return -EINVAL;

	priv->regs = (void __iomem *)addr_base;

	ret = fdtdec_parse_phandle_with_args(gd->fdt_blob, node, "gpio-ranges",
					     NULL, 3, 0, &args);
	priv->bank = ret == 0 ? (args.args[1] / RZA1_MAX_GPIO_PER_BANK) : -1;
	uc_priv->gpio_count = ret == 0 ? args.args[2] : RZA1_MAX_GPIO_PER_BANK;

	return 0;
}

U_BOOT_DRIVER(r7s72100_gpio) = {
	.name	= "r7s72100-gpio",
	.id	= UCLASS_GPIO,
	.ops	= &r7s72100_gpio_ops,
	.priv_auto_alloc_size = sizeof(struct r7s72100_gpio_priv),
	.probe	= r7s72100_gpio_probe,
};
