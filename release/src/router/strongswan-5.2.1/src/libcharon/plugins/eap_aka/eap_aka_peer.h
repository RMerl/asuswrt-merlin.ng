/*
 * Copyright (C) 2008-2009 Martin Willi
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
 * @defgroup eap_aka_peer eap_aka_peer
 * @{ @ingroup eap_aka
 */

#ifndef EAP_AKA_PEER_H_
#define EAP_AKA_PEER_H_

typedef struct eap_aka_peer_t eap_aka_peer_t;

#include <sa/eap/eap_method.h>

/**
 * EAP-AKA peer implementation.
 */
struct eap_aka_peer_t {

	/**
	 * Implemented eap_method_t interface.
	 */
	eap_method_t interface;
};

/**
 * Creates the peer implementation of the EAP method EAP-AKA.
 *
 * @param server	ID of the EAP server
 * @param peer		ID of the EAP client
 * @return			eap_aka_peer_t object
 */
eap_aka_peer_t *eap_aka_peer_create(identification_t *server,
									identification_t *peer);

#endif /** EAP_AKA_PEER_H_ @}*/
