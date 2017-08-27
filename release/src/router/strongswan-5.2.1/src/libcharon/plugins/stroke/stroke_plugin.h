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
 * @defgroup stroke stroke
 * @ingroup cplugins
 *
 * @defgroup stroke_plugin stroke_plugin
 * @{ @ingroup stroke
 */

#ifndef STROKE_PLUGIN_H_
#define STROKE_PLUGIN_H_

#include <plugins/plugin.h>

typedef struct stroke_plugin_t stroke_plugin_t;

/**
 * strongSwan 2.x style configuration and control interface.
 *
 * Stroke is a home-brewed communication interface inspired by whack. It
 * uses a unix socket (/var/run/charon.ctl).
 */
struct stroke_plugin_t {

	/**
	 * implements plugin interface
	 */
	plugin_t plugin;
};

#endif /** STROKE_PLUGIN_H_ @}*/
