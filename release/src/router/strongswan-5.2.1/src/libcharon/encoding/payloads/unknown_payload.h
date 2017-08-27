/*
 * Copyright (C) 2005-2006 Martin Willi
 * Copyright (C) 2005 Jan Hutter
 * Hochschule fuer Technik Rapperswil
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
 * @defgroup unknown_payload unknown_payload
 * @{ @ingroup payloads
 */

#ifndef UNKNOWN_PAYLOAD_H_
#define UNKNOWN_PAYLOAD_H_

typedef struct unknown_payload_t unknown_payload_t;

#include <library.h>
#include <encoding/payloads/payload.h>

/**
 * Payload which can't be processed further.
 *
 * When the parser finds an unknown payload, he builds an instance of
 * this class. This allows further processing of this payload, such as
 * a check for the critical bit in the header.
 */
struct unknown_payload_t {

	/**
	 * The payload_t interface.
	 */
	payload_t payload_interface;

	/**
	 * Get the raw data of this payload, without
	 * the generic payload header.
	 *
	 * Returned data are NOT copied and must not be freed.
	 *
	 * @return				data as chunk_t
	 */
	chunk_t (*get_data) (unknown_payload_t *this);

	/**
	 * Get the critical flag.
	 *
	 * @return				TRUE if payload is critical, FALSE if not
	 */
	bool (*is_critical) (unknown_payload_t *this);

	/**
	 * Destroys an unknown_payload_t object.
	 */
	void (*destroy) (unknown_payload_t *this);
};

/**
 * Creates an empty unknown_payload_t.
 *
 * @param			type of the payload
 * @return			unknown_payload_t object
 */
unknown_payload_t *unknown_payload_create(payload_type_t type);

/**
 * Create an unknown payload with data.
 *
 * @param type		type of payload to create
 * @param critical	TRUE to set critical bit
 * @param data		data to set for this payload, gets owned by payload
 * @return			payload object
 */
unknown_payload_t *unknown_payload_create_data(payload_type_t type,
											   bool critical, chunk_t data);

#endif /** UNKNOWN_PAYLOAD_H_ @}*/
