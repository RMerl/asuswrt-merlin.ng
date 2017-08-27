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
 * @defgroup radius_client radius_client
 * @{ @ingroup libradius
 */

#ifndef RADIUS_CLIENT_H_
#define RADIUS_CLIENT_H_

#include "radius_message.h"
#include "radius_config.h"

typedef struct radius_client_t radius_client_t;

/**
 * RADIUS client functionality.
 *
 * To communicate with a RADIUS server, create a client and send messages over
 * it. The client allocates a socket from the best RADIUS server abailable.
 */
struct radius_client_t {

	/**
	 * Send a RADIUS request and wait for the response.
	 *
	 * The client fills in NAS-Identifier nad NAS-Port-Type
	 *
	 * @param msg			RADIUS request message to send
	 * @return				response, NULL if timed out/verification failed
	 */
	radius_message_t* (*request)(radius_client_t *this, radius_message_t *msg);

	/**
	 * Get the EAP MSK after successful RADIUS authentication.
	 *
	 * @return				MSK, allocated
	 */
	chunk_t (*get_msk)(radius_client_t *this);

	/**
	 * Destroy the client, release the socket.
	 */
	void (*destroy)(radius_client_t *this);
};

/**
 * Create a RADIUS client.
 *
 * @param config	reference to a server configuration, gets owned
 * @return			radius_client_t object
 */
radius_client_t *radius_client_create(radius_config_t *config);

#endif /** RADIUS_CLIENT_H_ @}*/
