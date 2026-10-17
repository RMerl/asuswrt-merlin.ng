// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017-2020 STMicroelectronics - All Rights Reserved
 */

#define LOG_CATEGORY UCLASS_PINCTRL

#include <common.h>
#include <dm.h>
#include <hwspinlock.h>
#include <log.h>
#include <malloc.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <dm/device_compat.h>
#include <dm/lists.h>
#include <dm/pinctrl.h>
#include <linux/bitops.h>
#include <linux/err.h>
#include <linux/libfdt.h>
#include <linux/printk.h>

#include "../gpio/stm32_gpio_priv.h"

#define MAX_PINS_ONE_IP			70
#define MODE_BITS_MASK			3
#define OSPEED_MASK			3
#define PUPD_MASK			3
#define OTYPE_MSK			1
#define AFR_MASK			0xF

struct stm32_pinctrl_priv {
	struct hwspinlock hws;
	int pinctrl_ngpios;
	struct list_head gpio_dev;
};

struct stm32_gpio_bank {
	struct udevice *gpio_dev;
	struct list_head list;
};

#ifndef CONFIG_SPL_BUILD

static char pin_name[PINNAME_SIZE];
static const char * const pinmux_mode[GPIOF_COUNT] = {
	[GPIOF_INPUT] = "gpio input",
	[GPIOF_OUTPUT] = "gpio output",
	[GPIOF_UNUSED] = "analog",
	[GPIOF_UNKNOWN] = "unknown",
	[GPIOF_FUNC] = "alt function",
};

static const char * const pinmux_bias[] = {
	[STM32_GPIO_PUPD_NO] = "",
	[STM32_GPIO_PUPD_UP] = "pull-up",
	[STM32_GPIO_PUPD_DOWN] = "pull-down",
};

static const char * const pinmux_otype[] = {
	[STM32_GPIO_OTYPE_PP] = "push-pull",
	[STM32_GPIO_OTYPE_OD] = "open-drain",
};

static const char * const pinmux_speed[] = {
	[STM32_GPIO_SPEED_2M] = "Low speed",
	[STM32_GPIO_SPEED_25M] = "Medium speed",
	[STM32_GPIO_SPEED_50M] = "High speed",
	[STM32_GPIO_SPEED_100M] = "Very-high speed",
};

static int stm32_pinctrl_get_af(struct udevice *dev, unsigned int offset)
{
	struct stm32_gpio_priv *priv = dev_get_priv(dev);
	struct stm32_gpio_regs *regs = priv->regs;
	u32 af;
	u32 alt_shift = (offset % 8) * 4;
	u32 alt_index =  offset / 8;

	af = (readl(&regs->afr[alt_index]) &
	      GENMASK(alt_shift + 3, alt_shift)) >> alt_shift;

	return af;
}

static int stm32_populate_gpio_dev_list(struct udevice *dev)
{
	struct stm32_pinctrl_priv *priv = dev_get_priv(dev);
	struct udevice *gpio_dev;
	struct udevice *child;
	struct stm32_gpio_bank *gpio_bank;
	int ret;

	/*
	 * parse pin-controller sub-nodes (ie gpio bank nodes) and fill
	 * a list with all gpio device reference which belongs to the
	 * current pin-controller. This list is used to find pin_name and
	 * pin muxing
	 */
	list_for_each_entry(child, &dev->child_head, sibling_node) {
		ret = uclass_get_device_by_name(UCLASS_GPIO, child->name,
						&gpio_dev);
		if (ret < 0)
			continue;

		gpio_bank = malloc(sizeof(*gpio_bank));
		if (!gpio_bank) {
			dev_err(dev, "Not enough memory\n");
			return -ENOMEM;
		}

		gpio_bank->gpio_dev = gpio_dev;
		list_add_tail(&gpio_bank->list, &priv->gpio_dev);
	}

	return 0;
}

static int stm32_pinctrl_get_pins_count(struct udevice *dev)
{
	struct stm32_pinctrl_priv *priv = dev_get_priv(dev);
	struct gpio_dev_priv *uc_priv;
	struct stm32_gpio_bank *gpio_bank;

	/*
	 * if get_pins_count has already been executed once on this
	 * pin-controller, no need to run it again
	 */
	if (priv->pinctrl_ngpios)
		return priv->pinctrl_ngpios;

	if (list_empty(&priv->gpio_dev))
		stm32_populate_gpio_dev_list(dev);
	/*
	 * walk through all banks to retrieve the pin-controller
	 * pins number
	 */
	list_for_each_entry(gpio_bank, &priv->gpio_dev, list) {
		uc_priv = dev_get_uclass_priv(gpio_bank->gpio_dev);

		priv->pinctrl_ngpios += uc_priv->gpio_count;
	}

	return priv->pinctrl_ngpios;
}

static struct udevice *stm32_pinctrl_get_gpio_dev(struct udevice *dev,
						  unsigned int selector,
						  unsigned int *idx)
{
	struct stm32_pinctrl_priv *priv = dev_get_priv(dev);
	struct stm32_gpio_bank *gpio_bank;
	struct gpio_dev_priv *uc_priv;
	int pin_count = 0;

	if (list_empty(&priv->gpio_dev))
		stm32_populate_gpio_dev_list(dev);

	/* look up for the bank which owns the requested pin */
	list_for_each_entry(gpio_bank, &priv->gpio_dev, list) {
		uc_priv = dev_get_uclass_priv(gpio_bank->gpio_dev);

		if (selector < (pin_count + uc_priv->gpio_count)) {
			/*
			 * we found the bank, convert pin selector to
			 * gpio bank index
			 */
			*idx = selector - pin_count;

			return gpio_bank->gpio_dev;
		}
		pin_count += uc_priv->gpio_count;
	}

	return NULL;
}

static const char *stm32_pinctrl_get_pin_name(struct udevice *dev,
					      unsigned int selector)
{
	struct gpio_dev_priv *uc_priv;
	struct udevice *gpio_dev;
	unsigned int gpio_idx;

	/* look up for the bank which owns the requested pin */
	gpio_dev = stm32_pinctrl_get_gpio_dev(dev, selector, &gpio_idx);
	if (!gpio_dev) {
		snprintf(pin_name, PINNAME_SIZE, "Error");
	} else {
		uc_priv = dev_get_uclass_priv(gpio_dev);

		snprintf(pin_name, PINNAME_SIZE, "%s%d",
			 uc_priv->bank_name,
			 gpio_idx);
	}

	return pin_name;
}

static int stm32_pinctrl_get_pin_muxing(struct udevice *dev,
					unsigned int selector,
					char *buf,
					int size)
{
	struct udevice *gpio_dev;
	struct stm32_gpio_priv *priv;
	const char *label;
	int mode;
	int af_num;
	unsigned int gpio_idx;
	u32 pupd, otype;
	u8 speed;

	/* look up for the bank which owns the requested pin */
	gpio_dev = stm32_pinctrl_get_gpio_dev(dev, selector, &gpio_idx);

	if (!gpio_dev)
		return -ENODEV;

	mode = gpio_get_raw_function(gpio_dev, gpio_idx, &label);
	dev_dbg(dev, "selector = %d gpio_idx = %d mode = %d\n",
		selector, gpio_idx, mode);
	priv = dev_get_priv(gpio_dev);
	pupd = (readl(&priv->regs->pupdr) >> (gpio_idx * 2)) & PUPD_MASK;
	otype = (readl(&priv->regs->otyper) >> gpio_idx) & OTYPE_MSK;
	speed = (readl(&priv->regs->ospeedr) >> gpio_idx * 2) & OSPEED_MASK;

	switch (mode) {
	case GPIOF_UNKNOWN:
	case GPIOF_UNUSED:
		snprintf(buf, size, "%s", pinmux_mode[mode]);
		break;
	case GPIOF_FUNC:
		af_num = stm32_pinctrl_get_af(gpio_dev, gpio_idx);
		snprintf(buf, size, "%s %d %s %s %s", pinmux_mode[mode], af_num,
			 pinmux_otype[otype], pinmux_bias[pupd],
			 pinmux_speed[speed]);
		break;
	case GPIOF_OUTPUT:
		snprintf(buf, size, "%s %s %s %s %s",
			 pinmux_mode[mode], pinmux_otype[otype],
			 pinmux_bias[pupd], label ? label : "",
			 pinmux_speed[speed]);
		break;
	case GPIOF_INPUT:
		snprintf(buf, size, "%s %s %s", pinmux_mode[mode],
			 pinmux_bias[pupd], label ? label : "");
		break;
	}

	return 0;
}

#endif

static int stm32_pinctrl_probe(struct udevice *dev)
{
	struct stm32_pinctrl_priv *priv = dev_get_priv(dev);
	int ret;

	INIT_LIST_HEAD(&priv->gpio_dev);

	/* hwspinlock property is optional, just log the error */
	ret = hwspinlock_get_by_index(dev, 0, &priv->hws);
	if (ret)
		dev_dbg(dev, "hwspinlock_get_by_index may have failed (%d)\n",
			ret);

	return 0;
}

static int stm32_gpio_config(ofnode node,
			     struct gpio_desc *desc,
			     const struct stm32_gpio_ctl *ctl)
{
	struct stm32_gpio_priv *priv = dev_get_priv(desc->dev);
	struct gpio_dev_priv *uc_priv = dev_get_uclass_priv(desc->dev);
	struct stm32_gpio_regs *regs = priv->regs;
	struct stm32_pinctrl_priv *ctrl_priv;
	int ret;
	u32 index;

	if (!ctl || ctl->af > 15 || ctl->mode > 3 || ctl->otype > 1 ||
	    ctl->pupd > 2 || ctl->speed > 3)
		return -EINVAL;

	ctrl_priv = dev_get_priv(dev_get_parent(desc->dev));
	ret = hwspinlock_lock_timeout(&ctrl_priv->hws, 10);
	if (ret == -ETIME) {
		dev_err(desc->dev, "HWSpinlock timeout\n");
		return ret;
	}

	index = (desc->offset & 0x07) * 4;
	clrsetbits_le32(&regs->afr[desc->offset >> 3], AFR_MASK << index,
			ctl->af << index);

	index = desc->offset * 2;
	clrsetbits_le32(&regs->moder, MODE_BITS_MASK << index,
			ctl->mode << index);
	clrsetbits_le32(&regs->ospeedr, OSPEED_MASK << index,
			ctl->speed << index);
	clrsetbits_le32(&regs->pupdr, PUPD_MASK << index, ctl->pupd << index);

	index = desc->offset;
	clrsetbits_le32(&regs->otyper, OTYPE_MSK << index, ctl->otype << index);

	uc_priv->name[desc->offset] = strdup(ofnode_get_name(node));

	hwspinlock_unlock(&ctrl_priv->hws);

	return 0;
}

static int prep_gpio_dsc(struct stm32_gpio_dsc *gpio_dsc, u32 port_pin)
{
	gpio_dsc->port = (port_pin & 0x1F000) >> 12;
	gpio_dsc->pin = (port_pin & 0x0F00) >> 8;
	log_debug("GPIO:port= %d, pin= %d\n", gpio_dsc->port, gpio_dsc->pin);

	return 0;
}

static int prep_gpio_ctl(struct stm32_gpio_ctl *gpio_ctl, u32 gpio_fn,
			 ofnode node)
{
	gpio_fn &= 0x00FF;
	gpio_ctl->af = 0;

	switch (gpio_fn) {
	case 0:
		gpio_ctl->mode = STM32_GPIO_MODE_IN;
		break;
	case 1 ... 16:
		gpio_ctl->mode = STM32_GPIO_MODE_AF;
		gpio_ctl->af = gpio_fn - 1;
		break;
	case 17:
		gpio_ctl->mode = STM32_GPIO_MODE_AN;
		break;
	default:
		gpio_ctl->mode = STM32_GPIO_MODE_OUT;
		break;
	}

	gpio_ctl->speed = ofnode_read_u32_default(node, "slew-rate", 0);

	if (ofnode_read_bool(node, "drive-open-drain"))
		gpio_ctl->otype = STM32_GPIO_OTYPE_OD;
	else
		gpio_ctl->otype = STM32_GPIO_OTYPE_PP;

	if (ofnode_read_bool(node, "bias-pull-up"))
		gpio_ctl->pupd = STM32_GPIO_PUPD_UP;
	else if (ofnode_read_bool(node, "bias-pull-down"))
		gpio_ctl->pupd = STM32_GPIO_PUPD_DOWN;
	else
		gpio_ctl->pupd = STM32_GPIO_PUPD_NO;

	log_debug("gpio fn= %d, slew-rate= %x, op type= %x, pull-upd is = %x\n",
		  gpio_fn, gpio_ctl->speed, gpio_ctl->otype,
		  gpio_ctl->pupd);

	return 0;
}

static int stm32_pinctrl_config(ofnode node)
{
	u32 pin_mux[MAX_PINS_ONE_IP];
	int rv, len;
	ofnode subnode;

	/*
	 * check for "pinmux" property in each subnode (e.g. pins1 and pins2 for
	 * usart1) of pin controller phandle "pinctrl-0"
	 * */
	ofnode_for_each_subnode(subnode, node) {
		struct stm32_gpio_dsc gpio_dsc;
		struct stm32_gpio_ctl gpio_ctl;
		int i;

		rv = ofnode_read_size(subnode, "pinmux");
		if (rv < 0)
			return rv;
		len = rv / sizeof(pin_mux[0]);
		log_debug("No of pinmux entries= %d\n", len);
		if (len > MAX_PINS_ONE_IP)
			return -EINVAL;
		rv = ofnode_read_u32_array(subnode, "pinmux", pin_mux, len);
		if (rv < 0)
			return rv;
		for (i = 0; i < len; i++) {
			struct gpio_desc desc;

			log_debug("pinmux = %x\n", *(pin_mux + i));
			prep_gpio_dsc(&gpio_dsc, *(pin_mux + i));
			prep_gpio_ctl(&gpio_ctl, *(pin_mux + i), subnode);
			rv = uclass_get_device_by_seq(UCLASS_GPIO,
						      gpio_dsc.port,
						      &desc.dev);
			if (rv)
				return rv;
			desc.offset = gpio_dsc.pin;
			rv = stm32_gpio_config(node, &desc, &gpio_ctl);
			log_debug("rv = %d\n\n", rv);
			if (rv)
				return rv;
		}
	}

	return 0;
}

static int stm32_pinctrl_bind(struct udevice *dev)
{
	ofnode node;
	const char *name;
	int ret;

	dev_for_each_subnode(node, dev) {
		dev_dbg(dev, "bind %s\n", ofnode_get_name(node));

		if (!ofnode_is_enabled(node))
			continue;

		ofnode_get_property(node, "gpio-controller", &ret);
		if (ret < 0)
			continue;
		/* Get the name of each gpio node */
		name = ofnode_get_name(node);
		if (!name)
			return -EINVAL;

		/* Bind each gpio node */
		ret = device_bind_driver_to_node(dev, "gpio_stm32",
						 name, node, NULL);
		if (ret)
			return ret;

		dev_dbg(dev, "bind %s\n", name);
	}

	return 0;
}

#if CONFIG_IS_ENABLED(PINCTRL_FULL)
static int stm32_pinctrl_set_state(struct udevice *dev, struct udevice *config)
{
	return stm32_pinctrl_config(dev_ofnode(config));
}
#else /* PINCTRL_FULL */
static int stm32_pinctrl_set_state_simple(struct udevice *dev,
					  struct udevice *periph)
{
	const fdt32_t *list;
	uint32_t phandle;
	ofnode config_node;
	int size, i, ret;

	list = ofnode_get_property(dev_ofnode(periph), "pinctrl-0", &size);
	if (!list)
		return -EINVAL;

	dev_dbg(dev, "periph->name = %s\n", periph->name);

	size /= sizeof(*list);
	for (i = 0; i < size; i++) {
		phandle = fdt32_to_cpu(*list++);

		config_node = ofnode_get_by_phandle(phandle);
		if (!ofnode_valid(config_node)) {
			dev_err(periph,
				"prop pinctrl-0 index %d invalid phandle\n", i);
			return -EINVAL;
		}

		ret = stm32_pinctrl_config(config_node);
		if (ret)
			return ret;
	}

	return 0;
}
#endif /* PINCTRL_FULL */

static struct pinctrl_ops stm32_pinctrl_ops = {
#if CONFIG_IS_ENABLED(PINCTRL_FULL)
	.set_state		= stm32_pinctrl_set_state,
#else /* PINCTRL_FULL */
	.set_state_simple	= stm32_pinctrl_set_state_simple,
#endif /* PINCTRL_FULL */
#ifndef CONFIG_SPL_BUILD
	.get_pin_name		= stm32_pinctrl_get_pin_name,
	.get_pins_count		= stm32_pinctrl_get_pins_count,
	.get_pin_muxing		= stm32_pinctrl_get_pin_muxing,
#endif
};

static const struct udevice_id stm32_pinctrl_ids[] = {
	{ .compatible = "st,stm32f429-pinctrl" },
	{ .compatible = "st,stm32f469-pinctrl" },
	{ .compatible = "st,stm32f746-pinctrl" },
	{ .compatible = "st,stm32f769-pinctrl" },
	{ .compatible = "st,stm32h743-pinctrl" },
	{ .compatible = "st,stm32mp157-pinctrl" },
	{ .compatible = "st,stm32mp157-z-pinctrl" },
	{ .compatible = "st,stm32mp135-pinctrl" },
	{ .compatible = "st,stm32mp257-pinctrl" },
	{ .compatible = "st,stm32mp257-z-pinctrl" },
	{ }
};

U_BOOT_DRIVER(pinctrl_stm32) = {
	.name			= "pinctrl_stm32",
	.id			= UCLASS_PINCTRL,
	.of_match		= stm32_pinctrl_ids,
	.ops			= &stm32_pinctrl_ops,
	.bind			= stm32_pinctrl_bind,
	.probe			= stm32_pinctrl_probe,
	.priv_auto	= sizeof(struct stm32_pinctrl_priv),
};
