/*
 * Copyright (C) 2009 Martin Willi
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
 * @defgroup attr attr
 * @ingroup hplugins
 *
 * @defgroup attr_plugin attr_plugin
 * @{ @ingroup attr
 */

#ifndef ATTR_PLUGIN_H_
#define ATTR_PLUGIN_H_

#include <plugins/plugin.h>

typedef struct attr_plugin_t attr_plugin_t;

/**
 * Plugin providing configuration attribute through strongswan.conf.
 */
struct attr_plugin_t {

	/**
	 * implements plugin interface
	 */
	plugin_t plugin;
};

#endif /** ATTR_PLUGIN_H_ @}*/
