/*
 * Copyright (C) 2012 Martin Willi
 * Copyright (C) 2012 revosec AG
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
 * @defgroup rdrand_p rdrand
 * @ingroup plugins
 *
 * @defgroup rdrand_plugin rdrand_plugin
 * @{ @ingroup rdrand_p
 */

#ifndef RDRAND_PLUGIN_H_
#define RDRAND_PLUGIN_H_

#include <plugins/plugin.h>

typedef struct rdrand_plugin_t rdrand_plugin_t;

/**
 * Plugin providing random generators based on Intels RDRAND instruction.
 */
struct rdrand_plugin_t {

	/**
	 * implements plugin interface
	 */
	plugin_t plugin;
};

#endif /** RDRAND_PLUGIN_H_ @}*/
