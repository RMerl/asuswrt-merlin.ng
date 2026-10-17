// SPDX-License-Identifier: GPL-2.0
/*
 * RZ/G2L Pin Function Controller
 *
 * Copyright (C) 2021-2023 Renesas Electronics Corp.
 */

#include <asm-generic/gpio.h>
#include <asm/io.h>
#include <dm/device.h>
#include <dm/device_compat.h>
#include <renesas/rzg2l-pfc.h>

static void rzg2l_gpio_set(const struct rzg2l_pfc_data *data, u32 port, u8 pin,
			   bool value)
{
	if (value)
		setbits_8(data->base + P(port), BIT(pin));
	else
		clrbits_8(data->base + P(port), BIT(pin));
}

static int rzg2l_gpio_get_value(struct udevice *dev, unsigned int offset)
{
	const struct rzg2l_pfc_data *data =
		(const struct rzg2l_pfc_data *)dev_get_driver_data(dev);
	const u32 port = RZG2L_PINMUX_TO_PORT(offset);
	const u8 pin = RZG2L_PINMUX_TO_PIN(offset);
	u16 pm_state;

	pm_state = (readw(data->base + PM(port)) >> (pin * 2)) & PM_MASK;
	switch (pm_state) {
	case PM_INPUT:
		return !!(readb(data->base + PIN(port)) & BIT(pin));
	case PM_OUTPUT:
	case PM_OUTPUT_IEN:
		return !!(readb(data->base + P(port)) & BIT(pin));
	default:	/* PM_HIGH_Z */
		return 0;
	}
}

static int rzg2l_gpio_set_value(struct udevice *dev, unsigned int offset,
				int value)
{
	const struct rzg2l_pfc_data *data =
		(const struct rzg2l_pfc_data *)dev_get_driver_data(dev);
	const u32 port = RZG2L_PINMUX_TO_PORT(offset);
	const u8 pin = RZG2L_PINMUX_TO_PIN(offset);

	rzg2l_gpio_set(data, port, pin, (bool)value);
	return 0;
}

static void rzg2l_gpio_set_direction(const struct rzg2l_pfc_data *data,
				     u32 port, u8 pin, bool output)
{
	clrsetbits_le16(data->base + PM(port), PM_MASK << (pin * 2),
			(output ? PM_OUTPUT : PM_INPUT) << (pin * 2));
}

static int rzg2l_gpio_direction_input(struct udevice *dev, unsigned int offset)
{
	const struct rzg2l_pfc_data *data =
		(const struct rzg2l_pfc_data *)dev_get_driver_data(dev);
	const u32 port = RZG2L_PINMUX_TO_PORT(offset);
	const u8 pin = RZG2L_PINMUX_TO_PIN(offset);

	rzg2l_gpio_set_direction(data, port, pin, false);
	return 0;
}

static int rzg2l_gpio_direction_output(struct udevice *dev, unsigned int offset,
				       int value)
{
	const struct rzg2l_pfc_data *data =
		(const struct rzg2l_pfc_data *)dev_get_driver_data(dev);
	const u32 port = RZG2L_PINMUX_TO_PORT(offset);
	const u8 pin = RZG2L_PINMUX_TO_PIN(offset);

	rzg2l_gpio_set(data, port, pin, (bool)value);
	rzg2l_gpio_set_direction(data, port, pin, true);
	return 0;
}

static int rzg2l_gpio_request(struct udevice *dev, unsigned int offset,
			      const char *label)
{
	const struct rzg2l_pfc_data *data =
		(const struct rzg2l_pfc_data *)dev_get_driver_data(dev);
	const u32 port = RZG2L_PINMUX_TO_PORT(offset);
	const u8 pin = RZG2L_PINMUX_TO_PIN(offset);

	if (!rzg2l_port_validate(data, port, pin)) {
		dev_err(dev, "Invalid GPIO %u:%u\n", port, pin);
		return -EINVAL;
	}

	/* Select GPIO mode in PMC Register */
	clrbits_8(data->base + PMC(port), BIT(pin));

	return 0;
}

static int rzg2l_gpio_get_function(struct udevice *dev, unsigned int offset)
{
	const struct rzg2l_pfc_data *data =
		(const struct rzg2l_pfc_data *)dev_get_driver_data(dev);
	const u32 port = RZG2L_PINMUX_TO_PORT(offset);
	const u8 pin = RZG2L_PINMUX_TO_PIN(offset);
	u16 pm_state;
	u8 pmc_state;

	if (!rzg2l_port_validate(data, port, pin)) {
		/* This offset does not correspond to a valid GPIO pin. */
		return -ENOENT;
	}

	/* Check if the pin is in GPIO or function mode. */
	pmc_state = readb(data->base + PMC(port)) & BIT(pin);
	if (pmc_state)
		return GPIOF_FUNC;

	/* Check the pin direction. */
	pm_state = (readw(data->base + PM(port)) >> (pin * 2)) & PM_MASK;
	switch (pm_state) {
	case PM_INPUT:
		return GPIOF_INPUT;
	case PM_OUTPUT:
	case PM_OUTPUT_IEN:
		return GPIOF_OUTPUT;
	default:	/* PM_HIGH_Z */
		return GPIOF_UNUSED;
	}
}

static const struct dm_gpio_ops rzg2l_gpio_ops = {
	.direction_input	= rzg2l_gpio_direction_input,
	.direction_output	= rzg2l_gpio_direction_output,
	.get_value		= rzg2l_gpio_get_value,
	.set_value		= rzg2l_gpio_set_value,
	.request		= rzg2l_gpio_request,
	.get_function		= rzg2l_gpio_get_function,
};

static int rzg2l_gpio_probe(struct udevice *dev)
{
	struct gpio_dev_priv *uc_priv = dev_get_uclass_priv(dev);
	struct ofnode_phandle_args args;
	int ret;

	uc_priv->bank_name = "rzg2l-pfc-gpio";
	ret = ofnode_parse_phandle_with_args(dev_ofnode(dev), "gpio-ranges",
					     NULL, 3, 0, &args);
	if (ret < 0) {
		dev_err(dev, "Failed to parse gpio-ranges: %d\n", ret);
		return -EINVAL;
	}

	uc_priv->gpio_count = args.args[2];
	return rzg2l_pfc_enable(dev);
}

U_BOOT_DRIVER(rzg2l_pfc_gpio) = {
	.name		= "rzg2l-pfc-gpio",
	.id		= UCLASS_GPIO,
	.ops		= &rzg2l_gpio_ops,
	.probe		= rzg2l_gpio_probe,
};
