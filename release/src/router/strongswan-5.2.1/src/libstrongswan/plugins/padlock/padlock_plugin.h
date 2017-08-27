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
 * @defgroup padlock_p padlock
 * @ingroup plugins
 *
 * @defgroup padlock_plugin padlock_plugin
 * @{ @ingroup padlock_p
 */

#ifndef PADLOCK_PLUGIN_H_
#define PADLOCK_PLUGIN_H_

#include <plugins/plugin.h>

typedef struct padlock_plugin_t padlock_plugin_t;

/**
 * Plugin implementing VIA Padlock crypto functions
 */
struct padlock_plugin_t {

	/**
	 * implements plugin interface
	 */
	plugin_t plugin;
};

#endif /** PADLOCK_PLUGIN_H_ @}*/
