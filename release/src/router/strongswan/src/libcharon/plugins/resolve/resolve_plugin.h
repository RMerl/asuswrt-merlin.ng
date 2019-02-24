/*
 * Copyright (C) 2009 Martin Willi
 * HSR Hochschule fuer Technik Rapperswil
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
 * @defgroup resolve resolve
 * @ingroup cplugins
 *
 * @defgroup resolve_plugin resolve_plugin
 * @{ @ingroup resolve
 */

#ifndef RESOLVE_PLUGIN_H_
#define RESOLVE_PLUGIN_H_

#include <plugins/plugin.h>

typedef struct resolve_plugin_t resolve_plugin_t;

/**
 * Plugin that writes received DNS servers in a resolv.conf file.
 */
struct resolve_plugin_t {

	/**
	 * implements plugin interface
	 */
	plugin_t plugin;
};

#endif /** RESOLVE_PLUGIN_H_ @}*/
