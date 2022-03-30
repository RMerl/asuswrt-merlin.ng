// SPDX-License-Identifier: GPL-2.0+
/*
 * Take drivers/gpio/gpio-74x164.c as reference.
 *
 * 74Hx164 - Generic serial-in/parallel-out 8-bits shift register GPIO driver
 *
 * Copyright (C) 2016 Peng Fan <van.freenix@gmail.com>
 *
 */

#include <common.h>
#include <errno.h>
#include <dm.h>
#include <fdtdec.h>
#include <malloc.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <dt-bindings/gpio/gpio.h>
#include <spi.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * struct gen_74x164_chip - Data for 74Hx164
 *
 * @oe: OE pin
 * @nregs: number of registers
 * @buffer: buffer for chained chips
 */
#define GEN_74X164_NUMBER_GPIOS 8

struct gen_74x164_priv {
	struct gpio_desc oe;
	u32 nregs;
	/*
	 * Since the nregs are chained, every byte sent will make
	 * the previous byte shift to the next register in the
	 * chain. Thus, the first byte sent will end up in the last
	 * register at the end of the transfer. So, to have a logical
	 * numbering, store the bytes in reverse order.
	 */
	u8 *buffer;
};

static int gen_74x164_write_conf(struct udevice *dev)
{
	struct gen_74x164_priv *priv = dev_get_priv(dev);
	int ret;

	ret = dm_spi_claim_bus(dev);
	if (ret)
		return ret;

	ret = dm_spi_xfer(dev, priv->nregs * 8, priv->buffer, NULL,
			  SPI_XFER_BEGIN | SPI_XFER_END);

	dm_spi_release_bus(dev);

	return ret;
}

static int gen_74x164_get_value(struct udevice *dev, unsigned offset)
{
	struct gen_74x164_priv *priv = dev_get_priv(dev);
	uint bank = priv->nregs - 1 - offset / 8;
	uint pin = offset % 8;

	return (priv->buffer[bank] >> pin) & 0x1;
}

static int gen_74x164_set_value(struct udevice *dev, unsigned offset,
				int value)
{
	struct gen_74x164_priv *priv = dev_get_priv(dev);
	uint bank = priv->nregs - 1 - offset / 8;
	uint pin = offset % 8;
	int ret;

	if (value)
		priv->buffer[bank] |= 1 << pin;
	else
		priv->buffer[bank] &= ~(1 << pin);

	ret = gen_74x164_write_conf(dev);
	if (ret)
		return ret;

	return 0;
}

static int gen_74x164_direction_input(struct udevice *dev, unsigned offset)
{
	return -ENOSYS;
}

static int gen_74x164_direction_output(struct udevice *dev, unsigned offset,
				      int value)
{
	return gen_74x164_set_value(dev, offset, value);
}

static int gen_74x164_get_function(struct udevice *dev, unsigned offset)
{
	return GPIOF_OUTPUT;
}

static int gen_74x164_xlate(struct udevice *dev, struct gpio_desc *desc,
			    struct ofnode_phandle_args *args)
{
	desc->offset = args->args[0];
	desc->flags = args->args[1] & GPIO_ACTIVE_LOW ? GPIOD_ACTIVE_LOW : 0;

	return 0;
}

static const struct dm_gpio_ops gen_74x164_ops = {
	.direction_input	= gen_74x164_direction_input,
	.direction_output	= gen_74x164_direction_output,
	.get_value		= gen_74x164_get_value,
	.set_value		= gen_74x164_set_value,
	.get_function		= gen_74x164_get_function,
	.xlate			= gen_74x164_xlate,
};

static int gen_74x164_probe(struct udevice *dev)
{
	struct gen_74x164_priv *priv = dev_get_priv(dev);
	struct gpio_dev_priv *uc_priv = dev_get_uclass_priv(dev);
	char *str, name[32];
	int ret;
	const void *fdt = gd->fdt_blob;
	int node = dev_of_offset(dev);

	snprintf(name, sizeof(name), "%s_", dev->name);
	str = strdup(name);
	if (!str)
		return -ENOMEM;

	/*
	 * See Linux kernel:
	 * Documentation/devicetree/bindings/gpio/gpio-74x164.txt
	 */
	priv->nregs = fdtdec_get_int(fdt, node, "registers-number", 1);
	priv->buffer = calloc(priv->nregs, sizeof(u8));
	if (!priv->buffer) {
		ret = -ENOMEM;
		goto free_str;
	}

	ret = fdtdec_get_byte_array(fdt, node, "registers-default",
				    priv->buffer, priv->nregs);
	if (ret)
		dev_dbg(dev, "No registers-default property\n");

	ret = gpio_request_by_name(dev, "oe-gpios", 0, &priv->oe,
				   GPIOD_IS_OUT | GPIOD_IS_OUT_ACTIVE);
	if (ret) {
		dev_dbg(dev, "No oe-pins property\n");
	}

	uc_priv->bank_name = str;
	uc_priv->gpio_count = priv->nregs * 8;

	ret = gen_74x164_write_conf(dev);
	if (ret)
		goto free_buf;

	dev_dbg(dev, "%s is ready\n", dev->name);

	return 0;

free_buf:
	free(priv->buffer);
free_str:
	free(str);
	return ret;
}

static const struct udevice_id gen_74x164_ids[] = {
	{ .compatible = "fairchild,74hc595" },
	{ }
};

U_BOOT_DRIVER(74x164) = {
	.name		= "74x164",
	.id		= UCLASS_GPIO,
	.ops		= &gen_74x164_ops,
	.probe		= gen_74x164_probe,
	.priv_auto_alloc_size = sizeof(struct gen_74x164_priv),
	.of_match	= gen_74x164_ids,
};
