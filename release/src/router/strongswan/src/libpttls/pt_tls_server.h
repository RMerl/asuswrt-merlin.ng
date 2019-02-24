/*
 * Copyright (C) 2012 Martin Willi
 * Copyright (C) 2012 revosec AG
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
 * @defgroup pt_tls_server pt_tls_server
 * @{ @ingroup pt_tls
 */

#ifndef PT_TLS_SERVER_H_
#define PT_TLS_SERVER_H_

#include <utils/identification.h>

#include <tnc/tnccs/tnccs.h>

#include "pt_tls.h"

typedef struct pt_tls_server_t pt_tls_server_t;

/**
 * IF-T for TLS aka PT-TLS transport server.
 */
struct pt_tls_server_t {

	/**
	 * Handle assessment data read from socket.
	 *
	 * @return
	 *						- NEED_MORE if more exchanges required,
	 *						- SUCCESS if assessment complete
	 *						- FAILED if assessment failed
	 */
	status_t (*handle)(pt_tls_server_t *this);

	/**
	 * Get the underlying client connection socket.
	 *
	 * @return			socket fd, suitable to select()
	 */
	int (*get_fd)(pt_tls_server_t *this);

	/**
	 * Destroy a pt_tls_server_t.
	 */
	void (*destroy)(pt_tls_server_t *this);
};

/**
 * Create a pt_tls_server connection instance.
 *
 * @param server	TLS server identity
 * @param fd		client connection socket
 * @param auth		client authentication requirements
 * @param tnccs		inner TNCCS protocol handler to use for this connection
 * @return			PT-TLS server
 */
pt_tls_server_t *pt_tls_server_create(identification_t *server, int fd,
									  pt_tls_auth_t auth, tnccs_t *tnccs);

#endif /** PT_TLS_SERVER_H_ @}*/
