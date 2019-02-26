/*
 * Copyright (C) 2012 Tobias Brunner
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
 * @defgroup pkcs8 pkcs8
 * @ingroup plugins
 *
 * @defgroup pkcs8_plugin pkcs8_plugin
 * @{ @ingroup pkcs8
 */

#ifndef PKCS8_PLUGIN_H_
#define PKCS8_PLUGIN_H_

#include <plugins/plugin.h>

typedef struct pkcs8_plugin_t pkcs8_plugin_t;

/**
 * Plugin providing PKCS#8 private key decoding functions
 */
struct pkcs8_plugin_t {

	/**
	 * implements plugin interface
	 */
	plugin_t plugin;
};

#endif /** PKCS8_PLUGIN_H_ @}*/
