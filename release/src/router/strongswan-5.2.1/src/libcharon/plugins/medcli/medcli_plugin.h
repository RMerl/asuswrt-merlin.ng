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
 * @defgroup medcli medcli
 * @ingroup cplugins
 *
 * @defgroup medcli_plugin medcli_plugin
 * @{ @ingroup medcli
 */

#ifndef MEDCLI_PLUGIN_H_
#define MEDCLI_PLUGIN_H_

#include <plugins/plugin.h>

typedef struct medcli_plugin_t medcli_plugin_t;

/**
 * Mediation client database plugin.
 */
struct medcli_plugin_t {

	/**
	 * implements plugin interface
	 */
	plugin_t plugin;
};

#endif /** MEDCLI_PLUGIN_H_ @}*/
