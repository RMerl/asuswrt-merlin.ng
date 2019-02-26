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
 * @defgroup eap_gtc_i eap_gtc
 * @{ @ingroup eap_gtc
 */

#ifndef EAP_GTC_H_
#define EAP_GTC_H_

typedef struct eap_gtc_t eap_gtc_t;

#include <sa/eap/eap_method.h>

/**
 * Implementation of the eap_method_t interface using EAP-GTC.
 *
 * This implementation of draft-sheffer-ikev2-gtc-00.txt uses PAM to
 * verify user credentials.
 */
struct eap_gtc_t {

	/**
	 * Implemented eap_method_t interface.
	 */
	eap_method_t eap_method_interface;
};

/**
 * Creates the EAP method EAP-GTC acting as server.
 *
 * @param server	ID of the EAP server
 * @param peer		ID of the EAP client
 * @return			eap_gtc_t object
 */
eap_gtc_t *eap_gtc_create_server(identification_t *server, identification_t *peer);

/**
 * Creates the EAP method EAP-GTC acting as peer.
 *
 * @param server	ID of the EAP server
 * @param peer		ID of the EAP client
 * @return			eap_gtc_t object
 */
eap_gtc_t *eap_gtc_create_peer(identification_t *server, identification_t *peer);

#endif /** EAP_GTC_H_ @}*/
