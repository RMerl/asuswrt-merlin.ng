/*
 * Copyright (C) 2008 Martin Willi
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
 * @defgroup ldap_p ldap
 * @ingroup plugins
 *
 * @defgroup ldap_plugin ldap_plugin
 * @{ @ingroup ldap_p
 */

#ifndef LDAP_PLUGIN_H_
#define LDAP_PLUGIN_H_

#include <plugins/plugin.h>

typedef struct ldap_plugin_t ldap_plugin_t;

/**
 * Plugin implementing LDAP fetcher using OpenLDAP.
 */
struct ldap_plugin_t {

	/**
	 * implements plugin interface
	 */
	plugin_t plugin;
};

#endif /** LDAP_PLUGIN_H_ @}*/
