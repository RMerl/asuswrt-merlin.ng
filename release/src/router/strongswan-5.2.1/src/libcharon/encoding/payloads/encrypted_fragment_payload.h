/*
 * Copyright (C) 2014 Tobias Brunner
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
 * @defgroup encrypted_fragment_payload encrypted_fragment_payload
 * @{ @ingroup payloads
 */

#ifndef ENCRYPTED_FRAGMENT_PAYLOAD_H_
#define ENCRYPTED_FRAGMENT_PAYLOAD_H_

typedef struct encrypted_fragment_payload_t encrypted_fragment_payload_t;

#include <encoding/payloads/encrypted_payload.h>

/**
 * The Encrypted Fragment Payload as described in RFC 7383
 *
 * The implementation is located in encrypted_payload.c as it is very similar.
 */
struct encrypted_fragment_payload_t {

	/**
	 * Implements payload_t interface.
	 */
	encrypted_payload_t encrypted;

	/**
	 * Get the fragment number.
	 *
	 * @return			fragment number
	 */
	u_int16_t (*get_fragment_number)(encrypted_fragment_payload_t *this);

	/**
	 * Get the total number of fragments.
	 *
	 * @return			total number of fragments
	 */
	u_int16_t (*get_total_fragments)(encrypted_fragment_payload_t *this);

	/**
	 * Get the (decrypted) content of this payload.
	 *
	 * @return			internal payload data
	 */
	chunk_t (*get_content)(encrypted_fragment_payload_t *this);

	/**
	 * Destroys an encrypted_fragment_payload_t object.
	 */
	void (*destroy)(encrypted_fragment_payload_t *this);
};

/**
 * Creates an empty encrypted_fragment_payload_t object.
 *
 * @return			encrypted_fragment_payload_t object
 */
encrypted_fragment_payload_t *encrypted_fragment_payload_create();

/**
 * Creates an encrypted fragment payload from the given data.
 *
 * @param num		fragment number (first one should be 1)
 * @param total		total number of fragments
 * @param data		fragment data (gets cloned)
 * @return			encrypted_fragment_payload_t object
 */
encrypted_fragment_payload_t *encrypted_fragment_payload_create_from_data(
								u_int16_t num, u_int16_t total, chunk_t data);

#endif /** ENCRYPTED_FRAGMENT_PAYLOAD_H_ @}*/
