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
 * @defgroup radius_socket radius_socket
 * @{ @ingroup libradius
 */

#ifndef RADIUS_SOCKET_H_
#define RADIUS_SOCKET_H_

typedef struct radius_socket_t radius_socket_t;

#include "radius_message.h"

#include <networking/host.h>

/**
 * RADIUS socket to a server.
 */
struct radius_socket_t {

	/**
	 * Send a RADIUS request, wait for response.
	 *
	 * The socket fills in RADIUS Message identifier, builds a
	 * Request-Authenticator and calculates the Message-Authenticator
	 * attribute.
	 * The received response gets verified using the Response-Identifier
	 * and the Message-Authenticator attribute.
	 *
	 * @param request		request message
	 * @return				response message, NULL if timed out
	 */
	radius_message_t* (*request)(radius_socket_t *this,
								 radius_message_t *request);

	/**
	 * Decrypt the MSK encoded in a messages MS-MPPE-Send/Recv-Key.
	 *
	 * @param request		associated RADIUS request message
	 * @param response		RADIUS response message containing attributes
	 * @return				allocated MSK, empty chunk if none found
	 */
	chunk_t (*decrypt_msk)(radius_socket_t *this, radius_message_t *request,
						   radius_message_t *response);

	/**
	 * Destroy a radius_socket_t.
	 */
	void (*destroy)(radius_socket_t *this);
};

/**
 * Create a radius_socket instance.
 *
 * @param address	server name
 * @param auth_port	server port for authentication
 * @param acct_port	server port for accounting
 * @param secret	RADIUS secret
 */
radius_socket_t *radius_socket_create(char *address, u_int16_t auth_port,
									  u_int16_t acct_port, chunk_t secret);

#endif /** RADIUS_SOCKET_H_ @}*/
