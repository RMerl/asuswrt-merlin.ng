/*
 * Copyright (C) 2010 Martin Willi
 * Copyright (C) 2010 revosec AG
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
 * @defgroup pkcs11 pkcs11
 * @ingroup plugins
 *
 * @defgroup pkcs11_plugin pkcs11_plugin
 * @{ @ingroup pkcs11
 */

#ifndef PKCS11_PLUGIN_H_
#define PKCS11_PLUGIN_H_

#include <plugins/plugin.h>

typedef struct pkcs11_plugin_t pkcs11_plugin_t;

/**
 * Plugin providing PKCS#11 token support.
 */
struct pkcs11_plugin_t {

	/**
	 * Implements plugin interface,
	 */
	plugin_t plugin;
};

#endif /** PKCS11_PLUGIN_H_ @}*/
