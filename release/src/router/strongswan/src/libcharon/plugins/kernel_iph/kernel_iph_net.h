/*
 * Copyright (C) 2013 Martin Willi
 * Copyright (C) 2013 revosec AG
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
 * @defgroup kernel_iph_net_i kernel_iph_net
 * @{ @ingroup kernel_iph
 */

#ifndef KERNEL_IPH_NET_H_
#define KERNEL_IPH_NET_H_

#include <kernel/kernel_net.h>

typedef struct kernel_iph_net_t kernel_iph_net_t;

/**
 * Implementation of the kernel network interface using Windows IP Helper.
 */
struct kernel_iph_net_t {

	/**
	 * Implements kernel_net_t interface
	 */
	kernel_net_t interface;
};

/**
 * Create IP Helper network backend instance.
 *
 * @return			kernel_iph_net_t instance
 */
kernel_iph_net_t *kernel_iph_net_create();

#endif /** KERNEL_IPH_NET_H_ @}*/
