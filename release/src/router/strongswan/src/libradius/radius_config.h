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

/*
 * Copyright (C) 2015 Thom Troy
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

/**
 * @defgroup radius_config radius_config
 * @{ @ingroup libradius
 */

#ifndef RADIUS_CONFIG_H_
#define RADIUS_CONFIG_H_

typedef struct radius_config_t radius_config_t;

#include "radius_socket.h"

/**
 * RADIUS server configuration.
 */
struct radius_config_t {

	/**
	 * Get a RADIUS socket from the pool to communicate with this config.
	 *
	 * @return			RADIUS socket
	 */
	radius_socket_t* (*get_socket)(radius_config_t *this);

	/**
	 * Release a socket to the pool after use.
	 *
	 * @param skt		RADIUS socket to release
	 * @param result	result of the socket use, TRUE for success
	 */
	void (*put_socket)(radius_config_t *this, radius_socket_t *skt, bool result);

	/**
	 * Get the NAS-Identifier to use with this server.
	 *
	 * @return			NAS-Identifier, internal data
	 */
	chunk_t (*get_nas_identifier)(radius_config_t *this);

	/**
	 * Get the preference of this server.
	 *
	 * Based on the available sockets and the server reachability a preference
	 * value is calculated: better servers return a higher value.
	 */
	int (*get_preference)(radius_config_t *this);

	/**
	 * Get the name of the RADIUS server.
	 *
	 * @return			server name
	 */
	char* (*get_name)(radius_config_t *this);

	/**
	 * Increase reference count of this server configuration.
	 *
	 * @return			this
	 */
	radius_config_t* (*get_ref)(radius_config_t *this);

	/**
	 * Destroy a radius_config_t.
	 */
	void (*destroy)(radius_config_t *this);
};

/**
 * Create a radius_config_t instance.
 *
 * @param name				server name
 * @param address			server address
 * @param auth_port			server port for authentication
 * @param acct_port			server port for accounting
 * @param nas_identifier	NAS-Identifier to use with this server
 * @param secret			secret to use with this server
 * @param sockets			number of sockets to create in pool
 * @param preference		preference boost for this server
 * @param tries				number of times we retransmit messages
 * @param timeout			retransmission timeout
 * @param base				base to calculate retransmission timeout
 */
radius_config_t *radius_config_create(char *name, char *address,
									  uint16_t auth_port, uint16_t acct_port,
									  char *nas_identifier, char *secret,
									  int sockets, int preference,
									  u_int tries, double timeout, double base);

#endif /** RADIUS_CONFIG_H_ @}*/
