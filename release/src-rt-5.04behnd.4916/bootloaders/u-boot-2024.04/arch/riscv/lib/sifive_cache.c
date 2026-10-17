// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2021 SiFive, Inc
 */

#include <cache.h>
#include <cpu_func.h>
#include <log.h>
#include <dm.h>
#include <dm/device-internal.h>
#include <dm/uclass-internal.h>

#ifndef CONFIG_SPL_BUILD
void enable_caches(void)
{
	struct udevice *dev;
	int ret;

	/* Enable ways of ccache */
	ret = uclass_get_device_by_driver(UCLASS_CACHE,
					  DM_DRIVER_GET(sifive_ccache),
					  &dev);
	if (ret) {
		log_debug("Cannot enable cache ways");
	} else {
		ret = cache_enable(dev);
		if (ret)
			log_debug("ccache enable failed");
	}
}
#else
static inline void probe_cache_device(struct driver *driver, struct udevice *dev)
{
	for (uclass_find_first_device(UCLASS_CACHE, &dev);
	     dev;
	     uclass_find_next_device(&dev)) {
		if (dev->driver == driver)
			device_probe(dev);
	}
}

void enable_caches(void)
{
	struct udevice *dev = NULL;

	probe_cache_device(DM_DRIVER_GET(sifive_pl2), dev);
}
#endif /* !CONFIG_SPL_BUILD */
