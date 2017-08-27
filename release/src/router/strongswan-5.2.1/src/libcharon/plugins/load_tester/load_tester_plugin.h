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
 * @defgroup load_tester load_tester
 * @ingroup cplugins
 *
 * @defgroup load_tester_plugin load_tester_plugin
 * @{ @ingroup load_tester
 */

#ifndef LOAD_TESTER_PLUGIN_H_
#define LOAD_TESTER_PLUGIN_H_

#include <plugins/plugin.h>

typedef struct load_tester_plugin_t load_tester_plugin_t;

/**
 * Load tester plugin to inspect system core under high load.
 *
 * This plugin
 */
struct load_tester_plugin_t {

	/**
	 * implements plugin interface
	 */
	plugin_t plugin;
};

#endif /** LOAD_TESTER_PLUGIN_H_ @}*/
