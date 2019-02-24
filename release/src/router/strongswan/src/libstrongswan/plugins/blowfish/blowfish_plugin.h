/*
 * Copyright (C) 2008 Martin Willi
 * Copyright (C) 2009 Andreas Steffen
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
 * @defgroup blowfish_p blowfish
 * @ingroup plugins
 *
 * @defgroup blowfish_plugin blowfish_plugin
 * @{ @ingroup blowfish_p
 */

#ifndef BLOWFISH_PLUGIN_H_
#define BLOWFISH_PLUGIN_H_

#include <plugins/plugin.h>

typedef struct blowfish_plugin_t blowfish_plugin_t;

/**
 * Plugin implementing Blowfish based algorithms in software.
 */
struct blowfish_plugin_t {

	/**
	 * implements plugin interface
	 */
	plugin_t plugin;
};

#endif /** BLOWFISH_PLUGIN_H_ @}*/
