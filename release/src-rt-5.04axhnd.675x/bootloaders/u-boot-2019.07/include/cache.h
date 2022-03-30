// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2019 Intel Corporation <www.intel.com>
 */

#ifndef __CACHE_H
#define __CACHE_H

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
};

#define cache_get_ops(dev)	((struct cache_ops *)(dev)->driver->ops)

/**
 * cache_get_info() - Get information about a cache controller
 *
 * @dev:	Device to check (UCLASS_CACHE)
 * @info:	Returns cache info
 * @return 0 if OK, -ve on error
 */
int cache_get_info(struct udevice *dev, struct cache_info *info);

#endif
