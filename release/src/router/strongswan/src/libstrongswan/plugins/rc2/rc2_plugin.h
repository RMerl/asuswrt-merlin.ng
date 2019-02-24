/*
 * Copyright (C) 2013 Tobias Brunner
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
 * @defgroup rc2_p rc2
 * @ingroup plugins
 *
 * @defgroup rc2_plugin rc2_plugin
 * @{ @ingroup rc2_p
 */

#ifndef RC2_PLUGIN_H_
#define RC2_PLUGIN_H_

#include <plugins/plugin.h>

typedef struct rc2_plugin_t rc2_plugin_t;

/**
 * Plugin implementing RC2 (RFC 2268).
 */
struct rc2_plugin_t {

	/**
	 * Implements plugin interface
	 */
	plugin_t plugin;
};

#endif /** RC2_PLUGIN_H_ @}*/
