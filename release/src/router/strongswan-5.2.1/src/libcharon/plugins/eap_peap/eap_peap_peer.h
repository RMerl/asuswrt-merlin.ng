/*
 * Copyright (C) 2011 Andreas Steffen
 * Copyright (C) 2011 HSR Hochschule fuer Technik Rapperswil
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
 * @defgroup eap_peap_peer eap_peap_peer
 * @{ @ingroup eap_peap
 */

#ifndef EAP_PEAP_PEER_H_
#define EAP_PEAP_PEER_H_

typedef struct eap_peap_peer_t eap_peap_peer_t;

#include "tls_application.h"

#include <library.h>
#include <sa/eap/eap_method.h>

/**
 * TLS application data handler as peer.
 */
struct eap_peap_peer_t {

	/**
	 * Implements the TLS application data handler.
	 */
	tls_application_t application;
};

/**
 * Create an eap_peap_peer instance.
 */
eap_peap_peer_t *eap_peap_peer_create(identification_t *server,
									  identification_t *peer,
									  eap_method_t *eap_method);

#endif /** EAP_PEAP_PEER_H_ @}*/
