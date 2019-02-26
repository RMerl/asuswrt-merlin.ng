/*
 * Copyright (C) 2008 Martin Willi
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
 * @defgroup mysql_p mysql
 * @ingroup plugins
 *
 * @defgroup mysql_plugin mysql_plugin
 * @{ @ingroup mysql_p
 */

#ifndef MYSQL_PLUGIN_H_
#define MYSQL_PLUGIN_H_

#include <plugins/plugin.h>

typedef struct mysql_plugin_t mysql_plugin_t;

/**
 * Plugin implementing mysql database connectivity.
 */
struct mysql_plugin_t {

	/**
	 * implements plugin interface
	 */
	plugin_t plugin;
};

#endif /** MYSQL_PLUGIN_H_ @}*/
