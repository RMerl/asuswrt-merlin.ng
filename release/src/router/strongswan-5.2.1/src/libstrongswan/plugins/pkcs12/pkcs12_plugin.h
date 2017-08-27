/*
 * Copyright (C) 2013 Tobias Brunner
 * Hochschule fuer Technik Rapperswil
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPSE.  See the GNU General Public License
 * for more details.
 */

/**
 * @defgroup pkcs12 pkcs12
 * @ingroup plugins
 *
 * @defgroup pkcs12_plugin pkcs12_plugin
 * @{ @ingroup pkcs12
 */

#ifndef PKCS12_PLUGIN_H_
#define PKCS12_PLUGIN_H_

#include <plugins/plugin.h>

typedef struct pkcs12_plugin_t pkcs12_plugin_t;

/**
 * Plugin providing PKCS#12 decoding functions
 */
struct pkcs12_plugin_t {

	/**
	 * Implements plugin interface.
	 */
	plugin_t plugin;
};

#endif /** PKCS12_PLUGIN_H_ @}*/
