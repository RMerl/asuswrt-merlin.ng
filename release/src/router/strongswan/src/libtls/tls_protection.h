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
 * @defgroup tls_protection tls_protection
 * @{ @ingroup libtls
 */

#ifndef TLS_PROTECTION_H_
#define TLS_PROTECTION_H_

#include <library.h>

typedef struct tls_protection_t tls_protection_t;

#include "tls.h"
#include "tls_aead.h"
#include "tls_alert.h"
#include "tls_compression.h"

/**
 * TLS record protocol protection layer.
 */
struct tls_protection_t {

	/**
	 * Process a protected TLS record, pass it to upper layers.
	 *
	 * @param type		type of the TLS record to process
	 * @param data		associated TLS record data
	 * @return
	 *					- SUCCESS if TLS negotiation complete
	 *					- FAILED if TLS handshake failed
	 *					- NEED_MORE if more invocations to process/build needed
	 */
	status_t (*process)(tls_protection_t *this,
						tls_content_type_t type, chunk_t data);

	/**
	 * Query upper layer for TLS record, build protected record.
	 *
	 * @param type		type of the built TLS record
	 * @param data		allocated data of the built TLS record
	 * @return
	 *					- SUCCESS if TLS negotiation complete
	 *					- FAILED if TLS handshake failed
	 *					- NEED_MORE if upper layers have more records to send
	 *					- INVALID_STATE if more input records required
	 */
	status_t (*build)(tls_protection_t *this,
					  tls_content_type_t *type, chunk_t *data);

	/**
	 * Set a new transforms to use at protection layer
	 *
	 * @param inbound	TRUE to use cipher for inbound data, FALSE for outbound
	 * @param aead		new AEAD transform
	 */
	void (*set_cipher)(tls_protection_t *this, bool inbound, tls_aead_t *aead);

	/**
	 * Set the TLS version negotiated, used for MAC calculation.
	 *
	 * @param version	TLS version negotiated
	 */
	void (*set_version)(tls_protection_t *this, tls_version_t version);

	/**
	 * Destroy a tls_protection_t.
	 */
	void (*destroy)(tls_protection_t *this);
};

/**
 * Create a tls_protection instance.
 *
 * @param compression		compression layer of TLS stack
 * @param alert				TLS alert handler
 * @return					TLS protection layer.
 */
tls_protection_t *tls_protection_create(tls_compression_t *compression,
										tls_alert_t *alert);

#endif /** TLS_PROTECTION_H_ @}*/
