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
 * @defgroup hmac_p hmac
 * @ingroup plugins
 *
 * @defgroup hmac_plugin hmac_plugin
 * @{ @ingroup hmac_p
 */

#ifndef HMAC_PLUGIN_H_
#define HMAC_PLUGIN_H_

#include <plugins/plugin.h>

typedef struct hmac_plugin_t hmac_plugin_t;

/**
 * Plugin implementing HMAC algorithm to prvoide hash based PRF and signers.
 */
struct hmac_plugin_t {

	/**
	 * implements plugin interface
	 */
	plugin_t plugin;
};

#endif /** HMAC_PLUGIN_H_ @}*/
