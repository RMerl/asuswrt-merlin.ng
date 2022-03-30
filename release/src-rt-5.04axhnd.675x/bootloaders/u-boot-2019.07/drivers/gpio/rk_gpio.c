// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2015 Google, Inc
 *
 * (C) Copyright 2008-2014 Rockchip Electronics
 * Peter, Software Engineering, <superpeter.cai@gmail.com>.
 */

#include <common.h>
#include <dm.h>
#include <syscon.h>
#include <linux/errno.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <asm/arch-rockchip/clock.h>
#include <asm/arch-rockchip/gpio.h>
#include <dm/pinctrl.h>
#include <dt-bindings/clock/rk3288-cru.h>

enum {
	ROCKCHIP_GPIOS_PER_BANK		= 32,
};

#define OFFSET_TO_BIT(bit)	(1UL << (bit))

struct rockchip_gpio_priv {
	struct rockchip_gpio_regs *regs;
	struct udevice *pinctrl;
	int bank;
	char name[2];
};

static int rockchip_gpio_direction_input(struct udevice *dev, unsigned offset)
{
	struct rockchip_gpio_priv *priv = dev_get_priv(dev);
	struct rockchip_gpio_regs *regs = priv->regs;

	clrbits_le32(&regs->swport_ddr, OFFSET_TO_BIT(offset));

	return 0;
}

static int rockchip_gpio_direction_output(struct udevice *dev, unsigned offset,
					  int value)
{
	struct rockchip_gpio_priv *priv = dev_get_priv(dev);
	struct rockchip_gpio_regs *regs = priv->regs;
	int mask = OFFSET_TO_BIT(offset);

	clrsetbits_le32(&regs->swport_dr, mask, value ? mask : 0);
	setbits_le32(&regs->swport_ddr, mask);

	return 0;
}

static int rockchip_gpio_get_value(struct udevice *dev, unsigned offset)
{
	struct rockchip_gpio_priv *priv = dev_get_priv(dev);
	struct rockchip_gpio_regs *regs = priv->regs;

	return readl(&regs->ext_port) & OFFSET_TO_BIT(offset) ? 1 : 0;
}

static int rockchip_gpio_set_value(struct udevice *dev, unsigned offset,
				   int value)
{
	struct rockchip_gpio_priv *priv = dev_get_priv(dev);
	struct rockchip_gpio_regs *regs = priv->regs;
	int mask = OFFSET_TO_BIT(offset);

	clrsetbits_le32(&regs->swport_dr, mask, value ? mask : 0);

	return 0;
}

static int rockchip_gpio_get_function(struct udevice *dev, unsigned offset)
{
#ifdef CONFIG_SPL_BUILD
	return -ENODATA;
#else
	struct rockchip_gpio_priv *priv = dev_get_priv(dev);
	struct rockchip_gpio_regs *regs = priv->regs;
	bool is_output;
	int ret;

	ret = pinctrl_get_gpio_mux(priv->pinctrl, priv->bank, offset);
	if (ret)
		return ret;
	is_output = readl(&regs->swport_ddr) & OFFSET_TO_BIT(offset);

	return is_output ? GPIOF_OUTPUT : GPIOF_INPUT;
#endif
}

/* Simple SPL interface to GPIOs */
#ifdef CONFIG_SPL_BUILD

enum {
	PULL_NONE_1V8 = 0,
	PULL_DOWN_1V8 = 1,
	PULL_UP_1V8 = 3,
};

int spl_gpio_set_pull(void *vregs, uint gpio, int pull)
{
	u32 *regs = vregs;
	uint val;

	regs += gpio >> GPIO_BANK_SHIFT;
	gpio &= GPIO_OFFSET_MASK;
	switch (pull) {
	case GPIO_PULL_UP:
		val = PULL_UP_1V8;
		break;
	case GPIO_PULL_DOWN:
		val = PULL_DOWN_1V8;
		break;
	case GPIO_PULL_NORMAL:
	default:
		val = PULL_NONE_1V8;
		break;
	}
	clrsetbits_le32(regs, 3 << (gpio * 2), val << (gpio * 2));

	return 0;
}

int spl_gpio_output(void *vregs, uint gpio, int value)
{
	struct rockchip_gpio_regs * const regs = vregs;

	clrsetbits_le32(&regs->swport_dr, 1 << gpio, value << gpio);

	/* Set direction */
	clrsetbits_le32(&regs->swport_ddr, 1 << gpio, 1 << gpio);

	return 0;
}
#endif /* CONFIG_SPL_BUILD */

static int rockchip_gpio_probe(struct udevice *dev)
{
	struct gpio_dev_priv *uc_priv = dev_get_uclass_priv(dev);
	struct rockchip_gpio_priv *priv = dev_get_priv(dev);
	char *end;
	int ret;

	priv->regs = dev_read_addr_ptr(dev);
	ret = uclass_first_device_err(UCLASS_PINCTRL, &priv->pinctrl);
	if (ret)
		return ret;

	uc_priv->gpio_count = ROCKCHIP_GPIOS_PER_BANK;
	end = strrchr(dev->name, '@');
	priv->bank = trailing_strtoln(dev->name, end);
	priv->name[0] = 'A' + priv->bank;
	uc_priv->bank_name = priv->name;

	return 0;
}

static const struct dm_gpio_ops gpio_rockchip_ops = {
	.direction_input	= rockchip_gpio_direction_input,
	.direction_output	= rockchip_gpio_direction_output,
	.get_value		= rockchip_gpio_get_value,
	.set_value		= rockchip_gpio_set_value,
	.get_function		= rockchip_gpio_get_function,
};

static const struct udevice_id rockchip_gpio_ids[] = {
	{ .compatible = "rockchip,gpio-bank" },
	{ }
};

U_BOOT_DRIVER(gpio_rockchip) = {
	.name	= "gpio_rockchip",
	.id	= UCLASS_GPIO,
	.of_match = rockchip_gpio_ids,
	.ops	= &gpio_rockchip_ops,
	.priv_auto_alloc_size = sizeof(struct rockchip_gpio_priv),
	.probe	= rockchip_gpio_probe,
};
