/*
 * Copyright (C) 2017 Tobias Brunner
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
 * @defgroup counters counters
 * @ingroup cplugins
 *
 * @defgroup counters_plugin counters_plugin
 * @{ @ingroup counters
 */

#ifndef COUNTERS_PLUGIN_H_
#define COUNTERS_PLUGIN_H_

#include <plugins/plugin.h>

typedef struct counters_plugin_t counters_plugin_t;

/**
 * Plugin collecting several IKE event counters.
 *
 * Interested components can query individual counters via the 'counters'
 * object registered on lib that implements the counters_query.h interface.
 */
struct counters_plugin_t {

	/**
	 * Implements plugin_t. interface.
	 */
	plugin_t plugin;
};

#endif /** COUNTERS_PLUGIN_H_ @}*/
