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
 * @defgroup tls_handshake tls_handshake
 * @{ @ingroup libtls
 */

#ifndef TLS_HANDSHAKE_H_
#define TLS_HANDSHAKE_H_

typedef struct tls_handshake_t tls_handshake_t;

#include "tls.h"

#include <bio/bio_reader.h>
#include <bio/bio_writer.h>

/**
 * TLS handshake state machine interface.
 */
struct tls_handshake_t {

	/**
	 * Process received TLS handshake message.
	 *
	 * @param type		TLS handshake message type
	 * @param reader	TLS data buffer
	 * @return
	 *					- SUCCESS if TLS negotiation complete
	 *					- FAILED if a fatal TLS alert queued
	 *					- NEED_MORE if more invocations to process/build needed
	 *					- DESTROY_ME if a fatal TLS alert received
	 */
	status_t (*process)(tls_handshake_t *this,
						tls_handshake_type_t type, bio_reader_t *reader);

	/**
	 * Build TLS handshake messages to send out.
	 *
	 * @param type		type of created handshake message
	 * @param writer	TLS data buffer to write to
	 * @return
	 *					- SUCCESS if handshake complete
	 *					- FAILED if handshake failed
	 *					- NEED_MORE if more messages ready for delivery
	 *					- INVALID_STATE if more input to process() required
	 */
	status_t (*build)(tls_handshake_t *this,
					  tls_handshake_type_t *type, bio_writer_t *writer);

	/**
	 * Check if the cipher spec should be changed for outgoing messages.
	 *
	 * @param inbound	TRUE to check for inbound cipherspec change
	 * @return			TRUE if cipher spec should be changed
	 */
	bool (*cipherspec_changed)(tls_handshake_t *this, bool inbound);

	/**
	 * Change the cipher for a direction.
	 *
	 * @param inbound	TRUE to change inbound cipherspec, FALSE for outbound
	 */
	void (*change_cipherspec)(tls_handshake_t *this, bool inbound);

	/**
	 * Check if the finished message was decoded successfully.
	 *
	 * @return			TRUE if finished message was decoded successfully
	 */
	bool (*finished)(tls_handshake_t *this);

	/**
	 * Get the peer identity authenticated/to authenticate during handshake.
	 *
	 * @return			peer identity
	 */
	identification_t* (*get_peer_id)(tls_handshake_t *this);

	/**
	 * Get the server identity authenticated/to authenticate during handshake.
	 *
	 * @return			server identity
	 */
	identification_t* (*get_server_id)(tls_handshake_t *this);

	/**
	 * Get the peers authentication information after completing the handshake.
	 *
	 * @return			authentication data, internal data
	 */
	auth_cfg_t* (*get_auth)(tls_handshake_t *this);

	/**
	 * Destroy a tls_handshake_t.
	 */
	void (*destroy)(tls_handshake_t *this);
};

#endif /** TLS_HANDSHAKE_H_ @}*/
