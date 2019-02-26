/*
 * Copyright (C) 2015 Martin Willi
 * Copyright (C) 2015 revosec AG
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
 * @defgroup chapoly chapoly
 * @ingroup plugins
 *
 * @defgroup chapoly_plugin chapoly_plugin
 * @{ @ingroup chapoly
 */

#ifndef CHAPOLY_PLUGIN_H_
#define CHAPOLY_PLUGIN_H_

#include <plugins/plugin.h>

typedef struct chapoly_plugin_t chapoly_plugin_t;

/**
 * Plugin providing a ChaCha20/Poly1305 AEAD.
 */
struct chapoly_plugin_t {

	/**
	 * Implements plugin interface.
	 */
	plugin_t plugin;
};

#endif /** CHAPOLY_PLUGIN_H_ @}*/
