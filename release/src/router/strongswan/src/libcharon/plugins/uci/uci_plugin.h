/*
 * Copyright (C) 2008 Thomas Kallenberg
 * Copyright (C) 2008 Martin Willi
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
 * @defgroup uci uci
 * @ingroup cplugins
 *
 * @defgroup uci_plugin uci_plugin
 * @{ @ingroup uci
 */

#ifndef UCI_PLUGIN_H_
#define UCI_PLUGIN_H_

#include <plugins/plugin.h>

typedef struct uci_plugin_t uci_plugin_t;

/**
 * OpenWRT UCI (Unified Configuration Interface) configuration plugin.
 */
struct uci_plugin_t {

	/**
	 * implements plugin interface
	 */
	plugin_t plugin;
};

#endif /** UCI_PLUGIN_H_ @}*/
