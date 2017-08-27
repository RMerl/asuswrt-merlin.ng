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
 * @defgroup pubkey_p pubkey
 * @ingroup plugins
 *
 * @defgroup pubkey_plugin pubkey_plugin
 * @{ @ingroup pubkey_p
 */

#ifndef PUBKEY_PLUGIN_H_
#define PUBKEY_PLUGIN_H_

#include <plugins/plugin.h>

typedef struct pubkey_plugin_t pubkey_plugin_t;

/**
 * Plugin implementing CERT_TRUSTED_PUBKEY certificate type.
 */
struct pubkey_plugin_t {

	/**
	 * implements plugin interface
	 */
	plugin_t plugin;
};

#endif /** PUBKEY_PLUGIN_H_ @}*/
