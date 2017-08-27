/*
 * Copyright (C) 2008 Tobias Brunner
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
 * @defgroup openssl_p openssl
 * @ingroup plugins
 *
 * @defgroup openssl_plugin openssl_plugin
 * @{ @ingroup openssl_p
 */

#ifndef OPENSSL_PLUGIN_H_
#define OPENSSL_PLUGIN_H_

#include <plugins/plugin.h>

typedef struct openssl_plugin_t openssl_plugin_t;

/**
 * Plugin implementing crypto functions via the OpenSSL library
 */
struct openssl_plugin_t {

	/**
	 * implements plugin interface
	 */
	plugin_t plugin;
};

#endif /** OPENSSL_PLUGIN_H_ @}*/
