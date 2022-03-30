/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2011 The Chromium OS Authors.
 * Copyright (c) 2011, NVIDIA Corp. All rights reserved.
 */

#ifndef _ASM_GENERIC_GPIO_H_
#define _ASM_GENERIC_GPIO_H_

#include <dm/ofnode.h>

struct ofnode_phandle_args;

/*
 * Generic GPIO API for U-Boot
 *
 * --
 * NB: This is deprecated. Please use the driver model functions instead:
 *
 *    - gpio_request_by_name()
 *    - dm_gpio_get_value() etc.
 *
 * For now we need a dm_ prefix on some functions to avoid name collision.
 * --
 *
 * GPIOs are numbered from 0 to GPIO_COUNT-1 which value is defined
 * by the SOC/architecture.
 *
 * Each GPIO can be an input or output. If an input then its value can
 * be read as 0 or 1. If an output then its value can be set to 0 or 1.
 * If you try to write an input then the value is undefined. If you try
 * to read an output, barring something very unusual,  you will get
 * back the value of the output that you previously set.
 *
 * In some cases the operation may fail, for example if the GPIO number
 * is out of range, or the GPIO is not available because its pin is
 * being used by another function. In that case, functions may return
 * an error value of -1.
 */

/**
 * @deprecated	Please use driver model instead
 * Request a GPIO. This should be called before any of the other functions
 * are used on this GPIO.
 *
 * Note: With driver model, the label is allocated so there is no need for
 * the caller to preserve it.
 *
 * @param gpio	GPIO number
 * @param label	User label for this GPIO
 * @return 0 if ok, -1 on error
 */
int gpio_request(unsigned gpio, const char *label);

/**
 * @deprecated	Please use driver model instead
 * Stop using the GPIO.  This function should not alter pin configuration.
 *
 * @param gpio	GPIO number
 * @return 0 if ok, -1 on error
 */
int gpio_free(unsigned gpio);

/**
 * @deprecated	Please use driver model instead
 * Make a GPIO an input.
 *
 * @param gpio	GPIO number
 * @return 0 if ok, -1 on error
 */
int gpio_direction_input(unsigned gpio);

/**
 * @deprecated	Please use driver model instead
 * Make a GPIO an output, and set its value.
 *
 * @param gpio	GPIO number
 * @param value	GPIO value (0 for low or 1 for high)
 * @return 0 if ok, -1 on error
 */
int gpio_direction_output(unsigned gpio, int value);

/**
 * @deprecated	Please use driver model instead
 * Get a GPIO's value. This will work whether the GPIO is an input
 * or an output.
 *
 * @param gpio	GPIO number
 * @return 0 if low, 1 if high, -1 on error
 */
int gpio_get_value(unsigned gpio);

/**
 * @deprecated	Please use driver model instead
 * Set an output GPIO's value. The GPIO must already be an output or
 * this function may have no effect.
 *
 * @param gpio	GPIO number
 * @param value	GPIO value (0 for low or 1 for high)
 * @return 0 if ok, -1 on error
 */
int gpio_set_value(unsigned gpio, int value);

/* State of a GPIO, as reported by get_function() */
enum gpio_func_t {
	GPIOF_INPUT = 0,
	GPIOF_OUTPUT,
	GPIOF_UNUSED,		/* Not claimed */
	GPIOF_UNKNOWN,		/* Not known */
	GPIOF_FUNC,		/* Not used as a GPIO */

	GPIOF_COUNT,
};

struct udevice;

struct gpio_desc {
	struct udevice *dev;	/* Device, NULL for invalid GPIO */
	unsigned long flags;
#define GPIOD_REQUESTED		(1 << 0)	/* Requested/claimed */
#define GPIOD_IS_OUT		(1 << 1)	/* GPIO is an output */
#define GPIOD_IS_IN		(1 << 2)	/* GPIO is an input */
#define GPIOD_ACTIVE_LOW	(1 << 3)	/* value has active low */
#define GPIOD_IS_OUT_ACTIVE	(1 << 4)	/* set output active */

	uint offset;		/* GPIO offset within the device */
	/*
	 * We could consider adding the GPIO label in here. Possibly we could
	 * use this structure for internal GPIO information.
	 */
};

/**
 * dm_gpio_is_valid() - Check if a GPIO is valid
 *
 * @desc:	GPIO description containing device, offset and flags,
 *		previously returned by gpio_request_by_name()
 * @return true if valid, false if not
 */
static inline bool dm_gpio_is_valid(const struct gpio_desc *desc)
{
	return desc->dev != NULL;
}

/**
 * gpio_get_status() - get the current GPIO status as a string
 *
 * Obtain the current GPIO status as a string which can be presented to the
 * user. A typical string is:
 *
 * "b4:  in: 1 [x] sdmmc_cd"
 *
 * which means this is GPIO bank b, offset 4, currently set to input, current
 * value 1, [x] means that it is requested and the owner is 'sdmmc_cd'
 *
 * TODO(sjg@chromium.org): This should use struct gpio_desc
 *
 * @dev:	Device to check
 * @offset:	Offset of device GPIO to check
 * @buf:	Place to put string
 * @buffsize:	Size of string including \0
 */
int gpio_get_status(struct udevice *dev, int offset, char *buf, int buffsize);

/**
 * gpio_get_function() - get the current function for a GPIO pin
 *
 * Note this returns GPIOF_UNUSED if the GPIO is not requested.
 *
 * TODO(sjg@chromium.org): This should use struct gpio_desc
 *
 * @dev:	Device to check
 * @offset:	Offset of device GPIO to check
 * @namep:	If non-NULL, this is set to the name given when the GPIO
 *		was requested, or -1 if it has not been requested
 * @return  -ENODATA if the driver returned an unknown function,
 * -ENODEV if the device is not active, -EINVAL if the offset is invalid.
 * GPIOF_UNUSED if the GPIO has not been requested. Otherwise returns the
 * function from enum gpio_func_t.
 */
int gpio_get_function(struct udevice *dev, int offset, const char **namep);

/**
 * gpio_get_raw_function() - get the current raw function for a GPIO pin
 *
 * Note this does not return GPIOF_UNUSED - it will always return the GPIO
 * driver's view of a pin function, even if it is not correctly set up.
 *
 * TODO(sjg@chromium.org): This should use struct gpio_desc
 *
 * @dev:	Device to check
 * @offset:	Offset of device GPIO to check
 * @namep:	If non-NULL, this is set to the name given when the GPIO
 *		was requested, or -1 if it has not been requested
 * @return  -ENODATA if the driver returned an unknown function,
 * -ENODEV if the device is not active, -EINVAL if the offset is invalid.
 * Otherwise returns the function from enum gpio_func_t.
 */
int gpio_get_raw_function(struct udevice *dev, int offset, const char **namep);

/**
 * gpio_requestf() - request a GPIO using a format string for the owner
 *
 * This is a helper function for gpio_request(). It allows you to provide
 * a printf()-format string for the GPIO owner. It calls gpio_request() with
 * the string that is created
 */
int gpio_requestf(unsigned gpio, const char *fmt, ...)
		__attribute__ ((format (__printf__, 2, 3)));

struct fdtdec_phandle_args;

/**
 * gpio_xlate_offs_flags() - implementation for common use of dm_gpio_ops.xlate
 *
 * This routine sets the offset field to args[0] and the flags field to
 * GPIOD_ACTIVE_LOW if the GPIO_ACTIVE_LOW flag is present in args[1].
 */
int gpio_xlate_offs_flags(struct udevice *dev, struct gpio_desc *desc,
			  struct ofnode_phandle_args *args);

/**
 * struct struct dm_gpio_ops - Driver model GPIO operations
 *
 * Refer to functions above for description. These function largely copy
 * the old API.
 *
 * This is trying to be close to Linux GPIO API. Once the U-Boot uses the
 * new DM GPIO API, this should be really easy to flip over to the Linux
 * GPIO API-alike interface.
 *
 * Also it would be useful to standardise additional functions like
 * pullup, slew rate and drive strength.
 *
 * gpio_request() and gpio_free() are optional - if NULL then they will
 * not be called.
 *
 * Note that @offset is the offset from the base GPIO of the device. So
 * offset 0 is the device's first GPIO and offset o-1 is the last GPIO,
 * where o is the number of GPIO lines controlled by the device. A device
 * is typically used to control a single bank of GPIOs. Within complex
 * SoCs there may be many banks and therefore many devices all referring
 * to the different IO addresses within the SoC.
 *
 * The uclass combines all GPIO devices together to provide a consistent
 * numbering from 0 to n-1, where n is the number of GPIOs in total across
 * all devices. Be careful not to confuse offset with gpio in the parameters.
 */
struct dm_gpio_ops {
	int (*request)(struct udevice *dev, unsigned offset, const char *label);
	int (*free)(struct udevice *dev, unsigned offset);
	int (*direction_input)(struct udevice *dev, unsigned offset);
	int (*direction_output)(struct udevice *dev, unsigned offset,
				int value);
	int (*get_value)(struct udevice *dev, unsigned offset);
	int (*set_value)(struct udevice *dev, unsigned offset, int value);
	int (*get_open_drain)(struct udevice *dev, unsigned offset);
	int (*set_open_drain)(struct udevice *dev, unsigned offset, int value);
	/**
	 * get_function() Get the GPIO function
	 *
	 * @dev:     Device to check
	 * @offset:  GPIO offset within that device
	 * @return current function - GPIOF_...
	 */
	int (*get_function)(struct udevice *dev, unsigned offset);

	/**
	 * xlate() - Translate phandle arguments into a GPIO description
	 *
	 * This function should set up the fields in desc according to the
	 * information in the arguments. The uclass will have set up:
	 *
	 *   @desc->dev to @dev
	 *   @desc->flags to 0
	 *   @desc->offset to 0
	 *
	 * This method is optional and defaults to gpio_xlate_offs_flags,
	 * which will parse offset and the GPIO_ACTIVE_LOW flag in the first
	 * two arguments.
	 *
	 * Note that @dev is passed in as a parameter to follow driver model
	 * uclass conventions, even though it is already available as
	 * desc->dev.
	 *
	 * @dev:	GPIO device
	 * @desc:	Place to put GPIO description
	 * @args:	Arguments provided in description
	 * @return 0 if OK, -ve on error
	 */
	int (*xlate)(struct udevice *dev, struct gpio_desc *desc,
		     struct ofnode_phandle_args *args);
};

/**
 * struct gpio_dev_priv - information about a device used by the uclass
 *
 * The uclass combines all active GPIO devices into a unified numbering
 * scheme. To do this it maintains some private information about each
 * device.
 *
 * To implement driver model support in your GPIO driver, add a probe
 * handler, and set @gpio_count and @bank_name correctly in that handler.
 * This tells the uclass the name of the GPIO bank and the number of GPIOs
 * it contains.
 *
 * @bank_name: Name of the GPIO device (e.g 'a' means GPIOs will be called
 * 'A0', 'A1', etc.
 * @gpio_count: Number of GPIOs in this device
 * @gpio_base: Base GPIO number for this device. For the first active device
 * this will be 0; the numbering for others will follow sequentially so that
 * @gpio_base for device 1 will equal the number of GPIOs in device 0.
 * @name: Array of pointers to the name for each GPIO in this bank. The
 * value of the pointer will be NULL if the GPIO has not been claimed.
 */
struct gpio_dev_priv {
	const char *bank_name;
	unsigned gpio_count;
	unsigned gpio_base;
	char **name;
};

/* Access the GPIO operations for a device */
#define gpio_get_ops(dev)	((struct dm_gpio_ops *)(dev)->driver->ops)

/**
 * gpio_get_bank_info - Return information about a GPIO bank/device
 *
 * This looks up a device and returns both its GPIO base name and the number
 * of GPIOs it controls.
 *
 * @dev: Device to look up
 * @offset_count: Returns number of GPIOs within this bank
 * @return bank name of this device
 */
const char *gpio_get_bank_info(struct udevice *dev, int *offset_count);

/**
 * dm_gpio_lookup_name() - Look up a named GPIO and return its description
 *
 * The name of a GPIO is typically its bank name followed by a number from 0.
 * For example A0 is the first GPIO in bank A. Each bank is a separate driver
 * model device.
 *
 * @name:	Name to look up
 * @desc:	Returns description, on success
 * @return 0 if OK, -ve on error
 */
int dm_gpio_lookup_name(const char *name, struct gpio_desc *desc);

/**
 * gpio_lookup_name - Look up a GPIO name and return its details
 *
 * This is used to convert a named GPIO into a device, offset and GPIO
 * number.
 *
 * @name: GPIO name to look up
 * @devp: Returns pointer to device which contains this GPIO
 * @offsetp: Returns the offset number within this device
 * @gpiop: Returns the absolute GPIO number, numbered from 0
 */
int gpio_lookup_name(const char *name, struct udevice **devp,
		     unsigned int *offsetp, unsigned int *gpiop);

/**
 * gpio_get_values_as_int() - Turn the values of a list of GPIOs into an int
 *
 * This puts the value of the first GPIO into bit 0, the second into bit 1,
 * etc. then returns the resulting integer.
 *
 * @gpio_list: List of GPIOs to collect
 * @return resulting integer value, or -ve on error
 */
int gpio_get_values_as_int(const int *gpio_list);

/**
 * dm_gpio_get_values_as_int() - Turn the values of a list of GPIOs into an int
 *
 * This puts the value of the first GPIO into bit 0, the second into bit 1,
 * etc. then returns the resulting integer.
 *
 * @desc_list: List of GPIOs to collect
 * @count: Number of GPIOs
 * @return resulting integer value, or -ve on error
 */
int dm_gpio_get_values_as_int(const struct gpio_desc *desc_list, int count);

/**
 * gpio_claim_vector() - claim a number of GPIOs for input
 *
 * @gpio_num_array:	array of gpios to claim, terminated by -1
 * @fmt:		format string for GPIO names, e.g. "board_id%d"
 * @return 0 if OK, -ve on error
 */
int gpio_claim_vector(const int *gpio_num_array, const char *fmt);

/**
 * gpio_request_by_name() - Locate and request a GPIO by name
 *
 * This operates by looking up the given list name in the device (device
 * tree property) and requesting the GPIO for use. The property must exist
 * in @dev's node.
 *
 * Use @flags to specify whether the GPIO should be an input or output. In
 * principle this can also come from the device tree binding but most
 * bindings don't provide this information. Specifically, when the GPIO uclass
 * calls the xlate() method, it can return default flags, which are then
 * ORed with this @flags.
 *
 * If we find that requesting the GPIO is not always needed we could add a
 * new function or a new GPIOD_NO_REQUEST flag.
 *
 * At present driver model has no reference counting so if one device
 * requests a GPIO which subsequently is unbound, the @desc->dev pointer
 * will be invalid. However this will only happen if the GPIO device is
 * unbound, not if it is removed, so this seems like a reasonable limitation
 * for now. There is no real use case for unbinding drivers in normal
 * operation.
 *
 * The device tree binding is doc/device-tree-bindings/gpio/gpio.txt in
 * generate terms and each specific device may add additional details in
 * a binding file in the same directory.
 *
 * @dev:	Device requesting the GPIO
 * @list_name:	Name of GPIO list (e.g. "board-id-gpios")
 * @index:	Index number of the GPIO in that list use request (0=first)
 * @desc:	Returns GPIO description information. If there is no such
 *		GPIO, dev->dev will be NULL.
 * @flags:	Indicates the GPIO input/output settings (GPIOD_...)
 * @return 0 if OK, -ENOENT if the GPIO does not exist, -EINVAL if there is
 * something wrong with the list, or other -ve for another error (e.g.
 * -EBUSY if a GPIO was already requested)
 */
int gpio_request_by_name(struct udevice *dev, const char *list_name,
			 int index, struct gpio_desc *desc, int flags);

/**
 * gpio_request_list_by_name() - Request a list of GPIOs
 *
 * Reads all the GPIOs from a list and requests them. See
 * gpio_request_by_name() for additional details. Lists should not be
 * misused to hold unrelated or optional GPIOs. They should only be used
 * for things like parallel data lines. A zero phandle terminates the list
 * the list.
 *
 * This function will either succeed, and request all GPIOs in the list, or
 * fail and request none (it will free already-requested GPIOs in case of
 * an error part-way through).
 *
 * @dev:	Device requesting the GPIO
 * @list_name:	Name of GPIO list (e.g. "board-id-gpios")
 * @desc_list:	Returns a list of GPIO description information
 * @max_count:	Maximum number of GPIOs to return (@desc_list must be at least
 *		this big)
 * @flags:	Indicates the GPIO input/output settings (GPIOD_...)
 * @return number of GPIOs requested, or -ve on error
 */
int gpio_request_list_by_name(struct udevice *dev, const char *list_name,
			      struct gpio_desc *desc_list, int max_count,
			      int flags);

/**
 * dm_gpio_request() - manually request a GPIO
 *
 * Note: This function should only be used for testing / debugging. Instead.
 * use gpio_request_by_name() to pull GPIOs from the device tree.
 *
 * @desc:	GPIO description of GPIO to request (see dm_gpio_lookup_name())
 * @label:	Label to attach to the GPIO while claimed
 * @return 0 if OK, -ve on error
 */
int dm_gpio_request(struct gpio_desc *desc, const char *label);

/**
 * gpio_get_list_count() - Returns the number of GPIOs in a list
 *
 * Counts the GPIOs in a list. See gpio_request_by_name() for additional
 * details.
 *
 * @dev:	Device requesting the GPIO
 * @list_name:	Name of GPIO list (e.g. "board-id-gpios")
 * @return number of GPIOs (0 for an empty property) or -ENOENT if the list
 * does not exist
 */
int gpio_get_list_count(struct udevice *dev, const char *list_name);

/**
 * gpio_request_by_name_nodev() - request GPIOs without a device
 *
 * This is a version of gpio_request_list_by_name() that does not use a
 * device. Avoid it unless the caller is not yet using driver model
 */
int gpio_request_by_name_nodev(ofnode node, const char *list_name, int index,
			       struct gpio_desc *desc, int flags);

/**
 * gpio_request_list_by_name_nodev() - request GPIOs without a device
 *
 * This is a version of gpio_request_list_by_name() that does not use a
 * device. Avoid it unless the caller is not yet using driver model
 */
int gpio_request_list_by_name_nodev(ofnode node, const char *list_name,
				    struct gpio_desc *desc_list, int max_count,
				    int flags);

/**
 * dm_gpio_free() - Free a single GPIO
 *
 * This frees a single GPIOs previously returned from gpio_request_by_name().
 *
 * @dev:	Device which requested the GPIO
 * @desc:	GPIO to free
 * @return 0 if OK, -ve on error
 */
int dm_gpio_free(struct udevice *dev, struct gpio_desc *desc);

/**
 * gpio_free_list() - Free a list of GPIOs
 *
 * This frees a list of GPIOs previously returned from
 * gpio_request_list_by_name().
 *
 * @dev:	Device which requested the GPIOs
 * @desc:	List of GPIOs to free
 * @count:	Number of GPIOs in the list
 * @return 0 if OK, -ve on error
 */
int gpio_free_list(struct udevice *dev, struct gpio_desc *desc, int count);

/**
 * gpio_free_list_nodev() - free GPIOs without a device
 *
 * This is a version of gpio_free_list() that does not use a
 * device. Avoid it unless the caller is not yet using driver model
 */
int gpio_free_list_nodev(struct gpio_desc *desc, int count);

/**
 * dm_gpio_get_value() - Get the value of a GPIO
 *
 * This is the driver model version of the existing gpio_get_value() function
 * and should be used instead of that.
 *
 * For now, these functions have a dm_ prefix since they conflict with
 * existing names.
 *
 * @desc:	GPIO description containing device, offset and flags,
 *		previously returned by gpio_request_by_name()
 * @return GPIO value (0 for inactive, 1 for active) or -ve on error
 */
int dm_gpio_get_value(const struct gpio_desc *desc);

int dm_gpio_set_value(const struct gpio_desc *desc, int value);

/**
 * dm_gpio_get_open_drain() - Check if open-drain-mode of a GPIO is active
 *
 * This checks if open-drain-mode for a GPIO is enabled or not. This method is
 * optional.
 *
 * @desc:	GPIO description containing device, offset and flags,
 *		previously returned by gpio_request_by_name()
 * @return Value of open drain mode for GPIO (0 for inactive, 1 for active) or
 *	   -ve on error
 */
int dm_gpio_get_open_drain(struct gpio_desc *desc);

/**
 * dm_gpio_set_open_drain() - Switch open-drain-mode of a GPIO on or off
 *
 * This enables or disables open-drain mode for a GPIO. This method is
 * optional; if the driver does not support it, nothing happens when the method
 * is called.
 *
 * In open-drain mode, instead of actively driving the output (Push-pull
 * output), the GPIO's pin is connected to the collector (for a NPN transistor)
 * or the drain (for a MOSFET) of a transistor, respectively. The pin then
 * either forms an open circuit or a connection to ground, depending on the
 * state of the transistor.
 *
 * @desc:	GPIO description containing device, offset and flags,
 *		previously returned by gpio_request_by_name()
 * @return 0 if OK, -ve on error
 */
int dm_gpio_set_open_drain(struct gpio_desc *desc, int value);

/**
 * dm_gpio_set_dir() - Set the direction for a GPIO
 *
 * This sets up the direction according tot the provided flags. It will do
 * nothing unless the direction is actually specified.
 *
 * @desc:	GPIO description containing device, offset and flags,
 *		previously returned by gpio_request_by_name()
 * @return 0 if OK, -ve on error
 */
int dm_gpio_set_dir(struct gpio_desc *desc);

/**
 * dm_gpio_set_dir_flags() - Set direction using specific flags
 *
 * This is like dm_gpio_set_dir() except that the flags value is provided
 * instead of being used from desc->flags. This is needed because in many
 * cases the GPIO description does not include direction information.
 * Note that desc->flags is updated by this function.
 *
 * @desc:	GPIO description containing device, offset and flags,
 *		previously returned by gpio_request_by_name()
 * @flags:	New flags to use
 * @return 0 if OK, -ve on error, in which case desc->flags is not updated
 */
int dm_gpio_set_dir_flags(struct gpio_desc *desc, ulong flags);

/**
 * gpio_get_number() - Get the global GPIO number of a GPIO
 *
 * This should only be used for debugging or interest. It returns the number
 * that should be used for gpio_get_value() etc. to access this GPIO.
 *
 * @desc:	GPIO description containing device, offset and flags,
 *		previously returned by gpio_request_by_name()
 * @return GPIO number, or -ve if not found
 */
int gpio_get_number(const struct gpio_desc *desc);

#endif	/* _ASM_GENERIC_GPIO_H_ */
