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
 * @defgroup tls_fragmentation tls_fragmentation
 * @{ @ingroup libtls
 */

#ifndef TLS_FRAGMENTATION_H_
#define TLS_FRAGMENTATION_H_

#include <library.h>

typedef struct tls_fragmentation_t tls_fragmentation_t;

#include "tls.h"
#include "tls_alert.h"
#include "tls_handshake.h"

/**
 * TLS record protocol fragmentation layer.
 */
struct tls_fragmentation_t {

	/**
	 * Process a fragmented TLS record, pass it to upper layers.
	 *
	 * @param type		type of the TLS record to process
	 * @param data		associated TLS record data
	 * @return
	 *					- SUCCESS if TLS negotiation complete
	 *					- FAILED if TLS handshake failed
	 *					- NEED_MORE if more invocations to process/build needed
	 */
	status_t (*process)(tls_fragmentation_t *this,
						tls_content_type_t type, chunk_t data);

	/**
	 * Query upper layer for TLS messages, build fragmented records.
	 *
	 * @param type		type of the built TLS record
	 * @param data		allocated data of the built TLS record
	 * @return
	 *					- SUCCESS if TLS negotiation complete
	 *					- FAILED if TLS handshake failed
	 *					- NEED_MORE if upper layers have more records to send
	 *					- INVALID_STATE if more input records required
	 */
	status_t (*build)(tls_fragmentation_t *this,
					  tls_content_type_t *type, chunk_t *data);

	/**
	 * Has the application layer finished (returned SUCCESS)?.
	 *
	 * @return			TRUE if application layer finished
	 */
	bool (*application_finished)(tls_fragmentation_t *this);

	/**
	 * Destroy a tls_fragmentation_t.
	 */
	void (*destroy)(tls_fragmentation_t *this);
};

/**
 * Create a tls_fragmentation instance.
 *
 * @param handshake			upper layer handshake protocol
 * @param alert				TLS alert handler
 * @param application		upper layer application data or NULL
 * @param purpose			type of context this TLS stack is running in
 * @return					TLS fragmentation layer
 */
tls_fragmentation_t *tls_fragmentation_create(tls_handshake_t *handshake,
							tls_alert_t *alert, tls_application_t *application,
							tls_purpose_t purpose);

#endif /** TLS_FRAGMENTATION_H_ @}*/
