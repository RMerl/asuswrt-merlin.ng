/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2017 Google, Inc
 */

#ifndef _WDT_H_
#define _WDT_H_

#include <dm.h>
#include <dm/read.h>

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
 * @return 0 if OK -ve on error. If wdt action is system reset,
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

#if defined(CONFIG_WDT)
#ifndef CONFIG_WATCHDOG_TIMEOUT_MSECS
#define CONFIG_WATCHDOG_TIMEOUT_MSECS	(60 * 1000)
#endif
#define WATCHDOG_TIMEOUT_SECS	(CONFIG_WATCHDOG_TIMEOUT_MSECS / 1000)

static inline int initr_watchdog(void)
{
	u32 timeout = WATCHDOG_TIMEOUT_SECS;

	/*
	 * Init watchdog: This will call the probe function of the
	 * watchdog driver, enabling the use of the device
	 */
	if (uclass_get_device_by_seq(UCLASS_WDT, 0,
				     (struct udevice **)&gd->watchdog_dev)) {
		debug("WDT:   Not found by seq!\n");
		if (uclass_get_device(UCLASS_WDT, 0,
				      (struct udevice **)&gd->watchdog_dev)) {
			printf("WDT:   Not found!\n");
			return 0;
		}
	}

	if (CONFIG_IS_ENABLED(OF_CONTROL)) {
		timeout = dev_read_u32_default(gd->watchdog_dev, "timeout-sec",
					       WATCHDOG_TIMEOUT_SECS);
	}

	wdt_start(gd->watchdog_dev, timeout * 1000, 0);
	gd->flags |= GD_FLG_WDT_READY;
	printf("WDT:   Started with%s servicing (%ds timeout)\n",
	       IS_ENABLED(CONFIG_WATCHDOG) ? "" : "out", timeout);

	return 0;
}
#endif

#endif  /* _WDT_H_ */
