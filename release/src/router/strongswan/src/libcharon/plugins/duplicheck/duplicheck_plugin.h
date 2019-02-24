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
 * @defgroup duplicheck duplicheck
 * @ingroup cplugins
 *
 * @defgroup duplicheck_plugin duplicheck_plugin
 * @{ @ingroup duplicheck
 */

#ifndef DUPLICHECK_PLUGIN_H_
#define DUPLICHECK_PLUGIN_H_

#include <plugins/plugin.h>

typedef struct duplicheck_plugin_t duplicheck_plugin_t;

/**
 * Advanced duplicate checking using liveness checks.
 */
struct duplicheck_plugin_t {

	/**
	 * Implements plugin interface
	 */
	plugin_t plugin;
};

#endif /** DUPLICHECK_PLUGIN_H_ @}*/
