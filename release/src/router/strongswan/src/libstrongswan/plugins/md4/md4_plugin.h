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
 * @defgroup md4_p md4
 * @ingroup plugins
 *
 * @defgroup md4_plugin md4_plugin
 * @{ @ingroup md4_p
 */

#ifndef MD4_PLUGIN_H_
#define MD4_PLUGIN_H_

#include <plugins/plugin.h>

typedef struct md4_plugin_t md4_plugin_t;

/**
 * Plugin implementing the md4 hash algorithm in software.
 */
struct md4_plugin_t {

	/**
	 * implements plugin interface
	 */
	plugin_t plugin;
};

#endif /** MD4_PLUGIN_H_ @}*/
