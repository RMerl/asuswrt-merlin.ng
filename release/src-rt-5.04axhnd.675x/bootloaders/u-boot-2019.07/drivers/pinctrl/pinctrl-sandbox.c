// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015  Masahiro Yamada <yamada.masahiro@socionext.com>
 */

/* #define DEBUG */

#include <common.h>
#include <dm.h>
#include <dm/pinctrl.h>

static const char * const sandbox_pins[] = {
	"SCL",
	"SDA",
	"TX",
	"RX",
	"W1"
};

static const char * const sandbox_pins_muxing[] = {
	"I2C SCL",
	"I2C SDA",
	"Uart TX",
	"Uart RX",
	"1-wire gpio",
};

static const char * const sandbox_groups[] = {
	"i2c",
	"serial_a",
	"serial_b",
	"spi",
	"w1",
};

static const char * const sandbox_functions[] = {
	"i2c",
	"serial",
	"spi",
	"w1",
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
	snprintf(buf, size, "%s", sandbox_pins_muxing[selector]);

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
	debug("sandbox pinmux: pin = %d (%s), function = %d (%s)\n",
	      pin_selector, sandbox_get_pin_name(dev, pin_selector),
	      func_selector, sandbox_get_function_name(dev, func_selector));

	return 0;
}

static int sandbox_pinmux_group_set(struct udevice *dev,
				    unsigned group_selector,
				    unsigned func_selector)
{
	debug("sandbox pinmux: group = %d (%s), function = %d (%s)\n",
	      group_selector, sandbox_get_group_name(dev, group_selector),
	      func_selector, sandbox_get_function_name(dev, func_selector));

	return 0;
}

static int sandbox_pinconf_set(struct udevice *dev, unsigned pin_selector,
			       unsigned param, unsigned argument)
{
	debug("sandbox pinconf: pin = %d (%s), param = %d, arg = %d\n",
	      pin_selector, sandbox_get_pin_name(dev, pin_selector),
	      param, argument);

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
	.ops = &sandbox_pinctrl_ops,
};
