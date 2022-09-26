/*
 * Copyright (C) 2019 Andreas Steffen
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
 * @defgroup drbg_p drbg
 * @ingroup plugins
 *
 * @defgroup drbg_plugin drbg_plugin
 * @{ @ingroup drbg_p
 */

#ifndef DRBG_PLUGIN_H_
#define DRBG_PLUGIN_H_

#include <plugins/plugin.h>

typedef struct drbg_plugin_t drbg_plugin_t;

/**
 * Plugin providing a NIST CTR and HMAC DRBG implementation
 */
struct drbg_plugin_t {

	/**
	 * implements plugin interface
	 */
	plugin_t plugin;
};

#endif /** DRBG_PLUGIN_H_ @}*/
