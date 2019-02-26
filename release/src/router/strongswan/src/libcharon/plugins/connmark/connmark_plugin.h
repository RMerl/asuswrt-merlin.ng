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
 * @defgroup connmark connmark
 * @ingroup cplugins
 *
 * @defgroup connmark_plugin connmark_plugin
 * @{ @ingroup connmark
 */

#ifndef CONNMARK_PLUGIN_H_
#define CONNMARK_PLUGIN_H_

#include <plugins/plugin.h>

typedef struct connmark_plugin_t connmark_plugin_t;

/**
 * Plugin using marks to select return path SA based on conntrack.
 */
struct connmark_plugin_t {

	/**
	 * implements plugin interface
	 */
	plugin_t plugin;
};

#endif /** CONNMARK_PLUGIN_H_ @}*/
