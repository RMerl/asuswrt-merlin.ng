/*
 * Copyright (C) 2012 Martin Willi
 * Copyright (C) 2012 revosec AG
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
 * @defgroup pkcs7p pkcs7
 * @ingroup plugins
 *
 * @defgroup pkcs7_plugin pkcs7_plugin
 * @{ @ingroup pkcs7p
 */

#ifndef PKCS7_PLUGIN_H_
#define PKCS7_PLUGIN_H_

#include <plugins/plugin.h>

typedef struct pkcs7_plugin_t pkcs7_plugin_t;

/**
 * Plugin providing PKCS#7 container functionality.
 */
struct pkcs7_plugin_t {

	/**
	 * Implements plugin interface.
	 */
	plugin_t plugin;
};

#endif /** PKCS7_PLUGIN_H_ @}*/
