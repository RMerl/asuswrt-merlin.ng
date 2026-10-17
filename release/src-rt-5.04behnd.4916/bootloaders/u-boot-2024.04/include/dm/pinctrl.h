/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2015  Masahiro Yamada <yamada.masahiro@com>
 */

#ifndef __PINCTRL_H
#define __PINCTRL_H

#include <linux/errno.h>

#define PINNAME_SIZE	10
#define PINMUX_SIZE	90

/**
 * struct pinconf_param - pin config parameters
 * @property:		Property name in DT nodes
 * @param:		ID for this config parameter
 * @default_value:	default value for this config parameter used in case
 *			no value is specified in DT nodes
 */
struct pinconf_param {
	const char * const property;
	unsigned int param;
	u32 default_value;
};

/**
 * struct pinctrl_ops - pin control operations, to be implemented by
 * pin controller drivers.
 *
 * set_state() is the only mandatory operation. You can implement your pinctrl
 * driver with its own @set_state. In this case, the other callbacks are not
 * required. Otherwise, generic pinctrl framework is also available; use
 * pinctrl_generic_set_state for @set_state, and implement other operations
 * depending on your necessity.
 */
struct pinctrl_ops {
	/**
	 * @get_pins_count: Get the number of selectable pins
	 *
	 * @dev: Pinctrl device to use
	 *
	 * This function is necessary to parse the "pins" property in DTS.
	 *
	 * @Return:
	 *	number of selectable named pins available in this driver
	 */
	int (*get_pins_count)(struct udevice *dev);

	/**
	 * @get_pin_name: Get the name of a pin
	 *
	 * @dev: Pinctrl device of the pin
	 *
	 * @selector: The pin selector
	 *
	 * This function is called by the core to figure out which pin it will
	 * do operations to. This function is necessary to parse the "pins"
	 * property in DTS.
	 *
	 * @Return: const pointer to the name of the pin
	 */
	const char *(*get_pin_name)(struct udevice *dev, unsigned selector);

	/**
	 * @get_groups_count: Get the number of selectable groups
	 *
	 * @dev: Pinctrl device to use
	 *
	 * This function is necessary to parse the "groups" property in DTS.
	 *
	 * @Return:
	 *	number of selectable named groups available in the driver
	 */
	int (*get_groups_count)(struct udevice *dev);

	/**
	 * @get_group_name: Get the name of a group
	 *
	 * @dev: Pinctrl device of the group
	 *
	 * @selector: The group selector
	 *
	 * This function is called by the core to figure out which group it
	 * will do operations to. This function is necessary to parse the
	 * "groups" property in DTS.
	 *
	 * @Return: Pointer to the name of the group
	 */
	const char *(*get_group_name)(struct udevice *dev, unsigned selector);

	/**
	 * @get_functions_count: Get the number of selectable functions
	 *
	 * @dev: Pinctrl device to use
	 *
	 * This function is necessary for pin-muxing.
	 *
	 * @Return:
	 *	number of selectable named functions available in this driver
	 */
	int (*get_functions_count)(struct udevice *dev);

	/**
	 * @get_function_name: Get the name of a function
	 *
	 * @dev: Pinmux device of the function
	 *
	 * @selector: The function selector
	 *
	 * This function is called by the core to figure out which mux setting
	 * it will map a certain device to. This function is necessary for
	 * pin-muxing.
	 *
	 * @Return:
	 *	Pointer to the function name of the muxing selector
	 */
	const char *(*get_function_name)(struct udevice *dev,
					 unsigned selector);

	/**
	 * @pinmux_set: Mux a pin to a function
	 *
	 * @dev: Pinctrl device to use
	 *
	 * @pin_selector: The pin selector
	 *
	 * @func_selector: The func selector
	 *
	 * On simple controllers one of @pin_selector or @func_selector may be
	 * ignored. This function is necessary for pin-muxing against a single
	 * pin.
	 *
	 * @Return: 0 if OK, or negative error code on failure
	 */
	int (*pinmux_set)(struct udevice *dev, unsigned pin_selector,
			  unsigned func_selector);

	/**
	 * @pinmux_group_set: Mux a group of pins to a function
	 *
	 * @dev: Pinctrl device to use
	 *
	 * @group_selector: The group selector
	 *
	 * @func_selector: The func selector
	 *
	 * On simple controllers one of @group_selector or @func_selector may be
	 * ignored. This function is necessary for pin-muxing against a group of
	 * pins.
	 *
	 * @Return: 0 if OK, or negative error code on failure
	 */
	int (*pinmux_group_set)(struct udevice *dev, unsigned group_selector,
				unsigned func_selector);

	/**
	 * @pinmux_property_set: Enable a pinmux group
	 *
	 * @dev: Pinctrl device to use
	 *
	 * @pinmux_group: A u32 representing the pin identifier and mux
	 *                settings. The exact format of a pinmux group is left
	 *                up to the driver.
	 *
	 * Mux a single pin to a single function based on a driver-specific
	 * pinmux group. This function is necessary for parsing the "pinmux"
	 * property in DTS, and for pin-muxing against a pinmux group.
	 *
	 * @Return:
	 *	Pin selector for the muxed pin if OK, or negative error code on
	 *	failure
	 */
	int (*pinmux_property_set)(struct udevice *dev, u32 pinmux_group);

	/**
	 * @pinconf_num_params:
	 *	Number of driver-specific parameters to be parsed from device
	 *	trees. This member is necessary for pin configuration.
	 */
	unsigned int pinconf_num_params;

	/**
	 * @pinconf_params:
	 *	List of driver-specific parameters to be parsed from the device
	 *	tree. This member is necessary for pin configuration.
	 */
	const struct pinconf_param *pinconf_params;

	/**
	 * @pinconf_set: Configure an individual pin with a parameter
	 *
	 * @dev: Pinctrl device to use
	 *
	 * @pin_selector: The pin selector
	 *
	 * @param: An &enum pin_config_param from @pinconf_params
	 *
	 * @argument: The argument to this param from the device tree, or
	 *            @pinconf_params.default_value
	 *
	 * This function is necessary for pin configuration against a single
	 * pin.
	 *
	 * @Return: 0 if OK, or negative error code on failure
	 */
	int (*pinconf_set)(struct udevice *dev, unsigned pin_selector,
			   unsigned param, unsigned argument);

	/**
	 * @pinconf_group_set: Configure all pins in a group with a parameter
	 *
	 * @dev: Pinctrl device to use
	 *
	 * @pin_selector: The group selector
	 *
	 * @param: A &enum pin_config_param from
	 *         @pinconf_params
	 *
	 * @argument: The argument to this param from the device tree, or
	 *            @pinconf_params.default_value
	 *
	 * This function is necessary for pin configuration against a group of
	 * pins.
	 *
	 * @Return: 0 if OK, or negative error code on failure
	 */
	int (*pinconf_group_set)(struct udevice *dev, unsigned group_selector,
				 unsigned param, unsigned argument);

	/**
	 * @set_state: Configure a pinctrl device
	 *
	 * @dev: Pinctrl device to use
	 *
	 * @config: Pseudo device pointing a config node
	 *
	 * This function is required to be implemented by all pinctrl drivers.
	 * Drivers may set this member to pinctrl_generic_set_state(), which
	 * will call other functions in &struct pinctrl_ops to parse
	 * @config.
	 *
	 * @Return: 0 if OK, or negative error code on failure
	 */
	int (*set_state)(struct udevice *dev, struct udevice *config);

	/**
	 * @set_state_simple: Configure a pinctrl device
	 *
	 * @dev: Pinctrl device to use
	 *
	 * @config: Pseudo-device pointing a config node
	 *
	 * This function is usually a simpler version of set_state(). Only the
	 * first pinctrl device on the system is supported by this function.
	 *
	 * @Return: 0 if OK, or negative error code on failure
	 */
	int (*set_state_simple)(struct udevice *dev, struct udevice *periph);

	/**
	 * @request: Request a particular pinctrl function
	 *
	 * @dev: Device to adjust (%UCLASS_PINCTRL)
	 *
	 * @func: Function number (driver-specific)
	 *
	 * This activates the selected function.
	 *
	 * @Return: 0 if OK, or negative error code on failure
	 */
	int (*request)(struct udevice *dev, int func, int flags);

	/**
	* @get_periph_id: Get the peripheral ID for a device
	*
	* @dev: Pinctrl device to use for decoding
	*
	* @periph: Device to check
	*
	* This generally looks at the peripheral's device tree node to work
	* out the peripheral ID. The return value is normally interpreted as
	* &enum periph_id. so long as this is defined by the platform (which it
	* should be).
	*
	* @Return:
	*	Peripheral ID of @periph, or %-ENOENT on error
	*/
	int (*get_periph_id)(struct udevice *dev, struct udevice *periph);

	/**
	 * @get_gpio_mux: Get the mux value for a particular GPIO
	 *
	 * @dev: Pinctrl device to use
	 *
	 * @banknum: GPIO bank number
	 *
	 * @index: GPIO index within the bank
	 *
	 * This allows the raw mux value for a GPIO to be obtained. It is
	 * useful for displaying the function being used by that GPIO, such
	 * as with the 'gpio' command. This function is internal to the GPIO
	 * subsystem and should not be used by generic code. Typically it is
	 * used by a GPIO driver with knowledge of the SoC pinctrl setup.
	 *
	 * @Return:
	 *	Mux value (SoC-specific, e.g. 0 for input, 1 for output)
	 */
	int (*get_gpio_mux)(struct udevice *dev, int banknum, int index);

	/**
	 * @get_pin_muxing: Show pin muxing
	 *
	 * @dev: Pinctrl device to use
	 *
	 * @selector: Pin selector
	 *
	 * @buf: Buffer to fill with pin muxing description
	 *
	 * @size: Size of @buf
	 *
	 * This allows to display the muxing of a given pin. It's useful for
	 * debug purposes to know if a pin is configured as GPIO or as an
	 * alternate function and which one. Typically it is used by a PINCTRL
	 * driver with knowledge of the SoC pinctrl setup.
	 *
	 * @Return: 0 if OK, or negative error code on failure
	 */
	 int (*get_pin_muxing)(struct udevice *dev, unsigned int selector,
			       char *buf, int size);

	/**
	 * @gpio_request_enable: Request and enable GPIO on a certain pin.
	 *
	 * @dev: Pinctrl device to use
	 *
	 * @selector: Pin selector
	 *
	 * Implement this only if you can mux every pin individually as GPIO.
	 * The affected GPIO range is passed along with an offset(pin number)
	 * into that specific GPIO range - function selectors and pin groups are
	 * orthogonal to this, the core will however make sure the pins do not
	 * collide.
	 *
	 * @Return:
	 *	0 if OK, or negative error code on failure
	 */
	int (*gpio_request_enable)(struct udevice *dev, unsigned int selector);

	/**
	 * @gpio_disable_free: Free up GPIO muxing on a certain pin.
	 *
	 * @dev: Pinctrl device to use
	 *
	 * @selector: Pin selector
	 *
	 * This function is the reverse of @gpio_request_enable.
	 *
	 * @Return: 0 if OK, or negative error code on failure
	 */
	int (*gpio_disable_free)(struct udevice *dev, unsigned int selector);
};

#define pinctrl_get_ops(dev)	((struct pinctrl_ops *)(dev)->driver->ops)

/**
 * enum pin_config_param - Generic pin configuration parameters
 *
 * @PIN_CONFIG_BIAS_BUS_HOLD: The pin will be set to weakly latch so that it
 *	weakly drives the last value on a tristate bus, also known as a "bus
 *	holder", "bus keeper" or "repeater". This allows another device on the
 *	bus to change the value by driving the bus high or low and switching to
 *	tristate. The argument is ignored.
 * @PIN_CONFIG_BIAS_DISABLE: Disable any pin bias on the pin, a
 *	transition from say pull-up to pull-down implies that you disable
 *	pull-up in the process, this setting disables all biasing.
 * @PIN_CONFIG_BIAS_HIGH_IMPEDANCE: The pin will be set to a high impedance
 *	mode, also know as "third-state" (tristate) or "high-Z" or "floating".
 *	On output pins this effectively disconnects the pin, which is useful
 *	if for example some other pin is going to drive the signal connected
 *	to it for a while. Pins used for input are usually always high
 *	impedance.
 * @PIN_CONFIG_BIAS_PULL_DOWN: The pin will be pulled down (usually with high
 *	impedance to GROUND). If the argument is != 0 pull-down is enabled,
 *	if it is 0, pull-down is total, i.e. the pin is connected to GROUND.
 * @PIN_CONFIG_BIAS_PULL_PIN_DEFAULT: The pin will be pulled up or down based
 *	on embedded knowledge of the controller hardware, like current mux
 *	function. The pull direction and possibly strength too will normally
 *	be decided completely inside the hardware block and not be readable
 *	from the kernel side.
 *	If the argument is != 0 pull up/down is enabled, if it is 0, the
 *	configuration is ignored. The proper way to disable it is to use
 *	@PIN_CONFIG_BIAS_DISABLE.
 * @PIN_CONFIG_BIAS_PULL_UP: The pin will be pulled up (usually with high
 *	impedance to VDD). If the argument is != 0 pull-up is enabled,
 *	if it is 0, pull-up is total, i.e. the pin is connected to VDD.
 * @PIN_CONFIG_DRIVE_OPEN_DRAIN: The pin will be driven with open drain (open
 *	collector) which means it is usually wired with other output ports
 *	which are then pulled up with an external resistor. Setting this
 *	config will enable open drain mode, the argument is ignored.
 * @PIN_CONFIG_DRIVE_OPEN_SOURCE: The pin will be driven with open source
 *	(open emitter). Setting this config will enable open source mode, the
 *	argument is ignored.
 * @PIN_CONFIG_DRIVE_PUSH_PULL: The pin will be driven actively high and
 *	low, this is the most typical case and is typically achieved with two
 *	active transistors on the output. Setting this config will enable
 *	push-pull mode, the argument is ignored.
 * @PIN_CONFIG_DRIVE_STRENGTH: The pin will sink or source at most the current
 *	passed as argument. The argument is in mA.
 * @PIN_CONFIG_DRIVE_STRENGTH_UA: The pin will sink or source at most the
 *	current passed as argument. The argument is in uA.
 * @PIN_CONFIG_INPUT_DEBOUNCE: This will configure the pin to debounce mode,
 *	which means it will wait for signals to settle when reading inputs. The
 *	argument gives the debounce time in usecs. Setting the
 *	argument to zero turns debouncing off.
 * @PIN_CONFIG_INPUT_ENABLE: Enable the pin's input.  Note that this does not
 *	affect the pin's ability to drive output.  1 enables input, 0 disables
 *	input.
 * @PIN_CONFIG_INPUT_SCHMITT: This will configure an input pin to run in
 *	schmitt-trigger mode. If the schmitt-trigger has adjustable hysteresis,
 *	the threshold value is given on a custom format as argument when
 *	setting pins to this mode.
 * @PIN_CONFIG_INPUT_SCHMITT_ENABLE: Control schmitt-trigger mode on the pin.
 *      If the argument != 0, schmitt-trigger mode is enabled. If it's 0,
 *      schmitt-trigger mode is disabled.
 * @PIN_CONFIG_LOW_POWER_MODE: This will configure the pin for low power
 *	operation, if several modes of operation are supported these can be
 *	passed in the argument on a custom form, else just use argument 1
 *	to indicate low power mode, argument 0 turns low power mode off.
 * @PIN_CONFIG_OUTPUT_ENABLE: This will enable the pin's output mode
 *	without driving a value there. For most platforms this reduces to
 *	enable the output buffers and then let the pin controller current
 *	configuration (eg. the currently selected mux function) drive values on
 *	the line. Use argument 1 to enable output mode, argument 0 to disable
 *	it.
 * @PIN_CONFIG_OUTPUT: This will configure the pin as an output and drive a
 *	value on the line. Use argument 1 to indicate high level, argument 0 to
 *	indicate low level. (Please see Documentation/driver-api/pinctl.rst,
 *	section "GPIO mode pitfalls" for a discussion around this parameter.)
 * @PIN_CONFIG_POWER_SOURCE: If the pin can select between different power
 *	supplies, the argument to this parameter (on a custom format) tells
 *	the driver which alternative power source to use.
 * @PIN_CONFIG_SLEEP_HARDWARE_STATE: Indicate this is sleep related state.
 * @PIN_CONFIG_SLEW_RATE: If the pin can select slew rate, the argument to
 *	this parameter (on a custom format) tells the driver which alternative
 *	slew rate to use.
 * @PIN_CONFIG_SKEW_DELAY: If the pin has programmable skew rate (on inputs)
 *	or latch delay (on outputs) this parameter (in a custom format)
 *	specifies the clock skew or latch delay. It typically controls how
 *	many double inverters are put in front of the line.
 * @PIN_CONFIG_END: This is the last enumerator for pin configurations, if
 *	you need to pass in custom configurations to the pin controller, use
 *	PIN_CONFIG_END+1 as the base offset.
 * @PIN_CONFIG_MAX: This is the maximum configuration value that can be
 *	presented using the packed format.
 */
enum pin_config_param {
	PIN_CONFIG_BIAS_BUS_HOLD = 0,
	PIN_CONFIG_BIAS_DISABLE = 1,
	PIN_CONFIG_BIAS_HIGH_IMPEDANCE = 2,
	PIN_CONFIG_BIAS_PULL_DOWN = 3,
	PIN_CONFIG_BIAS_PULL_PIN_DEFAULT = 4,
	PIN_CONFIG_BIAS_PULL_UP = 5,
	PIN_CONFIG_DRIVE_OPEN_DRAIN = 6,
	PIN_CONFIG_DRIVE_OPEN_SOURCE = 7,
	PIN_CONFIG_DRIVE_PUSH_PULL = 8,
	PIN_CONFIG_DRIVE_STRENGTH = 9,
	PIN_CONFIG_DRIVE_STRENGTH_UA = 10,
	PIN_CONFIG_INPUT_DEBOUNCE = 11,
	PIN_CONFIG_INPUT_ENABLE = 12,
	PIN_CONFIG_INPUT_SCHMITT = 13,
	PIN_CONFIG_INPUT_SCHMITT_ENABLE = 14,
	PIN_CONFIG_LOW_POWER_MODE = 15,
	PIN_CONFIG_OUTPUT_ENABLE = 16,
	PIN_CONFIG_OUTPUT = 17,
	PIN_CONFIG_POWER_SOURCE = 18,
	PIN_CONFIG_SLEEP_HARDWARE_STATE = 19,
	PIN_CONFIG_SLEW_RATE = 20,
	PIN_CONFIG_SKEW_DELAY = 21,
	PIN_CONFIG_END = 127,	/* 0x7F */
	PIN_CONFIG_MAX = 255, /* 0xFF */
};

#if CONFIG_IS_ENABLED(PINCTRL_GENERIC)
/**
 * pinctrl_generic_set_state() - Generic set_state operation
 * @pctldev:	Pinctrl device to use
 * @config:	Config device (pseudo device), pointing a config node in DTS
 *
 * Parse the DT node of @config and its children and handle generic properties
 * such as "pins", "groups", "functions", and pin configuration parameters.
 *
 * Return: 0 on success, or negative error code on failure
 */
int pinctrl_generic_set_state(struct udevice *pctldev, struct udevice *config);
int pinctrl_generic_set_state_prefix(struct udevice *pctldev, struct udevice *config,
				     const char *prefix);
#else
static inline int pinctrl_generic_set_state(struct udevice *pctldev,
					    struct udevice *config)
{
	return -ENOSYS;
}
#endif

#if CONFIG_IS_ENABLED(PINCTRL)
/**
 * pinctrl_select_state() - Set a device to a given state
 * @dev:	Peripheral device
 * @statename:	State name, like "default"
 *
 * Return: 0 on success, or negative error code on failure
 */
int pinctrl_select_state(struct udevice *dev, const char *statename);
#else
static inline int pinctrl_select_state(struct udevice *dev,
				       const char *statename)
{
	return -ENOSYS;
}
#endif

/**
 * pinctrl_request() - Request a particular pinctrl function
 * @dev:	Pinctrl device to use
 * @func:	Function number (driver-specific)
 * @flags:	Flags (driver-specific)
 *
 * Return: 0 if OK, or negative error code on failure
 */
int pinctrl_request(struct udevice *dev, int func, int flags);

/**
 * pinctrl_request_noflags() - Request a particular pinctrl function
 * @dev:	Pinctrl device to use
 * @func:	Function number (driver-specific)
 *
 * This is similar to pinctrl_request() but uses 0 for @flags.
 *
 * Return: 0 if OK, or negative error code on failure
 */
int pinctrl_request_noflags(struct udevice *dev, int func);

/**
 * pinctrl_get_periph_id() - Get the peripheral ID for a device
 * @dev:	Pinctrl device to use for decoding
 * @periph:	Device to check
 *
 * This generally looks at the peripheral's device tree node to work out the
 * peripheral ID. The return value is normally interpreted as enum periph_id.
 * so long as this is defined by the platform (which it should be).
 *
 * Return: Peripheral ID of @periph, or -ENOENT on error
 */
int pinctrl_get_periph_id(struct udevice *dev, struct udevice *periph);

/**
 * pinctrl_get_gpio_mux() - get the mux value for a particular GPIO
 * @dev:	Pinctrl device to use
 * @banknum:	GPIO bank number
 * @index:	GPIO index within the bank
 *
 * This allows the raw mux value for a GPIO to be obtained. It is
 * useful for displaying the function being used by that GPIO, such
 * as with the 'gpio' command. This function is internal to the GPIO
 * subsystem and should not be used by generic code. Typically it is
 * used by a GPIO driver with knowledge of the SoC pinctrl setup.
 *
 * Return: Mux value (SoC-specific, e.g. 0 for input, 1 for output)
*/
int pinctrl_get_gpio_mux(struct udevice *dev, int banknum, int index);

/**
 * pinctrl_get_pin_muxing() - Returns the muxing description
 * @dev:	Pinctrl device to use
 * @selector:	Pin index within pin-controller
 * @buf:	Pin's muxing description
 * @size:	Pin's muxing description length
 *
 * This allows to display the muxing description of the given pin for
 * debug purpose
 *
 * Return: 0 if OK, or negative error code on failure
 */
int pinctrl_get_pin_muxing(struct udevice *dev, int selector, char *buf,
			   int size);

/**
 * pinctrl_get_pins_count() - Display pin-controller pins number
 * @dev:	Pinctrl device to use
 *
 * This allows to know the number of pins owned by a given pin-controller
 *
 * Return: Number of pins if OK, or -ENOSYS when not supported
 */
int pinctrl_get_pins_count(struct udevice *dev);

/**
 * pinctrl_get_pin_name() - Returns the pin's name
 * @dev:	Pinctrl device to use
 * @selector:	Pin index within pin-controller
 * @buf:	Buffer to fill with the name of the pin
 * @size:	Size of @buf
 *
 * This allows to display the pin's name for debug purpose
 *
 * Return: 0 if OK, or negative error code on failure
 */
int pinctrl_get_pin_name(struct udevice *dev, int selector, char *buf,
			 int size);

/**
 * pinctrl_gpio_request() - Request a single pin to be used as GPIO
 * @dev:	GPIO peripheral device
 * @offset:	GPIO pin offset from the GPIO controller
 * @label:	GPIO label
 *
 * Return: 0 on success, or negative error code on failure
 */
int pinctrl_gpio_request(struct udevice *dev, unsigned offset, const char *label);

/**
 * pinctrl_gpio_free() - Free a single pin used as GPIO
 * @dev:	GPIO peripheral device
 * @offset:	GPIO pin offset from the GPIO controller
 *
 * Return: 0 on success, or negative error code on failure
 */
int pinctrl_gpio_free(struct udevice *dev, unsigned offset);

#endif /* __PINCTRL_H */
