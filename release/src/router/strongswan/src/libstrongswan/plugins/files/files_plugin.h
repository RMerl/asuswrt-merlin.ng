/*
 * Copyright (C) 2015 Tobias Brunner
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
 * @defgroup files_p files
 * @ingroup plugins
 *
 * @defgroup files_plugin files_plugin
 * @{ @ingroup files_p
 */

#ifndef FILES_PLUGIN_H_
#define FILES_PLUGIN_H_

#include <plugins/plugin.h>

typedef struct files_plugin_t files_plugin_t;

/**
 * Plugin implementing fetcher interface loading local files directly.
 */
struct files_plugin_t {

	/**
	 * implements plugin interface
	 */
	plugin_t plugin;
};

#endif /** FILES_PLUGIN_H_ @}*/
