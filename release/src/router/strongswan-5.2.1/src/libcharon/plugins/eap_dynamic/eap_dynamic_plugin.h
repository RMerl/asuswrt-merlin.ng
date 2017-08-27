/*
 * Copyright (C) 2012 Tobias Brunner
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
 * @defgroup eap_dynamic eap_dynamic
 * @ingroup cplugins
 *
 * @defgroup eap_dynamic_plugin eap_dynamic_plugin
 * @{ @ingroup eap_dynamic
 */

#ifndef EAP_DYNAMIC_PLUGIN_H_
#define EAP_DYNAMIC_PLUGIN_H_

#include <plugins/plugin.h>

typedef struct eap_dynamic_plugin_t eap_dynamic_plugin_t;

/**
 * EAP plugin that can use any supported EAP method the client supports or
 * prefers to use.
 */
struct eap_dynamic_plugin_t {

	/**
	 * implements plugin interface
	 */
	plugin_t plugin;
};

#endif /** EAP_DYNAMIC_PLUGIN_H_ @}*/
