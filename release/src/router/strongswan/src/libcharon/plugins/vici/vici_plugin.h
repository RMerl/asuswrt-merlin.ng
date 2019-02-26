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
 * @defgroup vici vici
 * @ingroup cplugins
 *
 * @defgroup vici_plugin vici_plugin
 * @{ @ingroup vici
 */

#ifndef VICI_PLUGIN_H_
#define VICI_PLUGIN_H_

#include <plugins/plugin.h>

typedef struct vici_plugin_t vici_plugin_t;

/**
 * vici plugin, the "Versatile IKE Control Interface" interface.
 */
struct vici_plugin_t {

	/**
	 * Implements plugin interface
	 */
	plugin_t plugin;
};

#endif /** VICI_PLUGIN_H_ @}*/
