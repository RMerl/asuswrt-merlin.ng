/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2015 Thomas Chou <thomas@wytron.com.tw>
 */

#ifndef _TIMER_H_
#define _TIMER_H_

#define timer_get_ops(dev)	((struct timer_ops *)(dev)->driver->ops)

/**
 * dm_timer_init() - set up a timer for time keeping
 *
 * Sets up gd->timer if the device is not already bound, making sure it is
 * probed and ready for use
 *
 * On success, inits gd->timer so that lib/timer can use it for future reference
 *
 * Returns: 0 on success, -EAGAIN if driver model is not ready yet, -ENODEV if
 * no timer could be found, other error if the timer could not be bound or
 * probed
 */
int dm_timer_init(void);

/**
 * timer_timebase_fallback() - Helper for timers using timebase fallback
 * @dev: A timer partially-probed timer device
 *
 * This is a helper function designed for timers which need to fall back on the
 * cpu's timebase. This function is designed to be called during the driver's
 * probe(). If there is a clocks or clock-frequency property in the timer's
 * binding, then it will be used. Otherwise, the timebase of the current cpu
 * will be used. This is initialized by the cpu driver, and usually gotten from
 * ``/cpus/timebase-frequency`` or ``/cpus/cpu@X/timebase-frequency``.
 *
 * Return: 0 if OK, or negative error code on failure
 */
int timer_timebase_fallback(struct udevice *dev);

/**
 * timer_conv_64() - convert 32-bit counter value to 64-bit
 * @count: 32-bit counter value
 *
 * Return: 64-bit counter value
 */
u64 timer_conv_64(u32 count);

/**
 * timer_get_count() - Get the current timer count
 * @dev: The timer device
 * @count: pointer that returns the current timer count
 *
 * Return: 0 if OK, -ve on error
 */
int timer_get_count(struct udevice *dev, u64 *count);

/**
 * timer_get_rate() - Get the timer input clock frequency in Hz
 * @dev: The timer device
 *
 * Return: the timer input clock frequency in Hz
 */
unsigned long timer_get_rate(struct udevice *dev);

/**
 * struct timer_ops - Driver model timer operations
 *
 * The uclass interface is implemented by all timer devices which use
 * driver model.
 */
struct timer_ops {
	/**
	 * @get_count: Get the current timer count
	 *
	 * @dev: The timer device
	 *
	 * This function may be called at any time after the driver is probed.
	 * All necessary initialization must be completed by the time probe()
	 * returns. The count returned by this functions should be monotonic.
	 * This function must succeed.
	 *
	 * Return: The current 64-bit timer count
	 */
	u64 (*get_count)(struct udevice *dev);
};

/**
 * struct timer_dev_priv - information about a device used by the uclass
 *
 * @clock_rate: the timer input clock frequency in Hz
 */
struct timer_dev_priv {
	unsigned long clock_rate;
};

/**
 * timer_early_get_count() - Implement timer_get_count() before driver model
 *
 * If ``CONFIG_TIMER_EARLY`` is enabled, this function wil be called to return
 * the current timer value before the proper driver model timer is ready.
 * It should be implemented by one of the timer values. This is mostly useful
 * for tracing.
 */
u64 timer_early_get_count(void);

/**
 * timer_early_get_rate() - Get the timer rate before driver model
 *
 * If ``CONFIG_TIMER_EARLY`` is enabled, this function wil be called to return
 * the current timer rate in Hz before the proper driver model timer is ready.
 * It should be implemented by one of the timer values. This is mostly useful
 * for tracing. This corresponds to the clock_rate value in struct
 * timer_dev_priv.
 */
unsigned long timer_early_get_rate(void);

#endif	/* _TIMER_H_ */
