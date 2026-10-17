// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2020 Sean Anderson <seanga2@gmail.com>
 * Copyright (C) 2015 Masahiro Yamada <yamada.masahiro@socionext.com>
 */

#include <common.h>
#include <dm.h>
#include <dm/pinctrl.h>
#include <dt-bindings/pinctrl/sandbox-pinmux.h>
#include <log.h>
#include <linux/bitops.h>

/*
 * This driver emulates a pin controller with the following rules:
 * - The pinctrl config for each pin must be set individually
 * - The first three pins (P0-P2) must be muxed as a group
 * - The next two pins (P3-P4) must be muxed as a group
 * - The last four pins (P5-P8) must be muxed individually
 */

static const char * const sandbox_pins[] = {
#define PIN(x) \
	[x] = "P" #x
	PIN(0),
	PIN(1),
	PIN(2),
	PIN(3),
	PIN(4),
	PIN(5),
	PIN(6),
	PIN(7),
	PIN(8),
#undef PIN
};

static const char * const sandbox_pins_muxing[][2] = {
	{ "UART TX", "I2C SCL" },
	{ "UART RX", "I2C SDA" },
	{ "SPI SCLK", "I2S SCK" },
	{ "SPI MOSI", "I2S SD" },
	{ "SPI MISO", "I2S WS" },
	{ "GPIO0", "SPI CS0" },
	{ "GPIO1", "SPI CS1" },
	{ "GPIO2", "PWM0" },
	{ "GPIO3", "PWM1" },
};

#define SANDBOX_GROUP_I2C_UART 0
#define SANDBOX_GROUP_SPI_I2S 1

static const char * const sandbox_groups[] = {
	[SANDBOX_GROUP_I2C_UART] = "I2C_UART",
	[SANDBOX_GROUP_SPI_I2S] = "SPI_I2S",
};

static const char * const sandbox_functions[] = {
#define FUNC(id) \
	[SANDBOX_PINMUX_##id] = #id
	FUNC(UART),
	FUNC(I2C),
	FUNC(SPI),
	FUNC(I2S),
	FUNC(GPIO),
	FUNC(CS),
	FUNC(PWM),
#undef FUNC
};

static const struct pinconf_param sandbox_conf_params[] = {
	{ "bias-disable", PIN_CONFIG_BIAS_DISABLE, 0 },
	{ "bias-high-impedance", PIN_CONFIG_BIAS_HIGH_IMPEDANCE, 0 },
	{ "bias-bus-hold", PIN_CONFIG_BIAS_BUS_HOLD, 0 },
	{ "bias-pull-up", PIN_CONFIG_BIAS_PULL_UP, 1 },
	{ "bias-pull-down", PIN_CONFIG_BIAS_PULL_DOWN, 1 },
	{ "bias-pull-pin-default", PIN_CONFIG_BIAS_PULL_PIN_DEFAULT, 1 },
	{ "drive-open-drain", PIN_CONFIG_DRIVE_OPEN_DRAIN, 0 },
	{ "drive-open-source", PIN_CONFIG_DRIVE_OPEN_SOURCE, 0 },
	{ "drive-strength", PIN_CONFIG_DRIVE_STRENGTH, 0 },
	{ "input-enable", PIN_CONFIG_INPUT_ENABLE, 1 },
	{ "input-disable", PIN_CONFIG_INPUT_ENABLE, 0 },
};

/* Bitfield used to save param and value of each pin/selector */
struct sandbox_pinctrl_priv {
	unsigned int mux;
	unsigned int pins_param[ARRAY_SIZE(sandbox_pins)];
	unsigned int pins_value[ARRAY_SIZE(sandbox_pins)];
};

static int sandbox_get_pins_count(struct udevice *dev)
{
	return ARRAY_SIZE(sandbox_pins);
}

static const char *sandbox_get_pin_name(struct udevice *dev, unsigned selector)
{
	return sandbox_pins[selector];
}

static int sandbox_get_pin_muxing(struct udevice *dev,
				  unsigned int selector,
				  char *buf, int size)
{
	const struct pinconf_param *p;
	struct sandbox_pinctrl_priv *priv = dev_get_priv(dev);
	int i;

	snprintf(buf, size, "%s",
		 sandbox_pins_muxing[selector][!!(priv->mux & BIT(selector))]);

	if (priv->pins_param[selector]) {
		for (i = 0, p = sandbox_conf_params;
		     i < ARRAY_SIZE(sandbox_conf_params);
		     i++, p++) {
			if ((priv->pins_param[selector] & BIT(p->param)) &&
			    (!!(priv->pins_value[selector] & BIT(p->param)) ==
			     p->default_value)) {
				strncat(buf, " ", size);
				strncat(buf, p->property, size);
			}
		}
	}
	strncat(buf, ".", size);

	return 0;
}

static int sandbox_get_groups_count(struct udevice *dev)
{
	return ARRAY_SIZE(sandbox_groups);
}

static const char *sandbox_get_group_name(struct udevice *dev,
					  unsigned selector)
{
	return sandbox_groups[selector];
}

static int sandbox_get_functions_count(struct udevice *dev)
{
	return ARRAY_SIZE(sandbox_functions);
}

static const char *sandbox_get_function_name(struct udevice *dev,
					     unsigned selector)
{
	return sandbox_functions[selector];
}

static int sandbox_pinmux_set(struct udevice *dev, unsigned pin_selector,
			      unsigned func_selector)
{
	int mux;
	struct sandbox_pinctrl_priv *priv = dev_get_priv(dev);

	debug("sandbox pinmux: pin = %d (%s), function = %d (%s)\n",
	      pin_selector, sandbox_get_pin_name(dev, pin_selector),
	      func_selector, sandbox_get_function_name(dev, func_selector));

	if (pin_selector < 5)
		return -EINVAL;

	switch (func_selector) {
	case SANDBOX_PINMUX_GPIO:
		mux = 0;
		break;
	case SANDBOX_PINMUX_CS:
	case SANDBOX_PINMUX_PWM:
		mux = BIT(pin_selector);
		break;
	default:
		return -EINVAL;
	}

	priv->mux &= ~BIT(pin_selector);
	priv->mux |= mux;
	priv->pins_param[pin_selector] = 0;
	priv->pins_value[pin_selector] = 0;

	return 0;
}

static int sandbox_pinmux_group_set(struct udevice *dev,
				    unsigned group_selector,
				    unsigned func_selector)
{
	bool mux;
	int i, group_start, group_end;
	struct sandbox_pinctrl_priv *priv = dev_get_priv(dev);
	unsigned int mask;

	debug("sandbox pinmux: group = %d (%s), function = %d (%s)\n",
	      group_selector, sandbox_get_group_name(dev, group_selector),
	      func_selector, sandbox_get_function_name(dev, func_selector));

	if (group_selector == SANDBOX_GROUP_I2C_UART) {
		group_start = 0;
		group_end = 1;

		if (func_selector == SANDBOX_PINMUX_UART)
			mux = false;
		else if (func_selector == SANDBOX_PINMUX_I2C)
			mux = true;
		else
			return -EINVAL;
	} else if (group_selector == SANDBOX_GROUP_SPI_I2S) {
		group_start = 2;
		group_end = 4;

		if (func_selector == SANDBOX_PINMUX_SPI)
			mux = false;
		else if (func_selector == SANDBOX_PINMUX_I2S)
			mux = true;
		else
			return -EINVAL;
	} else {
		return -EINVAL;
	}

	mask = GENMASK(group_end, group_start);
	priv->mux &= ~mask;
	priv->mux |= mux ? mask : 0;

	for (i = group_start; i < group_end; i++) {
		priv->pins_param[i] = 0;
		priv->pins_value[i] = 0;
	}

	return 0;
}

static int sandbox_pinmux_property_set(struct udevice *dev, u32 pinmux_group)
{
	int ret;
	unsigned pin_selector = pinmux_group & 0xFFFF;
	unsigned func_selector = pinmux_group >> 16;

	ret = sandbox_pinmux_set(dev, pin_selector, func_selector);
	return ret ? ret : pin_selector;
}

static int sandbox_pinconf_set(struct udevice *dev, unsigned pin_selector,
			       unsigned param, unsigned argument)
{
	struct sandbox_pinctrl_priv *priv = dev_get_priv(dev);

	debug("sandbox pinconf: pin = %d (%s), param = %d, arg = %d\n",
	      pin_selector, sandbox_get_pin_name(dev, pin_selector),
	      param, argument);

	priv->pins_param[pin_selector] |= BIT(param);
	if (argument)
		priv->pins_value[pin_selector] |= BIT(param);
	else
		priv->pins_value[pin_selector] &= ~BIT(param);

	return 0;
}

static int sandbox_pinconf_group_set(struct udevice *dev,
				     unsigned group_selector,
				     unsigned param, unsigned argument)
{
	debug("sandbox pinconf: group = %d (%s), param = %d, arg = %d\n",
	      group_selector, sandbox_get_group_name(dev, group_selector),
	      param, argument);

	return 0;
}

const struct pinctrl_ops sandbox_pinctrl_ops = {
	.get_pins_count = sandbox_get_pins_count,
	.get_pin_name = sandbox_get_pin_name,
	.get_pin_muxing = sandbox_get_pin_muxing,
	.get_groups_count = sandbox_get_groups_count,
	.get_group_name = sandbox_get_group_name,
	.get_functions_count = sandbox_get_functions_count,
	.get_function_name = sandbox_get_function_name,
	.pinmux_set = sandbox_pinmux_set,
	.pinmux_group_set = sandbox_pinmux_group_set,
	.pinmux_property_set = sandbox_pinmux_property_set,
	.pinconf_num_params = ARRAY_SIZE(sandbox_conf_params),
	.pinconf_params = sandbox_conf_params,
	.pinconf_set = sandbox_pinconf_set,
	.pinconf_group_set = sandbox_pinconf_group_set,
	.set_state = pinctrl_generic_set_state,
};

static const struct udevice_id sandbox_pinctrl_match[] = {
	{ .compatible = "sandbox,pinctrl" },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(sandbox_pinctrl) = {
	.name = "sandbox_pinctrl",
	.id = UCLASS_PINCTRL,
	.of_match = sandbox_pinctrl_match,
	.priv_auto	= sizeof(struct sandbox_pinctrl_priv),
	.ops = &sandbox_pinctrl_ops,
};
