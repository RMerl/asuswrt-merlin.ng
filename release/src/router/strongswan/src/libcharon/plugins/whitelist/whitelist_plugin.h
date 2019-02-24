/*
 * Copyright (C) 2011 Martin Willi
 * Copyright (C) 2011 revosec AG
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
 * @defgroup whitelist whitelist
 * @ingroup cplugins
 *
 * @defgroup whitelist_plugin whitelist_plugin
 * @{ @ingroup whitelist
 */

#ifndef WHITELIST_PLUGIN_H_
#define WHITELIST_PLUGIN_H_

#include <plugins/plugin.h>

typedef struct whitelist_plugin_t whitelist_plugin_t;

/**
 * Peer identity whitelisting plugin.
 */
struct whitelist_plugin_t {

	/**
	 * Implements plugin interface.
	 */
	plugin_t plugin;
};

#endif /** WHITELIST_PLUGIN_H_ @}*/
