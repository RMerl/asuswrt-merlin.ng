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
 * @defgroup eap_md5_i eap_md5
 * @{ @ingroup eap_md5
 */

#ifndef EAP_MD5_H_
#define EAP_MD5_H_

typedef struct eap_md5_t eap_md5_t;

#include <sa/eap/eap_method.h>

/**
 * Implementation of the eap_method_t interface using EAP-MD5 (CHAP).
 */
struct eap_md5_t {

	/**
	 * Implemented eap_method_t interface.
	 */
	eap_method_t eap_method;
};

/**
 * Creates the EAP method EAP-MD5 acting as server.
 *
 * @param server	ID of the EAP server
 * @param peer		ID of the EAP client
 * @return			eap_md5_t object
 */
eap_md5_t *eap_md5_create_server(identification_t *server, identification_t *peer);

/**
 * Creates the EAP method EAP-MD5 acting as peer.
 *
 * @param server	ID of the EAP server
 * @param peer		ID of the EAP client
 * @return			eap_md5_t object
 */
eap_md5_t *eap_md5_create_peer(identification_t *server, identification_t *peer);

#endif /** EAP_MD5_H_ @}*/
