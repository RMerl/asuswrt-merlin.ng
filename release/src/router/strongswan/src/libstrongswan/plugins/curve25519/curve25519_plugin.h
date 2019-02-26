/*
 * Copyright (C) 2014 Martin Willi
 * Copyright (C) 2014 revosec AG
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
 * @defgroup curve25519_p curve25519
 * @ingroup plugins
 *
 * @defgroup curve25519_plugin curve25519_plugin
 * @{ @ingroup curve25519_p
 */

#ifndef CURVE25519_PLUGIN_H_
#define CURVE25519_PLUGIN_H_

#include <plugins/plugin.h>

typedef struct curve25519_plugin_t curve25519_plugin_t;

/**
 * Plugin providing a Curve25519 DH implementation
 */
struct curve25519_plugin_t {

	/**
	 * implements plugin interface
	 */
	plugin_t plugin;
};

#endif /** CURVE25519_PLUGIN_H_ @}*/
