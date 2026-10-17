// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2019 DEIF A/S
 *
 * GPIO driver to set/clear SPISEL_BOOT pin on mpc83xx.
 */

#include <common.h>
#include <log.h>
#include <dm.h>
#include <mapmem.h>
#include <asm/gpio.h>

struct mpc83xx_spisel_boot {
	u32 __iomem *spi_cs;
	ulong addr;
	uint gpio_count;
	ulong type;
};

static u32 gpio_mask(uint gpio)
{
	return (1U << (31 - (gpio)));
}

static int mpc83xx_spisel_boot_direction_input(struct udevice *dev, uint gpio)
{
	return -EINVAL;
}

static int mpc83xx_spisel_boot_set_value(struct udevice *dev, uint gpio, int value)
{
	struct mpc83xx_spisel_boot *data = dev_get_priv(dev);

	debug("%s: gpio=%d, value=%u, gpio_mask=0x%08x\n", __func__,
	      gpio, value, gpio_mask(gpio));

	if (value)
		setbits_be32(data->spi_cs, gpio_mask(gpio));
	else
		clrbits_be32(data->spi_cs, gpio_mask(gpio));

	return 0;
}

static int mpc83xx_spisel_boot_direction_output(struct udevice *dev, uint gpio, int value)
{
	return 0;
}

static int mpc83xx_spisel_boot_get_value(struct udevice *dev, uint gpio)
{
	struct mpc83xx_spisel_boot *data = dev_get_priv(dev);

	return !!(in_be32(data->spi_cs) & gpio_mask(gpio));
}

static int mpc83xx_spisel_boot_get_function(struct udevice *dev, uint gpio)
{
	return GPIOF_OUTPUT;
}

#if CONFIG_IS_ENABLED(OF_CONTROL)
static int mpc83xx_spisel_boot_of_to_plat(struct udevice *dev)
{
	struct mpc8xxx_gpio_plat *plat = dev_get_plat(dev);
	fdt_addr_t addr;
	u32 reg[2];

	dev_read_u32_array(dev, "reg", reg, 2);
	addr = dev_translate_address(dev, reg);

	plat->addr = addr;
	plat->size = reg[1];
	plat->ngpios = dev_read_u32_default(dev, "ngpios", 1);

	return 0;
}
#endif

static int mpc83xx_spisel_boot_plat_to_priv(struct udevice *dev)
{
	struct mpc83xx_spisel_boot *priv = dev_get_priv(dev);
	struct mpc8xxx_gpio_plat *plat = dev_get_plat(dev);
	unsigned long size = plat->size;
	ulong driver_data = dev_get_driver_data(dev);

	if (size == 0)
		size = 0x04;

	priv->addr = plat->addr;
	priv->spi_cs = map_sysmem(plat->addr, size);

	if (!priv->spi_cs)
		return -ENOMEM;

	priv->gpio_count = plat->ngpios;

	priv->type = driver_data;

	return 0;
}

static int mpc83xx_spisel_boot_probe(struct udevice *dev)
{
	struct gpio_dev_priv *uc_priv = dev_get_uclass_priv(dev);
	struct mpc83xx_spisel_boot *data = dev_get_priv(dev);
	char name[32], *str;

	mpc83xx_spisel_boot_plat_to_priv(dev);

	snprintf(name, sizeof(name), "MPC@%lx_", data->addr);
	str = strdup(name);

	if (!str)
		return -ENOMEM;

	uc_priv->bank_name = str;
	uc_priv->gpio_count = data->gpio_count;

	return 0;
}

static const struct dm_gpio_ops mpc83xx_spisel_boot_ops = {
	.direction_input	= mpc83xx_spisel_boot_direction_input,
	.direction_output	= mpc83xx_spisel_boot_direction_output,
	.get_value		= mpc83xx_spisel_boot_get_value,
	.set_value		= mpc83xx_spisel_boot_set_value,
	.get_function		= mpc83xx_spisel_boot_get_function,
};

static const struct udevice_id mpc83xx_spisel_boot_ids[] = {
	{ .compatible = "fsl,mpc8309-spisel-boot" },
	{ .compatible = "fsl,mpc83xx-spisel-boot" },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(spisel_boot_mpc83xx) = {
	.name	= "spisel_boot_mpc83xx",
	.id	= UCLASS_GPIO,
	.ops	= &mpc83xx_spisel_boot_ops,
#if CONFIG_IS_ENABLED(OF_CONTROL)
	.of_to_plat = mpc83xx_spisel_boot_of_to_plat,
	.plat_auto	= sizeof(struct mpc8xxx_gpio_plat),
	.of_match = mpc83xx_spisel_boot_ids,
#endif
	.probe	= mpc83xx_spisel_boot_probe,
	.priv_auto	= sizeof(struct mpc83xx_spisel_boot),
};
