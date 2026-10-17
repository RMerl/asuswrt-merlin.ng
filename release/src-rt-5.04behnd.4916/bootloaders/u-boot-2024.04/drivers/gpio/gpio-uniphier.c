// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016-2017 Socionext Inc.
 *   Author: Masahiro Yamada <yamada.masahiro@socionext.com>
 */

#include <common.h>
#include <dm.h>
#include <linux/bitops.h>
#include <linux/io.h>
#include <linux/sizes.h>
#include <linux/errno.h>
#include <asm/global_data.h>
#include <asm/gpio.h>
#include <dt-bindings/gpio/uniphier-gpio.h>

#define UNIPHIER_GPIO_PORT_DATA		0x0	/* data */
#define UNIPHIER_GPIO_PORT_DIR		0x4	/* direction (1:in, 0:out) */
#define UNIPHIER_GPIO_IRQ_EN		0x90	/* irq enable */

struct uniphier_gpio_priv {
	void __iomem *regs;
};

static unsigned int uniphier_gpio_bank_to_reg(unsigned int bank)
{
	unsigned int reg;

	reg = (bank + 1) * 8;

	/*
	 * Unfortunately, the GPIO port registers are not contiguous because
	 * offset 0x90-0x9f is used for IRQ.  Add 0x10 when crossing the region.
	 */
	if (reg >= UNIPHIER_GPIO_IRQ_EN)
		reg += 0x10;

	return reg;
}

static void uniphier_gpio_get_bank_and_mask(unsigned int offset,
					    unsigned int *bank, u32 *mask)
{
	*bank = offset / UNIPHIER_GPIO_LINES_PER_BANK;
	*mask = BIT(offset % UNIPHIER_GPIO_LINES_PER_BANK);
}

static void uniphier_gpio_reg_update(struct uniphier_gpio_priv *priv,
				     unsigned int reg, u32 mask, u32 val)
{
	u32 tmp;

	tmp = readl(priv->regs + reg);
	tmp &= ~mask;
	tmp |= mask & val;
	writel(tmp, priv->regs + reg);
}

static void uniphier_gpio_bank_write(struct udevice *dev, unsigned int bank,
				     unsigned int reg, u32 mask, u32 val)
{
	struct uniphier_gpio_priv *priv = dev_get_priv(dev);

	if (!mask)
		return;

	uniphier_gpio_reg_update(priv, uniphier_gpio_bank_to_reg(bank) + reg,
				 mask, val);
}

static void uniphier_gpio_offset_write(struct udevice *dev, unsigned int offset,
				       unsigned int reg, int val)
{
	unsigned int bank;
	u32 mask;

	uniphier_gpio_get_bank_and_mask(offset, &bank, &mask);

	uniphier_gpio_bank_write(dev, bank, reg, mask, val ? mask : 0);
}

static int uniphier_gpio_offset_read(struct udevice *dev,
				     unsigned int offset, unsigned int reg)
{
	struct uniphier_gpio_priv *priv = dev_get_priv(dev);
	unsigned int bank, reg_offset;
	u32 mask;

	uniphier_gpio_get_bank_and_mask(offset, &bank, &mask);
	reg_offset = uniphier_gpio_bank_to_reg(bank) + reg;

	return !!(readl(priv->regs + reg_offset) & mask);
}

static int uniphier_gpio_get_function(struct udevice *dev, unsigned int offset)
{
	return uniphier_gpio_offset_read(dev, offset, UNIPHIER_GPIO_PORT_DIR) ?
						GPIOF_INPUT : GPIOF_OUTPUT;
}

static int uniphier_gpio_direction_input(struct udevice *dev,
					 unsigned int offset)
{
	uniphier_gpio_offset_write(dev, offset, UNIPHIER_GPIO_PORT_DIR, 1);

	return 0;
}

static int uniphier_gpio_direction_output(struct udevice *dev,
					  unsigned int offset, int value)
{
	uniphier_gpio_offset_write(dev, offset, UNIPHIER_GPIO_PORT_DATA, value);
	uniphier_gpio_offset_write(dev, offset, UNIPHIER_GPIO_PORT_DIR, 0);

	return 0;
}

static int uniphier_gpio_get_value(struct udevice *dev, unsigned int offset)
{
	return uniphier_gpio_offset_read(dev, offset, UNIPHIER_GPIO_PORT_DATA);
}

static int uniphier_gpio_set_value(struct udevice *dev,
				   unsigned int offset, int value)
{
	uniphier_gpio_offset_write(dev, offset, UNIPHIER_GPIO_PORT_DATA, value);

	return 0;
}

static const struct dm_gpio_ops uniphier_gpio_ops = {
	.direction_input	= uniphier_gpio_direction_input,
	.direction_output	= uniphier_gpio_direction_output,
	.get_value		= uniphier_gpio_get_value,
	.set_value		= uniphier_gpio_set_value,
	.get_function		= uniphier_gpio_get_function,
};

static int uniphier_gpio_probe(struct udevice *dev)
{
	struct uniphier_gpio_priv *priv = dev_get_priv(dev);
	struct gpio_dev_priv *uc_priv = dev_get_uclass_priv(dev);
	fdt_addr_t addr;

	addr = dev_read_addr(dev);
	if (addr == FDT_ADDR_T_NONE)
		return -EINVAL;

	priv->regs = devm_ioremap(dev, addr, SZ_512);
	if (!priv->regs)
		return -ENOMEM;

	uc_priv->gpio_count = fdtdec_get_uint(gd->fdt_blob, dev_of_offset(dev),
					      "ngpios", 0);

	return 0;
}

static const struct udevice_id uniphier_gpio_match[] = {
	{ .compatible = "socionext,uniphier-gpio" },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(uniphier_gpio) = {
	.name	= "uniphier-gpio",
	.id	= UCLASS_GPIO,
	.of_match = uniphier_gpio_match,
	.probe	= uniphier_gpio_probe,
	.priv_auto	= sizeof(struct uniphier_gpio_priv),
	.ops	= &uniphier_gpio_ops,
};
