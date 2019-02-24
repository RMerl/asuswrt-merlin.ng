/*
 * Copyright (C) 2012 Andreas Steffen
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
 * @defgroup tnc_pdp_connections tnc_pdp_connections
 * @{ @ingroup tnc_pdp
 */

#ifndef TNC_PDP_CONNECTIONS_H_
#define TNC_PDP_CONNECTIONS_H_

typedef struct tnc_pdp_connections_t tnc_pdp_connections_t;

#include <library.h>
#include <sa/ike_sa.h>
#include <sa/eap/eap_method.h>

/**
 * Public interface of a tnc_pdp_connections object
 */
struct tnc_pdp_connections_t {

	/**
	 * Register a new TNC PEP RADIUS Connection
	 *
	 * @param nas_id		NAS identifier of Policy Enforcement Point
	 * @param user_name		User name of TNC Client
	 * @param peer			Peer identity
	 * @param method		EAP method state for this TNC PEP Connection
	 */
	void (*add)(tnc_pdp_connections_t *this, chunk_t nas_id, chunk_t user_name,
				identification_t *peer, eap_method_t *method);

	/**
	 * Remove a TNC PEP RADIUS Connection
	 *
	 * @param nas_id		NAS identifier of Policy Enforcement Point
	 * @param user_name		User name of TNC Client
	 */
	void (*remove)(tnc_pdp_connections_t *this, chunk_t nas_id,
				   chunk_t user_name);

	/**
	 * Get the EAP method and IKE_SA of a registered TNC PEP RADIUS Connection.
	 *
	 * If this call succeeds, the connection manager is locked. Call unlock
	 * after using the return objects.
	 *
	 * @param nas_id		NAS identifier of Policy Enforcement Point
	 * @param user_name		User name of TNC Client
	 * @param ike_sa		IKE_SA used for bus communication only
	 * @return				EAP method for this connection or NULL if not found
	 */
	eap_method_t* (*get_state)(tnc_pdp_connections_t *this, chunk_t nas_id,
							   chunk_t user_name, ike_sa_t **ike_sa);

	/**
	 * Unlock connections after successfully calling get_state().
	 */
	void (*unlock)(tnc_pdp_connections_t *this);

	/**
	 * Destroys a tnc_pdp_connections_t object.
	 */
	void (*destroy)(tnc_pdp_connections_t *this);
};

/**
 * Create a tnc_pdp_connections_t instance
 */
tnc_pdp_connections_t* tnc_pdp_connections_create(void);

#endif /** TNC_PDP_CONNECTIONS_PLUGIN_H_ @}*/
