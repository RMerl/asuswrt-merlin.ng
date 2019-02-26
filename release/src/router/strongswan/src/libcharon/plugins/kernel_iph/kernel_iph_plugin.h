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
 * @defgroup kernel_iph kernel_iph
 * @ingroup cplugins
 *
 * @defgroup kernel_iph_plugin kernel_iph_plugin
 * @{ @ingroup kernel_iph
 */

#ifndef KERNEL_IPH_PLUGIN_H_
#define KERNEL_IPH_PLUGIN_H_

#include <plugins/plugin.h>

typedef struct kernel_iph_plugin_t kernel_iph_plugin_t;

/**
 * Windows IP Helper API based networking backend.
 */
struct kernel_iph_plugin_t {

	/**
	 * Implements plugin interface.
	 */
	plugin_t plugin;
};

#endif /** KERNEL_IPH_PLUGIN_H_ @}*/
