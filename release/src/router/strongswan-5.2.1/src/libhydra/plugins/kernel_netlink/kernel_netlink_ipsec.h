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
 * @defgroup kernel_netlink_ipsec_i kernel_netlink_ipsec
 * @{ @ingroup kernel_netlink
 */

#ifndef KERNEL_NETLINK_IPSEC_H_
#define KERNEL_NETLINK_IPSEC_H_

#include <kernel/kernel_ipsec.h>

typedef struct kernel_netlink_ipsec_t kernel_netlink_ipsec_t;

/**
 * Implementation of the kernel ipsec interface using Netlink.
 */
struct kernel_netlink_ipsec_t {

	/**
	 * Implements kernel_ipsec_t interface
	 */
	kernel_ipsec_t interface;
};

/**
 * Create a netlink kernel ipsec interface instance.
 *
 * @return			kernel_netlink_ipsec_t instance
 */
kernel_netlink_ipsec_t *kernel_netlink_ipsec_create();

#endif /** KERNEL_NETLINK_IPSEC_H_ @}*/
