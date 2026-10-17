// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2021 Mark Kettenis <kettenis@openbsd.org>
 */

#include <common.h>
#include <dm.h>
#include <dm/device-internal.h>
#include <dm/pinctrl.h>
#include <dt-bindings/pinctrl/apple.h>
#include <asm/io.h>
#include <asm-generic/gpio.h>
#include <linux/bitfield.h>

struct apple_pinctrl_priv {
	void *base;
	int pin_count;
};

#define REG_GPIO(x)	(4 * (x))
#define  REG_GPIO_DATA		BIT(0)
#define  REG_GPIO_MODE		GENMASK(3, 1)
#define  REG_GPIO_OUT		1
#define  REG_GPIO_PERIPH	GENMASK(6, 5)
#define  REG_GPIO_INPUT_ENABLE	BIT(9)

static void apple_pinctrl_config_pin(struct apple_pinctrl_priv *priv,
				     unsigned pin, u32 clr, u32 set)
{
	unsigned reg = REG_GPIO(pin);
	u32 old, new;

	old = readl(priv->base + REG_GPIO(pin));
	new = (old & ~clr) | set;
	writel(new, priv->base + reg);
}

static int apple_gpio_get_value(struct udevice *dev, unsigned offset)
{
	struct apple_pinctrl_priv *priv = dev_get_priv(dev->parent);

	return !!(readl(priv->base + REG_GPIO(offset)) & REG_GPIO_DATA);
}

static int apple_gpio_set_value(struct udevice *dev, unsigned offset,
				int value)
{
	struct apple_pinctrl_priv *priv = dev_get_priv(dev->parent);

	apple_pinctrl_config_pin(priv, offset, REG_GPIO_DATA,
				 value ? REG_GPIO_DATA : 0);
	return 0;
}

static int apple_gpio_get_direction(struct udevice *dev, unsigned offset)
{
	struct apple_pinctrl_priv *priv = dev_get_priv(dev->parent);
	u32 reg = readl(priv->base + REG_GPIO(offset));

	if (FIELD_GET(REG_GPIO_MODE, reg) == REG_GPIO_OUT)
		return GPIOF_OUTPUT;
	else
		return GPIOF_INPUT;
}

static int apple_gpio_direction_input(struct udevice *dev, unsigned offset)
{
	struct apple_pinctrl_priv *priv = dev_get_priv(dev->parent);

	apple_pinctrl_config_pin(priv, offset,
				 REG_GPIO_PERIPH | REG_GPIO_MODE,
				 REG_GPIO_INPUT_ENABLE);
	return 0;
}

static int apple_gpio_direction_output(struct udevice *dev, unsigned offset,
				       int value)
{
	struct apple_pinctrl_priv *priv = dev_get_priv(dev->parent);
	u32 set = (value ? REG_GPIO_DATA : 0);

	apple_pinctrl_config_pin(priv, offset, REG_GPIO_DATA |
				 REG_GPIO_PERIPH | REG_GPIO_MODE,
				 set | FIELD_PREP(REG_GPIO_MODE, REG_GPIO_OUT));
	return 0;
}

static int apple_gpio_probe(struct udevice *dev)
{
	struct apple_pinctrl_priv *priv = dev_get_priv(dev->parent);
	struct gpio_dev_priv *uc_priv;

	uc_priv = dev_get_uclass_priv(dev);
	uc_priv->bank_name = "gpio";
	uc_priv->gpio_count = priv->pin_count;

	return 0;
}

static struct dm_gpio_ops apple_gpio_ops = {
	.get_value = apple_gpio_get_value,
	.set_value = apple_gpio_set_value,
	.get_function = apple_gpio_get_direction,
	.direction_input = apple_gpio_direction_input,
	.direction_output = apple_gpio_direction_output,
};

static struct driver apple_gpio_driver = {
	.name = "apple_gpio",
	.id = UCLASS_GPIO,
	.probe = apple_gpio_probe,
	.ops = &apple_gpio_ops,
};

static int apple_pinctrl_get_pins_count(struct udevice *dev)
{
	struct apple_pinctrl_priv *priv = dev_get_priv(dev);

	return priv->pin_count;
}

static const char *apple_pinctrl_get_pin_name(struct udevice *dev,
					      unsigned selector)
{
	static char pin_name[PINNAME_SIZE];

	snprintf(pin_name, PINNAME_SIZE, "pin%d", selector);
	return pin_name;
}

static int apple_pinctrl_get_pin_muxing(struct udevice *dev, unsigned selector,
					char *buf, int size)
{
	struct apple_pinctrl_priv *priv = dev_get_priv(dev);

	if (readl(priv->base + REG_GPIO(selector)) & REG_GPIO_PERIPH)
		strncpy(buf, "periph", size);
	else
		strncpy(buf, "gpio", size);
	return 0;
}

static int apple_pinctrl_pinmux_set(struct udevice *dev, unsigned pin_selector,
				    unsigned func_selector)
{
	struct apple_pinctrl_priv *priv = dev_get_priv(dev);

	apple_pinctrl_config_pin(priv, pin_selector,
				 REG_GPIO_DATA | REG_GPIO_MODE,
				 FIELD_PREP(REG_GPIO_PERIPH, func_selector) |
				 REG_GPIO_INPUT_ENABLE);
	return 0;
}

static int apple_pinctrl_pinmux_property_set(struct udevice *dev,
					     u32 pinmux_group)
{
	unsigned pin_selector = APPLE_PIN(pinmux_group);
	unsigned func_selector = APPLE_FUNC(pinmux_group);
	int ret;

	ret = apple_pinctrl_pinmux_set(dev, pin_selector, func_selector);
	return ret ? ret : pin_selector;
}

static int apple_pinctrl_probe(struct udevice *dev)
{
	struct apple_pinctrl_priv *priv = dev_get_priv(dev);
	struct ofnode_phandle_args args;
	struct udevice *child;

	priv->base = dev_read_addr_ptr(dev);
	if (!priv->base)
		return -EINVAL;

	if (!dev_read_phandle_with_args(dev, "gpio-ranges",
					NULL, 3, 0, &args))
		priv->pin_count = args.args[2];

	device_bind(dev, &apple_gpio_driver, "apple_gpio", NULL,
		    dev_ofnode(dev), &child);

	return 0;
}

static struct pinctrl_ops apple_pinctrl_ops = {
	.set_state = pinctrl_generic_set_state,
	.get_pins_count = apple_pinctrl_get_pins_count,
	.get_pin_name = apple_pinctrl_get_pin_name,
	.pinmux_set = apple_pinctrl_pinmux_set,
	.pinmux_property_set = apple_pinctrl_pinmux_property_set,
	.get_pin_muxing = apple_pinctrl_get_pin_muxing,
};

static const struct udevice_id apple_pinctrl_ids[] = {
	{ .compatible = "apple,pinctrl" },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(pinctrl_apple) = {
	.name = "apple_pinctrl",
	.id = UCLASS_PINCTRL,
	.of_match = apple_pinctrl_ids,
	.priv_auto = sizeof(struct apple_pinctrl_priv),
	.ops = &apple_pinctrl_ops,
	.probe = apple_pinctrl_probe,
};
