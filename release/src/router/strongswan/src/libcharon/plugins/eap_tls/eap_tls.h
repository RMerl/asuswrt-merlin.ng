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
 * @defgroup eap_tls_i eap_tls
 * @{ @ingroup eap_tls
 */

#ifndef EAP_TLS_H_
#define EAP_TLS_H_

typedef struct eap_tls_t eap_tls_t;

#include <sa/eap/eap_method.h>

/**
 * Implementation of eap_method_t using EAP-TLS.
 */
struct eap_tls_t {

	/**
	 * Implements eap_method_t interface.
	 */
	eap_method_t eap_method;
};

/**
 * Creates the EAP method EAP-TLS acting as server.
 *
 * @param server	ID of the EAP server
 * @param peer		ID of the EAP client
 * @return			eap_tls_t object
 */
eap_tls_t *eap_tls_create_server(identification_t *server,
								 identification_t *peer);

/**
 * Creates the EAP method EAP-TLS acting as peer.
 *
 * @param server	ID of the EAP server
 * @param peer		ID of the EAP client
 * @return			eap_tls_t object
 */
eap_tls_t *eap_tls_create_peer(identification_t *server,
							   identification_t *peer);

#endif /** EAP_TLS_H_ @}*/
