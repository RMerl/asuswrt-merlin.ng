// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2019 Intel Corporation <www.intel.com>
 */

#ifndef __CACHE_H
#define __CACHE_H

#include <linux/types.h>

struct udevice;

/*
 * Structure for the cache controller
 */
struct cache_info {
	phys_addr_t base; /* Base physical address of cache device. */
};

struct cache_ops {
	/**
	 * get_info() - Get basic cache info
	 *
	 * @dev:	Device to check (UCLASS_CACHE)
	 * @info:	Place to put info
	 * @return 0 if OK, -ve on error
	 */
	int (*get_info)(struct udevice *dev, struct cache_info *info);

	/**
	 * enable() - Enable cache
	 *
	 * @dev:	Device to check (UCLASS_CACHE)
	 * @return 0 if OK, -ve on error
	 */
	int (*enable)(struct udevice *dev);

	/**
	 * disable() - Flush and disable cache
	 *
	 * @dev:	Device to check (UCLASS_CACHE)
	 * @return 0 if OK, -ve on error
	 */
	int (*disable)(struct udevice *dev);
};

#define cache_get_ops(dev)	((struct cache_ops *)(dev)->driver->ops)

/**
 * cache_get_info() - Get information about a cache controller
 *
 * @dev:	Device to check (UCLASS_CACHE)
 * @info:	Returns cache info
 * Return: 0 if OK, -ve on error
 */
int cache_get_info(struct udevice *dev, struct cache_info *info);

/**
 * cache_enable() - Enable cache
 *
 * @dev:	Device to check (UCLASS_CACHE)
 * Return: 0 if OK, -ve on error
 */
int cache_enable(struct udevice *dev);

/**
 * cache_disable() - Flush and disable cache
 *
 * @dev:	Device to check (UCLASS_CACHE)
 * Return: 0 if OK, -ve on error
 */
int cache_disable(struct udevice *dev);
#endif
