/*
 * Copyright (C) 2010-2012 Andreas Steffen
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
 * @defgroup eap_tnc_i eap_tnc
 * @{ @ingroup eap_tnc
 */

#ifndef EAP_TNC_H_
#define EAP_TNC_H_

typedef struct eap_tnc_t eap_tnc_t;

#include <sa/eap/eap_inner_method.h>

/**
 * Implementation of the eap_method_t interface using EAP-TNC or PT-EAP.
 */
struct eap_tnc_t {

	/**
	 * Implemented eap_inner_method_t interface.
	 */
	eap_inner_method_t eap_inner_method;
};

/**
 * Creates the EAP method EAP-TNC acting as server.
 *
 * @param server	ID of the EAP server
 * @param peer		ID of the EAP client
 * @return			eap_tnc_t object
 */
eap_tnc_t *eap_tnc_create_server(identification_t *server,
								 identification_t *peer);

/**
 * Creates the EAP method EAP-TNC acting as peer.
 *
 * @param server	ID of the EAP server
 * @param peer		ID of the EAP client
 * @return			eap_tnc_t object
 */
eap_tnc_t *eap_tnc_create_peer(identification_t *server,
							   identification_t *peer);

/**
 * Creates the EAP method PT-EAP acting as server.
 *
 * @param server	ID of the EAP server
 * @param peer		ID of the EAP client
 * @return			eap_tnc_t object
 */
eap_tnc_t *eap_tnc_pt_create_server(identification_t *server,
									identification_t *peer);

/**
 * Creates the EAP method PT-EAP acting as peer.
 *
 * @param server	ID of the EAP server
 * @param peer		ID of the EAP client
 * @return			eap_tnc_t object
 */
eap_tnc_t *eap_tnc_pt_create_peer(identification_t *server,
								  identification_t *peer);

#endif /** EAP_TNC_H_ @}*/
