/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2016, NVIDIA CORPORATION.
 */

#ifndef _POWER_DOMAIN_UCLASS_H
#define _POWER_DOMAIN_UCLASS_H

/* See power-domain.h for background documentation. */

#include <power-domain.h>

struct udevice;

/**
 * struct power_domain_ops - The functions that a power domain controller driver
 * must implement.
 */
struct power_domain_ops {
	/**
	 * of_xlate - Translate a client's device-tree (OF) power domain
	 * specifier.
	 *
	 * The power domain core calls this function as the first step in
	 * implementing a client's power_domain_get() call.
	 *
	 * If this function pointer is set to NULL, the power domain core will
	 * use a default implementation, which assumes #power-domain-cells =
	 * <1>, and that the DT cell contains a simple integer power domain ID.
	 *
	 * At present, the power domain API solely supports device-tree. If
	 * this changes, other xxx_xlate() functions may be added to support
	 * those other mechanisms.
	 *
	 * @power_domain:	The power domain struct to hold the
	 *			translation result.
	 * @args:		The power domain specifier values from device
	 *			tree.
	 * @return 0 if OK, or a negative error code.
	 */
	int (*of_xlate)(struct power_domain *power_domain,
			struct ofnode_phandle_args *args);
	/**
	 * request - Request a translated power domain.
	 *
	 * The power domain core calls this function as the second step in
	 * implementing a client's power_domain_get() call, following a
	 * successful xxx_xlate() call.
	 *
	 * @power_domain:	The power domain to request; this has been
	 *			filled in by a previous xxx_xlate() function
	 *			call.
	 * @return 0 if OK, or a negative error code.
	 */
	int (*request)(struct power_domain *power_domain);
	/**
	 * free - Free a previously requested power domain.
	 *
	 * This is the implementation of the client power_domain_free() API.
	 *
	 * @power_domain:	The power domain to free.
	 * @return 0 if OK, or a negative error code.
	 */
	int (*free)(struct power_domain *power_domain);
	/**
	 * on - Power on a power domain.
	 *
	 * @power_domain:	The power domain to turn on.
	 * @return 0 if OK, or a negative error code.
	 */
	int (*on)(struct power_domain *power_domain);
	/**
	 * off - Power off a power domain.
	 *
	 * @power_domain:	The power domain to turn off.
	 * @return 0 if OK, or a negative error code.
	 */
	int (*off)(struct power_domain *power_domain);
};

#endif
