/*
 * Copyright (C) 2009 Martin Willi
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
 * @defgroup eap_radius eap_radius
 * @ingroup cplugins
 *
 * @defgroup eap_radius_plugin eap_radius_plugin
 * @{ @ingroup eap_radius
 */

#ifndef EAP_RADIUS_PLUGIN_H_
#define EAP_RADIUS_PLUGIN_H_

#include <plugins/plugin.h>

#include <radius_client.h>
#include <daemon.h>

typedef struct eap_radius_plugin_t eap_radius_plugin_t;

/**
 * EAP RADIUS proxy plugin.
 *
 * This plugin provides not a single EAP method, but a proxy to forwared
 * EAP packets to a RADIUS server. It only provides server implementations.
 */
struct eap_radius_plugin_t {

	/**
	 * implements plugin interface
	 */
	plugin_t plugin;
};

/**
 * Get a RADIUS client instance to connect to servers.
 *
 * @return			RADIUS client
 */
radius_client_t *eap_radius_create_client();

/**
 * Handle a RADIUS request timeout.
 *
 * If an IKE_SA is given, it gets deleted (unless the policy says to delete
 * any established IKE_SA).
 *
 * @param id		associated IKE_SA where timeout happened, or NULL
 */
void eap_radius_handle_timeout(ike_sa_id_t *id);

#endif /** EAP_RADIUS_PLUGIN_H_ @}*/
