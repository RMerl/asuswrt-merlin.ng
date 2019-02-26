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
 * @defgroup socket_default_socket socket_default_socket
 * @{ @ingroup socket_default
 */

#ifndef SOCKET_DEFAULT_SOCKET_H_
#define SOCKET_DEFAULT_SOCKET_H_

typedef struct socket_default_socket_t socket_default_socket_t;

#include <network/socket.h>

/**
 * Default socket, binds to port 500/4500 using any IPv4/IPv6 address.
 */
struct socket_default_socket_t {

	/**
	 * Implements the socket_t interface.
	 */
	socket_t socket;

};

/**
 * Create a socket_default_socket instance.
 */
socket_default_socket_t *socket_default_socket_create();

#endif /** SOCKET_DEFAULT_SOCKET_H_ @}*/
