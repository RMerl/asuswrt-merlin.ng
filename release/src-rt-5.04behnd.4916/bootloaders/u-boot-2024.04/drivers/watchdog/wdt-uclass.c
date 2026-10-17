// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2017 Google, Inc
 */

#define LOG_CATEGORY UCLASS_WDT

#include <common.h>
#include <cyclic.h>
#include <div64.h>
#include <dm.h>
#include <errno.h>
#include <hang.h>
#include <log.h>
#include <sysreset.h>
#include <time.h>
#include <wdt.h>
#include <asm/global_data.h>
#include <dm/device-internal.h>
#include <dm/lists.h>

DECLARE_GLOBAL_DATA_PTR;

#define WATCHDOG_TIMEOUT_SECS	(CONFIG_WATCHDOG_TIMEOUT_MSECS / 1000)

struct wdt_priv {
	/* Timeout, in seconds, to configure this device to. */
	u32 timeout;
	/*
	 * Time, in milliseconds, between calling the device's ->reset()
	 * method from watchdog_reset().
	 */
	ulong reset_period;
	/*
	 * Next time (as returned by get_timer(0)) to call
	 * ->reset().
	 */
	ulong next_reset;
	/* Whether watchdog_start() has been called on the device. */
	bool running;
	/* autostart */
	bool autostart;

	struct cyclic_info *cyclic;
};

static void wdt_cyclic(void *ctx)
{
	struct udevice *dev = ctx;
	struct wdt_priv *priv;

	if (!device_active(dev))
		return;

	priv = dev_get_uclass_priv(dev);
	if (!priv->running)
		return;

	wdt_reset(dev);
}

static void init_watchdog_dev(struct udevice *dev)
{
	struct wdt_priv *priv;
	int ret;

	priv = dev_get_uclass_priv(dev);

	if (IS_ENABLED(CONFIG_SYSRESET_WATCHDOG_AUTO)) {
		ret = sysreset_register_wdt(dev);
		if (ret)
			printf("WDT:   Failed to register %s for sysreset\n",
			       dev->name);
	}

	if (!priv->autostart) {
		printf("WDT:   Not starting %s\n", dev->name);
		return;
	}

	ret = wdt_start(dev, priv->timeout * 1000, 0);
	if (ret != 0) {
		printf("WDT:   Failed to start %s\n", dev->name);
		return;
	}
}

int initr_watchdog(void)
{
	struct udevice *dev;
	struct uclass *uc;
	int ret;

	ret = uclass_get(UCLASS_WDT, &uc);
	if (ret) {
		log_debug("Error getting UCLASS_WDT: %d\n", ret);
		return 0;
	}

	uclass_foreach_dev(dev, uc) {
		ret = device_probe(dev);
		if (ret) {
			log_debug("Error probing %s: %d\n", dev->name, ret);
			continue;
		}
		init_watchdog_dev(dev);
	}

	return 0;
}

int wdt_start(struct udevice *dev, u64 timeout_ms, ulong flags)
{
	const struct wdt_ops *ops = device_get_ops(dev);
	int ret;

	if (!ops->start)
		return -ENOSYS;

	ret = ops->start(dev, timeout_ms, flags);
	if (ret == 0) {
		struct wdt_priv *priv = dev_get_uclass_priv(dev);
		char str[16];

		priv->running = true;

		memset(str, 0, 16);
		if (IS_ENABLED(CONFIG_WATCHDOG)) {
			/* Register the watchdog driver as a cyclic function */
			priv->cyclic = cyclic_register(wdt_cyclic,
						       priv->reset_period * 1000,
						       dev->name, dev);
			if (!priv->cyclic) {
				printf("cyclic_register for %s failed\n",
				       dev->name);
				return -ENODEV;
			} else {
				snprintf(str, 16, "every %ldms",
					 priv->reset_period);
			}
		}

		printf("WDT:   Started %s with%s servicing %s (%ds timeout)\n",
		       dev->name, IS_ENABLED(CONFIG_WATCHDOG) ? "" : "out",
		       str, (u32)lldiv(timeout_ms, 1000));
	}

	return ret;
}

int wdt_stop(struct udevice *dev)
{
	const struct wdt_ops *ops = device_get_ops(dev);
	int ret;

	if (!ops->stop)
		return -ENOSYS;

	ret = ops->stop(dev);
	if (ret == 0) {
		struct wdt_priv *priv = dev_get_uclass_priv(dev);

		priv->running = false;
	}

	return ret;
}

int wdt_stop_all(void)
{
	struct wdt_priv *priv;
	struct udevice *dev;
	struct uclass *uc;
	int ret, err;

	ret = uclass_get(UCLASS_WDT, &uc);
	if (ret)
		return ret;

	uclass_foreach_dev(dev, uc) {
		if (!device_active(dev))
			continue;
		priv = dev_get_uclass_priv(dev);
		if (!priv->running)
			continue;
		err = wdt_stop(dev);
		if (!ret)
			ret = err;
	}

	return ret;
}

int wdt_reset(struct udevice *dev)
{
	const struct wdt_ops *ops = device_get_ops(dev);

	if (!ops->reset)
		return -ENOSYS;

	return ops->reset(dev);
}

int wdt_expire_now(struct udevice *dev, ulong flags)
{
	int ret = 0;
	const struct wdt_ops *ops;

	debug("WDT Resetting: %lu\n", flags);
	ops = device_get_ops(dev);
	if (ops->expire_now) {
		return ops->expire_now(dev, flags);
	} else {
		ret = wdt_start(dev, 1, flags);

		if (ret < 0)
			return ret;

		hang();
	}

	return ret;
}

#if defined(CONFIG_WATCHDOG)
/*
 * Called by macro WATCHDOG_RESET. This function be called *very* early,
 * so we need to make sure, that the watchdog driver is ready before using
 * it in this function.
 */
void watchdog_reset(void)
{
	/*
	 * Empty function for now. The actual WDT handling is now done in
	 * the cyclic function instead.
	 */
}
#endif

static int wdt_pre_probe(struct udevice *dev)
{
	u32 timeout = WATCHDOG_TIMEOUT_SECS;
	/*
	 * Reset every 1000ms, or however often is required as
	 * indicated by a hw_margin_ms property.
	 */
	ulong reset_period = 1000;
	bool autostart = IS_ENABLED(CONFIG_WATCHDOG_AUTOSTART);
	struct wdt_priv *priv;

	if (CONFIG_IS_ENABLED(OF_CONTROL) && !CONFIG_IS_ENABLED(OF_PLATDATA)) {
		timeout = dev_read_u32_default(dev, "timeout-sec", timeout);
		reset_period = dev_read_u32_default(dev, "hw_margin_ms",
						    4 * reset_period) / 4;
		if (dev_read_bool(dev, "u-boot,noautostart"))
			autostart = false;
		else if (dev_read_bool(dev, "u-boot,autostart"))
			autostart = true;
	}
	priv = dev_get_uclass_priv(dev);
	priv->timeout = timeout;
	priv->reset_period = reset_period;
	priv->autostart = autostart;
	/*
	 * Pretend this device was last reset "long" ago so the first
	 * watchdog_reset will actually call its ->reset method.
	 */
	priv->next_reset = get_timer(0);

	return 0;
}

UCLASS_DRIVER(wdt) = {
	.id			= UCLASS_WDT,
	.name			= "watchdog",
	.flags			= DM_UC_FLAG_SEQ_ALIAS,
	.pre_probe		= wdt_pre_probe,
	.per_device_auto	= sizeof(struct wdt_priv),
};
