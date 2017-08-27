/*
 * Copyright (C) 2012 Tobias Brunner
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
 * @defgroup fragment_payload fragment_payload
 * @{ @ingroup payloads
 */

#ifndef FRAGMENT_PAYLOAD_H_
#define FRAGMENT_PAYLOAD_H_

typedef struct fragment_payload_t fragment_payload_t;

#include <library.h>
#include <encoding/payloads/payload.h>

/**
 * Object representing an IKEv1 fragment payload.
 */
struct fragment_payload_t {

	/**
	 * The payload_t interface.
	 */
	payload_t payload_interface;

	/**
	 * Get the fragment ID. Identifies the fragments for a particular IKE
	 * message.
	 *
	 * @return				fragment ID
	 */
	u_int16_t (*get_id)(fragment_payload_t *this);

	/**
	 * Get the fragment number. Defines the order of the fragments.
	 *
	 * @return				fragment number
	 */
	u_int8_t (*get_number)(fragment_payload_t *this);

	/**
	 * Check if this is the last fragment.
	 *
	 * @return				TRUE if this is the last fragment
	 */
	bool (*is_last)(fragment_payload_t *this);

	/**
	 * Get the fragment data.
	 *
	 * @return				chunkt to internal fragment data
	 */
	chunk_t (*get_data)(fragment_payload_t *this);

	/**
	 * Destroys an fragment_payload_t object.
	 */
	void (*destroy)(fragment_payload_t *this);
};

/**
 * Creates an empty fragment_payload_t object.
 *
 * @return			fragment_payload_t object
 */
fragment_payload_t *fragment_payload_create();

/**
 * Creates a fragment payload from the given data.  All fragments currently
 * have the same fragment ID (1), which seems what other implementations are
 * doing.
 *
 * @param num		fragment number (first one should be 1)
 * @param last		TRUE to indicate that this is the last fragment
 * @param data		fragment data (gets cloned)
 * @return			fragment_payload_t object
 */
fragment_payload_t *fragment_payload_create_from_data(u_int8_t num, bool last,
													  chunk_t data);

#endif /** FRAGMENT_PAYLOAD_H_ @}*/
