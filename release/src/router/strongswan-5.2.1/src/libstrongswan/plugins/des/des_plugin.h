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
 * @defgroup des_p des
 * @ingroup plugins
 *
 * @defgroup des_plugin des_plugin
 * @{ @ingroup des_p
 */

#ifndef DES_PLUGIN_H_
#define DES_PLUGIN_H_

#include <plugins/plugin.h>

typedef struct des_plugin_t des_plugin_t;

/**
 * Plugin implementing DES based algorithms in software.
 */
struct des_plugin_t {

	/**
	 * implements plugin interface
	 */
	plugin_t plugin;
};

#endif /** DES_PLUGIN_H_ @}*/
