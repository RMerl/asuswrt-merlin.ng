// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2009 Samsung Electronics
 * Minkyu Kang <mk7.kang@samsung.com>
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <fdtdec.h>
#include <malloc.h>
#include <asm/io.h>
#include <asm/gpio.h>
#include <dm/device-internal.h>

DECLARE_GLOBAL_DATA_PTR;

#define S5P_GPIO_GET_PIN(x)	(x % GPIO_PER_BANK)

#define CON_MASK(val)			(0xf << ((val) << 2))
#define CON_SFR(gpio, cfg)		((cfg) << ((gpio) << 2))
#define CON_SFR_UNSHIFT(val, gpio)	((val) >> ((gpio) << 2))

#define DAT_MASK(gpio)			(0x1 << (gpio))
#define DAT_SET(gpio)			(0x1 << (gpio))

#define PULL_MASK(gpio)		(0x3 << ((gpio) << 1))
#define PULL_MODE(gpio, pull)		((pull) << ((gpio) << 1))

#define DRV_MASK(gpio)			(0x3 << ((gpio) << 1))
#define DRV_SET(gpio, mode)		((mode) << ((gpio) << 1))
#define RATE_MASK(gpio)		(0x1 << (gpio + 16))
#define RATE_SET(gpio)			(0x1 << (gpio + 16))

/* Platform data for each bank */
struct exynos_gpio_platdata {
	struct s5p_gpio_bank *bank;
	const char *bank_name;	/* Name of port, e.g. 'gpa0" */
};

/* Information about each bank at run-time */
struct exynos_bank_info {
	struct s5p_gpio_bank *bank;
};

static struct s5p_gpio_bank *s5p_gpio_get_bank(unsigned int gpio)
{
	const struct gpio_info *data;
	unsigned int upto;
	int i, count;

	data = get_gpio_data();
	count = get_bank_num();
	upto = 0;

	for (i = 0; i < count; i++) {
		debug("i=%d, upto=%d\n", i, upto);
		if (gpio < data->max_gpio) {
			struct s5p_gpio_bank *bank;
			bank = (struct s5p_gpio_bank *)data->reg_addr;
			bank += (gpio - upto) / GPIO_PER_BANK;
			debug("gpio=%d, bank=%p\n", gpio, bank);
			return bank;
		}

		upto = data->max_gpio;
		data++;
	}

	return NULL;
}

static void s5p_gpio_cfg_pin(struct s5p_gpio_bank *bank, int gpio, int cfg)
{
	unsigned int value;

	value = readl(&bank->con);
	value &= ~CON_MASK(gpio);
	value |= CON_SFR(gpio, cfg);
	writel(value, &bank->con);
}

static void s5p_gpio_set_value(struct s5p_gpio_bank *bank, int gpio, int en)
{
	unsigned int value;

	value = readl(&bank->dat);
	value &= ~DAT_MASK(gpio);
	if (en)
		value |= DAT_SET(gpio);
	writel(value, &bank->dat);
}

#ifdef CONFIG_SPL_BUILD
/* Common GPIO API - SPL does not support driver model yet */
int gpio_set_value(unsigned gpio, int value)
{
	s5p_gpio_set_value(s5p_gpio_get_bank(gpio),
			   s5p_gpio_get_pin(gpio), value);

	return 0;
}
#else
static int s5p_gpio_get_cfg_pin(struct s5p_gpio_bank *bank, int gpio)
{
	unsigned int value;

	value = readl(&bank->con);
	value &= CON_MASK(gpio);
	return CON_SFR_UNSHIFT(value, gpio);
}

static unsigned int s5p_gpio_get_value(struct s5p_gpio_bank *bank, int gpio)
{
	unsigned int value;

	value = readl(&bank->dat);
	return !!(value & DAT_MASK(gpio));
}
#endif /* CONFIG_SPL_BUILD */

static void s5p_gpio_set_pull(struct s5p_gpio_bank *bank, int gpio, int mode)
{
	unsigned int value;

	value = readl(&bank->pull);
	value &= ~PULL_MASK(gpio);

	switch (mode) {
	case S5P_GPIO_PULL_DOWN:
	case S5P_GPIO_PULL_UP:
		value |= PULL_MODE(gpio, mode);
		break;
	default:
		break;
	}

	writel(value, &bank->pull);
}

static void s5p_gpio_set_drv(struct s5p_gpio_bank *bank, int gpio, int mode)
{
	unsigned int value;

	value = readl(&bank->drv);
	value &= ~DRV_MASK(gpio);

	switch (mode) {
	case S5P_GPIO_DRV_1X:
	case S5P_GPIO_DRV_2X:
	case S5P_GPIO_DRV_3X:
	case S5P_GPIO_DRV_4X:
		value |= DRV_SET(gpio, mode);
		break;
	default:
		return;
	}

	writel(value, &bank->drv);
}

static void s5p_gpio_set_rate(struct s5p_gpio_bank *bank, int gpio, int mode)
{
	unsigned int value;

	value = readl(&bank->drv);
	value &= ~RATE_MASK(gpio);

	switch (mode) {
	case S5P_GPIO_DRV_FAST:
	case S5P_GPIO_DRV_SLOW:
		value |= RATE_SET(gpio);
		break;
	default:
		return;
	}

	writel(value, &bank->drv);
}

int s5p_gpio_get_pin(unsigned gpio)
{
	return S5P_GPIO_GET_PIN(gpio);
}

/* Driver model interface */
#ifndef CONFIG_SPL_BUILD
/* set GPIO pin 'gpio' as an input */
static int exynos_gpio_direction_input(struct udevice *dev, unsigned offset)
{
	struct exynos_bank_info *state = dev_get_priv(dev);

	/* Configure GPIO direction as input. */
	s5p_gpio_cfg_pin(state->bank, offset, S5P_GPIO_INPUT);

	return 0;
}

/* set GPIO pin 'gpio' as an output, with polarity 'value' */
static int exynos_gpio_direction_output(struct udevice *dev, unsigned offset,
				       int value)
{
	struct exynos_bank_info *state = dev_get_priv(dev);

	/* Configure GPIO output value. */
	s5p_gpio_set_value(state->bank, offset, value);

	/* Configure GPIO direction as output. */
	s5p_gpio_cfg_pin(state->bank, offset, S5P_GPIO_OUTPUT);

	return 0;
}

/* read GPIO IN value of pin 'gpio' */
static int exynos_gpio_get_value(struct udevice *dev, unsigned offset)
{
	struct exynos_bank_info *state = dev_get_priv(dev);

	return s5p_gpio_get_value(state->bank, offset);
}

/* write GPIO OUT value to pin 'gpio' */
static int exynos_gpio_set_value(struct udevice *dev, unsigned offset,
				 int value)
{
	struct exynos_bank_info *state = dev_get_priv(dev);

	s5p_gpio_set_value(state->bank, offset, value);

	return 0;
}
#endif /* nCONFIG_SPL_BUILD */

/*
 * There is no common GPIO API for pull, drv, pin, rate (yet). These
 * functions are kept here to preserve function ordering for review.
 */
void gpio_set_pull(int gpio, int mode)
{
	s5p_gpio_set_pull(s5p_gpio_get_bank(gpio),
			  s5p_gpio_get_pin(gpio), mode);
}

void gpio_set_drv(int gpio, int mode)
{
	s5p_gpio_set_drv(s5p_gpio_get_bank(gpio),
			 s5p_gpio_get_pin(gpio), mode);
}

void gpio_cfg_pin(int gpio, int cfg)
{
	s5p_gpio_cfg_pin(s5p_gpio_get_bank(gpio),
			 s5p_gpio_get_pin(gpio), cfg);
}

void gpio_set_rate(int gpio, int mode)
{
	s5p_gpio_set_rate(s5p_gpio_get_bank(gpio),
			  s5p_gpio_get_pin(gpio), mode);
}

#ifndef CONFIG_SPL_BUILD
static int exynos_gpio_get_function(struct udevice *dev, unsigned offset)
{
	struct exynos_bank_info *state = dev_get_priv(dev);
	int cfg;

	cfg = s5p_gpio_get_cfg_pin(state->bank, offset);
	if (cfg == S5P_GPIO_OUTPUT)
		return GPIOF_OUTPUT;
	else if (cfg == S5P_GPIO_INPUT)
		return GPIOF_INPUT;
	else
		return GPIOF_FUNC;
}

static const struct dm_gpio_ops gpio_exynos_ops = {
	.direction_input	= exynos_gpio_direction_input,
	.direction_output	= exynos_gpio_direction_output,
	.get_value		= exynos_gpio_get_value,
	.set_value		= exynos_gpio_set_value,
	.get_function		= exynos_gpio_get_function,
};

static int gpio_exynos_probe(struct udevice *dev)
{
	struct gpio_dev_priv *uc_priv = dev_get_uclass_priv(dev);
	struct exynos_bank_info *priv = dev->priv;
	struct exynos_gpio_platdata *plat = dev->platdata;

	/* Only child devices have ports */
	if (!plat)
		return 0;

	priv->bank = plat->bank;

	uc_priv->gpio_count = GPIO_PER_BANK;
	uc_priv->bank_name = plat->bank_name;

	return 0;
}

/**
 * We have a top-level GPIO device with no actual GPIOs. It has a child
 * device for each Exynos GPIO bank.
 */
static int gpio_exynos_bind(struct udevice *parent)
{
	struct exynos_gpio_platdata *plat = parent->platdata;
	struct s5p_gpio_bank *bank, *base;
	const void *blob = gd->fdt_blob;
	int node;

	/* If this is a child device, there is nothing to do here */
	if (plat)
		return 0;

	base = (struct s5p_gpio_bank *)devfdt_get_addr(parent);
	for (node = fdt_first_subnode(blob, dev_of_offset(parent)), bank = base;
	     node > 0;
	     node = fdt_next_subnode(blob, node), bank++) {
		struct exynos_gpio_platdata *plat;
		struct udevice *dev;
		fdt_addr_t reg;
		int ret;

		if (!fdtdec_get_bool(blob, node, "gpio-controller"))
			continue;
		plat = calloc(1, sizeof(*plat));
		if (!plat)
			return -ENOMEM;

		plat->bank_name = fdt_get_name(blob, node, NULL);
		ret = device_bind(parent, parent->driver,
				  plat->bank_name, plat, -1, &dev);
		if (ret)
			return ret;

		dev_set_of_offset(dev, node);

		reg = devfdt_get_addr(dev);
		if (reg != FDT_ADDR_T_NONE)
			bank = (struct s5p_gpio_bank *)((ulong)base + reg);

		plat->bank = bank;

		debug("dev at %p: %s\n", bank, plat->bank_name);
	}

	return 0;
}

static const struct udevice_id exynos_gpio_ids[] = {
	{ .compatible = "samsung,s5pc100-pinctrl" },
	{ .compatible = "samsung,s5pc110-pinctrl" },
	{ .compatible = "samsung,exynos4210-pinctrl" },
	{ .compatible = "samsung,exynos4x12-pinctrl" },
	{ .compatible = "samsung,exynos5250-pinctrl" },
	{ .compatible = "samsung,exynos5420-pinctrl" },
	{ }
};

U_BOOT_DRIVER(gpio_exynos) = {
	.name	= "gpio_exynos",
	.id	= UCLASS_GPIO,
	.of_match = exynos_gpio_ids,
	.bind	= gpio_exynos_bind,
	.probe = gpio_exynos_probe,
	.priv_auto_alloc_size = sizeof(struct exynos_bank_info),
	.ops	= &gpio_exynos_ops,
};
#endif
