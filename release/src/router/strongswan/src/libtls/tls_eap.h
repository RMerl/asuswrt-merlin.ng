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
 * @defgroup tls_eap tls_eap
 * @{ @ingroup libtls
 */

#ifndef TLS_EAP_H_
#define TLS_EAP_H_

typedef struct tls_eap_t tls_eap_t;

#include <eap/eap.h>

#include "tls.h"

/**
 * TLS over EAP helper, as used by EAP-TLS and EAP-TTLS.
 */
struct tls_eap_t {

	/**
	 * Initiate TLS/TTLS/TNC over EAP exchange (as client).
	 *
	 * @param out			allocated EAP packet data to send
	 * @return
	 *						- NEED_MORE if more exchanges required
	 *						- FAILED if initiation failed
	 */
	status_t (*initiate)(tls_eap_t *this, chunk_t *out);

	/**
	 * Process a received EAP-TLS/TTLS/TNC packet, create response.
	 *
	 * @param in			EAP packet data to process
	 * @param out			allocated EAP packet data to send
	 * @return
	 *						- SUCCESS if TLS negotiation completed
	 *						- FAILED if TLS negotiation failed
	 *						- NEED_MORE if more exchanges required
	 */
	status_t (*process)(tls_eap_t *this, chunk_t in, chunk_t *out);

	/**
	 * Get the EAP-MSK.
	 *
	 * @return				MSK
	 */
	chunk_t (*get_msk)(tls_eap_t *this);

	/**
	 * Get the current EAP identifier.
	 *
	 * @return				identifier
	 */
	uint8_t (*get_identifier)(tls_eap_t *this);

	/**
	 * Set the EAP identifier to a deterministic value, overwriting
	 * the randomly initialized default value.
	 *
	 * @param identifier	EAP identifier
	 */
	void (*set_identifier) (tls_eap_t *this, uint8_t identifier);

	/**
	 * Get the authentication details after completing the handshake.
	 *
	 * @return				authentication details, internal data
	 */
	auth_cfg_t* (*get_auth)(tls_eap_t *this);

	/**
	 * Destroy a tls_eap_t.
	 */
	void (*destroy)(tls_eap_t *this);
};

/**
 * Create a tls_eap instance.
 *
 * @param type				EAP type, EAP-TLS or EAP-TTLS
 * @param tls				TLS implementation
 * @param frag_size			maximum size of a TLS fragment we send
 * @param max_msg_count		maximum number of processed messages
 * @param include_length	if TRUE include length in non-fragmented packets
 */
tls_eap_t *tls_eap_create(eap_type_t type, tls_t *tls, size_t frag_size,
						  int max_msg_count, bool include_length);

#endif /** TLS_EAP_H_ @}*/
