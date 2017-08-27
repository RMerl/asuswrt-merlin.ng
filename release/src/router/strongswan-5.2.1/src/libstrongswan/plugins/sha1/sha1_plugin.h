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
 * @defgroup sha1_p sha1
 * @ingroup plugins
 *
 * @defgroup sha1_plugin sha1_plugin
 * @{ @ingroup sha1_p
 */

#ifndef SHA1_PLUGIN_H_
#define SHA1_PLUGIN_H_

#include <plugins/plugin.h>

typedef struct sha1_plugin_t sha1_plugin_t;

/**
 * Plugin implementing the SHA1 algorithm in software.
 */
struct sha1_plugin_t {

	/**
	 * implements plugin interface
	 */
	plugin_t plugin;
};

#endif /** SHA1_PLUGIN_H_ @}*/
