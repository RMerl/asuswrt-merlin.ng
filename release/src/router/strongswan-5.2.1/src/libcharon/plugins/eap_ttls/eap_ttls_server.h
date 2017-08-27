/*
 * Copyright (C) 2010 Andreas Steffen
 * Copyright (C) 2010 HSR Hochschule fuer Technik Rapperswil
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
 * @defgroup eap_ttls_server eap_ttls_server
 * @{ @ingroup eap_ttls
 */

#ifndef EAP_TTLS_SERVER_H_
#define EAP_TTLS_SERVER_H_

typedef struct eap_ttls_server_t eap_ttls_server_t;

#include "tls_application.h"

#include <library.h>

/**
 * TLS application data handler as server.
 */
struct eap_ttls_server_t {

	/**
	 * Implements the TLS application data handler.
	 */
	tls_application_t application;
};

/**
 * Create an eap_ttls_server instance.
 */
eap_ttls_server_t *eap_ttls_server_create(identification_t *server,
										  identification_t *peer);

#endif /** EAP_TTLS_SERVER_H_ @}*/
