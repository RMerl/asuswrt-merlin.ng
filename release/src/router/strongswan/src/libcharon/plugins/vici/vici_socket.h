/*
 * Copyright (C) 2014 Martin Willi
 * Copyright (C) 2014 revosec AG
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
 * @defgroup vici_socket vici_socket
 * @{ @ingroup vici
 */

#ifndef VICI_SOCKET_H_
#define VICI_SOCKET_H_

#include <library.h>

/**
 * Maximum size of a single message exchanged.
 */
#define VICI_MESSAGE_SIZE_MAX (512 * 1024)

typedef struct vici_socket_t vici_socket_t;

/**
 * Callback function for dispatching inbound client messages.
 *
 * @param user		user data, as passed during registration
 * @param id		unique client connection identifier
 * @param data		incoming message data
 */
typedef void (*vici_inbound_cb_t)(void *user, u_int id, chunk_t data);

/**
 * Callback function invoked when new clients connect
 *
 * @param user		user data, as passed during registration
 * @param id		unique client connection identifier
 * @return			client connection context
 */
typedef void (*vici_connect_cb_t)(void *user, u_int id);

/**
 * Callback function invoked when connected clients disconnect
 *
 * @param user		user data, as passed during registration
 * @param id		unique client connection identifier
 */
typedef void (*vici_disconnect_cb_t)(void *user, u_int id);

/**
 * Vici socket, low level socket input/output handling.
 *
 * On the socket, we pass raw chunks having a 2 byte network order length
 * prefix. The length field does not count the length header itself, and
 * is not included in the data passed over this interface.
 */
struct vici_socket_t {

	/**
	 * Send a message to a client identified by connection identifier.
	 *
	 * @param id		unique client connection identifier
	 * @param data		data to send to client, gets owned
	 */
	void (*send)(vici_socket_t *this, u_int id, chunk_t data);

	/**
	 * Destroy socket.
	 */
	void (*destroy)(vici_socket_t *this);
};

/**
 * Create a vici_socket instance.
 *
 * @param uri			socket URI to listen on
 * @param inbound		inbound message callback
 * @param connect		connect callback
 * @param disconnect	disconnect callback
 * @param user			user data to pass to callbacks
 */
vici_socket_t *vici_socket_create(char *uri, vici_inbound_cb_t inbound,
								  vici_connect_cb_t connect,
								  vici_disconnect_cb_t disconnect, void *user);

#endif /** VICI_SOCKET_H_ @}*/
