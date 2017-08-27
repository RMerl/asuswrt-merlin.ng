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
 * @defgroup kernel_netlink kernel_netlink
 * @ingroup hplugins
 *
 * @defgroup kernel_netlink_plugin kernel_netlink_plugin
 * @{ @ingroup kernel_netlink
 */

#ifndef KERNEL_NETLINK_PLUGIN_H_
#define KERNEL_NETLINK_PLUGIN_H_

#include <plugins/plugin.h>

typedef struct kernel_netlink_plugin_t kernel_netlink_plugin_t;

/**
 * netlink kernel interface plugin
 */
struct kernel_netlink_plugin_t {

	/**
	 * implements plugin interface
	 */
	plugin_t plugin;
};

#endif /** KERNEL_NETLINK_PLUGIN_H_ @}*/
