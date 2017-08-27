/*
 * Copyright (C) 2009 Martin Willi
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
 * @defgroup pkcs1 pkcs1
 * @ingroup plugins
 *
 * @defgroup pkcs1_plugin pkcs1_plugin
 * @{ @ingroup pkcs1
 */

#ifndef PKCS1_PLUGIN_H_
#define PKCS1_PLUGIN_H_

#include <plugins/plugin.h>

typedef struct pkcs1_plugin_t pkcs1_plugin_t;

/**
 * Plugin providing PKCS#1 private/public key decoding functions
 */
struct pkcs1_plugin_t {

	/**
	 * implements plugin interface
	 */
	plugin_t plugin;
};

#endif /** PKCS1_PLUGIN_H_ @}*/
