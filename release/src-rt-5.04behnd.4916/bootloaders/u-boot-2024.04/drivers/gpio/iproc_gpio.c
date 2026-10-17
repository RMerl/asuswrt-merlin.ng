// SPDX-License-Identifier:      GPL-2.0+
/*
 * Copyright (C) 2020 Broadcom
 */

#include <common.h>
#include <errno.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <dm/devres.h>
#include <dm/pinctrl.h>

/*
 * There are five GPIO bank register. Each bank can configure max of 32 gpios.
 * BANK0 - gpios 0 to 31
 * BANK1 - gpios 32 to 63
 * BANK2 - gpios 64 to 95
 * BANK3 - gpios 96 to 127
 * BANK4 - gpios 128 to 150
 *
 * Offset difference between consecutive bank register is 0x200
 */
#define NGPIO_PER_BANK		32
#define GPIO_BANK_SIZE		0x200
#define GPIO_BANK(pin)		((pin) / NGPIO_PER_BANK)
#define GPIO_SHIFT(pin)		((pin) % NGPIO_PER_BANK)
#define GPIO_REG(pin, reg)	(GPIO_BANK_SIZE * GPIO_BANK(pin) + (reg))

/* device register offset */
#define DATA_IN_OFFSET   0x00
#define DATA_OUT_OFFSET  0x04
#define OUT_EN_OFFSET    0x08

/**
 * struct iproc_gpio_pctrl_map - gpio and pinctrl mapping
 * @gpio_pin:	start of gpio number in gpio-ranges
 * @pctrl_pin:	start of pinctrl number in gpio-ranges
 * @npins:	total number of pins in gpio-ranges
 * @node:	list node
 */
struct iproc_gpio_pctrl_map {
	u32 gpio_pin;
	u32 pctrl_pin;
	u32 npins;
	struct list_head node;
};

/**
 * struct iproc_gpio_pctrl_map - gpio device instance
 * @pinctrl_dev:pointer to pinctrl device
 * @gpiomap:	list node having mapping between gpio and pinctrl
 * @base:	I/O register base address of gpio device
 * @name:	gpio device name, ex GPIO0, GPIO1
 * @ngpios:	total number of gpios
 */
struct iproc_gpio_plat {
	struct udevice *pinctrl_dev;
	struct list_head gpiomap;
	void __iomem *base;
	char *name;
	u32 ngpios;
};

/**
 * iproc_gpio_set_bit - set or clear one bit in an iproc GPIO register.
 *
 * The bit relates to a GPIO pin.
 *
 * @plat: iproc GPIO device
 * @reg: register offset
 * @gpio: GPIO pin
 * @set: set or clear
 */
static inline void iproc_gpio_set_bit(struct iproc_gpio_plat *plat,
				      u32 reg, u32 gpio, bool set)
{
	u32 offset = GPIO_REG(gpio, reg);
	u32 shift = GPIO_SHIFT(gpio);

	clrsetbits_le32(plat->base + offset, BIT(shift),
			(set ? BIT(shift) : 0));
}

static inline bool iproc_gpio_get_bit(struct iproc_gpio_plat *plat,
				      u32 reg, u32 gpio)
{
	u32 offset = GPIO_REG(gpio, reg);
	u32 shift = GPIO_SHIFT(gpio);

	return readl(plat->base + offset) & BIT(shift);
}

/**
 * iproc_get_gpio_pctrl_mapping() - get associated pinctrl pin from gpio pin
 *
 * @plat: iproc GPIO device
 * @gpio: GPIO pin
 */
static u32 iproc_get_pctrl_from_gpio(struct iproc_gpio_plat *plat, u32 gpio)
{
	struct iproc_gpio_pctrl_map *range = NULL;
	struct list_head *pos, *tmp;
	u32 ret = 0;

	list_for_each_safe(pos, tmp, &plat->gpiomap) {
		range = list_entry(pos, struct iproc_gpio_pctrl_map, node);
		if (gpio == range->gpio_pin ||
		    gpio < (range->gpio_pin + range->npins)) {
			ret = range->pctrl_pin + (gpio - range->gpio_pin);
			break;
		}
	}

	return ret;
}

/**
 * iproc_get_gpio_pctrl_mapping() - get mapping between gpio and pinctrl
 *
 * Read dt node "gpio-ranges" to get gpio and pinctrl mapping and store
 * in private data structure to use it later while enabling gpio.
 *
 * @dev: pointer to GPIO device
 * Return: 0 on success and -ENOMEM on failure
 */
static int iproc_get_gpio_pctrl_mapping(struct udevice *dev)
{
	struct iproc_gpio_plat *plat = dev_get_plat(dev);
	struct iproc_gpio_pctrl_map *range = NULL;
	struct ofnode_phandle_args args;
	int index = 0, ret;

	for (;; index++) {
		ret = dev_read_phandle_with_args(dev, "gpio-ranges",
						 NULL, 3, index, &args);
		if (ret)
			break;

		range = devm_kzalloc(dev, sizeof(*range), GFP_KERNEL);
		if (!range)
			return -ENOMEM;

		range->gpio_pin = args.args[0];
		range->pctrl_pin = args.args[1];
		range->npins = args.args[2];
		list_add_tail(&range->node, &plat->gpiomap);
	}

	return 0;
}

static int iproc_gpio_request(struct udevice *dev, u32 gpio, const char *label)
{
	struct iproc_gpio_plat *plat = dev_get_plat(dev);
	u32 pctrl;

	/* nothing to do if there is no corresponding pinctrl device */
	if (!plat->pinctrl_dev)
		return 0;

	pctrl = iproc_get_pctrl_from_gpio(plat, gpio);

	return pinctrl_request(plat->pinctrl_dev, pctrl, 0);
}

static int iproc_gpio_direction_input(struct udevice *dev, u32 gpio)
{
	struct iproc_gpio_plat *plat = dev_get_plat(dev);

	iproc_gpio_set_bit(plat, OUT_EN_OFFSET, gpio, false);
	dev_dbg(dev, "gpio:%u set input\n", gpio);

	return 0;
}

static int iproc_gpio_direction_output(struct udevice *dev, u32 gpio, int value)
{
	struct iproc_gpio_plat *plat = dev_get_plat(dev);

	iproc_gpio_set_bit(plat, OUT_EN_OFFSET, gpio, true);
	iproc_gpio_set_bit(plat, DATA_OUT_OFFSET, gpio, value);
	dev_dbg(dev, "gpio:%u set output, value:%d\n", gpio, value);

	return 0;
}

static int iproc_gpio_get_value(struct udevice *dev, u32 gpio)
{
	struct iproc_gpio_plat *plat = dev_get_plat(dev);
	int value;

	value = iproc_gpio_get_bit(plat, DATA_IN_OFFSET, gpio);
	dev_dbg(dev, "gpio:%u get, value:%d\n", gpio, value);

	return value;
}

static int iproc_gpio_set_value(struct udevice *dev, u32 gpio, int value)
{
	struct iproc_gpio_plat *plat = dev_get_plat(dev);

	if (iproc_gpio_get_bit(plat, OUT_EN_OFFSET, gpio))
		iproc_gpio_set_bit(plat, DATA_OUT_OFFSET, gpio, value);

	dev_dbg(dev, "gpio:%u set, value:%d\n", gpio, value);
	return 0;
}

static int iproc_gpio_get_function(struct udevice *dev, u32 gpio)
{
	struct iproc_gpio_plat *plat = dev_get_plat(dev);

	if (iproc_gpio_get_bit(plat, OUT_EN_OFFSET, gpio))
		return GPIOF_OUTPUT;
	else
		return GPIOF_INPUT;
}

static int iproc_gpio_of_to_plat(struct udevice *dev)
{
	struct iproc_gpio_plat *plat = dev_get_plat(dev);
	struct gpio_dev_priv *uc_priv = dev_get_uclass_priv(dev);
	int ret;
	char name[10];

	plat->base = dev_read_addr_ptr(dev);
	if (!plat->base) {
		debug("%s: Failed to get base address\n", __func__);
		return -EINVAL;
	}

	ret = dev_read_u32(dev, "ngpios", &plat->ngpios);
	if (ret < 0) {
		dev_err(dev, "%s: Failed to get ngpios\n", __func__);
		return ret;
	}

	uclass_get_device_by_phandle(UCLASS_PINCTRL, dev, "gpio-ranges",
				     &plat->pinctrl_dev);
	if (ret < 0) {
		dev_err(dev, "%s: Failed to get pinctrl phandle\n", __func__);
		return ret;
	}

	INIT_LIST_HEAD(&plat->gpiomap);
	ret = iproc_get_gpio_pctrl_mapping(dev);
	if (ret < 0) {
		dev_err(dev, "%s: Failed to get gpio to pctrl map ret(%d)\n",
			__func__, ret);
		return ret;
	}

	snprintf(name, sizeof(name), "GPIO%d", dev_seq(dev));
	plat->name = strdup(name);
	if (!plat->name)
		return -ENOMEM;

	uc_priv->gpio_count = plat->ngpios;
	uc_priv->bank_name = plat->name;

	dev_info(dev, ":bank name(%s) base %p, #gpios %d\n",
		 plat->name, plat->base, plat->ngpios);

	return 0;
}

static const struct dm_gpio_ops iproc_gpio_ops = {
	.request		= iproc_gpio_request,
	.direction_input	= iproc_gpio_direction_input,
	.direction_output	= iproc_gpio_direction_output,
	.get_value		= iproc_gpio_get_value,
	.set_value		= iproc_gpio_set_value,
	.get_function		= iproc_gpio_get_function,
};

static const struct udevice_id iproc_gpio_ids[] = {
	{ .compatible = "brcm,iproc-gpio" },
	{ }
};

U_BOOT_DRIVER(iproc_gpio) = {
	.name			= "iproc_gpio",
	.id			= UCLASS_GPIO,
	.of_match		= iproc_gpio_ids,
	.ops			= &iproc_gpio_ops,
	.of_to_plat	= iproc_gpio_of_to_plat,
	.plat_auto	= sizeof(struct iproc_gpio_plat),
};
