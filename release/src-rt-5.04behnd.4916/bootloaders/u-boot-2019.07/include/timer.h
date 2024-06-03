/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2015 Thomas Chou <thomas@wytron.com.tw>
 */

#ifndef _TIMER_H_
#define _TIMER_H_

/*
 * dm_timer_init - initialize a timer for time keeping. On success
 * initializes gd->timer so that lib/timer can use it for future
 * referrence.
 *
 * @return - 0 on success or error number
 */
int dm_timer_init(void);

/*
 * timer_conv_64 - convert 32-bit counter value to 64-bit
 *
 * @count: 32-bit counter value
 * @return: 64-bit counter value
 */
u64 timer_conv_64(u32 count);

/*
 * Get the current timer count
 *
 * @dev: The timer device
 * @count: pointer that returns the current timer count
 * @return: 0 if OK, -ve on error
 */
int timer_get_count(struct udevice *dev, u64 *count);

/*
 * Get the timer input clock frequency
 *
 * @dev: The timer device
 * @return: the timer input clock frequency
 */
unsigned long timer_get_rate(struct udevice *dev);

/*
 * struct timer_ops - Driver model timer operations
 *
 * The uclass interface is implemented by all timer devices which use
 * driver model.
 */
struct timer_ops {
	/*
	 * Get the current timer count
	 *
	 * @dev: The timer device
	 * @count: pointer that returns the current 64-bit timer count
	 * @return: 0 if OK, -ve on error
	 */
	int (*get_count)(struct udevice *dev, u64 *count);
};

/*
 * struct timer_dev_priv - information about a device used by the uclass
 *
 * @clock_rate: the timer input clock frequency
 */
struct timer_dev_priv {
	unsigned long clock_rate;
};

/**
 * timer_early_get_count() - Implement timer_get_count() before driver model
 *
 * If CONFIG_TIMER_EARLY is enabled, this function wil be called to return
 * the current timer value before the proper driver model timer is ready.
 * It should be implemented by one of the timer values. This is mostly useful
 * for tracing.
 */
u64 timer_early_get_count(void);

/**
 * timer_early_get_rate() - Get the timer rate before driver model
 *
 * If CONFIG_TIMER_EARLY is enabled, this function wil be called to return
 * the current timer rate in Hz before the proper driver model timer is ready.
 * It should be implemented by one of the timer values. This is mostly useful
 * for tracing. This corresponds to the clock_rate value in struct
 * timer_dev_priv.
 */
unsigned long timer_early_get_rate(void);

#endif	/* _TIMER_H_ */
