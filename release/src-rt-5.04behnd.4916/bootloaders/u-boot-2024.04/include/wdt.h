/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2017 Google, Inc
 */

#ifndef _WDT_H_
#define _WDT_H_

struct udevice;

/*
 * Implement a simple watchdog uclass. Watchdog is basically a timer that
 * is used to detect or recover from malfunction. During normal operation
 * the watchdog would be regularly reset to prevent it from timing out.
 * If, due to a hardware fault or program error, the computer fails to reset
 * the watchdog, the timer will elapse and generate a timeout signal.
 * The timeout signal is used to initiate corrective action or actions,
 * which typically include placing the system in a safe, known state.
 */

/*
 * Start the timer
 *
 * @dev: WDT Device
 * @timeout_ms: Number of ticks (milliseconds) before timer expires
 * @flags: Driver specific flags. This might be used to specify
 * which action needs to be executed when the timer expires
 * @return: 0 if OK, -ve on error
 */
int wdt_start(struct udevice *dev, u64 timeout_ms, ulong flags);

/*
 * Stop the timer, thus disabling the Watchdog. Use wdt_start to start it again.
 *
 * @dev: WDT Device
 * @return: 0 if OK, -ve on error
 */
int wdt_stop(struct udevice *dev);

/*
 * Stop all registered watchdog devices.
 *
 * @return: 0 if ok, first error encountered otherwise (but wdt_stop()
 * is still called on following devices)
 */
int wdt_stop_all(void);

/*
 * Reset the timer, typically restoring the counter to
 * the value configured by start()
 *
 * @dev: WDT Device
 * @return: 0 if OK, -ve on error
 */
int wdt_reset(struct udevice *dev);

/*
 * Expire the timer, thus executing its action immediately.
 * This is typically used to reset the board or peripherals.
 *
 * @dev: WDT Device
 * @flags: Driver specific flags
 * Return: 0 if OK -ve on error. If wdt action is system reset,
 * this function may never return.
 */
int wdt_expire_now(struct udevice *dev, ulong flags);

/*
 * struct wdt_ops - Driver model wdt operations
 *
 * The uclass interface is implemented by all wdt devices which use
 * driver model.
 */
struct wdt_ops {
	/*
	 * Start the timer
	 *
	 * @dev: WDT Device
	 * @timeout_ms: Number of ticks (milliseconds) before the timer expires
	 * @flags: Driver specific flags. This might be used to specify
	 * which action needs to be executed when the timer expires
	 * @return: 0 if OK, -ve on error
	 */
	int (*start)(struct udevice *dev, u64 timeout_ms, ulong flags);
	/*
	 * Stop the timer
	 *
	 * @dev: WDT Device
	 * @return: 0 if OK, -ve on error
	 */
	int (*stop)(struct udevice *dev);
	/*
	 * Reset the timer, typically restoring the counter to
	 * the value configured by start()
	 *
	 * @dev: WDT Device
	 * @return: 0 if OK, -ve on error
	 */
	int (*reset)(struct udevice *dev);
	/*
	 * Expire the timer, thus executing the action immediately (optional)
	 *
	 * If this function is not provided, a default implementation
	 * will be used, which sets the counter to 1
	 * and waits forever. This is good enough for system level
	 * reset, where the function is not expected to return, but might not be
	 * good enough for other use cases.
	 *
	 * @dev: WDT Device
	 * @flags: Driver specific flags
	 * @return 0 if OK -ve on error. May not return.
	 */
	int (*expire_now)(struct udevice *dev, ulong flags);
};

int initr_watchdog(void);

#endif  /* _WDT_H_ */
