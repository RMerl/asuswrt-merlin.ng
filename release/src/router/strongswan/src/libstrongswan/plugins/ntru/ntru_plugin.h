/*
 * Copyright (C) 2013 Andreas Steffen
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
 * @defgroup ntru_p ntru
 * @ingroup plugins
 *
 * @defgroup ntru_plugin ntru_plugin
 * @{ @ingroup ntru_p
 */

#ifndef NTRU_PLUGIN_H_
#define NTRU_PLUGIN_H_

#include <plugins/plugin.h>

typedef struct ntru_plugin_t ntru_plugin_t;

/**
 * Plugin implementing NTRU-base key exchange
 */
struct ntru_plugin_t {

	/**
	 * implements plugin interface
	 */
	plugin_t plugin;
};

#endif /** NTRU_PLUGIN_H_ @}*/
