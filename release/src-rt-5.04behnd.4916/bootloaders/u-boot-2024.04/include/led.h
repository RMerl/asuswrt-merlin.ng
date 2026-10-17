/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2015 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#ifndef __LED_H
#define __LED_H

struct udevice;

enum led_state_t {
	LEDST_OFF = 0,
	LEDST_ON = 1,
	LEDST_TOGGLE,
#ifdef CONFIG_LED_BLINK
	LEDST_BLINK,
#endif

	LEDST_COUNT,
};

/**
 * struct led_uc_plat - Platform data the uclass stores about each device
 *
 * @label:	LED label
 * @default_state:	LED default state
 */
struct led_uc_plat {
	const char *label;
	enum led_state_t default_state;
};

/**
 * struct led_uc_priv - Private data the uclass stores about each device
 *
 * @period_ms:	Flash period in milliseconds
 */
struct led_uc_priv {
	int period_ms;
};

struct led_ops {
	/**
	 * set_state() - set the state of an LED
	 *
	 * @dev:	LED device to change
	 * @state:	LED state to set
	 * @return 0 if OK, -ve on error
	 */
	int (*set_state)(struct udevice *dev, enum led_state_t state);

	/**
	 * led_get_state() - get the state of an LED
	 *
	 * @dev:	LED device to change
	 * @return LED state led_state_t, or -ve on error
	 */
	enum led_state_t (*get_state)(struct udevice *dev);

#ifdef CONFIG_LED_BLINK
	/**
	 * led_set_period() - set the blink period of an LED
	 *
	 * Thie records the period if supported, or returns -ENOSYS if not.
	 * To start the LED blinking, use set_state().
	 *
	 * @dev:	LED device to change
	 * @period_ms:	LED blink period in milliseconds
	 * @return 0 if OK, -ve on error
	 */
	int (*set_period)(struct udevice *dev, int period_ms);
#endif
};

#define led_get_ops(dev)	((struct led_ops *)(dev)->driver->ops)

/**
 * led_get_by_label() - Find an LED device by label
 *
 * @label:	LED label to look up
 * @devp:	Returns the associated device, if found
 * Return: 0 if found, -ENODEV if not found, other -ve on error
 */
int led_get_by_label(const char *label, struct udevice **devp);

/**
 * led_set_state() - set the state of an LED
 *
 * @dev:	LED device to change
 * @state:	LED state to set
 * Return: 0 if OK, -ve on error
 */
int led_set_state(struct udevice *dev, enum led_state_t state);

/**
 * led_get_state() - get the state of an LED
 *
 * @dev:	LED device to change
 * Return: LED state led_state_t, or -ve on error
 */
enum led_state_t led_get_state(struct udevice *dev);

/**
 * led_set_period() - set the blink period of an LED
 *
 * @dev:	LED device to change
 * @period_ms:	LED blink period in milliseconds
 * Return: 0 if OK, -ve on error
 */
int led_set_period(struct udevice *dev, int period_ms);

/**
 * led_bind_generic() - bind children of parent to given driver
 *
 * @parent:      Top-level LED device
 * @driver_name: Driver for handling individual child nodes
 */
int led_bind_generic(struct udevice *parent, const char *driver_name);

#endif
