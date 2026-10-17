// SPDX-License-Identifier: GPL-2.0+
/*
 * Faraday Technology's FTGPIO010 controller.
 */

#include <common.h>
#include <dm.h>
#include <asm/io.h>
#include <asm/gpio.h>

struct ftgpio010_regs {
	u32 out;
	u32 in;
	u32 direction;	// 1 - output
	u32 reserved;
	u32 set;
	u32 clear;
};

struct ftgpio010_plat {
	struct ftgpio010_regs __iomem *regs;
};

static int ftgpio010_direction_input(struct udevice *dev, unsigned int pin)
{
	struct ftgpio010_plat *plat = dev_get_plat(dev);
	struct ftgpio010_regs *const regs = plat->regs;

	clrbits_le32(&regs->direction, 1 << pin);
	return 0;
}

static int ftgpio010_direction_output(struct udevice *dev, unsigned int pin,
				      int val)
{
	struct ftgpio010_plat *plat = dev_get_plat(dev);
	struct ftgpio010_regs *const regs = plat->regs;

	/* change the data first, then the direction. to avoid glitch */
	out_le32(val ? &regs->set : &regs->clear, 1 << pin);
	setbits_le32(&regs->direction, 1 << pin);

	return 0;
}

static int ftgpio010_get_value(struct udevice *dev, unsigned int pin)
{
	struct ftgpio010_plat *plat = dev_get_plat(dev);
	struct ftgpio010_regs *const regs = plat->regs;

	return in_le32(&regs->in) >> pin & 1;
}

static int ftgpio010_set_value(struct udevice *dev, unsigned int pin, int val)
{
	struct ftgpio010_plat *plat = dev_get_plat(dev);
	struct ftgpio010_regs *const regs = plat->regs;

	out_le32(val ? &regs->set : &regs->clear, 1 << pin);
	return 0;
}

static int ftgpio010_get_function(struct udevice *dev, unsigned int pin)
{
	struct ftgpio010_plat *plat = dev_get_plat(dev);
	struct ftgpio010_regs *const regs = plat->regs;

	if (in_le32(&regs->direction) >> pin & 1)
		return GPIOF_OUTPUT;
	return GPIOF_INPUT;
}

static int ftgpio010_probe(struct udevice *dev)
{
	struct gpio_dev_priv *uc_priv = dev_get_uclass_priv(dev);

	uc_priv->gpio_count = ofnode_read_u32_default(dev_ofnode(dev),
						      "nr-gpios", 32);
	return 0;
}

static int ftgpio010_of_to_plat(struct udevice *dev)
{
	struct ftgpio010_plat *plat = dev_get_plat(dev);

	plat->regs = dev_read_addr_ptr(dev);
	return 0;
}

static const struct dm_gpio_ops ftgpio010_ops = {
	.direction_input	= ftgpio010_direction_input,
	.direction_output	= ftgpio010_direction_output,
	.get_value		= ftgpio010_get_value,
	.set_value		= ftgpio010_set_value,
	.get_function		= ftgpio010_get_function,
};

static const struct udevice_id ftgpio010_ids[] = {
	{ .compatible = "faraday,ftgpio010" },
	{ }
};

U_BOOT_DRIVER(ftgpio010) = {
	.name		= "ftgpio010",
	.id		= UCLASS_GPIO,
	.of_match	= ftgpio010_ids,
	.ops		= &ftgpio010_ops,
	.of_to_plat	= ftgpio010_of_to_plat,
	.plat_auto	= sizeof(struct ftgpio010_plat),
	.probe		= ftgpio010_probe,
};
