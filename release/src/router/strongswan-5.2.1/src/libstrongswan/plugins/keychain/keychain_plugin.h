/*
 * Copyright (C) 2013 Martin Willi
 * Copyright (C) 2013 revosec AG
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
 * @defgroup keychain keychain
 * @ingroup plugins
 *
 * @defgroup keychain_plugin keychain_plugin
 * @{ @ingroup keychain
 */

#ifndef KEYCHAIN_PLUGIN_H_
#define KEYCHAIN_PLUGIN_H_

#include <plugins/plugin.h>

typedef struct keychain_plugin_t keychain_plugin_t;

/**
 * Plugin providing OS X Keychain Services support.
 */
struct keychain_plugin_t {

	/**
	 * Implements plugin interface,
	 */
	plugin_t plugin;
};

#endif /** KEYCHAIN_PLUGIN_H_ @}*/
