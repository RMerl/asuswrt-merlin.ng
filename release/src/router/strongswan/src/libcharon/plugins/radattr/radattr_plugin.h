/*
 * Copyright (C) 2012 Martin Willi
 * Copyright (C) 2012 revosec AG
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
 * @defgroup radattr radattr
 * @ingroup cplugins
 *
 * @defgroup radattr_plugin radattr_plugin
 * @{ @ingroup radattr
 */

#ifndef RADATTR_PLUGIN_H_
#define RADATTR_PLUGIN_H_

#include <plugins/plugin.h>

typedef struct radattr_plugin_t radattr_plugin_t;

/**
 * Plugin to inject/process custom RADIUS attributes.
 */
struct radattr_plugin_t {

	/**
	 * implements plugin interface
	 */
	plugin_t plugin;
};

#endif /** RADATTR_PLUGIN_H_ @}*/
