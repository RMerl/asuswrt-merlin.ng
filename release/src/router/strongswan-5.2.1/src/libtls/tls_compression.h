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
 * @defgroup tls_compression tls_compression
 * @{ @ingroup libtls
 */

#ifndef TLS_COMPRESSION_H_
#define TLS_COMPRESSION_H_

#include <library.h>

typedef struct tls_compression_t tls_compression_t;

#include "tls.h"
#include "tls_alert.h"
#include "tls_fragmentation.h"

/**
 * TLS record protocol compression layer.
 */
struct tls_compression_t {

	/**
	 * Process a compressed TLS record, pass it to upper layers.
	 *
	 * @param type		type of the TLS record to process
	 * @param data		associated TLS record data
	 * @return
	 *					- SUCCESS if TLS negotiation complete
	 *					- FAILED if TLS handshake failed
	 *					- NEED_MORE if more invocations to process/build needed
	 */
	status_t (*process)(tls_compression_t *this,
						tls_content_type_t type, chunk_t data);

	/**
	 * Query upper layer for TLS record, build compressed record.
	 *
	 * @param type		type of the built TLS record
	 * @param data		allocated data of the built TLS record
	 * @return
	 *					- SUCCESS if TLS negotiation complete
	 *					- FAILED if TLS handshake failed
	 *					- NEED_MORE if upper layers have more records to send
	 *					- INVALID_STATE if more input records required
	 */
	status_t (*build)(tls_compression_t *this,
					  tls_content_type_t *type, chunk_t *data);

	/**
	 * Destroy a tls_compression_t.
	 */
	void (*destroy)(tls_compression_t *this);
};

/**
 * Create a tls_compression instance.
 *
 * @param fragmentation		fragmentation layer of TLS stack
 * @param alert				TLS alert handler
 * @return					TLS compression layer.
 */
tls_compression_t *tls_compression_create(tls_fragmentation_t *fragmentation,
										  tls_alert_t *alert);

#endif /** TLS_COMPRESSION_H_ @}*/
