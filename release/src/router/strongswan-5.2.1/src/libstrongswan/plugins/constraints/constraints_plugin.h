/*
 * Copyright (C) 2010 Martin Willi
 * Copyright (C) 2010 revosec AG
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
 * @defgroup constraints constraints
 * @ingroup plugins
 *
 * @defgroup constraints_plugin constraints_plugin
 * @{ @ingroup constraints
 */

#ifndef CONSTRAINTS_PLUGIN_H_
#define CONSTRAINTS_PLUGIN_H_

#include <plugins/plugin.h>

typedef struct constraints_plugin_t constraints_plugin_t;

/**
 * Advanced X509 constraint checking.
 */
struct constraints_plugin_t {

	/**
	 * Implements plugin_t. interface.
	 */
	plugin_t plugin;
};

#endif /** CONSTRAINTS_PLUGIN_H_ @}*/
