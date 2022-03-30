// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2011 The Chromium OS Authors.
 */

#include <common.h>
#include <dm.h>
#include <fdtdec.h>
#include <malloc.h>
#include <asm/gpio.h>
#include <dm/of.h>
#include <dt-bindings/gpio/gpio.h>

/* Flags for each GPIO */
#define GPIOF_OUTPUT	(1 << 0)	/* Currently set as an output */
#define GPIOF_HIGH	(1 << 1)	/* Currently set high */
#define GPIOF_ODR	(1 << 2)	/* Currently set to open drain mode */

struct gpio_state {
	const char *label;	/* label given by requester */
	u8 flags;		/* flags (GPIOF_...) */
};

/* Access routines for GPIO state */
static u8 *get_gpio_flags(struct udevice *dev, unsigned offset)
{
	struct gpio_dev_priv *uc_priv = dev_get_uclass_priv(dev);
	struct gpio_state *state = dev_get_priv(dev);

	if (offset >= uc_priv->gpio_count) {
		static u8 invalid_flags;
		printf("sandbox_gpio: error: invalid gpio %u\n", offset);
		return &invalid_flags;
	}

	return &state[offset].flags;
}

static int get_gpio_flag(struct udevice *dev, unsigned offset, int flag)
{
	return (*get_gpio_flags(dev, offset) & flag) != 0;
}

static int set_gpio_flag(struct udevice *dev, unsigned offset, int flag,
			 int value)
{
	u8 *gpio = get_gpio_flags(dev, offset);

	if (value)
		*gpio |= flag;
	else
		*gpio &= ~flag;

	return 0;
}

/*
 * Back-channel sandbox-internal-only access to GPIO state
 */

int sandbox_gpio_get_value(struct udevice *dev, unsigned offset)
{
	if (get_gpio_flag(dev, offset, GPIOF_OUTPUT))
		debug("sandbox_gpio: get_value on output gpio %u\n", offset);
	return get_gpio_flag(dev, offset, GPIOF_HIGH);
}

int sandbox_gpio_set_value(struct udevice *dev, unsigned offset, int value)
{
	return set_gpio_flag(dev, offset, GPIOF_HIGH, value);
}

int sandbox_gpio_get_open_drain(struct udevice *dev, unsigned offset)
{
	return get_gpio_flag(dev, offset, GPIOF_ODR);
}

int sandbox_gpio_set_open_drain(struct udevice *dev, unsigned offset, int value)
{
	return set_gpio_flag(dev, offset, GPIOF_ODR, value);
}

int sandbox_gpio_get_direction(struct udevice *dev, unsigned offset)
{
	return get_gpio_flag(dev, offset, GPIOF_OUTPUT);
}

int sandbox_gpio_set_direction(struct udevice *dev, unsigned offset, int output)
{
	return set_gpio_flag(dev, offset, GPIOF_OUTPUT, output);
}

/*
 * These functions implement the public interface within U-Boot
 */

/* set GPIO port 'offset' as an input */
static int sb_gpio_direction_input(struct udevice *dev, unsigned offset)
{
	debug("%s: offset:%u\n", __func__, offset);

	return sandbox_gpio_set_direction(dev, offset, 0);
}

/* set GPIO port 'offset' as an output, with polarity 'value' */
static int sb_gpio_direction_output(struct udevice *dev, unsigned offset,
				    int value)
{
	debug("%s: offset:%u, value = %d\n", __func__, offset, value);

	return sandbox_gpio_set_direction(dev, offset, 1) |
		sandbox_gpio_set_value(dev, offset, value);
}

/* read GPIO IN value of port 'offset' */
static int sb_gpio_get_value(struct udevice *dev, unsigned offset)
{
	debug("%s: offset:%u\n", __func__, offset);

	return sandbox_gpio_get_value(dev, offset);
}

/* write GPIO OUT value to port 'offset' */
static int sb_gpio_set_value(struct udevice *dev, unsigned offset, int value)
{
	debug("%s: offset:%u, value = %d\n", __func__, offset, value);

	if (!sandbox_gpio_get_direction(dev, offset)) {
		printf("sandbox_gpio: error: set_value on input gpio %u\n",
		       offset);
		return -1;
	}

	return sandbox_gpio_set_value(dev, offset, value);
}

/* read GPIO ODR value of port 'offset' */
static int sb_gpio_get_open_drain(struct udevice *dev, unsigned offset)
{
	debug("%s: offset:%u\n", __func__, offset);

	return sandbox_gpio_get_open_drain(dev, offset);
}

/* write GPIO ODR value to port 'offset' */
static int sb_gpio_set_open_drain(struct udevice *dev, unsigned offset, int value)
{
	debug("%s: offset:%u, value = %d\n", __func__, offset, value);

	if (!sandbox_gpio_get_direction(dev, offset)) {
		printf("sandbox_gpio: error: set_open_drain on input gpio %u\n",
		       offset);
		return -1;
	}

	return sandbox_gpio_set_open_drain(dev, offset, value);
}

static int sb_gpio_get_function(struct udevice *dev, unsigned offset)
{
	if (get_gpio_flag(dev, offset, GPIOF_OUTPUT))
		return GPIOF_OUTPUT;
	return GPIOF_INPUT;
}

static int sb_gpio_xlate(struct udevice *dev, struct gpio_desc *desc,
			 struct ofnode_phandle_args *args)
{
	desc->offset = args->args[0];
	if (args->args_count < 2)
		return 0;
	if (args->args[1] & GPIO_ACTIVE_LOW)
		desc->flags |= GPIOD_ACTIVE_LOW;
	if (args->args[1] & 2)
		desc->flags |= GPIOD_IS_IN;
	if (args->args[1] & 4)
		desc->flags |= GPIOD_IS_OUT;
	if (args->args[1] & 8)
		desc->flags |= GPIOD_IS_OUT_ACTIVE;

	return 0;
}

static const struct dm_gpio_ops gpio_sandbox_ops = {
	.direction_input	= sb_gpio_direction_input,
	.direction_output	= sb_gpio_direction_output,
	.get_value		= sb_gpio_get_value,
	.set_value		= sb_gpio_set_value,
	.get_open_drain		= sb_gpio_get_open_drain,
	.set_open_drain		= sb_gpio_set_open_drain,
	.get_function		= sb_gpio_get_function,
	.xlate			= sb_gpio_xlate,
};

static int sandbox_gpio_ofdata_to_platdata(struct udevice *dev)
{
	struct gpio_dev_priv *uc_priv = dev_get_uclass_priv(dev);

	uc_priv->gpio_count = dev_read_u32_default(dev, "sandbox,gpio-count",
						   0);
	uc_priv->bank_name = dev_read_string(dev, "gpio-bank-name");

	return 0;
}

static int gpio_sandbox_probe(struct udevice *dev)
{
	struct gpio_dev_priv *uc_priv = dev_get_uclass_priv(dev);

	if (!dev_of_valid(dev))
		/* Tell the uclass how many GPIOs we have */
		uc_priv->gpio_count = CONFIG_SANDBOX_GPIO_COUNT;

	dev->priv = calloc(sizeof(struct gpio_state), uc_priv->gpio_count);

	return 0;
}

static int gpio_sandbox_remove(struct udevice *dev)
{
	free(dev->priv);

	return 0;
}

static const struct udevice_id sandbox_gpio_ids[] = {
	{ .compatible = "sandbox,gpio" },
	{ }
};

U_BOOT_DRIVER(gpio_sandbox) = {
	.name	= "gpio_sandbox",
	.id	= UCLASS_GPIO,
	.of_match = sandbox_gpio_ids,
	.ofdata_to_platdata = sandbox_gpio_ofdata_to_platdata,
	.probe	= gpio_sandbox_probe,
	.remove	= gpio_sandbox_remove,
	.ops	= &gpio_sandbox_ops,
};
