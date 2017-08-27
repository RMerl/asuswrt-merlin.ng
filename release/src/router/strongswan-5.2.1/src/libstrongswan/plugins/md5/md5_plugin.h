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
 * @defgroup md5_p md5
 * @ingroup plugins
 *
 * @defgroup md5_plugin md5_plugin
 * @{ @ingroup md5_p
 */

#ifndef MD5_PLUGIN_H_
#define MD5_PLUGIN_H_

#include <plugins/plugin.h>

typedef struct md5_plugin_t md5_plugin_t;

/**
 * Plugin implementing the MD5 hash algorithm in software.
 */
struct md5_plugin_t {

	/**
	 * implements plugin interface
	 */
	plugin_t plugin;
};

#endif /** MD5_PLUGIN_H_ @}*/
