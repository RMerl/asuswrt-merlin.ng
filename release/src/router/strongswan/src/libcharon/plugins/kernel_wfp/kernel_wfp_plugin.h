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
 * @defgroup kernel_wfp kernel_wfp
 * @ingroup cplugins
 *
 * @defgroup kernel_wfp_plugin kernel_wfp_plugin
 * @{ @ingroup kernel_wfp
 */

#ifndef KERNEL_WFP_PLUGIN_H_
#define KERNEL_WFP_PLUGIN_H_

#include <library.h>
#include <plugins/plugin.h>

typedef struct kernel_wfp_plugin_t kernel_wfp_plugin_t;

/**
 * Windows Filter Platform based IPsec backend plugin.
 */
struct kernel_wfp_plugin_t {

	/**
	 * implements plugin interface
	 */
	plugin_t plugin;
};

#endif /** KERNEL_WFP_PLUGIN_H_ @}*/
