/*
 * Copyright (C) 2011-2012 Reto Guadagnini
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
 * @defgroup unbound_p unbound
 * @ingroup plugins
 *
 * @defgroup unbound_plugin unbound_plugin
 * @{ @ingroup unbound_p
 */

#ifndef unbound_PLUGIN_H_
#define unbound_PLUGIN_H_

#include <plugins/plugin.h>

typedef struct unbound_plugin_t unbound_plugin_t;

/**
 * Plugin implementing the resolver interface using the libunbound DNS library.
 */
struct unbound_plugin_t {

	/**
	 * implements plugin interface
	 */
	plugin_t plugin;
};

#endif /** unbound_PLUGIN_H_ @}*/
