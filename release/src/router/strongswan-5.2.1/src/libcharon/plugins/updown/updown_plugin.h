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
 * @defgroup updown updown
 * @ingroup cplugins
 *
 * @defgroup updown_plugin updown_plugin
 * @{ @ingroup updown
 */

#ifndef UPDOWN_PLUGIN_H_
#define UPDOWN_PLUGIN_H_

#include <plugins/plugin.h>

typedef struct updown_plugin_t updown_plugin_t;

/**
 * Updown firewall script invocation plugin, compatible to pluto ones.
 */
struct updown_plugin_t {

	/**
	 * implements plugin interface
	 */
	plugin_t plugin;
};

#endif /** UPDOWN_PLUGIN_H_ @}*/
