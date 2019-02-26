/*
 * Copyright (C) 2012 Adrian-Ken Rueegsegger
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
 * @defgroup nonce_p nonce
 * @ingroup plugins
 *
 * @defgroup nonce_plugin nonce_plugin
 * @{ @ingroup nonce_p
 */

#ifndef NONCE_PLUGIN_H_
#define NONCE_PLUGIN_H_

#include <plugins/plugin.h>

typedef struct nonce_plugin_t nonce_plugin_t;

/**
 * Plugin implementing a nonce generator using an RNG.
 */
struct nonce_plugin_t {

	/**
	 * Implements plugin interface
	 */
	plugin_t plugin;
};

#endif /** NONCE_PLUGIN_H_ @}*/
