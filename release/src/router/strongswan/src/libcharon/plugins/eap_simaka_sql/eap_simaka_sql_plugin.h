/*
 * Copyright (C) 2010 Martin Willi
 * Copyright (C) 2010 revosec AG
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
 * @defgroup eap_simaka_sql eap_simaka_sql
 * @ingroup cplugins
 *
 * @defgroup eap_simaka_sql_plugin eap_simaka_sql_plugin
 * @{ @ingroup eap_simaka_sql
 */

#ifndef EAP_SIMAKA_SQL_PLUGIN_H_
#define EAP_SIMAKA_SQL_PLUGIN_H_

#include <plugins/plugin.h>

typedef struct eap_simaka_sql_plugin_t eap_simaka_sql_plugin_t;

/**
 * Plugin to provide SIM/AKA cards/providers using triplets from a database.
 */
struct eap_simaka_sql_plugin_t {

	/**
	 * Implements plugin interface
	 */
	plugin_t plugin;
};

#endif /** EAP_SIMAKA_SQL_PLUGIN_H_ @}*/
