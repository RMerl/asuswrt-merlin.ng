/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2013 Google, Inc
 *
 * (C) Copyright 2012
 * Pavel Herrmann <morpheus.ibis@gmail.com>
 * Marek Vasut <marex@denx.de>
 */

#ifndef _DM_PLATDATA_H
#define _DM_PLATDATA_H

#include <linker_lists.h>

/**
 * struct driver_info - Information required to instantiate a device
 *
 * NOTE: Avoid using this except in extreme circumstances, where device tree
 * is not feasible (e.g. serial driver in SPL where <8KB of SRAM is
 * available). U-Boot's driver model uses device tree for configuration.
 *
 * @name:	Driver name
 * @plat:	Driver-specific platform data
 * @plat_size: Size of platform data structure
 * @parent_idx:	Index of the parent driver_info structure
 */
struct driver_info {
	const char *name;
	const void *plat;
#if CONFIG_IS_ENABLED(OF_PLATDATA)
	unsigned short plat_size;
	short parent_idx;
#endif
};

#if CONFIG_IS_ENABLED(OF_PLATDATA)
#define driver_info_parent_id(driver_info)	driver_info->parent_idx
#else
#define driver_info_parent_id(driver_info)	(-1)
#endif

/**
 * struct driver_rt - runtime information set up by U-Boot
 *
 * There is one of these for every driver_info in the linker list, indexed by
 * the driver_info idx value.
 *
 * @dev: Device created from this idx
 */
struct driver_rt {
	struct udevice *dev;
};

/*
 * NOTE: Avoid using these except in extreme circumstances, where device tree
 * is not feasible (e.g. serial driver in SPL where <8KB of SRAM is
 * available). U-Boot's driver model uses device tree for configuration.
 *
 * When of-platdata is in use, U_BOOT_DRVINFO() cannot be used outside of the
 * dt-plat.c file created by dtoc
 */
#if CONFIG_IS_ENABLED(OF_PLATDATA) && !defined(DT_PLAT_C)
#define U_BOOT_DRVINFO(__name)	_Static_assert(false, \
	"Cannot use U_BOOT_DRVINFO with of-platdata. Please use devicetree instead")
#else
#define U_BOOT_DRVINFO(__name)						\
	ll_entry_declare(struct driver_info, __name, driver_info)
#endif

/* Declare a list of devices. The argument is a driver_info[] array */
#define U_BOOT_DRVINFOS(__name)						\
	ll_entry_declare_list(struct driver_info, __name, driver_info)

#endif
