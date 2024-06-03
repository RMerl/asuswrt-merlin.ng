/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2015  Masahiro Yamada <yamada.masahiro@socionext.com>
 */

#ifndef __PINCTRL_H
#define __PINCTRL_H

#define PINNAME_SIZE	10
#define PINMUX_SIZE	40

/**
 * struct pinconf_param - pin config parameters
 *
 * @property: property name in DT nodes
 * @param: ID for this config parameter
 * @default_value: default value for this config parameter used in case
 *	no value is specified in DT nodes
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
 * The @set_state is the only mandatory operation.  You can implement your
 * pinctrl driver with its own @set_state.  In this case, the other callbacks
 * are not required.  Otherwise, generic pinctrl framework is also available;
 * use pinctrl_generic_set_state for @set_state, and implement other operations
 * depending on your necessity.
 *
 * @get_pins_count: return number of selectable named pins available
 *	in this driver.  (necessary to parse "pins" property in DTS)
 * @get_pin_name: return the pin name of the pin selector,
 *	called by the core to figure out which pin it shall do
 *	operations to.  (necessary to parse "pins" property in DTS)
 * @get_groups_count: return number of selectable named groups available
 *	in this driver.  (necessary to parse "groups" property in DTS)
 * @get_group_name: return the group name of the group selector,
 *	called by the core to figure out which pin group it shall do
 *	operations to.  (necessary to parse "groups" property in DTS)
 * @get_functions_count: return number of selectable named functions available
 *	in this driver.  (necessary for pin-muxing)
 * @get_function_name: return the function name of the muxing selector,
 *	called by the core to figure out which mux setting it shall map a
 *	certain device to.  (necessary for pin-muxing)
 * @pinmux_set: enable a certain muxing function with a certain pin.
 *	The @func_selector selects a certain function whereas @pin_selector
 *	selects a certain pin to be used. On simple controllers one of them
 *	may be ignored.  (necessary for pin-muxing against a single pin)
 * @pinmux_group_set: enable a certain muxing function with a certain pin
 *	group.  The @func_selector selects a certain function whereas
 *	@group_selector selects a certain set of pins to be used. On simple
 *	controllers one of them may be ignored.
 *	(necessary for pin-muxing against a pin group)
 * @pinconf_num_params: number of driver-specific parameters to be parsed
 *	from device trees  (necessary for pin-configuration)
 * @pinconf_params: list of driver_specific parameters to be parsed from
 *	device trees  (necessary for pin-configuration)
 * @pinconf_set: configure an individual pin with a given parameter.
 *	(necessary for pin-configuration against a single pin)
 * @pinconf_group_set: configure all pins in a group with a given parameter.
 *	(necessary for pin-configuration against a pin group)
 * @set_state: do pinctrl operations specified by @config, a pseudo device
 *	pointing a config node. (necessary for pinctrl_full)
 * @set_state_simple: do needed pinctrl operations for a peripherl @periph.
 *	(necessary for pinctrl_simple)
 * @get_pin_muxing: display the muxing of a given pin.
 * @gpio_request_enable: requests and enables GPIO on a certain pin.
 *	Implement this only if you can mux every pin individually as GPIO. The
 *	affected GPIO range is passed along with an offset(pin number) into that
 *	specific GPIO range - function selectors and pin groups are orthogonal
 *	to this, the core will however make sure the pins do not collide.
 * @gpio_disable_free: free up GPIO muxing on a certain pin, the reverse of
 *	@gpio_request_enable
 */
struct pinctrl_ops {
	int (*get_pins_count)(struct udevice *dev);
	const char *(*get_pin_name)(struct udevice *dev, unsigned selector);
	int (*get_groups_count)(struct udevice *dev);
	const char *(*get_group_name)(struct udevice *dev, unsigned selector);
	int (*get_functions_count)(struct udevice *dev);
	const char *(*get_function_name)(struct udevice *dev,
					 unsigned selector);
	int (*pinmux_set)(struct udevice *dev, unsigned pin_selector,
			  unsigned func_selector);
	int (*pinmux_group_set)(struct udevice *dev, unsigned group_selector,
				unsigned func_selector);
	unsigned int pinconf_num_params;
	const struct pinconf_param *pinconf_params;
	int (*pinconf_set)(struct udevice *dev, unsigned pin_selector,
			   unsigned param, unsigned argument);
	int (*pinconf_group_set)(struct udevice *dev, unsigned group_selector,
				 unsigned param, unsigned argument);
	int (*set_state)(struct udevice *dev, struct udevice *config);

	/* for pinctrl-simple */
	int (*set_state_simple)(struct udevice *dev, struct udevice *periph);
	/**
	 * request() - Request a particular pinctrl function
	 *
	 * This activates the selected function.
	 *
	 * @dev:	Device to adjust (UCLASS_PINCTRL)
	 * @func:	Function number (driver-specific)
	 * @return 0 if OK, -ve on error
	 */
	int (*request)(struct udevice *dev, int func, int flags);

	/**
	* get_periph_id() - get the peripheral ID for a device
	*
	* This generally looks at the peripheral's device tree node to work
	* out the peripheral ID. The return value is normally interpreted as
	* enum periph_id. so long as this is defined by the platform (which it
	* should be).
	*
	* @dev:		Pinctrl device to use for decoding
	* @periph:	Device to check
	* @return peripheral ID of @periph, or -ENOENT on error
	*/
	int (*get_periph_id)(struct udevice *dev, struct udevice *periph);

	/**
	 * get_gpio_mux() - get the mux value for a particular GPIO
	 *
	 * This allows the raw mux value for a GPIO to be obtained. It is
	 * useful for displaying the function being used by that GPIO, such
	 * as with the 'gpio' command. This function is internal to the GPIO
	 * subsystem and should not be used by generic code. Typically it is
	 * used by a GPIO driver with knowledge of the SoC pinctrl setup.
	 *
	* @dev:		Pinctrl device to use
	* @banknum:	GPIO bank number
	* @index:	GPIO index within the bank
	* @return mux value (SoC-specific, e.g. 0 for input, 1 for output)
	 */
	int (*get_gpio_mux)(struct udevice *dev, int banknum, int index);

	/**
	 * get_pin_muxing() - show pin muxing
	 *
	 * This allows to display the muxing of a given pin. It's useful for
	 * debug purpose to know if a pin is configured as GPIO or as an
	 * alternate function and which one.
	 * Typically it is used by a PINCTRL driver with knowledge of the SoC
	 * pinctrl setup.
	 *
	 * @dev:	Pinctrl device to use
	 * @selector:	Pin selector
	 * @buf		Pin's muxing description
	 * @size	Pin's muxing description length
	 * return 0 if OK, -ve on error
	 */
	 int (*get_pin_muxing)(struct udevice *dev, unsigned int selector,
			       char *buf, int size);

	/**
	 * gpio_request_enable: requests and enables GPIO on a certain pin.
	 *
	 * @dev:	Pinctrl device to use
	 * @selector:	Pin selector
	 * return 0 if OK, -ve on error
	 */
	int (*gpio_request_enable)(struct udevice *dev, unsigned int selector);

	/**
	 * gpio_disable_free: free up GPIO muxing on a certain pin.
	 *
	 * @dev:	Pinctrl device to use
	 * @selector:	Pin selector
	 * return 0 if OK, -ve on error
	 */
	int (*gpio_disable_free)(struct udevice *dev, unsigned int selector);
};

#define pinctrl_get_ops(dev)	((struct pinctrl_ops *)(dev)->driver->ops)

/**
 * Generic pin configuration paramters
 *
 * enum pin_config_param - possible pin configuration parameters
 * @PIN_CONFIG_BIAS_BUS_HOLD: the pin will be set to weakly latch so that it
 *	weakly drives the last value on a tristate bus, also known as a "bus
 *	holder", "bus keeper" or "repeater". This allows another device on the
 *	bus to change the value by driving the bus high or low and switching to
 *	tristate. The argument is ignored.
 * @PIN_CONFIG_BIAS_DISABLE: disable any pin bias on the pin, a
 *	transition from say pull-up to pull-down implies that you disable
 *	pull-up in the process, this setting disables all biasing.
 * @PIN_CONFIG_BIAS_HIGH_IMPEDANCE: the pin will be set to a high impedance
 *	mode, also know as "third-state" (tristate) or "high-Z" or "floating".
 *	On output pins this effectively disconnects the pin, which is useful
 *	if for example some other pin is going to drive the signal connected
 *	to it for a while. Pins used for input are usually always high
 *	impedance.
 * @PIN_CONFIG_BIAS_PULL_DOWN: the pin will be pulled down (usually with high
 *	impedance to GROUND). If the argument is != 0 pull-down is enabled,
 *	if it is 0, pull-down is total, i.e. the pin is connected to GROUND.
 * @PIN_CONFIG_BIAS_PULL_PIN_DEFAULT: the pin will be pulled up or down based
 *	on embedded knowledge of the controller hardware, like current mux
 *	function. The pull direction and possibly strength too will normally
 *	be decided completely inside the hardware block and not be readable
 *	from the kernel side.
 *	If the argument is != 0 pull up/down is enabled, if it is 0, the
 *	configuration is ignored. The proper way to disable it is to use
 *	@PIN_CONFIG_BIAS_DISABLE.
 * @PIN_CONFIG_BIAS_PULL_UP: the pin will be pulled up (usually with high
 *	impedance to VDD). If the argument is != 0 pull-up is enabled,
 *	if it is 0, pull-up is total, i.e. the pin is connected to VDD.
 * @PIN_CONFIG_DRIVE_OPEN_DRAIN: the pin will be driven with open drain (open
 *	collector) which means it is usually wired with other output ports
 *	which are then pulled up with an external resistor. Setting this
 *	config will enable open drain mode, the argument is ignored.
 * @PIN_CONFIG_DRIVE_OPEN_SOURCE: the pin will be driven with open source
 *	(open emitter). Setting this config will enable open source mode, the
 *	argument is ignored.
 * @PIN_CONFIG_DRIVE_PUSH_PULL: the pin will be driven actively high and
 *	low, this is the most typical case and is typically achieved with two
 *	active transistors on the output. Setting this config will enable
 *	push-pull mode, the argument is ignored.
 * @PIN_CONFIG_DRIVE_STRENGTH: the pin will sink or source at most the current
 *	passed as argument. The argument is in mA.
 * @PIN_CONFIG_DRIVE_STRENGTH_UA: the pin will sink or source at most the current
 *	passed as argument. The argument is in uA.
 * @PIN_CONFIG_INPUT_DEBOUNCE: this will configure the pin to debounce mode,
 *	which means it will wait for signals to settle when reading inputs. The
 *	argument gives the debounce time in usecs. Setting the
 *	argument to zero turns debouncing off.
 * @PIN_CONFIG_INPUT_ENABLE: enable the pin's input.  Note that this does not
 *	affect the pin's ability to drive output.  1 enables input, 0 disables
 *	input.
 * @PIN_CONFIG_INPUT_SCHMITT: this will configure an input pin to run in
 *	schmitt-trigger mode. If the schmitt-trigger has adjustable hysteresis,
 *	the threshold value is given on a custom format as argument when
 *	setting pins to this mode.
 * @PIN_CONFIG_INPUT_SCHMITT_ENABLE: control schmitt-trigger mode on the pin.
 *      If the argument != 0, schmitt-trigger mode is enabled. If it's 0,
 *      schmitt-trigger mode is disabled.
 * @PIN_CONFIG_LOW_POWER_MODE: this will configure the pin for low power
 *	operation, if several modes of operation are supported these can be
 *	passed in the argument on a custom form, else just use argument 1
 *	to indicate low power mode, argument 0 turns low power mode off.
 * @PIN_CONFIG_OUTPUT_ENABLE: this will enable the pin's output mode
 *	without driving a value there. For most platforms this reduces to
 *	enable the output buffers and then let the pin controller current
 *	configuration (eg. the currently selected mux function) drive values on
 *	the line. Use argument 1 to enable output mode, argument 0 to disable
 *	it.
 * @PIN_CONFIG_OUTPUT: this will configure the pin as an output and drive a
 *	value on the line. Use argument 1 to indicate high level, argument 0 to
 *	indicate low level. (Please see Documentation/driver-api/pinctl.rst,
 *	section "GPIO mode pitfalls" for a discussion around this parameter.)
 * @PIN_CONFIG_POWER_SOURCE: if the pin can select between different power
 *	supplies, the argument to this parameter (on a custom format) tells
 *	the driver which alternative power source to use.
 * @PIN_CONFIG_SLEEP_HARDWARE_STATE: indicate this is sleep related state.
 * @PIN_CONFIG_SLEW_RATE: if the pin can select slew rate, the argument to
 *	this parameter (on a custom format) tells the driver which alternative
 *	slew rate to use.
 * @PIN_CONFIG_SKEW_DELAY: if the pin has programmable skew rate (on inputs)
 *	or latch delay (on outputs) this parameter (in a custom format)
 *	specifies the clock skew or latch delay. It typically controls how
 *	many double inverters are put in front of the line.
 * @PIN_CONFIG_END: this is the last enumerator for pin configurations, if
 *	you need to pass in custom configurations to the pin controller, use
 *	PIN_CONFIG_END+1 as the base offset.
 * @PIN_CONFIG_MAX: this is the maximum configuration value that can be
 *	presented using the packed format.
 */
enum pin_config_param {
	PIN_CONFIG_BIAS_BUS_HOLD,
	PIN_CONFIG_BIAS_DISABLE,
	PIN_CONFIG_BIAS_HIGH_IMPEDANCE,
	PIN_CONFIG_BIAS_PULL_DOWN,
	PIN_CONFIG_BIAS_PULL_PIN_DEFAULT,
	PIN_CONFIG_BIAS_PULL_UP,
	PIN_CONFIG_DRIVE_OPEN_DRAIN,
	PIN_CONFIG_DRIVE_OPEN_SOURCE,
	PIN_CONFIG_DRIVE_PUSH_PULL,
	PIN_CONFIG_DRIVE_STRENGTH,
	PIN_CONFIG_DRIVE_STRENGTH_UA,
	PIN_CONFIG_INPUT_DEBOUNCE,
	PIN_CONFIG_INPUT_ENABLE,
	PIN_CONFIG_INPUT_SCHMITT,
	PIN_CONFIG_INPUT_SCHMITT_ENABLE,
	PIN_CONFIG_LOW_POWER_MODE,
	PIN_CONFIG_OUTPUT_ENABLE,
	PIN_CONFIG_OUTPUT,
	PIN_CONFIG_POWER_SOURCE,
	PIN_CONFIG_SLEEP_HARDWARE_STATE,
	PIN_CONFIG_SLEW_RATE,
	PIN_CONFIG_SKEW_DELAY,
	PIN_CONFIG_END = 0x7F,
	PIN_CONFIG_MAX = 0xFF,
};

#if CONFIG_IS_ENABLED(PINCTRL_GENERIC)
/**
 * pinctrl_generic_set_state() - generic set_state operation
 * Parse the DT node of @config and its children and handle generic properties
 * such as "pins", "groups", "functions", and pin configuration parameters.
 *
 * @pctldev: pinctrl device
 * @config: config device (pseudo device), pointing a config node in DTS
 * @return: 0 on success, or negative error code on failure
 */
int pinctrl_generic_set_state(struct udevice *pctldev, struct udevice *config);
#else
static inline int pinctrl_generic_set_state(struct udevice *pctldev,
					    struct udevice *config)
{
	return -EINVAL;
}
#endif

#if CONFIG_IS_ENABLED(PINCTRL)
/**
 * pinctrl_select_state() - set a device to a given state
 *
 * @dev: peripheral device
 * @statename: state name, like "default"
 * @return: 0 on success, or negative error code on failure
 */
int pinctrl_select_state(struct udevice *dev, const char *statename);
#else
static inline int pinctrl_select_state(struct udevice *dev,
				       const char *statename)
{
	return -EINVAL;
}
#endif

/**
 * pinctrl_request() - Request a particular pinctrl function
 *
 * @dev:	Device to check (UCLASS_PINCTRL)
 * @func:	Function number (driver-specific)
 * @flags:	Flags (driver-specific)
 * @return 0 if OK, -ve on error
 */
int pinctrl_request(struct udevice *dev, int func, int flags);

/**
 * pinctrl_request_noflags() - Request a particular pinctrl function
 *
 * This is similar to pinctrl_request() but uses 0 for @flags.
 *
 * @dev:	Device to check (UCLASS_PINCTRL)
 * @func:	Function number (driver-specific)
 * @return 0 if OK, -ve on error
 */
int pinctrl_request_noflags(struct udevice *dev, int func);

/**
 * pinctrl_get_periph_id() - get the peripheral ID for a device
 *
 * This generally looks at the peripheral's device tree node to work out the
 * peripheral ID. The return value is normally interpreted as enum periph_id.
 * so long as this is defined by the platform (which it should be).
 *
 * @dev:	Pinctrl device to use for decoding
 * @periph:	Device to check
 * @return peripheral ID of @periph, or -ENOENT on error
 */
int pinctrl_get_periph_id(struct udevice *dev, struct udevice *periph);

/**
 * pinctrl_decode_pin_config() - decode pin configuration flags
 *
 * This decodes some of the PIN_CONFIG values into flags, with each value
 * being (1 << pin_cfg). This does not support things with values like the
 * slew rate.
 *
 * @blob:	Device tree blob
 * @node:	Node containing the PIN_CONFIG values
 * @return decoded flag value, or -ve on error
 */
int pinctrl_decode_pin_config(const void *blob, int node);

/**
 * pinctrl_get_gpio_mux() - get the mux value for a particular GPIO
 *
 * This allows the raw mux value for a GPIO to be obtained. It is
 * useful for displaying the function being used by that GPIO, such
 * as with the 'gpio' command. This function is internal to the GPIO
 * subsystem and should not be used by generic code. Typically it is
 * used by a GPIO driver with knowledge of the SoC pinctrl setup.
 *
 * @dev:	Pinctrl device to use
 * @banknum:	GPIO bank number
 * @index:	GPIO index within the bank
 * @return mux value (SoC-specific, e.g. 0 for input, 1 for output)
*/
int pinctrl_get_gpio_mux(struct udevice *dev, int banknum, int index);

/**
 * pinctrl_get_pin_muxing() - Returns the muxing description
 *
 * This allows to display the muxing description of the given pin for
 * debug purpose
 *
 * @dev:	Pinctrl device to use
 * @selector	Pin index within pin-controller
 * @buf		Pin's muxing description
 * @size	Pin's muxing description length
 * @return 0 if OK, -ve on error
 */
int pinctrl_get_pin_muxing(struct udevice *dev, int selector, char *buf,
			   int size);

/**
 * pinctrl_get_pins_count() - display pin-controller pins number
 *
 * This allows to know the number of pins owned by a given pin-controller
 *
 * @dev:	Pinctrl device to use
 * @return pins number if OK, -ve on error
 */
int pinctrl_get_pins_count(struct udevice *dev);

/**
 * pinctrl_get_pin_name() - Returns the pin's name
 *
 * This allows to display the pin's name for debug purpose
 *
 * @dev:	Pinctrl device to use
 * @selector	Pin index within pin-controller
 * @buf		Pin's name
 * @return 0 if OK, -ve on error
 */
int pinctrl_get_pin_name(struct udevice *dev, int selector, char *buf,
			 int size);

/**
 * pinctrl_gpio_request() - request a single pin to be used as GPIO
 *
 * @dev: GPIO peripheral device
 * @offset: the GPIO pin offset from the GPIO controller
 * @return: 0 on success, or negative error code on failure
 */
int pinctrl_gpio_request(struct udevice *dev, unsigned offset);

/**
 * pinctrl_gpio_free() - free a single pin used as GPIO
 *
 * @dev: GPIO peripheral device
 * @offset: the GPIO pin offset from the GPIO controller
 * @return: 0 on success, or negative error code on failure
 */
int pinctrl_gpio_free(struct udevice *dev, unsigned offset);

#endif /* __PINCTRL_H */
