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
 * @defgroup tls_server tls_server
 * @{ @ingroup libtls
 */

#ifndef TLS_SERVER_H_
#define TLS_SERVER_H_

typedef struct tls_server_t tls_server_t;

#include "tls_handshake.h"
#include "tls_crypto.h"

#include <library.h>

/**
 * TLS handshake protocol handler as peer.
 */
struct tls_server_t {

	/**
	 * Implements the TLS handshake protocol handler.
	 */
	tls_handshake_t handshake;
};

/**
 * Create a tls_server instance.
 *
 * If a peer identity is given, the client must authenticate with a valid
 * certificate for this identity, or the connection fails. If peer is NULL,
 * but the client authenticates nonetheless, the authenticated identity
 * gets returned by tls_handshake_t.get_peer_id().
 *
 * @param tls		TLS stack
 * @param crypto	TLS crypto helper
 * @param alert		TLS alert handler
 * @param server	server identity
 * @param peer		peer identity, or NULL
 */
tls_server_t *tls_server_create(tls_t *tls,
						tls_crypto_t *crypto, tls_alert_t *alert,
						identification_t *server, identification_t *peer);

#endif /** TLS_SERVER_H_ @}*/
