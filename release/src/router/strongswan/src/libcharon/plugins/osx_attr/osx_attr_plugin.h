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
 * @defgroup osx_attr osx_attr
 * @ingroup cplugins
 *
 * @defgroup osx_attr_plugin osx_attr_plugin
 * @{ @ingroup osx_attr
 */

#ifndef OSX_ATTR_PLUGIN_H_
#define OSX_ATTR_PLUGIN_H_

#include <plugins/plugin.h>

typedef struct osx_attr_plugin_t osx_attr_plugin_t;

/**
 * Plugin providing an OS X specific configuration attribute handler.
 */
struct osx_attr_plugin_t {

	/**
	 * Implements plugin interface.
	 */
	plugin_t plugin;
};

#endif /** OSX_ATTR_PLUGIN_H_ @}*/
