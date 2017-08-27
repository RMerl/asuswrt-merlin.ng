/*
 * Copyright (C) 2008 Martin Willi
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
 * @defgroup ha ha
 * @ingroup cplugins
 *
 * @defgroup ha_plugin ha_plugin
 * @{ @ingroup ha
 */

#ifndef HA_PLUGIN_H_
#define HA_PLUGIN_H_

#include <plugins/plugin.h>

/**
 * UDP port we use for communication
 */
#define HA_PORT 4510

typedef struct ha_plugin_t ha_plugin_t;

/**
 * Plugin to synchronize state in a high availability cluster.
 */
struct ha_plugin_t {

	/**
	 * implements plugin interface
	 */
	plugin_t plugin;
};

#endif /** HA_PLUGIN_H_ @}*/
