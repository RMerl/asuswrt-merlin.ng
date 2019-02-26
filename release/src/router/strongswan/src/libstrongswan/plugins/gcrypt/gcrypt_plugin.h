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
 * @defgroup gcrypt_p gcrypt
 * @ingroup plugins
 *
 * @defgroup gcrypt_plugin gcrypt_plugin
 * @{ @ingroup gcrypt_p
 */

#ifndef GCRYPT_PLUGIN_H_
#define GCRYPT_PLUGIN_H_

#include <plugins/plugin.h>

typedef struct gcrypt_plugin_t gcrypt_plugin_t;

/**
 * Plugin implementing crypto functions via libgcrypt.
 */
struct gcrypt_plugin_t {

	/**
	 * implements plugin interface
	 */
	plugin_t plugin;
};

#endif /** GCRYPT_PLUGIN_H_ @}*/
