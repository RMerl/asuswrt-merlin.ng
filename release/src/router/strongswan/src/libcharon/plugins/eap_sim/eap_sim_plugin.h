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
 * @defgroup eap_sim eap_sim
 * @ingroup cplugins
 *
 * @defgroup eap_sim_plugin eap_sim_plugin
 * @{ @ingroup eap_sim
 */

#ifndef EAP_SIM_PLUGIN_H_
#define EAP_SIM_PLUGIN_H_

#include <plugins/plugin.h>

typedef struct eap_sim_plugin_t eap_sim_plugin_t;

/**
 * EAP-SIM plugin.
 *
 * This plugin implements the protocol level of EAP-SIM and uses simaka_card_t
 * and simaka_provider_t backends to provide triplets. It registers a
 * simaka_manager_t on the library as "sim-manager", other plugins can use it
 * to provide the required backends.
 */
struct eap_sim_plugin_t {

	/**
	 * implements plugin interface
	 */
	plugin_t plugin;
};

#endif /** EAP_SIM_PLUGIN_H_ @}*/
