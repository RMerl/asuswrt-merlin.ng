/*
 * Copyright (C) 2008 Tobias Brunner
 * Hochschule fuer Technik Rapperswil
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
 * @defgroup kernel_netlink_net_i kernel_netlink_net
 * @{ @ingroup kernel_netlink
 */

#ifndef KERNEL_NETLINK_NET_H_
#define KERNEL_NETLINK_NET_H_

#include <kernel/kernel_net.h>

typedef struct kernel_netlink_net_t kernel_netlink_net_t;

/**
 * Implementation of the kernel network interface using Netlink.
 */
struct kernel_netlink_net_t {

	/**
	 * Implements kernel_net_t interface
	 */
	kernel_net_t interface;
};

/**
 * Create a netlink kernel network interface instance.
 *
 * @return			kernel_netlink_net_t instance
 */
kernel_netlink_net_t *kernel_netlink_net_create();

#endif /** KERNEL_NETLINK_NET_H_ @}*/
