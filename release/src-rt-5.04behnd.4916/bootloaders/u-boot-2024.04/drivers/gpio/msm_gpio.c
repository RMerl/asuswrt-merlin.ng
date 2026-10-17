// SPDX-License-Identifier: GPL-2.0+
/*
 * Qualcomm GPIO driver
 *
 * (C) Copyright 2015 Mateusz Kulikowski <mateusz.kulikowski@gmail.com>
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <asm/global_data.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <mach/gpio.h>

DECLARE_GLOBAL_DATA_PTR;

/* OE */
#define GPIO_OE_DISABLE  (0x0 << 9)
#define GPIO_OE_ENABLE   (0x1 << 9)
#define GPIO_OE_MASK     (0x1 << 9)

/* GPIO_IN_OUT register shifts. */
#define GPIO_IN          0
#define GPIO_OUT         1

struct msm_gpio_bank {
	phys_addr_t base;
	const struct msm_pin_data *pin_data;
};

#define GPIO_CONFIG_REG(dev, x) \
	(qcom_pin_offset(((struct msm_gpio_bank *)dev_get_priv(dev))->pin_data->pin_offsets, x))

#define GPIO_IN_OUT_REG(dev, x) \
	(GPIO_CONFIG_REG(dev, x) + 0x4)

static int msm_gpio_direction_input(struct udevice *dev, unsigned int gpio)
{
	struct msm_gpio_bank *priv = dev_get_priv(dev);

	/* Disable OE bit */
	clrsetbits_le32(priv->base + GPIO_CONFIG_REG(dev, gpio),
			GPIO_OE_MASK, GPIO_OE_DISABLE);

	return 0;
}

static int msm_gpio_set_value(struct udevice *dev, unsigned int gpio, int value)
{
	struct msm_gpio_bank *priv = dev_get_priv(dev);

	value = !!value;
	/* set value */
	writel(value << GPIO_OUT, priv->base + GPIO_IN_OUT_REG(dev, gpio));

	return 0;
}

static int msm_gpio_direction_output(struct udevice *dev, unsigned int gpio,
				     int value)
{
	struct msm_gpio_bank *priv = dev_get_priv(dev);

	value = !!value;
	/* set value */
	writel(value << GPIO_OUT, priv->base + GPIO_IN_OUT_REG(dev, gpio));
	/* switch direction */
	clrsetbits_le32(priv->base + GPIO_CONFIG_REG(dev, gpio),
			GPIO_OE_MASK, GPIO_OE_ENABLE);

	return 0;
}

static int msm_gpio_get_value(struct udevice *dev, unsigned int gpio)
{
	struct msm_gpio_bank *priv = dev_get_priv(dev);

	return !!(readl(priv->base + GPIO_IN_OUT_REG(dev, gpio)) >> GPIO_IN);
}

static int msm_gpio_get_function(struct udevice *dev, unsigned int gpio)
{
	struct msm_gpio_bank *priv = dev_get_priv(dev);

	if (readl(priv->base + GPIO_CONFIG_REG(dev, gpio)) & GPIO_OE_ENABLE)
		return GPIOF_OUTPUT;

	return GPIOF_INPUT;
}

static const struct dm_gpio_ops gpio_msm_ops = {
	.direction_input	= msm_gpio_direction_input,
	.direction_output	= msm_gpio_direction_output,
	.get_value		= msm_gpio_get_value,
	.set_value		= msm_gpio_set_value,
	.get_function		= msm_gpio_get_function,
};

static int msm_gpio_probe(struct udevice *dev)
{
	struct msm_gpio_bank *priv = dev_get_priv(dev);

	priv->base = dev_read_addr(dev);
	priv->pin_data = (struct msm_pin_data *)dev_get_driver_data(dev);

	return priv->base == FDT_ADDR_T_NONE ? -EINVAL : 0;
}

static int msm_gpio_of_to_plat(struct udevice *dev)
{
	struct gpio_dev_priv *uc_priv = dev_get_uclass_priv(dev);
	const struct msm_pin_data *pin_data = (struct msm_pin_data *)dev_get_driver_data(dev);

	/* Get the pin count from the pinctrl driver */
	uc_priv->gpio_count = pin_data->pin_count;
	uc_priv->bank_name = fdt_getprop(gd->fdt_blob, dev_of_offset(dev),
					 "gpio-bank-name", NULL);
	if (uc_priv->bank_name == NULL)
		uc_priv->bank_name = "soc";

	return 0;
}

U_BOOT_DRIVER(gpio_msm) = {
	.name	= "gpio_msm",
	.id	= UCLASS_GPIO,
	.of_to_plat = msm_gpio_of_to_plat,
	.probe	= msm_gpio_probe,
	.ops	= &gpio_msm_ops,
	.flags	= DM_UC_FLAG_SEQ_ALIAS,
	.priv_auto	= sizeof(struct msm_gpio_bank),
};
