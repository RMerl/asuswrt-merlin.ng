/*
 * Copyright (C) 2022-2023 Tobias Brunner
 *
 * Copyright (C) secunet Security Networks AG
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */

/**
 * @defgroup kernel_netlink_xfrmi kernel_netlink_xfrmi
 * @{ @ingroup kernel_netlink
 */

#ifndef KERNEL_NETLINK_XFRMI_H_
#define KERNEL_NETLINK_XFRMI_H_

#include <library.h>

#define KERNEL_NETLINK_XFRMI_MANAGER "kernel-netlink-xfrmi"

typedef struct kernel_netlink_xfrmi_t kernel_netlink_xfrmi_t;

/**
 * Simple manager for XFRM interfaces. An instance can be retrieved via
 * lib::get() under the key "kernel-netlink-xfrmi" if the kernel-netlink plugin
 * is loaded and XFRM interfaces are supported by the kernel.
 */
struct kernel_netlink_xfrmi_t {

	/**
	 * Creates an XFRM interface with the given name, interface ID and
	 * optional underlying physical interface and MTU.
	 *
	 * @param name			name of the XFRM interface
	 * @param if_id			interface ID (has to match SAs/policies)
	 * @param phys			name of the underlying physical interface (optional)
	 * @param mtu			MTU of the interface (optional, 0 for default)
	 * @return				TRUE if interface was successfully created
	 */
	bool (*create)(kernel_netlink_xfrmi_t *this, char *name, uint32_t if_id,
				   char *phys, uint32_t mtu);

	/**
	 * Enumerate existing XFRM interfaces.
	 *
	 * @return				enumerator over (char *name, uint32_t if_id,
	 *						char *phys, u_int mtu)
	 */
	enumerator_t *(*create_enumerator)(kernel_netlink_xfrmi_t *this);

	/**
	 * Deletes the XFRM interface with the given name.
	 *
	 * @note This deletes any type of interface with the given name.
	 *
	 * @param name			name of the XFRM interface
	 * @return				TRUE if interface was successfully deleted
	 */
	bool (*delete)(kernel_netlink_xfrmi_t *this, char *name);
};

/**
 * Create the manager.
 *
 * @param test		test if XFRM interfaces can be created (requires CAP_NET_ADMIN)
 * @return			kernel_netlink_xfrmi_t instance, or NULL if test failed
 */
kernel_netlink_xfrmi_t *kernel_netlink_xfrmi_create(bool test);

/**
 * Destroy the given manager. Not a method in the interface above to prevent
 * users from destroying the manager.
 */
void kernel_netlink_xfrmi_destroy(kernel_netlink_xfrmi_t *this);

#endif /** KERNEL_NETLINK_XFRMI_H_ @}*/
